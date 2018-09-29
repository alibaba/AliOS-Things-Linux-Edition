/*=============================================================================+
|                                                                              |
| Copyright 2017                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*!
*   \file mac_regs.h
*   \brief
*   \author Montage
*/

#ifndef __MAC_REGS_H__
#define __MAC_REGS_H__

#include <arch/chip.h>

#define LMAC_REG_OFFSET 0x00000400UL
#define SEC_REG_OFFSET  0x00000700UL

#define MACREG_READ32(x)  (*(volatile u32*)(UMAC_REG_BASE+(x)))
#define MACREG_WRITE32(x,val) (*(volatile u32*)(UMAC_REG_BASE+(x)) = (u32)(val))

#define MACREG_READ64(x)  (*(volatile u64*)(UMAC_REG_BASE+(x)))
#define MACREG_WRITE64(x,val) (*(volatile u64*)(UMAC_REG_BASE+(x)) = (u64)(val))

#define MACREG_UPDATE32(x,val,mask) do {           \
    u32 newval;                                        \
    newval = *(volatile u32*) (UMAC_REG_BASE+(x));     \
    newval = (( newval & ~(mask) ) | ( (val) & (mask) ));\
    *(volatile u32*)(UMAC_REG_BASE+(x)) = newval;      \
} while(0)

#define MACREG_POLL32(x,val,mask) do {           \
    while( ((*(volatile u32*) (UMAC_REG_BASE+(x))) & mask ) != val);     \
} while(0)

#define RXDSC_BADDR     0x808       /* based address of RX descriptor arrays (rx_descr) */

#define SWBL_BADDR      0x80C       /* based address of buffer header (data_frame_header) arrays (SW path pool) */
#define BSS0_STA_BITMAP 0x810
#define BSS1_STA_BITMAP 0x814
#define BSS2_STA_BITMAP 0x818
#define BSS3_STA_BITMAP 0x81C
#define SWB_HT          0x820       /* SW RX link list tail (data_frame_header link list for software Rx) */
#define STA_DS_BITMAP   0x824       /* STA/DS bitmap in address lookup engine */
   #define VLD_STABP       0xffff0000UL
   #define VLD_DSBP        0x0000ffffUL
#define STA_BITMAP2     0x868
   #define VLD_STABP2      0x0000ffffUL

#define BSSID0_INFO     0x828
#define BSSID0_INFO2    0x82c
#define BSSID1_INFO     0x830
#define BSSID1_INFO2    0x834
#define BSSID2_INFO     0x838
#define BSSID2_INFO2    0x83c
#define BSSID3_INFO     0x860
#define BSSID3_INFO2    0x864
   #define BSSID_INFO_EN   0x80000000UL
   #define BSSID_INFO_P2P  0x40000000UL
   #define BSSID_INFO_MODE 0x30000000UL
      #define BSSID_INFO_MODE_IBSS  0x00000000UL   
      #define BSSID_INFO_MODE_AP    0x10000000UL
      #define BSSID_INFO_MODE_STA   0x20000000UL
   #define BSSID_INFO_TIMER_INDEX   0x03000000UL
   #define BSSID_INFO_MAC_HI        0x0000FFFFUL
   #define BSSID_INFO2_MAC_LO       0xFFFFFFFFUL

#define BUFF_POOL_CNTL  0x840       /* buffer pool control */

#define SWBUFF_CURR_HT  0x848       /* SW buffer pool current head and tail */

#define CAPT_SIZE       0x84C       /* size of station capability table */
    #define CAPT_SIZE_64_BYTES  0
    #define CAPT_SIZE_128_BYTES  1

#define MAC_FREE_PTR        0x854   /* free buffer head/tail index */
#define MAC_FREE_CMD        0x858   /* free buffer command */
    #define MAC_FREE_CMD_BUSY   0x4
    #define MAC_FREE_CMD_SW_BUF 0x2 /* 1 for free to SW buffer, 0 for free to fastpath */
    #define MAC_FREE_CMD_KICK   0x1

#define MAX_DS_TABLE_NUMBER 0x870
#define EXT_STA_TABLE_ADDR  0x874
#define EXT_DS_TABLE_ADDR   0x878

#define STA_DS_TABLE_CFG    0x87C
    #define STA_DS_TABLE_MAX_STA_SEARCH 0x000001FFUL
    #define STA_DS_TABLE_MAX_DS_SEARCH  0x00003E00UL
    #define STA_DS_TABLE_HASH_MODE      0x00008000UL
    #define STA_DS_TABLE_HASH_MASK      0x00FF0000UL
    #define STA_DS_TABLE_INT_STA_NO     0x1F000000UL
    #define STA_DS_TABLE_STA_CLR        0x20000000UL
    #define STA_DS_TABLE_DS_CLR         0x40000000UL
    #define STA_DS_TABLE_CFG_DONE       0x80000000UL

#define LUBASIC_CAP     0x880
    #define LUBASIC_CAP_VALID           0x80000000UL

