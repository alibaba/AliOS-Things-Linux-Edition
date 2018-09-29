/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
#ifndef __MT_TYPES_H__
#define __MT_TYPES_H__
typedef __signed__ char __s8;
typedef unsigned char __u8;

typedef __signed__ short __s16;
typedef unsigned short __u16;

typedef __signed__ int __s32;
typedef unsigned int __u32;

#undef s8  // conflict with arch/cpu.h
typedef __signed char s8;
typedef unsigned char u8;

typedef __signed short s16;
typedef unsigned short u16;

typedef __signed int s32;
typedef unsigned int u32;

typedef __signed long long s64;
typedef unsigned long long u64;

typedef u32 dma_addr_t;

typedef u32 phys_addr_t;
typedef u32 phys_size_t;

//typedef int size_t;

#define true 	1
#define false	0

#ifndef NULL
#define NULL					0
#endif

#ifndef BIT
#define BIT(s) (1 << (s))
#endif

#define BITS_PER_LONG			32
#define SHA1_MAC_LEN	20

#if 0
/* unaligned memory accesses */
#define READ_BE16(a) ((u16) (((a)[0] << 8) | ((a)[1]&0xff)))
#define WRITE_BE16(a, v)			\
	do {					\
		(a)[0] = ((u16) (v)) >> 8;	\
		(a)[1] = ((u16) (v)) & 0xff;	\
	} while (0)

#define READ_LE16(a) ((u16) (((a)[1] << 8) | ((a)[0]&0xff)))
#define WRITE_LE16(a, v)			\
	do {					\
		(a)[1] = ((u16) (v)) >> 8;	\
		(a)[0] = ((u16) (v)) & 0xff;	\
	} while (0)

#define READ_BE24(a) ((u32)(((a)[0] << 16) | ((a)[1] << 8) | \
			 ((a)[2]&0xff)))
#define WRITE_BE24(a, v)					\
	do {							\
		(a)[0] = (u8) ((((u32) (v)) >> 16) & 0xff);	\
		(a)[1] = (u8) ((((u32) (v)) >> 8) & 0xff);	\
		(a)[2] = (u8) (((u32) (v)) & 0xff);		\
	} while (0)

#define READ_BE32(a) ((u32) (((a)[0] << 24) | (((a)[1]&0xff) << 16) | \
			 (((a)[2]&0xff) << 8) | ((a)[3] & 0xff)))
#define WRITE_BE32(a, v)					\
	do {							\
		(a)[0] = (u8) ((((u32) (v)) >> 24) & 0xff);	\
		(a)[1] = (u8) ((((u32) (v)) >> 16) & 0xff);	\
		(a)[2] = (u8) ((((u32) (v)) >> 8) & 0xff);	\
		(a)[3] = (u8) (((u32) (v)) & 0xff);		\
	} while (0)

#define READ_LE32(a) ((u32) (((a)[3] << 24) | ((a)[2] << 16) | \
			 ((a)[1] << 8) | ((a)[0]&0xff)))
#define WRITE_LE32(a, v)					\
	do {							\
		(a)[3] = (u8) ((((u32) (v)) >> 24) & 0xff);	\
		(a)[2] = (u8) ((((u32) (v)) >> 16) & 0xff);	\
		(a)[1] = (u8) ((((u32) (v)) >> 8) & 0xff);	\
		(a)[0] = (u8) (((u32) (v)) & 0xff);		\
	} while (0)

#define READ_BE64(a) ((((u64) (a)[0]) << 56) | (((u64) (a)[1]) << 48) | \
			 (((u64) (a)[2]) << 40) | (((u64) (a)[3]) << 32) | \
			 (((u64) (a)[4]) << 24) | (((u64) (a)[5]) << 16) | \
			 (((u64) (a)[6]) << 8) | ((u64) (a)[7]&0xff))
#define WRITE_BE64(a, v)				\
	do {						\
		(a)[0] = (u8) (((u64) (v)) >> 56);	\
		(a)[1] = (u8) (((u64) (v)) >> 48);	\
		(a)[2] = (u8) (((u64) (v)) >> 40);	\
		(a)[3] = (u8) (((u64) (v)) >> 32);	\
		(a)[4] = (u8) (((u64) (v)) >> 24);	\
		(a)[5] = (u8) (((u64) (v)) >> 16);	\
		(a)[6] = (u8) (((u64) (v)) >> 8);	\
		(a)[7] = (u8) (((u64) (v)) & 0xff);	\
	} while (0)

#define READ_LE64(a) ((((u64) (a)[7]) << 56) | (((u64) (a)[6]) << 48) | \
			 (((u64) (a)[5]) << 40) | (((u64) (a)[4]) << 32) | \
			 (((u64) (a)[3]) << 24) | (((u64) (a)[2]) << 16) | \
			 (((u64) (a)[1]) << 8) | ((u64) (a)[0]&0xff))
#define WRITE_LE64(a, v)				\
	do {						\
		(a)[7] = (u8) (((u64) (v)) >> 56);	\
		(a)[6] = (u8) (((u64) (v)) >> 48);	\
		(a)[5] = (u8) (((u64) (v)) >> 40);	\
		(a)[4] = (u8) (((u64) (v)) >> 32);	\
		(a)[3] = (u8) (((u64) (v)) >> 24);	\
		(a)[2] = (u8) (((u64) (v)) >> 16);	\
		(a)[1] = (u8) (((u64) (v)) >> 8);	\
		(a)[0] = (u8) (((u64) (v)) & 0xff);	\
	} while (0)
#else

#endif

#if 0
#define READ_BE16		read_be16
#define READ_BE24		read_be24
#define READ_BE32		read_be32
#define READ_BE64		read_be64

#define WRITE_BE16 		write_be16
#define WRITE_BE24 		write_be24
#define WRITE_BE32 		write_be32
#define WRITE_BE64		write_be64

#define READ_LE16		read_le16
#define READ_LE24		read_le24
#define READ_LE32		read_le32
#define READ_LE64		read_le64

#define WRITE_LE16 		write_le16
#define WRITE_LE24 		write_le24
#define WRITE_LE32 		write_le32
#define WRITE_LE64		write_le64

unsigned short read_be16(const unsigned char *a);
unsigned short read_le16(const unsigned char *a);
unsigned int read_be24(const unsigned char *a);
unsigned int read_le24(const unsigned char *a);
unsigned int read_be32(const unsigned char *a);
unsigned int read_le32(const unsigned char *a);
unsigned long long read_be64(const unsigned char *a);
unsigned long long read_le64(const unsigned char *a);

void write_be16(unsigned char *a, unsigned short v);
void write_le16(unsigned char *a, unsigned short v);
void write_be24(unsigned char *a, unsigned int v);
void write_le24(unsigned char *a, unsigned int v);
void write_be32(unsigned char *a, unsigned int v);
void write_le32(unsigned char *a, unsigned int v);
void write_be64(unsigned char *a, unsigned long long v);
void write_le64(unsigned char *a, unsigned long long v);
#endif

#endif /* __MT_TYPES_H__ */
