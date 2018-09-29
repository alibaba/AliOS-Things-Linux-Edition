/*!
*   \file ntv_mt.c
*   \brief Interface for Memory Test
*   \author
*/
/*
+----------------------------------------------------------------------------+
|         (c) Copyright  Embedded Performance Incorporated,  1993            |
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



MODULE NAME:    NTV_MT.C
DESCRIPTION:

    This module implements a simple user interface (via HIF calls) to drive
the turbo memory test code.  Since it is compiled and linked to run natively
on the target processor, it is executed at full target speed.  All of the
standard MT tests are supported.

NOTES:  All memory tests are performed word wide, and therefore the start and
        end addresses must be word aligned.

SPECIAL COMPILE/LINK INSTRUCTIONS:
..............................................................................
*/


#ifdef CONFIG_CMD_MT2
#include "ntv_mt.h"
#include "cmd.h"
#include <common.h>
#include <lib.h>

ULONG   get_dec_word    ( TEXT *querry, ULONG min, ULONG max, ULONG def_val );
ULONG   get_hex_word    ( TEXT *querry, ULONG min, ULONG max, ULONG def_val );
VOIDFN  main_menu       ( VOID );
USHORT  mt_flags        ( TEXT *iptr );
VOIDFN  print_error     ( int errno );
VOIDFN  print_usage     ( BOOL menu );
VOIDFN  start_test              ( BOOL extra );
VOIDFN  send_mt_end_pass    ( void );
VOIDFN  send_mt_end_test    ( ULONG pass_count );
VOIDFN  send_mt_error       ( SINT2 status, ULONG pass_count, ULONG address,
                              ULONG wrote, ULONG read);
VOIDFN  send_mt_status      ( USHORT type, ULONG val, BOOL send_pat );


typedef struct
{
    TEXT    *menu_text;
    BOOL    hex;
    ULONG   def_data;
    TEXT    *querry;
} S_MT_TABLE;

S_MT_TABLE mt_table[] =
{
/* Test     menu text       hex     def_data    querry  */
/* ----     ---------       -----   --------    ---------   */
{/*  0 */    "Quit",         FALSE,  0x00000000, NULL},

{/*  1 */    "Patterns",     FALSE,  0x00000000, NULL},
{/*  2 */    "Walking 1&0",  FALSE,  0x00000000, NULL},
{/*  3 */    "Addr Rotate",  FALSE,  0x00000000, NULL},
{/*  4 */    "~Addr Rotate", FALSE,  0x00000000, NULL},
{/*  5 */    "Byte & Hword", FALSE,  0x00000000, NULL},

{/*  6 */    NULL,           FALSE,  0x00000000, NULL},
{/*  7 */    NULL,           FALSE,  0x00000000, NULL},

{/*  8 */    "Refresh",      FALSE,  100 /*ms*/, "delay in milliseconds"},
{/*  9 */    "Combo (1-5)",  FALSE,  0x00000000, NULL},

{/* 10 */    "Scope Read",   TRUE,   0x00000000, NULL},
{/* 11 */    "Scope Write",  TRUE,   0x55555555, "data pattern"},
{/* 12 */    "Scope Wr/Rd",  TRUE,   0x55555555, "data pattern"},
};

TEXT    msg_version[]   = "3.1, January 28, 1999";

TEXT *msg_err[] =
{
    "OK",
    "start address is required, and must be in hex",
    "end address is required, and must be in hex",
    "end < start",
    "illegal test number"
};

TEXT *msg_mt[] =
{
    "\n\n%d passes completed with %d errors\n",
    "    Pass: %ld\r",
    "\rData pattern %08lX",
    "\rRotated Address %2d   ",
    "\r~Rotated Address %2d  ",
    "\rWalking 0's          ",
    "\rWalking 1's          ",
    "\rByte Access %08lX ",
    "\rHword Access %08lX"
};

LOCAL   TEXT    in_buf[256];

