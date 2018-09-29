/*!
*   \file ntv_mt.h
*   \brief Memory Test API
*   \author
*/
#ifndef NTV_MT_INCLUDED
#define NTV_MT_INCLUDED

#ifdef __STDC__
#define SIGNED signed
#else
#define SIGNED
#endif
            /*
            **  The following types are strictly defined for portability
            **  purposes.  Ranges and sizes are relied on in the code,
            **  for reasons such as making sure that variables and structures
            **  match the contents of files that must be machine portable.
            */
                /*
                **  The following types must be one byte wide
                */
typedef unsigned char   UINT1;      /* Must be unsigned */
typedef SIGNED char     SINT1;      /* Must be signed */
typedef unsigned char   BYTE;       /* Unsigned, used for memory access */
typedef unsigned char   FLAGS1;     /* 1-8 flag bits, |'ed together */
                /*
                **  The following types must be two bytes wide
                */
typedef unsigned short  UINT2;      /* Must be unsigned */
typedef SIGNED short    SINT2;      /* Must be signed */
typedef unsigned short  BYTE2;      /* Unsigned, used for memory access */
typedef unsigned short  FLAGS2;     /* 1-16 flag bits, |'ed together */
                /*
                **  The following types must be four bytes wide
                */
typedef unsigned long   UINT4;      /* Must be unsigned */
typedef SIGNED long     SINT4;      /* Must be signed */
typedef unsigned long   BYTE4;      /* Unsigned, used for memory access */
typedef unsigned long   FLAGS4;     /* 1-32 flag bits, |'ed together */
                /*
                **  The following types must be eight bytes wide
                */
typedef unsigned long long  UINT8;
typedef SIGNED long long    SINT8;

                /*
                **  The following types are used for bitfields up to 16 bits
                **  wide.
                */
typedef unsigned int    UBITS;      /* Must be unsigned */
typedef SIGNED int      SBITS;      /* Must be signed */
typedef int             BITS;       /* Any sign */
                /*
                **  The following types are used for bitfields up to 32 bits
                **  wide.  WARNING - bitfields wider than an int are non-ANSI!
                **  At least using these typedefs will make non-portable code.
                **  easier to find.
                */
typedef unsigned long   ULBITS;     /* Must be unsigned */
typedef SIGNED long     SLBITS;     /* Must be signed */
typedef long            LBITS;      /* Any sign */
            /*
            **  The following variations on "void" are defined because some
            **  compilers implement a limited form of "void" that is valid
            **  only as a function return type.
            */
#define VOIDFN          void        /* Function return type */
#define VOID            void        /* Other non-pointer uses */
typedef VOID            *VOIDPTR;   /* Generic pointer-to-anything */

typedef int             BOOL;

typedef char            TEXT;       /* Text, not one of the many other uses */
typedef unsigned char   UCHAR;      /* Numeric, range 0 to 255 */
typedef SIGNED char     SCHAR;      /* Numeric, range -127 to 127 */
typedef unsigned short  USHORT;     /* Numeric, range 0 to 64K */
typedef SIGNED short    SSHORT;     /* Numeric, range -32767 to 32767 */

typedef unsigned int    UINT;       /* Numeric, range 0 to 64K */
typedef unsigned long   ULONG;      /* Numeric, range 0 to 4G */

typedef SIGNED int      SINT;       /* Numeric, range -32767 to 32767 */
typedef SIGNED long     SLONG;      /* Numeric, range -2G to 2G */
typedef unsigned int    FLAGS;      /* 1-16 flag bits, |'ed together */
typedef unsigned long   LFLAGS;     /* 1-32 flag bits, |'ed together */

#define LOCAL           static      /* For file-scope global definitions */
                                    /* GLOBAL is used for readability on */
                                    /* global definitions (as opposed to */
                                    /* declarations, which use "extern") */
#define GLOBAL
                                    /* If register declarations cause lower */
                                    /* performance by adversly affecting */
                                    /* optimization, set FAST to nothing */

#define CONST           const
#define VOLATILE        volatile

/*
            **  For ANSI compilers, the best NULL pointer is actually
            **  the simple constant 0.  Thanks to prototypes, 0 is equally
            **  valid for a function pointer.
            */
