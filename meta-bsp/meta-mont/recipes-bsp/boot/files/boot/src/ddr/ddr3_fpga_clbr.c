/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file ddr3_clbr.c
*   \brief DDR3 calibration
*   \author Montage
*/
/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <arch/chip.h>
#include <common.h>
/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
#define DATACASE 8
#define ADDRCASE 8
//#define printf(...)
/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/

/*!
 * function: ReadWriteCheck_Ok
 *
 *  \brief access memory and indicate current setting is valid or not
 *  \return 1:success 0:failed
 */
int ReadWriteCheck_Ok(void)
{
    int i, j;
    int rc = 1;
    int data[DATACASE];
    int addr[ADDRCASE];
    volatile unsigned int *w;

    //addrress data initialization
    addr[0] = 0xa00130f0;
    addr[1] = 0xa00ecf0c;
    addr[2] = 0xa02130f0;
    addr[3] = 0xa0decf0c;
    addr[4] = 0xa08130f0;
    addr[5] = 0xa07ecf0c;
    addr[6] = 0xa18130f0;
    addr[7] = 0xa27ecf0c;

    //write data initialization
    data[0] = 0x1ee1c33c;
    data[1] = 0x87782dd2;
    data[2] = 0x5aa5a55a;
    data[3] = 0xb44b0ff0;
    data[4] = 0xe11e3cc3;
    data[5] = 0x7887d22d;
    data[6] = 0xa55a5aa5;
    data[7] = 0x4bb4f00f;

    for (j = 0; j < ADDRCASE; j++)
    {
        w = (void *) addr[j];
        for (i = 0; i < DATACASE; i++)
        {
            *w = data[i];
            if (data[i] != *w)
            {
                rc = 0;
                j = ADDRCASE;
                break;
            }
        }
    }
    return rc;
}

#if defined(CONFIG_FPGA)
/*!
 * function: InitializeDDRController
 *
 *  \brief Initialize DDR Controller
 *  \return
 */
