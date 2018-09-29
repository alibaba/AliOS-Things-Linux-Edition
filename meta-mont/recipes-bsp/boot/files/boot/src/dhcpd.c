/*
 * Copyright (c) 2015, mleaf mleaf90@gmail.com or 350983773@qq.com
 * All rights reserved.
 *
 * Modify this file to fit out demand of MINI_AP by Hsuan
 */

/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <dhcpd.h>
#include <common.h>
#include <lib.h>

/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
//#define CONFIG_NODE_LIST
#ifdef CONFIG_NODE_LIST
    #define STACK_MALLOC_NODE
#endif

struct dhcp_client *head_dhcpd;
typedef struct dhcp_client NODE;

#define TRUE    1  
#define FALSE   0  

#define STATE_INITIAL         0
#define STATE_SENDING         1
#define STATE_DHCP_DISCOVER   2
#define STATE_DHCP_REQUEST    3
#define STATE_DHCP_OVER       4
#define STATE_DHCP_DISCOVER_OVER       5     

struct dhcpd_msg
{
    u8 op, htype, hlen, hops;

    u8 xid[4];        //DHCP REQUEST 時產生的數值，以作 DHCPREPLY 時的依據

    u16 secs;         //Client 端啟動時間（秒）

    u16 flags;        //從 0 到 15 共 16 bits ，最左一 bit 為 1 時表示 server 將以廣播方式傳送封包給 client ，其余尚未使用。

    u8 ciaddr[4];     //要是 client 端想繼續使用之前取得之 IP 地址，則列於這裡。

    u8 yiaddr[4];     //從 server 送回 client 之 DHCP OFFER 與 DHCPACK封包中，此欄填寫分配給 client 的 IP 地址。

    u8 siaddr[4];     //若 client 需要透過網絡開機，從 server 送出之 DHCP OFFER、DHCPACK、DHCPNACK封包中，此欄填寫開機程序代碼所在 server 之地址。

    u8 giaddr[4];     //若需跨網域進行 DHCP 發放，此欄為 relay agent 的地址，否則為 0。

    u8 chaddr[16];    //Client 硬件地址。

    u8 sname[64];     //Server 名稱字符串，以 0x00 結尾。

    u8 file[128];     //若 client 需要透過網絡開機，此欄將指出開機程序名稱，稍後以 TFTP 傳送。

    u8 options[312];
};

/*
OP:
若是 client 送給 server 的封包，設為 1 ，反向為 2。
HTYPE:
硬件類別，Ethernet 為 1。
HLEN:
硬件地址長度， Ethernet 為 6。
HOPS:
若封包需經過 router 傳送，每站加 1 ，若在同一網內，為 0。
options:
允許廠商定議選項（Vendor-Specific Area)，以提供更多的設定信息（如：Netmask、Gateway、DNS、等等）。
其長度可變，同時可攜帶多個選項，每一選項之第一個 byte 為信息代碼，其後一個 byte 為該項數據長度，最後為項目內容。
CODE LEN VALUE 此字段完全兼容 BOOTP ，同時擴充了更多選項。其中，DHCP封包可利用編碼為 0x53 之選項來設定封包類別：
項值類別:
1 DHCP DISCOVER
2 DHCP OFFER
3 DHCP REQUEST
4 DHCPDECLINE
5 DHCPACK
6 DHCPNACK
7 DHCPRELEASE
*/
#define BOOTP_BROADCAST 0x8000

#define DHCP_REQUEST        1
#define DHCP_REPLY          2
#define DHCP_HTYPE_ETHERNET 1
#define DHCP_HLEN_ETHERNET  6
#define DHCP_MSG_LEN      236

#define DHCPC_SERVER_PORT  67
#define DHCPC_CLIENT_PORT  68

#define DHCPDISCOVER  1
#define DHCPOFFER     2
#define DHCPREQUEST   3
#define DHCPDECLINE   4
#define DHCPACK       5
#define DHCPNAK       6
#define DHCPRELEASE   7


