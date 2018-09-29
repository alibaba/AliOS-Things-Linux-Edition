/*!
*   \file xmodem.c
*   \brief Xmodem Implementation
*   \author
*/
/*

The Xmodem Protocol Specification

MODEM PROTOCOL OVERVIEW

1/1/82 by Ward Christensen. I will maintain a master copy of
this. Please pass on changes or suggestions via CBBS/Chicago
at (312) 545-8086, or by voice at (312) 849-6279.

NOTE this does not include things which I am not familiar with,
such as the CRC option implemented by John Mahr.

Last Rev: (none)

At the request of Rick Mallinak on behalf of the guys at
Standard Oil with IBM P.C.s, as well as several previous
requests, I finally decided to put my modem protocol into
writing. It had been previously formally published only in the
AMRAD newsletter.

Table of Contents
1. DEFINITIONS
2. TRANSMISSION MEDIUM LEVEL PROTOCOL
3. MESSAGE BLOCK LEVEL PROTOCOL
4. FILE LEVEL PROTOCOL
5. DATA FLOW EXAMPLE INCLUDING ERROR RECOVERY
6. PROGRAMMING TIPS.

-------- 1. DEFINITIONS.
<soh> 01H
<eot> 04H
<ack> 05H
<nak> 15H
<can> 18H

-------- 2. TRANSMISSION MEDIUM LEVEL PROTOCOL
Asynchronous, 8 data bits, no parity, one stop bit.

The protocol imposes no restrictions on the contents of the
data being transmitted. No control characters are looked for
in the 128-byte data messages. Absolutely any kind of data may
be sent - binary, ASCII, etc. The protocol has not formally
been adopted to a 7-bit environment for the transmission of
ASCII-only (or unpacked-hex) data , although it could be simply
by having both ends agree to AND the protocol-dependent data
with 7F hex before validating it. I specifically am referring
to the checksum, and the block numbers and their ones-
complement.
Those wishing to maintain compatibility of the CP/M file
structure, i.e. to allow modemming ASCII files to or from CP/M
systems should follow this data format:
* ASCII tabs used (09H); tabs set every 8.
* Lines terminated by CR/LF (0DH 0AH)
* End-of-file indicated by ^Z, 1AH. (one or more)
* Data is variable length, i.e. should be considered a
continuous stream of data bytes, broken into 128-byte
chunks purely for the purpose of transmission.
* A CP/M "peculiarity": If the data ends exactly on a
128-byte boundary, i.e. CR in 127, and LF in 128, a
subsequent sector containing the ^Z EOF character(s)
is optional, but is preferred. Some utilities or
user programs still do not handle EOF without ^Zs.
* The last block sent is no different from others, i.e.
there is no "short block".

-------- 3. MESSAGE BLOCK LEVEL PROTOCOL
Each block of the transfer looks like:
<SOH><blk #><255-blk #><--128 data bytes--><cksum>
in which:
<SOH> = 01 hex
<blk #> = binary number, starts at 01 increments by 1, and
wraps 0FFH to 00H (not to 01)
<255-blk #> = blk # after going thru 8080 "CMA" instr, i.e.
each bit complemented in the 8-bit block number.
Formally, this is the "ones complement".
<cksum> = the sum of the data bytes only. Toss any carry.

-------- 4. FILE LEVEL PROTOCOL

---- 4A. COMMON TO BOTH SENDER AND RECEIVER:

All errors are retried 10 times. For versions running with
an operator (i.e. NOT with XMODEM), a message is typed after 10
errors asking the operator whether to "retry or quit".
Some versions of the protocol use <can>, ASCII ^X, to
cancel transmission. This was never adopted as a standard, as
having a single "abort" character makes the transmission
susceptible to false termination due to an <ack> <nak> or <soh>
being corrupted into a <can> and canceling transmission.
The protocol may be considered "receiver driven", that is,
the sender need not automatically re-transmit, although it does
in the current implementations.

---- 4B. RECEIVE PROGRAM CONSIDERATIONS:
The receiver has a 10-second timeout. It sends a <nak>
every time it times out. The receiver's first timeout, which
sends a <nak>, signals the transmitter to start. Optionally,
the receiver could send a <nak> immediately, in case the sender
was ready. This would save the initial 10 second timeout.
However, the receiver MUST continue to timeout every 10 seconds
in case the sender wasn't ready.
Once into a receiving a block, the receiver goes into a
one-second timeout for each character and the checksum. If the
receiver wishes to <nak> a block for any reason (invalid
header, timeout receiving data), it must wait for the line to
clear. See "programming tips" for ideas
Synchronizing: If a valid block number is received, it
will be: 1) the expected one, in which case everything is fine;
or 2) a repeat of the previously received block. This should
be considered OK, and only indicates that the receivers <ack>
got glitched, and the sender re-transmitted; 3) any other block
number indicates a fatal loss of synchronization, such as the
rare case of the sender getting a line-glitch that looked like
an <ack>. Abort the transmission, sending a <can>

---- 4C. SENDING PROGRAM CONSIDERATIONS.

While waiting for transmission to begin, the sender has
only a single very long timeout, say one minute. In the
current protocol, the sender has a 10 second timeout before
retrying. I suggest NOT doing this, and letting the protocol
be completely receiver-driven. This will be compatible with
existing programs.
When the sender has no more data, it sends an <eot>, and
awaits an <ack>, resending the <eot> if it doesn't get one.
Again, the protocol could be receiver-driven, with the sender
only having the high-level 1-minute timeout to abort.

-------- 5. DATA FLOW EXAMPLE INCLUDING ERROR RECOVERY

Here is a sample of the data flow, sending a 3-block message.
It includes the two most common line hits - a garbaged block,
and an <ack> reply getting garbaged. <xx> represents the
checksum byte.

SENDER RECEIVER
times out after 10 seconds,
                        <--- <nak>
<soh> 01 FE -data- <xx> --->
                        <--- <ack>
<soh> 02 FD -data- xx   ---> (data gets line hit)
                        <--- <nak>
<soh> 02 FD -data- xx   --->
                        <--- <ack>
<soh> 03 FC -data- xx   --->
    (ack gets garbaged) <--- <ack>
<soh> 03 FC -data- xx   ---> <ack>
<eot>                   --->
                        <--- <ack>

-------- 6. PROGRAMMING TIPS.

* The character-receive subroutine should be called with a
parameter specifying the number of seconds to wait. The
receiver should first call it with a time of 10, then <nak> and
try again, 10 times.
After receiving the <soh>, the receiver should call the
character receive subroutine with a 1-second timeout, for the
remainder of the message and the <cksum>. Since they are sent
as a continuous stream, timing out of this implies a serious
like glitch that caused, say, 127 characters to be seen instead
of 128.

* When the receiver wishes to <nak>, it should call a "PURGE"
subroutine, to wait for the line to clear. Recall the sender
tosses any characters in its UART buffer immediately upon
completing sending a block, to ensure no glitches were mis-
interpreted.
The most common technique is for "PURGE" to call the
character receive subroutine, specifying a 1-second timeout,
and looping back to PURGE until a timeout occurs. The <nak> is
then sent, ensuring the other end will see it.

* You may wish to add code recommended by Jonh Mahr to your
character receive routine - to set an error flag if the UART
shows framing error, or overrun. This will help catch a few
more glitches - the most common of which is a hit in the high
bits of the byte in two consecutive bytes. The <cksum> comes
out OK since counting in 1-byte produces the same result of
adding 80H + 80H as with adding 00H + 00H.




*/
#include <common.h>
#include <lib.h>