#ifndef NULL
#define NULL            0
#endif
            /*
            **  These are what BOOL's are assigned and compared against
            */
#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif
            /*
            **  Useful macros for getting the element count and end
            **  bound address of arrays.
            */
#define numof( a )  (sizeof(a)/sizeof((a)[0]))
#define endof( a )  (&(a)[ numof(a) ])

#ifndef min
#define min( a, b ) ((a) < (b) ? (a) : (b))
#endif
#ifndef max
#define max( a, b ) ((a) > (b) ? (a) : (b))
#endif


    /*  Target endian code  */

#define ENDIAN_BIG          1
#define ENDIAN_LITTLE       2
#define ENDIAN_UNKNOWN      3
#define ENDIAN_EITHER       4




/*
**  Flag bits in the memory test mode field.
*/
#define MTST_HALT_FLAG          0x100
#define MTST_VERBOSE_FLAG       0x200
#define MTST_QUIET_FLAG         0x400
#define MTST_SILENT_FLAG        0x800




/*
**  Memtest modes in command packet
*/
#define MEMTEST_MAP              0
#define MEMTEST_BASIC            1
#define MEMTEST_WALKING          2
#define MEMTEST_ADR_ROT          3
#define MEMTEST_ADR_CMP          4
#define MEMTEST_BYTES            5
#define MEMTEST_REFRESH          8
#define MEMTEST_ALL              9
#define MEMTEST_MEM_READ        10
#define MEMTEST_MEM_WRITE       11
#define MEMTEST_MEM_WR_RD       12
#define MEMTEST_REG_READ        13
#define MEMTEST_REG_WRITE       14
#define MEMTEST_REG_WR_RD       15


#define ALIGN_PTR( p, s )   ( ( (int)p & ( (s) - 1 )) ? \
        p = (VOIDPTR) ((BYTE *)p + ((s) - ( (int)p & ( (s) - 1 )))) : 0 )


/*
**  Diagnostic port control flags (used in the MON P command).
**  MS 16 bits are reserved for target specific diagnostic messages.
*/

#define DBG_ERROR           0x0001
#define DBG_EXEC            0x0002
#define DBG_NET             0x0004
#define DBG_COMM            0x0008
#define DBG_TC              0x0010
#define DBG_TD              0x0020
#define DBG_XFER            0x0040
#define DBG_MAIN            0x0080
#define DBG_SELF_TEST       0x0100
#define DBG_RECV_PKT        0x0200
#define DBG_XMIT_PKT        0x0400
#define DBG_OVM             0x0800
#define DBG_PRT_NET_STATUS  0x2000
#define DBG_TSTAMP          0x4000

#define DBG_FLUSH_BUF       0x8000      /* Flush buffer after all printf's */


#define ACK_WARNING             0x80000000  /* warning message flag */
#define ACK_INTERNAL            0x40000000  /* internal errors */
#define ACK_REPORTED_ERR        0x20000000  /* Target used REM_NOTIFY to explain the problem */
#define ACK_NO_ERR              0

#ifdef ADDR64
typedef UINT8 TGTADDR;
#else
typedef ULONG TGTADDR;
#endif


/* Memory access control structure  */

typedef struct ST_ACCESS
{
    TGTADDR     next;           /* Next address to read/write               */
    ULONG       space;          /* Address space being accessed             */
    BOOL        cacheable;      /* Is physical address cacheable            */
    ULONG       rd_ctrl;        /* Access method for read                   */
    ULONG       wr_ctrl;        /* Access method for write                  */
    union
    {
        ULONG   word;
        USHORT  hword[4];
        UCHAR   byte[8];
    } partial_rd;               /* Word containing last partial word read   */
    TGTADDR     last_wrd_addr;  /* Word address of last partial word read   */
    union
    {
        ULONG   word;
        USHORT  hword[4];
        UCHAR   byte[8];
    } partial_wr;               /* Word containing last partial word write  */
    TGTADDR     last_wwr_addr;  /* Word address of last partial word write  */
    BOOL        write_pending;  /* Word partially filled, but not written   */
} S_ACCESS;


