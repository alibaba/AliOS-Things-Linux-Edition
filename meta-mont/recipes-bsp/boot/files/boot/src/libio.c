/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file libio.c
*   \brief Base IO functions
*   \author Montage
*/

#include <arch/chip.h>
#include <stdarg.h>
#include <common.h>
#include <netprot.h>
#include <lib.h>

int putchar(int cc);

#define isprint(c)  (c!=0)
#define isascii(c)  !(c&0x80)
#define isnum(c)    (c>=48 && c<=57)
#define GETAP(ap) \
    ap = (char *) va_arg(args,char *);

#define     MAXPFLEN    (1<<10)
#define     COUNT   count++; if(count == MAXPFLEN-1) return(count);

#ifndef NULL
#define NULL            0
#endif

int (*io_redirect) (char *) = 0;
#if defined(CONFIG_TELNETD)
int (*telnetd_input) (unsigned char *) = 0;
int (*telnetd_output) (char *) = 0;
int (*telnetd_poll) (void) = 0;
#endif
int printf(char *fmt, ...);
void (*hook_function_p) (void) = NULL;

enum
{
    NUL = 0x00,
    ETX = 0x03,
    BS = 0x08,
    TAB = 0x09,
    LF = 0x0a,
    CR = 0x0d,
    //ESC = 0x1b,
    SPACE = 0X20,
    UP = 0x41,
    DOWN = 0x42,
    RIGHT = 0x43,
    LEFT = 0x44,
    DEL = 0x7f,
};

enum
{
    STATE_ESC_NONE = 0,
    STATE_ESC_START = 1,
    STATE_ESC_CODE = 2,
};

static const char erasetoeol[] = { ESC, '[', 'K', NUL };        //<ESC>[K<NULL>
static const char eraseback[] = { BS, SPACE, BS, NUL }; //erase backspace

/*!
 * function:
 *
 *  \brief
 *  \param fmt
 *  \param cc
 *  \return
 */

static int match(char *fmt, char cc)
{
    int exc = 0;
    char *cp1;

    if (!cc)
        return 0;
    if (*fmt == '^')
    {
        exc = 1;
        fmt++;
    }
    for (cp1 = fmt; *cp1 && *cp1 != ']';)
    {
        if (cp1[1] == '-' && cp1[2] > cp1[0])
        {
            if (cc >= cp1[0] && cc <= cp1[2])
                return exc ^ 1;
            cp1 += 3;
        }
        else
        {
            if (cc == *cp1)
                return exc ^ 1;
            cp1++;
        }
    }
    return exc;
}

/*!
 * function:
 *
 *  \brief
 *  \param cp
 *  \param width
 *  \param base
 *  \return
 */

static long asclng(char **cp, int width, int base)
{
    long answer;
    char cc, sign, *cp1;

    answer = sign = 0;
    for (cp1 = *cp; *cp1; cp1++)
    {
        if (*cp1 > ' ')
            break;
    }
    if (*cp1 == '-' || *cp1 == '+')
        sign = *cp1++, width--;
    if (!*cp1 || !width)
        goto exi1;
    if (!base)
    {
        base = 10;
        if (*cp1 == '0')
        {
            base = 8;
            goto lab4;
        }
    }
    else if (base == 16)
    {
        if (*cp1 == '0')
        {
          lab4:cp1++, width--;
            if (width > 0)
            {
                if (*cp1 == 'x' || *cp1 == 'X')
                    base = 16, cp1++, width--;
            }
        }
    }
    for (; width && *cp1; cp1++, width--)
    {
        if ((cc = *cp1) < '0')
            break;
        if (cc <= '9')
            cc &= 0xf;
        else
        {
            cc &= 0xdf;
            if (cc >= 'A')
                cc -= 'A' - 10;
        }
        if (cc >= base)
            break;
        answer = base * answer + cc;
    }
  exi1:
    *cp = cp1;
    if (sign == '-')
        answer = -answer;
    return answer;
}

