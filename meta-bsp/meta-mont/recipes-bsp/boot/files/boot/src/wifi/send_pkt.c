#include "mt_types.h"
#define BSS0_AP_NUM     128
#define BSS0_BC_STA_NUM 255
#define BSS0_DUT_STA_NUM 129
#define BSS0_STA_NUM	64 
#define BSS4_AP_NUM           65    // with RX header conversion
extern u16 tx_rates[4];
extern int TX_packet_simply(int source, int target, int tid, int payload_length, u16 *rates);
int send_pkt(void)
{
	int payload_length = 64;
	int rt;
	rt=TX_packet_simply(BSS0_AP_NUM, BSS0_BC_STA_NUM, 0, payload_length, tx_rates);
	return rt;
}