#define DHCP_OPTION_SUBNET_MASK   1
#define DHCP_OPTION_ROUTER        3
#define DHCP_OPTION_DNS_SERVER    6
#define DHCP_OPTION_DOMAIN_NAME  15
#define DHCP_OPTION_REQ_IPADDR   50
#define DHCP_OPTION_LEASE_TIME   51
#define DHCP_OPTION_MSG_TYPE     53
#define DHCP_OPTION_SERVER_ID    54
#define DHCP_OPTION_REQ_LIST     55
#define DHCP_OPTION_END         255
#define DHCP_OPTION_OVER		0x34

/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
static struct dhcpd_state s;

//static  u32 server_ipaddr = (192<<24) | (168<<16) | (0<<8) | (1);
//static  u32 dhcpd_ipaddr = (192<<24) | (168<<16) | (0<<8) | (5);
//static  u32 uip_server_netmask = (255<<24) | (255<<16) | (255<<8) | (0);
//static  u32 uip_dhcp_leasetime = 3600;    //1h
static u32 server_ipaddr = 0;
static u32 dhcpd_ipaddr = 0;
static  u32 uip_server_netmask = 0;
static  u32 uip_dhcp_leasetime = 86400;    // 1 day

/*---------------------------------------------------------------------------*/

u32 *get_server_ipaddr(void)
{
    if(0==server_ipaddr)
        server_ipaddr = bootvars.ip;

    return &server_ipaddr;
}

/**
*add dhcp msg type
*/
static u8 *add_msg_type(u8 *optptr, u8 type)
{
    *optptr++ = DHCP_OPTION_MSG_TYPE;
    *optptr++ = 1;
    *optptr++ = type;
    return optptr;
}

/**
*add dhcp server id
*/
static u8 *add_dhcpd_server_id(u8 *optptr)
{
    u32 server_id;

    if(0==server_ipaddr)
        server_ipaddr = bootvars.ip;

    *optptr++ = DHCP_OPTION_SERVER_ID;
    *optptr++ = 4;
    server_id = ntohl(server_ipaddr);
    memcpy(optptr, &server_id, 4);

    return(optptr + 4);
}

/**
*add dhcp default router
*/
static u8 *add_dhcpd_default_router(u8 *optptr)
{
    u32 default_router;

    if(0==server_ipaddr)
        server_ipaddr = bootvars.ip;

    *optptr++ = DHCP_OPTION_ROUTER;
    *optptr++ = 4;
    default_router = ntohl(server_ipaddr);
    memcpy(optptr, &default_router, 4);

    return(optptr + 4);
}

/**
*add dhcp dns server
*/
static u8 *add_dhcpd_dns_server(u8 *optptr)
{
    u32 dns;

    if(0==server_ipaddr)
        server_ipaddr = bootvars.ip;

    *optptr++ = DHCP_OPTION_DNS_SERVER;
    *optptr++ = 4;
    dns = ntohl(server_ipaddr);
    memcpy(optptr, &dns, 4);

    return(optptr + 4);
}

#if 0
/**
*add dhcp domain name
*/
static u8 *add_dhcpd_domain_name(u8 *optptr)
{
    *optptr++ = DHCP_OPTION_DOMAIN_NAME;
    *optptr++ = 4;
    memcpy(optptr, "hsuan", 6);

    return optptr + 4;
}
#endif

/**
*add dhcp lease time
*/
static u8 *add_dhcpd_lease_time(u8 *optptr)
{
    u32 leasetime;

    *optptr++ = DHCP_OPTION_LEASE_TIME;
    *optptr++ = 4;
    leasetime = ntohl(uip_dhcp_leasetime);
    memcpy(optptr, &leasetime, 4);

    return optptr + 4;
}

/**
*add dhcp subnet mask
*/
static u8 *add_dhcpd_subnet_mask(u8 *optptr)
{
    u32 subnet_mask;

    if(0==uip_server_netmask)
        uip_server_netmask = bootvars.msk;

    *(optptr++) = DHCP_OPTION_SUBNET_MASK;
    *(optptr++) = 4;
    subnet_mask = ntohl(uip_server_netmask);
    memcpy(optptr, &subnet_mask, 4);

    return optptr + 4;
}

