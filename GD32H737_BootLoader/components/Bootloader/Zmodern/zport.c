#include "zport.h"
#include "bootloader.h"

int zport_getc(int timeout_ms)
{
    if(timeout_ms <= 0) {
        int c = boot_getc();
        return (c < 0) ? ZPORT_TIMEOUT : c;
    }

    int c = boot_getc_timeout((uint32_t)timeout_ms);
    return (c < 0) ? ZPORT_TIMEOUT : c;
}

void zport_putc(int c)
{
    boot_putc((uint8_t)c);
}

void zport_write(const void *buf, size_t len)
{
    const uint8_t *p = (const uint8_t *)buf;

    while(len-- > 0U) {
        boot_putc(*p++);
    }
}

void zport_flush(void)
{
    /* 发送是轮询等待 TC，天然无缓冲 */
}

void putdatas(const char *data, size_t len)
{
    zport_write(data, len);
}