#define LUT_ADDR0       0x884
#define LUT_ADDR1       0x888
#define LUT_CMD         0x88C
#define LUR_BASIC_CAP   0x890

#define LUR_INDX_ADDR   0x894
    #define LUR_INDX_ADDR_CACHE_NO_HIT  0x20000000UL
    #define LUR_INDX_ADDR_CMD_SUCCEED   0x10000000UL
    #define LUR_INDX_ADDR_DS_HIT        0x08000000UL         
    #define LUR_INDX_ADDR_STA_HIT       0x04000000UL

#define LUR_ADDR        0x898

    // ADDR Lookup Engine Control
    #define LU_TRIGGER  0x80000000
    #define LU_DONE     0x40000000

    #define LU_CMD_SEL_STA_TABLE                0x00000010

    #define LU_CMD_MASK                         0x0000000F
    #define LU_CMD_SEARCH_BY_ADDR               0
    #define LU_CMD_READ_BY_INDEX                2
    #define LU_CMD_INSERT_INTO_STA_BY_ADDR      3
    #define LU_CMD_INSERT_INTO_DS_BY_ADDR       4
    #define LU_CMD_UPDATE_INTO_STA_BY_ADDR      5
    #define LU_CMD_UPDATE_INTO_DS_BY_ADDR       6
    #define LU_FLUSH_STA_TABLE                  7
    #define LU_FLUSH_DS_TABLE                   8
    #define LU_CMD_UPDATE_STA_BY_INDEX          9
    #define LU_CMD_UPDATE_DS_BY_INDEX           10

/* Interrupt register */
#define MAC_INT_MASK_REG    0x89c
#define MAC_INT_CLR         0x8a0
#define MAC_INT_STATUS      0x8a4

    #define MAC_INT_MASK_BITS       0x0000FFFF
    #define MAC_INT_SW_RX           0x00000040      /* SW buffer gets data */
    #define MAC_INT_RX_DESCR_FULL   0x00000080      /* RX descriptor full */
    #define MAC_INT_RX_FIFO_FULL    0x00000100      /* RX STA fifo full */
    #define MAC_INT_TSF             0x00000200      

    #define MAC_INT_ACQ_TX_DONE     0x00000400      
    #define MAC_INT_ACQ_CMD_SWITCH  0x00000800
    #define MAC_INT_ACQ_RETRY_FAIL  0x00001000

    #define MAC_INT_BEACON          0x00002000
    #define MAC_INT_SW_BUF_FULL     0x00004000      /* software buffer full */

    #define MAC_INT_FULL_MASK       0xFFFFFFFFUL

#define BB_SPI_BASE     0x8c0      
#define     BB_SPI_DONE     0x80000000UL

#define BB_SPI_CLK_DIV  0x8c4
#define BB_RF_SPI       0x8c8

#define MAC_LLC_OFFSET  0x8cc
 
#define PS_FUNC_CTRL    0x8f0
    #define PS_FUNC_DISABLE                     0x00000001UL
    #define PS_FUNC_POSTPONE_UAPSD_TRIGGER      0x00000002UL
    #define PS_FUNC_POSTPONE_PSPOLL             0x00000004UL

#define AID_BSSID_0_1   0x8f8
#define AID_BSSID_2_3   0x8fc
#define MAC_DEBUG_SEL   0x900
#define MAC_DEBUG_OUT0  0x908
#define MAC_DEBUG_OUT1  0x90c

#define MAC_RX_FILTER                 0xa00
    #define RXF_GC_MGT                                      0x00000c00
    #define RXF_GC_MGT_ALL                                  0x00000000
    #define RXF_GC_MGT_HS_HIT                               0x00000400
    #define RXF_GC_MGT_TA_HIT                               0x00000800
    #define RXF_GC_MGT_TA_HS_HIT                            0x00000c00
    #define RXF_GC_DAT                                      0x00000300
    #define RXF_GC_DAT_ALL                                  0x00000000
    #define RXF_GC_DAT_HS_HIT                               0x00000100
    #define RXF_GC_DAT_TA_HIT                               0x00000200
    #define RXF_GC_DAT_TA_HS_HIT                            0x00000300
    #define RXF_UC_MGT                                      0x000000c0
    #define RXF_UC_MGT_ALL                                  0x00000000
    #define RXF_UC_MGT_RA_HIT                               0x00000040
    #define RXF_UC_MGT_TA_HIT                               0x00000080
    #define RXF_UC_MGT_TA_RA_HIT                            0x000000c0
    #define RXF_UC_DAT                                      0x00000030
    #define RXF_UC_DAT_ALL                                  0x00000000
    #define RXF_UC_DAT_RA_HIT                               0x00000010
    #define RXF_UC_DAT_TA_HIT                               0x00000020
    #define RXF_UC_DAT_TA_RA_HIT                            0x00000030
    #define RXF_BEACON                                      0x0000000c
    #define RXF_BEACON_ALL                                  0x00000008
    #define RXF_BEACON_TA                                   0x00000004
    #define RXF_NO_BEACON                                   0x00000000
    #define RXF_PROBE_REQ                                   0x00000003
    #define RXF_PROBE_REQ_ALL                               0x00000002
    #define RXF_PROBE_REQ_TA                                0x00000001
    #define RXF_NO_PROBE_REQ                                0x00000000