/**
*add dhcp end msg
*/
static u8 *add_end(u8 *optptr)
{
    *optptr++ = DHCP_OPTION_END;
    return optptr;
}

/**
*show dhcp list
*/
void display_list(NODE *head) {
    NODE *p;
    for (p=head->next; p!=NULL; p=p->next)
    {
        printf("ipaddr: %d.%d.%d.%d\n",
               p->ipaddr.addr&0x000000ff,(p->ipaddr.addr&0x0000ff00)>>8,
               (p->ipaddr.addr&0x00ff0000)>>16,(p->ipaddr.addr&0xff000000)>>24);
    }
    printf("\n");
}

int find_vaild_hwaddr(NODE* head,char *HwAddress)
{
    NODE *p;
    p =head;
    while ((p->next)&&(strcmp((char *)p->next->hwaddr,HwAddress)))
        p = p->next;
    if (p->next)
    {
        printf("Found match ipaddr: %d.%d.%d.%d\n",
               p->next->ipaddr.addr&0x000000ff,(p->next->ipaddr.addr&0x0000ff00)>>8,
               (p->next->ipaddr.addr&0x00ff0000)>>16,(p->next->ipaddr.addr&0xff000000)>>24);
        return TRUE;
    }
    else
    {
        printf("Could not find vaild ipaddr\n");
        return FALSE;
    }
}

#ifdef CONFIG_NODE_LIST
#define MAX_NODE_SIZE 100
NODE malloc_data[MAX_NODE_SIZE];
u32 malloc_idx = 0; 
NODE *node_malloc(void) {
    if ((malloc_idx + 1) >= MAX_NODE_SIZE)
    {
        printf("out of memory\n");
        return 0;
    }

    NODE *node = &malloc_data[malloc_idx++];
    return node;
}

int list_add(NODE **rootp, int ipaddr, u8 *hw_addr)  
{  
    NODE *newNode;
    NODE *previous;
    NODE *current;

    current = *rootp;  
    previous = NULL;  

    while (current != NULL && current->ipaddr.addr < ipaddr)
    {
        previous = current;  
        current = current->next;  
    }  

#ifdef STACK_MALLOC_NODE
    newNode = node_malloc();
#else
    newNode = (NODE *)malloc(sizeof(NODE));
#endif
    if (newNode == NULL)
    {
        return FALSE; 
    }
    newNode->ipaddr.addr = ipaddr;
    memcpy(newNode->hwaddr, hw_addr, DHCPD_CHADDR_LEN);

    newNode->next = current;  
    if (previous == NULL)
    {
        *rootp = newNode;
    }
    else
    {
        previous->next = newNode;  
    }

    return TRUE;  
}

static int search_vaild_bitmap(u8 *bitmap, int size)
{
    int i = 0, index = 0;
    for (index = 0; index < size; index++, bitmap++)
    {
        i = 0;
        while (i < 8)
        {
            if ((*bitmap) & (1 << i++))
                continue;
            *bitmap = (*bitmap) | (1 << (--i));
            return((index * 8) + i);
        }
    }
    return -1;
}

static u32 allocate_new_ipaddr(NODE *head)
{
    struct dhcp_client *dhcp = head;
    int ret;
    u32 ipaddr;

    ret = search_vaild_bitmap(dhcp->ip_bitmap, IP_BITMAP_SIZE);
    if ((ret < 0) || ((ret + DHCPD_ADDR_START) > DHCPD_ADDR_END))
    {
        printf("allocate ip vaild bitmap failed.\n");
        return FALSE;
    }

    if(dhcpd_ipaddr==0)
        dhcpd_ipaddr = bootvars.ip;

    ipaddr = ntohl(dhcpd_ipaddr);
    ipaddr += (ret + DHCPD_ADDR_START);
    return htonl(ipaddr);
}
#else
u32 last_set_addr;
u32 dhcp_ip_offset = 0;
static u32 force_allocate_new_ipaddr(void)
{
    u32 ipaddr;
    u32 ip_idx;

    if(dhcpd_ipaddr==0)
        dhcpd_ipaddr = bootvars.ip;

    ip_idx = (dhcp_ip_offset + DHCPD_ADDR_START + (dhcpd_ipaddr&0xff));
    if (ip_idx > DHCPD_ADDR_END)
    {
        dhcp_ip_offset = 0;
    }

    ipaddr = (dhcpd_ipaddr + (dhcp_ip_offset++));
    ipaddr += DHCPD_ADDR_START;

    last_set_addr = ipaddr;
    return last_set_addr;
}
#endif

