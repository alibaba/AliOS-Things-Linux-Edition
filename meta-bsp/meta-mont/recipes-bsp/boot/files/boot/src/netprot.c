/*=============================================================================+
|                                                                              |
| Copyright 2016, 2018                                                         |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file netprot.c
*   \brief  portion of code from etherboot 4.6.11 and uip
*   \author Montage
*/
/**************************************************************************
ETHERBOOT -  BOOTP/TFTP Bootstrap Program

Author: Martin Renters
  Date: Dec/93

Literature dealing with the network protocols:
    ARP - RFC826
    RARP - RFC903
    UDP - RFC768
    BOOTP - RFC951, RFC2132 (vendor extensions)
    DHCP - RFC2131, RFC2132 (options)
    TFTP - RFC1350, RFC2347 (options), RFC2348 (blocksize), RFC2349 (tsize)
    RPC - RFC1831, RFC1832 (XDR), RFC1833 (rpcbind/portmapper)
    NFS - RFC1094, RFC1813 (v3, useful for clarifications, not implemented)

**************************************************************************/
/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <common.h>
#include <netprot.h>
#include <netdev.h>
#include "sflash/include/flash_config.h"
#include <lib.h>
#include <dhcpd.h>

/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
#define netmask     (bootvars.msk)
#define ip_addl(ui32,ui16) (ui32=(ui32 + ui16))
/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
struct tcp_conn_t tcp_conn = { .rport = 0xffff, };
unsigned int init_ss = 0;
#ifdef CONFIG_SIMPLE_CMD_WEBPAGE
int http_in_used = 0;
#endif
struct arptable_t arptable[MAX_ARP];
bootvar bootvars;
struct tftpreq_t tp;
unsigned int tftp_retry;

bootvar bootvar_default = {
  .mode = CONFIG_BOOT_MODE,
  //.ver = CONFIG_BOOT_VER,
  //.cver = CONFIG_BOOT_CVER,
  .mac0 = CONFIG_BOOT_MAC0,
  .mac1 = CONFIG_BOOT_MAC1,
  .mac2 = CONFIG_BOOT_MAC2,
  .file = CONFIG_BOOT_FILE,
  .ip = CONFIG_BOOT_IP,
  .msk = CONFIG_BOOT_MSK,
  .gw = CONFIG_BOOT_GW,
  .server = CONFIG_BOOT_SERVER,
  .load_sz = CONFIG_BOOT_LOAD_SZ,
  .load_addr = CONFIG_BOOT_DL_ADDR,
  .load_src = KUSEG02KSEG0(CONFIG_FLASH_BASE + FLASH_BLOCK_SIZE),
  .load_src2 = KUSEG02KSEG0(CONFIG_FLASH_BASE + FLASH_BLOCK_SIZE / 2),
  .log_src = (CONFIG_FLASH_BASE + CONFIG_FLASH_SYSLOG_OFS),
  .log_sz = CONFIG_FLASH_SYSLOG_SZ,
  .id = CONFIG_BOOT_ID,
  .hver = CONFIG_BOOT_HWVER,
  .pll = CONFIG_BOOT_PLL,
  .txvga = CONFIG_BOOT_TXVGA,
  .rxvga = CONFIG_BOOT_RXVGA,
  .serial = CONFIG_BOOT_SN,
  .pin = CONFIG_BOOT_PIN,
  .freq_ofs = CONFIG_BOOT_FREQ_OFS,
  .madc_val0 = CONFIG_BOOT_MADC_VAL0,
  .madc_val1 = CONFIG_BOOT_MADC_VAL1,
  .lna = CONFIG_BOOT_LNA,
  .autocal = 0,
  .quiet = 1,
  .swcfg = CONFIG_BOOT_SWCFG,
  .ai = CONFIG_BOOT_AUDIO_INTERFACE,
  .upgrade = 0,
  .ci_offset = CONFIG_BOOT_CI_OFFSET,
  .recovery_offset = CONFIG_BOOT_RECOVERY_OFFSET,
  .second_img_offset = CONFIG_SECOND_IMAGE_OFFSET,
  .recovery = CONFIG_BOOT_RECOVERY,
  .txp_diff = CONFIG_BOOT_TXPDIFF,
  .fem_product_id = CONFIG_BOOT_FEM_PRODUCT_ID,
  .fem_en = CONFIG_BOOT_FEM_EN,
  .pinmux = CONFIG_BOOT_PINMUX,
  .powercfg = CONFIG_BOOT_POWERCFG,
  .gpio_driving = CONFIG_BOOT_DRIV_STR,
  .gpio_setting = CONFIG_BOOT_GPIO_SETTING,
  .clkcfg = CONFIG_BOOT_CLKCFG,
  .watchdog_timer = CONFIG_BOOT_WATCHDOG_TIMER,
  .mic_gain_ctrl = CONFIG_BOOT_MIC_GAIN_CTRL_STR,
  .sd_root = "none",
};

struct img
{
    unsigned long count;
    unsigned long addr;
};

static struct img img;
static const unsigned char broadcast[] = {[0 ... 5] = 0xff };
static const unsigned char zeros[] = {[0 ... 5] = 0 };

/*=============================================================================+
| Function Prototypes                                                          |
+=============================================================================*/
static int download_file(unsigned char *data, int block, int len, int eof);
static int upload_file(unsigned char *data, int block, int len, int eof);
unsigned short ipchksum(void *ip, int len);
static unsigned short chksum_pseudo(void *p, unsigned int *src,
                                    unsigned int *dst,
                                    unsigned char prot, int len);
/*=============================================================================+
| Extern Function/Variables                                                    |
+=============================================================================*/
#if defined(CONFIG_CMD_FLASH) && defined(CONFIG_FLASH_CMD_PROGRAM_AFTER_DOWNLOAD_FILE)
extern char downloaded;
#endif
/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/
/*!
 * function:
 *
 *  \brief
 *  \return
 */
int update_mac()
{
    arptable[ARP_CLIENT].ipaddr.s_addr = bootvars.ip;
    arptable[ARP_GATEWAY].ipaddr.s_addr = bootvars.gw;
    memcpy(arptable[ARP_CLIENT].node, bootvars.mac0, 6);
    return 0;
}

/*!
 * function:
 *
 *  \brief
 *  \return
 */
#if defined(CONFIG_ATE)
void config_init(int *s)
{
    bootvars = bootvar_default;

    bootvars.network = 1;

    update_mac();

    buf_address = bootvars.load_addr;
}
#else
void config_init(int *s)
{
    int result;

    bootvars = bootvar_default;
    if ((result = cdb_init(0)))
    {
        init_cdb_page_idx();
        printf("Take default cfg\n\r");
        *s = 1;
    }
    else
    {
        *s = 0;
    }

    //reserve version according to CONFIG_BOOT_VER
    //bootvars.ver = ntohl(CONFIG_BOOT_VER);
    update_mac();

    buf_address = bootvars.load_addr;
    //byte_count = bootvars.load_sz;

}
#endif