#define FETCH_MEM(PAC,S)        local_data_read((PAC),(S))
#define STORE_MEM(PAC,S,D)      local_data_write((PAC),(S),(D))




typedef struct ST_CONFIG_A32
{
    UINT1   ta_endian;      /* target arch endian   */
} S_CONFIG;

/*
**  The testID and error status responses would have had to be changed for
**  64-bit (address OR data) targets.  Instead, such targets will use
**  REM_NOTIFY (category NOTIFY_CAT_ICE_INFO) packets to display status and
**  errors to the user.  The host will receive error status only in
**  Halt-on-error mode, and only to prompt the user whether to continue or not.
**  It will not receive testID status, or end-of-pass status, and will not
**  print anything on end-of-test.
*/
typedef struct mtst_stat
{                           /* REM_MTST_STATUS */
    SINT2   status;             /* -N = error #N, 0 = end-of-test, */
                                /* 1 = end-of-pass, 2-6 = testid + 1 */
    UINT2   filler;             /* Need to word align the union */
    union
    {
        UINT4   pattern;        /* Current data pattern (testids) */
        UINT4   rot;            /* Current rotation (testids) */
        UINT4   passes;         /* Total passes done (end-of-test) */
        struct
        {                       /* Error information */
            UINT4   pass;
            UINT4   addr;
            UINT4   wrote;
            UINT4   read;
        } err;
    } data;
} S_MTST_STAT;



/*
**  Status values for the memory test status response packets
*/
#define MTST_END_TEST           0
#define MTST_END_PASS           1
#define MTST_TESTID_1           2
#define MTST_TESTID_2           3
#define MTST_TESTID_3           4
#define MTST_TESTID_4           5   /* Walking zeros */
#define MTST_TESTID_5           6   /* Walking ones */
#define MTST_TESTID_6           7   /* Byte access */
#define MTST_TESTID_7           8   /* Hword access */

/*
**  Memtest error codes
*/
#define MT_ERR_TARGET   0   /* TSI/target failure   */
#define MT_ERR_WRV      1   /* Write, read, verify  */
#define MT_ERR_RV       2   /* Re-read, verify      */
#define MT_ERR_CWRV     3   /* Compliment, write read verify */
#define MT_ERR_VIB      4   /* Verify inverse bit   */
#define MT_ERR_VWB      5   /* Verify walking bit   */
#define MT_ERR_VAB      6   /* Verify all bits      */
#define MT_ERR_VRA      7   /* Verify rotated addr  */

#define ERR_OVLY_MAP        -100    /* MT-0 response for map RAM error */
#define ERR_OVLY_MEM        -200    /* MT-0 response for overlay memory error */

/*
**  Test ID used in failure messages
*/
#define MT_ID_PAT_INC       1   /* Patterns, inc addr   */
#define MT_ID_PAT_DEC       2   /* Patterns, dec addr   */
#define MT_ID_WALK_1        3   /* Walking ones         */
#define MT_ID_WALK_0        4   /* Walking zeros        */
#define MT_ID_ROT_ADR       5   /* Rotated addr, 5=inc, 6=dec       */
#define MT_ID_CMP_ADR       7   /* Compliment addr, 7=inc, 8=dec    */
#define MT_ID_REFRESH       9   /* Refresh test         */
#define MT_ID_SCOPE_RD      10  /* Scope loop - Read    */
#define MT_ID_SCOPE_WR      11  /* Scope loop - Write   */
#define MT_ID_SCOPE_WRRD    12  /* Scope loop - Wr/Rd   */


#define REM_MTST_STATUS         37

#define MS_FWCMN_XX         0xFF    /* Firmware space, ices only */


#ifndef ADDR64

/* assume 32 bit math only */
#define BYTE_ALIGN(addr)            ((addr) & 0x01)
#define HWORD_ALIGN(addr)           ((addr) & 0x02)
#define TGTADDR_FROM_2ULONGS(A,B)   (B)

