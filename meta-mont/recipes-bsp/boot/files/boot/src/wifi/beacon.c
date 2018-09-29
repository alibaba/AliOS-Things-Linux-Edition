#include <lib.h>
#include "mac.h"
#include "mac_internal.h"
//#include "network.h"
#include "genpkts.h"

//#include "int.h"

#include "packet.h"
//#include "cosim_api.h"
#include "buffer.h"

#include "genpkts.h"

#include "packet.h"
#include "beacon.h"
#include "netprot.h"

int printf(char *fmt, ...);

int bss_beacon_interval[MAX_BSSIDS];
int tsf_mode[MAX_BSSIDS];

#define TIME_UNIT    1024

void arthur_beacon_setup(unsigned short beacon_interval, unsigned char dtim_period)
{
#define LMAC_DEFAULT_SLOT_TIME          1024
#define LMAC_DEFAULT_BEACON_TX_TIMEOUT  (30 *1000) /* the unit is us */



#define PRE_HW_TBTT_DEFAULT_TIME				0x10

    MACREG_WRITE32(TS0_DTIM, dtim_period);

    if (beacon_interval <= 6)
    {
        MACREG_WRITE32(GLOBAL_PRE_TBTT, (beacon_interval / 2) | ((((beacon_interval)/2)&0xffff) << 16));
        MACREG_WRITE32(TS0_BE, (beacon_interval << 16)|((((beacon_interval)/2)&0xffff)));
    }
    else
    {
        /*	
        ---|----------------|-----------------|-------------------|------------------|--- 
        HW pre-TBTT    SW pre-TBTT           TBTT               timeout          HW pre-TBTT   
                                                                                   
               Under AdHoc mode, beacon listen and sync window is from HW pre-TBTT to timeout. 
               To speed timestamp synchronization of beacon, we shall enlarge listen 
               window as possiable as we can.
        */
        /*the pre-tbtt setting is 16bits width, beware of overflow problem */
        MACREG_WRITE32(GLOBAL_PRE_TBTT, PRE_TBTT_TIME(beacon_interval)|((((beacon_interval)/2)&0xffff) << 16));
        /* JH modify beacon sync window to entire TBTT. So we do not extend duration of timeout */
        //MACREG_WRITE32(TS0_BE, (beacon_interval << 16)|((((beacon_interval*1000)/2)&0xffff) - 1));
        MACREG_WRITE32(TS0_BE, (beacon_interval << 16)|((((beacon_interval)/3)&0xffff) - 1));
    }
}

void arthur_beacon_start(unsigned long beacon_bitrate)
{
    /* enable LMAC beacon timer */
    MACREG_UPDATE32(LMAC_CNTL, LMAC_CNTL_BEACON_ENABLE, LMAC_CNTL_BEACON_ENABLE);

    MACREG_UPDATE32(BEACON_CONTROL, (beacon_bitrate << 8)|BEACON_CONTROL_ENABLE,
                    BEACON_CONTROL_TX_IDLE|BEACON_CONTROL_RATE|BEACON_CONTROL_ENABLE);

    /* enable TS timer */
    MACREG_WRITE32(TS0_CTRL, TS_ENABLE);

    MACREG_UPDATE32(TS_INT_MASK, 0, 0x01);
}

void arthur_beacon_stop(void)
{
    MACREG_UPDATE32(TS_INT_MASK, 0x1, 0x1);

    MACREG_WRITE32(TS0_CTRL, 0);

    MACREG_WRITE32(GLOBAL_PRE_TBTT, PRE_TBTT_INTERVAL_S|(PRE_HW_TBTT_DEFAULT_TIME << 16));
    MACREG_UPDATE32(BEACON_CONTROL, 0, BEACON_CONTROL_ENABLE);
    MACREG_UPDATE32(LMAC_CNTL, 0x0, LMAC_CNTL_BEACON_ENABLE);
}

