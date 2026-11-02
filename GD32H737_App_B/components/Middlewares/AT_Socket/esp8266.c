#include "esp8266.h"
#include "portmacro.h"
#include "projdefs.h"
#include "uart_device.h"
#include "at_device.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/_intsup.h>

#include "at_socket.h"
#include "at_command.h"


#define ESP8266PARSER_TASK_STACK 1024
#define ESP8266PARSER_TASK_PRIORITY 4
TaskHandle_t esp8266_parserHandle;
static void esp8266_parser(void * pvParameters);



static PAT_Socket get_socket_for_hw_socket(int hw_socket)
{
    PAT_Device ptDev =  get_At_Device();

    for(int i = 0;i < AT_DEVICE_SOCKETS_NUM; i++)
    {
        if(ptDev->sockets[i].used && (int)ptDev->sockets[i].user_data == hw_socket)
        {
            return &ptDev->sockets[i];
        }
    }
    return NULL;
}


static void esp8266_recv_packet(PAT_Device ptDev)
{
    struct UART_Device *ptUARTDev = ptDev->ptUARTDev;
    uint8_t data;
    int hw_socket = 0;
    int len = 0;
    int state = 0; // 0:等待hw_socket, 1:等待len, 2:读取数据
    int received = 0;
    PAT_Socket ptSocket = NULL;

    // +IPD,<link id>,<len>:<data>
    while (1) {
        // 1. 读取串口数据
        if (0 != ptUARTDev->Recv(ptUARTDev, &data, portMAX_DELAY)) {
            return;
        }

        switch (state) {
            case 0: // 解析hw_socket
                if (data == ',') {
                    state = 1; // 切换到解析len状态

                    // 根据hw_socket找到对应的PAT_Socket
                    ptSocket = get_socket_for_hw_socket(hw_socket);
                    
                    if (!ptSocket) {
                        //return; // 找不到对应的socket
                    }
                }
                continue;

                if (data >= '0' && data <= '9') {
                    hw_socket = hw_socket * 10 + (data - '0');
                }
                break;

            case 1: // 解析len
                if (data == ':') {
                    state = 2; // 切换到读取数据状态
                    continue;
                }
                if (data >= '0' && data <= '9') {
                    len = len * 10 + (data - '0');
                }
                break;

            case 2: // 读取数据
                if (received < len) {
                    if(ptSocket)
                    {
                        xQueueSend(ptSocket->recv_queue, &data, 0);
                    }
                    
                }
                received++;
                if (received >= len) {
                    // 释放信号量通知有新数据
                    if(ptSocket)
                    {
                        xSemaphoreGive(ptSocket->at_packet_sem);
                    }
                    
                    return;
                }
                break;
        }
    }
}
// typedef struct AT_Device {
// 	char *name; /* WIFI模块的名字 */
// 	SemaphoreHandle_t at_lock;      /* 发送AT命令前需要先获得这个锁 */
// 	SemaphoreHandle_t at_resp_sem;  /* 发送AT命令后等待这个信号量(等待AT命令的回应) */
// 	uint8_t resp[AT_RESP_BUF_SIZE]; /* 存放AT命令的回应数据 */ 
// 	uint32_t resp_len;              /* AT命令回应数据的长度 */
// 	uint32_t resp_line_counts;      /* AT命令回应的数据有多少行 */
// 	uint32_t resp_status;           /* AT命令的回应是OK还是ERR */
// 	PUART_Device ptUARTDev;         /* 使用这个串口设备访问WIFI模块 */
// 	AT_Socket sockets[AT_DEVICE_SOCKETS_NUM]; /* socket结构体数组 */
// }AT_Device, *PAT_Device;

static AT_Device g_esp8266_device = {
    .name = "esp8266",
    .at_lock = NULL,
    .at_resp_sem = NULL,
    .ptUARTDev = NULL,
    .resp = {0},
    .resp_len = 0


};


PAT_Device get_At_Device(void)
{
    return &g_esp8266_device;
}


