#include "boot_flash.h"
#include "bootloader.h"

#include <string.h>

#include "gd32h7xx_fmc.h"

#define BOOT_CFG_MAGIC  0x42444347U  /* "GCDB" */

typedef struct boot_cfg {
	uint32_t magic;
	uint32_t active_slot;
	uint32_t inv_slot;
	uint32_t reserved;
} boot_cfg_t;

_Static_assert(sizeof(boot_cfg_t) == 16U, "boot_cfg_t must be 16 bytes");

/* FMC 错误标志是粘滞的，上一次异常会卡住后续所有擦写 */
static void fmc_clear_error_flags(void)
{
	fmc_flag_clear(FMC_FLAG_END);
	fmc_flag_clear(FMC_FLAG_WPERR);
	fmc_flag_clear(FMC_FLAG_PGSERR);
	fmc_flag_clear(FMC_FLAG_RPERR);
	fmc_flag_clear(FMC_FLAG_RSERR);
	fmc_flag_clear(FMC_FLAG_ECCCOR);
	fmc_flag_clear(FMC_FLAG_ECCDET);
	fmc_flag_clear(FMC_FLAG_OBMERR);
}

int boot_flash_erase(uint32_t addr, uint32_t len)
{
	uint32_t start;
	uint32_t end;

	if(0U == len) {
		return 1;
	}

	start = addr & ~(BOOT_FLASH_SECTOR_SIZE - 1U);
	end   = (addr + len + BOOT_FLASH_SECTOR_SIZE - 1U) & ~(BOOT_FLASH_SECTOR_SIZE - 1U);
	if((end < start) || (end > FLASH_END_ADDR)) {
		return 0;
	}

	fmc_unlock();
	for(uint32_t sector = start; sector < end; sector += BOOT_FLASH_SECTOR_SIZE) {
		fmc_clear_error_flags();
		if(FMC_READY != fmc_sector_erase(sector)) {
			fmc_lock();
			return 0;
		}
	}
	fmc_clear_error_flags();
	fmc_lock();
	return 1;
}

int boot_flash_write(uint32_t addr, const void *data, uint32_t len)
{
	const uint8_t *p = (const uint8_t *)data;
	uint32_t off = 0U;

	if(0U == len) {
		return 1;
	}
	if((NULL == data) || (0U != (addr & 3U))) {
		return 0;
	}

	fmc_unlock();
	while(off < len) {
		uint32_t word = 0xFFFFFFFFU;

		/* 尾部不足 4 字节时高位保持 0xFF，与擦除后状态一致 */
		for(uint32_t b = 0U; (b < 4U) && ((off + b) < len); b++) {
			word &= ~(0xFFU << (8U * b));
			word |= ((uint32_t)p[off + b]) << (8U * b);
		}

		/* 擦除后即为 0xFF，跳过可少编程 uImage 头与数据间的填充区 */
		if(0xFFFFFFFFU != word) {
			fmc_clear_error_flags();
			if(FMC_READY != fmc_word_program(addr + off, word)) {
				fmc_lock();
				return 0;
			}
		}
		off += 4U;
	}
	fmc_clear_error_flags();
	fmc_lock();
	return 1;
}

int boot_flash_write_image(uint32_t slot_base, const uint8_t *data, uint32_t len)
{
	if((NULL == data) || (len < IMAGE_HEADER_SIZE)) {
		return 0;
	}
	if((slot_base < APP_SLOT_A_ADDR) || (len > (FLASH_END_ADDR - slot_base))) {
		return 0;
	}

	if(!boot_flash_erase(slot_base, len)) {
		return 0;
	}

	/* 先写 payload，最后写 uImage 头：中途掉电时头无效，不会误启动残缺镜像 */
	if(len > IMAGE_HEADER_SIZE) {
		if(!boot_flash_write(slot_base + IMAGE_HEADER_SIZE,
		                     data + IMAGE_HEADER_SIZE,
		                     len - IMAGE_HEADER_SIZE)) {
			return 0;
		}
	}
	if(!boot_flash_write(slot_base, data, IMAGE_HEADER_SIZE)) {
		return 0;
	}

	return (0 == memcmp((const void *)slot_base, data, len)) ? 1 : 0;
}

int boot_cfg_get_active_slot(void)
{
	const boot_cfg_t *cfg = (const boot_cfg_t *)BOOT_CFG_ADDR;

	if((BOOT_CFG_MAGIC == cfg->magic) &&
	   (cfg->active_slot <= 1U) &&
	   (cfg->inv_slot == ~cfg->active_slot)) {
		return (int)cfg->active_slot;
	}
	return 0;
}

int boot_cfg_set_active_slot(int slot)
{
	boot_cfg_t cfg;

	if((slot != 0) && (slot != 1)) {
		return 0;
	}
	if(boot_cfg_get_active_slot() == slot) {
		return 1;
	}

	cfg.magic       = BOOT_CFG_MAGIC;
	cfg.active_slot = (uint32_t)slot;
	cfg.inv_slot    = ~(uint32_t)slot;
	cfg.reserved    = 0U;

	if(!boot_flash_erase(BOOT_CFG_ADDR, sizeof(cfg))) {
		return 0;
	}
	if(!boot_flash_write(BOOT_CFG_ADDR, &cfg, sizeof(cfg))) {
		return 0;
	}
	return (0 == memcmp((const void *)BOOT_CFG_ADDR, &cfg, sizeof(cfg))) ? 1 : 0;
}
