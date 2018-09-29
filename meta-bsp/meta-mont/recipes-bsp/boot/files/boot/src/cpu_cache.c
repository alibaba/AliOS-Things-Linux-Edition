/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
#include "include/cache.h"

void sf_flush_cache_all()
{
    HAL_DCACHE_WB_INVALIDATE_ALL();
}
