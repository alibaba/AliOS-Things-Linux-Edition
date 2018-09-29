/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file image.h
*   \brief Image Header Format Definition
*   \author Montage
*/

#ifndef IMAGE_H
#define IMAGE_H

struct img_head
{
    unsigned char magic[3];
    unsigned char hlen;         //header length
    unsigned int time;          // time when building image
    unsigned int run;           // where to run
    unsigned int size;          // image size

    unsigned int load_addr;     // where to load
    unsigned short flags;       // flags
    unsigned short mid;         // model
    unsigned int ver;           // ver info
    unsigned short chksum;      //
    unsigned short resv;        //

    char desc[16];
} __attribute__ ((packed));

#define IH_MAGIC    { 'A', 'I', 'H' }
#define IH_CHKSUM_EN    (1<<2)
#define IH_NEXT_IMG     (1<<3)

#define BOOT_IMAGE_BE 1
#define BOOT_IMAGE_LE 2
int verify_image(unsigned int h);
int verify_ubimage(unsigned int h);
int is_secure_boot_image(unsigned long src, unsigned int len);
int is_boot_image(unsigned long src);
int load_image(char *h, unsigned long *loc);
int check_mptest();

#endif                          /* IMAGE_H */