USHORT  flags = 0;
USHORT  repeat  = 1;
ULONG   start_addr  = uncached_addr(CONFIG_GENERAL_EXCEPTION_VECTOR+VECTOR_JUMP_SIZE);
ULONG   end_addr;
USHORT  test = MEMTEST_ALL;
ULONG   data;
char flagstr[8]="v";

UINT    pass_cnt;
UINT    error_cnt;

GLOBAL  S_CONFIG    gen_config;

UINT    debug_cats = DBG_MAIN | DBG_ERROR;
UINT    debug_level= 3;

UINT1   received_break;

//UINT1 prevent_derr_death;
UINT1   tsi_error;
BOOL flag_derr_jmpbuf;
//jmp_buf derr_jmpbuf;

/*
..............................................................................
FUNCTION NAME:  main
DESCRIPTION:

    This is the main entry point and program loop for the native mode memory
test module.

NOTES:  there is 1 argument (the executable name) if none are entered, and one
        if one is entered.
..............................................................................
*/
extern int sdram_test_size(unsigned int *pbase);
int mem_test_cmd2( int argc, char *argv[] )
{
    BOOL    extra = FALSE;
    int     len = 0x10000;
    int     l_reapeat;

    len = sdram_test_size((void *) &start_addr);
    end_addr = start_addr + len - 1;

    printf( "\nNative mode MT, v. %s -- \n", msg_version );

    gen_config.ta_endian = ENDIAN_BIG;
    if (argc < 1)
    {
        printf( "interactive mode\n" );
        main_menu( );
    }
    else
    {
        if (1!=sscanf( argv[0], "%x", &start_addr ))
            goto help;

        start_addr &= (~3);
        if (argc > 1)
        {
            if (1!=sscanf( argv[1], "%x", &len ))
                goto help;
        }
        else
        {
            len = sdram_test_size((void *) &start_addr);
        }
        end_addr = start_addr + len -4;

        if (argc > 2) {
            flags = mt_flags( argv[2] );
            strncpy(flagstr, argv[2], 7);
            flagstr[7] = '\0';
        }
        else
            flags = mt_flags( flagstr );

        if (argc > 3)
        {
            if (1!=sscanf( argv[3], "%d", &l_reapeat )) goto help;
            repeat = l_reapeat;
        }

        start_test( extra );
    }
    if (error_cnt)
        return ERR_PARM;
    return ERR_OK;

help:
    printf("mt start len [0hvqs] [repeat]");
    return ERR_PARM;
}

cmdt cmdt_mt2 __attribute__ ((section ("cmdt")))=
  {"mt2", mem_test_cmd2, "mt2 <addr> <len> [0hvqs] [repeat]"};

/*
...............................................................................
FUNCTION NAME:  char_to_hex
DESCRIPTION:

This function converts a byte into two hexidecimal digits, and stores them in
the specified string. It returns a pointer to the string position after the
hex digits, where a null terminator has been appended.
...............................................................................
*/

TEXT *char_to_hex(UCHAR hex, TEXT *str)
{
    static TEXT hex_digit[]     = "0123456789ABCDEF";

    *str++ = hex_digit[hex >> 4];     /* Most  signifigant digit */
    *str++ = hex_digit[hex & 0x0F];   /* Least signifigant digit */
    *str   = '\0';

    return(str);
}

/*
...............................................................................
FUNCTION NAME:  display_packet
DESCRIPTION:

This function displays a received packet.  It is intended for packets which are
unexpected, and therefore, not translated for display.  The packet is shown in
both hex and ASCII format.

...............................................................................
*/