/*!
 * function:
 *
 *  \brief
 *  \param put
 *  \param dest_address
 *  \param bytecount
 *  \param file
 *  \return
 */
int net_tftp(int put, unsigned long dest_address, unsigned int bytecount,
             char *file)
{
    unsigned long tmp;
    img.addr = dest_address;
    img.count = bytecount;

    tmp = htonl(bootvars.server);
    printf("tftp %s %s:%s \n", (put ? "put" : "get"), inet_ntoa(&tmp), file);
    return ((tftp(file, (put ? upload_file : download_file))) ? 1 : 0);
}

/*!
 * function:
 *
 *  \brief
 *  \param destip
 *  \param prot
 *  \param len
 *  \param buf
 *  \return
 */
//separate from original udp_transmit()
int ip_tx(unsigned long destip, char prot, int len, const void *buf, struct nbuf *clone_pkt)
{
    struct iphdr *ip;
//  struct udphdr *udp;
    struct arprequest arpreq;
    int arpentry, retry, timeout, result;
    unsigned long ipaddr;

    ip = (struct iphdr *) buf;
//  udp = (struct udphdr *)((long)buf + sizeof(struct iphdr));
    ip->verhdrlen = 0x45;
    ip->service = 0;
    ip->len = htons(len);
    ip->ident = 0;
    ip->frags = 0;
    ip->ttl = 64;
//  ip->protocol = IP_UDP;
    ip->protocol = prot;
    ip->chksum = 0;
    ip->src.s_addr = htonl(arptable[ARP_CLIENT].ipaddr.s_addr);
    ip->dest.s_addr = htonl(destip);
    ip->chksum = ~ipchksum((unsigned short *) buf, IPHDR_SZ);
/*
    udp->src = htons(srcsock);
    udp->dest = htons(destsock);
    udp->len = htons(len - sizeof(struct iphdr));
    udp->chksum = 0;
*/
    if (destip == IP_BROADCAST)
        eth_tx((const char *)broadcast, IP, len, buf);
    else
    {
        if (((destip & netmask) !=
             (arptable[ARP_CLIENT].ipaddr.s_addr & netmask)) &&
            arptable[ARP_GATEWAY].ipaddr.s_addr)
            destip = arptable[ARP_GATEWAY].ipaddr.s_addr;
        for (arpentry = 0; arpentry < MAX_ARP; arpentry++)
            if (arptable[arpentry].ipaddr.s_addr == destip)
                break;

        if (arpentry == MAX_ARP)
        {
            arptable[--arpentry].ipaddr.s_addr = destip;
            memset((void *)arptable[arpentry].node, 0, 6);
        }

        if (!memcmp((void *)arptable[arpentry].node, (void *)zeros, 6))
        {
            arpreq.hwtype = htons(1);
            arpreq.protocol = htons(IP);
            arpreq.hwlen = ETHER_ADDR_SZ;
            arpreq.protolen = 4;
            arpreq.opcode = htons(ARP_REQUEST);
            memcpy(arpreq.shwaddr, arptable[ARP_CLIENT].node, ETHER_ADDR_SZ);
            ipaddr = cpu_to_be32(arptable[ARP_CLIENT].ipaddr.s_addr);
            memcpy(&arpreq.sipaddr[0], &ipaddr, sizeof(ipaddr));
            memset((char *)arpreq.thwaddr, 0, ETHER_ADDR_SZ);
            ipaddr = cpu_to_be32(destip);
            memcpy(&arpreq.tipaddr[0], &ipaddr, sizeof(ipaddr));
            for (retry = 1; retry <= MAX_ARP_RETRIES; retry++)
            {
                eth_tx((const char *)broadcast, ARP, sizeof (arpreq), (const void *)&arpreq);
                timeout = rfc951_sleep(TIMEOUT, retry);
                if (ESC ==
                    (result = await_reply(AWAIT_ARP, &arpentry, timeout)))
                    return 0;
                else if (1 == result)
                    goto xmit;
            }
            return (0);
        }
      xmit:
        eth_tx((const char *)arptable[arpentry].node, (unsigned int)IP, (unsigned int)len, buf);
    }
    return (1);
}

/*!
 * function:
 *
 *  \brief
 *  \param dip
 *  \param sp
 *  \param dp
 *  \param len
 *  \param buf
 *  \return
 */
int udp_tx(unsigned long dip, unsigned int sp, unsigned int dp, int len,
           const void *buf)
{
    struct udphdr *pudp;

    //separate from original udp_transmit()
    pudp = (struct udphdr *) ((char *) buf + IPHDR_SZ);
    pudp->chksum = 0;           //no checksum
    pudp->dest = htons(dp);
    pudp->src = htons(sp);
    pudp->len = htons(len - IPHDR_SZ);
    return ip_tx(dip, IP_UDP, len, buf, NULL);
}


//#define ENABLE_TCP_DEBUG
//#define TCP_PACKET_LOST_TEST

#if defined(ENABLE_TCP_DEBUG)
#define TCP_DBG  printf_no_redirect
#else
#define TCP_DBG(...)
#endif

#define IP_HEADER_LEN   20
#define TCP_HEADER_LEN  40
#define MTU             1500

int tcp_output_queue_depth(struct tcp_conn_t *tpc)
{
    int depth = (tpc->od_write_idx - tpc->od_read_idx);

    if(depth < 0)
        depth += TCP_OUTPUT_DATA_BUFSIZE;

    return depth;
}

void tcp_dequeue_output_data_commit(struct tcp_conn_t *tpc, int length)
{
    tpc->od_read_idx = (tpc->od_read_idx + length) % TCP_OUTPUT_DATA_BUFSIZE;
}

int tcp_dequeue_output_data_peek(struct tcp_conn_t *tpc, unsigned char *data, int max_length)
{
    int curr_depth = tcp_output_queue_depth(tpc);
    int length;
    int len_to_tail;

    if(curr_depth > max_length)
        length = max_length;
    else
        length = curr_depth;

    if(length==0)
        return length;

    len_to_tail = TCP_OUTPUT_DATA_BUFSIZE - tpc->od_read_idx;
    if(length > len_to_tail)
    {
        memcpy(data, &tpc->output_data[tpc->od_read_idx], len_to_tail);
        memcpy(&data[len_to_tail], tpc->output_data, length - len_to_tail);
    }
    else
    {
        memcpy(data, &tpc->output_data[tpc->od_read_idx], length);
    }

    return length;
}