#define TGTADDR_AND_TGTADDR(A,B)    ((A)  & (B))
#define TGTADDR_AND_ULONG(A,B)      ((A)  & (B))
#define TGTADDR_OR_TGTADDR(A,B)     ((A)  | (B))
#define TGTADDR_OR_ULONG(A,B)       ((A)  | (B))
#define TGTADDR_MINUS_TGTADDR(A,B)  ((A)  - (B))
#define TGTADDR_MINUS_ULONG(A,B)    ((A)  - (B))
#define TGTADDR_NOT(A)              (~(A))
#define TGTADDR_NOT_ULONG(A)        (~(A))
#define TGTADDR_PLUS_TGTADDR(A,B)   ((A)  + (B))
#define TGTADDR_PLUS_ULONG(A,B)     ((A)  + (B))
#define TGTADDR_SHIFT_LEFT(A,x)     ((A) << (x))
#define TGTADDR_SHIFT_RIGHT(A,x)    ((A) >> (x))
#define TGTADDR_XOR_TGTADDR(A,B)    ((A)  ^ (B))

#define TGTADDR_EQ_TGTADDR(A,B)     ((A) == (B))
#define TGTADDR_EQ_ULONG(A,B)       ((A) == (B))
#define TGTADDR_NE_TGTADDR(A,B)     ((A) != (B))
#define TGTADDR_NE_ULONG(A,B)       ((A) != (B))
#define TGTADDR_GT_TGTADDR(A,B)     ((A)  > (B))
#define TGTADDR_GT_ULONG(A,B)       ((A)  > (B))
#define TGTADDR_GE_TGTADDR(A,B)     ((A) >= (B))
#define TGTADDR_GE_ULONG(A,B)       ((A) >= (B))
#define TGTADDR_LT_TGTADDR(A,B)     ((A)  < (B))
#define TGTADDR_LT_ULONG(A,B)       ((A)  < (B))
#define TGTADDR_LE_TGTADDR(A,B)     ((A) <= (B))
#define TGTADDR_LE_ULONG(A,B)       ((A) <= (B))
#define TGTADDR_IS_ZERO(A)          ((A) == 0)

#define LO_32(VAL)                  (VAL)
#define HI_32(VAL)                  (0)

#define DW_MAX(A,B)                 ( ((A) > (B)) ? (A) : (B) )
#define DW_MIN(A,B)                 ( ((A) < (B)) ? (A) : (B) )

#else  /* ADDR64  */
/* assume 64 bit math available */

#define BYTE_ALIGN(addr)            ((addr) & 0x01)
#define HWORD_ALIGN(addr)           ((addr) & 0x02)

#define TGTADDR_FROM_ULONG(UL)      ( (TGTADDR) (UL) )
#define TGTADDR_FROM_SLONG(SL)      ( (TGTADDR) ((long long) ((long int) (SL))) )
#define TGTADDR_FROM_2ULONGS(A,B)   ( (((TGTADDR) (A)) << 32) | ((TGTADDR) (B)) )

#define TGTADDR_AND_TGTADDR(A,B)    ((A)  & (B))
#define TGTADDR_AND_ULONG(A,B)      ((A)  & (B))
#define TGTADDR_OR_TGTADDR(A,B)     ((A)  | (B))
#define TGTADDR_OR_ULONG(A,B)       ((A)  | (B))
#define TGTADDR_MINUS_TGTADDR(A,B)  ((A)  - (B))
#define TGTADDR_MINUS_ULONG(A,B)    TGTADDR_MINUS_TGTADDR((A), TGTADDR_FROM_ULONG(B))
#define TGTADDR_NOT(A)              (~(A))
#define TGTADDR_NOT_ULONG(A)        (~(A))
#define TGTADDR_PLUS_TGTADDR(A,B)   ((A)  + (B))
#define TGTADDR_PLUS_ULONG(A,B)     ((A)  + (B))
#define TGTADDR_SHIFT_LEFT(A,x)     ((A) << (x))
#define TGTADDR_SHIFT_RIGHT(A,x)    ((A) >> (x))
#define TGTADDR_XOR_TGTADDR(A,B)    ((A)  ^ (B))