unsigned char beacon_packet[] = {
    0x80, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x78, 0x00, 0x00, 0x00, 0x00, 0x42,
    0x78, 0x00, 0x00, 0x00, 0x00, 0x42,
    0x00, 0x00,            /* seqnum */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* timestamp */
    0x64, 0x00,   /* beacon interval */
    0x21, 0x04,   /* capability info */
    0x00, 0x08,  'r',  'e',  'c',  'o',  'v',  'e',  'r',  'y',
    0x01, 0x08, 0x82, 0x84, 0x8B, 0x96, 0x8C, 0x12, 0x98, 0x24,     /* support rates */
    0x05, 0x13, 0x00, 0x01, 0x01, /* AID0, TIM , length 4, dtim count 0, peroid 1, bitmap control 0,*/
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, /* partial bitmap set all aid = 1 */
    0x32, 0x04, 0xB0, 0x48, 0x60, 0x6C,  /* extended support rates */
};

void init_beacon_settings(void)
{
    if (bootvars.mac0)
    {
        memcpy(&beacon_packet[10], bootvars.mac0, 6);
        memcpy(&beacon_packet[16], bootvars.mac0, 6);
    }
}

void beacon_kick(void)
{
    if(!(MACREG_READ32(BEACON_CONTROL) & BEACON_CONTROL_ENABLE))
        MACREG_UPDATE32(BEACON_CONTROL, BEACON_CONTROL_ENABLE, BEACON_CONTROL_ENABLE);
    MACREG_WRITE32(BEACON_CONTROL, MACREG_READ32(BEACON_CONTROL));
}

extern void cosim_panic(char *message);
ssq_tx_descr beacon_descr;
void beacon_descriptor_setup(int beacon_packet_length)
{
    u8 *dptr;
    ssq_tx_descr *b_desc;
    buf_header *bhdr;
    int pkt_length = beacon_packet_length;

    bhdr = bhdr_get_first(info);
    if (bhdr == NULL)
        cosim_panic("beacon_descriptor_setup: no bhdr");

    bhdr->offset = 0;
#if !defined(NEW_BUFH)
    bhdr->offset_h = 0;
#endif
    bhdr->len = pkt_length;
    bhdr->next_index = 0;
    bhdr->ep = 1;

    dptr = (u8 *)UNCACHED_ADDR(bhdr->dptr);

//  memset(dptr, 0xff, pkt_length);
    memcpy(dptr, beacon_packet, sizeof(beacon_packet));
//  dptr[0] = 0x80;  /* BEACON */
//  dptr[1] = 0x00;

    b_desc = (ssq_tx_descr*)UNCACHED_ADDR(&beacon_descr);

    b_desc->tx_descr.bssid = 0;

#define MAC_AC_POLICY_NORMAL_ACK 0
#define MAC_ACK_POLICY_NOACK     1

    b_desc->tx_descr.ack_policy = MAC_ACK_POLICY_NOACK;
    b_desc->tx_descr.dis_duration = 0;
    b_desc->tx_descr.ins_ts = 1;
    b_desc->tx_descr.bc = 1;

    b_desc->tx_descr.pkt_length = pkt_length;
    b_desc->tx_descr.frame_header_head_index = bhdr_to_idx(info, bhdr);
    b_desc->next_pointer = 0;

//  b_desc->tx_descr.u.beacon.format = 0; /* NONE_HT */
    b_desc->tx_descr.u.beacon.format = FMT_11B;
    b_desc->tx_descr.u.beacon.ch_offset = 0; /* OFFSET_20 */
    b_desc->tx_descr.eor = 1;

    MACREG_WRITE32(BEACON_TXDSC_ADDR, PHYSICAL_ADDR(&beacon_descr));

    beacon_kick();
}

void mac_config_tbtt_intr(u32 tsf_idx, u32 beacon_interval)
{
    if (beacon_interval <= 6)
    {
#if 1
        /* simultion env. only, fix pre-tbtt & post-tbtt to 1 TU */
        MACREG_WRITE32(GLOBAL_PRE_TBTT, 0x00010001UL);
        MACREG_WRITE32(TS_BE(tsf_idx), (beacon_interval << 16)| 0x0001UL);
#else
        MACREG_WRITE32(GLOBAL_PRE_TBTT, (beacon_interval / 2) | ((((beacon_interval)/2)&0xffff) << 16));
        MACREG_WRITE32(TS_BE(tsf_idx), (beacon_interval << 16)|((((beacon_interval)/2)&0xffff)));
#endif
    }
    else
    {
        MACREG_WRITE32(GLOBAL_PRE_TBTT, PRE_TBTT_TIME(beacon_interval)|((((beacon_interval)/2)&0xffff) << 16));
        MACREG_WRITE32(TS_BE(tsf_idx), (beacon_interval << 16)|((((beacon_interval)/3)&0xffff) - 1));
    }

    MACREG_UPDATE32(TS_INT_MASK, 0, (0x01 << tsf_idx));
    tsf_mode[tsf_idx] = TSF_MODE_STA;
}

