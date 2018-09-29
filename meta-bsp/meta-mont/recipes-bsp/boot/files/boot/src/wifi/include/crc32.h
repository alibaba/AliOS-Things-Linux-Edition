#ifndef _CRC32_H_
#define _CRC32_H_

u32 crc32_be(u32 crc, unsigned char *p, size_t len);
u32 crc32_le(u32 crc, unsigned char *p, size_t len);
void crc32init_be(void);
void crc32init_le(void);

unsigned char delimiter_crc8(unsigned short data);

#endif // _CRC32_H_
