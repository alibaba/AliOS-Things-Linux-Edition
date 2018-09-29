/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file cdb.c
*   \brief CDB access function
*   \author Montage
*/

/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <netprot.h>
#include <common.h>
#include "sflash/include/flash_config.h"
#include <mt_types.h>
#include <lib.h>
#include "sflash/include/flash_api.h"

#define printk printf

extern unsigned short crc16_ccitt(const void *buf, int len);

struct cdb_id
{
#if defined(BIG_ENDIAN)
    unsigned short mod:5;
    unsigned short type:3;
    unsigned short idx:7;
    unsigned short array:1;
#else
    unsigned short array:1;
    unsigned short idx:7;
    unsigned short type:3;
    unsigned short mod:5;
#endif
};

#define CDB_ID_(type, index, array)     (((31 & 0x1f) << 11) | ((type & 0x07) << 8) | ((index & 0x7f) << 1) | (array & 0x01))

enum
{
    CDB_INT = 1,
    CDB_STR = 2,
    CDB_IP = 3,
    CDB_MAC = 4,
    CDB_VER = 3,

    CDB_BOOT_HVER = CDB_ID_(CDB_VER, 0, 0),
    CDB_BOOT_ID = CDB_ID_(CDB_INT, 1, 0),
    CDB_BOOT_FILE = CDB_ID_(CDB_STR, 2, 0),
    CDB_BOOT_MAC = CDB_ID_(CDB_MAC, 3, 1),
    CDB_BOOT_MODE = CDB_ID_(CDB_INT, 4, 0),
    CDB_BOOT_VER = CDB_ID_(CDB_VER, 5, 0),
    CDB_BOOT_BUF = CDB_ID_(CDB_INT, 6, 0),
    CDB_BOOT_SZ = CDB_ID_(CDB_INT, 7, 0),
    CDB_BOOT_SRC = CDB_ID_(CDB_INT, 8, 0),
    CDB_BOOT_IP = CDB_ID_(CDB_IP, 9, 0),
    CDB_BOOT_MSK = CDB_ID_(CDB_IP, 10, 0),
    CDB_BOOT_GW = CDB_ID_(CDB_IP, 11, 0),
    CDB_BOOT_SVR = CDB_ID_(CDB_IP, 12, 0),
    CDB_BOOT_SRC2 = CDB_ID_(CDB_INT, 13, 0),
    CDB_BOOT_LOG_SRC = CDB_ID_(CDB_INT, 14, 0),
    CDB_BOOT_LOG_SZ = CDB_ID_(CDB_INT, 15, 0),
    CDB_BOOT_CDB_LOC = CDB_ID_(CDB_INT, 16, 0),
    CDB_BOOT_CDB_SZ = CDB_ID_(CDB_INT, 17, 0),
    CDB_BOOT_CVER = CDB_ID_(CDB_STR, 18, 0),
    CDB_BOOT_RFC = CDB_ID_(CDB_STR, 19, 0),
    CDB_BOOT_PLL = CDB_ID_(CDB_INT, 20, 0),
    CDB_BOOT_TXVGA = CDB_ID_(CDB_STR, 21, 0),
    CDB_BOOT_RXVGA = CDB_ID_(CDB_STR, 22, 0),
    CDB_BOOT_SERIAL = CDB_ID_(CDB_INT, 23, 0),
    CDB_BOOT_PIN = CDB_ID_(CDB_STR, 24, 0),
    CDB_BOOT_FREQ_OFS = CDB_ID_(CDB_INT, 25, 0),
    CDB_BOOT_MADC_VAL = CDB_ID_(CDB_STR, 26, 1),
    CDB_BOOT_LNA = CDB_ID_(CDB_STR, 27, 0),
    CDB_BOOT_AUTOCAL = CDB_ID_(CDB_INT, 28, 0),
    CDB_BOOT_QUIET = CDB_ID_(CDB_INT, 29, 0),
    CDB_BOOT_SWCFG = CDB_ID_(CDB_STR, 30, 0),
    CDB_BOOT_AI_INTERFACE = CDB_ID_(CDB_STR, 32, 0),
    CDB_BOOT_UPGRADE = CDB_ID_(CDB_INT, 33, 0),
    CDB_BOOT_NETWORK = CDB_ID_(CDB_INT, 34, 0),
    CDB_BOOT_CI_OFFSET = CDB_ID_(CDB_INT, 35, 0),
    CDB_BOOT_RECOVERY_OFFSET = CDB_ID_(CDB_INT, 36, 0),
    CDB_BOOT_RECOVERY = CDB_ID_(CDB_INT, 37, 0),
    CDB_BOOT_TXPDIFF = CDB_ID_(CDB_STR, 38, 0),
    CDB_BOOT_FEM_PRODUCT_ID = CDB_ID_(CDB_STR, 39, 0),
    CDB_BOOT_FEM_EN = CDB_ID_(CDB_INT, 40, 0),
    CDB_BOOT_PINMUX = CDB_ID_(CDB_STR, 41, 0),
    CDB_BOOT_POWERCFG = CDB_ID_(CDB_INT, 42, 0),
    CDB_BOOT_DRIV_STR = CDB_ID_(CDB_STR, 43, 0),
    CDB_BOOT_GPIO_SETTING = CDB_ID_(CDB_STR, 44, 0),
    CDB_BOOT_CLKCFG = CDB_ID_(CDB_INT, 45, 0),
    CDB_BOOT_WATCHDOG_TIMER = CDB_ID_(CDB_INT, 46, 0),
    CDB_BOOT_SECOND_IMG_OFFSET = CDB_ID_(CDB_INT, 47, 0),
    CDB_BOOT_MIC_GAIN_CTRL = CDB_ID_(CDB_STR, 48, 0),
    CDB_BOOT_SD_ROOT = CDB_ID_(CDB_STR, 49, 0),

