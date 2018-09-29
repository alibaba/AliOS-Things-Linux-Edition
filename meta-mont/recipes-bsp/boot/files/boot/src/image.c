/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file image.c
*   \brief Image Verification and Handler
*   \author Montage
*/

#include <common.h>
#include <lib.h>
#include <image.h>
#include <ubimage.h>
#include <LzmaDecode.h>

#define _LZMA_FIXED_INT_DATA
#include "LzmaDecode.c"
#include <arch/chip.h>
#include "sflash/include/flash_config.h"

#define swab16(x) \
     ((unsigned short)( \
         (((unsigned short)(x) & (unsigned short)0x00ffU) << 8) | \
         (((unsigned short)(x) & (unsigned short)0xff00U) >> 8) ))

#define swab32(x) \
     ((unsigned int)( \
         (((unsigned int)(x) & (unsigned int)0x000000ffUL) << 24) | \
         (((unsigned int)(x) & (unsigned int)0x0000ff00UL) <<  8) | \
         (((unsigned int)(x) & (unsigned int)0x00ff0000UL) >>  8) | \
         (((unsigned int)(x) & (unsigned int)0xff000000UL) >> 24) ))

int compute_image_size(struct img_head *h);

/*!
 * function:
 *
 *  \brief
 *  \param argc
 *  \param argv
 *  \return
 */
int cmd_unz(int argc, char *argv[])
{
    unsigned long src, dst;
    unsigned long size;
    int len = 0;
    int rc = 0;

    if (argc < 3)
        goto help;
    if (!hextoul(argv[0], &src))
        goto err1;
    if (!hextoul(argv[1], &dst))
        goto err1;
    if (!hextoul(argv[2], &size))
        goto err1;

    rc = lzmaBuffToBuffDecompress((char *) dst, &len, (char *) src, size, 0);
    if (rc != LZMA_RESULT_OK)
    {
        printf("unz err:%d\n", rc);
        return ERR_PARM;
    }
    printf("unz len=%d(0x%x)\n", len, len);
    return ERR_OK;
  err1:
    rc = ERR_PARM;
  help:
    printf("%s src dst len", argv[-1]);
    return rc;
}

cmdt cmdt_unz __attribute__ ((section("cmdt"))) =
{
"unz", cmd_unz, "unz src dst zlen; lzma "};

char ih_magic[3] = IH_MAGIC;

/*!
 * function:
 *
 *  \brief
 *  \param addr1
 *  \param addr2
 *  \return
 */
int chk_bootarea(unsigned int addr1, unsigned int addr2)
{
    unsigned int image_size;
    unsigned short flag, *p;
    int rc1, rc2;

    rc2 = verify_image(addr2);
    if (rc2 <= 0)
    {
        rc1 = verify_image(addr1);
        if (rc1 <= 0)
            return rc1;
        return 1;
    }
    else
        image_size = compute_image_size((void *) addr2);

    rc1 = verify_image(addr1);
    if (rc1 <= 0)
        return 2;

    p = (unsigned short *) addr2;
    flag = *(p + ((image_size + 1) & ~1) / 2);

    if (flag != 1)
        return 1;
    else
        return 2;
}

/*!
 * function:
 *
 *  \brief
 *  \param h
 *  \param loc
 *  \return
 */
int load_image(char *h, unsigned long *loc)
{
    struct img_head *ih = (struct img_head *) h;
    char *lz;
    int rc, len;
    unsigned int run, load;

    rc = verify_image((unsigned int) ih);
    if (rc <= 0)
        return rc;

#ifdef  CONFIG_UBIMAGE
    if (rc == 2)                //u-boot comptable
    {
        image_header_t *uh = (image_header_t *) (h - sizeof(image_header_t));
//      lz = (char *) ih + sizeof (image_header_t);
#if defined(BIG_ENDIAN)
        load = uh->ih_load;
        run = uh->ih_ep;
        len = uh->ih_size;
#else
        load = swab32(uh->ih_load);
        run = swab32(uh->ih_ep);
        len = swab32(uh->ih_size);
#endif
        *loc = run;
        dbg_log(LOG_VERBOSE, "load = 0x%x\n", load);
        dbg_log(LOG_VERBOSE, "run = 0x%x\n", run);
        return len;
    }
    else
#endif
    {
        lz = (char *) ih + ih->hlen;
        load = run = ih->run;
        if(ih->load_addr)
           load = ih->load_addr;
    }
    if (*lz != 0x5d && 0 != *(lz + 1))
    {
        printf("IH not lzma\n");
        return -1;
    }

    if (vaddr_check((unsigned long) run))
    {
        printf("IH run @(%08x) err\n", run);
        return -1;
    }
    *loc = run;

    printf("load = 0x%x\n", load);
    printf("lz = 0x%x\n", lz);
    printf("ih->size = 0x%x\n", ih->size);
    rc = lzmaBuffToBuffDecompress((char *) load, &len, lz, ih->size,
                                  check_stop_condition);
    printf("len = 0x%x\n", len);

    if (rc == LZMA_RESULT_STOP_BY_USER)
    {
        return -2;
    }
    else if (rc != LZMA_RESULT_OK)
    {
        printf("img unz err:%d\n", rc);
        return 0;
    }
    printf("img unz len=%d(0x%x)\n", len, len);
    return len;
}

/*!
 * function:
 *
 *  \brief
 *  \param h
 *  \return
 */