void tcp_queue_output_data(struct tcp_conn_t *tpc, unsigned char *data, int length)
{
    int curr_depth = tcp_output_queue_depth(tpc);
    int available;
    int len_to_tail;

    available = (TCP_OUTPUT_DATA_BUFSIZE - 1) - curr_depth;

    if(available==0)
        return;

    if(length > available)
        length = available;

    len_to_tail = TCP_OUTPUT_DATA_BUFSIZE - tpc->od_write_idx;
    if(length > len_to_tail)
    {
        memcpy(&tpc->output_data[tpc->od_write_idx], data, len_to_tail);
        memcpy(&tpc->output_data[0], &data[len_to_tail], length - len_to_tail);
    }
    else
    {
        memcpy(&tpc->output_data[tpc->od_write_idx], data, length);
    }

    tpc->od_write_idx = (tpc->od_write_idx + length) % TCP_OUTPUT_DATA_BUFSIZE;  
}

void tcp_clear_output_data(struct tcp_conn_t *tpc)
{
    tpc->od_read_idx = 0;
    tpc->od_write_idx = 0;
}

void tcp_tx(struct tcp_conn_t *tpc, unsigned char flags, int keep_alive, int payload_length)
{
    unsigned char __tcp_pkt[2048];
    struct iphdr *ip = (struct iphdr *) __tcp_pkt;
    struct tcphdr *tp = (struct tcphdr *) (&ip[1]);

    memset((char *)__tcp_pkt, 0, IP_HEADER_LEN + TCP_HEADER_LEN);

    if(tpc->wait_ack)
        payload_length = tpc->wait_ack;

    if(payload_length)
        payload_length = tcp_dequeue_output_data_peek(tpc, (unsigned char *) &tp[1], payload_length);

    tpc->wait_ack = payload_length;

    tp->flags = flags;
    tp->src = htons(tpc->lport);
    tp->dest = htons(tpc->rport);
    tp->ackno = htonl(tpc->rcv_nxt);
    if(keep_alive)
        tp->seqno = htonl((tpc->snd_nxt-1));
    else
        tp->seqno = htonl(tpc->snd_nxt);
    tp->tcpoffset = 5 << 4;
    tp->window = payload_length ? htons(0) : htons(MTU);
    tp->chksum = 0;
    tp->chksum =
      ~chksum_pseudo(tp, (unsigned int *) &tpc->rip,
                     (unsigned int *) &tpc->dest_ip_addr, IP_TCP, IP_HEADER_LEN + payload_length);

#if defined(TCP_PACKET_LOST_TEST) // enable this to test TCP TX data lost recovery
    static int tx_drop = 0;
    if(tx_drop++>2)
    {
        //TCP_DBG("TX DROP\n");
        tx_drop = 0;
        return;
    }
#endif

    tpc->last_tx_jiffies = clock_get();
    ip_tx(ntohl(tpc->rip), IP_TCP, TCP_HEADER_LEN + payload_length, ip, NULL);
}

void tcp_close_connection(void)
{
    tcp_tx(&tcp_conn, (TH_FIN | TH_RST | TH_ACK), 0, 0);
    tcp_conn.rport = 0;
}

int exit_telnet_cmd(int argc, char *argv[])
{
    tcp_close_connection();
    return ERR_OK;
}

cmdt cmdt_telnet __attribute__ ((section("cmdt"))) =
{
"exit", exit_telnet_cmd, "close telnet session"
};

void tcp_tx_handle(void)
{
    struct tcp_conn_t *tpc = &tcp_conn;
    int curr_depth;

    if(tpc->state!=ESTABLISHED)
        return;

    if(tpc->is_telnet_port)
    {
        tpc->rxlen = 0;
        telnetd_handle(tpc);
        if (tpc->txlen)
            tcp_queue_output_data(tpc, tpc->txbuf, tpc->txlen);
    }

    if(tpc->wait_ack)
    {
        if((tpc->last_tx_jiffies + 10) < clock_get())
            tcp_tx(tpc, (TH_ACK | TH_PUSH), 0, tpc->wait_ack);
    }
    else
    {
        curr_depth = tcp_output_queue_depth(tpc);
        //TCP_DBG("tcp_tx_handle %d %d\n", curr_depth, tpc->wait_ack);

        if(curr_depth)
            tcp_tx(tpc, (TH_ACK | TH_PUSH), 0, (MTU/2));
    }
}

extern struct httpd_state http_state;
/*
    return 0, -1: drop this packet
    return 1: keep/queue the packet and try again later
 */
