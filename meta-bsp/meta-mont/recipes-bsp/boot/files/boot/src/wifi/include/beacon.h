#ifndef __BEACON_H__
#define __BEACON_H__

void arthur_beacon_setup(unsigned short beacon_interval, unsigned char dtim_period);
void arthur_beacon_start(unsigned long beacon_bitrate);
void arthur_beacon_stop(void);
void tsf_intr(void);
void dump_ts_o(void);

void beacon_descriptor_setup(int beacon_packet_length);

#define TU  (1024 * 1000)

/* beacon_interval unit: TU,  TU = 1024 micro-seconds */
void cosim_beacon_setup(int beacon_id, int source_sta_num, int target_sta_num, int length, int beacon_interval, int dtim_period);

void cosim_beacon_ext_config(int beacon_id, int beacon_drop_control, u64 first_beacon_time);

void cosim_beacon_start(int beacon_id);

void cosim_beacon_stop(int beacon_id);

void cosim_beacon_inject(int beacon_id);

void arthur_cfg_start_tsync(u32 tsf_idx, u32 beacon_interval, u32 dtim_interval, u32 timeout);

extern int tsf_mode[];
#define TSF_MODE_AP     0
#define TSF_MODE_STA    1
#define TSF_MODE_IBSS   2

#define TEST_TSF_START  0x100000ULL
#define TEST_TSF_IBSS_BACKOFF 3

#endif //__BEACON_H__