/*!
 * function:
 *
 *  \brief
 *  \param buf
 *  \param fmt
 *  \return
 */

int sscanf(char *buf, char *fmt, ...)
{
    int field;                  /* field flag: 0=background, 1=%field */
    int sizdef;                 /* size: 0=default, 1=short, 2=long, 3=long double */
    int width;                  /* field width */
    int par1;
    long l1;
    int nfields;
    char fch;
    char *orgbuf, *prebuf;
    char *ap;
    va_list args;

    if (!*buf)
        return -1;

    va_start(args, fmt);        /* get variable arg list address */
    nfields = field = sizdef = 0;
    orgbuf = buf;
    while ((fch = *fmt++) != 0)
    {
        if (!field)
        {
            if (fch == '%')
            {
                if (*fmt != '%')
                {
                    field = 1;
                    continue;
                }
                fch = *fmt++;
            }
            if (fch <= ' ')
                for (; *buf == ' ' || *buf == '\t'; buf++) ;
            else if (fch == *buf)
                buf++;
        }
        else
        {
            width = 0x7fff;
            if (fch == '*')
            {
                width = va_arg(args, int);
                goto lab6;
            }
            else if (fch >= '0' && fch <= '9')
            {
                fmt--;
                width = asclng(&fmt, 9, 10);
              lab6:fch = *fmt++;
            }
            if (fch == 'h')
            {
                sizdef = 1;
                goto lab7;
            }
            else if (fch == 'l')
            {
                sizdef = 2;
              lab7:fch = *fmt++;
            }
            prebuf = buf;
            switch (fch)
            {
            case 'd':           /* signed integer */
                par1 = 10;
                goto lab3;
            case 'o':           /* signed integer */
                par1 = 8;
                goto lab3;
            case 'x':           /* signed integer */
            case 'X':           /* long signed integer */
                par1 = 16;
                goto lab3;
            case 'u':           /* unsigned integer */
            case 'i':           /* signed integer */
                par1 = 0;
              lab3:GETAP(ap);
                l1 = asclng(&buf, width, par1);
                if (prebuf == buf)
                    break;
                if (sizdef == 2)
                    *(long *) ap = l1;
                else if (sizdef == 1)
                    *(short *) ap = l1;
                else
                    *(int *) ap = l1;
                goto lab12;
            case 'c':           /* character */
                GETAP(ap);
                for (; width && *buf; width--)
                {
                    *ap++ = *buf++;
                    if (width == 0x7fff)
                        break;
                }
                goto lab12;
            case '[':           /* search set */
                GETAP(ap);
                for (; width && match(fmt, *buf); width--)
                    *ap++ = *buf++;
                while (*fmt++ != ']') ;
                goto lab11;
            case 's':           /* character array */
                GETAP(ap);
                for (; *buf == ' ' || *buf == 0x07; buf++) ;
                for (; width && *buf && *buf > ' '; width--)
                    *ap++ = *buf++;
              lab11:if (prebuf == buf)
                    break;
                *(char *) ap = 0;
                goto lab12;
            case 'n':           /* store # chars */
                GETAP(ap);
                *(int *) ap = buf - orgbuf;
                break;
            case 'p':           /* pointer */
                GETAP(ap);
                *(long *) ap = asclng(&buf, width, 16);
              lab12:nfields++;
                break;
            default:            /* illegal */
                goto term;
            }
            field = 0;
        }
        if (!fch)
            break;
    }
  term:
    va_end(args);
    return nfields;
}

/*!
 * function:
 *
 *  \brief
 *  \param buf
 *  \param p_cmd
 *  \return
 */
