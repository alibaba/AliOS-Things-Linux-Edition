/*!
*   \file ntv_mt_main.c
*   \brief Memory Test
*   \author
*/
/*
+----------------------------------------------------------------------------+
|         (c) Copyright  Embedded Performance Incorporated,  1991-99, 2000   |
|                           All rights reserved                              |
|                                                                            |
| This software is furnished under a license and may be used and copied      |
| only in accordance with the terms of the license and with the inclusion    |
| of the above copyright notice.  This software or any other copies          |
| thereof may not be provided or otherwise made available to any other       |
| person.  No title to or ownership of the software is hereby transferred.   |
|                                                                            |
| The information in this software is subject to change without notice       |
| and should not be construed as a commitment by Embedded Performance.       |
|                                                                            |
| Embedded Performance assumes no responsibility for the use and reliability |
| of its software on equipment which is not supplied by Embedded Performance.|
+----------------------------------------------------------------------------+



MODULE NAME:    MEM_TEST.C
DESCRIPTION:

    This module implements the memory test feature by calling functions in the
target specific module to read and write memory.

NOTES:  All memory tests are performed word wide, and therefore the start and
        end addresses must be word aligned.

SPECIAL COMPILE/LINK INSTRUCTIONS:

..............................................................................
*/

#ifdef CONFIG_CMD_MT2
#include <common.h>
#include <lib.h>
#include "ntv_mt.h"
#include "mem_test.h"

    /* Assumes 0's are shifted in from both sides */

#define ROT_ADDR(A,R)   ( (LOWER_ADDR(A) >> (32-R)) | (LOWER_ADDR(A) << R) )


    /*  STATIC VARIABLES  */

LOCAL   ULONG   error_cnt;
LOCAL   BOOL    error_msg;      /* TRUE to send error indication to the host */
LOCAL   BOOL    halt_on_error;  /* TRUE to halt if error detected */
//LOCAL TEXT    *msg_derr_loop = "%s:DERR(s) occured during test\n";
LOCAL   ULONG   mtst_bytes  = 0x11223344;   /* Byte  test patterns */
LOCAL   ULONG   mtst_hwords = 0x12345678;   /* Hword test patterns */
LOCAL   ULONG   mtst_pattern [] =
{
    0x00000000, 0xFFFFFFFF, 0xAAAAAAAA, 0x55555555, 0x01234567, 0x89ABCDEF
};

LOCAL   ULONG   pass_cnt;
LOCAL   USHORT  test_id;        /* Test number in progress */
LOCAL   BOOL    verbose;        /* TRUE to send message at start of test    */



    /*  EXTERNAL VARIABLES  */

#ifdef MAJIC
extern  UCHAR   co_ice_trigger_out; /* Specifies when TRIGGER OUT is asserted */
#endif

extern  UINT            debug_cats;
extern  UINT            debug_level;
int         derr_jmpbuf;        /* used to retain control upon memory errors */
extern  BOOL            flag_derr_jmpbuf;   /* used to retain control upon memory errors */
extern  volatile UINT1  received_break; /* set TRUE when break packet received  */
extern  volatile UINT1  tsi_error;

extern  S_CONFIG        gen_config;


    /* INTERNAL FUNCTION PROTOTYPES */

LOCAL   UINT    fill_range      (TGTADDR address, TGTADDR end, ULONG space,
                                    ULONG pattern, BOOL forward, BOOL verify);

LOCAL   UINT    memtest_adr_rot (TGTADDR start, TGTADDR end, ULONG space,
                                    BOOL comp);

LOCAL   UINT    memtest_basic   (TGTADDR start, TGTADDR end, ULONG space);
LOCAL   UINT    memtest_bytes   (TGTADDR address, TGTADDR end, ULONG space);
LOCAL   UINT    memtest_combination(TGTADDR start, TGTADDR end, ULONG space);
LOCAL   UINT    memtest_hwords  (TGTADDR address, TGTADDR end, ULONG space);
LOCAL   UINT    memtest_refresh (TGTADDR start, TGTADDR end, ULONG space,
                                    ULONG delay);

LOCAL   UINT    memtest_walking (TGTADDR start, TGTADDR end, ULONG space);
LOCAL   UINT    mem_read_loop   (TGTADDR start, TGTADDR end, ULONG space);
LOCAL   UINT    mem_write_loop  (TGTADDR start, TGTADDR end, ULONG space,
                                    ULONG pattern);

LOCAL   UINT    mem_wr_rd_loop  (TGTADDR start, TGTADDR end, ULONG space,
                                    ULONG pattern);

LOCAL   UINT    verify_range    (TGTADDR address, TGTADDR end, ULONG space,
                                    ULONG pattern, BOOL forward);

LOCAL   UINT    verify_rot_address(TGTADDR address, TGTADDR end, ULONG space,
                                    UINT rotate, ULONG mask, BOOL forward);

LOCAL   UINT    walk_a_bit      (TGTADDR start, TGTADDR end, ULONG space,
                                    ULONG pattern);