#define SOH  0x01
#define STX  0x02
#define EOT  0x04
#define ACK  0x06
#define NAK  0x15
#define CAN  0x18
#define CTRLZ 0x1A

#define ONE_SECOND 1000
#define MAXRETRANS 25

#if defined(BOOT_MODE_BOOT1)
unsigned short crc16_ccitt(const unsigned char* data_p, unsigned char length)
{
    unsigned char x;
    unsigned long crc = 0;

    while (length--)
    {
        x = (crc >> 8) ^ (*data_p++);
        x ^= (x >> 4);
        crc = (crc << 8) ^ ((unsigned short)(x << 12)) ^ ((unsigned short)(x << 5)) ^ ((unsigned short)x);
        crc = crc & 0x0ffff;
    }
    return crc & 0xffff;
}
#else
extern unsigned short crc16_ccitt(const void *buf, int len);
#endif

#if defined(CONFIG_CMD_FLASH) && defined(CONFIG_FLASH_CMD_PROGRAM_AFTER_DOWNLOAD_FILE) && !defined(BOOT_MODE_BOOT1)
extern char downloaded;
#endif

static int _inbyte(int ms)
{
    unsigned int time = clock_get();
    do
    {
        if (serial_poll())
        {
            return serial_getc();
        }
    } while ( how_long(time)<ms);
    return -1;
}

