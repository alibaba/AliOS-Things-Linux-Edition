/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file sflash_controller.c
*   \brief c main entry
*   \author Montage
*/

/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <common.h>
#include <otp.h>
#include <lib.h>

#if 1 //defined(CONFIG_FPGA)
    #include "include/flash_config.h"
    #include "include/mt_types.h"
    #ifdef BOOT_MODE_IPL
        #include <common/chip.h>
    #else
        #include <arch/chip.h>
    #endif
    #include <common.h>
#else
    #include "mac_regs.h"
    #include "common.h"
#endif


/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
#define AXI_CLK_MHZ  230
#define PGMEN_USEC   10

#define OTP_BYTE_SIZE         256
#define OTP_BIT_SIZE          (8 * OTP_BYTE_SIZE)
#define OTP_ADDR_SHIFT_BIT    8

//#define OTP_CLK_PLUSE_WIDTH_SHIFT_BIT 21

#if defined(CONFIG_FPGA)
// not doing the delay to speedup testing time, as the OTP is not the real one in FPGA mode
#undef OTP_CLK_PLUSE_WIDTH
#define OTP_CLK_PLUSE_WIDTH       0x003000000UL
#undef OTP_CLK_PLUSE_WIDTH_BIT11
#define OTP_CLK_PLUSE_WIDTH_BIT11 0x0
#endif

#define RF_CTRL_4_REG    0x00004F04UL
    #define OTPPWR_EN_LV      0x40000000UL

/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
// Global variables
unsigned long otp_config;


int efuse_read(u8 *bufp, u32 index_start, u32 element_num)
{
    u32 i, index_max;
    u32 reg_data;

#if 1 // defined(CONFIG_FPGA)
    __REG_WRITE32(OTP_AVDD_REG, 0);
#endif

    index_max = ((index_start + element_num) > OTP_BYTE_SIZE) ? (OTP_BYTE_SIZE) : (index_start + element_num);
    reg_data = OTP_READ_MODE_EN | OTP_CLK_PLUSE_WIDTH;
    __REG_WRITE32(OTP_REG, reg_data);
    __REG_UPDATE32(OTP_AVDD_REG, 0, OTP_CLK_PLUSE_WIDTH_BIT11);
    for (i = index_start; index_max > i; i++)
    {
        // Set address
        reg_data &= ~(OTP_ADDR_VALID_BIT << OTP_ADDR_SHIFT_BIT);
        reg_data |= ((i & OTP_ADDR_VALID_BIT) << OTP_ADDR_SHIFT_BIT);
        __REG_WRITE32(OTP_REG, reg_data);
        // Enable clock generation
        //wait_for_period(1);
        reg_data |= OTP_CLK_PLUSE_GEN;
        __REG_WRITE32(OTP_REG, reg_data);
        // Wait for data being ready
        while (reg_data & OTP_CLK_PLUSE_GEN)
        {
            reg_data = __REG_READ32(OTP_REG);
        }

        bufp[(i - index_start)] = (u8) (reg_data & OTP_DATA_VALID_BIT);
    }

    return (int) (i - index_start);
}

unsigned long otp_read_raw_config(void)
{
    u8 _otp_config[OTP_CONFIG_LEN];

    efuse_read(_otp_config, OTP_CONFIG_BASE, OTP_CONFIG_LEN);

    // reorder bytes to support bi-endian
    return (_otp_config[3] << 24) | (_otp_config[2] << 16) | (_otp_config[1] << 8) | _otp_config[0];
}

void otp_read_config(void)
{
    u8 _otp_config[OTP_CONFIG_LEN];

    otp_config = *((volatile unsigned long *)0xbf005520UL);
    if(0==(otp_config & 0x80000000))
    {
        efuse_read(_otp_config, OTP_CONFIG_BASE, OTP_CONFIG_LEN);
        // reorder bytes to support bi-endian
        otp_config = (_otp_config[3] << 24) | (_otp_config[2] << 16) | (_otp_config[1] << 8) | _otp_config[0];
    }
    //dbg_log(LOG_INFO, "OTP_CONFIG: %08x\n", otp_config);
}

#if !defined(BOOT_MODE_BOOT1)
extern int efuse_write(u8 *bufp, u32 index_start, u32 element_num);
void otp_write_config(unsigned long new_otp_config)
{
    u8 _otp_config[OTP_CONFIG_LEN];

    _otp_config[3] = (new_otp_config >> 24) & 0xff;
    _otp_config[2] = (new_otp_config >> 16) & 0xff;
    _otp_config[1] = (new_otp_config >> 8) & 0xff;
    _otp_config[0] = (new_otp_config) & 0xff;

    printf("Rewrite OTP config to %08x ...", new_otp_config);

    efuse_write(_otp_config, OTP_CONFIG_BASE, OTP_CONFIG_LEN);

    printf(" done\n");
}

#if defined(NOR_4ADDR_MODE) || defined(NAND_OTP_MODE)
extern void delayed_restart_async(void);
#endif

#if defined(NAND_OTP_MODE)

#define USE_ANYBOOT

