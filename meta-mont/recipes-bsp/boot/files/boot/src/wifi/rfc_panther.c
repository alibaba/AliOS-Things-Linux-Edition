#ifdef __KERNEL__
    #include <linux/kernel.h>
    #if defined(CONFIG_PANTHER_INTERNAL_DEBUGGER)
        #include <linux/proc_fs.h>
        #include <linux/seq_file.h>
        #include <linux/uaccess.h>
        #include <asm/mach-panther/idb.h>
    #endif
#endif

#include <lib.h>
#include <panther_rf.h>
#include <panther_rfac.h>
#include <rfc_comm.h>
#include <math.h>
#include <rfc.h>
//#include <cmd.h>
#include <cli_api.h>
#include <panther_debug.h>
#include <rf.h>
#include <bb.h>
#include <rfc_panther.h>
#include <os_compat.h>
#include "mac_regs.h"

int printf(char *fmt, ...);

#define RFC_REG_BIT 8
#define RFC_REG_BIT_LONG 10
#define PHASE_SHIFTER_TURNON_THRESHOULD  0.02 /* unit: radian, 0.02*180/pi ~= 1.14 degree */

#define PANTHER_TXCAL_VGA_20MHZ {1, 0, -12}
#define PANTHER_RXCAL_VGA_20MHZ {2, 18, -12}
#define PANTHER_TXCAL_VGA_40MHZ {1, 9, 6}
#define PANTHER_RXCAL_VGA_40MHZ {2, 8, 6}

/* vga entry : {{TXCAL}, {RXCAL_PANTHER}, {TXCAL for RXCAL}, {TXLO}, {RXCAL_LYNX}} */
//struct vga_entry vga_20mhz[5] = {{1, 15, 0, 5}, {3, 20, 0, 0}, {1, 0, 0, 0}, {1, 15, 0, 0}, {0, 18, 3, 3}};	// A board pass, but TC board not work
struct vga_entry vga_20mhz[5] = {{2, 15, 0, 0}, {2, 20, 0, 0}, {1, 0, 0, 0}, {2, 15, 0, 0}, {0, 18, 3, 3}};  // TC board need bigger power(A board)
struct vga_entry vga_40mhz[5] = {{1, 0, -12, 0}, {2, 18, -12, 0}, {1, 0, -12, 0}, {1, 0, -12, 0}, {2, 18, -12, 0}};

double tones_freq_20[FREQ_QUANTITY_20MHZ] = {-5, -2.5, 2.5, 5};
double tones_freq_40[FREQ_QUANTITY_40MHZ] = {-16.25, -13.75, -12.5, -11.25, -7.5, -4.375, -2.5, -1.25, 1.25, 2.5, 4.375, 7.5, 11.25, 12.5, 13.75, 16.25};

#define PANTHER_RFC_TXLO_TONE_NUM	2
#define PANTHER_RFC_TONE_NUM_20	4
#define PANTHER_RFC_TONE_NUM_40	16

//#define DBG_TIME(...) printk(KERN_EMERG ...)
#define DBG_TIME(_str...) DBG_PRINTF(INFO_NO_PREFIX, _str)

//#define PLL_CAL_IN_RFC

//#define RFC_TIME_MEASURE 1
//#define RFC_TIME_MEASURE_TXLO 1

enum
{
    GAIN_I_CAL,
    GAIN_Q_CAL,
    PHI_CAL_1,
    PHI_CAL_2,
    PHI_CAL_3,
};

struct env_params
{
    char iqswap;
    unsigned int lmac_ctrl_val;
    unsigned int rf0x00;
    unsigned int rf0x01;
    unsigned int rf0x08;
    unsigned int rf0x0a;
    unsigned int rf0x12;
    unsigned int rf0x13;
    unsigned int rf0x1d;
    unsigned char bb_1d;
    unsigned char bb_1c;
};

extern unsigned int *rxdc_rec;
extern struct rfc_cal_reg *rfc_result_ptr;
#ifdef CONFIG_RFC_AT_BOOT
extern unsigned int *rxdc_rec_ptr;
#endif

unsigned int pre_rxdc_rec[22], *pre_rxdc_rec_ptr=&pre_rxdc_rec[0]; // 22 = vga_max - vga_min + 1 = 26 - 5 + 1
struct env_params ori_env;

#if defined(RFC_DEBUG)
int rfc_dbg_level = RFC_DBG_LEVEL;
#endif

#ifdef RFC_TIME_MEASURE
int rfc_time_measure = 1;
#endif

#if defined(RFC_ATE)
struct rfc_record_parm ate_record;
#endif


#ifdef RFC_I2C_TEST
extern unsigned long panther_i2c_read_data(unsigned long addr);
extern void panther_i2c_write_data(unsigned long addr, unsigned long data);

    #define PANTHER_RF_REG_BASE	0xc0b00

unsigned int panther_i2c_rf_read(char reg)
{
    unsigned long address = PANTHER_RF_REG_BASE + (reg << 2);
    unsigned int val[2];
    int j;
    int fail_count=0;

    while (fail_count < 10)
    {
        for (j=0; j<2; j++)
            val[j] = (unsigned int) panther_i2c_read_data(address);
        if (val[0] != val[1])
            fail_count++;
        else
            break;
    }

    if (fail_count >= 10)
    {
        val[0] = 0xffffffff;
        serial_printf("%s(): read reg.0x%x(addr:%x) fail\n", __FUNCTION__, reg, address);
    }
#if 0
    else
    {
        serial_printf("%s(): read reg.0x%x(addr:%x) = %x\n", __FUNCTION__, reg, address, val[0]);
    }
#endif

    return val[0];
}

void panther_i2c_rf_write(char reg, unsigned int val)
{
    unsigned long address = PANTHER_RF_REG_BASE + (reg << 2);
    unsigned int read_val;
    int fail_count=0;

    while (fail_count++ <10)
    {
        panther_i2c_write_data(address, val);
        read_val = (unsigned int) panther_i2c_read_data(address);

        if (val == read_val)
            break;
    }

    if (fail_count >= 10)
        serial_printf("%s(): write reg.0x%x = %x fail(read_val=%x)\n", __FUNCTION__, reg, val, read_val);
#if 0
    else
        serial_printf("%s(): write reg.0x%x = %x success\n", __FUNCTION__, reg, val);
#endif
}

void panther_i2c_rf_update(char reg, unsigned int val, unsigned int mask)
{
    unsigned long address = PANTHER_RF_REG_BASE + (reg << 2);
    unsigned int read_val;
    int fail_count=0;

    read_val = panther_i2c_rf_read(reg);
    val = (read_val & (~mask)) | val;

    while (fail_count++ <10)
    {
        panther_i2c_write_data(address, val);
        read_val = (unsigned int) panther_i2c_read_data(address);

        if (val == read_val)
            break;
    }

    if (fail_count >= 10)
        serial_printf("%s(): update reg.0x%x = %x fail(read_val=%x)\n", __FUNCTION__, reg, val, read_val);
#if 0
    else
        serial_printf("%s(): update reg.0x%x = %x success\n", __FUNCTION__, reg, val);
#endif
}

#endif	// RFC_I2C_TEST