#define MAC_RX_FILTER_VECTOR_L_DATA   0xa04
#define MAC_RX_FILTER_VECTOR_H_DATA   0xa08
#define MAC_RX_FILTER_VECTOR_L_MANAGE 0xa0c
#define MAC_RX_FILTER_VECTOR_H_MANAGE 0xa10

#define REVISION_REG    0x20dc

/* TX Register set */
#define UMAC_TX_CNTL    0xE00       /* upper mac transimmion control */
    #define UMAC_TX_CNTL_TXINFO_LEN     0x00000070UL
    #define UMAC_TX_CNTL_LU_CACHE_EN    0x00000002UL
    #define UMAC_TX_CNTL_TX_ENABLE      0x00000001UL
#define TXCACHE_CNTL    0xE04
    #define TXCACHE_CNTL_STACAP_CACHE_WRITEBACK     0x00000100UL
    #define TXCACHE_CNTL_STACAP_CACHE_CLEAR         0x00000200UL
#define TXFRAG_CNTL     0xE08
    #define TXFRAG_CNTL_THRESHOLD   0x0000FFFFUL
#define TXRTURNQ_CNTL   0xE0C
#define GETRECYCQ_TRIG  0xE10
#define TX_SERVICE_RATE 0xE14
#define TXQ_TRIGGER     0xE18       /* trigger transmit */
#define LSIG_RATE       0xE1C

#define PSQ_TRIGGER		0xE34
	#define RA_INDEX			0x000000FFUL
	#define TRIG_TYPE			0x00000100UL
	#define TRIG_REQUEST		0x00000200UL

#define SLOTTIME_SET    0xE40       /* slot time */
#define CW_SET          0xE44
    #define CW_SET_VO_CWMAX     0xF0000000UL
    #define CW_SET_VO_CWMIN     0x0F000000UL
    #define CW_SET_VI_CWMAX     0x00F00000UL
    #define CW_SET_VI_CWMIN     0x000F0000UL
    #define CW_SET_BE_CWMAX     0x0000F000UL
    #define CW_SET_BE_CWMIN     0x00000F00UL
    #define CW_SET_BK_CWMAX     0x000000F0UL
    #define CW_SET_BK_CWMIN     0x0000000FUL

#define CW_SET_BG       0xE48
    #define CW_SET_G_CWMAX      0x0000F000UL
    #define CW_SET_G_CWMIN      0x00000F00UL    

#define TXQ_CACHE_BADDR   0xE4C   /* external 64 x 8 bytes buffer to TX logic to cache data */

#define RW_TXFIFO_DATA      0xE68
#define RW_TXFIFO_CMD       0xE6C

#define GLOBAL_SN0_1        0xE70
#define GLOBAL_SN2_3        0xE74
#define GLOBAL_SN4_5        0xE78
#define GLOBAL_SN6_7        0xE7C
#define LOAD_SN_HB_INFO         0xE80
#define HEADER_BUFFER_RW_CMD    0xE84
#define HEADER_BUFFER_RW_DATA   0xE88

#define AC_QUEUE_COUNTER1   0xEB8
    #define AC_BE_QUEUE_COUNT   0xFFFF0000UL
    #define AC_BK_QUEUE_COUNT   0x0000FFFFUL
#define AC_QUEUE_COUNTER2   0xEBC
    #define AC_VO_QUEUE_COUNT   0xFFFF0000UL
    #define AC_VI_QUEUE_COUNT   0x0000FFFFUL

#define STACAP_CACHE_STATUS     0xEC0
    #define STACAP_CACHE_EMPTY      0x00002000UL
    #define STACAP_CACHE_NP         0x00001000UL
    #define STACAP_CACHE_TID        0x00000F00UL
    #define STACAP_CACHE_RA_INDEX   0x000000FFUL

#define FC_PUBLIC_THRESHOLD	0xECC
	#define FC_ENABLE			0x80000000UL
	#define FC_THD_RANGE		0x0000FF00UL
	#define EDCA_TXOP_LIMIT		0x000000FFUL

#define	FC_ACQ_THRESHOLD	0xED0
	#define FC_THD_AC_BK		0xFF000000UL
	#define FC_THD_AC_BE		0x00FF0000UL
	#define FC_THD_AC_VI		0x0000FF00UL
	#define FC_THD_AC_VO		0x000000FFUL

#define SEARCH_ENGINE_THD	0xED4
	#define THD_SH0				0xFF000000UL
	#define THD_SH1				0x00FF0000UL
	#define THD_SH2				0x0000FF00UL
	#define THD_SH3				0x000000FFUL

/* RX Register set */
#define BA_LIFE_TIME_REG 0xB00
#define RX_BLOCK_SIZE   0xB04
#define SUB_BLK_SIZE    0xB08
#define LNK_BLK_SIZE    0xB0C       /* max dequeue MSDU size */

