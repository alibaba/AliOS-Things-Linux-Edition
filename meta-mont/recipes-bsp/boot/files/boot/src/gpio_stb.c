/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file gpio_stb.c
*   \brief STB slave by gpio
*   \author Montage
*/
#ifdef CONFIG_STB
/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <arch/chip.h>
#include <common.h>
#include <mt_types.h>
/*=============================================================================+
| Define                                                                       |
+=============================================================================*/

/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/

enum
{
    SD_STATE_IDLE = 1,
    SD_STATE_CD,
    SD_STATE_CMD,
    SD_STATE_RSP,
    SD_STATE_DATA,
    SD_ACT_IDLE = 11,
    SD_ACT_RISING,
    SD_ACT_FALLING
};

#define GPIO_SD_CD              (1 << 4)
#define GPIO_SD_CLK             (1 << 5)
#define GPIO_SD_CMD             (1 << 6)
#define GPIO_SD_DAT0            (1 << 22)
#define GPIO_SD_DAT1            (1 << 23)
#define GPIO_SD_DAT2            (1 << 24)
#define GPIO_SD_DAT3            (1 << 25)
#define GPIO_SD_DAT4            (1 << 26)
#define GPIO_SD_DAT5            (1 << 27)
#define GPIO_SD_DAT6            (1 << 28)
#define GPIO_SD_DAT7            (1 << 29)
#define GPIO_SD_CTRL            (0x07 << 4)
#define GPIO_SD_DATA            (0xFF << 22)
#define GPIO_SD_DIR_CD          (GPIO_SD_CD | GPIO_SD_DATA)
#define GPIO_SD_DIR_CMD         (GPIO_SD_DIR_CD | GPIO_SD_CMD)
#define GPIO_SD_DIR_DATA        (GPIO_SD_DIR_CD)
#define GPIO_SD_SEL             (GPIO_SD_CTRL | GPIO_SD_DATA)

#define MMC_CMD_READ_SINGLE_BLOCK        17
#define MMC_CMD_READ_SINGLE_BLOCK_CRC7   0xc1
#define MMC_DAT_BLOCK_SIZE               512

#define HAL_PAGE_BOUNDARY_SIZE 4096     // 4 KB buffer boundary alignment

/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/
#if 0
static unsigned char Encode(unsigned char Seed, unsigned char Input,
                            unsigned char Depth)
{
    register unsigned char regval;
    register unsigned char count;
    register unsigned char cc;
#define POLYNOM (0x9)
    regval = Seed;
    cc = Input;
    for (count = Depth; count--; cc <<= 1)
    {
        regval = (regval << 1) + ((cc & 0x80) ? 1 : 0);
        if (regval & 0x80)
            regval ^= POLYNOM;
    }
    return (regval & 0x7f);
}

unsigned char cal_crc7(unsigned char *ptr, unsigned char len)
{
    /* len should be 5 */
    /*
     * CRC7 Examples
     * The CRC section of the command/response is bolded.
     * CMD0 (Argument=0)  --> 01 000000 00000000000000000000000000000000 "1001010" 1
     * CMD17 (Argument=0) --> 01 010001 00000000000000000000000000000000 "0101010" 1
     * Response of CMD17  --> 00 010001 00000000000000000000100100000000 "0110011" 1
     */
    unsigned char CrcAccum = 0;
    unsigned char value;

    while (len-- != 0)
    {
        value = *ptr;
        value = Encode(CrcAccum, value, 8);
        CrcAccum = value;
        ptr++;
    }
    value = Encode(CrcAccum, 0, 7);
    value = (value << 1) + 1;
    return value;
}