LOCAL VOIDFN display_packet( UCHAR direction,
        SINT2 len, UINT2 type, UCHAR *pkt_data )
{
    #define MAX_DIRECTION_MSG  8
    #define MAX_BYTE_PER_LINE 16
    static  TEXT    out_line[80];

    BOOL    first_line;
    UINT    col, i;
    TEXT    *next;
    TEXT    msg[MAX_DIRECTION_MSG];
    TEXT    ascii[MAX_BYTE_PER_LINE + 1];

    if (direction == 'r')
    {
        strcpy(msg, "H-T>");
    }
    else
    {
        strcpy(msg, "T-H>");
    }

    col     = 0;
    next    = out_line;

    *next++ = ' ';
    next = char_to_hex( (len >> 8), next);
    ascii[col++] = '<';
    *next++ = ' ';
    next = char_to_hex( (len & 0xFF), next);
    ascii[col++] = 'L';

    *next++ = ' ';
    next = char_to_hex( (type >> 8), next);
    ascii[col++] = 'T';
    *next++ = ' ';
    next = char_to_hex( (type & 0xFF), next);
    ascii[col++] = '>';

    for (first_line = TRUE, i=0; i < len; i++)
    {
        if (col >= MAX_BYTE_PER_LINE)
        {
            ascii[col] = '\0';

            printf( "%s%-52s%s\n", msg, out_line, ascii);

            if (first_line)
            {
                first_line = FALSE;
                for (next = msg;  *next;  next++)
                    *next = ' ';
            }

            next = out_line;
            col  = 0;
        }

        *next++ = ' ';

        if (col == MAX_BYTE_PER_LINE / 2)
            *next++ = ' ';

        next = char_to_hex(*pkt_data, next);

        if ( (*pkt_data >= ' ')  &&  (*pkt_data <= 0x7F) )
            ascii[col] = *pkt_data;
        else
            ascii[col] = '.';

        ++pkt_data;
        ++col;
    }

    if (col > 0)
    {
        ascii[col] = '\0';

        printf( "%s%-52s%s\n", msg, out_line, ascii);
    }
}

/*
..............................................................................
FUNCTION NAME:  fetch_next
DESCRIPTION:

    This function fetches the next datum from the previously setup address.
This function may be called repeatedly to read subsequent memory locations.
The data may be read as byte, half-word or word, but is always returned as a
full word with the upper bits masked off. After each fetch the address is
incremented or decremented by the size of the datum.

NOTES:  The access control structure should be initialized by setup_address
        before this function is called.
..............................................................................
*/

ULONG fetch_next(
    S_ACCESS    *pac,       /* Pointer to the access control structure */
    BOOL        forward,    /* TRUE to increment    */
    USHORT      size    )   /* 1, 2 or 4 ONLY!      */
{
    ULONG   data;

    data = FETCH_MEM( pac, size );

    if (forward)
        pac->next += size;
    else
        pac->next -= size;

    return (data);
}

/*
..............................................................................
FUNCTION NAME:  get_dec_word
DESCRIPTION:

    Gets an address value from the user.

NOTES:
..............................................................................
*/

ULONG   get_dec_word ( TEXT *querry, ULONG min, ULONG max, ULONG def_val )
{
    ULONG   number;
    BOOL    asking = TRUE;

    while (asking)
    {
        printf( "<%ld> %s in decimal? ", def_val, querry );
        gets( in_buf );
        if ( strlen(in_buf)  == 0 )
        {
            asking = FALSE;
            number = def_val;
        }
        else if ( (sscanf(in_buf, "%ld", &number))  == 1)
        {
            if ( (number < min)  ||  (number > max) )
            {
                printf( "valid range for %s is %ld to %ld\n", querry, min, max);
            }
            else
                asking = FALSE;
        }
        else
            printf( "please enter a decimal value\n" );
    }

    return( number );
}

/*
..............................................................................
FUNCTION NAME:  get_hex_word
DESCRIPTION:

    Gets an address value from the user.

NOTES:
..............................................................................
*/

ULONG   get_hex_word ( TEXT *querry, ULONG min, ULONG max, ULONG def_val )
{
    ULONG   number;
    BOOL    asking = TRUE;

    while (asking)
    {
        printf( "<%lX> %s in hex? ", def_val, querry );
        gets( in_buf );
        if ( strlen(in_buf)  == 0 )
        {
            asking = FALSE;
            number = def_val;
        }
        else if ( (sscanf(in_buf, "%lx", &number))  == 1)
        {
            if ( (number < min)  ||  (number > max) )
            {
                printf( "valid range for %s is %lX to %lX\n", querry, min, max);
            }
            else
                asking = FALSE;
        }
        else
            printf( "please enter a hexidecimal value\n" );
    }

    return( number );
}