static unsigned char bufidx = 0;
int cli_gets(char *buf, str_chain * p_cmd, char peek)
{
    char key;
    char *bp = &buf[bufidx];
    short state = STATE_ESC_NONE;

    do
    {
        if (!tstc())
        {
            if (bootvars.network)
            {
                await_reply(7, 0, 0);
            }
            if (hook_function_p)
            {
                (*hook_function_p) ();
                hook_function_p = NULL;
            }
            if(peek)
                return ERR_LAST;
            continue;
        }
        key = getchar();

        if(peek)
        {
            switch (key)
            {
                case LF:
                case CR:
                    putchar(key);
                    goto out;
                case BS:
                case DEL:
                    if (bp > buf)
                    {
                        bp--;
                        if(bufidx > 0)
                            bufidx--;
                        printf("%s", eraseback);
                    }
                    return ERR_LAST;
                default:
                    *bp++ = key;
                    bufidx++;
                    putchar(key);
                    return ERR_LAST;
            }
        }
        else
        {
            switch (key)
            {
                case ETX:
                    continue;
                case NUL:
                    continue;
                case ESC:
                    state = STATE_ESC_START;
                    continue;
                case LF:
                case CR:
                    putchar(key);
                    goto out;
                case TAB:
                    while (bp > buf)
                    {
                        bp--;
                        putchar(BS);
                    }
                    printf("%s", erasetoeol);
                    continue;
                case BS:
                case DEL:
                    if (bp > buf)
                    {
                        bp--;
                        printf("%s", eraseback);
                    }
                    continue;
                case '[':
    #ifdef  CONFIG_CLI_HISTORY
                case UP:
                case DOWN:
                    if (state == STATE_ESC_CODE)    //for 'A' and 'B'
                    {
                        state = STATE_ESC_NONE;
                        if (key == UP && p_cmd->prev->buffer[0] != NUL)
                            p_cmd = p_cmd->prev;
                        else if (key == DOWN && p_cmd->next->buffer[0] != NUL)
                            p_cmd = p_cmd->next;
                        else
                            continue;
                        while (bp > buf)
                        {
                            bp--;
                            putchar(BS);
                        }
                        printf("%s", erasetoeol);
                        printf(p_cmd->buffer);
                        strcpy(bp, p_cmd->buffer);
                        while (*bp)
                            bp++;
                        continue;
                    }
    #endif
                    if (state == STATE_ESC_START)   //for '['
                    {
                        state = STATE_ESC_CODE;
                        continue;
                    }
                default:
                    if (state == STATE_ESC_START)
                    {
                        state = STATE_ESC_NONE;
                    }
                    else if (state == STATE_ESC_CODE)
                    {
                        if (isnum(key))
                            state = STATE_ESC_CODE;
                        else
                            state = STATE_ESC_NONE;
                    }
                    else
                    {
                        *bp++ = key;
                        putchar(key);
                    }
                    break;
            }
        }
    }
    while ((bp - buf) < (249));
  out:
//double NUL avoid that a remnant to be see an argument
    *bp++ = NUL;
    *bp-- = NUL;
    bufidx = 0;
    return ERR_OK;//(int) (bp - buf);
}

/*!
 * function:
 *
 *  \brief
 *  \param buf
 *  \return
 */
int gets(char *buf)
{
    char cc;
    char *bp;
    bp = buf;
    *bp = 0;
    do
    {
        if (!tstc())
        {
            //await_reply(7, 0, 0);
            continue;
        }
        cc = getchar();
        putchar(cc);
        if ((cc == LF) | (cc == CR))
            break;
        if ((cc == BS || cc == DEL) & (bp > buf))
        {
            bp--;
        }
        else if (isprint(cc))
        {
            *bp = cc;
            bp++;
        }
    }
    while ((bp - buf) < (250));
    *bp = 0;
    return (int) (bp - buf);
}

/*!
 * function:
 *
 *  \brief
 *  \param string
 *  \param argv
 *  \return
 */

int get_args(const char *string, char *argv[])
{
    char *p;
    int n;

    n = 0;
    p = (char *) string;
    while (*p == ' ')
        p++;
    while (*p)
    {
        argv[n] = p;
        while (*p != ' ' && *p)
            p++;
        *p++ = '\0';
        while (*p == ' ' && *p)
            p++;
        n++;
        if (n == MAX_ARGV)
            break;
    }
    return n;
}

