
#include "at_device.h"
#include "at_socket.h"




#define SERVER_IP "192.168.1.26"
#define SERVER_PORT 8888



//#define LOG_INFO(fmt, ...)  printf("%s:%s:%d: " fmt, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
/*argv[0]         argv[1]*/
/*Socket_Client 127.0.0.1*/
void tcp_client_test_task(void *arg) {


	at_init("stm32_uart1");

	while(1)
	{
		if(0 ==	at_connect_ap("Wi-Fi","chuang123."))
		{
			break;
		}
		vTaskDelay(1000);

	}




	int iSocketClient;
	struct sockaddr_in tSocketServerAddr;
	

	iSocketClient = socket(AF_INET, SOCK_STREAM, 0);


	tSocketServerAddr.sin_family = AF_INET;
	tSocketServerAddr.sin_port = htons(SERVER_PORT);
	int res = inet_pton(tSocketServerAddr.sin_family,SERVER_IP, &(tSocketServerAddr.sin_addr));

	if(res <= 0)
	{
		return;
	}


	memset(tSocketServerAddr.sin_zero, 0, 8);


	int iRet = connect(iSocketClient, (struct sockaddr*)&tSocketServerAddr, sizeof(struct sockaddr));

	if(-1 == iRet)
	{
		return;
	}



	while(1)
	{
		char ucSendBuf[1000];
		if(fgets((char*)ucSendBuf, 999, stdin))
		{
			send(iSocketClient, (char*)ucSendBuf, (int)strlen((char*)ucSendBuf), 0);
		}
		else
		{
			close(iSocketClient);
			return;
		}
	}

	return;
	
}