/*---------------------------------------------------------------------------*/

static u8 parse_dhcp_options(u8 *optptr, int len)
{
    u8 *end = optptr + len;
    u8 type = 0;

    while (optptr < end)
    {
        switch (*optptr)
        {
            case DHCP_OPTION_SUBNET_MASK:
                memcpy(s.netmask, optptr + 2, 4);
                break;
            case DHCP_OPTION_ROUTER:
                memcpy(s.default_router, optptr + 2, 4);
                break;
            case DHCP_OPTION_DNS_SERVER:
                memcpy(s.dnsaddr, optptr + 2, 4);
                break;
            case DHCP_OPTION_MSG_TYPE:
                type = *(optptr + 2);
                break;
            case DHCP_OPTION_SERVER_ID:
                memcpy(s.serverid, optptr + 2, 4);
                break;
            case DHCP_OPTION_LEASE_TIME:
                memcpy(s.lease_time, optptr + 2, 4);
                break;
            case DHCP_OPTION_REQ_IPADDR:
                memcpy(s.ipaddr, optptr + 2, 4);
                break;
            case DHCP_OPTION_END:
                return type;
        }

        optptr += optptr[1] + 2;
    }
    return type;
}

static u8 parse_dhcp_msg(u8 *payload, u32 len)
{
    struct dhcpd_msg *m = (struct dhcpd_msg *)payload;

    //判斷是否是DHCP請求
    if (m->op == DHCP_REQUEST)
    {
        return parse_dhcp_options(&m->options[4], len);
    }

    return 0;
}

/* Header sizes. */
#if UIP_CONF_IPV6
    #define UIP_IPH_LEN    40
#else /* UIP_CONF_IPV6 */
    #define UIP_IPH_LEN    20    /* Size of IP header */
#endif /* UIP_CONF_IPV6 */
#define UIP_UDPH_LEN    8    /* Size of UDP header */
#define UIP_IPUDPH_LEN  (UIP_UDPH_LEN + UIP_IPH_LEN)    /* Size of IP + UDP header */

u8 uip_buf[UIP_BUFSIZE + 2];
u8 *uip_sappdata = &uip_buf[UIP_IPUDPH_LEN];

void uip_send(const void *data, u32 len)
{
    //printf("enter %s()\n", __FUNCTION__);

    if (len > 0)
    {
        memcpy(uip_sappdata, (void*)(data), len);
    }

    if (0 == udp_tx(IP_BROADCAST, BOOTP_SERVER, BOOTP_CLIENT, len + UIP_IPUDPH_LEN, uip_buf))
    {
        printf("not send DHCP OFFER/ACK packet back\n");
    }
}

