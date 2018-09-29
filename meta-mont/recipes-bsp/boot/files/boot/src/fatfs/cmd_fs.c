/*=============================================================================+
|                                                                              |
| Copyright 2017                                                              |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
#include <arch/chip.h>
#include <lib.h>
#include <common.h>
#include <part.h>
#include <netprot.h>
#include <usb.h>

#include "ff.h"
#include "diskio.h"

#define DEV_USB0	0
#define DEV_USB1	1
#define DEV_MMC		2

static BYTE curr_dev;

#if defined(CONFIG_USB)

extern int dev_index;
extern int usb_stor_curr_dev;   /* current device */

int USB_disk_initialize(void)
{
    usb_stop();
    if (!usb_init())
        usb_stor_curr_dev = usb_stor_scan(1);

    return 0;
}

int USB_disk_read(void* buff, int sector, int count)
{
    block_dev_desc_t *stor_dev;

    //printf("USB_disk_read %d %d\n", sector, count);
    printf(".");
    stor_dev = usb_stor_get_dev(usb_stor_curr_dev);
    stor_dev->block_read(usb_stor_curr_dev, sector, count, buff);
    return 0;
}

int USB_disk_write(void* buff, int sector, int count)
{
    block_dev_desc_t *stor_dev;

    //printf("USB_disk_write %d %d\n", sector, count);
    printf(".");
    stor_dev = usb_stor_get_dev(usb_stor_curr_dev);
    stor_dev->block_write(usb_stor_curr_dev, sector, count, buff);
    return 0;
}

#endif

#if defined(CONFIG_MINI_SDHC)

#include <sdhci.h>
void mini_sdhc_init(void);
int mmc_bread(int dev_num, u32 start, u32 blkcnt, void *dst);
int mmc_bwrite(int dev_num, u32 start, u32 blkcnt, const void *src);
int MMC_disk_initialize(void)
{
    //printf("MMC_disk_initialize\n");
    mini_sdhc_init();
    return 0;
}

int MMC_disk_read(void* buff, int sector, int count)
{
    //printf("MMC_disk_read %d %d\n", sector, count);
    mmc_bread(0, sector, count, buff);

    return 0;
}

int MMC_disk_write(void* buff, int sector, int count)
{
    //printf("MMC_disk_write %d %d\n", sector, count);
    mmc_bwrite(0, sector, count, buff);

    return 0;
}

#endif

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat = STA_NOINIT;

    switch (curr_dev)
    {
#if defined(CONFIG_USB)
        case DEV_USB0:
        case DEV_USB1:
            stat = 0;
            break;
#endif

#if defined(CONFIG_MINI_SDHC)
        case DEV_MMC:
            stat = 0;
            break;
#endif
    }

    return stat;
}

#if defined(CONFIG_USB)
extern int usb_select_port;
extern int dev_index;
#endif
DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat = STA_NOINIT;

    switch (curr_dev)
    {
#if defined(CONFIG_USB)
        case DEV_USB0:
        case DEV_USB1:
            usb_select_port = curr_dev;
            USB_disk_initialize();
            if(dev_index < 2)
            {
                stat = STA_NODISK;
            }
            else
            {
                stat = 0;
            }
            break;
#endif

#if defined(CONFIG_MINI_SDHC)
        case DEV_MMC:
            MMC_disk_initialize();
            stat = 0;
            break;
#endif
    }

	return stat;
}

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res = RES_PARERR;

    switch (curr_dev)
    {
#if defined(CONFIG_USB)
        case DEV_USB0:
        case DEV_USB1:
            USB_disk_read((void *) buff, sector, count);
            res = 0;
            break;
#endif

#if defined(CONFIG_MINI_SDHC)
        case DEV_MMC:
            MMC_disk_read((void *) buff, sector, count);
            res = 0;
            break;
#endif
	}

    return res;
}

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
    DRESULT res = RES_PARERR;

    switch (curr_dev)
    {
#if defined(CONFIG_USB)
        case DEV_USB0:
        case DEV_USB1:
            USB_disk_write((void *) buff, sector, count);
            res = 0;
            break;
#endif

#if defined(CONFIG_MINI_SDHC)
        case DEV_MMC:
            MMC_disk_write((void *) buff, sector, count);
            res = 0;
            break;
#endif
    }

	return res;
}

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;

    switch (curr_dev)
    {
#if defined(CONFIG_USB)
        case DEV_USB0:
        case DEV_USB1:
            res = 0;
            break;
#endif

#if defined(CONFIG_MINI_SDHC)
        case DEV_MMC:
            res = 0;
            break;
#endif
	}

	return res;
}

int show_fs_device(void)
{
    printf("fs device: %s\n", (curr_dev==0) ? "usb0" : ((curr_dev==1) ? "usb1" : "mmc"));
    return 0;
}

int fs_cmd(int argc, char *argv[])
{
    char *pfname = bootvars.file;
    unsigned int src, len, rlen;
    int option = argv[-1][2];
    void *buff;
    FRESULT result;

    FATFS FatFs;
    FIL Fil;

    buff = (void *) bootvars.load_addr;
    len = byte_count;

    if(option == 'd')
    {
        if (argc > 0)
        {
            if(!strcmp(argv[0], "usb0"))
                curr_dev = 0;
            else if(!strcmp(argv[0], "usb1"))
                curr_dev = 1;
            else if(!strcmp(argv[0], "mmc"))
                curr_dev = 2;
        }
        show_fs_device();
        goto done;
    }

    if (argc > 0)
        pfname = argv[0];

    if (argc > 1)
    {
        if(sscanf(argv[1], "%x", &src) != 1)
            goto Usage;
        else
            buff = (void *) src;
    }

    if (argc > 2 && sscanf(argv[2], "%x", &len) != 1)
        goto Usage;

    show_fs_device();
    printf("filename = %s\n", pfname);
    if(option=='w')
        printf("length = 0x%x\n", len);
    printf("memory address = 0x%x\n", buff);

    if((option=='w') && (0>=len))
        goto Usage;

    switch (option)
    {
        case 'r':
            printf("Reading from %s...\n", pfname);
            f_mount(&FatFs, "", 0);
            result = f_open(&Fil, pfname, FA_READ | FA_OPEN_EXISTING);

            if(result==FR_OK)
            {
                len = f_size(&Fil);
                result = f_read(&Fil, buff, len, &rlen);

                f_close(&Fil);

                printf("\n0x%x bytes\n", len);
                byte_count = len;
            }
            else
                printf("f_open err 0x%x\n", result);

            f_mount(0, "", 0);
            break;

        case 'w':
            printf("Writing to %s...\n", pfname);
            f_mount(&FatFs, "", 0);
            result = f_open(&Fil, pfname, FA_WRITE | FA_CREATE_ALWAYS);

            if(result==FR_OK)
            {
                result = f_write(&Fil, buff, len, &rlen);

                f_close(&Fil);
            }
            else
                printf("f_open err 0x%x\n", result);

            f_mount(0, "", 0);
            break;
    }
    return 0;

Usage:
    printf("Usage:\n");
    printf("      fs[w|r] <filename> <memory address> <length>\n");
    printf("      fsd [usb0|usb1|mmc]\n");
    return 0;

done:
    return 0;
}

cmdt cmdt_fs __attribute__ ((section("cmdt"))) =
{ "fs", fs_cmd, ""};

