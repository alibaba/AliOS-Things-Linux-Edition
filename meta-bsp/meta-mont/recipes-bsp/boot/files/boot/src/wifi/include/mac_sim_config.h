#ifndef __MAC_SIM_CONFIG_H__
#define __MAC_SIM_CONFIG_H__

#define MAX_STA_NUM     32
#define MAX_DS_NUM      16

#define MAX_AP_COUNT                4              /* max entries in DS table */

#define MAX_BSSIDS                  4

#define BUFFER_HEADER_POOL_SIZE                 192
#define BEACON_Q_BUFFER_HEADER_POOL_SIZE        8 //( MAX_BSSIDS * 8 )

#define RX_DESCRIPTOR_COUNT         16
#define BEACON_TX_DESCRIPTOR_COUNT  MAX_BSSIDS

#define DEF_BUF_SIZE        2048    /* could be overwrite by global config */

#if defined(TESTCASE) && (TESTCASE == 10000906)  // change BH pool in testcase 906
#undef   BUFFER_HEADER_POOL_SIZE
#define  BUFFER_HEADER_POOL_SIZE     64
#endif

#if defined(TESTCASE) && (TESTCASE == 10000907)  // change packet to QRAM for testcase 907
#define PACKET_BUFFER_IN_QRAM
#endif

#endif // __MAC_SIM_CONFIG_H__