LOCAL   UINT    write_rot_address(TGTADDR start, TGTADDR end, ULONG space,
                                    UINT rotate, ULONG mask, BOOL forward);

/*
..............................................................................
FUNCTION NAME:  fill_range
DESCRIPTION:

    This function fills a range of memory with the specified data pattern.
Optionally, each write may be read back and verified.

NOTES:
..............................................................................
*/

LOCAL UINT fill_range (
    TGTADDR     start,      /* Start address (hi or lo) */
    TGTADDR     end,        /* End address (inclusive)  */
    ULONG       space,      /* Address space            */
    ULONG       pattern,    /* test pattern             */
    BOOL        forward,    /* TRUE to increment addr   */
    BOOL        verify  )   /* TRUE to verify writes    */
{
    TGTADDR     address;
    UINT        status;
    ULONG       data;
    S_ACCESS    sac, fac;

    status = TEST_PASSED;

    setup_address( &sac, start, space );

    if (verify)
        setup_address( &fac, start, space );

    if (forward)
        end = TGTADDR_PLUS_ULONG(end, sizeof(pattern));
    else
        end = TGTADDR_MINUS_ULONG(end, sizeof(pattern));

    while ( TGTADDR_NE_TGTADDR(sac.next, end) )
    {
        if (received_break)
        {
            received_break = FALSE;
            status = TEST_ABORTED;
            break;
        }

        store_next( &sac, forward, sizeof(pattern), pattern );

        if (verify)
        {
            address = fac.next;
            fac.last_wrd_addr = TGTADDR_PLUS_ULONG(fac.next, sizeof(data));
            data    = fetch_next( &fac, forward, sizeof(data) );

            if (data != pattern)
            {
                if (memtest_failed( MT_ERR_WRV, address, pattern, data ))
                {
                    status = TEST_ABORTED;
                    break;
                }
                else
                    status = TEST_FAILED;
            }
        }
    }

    return (status);
}

/*
..............................................................................
FUNCTION NAME:  memtest_adr_rot
DESCRIPTION:

    This function performs a rotating address test on the specified address
range. First it fills the entire range with its own address. Then it reads and
verifies the entire range. This is repeated eight times, with the address
being rotated by 4 bits each pass. Then the whole process is repeated with
decrementing addresses. Optionally, the rotated addresses may be complimented
before being written. If these tests pass, a TRUE is returned, otherwise a
FALSE.

NOTES:
..............................................................................
*/

LOCAL UINT memtest_adr_rot (
    TGTADDR start,      /* Start address (low)  */
    TGTADDR end,        /* End address  (high)  */
    ULONG   space,      /* Address space        */
    BOOL    comp    )   /* TRUE to compliment   */
{
    ULONG   mask;
    UINT    rotate;
    UINT    status;
    USHORT  test_type;

    status = TEST_PASSED;

    if (comp)
    {
        test_id = MT_ID_CMP_ADR;
        mask    = 0xFFFFFFFF;       /* XOR with F's to compliment */
        test_type   = MTST_TESTID_3;
    }
    else
    {
        test_id = MT_ID_ROT_ADR;
        mask    = 0;                /* Don't compliment addresses */
        test_type   = MTST_TESTID_2;
    }

    /* Test incrementing addresses  */

    for (rotate = 0; (status != TEST_ABORTED) && (rotate < 32);  rotate += 4)
    {
        if (received_break)
        {
            received_break = FALSE;
            status = TEST_ABORTED;
            break;
        }

        if (verbose)
            send_mt_status( test_type, rotate, TRUE /* send_pat */ );

        if (write_rot_address( start, end, space, rotate, mask, TRUE)
                == TEST_ABORTED)
        {
            status = TEST_ABORTED;
            break;
        }
        else
        {
            status = verify_rot_address( start, end, space, rotate, mask, TRUE);
        }
    }

    if (status != TEST_ABORTED)
    {
        ++test_id;              /* Retest with decrementing addresses */

        for (rotate = 0; (status == TEST_PASSED) && (rotate < 32); rotate += 4)
        {
            if (received_break)
            {
                received_break = FALSE;
                status = TEST_ABORTED;
                break;
            }

            if (verbose)
            {
                send_mt_status( test_type, rotate, TRUE /* send_pat */ );
            }

            if (write_rot_address( start, end, space, rotate, mask, TRUE)
                    == TEST_ABORTED)
            {
                status = TEST_ABORTED;
                break;
            }
            else
            {
                status = verify_rot_address( start, end, space,
                        rotate, mask, TRUE);
            }
        }
    }

    return (status);
}

/*
..............................................................................
FUNCTION NAME:  memtest_basic
DESCRIPTION:

    This function performs a basic data pattern memory test on the specified
address range. It will test all of the data patterns in a forward direction,
then in reverse. If these tests pass, a TRUE is returned, otherwise a FALSE.

NOTES:
..............................................................................
*/

