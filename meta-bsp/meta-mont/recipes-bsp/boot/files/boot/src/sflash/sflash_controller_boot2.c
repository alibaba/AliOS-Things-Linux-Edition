/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/

/*!
 *   \file sflash_controller_boot2.c
 *   \brief trick for split sflash_controller.c functions with different define statement
 *   \author Montage
 */ 
    
/*=============================================================================+
| Define                                                                       |
+=============================================================================*/ 
//#define SERIAL_FLASH_TEST
#define BOOT_MODE_BOOT2
    
/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/ 
#include "sflash_controller.c"
#ifdef SERIAL_FLASH_TEST
#include "sflash_test.c"
#endif                          /*  */
    
