#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/reboot.h>
#include <pthread.h>
#include <netmgr.h>
#include <wifi.h>
#include <signal.h>
#include <breeze_export.h>
#include <breeze_awss_export.h>
#include "infra_compat.h"

#include "ota_service.h"


#define PRODUCT_KEY_LEN            (20)
#define DEVICE_NAME_LEN            (32)
#define DEVICE_SECRET_LEN          (64)
#define PRODUCT_SECRET_LEN         (64)

extern int awss_success_notify();
extern void awss_report_reset(void);
extern int linkkit_main(int argc, char **argv);
extern int HAL_SetProductKey(char *product_key);
extern int HAL_SetProductSecret(char *product_secret);
extern int HAL_SetDeviceName(char *device_name);
extern int HAL_SetDeviceSecret(char *device_secret);

static void notify_wifi_status(void *arg);
static void* combo_work(void *arg);
static void combo_apinfo_ready_handler(breeze_apinfo_t *info);

static int             linkkit_started = 0;
static int             awss_running    = 0;
static breeze_apinfo_t apinfo;
static bool awss_success = false;
static uint8_t connect_type;

enum {
    CONNECT_TYPE_WIFI,
    CONNECT_TYPE_COMBO,
    CONNECT_TYPE_ETH,
};

static void _awss_reset(int sig)
{
    static int cnt = 0;
    cnt++;
    printf("%s (the %dth)\n", __func__, cnt);
    awss_report_reset();
    usleep(1 * 1000 * 1000);
    if (connect_type == CONNECT_TYPE_WIFI || connect_type == CONNECT_TYPE_COMBO) {
        netmgr_clear_ap_config();
        system("ifconfig wlan0 down"); // TODO
    }
    exit(1);
}

static int mqtt_connected_event_handler(void)
{
    printf("MQTT Construct  OTA start");
    char product_key[PRODUCT_KEY_LEN + 1] = {0};
    char device_name[DEVICE_NAME_LEN + 1] = {0};
    char device_secret[DEVICE_SECRET_LEN + 1] = {0};
    HAL_GetProductKey(product_key);
    HAL_GetDeviceName(device_name);
    HAL_GetDeviceSecret(device_secret);
    static ota_service_t ctx = {0};
    memset(&ctx, 0, sizeof(ota_service_t));
    strncpy(ctx.pk, product_key, sizeof(ctx.pk)-1);
    strncpy(ctx.dn, device_name, sizeof(ctx.dn)-1);
    strncpy(ctx.ds, device_secret, sizeof(ctx.ds)-1);
    ctx.trans_protcol = 0;
    ctx.dl_protcol = 3;
    ota_service_init(&ctx);
    return 0;
}

static void * linkkit_helper(void *arg)
{
    signal(SIGUSR1, _awss_reset);
    IOT_RegisterCallback(ITE_MQTT_CONNECT_SUCC,mqtt_connected_event_handler);
#if 1
    linkkit_main(1, NULL);
#else
    system("linkkit-example-solo &");
#endif
}

static void* awss_notify_work(void *arg)
{
    (void)arg;
    awss_success_notify();
    awss_success = true;
}

static void apinfo_ready_handler(breeze_apinfo_t *ap)
{
    if (!ap)
        return;

    memcpy(&apinfo, ap, sizeof(apinfo));
    combo_apinfo_ready_handler(&apinfo);
}

static void notify_wifi_status(void *arg)
{
    /* tlv response */
    uint8_t rsp[] = { 0x01, 0x01, 0x01 };

    /* tx_cmd is defaulted to ALI_CMD_STATUS so we don't worry here. */
    breeze_post(rsp, sizeof(rsp));
}

void got_ip_work(void)
{
    int ret;
    pthread_t id1, id2;

    //iotx_event_regist_cb(linkkit_event_monitor);

    if (connect_type == CONNECT_TYPE_COMBO) {
        notify_wifi_status(NULL);
    }

    if (/*awss_running &&*/ !awss_success) {
        ret = pthread_create(&id1, NULL, awss_notify_work, NULL);
        if (ret != 0) {
            printf("Failed to crreate awss notify process.\r\n");
        } else {
            awss_success = true;
        }
    }

    if (!linkkit_started) {
        ret = pthread_create(&id2, NULL, linkkit_helper, NULL);
        if (ret != 0) {
            printf("Failed to create linkkit process.\r\n");
        } else {
            linkkit_started = 1;
        }
    }
}