LOCAL UINT memtest_basic (
    TGTADDR start,      /* Start address (low)  */
    TGTADDR end,        /* End address  (high)  */
    ULONG   space   )   /* Address space        */
{
    UINT    i, p_cnt;
    ULONG   *pattern, pat;
    UINT    status = TEST_PASSED;

    p_cnt   = sizeof(mtst_pattern) / sizeof(mtst_pattern[0]);
    test_id = MT_ID_PAT_INC;

    for (i = 0, pattern = mtst_pattern; (status != TEST_ABORTED) && (i < p_cnt);
            ++i, ++pattern)
    {
        if (received_break)
        {
            received_break = FALSE;
            status = TEST_ABORTED;
            break;
        }

        pat = *pattern;

        if (verbose)
            send_mt_status( MTST_TESTID_1, pat,  TRUE /* send_pat */ );

        /* Write, verify then reverify incrementing addresses   */

        status = fill_range( start, end, space, pat, TRUE, TRUE );

        if (status != TEST_ABORTED)
        {
            if (verbose)
                send_mt_status( MTST_TESTID_1, ~pat, TRUE /* send_pat */ );

            status = verify_range( start, end, space, pat, TRUE );
        }
    }

    if (status != TEST_ABORTED)
    {
        test_id = MT_ID_PAT_DEC;

        /* Write, verify then reverify decrementing addresses   */

        for (i = 0, pattern = mtst_pattern;
                (status != TEST_ABORTED) && (i < p_cnt);
                ++i, ++pattern)
        {
            if (received_break)
            {
                received_break = FALSE;
                status = TEST_ABORTED;
                break;
            }

            pat = *pattern;

            if (verbose)
                send_mt_status( MTST_TESTID_1, pat, TRUE /* send_pat */ );

            status = fill_range( end, start, space, pat, FALSE, TRUE );

            if (status != TEST_ABORTED)
            {
                if (verbose)
                    send_mt_status( MTST_TESTID_1, ~pat, TRUE /* send_pat */ );

                status = verify_range( end, start, space, pat, FALSE);
            }
        }
    }

    return (status);
}

/*
..............................................................................
FUNCTION NAME:  memtest_bytes
DESCRIPTION:

    This function tests byte access capability on the specified address range.
If these tests pass, a TRUE is returned, otherwise a FALSE.

NOTES:  It is not particularly useful to run this test against memory which is
        not byte accessible.
..............................................................................
*/

LOCAL UINT memtest_bytes (
    TGTADDR address,    /* Start address (low)  */
    TGTADDR end,        /* End address  (high)  */
    ULONG   space   )   /* Address space        */
{
    S_ACCESS pac;
    ULONG   read;
    UINT    status;

    if (verbose)
        send_mt_status( MTST_TESTID_6, mtst_bytes, TRUE /* send_pattern */ );

    status = TEST_PASSED;

    setup_address( &pac, address, space );
    end = TGTADDR_PLUS_ULONG(end, sizeof(mtst_bytes));

    while ( TGTADDR_NE_TGTADDR(pac.next, end) )
    {
        if (received_break)
        {
            received_break = FALSE;
            status = TEST_ABORTED;
            break;
        }

        address = pac.next;

        write_mem_block( sizeof(mtst_bytes), sizeof(UCHAR),
                address, space, (UCHAR *) &mtst_bytes );    /* Write 4 bytes */

        pac.last_wrd_addr = TGTADDR_PLUS_ULONG(pac.next,sizeof(mtst_bytes));
        read = FETCH_MEM(&pac, sizeof(read));

        if (read != mtst_bytes)                             /* Compare word */
        {
            if (memtest_failed( MT_ERR_WRV, address, mtst_bytes, read ))
            {
                status = TEST_ABORTED;
                break;
            }
            else
                status = TEST_FAILED;
        }

        store_next( &pac, TRUE /* forward */, sizeof(mtst_bytes), ~mtst_bytes);

        read_mem_block( sizeof(read), sizeof(UCHAR),
                address, space, (UCHAR *) &read );          /* read 4 ~bytes */

        if (read != ~mtst_bytes)                                    /* Compare word */
        {
            if (memtest_failed( MT_ERR_CWRV, address, ~mtst_bytes, read ))
            {
                status = TEST_ABORTED;
                break;
            }
            else
                status = TEST_FAILED;
        }
    }

    return( status );
}

/*
..............................................................................
FUNCTION NAME:  memtest_combination
DESCRIPTION:

    This function performs the basic pattern test, walking 1 and 0, address
rotation and address compliment test one after the other.

NOTES:
..............................................................................
*/