void InitializeDDRController(void)
{
    unsigned int unTmp;
    // Disable DDR Controller.
    //DDRREG(DDR_INITCTRL) = 0;

    //-- Parameter setting.
    // Set DDR type.
    DDRREG(DDR_DRAMTYPE) = DDR_LRGCL | DDR_DDR3 | (0x01 << 17);

    DDRREG(DDR_DRAMTYPE);

    idelay(1000);

    // Set rl_add([18:16]) for FPGA.
    DDRREG(DDR_CTRL0) = (7 << DDR_DQSSHFT) | (3 << DDR_ARLSHFT);
    DDRREG(DDR_CTRL0);
    idelay(1000);

    // Set tWTR_orig from 3 to 4 for FPGA.
    unTmp = DDRREG(DDR_PARA1);  // 0x6b010307
    unTmp &= 0xfffff0ff;
    unTmp |= (0x4 << DDR_WTRSHFT);

    // DDRREG(DDR_PARA1, unTmp );
    DDRREG(DDR_PARA1) = 0x6b010407;     // 0x6b010407
    DDRREG(DDR_PARA1);
    idelay(1000);

    //-- Reset DDR3 device.
    DDRREG(DDR_INITCTRL) = DDR_W500US;

    // flush CPU writebuffer and delay some time.
    DDRREG(DDR_INITCTRL);
    idelay(1000);

    // Release reset.
    DDRREG(DDR_INITCTRL) = 0;
    DDRREG(DDR_INITCTRL);
    idelay(1000);

    // Delay some time.
    DDRREG(DDR_INITCTRL);
    DDRREG(DDR_INITCTRL);
    DDRREG(DDR_INITCTRL);

    //-- Set MR2.
    // CAS Write Latency
    // CWL=5
    // CWL=6 (bit3)
    // CWL=7 (bit4)
    DDRREG(DDR_CTRLMODE) = (2 << DDR_BASHFT) | 0x10;    // CWL=7
    DDRREG(DDR_CTRLMODE);
    idelay(1000);
    DDRREG(DDR_CTRLMRS) = DDR_KCKMRS;
    DDRREG(DDR_CTRLMRS);
    idelay(1000);

    //-- Set MR3. No special setting.
    DDRREG(DDR_CTRLMODE) = (3 << DDR_BASHFT);
    DDRREG(DDR_CTRLMODE);
    idelay(1000);
    DDRREG(DDR_CTRLMRS) = DDR_KCKMRS;
    DDRREG(DDR_CTRLMRS);
    idelay(1000);

    //-- Set MR1. Enable DLL.
    // Enable DLL (bit0=0)
    // Rtt= Disabled.
    // Rtt= 60  Ohm. (bit2)
    // Rtt= 120 Ohm. (bit6)
    // Rtt= 40  Ohm. (bit6&bit2)
    // Rtt= 20  Ohm. (bit9)
    // Rtt= 30  Ohm. (bit9&bit2)
    DDRREG(DDR_CTRLMODE) = (1 << DDR_BASHFT) | 0x4;     // Rtt=60 Ohm
    DDRREG(DDR_CTRLMODE);
    idelay(1000);
    DDRREG(DDR_CTRLMRS) = DDR_KCKMRS;
    DDRREG(DDR_CTRLMRS);
    idelay(1000);

    //-- Set MR0. Set DLL reset, WR and CL
    DDRREG(DDR_WR_RGN) = (5 << DDR_TWRSHFT);    // Set tWR_orig = 5.
    DDRREG(DDR_WR_RGN);
    idelay(1000);
    // CAS Latency
    // CL=5 (bit4)
    // CL=6 (bit5)
    // CL=7 (bit6)
    // DLL reset (bit8)
    // Write Recovery
    // WR=16
    // WR=5 (bit9)
    // WR=6 (bit10)
    // WR=7 (bit10&bit9)
    DDRREG(DDR_CTRLMODE) = (0 << DDR_BASHFT) | 0x320;   // WR=5, CL=6.
    DDRREG(DDR_CTRLMODE);
    idelay(1000);
    DDRREG(DDR_CTRLMRS) = DDR_KCKMRS;
    DDRREG(DDR_CTRLMRS);
    idelay(1000);

    //-- Issue ZQCL command, bit[3] to start ZQ calibration
    DDRREG(DDR_SLFREF) = DDR_KCKZQCL;
    DDRREG(DDR_SLFREF);
    idelay(1000);

    // Delay some time.
    DDRREG(DDR_INITCTRL);
    DDRREG(DDR_INITCTRL);
    DDRREG(DDR_INITCTRL);
    idelay(10000);

    //-- Powerup the controller.
    DDRREG(DDR_INITCTRL) = DDR_PWRUP;
    DDRREG(DDR_INITCTRL);
    idelay(1000);
}

/*!
 * function: DelayReadClock1Step
 *
 *  \brief access check
 *  \return
 */
void DelayReadClock1Step(void)
{
#ifdef CONFIG_FPGA
    DDRREG(DDR_RDCLKDLY) = 1;
    //putchar(DDRREG(DDR_RDCLK) + 'A');
    //putchar(DDRREG(DDR_DCLK) + 'a');
    idelay(10000);
    DDRREG(DDR_RDCLKDLY) = 0;
    //putchar(DDRREG(DDR_RDCLK)+'A');
#endif
}

/*!
 * function: DelayDeviceClock1Step
 *
 *  \brief
 *  \return
 */
void DelayDeviceClock1Step(void)
{
#ifdef CONFIG_FPGA
    DDRREG(DDR_DCLKDLY) = 1;
    idelay(10000);
    DDRREG(DDR_DCLKDLY) = 0;
    //putchar(DDRREG(DDR_DCLK)+'A');
#endif
}

/*!
 * function: SetDeviceClockTo
 *
 *  \brief set Device clock delay
 *  \return
 */
void SetDeviceClockTo(int x)
{
    int i;
    for (i = 0; i < x; i++)
        DelayDeviceClock1Step();
}

