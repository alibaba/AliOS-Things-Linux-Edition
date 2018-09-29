/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file serial.c
*   \brief UART Driver
*   \author Montage
*/

#include <arch/chip.h>
#include <common.h>
#include <lib.h>

unsigned int save_char;
#define wait_uart_delay() {int _www; for (_www=0; _www< 10000; _www++); }
#ifdef RTS_CTS_TEST
char rts_cts_test;
#endif
/*!
 * function:
 *
 *  \brief
 *  \return
 */
void cheetah_init(void)
{
    /* enable RX */
    URREG(URCS) |= URCS_REN;

//    /* enable RX and assign baudrate */
//    URREG(URCS) = URCS_REN | ((CONFIG_SYS_CLK/CONFIG_CONSOLE_BAUD)<<URCS_BRSHFT);
//
//    /* to wait UART ready */
//    wait_uart_delay();
}

#if !defined(BOOT_MODE_BOOT1)
/*!
 * function:
 *
 *  \brief
 *  \param uartclk
 *  \return
 */

void cheetah_uart_reinit(unsigned int uartclk)
{
    URREG(URCS) = (uartclk / CONFIG_CONSOLE_BAUD) << URCS_BRSHFT;

    /* to wait UART ready */
    wait_uart_delay();
}
#endif

/*!
 * function:
 *
 *  \brief
 *  \param c
 *  \return
 */
#ifdef CONFIG_UART_EN_DET
extern int boot_detect_flag;
#endif
void cheetah_putc(char c)
{
#ifdef CONFIG_UART_EN_DET
    if (boot_detect_flag == 0)
        return;
#endif
    while (URREG(URCS) & URCS_TB) ;     //1: tx busy
    URREG(URBR) = c << URBR_DTSHFT;
}

/*!
 * function:
 *
 *  \brief
 *  \return
 */

int cheetah_getc(void)
{
#ifdef RTS_CTS_TEST
    if(rts_cts_test)
        return 0;
#endif
    if (!(save_char & URBR_RDY))
    {
#if defined(CONFIG_TELNETD) && !defined(BOOT_MODE_BOOT1)
        while (!((save_char = URREG(URBR)) & URBR_RDY))
        {
            unsigned char ch;
            if(telnetd_input)
            {
                if((*telnetd_input) (&ch))
                    return (int) ch;
            }
        }
#else
        while (!((save_char = URREG(URBR)) & URBR_RDY)) ;
#endif
    }

    TMREG(T2CR) = 0;            //reset wd for manual input

    save_char >>= URBR_DTSHFT;
    return save_char;
}

/*!
 * function:
 *
 *  \brief
 *  \return
 */

int cheetah_uart_poll(void)
{
    // already detect, just return
    if (save_char & URBR_RDY)
        return 1;
    // else test hw
    save_char = URREG(URBR);
    if (save_char & URBR_RDY)
    {
        return 1;
    }
    else
    {
#if defined(CONFIG_TELNETD) && !defined(BOOT_MODE_BOOT1)
        if(telnetd_poll)
            if((*telnetd_poll)())
                return 1;
#endif
        return 0;
    }
}

/*!
 * function:
 *
 *  \brief
 *  \param m
 *  \return
 */

void cheetah_uart_loop(int m)
{
    if (m)
        URREG(URCS) |= URCS_LB;
    else
        URREG(URCS) &= ~URCS_LB;
}

/*!
 * function:
 *
 *  \brief
 *  \return
 */

void serial_init(void)
{
    cheetah_init();
}

/*!
 * function:
 *
 *  \brief
 *  \return
 */

int serial_getc(void)
{
    return cheetah_getc();
}

/*!
 * function:
 *
 *  \brief
 *  \param c
 *  \return
 */

void serial_putc(int c)
{
    cheetah_putc(c);
    return;
}

#if !defined(BOOT_MODE_BOOT1)
/*!
 * function:
 *
 *  \brief
 *  \param m
 *  \return
 */

void serial_loopback(int m)
{
    cheetah_uart_loop(m);
}
#endif

/*!
 * function:
 *
 *  \brief
 *  \return
 */

int serial_poll(void)
{
#ifdef RTS_CTS_TEST
    if(rts_cts_test)
        return 0;
#endif

#ifdef CONFIG_UART_EN_DET
    if (boot_detect_flag == 0)
        return 0;
#endif
    return cheetah_uart_poll();
}

