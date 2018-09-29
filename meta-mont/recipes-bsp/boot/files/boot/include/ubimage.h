/*!
*   \file ubimage.h
*   \brief u-boot image header definition
*   \author
*/

#ifndef __UBIMAGE_H__
#define __UBIMAGE_H__

/*
 * Operating System Codes
 */
#define IH_OS_INVALID       0   /* Invalid OS   */
#define IH_OS_LINUX         5   /* Linux    */
#define IH_OS_U_BOOT        17  /* Firmware */

/*
 * CPU Architecture Codes (supported by Linux)
 */
#define IH_ARCH_INVALID     0   /* Invalid CPU  */
#define IH_ARCH_ARM         2   /* ARM      */
#define IH_ARCH_I386        3   /* Intel x86    */
#define IH_ARCH_IA64        4   /* IA64     */
#define IH_ARCH_MIPS        5   /* MIPS     */
#define IH_ARCH_MIPS64      6   /* MIPS  64 Bit */
#define IH_TYPE_INVALID     0   /* Invalid Image        */

#define IH_TYPE_STANDALONE  1   /* Standalone Program       */
#define IH_TYPE_KERNEL      2   /* OS Kernel Image      */
#define IH_TYPE_RAMDISK     3   /* RAMDisk Image        */
#define IH_TYPE_MULTI       4   /* Multi-File Image     */
#define IH_TYPE_FIRMWARE    5   /* Firmware Image       */
#define IH_TYPE_SCRIPT      6   /* Script file          */
#define IH_TYPE_FILESYSTEM  7   /* Filesystem Image (any type)  */
#define IH_TYPE_FLATDT      8   /* Binary Flat Device Tree Blob */
#define IH_TYPE_KWBIMAGE    9   /* Kirkwood Boot Image      */
#define IH_TYPE_IMXIMAGE    10  /* Freescale IMXBoot Image  */

/*
 * Compression Types
 */
#define IH_COMP_NONE        0   /*  No   Compression Used   */
#define IH_COMP_GZIP        1   /* gzip  Compression Used   */
#define IH_COMP_BZIP2       2   /* bzip2 Compression Used   */
#define IH_COMP_LZMA        3   /* lzma  Compression Used   */
#define IH_COMP_LZO     4       /* lzo   Compression Used   */

#define IH_UBMAGIC  0x27051956  /* Image Magic Number       */
#define IH_NMLEN        32      /* Image Name Length        */

/*
 * Legacy format image header,
 * all data in network byte order (aka natural aka bigendian).
 */
typedef struct image_header
{
    unsigned int ih_magic;      /* Image Header Magic Number    */
    unsigned int ih_hcrc;       /* Image Header CRC Checksum    */
    unsigned int ih_time;       /* Image Creation Timestamp */
    unsigned int ih_size;       /* Image Data Size      */
    unsigned int ih_load;       /* Data  Load  Address      */
    unsigned int ih_ep;         /* Entry Point Address      */
    unsigned int ih_dcrc;       /* Image Data CRC Checksum  */
    unsigned char ih_os;        /* Operating System     */
    unsigned char ih_arch;      /* CPU architecture     */
    unsigned char ih_type;      /* Image Type           */
    unsigned char ih_comp;      /* Compression Type     */
    unsigned char ih_name[IH_NMLEN];    /* Image Name       */
} image_header_t;

#define CI_MAGIC          0x43494948     /* { 'C', 'I', 'I', 'H' }  */
#define UBI_EC_HDR_MAGIC  0x55424923
#define SQUASHFS_MAGIC    0x68737173
#define UBIFS_NODE_MAGIC  0x06101831

typedef struct combined_image_header
{
    unsigned long magic;             /* magic word ("Combined Image")  */
    unsigned long version;
    unsigned long kernel_signature;
    unsigned long rootfs_signature;
    unsigned long kernel_size[2];    /* size of kernel encoded as zero padded 8 digit hex  */
    unsigned long rootfs_size[2];    /* size of rootfs encoded as zero padded 8 digit hex  */
    unsigned long all_md5sum[8];     /* checksum of the combined kernel and rootfs image   */
    unsigned long kernel_md5sum[8];
    unsigned long rootfs_md5sum[8];
} ci_header_t;

#endif                          /* __UBIMAGE_H__ */