#define _outbyte(c) serial_putc(c)

static int check(int crc, const unsigned char *buf, int sz)
{
    if (crc) {
        unsigned short crc = crc16_ccitt(buf, sz);
        unsigned short tcrc = (buf[sz]<<8)+buf[sz+1];
        if (crc == tcrc)
            return 1;
    }
    else {
        int i;
        unsigned char cks = 0;
        for (i = 0; i < sz; ++i) {
            cks += buf[i];
        }
        if (cks == buf[sz])
        return 1;
    }

    return 0;
}

static void flushinput(void)
{
    while (_inbyte(((ONE_SECOND)*3)>>1) >= 0)
        ;
}

#if defined(BOOT_MODE_BOOT1)
#define memcpy xmodem_memcpy
static void xmodem_memcpy(void *dst, const void *src, unsigned int len)
{
    register int i;
    register char *s = (char *) src;
    register char *d = (char *) dst;

    for (i = 0; i < len; i++)
        d[i] = s[i];
}
#endif

int xmodem_rx(unsigned char *dest, int destsz)
{
    unsigned char xbuff[1030]; /* 1024 for XModem 1k + 3 head chars + 2 crc + nul */
    unsigned char *p;
    int bufsz, crc = 0;
    unsigned char trychar = 'C';
    unsigned char packetno = 1;
    int i, c, len = 0;
    int retry, retrans = MAXRETRANS;

    for(;;) {
        for( retry = 0; retry < 16; ++retry)
        {
            if (trychar)
                _outbyte(trychar);
            if ((c = _inbyte((ONE_SECOND)<<1)) >= 0)
            {
                switch (c) {
                case SOH:
                    bufsz = 128;
                    goto start_recv;
                case STX:
                    bufsz = 1024;
                    goto start_recv;
                case EOT:
                    //flushinput();
                    _outbyte(ACK);
#if defined(CONFIG_CMD_FLASH) && defined(CONFIG_FLASH_CMD_PROGRAM_AFTER_DOWNLOAD_FILE) && !defined(BOOT_MODE_BOOT1)
                    downloaded = 1;
#endif
                    return len; /* normal end */
                case CAN:
                    if ((c = _inbyte(ONE_SECOND)) == CAN) {
                        flushinput();
                        _outbyte(ACK);
                        return -1; /* canceled by remote */
                    }
                    break;
                default:
                    break;
                }
            }
        }

        if (trychar == 'C')
        {
            trychar = NAK;
            continue;
        }
        flushinput();
        _outbyte(CAN);
        _outbyte(CAN);
        _outbyte(CAN);
        return -2; /* sync error */

    start_recv:
        if (trychar == 'C') crc = 1;
        trychar = 0;
        p = xbuff;
        *p++ = c;
        for (i = 0;  i < (bufsz+(crc?1:0)+3); ++i)
        {
            if ((c = _inbyte(ONE_SECOND)) < 0)
                goto reject;
            *p++ = c;
        }

        if (xbuff[1] == (unsigned char)(~xbuff[2]) &&
            (xbuff[1] == packetno || xbuff[1] == (unsigned char)packetno-1) &&
            check(crc, &xbuff[3], bufsz))
        {
            if (xbuff[1] == packetno)
            {
                register int count = destsz - len;
                if (count > bufsz)
                    count = bufsz;

                if (count > 0)
                {
                    memcpy (&dest[len], &xbuff[3], count);
                    len += count;
                }
                ++packetno;
                retrans = MAXRETRANS+1;
            }

            if (--retrans <= 0)
            {
                flushinput();
                _outbyte(CAN);
                _outbyte(CAN);
                _outbyte(CAN);
                return -3; /* too many retry error */
            }
            _outbyte(ACK);
            continue;
        }
    reject:
        flushinput();
        _outbyte(NAK);
    }
}