/*!
 * function: SetReadClockTo
 *
 *  \brief set read clock delay
 *  \return
 */
void SetReadClockTo(int x)
{
    int i;
    for (i = 0; i < x; i++)
        DelayReadClock1Step();
}

/*!
 * function: ValidRange
 *
 *  \brief find a range and store by min and max
 *  \return
 */
void ValidRange(int ValidDeviceClockDelay, int *Min, int *Max)
{
    int match = 0, bit;
    *Max = *Min = 0;
    for (bit = 0; bit < 32; bit++)
    {
        if (ValidDeviceClockDelay & (1 << bit))
        {
            if (match)
            {
                *Max = bit;
            }
            else
            {
                match = 1;
                *Max = *Min = bit;
            }
        }
        else if (match)
            return;
    }
}

void ddr3_calibration_reset(void)
{
    while (DDRREG(DDR_DCLK) != 0)
    {
        DelayDeviceClock1Step();
    }

    while (DDRREG(DDR_RDCLK) != 0)
    {
        DelayReadClock1Step();
    }
}

/*!
 * function: ddr_calibration_c
 *
 *  \brief do ddr calibration (coding by c)
 *  \return
 */
void ddr3_calibration_c(void)
{
    putchar('C');
    // Step1: Find the valid delay for device clock.
    int ValidDeviceClockDelay, device_clock_delay, read_clock_delay, Min, Max,
        TargetDelay, ValidReadClockDelay;

    ddr3_calibration_reset();

    ValidDeviceClockDelay = 0;
    ValidReadClockDelay = 0;
    for (device_clock_delay = 0; device_clock_delay < 32; device_clock_delay++)
    {
        for (read_clock_delay = 0; read_clock_delay < 32; read_clock_delay++)
        {
            InitializeDDRController();
            if (ReadWriteCheck_Ok())
            {
                ValidDeviceClockDelay |= (1 << device_clock_delay);     // Bit No.X is 1 means delay step X is valid for device clock. 
            }
            //else if (read_clock_delay == 31)
            //{
            //    ValidDeviceClockDelay &= ~(1 << device_clock_delay);
            //    break;
            //}
            //else
            //{
            //read_clock_delay++;
            DelayReadClock1Step();
            //}
        }

        DelayDeviceClock1Step();
    }

    // Step2: Set the device clock delay to the middle point of the valid window.
    // Find out the valid window for device clock delay by check the continous 1s in ValidDeviceClockDelay.
    ValidRange(ValidDeviceClockDelay, &Min, &Max);

    TargetDelay = (Min + Max) / 2;

    putchar('D');
    putchar('A' + TargetDelay);
    // Set the device clock delay to the target value 
    SetDeviceClockTo(TargetDelay);

    // Step3: Check the valid range for read clock delay after device clock delay is set. 
    for (read_clock_delay = 0; read_clock_delay < 32; read_clock_delay++)
    {
        if (ReadWriteCheck_Ok())
        {
            ValidReadClockDelay |= (1 << read_clock_delay);     // Bit No.X is 1 means delay step X is valid for device clock. 
        }
        else
        {
            ValidReadClockDelay &= ~(1 << read_clock_delay);
        }

        DelayReadClock1Step();
    }

    // Step4: Set the read clock delay to the middle point of the valid window.
    // Find out the valid window for read clock delay by check the continous 1s in ValidReadClockDelay.
    ValidRange(ValidReadClockDelay, &Min, &Max);

    TargetDelay = (Min + Max) / 2;

    putchar('R');
    putchar('A' + TargetDelay);
    // Set the device clock delay to the target value 
    SetReadClockTo(TargetDelay);

    // Now calibration is done.
}

#else

void delay_ns(unsigned long ns)
{
    int _c;
    for (_c = 0; _c < ((ns / 100) + 1); _c++)
        *((volatile unsigned long *) 0xbf003800) = _c;
    //COSIM_REG_WRITE32(COSIM_WAIT_RELATE_TIME, ns);
}

