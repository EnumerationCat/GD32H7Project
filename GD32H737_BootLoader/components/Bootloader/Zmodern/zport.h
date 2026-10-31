#ifndef LIBZMODEM_ZPORT_H
#define LIBZMODEM_ZPORT_H

#include <stddef.h>
#include <stdint.h>

/* 底层串口/计时适配，供 lrzsz 代码调用。
   返回值 -2 与 ZMODEM 的 TIMEOUT 一致。 */
#define ZPORT_TIMEOUT (-2)

int  zport_getc(int timeout_ms);          /* 阻塞至超时；返回字节或 ZPORT_TIMEOUT */
void zport_putc(int c);
void zport_write(const void *buf, size_t len);
void zport_flush(void);

/* 原韦东山 uart.h 的接口名，lrzsz 内部直接调用 */
void putdatas(const char *data, size_t len);

#endif