    CDB_ID_SZ = sizeof (struct cdb_id),
    CDB_AID_SZ = CDB_ID_SZ + sizeof (int),      /* array idx , ie 4 */
    CDB_LEN_SZ = sizeof (short),
    CDB_CRC_SZ = 4,
};

#define cdb_log(format, args...)        //printk(format, ##args)

//-------------------------------------------------------------
struct cdbobj
{
    union
    {
        unsigned short v;
        struct cdb_id f;
    } id;
    unsigned short len;
    int idx;                    // skip if not arrary
};

struct parmd
{
    void *val;
    union
    {
        unsigned short v;
        struct cdb_id f;
    } id;
    char *name;
    char *desc;
    int dirty_bit;
};

#define CDB_DPTR(p) ((void*)(&p->idx))
#define CDB_IDV(p)   (be16_to_cpu(p->id.v))
#define CDB_DLEN(p) ((be16_to_cpu(p->len)+3)&0xfffc)
#define CDB_AIDX(p) (be32_to_cpu(p->idx))
#define CDB_AIDX_SZ sizeof(int)
#define CDBV(id)    (*((unsigned short*)&id))

#define CDBE(v, i, n, d, b)    { val: v, id: {i}, name: n, desc: d, dirty_bit: b}

struct parmd parmds[] = {
    CDBE(&bootvars.hver, CDB_BOOT_HVER, "hver", "h/w ver info", 1),
    //CDBE(&bootvars.cver, CDB_BOOT_CVER, "cver", "customer ver info", 1),
    CDBE(&bootvars.id, CDB_BOOT_ID, "id", "MSB: verder id, LSB: product id", 1),
    CDBE(&bootvars.file, CDB_BOOT_FILE, "file", "loading file name", 1),
    CDBE(&bootvars.mac0, CDB_BOOT_MAC, "mac0", "mac address 0", 1),
    CDBE(&bootvars.mac1, CDB_BOOT_MAC, "mac1", "mac address 1", 1),
    CDBE(&bootvars.mac2, CDB_BOOT_MAC, "mac2", "mac address 2", 1),
    CDBE(&bootvars.mode, CDB_BOOT_MODE, "mode", "boot mode", 1),
    //CDBE(&bootvars.ver, CDB_BOOT_VER, "ver", "ver info", 1),
    CDBE(&bootvars.load_addr, CDB_BOOT_BUF, "buf", "loading buffer", 1),
    CDBE(&bootvars.load_sz, CDB_BOOT_SZ, "size", "loading size", 1),
    CDBE(&bootvars.load_src, CDB_BOOT_SRC, "addr", "loading from", 1),
    CDBE(&bootvars.ip, CDB_BOOT_IP, "ip", "ip address", 1),
    CDBE(&bootvars.msk, CDB_BOOT_MSK, "msk", "net mask", 1),
    CDBE(&bootvars.gw, CDB_BOOT_GW, "gw", "gateway ip", 1),
    CDBE(&bootvars.server, CDB_BOOT_SVR, "server", "tftp server ip", 1),
    CDBE(&bootvars.load_src2, CDB_BOOT_SRC2, "backup_addr",
         "loading from backup addr", 1),
    CDBE(&bootvars.log_src, CDB_BOOT_LOG_SRC, "log_addr", "loading log from",
         1),
    CDBE(&bootvars.log_sz, CDB_BOOT_LOG_SZ, "log_size", "log size", 1),
    CDBE(&bootvars.rfc, CDB_BOOT_RFC, "rfc", "rfc", 1),
    CDBE(&bootvars.pll, CDB_BOOT_PLL, "pll", "custom PLL setting", 1),
    CDBE(&bootvars.txvga, CDB_BOOT_TXVGA, "txvga", "txvga calib value", 1),
    CDBE(&bootvars.rxvga, CDB_BOOT_RXVGA, "rxvga", "rxvga calib value", 1),
    CDBE(&bootvars.serial, CDB_BOOT_SERIAL, "serial", "serial number", 1),
    CDBE(&bootvars.pin, CDB_BOOT_PIN, "pin", "WPS PIN number", 1),
    CDBE(&bootvars.freq_ofs, CDB_BOOT_FREQ_OFS, "freq_ofs",
         "frequnecy offset calib value", 1),
    CDBE(&bootvars.madc_val0, CDB_BOOT_MADC_VAL, "madc_val0", "madc value 0",
         1),
    CDBE(&bootvars.madc_val1, CDB_BOOT_MADC_VAL, "madc_val1", "madc value 1",
         1),
    CDBE(&bootvars.lna, CDB_BOOT_LNA, "lna", "lna gain", 1),
    CDBE(&bootvars.autocal, CDB_BOOT_AUTOCAL, "autocal", "auto calibration", 1),
    CDBE(&bootvars.quiet, CDB_BOOT_QUIET, "quiet", "quiet", 1),
    CDBE(&bootvars.swcfg, CDB_BOOT_SWCFG, "swcfg", "switch port config", 1),
    CDBE(&bootvars.ai, CDB_BOOT_AI_INTERFACE, "ai", "configure pcm/i2s for external codec", 1),
    CDBE(&bootvars.upgrade, CDB_BOOT_UPGRADE, "upgrade", "upgrade", 1),
    CDBE(&bootvars.network, CDB_BOOT_NETWORK, "network", "network", 0),
    CDBE(&bootvars.ci_offset, CDB_BOOT_CI_OFFSET, "ci_offset", "ci_offset", 1),
    CDBE(&bootvars.recovery_offset, CDB_BOOT_RECOVERY_OFFSET, "recovery_offset", "recovery_offset", 1),
    CDBE(&bootvars.recovery, CDB_BOOT_RECOVERY, "recovery", "recovery", 1),
    CDBE(&bootvars.txp_diff, CDB_BOOT_TXPDIFF, "txp_diff", "txp_diff", 1),
    CDBE(&bootvars.fem_product_id, CDB_BOOT_FEM_PRODUCT_ID, "fem_product_id", "fem_product_id", 1),
    CDBE(&bootvars.fem_en, CDB_BOOT_FEM_EN, "fem_en", "fem_en", 1),
    CDBE(&bootvars.pinmux, CDB_BOOT_PINMUX, "pinmux", "pinmux settings", 1),
    CDBE(&bootvars.powercfg, CDB_BOOT_POWERCFG, "powercfg", "power configurations", 1),
    CDBE(&bootvars.gpio_driving, CDB_BOOT_DRIV_STR, "gpio_driving", "gpio driving strength", 1),
    CDBE(&bootvars.gpio_setting, CDB_BOOT_GPIO_SETTING, "gpio_setting", "gpio setting", 1),
    CDBE(&bootvars.clkcfg, CDB_BOOT_CLKCFG, "clkcfg", "clk setting", 1),
    CDBE(&bootvars.watchdog_timer, CDB_BOOT_WATCHDOG_TIMER, "watchdog", "watchdog timer", 1),
    CDBE(&bootvars.second_img_offset, CDB_BOOT_SECOND_IMG_OFFSET, "second_img_offset", "second image offset", 1),
    CDBE(&bootvars.mic_gain_ctrl, CDB_BOOT_MIC_GAIN_CTRL, "mic_gain_ctrl", "mic array gain control", 1),
    CDBE(&bootvars.sd_root, CDB_BOOT_SD_ROOT, "sd_root", "sd boot device", 1),
};