void otp_program_nand_otp_mode(void)
{
    unsigned long raw_otp_config;
    unsigned long new_otp_config;

    /* do not program otp if the config is a trial one controlled by 0xbf0048fc bit 1 */
    if(0!=(0x01 & *((volatile unsigned long *)0xbf0048FCUL)))
        return;

    raw_otp_config = otp_read_raw_config();

#if defined(USE_ANYBOOT)
    new_otp_config = (0x01 << OTP_ANYBOOT_ENABLE_SHIFT);
#else
    new_otp_config = (otp_config & 0x00000F00)
                  | ((0x01 << OTP_DISABLE_BAD_BLOCK_CHECK_SHIFT)
                  |  (0x01 << OTP_BOOT1_CKSUM_SHIFT)
                  |  (0x01 << OTP_FLASH_BOOT_SELECT_SHIFT)
                  |  (0x01 << OTP_ENABLE_FLASH_SHIFT)
                  |  (0x01 << OTP_UART_SD_BOOT_DISABLE_SHIFT));

    if(raw_otp_config & (0x01 << OTP_ANYBOOT_ENABLE_SHIFT))
        new_otp_config |= ((0x01 << OTP_ANYBOOT_DISABLE_SHIFT)|(0x01 << OTP_ANYBOOT_ENABLE_SHIFT));
#endif

    if((raw_otp_config & new_otp_config) != new_otp_config)
    {
#if defined(USE_ANYBOOT)
        printf("NAND_OTP: enable anyboot, change otp config %lx->%lx\n", raw_otp_config, new_otp_config);
#else
        printf("NAND_OTP: change otp config %lx->%lx\n", raw_otp_config, new_otp_config);
#endif

        otp_write_config(new_otp_config);

        delayed_restart_async();
    }
}
#endif

#if defined(NOR_4ADDR_MODE)
extern int flash_read_bytes(u32 addr, u32 src, int len, u32 check_sel);
extern void sf_flush_cache_all();
void otp_program_nor_4addr_mode(void)
{
    unsigned long otp_config_bak;
    unsigned long new_otp_config;

#define NOR_TEST_READ_LEN 256
    unsigned char temp[NOR_TEST_READ_LEN];

    /* do not program otp if the config is a trial one controlled by 0xbf0048fc bit 1 */
    /* refer to IPL code: allow usage of pmu_software_gpreg to over-write otp_config  */
    if(0==(0x01 & *((volatile unsigned long *)0xbf0048FCUL)))
    {
        if(0==(otp_parse_config(OTP_NOR_4ADDR_SHIFT)))
        {
            otp_config_bak = otp_config;
            new_otp_config = otp_config | (0x01 << OTP_NOR_4ADDR_SHIFT);
            otp_config = new_otp_config;

            /* doing 4ADDR mode flash check, issue 4ADDR read command */
            flash_init(BOOT_FROM_NOR);

            memset((void *) temp, 0, NOR_TEST_READ_LEN);
            sf_flush_cache_all();

            if(0>flash_read_bytes(0, (u32) &temp[0], NOR_TEST_READ_LEN, 0)
               || (*((unsigned long *)&temp[16])!=0xac800000))
            {
                printf("Flash read with 4ADDR failed, possibly wrong boot code is used.\n");
                printf("Please change the boot code and do power-cycle restart.\n");
                /* do not reinit flash, force user to fix the problem */

                otp_config = otp_config_bak;
                //flash_init(BOOT_FROM_NOR);
            }
            else
            {
                otp_write_config(new_otp_config);

                delayed_restart_async();
            }
        }
    }
}
#endif
#endif

unsigned long get_otp_config(void)
{
    return otp_config;
}

int otp_parse_config(int shift_val)
{
    int ret;

    switch (shift_val)
    {
        case OTP_PAGE_SIZE_SHIFT:
            ret = (otp_config >> shift_val) & 0x3;
            break;
        case OTP_SD_CLK_DIV_SHIFT:
            ret = (otp_config >> shift_val) & 0x7;
            break;
        case OTP_FLASH_CLK_DIV_SHIFT:
            ret = (otp_config >> shift_val) & 0x7;
            break;
#if 1
        default:
            ret = (otp_config >> shift_val) & 0x1;
            break;
#else
        case OTP_ENABLE_SECURE_SHIFT:
        case OTP_DISABLE_KEY_WRITE_SHIFT:
        case OTP_KEY_ENCRYPT_SHIFT:
        case OTP_ENABLE_FLASH_SHIFT:
        case OTP_FLASH_BOOT_SELECT_SHIFT:
        case OTP_UART_SD_BOOT_SEL_SHIFT:
        case OTP_WATCHDOG_SHIFT:
        case OTP_JTAG_SHIFT:
        case OTP_READ_CMD_DUMMY_SHIFT:
        case OTP_READ_DATA_DUMMY_SHIFT:
        case OTP_CHECK_BAD_BLOCK_SHIFT:
        case OTP_UART_SD_BOOT_DISABLE_SHIFT:
        case OTP_SD_CLK_INV_SHIFT:
        case OTP_BOOT1_IMAGE_CHECK_SHIFT:
        case OTP_DISABLE_BAD_BLOCK_CHECK_SHIFT:
        case OTP_ONE_SECOND_WATCHDOG_SHIFT:
        case OTP_ANYBOOT_ENABLE_SHIFT:
        case OTP_ANYBOOT_DISABLE_SHIFT:
        case OTP_BOOT1_CKSUM_SHIFT:
        case OTP_NOR_RESET_SHIFT:
        case OTP_NOR_4ADDR_SHIFT:
        case OTP_UART_BOOT_DETECT_SHIFT:
        case OTP_BOOT_FAILURE_DETECT_SHIFT:
            ret = (otp_config >> shift_val) & 0x1;
            break;
        default:
            ret = 0;
#endif
    }

    return ret;
}

