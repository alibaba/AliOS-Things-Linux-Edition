/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file byteorder.h
*   \brief Endian Convert API
*   \author Montage
*/
#ifndef __BYTEORDER_H__
#define __BYTEORDER_H__

#define ___swab16(x) \
     ((unsigned short)( \
         (((unsigned short)(x) & (unsigned short)0x00ffU) << 8) | \
         (((unsigned short)(x) & (unsigned short)0xff00U) >> 8) ))

#define ___swab32(x) \
     ((unsigned int)( \
         (((unsigned int)(x) & (unsigned int)0x000000ffUL) << 24) | \
         (((unsigned int)(x) & (unsigned int)0x0000ff00UL) <<  8) | \
         (((unsigned int)(x) & (unsigned int)0x00ff0000UL) >>  8) | \
         (((unsigned int)(x) & (unsigned int)0xff000000UL) >> 24) ))

extern inline void ___swab16s(volatile unsigned short *addr)
{
#if 1                           //avoid unailgned access
    unsigned char c = *(unsigned char *) addr;
    *((unsigned char *) addr) = *(((unsigned char *) addr) + 1);
    *(((unsigned char *) addr) + 1) = c;
#else
    unsigned short result = *addr;
    *addr = ___swab16(result);
#endif
}

#if defined(BIG_ENDIAN)
#define cpu_to_le16 ___swab16
#define cpu_to_le32 ___swab32
#define le16_to_cpu ___swab16
#define le32_to_cpu ___swab32
//// FIXME
#define le16_to_cpus  ___swab16s

#define cpu_to_be32(a) (a)
#define cpu_to_be16(a) (a)
#define be32_to_cpu(a) (a)
#define be16_to_cpu(a) (a)
#else
#define cpu_to_le16(a) (a)
#define cpu_to_le32(a) (a)
#define le16_to_cpu(a) (a)
#define le32_to_cpu(a) (a)
//// FIXME
// convert form big-endian part, i don't known what to fix
#define le16_to_cpus(a)

#define cpu_to_be32(a) ___swab32(a)
#define cpu_to_be16(a) ___swab16(a)
#define be32_to_cpu(a) ___swab32(a)
#define be16_to_cpu(a) ___swab16(a)
#endif
#endif                          //__BYTEORDER_H__