extern int (*io_redirect) (char *);

/*!
 * function:
 *
 *  \brief
 *  \param s
 *  \return
 */
void puts(char *s)
{
#if defined(CONFIG_TELNETD)
    if(telnetd_output)
        (*telnetd_output) (s);
#endif

    if (io_redirect)
    {
        (*io_redirect) (s);
        return;
    }
    else
        while (*s)
            putchar(*s++);
    putchar('\n');
}

/*!
 * function:
 *
 *  \brief
 *  \param msg
 *  \param buf
 *  \param len
 *  \return
 */

void dump_hex(char *msg, char *buf, int len)
{
    int j;
    printf("%s %08x :\n", msg, buf);
    for (j = 0; len; len--)
    {
        printf("%02x ", *(buf++) & 0xff);
        if (++j > 15)
        {
            j = 0;
            printf("\n");
        }
    }
}

/*!
 * function:
 *
 *  \brief
 *  \param msg
 *  \param buf
 *  \param len
 *  \return
 */

void dump_hex4(char *msg, unsigned int *buf, int len)
{
    int j = 0;
    unsigned int *end_address = (unsigned int *) ((unsigned int) buf + len);
    printf("%s %08x :\n", msg, buf);
    for (; buf < end_address;)
    {
        printf("%08x ", *(buf++));
        if (++j > 3)
        {
            j = 0;
            printf("\n");
        }
    }
    if (len % 16)
        printf("\n");
}

/*!
 * function:
 *
 *  \brief
 *  \return
 */

int getchar(void)
{
    return serial_getc();
}

/*!
 * function:
 *
 *  \brief
 *  \param cc
 *  \return
 */

int putchar(int cc)
{
    if (cc == '\n')
        serial_putc('\r');
    serial_putc(cc);
    return ERR_OK;
}

/*!
 * function:
 *
 *  \brief
 *  \return
 */

int tstc(void)
{
    return serial_poll();
}

/*!
 * function:
 *
 *  \brief
 *  \param msg
 *  \param buf
 *  \param len
 *  \return
 */

void hexdump(char *msg, unsigned char *buf, unsigned short len)
{
    unsigned short i;
    printf("%s", msg);
    for (i = 0; i < len; i++)
    {
        if (0 == (i % 32))
        {
            printf("\n%08x: ", buf + i);
        }
        printf("%02x ", (unsigned int) buf[i]);
    }
}

/*!
 * function:
 *
 *  \brief
 *  \param c
 *  \return
 */

static void putchr(int c)
{
    putchar(c);
    if (c == '\n')
        putchar('\r');
}

/*!
 * function:
 *
 *  \brief
 *  \param buf
 *  \param fmt
 *  \param args
 *  \return
 */