void ddrreg_w32(unsigned long phy_addr, unsigned long data)
{
    *((volatile unsigned long *) (0xa0000000 | phy_addr)) = data;
}

void InitializeDDRController_asic(void)
{
//DDR initial start @time = 104019370.0 ps
    delay_ns(124021);
    ddrreg_w32(0x1f000820, 0x00000404); // 124021170.0 ps
    delay_ns(811);
    ddrreg_w32(0x1f000848, 0x04040404); // 124832890.0 ps
    delay_ns(811);
    ddrreg_w32(0x1f000804, 0x00000020); // 125644610.0 ps
    delay_ns(811);
    ddrreg_w32(0x1f000804, 0x000000ef); // 126456330.0 ps
    delay_ns(811);
    ddrreg_w32(0x1f00082c, 0x00000001); // 127268050.0 ps
    delay_ns(811);
    ddrreg_w32(0x1f00084c, 0x00020000); // 128079770.0 ps
    delay_ns(811);
    ddrreg_w32(0x1f000808, 0xc040c040); // 128891490.0 ps
    delay_ns(811);
    ddrreg_w32(0x1f00080c, 0xc040c040); // 129703210.0 ps
    delay_ns(811);
    ddrreg_w32(0x1f000810, 0x00400040); // 130514930.0 ps
    delay_ns(811);
    ddrreg_w32(0x1f00084c, 0x00020080); // 131326650.0 ps
    delay_ns(811);
    ddrreg_w32(0x1f000814, 0x0000078f); // 132138370.0 ps
    delay_ns(811);
    ddrreg_w32(0x1f000818, 0x18181818); // 132950090.0 ps
    delay_ns(811);
    ddrreg_w32(0x1f00081c, 0x18181818); // 133761810.0 ps
    delay_ns(811);
    ddrreg_w32(0x1f000038, 0x70030000);
    // 134573530.0 ps
    delay_ns(811);
    ddrreg_w32(0x1f000034, 0x00020031); // 135385250.0 ps
    delay_ns(811);
    ddrreg_w32(0x1f000204, 0x00000211); // 136196970.0 ps
    delay_ns(811);
    ddrreg_w32(0x1f000018, 0x00010000); // 137008690.0 ps
    delay_ns(1311);
    ddrreg_w32(0x1f000018, 0x00000000); // 138320410.0 ps
    delay_ns(10810);
    ddrreg_w32(0x1f000020, 0x00008008); // 149130810.0 ps
    delay_ns(810);
    ddrreg_w32(0x1f000004, 0x00000001);
    // 149941610.0 ps
    delay_ns(811);
    ddrreg_w32(0x1f000020, 0x0000c000);

    // 150753330.0 ps
    delay_ns(811);
    ddrreg_w32(0x1f000004, 0x00000001); // 151565050.0 ps
    delay_ns(811);
    ddrreg_w32(0x1f000020, 0x00004004); // 152376770.0 ps
    delay_ns(811);
    ddrreg_w32(0x1f000004, 0x00000001); // 153188490.0 ps
    delay_ns(811);
    ddrreg_w32(0x1f000020, 0x00000940); // 154000210.0 ps
    delay_ns(811);
    ddrreg_w32(0x1f00005c, 0x00080000); // 154811930.0 ps
    delay_ns(811);
    ddrreg_w32(0x1f000004, 0x00000001); // 155623650.0 ps
    delay_ns(811);
    ddrreg_w32(0x1f00000c, 0x19000008); // 156435370.0 ps
    delay_ns(811);
    ddrreg_w32(0x1f000024, 0x02081408); // 157247090.0 ps
    delay_ns(811);
    ddrreg_w32(0x1f000028, 0xba010506); // 158058810.0 ps
    delay_ns(811);
    ddrreg_w32(0x1f00002c, 0x14041000);
    // 158870530.0 ps
    delay_ns(4539);
    ddrreg_w32(0x1f000018, 0x00000001);

    // 163409610.0 ps
//DDR initial done @time = 164224970.0 ps
    delay_ns(1000);
}

#endif