#ifdef RFC_TIME_MEASURE
int time_measure(int is_start)
{
    static int start_time = 0;
    int measure_time = 0;

    if (rfc_time_measure)
    {
        if (is_start)
        {
            if (start_time)
                serial_printf("start_time != 0 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            start_time = millis();
        }
        else
        {
            measure_time = millis() - start_time;
            start_time = 0;
        }
    }

    return measure_time;
}
#endif

struct dcoc_data
{
    unsigned char dco_dac_polar;    // 0/1 only (1bit)
    unsigned char dco_dac_n;        // 0~63 ( unsign 6 bit)
    unsigned char dco_dac_p;        // 0~63 ( unsign 6 bit)
};

void rf_req_read_dump(int sel)
{
    // sel : 1=dump, 0=read
    static unsigned int val[30];
    int i;

    if (sel == 1)
    {
        for (i=0; i<30; i++)
        {
            RFC_DBG(RFC_DBG_INFO, "rf reg.%02d = 0x%x\n", i, val[i]);
        }
    }
    else
    {
        for (i=0; i<30; i++)
        {
            val[i] = rf_read(i);
        }
    }
}



//static void set_sampling_rate(int bw)
//{
//    unsigned int mask=(1 << 23);
//    unsigned char bit0=0, bit1=0, bit6=0, bit23=0;
//
//    if (bw)
//    {
//        /* for 80Mhz mode, reverse ADC clock to meet timing*/
//        bit0 = 0x1;
//        bit1 = 0x2;
//        bit6 = 0x40;
//        bit23 = mask;
//#if !defined(CONFIG_FPGA)
//        PLLREG(ADC_CTRL) &= ~(DAC_DIVSEL_80M|ADC_DIVSEL_240M);
//#endif
//    }
//    else
//    {
//#if !defined(CONFIG_FPGA)
//        PLLREG(ADC_CTRL) |= (DAC_DIVSEL_80M|ADC_DIVSEL_240M);
//#endif
//    }
//
//    rf_update(10, bit23, mask);
//    bb_register_write(0, 0x01, ((bb_register_read(0, 0x01) & 0xFD) | bit1));
//    bb_register_write(0, 0x02, ((bb_register_read(0, 0x02) & 0xBF) | bit6)); /* ADC clock invert */
//    bb_register_write(0, 0x32, ((bb_register_read(0, 0x32) & 0xFE) | bit0));
//    bb_register_write(0, 0xf2, ((bb_register_read(0, 0xf2) & 0xfd) | bit1));
//    bb_register_write(0, 0xf3, ((bb_register_read(0, 0xf3) & 0xBF) | bit6));
//}

//#define PANTHER_RFC_PAUSE_CMD
#ifdef PANTHER_RFC_PAUSE_CMD
    #define TX_PAUSE_CMD
    #define RX_PAUSE_CMD
#endif

#ifdef PANTHER_RFC_PAUSE_CMD
u32 is_enable_pause_cmd = 1;
u32 is_keyin_leave = 0;
int panther_rfc_switch_pause(int val)
{
    if (val == 0)
    {
        is_enable_pause_cmd = 0;
    }
    else
    {
        is_enable_pause_cmd = 1;
    }

    return 0;
}
#endif

int panther_rfc_pause(void)
{
    int ret = 0;

#ifdef PANTHER_RFC_PAUSE_CMD
    if (!is_enable_pause_cmd)
    {
        return ret;
    }

    printf("Pause\n");
    while (URREG(URBR) & URBR_RDY);
    if (getchar() == 'q')
    {
        ret = 1;
        is_keyin_leave = 1;
        getchar();
    }

    if (!ret)
    {
        printf("continue...\n");
    }
    else
    {
        printf("leave...\n");
    }
#endif

    return ret;
}

int tonegen_man(int freq, int imag, int qmag, int pwr_drop_db)
{
    u8 reg_val;

    bb_register_write(0, 0x1c, 0xee);
    bb_register_write(0, 0x2f, 0x07);
    bb_register_write(0, 0x2b, freq);
    bb_register_write(0, 0x2c, ((imag << 4) | qmag));

    if (0 > pwr_drop_db)
    {
        DBG_PRINTF(WARN, "WARN: pwrDropdB is less than 0.\n");
        pwr_drop_db = 0;
    }
    else if (5 < pwr_drop_db)
    {
        DBG_PRINTF(WARN, "WARN: pwrDropdB is bigger than 5.\n");
        pwr_drop_db = 5;
    }
    reg_val = bb_register_read(0, 0x32);
    reg_val = (reg_val & 0xf1) | ((pwr_drop_db << 1) & 0x0eUL);
    bb_register_write(0, 0x32, reg_val);

    return 0;
}

int pwrdrop(int pwr_drop_db)
{
    u8 reg_val;

    if (0 > pwr_drop_db)
    {
        DBG_PRINTF(WARN, "WARN: pwrDropdB is less than 0.\n");
        pwr_drop_db = 0;
    }
    else if (5 < pwr_drop_db)
    {
        DBG_PRINTF(WARN, "WARN: pwrDropdB is bigger than 5.\n");
        pwr_drop_db = 5;
    }

    reg_val = bb_register_read(0, 0x32);
    reg_val = (reg_val & 0xf1) | ((pwr_drop_db << 1) & 0x0eUL);
    bb_register_write(0, 0x32, reg_val);

    return 0;
}

#define RFC_ENABLE  (0x04)
static void rfc_env_setup(int is_tx)
{
    unsigned int val;
    u8 reg_val;

#if defined(CONFIG_FPGA) && defined(RFC_I2C_TEST)
    val = (unsigned int) panther_i2c_read_data(0xc0834);
    val |= 0x100000;
    panther_i2c_write_data(0xc0834, val);
#endif

#ifdef CONFIG_PANTHER_FPGA
    lrf_rf_on(is_tx);
    lrf_set_pll(2412);  // channel 7
#else
    //lrf_set_freq(2412);
#endif

    val = 1 << 30;
    rf_update(0, val, val);

    //rf_write(18, 0x240010cc);

    ori_env.lmac_ctrl_val = MACREG_READ32(LMAC_CNTL);
    ori_env.rf0x00 = rf_read(0x0);
    ori_env.rf0x01 = rf_read(0x1);
    ori_env.rf0x08 = rf_read(0x8) & 0x00000180; // Panther used (Lynx no need)
    ori_env.rf0x0a = rf_read(0xa) & 0x00200000;
    ori_env.rf0x12 = rf_read(0x12) & 0xFF800000;    // sam added to test
    ori_env.rf0x13 = rf_read(0x13);
    ori_env.rf0x1d = rf_read(0x1d);
    ori_env.bb_1d = bb_register_read(0, 0x1d);
    ori_env.bb_1c = bb_register_read(0, 0x1c);
    ori_env.iqswap = !!(bb_register_read(0, 0x2) & 0x2);

    MACREG_UPDATE32(LMAC_CNTL, 0, LMAC_CNTL_TSTART); // rfc need the lmac stop
    bb_register_write(0, 0x1d, ori_env.bb_1d | 0x4);    // ADI enable

    /* set BBreg01[2](RFC_ENABLE) */
    reg_val = bb_register_read(0, 0x01);
    reg_val = reg_val | RFC_ENABLE;
    bb_register_write(0, 0x01, reg_val);

    bb_register_write(0, 0x1c, 0xee);   // AD/DA PD disbale
}

static void rfc_env_recovery(void)
{
    u8 reg_val;

    MACREG_UPDATE32(LMAC_CNTL, ori_env.lmac_ctrl_val, LMAC_CNTL_TSTART);
    bb_register_write(0, 0x1d, ori_env.bb_1d);
    bb_register_write(0, 0x1c, ori_env.bb_1c);
    bb_register_write(0, 0x3, 0);   // rx agc mode

    /* clean BBreg01[2](RFC_ENABLE) */
    reg_val = bb_register_read(0, 0x01);
    reg_val = reg_val & (~RFC_ENABLE);
    bb_register_write(0, 0x01, reg_val);

    //rf_update(1, 0, 0x1);		// reg_ctrl_mode: normal mode;
    rf_write(0x0, ori_env.rf0x00);
    rf_write(0x1, ori_env.rf0x01);
    rf_update(0x8, ori_env.rf0x08, 0x00000180);  // Panther used (Lynx no need)
    rf_update(0xa, ori_env.rf0x0a, 0x00200000);
    rf_update(0x12, ori_env.rf0x12, 0xFF800000);
    rf_write(0x13, ori_env.rf0x13);
    rf_write(0x1d, ori_env.rf0x1d);
}

double m_final_i;
double m_final_q;
static void rxvga_adjust_simple(void)
{
    int iqswap = ori_env.iqswap;

    /* force set iqswap = 0*/
    bb_register_write(0, 0x2, (bb_register_read(0, 0x2) & ~0x2));
    ori_env.iqswap = 0;
#if 0
    search_dcoc_lsb_and_track(5, -1, &dcoc_dac_lsb, &dcoc_dac_lsb_track);
    detect_dco_dac(5, dcoc_dac_lsb, dcoc_dac_lsb_track, -1, NULL, NULL);
#else
    detect_dco_dac(5, 2, 1, -1, NULL, NULL, &m_final_i, &m_final_q);    // panther test, date: 08/14
//  detect_dco_dac(5, 0, 5, -1, NULL, NULL);
#endif
    bb_register_write(0, 0x2, (bb_register_read(0, 0x2) & ~0x2) | (iqswap << 1));
    ori_env.iqswap = iqswap;
}

static unsigned short rxvga_adjust(unsigned short ovth, unsigned short okth,  short init_rxvga, int txvga_adjust, int dc_cal_en)
{
    int adjust_success=0, loop=0;
    short mid_rxvga=6;
    short rxvga;
    unsigned short peak_value=0;
    unsigned short gain_step=3;
    int max_gain=15;
    int min_gain=-3;
    int gain_setting_time = 1; // 2 us
    int txgain;
    //int dcoc_dac_lsb, dcoc_dac_lsb_track;
    unsigned int count_time_div64 = 32; // counter time = 2048, => 2048/64=32
    unsigned int delay_us = 12; // 102400/1000; // 2048*2*25
    //unsigned int reg9;
    int iqswap = ori_env.iqswap;

    txvga_adjust = 0;
    dc_cal_en = 1;

    RFC_DBG(RFC_DBG_INFO, "%s(): ovth=%d, okth=%d, init_rxvga=%d, txvga_adjust=%d, dc_cal_en=%d, iqswap=%d\n", __FUNCTION__, ovth, okth,  init_rxvga, txvga_adjust, dc_cal_en, iqswap);

    if ((init_rxvga <= -4) | (init_rxvga >=16))
        rxvga = mid_rxvga;
    else
        rxvga = init_rxvga;

    set_txcal_rxvga(rxvga);
    udelay(gain_setting_time);

    while ((adjust_success == 0) && (loop++ <= 1))
    {
        bb_register_write(0, 0x50, count_time_div64);  // disable peak detector & reset max value
        bb_register_write(0, 0x50, count_time_div64 | 0x80);   // enable peak detector
        udelay(delay_us);
        peak_value = bb_register_read(0, 0x55);

        RFC_DBG(RFC_DBG_INFO, "%s(), the %dnd peak_value=%d, ", __FUNCTION__, loop, peak_value);

        if ((peak_value > okth) && (peak_value < ovth))
        {
            RFC_DBG(RFC_DBG_INFO, "The adjusted rxvga = 0x%x\n", rxvga);
            adjust_success = 1;
        }
        else if (peak_value >= ovth)
        {
            rxvga -= gain_step;
            RFC_DBG(RFC_DBG_INFO, "The peak value >= %d, rx vga drop to %d dB\n", ovth, rxvga);

            if (rxvga < min_gain)
            {
                RFC_DBG(RFC_DBG_INFO, "RX limit to min gain %d dB\n", min_gain);
                rxvga = -3;                        
                if (txvga_adjust)
                {
                    txgain = read_txcal_txvga();
                    txgain -= gain_step;
                    RFC_DBG(RFC_DBG_INFO, "Rxgain is min, drop tx gain to %d dB\n", txgain);
                    set_txcal_txvga(txgain);
                }
            }
        }
        else // too small
        {
            rxvga += gain_step;
            RFC_DBG(RFC_DBG_INFO, "The peak value <= %d, rx vga up to %d dB\n", okth, rxvga);

            if (rxvga > max_gain)
            {
                RFC_DBG(RFC_DBG_INFO, "RX limit to max gain %d dB\n", max_gain);
                rxvga = 15;          
                if (txvga_adjust)
                {
                    txgain = read_txcal_txvga();
                    txgain += gain_step;
                    RFC_DBG(RFC_DBG_INFO, "Rxgain is max, add tx gain to %d dB\n", txgain);
                    set_txcal_txvga(txgain);
                }
            }
        }

        /* program rxvga */
        set_txcal_rxvga(rxvga);
        udelay(gain_setting_time);  //delay 600ns after gain change    

        if (dc_cal_en)
        {
            /* force set iqswap = 0*/
            bb_register_write(0, 0x2, (bb_register_read(0, 0x2) & ~0x2));
            ori_env.iqswap = 0;
#if 0
            search_dcoc_lsb_and_track(5, -1, &dcoc_dac_lsb, &dcoc_dac_lsb_track);
            detect_dco_dac(5, dcoc_dac_lsb, dcoc_dac_lsb_track, -1, NULL, NULL);
#else
            detect_dco_dac(5, 2, 1, -1, NULL, NULL, &m_final_i, &m_final_q);    // panther test, date: 08/14
//          detect_dco_dac(5, 0, 5, -1, NULL, NULL);
#endif
            bb_register_write(0, 0x2, (bb_register_read(0, 0x2) & ~0x2) | (iqswap << 1));
            ori_env.iqswap = iqswap;
        }
    }

    RFC_DBG(RFC_DBG_INFO, "%s(), the final rxvga = %d, peak_value = %d\n", __FUNCTION__, rxvga, peak_value);
    return rxvga;  
}

unsigned short rxvga_adjust_rxloop(unsigned short ovth, unsigned short okth,  short init_rxvga, int txvga_adjust, int dc_cal_en)
{
    int adjust_success=0, loop=0;
    short mid_rxvga=14;
    short rxvga;
    unsigned short peak_value=0;
    unsigned short rx_gain_step=2;
    //unsigned short tx_gain_step=3;
    int max_gain=30;    // modify set_rxcal_rxvga, so change from 20 to 30, date: 08/22
    int min_gain=0;
    int gain_setting_time = 1; // 2 us
    //int txgain;
    //int dcoc_dac_lsb, dcoc_dac_lsb_track;
    unsigned int count_time_div64 = 32; // counter time = 2048, => 2048/64=32
    unsigned int delay_us = 12; //102400/1000; // 2048*2*25
    //unsigned int reg9;
    int iqswap = ori_env.iqswap;

    dc_cal_en = 1;  // disable now becuase this may produce conflict with RXDC, date: 08/21

    RFC_DBG(RFC_DBG_INFO, "%s(): ovth=%d, okth=%d, init_rxvga=%d, txvga_adjust=%d, dc_cal_en=%d, iqswap=%d\n", 
            __FUNCTION__, ovth, okth,  init_rxvga, txvga_adjust, dc_cal_en, iqswap);

    if ((init_rxvga <= -1) | (init_rxvga >=31))
        rxvga = mid_rxvga;
    else
        rxvga = init_rxvga;

    set_rxcal_rxvga(rxvga);
    udelay(gain_setting_time);

    while ((adjust_success == 0) && (loop++ <= 16))
    {
        bb_register_write(0, 0x50, count_time_div64);  // disable peak detector & reset max value
        bb_register_write(0, 0x50, count_time_div64 | 0x80);   // enable peak detector
        udelay(delay_us);
        peak_value = bb_register_read(0, 0x55);

        //RFC_DBG(RFC_DBG_INFO, "%s(), the %dnd peak_value=%d, ", __FUNCTION__, loop, peak_value);

        if ((peak_value > okth) && (peak_value < ovth))
        {
            //RFC_DBG(RFC_DBG_INFO, "The adjusted rxvga = 0x%x\n", rxvga);
            adjust_success = 1;
        }
        else if (peak_value >= ovth)
        {
            rxvga -= rx_gain_step;
            //RFC_DBG(RFC_DBG_INFO, "The peak value >= %d, rx vga drop to %d dB\n", ovth, rxvga);

            if (rxvga < min_gain)
            {
                //RFC_DBG(RFC_DBG_INFO, "RX limit to min gain %d dB\n", min_gain);
                rxvga = 0;                        
            }
        }
        else // too small
        {
            rxvga += rx_gain_step;
            //RFC_DBG(RFC_DBG_INFO, "The peak value <= %d, rx vga up to %d dB\n", okth, rxvga);

            if (rxvga >= max_gain)
            {
                //RFC_DBG(RFC_DBG_INFO, "RX limit to max gain %d dB\n", max_gain);
                rxvga = max_gain;
                break;
            }
        }

        /* program rxvga */
        set_rxcal_rxvga(rxvga);
        udelay(gain_setting_time);  //delay 600ns after gain change    

        if (dc_cal_en)
        {
            /* force set iqswap = 0*/
            bb_register_write(0, 0x2, (bb_register_read(0, 0x2) & ~0x2));
            ori_env.iqswap = 0;
#if 0
            search_dcoc_lsb_and_track(5, -1, &dcoc_dac_lsb, &dcoc_dac_lsb_track);
            detect_dco_dac(5, dcoc_dac_lsb, dcoc_dac_lsb_track, -1, NULL, NULL);
#else
            detect_dco_dac(5, 0, 5, -1, NULL, NULL, &m_final_i, &m_final_q);
#endif
            bb_register_write(0, 0x2, (bb_register_read(0, 0x2) & ~0x2) | (iqswap << 1));
            ori_env.iqswap = iqswap;
        }
    }

    RFC_DBG(RFC_DBG_INFO, "%s(), the final rxvga = %d, peak_value = %d\n", __FUNCTION__, rxvga, peak_value);
    return rxvga;  
}

void set_bb_lnagain(unsigned int input)
{
    /*	input : 2 bits data
        bb reg.3 :
        %Bit7 MSB=Force LNA VGA to fix gain mode
        %Bit[6:5] =LNA gain
        %Bit[4:0] =VGA gain
    */
    unsigned char value;

    value = bb_register_read(0, 0x3);
    value &= 0x1f;
    value |= (0x80 | ((input & 0x3) << 5));

    bb_register_write(0, 3, value);

}

/*	input : 1:On  2:Off
    if ON , write BB22 bit 6 =1 (for example, BB22=0x40=ON)
    if OFF , write BB22 bit 6 =0 
    22=hex address   bb_register_write(0x22,........),
    only write bit6 and keep other bit value in BB22
*/
void rx_auto_cal_on_off(unsigned int input)
{
    u8 reg_val;

    reg_val = bb_register_read(0, 0x22);
    reg_val = (reg_val & 0xbf) | ((input << 6) & 0x40UL);
    bb_register_write(0, 0x22, reg_val);
}

/*	function  RxAutoCalUpdateCycle (unsigned int updatecycle)
    write updatecycle to BB22 [5:4] total 2 bit
    //22=hex address   bb_register_write(0x22,........);
    Hi Hsuan, plz only write bit[5:4] and keep other bit value in BB22
*/
void rx_auto_cal_update_cycle(unsigned int updatecycle)
{
    u8 reg_val;

    reg_val = bb_register_read(0, 0x22);
    reg_val = (reg_val & 0xcf) | ((updatecycle << 4) & 0x30UL);
    bb_register_write(0, 0x22, reg_val);
}

void dcoc_mapping_table(struct dcoc_data *dcoc_dac, int dco_value)
{
    /* 	input range  dco_value: +127~-128 (8 bit ctrl) */
    int dco_value_p;
    double abs_dco_value;

    // DCOC_DAC.dco_dac_polar logic
    if (dco_value > 0)
        dcoc_dac->dco_dac_polar = 1;
    else
        dcoc_dac->dco_dac_polar = 0;

    // DCOC_DAC.dco_dac_n logic
    if (dco_value == -128)
        dco_value_p = -126;
    else
        dco_value_p = dco_value;    

    abs_dco_value = fabs(dco_value_p);
    dcoc_dac->dco_dac_n = floor(abs_dco_value/2);  

    // DCOC_DAC.dco_dac_p logic
    if (dco_value < -125)
        dco_value_p = -125;
    else if (dco_value > 125)
        dco_value_p = 125;
    else
        dco_value_p = dco_value;

    abs_dco_value = fabs(dco_value_p) + 1;
    dcoc_dac->dco_dac_p = floor(abs_dco_value/2); 
}

void dcoc_dac_for_i(struct dcoc_data *dcoc_dac)
{
    /*	struct dcoc_data
        dco_dac_p = dco_dac_ip_ext[5:0]  :	addr11[5:0]
        dco_dac_n = dco_dac_in_ext[5:0]  :	addr11[11:6]
        dco_dac_polar = dco_polar_i_ext  :	addr11[29]
    */
    unsigned int mask=0;
    unsigned int value=0;

    mask = (1<<29) | 0xfff;
    value = (dcoc_dac->dco_dac_polar << 29) | 
            ((dcoc_dac->dco_dac_n & 0x3f) << 6) | 
            (dcoc_dac->dco_dac_p & 0x3f);

    rf_update(11, value, mask);
#if 0
    RFC_DBG(RFC_DBG_INFO, "%s(): write dco_dac_polar=%x, dco_dac_n=0x%x, dco_dac_p=0x%x\n", 
            __FUNCTION__, dcoc_dac->dco_dac_polar, dcoc_dac->dco_dac_n, dcoc_dac->dco_dac_p);
    value = rf_read(11);
    RFC_DBG(RFC_DBG_INFO, "%s(): read dco_dac_polar=%x, dco_dac_n=0x%x, dco_dac_p=0x%x\n", 
            __FUNCTION__, !!(value & (1<<29)), ((value >> 6) & 0x3f), (value & 0x3f));
#endif
}

void dcoc_dac_for_q(struct dcoc_data *dcoc_dac)
{
    /*	struct dcoc_data
        dco_dac_p = dco_dac_qp_ext[5:0]  :	addr11[17:12]
        dco_dac_n = dco_dac_qn_ext[5:0]  :	addr11[23:18]
        dco_dac_polar = dco_polar_q_ext  :	addr11[30]
    */
    unsigned int mask=0;
    unsigned int value=0;

    mask = (1<<30) | (0x3f << 18) | (0x3f << 12);
    value = (dcoc_dac->dco_dac_polar << 30) | 
            ((dcoc_dac->dco_dac_n & 0x3f) << 18) | 
            ((dcoc_dac->dco_dac_p & 0x3f) << 12);

    rf_update(11, value, mask);
#if 0
    RFC_DBG(RFC_DBG_INFO, "%s(): write dco_dac_polar=%x, dco_dac_n=0x%x, dco_dac_p=0x%x\n", 
            __FUNCTION__, dcoc_dac->dco_dac_polar, dcoc_dac->dco_dac_n, dcoc_dac->dco_dac_p);
    value = rf_read(11);
    RFC_DBG(RFC_DBG_INFO, "%s(): read dco_dac_polar=%x, dco_dac_n=0x%x, dco_dac_p=0x%x\n", 
            __FUNCTION__, !!(value & (1<<30)), ((value >> 18) & 0x3f), ((value >> 12) & 0x3f));
#endif
}

void set_dco_dac_lsb_ext(int value)
{
    /* value (2bits) : 
            0 = 5u
            1 = 2.5u
            2 = 1.67u
            3 = 1.25u
    */

    if (value > 3)
    {
        RFC_DBG(RFC_DBG_INFO, "%s(): input error = %x\n", __FUNCTION__, value);
        value = 3;
    }
    else if (value < 0)
    {
        RFC_DBG(RFC_DBG_INFO, "%s(): input error = %x\n", __FUNCTION__, value);
        value = 0;
    }

    rf_update(11, (value << 24), (0x3 << 24));
}

void set_dco_dac_lsb_track_ext(int value)
{
    /*	
            value    finger   if 5u
            %1			44     113.6nA 
            %2			33     151.5nA 
            %3			22     227.3nA 
            %4    		15     343.3nA 
            %5    		10     500nA  
    */

    if (value > 5)
    {
        RFC_DBG(RFC_DBG_INFO, "%s(): input error = %d\n", __FUNCTION__, value);
        value = 5;
    }
    else if (value < 1)
    {
        RFC_DBG(RFC_DBG_INFO, "%s(): input error = %d\n", __FUNCTION__, value);
        value = 1;
    }

    rf_update(11, (value << 26), (0x7 << 26));

}

int check_dcoc_cal_done(void)
{
    /*	rf reg.26 bit.0 : 1 = This VGA is Done, 0 = Under Calibration */

    unsigned int val = rf_read(26);

    return(val & 0x1);
}

int check_dcoc_dac_ctrl(void)
{
    /* rf reg.26 bit.1 : 1=Normal operation Mode, 0=Manual mode during Calibration */
    unsigned int val = rf_read(26);

    return !!(val & 0x2);
}


int dcoc_mapping_i(int dco_value)
{
    /* dco_value: +127~-128 (8 bit ctrl) */
    struct dcoc_data dcoc_dac;
#if 0
    if (check_dcoc_cal_done())
    {
        RFC_DBG(RFC_DBG_INFO, "%s(): This VGA is not under calibration , try set_dcoc_cal_done(0)\n", __FUNCTION__);
        return -1;
    }

    if (check_dcoc_dac_ctrl())
    {
        RFC_DBG(RFC_DBG_INFO, "%s(): Normal operation mode, try set_dcoc_dac_ctrl(0) to enter calibration mode firs\n", __FUNCTION__);
        return -1;
    }
#endif
    dcoc_mapping_table(&dcoc_dac, dco_value);
    dcoc_dac_for_i(&dcoc_dac);

    return 0;
}

int dcoc_mapping_q(int dco_value)
{
    /* dco_value: +127~-128 (8 bit ctrl) */
    struct dcoc_data dcoc_dac;
#if 0
    if (check_dcoc_cal_done())
    {
        RFC_DBG(RFC_DBG_INFO, "%s(): This VGA is not under calibration , try set_dcoc_cal_done(0)\n", __FUNCTION__);
        return -1;
    }

    if (check_dcoc_dac_ctrl())
    {
        RFC_DBG(RFC_DBG_INFO, "%s(): Normal operation mode, try set_dcoc_dac_ctrl(0) to enter calibration mode firs\n", __FUNCTION__);
        return -1;
    }
#endif
    dcoc_mapping_table(&dcoc_dac, dco_value);
    dcoc_dac_for_q(&dcoc_dac);

    return 0;
}

int search_dcoc_lsb_and_track(int vga, int qn_swap_mode, int *dcoc_dac_lsb, int *dcoc_dac_lsb_track)
{
    /* Note : This result (lsb_order, track_order) is come from  
              RF team's assumption +  Our static Algm ,  we have 
              to fine tune the list and apply dynamic algm after 
              we done basic structure 								*/

    int lsb_order[20] = {3,2,3,2,1,3,2,1,3,2,0,1,3,0,2,1,0,1,0,0};
    int track_order[20] = {1,1,2,2,1,3,3,2,4,4,1,3,5,2,5,4,3,5,4,5};
    int dco_dac_sm_i, dco_dac_sm_q;
    int search_limit = 125;
    int k;
    int iqswap = ori_env.iqswap;
    double tmp;
    double start_dc_i, start_dc_q;
    double max_i_path_dc, max_q_path_dc;
    double i_path_check, q_path_check;

    //RFC_DBG(RFC_DBG_INFO, "%s() : vga=%d, qn_swap_mode=%d, iqswap=%d\n", __FUNCTION__, vga, qn_swap_mode, iqswap);

    //set_dcoc_dac_ctrl(0);
    //set_bb_vgagain(vga);
    //set_dcoc_cal_done(0);
    dcoc_mapping_i(0);
    dcoc_mapping_q(0);

    udelay(10);

    // Try to find the dcoc_dac_lsb
    // detect RX DC level
    rx_dc_offset_comp(14, 2, &start_dc_i, &start_dc_q);

    dco_dac_sm_i = search_limit * qn_swap_mode;
    if (start_dc_i >= 0)
        dco_dac_sm_i = -1 * dco_dac_sm_i;

    dco_dac_sm_q = search_limit * qn_swap_mode;
    if (start_dc_q >= 0)
        dco_dac_sm_q = -1 * dco_dac_sm_q;

    if (iqswap)
    {
        tmp = dco_dac_sm_q;
        dco_dac_sm_q = dco_dac_sm_i;
        dco_dac_sm_i = tmp;
    }

    for (k=0; k<20; k++)
    {
        // Try Max to see if it is fit   
        *dcoc_dac_lsb = lsb_order[k];
        *dcoc_dac_lsb_track = track_order[k];

        set_dco_dac_lsb_ext(*dcoc_dac_lsb);
        set_dco_dac_lsb_track_ext(*dcoc_dac_lsb_track);

        dcoc_mapping_i(dco_dac_sm_i);
        dcoc_mapping_q(dco_dac_sm_q);

        udelay(10);

        // detect RX DC level
        rx_dc_offset_comp(14, 2, &max_i_path_dc, &max_q_path_dc);

        if (iqswap)
        {
            tmp = max_i_path_dc;
            max_i_path_dc = max_q_path_dc;
            max_q_path_dc = tmp;
        }

        i_path_check = max_i_path_dc * start_dc_i; //sign multiply, but here we could use sign() onl
        q_path_check = max_q_path_dc * start_dc_q;

#if 0
        RFC_DBG(RFC_DBG_INFO, "start_dc_i = ");
        RFC_DBG_DOUBLE(RFC_DBG_INFO, start_dc_i);
        RFC_DBG(RFC_DBG_INFO, ", start_dc_q = ");
        RFC_DBG_DOUBLE(RFC_DBG_INFO, start_dc_q); RFC_DBG(RFC_DBG_INFO, ")\n");
        RFC_DBG(RFC_DBG_INFO, "max_i_path_dc = ");
        RFC_DBG_DOUBLE(RFC_DBG_INFO, max_i_path_dc);
        RFC_DBG(RFC_DBG_INFO, ", max_q_path_dc = ");
        RFC_DBG_DOUBLE(RFC_DBG_INFO, max_q_path_dc); RFC_DBG(RFC_DBG_INFO, ")\n");
        RFC_DBG(RFC_DBG_INFO, " ======== \n");
#endif
        if ((i_path_check < 0) && (q_path_check < 0))
            break;
#if 0
        else
            RFC_DBG(RFC_DBG_INFO, "Try next level k=%d \n", k+1);
#endif
    }

    //RFC_DBG(RFC_DBG_INFO, "Final dcoc_dac_lsb=%d, dcoc_dac_lsb_track=%d\n", *dcoc_dac_lsb, *dcoc_dac_lsb_track);

    return 0;
}

int detect_dco_dac(int vga, int dcoc_dac_lsb, int dcoc_dac_lsb_track, int qn_swap_mode, int *dcoc_sm_i, int *dcoc_sm_q,
                   double *final_dc_i, double *final_dc_q)
{
    int dco_dac_sm_high_i, dco_dac_sm_i, dco_dac_sm_low_i;
    int dco_dac_sm_high_q, dco_dac_sm_q, dco_dac_sm_low_q;
    double dco_dac_sm_low_i_result = 999;   // it used to integer
    double dco_dac_sm_high_i_result = 999;
    double dco_dac_sm_low_q_result = 999;
    double dco_dac_sm_high_q_result = 999;
    int c;
#if 0
    int degree[7] = {11, 11, 11, 11, 11, 12, 13};
#else
    int degree[7] = {14, 14, 14, 14, 14, 14, 14};
#endif
    int iqswap = ori_env.iqswap;
    double tmp;
    double start_dc_i, start_dc_q;
    double i_path_dc, q_path_dc;

#if 0
    RFC_DBG(RFC_DBG_INFO, "%s() : vga=%d, dcoc_dac_lsb=%d, dcoc_dac_lsb_track=%d, qn_swap=%d, iqswap=%d\n", 
            __FUNCTION__, vga, dcoc_dac_lsb, dcoc_dac_lsb_track, qn_swap_mode, iqswap);
#endif

    //set_dcoc_dac_ctrl(0);
    //set_dcoc_cal_done(0);
    //set_bb_vgagain(vga);
    set_dco_dac_lsb_ext(dcoc_dac_lsb);
    set_dco_dac_lsb_track_ext(dcoc_dac_lsb_track);
    dcoc_mapping_i(0);
    dcoc_mapping_q(0);

    udelay(3);

    /* Try to find the dcoc_dac_lsb */
    //detect RX DC level
    rx_dc_offset_comp(14, 2, &start_dc_i, &start_dc_q);

    if (iqswap)  // for iq swap modify
    {
        tmp = start_dc_q;
        start_dc_q = start_dc_i;
        start_dc_i = tmp;
    }

    if (start_dc_i < 0)
    {
        dco_dac_sm_high_i = 127 * qn_swap_mode;
        dco_dac_sm_i = 64 * qn_swap_mode;
        dco_dac_sm_low_i = 0;
    }
    else
    {
        dco_dac_sm_high_i = 0;
        dco_dac_sm_i = - 64 * qn_swap_mode;   
        dco_dac_sm_low_i = -127 * qn_swap_mode;
    }

    if (start_dc_q < 0)
    {
        dco_dac_sm_high_q = 127 * qn_swap_mode;
        dco_dac_sm_q = 64 * qn_swap_mode;
        dco_dac_sm_low_q = 0;
    }
    else
    {
        dco_dac_sm_high_q = 0;
        dco_dac_sm_q = -64 * qn_swap_mode;
        dco_dac_sm_low_q = -127 * qn_swap_mode;
    }

    //RFC_DBG(RFC_DBG_INFO, "\n======= Start calibrate dco_dac =======\n");

    // loop for DCO DAC coarse
    for (c=0; c<7; c++)
    {
        set_dco_dac_lsb_ext(dcoc_dac_lsb);
        set_dco_dac_lsb_track_ext(dcoc_dac_lsb_track);
        dcoc_mapping_i(dco_dac_sm_i);
        dcoc_mapping_q(dco_dac_sm_q); 

        udelay(10);

        // detect RX DC level
        rx_dc_offset_comp(degree[c], 2, &i_path_dc, &q_path_dc);

        if (iqswap)  // for iq swap modify
        {
            tmp = q_path_dc;
            q_path_dc = i_path_dc;
            i_path_dc = tmp;
        }
#if 0
        RFC_DBG(RFC_DBG_INFO, "dco_dac_sm_i = %d , i_path_dc = ", dco_dac_sm_i);
        RFC_DBG_DOUBLE(RFC_DBG_INFO, i_path_dc);
        RFC_DBG(RFC_DBG_INFO, ", dco_dac_sm_q = %d, q_path_dc = ", dco_dac_sm_q);
        RFC_DBG_DOUBLE(RFC_DBG_INFO, q_path_dc); RFC_DBG(RFC_DBG_INFO, ")\n");
        RFC_DBG(RFC_DBG_INFO, " ====== \n");
#endif
        if (i_path_dc < 0)
        {
            dco_dac_sm_low_i = dco_dac_sm_i;
            dco_dac_sm_low_i_result = i_path_dc;
            dco_dac_sm_i = floor((dco_dac_sm_i + dco_dac_sm_high_i) / 2);
        }
        else
        {
            dco_dac_sm_high_i = dco_dac_sm_i;
            dco_dac_sm_high_i_result = i_path_dc;       
            dco_dac_sm_i = floor((dco_dac_sm_i + dco_dac_sm_low_i) / 2);
        }

        if (q_path_dc < 0)
        {
            dco_dac_sm_low_q = dco_dac_sm_q;
            dco_dac_sm_low_q_result = q_path_dc;
            dco_dac_sm_q = floor((dco_dac_sm_q + dco_dac_sm_high_q) / 2);
        }
        else
        {
            dco_dac_sm_high_q = dco_dac_sm_q;       
            dco_dac_sm_high_q_result = q_path_dc;       
            dco_dac_sm_q = floor((dco_dac_sm_q + dco_dac_sm_low_q) / 2);     
        }
    } 

    // compare high/low
    if (fabs(dco_dac_sm_low_i_result) < fabs(dco_dac_sm_high_i_result))
    {
        dco_dac_sm_i = dco_dac_sm_low_i;
        *final_dc_i = fabs(dco_dac_sm_low_i_result);
    }
    else
    {
        dco_dac_sm_i = dco_dac_sm_high_i;   
        *final_dc_i = fabs(dco_dac_sm_high_i_result);
    }

    if (fabs(dco_dac_sm_low_q_result) < fabs(dco_dac_sm_high_q_result))
    {
        dco_dac_sm_q = dco_dac_sm_low_q;
        *final_dc_q = fabs(dco_dac_sm_low_q_result);
    }
    else
    {
        dco_dac_sm_q = dco_dac_sm_high_q;
        *final_dc_q = fabs(dco_dac_sm_high_q_result);
    }

    // verification
    dcoc_mapping_i(dco_dac_sm_i);
    dcoc_mapping_q(dco_dac_sm_q);
    rx_dc_offset_comp(14, 2, &i_path_dc, &q_path_dc);
    *final_dc_i = i_path_dc;
    *final_dc_q = q_path_dc;
#if 0
//  rx_dc_offset_comp(14, 2, &i_path_dc, &q_path_dc);
    RFC_DBG(RFC_DBG_INFO, "\n======= Final Result: =======\n");
    RFC_DBG(RFC_DBG_INFO, "dco_dac_sm_i = %d , i_path_dc = ", dco_dac_sm_i);
    RFC_DBG_DOUBLE(RFC_DBG_INFO, i_path_dc);
    RFC_DBG(RFC_DBG_INFO, ", dco_dac_sm_q = %d, q_path_dc = ", dco_dac_sm_q);
    RFC_DBG_DOUBLE(RFC_DBG_INFO, q_path_dc); RFC_DBG(RFC_DBG_INFO, ")\n");
#endif
    if (dcoc_sm_i)
        *dcoc_sm_i = dco_dac_sm_i;
    if (dcoc_sm_q)
        *dcoc_sm_q = dco_dac_sm_q;

    return 0;
}

//#define DEBUG_RXDCOC
int vga_dcoc_run_th = 3;
void main_dcoc_calibration(int lna)
{
    // vga calibration range
    int vga_min = 5;
    int vga_max = 26;
    int vga;

    int idx = 0;
    int dcoc_dac_lsb[22], dcoc_dac_lsb_track[22]; // 22 = vga_max - vga_min + 1
    int dcoc_sm_i[22], dcoc_sm_q[22];    // 22 = vga_max - vga_min + 1

    // follow RF team's result
    int qn_swap_mode = -1;

    //////////////////////////////////////
    double dc_rerun_threshold = 30;
    int vga_dcoc_success = 0;
    int vga_dcoc_run = 0;
    double i_dc_check, q_dc_check;
    int loop_count = 1;
    double final_dc_i, final_dc_q;
	int alt_recal, dcoc_dac_lsb_alt, dcoc_dac_lsb_track_alt;
    double dcoc_th = 0.2;
    double dcoc_sm_i_mv;
    double dcoc_sm_q_mv;
    int Itakecare, Qtakecare, vga_take_care;
    //////////////////////////////////////

    RFC_DBG(RFC_DBG_INFO, "%s(): iqswap=%d\n", __FUNCTION__, ori_env.iqswap);

    set_bb_lnagain(lna);

    // set 0 to start
    set_dcoc_dac_ctrl(0);


    // start DCOC calibration
    for (vga = vga_min;  vga <= vga_max; vga++)
    {
        set_bb_vgagain(vga);
        set_dcoc_cal_done(0);

#if 0
        search_dcoc_lsb_and_track(vga, qn_swap_mode, iqswap, &dcoc_dac_lsb[idx], &dcoc_dac_lsb_track[idx]);
#else
        dcoc_dac_lsb[idx] = 1;
        dcoc_dac_lsb_track[idx] = 2;
#endif

        loop_count = 1;
        vga_dcoc_run = 0;
        vga_dcoc_success = 0;
        do
        {
            if (loop_count > 1)
            {
#ifdef DEBUG_RXDCOC
                printf("=======================================\n");
                printf("vga_dcoc_run loop over one time, loop:%d, vga:%d\n", loop_count, vga);
#endif
            }
            loop_count++;

            detect_dco_dac(vga, dcoc_dac_lsb[idx], dcoc_dac_lsb_track[idx], qn_swap_mode, &dcoc_sm_i[idx], &dcoc_sm_q[idx], &final_dc_i, &final_dc_q);
            //check DC
            //rx_dc_offset_comp(14, 2, &i_dc_check, &q_dc_check); //I'm not sure I have to use ref

            //printf("vga = %d, I path mainc dcoc check = ", vga);
            //dbg_double(RFC_DBG_TRUE, i_dc_check); printf(", dcoc_dac check = %f\n", final_dc_i);
            //printf("vga = %d, Q path mainc dcoc check = ", vga);
            //dbg_double(RFC_DBG_TRUE, q_dc_check); printf(", dcoc_dac check = %f\n", final_dc_q);

            //Hi Hsuan ,fabs=abs I want a ABS function, you c what's fit.
            if (fabs(final_dc_i) < dc_rerun_threshold && fabs(final_dc_q) < dc_rerun_threshold)
                vga_dcoc_success =1;
            vga_dcoc_run++;
        } while ((vga_dcoc_success == 0) && (vga_dcoc_run < vga_dcoc_run_th));

#ifdef CONFIG_RFC_AT_BOOT
        /* record the rxdc result */
        rxdc_rec_ptr[idx] = rf_read(11);
#endif		
        set_dcoc_cal_done(1);       

        idx++;
    }

    // set 1 to quit DCOC calibration
    set_dcoc_dac_ctrl(1);

    //Alternative//
    for (idx=12; idx < 21; idx++)
    {
        Itakecare = 0;
        Qtakecare = 0;
        vga_take_care = 0;
        dcoc_sm_i_mv = (double)(dcoc_sm_i[idx-1] + dcoc_sm_i[idx+1])/2;
        dcoc_sm_q_mv = (double)(dcoc_sm_q[idx-1] + dcoc_sm_q[idx+1])/2;

        if ((((double)dcoc_sm_i[idx] - dcoc_sm_i_mv)/ dcoc_sm_i_mv) > dcoc_th
            || (((double)dcoc_sm_i[idx] - dcoc_sm_i_mv)/ dcoc_sm_i_mv) < -dcoc_th)
        {
            Itakecare = 1;
        }
        if ((((double)dcoc_sm_q[idx] - dcoc_sm_q_mv ) / dcoc_sm_q_mv) > dcoc_th
            || (((double)dcoc_sm_q[idx] - dcoc_sm_q_mv ) / dcoc_sm_q_mv) < -dcoc_th)
        {
            Qtakecare = 1;
        }
        vga_take_care = (Itakecare | Qtakecare);
        vga = idx+5;
        i_dc_check = 0;  //default value =0 , if not enter vga_take_care
        q_dc_check = 0; //default value =0 , if not enter vga_take_care
        if (vga_take_care)
        {
#ifdef DEBUG_RXDCOC
            printf("=============VGA = %d, vga_take_care ING !!!!============\n", vga);
#endif
            set_bb_lnagain(lna);
            set_bb_vgagain(vga);
            rx_dc_offset_comp(14, 2, &i_dc_check, &q_dc_check); 

            //I'm not sure I have to use ref
            //printf("ALT i_dc_check=%f, q_dc_check=%f\n", i_dc_check, q_dc_check);
            //printf("ALT dc_rerun_threshold=%f\n", dc_rerun_threshold);
            if (fabs(i_dc_check) < dc_rerun_threshold 
                && fabs(q_dc_check) < dc_rerun_threshold)
            {
#ifdef DEBUG_RXDCOC
                printf("=============VGA = %d, no need to reCal .....============\n", vga);
#endif
                alt_recal=0;
            }
            else
            {
#ifdef DEBUG_RXDCOC
                printf("=============VGA = %d, need to reCal !!!! ============\n", vga);
#endif
                alt_recal=1;
            }
        }
        else
        {
            alt_recal=0;
        }

        if (alt_recal)
        {
            // set 0 to start
            set_bb_lnagain(lna);
            set_bb_vgagain(vga);
            set_dcoc_dac_ctrl(0);
            set_dcoc_cal_done(0);
            dcoc_dac_lsb_alt = 1;
            dcoc_dac_lsb_track_alt = 2;
            loop_count = 1;
            vga_dcoc_run = 0;
            vga_dcoc_success = 0;
            do
            {
                if (loop_count > 0)
                {
#ifdef DEBUG_RXDCOC
                    printf("=============ALT Recal=================\n");
                    printf("vga_dcoc_run loop over one time, loop:%d, vga:%d\n", loop_count, vga);
                    printf("\n");
#endif
                }
                loop_count++;

                detect_dco_dac(vga, dcoc_dac_lsb_alt, dcoc_dac_lsb_track_alt, qn_swap_mode, &dcoc_sm_i[idx], &dcoc_sm_q[idx], &final_dc_i, &final_dc_q);

                if (fabs(final_dc_i) < dc_rerun_threshold && fabs(final_dc_q) < dc_rerun_threshold)
                    vga_dcoc_success =1;
                vga_dcoc_run++;
            } while ((vga_dcoc_success == 0) && (vga_dcoc_run < vga_dcoc_run_th));
        }
    }

    RFC_DBG(RFC_DBG_INFO, "==== %s() result (lna=%d) ====\n", __FUNCTION__, lna);
    for (idx=0; idx<22; idx++)
    {
        RFC_DBG(RFC_DBG_INFO, "vga=%d, lsb=%d, track=%d, dcoc_sm_i=%d, dcoc_sm_q=%d\n", 
                (idx + vga_min), dcoc_dac_lsb[idx], dcoc_dac_lsb_track[idx], 
                dcoc_sm_i[idx], dcoc_sm_q[idx]);
    }
}

void test_rxdcoc_function(int vga, int lsb, int track, int dco_value_i, int dco_value_q)
{
    RFC_DBG(RFC_DBG_INFO, "%s(): vga=%d, lsb=%d, track=%d, dco_value_i=%d, dco_value_q=%d\n", 
            __FUNCTION__, vga, lsb, track, dco_value_i, dco_value_q);
    set_dcoc_dac_ctrl(0);
    set_bb_vgagain(vga);
    set_dcoc_cal_done(0);
    set_dco_dac_lsb_ext(lsb);
    set_dco_dac_lsb_track_ext(track);
    dcoc_mapping_i(dco_value_i);
    dcoc_mapping_q(dco_value_q);
    udelay(10000);
    rx_dc_offset_comp(14, 2, NULL, NULL);
}

double value_i[256], value_q[256];
int rxdcoc_scan(int vga, int lsb, int track)
{
    int k, i;
    //double value_i[256], value_q[256];

    set_dcoc_dac_ctrl(0);
    set_bb_vgagain(vga );
    set_dcoc_cal_done(0);

    //default lsb=0
    set_dco_dac_lsb_ext(lsb);

    //%default track=5
    set_dco_dac_lsb_track_ext(track);

    for (i=0; i<256; i++)
    {
        k = i - 128;
        dcoc_mapping_i(k);
        dcoc_mapping_q(k);   

        udelay(10000);

        rx_dc_offset_comp(14, 2, &value_i[i], &value_q[i]);
    }

    RFC_DBG(RFC_DBG_INFO, "=============== rxdcoc_scan result : ===============\n");
    RFC_DBG(RFC_DBG_INFO, "[dcoc_dac,  rxdc_i,  rxdc_q]\n");
    RFC_DBG(RFC_DBG_INFO, "[");
    for (i=0; i<256; i++)
    {
        k = i - 128;
        RFC_DBG(RFC_DBG_INFO, "%d, ", k);
        RFC_DBG_DOUBLE(RFC_DBG_INFO, value_i[i]);
        RFC_DBG(RFC_DBG_INFO, ", ");
        RFC_DBG_DOUBLE(RFC_DBG_INFO, value_q[i]); RFC_DBG(RFC_DBG_INFO, "...\n");
    }
    RFC_DBG(RFC_DBG_INFO, "]");

    return 0;
}

int rxdc_vga_scan(double *i_path_dc, double *q_path_dc, unsigned int *rf_record)
{
    // vga calibration range
    int vga_min = 5;
    int vga_max = 26;
    int vga;
    int idx = 0;

    for (vga = vga_min; vga <= vga_max; vga++)
    {
        set_bb_vgagain(vga);
        rx_dc_offset_comp(14, 2, &i_path_dc[idx], &q_path_dc[idx]);

        if (rf_record)
            rf_record[idx] = rf_read(35);

        idx++;
    }

    return 0;
}

int rxdcoc_exe_and_verify(int lna)
{
    int vga, vga_min=5, vga_max=26;
    int dc_pass_th=40;  //Pass Threshold
    int arrary_size = vga_max - vga_min + 1;
    int k=0, ret=0;
    double i_dc_before[arrary_size], q_dc_before[arrary_size];
    double i_dc_after[arrary_size], q_dc_after[arrary_size];

#if defined(RFC_DEBUG)
    if (rfc_dbg_level == RFC_DBG_INFO)
        rxdc_vga_scan(i_dc_before, q_dc_before, NULL);
#endif

#ifdef RFC_TIME_MEASURE
    time_measure(1);
#endif
    main_dcoc_calibration(lna);
#ifdef RFC_TIME_MEASURE
    DBG_TIME("[RFC_TIME] %s(%d): main_dcoc_calibration=%d ms\n", __FUNCTION__, __LINE__, time_measure(0));
#endif

#if defined(RFC_DEBUG)
    if (rfc_dbg_level == RFC_DBG_INFO)
        rxdc_vga_scan(i_dc_after, q_dc_after, pre_rxdc_rec_ptr);
#endif

#if defined(RFC_DEBUG)
    if (rfc_dbg_level == RFC_DBG_INFO)
    {
        RFC_DBG(RFC_DBG_INFO, "=============== rxdcoc_exe_and_verify (lna=%d): ===============\n", lna);
        for (vga = vga_min; vga <= vga_max; vga++)
        {
            RFC_DBG(RFC_DBG_INFO, "vga=%d, i_dc_before=", vga);
            RFC_DBG_DOUBLE(RFC_DBG_INFO, i_dc_before[k]);
            RFC_DBG(RFC_DBG_INFO, ", q_dc_before=");
            RFC_DBG_DOUBLE(RFC_DBG_INFO, q_dc_before[k]);
            RFC_DBG(RFC_DBG_INFO, ", i_dc_after=");
            RFC_DBG_DOUBLE(RFC_DBG_INFO, i_dc_after[k]);
            RFC_DBG(RFC_DBG_INFO, ", q_dc_after=");
            RFC_DBG_DOUBLE(RFC_DBG_INFO, q_dc_after[k]);

            if ((fabs(i_dc_after[k]) > dc_pass_th) || (fabs(q_dc_after[k]) > dc_pass_th))
            {
                RFC_DBG(RFC_DBG_INFO, ", *");
                ret = -1;
            }

            RFC_DBG(RFC_DBG_INFO, "\n");

            k++;
        }

        if (ret)
        {
            RFC_DBG(RFC_DBG_INFO, "%s(): fail !!!!!!!!!!!\n", __FUNCTION__);
        }
    }
#endif

    return ret;
}

static double i_path_dc[4][22], q_path_dc[4][22];
int scan_all_lna_vga_dc(void)
{
    int vga_min=5;
    int vga_max=26;
    int j=0, vga, idx;
    //double i_path_dc[4][22], q_path_dc[4][22];

    for (j=0; j<4; j++)
    {
        set_bb_lnagain(j);
        idx = 0;
        for (vga = vga_min; vga <= vga_max; vga++)
        {
            set_bb_vgagain(vga);
            rx_dc_offset_comp(14, 2, &i_path_dc[j][idx], &q_path_dc[j][idx]);
            idx++;
        }
    }

    for (j=0; j<4; j++)
    {
        if (j == 0)
            RFC_DBG(RFC_DBG_INFO, "\nLNA=Ultra Low , Scan DC result\n");
        else if (j == 1)
            RFC_DBG(RFC_DBG_INFO, "\nLNA=Low , Scan DC result\n");
        else if (j == 2)
            RFC_DBG(RFC_DBG_INFO, "\nLNA=Middle , Scan DC result\n");
        else if (j == 3)
            RFC_DBG(RFC_DBG_INFO, "\nLNA=High , Scan DC result\n");

        for (idx=0; idx < 22; idx++)
        {
            RFC_DBG(RFC_DBG_INFO, "vga=%d, i_path_dc=", (idx + vga_min));
            RFC_DBG_DOUBLE(RFC_DBG_INFO, i_path_dc[j][idx]);
            RFC_DBG(RFC_DBG_INFO, ", q_path_dc=");
            RFC_DBG_DOUBLE(RFC_DBG_INFO, q_path_dc[j][idx]);
            RFC_DBG(RFC_DBG_INFO, "\n");
        }
    }

    return 0;
}

#define ADC_CTRL_REG 0xBF004C10UL	// Panther adc & dac sample rate control
void tx_loopback_report(int bw, int freq, unsigned short tg_a_i, 
                        int tg_a_q, int txvga, int rxvga, int bb_scale_fine, unsigned short tx_i_dc, 
                        unsigned short tx_q_dc, unsigned short tx_nm, unsigned short br23_phase, 
                        unsigned short br24_gain, int agc_en, complex *read_f0, 
                        complex *read_2f0, int lpf_reset, int lpf_sel, 
                        int follow_last_rx_gain, int txvga_adjust,
                        unsigned short br23_phase_ext, unsigned short br24_gain_ext)
{
    unsigned char val, bb33;
    complex I_signal, Q_signal;
    complex *tx_signal;
    double ctl_coe;
//  double dc_i, dc_q;
    int init_rxvga;
    int iqswap = ori_env.iqswap;
    volatile u32 *ptr = (unsigned int *)ADC_CTRL_REG;   // Panther adc & dac sample rate control

    /* select the signal for calc */
    if (iqswap)
        tx_signal = &I_signal;
    else
        tx_signal = &Q_signal;
#if 1
    RFC_DBG(RFC_DBG_INFO, "The tx_loopback_report parm: bw = %d, tg_a_i = %d, tg_a_q = %d\n", bw, tg_a_i, tg_a_q);
    RFC_DBG(RFC_DBG_INFO, "txvga = %d, rxvga = %d, tx_i_dc = 0x%x, tx_q_dc = 0x%x\n", txvga, rxvga, tx_i_dc, tx_q_dc);
    RFC_DBG(RFC_DBG_INFO, "tx_nm = 0x%x, br23_phase = 0x%x, br24_gain = 0x%x, agc_en = %d\n", 
            tx_nm, br23_phase, br24_gain, agc_en);
    RFC_DBG(RFC_DBG_INFO, "lpf_reset = %d, lpf_sel = %d, iqswap = %d\n", lpf_reset, lpf_sel, iqswap);
#endif

    if (bw)
    {
        ctl_coe = tones_freq_40[freq];
        *ptr = *ptr & (~0xC);   // change sample clock
    }
    else
    {
        ctl_coe = tones_freq_20[freq];
        *ptr = *ptr | 0xC;
    }

    tx_mux_regs_new(MUX_TONEGEN, tg_a_i, tg_a_q, ctl_coe, bb_scale_fine);

    if (agc_en)
    {
        if (follow_last_rx_gain)
            init_rxvga = read_txcal_rx_gain();
        else
            init_rxvga = rxvga;

        rxvga = rxvga_adjust(RXVGA_OVTH_TX, 90, init_rxvga, txvga_adjust, 1);

        //RFC_DBG(RFC_DBG_INFO, "the new rxvga = %d\n", rxvga);
        //panther_set_iqcal_vga(TXLOOP, rxvga, txvga);
    }
    else
    {
        rxvga_adjust_simple();
    }

//  rx_dc_offset_comp(14, 2, &dc_i, &dc_q);
//  if((dc_i > 40) || (dc_i < -40) || (dc_q > 40) || (dc_q < -40))
//  {
//  RFC_DBG(RFC_DBG_INFO, "\n\n%s(): dc over the correct range !!!, dc_i=", __FUNCTION__);
//  RFC_DBG_DOUBLE(RFC_DBG_INFO, dc_i);
//  RFC_DBG(RFC_DBG_INFO, ", dc_q=");
//  RFC_DBG_DOUBLE(RFC_DBG_INFO, dc_q);
//  RFC_DBG(RFC_DBG_INFO, "\n");
    //return;
//  }

    /* setup bb reg */
    if (tx_nm < 65535)
    {
        val = bb_register_read(0, 0x20);
        bb_register_write(0, 0x20, (val & 0xf) | ((tx_nm & 0xf) << 4));
    }
    if (br23_phase < 65535)
        bb_register_write(0, 0x23, br23_phase);
    if (br24_gain < 65535)
        bb_register_write(0, 0x24, br24_gain);
    if (tx_i_dc < 65535)
    {
        /* This is done by Roger lian before
         * tx_i_dc is a 10bit data, and we separate to two part tx_i_dc_LSB8bit and tx_i_dc_MSB2bit
         * BB25 = tx_i_dc_LSB8bit
         * BB21[1:0] = tx_i_dc_MSB2bit
         */
        bb_register_write(0, 0x25, tx_i_dc & 0x0ffUL);
        val = bb_register_read(0, 0x21);
        val = (val & 0xfc) | ((tx_i_dc & 0x300UL)>>8);
        bb_register_write(0, 0x21, val & 0x0ffUL);

//  	bb_register_write(0, 0x25, tx_i_dc);	=> old code
    }
    if (tx_q_dc < 65535)
    {
        /* This is done by Roger lian before
         * tx_q_dc is a 10bit data, and we separate to two part tx_q_dc_LSB8bit and 				 	tx_q_dc_MSB2bit
         * BB30 = tx_q_dc_LSB8bit
         * BB21[3:2] = tx_q_dc_MSB2bit
         */
        bb_register_write(0, 0x30, tx_q_dc & 0x0ffUL);
        val = bb_register_read(0, 0x21);
        val = (val & 0xf3) | ((tx_q_dc & 0x300UL)>>6);
        bb_register_write(0, 0x21, val & 0x0ffUL);

//  	bb_register_write(0, 0x30, tx_q_dc);	=> old code
    }


    bb33 = bb_register_read(0, 0x33);
    if (br23_phase_ext < 65535)
        bb33 = (bb33 & 0x3F) | ((br23_phase_ext & 0x3) << 6);
    if (br24_gain_ext < 65535)
        bb33 = (bb33 & 0xCF) | ((br24_gain_ext & 0x3) << 4);
    bb_register_write(0, 0x33, bb33);

    RFC_DBG(RFC_DBG_INFO, "bb reg.0x33 = 0x%x\n", bb_register_read(0, 0x33));

    //udelay(2000);

    if (read_f0)
    {
        rx_demod_regs(0x3, ctl_coe);
        read_LPF(&I_signal, &Q_signal, lpf_reset, lpf_sel);
        memcpy(read_f0, tx_signal, sizeof(complex));
        RFC_DBG(RFC_DBG_INFO, "f0: "); RFC_DBG_COMPLEX(RFC_DBG_INFO, *tx_signal);
        RFC_DBG(RFC_DBG_INFO, "\n");
        RFC_DBG(RFC_DBG_INFO, "ABS value: ");
        RFC_DBG_DOUBLE(RFC_DBG_INFO, abs_complex(tx_signal)); RFC_DBG(RFC_DBG_INFO, "\n");
        RFC_DBG(RFC_DBG_INFO, "Angle: ");
        RFC_DBG_DOUBLE(RFC_DBG_INFO, calc_angle(tx_signal->real, tx_signal->imag)); RFC_DBG(RFC_DBG_INFO, " degree\n");
    }
    if (read_2f0)
    {
        ctl_coe_calc(2, ctl_coe, &ctl_coe);
        rx_demod_regs(0x3, ctl_coe);
        read_LPF(&I_signal, &Q_signal, lpf_reset, lpf_sel);
        memcpy(read_2f0, tx_signal, sizeof(complex));
        RFC_DBG(RFC_DBG_INFO, "2f0: "); RFC_DBG_COMPLEX(RFC_DBG_INFO, *tx_signal); 
        RFC_DBG(RFC_DBG_INFO, "\n");
        RFC_DBG(RFC_DBG_INFO, "ABS value: ");
        RFC_DBG_DOUBLE(RFC_DBG_INFO, abs_complex(tx_signal)); RFC_DBG(RFC_DBG_INFO, "\n");
        RFC_DBG(RFC_DBG_INFO, "Angle: ");
        RFC_DBG_DOUBLE(RFC_DBG_INFO, calc_angle(tx_signal->real, tx_signal->imag)); RFC_DBG(RFC_DBG_INFO, " degree\n");
    }
}

void rx_loopback_report(int bw, int tone_idx, int tg_a, 
                        int txvga, int rxvga, int bb_scale_fine, unsigned short rx_i_dc, 
                        unsigned short rx_q_dc, unsigned short rx_nm, unsigned short br28_phase, 
                        unsigned short br29_gain, int agc_en, complex *neg_f0, complex *pos_f0, 
                        int lpf_reset, int lpf_sel, int txvga_adjust)
{
    double ctl_coe;
    int cal_mode_sel = 0x0;
    unsigned char val;
//  double dc_i, dc_q;

#if 1
    RFC_DBG(RFC_DBG_INFO, "%s(): bw=%d, tone_idx=%d, tg_a=%d\n", __FUNCTION__, bw, tone_idx, tg_a);
    RFC_DBG(RFC_DBG_INFO, "txvga=%d, rxvga=%d, rx_i_dc=0x%x, rx_q_dc=0x%x\n", txvga, rxvga, rx_i_dc, rx_q_dc);
    RFC_DBG(RFC_DBG_INFO, "rx_nm=0x%x, br28_phase=0x%x, br29_gain=0x%x, agc_en=%d\n", 
            rx_nm, br28_phase, br29_gain, agc_en);
    RFC_DBG(RFC_DBG_INFO, "lpf_reset=%d, lpf_sel=%d\n", lpf_reset, lpf_sel);
#endif

    if (bw)
        ctl_coe = tones_freq_40[tone_idx];
    else
        ctl_coe = tones_freq_20[tone_idx];

    /* setup bb reg */
    if (rx_nm < 65535)
    {
        val = bb_register_read(0, 0x20);
        bb_register_write(0, 0x20, (val & 0xf0) | (rx_nm & 0xf));
    }
    if (br28_phase < 65535)
        bb_register_write(0, 0x28, br28_phase);
    if (br29_gain < 65535)
        bb_register_write(0, 0x29, br29_gain);
    if (rx_i_dc < 65535)
        bb_register_write(0, 0x2a, rx_i_dc);
    if (rx_q_dc < 65535)
        bb_register_write(0, 0x31, rx_q_dc);

    //Generate Tone
    tx_mux_regs_new(MUX_TONEGEN, tg_a, tg_a, ctl_coe, bb_scale_fine);

    if (agc_en)
    {
        rxvga_adjust_rxloop(RXVGA_OVTH_RX, 85, rxvga, txvga_adjust, 1);
        //RFC_DBG(RFC_DBG_INFO, "the new rxvga = %d\n", rxvga);
    }
    else
    {
        rxvga_adjust_simple();
    }

    //Configure RX Demod & LPF
    rx_demod_regs(cal_mode_sel, ctl_coe);

    if (tone_idx == 0)
        udelay(10000);
    else
        udelay(5000);

    //Read LPF data
    read_LPF(neg_f0, pos_f0, lpf_reset, lpf_sel);

//  rx_dc_offset_comp(14, 2, &dc_i, &dc_q);
//  if((dc_i > 40) || (dc_i < -40) || (dc_q > 40) || (dc_q < -40))
//  {
//  RFC_DBG(RFC_DBG_INFO, "\n\n%s(): dc_i=", __FUNCTION__);
//  RFC_DBG_DOUBLE(RFC_DBG_INFO, dc_i);
//  RFC_DBG(RFC_DBG_INFO, ", dc_q=");
//  RFC_DBG_DOUBLE(RFC_DBG_INFO, dc_q);
//  RFC_DBG(RFC_DBG_INFO, "\n");
//  }
}

int real_dc[MAX_RFC_ITERATIONS];
int image_dc[MAX_RFC_ITERATIONS];
int txlo_count = 0;
int txlo_cal(int bb_scale, int rxvga, int txvga, int bb_scale_fine, int loop_n)
{
    complex f0_result, f0_result_positive, f0_result_negative;
    complex f02_result, f02_result_positive, f02_result_negative;
    int i=0;
    int tone_freq=PANTHER_RFC_TXLO_TONE_NUM, bw=0;
    int tx_nm = 0;
    int br23_phase = 0;
    int br24_gain = 127;
    int bb_max_amp = 512 >> bb_scale;
    //int exe_init_txloop_setting = 0;
    int follow_last_rx_gain = 1;
    int agc_en=1;
    int lpf_reset=1;
    int lpf_sel=0;
    int txvga_adjust=1;

    // this part is use one byte before, but need to be 10 bit in panther, so use int now
    int dc_i_norm_amp, dc_q_norm_amp;
    int real_dc_est=0;
    int real_dc_est_last=0;
    int real_dc_est_pos=0;
    int real_dc_est_neg=0;
    int imag_dc_est=0;
    int imag_dc_est_last=0;
    int imag_dc_est_pos=0;
    int imag_dc_est_neg=0;
    u8 reg_val;

    unsigned short br23_phase_ext =0  ;
    unsigned short br24_gain_ext =0;
#if 0	
    unsigned char buf[4];
#endif
    RFC_DBG(RFC_DBG_INFO, "**************** txlo_cal():  ****************\n");

    //  set bb21 LSB 4bit [3:0] =0 
    //  keep  bb21 MSB 4bit [7:4]
    reg_val = bb_register_read(0, 0x21);
    reg_val = reg_val & (~0x0fUL);
    bb_register_write(0, 0x21, reg_val);

    bb_register_write(0, 0x25, 0);
    bb_register_write(0, 0x30, 0);

    //panther_set_iqcal_vga(TXLOOP, 15, txvga, 0, iqswap);

    /*------------TX LO leakage cancel first step--------*/
    for (i=0; i < loop_n; i++)
    {
        agc_en = 0; // tx time domain spur extect, date: 08/29
        follow_last_rx_gain = 0;

        //----------------send I part first-----------------
#ifdef RFC_TIME_MEASURE_TXLO
        time_measure(1);
#endif
        tx_loopback_report(bw, tone_freq, (unsigned short)bb_scale, 15, txvga, rxvga, bb_scale_fine,
                           real_dc_est, imag_dc_est, tx_nm, 
                           br23_phase, br24_gain, agc_en, &f0_result,
                           &f02_result, lpf_reset, lpf_sel,
                           follow_last_rx_gain, txvga_adjust, br23_phase_ext, 
                           br24_gain_ext);
#ifdef TX_PAUSE_CMD
        if (panther_rfc_pause())
            return 0;
#endif

#ifdef RFC_TIME_MEASURE_TXLO
        DBG_TIME("[RFC_TIME] %s(%d): tx_loopback_report=%d ms\n", __FUNCTION__, __LINE__, time_measure(0));
#endif
        //calc dc_i_norm_amp
#ifdef RFC_TIME_MEASURE_TXLO
        time_measure(1);
#endif
        dc_i_norm_amp = calc_abs_txlo(bb_max_amp, &f0_result, &f02_result, 1);
#ifdef RFC_TIME_MEASURE_TXLO
        DBG_TIME("[RFC_TIME] %s(%d): calc_abs_txlo=%d ms\n", __FUNCTION__, __LINE__, time_measure(0));
#endif
        RFC_DBG(RFC_DBG_INFO, "!! dc_i_norm_amp = %d\n", dc_i_norm_amp);
#ifdef RFC_TIME_MEASURE_TXLO
        time_measure(1);
#endif
        COMPLEX_DIV(f0_result, abs_complex(&f02_result));
#ifdef RFC_TIME_MEASURE_TXLO
        DBG_TIME("[RFC_TIME] %s(%d): calc_abs_txlo=%d ms\n", __FUNCTION__, __LINE__, time_measure(0));
#endif

        RFC_DBG(RFC_DBG_INFO, "I_dc before adjust =");
        RFC_DBG_DOUBLE(RFC_DBG_INFO, abs_complex(&f0_result));
        RFC_DBG(RFC_DBG_INFO, "\n");
#if 0	
        /* FIXME: for debug */
        RFC_DBG(RFC_DBG_INFO, "%s(1), press ENTER to continue.\n", __FUNCTION__);
        WLA_GETS(buf);
        if (strncmp(buf, "c", 1) == 0)
            return 1;
#endif
        //check I part 
        //set real_dc_est= + dc_i_norm_amp
        real_dc_est_pos = real_dc_est_last + dc_i_norm_amp;

        //transfer to 8bit, It will  write to the BB reg later
        real_dc_est = real_dc_est_pos; 

        RFC_DBG(RFC_DBG_INFO, "Try I_dc = %d = %d + %d\n", real_dc_est, real_dc_est_last, dc_i_norm_amp);

        agc_en = 0;
        follow_last_rx_gain = 1;
#ifdef RFC_TIME_MEASURE_TXLO
        time_measure(1);
#endif
        tx_loopback_report(bw, tone_freq, bb_scale, 15, txvga, rxvga, bb_scale_fine,
                           real_dc_est, imag_dc_est, tx_nm, br23_phase, br24_gain, 
                           agc_en, &f0_result_positive, &f02_result_positive, lpf_reset, 
                           lpf_sel, follow_last_rx_gain, 
                           txvga_adjust, br23_phase_ext, br24_gain_ext);
#ifdef TX_PAUSE_CMD
        if (panther_rfc_pause())
            return 0;
#endif

#ifdef RFC_TIME_MEASURE_TXLO
        DBG_TIME("[RFC_TIME] %s(%d): tx_loopback_report=%d ms\n", __FUNCTION__, __LINE__, time_measure(0));
#endif
#if 0	
        /* FIXME: for debug */
        RFC_DBG(RFC_DBG_INFO, "%s(1), press ENTER to continue.\n", __FUNCTION__);
        WLA_GETS(buf);
        if (strncmp(buf, "c", 1) == 0)
            return 1;
#endif
#ifdef RFC_TIME_MEASURE_TXLO
        time_measure(1);
#endif
        COMPLEX_DIV(f0_result_positive, abs_complex(&f02_result_positive));
#ifdef RFC_TIME_MEASURE_TXLO
        DBG_TIME("[RFC_TIME] %s(%d): calc_abs_txlo=%d ms\n", __FUNCTION__, __LINE__, time_measure(0));
#endif

        RFC_DBG(RFC_DBG_INFO, "I_dc positive result = ");
        RFC_DBG_DOUBLE(RFC_DBG_INFO, abs_complex(&f0_result_positive));
        RFC_DBG(RFC_DBG_INFO, "\n");

#if 0	
        /* FIXME: for debug */
        RFC_DBG(RFC_DBG_INFO, "%s(1), press ENTER to continue.\n", __FUNCTION__);
        WLA_GETS(buf);
        if (strncmp(buf, "c", 1) == 0)
            return 1;
#endif
        //set real_dc_est = -dc_i_norm_amp
        real_dc_est_neg = real_dc_est_last - dc_i_norm_amp;        

        //transfer to 8bit, It will  write to the BB reg later
        real_dc_est = real_dc_est_neg;   

        RFC_DBG(RFC_DBG_INFO, "Try I_dc = %d = %d - %d\n", real_dc_est, real_dc_est_last, dc_i_norm_amp);

#ifdef RFC_TIME_MEASURE_TXLO
        time_measure(1);
#endif
        tx_loopback_report(bw, tone_freq, bb_scale, 15, txvga, rxvga, bb_scale_fine, real_dc_est, 
                           imag_dc_est, tx_nm, br23_phase, br24_gain, agc_en,
                           &f0_result_negative, &f02_result_negative, lpf_reset, lpf_sel ,
                           follow_last_rx_gain, txvga_adjust, 
                           br23_phase_ext, br24_gain_ext);
#ifdef TX_PAUSE_CMD
        if (panther_rfc_pause())
            return 0;
#endif

#ifdef RFC_TIME_MEASURE_TXLO
        DBG_TIME("[RFC_TIME] %s(%d): tx_loopback_report=%d ms\n", __FUNCTION__, __LINE__, time_measure(0));
#endif
#if 0	
        /* FIXME: for debug */
        RFC_DBG(RFC_DBG_INFO, "%s(2), press ENTER to continue.\n", __FUNCTION__);
        WLA_GETS(buf);
        if (strncmp(buf, "c", 1) == 0)
            return 1;
#endif
#ifdef RFC_TIME_MEASURE_TXLO
        time_measure(1);
#endif
        COMPLEX_DIV(f0_result_negative, abs_complex(&f02_result_negative));
#ifdef RFC_TIME_MEASURE_TXLO
        DBG_TIME("[RFC_TIME] %s(%d): COMPLEX_DIV=%d ms\n", __FUNCTION__, __LINE__, time_measure(0));
#endif

        RFC_DBG(RFC_DBG_INFO, "I_dc negative result = ");
        RFC_DBG_DOUBLE(RFC_DBG_INFO, abs_complex(&f0_result_negative));
        RFC_DBG(RFC_DBG_INFO, "\n");

#if 0	
        /* FIXME: for debug */
        RFC_DBG(RFC_DBG_INFO, "%s(1), press ENTER to continue.\n", __FUNCTION__);
        WLA_GETS(buf);
        if (strncmp(buf, "c", 1) == 0)
            return 1;
#endif
        // compare 2 result
#ifdef RFC_TIME_MEASURE_TXLO
        time_measure(1);
#endif
        real_dc_est = txlo_pos_neg(real_dc_est_pos, real_dc_est_neg, &f0_result_positive, 
                                   &f0_result_negative, &f0_result);
#ifdef RFC_TIME_MEASURE_TXLO
        DBG_TIME("[RFC_TIME] %s(%d): txlo_pos_neg=%d ms\n", __FUNCTION__, __LINE__, time_measure(0));
#endif
        real_dc_est_last = real_dc_est;
        RFC_DBG(RFC_DBG_INFO, "!!!! real_dc_est = %d\n", real_dc_est);

#if 0	
        /* FIXME: for debug */
        RFC_DBG(RFC_DBG_INFO, "%s(1), press ENTER to continue.\n", __FUNCTION__);
        WLA_GETS(buf);
        if (strncmp(buf, "c", 1) == 0)
            return 1;
#endif
        //--------------------send Q part 2nd-------------------
        agc_en = 0; // tx time domain spur extect, date: 08/29
        follow_last_rx_gain = 0;
#ifdef RFC_TIME_MEASURE_TXLO
        time_measure(1);
#endif
        tx_loopback_report(bw, tone_freq, 15, bb_scale, txvga, rxvga, bb_scale_fine, real_dc_est, 
                           imag_dc_est, tx_nm, br23_phase, br24_gain, agc_en, &f0_result,
                           &f02_result, lpf_reset, lpf_sel,
                           follow_last_rx_gain, txvga_adjust, br23_phase_ext, br24_gain_ext);
#ifdef TX_PAUSE_CMD
        if (panther_rfc_pause())
            return 0;
#endif

#ifdef RFC_TIME_MEASURE_TXLO
        DBG_TIME("[RFC_TIME] %s(%d): tx_loopback_report=%d ms\n", __FUNCTION__, __LINE__, time_measure(0));
#endif

        //calc dc_q_norm_amp
#ifdef RFC_TIME_MEASURE_TXLO
        time_measure(1);
#endif
        dc_q_norm_amp = calc_abs_txlo(bb_max_amp, &f0_result, &f02_result, 1);
#ifdef RFC_TIME_MEASURE_TXLO
        DBG_TIME("[RFC_TIME] %s(%d): calc_abs_txlo=%d ms\n", __FUNCTION__, __LINE__, time_measure(0));
#endif
        RFC_DBG(RFC_DBG_INFO, "!! dc_q_norm_amp = %d\n", dc_q_norm_amp);
#ifdef RFC_TIME_MEASURE_TXLO
        time_measure(1);
#endif
        COMPLEX_DIV(f0_result, abs_complex(&f02_result));
#ifdef RFC_TIME_MEASURE_TXLO
        DBG_TIME("[RFC_TIME] %s(%d): COMPLEX_DIV=%d ms\n", __FUNCTION__, __LINE__, time_measure(0));
#endif

        RFC_DBG(RFC_DBG_INFO, "Q_dc before adjust = ");
        RFC_DBG_DOUBLE(RFC_DBG_INFO, abs_complex(&f0_result));
        RFC_DBG(RFC_DBG_INFO, "\n");

#if 0	
        /* FIXME: for debug */
        RFC_DBG(RFC_DBG_INFO, "%s(3), press ENTER to continue.\n", __FUNCTION__);
        WLA_GETS(buf);
        if (strncmp(buf, "c", 1) == 0)
            return 1;
#endif
        //check Q part 
        //set imag_dc_est= + dc_i_norm_amp
        imag_dc_est_pos = imag_dc_est_last + dc_q_norm_amp;

        //transfer to 8bit, It will  write to the BB reg later
        imag_dc_est = imag_dc_est_pos;    

        RFC_DBG(RFC_DBG_INFO, "Try Q_dc = %d = %d + %d\n", imag_dc_est, imag_dc_est_last, dc_q_norm_amp);

        agc_en = 0;
        follow_last_rx_gain = 1;
#ifdef RFC_TIME_MEASURE_TXLO
        time_measure(1);
#endif
        tx_loopback_report(bw, tone_freq, 15, bb_scale, txvga, rxvga, bb_scale_fine, real_dc_est, 
                           imag_dc_est, tx_nm, br23_phase, br24_gain, agc_en, 
                           &f0_result_positive, &f02_result_positive, lpf_reset, lpf_sel,
                           follow_last_rx_gain, txvga_adjust, 
                           br23_phase_ext, br24_gain_ext);
#ifdef TX_PAUSE_CMD
        if (panther_rfc_pause())
            return 0;
#endif

#ifdef RFC_TIME_MEASURE_TXLO
        DBG_TIME("[RFC_TIME] %s(%d): tx_loopback_report=%d ms\n", __FUNCTION__, __LINE__, time_measure(0));
#endif

#ifdef RFC_TIME_MEASURE_TXLO
        time_measure(1);
#endif
        COMPLEX_DIV(f0_result_positive, abs_complex(&f02_result_positive));
#ifdef RFC_TIME_MEASURE_TXLO
        DBG_TIME("[RFC_TIME] %s(%d): COMPLEX_DIV=%d ms\n", __FUNCTION__, __LINE__, time_measure(0));
#endif

        RFC_DBG(RFC_DBG_INFO, "Q_dc positive result = ");
        RFC_DBG_DOUBLE(RFC_DBG_INFO, abs_complex(&f0_result_positive));
        RFC_DBG(RFC_DBG_INFO, "\n");

#if 0	
        /* FIXME: for debug */
        RFC_DBG(RFC_DBG_INFO, "%s(4), press ENTER to continue.\n", __FUNCTION__);
        WLA_GETS(buf);
        if (strncmp(buf, "c", 1) == 0)
            return 1;
#endif
        //%set imag_dc_est = - dc_q_norm_amp
        imag_dc_est_neg = imag_dc_est_last - dc_q_norm_amp;
        imag_dc_est = imag_dc_est_neg;

        RFC_DBG(RFC_DBG_INFO, "Try Q_dc = %d = %d - %d\n", imag_dc_est, imag_dc_est_last, dc_q_norm_amp);

#ifdef RFC_TIME_MEASURE_TXLO
        time_measure(1);
#endif
        tx_loopback_report(bw, tone_freq, 15, bb_scale, txvga, rxvga, bb_scale_fine, real_dc_est, 
                           imag_dc_est, tx_nm, br23_phase, br24_gain, agc_en,
                           &f0_result_negative, &f02_result_negative, lpf_reset, lpf_sel,
                           follow_last_rx_gain, txvga_adjust, br23_phase_ext, br24_gain_ext);
#ifdef TX_PAUSE_CMD
        if (panther_rfc_pause())
            return 0;
#endif

#ifdef RFC_TIME_MEASURE_TXLO
        DBG_TIME("[RFC_TIME] %s(%d): tx_loopback_report=%d ms\n", __FUNCTION__, __LINE__, time_measure(0));
#endif

#ifdef RFC_TIME_MEASURE_TXLO
        time_measure(1);
#endif
        COMPLEX_DIV(f0_result_negative, abs_complex(&f02_result_negative));
#ifdef RFC_TIME_MEASURE_TXLO
        DBG_TIME("[RFC_TIME] %s(%d): COMPLEX_DIV=%d ms\n", __FUNCTION__, __LINE__, time_measure(0));
#endif

        RFC_DBG(RFC_DBG_INFO, "Q_dc negative result = ");
        RFC_DBG_DOUBLE(RFC_DBG_INFO, abs_complex(&f0_result_negative));
        RFC_DBG(RFC_DBG_INFO, "\n");

#if 0	
        /* FIXME: for debug */
        RFC_DBG(RFC_DBG_INFO, "%s(4), press ENTER to continue.\n", __FUNCTION__);
        WLA_GETS(buf);
        if (strncmp(buf, "c", 1) == 0)
            return 1;
#endif
        //compare 2 result
#ifdef RFC_TIME_MEASURE_TXLO
        time_measure(1);
#endif
        imag_dc_est = txlo_pos_neg(imag_dc_est_pos, imag_dc_est_neg, &f0_result_positive,
                                   &f0_result_negative, &f0_result);
#ifdef RFC_TIME_MEASURE_TXLO
        DBG_TIME("[RFC_TIME] %s(%d): txlo_pos_neg=%d ms\n", __FUNCTION__, __LINE__, time_measure(0));
#endif

        RFC_DBG(RFC_DBG_INFO, "** txlo_cal: f0_result_positive ="); 
        RFC_DBG_COMPLEX(RFC_DBG_INFO, f0_result_positive); RFC_DBG(RFC_DBG_INFO, "\n");
        RFC_DBG(RFC_DBG_INFO, "** txlo_cal: f0_result_negative ="); 
        RFC_DBG_COMPLEX(RFC_DBG_INFO, f0_result_negative); RFC_DBG(RFC_DBG_INFO, "\n");
        RFC_DBG(RFC_DBG_INFO, "** txlo_cal: f0_result = "); 
        RFC_DBG_COMPLEX(RFC_DBG_INFO, f0_result); RFC_DBG(RFC_DBG_INFO, "\n");

        imag_dc_est_last = imag_dc_est;
        RFC_DBG(RFC_DBG_INFO, "!!!! imag_dc_est = %d\n", imag_dc_est);

#if 0	
        /* FIXME: for debug */
        RFC_DBG(RFC_DBG_INFO, "%s(4), press ENTER to continue.\n", __FUNCTION__);
        WLA_GETS(buf);
        if (strncmp(buf, "c", 1) == 0)
            return 1;
#endif
    }

    RFC_DBG(RFC_DBG_INFO, "++++++++ txlo_cal final result ++++++++\n");
    RFC_DBG(RFC_DBG_INFO, "real_dc_est = %d\n", real_dc_est);
    RFC_DBG(RFC_DBG_INFO, "imag_dc_est = %d\n", imag_dc_est);

    if (fabs(real_dc_est) > 100)
        real_dc_est = 0;
    if (fabs(imag_dc_est) > 100)
        imag_dc_est = 0;

    real_dc[txlo_count] = real_dc_est;
    image_dc[txlo_count] = imag_dc_est;

//  bb_register_write(0, 0x25, (u8) real_dc_est);	=> old code
//  bb_register_write(0, 0x30, (u8) imag_dc_est);	=> old code

    // real_dc_est , imag_dc_est is a 10bit data,  Modify this put LSB 8bit to (0x25, 0x30) and MSB to BB21
    reg_val = bb_register_read(0, 0x21);

    // BB25 = real_dc_est_LSB8bit
    // BB21[1:0] = real_dc_est_MSB2bit
    bb_register_write(0, 0x25, (u8)(real_dc_est & 0x0ffUL));
    reg_val = (reg_val & 0xfc) | ((real_dc_est & 0x300UL) >> 8);

    // BB30 = imag_dc_est_LSB8bit
    // BB21[3:2] = imag_dc_est_MSB2bit
    bb_register_write(0, 0x30, (u8)(imag_dc_est & 0x0ffUL));
    reg_val = (reg_val & 0xf3) | ((imag_dc_est & 0x300UL) >> 6);

    bb_register_write(0, 0x21, (u8)(reg_val & 0x0ffUL));

    RFC_DBG(RFC_DBG_INFO, "**************** txlo_cal(): end ****************\n");

    return 0;
}

int txlo_manual(int idc, int qdc)
{
    u8 reg_val = 0;

    RFC_DBG(RFC_DBG_INFO, "**************** %s: start ****************\n", __FUNCTION__);
    RFC_DBG(RFC_DBG_INFO, "I_dc=%d, Q_dc=%d\n", idc, qdc);

    /* BB25 = I_dc  LSB 8bit
    * BB21[1:0] = I_dc MSB 2bit
    * write (I_dc to {BB21[1:0] , BB25) total 10 bit
    * BB30 = Q_dc LSB 8bit
    * BB21[3:2] =Q_dc MSB 2bit
    * write (Q_dc to {BB21[3:2] , BB30) total 10 bit
    * for example:
    * txlo_manual  20  -15
    * bb25=8'h14   BB21[1:0]=0
    * bb30=8'hF1   BB21[3:2]=3'h3
    */

    // BB25 = real_dc_est_LSB8bit
    // BB21[1:0] = real_dc_est_MSB2bit
    bb_register_write(0, 0x25, (u8)(idc & 0x0ffUL));
    reg_val = (reg_val & 0xfc) | ((idc & 0x300UL) >> 8);

    // BB30 = imag_dc_est_LSB8bit
    // BB21[3:2] = imag_dc_est_MSB2bit
    bb_register_write(0, 0x30, (u8)(qdc & 0x0ffUL));
    reg_val = (reg_val & 0xf3) | ((qdc & 0x300UL) >> 6);

    bb_register_write(0, 0x21, (u8)(reg_val & 0x0ffUL));

    RFC_DBG(RFC_DBG_INFO, "**************** %s: end ****************\n", __FUNCTION__);

    return 0;
}

void modtest_iq_balancer(mismatch_info *param, int is_tx)
{
    double alpha, beta;
    double a11, a12, a21, a22, n, m, t;
    double phi;
    complex gain;
    int a21_10bit, a22_10bit;

    //Input Parameter Structure Construction
    gain = param->gain;
    phi = param->phi;

    //Induced parameters
    if (is_tx)
    {
        /* try the inv sign @ 2013/01/01 */
        //alpha = -sin(phi)*gain.real/gain.imag;
        alpha = sin(phi)*gain.real/gain.imag;
        beta = gain.real*cos(phi)/(gain.imag);
    }
    else    // is_rx
    {
        alpha = tan(phi);
        beta = gain.real/(gain.imag*cos(phi));
    }
#if 0
    RFC_DBG(RFC_DBG_INFO, "%s(): is_tx=%d, (alpha,beta)=", __FUNCTION__, is_tx);
    RFC_DBG_DOUBLE(RFC_DBG_INFO, alpha); RFC_DBG(RFC_DBG_INFO, ",");
    RFC_DBG_DOUBLE(RFC_DBG_INFO, beta); RFC_DBG(RFC_DBG_INFO, "\n");
#endif
    a11 = 1; a12 = 0;
    a21 = alpha; a22 = beta;

    t = max_double(fabs(a11), fabs(a12));
    n = 0;
    while (1)
    {
        if (t <= 1)
            break;
        else
        {
            n = n + 1; 
            t = t/2;
            if (n > 3)
            {
                RFC_DBG(RFC_DBG_INFO, "Too large a11 or a12 detected in modtest_iq_balancer. (is_tx=%d)\n", is_tx); 
                break;
            }
        }
    }
    a11 = pow(2,-n)*a11; a12 = pow(2,-n)*a12; //Normalized coefficients

    t = max_double(fabs(a21), fabs(a22));
    m = 0;
    while (1)
    {
        if (t <= 1)
            break;
        else
        {
            m = m + 1; 
            t = t/2;
            if (m > 3)
            {
                RFC_DBG(RFC_DBG_INFO, "ERROR: Too large a22 detected in modtest_tx_iq_balancer_new.\n"); 
                break;
            }
        }
    }

    a21 = pow(2,-m)*a21; a22 = pow(2,-m)*a22; //Normalized coefficients

    param->n = (char) n; 
    param->m = (char) m;
    param->a11 = 0xff & flt2hex(a11, RFC_REG_BIT); 
    param->a12 = 0xff & flt2hex(a12, RFC_REG_BIT);


    if (bb_register_read(0, 0x0) >= 0x80)    // OWL chip
    {
        a21_10bit = flt2hex(a21,RFC_REG_BIT_LONG);  //RFC_REG_BIT_LONG=10
        a22_10bit = flt2hex(a22,RFC_REG_BIT_LONG);
        //a21_10bit may be negative number
        param->a21 = 0xff &  (a21_10bit >> 2);   //take a21_10bit MSB 8 bit to param->a21 
        param->a22 = 0xff &  (a22_10bit >> 2);   //take a22_10bit MSB 8 bit to param->a22 
        param->a21_ext = 0x3 & a21_10bit;
        param->a22_ext = 0x3 & a22_10bit;
    }
    else    // Cheetah chip
    {
        param->a21 = 0xff & flt2hex(a21,RFC_REG_BIT); 
        param->a22 = 0xff & flt2hex(a22,RFC_REG_BIT);
    }
#if 0
    RFC_DBG(RFC_DBG_INFO, "%s(): (a11,a12)=", __FUNCTION__);
    RFC_DBG_DOUBLE(RFC_DBG_INFO, a11); RFC_DBG(RFC_DBG_INFO, ",");
    RFC_DBG_DOUBLE(RFC_DBG_INFO, a12); RFC_DBG(RFC_DBG_INFO, "\n");
    RFC_DBG(RFC_DBG_INFO, "%s(): parm a11=%d, a12=%d, a21=%d, a22=%d, a21_ext=%d, "
            "a22_ext=%d, n=%d, m=%d\n", __FUNCTION__, param->a11, param->a12, param->a21, 
            param->a22, param->a21_ext, param->a22_ext, param->n, param->m);
#endif
}

/* phase_step/gain_step should be large enough to get correctly process */
int tx_fine_tune(mismatch_info *tx_result, double phase_ref, int loop_max, int bw, int freq,
                 unsigned short tg, unsigned short txvga, double phase_step, double gain_step, int debug_en, int cent_freq)
{
    int n_count=0, i=0;
    int dir=0;
    unsigned short rxvga;
    unsigned short tx_nm;
    complex read_2f0;
    double phase_diff;
    double phase_out;
    double fine_2f0_mag;
    double new_2f0_mag;
    double a21=0, a22=0;
    mismatch_info  tx_result_copy;
    int bb_scale_fine = 0;
#if 0
    unsigned char buf[4];
#endif

    memcpy(&tx_result_copy, tx_result, (sizeof(mismatch_info)));

#if 0
    RFC_DBG(RFC_DBG_INFO, "********** in tx_fine_tune() tx_result_copy: **********\n");
    RFC_DBG(RFC_DBG_INFO, "gain = "); RFC_DBG_COMPLEX(RFC_DBG_INFO, tx_result_copy.gain);
    RFC_DBG(RFC_DBG_INFO, "\n");
    RFC_DBG(RFC_DBG_INFO, "phi = "); RFC_DBG_DOUBLE(RFC_DBG_INFO, tx_result_copy.phi);
    RFC_DBG(RFC_DBG_INFO, "\n");
    RFC_DBG(RFC_DBG_INFO, "a11 = "); RFC_DBG_DOUBLE(RFC_DBG_INFO, tx_result_copy.a11);
    RFC_DBG(RFC_DBG_INFO, "\n");
    RFC_DBG(RFC_DBG_INFO, "a12 = "); RFC_DBG_DOUBLE(RFC_DBG_INFO, tx_result_copy.a12);
    RFC_DBG(RFC_DBG_INFO, "\n");
    RFC_DBG(RFC_DBG_INFO, "a21 = "); RFC_DBG_DOUBLE(RFC_DBG_INFO, tx_result_copy.a21);
    RFC_DBG(RFC_DBG_INFO, "\n");
    RFC_DBG(RFC_DBG_INFO, "a22 = "); RFC_DBG_DOUBLE(RFC_DBG_INFO, tx_result_copy.a22);
    RFC_DBG(RFC_DBG_INFO, "\n");
    RFC_DBG(RFC_DBG_INFO, "n = "); RFC_DBG_DOUBLE(RFC_DBG_INFO, tx_result_copy.n);
    RFC_DBG(RFC_DBG_INFO, "\n");
    RFC_DBG(RFC_DBG_INFO, "m = "); RFC_DBG_DOUBLE(RFC_DBG_INFO, tx_result_copy.m);
    RFC_DBG(RFC_DBG_INFO, "\n");
#endif
    while ((n_count++ < loop_max))
    {
        tx_nm = tx_result_copy.n*4 + tx_result_copy.m;

        tx_loopback_report(bw, freq, tg, tg, txvga, 16, bb_scale_fine, 65535,  65535,
                           tx_nm, tx_result_copy.a21, tx_result_copy.a22, 1, NULL, 
                           &read_2f0, 0, 0, 1, 0, 65535, 65535);

        fine_2f0_mag = c_square(&read_2f0);

        phase_out = calc_angle(read_2f0.real, read_2f0.imag);
        phase_diff = angle_diff(phase_out, phase_ref);
        dir = angle_phase(phase_diff);

        i=0;
        do
        {
            a21 = tx_result_copy.a21;
            a22 = tx_result_copy.a22;

            // generate new floating point gain/phase coefficient according to last step
            if (dir == ANGLE_180)
                tx_result_copy.gain.imag = tx_result_copy.gain.imag/gain_step;
            else if (dir == ANGLE_270)
                tx_result_copy.phi = tx_result_copy.phi + phase_step;
            else if (dir == ANGLE_0)
                tx_result_copy.gain.imag = tx_result_copy.gain.imag * gain_step;
            else // (dir == ANGLE_90)
                tx_result_copy.phi = tx_result_copy.phi - phase_step;

            // generate new fixed point reg value accroding to last step.
            modtest_iq_balancer(&tx_result_copy, 1);

            if (++i >= 10)
                break;
        } while ((abs_double(tx_result_copy.a21 - a21) <= 1) && (abs_double(tx_result_copy.a22 - a22) <= 1));
#if 0
        RFC_DBG(RFC_DBG_INFO, "The Tx mismatch : ");
        RFC_DBG_DOUBLE(RFC_DBG_INFO, 20*log10(tx_result_copy.gain.imag)); 
        RFC_DBG(RFC_DBG_INFO, " db, ");
        RFC_DBG_DOUBLE(RFC_DBG_INFO, (tx_result_copy.phi*180)/3.1415926); 
        RFC_DBG(RFC_DBG_INFO, " degree\n");
        RFC_DBG(RFC_DBG_INFO, "a12 = "); RFC_DBG_DOUBLE(RFC_DBG_INFO, tx_result_copy.a12);
        RFC_DBG(RFC_DBG_INFO, "\n");
        RFC_DBG(RFC_DBG_INFO, "a22 = "); RFC_DBG_DOUBLE(RFC_DBG_INFO, tx_result_copy.a22);
        RFC_DBG(RFC_DBG_INFO, "\n");
        RFC_DBG(RFC_DBG_INFO, "n = "); RFC_DBG_DOUBLE(RFC_DBG_INFO, tx_result_copy.n);
        RFC_DBG(RFC_DBG_INFO, "\n");
        RFC_DBG(RFC_DBG_INFO, "m = "); RFC_DBG_DOUBLE(RFC_DBG_INFO, tx_result_copy.m);
        RFC_DBG(RFC_DBG_INFO, "\n");
#endif	
        tx_nm = tx_result_copy.n*4 + tx_result_copy.m;

        rxvga = read_txcal_rx_gain();
        tx_loopback_report(bw, freq, tg, tg, txvga, rxvga, bb_scale_fine, 65535,  65535,
                           tx_nm, tx_result_copy.a21, tx_result_copy.a22, 0, NULL, &read_2f0, 0, 2, 1, 0, 
                           65535, 65535);
        new_2f0_mag = c_square(&read_2f0);

#if 0
        if (debug_en & 0x1)
        {
            RFC_DBG(RFC_DBG_INFO, "%s(), press ENTER to continue.\n", __FUNCTION__);
            WLA_GETS(buf);
            if (strncmp(buf, "c", 1) == 0)
                return 1;
        }
#endif //	RFC_DEBUG
        // if new_2f0_mag < fine_2f0_mag  that means new value is better
#if 0
        RFC_DBG(RFC_DBG_INFO, "new_2f0_mag = "); RFC_DBG_DOUBLE(RFC_DBG_INFO, new_2f0_mag);
        RFC_DBG(RFC_DBG_INFO, ", fine_2f0_mag = "); RFC_DBG_DOUBLE(RFC_DBG_INFO, fine_2f0_mag);
        RFC_DBG(RFC_DBG_INFO, "\n");
#endif
        if (new_2f0_mag < fine_2f0_mag)
        {
            memcpy(tx_result, &tx_result_copy, sizeof(mismatch_info));
            //RFC_DBG(RFC_DBG_INFO, "********** end this round : get better gain/phi **********\n");
        }
        else  //(old value is better, fine tune is over)
            break;
    }

    return 0;
}

int tx_fine_tune_nodir(mismatch_info *tx_result, int loop_max, int bw, int freq,
                       unsigned short tg, unsigned short txvga, double phase_step, double gain_step, 
                       int debug_en, int cent_freq)
{
    mismatch_info tx_result_copy, tx_result_phpos, tx_result_phneg;
    mismatch_info tx_result_gnpos, tx_result_gnneg;
    complex read_2f0;
    int n_count=0;
    int count=0;
    char a21=0, a22=0;
    //char a21_ext, a22_ext;
    unsigned short tx_nm;
    double last_2f0_mag, pos_2f0_mag, neg_2f0_mag;
    int agc_en = 0; // tx time domain spur extect, date: 08/29
    int is_equal = 1;
    int bb_scale_fine = 0;

    memcpy(&tx_result_copy, tx_result, (sizeof(mismatch_info)));

    ////////////////////////// phase
    n_count = 0;
    while (n_count++ < loop_max)
    {
        tx_nm = tx_result_copy.n*4 + tx_result_copy.m;

        tx_loopback_report(bw, freq, tg, tg, txvga, 0, bb_scale_fine, 65535, 65535, tx_nm, 
                           tx_result_copy.a21, tx_result_copy.a22, agc_en, NULL, &read_2f0, 1, 0, 1, 0, 
                           tx_result_copy.a21_ext, tx_result_copy.a22_ext);

        last_2f0_mag = c_square(&read_2f0);
        a21 = tx_result_copy.a21;
        a22 = tx_result_copy.a22;
        //a21_ext = tx_result_copy.a21_ext;
        //a22_ext = tx_result_copy.a22_ext;

        RFC_DBG(RFC_DBG_INFO, "********** phase last last_2f0_mag = ");
        RFC_DBG_DOUBLE(RFC_DBG_INFO, last_2f0_mag); RFC_DBG(RFC_DBG_INFO, "\n");

        count = 0;
        memcpy(&tx_result_phpos, &tx_result_copy, (sizeof(mismatch_info)));
        if (is_equal)
        {
            do
            {
                // generate new floating point gain/phase coefficient according to last step
                tx_result_phpos.phi = tx_result_phpos.phi + phase_step;

                // generate new fixed point reg value accroding to last step
                modtest_iq_balancer(&tx_result_phpos, 1);
            } while ((abs_double(tx_result_phpos.a21 - a21) != 0) &&
                     (++count <= 10));
        }
        else
        {
            do
            {
                // generate new floating point gain/phase coefficient according to last step
                tx_result_phpos.phi = tx_result_phpos.phi + phase_step;

                // generate new fixed point reg value accroding to last step
                modtest_iq_balancer(&tx_result_phpos, 1);
            } while ((abs_double(tx_result_phpos.a21 - a21) < 2) &&
                     (++count <= 10));
        }

        tx_nm = tx_result_phpos.n * 4 + tx_result_phpos.m;

        tx_loopback_report(bw, freq, tg, tg, txvga, 0, bb_scale_fine, 65535, 65535, tx_nm, 
                           tx_result_phpos.a21, tx_result_phpos.a22, 0, NULL, &read_2f0, 1, 0, 1, 0,
                           tx_result_phpos.a21_ext, tx_result_phpos.a22_ext);

        pos_2f0_mag = c_square(&read_2f0);

        RFC_DBG(RFC_DBG_INFO, "********** phase pos pos_2f0_mag = ");
        RFC_DBG_DOUBLE(RFC_DBG_INFO, pos_2f0_mag); RFC_DBG(RFC_DBG_INFO, "\n");

        count = 0;
        memcpy(&tx_result_phneg, &tx_result_copy, (sizeof(mismatch_info)));
        if (is_equal)
        {
            do
            {
                // generate new floating point gain/phase coefficient according to last step
                tx_result_phneg.phi = tx_result_phneg.phi - phase_step;

                // generate new fixed point reg value accroding to last step.
                modtest_iq_balancer(&tx_result_phneg, 1);
            } while ((abs_double(tx_result_phneg.a21 - a21) != 0) &&
                     (++count <= 10));
        }
        else
        {
            do
            {
                // generate new floating point gain/phase coefficient according to last step
                tx_result_phneg.phi = tx_result_phneg.phi - phase_step;

                // generate new fixed point reg value accroding to last step.
                modtest_iq_balancer(&tx_result_phneg, 1);
            } while ((abs_double(tx_result_phneg.a21 - a21) < 2) &&
                     (++count <= 10));
        }

        tx_nm = tx_result_phneg.n*4 + tx_result_phneg.m;

        tx_loopback_report(bw, freq, tg, tg, txvga, 0, bb_scale_fine, 65535, 65535, tx_nm, 
                           tx_result_phneg.a21, tx_result_phneg.a22, 0, NULL, &read_2f0, 1, 0, 1, 0,
                           tx_result_phneg.a21_ext, tx_result_phneg.a22_ext);

        neg_2f0_mag = c_square(&read_2f0);

        RFC_DBG(RFC_DBG_INFO, "********** phase neg neg_2f0_mag = ");
        RFC_DBG_DOUBLE(RFC_DBG_INFO, neg_2f0_mag); RFC_DBG(RFC_DBG_INFO, "\n");

        if ((neg_2f0_mag > last_2f0_mag) && (last_2f0_mag > pos_2f0_mag))  // pos is better
        {
            memcpy(&tx_result_copy, &tx_result_phpos, (sizeof(mismatch_info)));
            RFC_DBG(RFC_DBG_INFO, "**********TX Fine tune:  phase ++, (a21,a22,a21_ext,a22_ext)=(%d,%d,%d,%d)\n\n"
                    , tx_result_copy.a21, tx_result_copy.a22
                    , tx_result_copy.a21_ext, tx_result_copy.a22_ext);
        }
        else if ((pos_2f0_mag> last_2f0_mag) && (last_2f0_mag > neg_2f0_mag))  // neg is better
        {
            memcpy(&tx_result_copy, &tx_result_phneg, (sizeof(mismatch_info)));
            RFC_DBG(RFC_DBG_INFO, "**********TX Fine tune:  phase --, (a21,a22,a21_ext,a22_ext)=(%d,%d,%d,%d)\n\n"
                    ,tx_result_copy.a21, tx_result_copy.a22
                    ,tx_result_copy.a21_ext, tx_result_copy.a22_ext);
        }
        else if ((pos_2f0_mag > last_2f0_mag) && (neg_2f0_mag > last_2f0_mag))
        {
            //last is better and leave
            RFC_DBG(RFC_DBG_INFO, "**********TX Fine tune:  phase done\n\n");
            break;
        }
    }   // end phase

    //////////////////////////////////////////////////////////////////////////////
    // gain
#if 1
    n_count = 0;
    while (n_count++ < loop_max)
    {
        tx_nm = tx_result_copy.n * 4 + tx_result_copy.m;

        tx_loopback_report(bw, freq, tg, tg, txvga, 0, bb_scale_fine, 65535, 65535, tx_nm, 
                           tx_result_copy.a21, tx_result_copy.a22, agc_en, NULL, &read_2f0, 1, 0, 1, 0,
                           tx_result_copy.a21_ext, tx_result_copy.a22_ext);

        last_2f0_mag = c_square(&read_2f0);

        RFC_DBG(RFC_DBG_INFO, "********** gain last last_2f0_mag = ");
        RFC_DBG_DOUBLE(RFC_DBG_INFO, last_2f0_mag); RFC_DBG(RFC_DBG_INFO, "\n");

        a21 = tx_result_copy.a21;
        a22 = tx_result_copy.a22;
        //a21_ext = tx_result_copy.a21_ext;
        //a22_ext = tx_result_copy.a22_ext;

        // Hi Boforn plz help me to check  It's pos region, all declaration paramter have positive indicator
        count = 0;
        memcpy(&tx_result_gnpos, &tx_result_copy, (sizeof(mismatch_info)));
        if (is_equal)
        {
            do
            {
                // generate new floating point gain/phase coefficient according to last step
                tx_result_gnpos.gain.imag = tx_result_gnpos.gain.imag * gain_step;

                // generate new fixed point reg value accroding to last step.
                modtest_iq_balancer(&tx_result_gnpos, 1);
            } while ((abs_double(tx_result_gnpos.a22 - a22) != 0) &&
                     (++count <= 10));
        }
        else
        {
            do
            {
                // generate new floating point gain/phase coefficient according to last step
                tx_result_gnpos.gain.imag = tx_result_gnpos.gain.imag * gain_step;

                // generate new fixed point reg value accroding to last step.
                modtest_iq_balancer(&tx_result_gnpos, 1);
            } while ((abs_double(tx_result_gnpos.a22 - a22) < 2) &&
                     (++count <= 10));
        }

        tx_nm = tx_result_gnpos.n * 4 + tx_result_gnpos.m;

        tx_loopback_report(bw, freq, tg, tg, txvga, 0, bb_scale_fine, 65535, 65535, tx_nm, 
                           tx_result_gnpos.a21, tx_result_gnpos.a22, 0, NULL, &read_2f0, 1, 0, 1, 0,
                           tx_result_gnpos.a21_ext, tx_result_gnpos.a22_ext);

        pos_2f0_mag = c_square(&read_2f0);
        RFC_DBG(RFC_DBG_INFO, "********** gain pos pos_2f0_mag = ");
        RFC_DBG_DOUBLE(RFC_DBG_INFO, pos_2f0_mag); RFC_DBG(RFC_DBG_INFO, "\n");
        RFC_DBG(RFC_DBG_INFO, "(a21,a22,a21_ext,a22_ext)=(%d,%d,%d,%d)\n\n",
                tx_result_gnpos.a21, tx_result_gnpos.a22, 
                tx_result_gnpos.a21_ext, tx_result_gnpos.a22_ext);

        // FIXME: Hi Boforn plz help me to check  It's neg region, all declaration paramter have negative indicator
        count = 0;
        memcpy(&tx_result_gnneg, &tx_result_copy, (sizeof(mismatch_info)));
        if (is_equal)
        {
            do
            {
                // generate new floating point gain/phase coefficient according to last step
                tx_result_gnneg.gain.imag = tx_result_gnneg.gain.imag/gain_step;

                // generate new fixed point reg value accroding to last step.
                modtest_iq_balancer(&tx_result_gnneg, 1);
            } while ((abs_double(tx_result_gnneg.a22 - a22) != 0) &&
                     (++count <= 10));
        }
        else
        {
            do
            {
                // generate new floating point gain/phase coefficient according to last step
                tx_result_gnneg.gain.imag = tx_result_gnneg.gain.imag/gain_step;

                // generate new fixed point reg value accroding to last step.
                modtest_iq_balancer(&tx_result_gnneg, 1);
            } while ((abs_double(tx_result_gnneg.a22 - a22) < 2) &&
                     (++count <= 10));
        }

        tx_nm = tx_result_gnneg.n * 4 + tx_result_gnneg.m;

        tx_loopback_report(bw, freq, tg, tg, txvga, 0, bb_scale_fine, 65535, 65535, tx_nm, 
                           tx_result_gnneg.a21, tx_result_gnneg.a22, 0, NULL, &read_2f0, 1, 0, 1, 0,
                           tx_result_gnneg.a21_ext, tx_result_gnneg.a22_ext);

        neg_2f0_mag = c_square(&read_2f0);

        RFC_DBG(RFC_DBG_INFO, "********** gain neg neg_2f0_mag = ");
        RFC_DBG_DOUBLE(RFC_DBG_INFO, neg_2f0_mag); RFC_DBG(RFC_DBG_INFO, "\n");
        RFC_DBG(RFC_DBG_INFO, "(a21,a22,a21_ext,a22_ext)=(%d,%d,%d,%d)\n\n",
                tx_result_gnneg.a21, tx_result_gnneg.a22, 
                tx_result_gnneg.a21_ext, tx_result_gnneg.a22_ext);

        if ((neg_2f0_mag > last_2f0_mag) && (last_2f0_mag > pos_2f0_mag)) // pos is better
        {
            memcpy(&tx_result_copy, &tx_result_gnpos, (sizeof(mismatch_info)));
            RFC_DBG(RFC_DBG_INFO, "**********TX Fine tune:  gain ++, (a21,a22,a21_ext,a22_ext)=(%d,%d,%d,%d)\n\n",
                    tx_result_copy.a21, tx_result_copy.a22, 
                    tx_result_copy.a21_ext, tx_result_copy.a22_ext);
        }
        else if ((pos_2f0_mag > last_2f0_mag) && (last_2f0_mag > neg_2f0_mag))  // neg is better
        {
            memcpy(&tx_result_copy, &tx_result_gnneg, (sizeof(mismatch_info)));
            RFC_DBG(RFC_DBG_INFO, "**********TX Fine tune:  gain --, (a21,a22,a21_ext,a22_ext)=(%d,%d,%d,%d)\n\n",
                    tx_result_copy.a21, tx_result_copy.a22, 
                    tx_result_copy.a21_ext, tx_result_copy.a22_ext);
        }
        else if ((pos_2f0_mag > last_2f0_mag) && (neg_2f0_mag > last_2f0_mag))
        {
            // last is better and leave
            RFC_DBG(RFC_DBG_INFO, "**********TX Fine tune:  gain done\n\n");
            break;
        }
    }   // end gain
#endif

    memcpy(tx_result, &tx_result_copy, sizeof(mismatch_info));

    return 0;
}

static double tx_cal_lpfout(complex y_lpf, mismatch_info *x_tx_iq_balancer, unsigned char tx_cal_state)
{
    static double phi_est=0, gi=0, gq=0; //tx
    static complex gain_est = { 0, 0,}; //tx
#if 1
    static double mag_2f0_pos=0, mag_2f0_neg=0;
#endif
    /* light version add parm */
    static double phase_ref=0;
    //double phase_twoside;
    //double phase_diff;
    //int dirinfo;

    switch (tx_cal_state)
    {   //Observation and estimation
        case GAIN_I_CAL: //'gain_i_cal'
            gi = gain_estimator(y_lpf, 0, 0);
            break;

        case GAIN_Q_CAL: //'gain_q_cal'
            gq = gain_estimator(y_lpf, 0, 0);
            gain_est.real = gi/gi;
            gain_est.imag = gq/gi;
            x_tx_iq_balancer->gain = gain_est; x_tx_iq_balancer->phi = phi_est;
            modtest_iq_balancer(x_tx_iq_balancer, 1);
            //phase_ref = calc_angle(y_lpf.real, y_lpf.imag);
            break;
#if 0					 
        case PHI_CAL_1: //'phi_cal_1'
            phi_est = gain_estimator(y_lpf, gi, 1);
            phase_twoside = calc_angle(y_lpf.real, y_lpf.imag);
            phase_diff = angle_diff(phase_twoside, phase_ref); //calc phase diff
            dirinfo = angle_phase(phase_diff);                 //which dir is phase_diff close to?
            if (dirinfo == ANGLE_90)
                phi_est = -phi_est;
            else if (dirinfo == ANGLE_270)
                phi_est = phi_est;
            else
            {
                RFC_DBG(RFC_DBG_INFO, "tx_cal_lpf_out : wrong angle\n");
                phase_ref = 370;
            }
            x_tx_iq_balancer->gain = gain_est; x_tx_iq_balancer->phi = phi_est;
            modtest_iq_balancer(x_tx_iq_balancer, 1);
            break;
#else
        case PHI_CAL_1: //'phi_cal_1'
            phi_est = gain_estimator(y_lpf, gi, 1);
            x_tx_iq_balancer->gain = gain_est; x_tx_iq_balancer->phi = phi_est;
            modtest_iq_balancer(x_tx_iq_balancer, 1);
            break;
        case PHI_CAL_2: //'phi_cal_2'                                         
            mag_2f0_pos = abs_complex(&y_lpf);
            RFC_DBG(RFC_DBG_INFO, "************** mag_2f0_pos = ");
            RFC_DBG_DOUBLE(RFC_DBG_INFO, mag_2f0_pos); RFC_DBG(RFC_DBG_INFO, "\n");
            x_tx_iq_balancer->gain = gain_est; x_tx_iq_balancer->phi = -phi_est;
            modtest_iq_balancer(x_tx_iq_balancer, 1);
            break;
        case PHI_CAL_3: //'phi_cal_3'           
            mag_2f0_neg = abs_complex(&y_lpf);
            RFC_DBG(RFC_DBG_INFO, "************** mag_2f0_neg = ");
            RFC_DBG_DOUBLE(RFC_DBG_INFO, mag_2f0_neg); RFC_DBG(RFC_DBG_INFO, "\n");
            if (mag_2f0_neg < mag_2f0_pos)
                phi_est = -phi_est;
            //RFC_DBG(RFC_DBG_INFO, "************** phi_est = %f\n", phi_est);
            RFC_DBG(RFC_DBG_INFO, "************** phi_est = ");
            RFC_DBG_DOUBLE(RFC_DBG_INFO, (phi_est*180)/3.1415926); RFC_DBG(RFC_DBG_INFO, "\n");
            x_tx_iq_balancer->gain = gain_est; x_tx_iq_balancer->phi = phi_est;
            modtest_iq_balancer(x_tx_iq_balancer, 1);
            break;
#endif
        default:
            //Init global variables
            gain_est.real = 1; gain_est.imag = 1; 
            phi_est = 0;
            gi = gain_est.real; gq = gain_est.imag;
            phase_ref = 0;
            break;
    }

    return phase_ref;
}

void bal_regs(mismatch_info param, unsigned char mode) //TX_BAL:0 RX_BAL:1
{
#define RFC_TX_N 0x03 //<<6
#define RFC_TX_M 0x03 //<<4
#define RFC_RX_N 0x03 //<<2
#define RFC_RX_M 0x03 //<<0

    //char a11, a12;
    char a21, a22;
    char a21_ext=0, a22_ext=0;
    char n, m;

    char reg_val;
    unsigned char value20;
    unsigned char value33;

    n = param.n; m = param.m;
    //a11 = param.a11; a12 = param.a12;
    a21 = param.a21; a22 = param.a22;

    if (bb_register_read(0, 0x0) >= 0x80)    // OWL chip
    {
        a21_ext = param.a21_ext;
        a22_ext = param.a22_ext;
    }

    switch (mode)
    {
        case TX_BAL: //tx balancer
            //Write N, M
            value20 = bb_register_read(0, 0x20); //get RX N, RX M
            reg_val = (n & RFC_TX_N)<<6 | (m & RFC_TX_M)<<4 | (value20 & 0x0F);
            bb_register_write(0, 0x20, reg_val);

//          bb_register_write(0, 0x21, a11);
//          bb_register_write(0, 0x22, a12);
            bb_register_write(0, 0x23, a21);
            bb_register_write(0, 0x24, a22);

            if (bb_register_read(0, 0x0) >= 0x80)    // OWL chip
            {
                value33 = bb_register_read(0, 0x33);
                value33 = (value33 & 0x0f) | ((a22_ext & 0x3) << 4) | ((a21_ext & 0x3) << 6);
                bb_register_write(0, 0x33, value33);
            }

            break;
        case RX_BAL: //rx balancer
            //Write N, M
            value20 = bb_register_read(0, 0x20); //get RX N, RX M
            reg_val = (value20 & 0xF0) | (n & RFC_RX_N)<<2 | (m & RFC_RX_M) ;
            bb_register_write(0, 0x20, reg_val);

//          bb_register_write(0, 0x26, a11);
//          bb_register_write(0, 0x27, a12);
            bb_register_write(0, 0x28, a21);
            bb_register_write(0, 0x29, a22);

            if (bb_register_read(0, 0x0) >= 0x80)    // OWL chip
            {
                value33 = bb_register_read(0, 0x33);
                value33 = (value33 & 0xf0) | ((a21_ext & 0x3) << 2) | (a22_ext & 0x3);
                bb_register_write(0, 0x33, value33);
            }

            break;
    }
}

void compute_balancer(mismatch_info *tx_bal, mismatch_info *rx_bal, struct rfc_cal_parm *result, int freq_quantity, double *tone_list, int bw)
{
    int alpha;
    double tx_phi[4], rx_phi[4];
    double phi_diff=0;
    int end_tone;

    if (bw)
        end_tone = FREQ_QUANTITY_40MHZ - 1;
    else
        end_tone = FREQ_QUANTITY_20MHZ - 1;

    tx_phi[0] = tx_bal[0].phi;
    rx_phi[0] = rx_bal[0].phi;
    tx_phi[1] = tx_bal[1].phi;
    rx_phi[1] = rx_bal[1].phi;
    tx_phi[2] = tx_bal[end_tone].phi;
    rx_phi[2] = rx_bal[end_tone].phi;
    tx_phi[3] = tx_bal[end_tone].phi;
    rx_phi[3] = rx_bal[end_tone].phi;

    /* calc the tx/rx gains' & phis' mean */
    bal_parm_mean(tx_bal, rx_bal, result, freq_quantity);

    /* setup tx cal data*/
    phi_diff = diff_phy(tx_phi, 4);

    // curve data
    if (phi_diff >= PHASE_SHIFTER_TURNON_THRESHOULD)
    {
        alpha = cmp_phase_shifter_coe_coarse(tx_bal, bw, freq_quantity, tone_list);
        RFC_DBG(RFC_DBG_INFO, "coarse tx alpha = "); RFC_DBG_DOUBLE(RFC_DBG_INFO, alpha);
        RFC_DBG(RFC_DBG_INFO, "\n");
        alpha = cmp_phase_shifter_coe_fine(alpha, tx_bal, bw, freq_quantity, tone_list);
        RFC_DBG(RFC_DBG_INFO, "fine tx alpha = "); RFC_DBG_DOUBLE(RFC_DBG_INFO, alpha);
        RFC_DBG(RFC_DBG_INFO, "\n");
    }
    else
        alpha = 0;
    result->tx_curve.phase_shifter_coe = fabs(alpha);
    if ((alpha == 256) || (alpha == 0))
        result->tx_curve.phase_shifter_ctl = I_Q_NONE;
    else if (alpha>0)
        result->tx_curve.phase_shifter_ctl = I_PATH;
    else
        result->tx_curve.phase_shifter_ctl = Q_PATH;
    result->tx_curve.rolloff_filter_ctl = 0;
    result->tx_curve.rolloff_filter_coe = 0;
    // mismatch_info
    modtest_iq_balancer(&result->tx_avg, 1);

    /* setup rx cal data */
    // curve data
    phi_diff = diff_phy(rx_phi, 4);
    if (phi_diff >= PHASE_SHIFTER_TURNON_THRESHOULD)
    {
        alpha = cmp_phase_shifter_coe_coarse(rx_bal, bw, freq_quantity, tone_list);
        RFC_DBG(RFC_DBG_INFO, "coarse rx alpha = "); RFC_DBG_DOUBLE(RFC_DBG_INFO, alpha);
        RFC_DBG(RFC_DBG_INFO, "\n");
        alpha = cmp_phase_shifter_coe_fine(alpha, rx_bal, bw, freq_quantity, tone_list);
        RFC_DBG(RFC_DBG_INFO, "fine rx alpha = "); RFC_DBG_DOUBLE(RFC_DBG_INFO, alpha);
        RFC_DBG(RFC_DBG_INFO, "\n");
    }
    else
        alpha = 0;
    result->rx_curve.phase_shifter_coe = fabs(alpha);
    if ((alpha == 256) || (alpha == 0))
        result->rx_curve.phase_shifter_ctl = I_Q_NONE;
    else if (alpha>0)
        result->rx_curve.phase_shifter_ctl = I_PATH;
    else
        result->rx_curve.phase_shifter_ctl = Q_PATH;
    result->rx_curve.rolloff_filter_ctl = 0;
    result->rx_curve.rolloff_filter_coe = 0;
    // mismatch_info
    modtest_iq_balancer(&result->rx_avg, 0);
}

void transfer_twofilter_regs(struct rfc_cal_parm *cal_parm, struct rfc_cal_reg *result)
{
    int phase_shifter_coe_wlen=8;

#if 0
    // temporary test, date: 08/14
    result->balancer_nm = (cal_parm->tx_avg.n << 6) | (cal_parm->tx_avg.m << 4);// | (cal_parm->rx_avg.n << 2) | (cal_parm->rx_avg.m);
//  result->tx_a11 = bb_register_read(0, 0x21);
    result->tx_a12 = cal_parm->tx_avg.a12;
    result->tx_a21 = cal_parm->tx_avg.a21;
    result->tx_a22 = cal_parm->tx_avg.a22;
    result->tx_dc_i = bb_register_read(0, 0x25);
    result->rx_a21 = 0x0; //cal_parm->rx_avg.a21;
    result->rx_a22 = 0x7f; //cal_parm->rx_avg.a22;
    result->rx_dc_i = 0;
    result->tx_dc_q = bb_register_read(0, 0x30);
    result->rx_dc_q = 0;
    result->filter_switch = (cal_parm->tx_curve.rolloff_filter_ctl << 6) | (cal_parm->tx_curve.phase_shifter_ctl << 4) | (cal_parm->rx_curve.rolloff_filter_ctl << 2) | (cal_parm->rx_curve.phase_shifter_ctl);
    result->rolloff_rx_coe = (cal_parm->rx_curve.rolloff_filter_coe * pow(2, (phase_shifter_coe_wlen-1)));
    result->phaseshifter_rx_alfa = cal_parm->rx_curve.phase_shifter_coe;
    result->rolloff_tx_coe = (cal_parm->tx_curve.rolloff_filter_coe * pow(2, (phase_shifter_coe_wlen-1)));
    result->phaseshifter_tx_alfa = cal_parm->tx_curve.phase_shifter_coe;
#else
    result->balancer_nm = (cal_parm->tx_avg.n << 6) | (cal_parm->tx_avg.m << 4) | (cal_parm->rx_avg.n << 2) | (cal_parm->rx_avg.m);
//  result->tx_a11 = bb_register_read(0, 0x21);
    result->tx_a12 = cal_parm->tx_avg.a12;
    result->tx_a21 = cal_parm->tx_avg.a21;
    result->tx_a22 = cal_parm->tx_avg.a22;
    result->tx_dc_i = bb_register_read(0, 0x25);
    result->rx_a21 = cal_parm->rx_avg.a21;
    result->rx_a22 = cal_parm->rx_avg.a22;
    result->rx_dc_i = 0;
    result->tx_dc_q = bb_register_read(0, 0x30);
    result->rx_dc_q = 0;
    result->filter_switch = (cal_parm->tx_curve.rolloff_filter_ctl << 6) | (cal_parm->tx_curve.phase_shifter_ctl << 4) | (cal_parm->rx_curve.rolloff_filter_ctl << 2) | (cal_parm->rx_curve.phase_shifter_ctl);
    result->rolloff_rx_coe = (cal_parm->rx_curve.rolloff_filter_coe * pow(2, (phase_shifter_coe_wlen-1)));
    result->phaseshifter_rx_alfa = cal_parm->rx_curve.phase_shifter_coe;
    result->rolloff_tx_coe = (cal_parm->tx_curve.rolloff_filter_coe * pow(2, (phase_shifter_coe_wlen-1)));
    result->phaseshifter_tx_alfa = cal_parm->tx_curve.phase_shifter_coe;
#endif

    if (bb_register_read(0, 0x0) >= 0x80)    // OWL chip
    {
        result->a21a22_ext = (cal_parm->tx_avg.a21_ext<<6) | (cal_parm->tx_avg.a22_ext<<4)  | (cal_parm->rx_avg.a21_ext<<2) | (cal_parm->rx_avg.a22_ext);
    }

#if 0
    /* patch the issue : 5e = 0 & 5a = 1 => rfc will get wrong result */
    trans_2filter_regs_patch(result);
#endif
}

int rfc_tx_cal(int freq_quantity, int tone_mask, int freq, int bw, struct vga_entry *tx_table, 
               int debug_en, mismatch_info *tx_bal)
{
    mismatch_info *param;
    int agc_en=0, lpf_reset=0, lpf_sel=0;
    int tone_idx, j, k;
    int iqswap = ori_env.iqswap;
    int bb_scale=0, rxvga=0, txvga=0, bb_scale_fine=0;
    double phase_ref=0;
    unsigned char tg_a_i=0, tg_a_q=0;
    complex I_signal, Q_signal;
    //complex I_signal_f0, Q_signal_f0;
    complex *tx_signal;
    //complex *tx_signal_f0;
    //int exe_init_txloop_setting = 0;
#if 0
    unsigned char buf[4];
    (void) buf;
#endif

    if (iqswap)
    {
        tx_signal = &I_signal;
        //tx_signal_f0 = &I_signal_f0;
    }
    else
    {
        tx_signal = &Q_signal;
        //tx_signal_f0 = &Q_signal_f0;
    }

    txvga = tx_table->txvga;
    bb_scale = tx_table->bb_scale;
    bb_scale_fine = tx_table->bb_scale_fine;

    for (tone_idx=0; tone_idx<freq_quantity; tone_idx++)
    {
        if (!(tone_mask & (1 << tone_idx)))
            continue;

        bb_rfc_reset();

        RFC_DBG(RFC_DBG_INFO, "======= RFC test : bw = %d, tone_idx = %d\n", bw, tone_idx);
        /* Should confirm the iq swap setting to use I signal or Q signal in tx calibration */

        //----------TX Calibration----------

        //Init TX Calibration variables
        param = &tx_bal[tone_idx];
        param->gain.real = 1; 
        param->gain.imag = 1; 
        param->phi = 0;
        modtest_iq_balancer(param, 1);

        //reset tx variables. I_signal, param, -1 are useless
        tx_cal_lpfout(*tx_signal, param, -1);   // I_signal is meaningless

        rxvga = tx_table->rxvga;

        k = 0;  /* If "wrong angle" happen, re-calibrate one time.*/
        do
        {
            for (j=0; j<5; j++)
            {
                //Generate Tone
                switch (j)
                {
                    case 0:
                        tg_a_i = bb_scale;
                        tg_a_q = 15;
                        agc_en = 0; // tx time domain spur extect, date: 08/29
                        lpf_reset = 1;
                        lpf_sel = 0;
                        break;
                    case 1:
                        tg_a_i = 15;
                        tg_a_q = bb_scale;
                        agc_en = 0;
                        lpf_reset = 1;
                        lpf_sel = 0;
                        break;
                    case 2:
                        tg_a_i = bb_scale;
                        tg_a_q = bb_scale;
                        agc_en = 0;
                        lpf_reset = 0;
                        lpf_sel = 0;
                        break;
#if 1
                    case 3:
                    case 4:
                        tg_a_i = bb_scale;
                        tg_a_q = bb_scale;
                        agc_en = 0;
                        lpf_reset = 1;
                        lpf_sel = 0;
                        break;
#endif
                }

                //Program Balancer
                bal_regs(*param, TX_BAL);

                tx_loopback_report(bw, tone_idx, tg_a_i, tg_a_q, txvga, rxvga, bb_scale_fine, 65535, 65535, 65535, 65535, 65535, agc_en, NULL, tx_signal, lpf_reset, lpf_sel, 1, 0, 65535, 65535);
                RFC_DBG(RFC_DBG_INFO, "======================================\n");
                RFC_DBG(RFC_DBG_INFO, "rfc_tx_cal, Loop: %d, Case: %d\n", k, j);
                if (j == 3 || j == 4)
                {
                    RFC_DBG(RFC_DBG_INFO, "bb reg20:0x%x, reg23:0x%x, reg24:0x%x\n",
                            bb_register_read(0, 0x20), bb_register_read(0, 0x23), bb_register_read(0, 0x24));
                }
                RFC_DBG(RFC_DBG_INFO, "======================================\n");
#ifdef TX_PAUSE_CMD
                if (panther_rfc_pause())
                    return 0;
#endif

                if (agc_en)
                    rxvga = read_txcal_rx_gain();

#if 0
                if (debug_en & 0x1)
                {
                    RFC_DBG(RFC_DBG_INFO, "TX CAL. tone_idx=%d, j=%d, press ENTER to continue.\n", tone_idx, j);
                    WLA_GETS(buf);
                    if (strncmp(buf, "c", 1) == 0)
                        return 1;
                }
#endif
                // Calculation gain/phase mismatches
                // j values: GAIN_I_CAL 0 /GAIN_Q_CAL 1 /PHI_CAL_1 2 /PHI_CAL_2 3 /PHI_CAL_3 4
                phase_ref = tx_cal_lpfout(*tx_signal, param, j); 
            }
        } while ((k++ < 1) && (phase_ref > 360));

#if 0
        if (!(debug_en & 0x2))
        {
            tg_a_q = 1;
            RFC_DBG(RFC_DBG_INFO, "=======================================================\n");
            RFC_DBG(RFC_DBG_INFO, "Enter tx_fine_tune_nodir() function !\n");
            RFC_DBG(RFC_DBG_INFO, "=======================================================\n");
            tx_fine_tune_nodir(param, 4, bw, tone_idx, tg_a_q, txvga, 0.006, 1.004, debug_en, freq);
        }
#endif		
    }

    return 0;
}

int is_rxloop_panther = 0;  // LYNX/PANTHER RX_LOOPBACK
int is_enable_rf12_rw = 0;  // Is rw rf12
int rfc_rx_cal(int freq_quantity, int tone_mask, int bw, struct vga_entry *rx_table, 
               int debug_en, mismatch_info *tx_bal, mismatch_info *rx_bal)
{
    int tone_idx;
    int bb_scale=0, rxvga=0, txvga=0, bb_scale_fine=0;
    double rx_caldata[2];
    complex neg_f0, pos_f0;
    int agc_en = 0;

    bb_scale = rx_table->bb_scale;
    rxvga = rx_table->rxvga;
    txvga = rx_table->txvga;
    bb_scale_fine = rx_table->bb_scale_fine;

    for (tone_idx=0; tone_idx<freq_quantity; tone_idx++)
    {
        if (!(tone_mask & (1 << tone_idx)))
            continue;

        //Program TX Balancer
        bal_regs(tx_bal[tone_idx], TX_BAL);

        rx_loopback_report(bw, tone_idx, bb_scale, txvga, rxvga, bb_scale_fine, 65535, 65535, 65535, 65535, 65535, 
                           agc_en, &neg_f0, &pos_f0, 1, 0, 0);
#ifdef RX_PAUSE_CMD
        if (panther_rfc_pause())
            return 0;
#endif

        //Calculation gain/phase mismatches
        cmp_iq_mismatch(pos_f0, neg_f0, rx_caldata);

        rx_bal[tone_idx].gain.real = 1;
#if 1
        rx_bal[tone_idx].gain.imag = rx_caldata[0];
        rx_bal[tone_idx].phi = rx_caldata[1];
#else
        if (is_rxloop_panther)   // get gain with panther setting, get phase with lynx setting
        {
            //printf("panther gain: %f dB, phase: %f degree\n", 20*log10(rx_caldata[0]), ((rx_caldata[1]*180)/3.1415926));
            rx_bal[tone_idx].gain.imag = rx_caldata[0];
        }
        else
        {
            //printf("Lynx gain: %f dB, phase: %f degree\n", 20*log10(rx_caldata[0]), ((rx_caldata[1]*180)/3.1415926));
            rx_bal[tone_idx].phi = rx_caldata[1];
        }
#endif
    }

    return 0;
}

void shut_up_bb(void)
{
    bb_register_write(0, 0x2b, 0);
    bb_register_write(0, 0x2c, 0xff);
    bb_register_write(0, 0x2f, 7);

    printf("bb shut up finish !\n");
}

#include <mt_driver_Panther.h>
extern MT_DEVICE_SETTINGS_PANTHER mt_handle;
static mismatch_info tx_bal[FREQ_QUANTITY_40MHZ];
static mismatch_info rx_bal[FREQ_QUANTITY_40MHZ];
static mismatch_info tx_bal_rxcal[FREQ_QUANTITY_40MHZ];
double tx_db_sta[MAX_RFC_ITERATIONS];
double tx_phase_sta[MAX_RFC_ITERATIONS];
double rx_db_sta[MAX_RFC_ITERATIONS];
double rx_phase_sta[MAX_RFC_ITERATIONS];
int set_bw(int bw_val);
int rfc_ht(int bw, struct rfc_record_parm *parm_record, int test_case_no, int tone_mask, int debug_en)
{
    int is_skip_tx_for_rxcal = 1;   // for test, date: 08/29
    int i=0;
    int freq_quantity;
    int freq = 2412;
    int iqswap = ori_env.iqswap;
    double *tone_list;
    struct rfc_cal_parm calc_ret;
    struct vga_entry *tx_table, *rx_table, *tx_table_for_rxcal, *txlo_table;
//      mismatch_info tx_bal[FREQ_QUANTITY_40MHZ], rx_bal[FREQ_QUANTITY_40MHZ];
//      mismatch_info tx_bal_rxcal[FREQ_QUANTITY_40MHZ];
//      int   gain_mismatch_max=3;  //dB
//      int   phase_mismatch_max=20;  //degree
    double tx_avg_db = 0;
    double tx_avg_degree = 0;
    double rx_avg_db = 0;
    double rx_avg_degree = 0;

    if (bw)  /* 40MHz */
    {
        printf("!!!!!!!!!!!! WARNING !!!!!!!!!!!!!\n");
        printf("we should not use bw = 1(wide band) now\n");
        tx_table = &vga_40mhz[0];
        tx_table_for_rxcal = &vga_40mhz[2];
        txlo_table = &vga_40mhz[3];
        tone_list = tones_freq_40;
        freq_quantity = PANTHER_RFC_TONE_NUM_40;
//  	set_sampling_rate(1);	// FIXME : modify the set_sampling_rate() for panther
    }
    else    /* 20MHz */
    {
        tx_table = &vga_20mhz[0];
        tx_table_for_rxcal = &vga_20mhz[2];
        txlo_table = &vga_20mhz[3];
        tone_list = tones_freq_20;
        freq_quantity = PANTHER_RFC_TONE_NUM_20;
        set_bw(0);
//  	set_sampling_rate(0);
    }

#ifdef RFC_TIME_MEASURE
    time_measure(1);
#endif
    panther_set_iqcal_vga(TXLOOP, tx_table->rxvga, tx_table->txvga, bw, freq, iqswap);
#ifdef RFC_TIME_MEASURE
    DBG_TIME("[RFC_TIME] %s(%d): txloop=%d ms\n", __FUNCTION__, __LINE__, time_measure(0));
#endif
#ifdef TX_PAUSE_CMD
    if (panther_rfc_pause())
        return 0;
#endif

    /* cal txlo for system  */
    /* TODO: meaure time */
#ifdef RFC_TIME_MEASURE
    time_measure(1);
#endif
    RFC_DBG(RFC_DBG_INFO, "=========================\n");
    RFC_DBG(RFC_DBG_INFO, "txlo_table->bb_scale = %d\n", txlo_table->bb_scale);
    RFC_DBG(RFC_DBG_INFO, "txlo_table->txvga = %d\n", txlo_table->txvga);
    txlo_cal(txlo_table->bb_scale, txlo_table->rxvga, txlo_table->txvga, txlo_table->bb_scale_fine, 3);

#ifdef RFC_TIME_MEASURE
    DBG_TIME("[RFC_TIME] %s(%d): txlo=%d ms\n", __FUNCTION__, __LINE__, time_measure(0));
#endif

#ifdef PANTHER_RFC_PAUSE_CMD
    if (is_keyin_leave == 1)
    {
        return 0;
    }
#endif

    rfc_result_ptr[bw].txlo_msb = bb_register_read(0, 0x21);
    rfc_result_ptr[bw].tx_dc_i = bb_register_read(0, 0x25);
    rfc_result_ptr[bw].tx_dc_q = bb_register_read(0, 0x30);

#ifdef PLL_CAL_IN_RFC
#ifdef RFC_TIME_MEASURE
    time_measure(1);
#endif
    mt_Panther_Init(&mt_handle);  // do pll calibration now
    lrf_set_pll(2442);
//  lrf_k_pll_set_ch(freq, bw);
#ifdef RFC_TIME_MEASURE
    DBG_TIME("[RFC_TIME] %s(%d): pll k & set = %d ms\n", __FUNCTION__, __LINE__, time_measure(0));
#endif
#endif

    /* cal txcal for system  */
    /* TODO: meaure time */
#ifdef RFC_TIME_MEASURE
    time_measure(1);
#endif
    rfc_tx_cal(freq_quantity, tone_mask, freq, bw, tx_table, debug_en, tx_bal);
#ifdef RFC_TIME_MEASURE
    DBG_TIME("[RFC_TIME] %s(%d): txcal=%d ms\n", __FUNCTION__, __LINE__, time_measure(0));
#endif

#ifdef PANTHER_RFC_PAUSE_CMD
    if (is_keyin_leave == 1)
    {
        return 0;
    }
#endif

    /* cal txlo for rxcal  */
    /* TODO: meaure time */
    if (!is_skip_tx_for_rxcal)
    {
#ifdef RFC_TIME_MEASURE
        time_measure(1);
#endif
        txlo_cal(tx_table_for_rxcal->bb_scale, tx_table_for_rxcal->rxvga,
                 tx_table_for_rxcal->txvga, tx_table_for_rxcal->bb_scale_fine, 2);
#ifdef RFC_TIME_MEASURE
        DBG_TIME("[RFC_TIME] %s(%d): txlo=%d ms\n", __FUNCTION__, __LINE__, time_measure(0));
#endif
    }

#ifdef PANTHER_RFC_PAUSE_CMD
    if (is_keyin_leave == 1)
    {
        return 0;
    }
#endif

    if (!is_skip_tx_for_rxcal)
    {
#ifdef PLL_CAL_IN_RFC
        /* cal PLL & set channel */
        /* TODO: meaure time */
#ifdef RFC_TIME_MEASURE
        time_measure(1);
#endif
        mt_Panther_Init(&mt_handle);  // do pll calibration now
        lrf_set_pll(2442);  // do not pll calibration now, we may need calibration in the future
#ifdef RFC_TIME_MEASURE
        DBG_TIME("[RFC_TIME] %s(%d): pll k & set = %d ms\n", __FUNCTION__, __LINE__, time_measure(0));
#endif
#endif

        /* cal txcal for rxcal  */
        /* TODO: meaure time */
#ifdef RFC_TIME_MEASURE
        time_measure(1);
#endif
        rfc_tx_cal(freq_quantity, tone_mask, freq, bw, tx_table_for_rxcal, debug_en, tx_bal_rxcal);
#ifdef RFC_TIME_MEASURE
        DBG_TIME("[RFC_TIME] %s(%d): txcal=%d ms\n", __FUNCTION__, __LINE__, time_measure(0));
#endif
    }

    ////////////////////////////////////////////
    // Rx Calibration with lYNX setting first //
    /* set rx loopback */
    is_rxloop_panther = 0;
    if (bw)
        rx_table = &vga_40mhz[4];
    else
        rx_table = &vga_20mhz[4];
#ifdef RFC_TIME_MEASURE
    time_measure(1);
#endif
    panther_set_iqcal_vga(RXLOOP, rx_table->rxvga, rx_table->txvga, bw, freq, iqswap);
#ifdef RFC_TIME_MEASURE
    DBG_TIME("[RFC_TIME] %s(%d): rxloop with lynx=%d ms\n", __FUNCTION__, __LINE__, time_measure(0));
#endif

#ifdef PLL_CAL_IN_RFC
    /* cal PLL & set channel */
    /* TODO: meaure time */
#ifdef RFC_TIME_MEASURE
    time_measure(1);
#endif
    mt_Panther_Init(&mt_handle);  // do pll calibration now
    lrf_set_pll(2442);
    //  lrf_k_pll_set_ch(freq, bw);
#ifdef RFC_TIME_MEASURE
    DBG_TIME("[RFC_TIME] %s(%d): pll k & set = %d ms\n", __FUNCTION__, __LINE__, time_measure(0));
#endif
#endif

    /* cal rxcal for system  */
    RFC_DBG(RFC_DBG_INFO, "======= RFC test : test_case_no = %d, rx cal\n", test_case_no);
#ifdef RFC_TIME_MEASURE
    time_measure(1);
#endif
    if (is_skip_tx_for_rxcal)
    {
        rfc_rx_cal(freq_quantity, tone_mask, bw, rx_table, debug_en, tx_bal, rx_bal);
    }
    else
    {
        rfc_rx_cal(freq_quantity, tone_mask, bw, rx_table, debug_en, tx_bal_rxcal, rx_bal);
    }
#ifdef RFC_TIME_MEASURE
    DBG_TIME("[RFC_TIME] %s(%d): rxcal=%d ms\n", __FUNCTION__, __LINE__, time_measure(0));
#endif

#ifdef PANTHER_RFC_PAUSE_CMD
    if (is_keyin_leave == 1)
    {
        return 0;
    }
#endif
    /////////////////////////////////////////
    // Rx Calibration with Panther setting //
    /////////////////////////////////////////
//    is_rxloop_panther = 1;
//    if (bw)
//        rx_table = &vga_40mhz[1];
//    else
//        rx_table = &vga_20mhz[1];
//#ifdef RFC_TIME_MEASURE
//    time_measure(1);
//#endif
//    panther_set_iqcal_vga(RXLOOP, rx_table->rxvga, rx_table->txvga, bw, freq, iqswap);
//#ifdef RFC_TIME_MEASURE
//    DBG_TIME("[RFC_TIME] %s(%d): rxloop with panther=%d ms\n", __FUNCTION__, __LINE__, time_measure(0));
//#endif
//
//    /* cal PLL & set channel */
//    /* TODO: meaure time */
//#ifdef RFC_TIME_MEASURE
//    time_measure(1);
//#endif
//    mt_Panther_Init(&mt_handle);  // do pll calibration now
//    lrf_set_pll(2442);
////  lrf_k_pll_set_ch(freq, bw);
//#ifdef RFC_TIME_MEASURE
//    DBG_TIME("[RFC_TIME] %s(%d): pll k & set = %d ms\n", __FUNCTION__, __LINE__, time_measure(0));
//#endif
//    /* cal rxcal for system  */
//    RFC_DBG(RFC_DBG_INFO, "======= RFC test : test_case_no = %d, rx cal\n", test_case_no);
//    /* TODO: meaure time */
//#ifdef RFC_TIME_MEASURE
//    time_measure(1);
//#endif
//    if (is_skip_tx_for_rxcal)
//    {
//        rfc_rx_cal(freq_quantity, tone_mask, bw, rx_table, debug_en, tx_bal, rx_bal);
//    }
//    else
//    {
//        rfc_rx_cal(freq_quantity, tone_mask, bw, rx_table, debug_en, tx_bal_rxcal, rx_bal);
//    }
//#ifdef RFC_TIME_MEASURE
//    DBG_TIME("[RFC_TIME] %s(%d): rxcal=%d ms\n", __FUNCTION__, __LINE__, time_measure(0));
//#endif
    // End of rfc_rx_cal //
    ///////////////////////

#ifdef RFC_TIME_MEASURE
    time_measure(1);
#endif
//    //Reset tx to normal mode
//    tx_mux_regs(MUX_BASEBAND, 0, 0, 0); //unsigned char tg_a_i, unsigned char tg_a_q, double ctl_coe are useless

    /* start to calc the txcal & rxcal result */

    RFC_DBG(RFC_DBG_INFO, "before check_parm_with_mask\n\n");
#ifdef TONE_MASK_ENABLE
    freq_quantity = check_parm_with_mask(tx_bal, rx_bal, tone_mask, 0, freq_quantity);
#endif

    RFC_DBG(RFC_DBG_INFO, "before compute_balancer\n\n");
    // cal the tx/rx gain & phi mean
    compute_balancer(tx_bal, rx_bal, &calc_ret, freq_quantity, tone_list, bw);

    RFC_DBG(RFC_DBG_INFO, "before transfer_twofilter_regs\n\n");
    // Store the tx calibration result
    transfer_twofilter_regs(&calc_ret, &rfc_result_ptr[bw]);

    bal_regs(calc_ret.tx_avg, TX_BAL);
    bal_regs(calc_ret.rx_avg, RX_BAL);
#ifdef RFC_TIME_MEASURE
    DBG_TIME("[RFC_TIME] %s(%d): rfc calc=%d ms\n", __FUNCTION__, __LINE__, time_measure(0));
#endif

#if defined(RFC_DEBUG)
    RFC_DBG(RFC_DBG_ERR, "RFC final result:\n");
    RFC_DBG(RFC_DBG_ERR, "txgain=[");
    for (i=0; i<freq_quantity; i++)
    {
        tx_avg_db += 20*log10(tx_bal[i].gain.imag);
        RFC_DBG_DOUBLE(RFC_DBG_ERR, 20*log10(tx_bal[i].gain.imag)); RFC_DBG(RFC_DBG_ERR, " \n");
    }
    RFC_DBG(RFC_DBG_ERR, "];\n");
    tx_avg_db = (tx_avg_db / 4);
    RFC_DBG(RFC_DBG_ERR, "txphase=[");
    for (i=0; i<freq_quantity; i++)
    {
        tx_avg_degree += ((tx_bal[i].phi*180)/3.1415926);
        RFC_DBG_DOUBLE(RFC_DBG_ERR, (tx_bal[i].phi*180)/3.1415926); RFC_DBG(RFC_DBG_ERR, " \n");
    }
    RFC_DBG(RFC_DBG_ERR, "];\n");
    tx_avg_degree = (tx_avg_degree / 4);
    RFC_DBG(RFC_DBG_ERR, "rxgain=[");
    for (i=0; i<freq_quantity; i++)
    {
        rx_avg_db += 20*log10(rx_bal[i].gain.imag);
        RFC_DBG_DOUBLE(RFC_DBG_ERR, 20*log10(rx_bal[i].gain.imag)); RFC_DBG(RFC_DBG_ERR, " \n ");
    }
    RFC_DBG(RFC_DBG_ERR, "];\n");
    rx_avg_db = (rx_avg_db / 4);
    RFC_DBG(RFC_DBG_ERR, "rxphase=[");
    for (i=0; i<freq_quantity; i++)
    {
        rx_avg_degree += ((rx_bal[i].phi*180)/3.1415926);
        RFC_DBG_DOUBLE(RFC_DBG_ERR, (rx_bal[i].phi*180)/3.1415926); RFC_DBG(RFC_DBG_ERR, " \n");
    }
    RFC_DBG(RFC_DBG_ERR, "];\n");
    rx_avg_degree = (rx_avg_degree / 4);

    //Check results
    RFC_DBG(RFC_DBG_ERR, "\n============ RFC test %d final result:\n", test_case_no);
    RFC_DBG(RFC_DBG_ERR, "Tx mismatch : ");
    RFC_DBG_DOUBLE(RFC_DBG_ERR, tx_avg_db);
    RFC_DBG(RFC_DBG_ERR, " db, ");
    RFC_DBG_DOUBLE(RFC_DBG_ERR, tx_avg_degree);
    RFC_DBG(RFC_DBG_ERR, " degree\n");
    RFC_DBG(RFC_DBG_ERR, "Rx mismatch : ");
    RFC_DBG_DOUBLE(RFC_DBG_ERR, rx_avg_db);
    RFC_DBG(RFC_DBG_ERR, " db, ");
    RFC_DBG_DOUBLE(RFC_DBG_ERR, rx_avg_degree);
    RFC_DBG(RFC_DBG_ERR, " degree\n");
    RFC_DBG(RFC_DBG_ERR, "\n\n\n");
#endif //	RFC_DEBUG

#if defined(RFC_DEBUG) && defined(RFC_LOOP_STATISTIC)
    tx_avg_db = 0;
    tx_avg_degree = 0;
    rx_avg_db = 0;
    rx_avg_degree = 0;

    DBG_PRINTF(INFO_NO_PREFIX, "RFC final result:\n");
    DBG_PRINTF(INFO_NO_PREFIX, "txgain=[");
    tx_avg_db = 0;
    for (i=0; i<freq_quantity; i++)
    {
        tx_avg_db += 20*log10(tx_bal[i].gain.imag);
        dbg_double(RFC_DBG_TRUE, 20*log10(tx_bal[i].gain.imag)); DBG_PRINTF(INFO_NO_PREFIX, " \n");
    }
    DBG_PRINTF(INFO_NO_PREFIX, "];\n");
    tx_avg_db = (tx_avg_db / 4);
    DBG_PRINTF(INFO_NO_PREFIX, "txphase=[");
    for (i=0; i<freq_quantity; i++)
    {
        tx_avg_degree += ((tx_bal[i].phi*180)/3.1415926);
        dbg_double(RFC_DBG_TRUE, (tx_bal[i].phi*180)/3.1415926); DBG_PRINTF(INFO_NO_PREFIX, " \n");
    }
    DBG_PRINTF(INFO_NO_PREFIX, "];\n");
    tx_avg_degree = (tx_avg_degree / 4);
    DBG_PRINTF(INFO_NO_PREFIX, "rxgain=[");
    for (i=0; i<freq_quantity; i++)
    {
        rx_avg_db += 20*log10(rx_bal[i].gain.imag);
        dbg_double(RFC_DBG_TRUE, 20*log10(rx_bal[i].gain.imag)); DBG_PRINTF(INFO_NO_PREFIX, " \n ");
    }
    DBG_PRINTF(INFO_NO_PREFIX, "];\n");
    rx_avg_db = (rx_avg_db / 4);
    DBG_PRINTF(INFO_NO_PREFIX, "rxphase=[");
    for (i=0; i<freq_quantity; i++)
    {
        rx_avg_degree += ((rx_bal[i].phi*180)/3.1415926);
        dbg_double(RFC_DBG_TRUE, (rx_bal[i].phi*180)/3.1415926); DBG_PRINTF(INFO_NO_PREFIX, " \n");
    }
    DBG_PRINTF(INFO_NO_PREFIX, "];\n");
    rx_avg_degree = (rx_avg_degree / 4);

    //Check results
    DBG_PRINTF(INFO_NO_PREFIX, "============ RFC loop: %d, final result: ===========\n", test_case_no);
    DBG_PRINTF(INFO_NO_PREFIX, "Tx mismatch : ");
    dbg_double(RFC_DBG_TRUE, tx_avg_db);
    DBG_PRINTF(INFO_NO_PREFIX, " db, ");
    dbg_double(RFC_DBG_TRUE, tx_avg_degree);
    DBG_PRINTF(INFO_NO_PREFIX, " degree\n");
    DBG_PRINTF(INFO_NO_PREFIX, "Rx mismatch : ");
    dbg_double(RFC_DBG_TRUE, rx_avg_db);
    DBG_PRINTF(INFO_NO_PREFIX, " db, ");
    dbg_double(RFC_DBG_TRUE, rx_avg_degree);
    DBG_PRINTF(INFO_NO_PREFIX, " degree\n");

    tx_db_sta[test_case_no] = tx_avg_db;
    tx_phase_sta[test_case_no] = tx_avg_degree;
    rx_db_sta[test_case_no] = rx_avg_db;
    rx_phase_sta[test_case_no] = rx_avg_degree;
#endif

    return 0;
}

void adc_clk_enable(int is_enable)
{
    volatile u32 *ptr = (unsigned int *)ADC_CTRL_REG;

    if (is_enable == 1)
    {
        *ptr = (*ptr & (~0x2)) + 0x2;
    }
    else
    {
        *ptr = (*ptr & (~0x2));
    }
}

void dac_clk_enable(int is_enable)
{
    volatile u32 *ptr = (unsigned int *)ADC_CTRL_REG;

    if (is_enable == 1)
    {
        *ptr = (*ptr & (~0x1)) + 0x1;
    }
    else
    {
        *ptr = (*ptr & (~0x1));
    }
}

int old_panther_rfc_process(void)
{
    int ret = 0;
//  int lna = 3;
    //unsigned char bb25, bb30;
#ifdef RFC_TIME_MEASURE
    //int t_rxdc=0, t_txlo=0, t_rfc=0;
    //int m_start, m_rxdc, m_txlo, m_rfc;
#endif					
#if defined(RFC_ATE)
//  struct rfc_record_parm *record = &ate_record;
#else
    struct rfc_record_parm *record = NULL;
#endif

    // add here for panther
    adc_clk_enable(1);
    dac_clk_enable(1);

    //irq_disable();

    rfc_env_setup(0);
    RFC_DBG(RFC_DBG_INFO, "%s(): iqswap = %d\n", __FUNCTION__, !!(bb_register_read(0, 0x2) & 0x2));

    /* rfc */
    rfc_ht(0, record, 0, 0xff, 0x2);    // disable fine tune

    //scan_all_lna_vga_dc();
    rfc_env_recovery();

    //irq_enable();
    MACREG_UPDATE32(LMAC_CNTL, LMAC_CNTL_TSTART, LMAC_CNTL_TSTART);

#ifdef RFC_TIME_MEASURE
    time_measure(1);
#endif

//  ret = rxdcoc_exe_and_verify(lna);

#ifdef RFC_TIME_MEASURE
    DBG_TIME("[RFC_TIME] %s(%d): rxdc=%d ms\n", __FUNCTION__, __LINE__, time_measure(0));
#endif

    MACREG_UPDATE32(LMAC_CNTL, ori_env.lmac_ctrl_val, LMAC_CNTL_TSTART);
    bb_register_write(0, 0x3, 0);   // rx agc mode

    return ret;
}

int rfc_dump_adc(int is_i)
{
    unsigned char val = 0x80;
    volatile u32 *ptr;

    rf_write(18, 0x248010cc);

    ptr = (unsigned int*)0xC089C;
    *ptr = 0x80400000;

    ptr = (unsigned int*)0xC0834;
    *ptr = *ptr & (~(1<<10));

    if (is_i)
        val = 0x70;
    bb_register_write(0, 0x1d, val);
    bb_register_write(0, 0x6, 0x58);
    bb_register_write(0, 0x2, bb_register_read(0, 0x2) & (~0x40));

    return 0;
}

int dump_demod_lpf(int is_tx, double freq)
{
    int lpf_sel=0, lpf_reset=0;
    complex i_signal, q_signal;

    if (is_tx == 1) //Tx mode
        rx_demod_regs(0x3, freq);
    else if (is_tx == 0) //Rx mode
        rx_demod_regs(0, freq);

    read_LPF(&i_signal, &q_signal, lpf_reset, lpf_sel);
    if (!is_tx || ori_env.iqswap)
    {
        RFC_DBG(RFC_DBG_INFO, "i_signal (is_tx=%d): ", is_tx); 
        RFC_DBG_COMPLEX(RFC_DBG_INFO, i_signal); RFC_DBG(RFC_DBG_INFO, "\n");
    }
    if (!is_tx || !ori_env.iqswap)
    {
        RFC_DBG(RFC_DBG_INFO, "q_signal (is_tx=%d): ", is_tx); 
        RFC_DBG_COMPLEX(RFC_DBG_INFO, q_signal); RFC_DBG(RFC_DBG_INFO, "\n");
    }

    return 0;
}

int rf_reg_ctrl(int bit_index, int val)
{
    int new_val = 0;

    new_val = *(volatile unsigned int *)0xbf0048f8;
    new_val &= ~(1 << bit_index);
    new_val |= (val << bit_index);

    *(volatile unsigned int *)0xbf0048f8 = new_val;

    return 0;
}

#define CPLL_XDIV_REG 0xBF004C7CUL
#define WIFI_BW_MODE 0x80000000
int read_bw(void)
{
    int val = 0;

    val = *(volatile unsigned int *)CPLL_XDIV_REG;

    if (val & WIFI_BW_MODE)
    {
        RFC_DBG(RFC_DBG_INFO, "bw modw is \"wide band\" now\n");
    }
    else
    {
        RFC_DBG(RFC_DBG_INFO, "bw modw is \"narrow band\" now\n");
    }

    return 0;
}

int set_bw(int bw_val)
{
    int new_val = 0;

    new_val = *(volatile unsigned int *)CPLL_XDIV_REG;
    new_val &= ~(WIFI_BW_MODE);

    if (bw_val)
    {
        RFC_DBG(RFC_DBG_INFO, "set bw to wide band \n");
        new_val |= WIFI_BW_MODE;
    }
    else
    {
        RFC_DBG(RFC_DBG_INFO, "set bw to narrow band \n");
    }

    *(volatile unsigned int *)CPLL_XDIV_REG = new_val;

    return 0;
}

extern void lrf_tx_on(void);
int ez_test(int loop)
{
    int i = 0;
    struct rfc_record_parm *record = NULL;
#if defined(RFC_LOOP_STATISTIC) && !defined(__KERNEL__)
    double max_tx_db = 0, min_tx_db = 0;
    double max_tx_phase = 0, min_tx_phase = 0;
    double max_rx_db = 0, min_rx_db = 0;
    double max_rx_phase = 0, min_rx_phase = 0;
    double avg_tx_db = 0, sta_tx_db = 0;
    double avg_tx_phase = 0, sta_tx_phase = 0;
    double avg_rx_db = 0, sta_rx_db = 0;
    double avg_rx_phase = 0, sta_rx_phase = 0;

    // about txlo_cal result
    int max_real_dc = 0, min_real_dc = 0;
    int max_image_dc = 0, min_image_dc = 0;
    double avg_real_dc = 0, sta_real_dc = 0;
    double avg_image_dc = 0, sta_image_dc = 0;
#endif

    if (loop > MAX_RFC_ITERATIONS)
    {
        loop = MAX_RFC_ITERATIONS;
    }

    txlo_count = 0;
    for (i = 0; i < loop; i++)
    {
        rfc_env_setup(0);

        lrf_tx_on();
        lrf_set_pll(2442);
        bb_register_write(0, 0x1c, 0xee);

        // east test with rfc_ht(0, record, 0, 0xff, 0x2), run to txlo_cal
        rfc_ht(0, record, i, 0xff, 0x0);
        txlo_count++;

#ifdef PANTHER_RFC_PAUSE_CMD
        if (is_keyin_leave == 1)
        {
            is_keyin_leave = 0;
            return 0;
        }
#endif    
        rfc_env_recovery();
    }

#if 0//defined(RFC_LOOP_STATISTIC) && !defined(__KERNEL__)
    // initial value
    max_tx_db = min_tx_db = tx_db_sta[0];
    max_tx_phase = min_tx_phase = tx_phase_sta[0];
    max_rx_db = min_rx_db = rx_db_sta[0];
    max_rx_phase = min_rx_phase = rx_phase_sta[0];
    max_real_dc = min_real_dc = real_dc[0];
    max_image_dc = min_image_dc = image_dc[0];

    DBG_PRINTF(INFO_NO_PREFIX, "\n===========================================================================================================\n");
    DBG_PRINTF(INFO_NO_PREFIX, " Statistic Table:\n");
    DBG_PRINTF(INFO_NO_PREFIX, "-----------------------------------------------------------------------------------------------------------\n");
    DBG_PRINTF(INFO_NO_PREFIX, "| attr |    TX Gain    |    TX Phase    |    RX Gain    |    RX Phase    |    TX Real    |    TX Image    |\n");
    for (i = 0; i < loop; i++)
    {
        // get maximun value
        if (tx_db_sta[i] > max_tx_db)
            max_tx_db = tx_db_sta[i];
        if (tx_phase_sta[i] > max_tx_phase)
            max_tx_phase = tx_phase_sta[i];
        if (rx_db_sta[i] > max_rx_db)
            max_rx_db = rx_db_sta[i];
        if (rx_phase_sta[i] > max_rx_phase)
            max_rx_phase = rx_phase_sta[i];
        if (real_dc[i] > max_real_dc)
            max_real_dc = real_dc[i];
        if (image_dc[i] > max_image_dc)
            max_image_dc = image_dc[i];

        // get minimum value
        if (tx_db_sta[i] < min_tx_db)
            min_tx_db = tx_db_sta[i];
        if (tx_phase_sta[i] < min_tx_phase)
            min_tx_phase = tx_phase_sta[i];
        if (rx_db_sta[i] < min_rx_db)
            min_rx_db = rx_db_sta[i];
        if (rx_phase_sta[i] < min_rx_phase)
            min_rx_phase = rx_phase_sta[i];
        if (real_dc[i] < min_real_dc)
            min_real_dc = real_dc[i];
        if (image_dc[i] < min_image_dc)
            min_image_dc = image_dc[i];

        // add avg
        avg_tx_db += tx_db_sta[i];
        avg_tx_phase += tx_phase_sta[i];
        avg_rx_db += rx_db_sta[i];
        avg_rx_phase += rx_phase_sta[i];
        avg_real_dc += real_dc[i];
        avg_image_dc += image_dc[i];

        DBG_PRINTF(INFO_NO_PREFIX, "| %04d |   %+.5f    |    %+.5f    |   %+.5f    |    %+.5f    |  %+7d      |  %+8d      |\n"
                   , i + 1, tx_db_sta[i], tx_phase_sta[i], rx_db_sta[i], rx_phase_sta[i], real_dc[i], image_dc[i]);
    }

    // show max and min value
    DBG_PRINTF(INFO_NO_PREFIX, "| max. |   %+.5f    |    %+.5f    |   %+.5f    |    %+.5f    |  %+7d      |  %+8d      |\n"
               , max_tx_db, max_tx_phase, max_rx_db, max_rx_phase, max_real_dc, max_image_dc);
    DBG_PRINTF(INFO_NO_PREFIX, "| min. |   %+.5f    |    %+.5f    |   %+.5f    |    %+.5f    |  %+7d      |  %+8d      |\n"
               , min_tx_db, min_tx_phase, min_rx_db, min_rx_phase, min_real_dc, min_image_dc);

    // calculate and show avg
    avg_tx_db = (avg_tx_db / loop);
    avg_tx_phase = (avg_tx_phase / loop);
    avg_rx_db = (avg_rx_db / loop);
    avg_rx_phase = (avg_rx_phase / loop);
    avg_real_dc = (avg_real_dc / loop);
    avg_image_dc = (avg_image_dc / loop);
    DBG_PRINTF(INFO_NO_PREFIX, "| avg. |   %+.5f    |    %+.5f    |   %+.5f    |    %+.5f    |   %+2.5f   |    %+2.5f    |\n"
               , avg_tx_db, avg_tx_phase, avg_rx_db, avg_rx_phase, avg_real_dc, avg_image_dc);

    // calculate and show standard deviation
    for (i = 0; i < loop; i++)
    {
        sta_tx_db += pow((tx_db_sta[i] - avg_tx_db), 2);
        sta_tx_phase += pow((tx_phase_sta[i] - avg_tx_phase), 2);
        sta_rx_db += pow((rx_db_sta[i] - avg_rx_db), 2);
        sta_rx_phase += pow((rx_phase_sta[i] - avg_rx_phase), 2);
        sta_real_dc += pow((real_dc[i] - avg_real_dc), 2);
        sta_image_dc += pow((image_dc[i] - avg_image_dc), 2);
    }
    sta_tx_db = (sta_tx_db / loop);
    sta_tx_phase = (sta_tx_phase / loop);
    sta_rx_db = (sta_rx_db / loop);
    sta_rx_phase = (sta_rx_phase / loop);
    sta_real_dc = (sta_real_dc / loop);
    sta_image_dc = (sta_image_dc / loop);
    DBG_PRINTF(INFO_NO_PREFIX, "| sta. |   %+.5f    |    %+.5f    |   %+.5f    |    %+.5f    |    %+2.5f   |    %+2.5f    |\n"
               , sta_tx_db, sta_tx_phase, sta_rx_db, sta_rx_phase, sta_real_dc, sta_image_dc);

    DBG_PRINTF(INFO_NO_PREFIX, "===========================================================================================================\n");
#endif

    return 0;
}

int lb_dc_cancel(void)
{
    int iqswap = ori_env.iqswap;

    /* force set iqswap = 0*/
    bb_register_write(0, 0x2, (bb_register_read(0, 0x2) & ~0x2));
    ori_env.iqswap = 0;
#if 0
    search_dcoc_lsb_and_track(5, -1, &dcoc_dac_lsb, &dcoc_dac_lsb_track);
    detect_dco_dac(5, dcoc_dac_lsb, dcoc_dac_lsb_track, -1, NULL, NULL);
#else
    detect_dco_dac(5, 2, 1, -1, NULL, NULL, &m_final_i, &m_final_q);    // panther test, date: 08/14
//  detect_dco_dac(5, 0, 5, -1, NULL, NULL);
#endif
    bb_register_write(0, 0x2, (bb_register_read(0, 0x2) & ~0x2) | (iqswap << 1));
    ori_env.iqswap = iqswap;

    return 0;
}

#if 0 //defined(RFC_DEBUG)
static int tx_iq_mismatch_check_ht(int bw, int ch_sel, int rfc_test_case_no, double *tx_iq_mismatch_check)
{
    double ctl_coe;
    int freq = 2412;
    unsigned char cal_mode_sel;
    unsigned char tg_a_i, tg_a_q;
    complex I_signal_f0, Q_signal_f0;
    unsigned int txvga, rxvga;
    /* for break function */
#if 0
    unsigned char buf[4];
    (void) buf;
#endif

    if (bw)  /* 40MHz */
    {
        tg_a_i = vga_40mhz[0].bb_scale;
        tg_a_q = vga_40mhz[0].bb_scale;
        rxvga = vga_40mhz[0].rxvga;
        txvga = vga_40mhz[0].txvga;
    }
    else    /* 20MHz */
    {
        tg_a_i = vga_20mhz[0].bb_scale;
        tg_a_q = vga_20mhz[0].bb_scale;
        rxvga = vga_20mhz[0].rxvga;
        txvga = vga_20mhz[0].txvga;
    }

    /* need to confirm */
    bb_register_write(0, 0x1c, 0xee);

    RFC_DBG(RFC_DBG_INFO, "tx_iq_mismatch_check_ht()\n");

    panther_set_iqcal_vga(TXLOOP, rxvga, txvga, bw, freq, ori_env.iqswap);

    // Set f0 = 5 MHz tone and send f0 tone; set RFC demodulator to read f0 tone magnitude
    ctl_coe = 2.5;
    tx_mux_regs(MUX_TONEGEN, tg_a_i, tg_a_q, ctl_coe);

    cal_mode_sel = 0x3;
    rx_demod_regs(cal_mode_sel, ctl_coe);

#if 0
    RFC_DBG(RFC_DBG_INFO, "press ENTER to continue the test.\n");
    WLA_GETS(buf);
#endif
    if (read_LPF(&I_signal_f0, &Q_signal_f0, LPF_RESET, READ_LPF_DELAY_SEL))
        return 1;

    RFC_DBG(RFC_DBG_INFO, "f0: ");
    RFC_DBG_COMPLEX(RFC_DBG_INFO, Q_signal_f0);
    RFC_DBG(RFC_DBG_INFO, "  ");
    RFC_DBG_DOUBLE(RFC_DBG_INFO, __complex_to_db(&Q_signal_f0));
    RFC_DBG(RFC_DBG_INFO, " db\n");

    tx_iq_mismatch_check[0] = __complex_to_db(&Q_signal_f0);

    ctl_coe *= 2;
    cal_mode_sel = 0x3;
    rx_demod_regs(cal_mode_sel, ctl_coe);

#if 0
    RFC_DBG(RFC_DBG_INFO, "press ENTER to continue the test.\n");
    WLA_GETS(buf);
#endif

    if (read_LPF(&I_signal_f0, &Q_signal_f0, LPF_RESET, READ_LPF_DELAY_SEL))
        return 1;

    RFC_DBG(RFC_DBG_INFO, "2f0: ");
    RFC_DBG_COMPLEX(RFC_DBG_INFO, Q_signal_f0);
    RFC_DBG(RFC_DBG_INFO, "  ");
    RFC_DBG_DOUBLE(RFC_DBG_INFO, __complex_to_db(&Q_signal_f0));
    RFC_DBG(RFC_DBG_INFO, " db\n");

    tx_iq_mismatch_check[1] = __complex_to_db(&Q_signal_f0);

    tx_mux_regs(MUX_BASEBAND, 0, 0, 0);

    return 0;
}

    #if 0
static int rx_iq_mismatch_check_ht(int bw, int ch_sel, int rfc_test_case_no, double *rx_iq_mismatch_check)
{
    double ctl_coe;
    int freq = 2412;
    unsigned char cal_mode_sel;
    unsigned char tg_a_i, tg_a_q;
    complex I_signal_f0, Q_signal_f0;
    unsigned int txvga, rxvga;
    /* for break function */
#if 0
    unsigned char buf[16];
    (void) buf;
#endif

    if (bw)  /* 40MHz */
    {
        tg_a_i = vga_40mhz[1].bb_scale;
        tg_a_q = vga_40mhz[1].bb_scale;
        rxvga = vga_40mhz[1].rxvga;
        txvga = vga_40mhz[1].txvga;
    }
    else    /* 20MHz */
    {
        tg_a_i = vga_20mhz[1].bb_scale;
        tg_a_q = vga_20mhz[1].bb_scale;
        rxvga = vga_20mhz[1].rxvga;
        txvga = vga_20mhz[1].txvga;
    }

    RFC_DBG(RFC_DBG_INFO, "rx_iq_mismatch_check_ht()\n");

    // Set RX loopback mode
    panther_set_iqcal_vga(RXLOOP, rxvga, txvga, bw, freq, ori_env.iqswap);

    // Set f0 = 5 MHz tone and send f0 tone; set RFC demodulator to read f0 tone magnitude
    ctl_coe = 5.0;
    tx_mux_regs(MUX_TONEGEN, tg_a_i, tg_a_q, ctl_coe);

    if (bb_register_read(0, 0) >= 0x32)
        cal_mode_sel = 0x1;  //Dmod input control: from rxbnc 
    else
        cal_mode_sel = 0x0;  //Dmod input control: normal mode, from ADC 
    rx_demod_regs(cal_mode_sel, ctl_coe);

    //udelay(1000000);
#if 0
    RFC_DBG(RFC_DBG_INFO, "press ENTER to continue the test.\n");
    WLA_GETS(buf);
#endif

    if (read_LPF(&I_signal_f0, &Q_signal_f0, LPF_RESET, READ_LPF_DELAY_SEL))
        return 1;

    RFC_DBG(RFC_DBG_INFO, "-f0: ");
    RFC_DBG_COMPLEX(RFC_DBG_INFO, I_signal_f0);
    RFC_DBG(RFC_DBG_INFO, "  ");
    RFC_DBG_DOUBLE(RFC_DBG_INFO, __complex_to_db(&I_signal_f0));
    RFC_DBG(RFC_DBG_INFO, " db");
    RFC_DBG(RFC_DBG_INFO, ", f0: ");
    RFC_DBG_COMPLEX(RFC_DBG_INFO, Q_signal_f0);
    RFC_DBG(RFC_DBG_INFO, "  ");
    RFC_DBG_DOUBLE(RFC_DBG_INFO, __complex_to_db(&Q_signal_f0));
    RFC_DBG(RFC_DBG_INFO, " db");
    RFC_DBG(RFC_DBG_INFO, "\n");    

    rx_iq_mismatch_check[0] = __complex_to_db(&I_signal_f0);
    rx_iq_mismatch_check[1] = __complex_to_db(&Q_signal_f0);

    tx_mux_regs(MUX_BASEBAND, 0, 0, 0);

    return 0;
}
    #endif
#endif // RFC_DEBUG

struct mismatch_check mismatch_result;
struct rfc_test_record rfc_test_result[MAX_RFC_ITERATIONS];
int rfc_ht_cmd(int argc, char* argv[])
{
    int samples;
    int i;
    int bw = 0, bw_start = 0, bw_end=1; /* 0: 20MHZ, 1: 40MHz */
    //int ret_tx, ret_rx;
    int tx_scale=-127, rx_scale=-127;
    int tx_txvga=-127, rx_txvga=-127;
    int freq=2412;
    //unsigned int lmac_tstart;
    unsigned char bb_reg_f3=0;

    int tone_mask=0xffff, debug_en=0, bandwidth=0x3, repeat=0;

    /* For calibration record */
    struct rfc_test_record *record=NULL;
    //double tx_iq_mis[2][2];
#if defined(RFC_DEBUG)
    struct mismatch_check *mismatch=&mismatch_result;
#endif

    record = rfc_test_result;

    //libm_test();

    switch (argc)
    {
        case 5:
            debug_en = simple_strtol(argv[4], NULL, 10);
        case 4:
            bandwidth = simple_strtol(argv[3], NULL, 10);
            if (!(bandwidth & 0x1))
                bw_start = 1;
            if (!(bandwidth & 0x2))
                bw_end = 0;
        case 3:
            tone_mask = simple_strtol(argv[2], NULL, 10);
        case 2:
            repeat = simple_strtol(argv[1], NULL, 10);
            break;
        default:
            RFC_DBG(RFC_DBG_INFO, "Wrong argc!!!\n");
            goto out;
    }

    /* store bb reg f3 & restore it after rfc */
    bb_reg_f3 = bb_register_read(0, 0xf3);
    if (bb_reg_f3 & 0x40)
        bb_register_write(0, 0xf3, bb_reg_f3 & 0xBF);

    if (repeat == 65535)
        rx_dc_offset_comp(14, 1, NULL, NULL);

    RFC_DBG(RFC_DBG_INFO, "repeat = %d, tone_mask = 0x%x, bw = %d, bw_start = %d, bw_end = %d, debug = %d, rx_scale = %d, tx_scale = %d, rx_txvga = %d, tx_txvga = %d\n", repeat, tone_mask, bandwidth, bw_start, bw_end, debug_en, rx_scale, tx_scale, rx_txvga, tx_txvga);

    rfc_env_setup(0);

    if ((argc==0) || (repeat==1))
    {
        for (bw=bw_start; bw<=bw_end; bw++)
        {
            if (rfc_ht(bw, &(record[0].parm[bw]), 0, tone_mask, debug_en))
                goto out;
            config_rfc_parm(bw, freq);
#if 0	
            if (bw)
                RFC_DBG(RFC_DBG_INFO, "========= 40MHz Test Result =========\n");
            else
                RFC_DBG(RFC_DBG_INFO, "========= 20MHz Test Result =========\n");
            RFC_DBG(RFC_DBG_INFO, "TX Gain:"); 
            RFC_DBG_DOUBLE(RFC_DBG_INFO, record[0].parm[bw].tx_gain); 
            RFC_DBG(RFC_DBG_INFO, " db  ");
            RFC_DBG(RFC_DBG_INFO, "Phase:");  
            RFC_DBG_DOUBLE(RFC_DBG_INFO, record[0].parm[bw].tx_phase); 
            RFC_DBG(RFC_DBG_INFO, " degree\n");
            RFC_DBG(RFC_DBG_INFO, "RX Gain:"); 
            RFC_DBG_DOUBLE(RFC_DBG_INFO, record[0].parm[bw].rx_gain); 
            RFC_DBG(RFC_DBG_INFO, " db  ");
            RFC_DBG(RFC_DBG_INFO, "Phase:");  
            RFC_DBG_DOUBLE(RFC_DBG_INFO, record[0].parm[bw].rx_phase); 
            RFC_DBG(RFC_DBG_INFO, " degree\n");
            RFC_DBG(RFC_DBG_INFO, "=====================================\n");
            RFC_DBG(RFC_DBG_INFO, "== The rx calibration result ");
            if (ret_rx)
                RFC_DBG(RFC_DBG_INFO, "Fail!!! ==\n");
            else
                RFC_DBG(RFC_DBG_INFO, "Success!!! ==\n");
            RFC_DBG(RFC_DBG_INFO, "== The tx calibration result ");
            if (ret_tx)
                RFC_DBG(RFC_DBG_INFO, "Fail!!! ==\n");
            else
                RFC_DBG(RFC_DBG_INFO, "Success!!! ==\n");
            RFC_DBG(RFC_DBG_INFO, "=====================================\n");
#endif
        }
    }
    else
    {
        if ((repeat > 0) && (repeat <= MAX_RFC_ITERATIONS))
        {
            samples = repeat;

            for (i=0;i<samples;i++)
            {
                for (bw=bw_start; bw<=bw_end; bw++)
                {
                    RFC_DBG(RFC_DBG_INFO, "===== Calibration No.%d (bw = %d)=====\n", i, bw);

                    if (rfc_ht(bw, &(record[i].parm[bw]), i, tone_mask, debug_en))
                        goto out;
                    config_rfc_parm(bw, freq);
#if defined(RFC_DEBUG)
                    RFC_DBG(RFC_DBG_INFO, "===== Calibration No.%d result (bw = %d) =====\n", i, bw);
                    RFC_DBG(RFC_DBG_INFO, "TX Gain:"); 
                    RFC_DBG_DOUBLE(RFC_DBG_INFO, record[i].parm[bw].tx_gain); 
                    RFC_DBG(RFC_DBG_INFO, " db  ");
                    RFC_DBG(RFC_DBG_INFO, "Phase:");  
                    RFC_DBG_DOUBLE(RFC_DBG_INFO, record[i].parm[bw].tx_phase); 
                    RFC_DBG(RFC_DBG_INFO, " degree\n");
                    RFC_DBG(RFC_DBG_INFO, "RX Gain:"); 
                    RFC_DBG_DOUBLE(RFC_DBG_INFO, record[i].parm[bw].rx_gain); 
                    RFC_DBG(RFC_DBG_INFO, " db  ");
                    RFC_DBG(RFC_DBG_INFO, "Phase:");  
                    RFC_DBG_DOUBLE(RFC_DBG_INFO, record[i].parm[bw].rx_phase); 
                    RFC_DBG(RFC_DBG_INFO, " degree\n");
#if 0
                    rx_iq_mismatch_check_ht(bw, k, i, &(mismatch->rx_iq_mismatch[bw].rec[i][0]));
                    tx_iq_mismatch_check_ht(bw, k, i, &(mismatch->tx_iq_mismatch[bw].rec[i][0]));
#endif
#endif
                }
            }
#if defined(RFC_DEBUG)
            for (bw=bw_start; bw<=bw_end; bw++)
            {
                print_rfc_test_result(record, mismatch, samples, bw);
            }
#endif
        }
        else
        {
            RFC_DBG(RFC_DBG_INFO, "invalid repeat value %d\n", repeat);
        }
    }

    rfc_env_recovery();

out:
    return 0;
}

#if defined(RFC_ATE)
void print_rfc_result(void)
{

    RFC_DBG(RFC_DBG_TRUE, "tx_gain = ");
    RFC_DBG_DOUBLE(RFC_DBG_TRUE, ate_record.tx_gain);
    RFC_DBG(RFC_DBG_TRUE, ", tx_phase = ");
    RFC_DBG_DOUBLE(RFC_DBG_TRUE, ate_record.tx_phase);
    RFC_DBG(RFC_DBG_TRUE, ", rx_gain = ");
    RFC_DBG_DOUBLE(RFC_DBG_TRUE, ate_record.rx_gain);
    RFC_DBG(RFC_DBG_TRUE, ", rx_phase = ");
    RFC_DBG_DOUBLE(RFC_DBG_TRUE, ate_record.rx_phase);
    RFC_DBG(RFC_DBG_TRUE, "\n");
}
#endif

#if 1//def CONFIG_TODO
CMD_DECL(rfc_test_cmd)
{
    if (argc < 1)
    {
        serial_printf("rfc rxvga_adjust <ovth> <okth> <init_rxvga> <txvga_adjust> <dc_cal_en> <is_tx>\n");
        serial_printf("    set_env <is_setup: 1=setup, 0=recovery> <is_tx>\n");
        serial_printf("    set_bb_vgagain <vga>\n");
        serial_printf("    set_bb_lnagain <lna>\n");
        serial_printf("    rx_auto_cal_on_off <1=on, 0=off>\n");
        serial_printf("    rx_auto_cal_update_cycle <update_cycle>\n");
        serial_printf("    		update_cycle: 0=16384 ,1=32768 , 2=65536, 3=131072\n");
        serial_printf("    tx_loopback_report <bw> <freq> <tg_a_i> <tg_a_q> <txvga> <rxvga>\n");
        serial_printf("    rx_loopback_report <bw> <freq> <tg_a> <txvga>\n");
        serial_printf("    rfc_rx_dc_offset_comp <degree> <read_write> (read_write: bit.0 = read, bit.1 = write\n");
        serial_printf("    test_rxdcoc_function <vga> <lsb> <track> <dco_value_i> <dco_value_q>\n");
        serial_printf("    rxdcoc_scan <vga> <lsb> <track>\n");
        serial_printf("    search_dcoc_lsb_and_track <vga> <qn_swap_mode>\n");
        serial_printf("    detect_dco_dac <vga> <dcoc_dac_lsb> <dcoc_dac_lsb_track> <qn_swap_mode>\n");
        serial_printf("    main_dcoc_calibration <lna>\n");
        serial_printf("    rxdcoc_exe_and_verify <lna>\n");
        serial_printf("    rxdc_vga_scan \n");
        serial_printf("    scan_all_lna_vga_dc \n");
        serial_printf("    panther_set_iqcal_vga <loop_type> <rxvga> <txvga> <bw>\n");
        serial_printf("    set_txcal_vga <is_txvga> <vga>\n");
        serial_printf("    set_rxcal_vga <is_txvga> <vga>\n");
        serial_printf("    dump_adc <is_i_path>\n");
        serial_printf("    txlo_cal <bb_scale> <rxvga> <txvga> <bb_scale_fine> <loop_n>\n");
        serial_printf("    txlo_manual <I_dc> <Q_dc>\n");
        serial_printf("    rf_req_read_dump <sel: 1=dump, 0=read>\n");
        serial_printf("    analog_on \n");
        serial_printf("    rfc_ht <repeate> <tone_mask> <bw> <debug>\n");
        serial_printf("    		<debug>: BIT(0)=step by step, BIT(1)=disable finetune\n");
        serial_printf("    		         BIT(2)=exit after tx, BIT(3)=exit after rx\n");
        serial_printf("    		         BIT(4)=not set tx loopback, BIT(5)=bypass tx cal\n");
        serial_printf("    		         BIT(6)=not set rx loopback\n");
        serial_printf("    tonegen_man <freq> <imag> <qmag> <pwrDropdB(optional)>\n");
        serial_printf("    pwrdrop <pwrDropdB>\n");
        serial_printf("    dump_demod_lpf <is_tx> <freq>\n");
        serial_printf("    config_rfc <freq>\n");
        serial_printf("    rf_reg_ctrl <bit_index:0~4> <val>\n");
        serial_printf("    bw <val>\n");
        serial_printf("    read_bw\n");
        serial_printf("    ez_test <loop> <bbgain> <rxvga> <txvga> <pwrDrop>\n");
        serial_printf("    lb_dc_cancel\n");
        serial_printf("    pllk\n");
#ifdef PANTHER_RFC_PAUSE_CMD
        serial_printf("    pause <1=on, 0=off>\n");
#endif
#if defined(RFC_DEBUG)
        serial_printf("    dbg_lvl <0~2, 0=all print>\n");
        serial_printf("    en_time_measure <is_enable>\n");
#endif
#if defined(RFC_ATE)
        serial_printf("    print_rfc_result\n");
#endif
        return 0;
    }

    if (!strcmp(argv[0], "rxvga_adjust"))
    {
        int ovth=RXVGA_OVTH_TX, okth=85, init_rxvga=-99, txvga_adjust=1, dc_cal_en=1, is_tx=1;

        switch (argc)
        {
            case 7:
                is_tx = simple_strtol(argv[6], NULL, 10);
            case 6:
                dc_cal_en = simple_strtol(argv[5], NULL, 10);
            case 5:
                txvga_adjust = simple_strtol(argv[4], NULL, 10);
            case 4:
                init_rxvga = simple_strtol(argv[3], NULL, 10);
            case 3:
                okth = simple_strtol(argv[2], NULL, 10);
            case 2:
                ovth = simple_strtol(argv[1], NULL, 10);
            default:
                break;
        }

        serial_printf("%s(): rxvga_adjust: ovth=%d, okth=%d, init_rxvga=%d, txvga_adjust=%d, dc_cal_en=%d, is_tx=%d\n", __FUNCTION__, ovth, okth, init_rxvga, txvga_adjust, dc_cal_en, is_tx);
        if (is_tx)
            rxvga_adjust(ovth, okth, init_rxvga, txvga_adjust, dc_cal_en);
        else
            rxvga_adjust_rxloop(ovth, okth, init_rxvga, txvga_adjust, dc_cal_en);
    }
    else if (!strcmp(argv[0], "rfc_rx_dc_offset_comp"))
    {
        int degree = 10;
        int rw = 3;
        double dc_i, dc_q;
        char val_i, val_q;

        if (argc >= 2)
            degree = simple_strtol(argv[1], NULL, 10);
        if (argc >= 3)
            rw = simple_strtol(argv[2], NULL, 10);

        rx_dc_offset_comp(degree, rw, &dc_i, &dc_q);

        val_i = dc_i;
        val_q = dc_q;
        RFC_DBG(RFC_DBG_INFO, "val_i = %d (", val_i);
        RFC_DBG_DOUBLE(RFC_DBG_INFO, dc_i); RFC_DBG(RFC_DBG_INFO, ")\n");
        RFC_DBG(RFC_DBG_INFO, "val_q = %d (", val_q);
        RFC_DBG_DOUBLE(RFC_DBG_INFO, dc_q); RFC_DBG(RFC_DBG_INFO, ")\n");
    }
    else if (!strcmp(argv[0], "tx_loopback_report"))
    {
        int bw=0, freq=0;
        int agc_en=1, lpf_reset=1, lpf_sel=0;
        int follow_last_rx_gain=0;
        int tg_a_i=0, tg_a_q=0, txvga=0, rxvga=0, bb_scale_fine=0, tx_i_dc=0, tx_q_dc=0;
        unsigned short tx_nm=0, br23_phase=0, br24_gain=0x7f;
        complex read_f0, read_2f0;
        int txvga_adjust=1;

        switch (argc)
        {
            case 7:
                rxvga = simple_strtol(argv[6], NULL, 10);
            case 6:
                txvga = simple_strtol(argv[5], NULL, 10);
            case 5:
                tg_a_q = simple_strtol(argv[4], NULL, 10);
            case 4:
                tg_a_i = simple_strtol(argv[3], NULL, 10);
            case 3:
                freq = simple_strtol(argv[2], NULL, 10);
            case 2:
                bw = simple_strtol(argv[1], NULL, 10);
                break;
        }

        tx_loopback_report(bw, freq, tg_a_i, tg_a_q, txvga, rxvga, bb_scale_fine, tx_i_dc, tx_q_dc, 
                           tx_nm, br23_phase, br24_gain, agc_en, &read_f0, &read_2f0, 
                           lpf_reset, lpf_sel, follow_last_rx_gain, 
                           txvga_adjust, 65535, 65535);
    }
    else if (!strcmp(argv[0], "rx_loopback_report"))
    {
        int bw=0, freq=1;
        int agc_en=1, lpf_reset=1, lpf_sel=0;
        int tg_a=1, txvga=-3, rxvga=14, bb_scale_fine=0, rx_i_dc=0, rx_q_dc=0;
        unsigned short rx_nm=0, br28_phase=0, br29_gain=0x7f;
        int txvga_adjust=1;
        complex neg_f0, pos_f0;

        switch (argc)
        {
            case 5:
                txvga = simple_strtol(argv[4], NULL, 10);
            case 4:
                tg_a = simple_strtol(argv[3], NULL, 10);
            case 3:
                freq = simple_strtol(argv[2], NULL, 10);
            case 2:
                bw = simple_strtol(argv[1], NULL, 10);
                break;
        }

        rx_loopback_report(bw, freq, tg_a, txvga, rxvga, bb_scale_fine, rx_i_dc, rx_q_dc, rx_nm, 
                           br28_phase, br29_gain, agc_en, &neg_f0, &pos_f0, lpf_reset, lpf_sel,
                           txvga_adjust);

    }
    else if (!strcmp(argv[0], "set_bb_vgagain"))
    {
        int val = simple_strtol(argv[1], NULL, 10);

        set_bb_vgagain(val);
    }
    else if (!strcmp(argv[0], "set_bb_lnagain"))
    {
        int val = simple_strtol(argv[1], NULL, 10);

        set_bb_lnagain(val);
    }
    else if (!strcmp(argv[0], "rx_auto_cal_on_off"))
    {
        int val = simple_strtol(argv[1], NULL, 10);

        rx_auto_cal_on_off(val);
    }
    else if (!strcmp(argv[0], "rx_auto_cal_update_cycle"))
    {
        int val = simple_strtol(argv[1], NULL, 10);

        rx_auto_cal_update_cycle(val);
    }
    else if (!strcmp(argv[0], "test_rxdcoc_function"))
    {
        int vga=22, lsb=1, track=2, dco_value_i=0, dco_value_q=0;
        switch (argc)
        {
            case 6:
                dco_value_q = simple_strtol(argv[5], NULL, 10);
            case 5:
                dco_value_i = simple_strtol(argv[4], NULL, 10);
            case 4:
                track = simple_strtol(argv[3], NULL, 10);
            case 3:
                lsb = simple_strtol(argv[2], NULL, 10);
            case 2:
                vga = simple_strtol(argv[1], NULL, 10);
                break;
        }

        test_rxdcoc_function(vga, lsb, track, dco_value_i, dco_value_q);
    }
    else if (!strcmp(argv[0], "set_env"))
    {
        int is_tx = 1;
        int is_setup = 1;

        switch (argc)
        {
            case 3:
                is_tx = simple_strtol(argv[2], NULL, 10);
            case 2:
                is_setup = simple_strtol(argv[1], NULL, 10);
                break;
        }

        if (is_setup)
            rfc_env_setup(is_tx);
        else
            rfc_env_recovery();
    }
    else if (!strcmp(argv[0], "rxdcoc_scan"))
    {
        int vga=10, lsb=0, track=5;
        switch (argc)
        {
            case 4:
                track = simple_strtol(argv[3], NULL, 10);
            case 3:
                lsb = simple_strtol(argv[2], NULL, 10);
            case 2:
                vga = simple_strtol(argv[1], NULL, 10);
                break;
        }

        rxdcoc_scan(vga, lsb, track);
    }
    else if (!strcmp(argv[0], "search_dcoc_lsb_and_track"))
    {
        int vga=10, qn_swap_mode=-1, dcoc_dac_lsb, dcoc_dac_lsb_track;

        switch (argc)
        {
            case 3:
                qn_swap_mode = simple_strtol(argv[2], NULL, 10);
            case 2:
                vga = simple_strtol(argv[1], NULL, 10);
                break;
        }

        search_dcoc_lsb_and_track(vga, qn_swap_mode, &dcoc_dac_lsb, &dcoc_dac_lsb_track);
    }
    else if (!strcmp(argv[0], "detect_dco_dac"))
    {
        int vga=10, dcoc_dac_lsb=1, dcoc_dac_lsb_track=1, qn_swap_mode=-1;

        switch (argc)
        {
            case 5:
                qn_swap_mode = simple_strtol(argv[4], NULL, 10);
            case 4:
                dcoc_dac_lsb_track = simple_strtol(argv[3], NULL, 10);
            case 3:
                dcoc_dac_lsb = simple_strtol(argv[2], NULL, 10);
            case 2:
                vga = simple_strtol(argv[1], NULL, 10);
                break;
        }

        detect_dco_dac(vga, dcoc_dac_lsb, dcoc_dac_lsb_track, qn_swap_mode, NULL, NULL, &m_final_i, &m_final_q);
    }
    else if (!strcmp(argv[0], "main_dcoc_calibration"))
    {
        int lna=RXDC_HIGH;
        if (argc == 2)
            lna = simple_strtol(argv[1], NULL, 10);
        main_dcoc_calibration(lna);
    }
    else if (!strcmp(argv[0], "rxdcoc_exe_and_verify"))
    {
        int lna=3;
        if (argc == 2)
            lna = simple_strtol(argv[1], NULL, 10);
        rxdcoc_exe_and_verify(lna);
    }
    else if (!strcmp(argv[0], "rxdc_vga_scan"))
    {
        unsigned int rec[22];   // 22 = vga_max - vga_min + 1 = 26 - 5 +1
        int k=0;
        double i_dc[22], q_dc[22];

        rxdc_vga_scan(i_dc, q_dc, rec);

        for (k=0; k<22; k++) // 22 = vga_max - vga_min + 1 = 26 - 5 +1
        {
            RFC_DBG(RFC_DBG_INFO, "vga=%d, old=0x%x, new=0x%x", (k + 5), pre_rxdc_rec_ptr[k], rec[k]);

            if (pre_rxdc_rec_ptr[k] != rec[k])
                RFC_DBG(RFC_DBG_INFO, "  !!! not match\n");
            else
                RFC_DBG(RFC_DBG_INFO, "\n");
        }
    }
    else if (!strcmp(argv[0], "scan_all_lna_vga_dc"))
    {
        scan_all_lna_vga_dc();
    }
    else if (!strcmp(argv[0], "panther_set_iqcal_vga"))
    {
        int loop_type = TXLOOP;
        int rxvga = 5, txvga = 5, bw = 0;
        int iqswap = !!(bb_register_read(0, 0x2) & 0x2);
        int freq=2412;

        switch (argc)
        {
            case 5:
                bw = simple_strtol(argv[4], NULL, 10);
            case 4:
                txvga = simple_strtol(argv[3], NULL, 10);
            case 3:
                rxvga = simple_strtol(argv[2], NULL, 10);
            case 2:
                loop_type = simple_strtol(argv[1], NULL, 10);
                break;
        }

        panther_set_iqcal_vga(loop_type, rxvga, txvga, bw, freq, iqswap);
    }
    else if (!strcmp(argv[0], "set_txcal_vga"))
    {
        int vga=15, is_txvga=1;

        switch (argc)
        {
            case 3:
                vga = simple_strtol(argv[2], NULL, 10);
            case 2:
                is_txvga = simple_strtol(argv[1], NULL, 10);
                break;
        }

        if (is_txvga)
            set_txcal_txvga(vga);
        else
            set_txcal_rxvga(vga);
    }
    else if (!strcmp(argv[0], "set_rxcal_vga"))
    {
        int vga=15, is_txvga=0;

        switch (argc)
        {
            case 3:
                vga = simple_strtol(argv[2], NULL, 10);
            case 2:
                is_txvga = simple_strtol(argv[1], NULL, 10);
                break;
        }

        if (!is_txvga)
            set_rxcal_rxvga(vga);
    }
    else if (!strcmp(argv[0], "dump_adc"))
    {
        int is_i=0;

        if (argc == 2)
            is_i = simple_strtol(argv[1], NULL, 10);
        rfc_dump_adc(is_i);
    }
    else if (!strcmp(argv[0], "rf_req_read_dump"))
    {
        int sel = 1;

        if (argc == 2)
            sel = simple_strtol(argv[1], NULL, 10);

        rf_req_read_dump(sel);
    }
    else if (!strcmp(argv[0], "txlo_cal"))
    {
        int bb_scale=0, rxvga=15, txvga=0, bb_scale_fine=0, loop_n=2;

        switch (argc)
        {
            case 6:
                loop_n = simple_strtol(argv[5], NULL, 10);
            case 5:
                bb_scale_fine = simple_strtol(argv[4], NULL, 10);
            case 4:
                txvga = simple_strtol(argv[3], NULL, 10);
            case 3:
                rxvga = simple_strtol(argv[2], NULL, 10);
            case 2:
                bb_scale = simple_strtol(argv[1], NULL, 10);
                break;
        }

        txlo_cal(bb_scale, rxvga, txvga, bb_scale_fine, loop_n);
    }
    else if (!strcmp(argv[0], "txlo_manual"))
    {
        int idc = 0, qdc = 0;

        switch (argc)
        {
            case 3:
                qdc = simple_strtol(argv[2], NULL, 10);
            case 2:
                idc = simple_strtol(argv[1], NULL, 10);
                break;
        }

        txlo_manual(idc, qdc);
    }
    else if (!strcmp(argv[0], "analog_on"))
    {
        rf_write(18, (rf_read(18) & 0x007FFFFF) | 0xA0800000);  // for Sam's request, not confirm
    }
    else if (!strcmp(argv[0], "tonegen_man"))
    {
        int freq=12, imag=0, qmag=0, pwrDropdB=0;

        switch (argc)
        {
            case 5:
                pwrDropdB = simple_strtol(argv[4], NULL, 10);
            case 4:
                qmag = simple_strtol(argv[3], NULL, 10);
            case 3:
                imag = simple_strtol(argv[2], NULL, 10);
            case 2:
                freq = simple_strtol(argv[1], NULL, 10);
                break;
        }

        tonegen_man(freq, imag, qmag, pwrDropdB);
    }
    else if (!strcmp(argv[0], "pwrdrop"))
    {
        int pwrDropdB=0;

        switch (argc)
        {
            case 2:
                pwrDropdB = simple_strtol(argv[1], NULL, 10);
                break;
        }

        pwrdrop(pwrDropdB);
    }
    else if (!strcmp(argv[0], "dump_demod_lpf"))
    {
        int is_tx=0;
        double freq=-2.5;

        switch (argc)
        {
            case 3:
                freq = simple_strtol(argv[2], NULL, 10);
            case 2:
                is_tx = simple_strtol(argv[1], NULL, 10);
                break;
        }

        dump_demod_lpf(is_tx, freq);
    }
    else if (!strcmp(argv[0], "config_rfc"))
    {
        int freq=2412;

        if (argc >= 2)
            freq = simple_strtol(argv[1], NULL, 10);

        config_rfc_parm(0, freq);
    }
    else if (!strcmp(argv[0], "rf_reg_ctrl"))
    {
        int bit_index = 0;
        int val = 0;

        switch (argc)
        {
            case 3:
                val = simple_strtol(argv[2], NULL, 10);
            case 2:
                bit_index = simple_strtol(argv[1], NULL, 10);
                break;
        }

        rf_reg_ctrl(bit_index, val);
    }
    else if (!strcmp(argv[0], "bw"))
    {
        int bw_val = 0;

        if (argc >= 2)
        {
            bw_val = simple_strtol(argv[1], NULL, 10);
        }

        set_bw(bw_val);
    }
    else if (!strcmp(argv[0], "read_bw"))
    {
        read_bw();
    }
    else if (!strcmp(argv[0], "ez_test"))
    {
        int loop = 1;
        struct vga_entry *tx_table = &vga_20mhz[0];

        // reset to default
        is_rxloop_panther = 0;
        is_enable_rf12_rw = 1;

        switch (argc)
        {
            case 6:
                tx_table->bb_scale_fine = simple_strtol(argv[5], NULL, 10);
            case 5:
                tx_table->txvga = simple_strtol(argv[4], NULL, 10);
            case 4:
                tx_table->rxvga = simple_strtol(argv[3], NULL, 10);
            case 3:
                tx_table->bb_scale = simple_strtol(argv[2], NULL, 10);
            case 2:
                loop = simple_strtol(argv[1], NULL, 10);
                break;
            default:
                break;
        }

        ez_test(loop);
    }
    else if (!strcmp(argv[0], "lb_dc_cancel"))
    {
        lb_dc_cancel();
    }
    else if (!strcmp(argv[0], "pllk"))
    {
        mt_Panther_Init(&mt_handle);  // do pll calibration now
    }
#ifdef PANTHER_RFC_PAUSE_CMD
    else if (!strcmp(argv[0], "pause"))
    {
        int val = 0;

        if (argc >= 2)
        {
            val = simple_strtol(argv[1], NULL, 10);
        }

        panther_rfc_switch_pause(val);
    }
#endif
#if defined(RFC_DEBUG)
    else if (!strcmp(argv[0], "dbg_lvl"))
    {
        if (argc == 2)
            rfc_dbg_level = simple_strtol(argv[1], NULL, 10);

        serial_printf("rfc_dbg_level = %d\n", rfc_dbg_level);
    }
#if defined(RFC_TIME_MEASURE)
    else if (!strcmp(argv[0], "en_time_measure"))
    {
        if (argc == 2)
        {
            if (simple_strtol(argv[1], NULL, 10) >= 1)
                rfc_time_measure = RFC_DBG_ERR;
            else
                rfc_time_measure = 0;
        }

        serial_printf("rfc_time_measure = %d\n", rfc_time_measure);

        if (rfc_time_measure)
            old_panther_rfc_process();
    }
#endif
#endif
#if defined(RFC_ATE)
    else if (!strcmp(argv[0], "print_rfc_result"))
    {
        print_rfc_result();
    }
#endif

    return 1;
}

//cmdt cmdt_rfc __attribute__ ((section("cmdt"))) =
//    {"rfc", rfc_test_cmd, "rfc <cmd> <params>\n"};
#endif

//shell_cmd("rfc", "rfc <argv>; rfc test", "", rfc_test_cmd);
CLI_CMD(rfc, rfc_test_cmd, "rfc <argv>; rfc test");

#if defined(CONFIG_PANTHER_INTERNAL_DEBUGGER)
struct seq_file *psf;
static char buf[300];
    #define MAX_ARGV_NUM 8
static int rfc_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
    memset(buf, 0, 300);

    if (count > 0 && count < 299)
    {
        if (copy_from_user(buf, buffer, count))
            return -EFAULT;
        buf[count-1]='\0';
    }

    return count;
}