int vsprintf(char *buf, char *fmt, va_list args)
{
    int count = 0;
    int pwidth, width, pcnt, base;
    unsigned long num;
    char fch, c;                /* format char */
    char *s, *bp;
    char ljust, zsup;           /* left justify, zero suppress flags */
    char sign;                  /* signed number conversion flag */
    char letter = 0;                /* hex conversion letter increment */
    char islong;                /* long integer flag */
    char pflag;

    *buf = 0;
    bp = buf;                   /* point to start of buf */
    while ((fch = *fmt++) != 0)
    {
        while (*bp)             /* find end of current string */
            bp++;               /*  where next field starts */
        if (fch == '%')
        {
            ljust = 0;          /* reset flags and width */
            pwidth = sizeof (void *) * 2;       /* minimum pointer print width */
            pcnt = 0;           /* printed length of current field */
            islong = 0;         /* default int is not long */
            sign = 0;           /* default unsigned */
            pflag = 0;          /* not %p spec */

            zsup = 1;           /* zero suppression default */
            base = 10;          /* default base */
            switch (*fmt++)
            {                   /* parse flags */
                case '%':
                    goto copy;
                case '-':
                    ljust = 1;
                    break;
                case '*':      /* dynamic field width */
                    width = va_arg(args, int);
                    goto gotwidth;
                default:
                    fmt--;
                    break;
            }
            if (*fmt == '0')    /* get width if there */
                zsup = 0;       /* no zero suppression */
            width = atoi(fmt);  /* get minimum field width */
          gotwidth:
            while ((*fmt >= '0') && (*fmt <= '9'))
                fmt++;
          ismod:
            fch = *fmt++;
            switch (fch)
            {
                case 'l':
                case 'L':
                    islong = 1;
                    goto ismod; /* modifier character */
                case 'i':
                case 'd':
                    sign = 1;
                    goto donumber;
                case 'o':      /* octal */
                    base = 8;
                    goto donumber;
                case 'u':
                    goto donumber;
                case 'x':      /* hex */
                    base = 16;
                    letter = 'a' - 10;
                    goto donumber;
                case 'X':
                    base = 16;
                    letter = 'A' - 10;
                    goto donumber;
                case 'p':      /* void * */
                    pflag = 1;
                    if (width < pwidth)
                        width = pwidth;
                    base = 16;
                    letter = 'A' - 10;
                    num = va_arg(args, long int);       /* long argument */
                    goto doptr;
                case 'c':
                    append_chr(bp++, (char) (va_arg(args, int)));
                    COUNT goto endarg;
                case 's':
                    s = va_arg(args, char *);
                    if (!s)
                        s = "NULL";     /* null pointer passed for %s */
                    while (*s)
                    {           /* copy string to buf */
                        append_chr(bp, *s++);
                        COUNT pcnt++;
                    }
                    for (; pcnt < width; pcnt++)
                    {
                        COUNT if (ljust)
                             append_chr(bp, ' ');
                        else
                        {
                            ins_chr(bp, ' ');
                        }
                    }
                    goto endarg;
                default:
                    goto copy;
            }
          donumber:
            {
                if (islong)
                    num = va_arg(args, long int);       /* long argument */
                else if (sign)
                    num = (long) va_arg(args, int);     /* integer argument */
                else
                    num = (long) va_arg(args, unsigned int);    /* integer argument */
                if (sign && (num & 0x80000000))
                {
                    sign = 1;   /* do sign */
                    num = (unsigned long) (-(long) num);
                }
                else
                {
                    sign = 0;
                }
              doptr:
                while (num != 0l)
                {
                    c = (char) (num % base);
                    num /= base;
                    ins_chr(bp, (char) (c > 9 ? c + letter : c + '0'));
                    pcnt++;     /* count digits */
                COUNT}
                if (!*bp)
                {
                    ins_chr(bp, '0');   /* at least 1 zero */
                    pcnt++;
                COUNT}
                if (pflag)
                {
                    for (; pcnt < pwidth; pcnt++)
                    {
                        ins_chr(bp, '0');
                    COUNT}
                }
                c = (char) (zsup ? ' ' : '0');  /* pad char */
                for (pcnt += sign; pcnt < width; pcnt++)
                    if (ljust)
                    {           /* left justified ? */
                        append_chr(bp, ' ');
                    COUNT}
                    else
                    {
                        ins_chr(bp, c);
                    COUNT}
                if (sign)
                    ins_chr(bp, '-');
            }

        }
        else
        {
          copy:append_chr(bp++, fch);
            /* copy char to output */
        COUNT}
      endarg:
        continue;
    }
    return count;
}

/*!
 * function:
 *
 *  \brief
 *  \param buf
 *  \param fmt
 *  \return
 */
int sprintf(char *buf, char *fmt, ...)
{
    va_list args;
    int count;
    va_start(args, fmt);
    count = vsprintf(buf, fmt, args);
    va_end(args);
    return count;
}

