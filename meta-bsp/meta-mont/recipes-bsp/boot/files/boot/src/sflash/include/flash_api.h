/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file flash_api.h
*   \brief Flash API
*   \author Montage
*/

#ifndef __FLASH_API_H__
#define __FLASH_API_H__

#include "mt_types.h"

enum
{
    FL_OK = 0,
    FL_ERR = -1,
    FL_ERR_PROG = -2,
    FL_ERR_ERASE = -3,
    FL_ERR_TIMEOUT = -4,
    FL_ERR_UNSUPPORT = -5,
    FL_ERR_NOT_INIT = -6,
    FL_LOCK = -7,
};
#ifdef BOOT_MODE_BOOT1
typedef struct flashsec
{
    u32  size;         // in power of 2
    u32  num;          // number of block
    u32  ecmd;
} flashsec;
#endif

typedef struct
{
    u32  bank;         /* bank no. */
} drvarg;

#ifndef BOOT_MODE_BOOT1
typedef struct flashops flashops;
#endif
#define MAX_SW_PROT_SEC_NUM 0x100
#define FLASH_TYPE_NOR 0
#define FLASH_TYPE_NAND 1
typedef struct
{
    u32   id;
    u32   type;
    u32   base;
    // flags //////////
    u32   cmd_dummy_byte;
    u32   read_dummy_byte;
    u32   page_size;
#ifdef BOOT_MODE_BOOT1
    u32   block_size;
#endif
    u32   check_page_type;
    ///////////////////
    u32   blockNum;
#ifdef BOOT_MODE_BOOT1
    flashsec *sdesc;
#endif
    u32   size;

#ifdef BOOT_MODE_BOOT1
int  (*get_id) (drvarg * sfd);
void  *next;       // point to next driver
#endif
#ifndef BOOT_MODE_BOOT1
    flashops *ops;
#else
    int  (*erase) (drvarg * sfd, u32 addr, int type, u32 ecmd);
    int  (*prog) (drvarg * sfd, u32 addr, u32 len, u32 buf, u32 dma_sel, u32 aes_control);
    u32  (*read_bytes) (drvarg * sfd, u32 offset, u32 src, u32 count, u32 dma_sel, u32 aes_control);
#endif
    drvarg arg;
#ifdef CONFIG_FLASH_SW_LOCK
    u8 sw_lock[MAX_SW_PROT_SEC_NUM];
#endif
} flashdesc;
#ifndef BOOT_MODE_BOOT1
struct flashops
{
    void (*init_size)(u32 page_flag);
    u32  (*page_capability)();     // get capability for memory alignment between pages
    u32  (*block_size)();          // get real size of a block
    u32  (*block_capability)();    // get capability for memory alignment between blocks
    u32  (*get_addr)(u32 block_idx, u32 page_idx, u32 page_offset);
    u32  (*get_end_addr)(u32 from, u32 len);
    u32  (*block_index)(u32 addr);
    u32  (*get_remaining_in_block)(u32 from);
    u32  (*get_size_from_block_to_end)(u32 block_idx);
    int  (*erase) (u32 addr, int type);
    int  (*prog) (u32 addr, u32 len, u32 buf, u32 dma_sel, u32 aes_control);
    u32  (*read_bytes) (u32 offset, u32 src, u32 count, u32 dma_sel, u32 aes_control);
    u32  (*convert_addr)(u32 addr_not_aligned, u32 *block_idx);
};
#endif

struct flash_dev_tab
{
    u32      id;
    u32      flags;
#ifndef BOOT_MODE_BOOT1
    u32      num;
#else
    flashsec *sdesc;
#endif
    u32      size;
};


/* used for initializa linux cmdline*/
#define BOOT_CMDLINE_DRAM_ADDR 0xA0000300  // match with firmware: #define BOOT_CMDLINE_DRAM_ADDR  0xA0000300
#define COMMAND_LINE_SIZE 4096
#define MAX_BAD_BLOCK_NUM 40



//
int flash_write_no_check_block(u32 addr, u32 from, u32 len);
int flash_erase_no_check_block(u32 addr, u32 len);
int flash_whole_chip_erase(void);
int flash_read_without_pdma(u32 offset, u32 count);

// we should not use these function to read/write/erase
// these are used in sflash_controller.c only
//int flash_read_bytes(u32 addr, u32 src, int len, u32 check_sel);
//int flash_write(u32 addr, u32 from, u32 len, u32 check_sel);
//int flash_erase(u32 addr, u32 len, u32 check_sel);

void mark_bad_block(u32 block_index);
void mark_bad_block_by_interval(u32 block_index, u32 len);
void mark_good_block(u32 block_index);
void mark_good_block_by_interval(u32 block_index, u32 len);
void find_all_bad_block(void);
void flash_erase_appdata(void);
int flash_init(int boot_type);

int flash_control_write(u32 addr, u32 from, u32 len, u32 check_sel);
int flash_control_read_bytes(u32 addr, u32 src, int len, u32 check_sel);
int flash_control_erase(u32 addr, u32 len, u32 check_sel);
u32 get_recovery_offset(void);
int program_setting_page(u32 page_index, u32 src);
int erase_setting_block(void);
int read_setting_block(u32 src);
u32 get_page_size(void);
void change_aes_enable_status(u32 aes_status);
u32 get_block_size(void);
u32 get_linux_block_index(void);
int is_ubi_image(unsigned char *buf_addr);
u32 load_app_image(u32 *h, u32 img_offset);

#endif