LOCAL UINT memtest_combination (
    TGTADDR start,      /* Start address (low)  */
    TGTADDR end,        /* End address  (high)  */
    ULONG   space   )   /* Address space        */
{
    UINT    status;

    status = memtest_basic( start, end, space );

    if (status != TEST_ABORTED)
    {
        status = memtest_walking( start, end, space );

        if (status != TEST_ABORTED)
        {
            status = memtest_adr_rot( start, end, space, FALSE );

            if (status != TEST_ABORTED)
            {
                status = memtest_adr_rot( start, end, space, TRUE );

                if (status != TEST_ABORTED)
                {
                    status = memtest_bytes( start, end, space );

                    if (status != TEST_ABORTED)
                    {
                        status = memtest_hwords( start, end, space );
                    }
                }

            }
        }
    }

    return (status);
}

/*
..............................................................................
FUNCTION NAME:  memtest_failed
DESCRIPTION:

    This function is called if any of the memory tests fail. It will display
an error message, if debug messages are enabled, and send an error message to
the host if the test is not running in silent mode.  If halt on error mode is
enabled, it will ask the host whether or not to abort, and return that flag:
TRUE to abort, FALSE to continue.

NOTES:
..............................................................................
*/

GLOBAL BOOL memtest_failed (
    SINT2   error,      /* Error number     */
    TGTADDR address,    /* Failed address   */
    ULONG   wrote,      /* Data written     */
    ULONG   read    )   /* Data read back   */
{
#ifdef MAJIC
    if (co_ice_trigger_out == CO_ENUM_ITO_MTERR)
        set_trigger_out(TRUE);
#endif
    ++error_cnt;

    if ( (debug_cats & DBG_MAIN)  &&  (debug_level > 1) )
    {
        printf( "\nmemtest error %d in test %d\n", error, test_id );

        if (debug_level > 2)
        {
#ifdef ADDR64
            printf( "  Addr:  0x%lX_%08lX\n", UPPER_ADDR(address), LOWER_ADDR(address) );
#else
            printf( "  Addr: %08lX\n", address );
#endif
            printf( "  Wrote: 0x%lX\n", wrote );
            printf( "  Read:  0x%lX\n", read );
        }
    }

#ifdef MAJIC
    if (co_ice_trigger_out == CO_ENUM_ITO_MTERR)
        set_trigger_out(FALSE);
#endif

    if ( error_msg || halt_on_error )
    {
        send_mt_error( -(error + (test_id << 8)), pass_cnt, address,
                        wrote, read );
    }

    if (halt_on_error)
        return( query_abort( ) );   /* Query for abort  */
    else
        return( FALSE );            /* Don't abort  */
}

/*
..............................................................................
FUNCTION NAME:  memtest_hwords
DESCRIPTION:

    This function tests halfword access capability on the specified address
range.  If these tests pass, a TRUE is returned, otherwise a FALSE.

NOTES:  It is not particularly useful to run this test against memory which is
        not halfword accessible.
..............................................................................
*/

LOCAL UINT memtest_hwords (
    TGTADDR address,    /* Start address (low)  */
    TGTADDR end,        /* End address  (high)  */
    ULONG   space   )   /* Address space        */
{
    S_ACCESS pac;
    ULONG   read;
    UINT    status;

    if (verbose)
        send_mt_status( MTST_TESTID_7, mtst_hwords, TRUE /* send_pattern */ );

    status = TEST_PASSED;

    setup_address( &pac, address, space );
    end = TGTADDR_PLUS_ULONG(end, sizeof(mtst_hwords));

    while ( TGTADDR_NE_TGTADDR(pac.next, end) )
    {
        if (received_break)
        {
            received_break = FALSE;
            status = TEST_ABORTED;
            break;
        }

        address = pac.next;

        write_mem_block( sizeof(mtst_hwords) / sizeof(USHORT), sizeof(USHORT),
                address, space, (UCHAR *) &mtst_hwords );   /* Write 2 Hwords */

        pac.last_wrd_addr = TGTADDR_PLUS_ULONG(pac.next,sizeof(mtst_hwords));
        read = FETCH_MEM(&pac, sizeof(read));

        if (read != mtst_hwords)                            /* Compare word */
        {
            if (memtest_failed( MT_ERR_WRV, address, mtst_hwords, read ))
            {
                status = TEST_ABORTED;
                break;
            }
            else
                status = TEST_FAILED;
        }

        store_next( &pac, TRUE /* forward */, sizeof(mtst_hwords), ~mtst_hwords);

        read_mem_block( sizeof(read) / sizeof(USHORT), sizeof(USHORT),
                address, space, (UCHAR *) &read );          /* read 2 ~Hwords */

        if (read != ~mtst_hwords)                           /* Compare word */
        {
            if (memtest_failed( MT_ERR_CWRV, address, ~mtst_hwords, read ))
            {
                status = TEST_ABORTED;
                break;
            }
            else
                status = TEST_FAILED;
        }
    }

    return( status );
}

/*
..............................................................................
FUNCTION NAME:  memtest_main
DESCRIPTION:

    This is the main entry point for the memory test module. It will perform
the specified memory test repeatedly, sending status messages to the host as
required. The test will end when the iteration count is reached, or if an
error is detected and the halt on error flag is set.

NOTES:
..............................................................................
*/

