/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file mem_test.c
*   \brief Simple Memory Test
*   \author
*/

#include <common.h>
#include <lib.h>

#ifdef CONFIG_CMD_MT

#ifdef  CMD_MEM_DEBUG
#define PRINTF(fmt,args...) printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif

#define CFG_ALT_MEMTEST

#define WATCHDOG_RESET()
#define vu_long volatile unsigned long
#define ulong volatile unsigned long
#define uint unsigned int
#define putc serial_putc

extern int sdram_size(void);

int ctrlc()
{
    int c;
    if (serial_poll())
    {
        c = serial_getc();
        if (c == 3 || c == 27)
            return 1;
    }
    return 0;
}

/*
 * Return a memory test length
 */
int sdram_test_size(unsigned int *pbase)
{
    int sdram_sz = sdram_size();
    unsigned int base = virt_to_phy(*pbase);
    unsigned int text = virt_to_phy(_ftext);
    unsigned int test_sz;
    printf("sdram size = %x\n", sdram_sz);
    if (base < virt_to_phy(CONFIG_GENERAL_EXCEPTION_VECTOR + VECTOR_JUMP_SIZE))
        base = virt_to_phy(CONFIG_GENERAL_EXCEPTION_VECTOR + VECTOR_JUMP_SIZE);
#ifdef CONFIG_LOAD_BOOT2_TO_SRAM
    test_sz = sdram_sz - base;
#else
    test_sz = (text & (sdram_sz - 1)) - base;
#endif
    *pbase = ((*pbase) & 0xf0000000) + base;
    printf("base=%x test_sz=%x\n", *pbase, test_sz);
    return test_sz;
}

/*
 * Perform a memory test. A more complete alternative test can be
 * configured using CFG_ALT_MEMTEST. The complete test loops until
 * interrupted by ctrl-c or by a failure of one of the sub-tests.
 */