#if !defined(BOOT_MODE_BOOT1)
int xmodem_tx(unsigned char *src, int srcsz)
{
    unsigned char xbuff[1030]; /* 1024 for XModem 1k + 3 head chars + 2 crc + nul */
    int bufsz, crc = -1;
    unsigned char packetno = 1;
    int i, c, len = 0;
    int retry;

    for(;;)
    {
        for( retry = 0; retry < 16; ++retry)
        {
            if ((c = _inbyte((ONE_SECOND)<<1)) >= 0)
            {
                switch (c)
                {
                case 'C':
                    crc = 1;
                    goto start_trans;
                case NAK:
                    crc = 0;
                    goto start_trans;
                case CAN:
                    if ((c = _inbyte(ONE_SECOND)) == CAN)
                    {
                        _outbyte(ACK);
                        flushinput();
                        return -1; /* canceled by remote */
                    }
                    break;
                default:
                    break;
                }
            }
        }
        _outbyte(CAN);
        _outbyte(CAN);
        _outbyte(CAN);
        flushinput();
        return -2; /* no sync */

        for(;;)
        {
        start_trans:
            xbuff[0] = SOH; bufsz = 128;
            xbuff[1] = packetno;
            xbuff[2] = ~packetno;
            c = srcsz - len;
            if (c > bufsz)
                c = bufsz;
            if (c > 0)
            {
                memset ((void *) &xbuff[3], 0, bufsz);
                if (c == 0)
                {
                    xbuff[3] = CTRLZ;
                }
                else
                {
                    memcpy (&xbuff[3], &src[len], c);
                    if (c < bufsz)
                        xbuff[3+c] = CTRLZ;
                }
                if (crc)
                {
                    unsigned short ccrc = crc16_ccitt(&xbuff[3], bufsz);
                    xbuff[bufsz+3] = (ccrc>>8) & 0xFF;
                    xbuff[bufsz+4] = ccrc & 0xFF;
                }
                else
                {
                    unsigned char ccks = 0;
                    for (i = 3; i < bufsz+3; ++i)
                    {
                        ccks += xbuff[i];
                    }
                    xbuff[bufsz+3] = ccks;
                }
                for (retry = 0; retry < MAXRETRANS; ++retry)
                {
                    for (i = 0; i < bufsz+4+(crc?1:0); ++i)
                    {
                        _outbyte(xbuff[i]);
                    }
                    if ((c = _inbyte(ONE_SECOND)) >= 0 )
                    {
                        switch (c)
                        {
                        case ACK:
                            ++packetno;
                            len += bufsz;
                            goto start_trans;
                        case CAN:
                            if ((c = _inbyte(ONE_SECOND)) == CAN)
                            {
                                _outbyte(ACK);
                                flushinput();
                                return -1; /* canceled by remote */
                            }
                            break;
                        case NAK:
                        default:
                            break;
                        }
                    }
                }
                _outbyte(CAN);
                _outbyte(CAN);
                _outbyte(CAN);
                flushinput();
                return -4; /* xmit error */
            }
            else
            {
                for (retry = 0; retry < 10; ++retry)
                {
                    _outbyte(EOT);
                    if ((c = _inbyte((ONE_SECOND)<<1)) == ACK)
                        break;
                }
                flushinput();
                return (c == ACK)?len:-5;
            }
        }
    }
}
#endif