/*
..............................................................................
FUNCTION NAME: local_data_read
DESCRIPTION:

    This function reads an object of the specified size from the next address
in the access control structure within local CP memory.

NOTES:
..............................................................................
*/

GLOBAL ULONG local_data_read( S_ACCESS *pac, UINT size )
{
    switch( size )
    {
        case 1:
            return( *( (UCHAR *) pac->next) );

        case 2:
            return( *( (USHORT *) pac->next) );

        case 4:
            return( *( (ULONG *) pac->next) );
    }

    return(0xDEADBEEF);
}

/*
..............................................................................
FUNCTION NAME: local_data_write
DESCRIPTION:

    This function writes an object of the specified size to the next address
in the access control structure into local CP memory.

NOTES:
..............................................................................
*/

GLOBAL VOIDFN local_data_write( S_ACCESS *pac, UINT size, ULONG data )
{
    BOOL size_error = FALSE;

    if (!size_error)
    {
        switch( size )
        {
            case 1:
            {
                *( (UCHAR *) pac->next) = (UCHAR) data;
                break;
            }

            case 2:
            {
                *( (USHORT *) pac->next) = (USHORT) data;
                break;
            }

            case 4:
            {
                *( (ULONG *) pac->next) = data;
            }
        }
    }
}

/*
..............................................................................
FUNCTION NAME:  main_menu
DESCRIPTION:

    This function displays the main menu, and gets the test number.  Then it
calls start_test() to get the remaining parameters, and start the test.  Upon
test completion, it will redisplay the menu and start over.

NOTES:
..............................................................................
*/

extern int sdram_size(void);
VOIDFN main_menu( VOID )
{
    UINT    menu;
    BOOL    valid = TRUE;
    ULONG   min = 0;
    ULONG   max = uncached_addr(sdram_size());

    do
    {
        if (valid)
        {
            print_usage( TRUE /* menu driven */ );

            valid = FALSE;
        }
        else
            printf( "Not a valid choice\n" );

        printf( "Selection? " );
        gets( in_buf );
        printf("\n");

        if ( sscanf(in_buf, "%d", &menu)  == 1 )
        {
            if ((menu != 0)  &&  (menu < numof(mt_table))
                &&  (mt_table[menu].menu_text != NULL) )
            {
                valid   = TRUE;
                test    = (USHORT) menu;

                start_addr = (~3) & get_hex_word( "start address",
                        min, max, start_addr );
                printf("\n");

                end_addr   = 3 | get_hex_word( "end address",
                        start_addr, max,
                        ((end_addr >= start_addr) && (end_addr <= max)) ? end_addr : max );
                printf("\n");

                if (strlen(flagstr) > 0)
                    printf( "<%s> ", flagstr );

                printf( "mode flags ('0' = clear all, 'h' = halt, 'v' = verbose, 'q' = quiet, 's' = silent)? ");

                gets( in_buf );                             /* Get the flag string */
                printf("\n");

                if (strlen(in_buf) > 0) {
                    flags  = mt_flags( in_buf );    /* convert it to MTST_xxxx flags */
                    strncpy(flagstr, in_buf, 7);
                    flagstr[7] = '\0';
                }
                else
                    flags  = mt_flags( flagstr );   /* convert it to MTST_xxxx flags */

                repeat = (USHORT) get_dec_word( "Repeat (0 = forever)", 0, 0xFFFF, repeat );

                if (mt_table[test].querry)
                {
                    if (mt_table[test].hex)
                    {
                        data = get_hex_word( mt_table[test].querry,
                                0, 0xFFFFFFFF, mt_table[test].def_data );
                    }
                    else
                    {
                        data = get_dec_word( mt_table[test].querry,
                                0, 0xFFFFFFFF, mt_table[test].def_data );
                    }

                    start_test( TRUE /* "extra" param */ );
                }
                else
                    start_test( FALSE /* no "extra" param */ );
            }
        }
    }
    while (menu != 0);
}