int tcp_rx_handle(struct nbuf *ipkt)
{
    struct iphdr *ip = (struct iphdr *) &ipkt->data[ETHER_HEAD_SZ];
    struct tcphdr *tp = (struct tcphdr *) (ip + 1);
    struct tcp_conn_t *tpc = &tcp_conn;
    struct tcp_conn_t tpc_tmp;
    int payload_length, t = 0;
    short ts, *sp;
    unsigned long ul;
    int is_http_port = 0;
    int is_telnet_port = 0;

#if defined(TCP_PACKET_LOST_TEST) // enable this to test TCP RX data lost recovery
    static int rx_drop = 0;
    if(rx_drop++>2)
    {
        //TCP_DBG("RX DROP\n");
        rx_drop = 0;
        return 0;
    }
#endif

    if(tp->dest==ntohs(TCP_HTTP))
        is_http_port = 1;

    if(tp->dest==ntohs(TCP_TELNET))
        is_telnet_port = 1;

    if (!is_http_port && !is_telnet_port)
    {
        //printk("tcp port:%x", tp->dest);
        return -1;
    }

    if (tp->flags & (TH_FIN | TH_RST) && tpc->state != CLOSED)
    {
        return -1;
    }

    switch (tpc->state)
    {
        case SYN_SENT:
            if (ntohs(tp->src) == tpc->rport)
            {
                if (tp->flags & TH_ACK)
                {
                    if (tpc->rcv_nxt != ntohl(tp->seqno))
                        goto send_ack_nodata;

                    ul = tpc->snd_nxt;
                    ip_addl(ul, tpc->wait_ack);
                    if (ntohl(tp->ackno) == ul)
                    {
                        tpc->snd_nxt = ul;
                        tpc->wait_ack = 0;

                        tcp_clear_output_data(tpc);
                        tpc->is_http_port = 0;
                        tpc->is_telnet_port = 0;

#ifdef CONFIG_SIMPLE_CMD_WEBPAGE
                        if(is_http_port)
                        {
                            httpd_init();
                            tpc->is_http_port = 1;
                        }
#endif
#if defined(CONFIG_TELNETD)
                        if(is_telnet_port)
                        {
                            telnetd_init();
                            tpc->is_telnet_port = 1;
                        }
#endif
                        tpc->dest_ip_addr = ip->dest.s_addr;
                        tpc->state = ESTABLISHED;
                    }
                }
                break;
            }

        case ESTABLISHED:
            if (ntohs(tp->src) == tpc->rport)
            {
                TCP_DBG("* RX ACK:%x SEQ:%x *\n", ntohl(tp->ackno), ntohl(tp->seqno));
                if (tpc->rcv_nxt != ntohl(tp->seqno))
                {
                    TCP_DBG("* SEQERR (%x EXP:%x) *\n", ntohl(tp->seqno), tpc->rcv_nxt);
                    if((tpc->rcv_nxt == (ntohl(tp->seqno) + 1))
                        || ((ntohl(tp->seqno) > tpc->rcv_nxt) && (((ntohl(tp->seqno)+1) == 0))))
                    {
                        TCP_DBG("* keep-alive *\n");
                        goto send_ack_nodata;
                    }
                    else
                    {
                        //TCP_DBG("* strange SEQ *\n");
                        /* XXX: 32bits wrap-around not handled yet */
                        if(tpc->rcv_nxt > ntohl(tp->seqno))
                        {
                            TCP_DBG("* OLD SEQNO *\n");
                            goto send_ack_nodata;
                        }
                        else
                        {
                            TCP_DBG("* FUTURE PKT *\n");
                            return 1;
                        }
                    }
                }

                ul = tpc->snd_nxt;
                if (ntohl(tp->ackno) != (ul+tpc->wait_ack))
                {
                    TCP_DBG("* ACKERR (%x EXP:%x) *\n", ntohl(tp->ackno), ul+tpc->wait_ack);
                    tcp_tx(tpc, TH_ACK, 0, tpc->wait_ack);
                    return 0;
                }

                tcp_dequeue_output_data_commit(tpc, tpc->wait_ack);

                ip_addl(ul, tpc->wait_ack);
                tpc->snd_nxt = ul;
                tpc->rxflag = tp->flags;
                ul = ntohs(ip->len) - IP_HEADER_LEN - (tp->tcpoffset / 4);
                tpc->rxbuf = (unsigned char *) tp + (tp->tcpoffset / 4);
                tpc->rxlen = ul;
                tpc->txlen = 0;
                ip_addl(tpc->rcv_nxt, ul);

#ifdef CONFIG_SIMPLE_CMD_WEBPAGE
                if(is_http_port)
                {
                    if (http_in_used++)
                    {
                        printk("\n\nHTTP busy!\n");
                        t = -1;
                    }
                    else
                    {
                        t = httpd_handle(tpc);
                        http_in_used = 0;
                    }
                }
#endif
#if defined(CONFIG_TELNETD)
                if(is_telnet_port)
                {
                    t = telnetd_handle(tpc);
                }
#endif
                if (t)
                {
                    tp->flags = TH_ACK | TH_FIN;
                    //tpc->wait_ack = 1;
                    tpc->state = CLOSED;
                    TCP_DBG("tcp app, rc=%d\n", t);
                    goto send_ack_nodata;
                }

                if ((payload_length = tpc->txlen))
                    tcp_queue_output_data(tpc, tpc->txbuf, payload_length);

                //tpc->wait_ack = payload_length;

                if (tpc->state == CLOSED)
                {
                    tp->flags |= TH_FIN;
                }
                goto send_ack;
                break;
            }
#if defined(CONFIG_WIFI) && !defined(WLA_TEST)
            else
            {
                if (http_state.state == HTTP_ONE_KEY_UPLOAD)
                {
                    //printf("state UPLOAD\n");
                    break;
                }
                else
                {
                    //printf("ignore tcp data from the other port\n");
                }
            }
#endif

        case CLOSED:
            if (tpc->rport && (ntohs(tp->src) != tpc->rport) && !(tp->flags & TH_SYN))
            {
#if 1 //!defined(CONFIG_WIFI) || defined(WLA_TEST)
                TCP_DBG("tcp_close_connection\n");
                tp->flags = (TH_FIN | TH_RST | TH_ACK);
                tpc_tmp.rcv_nxt = ntohl(tp->seqno);
                ip_addl(tpc_tmp.rcv_nxt, 1);
                tpc_tmp.snd_nxt = ntohl(tp->ackno);
                tpc_tmp.rport = ntohs(tp->src);
                tpc_tmp.lport = ntohs(tp->dest);
                tpc_tmp.state = CLOSED;
                tpc_tmp.wait_ack = 1;

                tpc_tmp.rip = ip->src.s_addr;

                ts = ntohs(tp->dest);
                tp->dest = tp->src;
                tp->src = htons(ts);

                tp->seqno = htonl(tpc_tmp.snd_nxt);
                tp->ackno = htonl(tpc_tmp.rcv_nxt);
                tp->tcpoffset = 6 << 4;
                tp->window = htons(MTU);
                sp = (short *) (tp + 1);
                *sp++ = htons(0x0204);
                *sp++ = htons(MTU);
                tp->chksum = 0;
                tp->chksum =
                    ~chksum_pseudo(tp, (unsigned int *) &tpc_tmp.rip,
                                   (unsigned int *) &ip->dest.s_addr,
                                   IP_TCP, 24);
                t = ip_tx(ntohl(tpc_tmp.rip), IP_TCP, 44, ip, NULL);
#endif
                return 0;
            }

            if (tp->flags & TH_SYN)
            {
                tp->flags = TH_SYN | TH_ACK;
                tpc->rcv_nxt = ntohl(tp->seqno);
                ip_addl(tpc->rcv_nxt, 1);
                tpc->snd_nxt = init_ss;
                tpc->rport = ntohs(tp->src);
                tpc->lport = ntohs(tp->dest);
                tpc->state = SYN_SENT;
                tpc->wait_ack = 1;

                tpc->rip = ip->src.s_addr;

                ts = ntohs(tp->dest);
                tp->dest = tp->src;
                tp->src = htons(ts);

                tp->seqno = htonl(tpc->snd_nxt);
                tp->ackno = htonl(tpc->rcv_nxt);
                tp->tcpoffset = 6 << 4;
                tp->window = ntohs(MTU);
                sp = (short *) (tp + 1);
                *sp++ = htons(0x0204);
                *sp++ = htons(MTU);
                tp->chksum = 0;
                tp->chksum =
                    ~chksum_pseudo(tp, (unsigned int *) &tpc->rip,
                                   (unsigned int *) &ip->dest.s_addr, IP_TCP,
                                   24);
                t = ip_tx(ntohl(tpc->rip), IP_TCP, 44, ip, NULL);
                if (!t)
                    tpc->state = CLOSED;
            }
            else if (tp->flags & TH_FIN)
            {
                tp->flags = TH_ACK;
                tpc_tmp.rcv_nxt = ntohl(tp->seqno);
                ip_addl(tpc_tmp.rcv_nxt, 1);
                tpc_tmp.snd_nxt = ntohl(tp->ackno);
                tpc_tmp.rport = ntohs(tp->src);
                tpc_tmp.lport = ntohs(tp->dest);
                tpc_tmp.state = CLOSED;
                tpc_tmp.wait_ack = 1;

                tpc_tmp.rip = ip->src.s_addr;

                ts = ntohs(tp->dest);
                tp->dest = tp->src;
                tp->src = htons(ts);

                tp->seqno = htonl(tpc_tmp.snd_nxt);
                tp->ackno = htonl(tpc_tmp.rcv_nxt);
                tp->tcpoffset = 6 << 4;
                tp->window = htons(MTU);
                sp = (short *) (tp + 1);
                *sp++ = htons(0x0204);
                *sp++ = htons(MTU);
                tp->chksum = 0;
                tp->chksum =
                    ~chksum_pseudo(tp, (unsigned int *) &tpc_tmp.rip,
                                   (unsigned int *) &ip->dest.s_addr, IP_TCP,
                                   24);
                t = ip_tx(ntohl(tpc_tmp.rip), IP_TCP, 44, ip, NULL);
            }
            else if (tp->flags & TH_PUSH)
            {
                ul = tpc->snd_nxt;
                ip_addl(ul, tpc->wait_ack);
                tpc->snd_nxt = ntohl(tp->ackno);
                tpc->rxflag = tp->flags;
                ul = ntohs(ip->len) - 20 - (tp->tcpoffset / 4);
                tpc->rxbuf = (unsigned char *) tp + (tp->tcpoffset / 4);
                tpc->rxlen = ul;
                tpc->txlen = 0;
                ip_addl(tpc->rcv_nxt, 1);

                if ((payload_length = tpc->txlen))
                    tcp_queue_output_data(tpc, tpc->txbuf, payload_length);

                tp->flags = TH_ACK;
                goto send_ack;
            }
            else
            {
#if 1
                return 0;
#else
                ul = tpc->snd_nxt;
                ip_addl(ul, tpc->wait_ack);
                tpc->snd_nxt = ul;
                tpc->rxflag = tp->flags;
                ul = ntohs(ip->len) - 20 - (tp->tcpoffset / 4);
                tpc->rxbuf = (unsigned char *) tp + (tp->tcpoffset / 4);
                tpc->rxlen = ul;
                tpc->txlen = 0;
                ip_addl(tpc->rcv_nxt, ul);
#if 0
                if ((payload_length = tpc->txlen))
                    tcp_queue_output_data(tpc, tpc->txbuf, payload_length);
#endif

                tpc->wait_ack = 0;
                tcp_clear_output_data(tpc);

                tp->flags = TH_ACK | TH_FIN;
                goto send_ack;
#endif
            }
            break;
    }
    return 0;

  send_ack_nodata:
    tcp_tx(tpc, tp->flags, 0, 0);
    //TCP_DBG("* TX ACK:%x SEQ:%x\n", ntohl(tp->ackno), ntohl(tp->seqno));
    return 0;

  send_ack:
    tcp_tx(tpc, (tp->flags | TH_PUSH), 0, (MTU/2));
    //TCP_DBG("* TX ACK2:%x SEQ:%x\n",tpc->state, ntohl(tp->ackno), ntohl(tp->seqno));
    return 0;
}

