/*
 * Copyright (c) 2015, mleaf mleaf90@gmail.com or 350983773@qq.com
 * All rights reserved.
 *
 * Modify this file to fit out demand of MINI_AP by Hsuan
 */

#ifndef __DHCPD_H__
#define __DHCPD_H__

/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include "timer.h"
#include "mt_types.h"
#include "netprot.h"
//#include "pt.h"
//#include "uipopt.h"

/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
#define UIP_BUFSIZE 400
#define UIP_LLH_LEN 14
#define UIP_UDP_CONNS 10
#define UIP_TTL 64

#ifndef NULL
    #define NULL  0
#endif

#ifndef DHCPD_ADDR_START
    #define  DHCPD_ADDR_START 100
#endif

#ifndef DHCPD_ADDR_END 
    #define DHCPD_ADDR_END 200
#endif

#define IP_BITMAP_SIZE ((DHCPD_ADDR_END-DHCPD_ADDR_START)/sizeof(u8)+1)
#define DHCPD_CHADDR_LEN 16U
#define UIP_UDP_APPCALL mleaf_udp_appcall

#define ip_ipaddr(addr, addr0, addr1, addr2, addr3) do { \
                     ((u16 *)(addr))[0] = htons(((addr0) << 8) | (addr1)); \
                     ((u16 *)(addr))[1] = htons(((addr2) << 8) | (addr3)); \
                  } while(0)

struct ip_addr {
    u32 addr;
};

struct dhcp_client{
 	u8 hwaddr[DHCPD_CHADDR_LEN];
 	u8 ip_bitmap[IP_BITMAP_SIZE];
 	struct ip_addr ipaddr;
    struct dhcp_client *next;
};

/**
 * Repressentation of an IP address.
 */
typedef u16 ipv4_addr[2];
typedef u16 ipv6_addr[8];
#if UIP_CONF_IPV6
typedef ipv6_addr ip_addr;
#else /* IP_CONF_IPV6 */
typedef ipv4_addr ip_addr;
#endif /* IP_CONF_IPV6 */

/**
 * Copy an IP address to another IP address.
 *
 * Copies an IP address from one place to another.
 *
 * Example:
 \code
 uip_ipaddr_t ipaddr1, ipaddr2;

 uip_ipaddr(&ipaddr1, 192,16,1,2);
 uip_ipaddr_copy(&ipaddr2, &ipaddr1);
 \endcode
 *
 * \param dest The destination for the copy.
 * \param src The source from where to copy.
 *
 * \hideinitializer
 */
#if !UIP_CONF_IPV6
#define ipaddr_copy(dest, src) do { \
                     ((u16 *)dest)[0] = ((u16 *)src)[0]; \
                     ((u16 *)dest)[1] = ((u16 *)src)[1]; \
                  } while(0)
#else /* !UIP_CONF_IPV6 */
#define ipaddr_copy(dest, src) memcpy(dest, src, sizeof(ipv6_addr))
#endif /* !UIP_CONF_IPV6 */

/**
 * Bind a UDP connection to a local port.
 *
 * \param conn A pointer to the uip_udp_conn structure for the
 * connection.
 *
 * \param port The local port number, in network byte order.
 *
 * \hideinitializer
 */
#define udp_bind(conn, port) (conn)->lport = port


struct dhcpd_state {
    char state;
    struct udp_conn *conn;
    struct timer timer;
    u16 ticks;
    u8 mac_addr[16];
    int mac_len;
    u32 xid;
    u8 serverid[4];
    u16 lease_time[2];
    u16 ipaddr[4];
    u16 netmask[4];
    u16 dnsaddr[4];
    u16 default_router[4];
};

typedef struct dhcpd_state udp_appstate_t;

/**
 * Representation of a UDP connection.
 */
struct udp_conn {
  ip_addr ripaddr;   /**< The IP address of the remote peer. */
  u16 lport;         /**< The local port number in network byte order. */
  u16 rport;         /**< The remote port number in network byte order. */
  u8  ttl;           /**< Default time-to-live. */

  /** The application state. */
  udp_appstate_t appstate;
};

/*=============================================================================+
| Function Prototypes                                                          |
+=============================================================================*/
int dhcpd_init(void);
void udp_appcall(u8 *payload, u32 len);

#endif