void arthur_cfg_start_tsync(u32 tsf_idx, u32 beacon_interval, u32 dtim_interval, u32 timeout)
{
    if (timeout == 0) /* add for IBSS mode to fill atim window (TU) */
        timeout = ((beacon_interval/3) & 0xffff) - 1;

    //MACREG_UPDATE32(TS_INT_MASK, mask, mask);
    MACREG_UPDATE32(TS_ERROR_INT_MASK, 0, (0x01 << (tsf_idx)));

    /* JH modify beacon sync window to entire TBTT. So we resume original duration of timeout(1/3 TBTT). */
    MACREG_WRITE32(TS_BE(tsf_idx), ((beacon_interval << 16)|(timeout)));

    MACREG_WRITE32(TS_DTIM(tsf_idx), dtim_interval);

    /* preset/reset TSF counter */
#if 1
    MACREG_WRITE64(TS_NEXT_TBTT(tsf_idx), (TEST_TSF_START * tsf_idx));
    MACREG_WRITE64(TS_O(tsf_idx), (TEST_TSF_START * tsf_idx) - (beacon_interval/2));
#else
    MACREG_WRITE64(TS_NEXT_TBTT(tsf_idx), 0x0ULL);
    MACREG_WRITE64(TS_O(tsf_idx), 0x0ULL);
#endif

    MACREG_WRITE32(TS_INACC_LIMIT(tsf_idx), 10);

    bss_beacon_interval[tsf_idx] = beacon_interval;

    /* clear receive & loss counter for client mode check ap alive */
    MACREG_WRITE32(TS_BEACON_COUNT(tsf_idx), 0);

    mac_config_tbtt_intr(tsf_idx, beacon_interval);

    MACREG_WRITE32(TS_CTRL(tsf_idx), TS_ENABLE);
}

void dump_ts_o(void)
{
    int i;
    u64 ts_o;

    for (i=0;i<MAX_BSSIDS;i++)
    {
        ts_o = MACREG_READ64(TS_O(i));
        printf("TSF(%d) %08x %08x, ", i, (u32)(ts_o >> 32), (u32)(ts_o));
    }
    printf("\n");
}

#if defined(LYNX)
    #define NOA_INTR_SW_PROCESSING_DELAY_MAX 0x10
#else
/* more delay jitter allowed for QEMU */
    #define NOA_INTR_SW_PROCESSING_DELAY_MAX 0x100
#endif

