/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file libstr.c
*   \brief  string/memory library
*   \author Montage
*/

#include    <common.h>
#include    <lib.h>
/*!
 * function:
 *
 *  \brief
 *  \param str
 *  \param c
 *  \return
 */

void append_chr(char *str, char c)
{
    while (*str++) ;
    *str-- = 0;
    *str = c;
}

/*!
 * function:
 *
 *  \brief
 *  \param str
 *  \param c
 *  \return
 */

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

/*!
 * function:
 *
 *  \brief
 *  \param str
 *  \return
 */

int strlen(const char *str)
{
    int i = 0;
    while (*str++)
        i++;
    return i;
}

/*!
 * function:
 *
 *  \brief
 *  \param dst
 *  \param src
 *  \return
 */

char *strcpy(char *dst, const char *src)
{
    char *p = dst;
    while (*src)
    {
        *p++ = *src++;
    }
    *p = 0;
    return dst;
}

/*!
 * function:
 *
 *  \brief 
 *  \param s1  appended string
 *  \param s2  string cat after s1
 *  \return
 */

char *strcat(char *s1, const char *s2)
{
    strcpy(&s1[strlen(s1)], s2);
    return s1;
}

/*!
 * function:
 *
 *  \brief
 *  \param dst
 *  \param src
 *  \return
 */

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

/*!
 * function:
 *
 *  \brief
 *  \param str
 *  \return
 */

char *strupr(char *str)
{
    char *p;

    p = str;
    while (*p)
    {
        if (*p < 'a' || *p > 'z')
            continue;
        *p -= 0x20;
        p++;
    }
    return str;
}

/*!
 * function:
 *
 *  \brief
 *  \param dst
 *  \param src
 *  \param len
 *  \return
 */

int strncmp(const void *dst, const void *src, int len)
{
    const char *d = (const char *) dst;
    const char *s = (const char *) src;
    for (; len-- > 0;)
    {
        if (*d != *s)
            return *d - *s;
        d++;
        s++;
    }
    return 0;
}

/*!
 * function:
 *
 *  \brief
 *  \param src
 *  \param dst
 *  \param len
 *  \return
 */

void strncpy(void *dst, const void *src, int len)
{
    char *d = (char *) dst;
    const char *s = (const char *) src;
    for (; len-- > 0;)
    {
        *d++ = *s++;
    }
}

/*!
 * function:
 *
 *  \brief
 *  \param p
 *  \param data
 *  \param len
 *  \return
 */

void memset(char *p, int data, int len)
{
    register int i;

    for (i = 0; i < len; i++)
    {
        *p++ = data & 0xff;
    }
}

/*!
 * function:
 *
 *  \brief
 *  \param dst
 *  \param src
 *  \param len
 *  \return
 */

int memcmp(void *dst, void *src, int len)
{
    register int i;
    register char *s = (char *) src;
    register char *d = (char *) dst;

    for (i = 0; i < len; i++)
    {
        if (*d++ != *s++)
            break;
    }
    return (len - i);
}

#ifndef CONFIG_OPTIMIZED_MEMCPY
/*!
 * function:
 *
 *  \brief
 *  \param dst
 *  \param src
 *  \param len
 *  \return
 */
void memcpy(void *dst, void *src, int len)
{
    register int i;
    register char *s = (char *) src;
    register char *d = (char *) dst;

    for (i = 0; i < len; i++)
        d[i] = s[i];
}
#endif

/*!
 * function:
 *
 *  \brief
 *  \param dst
 *  \param src
 *  \param len
 *  \return
 */

void memmove(void *dst, void *src, int len)
{
    register char *s = (char *) src;
    register char *d = (char *) dst;

    for (; --len >= 0;)
        *d-- = *s--;
}

/*!
 * function:
 *
 *  \brief
 *  \param str
 *  \param v
 *  \return
 */

int hextoul(char *str, void *v)
{
    return (sscanf(str, "%x", v) == 1);
}

/*!
 * function:
 *
 *  \brief
 *  \param a
 *  \param b
 *  \return
 */

int strcasecmp(char *a, char *b)
{
    while (*a && *b && (*a & ~0x20) == (*b & ~0x20))
    {
        a++;
        b++;
    }
    return ((*a & ~0x20) - (*b & ~0x20));
}

#define is_digit(c) ((c) >= '0' && (c) <= '9')

/*!
 * function:
 *
 *  \brief
 *  \param s
 *  \param rp
 *  \param base
 *  \return
 */

int strtol(char *s, char **rp, int base)
{
    int i = 0;
    if (base != 10)
    {
        printk("base:%d not support\n", base);
        return 0;
    }
    while (is_digit(*s))
        i = i * 10 + (*s++ - '0');
    if (rp)
        *rp = s;
    return i;
}

/*!
 * function:
 *
 *  \brief
 *  \param s
 *  \return
 */

int atoi(char *s)
{
    return strtol(s, 0, 10);
}

/*!
 * function:
 *
 *  \brief
 *  \param buf
 *  \param mac
 *  \return
 */

int ether_aton(char *buf, unsigned char *mac)
{
    short i;
    unsigned int m[6];
    if (6 != (sscanf
              (buf, "%x:%x:%x:%x:%x:%x", m, m + 1, m + 2, m + 3, m + 4, m + 5)))
        return -1;
    for (i = 0; i < 6; i++)
        mac[i] = m[i];
    return 0;
}

/*!
 * function:
 *
 *  \brief
 *  \param m
 *  \return
 */

char *ether_ntoa(unsigned char *m)
{
    static char buf[20];
    sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", m[0], m[1], m[2], m[3], m[4],
            m[5]);
    return buf;
}

/*!
 * function:
 *
 *  \brief
 *  \param ipp
 *  \return
 */

char *inet_ntoa(void *ipp)
{
    static char buf[16];
    unsigned char *ip = (unsigned char *) ipp;
    sprintf(buf, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    return buf;
}

/*!
 * function:
 *
 *  \brief
 *  \param str
 *  \param dp
 *  \return
 */

int inet_aton(char *str, void *dp)
{
    short i;
    int d[6];
    char *ip = (char *) dp;
    if (4 != (sscanf(str, "%d.%d.%d.%d", d, d + 1, d + 2, d + 3)))
        return -1;
    for (i = 0; i < 4; i++)
        ip[i] = 0xff & d[i];
    return 0;
}