/*!
 * function:
 *
 *  \brief
 *  \param dip
 *  \param len
 *  \param timeout
 *  \return
 */
int net_ping(unsigned long dip, int len, int timeout)
{
    unsigned int time;
    int i;
    unsigned char *buf = (unsigned char *) &tp;
    static unsigned int ping_seqnum;
    struct icmphdr *icmp = (struct icmphdr *) (buf + IPHDR_SZ);

    icmp->type = ICMP_ECHO_REQUEST;
    icmp->part1 = ntohs(ping_seqnum >> 16);
    icmp->part2 = ntohs(ping_seqnum & 0xffff);

    for (i = sizeof (struct icmphdr) + IPHDR_SZ; i < len; i++)
        buf[i] = (unsigned char) i;

    icmp->chksum = 0;
    icmp->chksum = ~ipchksum((unsigned short *) icmp, len - IPHDR_SZ);

    await_reply(AWAIT_QDRAIN, 0, 0);

    printf("sq=%d sz=%d ", ping_seqnum, len);
    time = clock_get();
    ip_tx(dip, IP_ICMP, len, buf, NULL);

    i = await_reply(AWAIT_PING, &ping_seqnum, timeout);
    switch (i)
    {
        case ESC:
            printf("break\n");
            break;
        case 1:
            printf("rtt=%d ms\n", how_long(time));
            break;
        default:
            printf("time out\n");
    }
    ping_seqnum++;
    return i;
}

/*!
 * function:
 *
 *  \brief
 *  \param argc
 *  \param argv
 *  \return
 */
int cmd_ping(int argc, char *argv[])
{
    int to = PING_TIMEOUT;
    int rc, i, size = MIN_PING_SZ, iter = 4, pass = 0, mode = 0, miss;
    unsigned int dip = htonl(bootvars.server);

    if (0 < argc)
    {
        if ((i = inet_aton(argv[0], (void *)&dip)))
        {
            printf("error ip dip\n");
            goto err;
        }
    }

    if (1 < argc)
    {
        if (!hextoul(argv[1], &size))
        {
            printf("error ping size\n");
            goto err;
        }
        if (MAX_PING_SZ < size)
        {
            size = MAX_PING_SZ;
            printf("size > max, force to %d\n", MAX_PING_SZ);
        }
    }
    if (2 < argc)
        hextoul(argv[2], &iter);
    if (3 < argc)
        hextoul(argv[3], &to);

    printf("dip:%s to=%d ", inet_ntoa(&dip), to);
    if (size > 0)
        printf("sz=%d\n", size);
    else
    {
        printf("sz=46~1466\n");
        mode = 1;
        size = MIN_PING_SZ;
    }

    eth_open(0);
    for (i = 0; i < iter; i++)
    {
        rc = net_ping(ntohl(dip), size, to);
        if (1 == rc)
            pass++;
        else if (ESC == rc)
            break;
        if (mode)
        {
            if (size++ > (MAX_PING_SZ + 1))
                size = MIN_PING_SZ;
        }
    }
    miss = (iter - pass);
    printf("tx=%d rx=%d , lost=%d.%02d%%\n", iter, pass,
           miss * 100 / iter, ((unsigned long) miss * 100 % iter) * 100 / iter);
    return ERR_OK;

  err:
    printf("ping ip size num timeout\n");
    return ERR_PARM;
}

/*!
 * function:
 *
 *  \brief  TFTP - Download extended BOOTP data, or kernel image
 *  \param name
 *  \param func
 *  \return
 */