void tsf_intr(void)
{
    unsigned long ts_status;
    unsigned long ts_err_status;
    int i;
    u64 ts_o;
    u64 ts_noa;
    u64 bcn_interval;

    ts_status = MACREG_READ32(TS_INT_STATUS);
    ts_err_status = MACREG_READ32(TS_ERROR_INT_STATUS);
    //printf("====> ts_status %08x ts_err_status %08x\n", ts_status, ts_err_status);

    if (PRE_TBTT_SW & ts_status)
    {
        for (i=0;i<MAX_BSSIDS;i++)
        {
            if (( 0x01 << i ) & ts_status)
            {
                if (tsf_mode[i]==TSF_MODE_STA)
                {
                    dump_ts_o();
                    printf("STA PRE-TBTT, TSF-%d\n", i);
                }
                else
                {
                    dump_ts_o();

                    if (tsf_mode[i]==TSF_MODE_IBSS)
                    {
                        printf("IBSS PRE-TBTT, TSF-%d\n", i);

                        printf("IBSS beacon rx %d, tx %d\n", MACREG_READ32(GLOBAL_BEACON)>>24, (MACREG_READ32(GLOBAL_BEACON)>>16) & 0xff);

                        /* set next beacon backoff period */
                        MACREG_UPDATE32(GLOBAL_BEACON, TEST_TSF_IBSS_BACKOFF, BEACON_BACKOFF);
                    }
                    else
                    {
                        printf("AP PRE-TBTT, TSF-%d\n", i);
                    }

                    /* AP mode: PRE-TBTT interrupt for Beacon TX */
                    if (!(MACREG_READ32(BEACON_CONTROL) & BEACON_CONTROL_TX_IDLE))
                    {
                        MACREG_WRITE32(TS_INT_STATUS, ts_status);

                        printf("Beacon is busy, skip this TBTT???\n");
                    }

                    dump_ts_o();
                    printf("Trigger AP Beacon TX\n");

                    /* write 1 clear to IDLE bit, set HW to busy , trigger the HW */
                    MACREG_WRITE32(BEACON_CONTROL, MACREG_READ32(BEACON_CONTROL));
                }
            }
        }
    }

    if (TS_ERR0_MASK & ts_err_status)
    {
        for (i=0;i<MAX_BSSIDS;i++)
        {
            if (( 0x01 << i ) & ts_err_status)
            {
                ts_o = MACREG_READ64(TS_O(i));
                printf("RX TSF out-of-range, TSF-%d, %08x %08x\n", i, (u32)(ts_o >> 32), (u32)(ts_o));

                bcn_interval = (u64) (bss_beacon_interval[i] * TIME_UNIT);
                ts_o = (ts_o + (bcn_interval - 1));
                ts_o = ts_o - (ts_o % bcn_interval);
                MACREG_WRITE64(TS_NEXT_TBTT(i), ts_o);

                printf("ADJUST next TBTT to %08x %08x\n", (u32)(ts_o >> 32), (u32)(ts_o));
            }
        }
    }

    if (TS_NOA_START_MASK & ts_status)
    {
        for (i=0;i<MAX_BSSIDS;i++)
        {
            if (( 0x01 << (i + 24) ) & ts_status)
            {
                ts_o = MACREG_READ64(TS_O(i));
                ts_noa = MACREG_READ32(TS_NOA_START(i));

                printf("TSF(%d): NoA start intr @ %08x %08x, TS_NOA_START_LOW %08x\n", 
                       i, (u32)(ts_o >> 32), (u32)(ts_o), (u32) ts_noa);

                if ((((u32)(ts_noa + NOA_INTR_SW_PROCESSING_DELAY_MAX)) < ((u32) ts_o))
                    || (((u32)(ts_noa)) > ((u32) ts_o)))
                {
                    cosim_panic("Wrong NoA start interrupt");
                }
            }
        }
    }

    if (TS_NOA_END_MASK & ts_err_status)
    {
        for (i=0;i<MAX_BSSIDS;i++)
        {
            if (( 0x01 << (i + 8) ) & ts_err_status)
            {
                ts_o = MACREG_READ64(TS_O(i));
                ts_noa = MACREG_READ32(TS_NOA_END(i));

                printf("TSF(%d): NoA end intr @ %08x %08x, TS_NOA_END_LOW %08x\n", 
                       i, (u32)(ts_o >> 32), (u32)(ts_o), (u32) ts_noa);

                if ((((u32)(ts_noa + NOA_INTR_SW_PROCESSING_DELAY_MAX)) < ((u32) ts_o))
                    || (((u32)(ts_noa)) > ((u32) ts_o)))
                {
                    cosim_panic("Wrong NoA end interrupt");
                }
            }
        }
    }

    MACREG_WRITE32(TS_INT_STATUS, ts_status);
    MACREG_WRITE32(TS_ERROR_INT_STATUS, ts_err_status);
}



/* beacon_interval in TU */
mpdu *beacon_pkt[2];
PKT_CFG beacon_pktcfg;
struct beacon_config
{
    int source_sta_num;
    int target_sta_num;
    int length;
    int beacon_interval;
    int dtim_period;
    int beacon_count;
    u64 first_beacon_time;
    u64 first_timestamp;
    int beacon_drop_control;
};
struct beacon_config bcfg[2];