#ifdef  CONFIG_CMD_UART

/*!
 * function:
 *
 *  \brief
 *  \param argc
 *  \param argv
 *  \return
 */
int serial_cmd(int argc, char *argv[])
{
    int pass = 0, result = 0, id = 0, count = 100;
    int i, j, t, r;

    if (0 < argc)
    {
        if (!strcmp(argv[0], "init"))
        {
            serial_init();
            return 0;
        }
        if (!hextoul(argv[1], &count))
            goto err1;
    }

    printf("UART%d loopback %d bytes:\n", CONFIG_UART_PORT + 1, count);

    mdelay(10);

    serial_loopback(1);

    for (i = 0; i < count; i++)
    {
        t = '0' + (i % 0x4f);
        serial_putc(t);
        for (j = 1000; j; j--)
        {
            if (!serial_poll())
                continue;
            r = serial_getc();
            if ((t & 0xff) == (r & 0xff))
                pass++;
            break;
        }
    }
    mdelay(10);
    serial_loopback(0);

    if (pass < count)
    {
        printf("Fail count=%d\n", count - pass);
        result = -1;
    }
    return result;

  err1:
    return ERR_PARM;
  help:
    return ERR_HELP;
}

cmdt cmdt_serial __attribute__ ((section("cmdt"))) =
{
"serial", serial_cmd, "serial [init/count]"};
#endif                          //CONFIG_CMD_UART

#if (defined(CONFIG_CMD_XMODEM) && !defined(BOOT_MODE_BOOT1))

#define CONFIG_XMODEM
#include <src/crc16.c>
#include <src/xmodem.c>
#include <netprot.h>            //boot cdb structure

#ifndef CONFIG_UART_DL_TO
#define CONFIG_UART_DL_TO       30000   // 1st byte timeout
#define CONFIG_UART_DL_BYTE_TO  2000    // next byte timeout
#endif

/*!
 * function:
 *
 *  \brief
 *  \param argc
 *  \param argv
 *  \return
 */

int serial_dl_cmd(int argc, char *argv[])
{
    unsigned long i, j;
    char c = argv[-1][1];
    unsigned int time, to;
    unsigned int buf = bootvars.load_addr;
    int len = 0x0fffffff;

    if (argc > 0 && !hextoul(argv[0], &buf))
        goto err2;
    if (vaddr_check(buf))
        goto err2;
    if (argc > 1 && !hextoul(argv[1], &len))
        goto err2;

    printf("buf=%08x size=%08x\n", buf, len);

#ifdef  CONFIG_XMODEM
    if (c != 0)                 // xmodem
    {
        printf("XMODEM..%c\n", c);
        switch (c)
        {
            case 't':          // tx
                len = xmodem_tx((unsigned char *) buf, len);
                break;
            case 'r':
                len = xmodem_rx((unsigned char *) buf, len);
                break;
            default:
                goto err2;
        }
        printf("\nsize: %x", len);
        if (len >= 0)
        {
            byte_count = len;
            goto done;
        }
        else
            goto err3;
    }
#endif                          //CONFIG_XMODEM

    printf("Flush buffer! ");
    while (serial_poll())
        serial_getc();
    printf("start..\n");

    to = CONFIG_UART_DL_TO;
    for (i = 0, j = 0; i < len; i++)
    {
        time = clock_get();
        while (!serial_poll())
        {
            if (how_long(time) > to)
            {
                printf("rx time-out! count=%08d\n", i);
                goto err3;
            }
        }
        c = serial_getc();
        *(((volatile unsigned char *) buf) + i) = c;
        j++;
        if (j == 4096)
        {
            printf(".");
            j = 0;
        }
        to = CONFIG_UART_DL_BYTE_TO;
    }

    printf("\n");
    printf("Finished !\n");

  done:
    //update default value in mem_dump_cmd()
    buf_address = buf;
    return ERR_OK;
  err2:
    return ERR_ADDR;
  err3:
    return ERR_TIMEOUT;
}

cmdt cmdt_xmodem_dl __attribute__ ((section("cmdt"))) =
{
"x", serial_dl_cmd, "x<r,t> <buf> <len> ;serial download"};

#endif                          //CONFIG_CMD_XMODEM && !BOOT_MODE_BOOT1