/*!
 * function:
 *
 *  \brief
 *  \param fmt
 *  \return
 */

int printf(char *fmt, ...)
{
    char pf_buf[MAXPFLEN];

    int count;

    char *p;
    va_list args;

    va_start(args, fmt);
    count = vsprintf(pf_buf, fmt, args);
    va_end(args);

    p = pf_buf;

#if defined(CONFIG_TELNETD)
    if(telnetd_output)
        (*telnetd_output) (p);
#endif

    if (io_redirect)
        (*io_redirect) (p);
    else
        while (*p)
        {
            putchr((unsigned int) *p);
            p++;
        }

    return count;
}

int printf_no_redirect(char *fmt, ...)
{
    char pf_buf[MAXPFLEN];

    int count;

    char *p;
    va_list args;

    va_start(args, fmt);
    count = vsprintf(pf_buf, fmt, args);
    va_end(args);

    p = pf_buf;

    while (*p)
    {
        putchr((unsigned int) *p);
        p++;
    }

    return count;
}

/*!
 * function:
 *      
 *  \brief
 *  \
 *  \ 
 */

void fa(void)
{
    char *arg[] = { "fas", "" };
    flash_cmd(0, arg + 1);
}

/*!
 * function:
 *
 *  \brief
 *  \param argc
 *  \param argv
 *  \return
 */

int upgrade_cmd(int argc, char *argv[])
{
    if (argc < 1)
    {
        printf("Upgrading......\n");
        // hook function:
        hook_function_p = fa;
        goto valid;
    }

    if (argc > 1)
    {
        goto help;
    }

    if (!strcmp("enable", argv[0]))
    {
        printf("Enable webpage upgrade feature\n");
        eth_open(0);
        bootvars.network = 1;
    }
    else if (!strcmp("disable", argv[0]))
    {
        printf("Disable webpage upgrade feature\n");
        eth_open(0);
        bootvars.network = 0;
    }
    else
    {
        goto err;
    }

valid:
    return ERR_OK;
help:
    return ERR_HELP;
err:
    return ERR_PARM;
}

cmdt cmdt_upgrade __attribute__ ((section("cmdt"))) =
{
"upgrade", upgrade_cmd, "upgrade: start upgrade\n"
                        "upgrade <enable/disable>: enable/disable webpage upgrade"};

#ifdef CONFIG_WIFI
unsigned short read_be16(const unsigned char *a)
{
	return (unsigned short) (((a[0]&0xff) << 8) | ((a[1]&0xff)));
}

void write_be16(unsigned char *a, unsigned short v)
{
	a[0] = (unsigned char) ((((unsigned short)v) >> 8) & 0xff);	
	a[1] = (unsigned char) (((unsigned short)v) & 0xff);
}

unsigned short read_le16(const unsigned char *a)
{
	return (unsigned short) (((a[1]&0xff) << 8) | ((a[0]&0xff)));
}

void write_le16(unsigned char *a, unsigned short v)
{
	a[1] = (unsigned char) ((((unsigned short)v) >> 8) & 0xff);	
	a[0] = (unsigned char) (((unsigned short)v) & 0xff);
}

unsigned int read_be24(const unsigned char *a)
{
	return (unsigned int) (((a[0]&0xff) << 16) | ((a[1]&0xff) << 8) | ((a[2]&0xff)));
}

void write_be24(unsigned char *a, unsigned int v)
{
	a[0] = (unsigned char) ((((unsigned int)v) >> 16) & 0xff);	
	a[1] = (unsigned char) ((((unsigned int)v) >> 8) & 0xff);	
	a[2] = (unsigned char) (((unsigned int)v) & 0xff);
}

unsigned int read_le24(const unsigned char *a)
{
	return (unsigned int) (((a[2]&0xff) << 16) | ((a[1]&0xff) << 8) | ((a[0]&0xff)));
}