unsigned int cal_crc16(unsigned char *ptr, unsigned char len)
{
    unsigned char i;
    unsigned int crc = 0;
    while (len-- != 0)
    {
        for (i = 0x80; i != 0; i /= 2)
        {
            if ((crc & 0x8000) != 0)
            {
                crc *= 2;
                crc ^= 0x1021;
            }
            else
                crc *= 2;
            if ((*ptr & i) != 0)
                crc ^= 0x1021;
        }
        ptr++;
    }
    return (crc);
}
#else
#if 0
static const u8 CRC7_Table[256] = {
//    0      1      2      3      4      5      6      7      8      9     10     11     12     13     14     15
    0x00, 0x09, 0x12, 0x1B, 0x24, 0x2D, 0x36, 0x3F, 0x48, 0x41, 0x5A, 0x53,
    0x6C, 0x65, 0x7E, 0x77,
    0x19, 0x10, 0x0B, 0x02, 0x3D, 0x34, 0x2F, 0x26, 0x51, 0x58, 0x43, 0x4A,
    0x75, 0x7C, 0x67, 0x6E,
    0x32, 0x3B, 0x20, 0x29, 0x16, 0x1F, 0x04, 0x0D, 0x7A, 0x73, 0x68, 0x61,
    0x5E, 0x57, 0x4C, 0x45,
    0x2B, 0x22, 0x39, 0x30, 0x0F, 0x06, 0x1D, 0x14, 0x63, 0x6A, 0x71, 0x78,
    0x47, 0x4E, 0x55, 0x5C,
    0x64, 0x6D, 0x76, 0x7F, 0x40, 0x49, 0x52, 0x5B, 0x2C, 0x25, 0x3E, 0x37,
    0x08, 0x01, 0x1A, 0x13,
    0x7D, 0x74, 0x6F, 0x66, 0x59, 0x50, 0x4B, 0x42, 0x35, 0x3C, 0x27, 0x2E,
    0x11, 0x18, 0x03, 0x0A,
    0x56, 0x5F, 0x44, 0x4D, 0x72, 0x7B, 0x60, 0x69, 0x1E, 0x17, 0x0C, 0x05,
    0x3A, 0x33, 0x28, 0x21,
    0x4F, 0x46, 0x5D, 0x54, 0x6B, 0x62, 0x79, 0x70, 0x07, 0x0E, 0x15, 0x1C,
    0x23, 0x2A, 0x31, 0x38,
    0x41, 0x48, 0x53, 0x5A, 0x65, 0x6C, 0x77, 0x7E, 0x09, 0x00, 0x1B, 0x12,
    0x2D, 0x24, 0x3F, 0x36,
    0x58, 0x51, 0x4A, 0x43, 0x7C, 0x75, 0x6E, 0x67, 0x10, 0x19, 0x02, 0x0B,
    0x34, 0x3D, 0x26, 0x2F,
    0x73, 0x7A, 0x61, 0x68, 0x57, 0x5E, 0x45, 0x4C, 0x3B, 0x32, 0x29, 0x20,
    0x1F, 0x16, 0x0D, 0x04,
    0x6A, 0x63, 0x78, 0x71, 0x4E, 0x47, 0x5C, 0x55, 0x22, 0x2B, 0x30, 0x39,
    0x06, 0x0F, 0x14, 0x1D,
    0x25, 0x2C, 0x37, 0x3E, 0x01, 0x08, 0x13, 0x1A, 0x6D, 0x64, 0x7F, 0x76,
    0x49, 0x40, 0x5B, 0x52,
    0x3C, 0x35, 0x2E, 0x27, 0x18, 0x11, 0x0A, 0x03, 0x74, 0x7D, 0x66, 0x6F,
    0x50, 0x59, 0x42, 0x4B,
    0x17, 0x1E, 0x05, 0x0C, 0x33, 0x3A, 0x21, 0x28, 0x5F, 0x56, 0x4D, 0x44,
    0x7B, 0x72, 0x69, 0x60,
    0x0E, 0x07, 0x1C, 0x15, 0x2A, 0x23, 0x38, 0x31, 0x46, 0x4F, 0x54, 0x5D,
    0x62, 0x6B, 0x70, 0x79
};