int get_boot_block_num(void)
{
    return (0x0FF & (*((volatile unsigned long *)0xbf005524UL) >> 8));
}

int otp_get_boot_type(void)
{
    int boot_type;

    if(chip_revision>=2)
    {
        boot_type = *((volatile unsigned long *)0xbf005524UL);
    }
    else
    {
        // read pin strapping
        unsigned int pin_strap;
        pin_strap = (*(volatile unsigned long *)(PIN_STRAP_REG_ADDR) >> 1) & 0x1;

        if (otp_parse_config(OTP_ENABLE_FLASH_SHIFT))
        {
            if((pin_strap == 1) && (0==otp_parse_config(OTP_UART_SD_BOOT_DISABLE_SHIFT)))
            {
                if (otp_parse_config(OTP_UART_SD_BOOT_SEL_SHIFT))
                    boot_type = BOOT_FROM_UART;
                else
                    boot_type = BOOT_FROM_SD;
            }
            else
            {
                if (otp_parse_config(OTP_FLASH_BOOT_SELECT_SHIFT))
                {
                    boot_type = BOOT_FROM_NAND_WITH_OTP;
                }
                else
                {
                    boot_type = BOOT_FROM_NOR;
                }
            }
        }
        else
        {
            if (pin_strap == 1)
            {
                boot_type = BOOT_FROM_NAND;
            }
            else
            {
                boot_type = BOOT_FROM_NOR;
            }
        }
    }

    return (0x0F & boot_type);
}