int tftp(const char *name, int (*func) (unsigned char *, int, int, int))
{
    static unsigned short iport = 2000;
    unsigned short oport = 0, len, block = 0, prevblock = 0, txblk = 0;
    int bcounter = 0, retry = 0, timeout, result;
    struct tftp_t *tr;
    struct nbuf *pkt;
    int packetsize = TFTP_DEFAULTSIZE_PACKET;
    unsigned long server = bootvars.server;
    unsigned long data;
    short put = (func == upload_file);
    int upload_count = 0;

    await_reply(AWAIT_QDRAIN, 0, 0);
    tp.opcode = put ? htons(TFTP_WRQ) : htons(TFTP_RRQ);
    len = 1 + sizeof (tp.ip) + sizeof (tp.udp) +
        sizeof (tp.opcode) + sprintf((char *) tp.u.rrq, "%s%coctet", name, 0);
    if (0 == udp_tx(server, ++iport, TFTP_PORT, len, &tp))
        return (0);

    while (1)
    {
        timeout = rfc951_sleep(block ? TFTP_REXMT : TIMEOUT, retry);
        data = (unsigned long) iport;
        result = await_reply(AWAIT_TFTP, &data, timeout);
        if (ESC == result)
            return 0;
        if (0 == result)
        {
            tftp_retry++;
            if (!block && (MAX_TFTP_RETRIES > retry++))
            {
                if (0 == udp_tx(server, ++iport, TFTP_PORT, len, &tp))
                    return (0);
                continue;
            }
            if (put)
                goto retry;

            if (block && ((retry += TFTP_REXMT) < TFTP_TIMEOUT))
            {
                udp_tx(server, iport, oport, TFTP_MIN_PACKET, &tp);     /* re-send ack */
                continue;
            }
            break;
        }

        pkt = (struct nbuf *) data;
        tr = (struct tftp_t *) &pkt->data[ETHER_HEAD_SZ];
        if (ntohs(TFTP_ERROR) == tr->opcode)
        {
            printf("Err %d (%s)\n", ntohs(tr->u.err.errcode), tr->u.err.errmsg);
            break;
        }
        //download , ack
        if (tr->opcode == ntohs(TFTP_OACK))
        {
            char *e, *p = tr->u.oack.data;
            if (prevblock)
                continue;       /* skip */
            len = ntohs(tr->udp.len) - UDPHDR_SZ - 2;
            if (len > TFTP_MAX_PACKET)
                goto noak;
            e = p + len;
            while (*p != '\0' && p < e)
            {
                if (!strcasecmp("blksize", p))
                {
                    p += 8;
                    if ((packetsize =
                         strtol(p, &p, 10)) < TFTP_DEFAULTSIZE_PACKET)
                        goto noak;
                    while (p < e && *p)
                        p++;
                    if (p < e)
                        p++;
                }
                else
                {
                  noak:
                    tp.opcode = htons(TFTP_ERROR);
                    tp.u.err.errcode = 8;
                    len =
                        sizeof (tp.ip) + sizeof (tp.udp) + sizeof (tp.opcode) +
                        sizeof (tp.u.err.errcode) +
                        sprintf((char *) tp.u.err.errmsg, "RFC1782 error") + 1;
                    udp_tx(server, iport, ntohs(tr->udp.src), len, &tp);
                    return 0;
                }
            }
            if (p > e)
                goto noak;
            block = tp.u.ack.block = 0;
        }                       // request, response
        else                    //data
        if (tr->opcode == htons(TFTP_DATA))
        {
            len = ntohs(tr->udp.len) - UDPHDR_SZ - 4;
            if (len > packetsize)
                continue;
            tp.u.ack.block = tr->u.data.block;
            block = ntohs(tp.u.ack.block);
        }
        else /* upload ack */ if (tr->opcode == ntohs(TFTP_ACK))
        {
            static int prev_sent;
            block = ntohs(tr->u.ack.block);
            if (block == txblk)
                retry = 0;
            else
            {
                printf("blk# dis-continuous txblk=%d, ack-blk=%d\n", txblk,
                       block);
                len = prev_sent;
                tftp_retry++;
                if (retry++ < 16)
                {
                    img.addr -= prev_sent;
                    goto retry;
                }
                else
                {
                    printf("max retry fail\n");
                    return 0;
                }
            }

            if (0 >= img.count)
                return 1;

            txblk++;
            tp.u.data.block = htons(txblk);
            upload_count = (img.count >
                            TFTP_DEFAULTSIZE_PACKET) ? TFTP_DEFAULTSIZE_PACKET :
                img.count;
            img.count -= upload_count;

            tp.opcode = htons(TFTP_DATA);
            prev_sent = len = upload_count + sizeof (tp.ip) + sizeof (tp.udp) +
                sizeof (tp.opcode) + sizeof (tp.u.data.block);
            memcpy((char *) tp.u.data.download, (char *) img.addr,
                   upload_count);
            oport = ntohs(tr->udp.src);
          retry:
            udp_tx(server, iport, oport, len, &tp);
            upload_file(0, 0, upload_count, 0);
            continue;
        }
        if ((block || bcounter) && (block != prevblock + 1))
        {                       /* Block order should be continuous */
            block = prevblock;
            tp.u.ack.block = htons(block);
        }
        tp.opcode = htons(TFTP_ACK);
        oport = ntohs(tr->udp.src);
        udp_tx(server, iport, oport, TFTP_MIN_PACKET, &tp);
        if ((unsigned short) (block - prevblock) != 1)
            continue;
        prevblock = block;
        retry = 0;
        result = func((unsigned char *)tr->u.data.download, ++bcounter, (int)len, len < packetsize);
        if (len < packetsize)
        {
#if defined(CONFIG_CMD_FLASH) && defined(CONFIG_FLASH_CMD_PROGRAM_AFTER_DOWNLOAD_FILE)
            downloaded = 1;
#endif
            return (1);
        }
    }
    return (0);
}

/*!
 * function:
 *
 *  \brief
 *  \param data
 *  \param block
 *  \param len
 *  \param eof
 *  \return
 */
static int download_file(unsigned char *data, int block, int len, int eof)
{
    static int count;
    static unsigned int total;
    if (block == 1)
    {
        count = 0;
        total = 0;
    }

    if (++count % 32 == 0)
        putchar('.');

    memcpy((void *) img.addr, (void *) data, len);
    img.addr += len;
    total += len;

    if (!eof)
        return 0;
    printf("\nrx len=%d (0x%x)\n", total, total);
    byte_count = total;
    return (1);
}

/*!
 * function:
 *
 *  \brief
 *  \param data
 *  \param block
 *  \param len
 *  \param eof
 *  \return
 */
static int upload_file(unsigned char *data, int block, int len, int eof)
{
    static int count;
    if (block == 1)
        count = 0;

    if (++count % 32 == 0)
        putchar('.');

    img.addr += len;
    return -1;
}

/*!
 * function:
 *
 *  \brief
 *  \param packet
 *  \param brcst
 *  \return
 */

int eth_unmatch_da(unsigned char *packet, short brcst)
{
    /* not for me (DA), drop */
    if (!memcmp
        ((void *) &packet[0],
         (void *) &(arptable[ARP_CLIENT].node), ETHER_ADDR_SZ) != 0)
        return 0;
    if (brcst && !memcmp((void *)&packet[0], (void *)&broadcast[0], ETHER_ADDR_SZ))
        return 0;
    return 1;
}

/*!
 * function:
 *
 *  \brief
 *  \param type
 *  \param data
 *  \param timeout
 *  \return
 */