/*
..............................................................................
FUNCTION NAME:  mt_flags
DESCRIPTION:

    This function converts MT flag characters into the bit designations in the
MT packet.  It also creates the flags string printed as the equivalent MON
command.

NOTES:
..............................................................................
*/

USHORT mt_flags( TEXT *iptr )
{
    USHORT  flags;
    flags = 0;

    while (*iptr)
    {
        switch (toupper(*iptr++))
        {
            case '0':       /* a 0 clears all flags */
                flags = 0;
                break;
            case 'H':
                flags |= MTST_HALT_FLAG;
                break;
            case 'V':
                flags |= MTST_VERBOSE_FLAG;
                break;
            case 'Q':
                flags |= MTST_QUIET_FLAG;
                break;
            case 'S':
                flags |= MTST_SILENT_FLAG;
                break;
        }
    }
    return( flags );
}

/*
..............................................................................
FUNCTION NAME:  print_error
DESCRIPTION:

    This function prints an error message based on the errno parameter passed
in.  Then it prints the usage screen.

NOTES:
..............................................................................
*/

VOIDFN print_error( int errno )
{
    if (errno <= numof(msg_err) )
        printf( "\nERROR: %s\n", msg_err[errno] );
    else
        printf( "\nUnexpected error %d\n", errno );

    print_usage( FALSE /* not menu driven */ );
}

/*
..............................................................................
FUNCTION NAME:  print_usage
DESCRIPTION:

    This function prints the list of test numbers that MT supports.  If the
menu parameter is TRUE, then 0 (quit) is also printed, since that is the best
way to break out while awaiting HIF input.  If menu is false, then the list
is being displayed as part of the usage message, so 0 is not printed.

NOTES:
..............................................................................
*/

VOIDFN print_usage(BOOL menu)
{
    UINT    i;
    static  TEXT    msg_menu[]   = "\n\nNative Memory Test\n";
    static  TEXT    msg_choice[] = "\n\nValid test choices\n";
    static  TEXT    msg_sep[]    =     "==================\n";

    if (menu)
    {
        printf( msg_menu );
        i = 0;
    }
    else
    {
        printf( "\nUsage:\tNTV_MT    /* for interactive mode */\n" );
        printf(         "\tNTV_MT start end [1..5 | 9..10] [flagstr] [repeat]\n" );
        printf(         "\tNTV_MT start end 8 [delay] [flagstr] [repeat]\n" );
        printf(         "\tNTV_MT start end {11..12} [data] [flagstr] [repeat]\n" );
        printf( "\nWhere:\tstart, end, data    are in hex (without 0x prefix)\n" );
        printf(         "\ttest, repeat, delay are in decimal\n" );
        printf(         "\tflagstr is a string: [ H | V | Q | S ] ... or 0 to clear all\n" );
        printf( "\nNote:\tWCDB and EDB automatically provide the program name\n");
        printf(        "\tas argv[0], so there's no need to enter \"NTV_MT\".\n");
        printf( "Note:\tMON automatically provides the program name if -c is\n");
        printf(       "\tomitted, thereby selecting interactive mode.  However,\n");
        printf(       "\tif -c is used, then YOU MUST manually enter \"NTV_MT\" as\n");
        printf(       "\tthe program name for interactive or non-interactive mode.\n");

        printf( msg_choice );
        i = 1;
    }

    printf( msg_sep );

    while (i < numof(mt_table))
    {
        if (mt_table[i].menu_text != NULL)
            printf( "%3d: %s\n", i, mt_table[i].menu_text );

        ++i;
    }

    printf( "\n" );
}

/*
..............................................................................
FUNCTION NAME:  query_abort
DESCRIPTION:

    This function asks if the user wants to abort testing, and if so, returns
TRUE.

NOTES:
..............................................................................
*/