GLOBAL VOIDFN memtest_main (
    TGTADDR start,      /* Start address (lowest)   */
    TGTADDR end,        /* End address (highest)    */
    ULONG   space,      /* Address space to test    */
    USHORT  test,       /* Test number to perform   */
    USHORT  mode,       /* Halt, Verbose ...        */
    USHORT  repeat,     /* Iteration count          */
    ULONG   del_pat )   /* Test 8 delay, scope loop data */
{
    TGTADDR addr_align;
    BOOL    forever;
    UINT    status = TEST_PASSED;

    error_cnt = 0;
    pass_cnt  = 0;

    if (repeat == 0)
        forever = TRUE;
    else
        forever = FALSE;

    /* All memory tests are word aligned */
    addr_align = TGTADDR_FROM_2ULONGS(0xFFFFFFFF, ~(sizeof(ULONG) - 1) );
    start = TGTADDR_AND_TGTADDR(start, addr_align);
    end   = TGTADDR_AND_TGTADDR(end, addr_align);

    halt_on_error   = (mode & MTST_HALT_FLAG)   != 0;
    error_msg       = (mode & MTST_SILENT_FLAG) == 0;
    verbose         = (mode & MTST_VERBOSE_FLAG) != 0;

    while ( (status != TEST_ABORTED)
            && (forever || (repeat-- > 0)) )
    {
        if (received_break)
        {
            received_break = FALSE;
            status = TEST_ABORTED;
            break;
        }

        ++pass_cnt;

        switch (test)
        {
#ifdef TURBO
            case MEMTEST_MAP:
            {
                status = test_map_rams();
                break;
            }
#endif
            case MEMTEST_BASIC:
            {
                status = memtest_basic( start, end, space );
                break;
            }
            case MEMTEST_WALKING:
            {
                status = memtest_walking( start, end, space );
                break;
            }
            case MEMTEST_ADR_ROT:
            {
                status = memtest_adr_rot( start, end, space, FALSE );
                break;
            }
            case MEMTEST_ADR_CMP:
            {
                status = memtest_adr_rot( start, end, space, TRUE );
                break;
            }
            case MEMTEST_BYTES:
            {
                status = memtest_bytes( start, end, space );

                if (status != TEST_ABORTED)
                    status = memtest_hwords( start, end, space );

                break;
            }
            case MEMTEST_REFRESH:
            {
                status = memtest_refresh( start, end, space, del_pat );
                break;
            }
            case MEMTEST_ALL:
            {
                status = memtest_combination( start, end, space );
                break;
            }
            case MEMTEST_MEM_READ:
            {
                status = mem_read_loop( start, end, space );
                break;
            }
            case MEMTEST_MEM_WRITE:
            {
                status = mem_write_loop( start, end, space, del_pat );
                break;
            }
            case MEMTEST_MEM_WR_RD:
            {
                status = mem_wr_rd_loop( start, end, space, del_pat );
                break;
            }
            case MEMTEST_REG_READ:
            {
                /* reg_read_loop( );*/
                status = TEST_ABORTED;
                break;
            }
            case MEMTEST_REG_WRITE:
            {
                /* reg_write_loop( del_pat ); */
                status = TEST_ABORTED;
                break;
            }
            case MEMTEST_REG_WR_RD:
            {
                /* reg_wr_rd_loop( del_pat ); */
                status = TEST_ABORTED;
                break;
            }
            default:
            {
                status = TEST_ABORTED;
                break;
            }
        }

        if ( (status != TEST_ABORTED)
                && ((mode & MTST_QUIET_FLAG)  == 0)
                && ((mode & MTST_SILENT_FLAG) == 0) )
        {
            send_mt_end_pass();
        }
    }

    send_mt_end_test( pass_cnt );

}

/*
..............................................................................
FUNCTION NAME:  memtest_refresh
DESCRIPTION:

    This function tests the data retention of the specified address range.
First it fills and verifies the entire range with a test pattern. Then it
waits the specified time period, and rechecks the range. This is repeated with
each of the data test patterns. If these tests pass, a TRUE is returned,
otherwise a FALSE.

NOTES:
..............................................................................
*/

LOCAL UINT memtest_refresh (
    TGTADDR start,      /* Start address (low)  */
    TGTADDR end,        /* End address  (high)  */
    ULONG   space,      /* Address space        */
    ULONG   delay   )   /* Intertest wait       */
{
    UINT    i;
    ULONG   *pattern;
    UINT    status = TEST_PASSED;
    ULONG   time_stamp;

    test_id = MT_ID_REFRESH;

    for ( i = 0, pattern = mtst_pattern;
            (status != TEST_ABORTED) && (i < numof(mtst_pattern));
            ++i, ++pattern)
    {
        if (received_break)
        {
            received_break = FALSE;
            status = TEST_ABORTED;
            break;
        }

        if (verbose)
            send_mt_status( MTST_TESTID_1, *pattern, TRUE /* send_pat */);

        /* Write, verify, then reverify incrementing addresses  */

        status = fill_range( start, end, space, *pattern, TRUE, TRUE );

        if (status != TEST_ABORTED)
        {
            time_stamp = clock( );

            while ( (time_stamp + delay) > clock( ) )
            {
                if (received_break)
                {
                    received_break = FALSE;
                    status = TEST_ABORTED;
                    break;
                }
            }

            if (status != TEST_ABORTED)
                status = verify_range( start, end, space, *pattern, TRUE );
        }
    }

    return (status);
}