int mem_test_cmd(int argc, char *argv[])
{
    vu_long *addr, *start, *end;
    ulong val;
    ulong readback;
    int rcode = 0;
    int max_iteration = 1;
    int loop = 0;
    unsigned int mt_start = 0xa0000000;
    unsigned int mt_len = 0;

#if defined(CFG_ALT_MEMTEST)
    vu_long len;
    vu_long offset;
    vu_long test_offset;
    vu_long pattern;
    vu_long temp;
    vu_long anti_pattern;
    vu_long num_words;
    vu_long _dummy;
    vu_long *dummy = (vu_long *) & _dummy;
    int j;

    static const ulong bitpattern[] = {
        0x00000001,             /* single bit */
        0x00000003,             /* two adjacent bits */
        0x00000007,             /* three adjacent bits */
        0x0000000F,             /* four adjacent bits */
        0x00000005,             /* two non-adjacent bits */
        0x00000015,             /* three non-adjacent bits */
        0x00000055,             /* four non-adjacent bits */
        0xaaaaaaaa,             /* alternating 1/0 */
    };
#else
    ulong incr;
    ulong pattern;
#endif

    if (argc > 0)
    {
        if (!hextoul(argv[0], &mt_start))
            goto err1;
    }
    if (argc > 1)
    {
        if (!hextoul(argv[1], &mt_len))
            goto err1;
    }
//          val = sdram_test_size(&mt_start);
//          if (mt_len == 0 || mt_len > val)
//          {
//              mt_len = val;
//          }
    printf("test size = %x\n", mt_len);
    end = (vu_long *) (mt_start + mt_len);
    start = (vu_long *) mt_start;
    pattern = 0x12345678;
    if (argc > 2 && !hextoul(argv[2], (void *) &pattern))
        goto err1;
    if (argc > 3 && !hextoul(argv[3], &max_iteration))
        goto err1;

    if (vaddr_check((unsigned long) start) || vaddr_check((unsigned long) end))
        goto err2;
#if defined(CFG_ALT_MEMTEST)
    printf("Testing %08x ... %08x:\n", (uint) start, (uint) end);
    PRINTF("%s:%d: start 0x%p end 0x%p\n", __FUNCTION__, __LINE__, start, end);

    for (; loop < max_iteration; loop++)
    {
        if (ctrlc())
        {
            putc('\n');
            return ERR_HELP;
        }

        printf("Iteration: %6d\r", loop + 1);

        /*
         * Data line test: write a pattern to the first
         * location, write the 1's complement to a 'parking'
         * address (changes the state of the data bus so a
         * floating bus doen't give a false OK), and then
         * read the value back. Note that we read it back
         * into a variable because the next time we read it,
         * it might be right (been there, tough to explain to
         * the quality guys why it prints a failure when the
         * "is" and "should be" are obviously the same in the
         * error message).
         *
         * Rather than exhaustively testing, we test some
         * patterns by shifting '1' bits through a field of
         * '0's and '0' bits through a field of '1's (i.e.
         * pattern and ~pattern).
         */
        addr = start;
        for (j = 0; j < sizeof (bitpattern) / sizeof (bitpattern[0]); j++)
        {
            val = bitpattern[j];
            for (; val != 0; val <<= 1)
            {
                *addr = val;
                *dummy = ~val;  /* clear the test data off of the bus */
                readback = *addr;
                if (readback != val)
                {
                    printf("FAILURE (data line): "
                           "expected %08x, actual %08x\n", val, readback);
                }
                *addr = ~val;
                *dummy = val;
                readback = *addr;
                if (readback != ~val)
                {
                    printf("FAILURE (data line): "
                           "Is %08x, should be %08x\n", readback, ~val);
                }
            }
        }

        /*
         * Based on code whose Original Author and Copyright
         * information follows: Copyright (c) 1998 by Michael
         * Barr. This software is placed into the public
         * domain and may be used for any purpose. However,
         * this notice must not be changed or removed and no
         * warranty is either expressed or implied by its
         * publication or distribution.
         */

        /*
         * Address line test
         *
         * Description: Test the address bus wiring in a
         *              memory region by performing a walking
         *              1's test on the relevant bits of the
         *              address and checking for aliasing.
         *              This test will find single-bit
         *              address failures such as stuck -high,
         *              stuck-low, and shorted pins. The base
         *              address and size of the region are
         *              selected by the caller.
         *
         * Notes:   For best results, the selected base
         *              address should have enough LSB 0's to
         *              guarantee single address bit changes.
         *              For example, to test a 64-Kbyte
         *              region, select a base address on a
         *              64-Kbyte boundary. Also, select the
         *              region size as a power-of-two if at
         *              all possible.
         *
         * Returns:     0 if the test succeeds, 1 if the test fails.
         */
        len = ((ulong) end - (ulong) start) / sizeof (vu_long);
        pattern = (vu_long) 0xaaaaaaaa;
        anti_pattern = (vu_long) 0x55555555;

        PRINTF("%s:%d: length = 0x%8x\n", __FUNCTION__, __LINE__, len);
        /*
         * Write the default pattern at each of the
         * power-of-two offsets.
         */
        for (offset = 1; offset < len; offset <<= 1)
        {
            start[offset] = pattern;
        }

        /*
         * Check for address bits stuck high.
         */
        test_offset = 0;
        start[test_offset] = anti_pattern;

        for (offset = 1; offset < len; offset <<= 1)
        {
            temp = start[offset];
            if (temp != pattern)
            {
                printf("\nFAILURE: Address bit stuck high @ 0x%8x:"
                       " expected 0x%8x, actual 0x%8x\n",
                       (ulong) & start[offset], pattern, temp);
                return ERR_HELP;
            }
        }
        start[test_offset] = pattern;
        WATCHDOG_RESET();

        /*
         * Check for addr bits stuck low or shorted.
         */
        for (test_offset = 1; test_offset < len; test_offset <<= 1)
        {
            start[test_offset] = anti_pattern;

            for (offset = 1; offset < len; offset <<= 1)
            {
                temp = start[offset];
                if ((temp != pattern) && (offset != test_offset))
                {
                    printf("\nFAILURE: Address bit stuck low or shorted @"
                           " 0x%8x: expected 0x%8x, actual 0x%8x\n",
                           (ulong) & start[offset], pattern, temp);
                    return ERR_HELP;
                }
            }
            start[test_offset] = pattern;
        }

        /*
         * Description: Test the integrity of a physical
         *      memory device by performing an
         *      increment/decrement test over the
         *      entire region. In the process every
         *      storage bit in the device is tested
         *      as a zero and a one. The base address
         *      and the size of the region are
         *      selected by the caller.
         *
         * Returns:     0 if the test succeeds, 1 if the test fails.
         */
        num_words = ((ulong) end - (ulong) start) / sizeof (vu_long) + 1;

        /*
         * Fill memory with a known pattern.
         */
        for (pattern = 1, offset = 0; offset < num_words; pattern++, offset++)
        {
            WATCHDOG_RESET();
            start[offset] = pattern;
        }

        /*
         * Check each location and invert it for the second pass.
         */
        for (pattern = 1, offset = 0; offset < num_words; pattern++, offset++)
        {
            WATCHDOG_RESET();
            temp = start[offset];
            if (temp != pattern)
            {
                printf("\nFAILURE (read/write) @ 0x%8x:"
                       " expected 0x%8x, actual 0x%8x)\n",
                       (ulong) & start[offset], pattern, temp);
                return ERR_HELP;
            }

            anti_pattern = ~pattern;
            start[offset] = anti_pattern;
        }

        /*
         * Check each location for the inverted pattern and zero it.
         */
        for (pattern = 1, offset = 0; offset < num_words; pattern++, offset++)
        {
            WATCHDOG_RESET();
            anti_pattern = ~pattern;
            temp = start[offset];
            if (temp != anti_pattern)
            {
                printf("\nFAILURE (read/write): @ 0x%8x:"
                       " expected 0x%8x, actual 0x%8x)\n",
                       (ulong) & start[offset], anti_pattern, temp);
                return ERR_HELP;
            }
            start[offset] = 0;
        }
    }

#else                           /* The original, quickie test */
    incr = 1;
    for (; loop < max_iteration; loop++)
    {
        if (ctrlc())
        {
            putc('\n');
            return ERR_HELP;
        }

        printf("\rPattern %08lX  Writing..."
               "%12s" "\b\b\b\b\b\b\b\b\b\b", pattern, "");

        for (addr = start, val = pattern; addr < end; addr++)
        {
            WATCHDOG_RESET();
            *addr = val;
            val += incr;
        }

        puts("Reading...");

        for (addr = start, val = pattern; addr < end; addr++)
        {
            WATCHDOG_RESET();
            readback = *addr;
            if (readback != val)
            {
                printf("\nMem error @ 0x%08X: "
                       "found %08lX, expected %08lX\n",
                       (uint) addr, readback, val);
                rcode = ERR_MEM;
            }
            val += incr;
        }

        /*
         * Flip the pattern each time to make lots of zeros and
         * then, the next time, lots of ones.  We decrement
         * the "negative" patterns and increment the "positive"
         * patterns to preserve this feature.
         */
        if (pattern & 0x80000000)
        {
            pattern = -pattern; /* complement & increment */
        }
        else
        {
            pattern = ~pattern;
        }
        incr = -incr;
    }
#endif
    return rcode;

  err1:
    return ERR_PARM;
  err2:
    return ERR_ADDR;
}

cmdt cmdt_mt __attribute__ ((section("cmdt"))) =
{
"mt", mem_test_cmd, "mt <addr> <len> <pattern> <loop>"};
#endif