#define RXQ_MSDU_MSDU_SIZE  0xB14       /* software description interrupt register set */
#define UPRX_EN         0xB18       /* upper mac rx enable */

#define ERR_EN          0xB1C       /* error drop to cpu */
    #define ERR_EN_DATA_TA_MISS_TOCPU       0x00000001UL
    #define ERR_EN_MANGMENT_TA_MISS_TOCPU   0x00000002UL
    #define ERR_EN_ICV_ERROR_TOCPU          0x00000004UL
    #define ERR_EN_FCS_ERROR_TOCPU          0x00000008UL
    #define ERR_EN_BSSID_CONS_ERROR_TOCPU   0x00000010UL
    #define ERR_EN_TID_ERROR_TOCPU          0x00000020UL
	#define ERR_EN_SEC_MISMATCH_TOCPU		0x00000040UL
	#define FASTPATH_WIFI_TO_WIFI			0x00000080UL
    #define ERR_EN_BA_SESSION_MISMATCH      0x00000200UL


#define RXDMA_CTRL      0xB20
    #define RXDMA_CTRL_ACCEPT_CACHE_OUT_REQ     0x00000002UL      /* accept RX link-info cache out request */

#define AUTO_LEARN_EN   0xB24
#define DA_MISS_TOCPU   0xB28       /* set to 0 for fowarding DA miss frames to ETH */
#define INR_TRG_PKT_NUM 0xB2C
#define INR_TRG_TICK_NUM 0xB30

#define CPU_RD_SWDES_IND 0xB34      /* let RX logic know there is free slot for RX descriptor to write */
#define MAX_WIFI_LEN    0xB38
#define MAX_ETHER_LEN   0xB3C
#define MAX_LEN_EN      0xB40
#define DEBUG_GROUP_SEL 0xB44
#define MSDU_F_TRIG     0xB48
#define SW_DES_NO       0xB50
    #define PKT_DES_NO    0x00000001UL

#define TID_SEL         0xB64
#define BSSID_CONS_CHK_EN   0xB68   /* only forward packets with matching DA&SA BSSID */
#define DEBUG_MMACRX    0xB6C

#define SA_AGE_BITMAP   0xB70       /* 256bits address lookup table bitmap indicate SA lookup hit  */
    #define SA_AGE_BITMAP_WIDTH     MAC_ADDR_LOOKUP_STA_TABLE_ENTRIES
    #define SA_AGE_BITMAP_STAS_PER_REGISTER    32

#define RX_MPDU_MAXSIZE 0xB90

#define RX_ACK_POLICY   0xB98
    #define RX_ACK_POLICY_AMPDU_BA      0x00000001UL    /* enable this to send BA after receiving A-MPDU (w/ normal ack policy) */
    #define RX_SNIFFER_MODE             0x00000002UL
    #define RX_ACK_POLICY_ALWAYS_ACK    0x00000004UL    /* enable this to send ACK even on RX fifo full condition */
    #define RX_PLCP_LEN_CHK             0x00000008UL
    #define WIRELESS_SEPARATION         0x00000010UL    /* toggle the bit to enable WiFi separation */
    #define RX_AMPDU_REORDER_ENABLE     0x00000020UL
	#define RX_TEMP_QUEUE_DISABLE		0x00000040UL

#define RX_LINK_INFO_CACHE_CTRL  0xB9C
    #define RX_LINK_INFO_CACHE_OUT  0x00000001UL

#define AMPDU_BMG_CTRL  0xBA0
    #define BMG_CLEAR         0x80000000UL
    #define BMG_TID           0x00000F00UL
    #define BMG_RA_IDX        0x000000FFUL

#define USB_MODE                    0x20F0

#define LMAC_CNTL       ( LMAC_REG_OFFSET + 0x000 )     /* lower mac control register */    
    #define LMAC_CNTL_TSTART        0x00000001UL
	#define LMAC_CNTL_STAMODE		0x00000006UL
	#define LMAC_CNTL_NAV_CHK		0x00000008UL
	#define LMAC_CNTL_NOCHK_MMAC	0x00000010UL
    #define LMAC_CNTL_BEACON_ENABLE 0x00000400UL
	#define LMAC_CNTL_BEACON_FILTER 0x00000800UL
	#define LMAC_CNTL_CAL_REQ 		0x00010000UL
	#define LMAC_CNTL_CAL_GNT 		0x00020000UL

