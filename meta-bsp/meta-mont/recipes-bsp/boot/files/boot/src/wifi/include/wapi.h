/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file wapi.h
*   \brief  WPA definition.
*   \author Montage
*/

#ifndef _WAPI_H_
#define _WAPI_H_

#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif

#define WAI_BK_LENGTH			16
#define WAI_NMK_LENGTH			16
#define WAI_BKID_LENGTH			16

#define WAI_ADDID_LENGTH		(2 * ETH_ALEN)

#define WAI_AE_CHALLENGE_LENGTH			32
#define WAI_ASUE_CHALLENGE_LENGTH		32
#define WAI_CHALLENGE_SEED_LENGTH		32

#define WAI_UNICAST_PER_KEY_LENGTH		16
#define WAI_UEK_UCK_LENGTH				(WAI_UNICAST_PER_KEY_LENGTH * 2)
#define WAI_MEK_MCK_LENGTH				(WAI_UNICAST_PER_KEY_LENGTH * 2)		// derived from NMK
#define WAI_UEK_UCK_MAK_KEK_LENGTH		(WAI_UNICAST_PER_KEY_LENGTH * 4)

#define WAI_MESSAGE_DIGEST_LENGTH		20

#define WAI_UEK(keys)					&(keys)[0]								// Unicast Encryption Key
#define WAI_UCK(keys)					&(keys)[WAI_UNICAST_PER_KEY_LENGTH]		// Unicast Integrity check Key
#define WAI_MAK(keys)					&(keys)[2 * WAI_UNICAST_PER_KEY_LENGTH]	// Message Authentication Key
#define WAI_KEK(keys)					&(keys)[3 * WAI_UNICAST_PER_KEY_LENGTH]	// Key Encryption Key
#define WAI_CHALLENGE_SEED(keys)		&(keys)[4 * WAI_UNICAST_PER_KEY_LENGTH] // next AE challenge seed

#define WAI_DATA_PACKET_NUMBER_LENGTH			16
#define WAI_KEY_ANNOUNCEMENT_IDENTIFIER_LENGTH	16
#define WAI_GROUPKEY_ANNOUNCEMENT_DATA_LENGTH	16

#define SHA256_DIGEST_SIZE	SHA256_MAC_LEN

#define WAI_HDR_FLAG_MORE_FRAG					0x1
#define WAPI_MAX_IE_LENGTH						128

#define WAI_TYPE_WAI_AUTH						1
#define WAI_AUTH_FLAG_USK_UPDATE				0x10

#define WAI_UNICAST_KEY_NEGOTIATE_REQUEST		8
#define WAI_UNICAST_KEY_NEGOTIATE_RESPONSE		9
#define WAI_UNICAST_KEY_NEGOTIATE_CONFIRMATION	10
#define WAI_GROUPCAST_KEY_ANNOUNCEMENT			11
#define WAI_GROUPCAST_KEY_ANNOUNCEMENT_RESPONSE	12

#define WAPI_STA_TX_RETRY_TIME					5*WLAN_TIME_UNIT 	/* second */
#define WAPI_KEY_MAX_REPLAY						4	

#define WAPI_COUNTER_LENGTH						16
#define WAPI_GTK_MAX_LEN						32

struct wai_hdr
{
	u16 version;
	u8 type;
	u8 subtype;
	u16 reserved;
	u16 length;
	u16 packet_sequence_num;
	u8 fragment_sequence_num;
	u8 flag;
} __attribute__ ((packed));

struct wai_unikey_req
{
	u8 flag;
	u8 bkid[WAI_BKID_LENGTH];
	u8 uskid;
	u8 addid[WAI_ADDID_LENGTH];
	u8 ae_challenge[WAI_AE_CHALLENGE_LENGTH];
} __attribute__ ((packed));