static struct nbuf *last_ipkt;
int await_reply(int type, void *data, int timeout)
{
    unsigned long time;
    struct iphdr *ip;
    struct udphdr *udp;
    struct arprequest *arpreply;
    unsigned short ptype;
    struct nbuf *ipkt = NULL;
    unsigned char *packet;
    unsigned short packetlen;
    unsigned long ipaddr;

    unsigned int protohdrlen = ETHER_HEAD_SZ + IPHDR_SZ + UDPHDR_SZ;
    int key;
    int rc = 0;

    time = clock_get();
    for (;;)
    {
        if (0 == (ipkt = eth_rx()))
        {
            tcp_tx_handle();

            if ((type != AWAIT_IDLE) && tstc())
            {
                key = getchar();
                if (3 == key || key == ESC)
                    return ESC;
            }

            if (timeout == 0)
                break;

            if (how_long(time) > timeout)
            {
                break;
            }
            continue;
        }
        if (type == AWAIT_QDRAIN)
            goto free;
        if (ipkt->data[12] == (VLAN >> 8))
        {
            memmove((void *)(ipkt->data + 15), (void *)(ipkt->data + 11), 12);
            ipkt->data += 4;
            ipkt->len -= 4;
        }
        packet = ipkt->data;
        packetlen = ipkt->len;
        ptype =
            ((unsigned short) packet[12]) << 8 | ((unsigned short) packet[13]);

        if (ARP == ptype)
        {
            if (packetlen < (ETHER_HEAD_SZ + sizeof (struct arprequest)))
                goto free;
            unsigned long tmp;
            arpreply = (struct arprequest *) &packet[ETHER_HEAD_SZ];
            if (arpreply->opcode == htons(ARP_REPLY))
            {
                int ival;
                if (type != AWAIT_ARP)
                    goto free;
                ival = *(int *) data;
                if ((arptable[ival].ipaddr.s_addr) !=
                    ((unsigned long)
                     IP_ADDR(arpreply->sipaddr[0], arpreply->sipaddr[1],
                             arpreply->sipaddr[2], arpreply->sipaddr[3])))
                    goto free;
                memcpy(arptable[ival].node, arpreply->shwaddr, ETHER_ADDR_SZ);
                rc = 1;
                goto free_ret;
            }

            tmp =
                IP_ADDR(arpreply->tipaddr[0], arpreply->tipaddr[1],
                        arpreply->tipaddr[2], arpreply->tipaddr[3]);
            if ((arpreply->opcode == htons(ARP_REQUEST))
                && (tmp == arptable[ARP_CLIENT].ipaddr.s_addr))
            {
                arpreply->opcode = htons(ARP_REPLY);
                memcpy(arpreply->tipaddr, arpreply->sipaddr, sizeof (in_addr));
                memcpy(arpreply->thwaddr, arpreply->shwaddr, ETHER_ADDR_SZ);
                ipaddr = cpu_to_be32(arptable[ARP_CLIENT].ipaddr.s_addr);
                memcpy(arpreply->sipaddr, &ipaddr, sizeof (in_addr));
                memcpy(arpreply->shwaddr, arptable[ARP_CLIENT].node,
                       ETHER_ADDR_SZ);
                eth_tx((const char *)arpreply->thwaddr, ARP, sizeof (struct arprequest),
                       (const void *)arpreply);
            }
            goto free;
        }
        if (eth_unmatch_da(&packet[0], 1))      // ??? already matched before eth_rx
            goto free;

        if ((packetlen < protohdrlen) || (ptype != IP))
            goto free;

        ip = (struct iphdr *) &packet[ETHER_HEAD_SZ];
        if ((ip->verhdrlen != 0x45))
            goto free;
        if (0xffff != ipchksum((unsigned short *) ip, IPHDR_SZ))
            goto free;

        // not to me
        if ((arptable[ARP_CLIENT].ipaddr.s_addr) != (ntohl(ip->dest.s_addr)))
        {
#if defined(CONFIG_DHCPD)
            // to avoid drop packet of DHCP protocol
            if (ip->protocol == IP_UDP)
                goto udp;
#endif
            goto free;
        }

        if (ip->protocol == IP_ICMP)
        {
            int chksum;
            struct icmphdr *icmp = (struct icmphdr *) (ip + 1);
            if (icmp->type == ICMP_ECHO_REPLY)
            {
                unsigned int s1;
                if (type != AWAIT_PING)
                    goto free;

                s1 = ntohs(icmp->part1);
                s1 = ((s1 << 16) & 0xffff0000) | ntohs(icmp->part2);
                if (s1 != *(unsigned int *) data)
                    printf("seq=%08x != %08x", s1, *(unsigned int *) data);

                key =
                    (0 ==
                     (chksum =
                      (0xffff ^
                       ipchksum((unsigned short *) icmp,
                                packetlen - ETHER_HEAD_SZ - IPHDR_SZ))));

                rc = key;
                goto free_ret;
            }
            else if (icmp->type == ICMP_ECHO_REQUEST)
            {
                char tmp[4];
                icmp->type = ICMP_ECHO_REPLY;
                memcpy((void *) tmp, (void *) &(ip->src.s_addr), 4);
                memcpy((void *) &(ip->src.s_addr), (void *) &(ip->dest.s_addr),
                       4);
                memcpy((void *) &(ip->dest.s_addr), (void *) tmp, 4);

                ip->chksum = 0;
                ip->chksum = ~ipchksum((unsigned short *) ip, IPHDR_SZ);
                icmp->chksum = 0;
                icmp->chksum = ~ipchksum((unsigned short *) icmp,
                                         packetlen - ETHER_HEAD_SZ - IPHDR_SZ);
                eth_tx((const char *)(packet + 6), (unsigned int)IP, (unsigned int)(packetlen - ETHER_HEAD_SZ),
                       (const void *)(packet + ETHER_HEAD_SZ));
            }
            goto free;
        }

        /* UDP */
#if defined(CONFIG_DHCPD)
udp:
        if (ip->protocol == IP_UDP)
        {
            unsigned short ival;
            udp = (struct udphdr *) (ip + 1);
            unsigned int src_addr, dest_addr;

            if (type == AWAIT_TFTP)
            {
                ival = (unsigned short) (*(unsigned long *) data);
                if (ntohs(udp->dest) == ival)
                {
                    *(unsigned int *) data = (unsigned int) ipkt;
                    rc = 1;
                    goto free_ret;
                }
            }

            // to check whether the packet is used for DHCP DISCOVERY/REQUEST
            src_addr = ntohl(ip->src.s_addr);
            dest_addr = ntohl(ip->dest.s_addr);

            if (!memcmp((void *)&src_addr, (void *)&zeros[0], 4)
                && !memcmp((void *)&dest_addr, (void *)&broadcast[0], 4))
            {
                udp_appcall(&packet[ETHER_HEAD_SZ + IPHDR_SZ + UDPHDR_SZ],
                                  packetlen - (ETHER_HEAD_SZ + IPHDR_SZ + UDPHDR_SZ));
                goto free_ret;
            }
        }
#else
        if (ip->protocol == IP_UDP)
        {
            unsigned short ival;
            udp = (struct udphdr *) (ip + 1);

            if (type == AWAIT_TFTP)
            {
                ival = (unsigned short) (*(unsigned long *) data);
                if (ntohs(udp->dest) == ival)
                {
                    *(unsigned int *) data = (unsigned int) ipkt;
                    rc = 1;
                    goto free_ret;
                }
            }
        }
#endif
        else if (ip->protocol == IP_TCP)
        {
            if(1==tcp_rx_handle(ipkt))
            {
                if(last_ipkt)
                    nbuf_put(last_ipkt);
                last_ipkt = ipkt;
                ipkt = NULL;
            }
            else
            {
                if(last_ipkt)
                    tcp_rx_handle(last_ipkt);
            }
        }

      free:
        if(ipkt)
        {
            nbuf_put(ipkt);
            ipkt = NULL;
        }
        continue;
    }
    return (0);
  free_ret:
    if(ipkt)
    {
        nbuf_put(ipkt);
        ipkt = NULL;
    }
    return rc;

}

