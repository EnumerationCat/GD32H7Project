#include "file.h"

#include <string.h>

/* 接收数据先落在 AXI SRAM，由命令层/后续 flash 命令处理。
   放在 .bss，链接脚本会自动放在栈/堆下方 */
static unsigned char g_file_buf[ZMODEM_RAM_SIZE];

static struct stat g_rcv_st;

int fileno(FILE *stream)
{
	(void)stream;
	return 123;
}

int fstat(int fd, struct stat *statbuf)
{
	(void)fd;
	*statbuf = g_rcv_st;
	return 0;
}

FILE *popen(const char *command, const char *type)
{
	(void)command;
	(void)type;
	return (FILE *)1;
}

int pclose(FILE *stream)
{
	(void)stream;
	return 0;
}

unsigned int sleep(unsigned int seconds)
{
	(void)seconds;
	return 0;
}

FILE *fopen2(const char *pathname, const char *mode)
{
	(void)pathname;
	(void)mode;

	g_rcv_st.datas   = g_file_buf;
	g_rcv_st.offset  = 0;
	g_rcv_st.st_size = 0;
	return (FILE *)1;
}

/* 与 fwrite 语义一致：成功返回 nmemb，超出缓冲区返回 0 */
size_t fwrite2(const void *ptr, size_t size, size_t nmemb,
			  FILE *stream)
{
	size_t bytes = size * nmemb;
	unsigned char *pdata = (unsigned char *)ptr;

	(void)stream;

	if (bytes == 0)
		return nmemb;
	if ((pdata == NULL) || (bytes > ZMODEM_RAM_SIZE - g_rcv_st.offset))
		return 0;

	memcpy(&g_rcv_st.datas[g_rcv_st.offset], pdata, bytes);
	g_rcv_st.offset += (unsigned int)bytes;
	g_rcv_st.st_size = g_rcv_st.offset;
	return nmemb;
}

int putc2(int c, FILE *stream)
{
	(void)stream;

	if (g_rcv_st.offset >= ZMODEM_RAM_SIZE)
		return -1;

	g_rcv_st.datas[g_rcv_st.offset++] = (unsigned char)c;
	g_rcv_st.st_size = g_rcv_st.offset;
	return c;
}

unsigned char *zmodem_file_buffer(void)
{
	return g_file_buf;
}

size_t zmodem_file_capacity(void)
{
	return ZMODEM_RAM_SIZE;
}

size_t zmodem_file_size(void)
{
	return (size_t)g_rcv_st.st_size;
}