int esp8266_init(char *uart_dev)
{
    g_esp8266_device.ptUARTDev = GetUARTDevice(uart_dev);
    
    if(NULL == g_esp8266_device.ptUARTDev)
    {
        return -1;
    }

    g_esp8266_device.ptUARTDev->Init(g_esp8266_device.ptUARTDev,115200,8,'N',1);


    //初始化互斥量
    g_esp8266_device.at_lock = xSemaphoreCreateMutex();
    if(NULL == g_esp8266_device.at_lock)
    {
        return -1;
    }

    //初始化信号量
    g_esp8266_device.at_resp_sem = xSemaphoreCreateBinary();
    if(NULL == g_esp8266_device.at_resp_sem)
    {
        vSemaphoreDelete(g_esp8266_device.at_lock);
        return -1;
    }

    //创建后台任务
    BaseType_t ret = xTaskCreate((TaskFunction_t)esp8266_parser,
        (char *) "esp8266_parser",
        (configSTACK_DEPTH_TYPE)ESP8266PARSER_TASK_STACK,
        &g_esp8266_device,
        (UBaseType_t)ESP8266PARSER_TASK_PRIORITY,
        (TaskHandle_t *)&esp8266_parserHandle);

    if(pdPASS != ret)
    {
        return -1;
    }

    //复位esp8266
    at_exec_cmd(&g_esp8266_device,"AT+RST\r\n", NULL, 0, NULL, AT_TIMEOUT);

    vTaskDelay(2000);

    //关闭回显
    at_exec_cmd(&g_esp8266_device,"ATE0\r\n", NULL, 0, NULL, AT_TIMEOUT);

    return  0;
}

int esp8266_connect_ap(char *ssid, char *passwd)
{
    PAT_Device ptDev = get_At_Device();
    char cmd[128];

    // 1. 设置WiFi模式为STA
    if (at_exec_cmd(ptDev, (int8_t*)"AT+CWMODE=1\r\n", NULL, 0, NULL, AT_TIMEOUT)) {
        return -1;
    }

    // 2. 连接AP
    if (passwd) {
        snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, passwd);
    } else {
        snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"\"\r\n", ssid);
    }

    if (at_exec_cmd(ptDev, cmd, NULL, 0, NULL, 10000)) { // 10秒超时
        return -1;
    }

    // 3. 查询IP地址(可选)
    if (at_exec_cmd(ptDev, (int8_t*)"AT+CIFSR\r\n", NULL, 0, NULL, AT_TIMEOUT)) {
        return -1;
    }

    return 0;
	
}





int esp8266_socket(int domain, int type, int protocol)
{
    int index;
    for(index = 0; index < AT_DEVICE_SOCKETS_NUM; index++)
    {
        if(g_esp8266_device.sockets[index].used == 0)
        {
            break;
        }

    }

    if(index >= AT_DEVICE_SOCKETS_NUM)
    {
        return -1;
    }

    g_esp8266_device.sockets[index].used = 1;
    g_esp8266_device.sockets[index].type = type;


    
    if(NULL == g_esp8266_device.sockets[index].at_packet_sem)
    {
        g_esp8266_device.sockets[index].at_packet_sem = xSemaphoreCreateBinary();
        
        if(NULL == g_esp8266_device.sockets[index].at_packet_sem)
        {
            return -1;
        }
    }


    if(NULL == g_esp8266_device.sockets[index].recv_queue)
    {
        g_esp8266_device.sockets[index].recv_queue = xQueueCreate(AT_RECV_BUF_SIZE,sizeof(uint8_t*));
        if(NULL == g_esp8266_device.sockets[index].recv_queue)
        {
            return -1;
        }
    }

    return index;
}


int esp8266_bind(int socket, const struct sockaddr *name, socklen_t namelen)
{
    PAT_Device ptDev = get_At_Device();
    PAT_Socket ptSocket = &ptDev->sockets[socket];

    ptSocket->local = *name;

    return 0;



}



int esp8266_listen(int socket, int backlog)
{
    PAT_Device ptDev  =  get_At_Device();
    PAT_Socket ptSocket = &ptDev->sockets[socket];


    //AT+CIPMUX=1 使能多连接
    const char *cmd = "AT+CIPMUX=1\r\n";
    if(at_exec_cmd(ptDev, (int8_t *)cmd, NULL, 0, NULL, AT_TIMEOUT))
    {
        return -1;

    }

    //AT+CIPSERVERMAXCONN=4  允许最大连接
    cmd = "AT+CIPSERVERMAXCONN=4\r\n";
    at_exec_cmd(ptDev, (int8_t *)cmd, NULL, 0, NULL, AT_TIMEOUT);

    //AT+CIPSERVER=1,888 建立 服务器监听端口
    char cmdbuff[128];
    struct sockaddr_in *ptAddr = (struct sockaddr_in*)&ptSocket->local;
    uint16_t port = ntohs(ptAddr->sin_port);
    snprintf(cmdbuff, sizeof(cmdbuff),"AT+CIPSERVER=1,%d\r\n",port);

    if(at_exec_cmd(ptDev, (int8_t *)cmdbuff, NULL, 0, NULL, AT_TIMEOUT))
    {
        return -1;

    }

    return 0;
}