#if 0
void cosim_beacon_setup(int beacon_id, int source_sta_num, int target_sta_num, int length, int beacon_interval, int dtim_period)
{
    if ((beacon_id != 0) && (beacon_id != 1))
        return;

    beacon_pktcfg.wifi_type = 0;
    beacon_pktcfg.subtype = 8;

    bcfg[beacon_id].source_sta_num = source_sta_num;
    bcfg[beacon_id].target_sta_num = target_sta_num;
    bcfg[beacon_id].length = length;
    bcfg[beacon_id].beacon_interval = beacon_interval;
    bcfg[beacon_id].dtim_period = dtim_period;
    bcfg[beacon_id].beacon_count = 0;
    bcfg[beacon_id].first_beacon_time = 0;
    bcfg[beacon_id].first_timestamp = 0;
    bcfg[beacon_id].beacon_drop_control = 0;

    if (beacon_id == 0)
        COSIM_REG_WRITE32(ISS_TIMER0_INTERVAL, beacon_interval * TU);
    else if (beacon_id == 1)
        COSIM_REG_WRITE32(ISS_TIMER1_INTERVAL, beacon_interval * TU);
}

void cosim_beacon_ext_config(int beacon_id, int beacon_drop_control, u64 first_timestamp)
{
    if ((beacon_id != 0) && (beacon_id != 1))
        return;

    bcfg[beacon_id].first_timestamp = first_timestamp;
    bcfg[beacon_id].beacon_drop_control = beacon_drop_control;
}

raw_packet beacon_rawpkt;
void cosim_beacon_inject(int beacon_id)
{
    mpdu *pkt;
    u64 target_beacon_time;
    u64 target_beacon_time_us;
    unsigned char *tsf;


#define MAX_SW_PROC_DELAY  (1000 * 1000)          // in nano-seconds 

    if (bcfg[beacon_id].beacon_count==0)
        bcfg[beacon_id].first_beacon_time = (cosim_time() + MAX_SW_PROC_DELAY);

    target_beacon_time = ((bcfg[beacon_id].beacon_interval * TU) * bcfg[beacon_id].beacon_count) + bcfg[beacon_id].first_beacon_time;
    target_beacon_time_us = bcfg[beacon_id].beacon_interval * (TU / 1000) * bcfg[beacon_id].beacon_count;
    target_beacon_time_us += bcfg[beacon_id].first_timestamp;

    bcfg[beacon_id].beacon_count++;

    if (bcfg[beacon_id].beacon_drop_control && ((bcfg[beacon_id].beacon_count % bcfg[beacon_id].beacon_drop_control)==0))
    {
        /* drop this beacon */
        goto Exit;
    }

    pkt = packet_alloc();
    if (pkt)
    {
        cosim_generate_mpdu(pkt, &beacon_pktcfg, bcfg[beacon_id].source_sta_num, bcfg[beacon_id].target_sta_num, DIRECTION_IN, 0, bcfg[beacon_id].length);

        tsf = &pkt->data[24];
        tsf[7] = target_beacon_time_us >> 56;
        tsf[6] = target_beacon_time_us >> 48;
        tsf[5] = target_beacon_time_us >> 40;
        tsf[4] = target_beacon_time_us >> 32;
        tsf[3] = target_beacon_time_us >> 24;
        tsf[2] = target_beacon_time_us >> 16;
        tsf[1] = target_beacon_time_us >> 8;
        tsf[0] = target_beacon_time_us;

        pkt->flags = 0;
        pkt->tid = 0;

        packet_append_fcs(pkt);
        packets_to_raw_packet(pkt, &beacon_rawpkt, 0);

        //packet_dump(pkt);
        packet_enqueue(pkt);

        beacon_rawpkt.flags |= PKT_FLAG_AT_ABSOLUTE_TIME | PKT_FLAG_ZERO_IFS_BACKOFF;
        beacon_rawpkt.timestamp = target_beacon_time;

        cosim_packet_inject(&beacon_rawpkt);
    }

    Exit:
    return;
}

void cosim_beacon_start(int beacon_id)
{
    if (beacon_id == 0)
        COSIM_REG_UPDATE32(ISS_TIMER_MASK, 0x0, 0x01);
    else if (beacon_id == 1)
        COSIM_REG_UPDATE32(ISS_TIMER_MASK, 0x0, 0x02);
}

void cosim_beacon_stop(int beacon_id)
{
    if (beacon_id == 0)
        COSIM_REG_UPDATE32(ISS_TIMER_MASK, 0xF, 0x01);
    else if (beacon_id == 1)
        COSIM_REG_UPDATE32(ISS_TIMER_MASK, 0xF, 0x02);
}
#endif
