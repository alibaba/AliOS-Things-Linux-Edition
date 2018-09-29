/*!
*   \file mem_test.h
*   \brief Simple Memory Test API
*   \author
*/
/*
+----------------------------------------------------------------------------+
|         (c) Copyright  Embedded Performance Incorporated,  1991-94         |
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

MODULE NAME:  mem_test.h
DESCRIPTION:

    This module contains function prototypes for the global memory test
functions which are called by the common modules.

NOTES:
..............................................................................
*/

#ifndef MEM_TEST_INCLUDED
#define MEM_TEST_INCLUDED

#define TEST_PASSED     0
#define TEST_FAILED     1
#define TEST_ABORTED    2

extern BOOL memtest_failed(SINT2 error, TGTADDR address,
                           ULONG wrote, ULONG read);
extern VOIDFN memtest_main(TGTADDR start, TGTADDR end, ULONG space,
                           USHORT test, USHORT mode,
                           USHORT repeat, ULONG del_pat);
extern VOIDFN send_mt_end_pass(void);
extern VOIDFN send_mt_end_test(ULONG pass_count);
extern VOIDFN send_mt_error(SINT2 status, ULONG pass_count,
                            TGTADDR address, ULONG wrote, ULONG read);
extern VOIDFN send_mt_status(USHORT type, ULONG val, BOOL send_pat);

#endif