int esp8266_accept(int socket, struct sockaddr *name, socklen_t *namelen)
{
    PAT_Device ptDev = get_At_Device();
    PAT_Socket ptServerSock = &ptDev->sockets[socket];
    struct sockaddr_in *ptAddr = (struct sockaddr_in *)&ptServerSock->local;
    uint16_t server_port = ntohs(ptAddr->sin_port);

    // 修正符号警告：字面量强转int8_t*
    int8_t *cmd = (int8_t *)"AT+CIPSTATUS=1\r\n";
    if (at_exec_cmd(ptDev, cmd, NULL, 0, NULL, AT_TIMEOUT))
    {
        return -1;
    }

    // 无有效返回行直接退出
    if (ptDev->resp_line_counts <= 1)
    {
        return -1;
    }

    // resp整块缓冲区起始指针
    uint8_t *resp_buf = ptDev->resp;
    uint32_t offset = 0;
    uint32_t line_idx = 0;

    // 遍历所有行，不额外分配行缓存，原地操作
    while (offset < ptDev->resp_len && line_idx < ptDev->resp_line_counts)
    {
        uint8_t *p_line_start = resp_buf + offset;
        uint32_t line_len = 0;

        // 找到当前行 \r\n 换行
        while ((offset + line_len) < ptDev->resp_len)
        {
            if (resp_buf[offset + line_len] == '\r' && resp_buf[offset + line_len + 1] == '\n')
            {
                break;
            }
            line_len++;
        }

        // 跳过第一行 STATUS，只处理后续行
        if (line_idx > 0)
        {
            // 原地临时存结束符，解析完自动覆盖，不新开数组
            uint8_t old_char = p_line_start[line_len];
            p_line_start[line_len] = '\0';

            // 强制转char*用于strstr，消除无符号警告
            const char *line_str = (const char *)p_line_start;
            if (strstr(line_str, "+CIPSTATUS:") == line_str)
            {
                uint16_t hw_socket;
                char type[10];
                char remote_ip[32];
                uint16_t remote_port;
                uint16_t local_port;
                int tetype;

                int parsed = sscanf(line_str, "+CIPSTATUS:%hu,%9[^,],%31[^,],%hu,%hu,%d",
                                    &hw_socket, type, remote_ip, &remote_port, &local_port, &tetype);

                // 解析成功
                if (parsed == 6)
                {
                    // 已有对应软件socket，跳过本次连接
                    if (get_socket_for_hw_socket(hw_socket) != NULL)
                    {
                        // 恢复原字节，移动偏移继续下一行
                        p_line_start[line_len] = old_char;
                        offset += line_len + 2;
                        line_idx++;
                        continue;
                    }
                    // 不是当前服务器端口，跳过
                    if (local_port != server_port)
                    {
                        p_line_start[line_len] = old_char;
                        offset += line_len + 2;
                        line_idx++;
                        continue;
                    }

                    // 填充客户端地址
                    if (name != NULL)
                    {
                        struct sockaddr_in *addr = (struct sockaddr_in *)name;
                        addr->sin_family = AF_INET;
                        addr->sin_port = htons(remote_port);
                        inet_pton(AF_INET, remote_ip, &addr->sin_addr);
                        memset(addr->sin_zero, 0, SIN_ZERO_LEN);
                        if (namelen)
                            *namelen = sizeof(struct sockaddr_in);
                    }

                    // 创建新软件socket
                    int sw_socket = esp8266_socket(AF_INET, SOCK_STREAM, 0);
                    if (sw_socket < 0)
                    {
                        p_line_start[line_len] = old_char;
                        return -1;
                    }
                    PAT_Socket ptNewSock = &ptDev->sockets[sw_socket];
                    // 修复指针转换警告：uintptr_t中转
                    ptNewSock->user_data = (void *)(uintptr_t)hw_socket;

                    // 修复：不同结构体不能直接赋值，使用memcpy
                    if (name != NULL)
                        memcpy(&ptNewSock->remote, name, sizeof(struct sockaddr));

                    // 填充本地端口
                    struct sockaddr_in *local_addr = (struct sockaddr_in *)&ptNewSock->local;
                    local_addr->sin_family = AF_INET;
                    local_addr->sin_port = htons(local_port);
                    local_addr->sin_addr.s_addr = INADDR_ANY;
                    memset(local_addr->sin_zero, 0, SIN_ZERO_LEN);

                    p_line_start[line_len] = old_char;
                    return sw_socket;
                }
            }
            // 恢复缓冲区原始字节，不破坏resp数据
            p_line_start[line_len] = old_char;
        }

        // 跳到下一行（跳过\r\n两个字节）
        offset += line_len + 2;
        line_idx++;
    }

    return -1;
}