#define RTSCTS_SET      ( LMAC_REG_OFFSET + 0x004 )
    #define ERP_PROTECT             0x00000001UL
    #define CTS_SELF                0x00000002UL
    #define LMAC_FILTER_ALL_PASS    0x00000004UL
    #define DISABLE_IFS_CHECK       0x00000008UL
    #define BG_PROTECT              0x00000010UL
	#define PROTECT_MODE			0x00000060UL
		#define PROTECT_SHIFT				5
		#define PROTECT_MODE_HT_NO			0x0
		#define PROTECT_MODE_HT_NON_MEMBER	0x1
		#define PROTECT_MODE_HT_20M			0x2
		#define PROTECT_MODE_HT_MIXED		0x3
	#define NONE_GREEN_FIELD		0x00000080UL
    #define RTSCTS_MCS              0x00007F00UL
	#define FORCE_RX_RSPN_RATE		0x00008000UL
    #define LMAC_RTS_CTS_THRESHOLD  0x0FFF0000UL
    #define RTSCTS_RATE             0xF0000000UL        /* 11B or NON-HT rate */

#define BEACON_SETTING  ( LMAC_REG_OFFSET + 0x008 )
    #define BEACON_SETTING_BEACON_INTERVAL      0xFFFF0000UL    /* beacon interval in timeslot (a timeslot is typically 1024 microseconds)*/

#define DTIM_INTERVAL_REG   ( LMAC_REG_OFFSET + 0x00C )
    #define DTIM_INTERVAL_REG_DTIM_INTERVAL     0x000000FFUL
    #define DTIM_INVERVAL_REG_DTIM_COUNTER      0x0000FF00UL    /* RO: current DTIM counter */
    #define DTIM_INTERVAL_REG_SLOT_TIME         0x03FF0000UL    /* slottime, reset default to 1023 */

#define BB_PROC         ( LMAC_REG_OFFSET + 0x0010 )
    #define BB_PROCTIME    0x000000FFUL
#define DSSS_DEFER_SET  ( LMAC_REG_OFFSET + 0x0014 )
#define OFDM_DEFER_SET  ( LMAC_REG_OFFSET + 0x0018 )

#define AIFS_SET        ( LMAC_REG_OFFSET + 0x001C )
    #define AIFS_SET_AIFSN_VO     0xF0000000UL
    #define AIFS_SET_AIFSN_VI     0x0F000000UL
    #define AIFS_SET_AIFSN_BE     0x00F00000UL
    #define AIFS_SET_AIFSN_BK     0x000F0000UL
    #define AIFS_SET_AIFSN_LEGACY 0x0000000FUL

#define PHYDLY_SET      ( LMAC_REG_OFFSET + 0x0024 )
    #define PHYDLY_SET_DC_PT    0xFF000000UL
    #define PHYDLY_SET_RFRXDELY 0x00FF0000UL 
    #define PHYDLY_SET_RFTXDELY 0x0000FF00UL
    #define PHYDLY_SET_RES_TIMEOUT 0x000000FFUL

#define TXOP_THRESHOLD  ( LMAC_REG_OFFSET + 0x0028 )
	#define TXOP_TO_PKT				0x00020000UL
	#define PROTECT_LEGACY			0x00010000UL	
    #define REMAINING_TXOP_THRESHOLD  	0x00001FFFUL

#define TXOP_LIMIT      ( LMAC_REG_OFFSET + 0x002C )
    #define TXOP_LIMIT_VO   0xFF000000UL
    #define TXOP_LIMIT_VI   0x00FF0000UL
    #define TXOP_LIMIT_BE   0x0000FF00UL
    #define TXOP_LIMIT_BK   0x000000FFUL

#define BASIC_RATE_BITMAP  ( LMAC_REG_OFFSET + 0x0030 )

#define BASIC_MCS_BITMAP   ( LMAC_REG_OFFSET + 0x0034 )

#define BASIC_SET          ( LMAC_REG_OFFSET + 0x0038 )
    #define BASIC_FORMAT               0x00000003UL
    #define BASIC_NONE_HT_MODE         0x0000000CUL
    #define BASIC_CHANNEL_CH_BANDWIDTH 0x00000030UL
    #define BASIC_CHANNEL_OFFSET       0x000000C0UL
    #define BASIC_SGI                  0x00000100UL
    #define LMAC_CH_BW_CTRL_CH_OFFSET  0x00000600UL
	#define LMAC_DECIDE_CH_OFFSET	   0x00000800UL	/* enable LMAC to replace the channel offset of pre-heading */
	#define LMAC_FORCE_BW20			   0x00001000UL
	#define LMAC_OR_CCA_DIS			   0x00002000UL /* separate CCA1 and CCA0 */
	#define LMAC_CCA1_EN			   0x00004000UL /* enable the CCA1(secondary) checking*/
	#define LMAC_DECIDE_CTRL_FR	   	   0x00008000UL /* The pre-heading of initial control frame is decided by transmitted packet */	 
    #define LMAC_CH_BW_CTRL_AUTO       0x0000E800UL
    #define LMAC_CH_BW_CTRL_AUTO_MASK  0x0000F800UL

#define BEACON_TIMEOUT  ( LMAC_REG_OFFSET + 0x003C )            /* beacon TX timeout in microseconds */
    #define BEACON_TIMEOUT_VAL      0x0000FFFFUL

#define RF_TIME_CTRL    ( LMAC_REG_OFFSET + 0x0040 )

