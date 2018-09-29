/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file config.h
*   \brief Configuration file
*   \author Montage
*/

#ifndef CONFIG_H
#define CONFIG_H
/*=============================================================================+
| Main Functions                                                               |
+=============================================================================*/
// ethernet
#define CONFIG_ETH_CALIBRATION
//#define CONFIG_P0_AS_LAN
#define CONFIG_P0_AS_ADDR2
#define CONFIG_P0_EXT_PHY
//#define CONFIG_P0_RMII
//#define CONFIG_P0_RMII_CLKIN
//#define CONFIG_P0_RMII_CLKOUT
#define CONFIG_P1_EXT_PHY
#define CONFIG_P1_RMII
//#define CONFIG_P1_RMII_CLKIN
#define CONFIG_P1_RMII_CLKOUT
//#define CONFIG_ETH_POLLING
#define CONFIG_ETH_PAD64
// usb
//#define CONFIG_USB
//#define CONFIG_USB_LOOPBACK_DEV
//#define CONFIG_USB_LOOPBACK_TEST
//#define CONFIG_VBUS_DETECTION
// others
#define CONFIG_CPU_COUNT
#define CONFIG_CLOCK_NOISR
#ifdef CONFIG_PCM_I2S
#define CONFIG_AI
#define CONFIG_GPIO_I2C
#else
//#define CONFIG_GPIO_I2C
#endif

/*=============================================================================+
| Miscellaneous Functions                                                      |
+=============================================================================*/
#define CONFIG_EXAM_FIRM_BEFORE_PROGRAM
#define CONFIG_UBIMAGE
//#define CONFIG_GDB
//#define CONFIG_GPIO_SWRST 0
//#define CONFIG_GPIO_SPI
//#define CONFIG_PUSHTIME_TO_RSTCDB 6000 //6s
//#define CONFIG_BOOT_JUMP_GPIO 12
//#define CONFIG_BOOT_JUMP_CHAR 'z'
//#define CONFIG_BOOT_JUMP_ADDR (CONFIG_FLASH_BASE+0x20000)
//#define CONFIG_PINMUX (EN_FUNCMODE)
//#define CONFIG_LED
#define CONFIG_SHOW_PROMPT_CHARS_IN_BOOT1
#define CONFIG_SIMPLE_CMD_WEBPAGE
#define CONFIG_TELNETD
//#define CONFIG_OPTIMIZED_MEMCPY
//#define CONFIG_DDRAM_SIZE 2 //0:c8r12 1:c9r12 2:c9r13 3:c10r13
//#define CONFIG_KEYWORD_TO_STOP "\r\r\r"
#define CONFIG_FLASH_CMD_PROGRAM_AFTER_DOWNLOAD_FILE    //make downloading file as the premise of programing flash
#if defined(TEST_MODE_BOARD)
#define CONFIG_UART_PORT 2
#else
#define CONFIG_UART_PORT 0
#endif
//#define CONFIG_UART_EN_DET //UART tx enable detection
#define GPIO_UART_DET_NUM 25
#define CONFIG_MP_TEST
//#define CONFIG_OVERCLOCKING_FOR_APP
//#define CONFIG_SD_RECOVERY

/*=============================================================================+
| Configuration about bootloader command                                       |
+=============================================================================*/
#if !defined(WLA_TEST)
#define CONFIG_PROMPT "cmd>"
#else
#define CONFIG_PROMPT "CLI>" //match mp test prefix
#endif
#define CONFIG_CLI_HISTORY
#define CONFIG_CMD
#define CONFIG_CMD_MEM
#define CONFIG_CMD_TFTP
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_MDIO
//#define CONFIG_CMD_UART
//#define CONFIG_CMD_CLOCK
#define CONFIG_CMD_ET
#define CONFIG_CMD_MT
#define CONFIG_CMD_MT2
//#define CONFIG_CMD_DHRYSTONE
//#define CONFIG_CMD_CACHE
#define CONFIG_CMD_XMODEM
//#define CONFIG_CMD_BB
//#define CONFIG_CMD_EXMII
//#define CONFIG_CMD_BR
//#define CONFIG_CMD_CS3_SELECT
//#define CONFIG_CMD_SWITCH
//#define CONFIG_CMD_USB_MODE
#define CONFIG_CMD_OTP
#define CONFIG_CMD_SECURE_MODE
#define CONFIG_CMD_INIT

/*=============================================================================+
| Default value of bootloader variable                                         |
+=============================================================================*/
#define IP_ADDR(a,b,c,d)    (a<<24)|(b<<16)|(c<<8)|(d)

//#define CONFIG_BOOT_VER     BOOT_REVNUM
#define CONFIG_BOOT_IP      IP_ADDR(192,168,0,1)
#define CONFIG_BOOT_SERVER  IP_ADDR(192,168,0,2)
#define CONFIG_BOOT_GW      IP_ADDR(192,168,0,2)
#define CONFIG_BOOT_MSK     IP_ADDR(255,255,255,0)
#define CONFIG_BOOT_MAC0    "\x80\x50\xDF\x00\x00\x01"
#define CONFIG_BOOT_MAC1    "\x80\x50\xDF\x00\x00\x02"
#define CONFIG_BOOT_MAC2    "\x80\x50\xDF\x00\x00\x03"
#define CONFIG_BOOT_FILE    "le"
#define CONFIG_BOOT_MODE    0   //0: cmd, 1: flash, 2: net
#define CONFIG_BOOT_SN      12345
#ifdef CONFIG_I2CS
#define CONFIG_BOOT_HWVER   IP_ADDR(0,0x85,0,0)
#else
#define CONFIG_BOOT_HWVER   IP_ADDR(0,0x84,0,0)
#endif
   //bit30    TIMER   wdog enable
   //bit29,28 DRVING  0:8mA,12mA(dram) 1:4mA 2:16mA 3:8mA
   //bit23-20 CPU CLK 0:60.95 1:64 2:80 3:120 4:160 5:202.11 6:240 7:295.38 8:320 MHz
   //bit19-16 SYS CLK 0:60.95 1:64 2:80 3:120 4:150.59 5:160 MHz