GLOBAL BOOL query_abort( VOID )
{
    UCHAR   reply[4];

    while (1==1)
    {
        printf( "<y/n>  Abort testing? " );
        gets( in_buf );

        if ( (sscanf(in_buf, "%2s", reply))  == 1)
        {
            switch (tolower(reply[0]))
            {
                case 'y':
                {
                    return( TRUE );
                }
                case 'n':
                {
                    return( FALSE );
                }
            }
        }
    }
}

/*
..............................................................................
FUNCTION NAME:  read_mem_block
DESCRIPTION:

    This function reads a block of target memory and stores the data into the
buffer provided. It is assumed that the caller will never ask for more data
than can fit in the buffer.

NOTES:  A function pointer in the memory access structure points to the low
        level function to be used.
..............................................................................
*/

ULONG read_mem_block(
    UINT    count,      /* Number of objects to read    */
    USHORT  size,       /* Object size (1,2,4 only!)    */
    TGTADDR address,    /* Target memory base address   */
    ULONG   space,      /* Target memory address space  */
    UCHAR   *buffer )   /* Pointer to storage buffer    */
{
    S_ACCESS    pac;
    ULONG       err_code = ACK_NO_ERR;

    setup_address( &pac, address, space );

    while (count-- > 0)
    {
        switch (size)
        {
            case sizeof(ULONG):
            {
                *(ULONG *) buffer = FETCH_MEM( &pac, size );
                break;
            }
            case sizeof(UCHAR):
            {
                *(UCHAR *)  buffer = FETCH_MEM( &pac, size );
                break;
            }
            case sizeof(USHORT):
            {
                *(USHORT *) buffer = FETCH_MEM( &pac, size );
                break;
            }
        }

        pac.next += size;
        buffer += size;
    }

    return(err_code);
}

/*
..............................................................................
FUNCTION NAME:  send_mt_end_pass
DESCRIPTION:

    Sends 'end pass' to terminal.

NOTES: Replace send_packet calls from trgt_pkt.c     jsc 2/09/95
..............................................................................
*/
GLOBAL VOIDFN send_mt_end_pass( void )
{
    pass_cnt++;
    if ( (repeat == 0)  ||  (pass_cnt <= (UINT) repeat) )
        printf( msg_mt[MTST_END_PASS], pass_cnt );

    fflush( stdout );
}

/*
..............................................................................
FUNCTION NAME:  send_mt_end_test
DESCRIPTION:

    Sends 'end of test' to terminal.

NOTES: Replace send_packet calls from trgt_pkt.c     jsc 2/09/95
..............................................................................
*/
GLOBAL VOIDFN send_mt_end_test(
    ULONG   pass_count      /* used in trgt_pkt.c but not here. jsc 2/9/95 */
    )
{
    if ( flags & (MTST_QUIET_FLAG | MTST_SILENT_FLAG) )
        printf( "\n\nTest completed\n" );
    else
        printf ( msg_mt[MTST_END_TEST], pass_cnt, error_cnt );

    fflush( stdout );
}

/*
..............................................................................
FUNCTION NAME:  send_mt_error
DESCRIPTION:

NOTES: Replace send_packet calls from trgt_pkt.c     jsc 2/09/95
..............................................................................
*/

VOIDFN send_mt_error(
    SINT2   status,
    ULONG   pass_count,
    ULONG   address,
    ULONG   wrote,
    ULONG   read    )
{
    ++error_cnt;        /* output already done in mem_test.c */

    fflush( stdout );
}


/*
..............................................................................
FUNCTION NAME:  send_mt_status
DESCRIPTION:

    This function sends a status message to the terminal. It should be called at
the start of each test if the user has requested verbose operation.

NOTES: Replace send_packet calls from trgt_pkt.c     jsc 2/09/95
..............................................................................
*/