struct wai_unikey_resp
{
	u8 flag;
	u8 bkid[WAI_BKID_LENGTH];
	u8 uskid;
	u8 addid[WAI_ADDID_LENGTH];
	u8 asue_challenge[WAI_ASUE_CHALLENGE_LENGTH];
	u8 ae_challenge[WAI_AE_CHALLENGE_LENGTH];
} __attribute__ ((packed));

struct wai_unikey_confirm
{
	u8 flag;
	u8 bkid[WAI_BKID_LENGTH];
	u8 uskid;
	u8 addid[WAI_ADDID_LENGTH];
	u8 asue_challenge[WAI_ASUE_CHALLENGE_LENGTH];
} __attribute__ ((packed));

struct wai_groupkey_announce
{
	u8 flag;
	u8 mskid;
	u8 uskid;
	u8 addid[WAI_ADDID_LENGTH];
	u8 data_packet_number[WAI_DATA_PACKET_NUMBER_LENGTH];
	u8 key_announce_identifier[WAI_KEY_ANNOUNCEMENT_IDENTIFIER_LENGTH];
	u8 key_length;
	u8 key_data[WAI_GROUPKEY_ANNOUNCEMENT_DATA_LENGTH];
	u8 mic[WAI_MESSAGE_DIGEST_LENGTH];
} __attribute__ ((packed));

struct wai_groupkey_response
{
	u8 flag;
	u8 mskid;
	u8 uskid;
	u8 addid[WAI_ADDID_LENGTH];
	u8 key_announce_identifier[WAI_KEY_ANNOUNCEMENT_IDENTIFIER_LENGTH];
	u8 mic[WAI_MESSAGE_DIGEST_LENGTH];
} __attribute__ ((packed));

struct wapi_ctx {
	u32 flags;
	u8 state;
	u8 key_mgt;
	u8 pairwise;
	u8 timeout_counter;
	u8 bk[WAI_BK_LENGTH];
	u8 bkid[WAI_BK_LENGTH];
	u8 uskid;
	u8 addid[WAI_ADDID_LENGTH];
	u32 wai_authenticated;
	u8 ae_challenge[WAI_AE_CHALLENGE_LENGTH];
	u8 next_ae_challenge[WAI_AE_CHALLENGE_LENGTH];
	u8 asue_challenge[WAI_AE_CHALLENGE_LENGTH];
	u8 unicast_keys_and_seeds[WAI_UEK_UCK_MAK_KEK_LENGTH + WAI_CHALLENGE_SEED_LENGTH];
	u16 wai_auth_tx_seq;
	u16 wai_auth_rx_seq;
};

typedef union
{
	struct
	{
		u8 rawdata[WAPI_COUNTER_LENGTH];
	} data;
	struct
	{
		u64 hi_64bits;
		u64 lo_64bits;
	} counter_field;
} wapi_be_counter;

struct wapi_group_key
{
	u32 flags;
	u8 key_idx;
	u8 cipher;
	u8 state;
	u8 key_done_sta;
	u8 gtk[2][WAPI_GTK_MAX_LEN];
	wapi_be_counter wai_gkey_counter;
	wapi_be_counter wai_seq_counter;
	u8 mskid;
	u8 resv[3];
} __attribute__ ((packed, aligned(0x4)));

int wai_psk_derive_bk(u8 *pass_pharse, int pass_pharse_length, u8 *bk);
int wai_derive_addid(u8 *mac_ae, u8 *mac_asue, u8 *addid);
int wai_derive_bkid(u8 *bk, u8 *mac_ae, u8 *mac_asue, u8 *bkid);
int wai_derive_unicast_keys_and_challenge_seed(u8 *bk, u8 *addid, u8* ae_challenge, u8 *asue_challenge, u8 *unicast_keys_and_seeds);
int wai_derive_multicast_keys(u8 *nmk, u8 *multicast_keys);
void SMS4_OFB(u8 *key, u8 *iv, u8 *text, int length, u8 *output);

#endif