#if !defined(IPL) && !defined(BOOT_MODE_BOOT1)
static u8 pgm_byte_data[OTP_BYTE_SIZE]; // this is only for write
int efuse_write(u8 *bufp, u32 index_start, u32 element_num)
{
    u32 addr, bit_offset, byte_offset, index_max, i;
    u32 reg_data;
    u8 tmp_data;
    int ret;
    //u64 pgm_start_time, pgm_end_time;

    // Initial pgm_byte_data
    for (i = 0; i < OTP_BYTE_SIZE; i++)
    {
        pgm_byte_data[i] = 0;
    }

    index_max = ((index_start + element_num) > OTP_BYTE_SIZE) ? (OTP_BYTE_SIZE) : (index_start + element_num);
    if (index_max > index_start)
    {
        // Do not program the programed bit
        for (i = index_start; i < index_max; i++)
        {
            ret = efuse_read(&tmp_data, i, 1);
            if (ret > 0)
            {
                pgm_byte_data[i-index_start] = bufp[i-index_start] & (~tmp_data);
            }
            else
            {
                return -1;
            }
        }
    }
    else
    {
        return -1;
    }

#if 0
    for (i = index_start; i < index_max; i++)
    {
        dbg_log(LOG_INFO, "%04d:%02x\n", i, pgm_byte_data[i]);
    }
#endif
    // Disable read enable
    reg_data = OTP_CLK_PLUSE_WIDTH;
    __REG_WRITE32(OTP_REG, reg_data);
    __REG_UPDATE32(OTP_AVDD_REG, 0, OTP_CLK_PLUSE_WIDTH_BIT11);
    // Enable AVDD
    //get_sim_time(&pgm_start_time);
    __REG_WRITE32(OTP_AVDD_REG, OTP_AVDD_EN);
    //wait_for_period(1000);
    __REG_WRITE32(OTP_AVDD_REG, OTP_PROGRAM_MODE_EN | OTP_AVDD_EN);
    // enable OTPPWR_EN_LV
    __REG_UPDATE32(RF_CTRL_4_REG, OTPPWR_EN_LV, OTPPWR_EN_LV);
    reg_data = OTP_CLK_PLUSE_WIDTH;

    // obyte_data[i] = {odata[896+i],odata[768+i],odata[640+i],odata[512+i],odata[384+i],odata[256+i],odata[128+i],odata[i]};
#if 0
    for (i = 0; OTP_BIT_SIZE > i; i++)
    {
        bit_offset = i / OTP_BYTE_SIZE;
        byte_offset = i % OTP_BYTE_SIZE;

        // Program when data bit doesn't equal to 0
        if (write_byte_data[byte_offset] & (1 << bit_offset))
        {
            // Set address
            reg_data &= ~(OTP_ADDR_VALID_BIT << OTP_ADDR_SHIFT_BIT);
            reg_data |= ((i & OTP_ADDR_VALID_BIT) << OTP_ADDR_SHIFT_BIT);
            // Enable clock generation
            reg_data |= OTP_CLK_PLUSE_GEN;
            __REG_WRITE32(OTP_REG, reg_data);
            // Wait for program done
            while (reg_data & OTP_CLK_PLUSE_GEN)
            {
                reg_data = __REG_READ32(OTP_REG);
            }
        }
    }
#endif

#if defined(CONFIG_FPGA)
    for (byte_offset = index_start; index_max > byte_offset; byte_offset++)
    {
        for (bit_offset = 0; 8 > bit_offset; bit_offset++)
        {
            addr = bit_offset + (8 * byte_offset);
#else
    for (bit_offset = 0; 8 > bit_offset; bit_offset++)
    {
        for (byte_offset = index_start; index_max > byte_offset; byte_offset++)
        {
            addr = (bit_offset * OTP_BYTE_SIZE) + byte_offset;
#endif
            // Check address = [0, 2047]
            if (OTP_BIT_SIZE <= addr)
            {
                dbg_log(LOG_INFO, "!!!Address not valid %x\n", addr);
#if defined(SIM)
                cosim_stop();
#else
                return 0;
#endif
            }

            // Program when data bit doesn't equal to 0
            if (pgm_byte_data[(byte_offset - index_start)] & (1 << bit_offset))
            {
                // Set address
                reg_data &= ~(OTP_ADDR_VALID_BIT << OTP_ADDR_SHIFT_BIT);
                reg_data |= ((addr & OTP_ADDR_VALID_BIT) << OTP_ADDR_SHIFT_BIT);
                __REG_WRITE32(OTP_REG, reg_data);
                // Enable clock generation
                //wait_for_period(1);
                reg_data |= OTP_CLK_PLUSE_GEN;
                __REG_WRITE32(OTP_REG, reg_data);
                // Wait for program done
                while (reg_data & OTP_CLK_PLUSE_GEN)
                {
                    reg_data = __REG_READ32(OTP_REG);
                }
            }
        }
    }
    // Disable program
    __REG_WRITE32(OTP_AVDD_REG, OTP_AVDD_EN);
    reg_data = OTP_CLK_PLUSE_WIDTH;
    __REG_WRITE32(OTP_REG, reg_data);
    //wait_for_period(1000);
    // Disable AVDD
    __REG_WRITE32(OTP_AVDD_REG, 0);
    // disable OTPPWR_EN_LV
    __REG_UPDATE32(RF_CTRL_4_REG, 0, OTPPWR_EN_LV);
    //get_sim_time(&pgm_end_time);

#if 0
    time_gap(8, pgm_start_time, pgm_end_time);
    if (1000000000 < (pgm_end_time - pgm_start_time))
    {
        dbg_log(LOG_INFO, "!!!Programing time exceed 1s\n");
        cosim_stop();
    }
#endif

    return (int)(byte_offset-index_start);
}

void check_result(u8 *write_byte_data, u8 *read_byte_data, int element_num)
{
    int i;
    for (i = 0; i < element_num; i++)
    {
        if (write_byte_data[i] != read_byte_data[i])
        {
            dbg_log(LOG_INFO, "offset %d, !!! Matched Error, expected data = %02x, read data = %02x\n"
                   ,i
                   ,write_byte_data[i]
                   ,read_byte_data[i]);
            //cosim_stop();
        }
    }
    return;
}

//#define OTP_DEBUG
static u8 *otp_bufp = 0;
static u8 *otp_overwrite_bufp = 0;
static u8 otp_data[OTP_BYTE_SIZE*2];
int otp_datalen[OTP_DATA_TYPE_NUM] = {
    0, // custom length
    OTP_LEN_FOFS, //
    OTP_LEN_TXP_DIFF, //
    4, //
    OTP_LEN_MAC,    // MAC
    OTP_LEN_TXVGA,  // OTP_TXVGA
    RESERVED_LEN, RESERVED_LEN  // shuold not match this
};

/*-----------------------------------------------------------------------------
*  \brief check if otp_bufp & otp_overwrite_bufp is initialized
*  \return 1: YES, it's initialized
*  \return 0: NO
-----------------------------------------------------------------------------*/
int check_otp_load(void)
{
    if((otp_bufp != 0x0) && (otp_overwrite_bufp != 0x0))
        return 1;
    dbg_log(LOG_INFO, "OTP data has not been loaded into memory!!\n");
    return 0;
}

/*-----------------------------------------------------------------------------
*  \brief OTP search if otp_id existed
*  \return pos: position of otp_id matched
*  \return < 0: error
-----------------------------------------------------------------------------*/
int otp_search_by_id(int otp_id)
{
    int p, ret = OTP_ID_NOT_FOUND, data_len = 0, len_type, find_id;

    if(otp_id < OTP_MIN || otp_id > OTP_MAX) // error parameter
        return OTP_ERR_PARAM;

    for(p = OTP_START_INDEX; p < OTP_BYTE_SIZE && otp_bufp[p] != OTP_EMPTY_VALUE;)
    {
        if(otp_bufp[p] == OTP_OVERWRITTEN_VALUE)
        {
            ++p;
            continue;
        }

        find_id  = (otp_bufp[p] & OTP_ID_MASK);
        //printf("find_id %x, otp_id %x, otp_buf[%d] %x\n", find_id, otp_id, p, otp_bufp[p]);
        if(otp_id == find_id)
        {
            ret = p;
            break;
        }

        len_type = (otp_bufp[p] & OTP_DATALEN_MASK) >> OTP_DATALEN_SHIFT;
        data_len = (len_type > 0) ? otp_datalen[len_type] : otp_bufp[p + 1];
        p += (len_type > 0) ? 1 : 2;

        //printf("data_len %x, otp_buf[%d] %x\n", data_len, p, otp_bufp[p]);
        p += data_len;
    }

    return ret;
}

int otp_get_avaliable_space(void)
{
    int p = OTP_START_INDEX;
    int len_type, data_len;

    for(p = OTP_START_INDEX; p < OTP_BYTE_SIZE && otp_bufp[p] != OTP_EMPTY_VALUE;)
    {
        if(otp_bufp[p] == OTP_OVERWRITTEN_VALUE)
        {
            ++p;
            continue;
        }

        len_type = (otp_bufp[p] & OTP_DATALEN_MASK) >> OTP_DATALEN_SHIFT;
        data_len = (len_type > 0) ? otp_datalen[len_type] : otp_bufp[p + 1];
        p += (len_type > 0) ? 1 : 2;

        //printf("data_len %x, otp_buf[%d] %x\n", data_len, p, otp_bufp[p]);
        p += data_len;
    }

    return (OTP_BYTE_SIZE - p);
}

/*-----------------------------------------------------------------------------
*  \brief OTP get starting position to write
*  \param len: length of new data
*  \return pos: starting position to write
*  \return < 0: error
-----------------------------------------------------------------------------*/
int otp_get_avaliable_pos_to_write(int len)
{
    int p;
    int len_type, data_len;

    if (len < 1)
    {
        return OTP_ERR_PARAM;
    }

    p = OTP_START_INDEX;

    for(p = OTP_START_INDEX; p < OTP_BYTE_SIZE && otp_bufp[p] != OTP_EMPTY_VALUE;)
    {
        if(otp_bufp[p] == OTP_OVERWRITTEN_VALUE)
        {
            ++p;
            continue;
        }

        len_type = (otp_bufp[p] & OTP_DATALEN_MASK) >> OTP_DATALEN_SHIFT;
        data_len = (len_type > 0) ? otp_datalen[len_type] : otp_bufp[p + 1];
        p += (len_type > 0) ? 1 : 2;

        //printf("data_len %x, otp_buf[%d] %x\n", data_len, p, otp_bufp[p]);
        p += data_len;
    }

    for(len_type = OTP_DATA_TYPE_NUM - 1; len_type > 0; len_type--)
    {
        if(otp_datalen[len_type] == len)
            break;
    }

    if ((len_type == 0 && (p + len + 1) >= OTP_BYTE_SIZE ) ||
        (p + otp_datalen[len_type] >= OTP_BYTE_SIZE))
    {
        return OTP_OUT_OF_LIMIT;
    }

    return p;
}

/*-----------------------------------------------------------------------------
*  \brief OTP read data
*  \param *des_bufp: buffer to keep the data
*  \param id: 1 ~ 31
*  \return len: data length
*  \return < 0: error
-----------------------------------------------------------------------------*/
int otp_read(unsigned char *des_bufp, int otp_id)
{
    int datap, data_len;
    unsigned char *src_bufp;

    if(!check_otp_load())
        return OTP_NOT_INIT;

    datap = otp_search_by_id(otp_id);
    if(datap < 0)
    {
        //dbg_log(LOG_INFO, "OTP search error: %d, otp_id %d\n", datap, otp_id);
        return datap;
    }

    data_len = (otp_bufp[datap] & OTP_DATALEN_MASK) >> OTP_DATALEN_SHIFT;
    if (data_len == 0)
    {
        // [id][len][data]
        datap++;    // move to position of length
        data_len = otp_bufp[datap];
    }
    else
    {
        // [len_type & id][data]
        data_len = otp_datalen[data_len];
    }
    datap++;        // move to position of data
    src_bufp = otp_bufp + datap;

    if (datap + data_len >= OTP_BYTE_SIZE)   // protect for data read
    {
        data_len = OTP_BYTE_SIZE - datap;
    }

    memcpy(des_bufp, src_bufp, data_len);
    return data_len;
}

/*-----------------------------------------------------------------------------
*  \brief OTP write data to memory (not submitted)
*  \param *src_bufp: data to write
*  \param otp_id   : 1 ~ 31
*  \param len      : data length
*  \return ERR_OK  : success
*  \return < 0     : error
-----------------------------------------------------------------------------*/
int otp_write(unsigned char *src_bufp, int otp_id, int len)
{
    int old_datap, old_data_len, i;
    int datap, len_type;
    unsigned char *des_bufp;    // destination

    if (!check_otp_load())
        return OTP_NOT_INIT;

    if(otp_id < OTP_MIN || otp_id > OTP_MAX) // error parameter
        return OTP_ERR_PARAM;

    // ======== starting position to write =============
    datap = otp_get_avaliable_pos_to_write(len);
    if (datap < 0)
    {
        dbg_log(LOG_INFO, "OTP cannot write with error: %d, otp_id %d\n", datap, otp_id);
        return datap;
    }

    for(len_type = OTP_DATA_TYPE_NUM - 1; len_type > 0; len_type--)
    {
        if(otp_datalen[len_type] == len)
            break;
    }

    // =========== find old data ==============
    old_datap = otp_search_by_id(otp_id);
    if(old_datap < 0 && old_datap != OTP_ID_NOT_FOUND)
    {
        dbg_log(LOG_INFO, "OTP search error: %d, otp_id %d\n", datap, otp_id);
        return old_datap;
    }

    // write new data
    otp_bufp[datap] = (len_type << OTP_DATALEN_SHIFT) | otp_id;
    if(len_type > 0)
    {
        //[len_type & id][data]
        des_bufp = (otp_bufp + datap + 1);
    }
    else
    {
        //[id][len][data]
        otp_bufp[datap + 1] = len;
        des_bufp = (otp_bufp + datap + 2);
    }
    memcpy(des_bufp, src_bufp, len);

    // overwrite old data to 0xFF
    if (old_datap == OTP_ID_NOT_FOUND)
    {
        //printf("id not found\n");
        return ERR_OK;
    }

    old_data_len = (otp_bufp[old_datap] & OTP_DATALEN_MASK) >> OTP_DATALEN_SHIFT;
    if (old_data_len == 0)
    {
        // [id][len][data]
        old_data_len = otp_bufp[old_datap + 1];
        old_data_len += 2; // id & len
    }
    else
    {
        // [len_type & id][data]
        old_data_len = otp_datalen[old_data_len];
        old_data_len += 1; // id
    }

    for(i = 0; i < old_data_len; i++)
        otp_overwrite_bufp[old_datap + i] = 0xff;

    return ERR_OK;
}

/*-----------------------------------------------------------------------------
*  \brief OTP load data to memory, initialize OTP read/write data process
*  \return ERR_OK: success
*  \return < 0   : error
*  \details Execute otp_load before read/write otp data.
*           We can reload/sync via execute it again.
-----------------------------------------------------------------------------*/
int otp_load(void)
{
    int ret = 0;
#ifdef OTP_DEBUG
   if (otp_bufp != 0)
   {
       dbg_log(LOG_INFO, "OTP data has been loaded!!\n");
       return ERR_OK;
   }
#endif

    otp_bufp = otp_data;
    otp_overwrite_bufp = otp_data + OTP_BYTE_SIZE;

    for (ret = 0; ret < OTP_BYTE_SIZE; ++ret)
    {
        otp_bufp[ret] = 0x00;
        otp_overwrite_bufp[ret] = 0x00;
    }
#ifndef OTP_DEBUG
    if(0 >= (ret = efuse_read(otp_bufp, 0, OTP_BYTE_SIZE)))
    {
        dbg_log(LOG_ERR, "efuse read error!!!\n");
        return OTP_READ_FAILED;
    }
#endif
    return ERR_OK;
}

#ifdef OTP_DEBUG
void print_otp_mem(void)
{
    int p, c;
    printf("[debug]OTP memory\n");
    for(p = 0; p < 16;p++)
    {
        printf("read buffer[%03d]: ", p*16);
        for(c = 0; c < 16; c++)
        {
            printf("%02x ", otp_bufp[p*16 + c]);
        }
        printf("\n");
    }
}

void print_overw_mem(void)
{
    int p, c;
    printf("\n[debug]OTP overwrite memory\n");
    for(p = 0; p < 16;p++)
    {
        printf("read buffer[%03d]: ", p*16);
        for(c = 0; c < 16; c++)
        {
            printf("%02x ", otp_overwrite_bufp[p*16 + c]);
        }
        printf("\n");
    }
}
#endif

/*-----------------------------------------------------------------------------
*  \brief OTP submit data from memory to OTP(burn it!!)
*  \return ERR_OK : success
*  \return < 0    : error
-----------------------------------------------------------------------------*/
int otp_submit(void)
{
    int p;

    if(!check_otp_load())
        return OTP_NOT_INIT;

    for(p = 0; p < OTP_BYTE_SIZE; p++)
        otp_bufp[p] |= otp_overwrite_bufp[p];
#ifdef OTP_DEBUG
    //print_overw_mem();
    print_otp_mem();
#else
    int ret;
    if(0 >= (ret = efuse_write(otp_bufp, 0, OTP_BYTE_SIZE)))
    {
        dbg_log(LOG_ERR, "efuse write error!!!\n");
        return OTP_WRITE_FAILED;
    }
    if(0 >= (ret = efuse_write(otp_overwrite_bufp, 0, OTP_BYTE_SIZE)))
    {
        dbg_log(LOG_ERR, "efuse write error!!!\n");
        return OTP_WRITE_FAILED;
    }
#endif
    return ERR_OK;
}

void print_otp_buf(unsigned char *p_buf, unsigned int size)
{
    int i;

    for (i = 1; i <= size; i++)
    {
        if (i % 16 == 1)
        {
            printf("0x%04x ", (i - 1));
        }

        printf("%02x ", p_buf[i - 1]);
        if (i % 16 == 0)
        {
            printf("\n");
        }
    }
    printf("\n");
}

int otp_cmd(int argc, char *argv[])
{
    char option = 0;
    int otp_id, len;
    int i = 0, ret = ERR_OK;
    unsigned char otp_buf[OTP_BYTE_SIZE];
    unsigned char *p_buf;
    int number;
    char c[2];
    if (argc < 1)
    {
        return ERR_HELP;
    }
    option = argv[0][0];

    switch(option)
    {
        case 'w':
            if(argc < 4)
            {
                dbg_log(LOG_INFO, "otp w <id in HEX> <len in HEX> <value in string>\n");
                return ERR_PARM;
            }
            if (!hextoul(argv[1], &otp_id))
            {
                dbg_log(LOG_INFO, "id is invalid!\n");
                return ERR_PARM;
            }
            if (!hextoul(argv[2], &len))
            {
                dbg_log(LOG_INFO, "len is invalid!\n");
                return ERR_PARM;
            }
            // parse value
            p_buf = (unsigned char*)argv[3];
            for (i = 0; i < len; i++)
            {
                c[0] = *(p_buf + i * 2);
                c[1] = *(p_buf + i * 2 + 1);
                sscanf(c, "%x", &number);
                otp_buf[i] = number & 0xff;
            }
            dbg_log(LOG_INFO, "== write data id<0x%02x> ==\n", otp_id);
            for(i = 0; i < len; i++)
                dbg_log(LOG_INFO, "0x%02x ", otp_buf[i]);
            dbg_log(LOG_INFO, "\n");
            ret = otp_write(otp_buf, otp_id, len);
            break;
        case 'r':
            if(argc < 2)
            {
                dbg_log(LOG_INFO, "otp r <id in HEX>\n");
                return ERR_PARM;
            }
            if (!hextoul(argv[1], &otp_id))
            {
                dbg_log(LOG_INFO, "id is invalid!\n");
                return ERR_PARM;
            }
            len = otp_read(otp_buf, otp_id);
            #if 1
            if(len > 0)
            {
                dbg_log(LOG_INFO, "== success to read id<0x%02x> ==\n", otp_id);
                for(i = 0; i < len; i++)
                    dbg_log(LOG_INFO, "0x%02x ", otp_buf[i]);
                dbg_log(LOG_INFO, "\n");
            }
            else
            {
                ret = len;
            }
            #endif

            break;
        case 'i':
            ret = otp_load();
            break;
        case 's':
            ret = otp_submit();
            break;
        case 'l':
            len = otp_get_avaliable_space();
            if(len >= 0)
            {
                dbg_log(LOG_INFO, "Size of free space %d Bytes.\n", len);
            }
            else
            {
                ret = len;
            }
            break;
        case 'd':
            {
#ifndef OTP_DEBUG
                unsigned char otp_buf[OTP_BYTE_SIZE];
                efuse_read(otp_buf, 0, OTP_BYTE_SIZE);
                print_otp_buf(otp_buf, OTP_BYTE_SIZE);
#else
                print_otp_mem();
#endif
            }
            break;
#ifndef OTP_DEBUG
        case 'f':
            {
                if(argc < 3)
                {
                    dbg_log(LOG_INFO, "otp f <offset in HEX> <length in HEX>\n");
                    return ERR_PARM;
                }
                int offset = 0;
                unsigned char otp_buf[OTP_BYTE_SIZE];
                if (!hextoul(argv[1], &offset))
                {
                    dbg_log(LOG_INFO, "offset is invalid!\n");
                    return ERR_PARM;
                }

                if (!hextoul(argv[2], &len) || len == 0)
                {
                    dbg_log(LOG_INFO, "length is invalid!\n");
                    return ERR_PARM;
                }
                for(i = 0; i < len; ++i)
                {
                    otp_buf[i] = 0xff;
                }
                efuse_write(otp_buf, offset, len);
                otp_load(); // reload data to memory, for sync
            }
            break;
#endif
        default:
            break;
    }

    if (ret < 0)
    {
        dbg_log(LOG_INFO, "OTP operation error: %d\n", ret);
    }

    return ERR_OK;
}

cmdt cmdt_otp __attribute__ ((section("cmdt"))) =
{
"otp", otp_cmd,   "otp w <id in HEX> <len in HEX> <value in Hex-string>\n"
                  "otp r <id in HEX>; dump data of id\n"
                  "otp i; initialize otp\n"
                  "otp s; submit operations\n"
                  "otp l; get the size(in Decimal) of free space\n"
                  "otp d; dump all data\n"
#ifndef OTP_DEBUG
                  "otp f <offset in HEX> <length in HEX>; invalidate the data (for debug)\n"
#endif
};

#ifdef CONFIG_CMD_OTP

/*!
 * function: otp_cmd
 *
 *  \brief The command to test read/write otp key
 *  \param argc
 *  \param argv
 *  \return
 */
int otpw_cmd(int argc, char *argv[])
{
    unsigned char otp_buf[OTP_BYTE_SIZE] = {0};
    char *p_buf;
    char c[2];
    unsigned int offset = 0;
    unsigned int size = 0;
    int i;
    int number;

    if (argc != 3)
    {
        goto help;
    }

    // parse offset
    if (!hextoul(argv[0], &offset))
    {
        goto err1;
    }
    printf("offset = 0x%x\n", offset);

    // parse size
    if (!hextoul(argv[1], &size))
    {
        goto err1;
    }
    printf("size = 0x%x\n", size);
    if (size == 0)
    {
        printf("The length of key must be nonzero.\n");
        goto err1;
    }

    if ((offset + size) > OTP_BYTE_SIZE)
    {
        printf("Error: read/write otp in invalid address.\n");
        goto err1;
    }

    // parse value
    p_buf = argv[2];
    for (i = 0; i < size; i++)
    {
        c[0] = *(p_buf + i * 2);
        c[1] = *(p_buf + i * 2 + 1);
        sscanf(c, "%x", &number);
        otp_buf[i] = number & 0xff;
        //printf("%d %x\n", i, otp_buf[i]);
    }
    printf("write otp key, offset = 0x%x, size = 0x%x\n", offset, size);
    print_otp_buf(otp_buf, size);
#if 1
    efuse_write(otp_buf, offset, size);
#endif

    return ERR_OK;
  help:
    return ERR_HELP;
  err1:
    return ERR_PARM;
}

int otpr_cmd(int argc, char *argv[])
{
    unsigned char otp_buf[OTP_BYTE_SIZE] = {0};
    unsigned int offset = 0;
    unsigned int size = 0;

    if (argc != 2)
    {
        goto help;
    }

    // parse offset
    if (!hextoul(argv[0], &offset))
    {
        goto err1;
    }
    printf("offset = 0x%x\n", offset);

    // parse size
    if (!hextoul(argv[1], &size))
    {
        goto err1;
    }
    printf("size = 0x%x\n", size);
    if (size == 0)
    {
        printf("The length of key must be nonzero.\n");
        goto err1;
    }

    if ((offset + size) > OTP_BYTE_SIZE)
    {
        printf("Error: read/write otp in invalid address.\n");
        goto err1;
    }

    printf("read otp key, offset = 0x%x, size = 0x%x\n", offset, size);
    efuse_read(otp_buf, offset, size);
    print_otp_buf(otp_buf, size);

    return ERR_OK;
  help:
    return ERR_HELP;
  err1:
    return ERR_PARM;
}

cmdt cmdt_otpw __attribute__ ((section("cmdt"))) =
{
"otpw", otpw_cmd, "otpw <offset> <size> <value>, all arguments need to set in hex format\n"};

cmdt cmdt_otpr __attribute__ ((section("cmdt"))) =
{
"otpr", otpr_cmd, "otpr <offset> <size>, all arguments need to set in hex format\n"};
#endif

#endif

#if 0
//static u8 write_byte_data[OTP_BYTE_SIZE];
//static u8 read_byte_data[OTP_BYTE_SIZE];
void main(void)
{
    int i, ret;
    u16 read_offset, read_len, write_offset, write_len;
    static u8 write_byte_data[OTP_BYTE_SIZE];
    static u8 read_byte_data[OTP_BYTE_SIZE];

    dbg_log(LOG_INFO, "Simulation program start...\n");

    //dbg_log(LOG_INFO, "Read register %x: %08x\n", OTP_REG, __REG_READ32(OTP_REG));

    // Initial write_byte_data and read_byte_data
    for (i = 0; OTP_BYTE_SIZE > i; i++)
    {
        write_byte_data[i] = 0;
        read_byte_data[i] = 0;
    }

    read_offset = 0;
    read_len = 33;
    // Check the initial OTP value
    ret = efuse_read(read_byte_data, read_offset, read_len);
    if (ret > 0)
    {
        check_result(write_byte_data, read_byte_data, ret);
    }
    else
    {
        dbg_log(LOG_INFO, "!!! read no data\n");
        cosim_stop();
    }

    for (i = 0; OTP_BYTE_SIZE > i; i++)
    {
        write_byte_data[i] = 0xff;
    }
#if 0
    for (i = 1; OTP_BYTE_SIZE > i; i++)
    {
        dbg_log(LOG_INFO, "%04d : %02x", i, write_byte_data[i]);
    }
#endif

    write_offset = 0;
    write_len = 33;
    // Program OTP
    ret = efuse_write(write_byte_data, write_offset, write_len);
    if (ret <= 0)
    {
        dbg_log(LOG_INFO, "!!! write no data\n");
        cosim_stop();
    }

    read_offset = write_offset;
    read_len = write_len;
    // Check the OTP value after program
    ret = efuse_read(read_byte_data, read_offset, read_len);
    if (ret > 0)
    {
        check_result(write_byte_data, read_byte_data, ret);
    }
    else
    {
        dbg_log(LOG_INFO, "!!! read no data\n");
        cosim_stop();
    }

    dbg_log(LOG_INFO, "Simulation program done\n");

    cosim_set_test_passed();
    cosim_stop();
}
#endif
