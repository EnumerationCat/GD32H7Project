#include <stddef.h>

#include "boot_cmd.h"
#include "bootloader.h"
#include "shell.h"
#include "string.h"

static int boot_parse_u32(const char *s, uint32_t *out)
{
	uint32_t value = 0U;
	uint32_t base = 10U;

	if((NULL == s) || ('\0' == *s)) {
		return 0;
	}

	if(('0' == s[0]) && (('x' == s[1]) || ('X' == s[1]))) {
		base = 16U;
		s += 2;
	}
	if('\0' == *s) {
		return 0;
	}

	while('\0' != *s) {
		uint32_t digit;
		char c = *s++;

		if((c >= '0') && (c <= '9')) {
			digit = (uint32_t)(c - '0');
		} else if((16U == base) && (c >= 'a') && (c <= 'f')) {
			digit = (uint32_t)(c - 'a') + 10U;
		} else if((16U == base) && (c >= 'A') && (c <= 'F')) {
			digit = (uint32_t)(c - 'A') + 10U;
		} else {
			return 0;
		}

		if(digit >= base) {
			return 0;
		}
		value = (value * base) + digit;
	}

	*out = value;
	return 1;
}

static int boot_request_slot(int slot)
{
	uint32_t base = slot ? APP_SLOT_B_ADDR : APP_SLOT_A_ADDR;
	uint32_t end  = slot ? APP_SLOT_B_END  : APP_SLOT_A_END;
	uint32_t size = slot ? APP_SLOT_B_SIZE : APP_SLOT_A_SIZE;
	boot_image_info_t info;

	if(!bootloader_image_parse(base, size, base, end, 1, &info)) {
		shell_putstr("[boot] slot ");
		shell_putstr(slot ? "B" : "A");
		shell_putstr(" image invalid\r\n");
		return 0;
	}

	shell_putstr("[boot] slot ");
	shell_putstr(slot ? "B" : "A");
	shell_putstr(" image=");
	shell_puthex32(base);
	shell_putstr(" load=");
	shell_puthex32(info.load);
	shell_putstr(" ep=");
	shell_puthex32(info.ep);
	shell_putstr(" -> jump\r\n");

	boot_set_target(info.load);
	return 1;
}

/* addr 可以是 uImage 头地址，也可以直接是向量表/load 地址 */
static int boot_request_addr(uint32_t addr)
{
	uint32_t base, end;
	boot_image_info_t info;
	uint32_t sp, pc;

	if((addr >= APP_SLOT_A_ADDR) && (addr < APP_SLOT_A_END)) {
		base = APP_SLOT_A_ADDR;
		end  = APP_SLOT_A_END;
	} else if((addr >= APP_SLOT_B_ADDR) && (addr < APP_SLOT_B_END)) {
		base = APP_SLOT_B_ADDR;
		end  = APP_SLOT_B_END;
	} else {
		shell_putstr("[boot] address is not inside app slot A/B\r\n");
		return 0;
	}

	if(bootloader_image_parse(addr, end - addr, base, end, 1, &info)) {
		shell_putstr("[boot] image=");
		shell_puthex32(addr);
		shell_putstr(" load=");
		shell_puthex32(info.load);
		shell_putstr(" ep=");
		shell_puthex32(info.ep);
		shell_putstr(" -> jump\r\n");
		boot_set_target(info.load);
		return 1;
	}

	/* 没有 uImage 头时，把 addr 当作向量表地址直接跳 */
	if((addr + 8U) > end) {
		shell_putstr("[boot] invalid address\r\n");
		return 0;
	}
	sp = *(volatile uint32_t *)addr;
	pc = *(volatile uint32_t *)(addr + 4U);
	if((sp < APP_SRAM_START) || (sp > APP_SRAM_END) ||
	   (pc < base) || (pc >= end) || (0U == (pc & 1U))) {
		shell_putstr("[boot] no valid uImage or vector table at address\r\n");
		return 0;
	}

	shell_putstr("[boot] vector table load=");
	shell_puthex32(addr);
	shell_putstr(" -> jump\r\n");
	boot_set_target(addr);
	return 1;
}

/* 返回 1 告诉 shell() 退出主循环，由 bootloader 跳转；失败留在 shell */
static int boot_f(int argc, char **argv)
{
	uint32_t addr;

	if(1 == argc) {
		/* 本次会话默认跳 A 槽；下次上电由配置扇区决定 */
		return boot_request_slot(0);
	}

	if(2 == argc) {
		if((0 == strcmp(argv[1], "a")) || (0 == strcmp(argv[1], "A"))) {
			return boot_request_slot(0);
		}
		if((0 == strcmp(argv[1], "b")) || (0 == strcmp(argv[1], "B"))) {
			return boot_request_slot(1);
		}
		if(boot_parse_u32(argv[1], &addr)) {
			return boot_request_addr(addr);
		}
	}

	shell_putstr("[boot] usage: boot [a|b|0xADDR]\r\n");
	return 0;
}

struct command boot_cmd = {
	"boot",

	"exit shell and jump to App slot A\r\n",

	"Usage: \r\n"
	"       boot,        jump to slot A\r\n"
	"       boot a,      jump to slot A\r\n"
	"       boot b,      jump to slot B\r\n"
	"       boot 0xADDR, jump to uImage/vector table at address\r\n",

	boot_f,
};

struct command jump_cmd = {
	"jump",

	"alias of boot\r\n",

	"Usage: \r\n"
	"       jump [a|b|0xADDR], alias of boot\r\n",

	boot_f,
};

struct command exit_cmd = {
	"exit",

	"alias of boot\r\n",

	"Usage: \r\n"
	"       exit [a|b|0xADDR], alias of boot\r\n",

	boot_f,
};