VOIDFN send_mt_status(
    USHORT  type,       /* Type of test about to be performed       */
    ULONG   val,        /* Data value of test (except walking bit)  */
    BOOL    send_pat )  /* TRUE to send pattern field */
{
    SINT2   stat;
    ULONG   pattern;

    stat    = type;
    pattern = val;


    switch ( stat )
    {
        case MTST_TESTID_1: ;
        case MTST_TESTID_2: ;
        case MTST_TESTID_3: ;
        case MTST_TESTID_6: ;
        case MTST_TESTID_7:
        {
            printf( msg_mt[stat], pattern );
            break;
        }

        case MTST_TESTID_4: ;
        case MTST_TESTID_5:
        {
            printf( msg_mt[stat] );
            break;
        }
        default:
        {
            printf( "\nUnexpected status 0x%X\n", stat );
            break;
        }
    }

    fflush( stdout );
}

/*
..............................................................................
FUNCTION NAME:  setup_address
DESCRIPTION:

    This function initializes an access control structure with all of the
information necessary to access the indicated address space.

NOTES:  The caller of setup_address may derefernce the read and write function
        pointers with the FETCH_MEM and STORE_MEM macros, or it may call
        fetch_next or store_next to read or write each object in a block.
..............................................................................
*/

GLOBAL VOIDFN setup_address(
    S_ACCESS    *pac,       /* Access control structure     */
    TGTADDR     address,    /* Starting address */
    ULONG       space   )   /* Address space    */
{
    pac->space  = space;
    pac->next   = address;
    pac->rd_ctrl = 0;
    pac->wr_ctrl = 0;
    pac->last_wrd_addr = ~address;      /* Make sure it does initial read */
    pac->last_wwr_addr = ~address;      /* Make sure it does initial read */
    pac->write_pending = FALSE;
    pac->cacheable = FALSE;

    if ( (debug_cats & DBG_XFER)  &&  (debug_level > 1) )
    {
        printf( "  Access control struct setup for 0x%lX in 0x%lX space\n",
                address, space );
    }
}

/*
..............................................................................
FUNCTION NAME:  start_test
DESCRIPTION:

    This function gets the parameters required to perform to requested memory
test, initializes the pass count and error count, and starts the test.

NOTES:
..............................................................................
*/

VOIDFN start_test(
    BOOL    extra   )               /* "extra" MT parameter (data or delay) */
{
    printf( "\n\nMT %lX %lX, %d", start_addr, end_addr, test );

    if (extra)
    {
        if (mt_table[test].hex)
        {
            printf( ", %lX", data );
        }
        else
        {
            printf( ", %ld", data );
        }
    }

    printf( ", %s, %d\n", flagstr, repeat );

    pass_cnt  = 0;
    error_cnt = 0;

    memtest_main( start_addr, end_addr & (~3), MS_FWCMN_XX, test, flags,
            repeat, data );

    printf( "\n\n" );
}

/*
..............................................................................
FUNCTION NAME:  store_next
DESCRIPTION:

    This function stores the next datum to the previously setup address. This
function may be called repeatedly to write subsequent memory locations. The
data may be written as byte, half-word or word.  After each store the address
is incremented or decremented by the size of the data.

NOTES:  The access control structure should be initialized by setup_address
        before this function is called.
..............................................................................
*/

GLOBAL VOIDFN store_next(
    S_ACCESS    *pac,       /* Access control structure     */
    BOOL        forward,    /* TRUE to increment address    */
    USHORT      size,       /* Datum size (1, 2 or 4 ONLY!) */
    ULONG       data    )
{
    STORE_MEM( pac, size, data );

    if (forward)
        pac->next += size;
    else
        pac->next -= size;
}

/*
..............................................................................
FUNCTION NAME:  write_mem_block
DESCRIPTION:

    This function writes a block of data from the given buffer into target
memory.  After all the objects have been written, the write_pending flag is
checked to see if a partial word was inserted into the cached value, but not
yet written to memory.

NOTES:
..............................................................................
*/

