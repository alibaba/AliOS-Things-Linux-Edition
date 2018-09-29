/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file flash_db.h
*   \brief Build-in Flash Information
*   \author Montage
*/

#ifndef __FLASH_DB_H__
#define __FLASH_DB_H__

/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include "flash_api.h"
#include "flash_config.h"

/*=============================================================================+
| Serial Flash Database                                                        |
+=============================================================================*/
#ifdef CONFIG_SFLASH

enum
{
// stype 1
    EON = 0x1c,
    MACRONIX = 0xc2,
    SPANSION = 0x01,
    ST = 0x20,
    WINBOND = 0xef,
    GD = 0xc8,
    MD = 0x51,
// stype 2
    ATMEL = 0x1f,
// stype 3
    SST = 0xbf,
};

#ifndef BOOT_MODE_BOOT1
enum {
    BLOCKNUM_0  = 1,
    BLOCKNUM_1  = 2,
    BLOCKNUM_2  = 4,
    BLOCKNUM_3  = 8,
    BLOCKNUM_4  = 16,
    BLOCKNUM_5  = 32,
    BLOCKNUM_6  = 64,
    BLOCKNUM_7  = 128,
    BLOCKNUM_8  = 256,
    BLOCKNUM_9  = 512,
    BLOCKNUM_10 = 1024,
    BLOCKNUM_11 = 2048,
};
#else
/* {sector size, sector num} */
flashsec uniform_64KB[9] = {
    {16, 1},                    //64KB*1;   64KB
    {16, 2},                    //64KB*2;   128KB
    {16, 4},                    //64KB*4;   256KB
    {16, 8},                    //64KB*8;   512KB
    {16, 16},                   //64KB*16;  1MB
    {16, 32},                   //64KB*32;  2MB
    {16, 64},                   //64KB*64;  4MB
    {16, 128},                  //64KB*128; 8MB
    {16, 256}                   //64KB*256; 16MB
};
#endif

#define SF_ID_DEFAULT          0xffffffff  /* Default */
//	#define SF_ID_GD5F4GQ4U        0x00d4c8c8  /* 512 MB or 4 Gb */
#define SF_ID_GD5F1GQ4U        0xc8b14848  /* 128 MB or 1 Gb */
#define SF_ID_GD5F2GQ4U        0xc8b24848  /* 256 MB or 2 Gb */
#define SF_ID_GD5F4GQ4U        0x00d4c8d4  /* 512 MB or 4 Gb */
#define SF_ID_GD5F2GQ4UB       0x00d2c8d2  /* 256 MB or 2 Gb */
#define SF_ID_GD5F1GQ4R        0xc8a14848  /* 128 MB or 1 Gb */
#define SF_ID_GD5F2GQ4R        0xc8a24848  /* 256 MB or 2 Gb */
#define SF_ID_MX35LF1GE4AB     0x00c21212  /* 128 MB or 1 Gb */
#define SF_ID_MX35LF2GE4AB     0x00c22222  /* 256 MB or 2 Gb */
#define SF_ID_W25M02GV         0x00efab21  /* 256 MB or 2 Gb */
#define SF_ID_F50L2G41LB       0x00c80a7f  /* 256 MB or 2 Gb */

#define SF_ID_W25Q256JV        0xef401900  /* 256Mb */
#define SF_ID_XT26G01A         0x00e10be1  /* 128Mb */

#ifdef BOOT_MODE_BOOT1
/* {sector size, sector num} */
/* actually, only use block_num now */
flashsec uniform_256KB[12] = {
    {18, 1},                    //256KB*1;    256KB
    {18, 2},                    //256KB*2;    512KB
    {18, 4},                    //256KB*4;    1MB
    {18, 8},                    //256KB*8;    2MB
    {18, 16},                   //256KB*16;   4MB
    {18, 32},                   //256KB*32;   8MB
    {18, 64},                   //256KB*64;   16MB
    {18, 128},                  //256KB*128;  32MB
    {18, 256},                  //256KB*256;  64MB
    {18, 512},                  //256KB*512;  128MB
    {18, 1024},                 //256KB*1024; 256MB
    {18, 2048},                 //256KB*2048; 512MB
};
#endif

#define CMD_DUMMY_SHIFT 0
#define READ_DUMMY_SHIFT 1
#define PAGE_SIZE_SHIFT 2
#define BAD_BLOCK_CHECK_SHIFT 4