#define CCA_THRESHOLD	( LMAC_REG_OFFSET + 0x0044 )

#define BEACON_CONTROL      ( LMAC_REG_OFFSET + 0x0050 )
    #define BEACON_CONTROL_ENABLE   0x00000001UL
    #define BEACON_CONTROL_RATE_CNTL_BY_REG   0x00000002UL
    #define BEACON_CONTROL_RATE     0x00000F00UL
    #define BEACON_CONTROL_FORMAT   0x00003000UL
    #define BEACON_CONTROL_11B_SP   0x00004000UL
    #define BEACON_CONTROL_NON_SOUNDING  0x00008000UL
    #define BEACON_CONTROL_MCS      0x007F0000UL
    #define BEACON_CONTROL_SGI      0x00800000UL
    #define BEACON_CONTROL_TX_IDLE  0x01000000UL
#define BEACON_TXDSC_ADDR   ( LMAC_REG_OFFSET + 0x0054 )
#define BEACON_TX_STATUS    ( LMAC_REG_OFFSET + 0x005C )
    #define BEACON_TX_STATUS_RESULT 0x000000FFUL

#define BEACON_SETTING2     ( LMAC_REG_OFFSET + 0x0060 )
    #define BEACON_SETTING_PRE_BEACON_INTR      0x000003FFUL    /* pre TBTT interrupt time in TU */
    #define SW_CONTROL_SENDBAR_RATE		0x80000000UL

#define LMAC_MEDBUSY_CNT    ( LMAC_REG_OFFSET + 0x0078 )
    #define LMAC_MEDBUSY_CNT_CLEAR  0x80000000UL

#define LMAC_40MHZ_TX_REQUEST   ( LMAC_REG_OFFSET + 0x0084 )    /* 24bits counter for 40Mhz TX requests/attempts */
#define LMAC_40MHZ_TX_REJECT    ( LMAC_REG_OFFSET + 0x0088 )    /* 24bits counter for 40Mhz TX downgrade to 20Mhz TX due to seconard channel busy */
#define EXTEND_PD               ( LMAC_REG_OFFSET + 0x008c )
    #define EXTEND_PERIOD    0x0000003FUL

#define LMAC_OPTION_SEL    ( LMAC_REG_OFFSET + 0x00A4 )
    #define STOP_BKOFF_CNT_OPT     0x00000080UL
    #define SIFSCHK_OPT            0x00000200UL

#define FREE_CONTROL_DELAY_0    ( LMAC_REG_OFFSET + 0x00B0 )
    #define DAC_ON_TON             0xFF000000UL
    #define DAC_ON_TOFF            0x00FF0000UL
    #define BB_TXPE_TON            0x0000FF00UL
    #define BB_TXPE_TOFF           0x000000FFUL

#define FREE_CONTROL_DELAY_1    ( LMAC_REG_OFFSET + 0x00B4 )
    #define RF_TX_EN_TON           0xFF000000UL
    #define RF_TX_EN_TOFF          0x00FF0000UL
    #define PA_EN_TON              0x0000FF00UL
    #define PA_EN_TOFF             0x000000FFUL

#define DEBUG_PORT      ( LMAC_REG_OFFSET + 0x0200 )

#define SEC_KEY_CTRL    ( SEC_REG_OFFSET + 0x0000 )
    #define SEC_KEY_CTRL_STA_IDX            0x00FF0000UL    /* re-key station index, or bssid index */
    #define SEC_KEY_CTRL_STA_IDX_GLOBAL     0x01000000UL    /* set to 1 for group-key, set to 0 for pair-wise key */
    #define SEC_KEY_CTRL_REKEY_SET          0x02000000UL
    #define SEC_KEY_CTRL_REKEY_STA_GKEY     0x04000000UL
    #define SEC_KEY_CTRL_HW_STATUS          0xF0000000UL
    #define SEC_KEY_CTRL_RXPN_INI_VAL       0x0000FF00UL
    #define SEC_KEY_CTRL_CIPHER_MODE        0x0000000FUL
    #define SEC_KEY_CTRL_REKEY_REQ          (SEC_KEY_CTRL_HW_STATUS | SEC_KEY_CTRL_REKEY_SET)

#define SEC_GPKEY_BA    ( SEC_REG_OFFSET + 0x0004 )

#define SEC_STA_GPKEY_BA ( SEC_REG_OFFSET + 0x0008 )

#define SECENG_CTRL     ( SEC_REG_OFFSET + 0x000C )
	#define SECENG_CTRL_BYPASS				0x00000004UL
    #define SECENG_CTRL_RSC_CHECK_OFF       0x00000008UL
    #define SECENG_CTRL_WAPI_RSC_CHECK_OFF  0x00000040UL
    #define SECENG_CTRL_STA_RSC_CHECK_OFF   0x00000080UL
    #define SECENG_CTRL_AMPDU_RSC_CHECK_OFF 0x00000200UL
	#define SECENG_CTRL_ICV_ERR_DROP		0x00010000UL
	#define SECENG_CTRL_MIC_ERR_DROP		0x00020000UL
	#define SECENG_CTRL_GKEY_RSC_ERR_DROP   0x00040000UL /* unused function */
	#define SECENG_CTRL_KEYID_ERR_DROP		0x00080000UL
	#define SECENG_CTRL_REKEY_ERR_DROP		0x00100000UL
	#define SECENG_CTRL_KEY_NONE_DROP		0x00200000UL
	#define SECENG_CTRL_BYPASS_DROP			0x00400000UL