#if 0
u8 *uip_gen_eth_packet(u8 *data, u32 len)
{
    struct udphdr *pudp;
    struct iphdr *ip;

    if(0==server_ipaddr)
        server_ipaddr = bootvars.ip;

    memcpy(uip_sappdata + ETHER_HEAD_SZ, (data), len);
                                     // 67              68            28
//  udp_gen_tx_packet(IP_BROADCAST, BOOTP_SERVER, BOOTP_CLIENT, len + UIP_IPUDPH_LEN, uip_buf);
    pudp = (struct udphdr *)(uip_buf + ETHER_HEAD_SZ + UIP_IPH_LEN);
    pudp->chksum = 0;
    pudp->dest = htons(BOOTP_CLIENT);
    pudp->src = htons(BOOTP_SERVER);
    pudp->len = htons(len + UIP_UDPH_LEN);

//  ip_tx(dip, IP_UDP, len, buf);
    ip = (struct iphdr *)(uip_buf + ETHER_HEAD_SZ);
    ip->verhdrlen = 0x45;
    ip->service = 0;
    ip->len = htons(len + UIP_IPUDPH_LEN);
    ip->ident = 0;
    ip->frags = 0;
    ip->ttl = 64;
    ip->protocol = IP_UDP;
    ip->chksum = 0;
    ip->src.s_addr = htonl(server_ipaddr);
    ip->dest.s_addr = htonl(IP_BROADCAST);
    ip->chksum = ~ipchksum((unsigned short *)(uip_buf + ETHER_HEAD_SZ), IPHDR_SZ);

//  eth_tx(broadcast, IP, len, buf);
    memset(uip_buf, 0xff, 6);
    memcpy(uip_buf + 6, bootvars.mac0, 6);

    uip_buf[12] = (IP >> 8) & 0xff;
    uip_buf[13] = IP & 0xff;

    return uip_buf;
}
#endif

static int dhcpd_relay_offer(u8 *payload)
{
    u8 *end;
    u32 addr;
    u32 len;

    struct dhcpd_msg *m = (struct dhcpd_msg *)payload;

#ifdef CONFIG_NODE_LIST
    p = head_dhcpd;

    while ((p->next) && (strcmp(p->next->hwaddr, m->chaddr)))
        p = p->next;

    if (p->next)
    {
        addr = p->next->ipaddr.addr;
        printf("offer used ipaddr: %d.%d.%d.%d\n",
               (p->next->ipaddr.addr&0xff000000)>>24, (p->next->ipaddr.addr&0x00ff0000)>>16,
               (p->next->ipaddr.addr&0x0000ff00)>>8, (p->next->ipaddr.addr&0x000000ff));
    }
    else
    {
        addr = allocate_new_ipaddr(head_dhcpd);
        printf("allocate new ip addr:%d.%d.%d.%d\n",
               (addr&0xff000000)>>24, (addr&0x00ff0000)>>16,
               (addr&0x0000ff00)>>8, (addr&0x000000ff));
        list_add(&head_dhcpd, addr, m->chaddr);
    }
#else
    addr = force_allocate_new_ipaddr();
    printf("allocate new ip addr:%d.%d.%d.%d\n",
           (addr&0xff000000)>>24, (addr&0x00ff0000)>>16,
           (addr&0x0000ff00)>>8, (addr&0x000000ff));
#endif

    m->op = DHCP_REPLY;
    addr = htonl(addr);
    memcpy(m->yiaddr, &addr, sizeof(m->yiaddr));   //設置CLINET IP地址

    end = add_msg_type(&m->options[4], DHCPOFFER);
    end = add_dhcpd_server_id(end);
    end = add_dhcpd_default_router(end);
    end = add_dhcpd_dns_server(end);
    end = add_dhcpd_lease_time(end);
    end = add_dhcpd_subnet_mask(end);
    end = add_end(end);

    len = end - (u8*)payload;
    uip_send(payload, len);

    return 1;
}