static const u16 CRC16_Table[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
    0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
    0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
    0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
    0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
    0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
    0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
    0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
    0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
    0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
    0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
    0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
    0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
    0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
    0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
    0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
    0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
    0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

static void Calc_CRC7(u8 * buf, u16 len)
{
    register u8 acc = 0;
    register u16 i;

    buf[0] = 0;
    for (i = 0; i < len; ++i)
    {
        acc = CRC7_Table[(acc << 1) ^ buf[i]];
    }

    buf[6] = ((buf[0] ^ acc) << 1) | 0x1;
}

static u16 Calc_CRC16(u8 * ptr, u16 len)
{
    register u16 crc;
    u8 d;

    crc = 0;
    while (len-- != 0)
    {
        d = (u8) (crc >> 8);
        crc <<= 8;
        crc ^= CRC16_Table[d ^ *ptr];
        ptr++;
    }
    return crc;
}

/*
 * test code
 *
 * calc CRC7 for SD CMD17
 *	buf[1] = 0x11;
 *	for(i=2;i<6;i++) {
 *		buf[i] = 0;
 *	}
 *	Calc_CRC7(buf, 6);
 *	crc7 = buf[6];
 *	printf("crc7=0x%x(CMD17 RSP CRC7 should be 0xc1)\n", crc7);
 *
 * calc CRC16 for SD data block
 *	for(i=0;i<sizeof(buf);i++) {
 *		buf[i] = 0xff;
 *	}
 *	crc16 = Calc_CRC16(buf, 512);
 *	printf("crc16=0x%x(512 bytes with 0xFF data, CRC16 should be 0x7fa1)\n", crc16);
 */
#endif
#endif
static void stb_init(void)
{
    /*
     * GPIO_4  <-> SD_CD_I
     * GPIO_5  <-> SD_CLK_O
     * GPIO_6  <-> SD_CMD_IO
     * GPIO_22 <-> SD_DAT[0]
     * GPIO_23 <-> SD_DAT[1]
     * GPIO_24 <-> SD_DAT[2]
     * GPIO_25 <-> SD_DAT[3]
     * GPIO_26 <-> SD_DAT[4]
     * GPIO_27 <-> SD_DAT[5]
     * GPIO_28 <-> SD_DAT[6]
     * GPIO_29 <-> SD_DAT[7]
     */
    GPREG(PINMUX) &=
        ~(EN_SDIO_FNC | EN_ETH0_FNC | EN_ETH1_FNC | EN_ADC_OUT_FNC |
          EN_MDIO_FNC);
    GPREG(PINMUX) &= ~EN_MDIO_AUX_FNC;
    GPREG(PINMUX) |= EN_FUNCMODE;
}

int stb_go(void)
{
    static u8 buf[HAL_PAGE_BOUNDARY_SIZE]
        __attribute__ ((aligned(HAL_PAGE_BOUNDARY_SIZE)));

    unsigned int sd_dir = GPREG(GPDIR);
    unsigned int sd_val;
    unsigned int ctoken = 0;
    unsigned int ctoken1 = 0;
    unsigned int ctokencnt = 0;
    int state = SD_STATE_CD;
    int act = SD_ACT_IDLE;
    int scl = 1, sclo = 1;
    int i;

    for (i = 0; i < sizeof (buf); i++)
    {
        buf[i] = i % 0x100;
    }
    buf[510] = 0x00;
    buf[511] = 0x00;

    stb_init();
    GPREG(GPSEL) |= GPIO_SD_SEL;

    GPREG(GPSET) = GPIO_SD_CD | GPIO_SD_DATA;
    GPREG(GPDIR) = (sd_dir | GPIO_SD_DIR_CD);

/*************************************************/
    GPREG(GPCLR) = GPIO_SD_CD;

    while (state != SD_STATE_IDLE)
    {
        sd_val = GPREG(GPVAL);

        sclo = scl;
        scl = sd_val & GPIO_SD_CLK;
        if (!sclo && scl)
            act = SD_ACT_RISING;
        else if (sclo && !scl)
            act = SD_ACT_FALLING;
        else
            continue;

        switch (state)
        {
            case SD_STATE_CD:
                if (act == SD_ACT_FALLING)
                {
                    if (!(sd_val & GPIO_SD_CMD))
                    {
                        ctokencnt = 0;
                        state = SD_STATE_CMD;
                    }
                }
                break;
            case SD_STATE_CMD:
                if (act == SD_ACT_FALLING)
                {
                    ctokencnt++;
                    if (ctokencnt == 48)
                    {
                        GPREG(GPSET) = GPIO_SD_CMD;
                        GPREG(GPDIR) = (sd_dir | GPIO_SD_DIR_CMD);

                        ctokencnt = 0;
                        ctoken = MMC_CMD_READ_SINGLE_BLOCK;
                        state = SD_STATE_RSP;
                    }
                }
                break;
            case SD_STATE_RSP:
                if (act == SD_ACT_RISING)
                {
                    if (ctoken & 0x80)
                        GPREG(GPSET) = GPIO_SD_CMD;
                    else
                        GPREG(GPCLR) = GPIO_SD_CMD;
                    ctoken <<= 1;
                    ctokencnt++;
                    if (ctokencnt == 40)
                        ctoken = MMC_CMD_READ_SINGLE_BLOCK_CRC7;
                    else if (ctokencnt == 48)
                        ctoken = 0xff;
                    else if (ctokencnt == 49)
                    {
                        GPREG(GPCLR) = GPIO_SD_DATA;
                        GPREG(GPDIR) = (sd_dir | GPIO_SD_DIR_DATA);

                        ctokencnt = 0;
                        state = SD_STATE_DATA;
                    }
                }
                break;
            case SD_STATE_DATA:
                if (act == SD_ACT_RISING)
                {
                    if (ctokencnt < MMC_DAT_BLOCK_SIZE)
                        GPREG(GPVAL) = buf[ctokencnt] << 22;
                    else if (ctokencnt == MMC_DAT_BLOCK_SIZE)
                        GPREG(GPSET) = GPIO_SD_DATA;
                    else if (ctokencnt >= (MMC_DAT_BLOCK_SIZE + 16 + 1))
                        goto exit;
                    ctokencnt++;
                }
                break;
            default:
                break;
        }
    }
  exit:
    GPREG(GPSET) = GPIO_SD_CD;
    GPREG(GPDIR) = sd_dir;
    return ERR_OK;
}

int stb_cmd(int argc, char *argv[])
{
    return stb_go();
}

cmdt cmdt_stb[] __attribute__ ((section("cmdt"))) =
{
    {
    "stb", stb_cmd, "stb ; do gpio simulation sdhc"}
,};
#endif