extern int get_args (const char *string, char *argvs[]);
static int rfc_show(struct seq_file *s, void *priv)
{
    int rc;
    int argc ;
    char *argv[MAX_ARGV_NUM] ;

    //sc->seq_file = s;
    psf = s;
    argc = get_args((const char *)buf, argv);
    rc = rfc_test_cmd(argc, argv);
    //sc->seq_file = NULL;
    psf = NULL;

    return 0;
}

static int rfc_open(struct inode *inode, struct file *file)
{
    int ret;

    ret = single_open(file, rfc_show, NULL);

    return ret;
}

static const struct file_operations rfc_fops = {
    .open       = rfc_open,
    .read       = seq_read,
    .write      = rfc_write,
    .llseek     = seq_lseek,
    .release    = single_release,
};

struct idb_command idb_rfc_test_cmd =
{
    .cmdline = "rfc",
    .help_msg = "rfc                         RF calibration command list", 
    .func = rfc_test_cmd,
};
#endif

int rfc_test_init(void)
{
#if defined(CONFIG_PANTHER_INTERNAL_DEBUGGER)
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0))
    struct proc_dir_entry *res;
#endif

    register_idb_command(&idb_rfc_test_cmd);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
    if (!proc_create("rfc", S_IWUSR | S_IRUGO, NULL, &rfc_fops))
        return -EIO;