/*
..............................................................................
FUNCTION NAME:  memtest_walking
DESCRIPTION:

    This function performs walking ones and zeroes tests on the specified
address range. First it fills the entire range with the oposing bit, then the
bit is walked through all 32 bits of each address. If these tests pass, a TRUE
is returned, otherwise a FALSE.

NOTES:
..............................................................................
*/

LOCAL UINT memtest_walking (
    TGTADDR start,      /* Start address (low)  */
    TGTADDR end,        /* End address  (high)  */
    ULONG   space   )   /* Address space        */
{
    SINT    bit;
    ULONG   pattern;
    UINT    status = TEST_PASSED;

    for (bit = 1; (status != TEST_ABORTED) && (bit >= 0); --bit)
    {
        if (verbose)
        {
            if (bit == 1)
                send_mt_status( MTST_TESTID_5, 0, FALSE /* Don't send pat */ );
            else
                send_mt_status( MTST_TESTID_4, 0, FALSE /* Don't send pat */ );
        }

        test_id = MT_ID_WALK_0 - bit;
        pattern = bit - 1;

        /* Fill incrementing range without verifying    */

        status = fill_range( start, end, space, pattern, TRUE, FALSE );

        if (status != TEST_ABORTED)
        {
            status = walk_a_bit( start, end, space, pattern);
        }
    }

    return (status);
}

/*
..............................................................................
FUNCTION NAME:  mem_read_loop
DESCRIPTION:

    This function reads the specified address range as fast as it can, without
checking the data. If the address range can be successfully read, a TRUE is
returned. If the range cannot be read, then FALSE is returned.

NOTES:  The target memory is accessed directly by this function rather than
        calling fetch_word() to provide the fastest possible scope loop.

        The SYS29k does not send error messageS during this test, it is
        a new feature.
..............................................................................
*/

LOCAL UINT mem_read_loop (
    TGTADDR start,      /* Start address (low)  */
    TGTADDR end,        /* End address  (high)  */
    ULONG   space   )   /* Address space        */
{
    S_ACCESS    fac;
    UINT        status;
    UINT        jmp_code;

    status  = TEST_PASSED;
    test_id = MT_ID_SCOPE_RD;

    setup_address( &fac, start, space );

    if ( (jmp_code = setjmp(derr_jmpbuf)) != 0)
    {
        status = TEST_FAILED;
    }
    else
        flag_derr_jmpbuf = TRUE;

    while ( TGTADDR_LE_TGTADDR(fac.next, end) )
    {
        if (received_break)
        {
            received_break = FALSE;
            status = TEST_ABORTED;
            break;
        }

        (void) fetch_next( &fac, TRUE, sizeof(ULONG) );
    }

    flag_derr_jmpbuf = FALSE;

    return( status );
}

/*
..............................................................................
FUNCTION NAME:  mem_write_loop
DESCRIPTION:

    This function writes to the specified address range as fast as it can. The
data written alternates between the given pattern and its compliment. If the
address range can be successfully written, a TRUE is returned. If the range
cannot be written, then FALSE is returned.

NOTES:  The target memory is accessed directly by this function rather than
        calling store_word() to provide the fastest possible scope loop.

        The SYS29k does not send error messages during this test, it is
        a new feature.
..............................................................................
*/

LOCAL UINT mem_write_loop (
    TGTADDR start,      /* Start address (low)  */
    TGTADDR end,        /* End address  (high)  */
    ULONG   space,      /* Address space        */
    ULONG   pattern )   /* Pattern to write     */
{
    S_ACCESS    sac;
    UINT        status;
    UINT        jmp_code;

    status  = TEST_PASSED;
    test_id = MT_ID_SCOPE_WR;

    setup_address( &sac, start, space );

    if ( (jmp_code = setjmp(derr_jmpbuf)) != 0)
    {
        status = TEST_FAILED;
    }
    else
        flag_derr_jmpbuf = TRUE;

    while ( TGTADDR_LE_TGTADDR(sac.next, end) )
    {
        if (received_break)
        {
            received_break = FALSE;
            status = TEST_ABORTED;
            break;
        }

        store_next( &sac, TRUE, sizeof(pattern), pattern );

        pattern = ~pattern;
    }

    flag_derr_jmpbuf = FALSE;

    return( status );
}