extern bootvar bootvars;

//-------------------------------------------------------------
#define CDB_ID_NUM          (sizeof(parmds)/sizeof(struct parmd))
#define CDB_ID_END          0xffff

#define isdigit(c)          ((c) >= '0' && (c) <= '9')

//-------------------------------------------------------------
static int id_to_idx(unsigned short id);
unsigned short name_to_idx(char *name);

/*!
 * function: id_to_idx
 *
 *  \brief
 *  \param  id
 *  \return 0: ok
 */
static int id_to_idx(unsigned short id)
{
    int i;
    for (i = 0; i < CDB_ID_NUM; i++)
    {
        if (id == parmds[i].id.v)
            return i;
    }
    cdb_log("Invalid id: %08x\n", id);
    return -1;
}

/*!
 * function: name_to_id
 *
 *  \brief  store to flash
 *  \param  name
 *  \return 0: ok
 */
unsigned short name_to_id(char *name)
{
    struct parmd *pd;
    int i;

    for (i = 0, pd = &parmds[0]; i < CDB_ID_NUM; i++, pd++)
    {
        if (!strcmp(pd->name, name))
        {
            return pd->id.v;
        }
    }
    return 0;
}

/*!
 * function: name_to_idx
 *
 *  \brief  convert array index
 *  \param  name
 *  \return 0: ok
 */
