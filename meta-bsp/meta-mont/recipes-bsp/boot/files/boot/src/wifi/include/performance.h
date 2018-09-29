/*=============================================================================+
|                                                                              |
| Copyright 2018                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*!
*   \file performance.h
*   \brief
*   \author Montage
*/

#ifndef __PERFORMANCE_H__
#define __PERFORMANCE_H__

void panther_fem_config(int version);
void panther_fem_init(int version);
void panther_without_fem_init(void);
void panther_channel_config(int channel, int version);
void panther_throughput_config(void);

#define CONFIG_CHANNEL_14_IMPROVE
#define CONFIG_THROUGHPUT_IMPROVE

#endif // __PERFORMANCE_H__
