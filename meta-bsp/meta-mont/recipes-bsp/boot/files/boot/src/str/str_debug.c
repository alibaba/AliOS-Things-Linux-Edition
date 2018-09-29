#include "str.h"
#include <lib.h>

#if defined(TEST_MEMORY)
void test_mem_dump(unsigned long addr, int len)
{
    int i;

    for(i=0;i<len;i+=4)
    {
        putw32(addr + i);
        putc(':');
        putw32(*((volatile unsigned long *)(addr+i)));
        putc('\r'); putc('\n');
    }
    putc('\r'); putc('\n');
}
#endif

#if defined(DEBUG_COMMAND)

#include <stdarg.h>

#define     MAXPFLEN    (1<<10)
#define     COUNT   count++; if(count == MAXPFLEN-1) return(count);

void serial_putc(int c)
{
    putc(c);
    return;
}

unsigned int save_char;
int serial_getc(void)
{
    if (!(save_char & URBR_RDY))
    {
        while (!((save_char = REG_READ32(UART_BASE + URBR)) & URBR_RDY)) ;
    }

    save_char >>= URBR_DTSHFT;
    return save_char;
}

int getchar(void)
{
    return serial_getc();
}

int putchar(int cc)
{
    if (cc == '\n')
        serial_putc('\r');
    serial_putc(cc);
    return 0;
}

static void putchr(int c)
{
    putchar(c);
    if (c == '\n')
        putchar('\r');
}

void append_chr(char *str, char c)
{
    while (*str++) ;
    *str-- = 0;
    *str = c;
}

void ins_chr(char *str, char c)
{
    char *tmps;

    tmps = str;
    while (*tmps++) ;
    while (tmps > str)
    {
        tmps--;
        tmps[1] = tmps[0];
    }
    *str = c;
}

#define is_digit(c) ((c) >= '0' && (c) <= '9')

int strtol(char *s, char **rp, int base)
{
    int i = 0;
    if (base != 10)
        return 0;

    while (is_digit(*s))
        i = i * 10 + (*s++ - '0');
    if (rp)
        *rp = s;
    return i;
}

int atoi(char *s)
{
    return strtol(s, 0, 10);
}

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

    while (*p)
    {
        putchr((unsigned int) *p);
        p++;
    }

    return count;
}

enum
{
    STATE_ESC_NONE = 0,
    STATE_ESC_START = 1,
    STATE_ESC_CODE = 2,
};

enum
{
    NUL = 0x00,
    ETX = 0x03,
    BS = 0x08,
    TAB = 0x09,
    LF = 0x0a,
    CR = 0x0d,
    ESC = 0x1b,
    SPACE = 0X20,
    UP = 0x41,
    DOWN = 0x42,
    RIGHT = 0x43,
    LEFT = 0x44,
    DEL = 0x7f,
};

#define isnum(c)    (c>=48 && c<=57)
static const char erasetoeol[] = { ESC, '[', 'K', NUL };        //<ESC>[K<NULL>
static const char eraseback[] = { BS, SPACE, BS, NUL }; //erase backspace

#define MAX_ARGV    8
#define CMD_BUF_SZ  80
#define PROMPT "db>"

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

