/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file netdev.h
*   \brief Network Device API
*   \author Montage
*/

#ifndef NETDEV_H
#define NETDEV_H

#define NBDEBUG      0

struct nbuf
{
    struct nbuf *next;
    unsigned int len;
    unsigned char *head;
    unsigned char *data;
    unsigned char *tail;
    unsigned char *end;

    void *dev;
    int flag;
    short id2;
};

enum
{
    NB_FREE = 0xadad,
    NB_ALLOC = 0xacac,
    NB_RELEASE = 0xabab,
    NB_STALL = 0xaaaa,
    NB_OK = 0,
    NB_ERR_ID = 3,
    NB_HEAD_SZ = 0x40,
};

int nbuf_state();
struct nbuf *nbuf_get();
int nbuf_init(void *pool, int size, int count);
int nbuf_put(struct nbuf *mp);

typedef struct queue
{
    struct nbuf *head, *tail;
    int count;
    int max;
} queue;

struct nbuf_cfg
{
    queue queue;
    int size;
    int num;
    void *base;
};

struct net_device
{
    char name[16];

    unsigned long base_addr;
    unsigned int irq;

    unsigned char dev_addr[6];
    unsigned char broadcast[6];
    int addr_len;

    int (*open) (struct net_device * dev);
    int (*stop) (struct net_device * dev);
    int (*hard_start_xmit) (struct nbuf * skb, struct net_device * dev);
    int (*do_ioctl) (struct net_device * dev, void *ifr, int cmd);
    void (*poll) (void *dev);
    void *priv;
    int start;
};

struct net_device *netdev_alloc();
int netif_rx(struct nbuf *nbuf);

#endif                          /* NETDEV_H */