static int dhcpd_replay_ask(u8 *payload)
{
    u8 *end;
    u32 addr;
    int IfAsk = 1;
    u32 len;

    struct dhcpd_msg *m = (struct dhcpd_msg *)payload;

#ifdef CONFIG_NODE_LIST
    p = head_dhcpd;

    while ((p->next) && (strcmp(p->next->hwaddr, m->chaddr)))
        p = p->next;

    if (p->next)
    {
        printf("Find available ipaddr from list\n");
        addr = p->next->ipaddr.addr;
        IfAsk = 1;
    }
    else
    {
        printf("Could not find available ipaddr from list\n");
        IfAsk = 0;
    }
#else
    if (last_set_addr == 0)
    {
        printf("To decline request from client, and assign the new ip address\n");
        IfAsk = 0;
    }
    else
    {
        addr = last_set_addr;
        last_set_addr = 0;
    }
#endif

    m->op = DHCP_REPLY;

    if (IfAsk)
    {
        printf("panther dhcpd set client ipaddr: %d.%d.%d.%d\n",
               (addr&0xff000000)>>24, (addr&0x00ff0000)>>16,
               (addr&0x0000ff00)>>8, (addr&0x000000ff));

        addr = htonl(addr);
        memcpy(m->yiaddr, &addr, sizeof(m->yiaddr));//設置CLINET IP地址
        end = add_msg_type(&m->options[4], DHCPACK);
        end = add_dhcpd_server_id(end);
        end = add_dhcpd_default_router(end);
        end = add_dhcpd_dns_server(end);
        end = add_dhcpd_lease_time(end);
        end = add_dhcpd_subnet_mask(end);
    }
    else
    {
        printf("send DHCPNAK\n");
        end = add_msg_type(&m->options[4], DHCPNAK);
        end = add_dhcpd_server_id(end);
    }
    end = add_end(end);

    len = end - (u8*)payload;
    uip_send(payload, len);

    return 1;
}

void handle_dhcpd(u8 *payload, u32 len)
{
    unsigned char state;

//  if (uip_newdata())
//  {
    state = parse_dhcp_msg(payload, len);

    if (state == DHCPDISCOVER)
    {
        dhcpd_relay_offer(payload);
    }
    else if (state == DHCPREQUEST)
    {
        dhcpd_replay_ask(payload);
    }
//  }

    return;
}

struct udp_conn *m_udp_conn;
struct udp_conn udp_conns[UIP_UDP_CONNS];
static u16 lastport;       /* Keeps track of the last port used for a new connection. */
/* Temporary variables. */
u8 uip_acc32[4];
static u8 c;

struct udp_conn* udp_new(ip_addr *ripaddr, u16 rport)
{
    register struct udp_conn *conn;

    /* Find an unused local port. */
    again:
    ++lastport;

    if (lastport >= 32000)
    {
        lastport = 4096;
    }

    for (c = 0; c < UIP_UDP_CONNS; ++c)
    {
        if (udp_conns[c].lport == htons(lastport))
        {
            goto again;
        }
    }


    conn = 0;
    for (c = 0; c < UIP_UDP_CONNS; ++c)
    {
        if (udp_conns[c].lport == 0)
        {
            conn = &udp_conns[c];
            break;
        }
    }

    if (conn == 0)
    {
        return 0;
    }

    conn->lport = htons(lastport);
    conn->rport = rport;
    if (ripaddr == NULL)
    {
        memset((void *)conn->ripaddr, 0, sizeof(ip_addr));
    }
    else
    {
        ipaddr_copy(&conn->ripaddr, ripaddr);
    }
    conn->ttl = UIP_TTL;

    return conn;
}

int dhcpd_init(void)
{
//  printf("enter %s()\n", __FUNCTION__);

#ifdef CONFIG_NODE_LIST
#ifdef STACK_MALLOC_NODE
    head_dhcpd = node_malloc();
#else
    head_dhcpd = (NODE *)malloc(sizeof(NODE));
#endif

    if (head_dhcpd == NULL)
    {
        printf("allocated head_dhcpd failed.\n");
        return -1;
    }
    head_dhcpd->next = NULL;
#endif

    ip_addr addr;

    s.mac_len = DHCP_HLEN_ETHERNET;
    ip_ipaddr(addr, 255, 255, 255, 255);
    s.conn = udp_new(&addr, htons(DHCPC_CLIENT_PORT));

    if (s.conn != NULL)
    {
        udp_bind(s.conn, htons(DHCPC_SERVER_PORT));
    }

//  printf("leave %s()\n", __FUNCTION__);

    return TRUE;
}

void udp_appcall(u8 *payload, u32 len)
{
    handle_dhcpd(payload, len);
    return;
}