/*
..............................................................................
FUNCTION NAME:  mem_wr_rd_loop
DESCRIPTION:

    This function writes to and reads from the specified address range as fast
as it can, without checking the data. If the address range can be successfully
accessed, a TRUE is returned. If the range cannot be accessed, then FALSE is
returned.

NOTES:  The target memory is accessed directly by this function rather than
        calling store_word() to provide the fastest possible scope loop.

        The SYS29k does not send error messages during this test, it is
        a new feature.
..............................................................................
*/

LOCAL UINT mem_wr_rd_loop (
    TGTADDR start,      /* Start address (low)  */
    TGTADDR end,        /* End address  (high)  */
    ULONG   space,      /* Address space        */
    ULONG   pattern )   /* Pattern to write     */
{
    S_ACCESS    fac, sac;
    UINT        status;
    UINT        jmp_code;

    status  = TEST_PASSED;
    test_id = MT_ID_SCOPE_WRRD;

    setup_address( &fac, start, space );
    setup_address( &sac, start, space );

    if ( (jmp_code = setjmp(derr_jmpbuf)) != 0)
    {
        status = TEST_FAILED;
    }
    else
        flag_derr_jmpbuf = TRUE;

    while ( TGTADDR_LE_TGTADDR( fac.next, end) )
    {
        if (received_break)
        {
            received_break = FALSE;
            status = TEST_ABORTED;
            break;
        }

        store_next( &sac, TRUE, sizeof(ULONG), pattern );

        pattern = ~pattern;

        (void) fetch_next( &fac, TRUE, sizeof(ULONG) );
    }

    flag_derr_jmpbuf = FALSE;

    return( status );
}


/*
..............................................................................
FUNCTION NAME:  verify_range
DESCRIPTION:

    This function verifies that every address in the specified address range
contains the expected data pattern. If every address is found to be correct,
A TRUE is returned, otherwise FALSE.

NOTES:
..............................................................................
*/

LOCAL UINT verify_range (
    TGTADDR start,  /* Start address (hi or lo) */
    TGTADDR end,        /* End address (inclusive)  */
    ULONG   space,      /* Address space            */
    ULONG   pattern,    /* test pattern expected    */
    BOOL    forward )   /* TRUE to increment addr   */
{
    TGTADDR     address;
    ULONG       data;
    S_ACCESS    fac, sac;
    UINT        status;

    status = TEST_PASSED;

    if (forward)
        end = TGTADDR_PLUS_ULONG(end, sizeof(data));
    else
        end = TGTADDR_MINUS_ULONG(end, sizeof(data));

    setup_address( &fac, start, space );
    setup_address( &sac, start, space );

    while ( TGTADDR_NE_TGTADDR(fac.next, end) )
    {
        if (received_break)
        {
            received_break = FALSE;
            status = TEST_ABORTED;
            break;
        }

        data = FETCH_MEM( &fac, sizeof(data) );

        if (data != pattern)
        {
            if (memtest_failed( MT_ERR_RV, fac.next, pattern, data ))
            {
                status = TEST_ABORTED;
                break;
            }
            else
                status = TEST_FAILED;
        }

        store_next( &sac, forward, sizeof(pattern), ~pattern );
        address = fac.next;
        fac.last_wrd_addr = TGTADDR_PLUS_ULONG(fac.next, sizeof(data));
        data    = fetch_next( &fac, forward, sizeof(data) );

        if (data != ~pattern)
        {
            if (memtest_failed( MT_ERR_RV, address, ~pattern, data ))
            {
                status = TEST_ABORTED;
                break;
            }
            else
                status = TEST_FAILED;
        }
    }

    return (status);
}

/*
..............................................................................
FUNCTION NAME:  verify_rot_address
DESCRIPTION:

    This function verifies that every address in the specified address range
contains its own address, rotated left by the given amount. The ones compliment
of the rotated address is expected if the mask parameter is all F's. If every
address is found to be correct, a TRUE is returned, otherwise FALSE.

NOTES:
..............................................................................
*/

LOCAL UINT verify_rot_address (
    TGTADDR start,      /* Start address (hi or lo) */
    TGTADDR end,        /* End address (inclusive)  */
    ULONG   space,      /* Address space            */
    UINT    rotate,     /* Number of bits to rotate */
    ULONG   mask,       /* F's to comp, else 0's    */
    BOOL    forward )   /* TRUE to increment addr   */
{
    TGTADDR     address;
    ULONG       data;
    S_ACCESS    fac;
    ULONG       pattern;
    UINT        status;

    status = TEST_PASSED;

    if (forward)
        end = TGTADDR_PLUS_ULONG(end, sizeof(data));
    else
        end = TGTADDR_MINUS_ULONG(end, sizeof(data));

    setup_address( &fac, start, space );

    while ( TGTADDR_NE_TGTADDR(fac.next, end) )
    {
        if (received_break)
        {
            received_break = FALSE;
            status = TEST_ABORTED;
            break;
        }

        address = fac.next;

        pattern = ROT_ADDR( address, rotate ) ^ mask;
        data    = fetch_next( &fac, forward, sizeof(data) );

        if (data != pattern)
        {
            if (memtest_failed( MT_ERR_VRA, address, pattern, data ))
            {
                status = TEST_ABORTED;
                break;
            }
            else
                status = TEST_FAILED;
        }
    }

    return (status);
}

