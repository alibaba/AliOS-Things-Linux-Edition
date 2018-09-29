/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*-----------------------------------------------------------------------------+
| OTP Data ID                                                                  |
+-----------------------------------------------------------------------------*/
#ifndef __OTP_H__
#define __OTP_H__
enum
{
    OTP_MIN     = 1,
    OTP_TXVGA   = 1,
    OTP_FOFS    = 2,
    OTP_TXP_DIFF = 3,
    OTP_MAC_ADDR = 4,
    OTP_MAX     = 31,
};

enum
{
    OTP_ID_NOT_FOUND    = -1,
    OTP_NOT_INIT        = -2,
    OTP_ERR_PARAM       = -3,
    OTP_OUT_OF_LIMIT    = -4,
    OTP_WRITE_FAILED    = -5,
    OTP_READ_FAILED     = -6,
};

#define OTP_EMPTY_VALUE         0x00
#define OTP_OVERWRITTEN_VALUE   0xFF

#define OTP_DATA_TYPE_NUM   8
#define OTP_ID_MASK         0x1F
#define OTP_DATALEN_MASK    0xE0
#define OTP_DATALEN_SHIFT   5

#define OTP_START_INDEX     36

#define OTP_LEN_MAC         6
#define OTP_LEN_TXVGA       7
#define OTP_LEN_FOFS        1
#define OTP_LEN_TXP_DIFF    2
#define RESERVED_LEN        100

int otp_load(void);
int otp_submit(void);
int otp_read(unsigned char *des_bufp, int otp_id);
int otp_write(unsigned char *src_bufp, int otp_id, int len);
int otp_get_avaliable_space(void);
int otp_get_boot_type(void);
void otp_read_config(void);
unsigned long get_otp_config(void);

#endif // __OTP_H__