static void combo_apinfo_ready_handler(breeze_apinfo_t *info)
{
        int ret;
        pthread_t tid;

        awss_running = 1;

        ret = pthread_create(&tid, NULL, combo_work, (void *)info);
        if (ret != 0) {
            printf("Failed to crreate combo thread.\r\n");
        }
}

static void* combo_work(void *arg)
{
    netmgr_ap_config_t config;
    breeze_apinfo_t *info = arg;

    if (!info)
        return NULL;

#if 1
    printf("%s %d, ssid: %s, pw: %s\r\n", __func__, __LINE__, info->ssid,
           info->pw);
#endif

    strncpy(config.ssid, info->ssid, sizeof(config.ssid) - 1);
    strncpy(config.pwd, info->pw, sizeof(config.pwd) - 1);
    memcpy(config.bssid, info->bssid, ETH_ALEN);
    netmgr_set_ap_config(&config);
    hal_wifi_suspend_station(NULL);
    printf("Will reconnect wifi: %s\r\n", config.ssid);
    netmgr_reconnect_wifi();
}

static int combo_init(uint8_t use_static, netmgr_ap_config_t *ap)
{
    netmgr_register_got_ip_handler(got_ip_work);
    netmgr_init();

    if (use_static) {
        netmgr_set_ap_config(ap);
    }

    /* Do not start BLE if wifi AP available */
    if (netmgr_start(false) == 1) {
        return 1;
    }

    return 0;
}

static void set_lk_dev_info(breeze_dev_info_t *info)
{
    HAL_SetProductKey(info->product_key);
    HAL_SetProductSecret(info->product_secret);
    HAL_SetDeviceName(info->device_name);
    HAL_SetDeviceSecret(info->device_secret);
}

#if 1
static void dump_dev_info(breeze_dev_info_t *info)
{
    printf("Device info:\r\n");
    printf("  pi: %d\r\n", info->product_id);
    printf("  pk: %s\r\n", info->product_key);
    printf("  ps: %s\r\n", info->product_secret);
    printf("  dn: %s\r\n", info->device_name);
    printf("  ds: %s\r\n", info->device_secret);
}
#endif

#ifdef DEV_INFO_ENC
#include <triple.h>
static void fetch_enc_dev_info(breeze_dev_info_t *dinfo)
{
    char pi_str[PRODUCT_ID_MAXLEN] = {0};

    get_product_key(dinfo->product_key);
    get_product_secret(dinfo->product_secret);
    get_device_name(dinfo->device_name);
    get_device_secret(dinfo->device_secret);
    get_product_id(pi_str);
    dinfo->product_id = (uint32_t)atoi(pi_str);
}
#else
#include "devinfo.h"
static void fetch_dev_info(breeze_dev_info_t *dinfo)
{
    strncpy(dinfo->product_key, PRODUCT_KEY, strlen(PRODUCT_KEY));
    strncpy(dinfo->product_secret, PRODUCT_SECRET, strlen(PRODUCT_SECRET));
    strncpy(dinfo->device_name, DEVICE_NAME, strlen(DEVICE_NAME));
    strncpy(dinfo->device_secret, DEVICE_SECRET, strlen(DEVICE_SECRET));
    dinfo->product_id = (uint32_t)PRODUCT_ID;
}
#endif

int main(int argc, char* argv[])
{
    char pk[20+1] = {0}, ps[64+1] = {0}, dn[32+1] = {0}, ds[64+1] = {0};
    breeze_dev_info_t dinfo = {
        .product_key = pk,
        .product_secret = ps,
        .device_name = dn,
        .device_secret = ds
    };
    netmgr_ap_config_t config;
    uint8_t use_static_ap = 0;

#ifdef DEV_INFO_ENC
    fetch_enc_dev_info(&dinfo);
#else
    fetch_dev_info(&dinfo);
#endif

    //dump_dev_info(&dinfo);
    set_lk_dev_info(&dinfo);

    if (argc == 3) {
        printf("You have specified the AP information (ssid: %s, pw: %s), "
               "let's use it.\n", argv[1], argv[2]);
        strncpy(config.ssid, argv[1], sizeof(config.ssid) - 1);
        strncpy(config.pwd, argv[2], sizeof(config.pwd) - 1);
        memset(config.bssid, 0, ETH_ALEN);
        use_static_ap = 1;
    }

    /* Default connect type is wifi/ble combo */
    connect_type = CONNECT_TYPE_COMBO;
    if (combo_init(use_static_ap, &config) == 0) {
        breeze_awss_init(apinfo_ready_handler, &dinfo);
    }

    while (1) sleep(1);

    return 0;
}