/*!
 * function:
 *
 *  \brief  IPCHKSUM - Checksum IP Header
 *  \param ip
 *  \param len
 *  \return
 */

unsigned short ipchksum_align(ip, len)
register unsigned short *ip;
register int len;
{
    unsigned long sum = 0;
    len >>= 1;
    while (len--)
    {
        sum += *(ip++);
        if (sum > 0xFFFF)
            sum -= 0xFFFF;
    }
    return (sum & 0x0000FFFF);
}

/*!
 * function:
 *
 *  \brief  above function not considering un-aligned to 2
 *  \param data
 *  \param len
 *  \return
 */

unsigned short ipchksum(void *data, int len)
{
    unsigned short *ip = (unsigned short *) data;
    if (len & 1)
        ((unsigned char *) ip)[len++] = 0;
    return ipchksum_align(data, len);
}

#define ADD32_16(acc, d)    \
    acc +=  d & 0xffff; \
    acc += (d >> 16) & 0xffff;

/*!
 * function:
 *
 *  \brief
 *  \param data
 *  \param src
 *  \param dst
 *  \param prot
 *  \param len
 *  \return
 */
unsigned short chksum_pseudo(void *data, unsigned int *src, unsigned int *dst,
                             unsigned char prot, int len)
{
    unsigned int sum = 0;
    sum = (htons(len));
    sum += (htons(prot));
    ADD32_16(sum, *dst);
    ADD32_16(sum, *src);
    sum += ipchksum(data, len) & 0xffff;

    while (sum >> 16)
    {
        sum = (sum & 0xffff) + (sum >> 16);
    }

    return (unsigned short) sum;
}

static unsigned long next = 1;
#define RAND_MAX 32767
/*!
 * function:
 *
 *  \brief
 *  \return
 */
void rand_init(void)
{
    unsigned long now;
    asm volatile ("mfc0 %0, $9":"=r" (now):);
    next = now;
}

/*!
 * function:
 *
 *  \brief
 *  \return
 */
int rand(void)
{
    next = next * 1103515245 + 12345;
    return ((unsigned) (next / 65536) % RAND_MAX);
}

/*!
 * function:
 *
 *  \brief
 *  \param base
 *  \param exp
 *  \return
 */

int rfc951_sleep(int base, int exp)
#if 0
void rfc951_sleep(exp)
int exp;
#endif
{
//  static long seed = 0;
#if 0
    long q;
#endif
    unsigned long tmo;

#ifdef BACKOFF_LIMIT
    if (exp > BACKOFF_LIMIT)
        exp = BACKOFF_LIMIT;
#endif
#if 0
    if (!seed)                  /* Initialize linear congruential generator */
        seed = clock_get() + *(long *) &arptable[ARP_CLIENT].node
            + ((short *) arptable[ARP_CLIENT].node)[2];
    /* simplified version of the LCG given in Bruce Scheier's
       "Applied Cryptography" */
    q = seed / 53668;
    if ((seed = 40014 * (seed - 53668 * q) - 12211 * q) < 0)
        seed += 2147483563l;
    tmo = (base << exp) + (TICKS_PER_SEC - (seed / TWO_SECOND_DIVISOR));
#endif
    tmo = (base << exp) + (CONFIG_SYS_HZ * rand() / 32767);
    return tmo;
#if 0
    /* compute mask */
    for (tmo = 63; tmo <= 60 * TICKS_PER_SEC && --exp > 0; tmo = 2 * tmo + 1) ;
    /* sleep */
    printf("<sleep>\n");
    for (tmo = (tmo & seed) + currticks(); currticks() < tmo;)
        if (iskey() && (getchar() == ESC))
            longjmp(jmp_bootmenu, 1);
    return;
#endif
}

#ifdef  CONFIG_CMD_TFTP

/*!
 * function:
 *
 *  \brief
 *  \param
 *  \return
 */
int cmd_tftp(int argc, char *argv[])
{
    unsigned int buf = bootvars.load_addr;
    char *fname = bootvars.file;
    int ac = 0;
    int loop = 1;    // set default do only once
    int i = 0;
    int total_tftp_retry = 0;
    int ret = ERR_OK;

    if (argc == 0)
        goto tftp;

    if (argv[0][0] == 'p')
        ac = 1;

    if (argc < 2)
        goto tftp;
    fname = argv[1];
    if (argc < 3)
        goto tftp;
    if (!hextoul(argv[2], &buf))
        goto err;
    if (argc < 4)
        goto tftp;
    if (!hextoul(argv[3], &byte_count))
        goto err;
    if (argc < 5)
    {
        goto tftp;
    }
    if (!sscanf(argv[4], "%d", (unsigned *)&loop))
    {
        goto err;
    }
tftp:
    eth_open(0);
    for (i = 0; i < loop; i++)
    {
        tftp_retry = 0;
        if (!net_tftp(ac, buf, byte_count, fname))
            ret = ERR_FILE;

        if (ret)
        {
            break;
        }

        total_tftp_retry += tftp_retry;

        if (loop > 1)
        {
            printf("[Loop: %d] retry count = %d\n", i + 1, tftp_retry);
        }
    }
    if (loop > 1)
    {
        printf("[Total] tftp retry count = %d\n", total_tftp_retry);
    }
    return ret;
err:
    printf("tftp <get/put> <file> <buf> <len> <retry_count>\n");
    return ERR_PARM;
}

/*!
 * function:
 *
 *  \brief
 *  \param
 *  \return
 */

int cmd_nbuf(int argc, char **argv)
{
    nbuf_state();
    return ERR_OK;
}

cmdt cmdt_tftp[] __attribute__ ((section("cmdt"))) =
{
    {
    "tftp", cmd_tftp, "tftp <get/put> <file> <buf> <len> <retry_count>; tftp"}
    ,
    {
    "ping", cmd_ping, "ping <ipaddr> <len> <count> <timeout> ;Ping"}
    ,
    {
    "nbuf", cmd_nbuf, "nbuf info"}
,};
#endif