#define CMD_DUMMY_END       (0 << CMD_DUMMY_SHIFT)
#define CMD_DUMMY_START     (1 << CMD_DUMMY_SHIFT)
#define DISABLE_READ_DUMMY  (0 << READ_DUMMY_SHIFT)
#define ENABLE_READ_DUMMY   (1 << READ_DUMMY_SHIFT)
#define NAND_PAGE_SIZE_2K   0x00
#define NAND_PAGE_SIZE_4K   0x01
#define NAND_PAGE_SIZE_8K   0x02
#define NAND_PAGE_SIZE_16K  0x03
#define BAD_BLOCK_CHECK_ONE_PAGE    (0 << BAD_BLOCK_CHECK_SHIFT)
#define BAD_BLOCK_CHECK_TWO_PAGE    (1 << BAD_BLOCK_CHECK_SHIFT)

#ifndef BOOT_MODE_BOOT1
struct flash_dev_tab sflash_db[] = {
//  {id, flags, num of blocks, sdesc, size(with apare area and empty size, actually size = size / 2) }

    {SF_ID_GD5F1GQ4U, (CMD_DUMMY_START | ENABLE_READ_DUMMY | (NAND_PAGE_SIZE_2K << PAGE_SIZE_SHIFT) | BAD_BLOCK_CHECK_ONE_PAGE),
        BLOCKNUM_10, 0x10000000},

    {SF_ID_GD5F2GQ4U, (CMD_DUMMY_START | ENABLE_READ_DUMMY | (NAND_PAGE_SIZE_2K << PAGE_SIZE_SHIFT) | BAD_BLOCK_CHECK_ONE_PAGE),
        BLOCKNUM_11, 0x20000000},

    {SF_ID_GD5F4GQ4U, (CMD_DUMMY_END | ENABLE_READ_DUMMY | (NAND_PAGE_SIZE_4K << PAGE_SIZE_SHIFT) | BAD_BLOCK_CHECK_ONE_PAGE),
        BLOCKNUM_11, 0x40000000},

    {SF_ID_GD5F2GQ4UB, (CMD_DUMMY_END | DISABLE_READ_DUMMY | (NAND_PAGE_SIZE_2K << PAGE_SIZE_SHIFT) | BAD_BLOCK_CHECK_ONE_PAGE),
        BLOCKNUM_11, 0x20000000},

    {SF_ID_GD5F1GQ4R, (CMD_DUMMY_START | ENABLE_READ_DUMMY | (NAND_PAGE_SIZE_2K << PAGE_SIZE_SHIFT) | BAD_BLOCK_CHECK_ONE_PAGE),
        BLOCKNUM_10, 0x10000000},

    {SF_ID_GD5F2GQ4R, (CMD_DUMMY_START | ENABLE_READ_DUMMY | (NAND_PAGE_SIZE_2K << PAGE_SIZE_SHIFT) | BAD_BLOCK_CHECK_ONE_PAGE),
        BLOCKNUM_11, 0x20000000},

    {SF_ID_MX35LF1GE4AB, (CMD_DUMMY_END | DISABLE_READ_DUMMY | (NAND_PAGE_SIZE_2K << PAGE_SIZE_SHIFT) | BAD_BLOCK_CHECK_TWO_PAGE),
        BLOCKNUM_10, 0x10000000},

    {SF_ID_MX35LF2GE4AB, (CMD_DUMMY_END | DISABLE_READ_DUMMY | (NAND_PAGE_SIZE_2K << PAGE_SIZE_SHIFT) | BAD_BLOCK_CHECK_TWO_PAGE),
        BLOCKNUM_11, 0x20000000},

    {SF_ID_W25M02GV, (CMD_DUMMY_END | DISABLE_READ_DUMMY | (NAND_PAGE_SIZE_2K << PAGE_SIZE_SHIFT) | BAD_BLOCK_CHECK_ONE_PAGE),
        BLOCKNUM_10, 0x10000000},

    {SF_ID_F50L2G41LB, (CMD_DUMMY_END | DISABLE_READ_DUMMY | (NAND_PAGE_SIZE_2K << PAGE_SIZE_SHIFT) | BAD_BLOCK_CHECK_ONE_PAGE),
        BLOCKNUM_10, 0x10000000},