/*
..............................................................................
FUNCTION NAME:  walk_a_bit
DESCRIPTION:

    This function performs a walking bit test on the specified address range.
First it fills the entire range with the oposing bit, then the bit is walked
through all 32 bits of each address. If these tests pass, a TRUE is returned,
otherwise a FALSE.

NOTES:
..............................................................................
*/

LOCAL UINT walk_a_bit (
    TGTADDR start,      /* Start address (lowest)   */
    TGTADDR end,        /* End address (highest)    */
    ULONG   space,      /* Address space to test    */
    ULONG   pattern )   /* Opposing bit pattern     */
{
    TGTADDR     address;
    ULONG       data, walker;
    S_ACCESS    fac, sac;
    UINT        status;

    status = TEST_PASSED;

    setup_address( &fac, start, space );
    setup_address( &sac, start, space );

    while (   (status != TEST_ABORTED)
           && (TGTADDR_LE_TGTADDR(fac.next, end)) )
    {
        if (received_break)
        {
            received_break = FALSE;
            status = TEST_ABORTED;
            break;
        }

        /* Check prefilled pattern */

        data = FETCH_MEM( &fac, sizeof(data) );

        if (data == pattern)
        {
            for (walker = 1; walker != 0; walker <<= 1) /* Walk thru 32 bits */
            {
                STORE_MEM( &sac, sizeof(pattern), pattern ^ walker );

                /* K.G. 1-27-98: We need to write out the partial word to    */
                /* memory before reading from memory */
                if (sac.write_pending)
                    write_pwd_pending(&sac);

                fac.last_wrd_addr = TGTADDR_PLUS_ULONG(fac.next, sizeof(data));
                data = FETCH_MEM( &fac, sizeof(data) );

                if (data != (pattern ^ walker) )
                {
                    if (memtest_failed( MT_ERR_VWB,
                            fac.next, pattern ^ walker, data ) )
                    {
                        status = TEST_ABORTED;
                        break;
                    }
                    else
                        status = TEST_FAILED;
                }
            }

            if (status != TEST_ABORTED)                 /* Store compliment */
            {
                store_next( &sac, TRUE, sizeof(pattern), ~pattern );
                fac.next = TGTADDR_PLUS_ULONG(fac.next, sizeof(pattern));
            }
        }
        else
        {
            if (memtest_failed( MT_ERR_VWB, fac.next, pattern, data ))
            {
                status = TEST_ABORTED;
                break;
            }

            status = TEST_FAILED;

            fac.next = TGTADDR_PLUS_ULONG(fac.next, sizeof(pattern));
            sac.next = TGTADDR_PLUS_ULONG(sac.next, sizeof(pattern));
        }
    }

    if (status != TEST_ABORTED)
    {
        setup_address( &fac, start, space );

        while ( TGTADDR_LE_TGTADDR(fac.next, end) )
        {
            if (received_break)
            {
                received_break = FALSE;
                status = TEST_ABORTED;
                break;
            }

            /* Check compliment */

            address = fac.next;
            data    = fetch_next( &fac, TRUE, sizeof(data) );

            if (data != ~pattern)
            {
                if (memtest_failed( MT_ERR_VAB, address, ~pattern, data ))
                {
                    status = TEST_ABORTED;
                    break;
                }
                else
                    status = TEST_FAILED;
            }
        }
    }

    return (status);
}

/*
..............................................................................
FUNCTION NAME:  write_rot_address
DESCRIPTION:

    This function fills every address in the specified address range with its
own address, rotated left by the given amount. The ones compliment of the
rotated address is stored if the mask parameter is all F's.

NOTES:
..............................................................................
*/

LOCAL UINT write_rot_address (
    TGTADDR start,      /* Start address (hi or lo) */
    TGTADDR end,        /* End address (hi or lo)   */
    ULONG   space,      /* Address space            */
    UINT    rotate,     /* test pattern             */
    ULONG   mask,       /* F's to comp, else 0's    */
    BOOL    forward )   /* TRUE to increment addr   */
{
    S_ACCESS    sac;
    UINT        status;

    status = TEST_PASSED;

    /* Adjust end, since it is inclusive */
    if (forward)
        end = TGTADDR_PLUS_ULONG(end, sizeof(ULONG));
    else
        end = TGTADDR_MINUS_ULONG(end, sizeof(ULONG));

    setup_address( &sac, start, space );

    while ( TGTADDR_NE_TGTADDR(sac.next, end) )
    {
        if (received_break)
        {
            received_break = FALSE;
            status = TEST_ABORTED;
            break;
        }

        store_next( &sac, forward, sizeof(LOWER_ADDR(sac.next)),
                            (ROT_ADDR(sac.next, rotate) ^ mask) );
    }

    return (status);
}

#endif
