#ifndef _ESP8266_H
#define _ESP8266_H

#ifdef __cplusplus
extern "c"
{
#endif

#include "at_device.h"
#include "at_socket.h"

PAT_Device get_At_Device(void);
int esp8266_init(char *uart_dev);
int esp8266_connect_ap(char *ssid, char *passwd);

int esp8266_socket(int domain, int type, int protocol);
int esp8266_bind(int socket, const struct sockaddr *name, socklen_t namelen);
int esp8266_listen(int socket, int backlog);
int esp8266_accept(int socket, struct sockaddr *name, socklen_t *namelen);
int esp8266_connect(int socket, const struct sockaddr *name, socklen_t namelen);
int esp8266_sendto(int socket, const void *data, size_t size, int flags, const struct sockaddr *to, socklen_t tolen);
int esp8266_recvfrom(int socket, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen);
int esp8266_closesocket(int socket);


#ifdef __cplusplus
}
#endif

#endif