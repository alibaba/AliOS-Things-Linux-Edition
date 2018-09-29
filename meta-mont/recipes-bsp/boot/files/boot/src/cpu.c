/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file cpu.c
*   \brief Cache Flash/Invalidate function
*   \author Montage
*/

#include <arch/cpu.h>

/*!
 * function:
 *
 *  \brief
 *  \return
 */

void dcache_flush(void)
{
    HAL_DCACHE_FLUSH_ALL();
}

/*!
 * function:
 *
 *  \brief
 *  \param addr
 *  \param len
 *  \return
 */

void dcache_flush_range(void *addr, int len)
{
    HAL_DCACHE_FLUSH(addr, len);
}

/*!
 * function:
 *
 *  \brief
 *  \param addr
 *  \param len
 *  \return
 */

void dcache_inv_range(void *addr, int len)
{
    HAL_DCACHE_INVALIDATE(addr, len);
}

/*!
 * function:
 *
 *  \brief
 *  \return
 */

void icache_inv(void)
{
    HAL_ICACHE_INVALIDATE_ALL();
}

#ifdef WIFI_CHECK
/**
 * fls - find last (most-significant) bit set
 * @x: the word to search
 *
 * This is defined the same way as ffs.
 * Note fls(0) = 0, fls(1) = 1, fls(0x80000000) = 32.
 */

int fls(int x)
{
    int r = 32;

    if (!x)
        return 0;
    if (!(x & 0xffff0000u))
    {
        x <<= 16;
        r -= 16;
    }
    if (!(x & 0xff000000u))
    {
        x <<= 8;
        r -= 8;
    }
    if (!(x & 0xf0000000u))
    {
        x <<= 4;
        r -= 4;
    }
    if (!(x & 0xc0000000u))
    {
        x <<= 2;
        r -= 2;
    }
    if (!(x & 0x80000000u))
    {
        x <<= 1;
        r -= 1;
    }
    return r;
}

/**
 * ffs - find first (last-significant) bit set
 * @x: the word to search
 */
int ffs(int x)
{
    int r = 1;

    if (!x)
        return 0;
    if (!(x & 0xffff))
    {
        x >>= 16;
        r += 16;
    }
    if (!(x & 0xff))
    {
        x >>= 8;
        r += 8;
    }
    if (!(x & 0xf))
    {
        x >>= 4;
        r += 4;
    }
    if (!(x & 3))
    {
        x >>= 2;
        r += 2;
    }
    if (!(x & 1))
    {
        x >>= 1;
        r += 1;
    }
    return r;
}

#endif