static int get_unused_hw_socket(void)
{
    PAT_Device ptDev = get_At_Device();
    int used_hw_sockets[5] = {0}; // ESP8266最多支持5个硬件socket(0~4)

    // 1. 遍历所有已用socket，记录已占用的hw_socket
    for (int i = 0; i < AT_DEVICE_SOCKETS_NUM; i++) {
        if (ptDev->sockets[i].used) {
            int hw_socket = (int)ptDev->sockets[i].user_data;
            if (hw_socket >= 0 && hw_socket < 5) {
                used_hw_sockets[hw_socket] = 1;
            }
        }
    }

    // 2. 从0~4中找出第一个未使用的hw_socket
    for (int i = 0; i < 5; i++) {
        if (used_hw_sockets[i] == 0) {
            return i;
        }
    }

    return -1; // 没有可用的hw_socket

}

int esp8266_connect(int socket, const struct sockaddr *name, socklen_t namelen)
{
    PAT_Device ptDev = get_At_Device();
    PAT_Socket ptSocket = &ptDev->sockets[socket];

    //AT+CIPSTART=<link ID>,<type>,<remote IP>,<remote port>
    //1.找出一个空闲的link ID
    int link_id = get_unused_hw_socket();
    if(link_id < 0)
    {
        return -1;
    }
    //2.构造AT命令
    char cmd[64] = {0};
    char ipstr[16] = {0};
    struct sockaddr_in *paddr = (struct sockaddr_in *)name;
    uint16_t port = ntohs(paddr->sin_port);
    ipaddr_to_ipstr(name, ipstr);


    if(ptSocket->type == SOCK_STREAM)
    {
        sprintf(cmd, "AT+CIPSTART=%d,%s,%s,%d\r\n", link_id, "TCP", ipstr, port);

    }else{
        sprintf(cmd, "AT+CIPSTART=%d,%s,%s,%d\r\n", link_id, "UDP", ipstr, port);
    }
    

    // 3. 发送AT命令
    if (at_exec_cmd(ptDev, (int8_t*)cmd, NULL, 0, NULL, AT_TIMEOUT)) {
        return -1;
    }

    return 0;
}



int esp8266_sendto(int socket, const void *data, size_t size, int flags, const struct sockaddr *to, socklen_t tolen)
{
    // 对于TCP连接：AT+CIPSEND=<link ID>,<length>
    // 对于UDP:    AT+CIPSEND=[<link ID>,]<length>[,<remote IP>,<remote port>]
    PAT_Device ptDev = get_At_Device();
    PAT_Socket ptSocket = &ptDev->sockets[socket];

    uint16_t hw_socket = (uint16_t)ptSocket->user_data;

    int8_t cmd[64] = {0};

    if(ptSocket->type == SOCK_STREAM)
    {
        sprintf(cmd, "AT+CIPSEND=%d,%d\r\n", hw_socket, size);
        
    }
    else if(ptSocket->type == SOCK_DGRAM && to != NULL)
    {
        char ipstr[16] = {0};
        struct sockaddr_in *paddr = (struct sockaddr_in *)to;
        uint16_t port = ntohs(paddr->sin_port);
        ipaddr_to_ipstr((const struct sockaddr *)paddr, ipstr);
        sprintf(cmd, "AT+CIPSEND=%d,%d,%s,%d\r\n", hw_socket, size, ipstr, port);
    }
    else
    {
        return -1;

    }

    if(at_exec_cmd(ptDev, cmd, NULL, 0, NULL, AT_TIMEOUT))
    {
        return -1;
    }



    if(at_send_datas(ptDev, (uint8_t *)data, size, AT_TIMEOUT))
    {
        return -1;

    }

    return 0;
}