int cli_gets(char *buf)
{
    char key;
    char *bp = buf;
    short state = STATE_ESC_NONE;

    do
    {
        key = getchar();
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
    while ((bp - buf) < (249));
  out:
    *bp++ = NUL;
    *bp-- = NUL;
    return (int) (bp - buf);
}

int strcmp(const char *dst, const char *src)
{
    while (*dst)
    {
        if (*dst != *src)
            break;
        dst++;
        src++;
    }
    return *dst - *src;
}

enum
{
    ERR_OK = 0,
    ERR_HELP = 1,
    ERR_PARM = -1,
    ERR_ALIGN = -2,
    ERR_ADDR = -3,
    ERR_FILE = -4,
    ERR_TIMEOUT = -5,
    ERR_ETHER = -6,
    ERR_MEM = -7,
    ERR_LAST = -8,
    ERR_EXIT = -9,
};

#define GETAP(ap) \
    ap = (char *) va_arg(args,char *);


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

int vaddr_check(unsigned long addr)
{
    if (addr < 0x8000000)
        return -1;
    return 0;
}

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

    va_start(args, fmt);        /* get variable arg list address */
    if (!*buf)
    {
        va_end(args);
        return -1;
    }
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
                case 'd':      /* signed integer */
                    par1 = 10;
                    goto lab3;
                case 'o':      /* signed integer */
                    par1 = 8;
                    goto lab3;
                case 'x':      /* signed integer */
                case 'X':      /* long signed integer */
                    par1 = 16;
                    goto lab3;
                case 'u':      /* unsigned integer */
                case 'i':      /* signed integer */
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
                case 'c':      /* character */
                    GETAP(ap);
                    for (; width && *buf; width--)
                    {
                        *ap++ = *buf++;
                        if (width == 0x7fff)
                            break;
                    }
                    goto lab12;
                case '[':      /* search set */
                    GETAP(ap);
                    for (; width && match(fmt, *buf); width--)
                        *ap++ = *buf++;
                    while (*fmt++ != ']') ;
                    goto lab11;
                case 's':      /* character array */
                    GETAP(ap);
                    for (; *buf == ' ' || *buf == 0x07; buf++) ;
                    for (; width && *buf && *buf > ' '; width--)
                        *ap++ = *buf++;
                  lab11:if (prebuf == buf)
                        break;
                    *(char *) ap = 0;
                    goto lab12;
                case 'n':      /* store # chars */
                    GETAP(ap);
                    *(int *) ap = buf - orgbuf;
                    break;
                case 'p':      /* pointer */
                    GETAP(ap);
                    *(long *) ap = asclng(&buf, width, 16);
                  lab12:nfields++;
                    break;
                default:       /* illegal */
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

int hextoul(char *str, void *v)
{
    return (sscanf(str, "%x", v) == 1);
}

static int step_sz(char c)
{
    int step;
    if ('b' == c)
        step = 1;
    else if ('h' == c)
        step = 2;
    else
        step = 4;
    return step;
}

unsigned long buf_address = 0x81000000UL;
int mem_dump_cmd(int argc, char *argv[])
{
    unsigned long addr, caddr;
    unsigned long size;
    int step;

    step = step_sz(argv[-1][1]);
    addr = buf_address;
    size = 128;
    if (argc > 0 && !hextoul(argv[0], &addr))
        goto help;
    if (argc > 1 && !hextoul(argv[1], &size))
        goto help;
    if (size > 1024)
        size = 1024;

    addr &= ~(step - 1);
    if (vaddr_check(addr))
        goto err2;
    caddr = addr;
    while (caddr < (addr + size))
    {
        if (caddr % 16 == 0 || caddr == addr)
        {
            printf("\n%08x: ", caddr);
        }
        switch (step)
        {
            case 1:
                printf("%02x ", *((volatile unsigned char *) caddr));
                break;
            case 2:
                printf("%04x ", *((volatile unsigned short *) caddr));
                break;
            case 4:
                printf("%08x ", *((volatile unsigned long *) caddr));
                break;
        }
        caddr += step;
    }
    buf_address = addr + size;
    printf("\n\r");
    return ERR_OK;

  help:
    return ERR_HELP;
  err2:
    return ERR_ADDR;
}

int mem_enter_cmd(int argc, char *argv[])
{
    int step;
    unsigned long addr, size, val, i;

    step = step_sz(argv[-1][1]);

    size = 1;
    if (argc < 2)
        goto help;

    if (!hextoul(argv[0], &addr))
        goto err1;
    if (!hextoul(argv[1], &val))
        goto err1;
    if (argc > 2 && !hextoul(argv[2], &size))
        goto err1;

    addr &= ~(step - 1);
    if (vaddr_check(addr))
        goto err3;
    for (i = 0; i < size; i += step)
    {
        switch (step)
        {
            case 1:
                *(volatile unsigned char *) (addr + i) = val;
                break;
            case 2:
                *(volatile unsigned short *) (addr + i) = val;
                break;
            case 4:
                *(volatile unsigned long *) (addr + i) = val;
                break;
        }
    }
    return ERR_OK;

  help:
    return ERR_HELP;
  err1:
    return ERR_PARM;
  err3:
    return ERR_ADDR;
}

int cmd_proc(int argc, char **argv)
{
    if(!strcmp(argv[0], "exit"))
        return ERR_EXIT;

    if(argv[0][0]=='e')
        mem_enter_cmd(argc-1, &argv[1]);

    if(argv[0][0]=='d')
        mem_dump_cmd(argc-1, &argv[1]);

    return ERR_OK;
}

void cmd_loop(void)
{
    char *argv[10], buffer[CMD_BUF_SZ + 1];
    int argc;
    int ret;

    //memset(buffer, 0, CMD_BUF_SZ);
    for (;;)
    {
        printf(PROMPT);
        cli_gets(buffer);
        printf("\n");
        if (1 > (argc = get_args(&buffer[0], argv)))
            continue;
        ret = cmd_proc(argc, argv);
        if(ret == ERR_EXIT)
            break;
    }
}

#endif