unsigned short name_to_idx(char *name)
{
    unsigned short array_num = 0;
    int i, num, len = strlen(name);

    if (len == 0)
        return 0;

    for (num = 0, i = 1; i <= 3; i++, num++)
        if (!isdigit(name[len - i]))
            break;

    if (num)
        array_num = (unsigned short) atoi(name + len - num);
    return array_num;

}

//#define TEST_FLASH_BY_DDR
#define CONFIG_DDR_BASE     (0xa8000000ul)
#define CDB_BUF_BASE        (0xa9000000ul)
#define FLASH_MAX_PAGE_NUM  (64)
#define FLASH_MAX_PAGE_SIZE (4096)
static int flash_page_size = 512;
#define FLASH_PAGE_ADDR_MASK    (flash_page_size - 1)
static unsigned int flash_page_tail_idx = 0;
static unsigned char one_blk_flash_buf[FLASH_MAX_PAGE_NUM * FLASH_MAX_PAGE_SIZE] __attribute__ ((aligned (2048)));
void init_cdb_page_idx(void)
{
#if defined(TEST_FLASH_BY_DDR)
    memset((char *) CONFIG_DDR_BASE, 0xff, FLASH_MAX_PAGE_NUM * 0x1000);
#else
    erase_setting_block();
#endif
    flash_page_tail_idx = 0;
    return;
}

int cdb_program_flash(unsigned char *buf)
{
    int ret = 0;

    cdb_log("cdb_program_flash page %d\n", flash_page_tail_idx);
    if (FLASH_MAX_PAGE_NUM > flash_page_tail_idx)
    {
#if defined(TEST_FLASH_BY_DDR)
        memcpy((void *) (CONFIG_DDR_BASE + (flash_page_tail_idx * 0x1000)),
               (void *)buf, flash_page_size);
#else
        program_setting_page(flash_page_tail_idx, (u32)buf);
#endif
        flash_page_tail_idx++;
    }
    else
    {
        cdb_log("cdb_program_flash erase block \n");
        init_cdb_page_idx();
        ret = -1;
    }

    return ret;
}

int cdb_check(const unsigned char *cdb_base, int size)
{
    unsigned short tcrc =  be16_to_cpu(*((unsigned short *) (&cdb_base[size - 4])));

    return !(tcrc == crc16_ccitt(cdb_base, (size - 4)));
}

/*!
 * function: cdb_init
 *
 *  \brief  init
 *  \param  id
 *  \return 0: ok
 */