void write_le24(unsigned char *a, unsigned int v)
{
	a[2] = (unsigned char) ((((unsigned int)v) >> 16) & 0xff);	
	a[1] = (unsigned char) ((((unsigned int)v) >> 8) & 0xff);	
	a[0] = (unsigned char) (((unsigned int)v) & 0xff);
}

unsigned int read_be32(const unsigned char *a)
{
	return (unsigned int) ((a[0] << 24) | ((a[1]&0xff) << 16) | ((a[2]&0xff) << 8) | (a[3] & 0xff));
}

void write_be32(unsigned char *a, unsigned int v)
{
	a[0] = (unsigned char) ((((unsigned int)v) >> 24) & 0xff);	
	a[1] = (unsigned char) ((((unsigned int)v) >> 16) & 0xff);	
	a[2] = (unsigned char) ((((unsigned int)v) >> 8) & 0xff);
	a[3] = (unsigned char) (((unsigned int)v) & 0xff);
}

unsigned int read_le32(const unsigned char *a)
{
	return (unsigned int) ((a[3] << 24) | ((a[2]&0xff) << 16) | ((a[1]&0xff) << 8) | (a[0] & 0xff));
}

void write_le32(unsigned char *a, unsigned int v)
{
	a[3] = (unsigned char) ((((unsigned int)v) >> 24) & 0xff);	
	a[2] = (unsigned char) ((((unsigned int)v) >> 16) & 0xff);	
	a[1] = (unsigned char) ((((unsigned int)v) >> 8) & 0xff);
	a[0] = (unsigned char) (((unsigned int)v) & 0xff);
}

unsigned long long read_le64(const unsigned char *a)
{
	unsigned long long a64;
	unsigned int a32_lo = read_le32(&a[0]);
	unsigned int a32_hi = read_le32(&a[4]);
	a64 = a32_hi;
	return (a64 = (a64 << 32) | ((unsigned long long)a32_lo & 0xffffffff));
}

void write_le64(unsigned char *a, unsigned long long v)
{
	a[7] = (unsigned char) ((((unsigned long long)v) >> 56) & 0xff);	
	a[6] = (unsigned char) ((((unsigned long long)v) >> 48) & 0xff);	
	a[5] = (unsigned char) ((((unsigned long long)v) >> 40) & 0xff);
	a[4] = (unsigned char) ((((unsigned long long)v) >> 32)& 0xff);
	a[3] = (unsigned char) ((((unsigned long long)v) >> 24) & 0xff);	
	a[2] = (unsigned char) ((((unsigned long long)v) >> 16) & 0xff);	
	a[1] = (unsigned char) ((((unsigned long long)v) >> 8) & 0xff);
	a[0] = (unsigned char) (((unsigned long long)v) & 0xff);
}

unsigned long long read_be64(const unsigned char *a)
{
	unsigned long long a64;
	unsigned int a32_lo = read_be32(&a[4]);
	unsigned int a32_hi = read_be32(&a[0]);
	a64 = a32_hi;
	return (a64 = (a64 << 32) | ((unsigned long long)a32_lo & 0xffffffff));
}

void write_be64(unsigned char *a, unsigned long long v)
{
	a[0] = (unsigned char) ((((unsigned long long)v) >> 56) & 0xff);	
	a[1] = (unsigned char) ((((unsigned long long)v) >> 48) & 0xff);	
	a[2] = (unsigned char) ((((unsigned long long)v) >> 40) & 0xff);
	a[3] = (unsigned char) ((((unsigned long long)v) >> 32)& 0xff);
	a[4] = (unsigned char) ((((unsigned long long)v) >> 24) & 0xff);	
	a[5] = (unsigned char) ((((unsigned long long)v) >> 16) & 0xff);	
	a[6] = (unsigned char) ((((unsigned long long)v) >> 8) & 0xff);
	a[7] = (unsigned char) (((unsigned long long)v) & 0xff);
}
#endif