#define SEC_PVKEY_BA    ( SEC_REG_OFFSET + 0x0010 )
#define SEC_PVKEY_BA2   ( SEC_REG_OFFSET + 0x0014 )
#define SEC_PVKEY_BA3   ( SEC_REG_OFFSET + 0x0018 )
#define SEC_PVKEY_BA4   ( SEC_REG_OFFSET + 0x001C )

// PANTHER TXQ
#define ACQ_EN          0xC00
#define ACQ_KICK        0xC04    /* obsoleted */
#define ACQ_INTR        0xC08
   #define ACQ_INTR_TX_DONE      0x00000FFFUL
   #define ACQ_INTR_CMD_SWITCH   0x0FFF0000UL
#define ACQ_INTRM       0xC0C
   #define ACQ_INTRM_TX_DONE     0x00000FFFUL
   #define ACQ_INTRM_CMD_SWITCH  0x0FFF0000UL
#define ACQ_INTR2       0xC10
   #define ACQ_INTR2_RETRY_FAIL  0x00000FFFUL
#define ACQ_INTRM2      0xC14
   #define ACQ_INTRM2_RETRY_FAIL 0x00000FFFUL

#define AC0_CMD0_BADDR  0xC20
#define AC0_CMD0_INFO   0xC24
#define AC0_CMD0_INFO2  0xC28
#define AC0_CMD0_INFO3  0xC2C

#define ACQ_BADDR(qid, cmdid) (AC0_CMD0_BADDR + (qid * 0x20) + (cmdid * 0x10))
      #define ACQ_REQ         0x80000000UL
      #define ACQ_REQ_LOCK    0x40000000UL
      #define ACQ_TABLE_SIZE  0x30000000UL
#define ACQ_INFO(qid, cmdid)  (AC0_CMD0_INFO + (qid * 0x20) + (cmdid * 0x10))
      #define ACQ_SINGLE      0x80000000UL
      #define ACQ_SPACE       0x70000000UL
      #define ACQ_MAX_LEN     0x0C000000UL
      #define ACQ_BUF_SIZE    0x03000000UL
      #define ACQ_AIDX        0x00FF0000UL
      #define ACQ_TID         0x0000F000UL
      #define ACQ_SSN         0x00000FFFUL
#define ACQ_INFO2(qid, cmdid) (AC0_CMD0_INFO2 + (qid * 0x20) + (cmdid * 0x10))
#define ACQ_INFO3(qid, cmdid) (AC0_CMD0_INFO3 + (qid * 0x20) + (cmdid * 0x10))

#define UTX_DEBUG       0xE28
#define DEBUG_PORT_SEL  0XEFC
#define PS_RAW_STAT_BM  0xF00
#define PS_STAT_BM      0xF04
#define BSS_PS_STAT     0xF0C

#define NOA_STAT_BSS0_BM   0xF10
#define NOA_STAT_BSS1_BM   0xF14
#define NOA_STAT_BSS2_BM   0xF18
#define NOA_STAT           0xF1C
#define NOA_STAT_BSS3_BM   0xF20

#define TS0_CTRL			( LMAC_REG_OFFSET + 0x100 )
    #define PS_NOA_ENABLE                       0x00000200UL
    #define TS_ENABLE                           0x00000100UL

#define TS0_ADJUST			( LMAC_REG_OFFSET + 0x104 )
    #define TS_INACCURACY_US     		0x0000FFFFUL		/* Unit: us */
    #define TS_ADJUST_US			0xFFFF0000UL		/* Unit: us */

#define TS0_BE				( LMAC_REG_OFFSET + 0x108 )
    #define BEACON_TIMEOUT_INTERVAL	        0x0000FFFFUL		/* Unit: TU, start after TBTT */
    #define BEACON_INTERVAL                     0xFFFF0000UL		/* Unit: TU(1024us) */

#define TS0_DTIM			( LMAC_REG_OFFSET + 0x10C )
    #define DTIM_INTERVAL			0x000000FFUL
    #define DTIM_COUNT				0x0000FF00UL

#define TS0_O_H				( LMAC_REG_OFFSET + 0x140 )
   #define TS0_O     TS0_O_H
#define TS0_O_L				( LMAC_REG_OFFSET + 0x144 )
#define TS0_NEXT_TBTT_H		        ( LMAC_REG_OFFSET + 0x148 )
#define TS0_NEXT_TBTT_L		        ( LMAC_REG_OFFSET + 0x14C )
   #define TS0_NEXT_TBTT   TS0_NEXT_TBTT_H