GLOBAL ULONG write_mem_block(
    UINT    count,      /* Number of objects to write   */
    USHORT  size,       /* Object size (1,2,4 only!)    */
    TGTADDR address,    /* Target memory base address   */
    ULONG   space,      /* Target memory address space  */
    UCHAR   *buffer )   /* Pointer to source of data    */
{
    S_ACCESS pac;
    ULONG    err_code = ACK_NO_ERR;

    setup_address( &pac, address, space );

    while (count-- > 0)
    {
        switch (size)
        {
            case sizeof(ULONG):
            {
                STORE_MEM( &pac, size, (*(ULONG *) buffer) );
                break;
            }
            case sizeof(UCHAR):
            {
                STORE_MEM( &pac, size, (*(UCHAR *) buffer) );
                break;
            }
            case sizeof(USHORT):
            {
                STORE_MEM( &pac, size, (*(USHORT *) buffer) );
                break;
            }
        }

        pac.next += size;
        buffer  += size;
    }

    if (pac.write_pending)      /* Partial words not yet written */
    {
        pac.next = pac.last_wwr_addr;
        STORE_MEM( &pac, sizeof(ULONG), pac.partial_wr.word );

        if ( (debug_cats & DBG_XFER)  &&  (debug_level > 3) )
            printf( "\tPending partial word write completed\n" );
    }

    return (err_code);
}

/*
..............................................................................
FUNCTION NAME:  send_packet
DESCRIPTION:

    This function is called by the mem_test module to send a packet to the
host.  Since this program is comiled to run natively, the packet is decoded
and displayed for the user.  If the packet type is not expected, it is dumped
in hex and ASCII.
..............................................................................
*/

BOOL send_packet( UINT length, UINT type, VOIDPTR packet )
{
    struct mtst_stat *stat;

    static TEXT *msg_mt[] =
    {
        "\n\n%d passes completed with %d errors\n",
        "    Pass: %ld\r",
        "\rData pattern %08lX",
        "\rRotated Address %2d   ",
        "\r~Rotated Address %2d  ",
        "\rWalking 0's          ",
        "\rWalking 1's          ",
        "\rByte Access %08lX ",
        "\rHword Access %08lX"
    };

    if (type == REM_MTST_STATUS)
    {
        stat = (struct mtst_stat *) packet;

        if (stat->status >= 0)
        {
            switch (stat->status)
            {
                case MTST_END_TEST:
                {
                    if ( flags & (MTST_QUIET_FLAG | MTST_SILENT_FLAG) )
                        printf( "\n\nTest completed\n" );
                    else
                        printf ( msg_mt[MTST_END_TEST], pass_cnt, error_cnt );

                    break;
                }

                case MTST_END_PASS:
                {
                    ++pass_cnt;

                    if ( (repeat == 0)  ||  (pass_cnt <= (UINT) repeat) )
                        printf( msg_mt[MTST_END_PASS], pass_cnt );

                    break;
                }

                case MTST_TESTID_1: ;
                case MTST_TESTID_2: ;
                case MTST_TESTID_3: ;
                case MTST_TESTID_6: ;
                case MTST_TESTID_7:
                {
                    printf( msg_mt[stat->status], stat->data.pattern );
                    break;
                }

                case MTST_TESTID_4: ;
                case MTST_TESTID_5:
                {
                    printf( msg_mt[stat->status] );
                    break;
                }
                default:
                {
                    printf( "\nUnexpected status 0x%X\n", stat->status );
                    break;
                }
            }
        }
        else
            ++error_cnt;
    }
    else
    {
        /* Was not a normal mtst_status message, just dump it */
        printf( "\n" );
        display_packet( 'r', length, type, packet );
    }

    fflush( stdout );

    return( 0 );
}

VOIDFN  reg_read_loop           (VOID)
{
}
VOIDFN  reg_write_loop          (ULONG pattern)
{
}
VOIDFN  reg_wr_rd_loop          (ULONG pattern)
{
}
VOIDFN  write_pwd_pending       (S_ACCESS   *sac)
{
}

#endif
