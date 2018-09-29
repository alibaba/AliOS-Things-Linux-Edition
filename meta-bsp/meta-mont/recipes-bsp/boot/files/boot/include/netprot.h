/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file netprot.h
*   \brief Network Porotcol API
*   \author Montage
*/
#ifndef __NET_PROT_H__
#define __NET_PROT_H__

#include <byteorder.h>

#define ntohl(x) be32_to_cpu(x)
#define htonl(x) cpu_to_be32(x)
#define ntohs(x) be16_to_cpu(x)
#define htons(x) cpu_to_be16(x)

#define IP_ADDR(a,b,c,d)    (a<<24)|(b<<16)|(c<<8)|(d)
#define ESC                 0x1b

#define MAX_TFTP_RETRIES    40
#define MAX_ARP_RETRIES     20
#define TICKS_PER_SEC       1000        //(CONFIG_SYS_CLK/CONFIG_SYS_CLK_PR)
#define TIMEOUT             (10*TICKS_PER_SEC)
#define TFTP_TIMEOUT        (30*TICKS_PER_SEC)
#define TFTP_REXMT          (3*TICKS_PER_SEC)
#ifndef NULL
#define NULL                ((void *)0)
#endif
#define TWO_SECOND_DIVISOR  (2147483647l/TICKS_PER_SEC)

enum
{
    ETHER_ADDR_SZ = 6,
    ETHER_HEAD_SZ = 14,
    IP = 0x0800,
    ARP = 0x0806,
    VLAN = 0x8100,

    TCP_TELNET = 23,
    TCP_HTTP = 80,
    BOOTP_SERVER = 67,
    BOOTP_CLIENT = 68,
    TFTP_PORT = 69,

    IP_ICMP = 1,
    IP_UDP = 17,
    IP_TCP = 6,
    IP_BROADCAST = 0xFFFFFFFF,

    ICMP_ECHO_REQUEST = 8,
    ICMP_ECHO_REPLY = 0,

    ARP_REQUEST = 1,
    ARP_REPLY = 2,

    BOOTP_REQUEST = 1,
    BOOTP_REPLY = 2,

    PING_TIMEOUT = 3000,

    TFTP_DEFAULTSIZE_PACKET = 512,
    TFTP_MAX_PACKET = 1432,

    TFTP_RRQ = 1,
    TFTP_WRQ = 2,
    TFTP_DATA = 3,
    TFTP_ACK = 4,
    TFTP_ERROR = 5,
    TFTP_OACK = 6,

    TFTP_CODE_EOF = 1,
    TFTP_CODE_MORE = 2,
    TFTP_CODE_ERROR = 3,
    TFTP_CODE_BOOT = 4,
    TFTP_CODE_CFG = 5,

    AWAIT_ARP = 0,
    AWAIT_BOOTP = 1,
    AWAIT_TFTP = 2,
    AWAIT_RARP = 3,
    AWAIT_QDRAIN = 5,
    AWAIT_PING = 6,
    AWAIT_IDLE = 7,

    ARP_CLIENT = 0,
    ARP_SERVER = 1,
    ARP_GATEWAY = 2,
    ARP_TEMP = 5,
    MAX_ARP = ARP_TEMP + 1,
};

enum
{
    TH_FIN = 0x01,
    TH_SYN = 0x02,
    TH_RST = 0x04,
    TH_PUSH = 0x08,
    TH_ACK = 0x10,
    TH_URG = 0x20,

    CLOSED = 0,
    SYN_RCVD = 1,
    SYN_SENT = 2,
    ESTABLISHED = 3,
    FIN_WAIT_1 = 4,
    FIN_WAIT_2 = 5,
    CLOSING = 6,
    TIME_WAIT = 7,
    LAST_ACK = 8,

    HTTP_NOGET = 0,
    HTTP_FUNC = 1,
    HTTP_UPLOAD = 2,
    HTTP_ONE_KEY_UPLOAD = 3,
};

typedef struct
{
    unsigned long s_addr;
} in_addr;

struct arptable_t
{
    in_addr ipaddr;
    unsigned char node[6];
};

struct arprequest
{
    unsigned short hwtype;
    unsigned short protocol;
    unsigned char hwlen;
    unsigned char protolen;
    unsigned short opcode;
    unsigned char shwaddr[6];
    unsigned char sipaddr[4];
    unsigned char thwaddr[6];
    unsigned char tipaddr[4];
};

struct iphdr
{
    unsigned char verhdrlen;
    unsigned char service;
    unsigned short len;
    unsigned short ident;
    unsigned short frags;
    unsigned char ttl;
    unsigned char protocol;
    unsigned short chksum;
    in_addr src;
    in_addr dest;
};

struct udphdr
{
    unsigned short src;
    unsigned short dest;
    unsigned short len;
    unsigned short chksum;
};

struct tcphdr
{
    unsigned short src;
    unsigned short dest;
    unsigned int seqno;
    unsigned int ackno;
    unsigned char tcpoffset;
    unsigned char flags;
    unsigned short window;
    unsigned short chksum;
    unsigned short urgp;
};