#else
    res = create_proc_entry("rfc", S_IWUSR | S_IRUGO, NULL);
    if (!res)
        return -ENOMEM;

    res->proc_fops = &rfc_fops;
#endif
#endif

    return 0;
}

void rfc_test_exit(void)
{
#if defined(CONFIG_PANTHER_INTERNAL_DEBUGGER)
    unregister_idb_command(&idb_rfc_test_cmd);
    remove_proc_entry("rfc", NULL);
#endif
}

#if 1
void panther_rfc_preinit(void)
{
    // enable iq_swap, because the whole system need iq swap
    bb_register_write(0, 0x2, bb_register_read(0, 0x2) | 0x2);

    lrf_set_pll(2442);
    *(volatile unsigned int *)0xbf0048f8 = 0xb;
    rf_write(1, 0x400b10);
    WLAN_DBG("val = 0x%x\n", rf_read(1));
    rf_write(18, 0xa001108c);
    WLAN_DBG("val = 0x%x\n", rf_read(18));
}

void panther_bb_init(void)
{
    // enable iq_swap, because BB need iq swap
    //bb_register_write(0, 0x2, bb_register_read(0, 0x2) | 0x2);

    // because rxdc calibration will set bb fixed gain mode, so we need to recover it
    bb_register_write(0, 0x3, 0);

    // restore tx mux
    bb_register_write(0, 0x2f, 0);

    // restore bb21 bb25 bb30 to TXLO setting
    bb_register_write(0, 0x21, rfc_result_ptr[0].txlo_msb);
    bb_register_write(0, 0x25, rfc_result_ptr[0].tx_dc_i);
    bb_register_write(0, 0x30, rfc_result_ptr[0].tx_dc_q);

    RFC_DBG(RFC_DBG_INFO, "bb restore, reg 21: 0x%x, reg 25: 0x%x, reg 30: 0x%x\n", 
            bb_register_read(0, 0x21), bb_register_read(0, 0x25), bb_register_read(0, 0x30));
}