//#define CONFIG_BOOT_CVER
#define CONFIG_BOOT_PLL     0x03c00688
   //When the bit23-16 of hwver are both 1, use this configuration value
   //Frequency (MHz) = 0x1E000 / prediv / postdiv[ 1, 2, 3, 4, 2, 4, 6, 8 ]
   //bit26-24 CPU postdiv
   //bit23-16 CPU prediv
   //bit10-8  SYS postdiv
   //bit7-0   SYS prediv

#define CONFIG_BOOT_DL_ADDR     0x81000000      // 16m
#define CONFIG_BOOT_LOAD_SZ     0x400000        // 4m
#define CONFIG_FLASH_SYSLOG_SZ  0x20000 // 128k

#define CONFIG_BOOT_MPTEST_OFS  0x760000
#define CONFIG_FLASH_SYSLOG_OFS 0x1e0000
#define CONFIG_BOOT_TXVGA		"00000000000000"
#define CONFIG_BOOT_RXVGA		""
#define CONFIG_BOOT_TXPDIFF		"406"
#define CONFIG_BOOT_FEM_PRODUCT_ID		"0"
#define CONFIG_BOOT_FEM_EN      1
#define CONFIG_BOOT_PRODUCT     0
#define CONFIG_BOOT_PINMUX      ""
#define CONFIG_BOOT_DRIV_STR    ""
#define CONFIG_BOOT_GPIO_SETTING   ""
#define CONFIG_BOOT_POWERCFG    0x0FFFF
#define CONFIG_BOOT_CLKCFG      0x0
#define CONFIG_BOOT_WATCHDOG_TIMER 20000 // 20sec
#define CONFIG_BOOT_MIC_GAIN_CTRL_STR    "7777"
   //bit15-12 PRODUCT 0:AP/Router   1:WiFi Audio 2:IPCAM          3:IoT Gateway         4:IoT Sensors 
   //                 5:Storage/NAS 6:STB/TV     7:Home Appliance 8:Media Server/Player 9~15:Reserved
#define CONFIG_BOOT_ID      (((CONFIG_BOOT_PRODUCT & 0xf)<<12) | 0)
#define CONFIG_BOOT_PIN     ""
#define CONFIG_BOOT_FREQ_OFS     16
#define CONFIG_BOOT_MADC_VAL0 ""
#define CONFIG_BOOT_MADC_VAL1 ""
#define CONFIG_BOOT_LNA   	""
#ifdef CONFIG_FPGA
#define CONFIG_BOOT_SWCFG   "0x01002021,0x02003022"
#else
#define CONFIG_BOOT_SWCFG   "0x0,0x02003032"
#endif
#define CONFIG_BOOT_SPDIF_EN    1
#define CONFIG_BOOT_AUDIO_INTERFACE "i2s"
#define CONFIG_BOOT_RECOVERY 0
#define CONFIG_BOOT_RECOVERY_OFFSET   0x40000 // default recovery       (256k/block 2)
                                              // default cdb [0x80000]  (512k/block 4)
#define CONFIG_SECOND_IMAGE_OFFSET    0x40000 // temp. it should be changed on the basis of image size
#define CONFIG_BOOT_CI_OFFSET         0xa0000 // default combined-image (640k/block 5)
/*=============================================================================+
| Constant                                                                     |
+=============================================================================*/
// clock
#define CONFIG_REF_CLK 40000000
#ifdef CONFIG_FPGA
#define CONFIG_SYS_CLK 40000000
#define CONFIG_SFI_CLK_DIV 4    // system clk/n
#else
#define CONFIG_SYS_CLK 150590000
#define CONFIG_SFI_CLK_DIV 4    // system clk/n
#endif
// cpu exception vector
#define CONFIG_TLB_EXCEPTION_VECTOR 0x80000000
#define CONFIG_GENERAL_EXCEPTION_VECTOR (CONFIG_TLB_EXCEPTION_VECTOR+0x180)
// net
#define CONFIG_NETBUF_NUM   32
#define CONFIG_NETBUF_SZ    0x800
// uart
#define CONFIG_CONSOLE_BAUD 115200
// timer
#define CONFIG_SYS_HZ 1000
#define CONFIG_SYS_CLK_PR 16
// tsi
//#define CONFIG_TSI
//#define CONFIG_TSI_TEST
// layout
#define CONFIG_STACK_SZ      0x10000     //boot2 stack size
#define CONFIG_FLASH_BASE       0xb8000000
#define CONFIG_UNDIR_FLASH_BASE 0x81000000
#define CONFIG_BOOT_UNC_OFS  0x6000
#define CONFIG_BOOT_UNC_SZ   0xa000
#define CONFIG_BOOT_PARM_OFS 0x2000
#define CONFIG_BOOT_PARM_SZ  0x4000
#define CONFIG_BOOT_UNC_LOAD (B1_TEXT+CONFIG_BOOT_UNC_OFS)


#if defined(MPCFG)
/* change default bootvars for mass production */
#undef CONFIG_BOOT_MODE
#undef CONFIG_BOOT_RECOVERY

#define CONFIG_BOOT_MODE        1
#define CONFIG_BOOT_RECOVERY    1
#endif

#if defined(CONFIG_ATE)
#undef CONFIG_BOOT_FEM_EN
#define CONFIG_BOOT_FEM_EN      0
#endif

#endif                          // CONFIG_H
