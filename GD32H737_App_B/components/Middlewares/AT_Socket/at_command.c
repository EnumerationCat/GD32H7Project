#include "at_command.h"
#include "at_device.h"
#include "at_socket.h"




static void at_reset_resp(PAT_Device ptDev)
{
    ptDev->resp_len = 0;
    ptDev->resp_line_counts = 0;
    ptDev->resp_status = 0;
}

int at_exec_cmd(PAT_Device ptDev, int8_t *cmd, uint8_t *resp, uint32_t max_len, uint32_t *resp_len, uint32_t timeout)
{
    
    PUART_Device ptUARTDev = ptDev->ptUARTDev;
    int ret = -1;

    // 获得锁
    xSemaphoreTake(ptDev->at_lock, portMAX_DELAY);

    at_reset_resp(ptDev);

    // 通过串口发送AT命令
    ptUARTDev->Send(ptUARTDev, cmd, strlen(cmd), timeout);

    // 等待信号量(后台任务得到返回结果会释放信号量)
    if (pdTRUE == xSemaphoreTake(ptDev->at_resp_sem, timeout)) {
        if (resp) {
            *resp_len = ptDev->resp_len > max_len ? max_len : ptDev->resp_len ;
            memcpy(resp, ptDev->resp, *resp_len);
        }
        ret = ptDev->resp_status;
    }

    // 释放锁
    xSemaphoreGive(ptDev->at_lock);

    return ret;
}

int at_send_datas(PAT_Device ptDev, uint8_t *datas, uint32_t data_len, uint32_t timeout)
{
    PUART_Device ptUARTDev = ptDev->ptUARTDev;
    int ret = -1;

    // 获得锁
    xSemaphoreTake(ptDev->at_lock, portMAX_DELAY);

    // 通过串口发送AT命令
    ret = ptUARTDev->Send(ptUARTDev, datas, data_len, timeout);

    // 释放锁
    xSemaphoreGive(ptDev->at_lock);

    return ret;
}