    {SF_ID_XT26G01A, (CMD_DUMMY_END | DISABLE_READ_DUMMY | (NAND_PAGE_SIZE_2K << PAGE_SIZE_SHIFT)),
        BLOCKNUM_10, 0x10000000},

    // If not match with NAND flash, treat flash as NOR flash
    {SF_ID_DEFAULT, (0 | 0), 0x0, 0x000000}, //reserved for uniform 64KB setting
    {SF_ID_DEFAULT, (0 | 0), BLOCKNUM_8, 0x1000000},
};
#else
struct flash_dev_tab sflash_db[] = {
//  {id, flags, num of blocks, sdesc, size(with apare area and empty size, actually size = size / 2) }
     
    {SF_ID_GD5F1GQ4U, (CMD_DUMMY_START | ENABLE_READ_DUMMY | (NAND_PAGE_SIZE_2K << PAGE_SIZE_SHIFT) | BAD_BLOCK_CHECK_ONE_PAGE),
        &uniform_256KB[10], 0x10000000},
     
    {SF_ID_GD5F2GQ4U, (CMD_DUMMY_START | ENABLE_READ_DUMMY | (NAND_PAGE_SIZE_2K << PAGE_SIZE_SHIFT) | BAD_BLOCK_CHECK_ONE_PAGE),
        &uniform_256KB[11], 0x20000000},

    {SF_ID_GD5F4GQ4U, (CMD_DUMMY_END | ENABLE_READ_DUMMY | (NAND_PAGE_SIZE_4K << PAGE_SIZE_SHIFT) | BAD_BLOCK_CHECK_ONE_PAGE),
        &uniform_256KB[11], 0x40000000},

    {SF_ID_GD5F2GQ4UB, (CMD_DUMMY_END | DISABLE_READ_DUMMY | (NAND_PAGE_SIZE_2K << PAGE_SIZE_SHIFT) | BAD_BLOCK_CHECK_ONE_PAGE),
        &uniform_256KB[11], 0x20000000},
     
    {SF_ID_GD5F1GQ4R, (CMD_DUMMY_START | ENABLE_READ_DUMMY | (NAND_PAGE_SIZE_2K << PAGE_SIZE_SHIFT) | BAD_BLOCK_CHECK_ONE_PAGE),
        &uniform_256KB[10], 0x10000000},
        
    {SF_ID_GD5F2GQ4R, (CMD_DUMMY_START | ENABLE_READ_DUMMY | (NAND_PAGE_SIZE_2K << PAGE_SIZE_SHIFT) | BAD_BLOCK_CHECK_ONE_PAGE),
        &uniform_256KB[11], 0x20000000},
    
    {SF_ID_MX35LF1GE4AB, (CMD_DUMMY_END | DISABLE_READ_DUMMY | (NAND_PAGE_SIZE_2K << PAGE_SIZE_SHIFT) | BAD_BLOCK_CHECK_TWO_PAGE),
        &uniform_256KB[10], 0x10000000},
        
    {SF_ID_MX35LF2GE4AB, (CMD_DUMMY_END | DISABLE_READ_DUMMY | (NAND_PAGE_SIZE_2K << PAGE_SIZE_SHIFT) | BAD_BLOCK_CHECK_TWO_PAGE),
        &uniform_256KB[11], 0x20000000},

    {SF_ID_W25M02GV, (CMD_DUMMY_END | DISABLE_READ_DUMMY | (NAND_PAGE_SIZE_2K << PAGE_SIZE_SHIFT) | BAD_BLOCK_CHECK_ONE_PAGE),
        &uniform_256KB[10], 0x10000000},

    {SF_ID_F50L2G41LB, (CMD_DUMMY_END | DISABLE_READ_DUMMY | (NAND_PAGE_SIZE_2K << PAGE_SIZE_SHIFT) | BAD_BLOCK_CHECK_ONE_PAGE),
        &uniform_256KB[10], 0x10000000},

    // If not match with NAND flash, treat flash as NOR flash
    {SF_ID_DEFAULT, (0 | 0), 0x0, 0x000000}, //reserved for uniform 64KB setting
    {SF_ID_DEFAULT, (0 | 0), &uniform_64KB[8], 0x1000000},
};
#endif

#define SFL_DEVTAB_SZ (sizeof(sflash_db)/sizeof(struct flash_dev_tab))
#define SFL_DEVTAB_DEFAULT  (SFL_DEVTAB_SZ - 2)
#endif

#endif