int cdb_init(int id)
{

    struct parmd *pd;
    struct cdbobj *p;
    unsigned short array_num = 0;
    int idx, dl, num = 0, rc = 0;
    void *dp;

#if defined(TEST_FLASH_BY_DDR)
    for (p = (struct cdbobj *) (CONFIG_DDR_BASE);
         (unsigned int) p < (CONFIG_DDR_BASE + 0x40000);
         p = (struct cdbobj *) (CDB_DPTR(p) + CDB_DLEN(p)))
    {
        if (cdb_check
            ((const unsigned char *) p, (CDB_DLEN(p) + CDB_ID_SZ + CDB_LEN_SZ))
            || (p->id.v == CDB_ID_END))
        {
            if (0 == ((unsigned int) (p) & 0x0FFF))
            {
                break;
            }
            else
            {
                p = (struct cdbobj *) (((unsigned int) p + 0x0fff) &
                                       0xFFFFF000UL);
                flash_page_tail_idx++;
            }
        }
#else
    read_setting_block((u32)one_blk_flash_buf);
    flash_page_size = get_page_size();
    cdb_log("flash_page_size %d\n", flash_page_size);
    p = (struct cdbobj *) (one_blk_flash_buf);
    while((unsigned int)p < (unsigned int)(one_blk_flash_buf + (FLASH_MAX_PAGE_NUM * flash_page_size)))
    {
        if ((CDB_IDV(p) == CDB_ID_END)
            || ((CDB_DLEN(p) + CDB_ID_SZ + CDB_LEN_SZ)
                 > (flash_page_size - ((unsigned int)p & FLASH_PAGE_ADDR_MASK)))
            || cdb_check((const unsigned char *) p, (CDB_DLEN(p) + CDB_ID_SZ + CDB_LEN_SZ)))
        {
            if (0 == ((unsigned int) (p) & FLASH_PAGE_ADDR_MASK))
            {
                break;
            }
            else
            {
                p = (struct cdbobj *) (((unsigned int) p + FLASH_PAGE_ADDR_MASK)
                                       & (~FLASH_PAGE_ADDR_MASK));
                flash_page_tail_idx++;
                continue;
            }
        }
#endif
        // on flash format is always big-endian
        p->id.v = be16_to_cpu(p->id.v);

        if ((idx = id_to_idx(p->id.v)) < 0)
            goto Next;
        array_num = 0;

        if (p->id.f.array)
            array_num = CDB_AIDX(p);
        pd = &parmds[idx + array_num];

        if (pd->val == 0)
            goto Next;
        dp = CDB_DPTR(p);
        dl = CDB_DLEN(p);
        if (p->id.f.array)
        {
            dp += 4;            /* 4 bytes index */
            dl -= 4;
        }
        dl -= CDB_CRC_SZ;       // remove crc
        memcpy((void *)pd->val, (void *)dp, dl);

        if((p->id.f.type==CDB_IP) || (p->id.f.type==CDB_INT))
            *(unsigned long *)pd->val = be32_to_cpu(*(unsigned long *)pd->val);

        if (CDB_STR == p->id.f.type)
            *(((char *) pd->val) + dl) = 0;
        pd->dirty_bit = 0;
        num++;
Next:
        p = (struct cdbobj *) (CDB_DPTR(p) + CDB_DLEN(p));
    }

    if (!num)
    {
        cdb_log("cdb_init: empty!\n");
        rc = -1;
    }

    return rc;
}

/*!
 * function: cdb_write
 *
 *  \brief  init
 *  \param  buf
 *  \param  buf_offset
 *  \param  idv
 *  \param  array_num
 *  \param  val
 *  \return 0: ok
 */
int cdb_write(unsigned char *buf, int *offset, unsigned short idv,
              unsigned short array_num, void *val)
{
    struct cdb_id id;
    int rc = -1, len, wh;
    struct cdbobj wp;
    unsigned char *cdb_head = &buf[*offset];
    unsigned short crc;
    unsigned long temp_val;

    CDBV(id) = idv;
    if ((id_to_idx(idv)) < 0)
        return rc;

    switch (id.type)
    {
        case CDB_STR:
            len = strlen((char *) val);
            break;
        case CDB_MAC:
            len = 6;
            break;
        default:
            len = 4;
            break;
    }
    len = (len + 3) & ~3;       // algin len to multiple of 4

//  for (p = (struct cdbobj *) (CONFIG_FLASH_BASE + CONFIG_BOOT_PARM_OFS);
//       ((unsigned int) p + len + 8) <
//       (CONFIG_FLASH_BASE + CONFIG_BOOT_PARM_OFS + CONFIG_BOOT_PARM_SZ);
//       p = (struct cdbobj *) (CDB_DPTR(p) + CDB_DLEN(p)))
//  {
//      if (p->id.v == idv)
//      {
//          if (p->id.f.array && p->idx != array_num)   // not same array index
//              continue;
//          rp = p;             // where to be removed
//          continue;
//      }
//      else if (p->id.v != CDB_ID_END)
//          continue;           // next
//      // add to last
//      cdb_log("cdb_write: id=%x, aidx=%d, @ %x, old=%x\n", idv, array_num, p,
//              rp);
    cdb_log("cdb_write: id=%x, aidx=%d\n", idv, array_num);
    wp.id.v = cpu_to_be16(idv);
    wp.idx = cpu_to_be32(array_num);
    wp.len = len;
    wh = CDB_ID_SZ + CDB_LEN_SZ;
    if (id.array)
    {
        wp.len += CDB_AIDX_SZ;  // +4
        wh += CDB_AIDX_SZ;
    }
    // ADD crc
    wp.len += CDB_CRC_SZ;

    if (flash_page_size < (*offset + wh + len + CDB_CRC_SZ))
    {
        if (cdb_program_flash(buf))
        {
            return rc;
        }
        *offset = 0;
    }

    if (0 == *offset)
    {
        memset((char *)buf, 0xff, flash_page_size);
        if (flash_page_size < (wh + len + CDB_CRC_SZ))
        {
            cdb_log("buf_size not enough\n");
        }
    }

    wp.len = cpu_to_be16(wp.len);

    memcpy((void *)&buf[*offset], (void *)&wp, wh);
    *offset += wh;

    if((id.type==CDB_IP) || (id.type==CDB_INT))
    {
        temp_val = be32_to_cpu(*(unsigned long *)val);
        memcpy((void *)&buf[*offset], (void *)&temp_val, len);
    }
    else
    {
        memcpy((void *)&buf[*offset], (void *)val, len);
    }
    *offset += len;

    crc = cpu_to_be16(crc16_ccitt(cdb_head, wh + len));
    memcpy((void *)&buf[*offset], (void *)&crc, 2);
    *offset += CDB_CRC_SZ;
//      flash_write((unsigned int) p - CONFIG_FLASH_BASE, &wp, wh, CHECK_BAD_BLOCK);
//      flash_write(fp - CONFIG_FLASH_BASE, val, len, CHECK_BAD_BLOCK);
//      if (rp)
//      {
//          wp.id.v = 0;
//          flash_write((unsigned int) rp - CONFIG_FLASH_BASE, &wp, CDB_ID_SZ, CHECK_BAD_BLOCK);
//      }
    rc = 0;
//      break;
//  }

    return rc;
}

/*!
 * function: cdb_save
 *
 *  \brief  store to flash
 *  \param  flag
 *  \return 0: ok
 */
int cdb_save(int flag)
{
    int i, result = 0, saved = 0, retry = 0;
    unsigned char *page_buf = (unsigned char *)((unsigned int)one_blk_flash_buf | 0xa0000000);
    int data_tail_in_page = 0;

    if (0 >= flash_page_size)
    {
        cdb_log("cdb_save: Error with flash page size\n");
        return -1;
    }
    memset((char *)page_buf, 0xff, flash_page_size);

    for (i = 0; i < CDB_ID_NUM; i++)
    {
        if ((0 == flag) && (0 == parmds[i].dirty_bit))
        {
            continue;
        }
        if (0 <=
            (result =
             cdb_write(page_buf, &data_tail_in_page, parmds[i].id.v,
                       name_to_idx(parmds[i].name), parmds[i].val)))
        {
            parmds[i].dirty_bit = 0;
            saved++;
            continue;
        }

        cdb_log("cdb_save: cdb_save(0) fail\n");
        if (retry++ >= 1 || flag)
            return -1;
        cdb_log("cdb_save: wrap\n");
//#if ( CONFIG_BOOT_PARM_OFS > CONFIG_BOOT_SZ)
//        flash_erase(CONFIG_BOOT_PARM_OFS, CONFIG_BOOT_PARM_SZ, CHECK_BAD_BLOCK);
//#else
//        memcpy((void *) CONFIG_BOOT_BACKUP, (void *) CONFIG_FLASH_BASE,
//               CONFIG_BOOT_SZ);
//        memset(CONFIG_BOOT_BACKUP + CONFIG_BOOT_PARM_OFS, 0xff,
//               CONFIG_BOOT_PARM_SZ);
//        flash_erase(0, CONFIG_BOOT_SZ, CHECK_BAD_BLOCK);
//        flash_write(0, CONFIG_BOOT_BACKUP, CONFIG_BOOT_SZ, CHECK_BAD_BLOCK);
//#endif
        result = cdb_save(1);
        return result;
    }
    // program last flash page
    if ((0 < data_tail_in_page) && (cdb_program_flash(page_buf)))
    {
        result = cdb_save(1);
        return result;
    }

    cdb_log("cdb_save: num: %d\n", saved);

    if (result)
    {
        cdb_log("cdb_save err %d\n", result);
    }

    return result;
}

/*!
 * function:  cdb_list
 *
 *  \brief  display variable /list
 *  \param  mask
 *  \return 0: ok
 */
int cdb_list(char *mask)
{
    struct parmd *pd;
    int i, all, len, num = 0;
    unsigned long tmp;

    if (mask == 0 || (mask && *mask == '*'))
    {
        all = 1;
    }
    else
    {
        all = 0;
        len = strlen(mask);
    }
    for (i = 0, pd = &parmds[0]; i < CDB_ID_NUM; i++, pd++)
    {
        if (!all)
        {
            if (strncmp(pd->name, mask, len))
            {
                continue;
            }
        }

        printf("%s: ", pd->name);
        switch (pd->id.f.type)
        {
            case CDB_INT:
                printf("%08x", *(unsigned long *) pd->val);
                break;
            case CDB_IP:
                tmp = htonl(*(unsigned long *) (pd->val));
                printf("%s", inet_ntoa(&tmp));
                break;
            case CDB_MAC:
                printf("%s", ether_ntoa((unsigned char *)pd->val));
                break;
            case CDB_STR:
                printf("%s", pd->val);
                break;
        }
        printf("\n");
        num++;

    }
    return num;
}

/*!
 * function: set_cmd
 *
 *  \brief  set variable command handler
 *  \param argc
 *  \param argv
 *  \return 0: ok
 */
int set_cmd(int argc, char **argv)
{
    int set = 0, i, ip;
    struct parmd *pd;
    unsigned short id, array_num;
    char *name, mac[6];
    char *val = 0;

    if (argc < 1)
    {
        if (argv[-1][0] != '$' || !strcmp(argv[-1], "$"))
        {
            cdb_list(0);
            return ERR_OK;
        }
        set = 0;
        for (i = 1; i < strlen(argv[-1]); i++)
        {
            if (argv[-1][i] == '=')
            {
                set = 1;
                argv[-1][i] = 0;
                val = argv[-1] + i + 1;
                break;
            }
        }
        name = &argv[-1][1];
        goto set;

    }
    if ('?' == argv[0][0])      //set ?
    {
        for (i = 0, pd = &parmds[0]; i < CDB_ID_NUM; i++, pd++)
        {
            printf("%s - %s\n", pd->name, pd->desc);
        }
        return ERR_OK;
    }
    name = argv[0];
    if (argc > 1)
    {
        set = 1;
        val = argv[1];
    }
  set:
    if (!set)
    {
        if ((i = cdb_list(name)) > 0)
        {
            return ERR_OK;
        }
        return ERR_PARM;
    }
    id = name_to_id(name);
    array_num = name_to_idx(name);

    if (!id)
    {
        printf("%s not found\n", name);
        return -1;
    }

    for (i = 0, pd = &parmds[0]; i < CDB_ID_NUM; i++, pd++)
    {
        if (pd->id.v == id)
        {
            pd = &parmds[i + array_num];
            break;
        }
    }

    switch (pd->id.f.type)
    {
        case CDB_INT:
            if (sscanf(val, "%x", &i) != 1)
            {
                return ERR_PARM;
            }
            *(unsigned int *) pd->val = i;
            break;
        case CDB_MAC:
            if (ether_aton(val, (unsigned char *)mac))
            {
                return ERR_PARM;
            }
            memcpy((void *)pd->val, (void *)mac, 6);
            break;
        case CDB_IP:
            if (inet_aton(val, &ip))
            {
                return ERR_PARM;
            }
            *(unsigned int *) pd->val = ntohl(ip);
            break;
        case CDB_STR:
            strcpy(pd->val, val);
            break;
    }
    pd->dirty_bit = 1;
    update_mac();

    return ERR_OK;
}

int unset_cmd(int argc, char **argv)
{
    char *set_input[2];
    char set_input1[4] = { 0 };

    if (argc == 1)
    {
        set_input[0] = argv[0];
        set_input[1] = set_input1;
    
        return set_cmd(2, set_input);
    }

    return ERR_PARM;
}

/*!
 * function: save_cmd
 *
 *  \brief  save to flash
 *  \param  argc
 *  \param  argv
 *  \return 0: ok
 */
int save_cmd(int argc, char **argv)
{
    cdb_save(0);
    return ERR_OK;
}

//-------------------------------------------------------------
cmdt cmdt_set[] __attribute__ ((section("cmdt"))) =
{
    {
    "$", set_cmd, "$var[=value]"}
    ,
    {
    "set", set_cmd, "set [var] [value]"}
    ,
    {
    "unset", unset_cmd, "unset [var]"}
    ,
    {
    "save", save_cmd, "save"}
,};

#define CSW_CTRL_PHY_ADDR              0x0000000FUL
#define CSW_CTRL_PHY_INTERNAL          0x00000010UL
#define CSW_CTRL_PHY_ADDR_VALID        0x00000020UL
#define CSW_CTRL_RXCLK_INVERT          0x00000100UL
#define CSW_CTRL_TXCLK_INVERT          0x00000200UL
#define CSW_CTRL_IF_TYPE_WAN           0x01000000UL
#define CSW_CTRL_IF_TYPE_LAN           0x02000000UL
#define CSW_CTRL_IF_TYPE_USER_SPECIFIC 0x04000000UL
#define CSW_CTRL_VLAN_ID               0x00FFF000UL

#define VLAN_ID_BIT   0x0FFFUL
#define IF_TYPE_SHIFT (24)
#define VLAN_ID_SHIFT (12)
void display_swcfg(void)
{
    u32 swcfg[2];
    char if_type[2][9];
    int i;

    sscanf( bootvars.swcfg, "%x,%x", &swcfg[0], &swcfg[1]);
    //printk("p0: %x, p1: %x\n", swcfg[0], swcfg[1]);
    for(i=0; i<2; i++)
    {
        if (0 == swcfg[i])
        {
            printk("P%d: invalid\n", i);
            continue;
        }
        if (swcfg[i] & CSW_CTRL_IF_TYPE_USER_SPECIFIC)
        {
            sprintf(if_type[i], "%s", "specific");
        }
        else if (swcfg[i] & CSW_CTRL_IF_TYPE_LAN)
        {
            sprintf(if_type[i], "%s", "lan");
        }
        else if (swcfg[i] & CSW_CTRL_IF_TYPE_WAN)
        {
            sprintf(if_type[i], "%s", "wan");
        }

        printk("P%d: interface type: %s\n", i, if_type[i]);
        printk("     vlan id: %04d\n", (swcfg[i]&CSW_CTRL_VLAN_ID)>>VLAN_ID_SHIFT);
        printk("     txclk invert: %s\n", ((swcfg[i]&CSW_CTRL_TXCLK_INVERT)?"on":"off"));
        printk("     rxclk invert: %s\n", ((swcfg[i]&CSW_CTRL_RXCLK_INVERT)?"on":"off"));
        printk("     %sternal phy address: %d\n", ((swcfg[i]&CSW_CTRL_PHY_INTERNAL)?"in":"ex")
                                                , (swcfg[i]&CSW_CTRL_PHY_ADDR));
    }
}

void parse_swcfg(int argc, char **argv)
{
    u32 swcfg[2], value;
    int port_idx;
    char set_input0[8], set_input1[32];
    char *set_input[2];

    sscanf( bootvars.swcfg, "%x,%x", &swcfg[0], &swcfg[1]);
    port_idx = atoi(argv[0]);
    if ((port_idx != 0) && (port_idx != 1))
    {
        printk("port_idx %d is not valid\n", port_idx);
        goto ret;
    }
    sscanf(argv[2], "%x", &value);
    if (!strcmp(argv[1],"if"))
    {
        swcfg[port_idx] &= ~(CSW_CTRL_IF_TYPE_USER_SPECIFIC | CSW_CTRL_IF_TYPE_LAN | CSW_CTRL_IF_TYPE_WAN);
        swcfg[port_idx] |= CSW_CTRL_PHY_ADDR_VALID;
        if (value & (CSW_CTRL_IF_TYPE_USER_SPECIFIC >> IF_TYPE_SHIFT))
        {
            swcfg[port_idx] |= CSW_CTRL_IF_TYPE_USER_SPECIFIC;
        }
        else if (value & (CSW_CTRL_IF_TYPE_LAN >> IF_TYPE_SHIFT))
        {
            swcfg[port_idx] |= CSW_CTRL_IF_TYPE_LAN;
        }
        else if (value & (CSW_CTRL_IF_TYPE_WAN >> IF_TYPE_SHIFT))
        {
            swcfg[port_idx] |= CSW_CTRL_IF_TYPE_WAN;
        }
        else
        {
            swcfg[port_idx] = 0;
        }
    }
    else if (!strcmp(argv[1],"vlan"))
    {
        swcfg[port_idx] &= ~(CSW_CTRL_VLAN_ID);
        swcfg[port_idx] |= ((value & VLAN_ID_BIT) << VLAN_ID_SHIFT);
    }
    else if (!strcmp(argv[1],"txclk"))
    {
        if (value)
        {
            swcfg[port_idx] |= CSW_CTRL_TXCLK_INVERT;
        }
        else
        {
            swcfg[port_idx] &= ~CSW_CTRL_TXCLK_INVERT;
        }
    }
    else if (!strcmp(argv[1],"rxclk"))
    {
        if (value)
        {
            swcfg[port_idx] |= CSW_CTRL_RXCLK_INVERT;
        }
        else
        {
            swcfg[port_idx] &= ~CSW_CTRL_RXCLK_INVERT;
        }
    }
    else if (!strcmp(argv[1],"valid"))
    {
        if (0 == value)
        {
            swcfg[port_idx] = 0;
        }
    }
    else if (!strcmp(argv[1],"inter"))
    {
        if (value)
        {
            swcfg[port_idx] |= CSW_CTRL_PHY_INTERNAL;
        }
        else
        {
            swcfg[port_idx] &= ~CSW_CTRL_PHY_INTERNAL;
        }
    }
    else if (!strcmp(argv[1],"addr"))
    {
        swcfg[port_idx] &= ~CSW_CTRL_PHY_ADDR;
        swcfg[port_idx] |= (value & CSW_CTRL_PHY_ADDR);
    }

    //sprintf( bootvars.swcfg, "0x%08x,0x%08x", swcfg[0], swcfg[1]);
    sprintf(set_input0, "%s", "swcfg");
    sprintf(set_input1, "0x%08x,0x%08x", swcfg[0], swcfg[1]);
    set_input[0] = set_input0;
    set_input[1] = set_input1;

    set_cmd(2, set_input);
    display_swcfg();
    //printk("parse_swcfg, p0: %x, p1: %x\n", swcfg[0], swcfg[1]);
ret:
    return;
}

void swcfg_help(void)
{
    printk("swcfg port_idx if    0/1/2/4 (none/wan/lan/specific)\n");
    printk("               vlan  0~fff   (vlan id)\n");
    printk("               txclk 0|1     (txclk invert)\n");
    printk("               rxclk 0|1     (rxclk invert)\n");
    printk("               valid 0       (phy is not valid)\n");
    printk("               inter 0|1     (phy is internal)\n");
    printk("               addr  0~f     (phy address)\n");
}
/*!
 * function: swcfg_cmd
 *
 *  \brief  switch port control
 *  \param argc
 *  \param argv
 *  \return 0: ok
 */
int swcfg_cmd(int argc, char **argv)
{
    if (argc < 1)
    {
        display_swcfg();
    }
    else if(argc > 2)
    {
        parse_swcfg(argc, argv);
    }
    else
    {
        swcfg_help();
    }

    return ERR_OK;
}

cmdt cmdt_swcfg __attribute__ ((section("cmdt"))) =
{
    "swcfg", swcfg_cmd, "swcfg <field> <value>"
};