// to restore some rf register now
void set_normal_mode(void)
{
    *(volatile unsigned int *)0xbf0048f8 = 0x1;
    rf_update(0x0, 0x0, 0x80020800);
    rf_write(1, 0x400b10);
    rf_update(0x1, 0x80000000, 0x80000000);
    rf_write(8, 0xa80000);
    rf_write(18, 0xa001108c);
    set_txcal_txvga(0);

    // agc_table
#if 0
    bb_register_write(0x1, 0x00, 0x9c);
    bb_register_write(0x1, 0x01, 0x66);
    bb_register_write(0x1, 0x02, 0x30);
    bb_register_write(0x1, 0x03, 0x7d);
    bb_register_write(0x1, 0x04, 0x30);
    bb_register_write(0x1, 0x05, 0x42);
    bb_register_write(0x1, 0x06, 0x0f);
    bb_register_write(0x1, 0x07, 0x23);
    bb_register_write(0x1, 0x08, 0x66);
    bb_register_write(0x1, 0x09, 0x71);
    bb_register_write(0x1, 0x0a, 0x18);
    bb_register_write(0x1, 0x0b, 0xbd);
    bb_register_write(0x1, 0x0c, 0x0b);
    bb_register_write(0x1, 0x0d, 0x2f);
    bb_register_write(0x1, 0x0e, 0x70);
    bb_register_write(0x1, 0x0f, 0x04);
    bb_register_write(0x1, 0x10, 0x5b);
    bb_register_write(0x1, 0x11, 0x08);
#endif
}

extern int fem_en;
int panther_rfc_process(void)
{
    int lna = RXDC_HIGH;

    if(fem_en)
        lna = RXDC_MIDDLE;

    // set bb to 20Mhz mode
    bb_set_20mhz_mode(2442);

    panther_rfc_preinit();      // this funtction should not enable in release version

    ez_test(1);

    MACREG_UPDATE32(LMAC_CNTL, LMAC_CNTL_TSTART, LMAC_CNTL_TSTART);

//  rxdcoc_exe_and_verify(lna);
    main_dcoc_calibration(lna);

    MACREG_UPDATE32(LMAC_CNTL, 0, LMAC_CNTL_TSTART);

    panther_bb_init();

    set_normal_mode();

    return 0;
}
#endif