#define TS0_NOA_START                   ( LMAC_REG_OFFSET + 0x180 )
#define TS0_NOA_END		        ( LMAC_REG_OFFSET + 0x184 )
#define TS0_BTC_START                   ( LMAC_REG_OFFSET + 0x188 )
#define TS0_BTC_END		        ( LMAC_REG_OFFSET + 0x18C )

#define TS0_INACC_LIMIT 	        ( LMAC_REG_OFFSET + 0x1C0 )
#define TS0_BEACON_COUNT 	        ( LMAC_REG_OFFSET + 0x1C4 )
   #define TS_BEACON_LOST_COUNT        0x0000FFFFUL
   #define TS_BEACON_RECV_COUNT        0xFFFF0000UL

#define TS_CTRL(x)               ( TS0_CTRL + ((x) * 0x10) )
#define TS_ADJUST(x)             ( TS0_ADJUST + ((x) * 0x10) )
#define TS_BE(x)                 ( TS0_BE + ((x) * 0x10) )
#define TS_DTIM(x)               ( TS0_DTIM + ((x) * 0x10) )
#define TS_O_H(x)                ( TS0_O_H + ((x) * 0x10) )
#define TS_O_L(x)                ( TS0_O_L + ((x) * 0x10) )
#define TS_O(x)                  ( TS0_O_H + ((x) * 0x10) )
#define TS_NEXT_TBTT_H(x)        ( TS0_NEXT_TBTT_H + ((x) * 0x10) )
#define TS_NEXT_TBTT_L(x)        ( TS0_NEXT_TBTT_L + ((x) * 0x10) )
#define TS_NEXT_TBTT(x)          ( TS0_NEXT_TBTT + ((x) * 0x10) )
#define TS_NOA_START(x)          ( TS0_NOA_START + ((x) * 0x10) )
#define TS_NOA_END(x)            ( TS0_NOA_END + ((x) * 0x10) )
#define TS_BTC_START(x)          ( TS0_BTC_START + ((x) * 0x10) )
#define TS_BTC_END(x)            ( TS0_BTC_END + ((x) * 0x10) )
#define TS_INACC_LIMIT(x)        ( TS0_INACC_LIMIT + ((x) * 0x8) )          
#define TS_BEACON_COUNT(x)       ( TS0_BEACON_COUNT + ((x) * 0x8))

#define STA_GLOBAL_CONTROL       ( LMAC_REG_OFFSET + 0x1E4 )
    #define TX_BB_PROC_DELAY        0xFF000000UL
    #define RX_BB_PROC_DELAY        0x00FF0000UL
    #define IBSS_ATIM_RETRY_COUNT   0x00000F00UL
    #define IBSS_ATIM_EN            0x00000002UL
    #define IBSS_ALL_UPDATE         0x00000001UL

#define GLOBAL_BEACON		 ( LMAC_REG_OFFSET + 0x1E8 )
    #define IBSS_BEACON_RX_COUNT    0xFF000000UL
    #define IBSS_BEACON_TX_COUNT    0x00FF0000UL
    #define BEACON_BACKOFF	    0x000003FFUL

#define GLOBAL_PRE_TBTT		 ( LMAC_REG_OFFSET + 0x1EC )
    #define PRE_TBTT_INTERVAL_S	    0x0000FFFFUL
    #define PRE_TBTT_INTERVAL	    0xFFFF0000UL

#define TS_INT_STATUS		 ( LMAC_REG_OFFSET + 0x1F0 )

#define TS_ERROR_INT_STATUS	 ( LMAC_REG_OFFSET + 0x1F4 )

#define TS_INT_MASK              ( LMAC_REG_OFFSET + 0x1F8 )   /* 4 bits each, ts3:ts2:ts1:ts0 */
    #define TS_NOA_START_MASK          0x0F000000UL            /* NoA start */
    #define PRE_TBTT_TIMEOUT           0x000F0000UL            /* beacon timeout */
    #define PRE_TBTT_HW                0x00000F00UL            /* pre-tbtt for hardware */
    #define PRE_TBTT_SW                0x0000000FUL            /* pre-tbtt for software */
    #define PRE_TBTT_SW_TS3            0x00000008UL
    #define PRE_TBTT_SW_TS2            0x00000004UL
    #define PRE_TBTT_SW_TS1            0x00000002UL
    #define PRE_TBTT_SW_TS0            0x00000001UL

#define TS_ERROR_INT_MASK	 ( LMAC_REG_OFFSET + 0x1FC )    /* 4 bits each, ts3:ts2:ts1:ts0 */
    #define TS_ERR2_MASK               0x0F000000UL             /* next_tbtt > 2 x beacon_interval */
    #define TS_ERR1_MASK               0x000F0000UL             /* next_tbtt < ts_o */
    #define TS_NOA_END_MASK            0x00000F00UL             /* NoA end */
    #define TS_ERR0_MASK               0x0000000FUL             /* abs(ts_i-ts_o)>limit */

#endif // __MAC_REGS_H__