#define TGTADDR_EQ_TGTADDR(A,B)     ((A) == (B))
#define TGTADDR_EQ_ULONG(A,B)       ((A) == (B))
#define TGTADDR_NE_TGTADDR(A,B)     ((A) != (B))
#define TGTADDR_NE_ULONG(A,B)       ((A) != (B))
#define TGTADDR_GT_TGTADDR(A,B)     ((A)  > (B))
#define TGTADDR_GT_ULONG(A,B)       ((A)  > (B))
#define TGTADDR_GE_TGTADDR(A,B)     ((A) >= (B))
#define TGTADDR_GE_ULONG(A,B)       ((A) >= (B))
#define TGTADDR_LT_TGTADDR(A,B)     ((A)  < (B))
#define TGTADDR_LT_ULONG(A,B)       ((A)  < (B))
#define TGTADDR_LE_TGTADDR(A,B)     ((A) <= (B))
#define TGTADDR_LE_ULONG(A,B)       ((A) <= (B))
#define TGTADDR_IS_ZERO(A)          ((A) == 0)

#define LO_32(VAL)                  ( (ULONG) (VAL))
#define HI_32(VAL)                  ( (ULONG) ((VAL) >> 32))

#endif

/************************ Some extensions for readability ********************/
#define LOWER_ADDR(ADDR)            (LO_32(ADDR))
#define UPPER_ADDR(ADDR)            (HI_32(ADDR))
#define LOWER_DATA(DATA)            (LO_32(DATA))
#define UPPER_DATA(DATA)            (HI_32(DATA))
#define LOWER_REG(REG)              (LO_32(REG))
#define UPPER_REG(REG)              (HI_32(REG))
#define LOWER_CPUREG(REG)           (LO_32(REG))
#define UPPER_CPUREG(REG)           (HI_32(REG))
/************************* end 64/32 bit register access macros **************/

extern BOOL     query_abort     ( VOID );

extern ULONG    local_data_read (S_ACCESS *pac, UINT size );
extern VOIDFN   local_data_write(S_ACCESS *pac, UINT size, ULONG data );
extern ULONG    fetch_next      (S_ACCESS *pac, BOOL forward, USHORT size);
extern ULONG    fetch_word      (TGTADDR address, ULONG space);
extern VOIDFN   next_addr       (S_ACCESS *pac, BOOL forward, USHORT size);
extern ULONG    read_mem_block  (UINT count, USHORT size, TGTADDR address,
                                    ULONG space, UCHAR *buffer);
extern ULONG    read_ta_xspace  (UINT count, USHORT size, ULONG address,
                                    ULONG space, UCHAR *buffer);
extern VOIDFN   setup_address   (S_ACCESS *pac, TGTADDR address, ULONG space );
extern VOIDFN   store_next      (S_ACCESS *pac, BOOL forward, USHORT size,
                                    ULONG data);
extern VOIDFN   store_word      (TGTADDR address, ULONG space, ULONG data);
extern ULONG    write_mem_block (UINT count, USHORT size, TGTADDR address,
                                    ULONG space, UCHAR *buffer);
extern void     write_pwd_pending(S_ACCESS *pac);
extern ULONG    write_ta_xspace (USHORT count, ULONG address, BYTE *data);

extern BOOL     fill_data_ta    ( S_ACCESS *mac, BOOL forward, USHORT size,
                                    UINT count, ULONG *pattern, UINT p_count);

/* from mem_test.c */
extern VOIDFN memtest_main (
    TGTADDR start,      /* Start address (lowest)   */
    TGTADDR end,        /* End address (highest)    */
    ULONG   space,      /* Address space to test    */
    USHORT  test,       /* Test number to perform   */
    USHORT  mode,       /* Halt, Verbose ...        */
    USHORT  repeat,     /* Iteration count          */
    ULONG   del_pat );  /* Test 8 delay, scope loop data */

#endif   /* NTV_MT_INCLUDED */

#define fflush(s)
#define exit(e) { printf("exit code=%x\n", e); }
#define toupper(c) (c&~0x20)
#define tolower(c) (c|0x20)
#define isdigit(c) ((c) >= '0' && (c) <= '9')
#define isalpha(c) (((c) >= 'A' && (c) <= 'Z') || ((c) >= 'a' && (c) <= 'z'))
#define clock() clock_get()
#define setjmp(a) 0