struct icmphdr
{
    unsigned char type;         /* ICMP type field */
    unsigned char code;         /* ICMP code field */
    unsigned short chksum;      /* checksum */
    unsigned short part1;       /* depends on type and code */
    unsigned short part2;
};

struct tftp_t
{
    struct iphdr ip;
    struct udphdr udp;
    unsigned short opcode;
    union
    {
        char rrq[TFTP_DEFAULTSIZE_PACKET];
        struct
        {
            unsigned short block;
            char download[TFTP_MAX_PACKET];
        } data;
        struct
        {
            unsigned short block;
        } ack;
        struct
        {
            unsigned short errcode;
            char errmsg[TFTP_DEFAULTSIZE_PACKET];
        } err;
        struct
        {
            char data[TFTP_DEFAULTSIZE_PACKET + 2];
        } oack;
    } u;
} __attribute__ ((packed));

struct tftpreq_t
{
    struct iphdr ip;
    struct udphdr udp;
    unsigned short opcode;
    union
    {
        char rrq[512];
        struct
        {
            unsigned short block;
        } ack;
        struct
        {
            unsigned short errcode;
            char errmsg[512 - 2];
        } err;
        struct
        {
            unsigned short block;
            char download[TFTP_MAX_PACKET];
        } data;
    } u;
} __attribute__ ((packed));

#define TFTP_MIN_PACKET (sizeof(struct iphdr) + sizeof(struct udphdr) + 4)
#define MAX_PING_SZ     (TFTP_MAX_PACKET+14+20)
#define MIN_PING_SZ     46

struct tcp_conn_t
{
    unsigned long rcv_nxt;
    unsigned long snd_nxt;
    unsigned short lport;
    unsigned short rport;
    unsigned long rip;
    unsigned char *rxbuf;
    unsigned char *txbuf;
    unsigned short rxlen;
    unsigned short txlen;
    unsigned short wait_ack;
    char state;
    char rxflag;

    unsigned long dest_ip_addr;
#define TCP_OUTPUT_DATA_BUFSIZE 4096
    unsigned char output_data[TCP_OUTPUT_DATA_BUFSIZE];
    int od_read_idx;
    int od_write_idx;

    int is_http_port;
    int is_telnet_port;

    unsigned int last_tx_jiffies;
};

struct httpd_state
{
    unsigned char state;
    unsigned short count;
    char *dataptr;
};

struct upload_state
{
    int len;
    int current_ofs;
    int count;
    char boundary[64];
    short state;
};

typedef struct bootvar
{
    int mode;
    //unsigned int ver;
    unsigned char mac0[8];
    unsigned char mac1[8];
    unsigned char mac2[8];
    int vlan;

    unsigned int ip;
    unsigned int msk;
    unsigned int gw;
    unsigned int server;
    unsigned int load_sz;
    unsigned int load_addr;
    unsigned int load_src;
    unsigned int load_src2;
    unsigned int log_src;
    unsigned int log_sz;
    unsigned int id;
    unsigned int hver;
    unsigned int pll;
    unsigned int serial;
    char file[32];
    //char cver[16];
    char rfc[128];
    char txvga[128];
    char rxvga[128];
    char pin[16];
    int freq_ofs;
    char madc_val0[64];
    char madc_val1[64];
    char lna[32];
    int autocal;
    int quiet;
    char swcfg[32];
    char ai[16];
    int upgrade;
    int network;
    int ci_offset;
    int recovery_offset;
    int second_img_offset;
    int recovery;
    char txp_diff[16];
    char fem_product_id[8];
    int fem_en;
    char pinmux[256];
    unsigned int powercfg;
    char gpio_driving[256];
    char gpio_setting[256];
    unsigned int clkcfg;
    unsigned int watchdog_timer;
    unsigned char mic_gain_ctrl[8];
	char sd_root[16];
} bootvar;

#define     IPHDR_SZ    (sizeof(struct iphdr))
#define     UDPHDR_SZ   (sizeof(struct udphdr))
#define     TCPHDR_SZ   (sizeof(struct tcphdr))

extern int await_reply(int type, void *ptr, int timeout);
extern int rfc951_sleep(int base, int exp);
extern int tftp(const char *name, int (*)(unsigned char *, int, int, int));
extern int udp_tx(unsigned long destip, unsigned int srcsock,
                  unsigned int destsock, int len, const void *buf);

extern void *eth_rx(void);
extern void eth_tx(const char *d, unsigned int t, unsigned int s, const void *p);
extern void eth_disable(void);
extern void eth_reset(void);
extern int eth_probe(void);

extern bootvar bootvars;
extern struct net_device *dev;
extern struct arptable_t arptable[MAX_ARP];
extern char _start[], edata[], end[];

#ifdef CONFIG_SIMPLE_CMD_WEBPAGE
void httpd_init(void);
int httpd_handle(struct tcp_conn_t *tcps);
#endif
#ifdef CONFIG_TELNETD
void telnetd_init(void);
int telnetd_handle(struct tcp_conn_t *tcps);
#endif
void cheetah_phy_up(unsigned int lan_en, unsigned int wan_en);
int net_tftp(int put, unsigned long dest_address, unsigned int bytecount,
             char *file);
int update_mac();

#endif
