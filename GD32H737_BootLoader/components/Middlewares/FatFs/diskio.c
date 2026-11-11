/*-----------------------------------------------------------------------*/
/* Low level disk I/O module for FatFs on GD32H737                        */
/*                                                                       */
/*   pdrv 0 : on-board SPI NOR flash (W25Qxx, driven through OSPI)       */
/*   pdrv 1 : SD card (driven through SDIO)                              */
/*                                                                       */
/* Mount as "0:" (NOR) and "1:" (SD).                                    */
/*-----------------------------------------------------------------------*/

#include "ff.h"
#include "diskio.h"

#include "sys.h"
#include "sd_conf.h"
#include "norflash.h"

#define DEV_NOR     0U
#define DEV_SD      1U

#define NOR_SECTOR_SIZE     512U
#define NOR_ERASE_SIZE      4096U
#define SD_SECTOR_SIZE      512U

static uint8_t nor_ready = 0;
static uint8_t sd_ready = 0;

/* Capacity of the NOR flash, in bytes, derived from the JEDEC ID */
static uint32_t nor_capacity_bytes(void)
{
    switch (g_norflash_type) {
        case W25Q80:    return 1U * 1024U * 1024U;
        case W25Q16:    return 2U * 1024U * 1024U;
        case W25Q32:    return 4U * 1024U * 1024U;
        case W25Q64:    return 8U * 1024U * 1024U;
        case W25Q128:   return 16U * 1024U * 1024U;
        case W25Q256:   return 32U * 1024U * 1024U;
        case BY25Q64:   return 8U * 1024U * 1024U;
        case BY25Q128:  return 16U * 1024U * 1024U;
        case BY25Q256:  return 32U * 1024U * 1024U;
        case NM25Q64:   return 8U * 1024U * 1024U;
        case NM25Q128:  return 16U * 1024U * 1024U;
        default:        return 0U;
    }
}

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/
DSTATUS disk_status(BYTE pdrv)
{
    switch (pdrv) {
        case DEV_NOR:
            if (!nor_ready) {
                return STA_NOINIT;
            }
            if (nor_capacity_bytes() == 0U) {
                return STA_NOINIT | STA_NODISK;
            }
            return 0;

        case DEV_SD:
            return sd_ready ? 0 : STA_NOINIT;

        default:
            return STA_NOINIT;
    }
}

/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */
/*-----------------------------------------------------------------------*/
DSTATUS disk_initialize(BYTE pdrv)
{
    switch (pdrv) {
        case DEV_NOR:
            norflash_init();                    /* also refreshes g_norflash_type */
            if (nor_capacity_bytes() == 0U) {
                nor_ready = 0;
                return STA_NOINIT | STA_NODISK;
            }
            nor_ready = 1;
            return 0;

        case DEV_SD:
            if (sdio_sd_init() != SD_OK) {
                sd_ready = 0;
                return STA_NOINIT;
            }
            sd_ready = 1;
            return 0;

        default:
            return STA_NOINIT;
    }
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/
DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count)
{
    if (count == 0U) {
        return RES_PARERR;
    }

    switch (pdrv) {
        case DEV_NOR:
            if (!nor_ready) {
                return RES_NOTRDY;
            }
            norflash_read(buff, (uint32_t)sector * NOR_SECTOR_SIZE, (uint32_t)count * NOR_SECTOR_SIZE);
            return RES_OK;

        case DEV_SD:
            if (!sd_ready) {
                return RES_NOTRDY;
            }
            return (sd_read_disk(buff, (uint32_t)sector, count) == SD_OK) ? RES_OK : RES_ERROR;

        default:
            return RES_PARERR;
    }
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
#if FF_FS_READONLY == 0
DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count)
{
    if (count == 0U) {
        return RES_PARERR;
    }

    switch (pdrv) {
        case DEV_NOR:
            if (!nor_ready) {
                return RES_NOTRDY;
            }
            /* norflash_write() erases the 4K sector first (read-modify-write) */
            norflash_write((uint8_t *)buff, (uint32_t)sector * NOR_SECTOR_SIZE, (uint32_t)count * NOR_SECTOR_SIZE);
            return RES_OK;

        case DEV_SD:
            if (!sd_ready) {
                return RES_NOTRDY;
            }
            return (sd_write_disk((uint8_t *)buff, (uint32_t)sector, count) == SD_OK) ? RES_OK : RES_ERROR;

        default:
            return RES_PARERR;
    }
}
#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/
DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
    switch (pdrv) {
        case DEV_NOR:
            if (!nor_ready) {
                return RES_NOTRDY;
            }
            switch (cmd) {
                case CTRL_SYNC:
                    norflash_wait_busy();
                    return RES_OK;

                case GET_SECTOR_COUNT:
                    *(DWORD *)buff = nor_capacity_bytes() / NOR_SECTOR_SIZE;
                    return RES_OK;

                case GET_SECTOR_SIZE:
                    *(WORD *)buff = NOR_SECTOR_SIZE;
                    return RES_OK;

                case GET_BLOCK_SIZE:
                    *(DWORD *)buff = NOR_ERASE_SIZE / NOR_SECTOR_SIZE;
                    return RES_OK;

                default:
                    return RES_PARERR;
            }

        case DEV_SD:
            if (!sd_ready) {
                return RES_NOTRDY;
            }
            switch (cmd) {
                case CTRL_SYNC:
                    return RES_OK;

                case GET_SECTOR_COUNT: {
                    uint32_t sectors = sd_cardinfo.card_capacity / SD_SECTOR_SIZE;
                    if (sectors == 0U) {
                        sectors = sd_card_capacity_get() * 2U;   /* KB -> 512B sectors */
                    }
                    if (sectors == 0U) {
                        return RES_ERROR;
                    }
                    *(DWORD *)buff = sectors;
                    return RES_OK;
                }

                case GET_SECTOR_SIZE:
                    *(WORD *)buff = SD_SECTOR_SIZE;
                    return RES_OK;

                case GET_BLOCK_SIZE:
                    *(DWORD *)buff = 1U;
                    return RES_OK;

                default:
                    return RES_PARERR;
            }

        default:
            return RES_PARERR;
    }
}
