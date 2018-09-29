/*=============================================================================+
|                                                                              |
| Copyright 2017                                                              |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/

#if defined(CONFIG_TELNETD)

#include <common.h>
#include <lib.h>
#include <netprot.h>

# define NOP         0
# define IAC         255  /* interpret as command: */
# define DONT        254  /* you are not to use option */
# define DO          253  /* please, you use option */
# define WONT        252  /* I won't use option */
# define WILL        251  /* I will use option */
# define SB          250  /* interpret as subnegotiation */
# define SE          240  /* end sub negotiation */
# define TELOPT_ECHO   1  /* echo */
# define TELOPT_SGA    3  /* suppress go ahead */
# define TELOPT_TTYPE 24  /* terminal type */
# define TELOPT_NAWS  31  /* window size */


#define TELNETD_INPUT_BUFSIZE   2048
#define TELNETD_OUTPUT_BUFSIZE  4096

static char telnet_input_buf[TELNETD_INPUT_BUFSIZE];
static int curr_input_offset;
static int last_input_offset;
static int telnetd_input_func(unsigned char *buf)
{
    if(last_input_offset != curr_input_offset)
    {
        *buf = telnet_input_buf[last_input_offset];
        last_input_offset = (last_input_offset + 1) % TELNETD_INPUT_BUFSIZE;
        return 1;
    }
    return 0;
}

static int telnetd_poll_func(void)
{
    if(last_input_offset != curr_input_offset)
        return 1;
    return 0;
}

static int telnetd_recv_char(char c)
{
    if(((curr_input_offset + 1) % TELNETD_INPUT_BUFSIZE) != last_input_offset)
    {
        telnet_input_buf[curr_input_offset] = c;
        curr_input_offset = (curr_input_offset + 1) % TELNETD_INPUT_BUFSIZE;
        return 1;
    }
    return 0;    
}

static char telnet_output_buf[TELNETD_OUTPUT_BUFSIZE];
static int curr_output_offset;
static int last_output_offset;
static int telnetd_output_func(char *buf)
{
    char c;

    for (; (c = *buf++);)
    {
        if(last_output_offset == ((curr_output_offset + 1) % TELNETD_OUTPUT_BUFSIZE))
            break;

        telnet_output_buf[curr_output_offset] = c;
        curr_output_offset = (curr_output_offset + 1) % TELNETD_OUTPUT_BUFSIZE;
        if(c==IAC)
        {
            telnet_output_buf[curr_output_offset] = c;
            curr_output_offset = (curr_output_offset + 1) % TELNETD_OUTPUT_BUFSIZE;
        }
    }

    return 0;
}

#if defined(WLA_TEST)
#define WT_BBCNT_CMD "wt bbcnt"
extern void do_bbcnt(void);
#endif
int telnetd_handle(struct tcp_conn_t *tcps)
{
    int tlen;
    unsigned char *tdata;
    //unsigned char c;
    //int i;
    unsigned char *ptr, *end;

    tdata = tcps->rxbuf;
    tlen = tcps->rxlen;

#if 0
    if(tlen)
    {
        printf("(%d) ", tlen);
        for(i=0;i<tlen;i++)
        {
            printf(" %02x ", (unsigned char) tdata[i]);
        }
        printf("\n");
    }
#endif

    ptr = tdata;
    end = &tdata[tlen];
	while (ptr < end) {
		if (*ptr != IAC) {
			unsigned char c = *ptr;

            telnetd_recv_char(c);
			ptr++;
			/* We map \r\n ==> \r for pragmatic reasons.
			 * Many client implementations send \r\n when
			 * the user hits the CarriageReturn key.
			 * See RFC 1123 3.3.1 Telnet End-of-Line Convention.
			 */
			if (c == '\r' && ptr < end && (*ptr == '\n' || *ptr == '\0'))
				ptr++;
			continue;
		}
		if (ptr[1] == NOP) { /* Ignore? (putty keepalive, etc.) */
			ptr += 2;
			continue;
		}
		if (ptr[1] == IAC) { /* Literal IAC? (emacs M-DEL) */
            telnetd_recv_char(ptr[1]);
			ptr += 2;
			continue;
		}

		/*
		 * TELOPT_NAWS support!
		 */
		if ((ptr+2) >= end) {
			/* Only the beginning of the IAC is in the
			buffer we were asked to process, we can't
			process this char */
			break;
		}
		/*
		 * IAC -> SB -> TELOPT_NAWS -> 4-byte -> IAC -> SE
		 */
		if (ptr[1] == SB && ptr[2] == TELOPT_NAWS) {
#if 0
			struct winsize ws;
			if ((ptr+8) >= end)
				break;  /* incomplete, can't process */
			ws.ws_col = (ptr[3] << 8) | ptr[4];
			ws.ws_row = (ptr[5] << 8) | ptr[6];
			ioctl(ts->ptyfd, TIOCSWINSZ, (char *)&ws);
#endif
			ptr += 9;
			continue;
		}
		/* skip 3-byte IAC non-SB cmd */
#if DEBUG
		fprintf(stderr, "Ignoring IAC %s,%s\n",
				TELCMD(ptr[1]), TELOPT(ptr[2]));
#endif
		ptr += 3;
	}

    if(last_output_offset != curr_output_offset)
    {
        if(curr_output_offset < last_output_offset)
        {
            tcps->txlen = TELNETD_OUTPUT_BUFSIZE - last_output_offset;
            tcps->txbuf = (unsigned char *) &telnet_output_buf[last_output_offset];
            last_output_offset = 0;
        }
        else
        {
            tcps->txlen = curr_output_offset - last_output_offset;
            tcps->txbuf = (unsigned char *) &telnet_output_buf[last_output_offset];
            last_output_offset = curr_output_offset;
        }
    }
    else
    {
        tcps->txlen = 0;
    }

    return 0;
}

#if !defined(WLA_TEST)
#define TELNETD_DEFAULT_MANNER "Panther\ncmd>"
#else
#define TELNETD_DEFAULT_MANNER "CLI>" //match mp test prefix
#endif
void telnetd_init(void)
{
    curr_input_offset = last_input_offset = 0;

    last_output_offset = 0;
    strcpy(telnet_output_buf, TELNETD_DEFAULT_MANNER);
    curr_output_offset = strlen(TELNETD_DEFAULT_MANNER);

    telnetd_input = (int (*)(unsigned char *)) telnetd_input_func;
    telnetd_poll = (int (*)(void)) telnetd_poll_func;
    telnetd_output = (int (*)(char *)) telnetd_output_func;
}

#endif
