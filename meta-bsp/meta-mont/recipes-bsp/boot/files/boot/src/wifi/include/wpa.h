/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file wpa.h
*   \brief  WPA definition.
*   \author Montage
*/


#ifndef _WPA_H_
#define _WPA_H_

#include <wla_def.h>

#define GMK_LEN				32


#define WPA_GTK_MAX_LEN		32

#define SHA1_MAC_LEN		20
#define MD5_MAC_LEN			16
	
#define EAPOL_KEY_MAX_REPLAY		4	

#define MAX_KDE_LEN			256		

/* Define: the flags for the pmksa operation */
#define PMKSA_VALID					BIT(0)
#define PMKSA_TIMEOUT_DISCONNECT	BIT(1)	/* session timeout action (disconnect) set by Radius Server */
#define PMKSA_TIMEOUT_REAUTH		BIT(2)	/* session timeout action (re-auth) set by Radius Server */
#define PMKSA_TIMEOUT_CHECK			BIT(3)	/* period check the sta is alive or not (if alive, refresh experiation timer) */
#define PMKSA_IN_BSS_LIST			BIT(4)	/* pmksa is added into bss's pmksa list */

#define PMKSA_ENTRY_NUM		8
 
struct wpa_ptk {
	u8 kck[16]; /* EAPOL-Key Key Confirmation Key (KCK) */
	u8 kek[16]; /* EAPOL-Key Key Encryption Key (KEK) */
	u8 tk1[16]; /* Temporal Key 1 (TK1) */
	union {
		u8 tk2[16]; /* Temporal Key 2 (TK2) */
		struct {
			u8 tx_mic_key[8];
			u8 rx_mic_key[8];
		} auth;
	} u;
} __attribute__ ((packed));

struct pmksa_cache_entry {
	u8 pmkid[PMKID_LEN];
	u8 pmk[PMK_LEN];
	u8 pmk_len;
	u8 flags;
	/* FIXME: host driver handles the 8021x, then the below items are not needed? */
	u8 sta_addr[WLAN_ADDR_LEN];
	u32 expiration;
};

struct wpa_group_key {
	u32 flags;
	u8 key_idx;
	u8 cipher;
	u8 state;
	u8 key_done_sta;
	u8 gtk[2][WPA_GTK_MAX_LEN];
	u8 gmk[GMK_LEN];
	u8 gnonce[WPA_NONCE_LEN];
};

struct wpa_ctx {
	u32	flags;
	u8	state;
	u8	key_mgt;
	u8	pairwise;
	u8	timeout_counter;
	struct wpa_ptk ptk;
	u8	*pmk;
	u8	anonce[WPA_NONCE_LEN];
	u8	snonce[WPA_NONCE_LEN];
	u8	key_replay_counter[WPA_REPLAY_COUNTER_LEN];
	u8	req_replay_counter[WPA_REPLAY_COUNTER_LEN];
};

#define CTX_WPA_VALID			BIT(0)
#define CTX_WAPI_VALID			BIT(1)
#define DESC_TYPE_RSN			BIT(2)
#define WPA_PTK_VALID			BIT(3)
#define WPA_GTK_VALID			BIT(4)
#define WPA_GTK_UPDATING		BIT(5)
#define WPA_REPLAY_COUNTER		BIT(6)
#define WPA_PTK_NEGOTIATING		BIT(7)

#define RSN_KDE_GTK					OUI_SUITETYPE(0x00, 0x0f, 0xac, 1)
#define RSN_KDE_MAC					OUI_SUITETYPE(0x00, 0x0f, 0xac, 3)
#define RSN_KDE_PMDID				OUI_SUITETYPE(0x00, 0x0f, 0xac, 4)
#define RSN_KDE_SMK					OUI_SUITETYPE(0x00, 0x0f, 0xac, 5)
#define RSN_KDE_NONCE				OUI_SUITETYPE(0x00, 0x0f, 0xac, 6)
#define RSN_KDE_LIFTTIME			OUI_SUITETYPE(0x00, 0x0f, 0xac, 7)
#define RSN_KDE_ERROR				OUI_SUITETYPE(0x00, 0x0f, 0xac, 8)


/* authenticator state machines */
enum {
	AS_INITIALIZE = 1,
	AS_DISCONNECT,
	AS_DISCONNECTED,
	AS_AUTHENTICATION,
	AS_AUTHENTICATION2,
	AS_INITPMK,
	AS_INITPSK,
	AS_PTKSTART,
	AS_PTKCALCNEGOTIATING,
	AS_PTKCALCNEGOTIATING2,
	AS_PTKINITNEGOTIATING,
	AS_PTKINITDONE,
	AS_GTK_IDLE = 13,
	AS_GTK_REKEYNEGOTIATING,
	AS_GTK_REKEYESTABLISHED,
	AS_GTK_ERROR,
	AS_STAKEY_START,
};

enum {
	AS_GTK_INIT = 1,
	AS_GTK_SETKEYSDONE,
	AS_GTK_SETKEYS,
};

#define WPA_STA_TX_RETRY_TIME		(3*WLAN_TIME_UNIT) 	/* second */
#define WPA_GROUP_REKEY_TIME		(600*WLAN_TIME_UNIT)

char key_mgt_type(u8 val);

void pbkdf2(const char *passphrase, const char *ssid, u32 ssid_len, int iterations, u8 *buf, u32 buflen);
void sha1_prf(const u8 *key, u32 key_len, const char *label, const u8 *data, u32 data_len, u8 *buf, u32 buf_len);

#endif