int compute_image_size(struct img_head *h)
{
    unsigned int non_www_size, www_size = 0;
    struct img_head *www_ih;

    non_www_size = h->hlen + h->size;
    if (h->flags & IH_NEXT_IMG)
    {
        www_ih = (struct img_head *) ((char *) h + h->hlen + h->size);
        www_size = www_ih->hlen + www_ih->size;
    }
    return (non_www_size + www_size);
}

#ifndef CONFIG_UBIMAGE
#define verify_ubimage(h) (-1)
#endif
/*!
 * function:
 *
 *  \brief
 *  \param h
 *  \return -1:magic failed 0:checksum failed 1:normal image 2:uboot image
 */
int verify_image(unsigned int h)
{
    struct img_head *ih1 = (struct img_head *) h;
    int rc;

    if (memcmp(&ih1->magic[0], &ih_magic[0], 3))
    {
        //printf("ih1 magic err:%08x\n", *((int*)&ih1->magic[0]));
        rc = verify_ubimage(h);
        return rc;
    }

    if (ih1->flags & IH_CHKSUM_EN)
    {
        rc = 0xffff ^ ipchksum(ih1, ih1->hlen + ih1->size);
        if (rc)
        {
            printf("ih1 chksum err:%04x\n", rc);
            return 0;
        }
    }

    if (ih1->flags & IH_NEXT_IMG)
    {
        struct img_head *ih2 =
            (struct img_head *) ((char *) ih1 + ih1->hlen + ih1->size);
        if (memcmp(&ih2->magic[0], &ih_magic[0], 3))
        {
            printf("ih2 magic err:%08x\n", *((int *) &ih2->magic[0]));
            return -1;
        }

        if (ih2->flags & IH_CHKSUM_EN)
        {
            rc = 0xffff ^ ipchksum(ih2, ih2->hlen + ih2->size);
            if (rc)
            {
                printf("ih2 chksum err:%04x\n", rc);
                return 0;
            }
        }
    }

#if 0
    if (ih1->hlen != 0x30)
    {
        printf("IH len %x!=0x30\n", (unsigned int) ih1->hlen);
        return -1;
    }
    if (ih2->hlen != 0x30)
    {
        printf("IH len %x!=0x30\n", (unsigned int) ih2->hlen);
        return -1;
    }
#endif
    //printf("boot verify_image ....O.K\n");
    return 1;
}

#ifdef  CONFIG_UBIMAGE
#include <src/crc32.c>

static unsigned long ubih_magic = IH_UBMAGIC;

#if defined(BIG_ENDIAN)
/*!
 * function:
 *
 *  \brief verify uImage in "big endian"
 *  \param h
 *  \return
 */
int verify_ubimage(unsigned int h)
{
    image_header_t *ih1 = (image_header_t *) (h - sizeof(image_header_t));
    unsigned int crc_ck, hcrc;
    image_header_t th;

    if (memcmp(&ih1->ih_magic, &ubih_magic, 4))
    {
        //dbg_log(LOG_VERBOSE, "Invalid uboot magic:%08x\n", ih1->ih_magic);
        return -1;
    }
    memcpy(&th, ih1, sizeof (th));
    hcrc = th.ih_hcrc;
    th.ih_hcrc = 0;
    crc_ck = crc32(0, (void *) &th, sizeof (th));
    if (crc_ck != hcrc)
    {
        printf("Invalid uboot hcrc:%08x(org:%x)\n", crc_ck, hcrc);
        return 0;
    }

    crc_ck = crc32(0, (void *) (ih1 + 1), ih1->ih_size);
    if (crc_ck != ih1->ih_dcrc)
    {
        printf("Invalid uboot dcrc:%08x(org:%x)\n", crc_ck, ih1->ih_dcrc);
        return 0;
    }

    //printf("%s : OK\n", __func__);
    return 2;
}
#else
/*!
 * function:
 *
 *  \brief verify uImage in "little endian"
 *  \param h
 *  \return
 */
int verify_ubimage(unsigned int h)
{
    image_header_t *ih1 = (image_header_t *) (h - sizeof(image_header_t));
    unsigned int crc_ck, hcrc, dcrc;
    image_header_t th;
    unsigned int ubmagic;

    ubmagic = swab32(ih1->ih_magic);
    if (memcmp(&ubmagic, &ubih_magic, 4))
    {
        //dbg_log(LOG_VERBOSE, "Invalid uboot magic:%08x\n", ubmagic);
        return -1;
    }
    memcpy(&th, ih1, sizeof (th));
    hcrc = swab32(th.ih_hcrc);
    th.ih_hcrc = 0;
    crc_ck = crc32(0, (void *) &th, sizeof (th));
    if (crc_ck != hcrc)
    {
        printf("Invalid uboot hcrc:%08x(org:%x)\n", crc_ck, hcrc);
        return 0;
    }

    // data CRC need to fix when we maked uzImage
    dcrc = swab32(th.ih_dcrc);
    crc_ck = (crc32(0, (void *) (h), swab32(ih1->ih_size)));
    if (crc_ck != dcrc)
    {
        printf("Invalid uboot dcrc:%08x(org:%x)\n", crc_ck, dcrc);
        return 0;
    }

    //printf("%s : OK\n", __func__);
    return 2;
}
#endif

#endif

/*!
 * function:
 *
 *  \brief
 *  \return
 */
int check_mptest()
{
    unsigned int load_loc = CONFIG_FLASH_BASE + CONFIG_BOOT_MPTEST_OFS;
    int ret = 0;

    if ((ret = verify_image(load_loc)) <= 0)
    {
        return 0;
    }
    else
    {
        printf("mptest image exist\n");
        return 1;
    }
}