int esp8266_recvfrom(int socket, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen)
{

    PAT_Device ptDev = get_At_Device();
    PAT_Socket ptSocket = &ptDev->sockets[socket];
    uint16_t hw_socket = (uint16_t)ptSocket->user_data;

    uint8_t data;
    uint8_t *pdata = (uint8_t*)mem;
    size_t recv_len = 0;


    // 0. 对于UDP,先发起AT命令来连接
    if (ptSocket->type == SOCK_DGRAM&& from != NULL) {
        if(esp8266_connect(socket, from, *fromlen))
        {
            return -1;
        }
    }
    //1.尝试从接收队列，读取上次遗留的数据
    while(xQueueReceive(ptSocket->recv_queue, &data, 0) == pdTRUE)
    {
        pdata[recv_len] = data;
        recv_len++;
        if(recv_len>=len)
        {
            return recv_len;
        }
    }

    if(recv_len > 0)
    {
        return recv_len;
    }


    //2.无遗留数据，等待信号量
    if(xSemaphoreTake(ptSocket->at_packet_sem, portMAX_DELAY) != pdTRUE)
    {
        return -1;
    }



    //3.唤醒后读队列
    while(xQueueReceive(ptSocket->recv_queue, &data, 0) == pdTRUE)
    {
        pdata[recv_len] = data;
        recv_len++;
        if(recv_len>=len)
        {
            return recv_len;
        }
    }

    //读取失败
    return -1;
}




static void esp8266_parser(void * pvParameters)
{

    PAT_Device ptDev = (PAT_Device)pvParameters;

    PUART_Device ptUARTDev = ptDev->ptUARTDev;

    uint8_t data;

    uint8_t line[AT_RECV_BUF_SIZE];
    int32_t len = 0;

    while(1)
    {
        // 1. 读取串口数据

        if(0 != ptUARTDev->Recv(ptUARTDev,&data,portMAX_DELAY))
        {
            continue;
        }
        line[len++] = data;
        if(len >= AT_RECV_BUF_SIZE)
        {
            len = 0;
        }

        line[len] = '\0';


        // 2. 解析是否为"网络数据"，即：是否以"+IPD,"开头
        //    若是，根据它的<link id>找到socket，并把数据写入socket的接收队列
        //    然后释放信号量
        if(strstr(line, "+IPD,"))
        {
            esp8266_recv_packet(ptDev);
            len = 0;
            continue;
        }


        //3. 执行到这里读到的数据就是AT命令的回应，（也可能是其他的状态信息，比如 0，CONNECT）
        //   存储这些多行数据
        //   解析最后的"OK\r\n"和"ERROR\r\n"

        if (data == '\n') { // 检测到换行符
            if (ptDev->resp_line_counts < AT_RESP_LINES_MAX) {
                // 保存当前行到resp数组
                memcpy(&ptDev->resp[ptDev->resp_len], line, len+1);
                ptDev->resp_line_counts++;
                ptDev->resp_len += len;
            }
            len = 0;
        }


        if (strstr(line, "OK\r\n") || strstr(line, "ERROR\r\n")) {
            if (strstr(line, "OK\r\n")) {
                ptDev->resp_status = AT_RESP_OK;
            } else {
                ptDev->resp_status = AT_RESP_ERROR;
            }
            // 释放信号量通知命令完成
            xSemaphoreGive(ptDev->at_resp_sem);
            len = 0;
        }

    }

}


int esp8266_closesocket(int socket)
{
    PAT_Device ptDev = get_At_Device();
    PAT_Socket ptSocket = &ptDev->sockets[socket];

    int hw_socket = (int)ptSocket->user_data;

    char cmd[64];
    sprintf(cmd, "AT+CIPCLOSE=%d\r\n", hw_socket);

    if (at_exec_cmd(ptDev, cmd, NULL, 0, NULL, AT_TIMEOUT))
        return -1;

    ptSocket->used = 0;
}





















