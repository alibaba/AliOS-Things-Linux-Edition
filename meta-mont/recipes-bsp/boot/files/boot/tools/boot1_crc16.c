#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/io.h>
#include <string.h>
#include <sys/mman.h>

#define BOOT1_BIN   "boot1.bin"
#define BOOT1_IMAGE "boot1.img"

#if defined(BOOT1_24K)
#define BOOT1_IMAGE_SIZE    (24 * 1024)
#else
#define BOOT1_IMAGE_SIZE    16384
#endif
#define BOOT1_BIN_MAX_SIZE  (BOOT1_IMAGE_SIZE - 4)

unsigned char buffer[BOOT1_IMAGE_SIZE];

unsigned short crc16_ccitt(unsigned char* data_p, unsigned int length)
{
    unsigned char x;
    unsigned long crc = 0;

    while (length--)
    {
        x = (crc >> 8) ^ (*data_p++);
        x ^= (x >> 4);
        crc = (crc << 8) ^ ((unsigned short)(x << 12)) ^ ((unsigned short)(x << 5)) ^ ((unsigned short)x);
        crc = crc & 0x0ffff;
    }
    return crc & 0xffff;
}

int main(int argc, char const *argv[])
{
    unsigned char *p1, *p2;
    struct stat s1, s2;
    int fd1, fd2;
    int status1, status2;
    int i;
    unsigned short crc16;

    fd1 = open(BOOT1_BIN, O_RDONLY);
    fd2 = open(BOOT1_IMAGE, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);

    if((0>fd1)||(0>fd2))
    {
        printf("open() error\n");
        goto EXIT;
    }

    status1 = fstat(fd1, &s1);
    if(0>status1)
    {
        printf("fstat() error\n");
        goto EXIT;
    }

    if(s1.st_size >= (BOOT1_BIN_MAX_SIZE - 4))
    {
        printf("WARNING: boot1 size exceeds limit\n");
        s1.st_size = (BOOT1_BIN_MAX_SIZE - 4);
    }

    if(lseek(fd2, BOOT1_IMAGE_SIZE - 1, SEEK_SET) < 0)
    {
        printf("lseek error\n");
        goto EXIT;
    }

    if(write(fd2, " ", 1) != 1)
    {
        printf("write error\n");
        goto EXIT;
    }

    p1 = (char *) mmap (0, s1.st_size, PROT_READ, MAP_PRIVATE, fd1, 0);
    p2 = (char *) mmap (0, BOOT1_IMAGE_SIZE, PROT_WRITE, MAP_SHARED, fd2, 0);

    if((p1==NULL)||(p2==NULL))
    {
        printf("mmap() error\n");
        goto EXIT;
    }

    memset(buffer, 0, BOOT1_IMAGE_SIZE);
    memcpy(buffer, p1, s1.st_size);

    crc16 = crc16_ccitt(buffer, (BOOT1_IMAGE_SIZE - 2));
    buffer[BOOT1_IMAGE_SIZE-2] = ((crc16 >> 8) & 0xff);
    buffer[BOOT1_IMAGE_SIZE-1] = (crc16 & 0xff);

    memcpy(p2, buffer, BOOT1_IMAGE_SIZE);

    munmap(buffer, BOOT1_IMAGE_SIZE);

    close(fd1);
    close(fd2);

    return 0;

EXIT:
    return -1;
}
