//*** include ***
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <sys/param.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "mqtt_client.h"

#include "esp_bt.h"
#include "esp_bt_device.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"

#include <esp_http_server.h>
#include "esp_ota_ops.h"
#include "esp_sntp.h"
#include "esp_timer.h"


//*** define ***
//*** common ***
#define AP_TAG "R4S"
//????
#define MQTT_CMD_TOPIC "/cmd"
#define MQTT_RSP_TOPIC "/rsp"
#define MQTT_STAT_TOPIC "/stat"

//*** wifi ***
#define WIFI_MAXIMUM_RETRY  10
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

//*** bt ***
// REMOTE_SERVICE_UUID "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define REMOTE_SERVICE_UUID {0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x01, 0x00, 0x40, 0x6e}
// REMOTE_TXCHAR_UUID "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
#define REMOTE_TXCHAR_UUID {0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x02, 0x00, 0x40, 0x6e}
// REMOTE_RXCHAR_UUID "6e400003-b5a3-f393-e0a9-e50e24dcca9e"
#define REMOTE_RXCHAR_UUID {0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x03, 0x00, 0x40, 0x6e}

#define PROFILE_NUM      2
#define PROFILE_A_APP_ID 0
#define PROFILE_B_APP_ID 1
#define INVALID_HANDLE   0
#define BLE_INPUT_BUFFSIZE 64
#define cStatus_len 128
#define otabufsize 2048

static bool btconnect  = false;
static bool btauthorize = false;
static bool get_server = false;
static esp_gattc_char_elem_t *char_elem_result   = NULL;
static esp_gattc_descr_elem_t *descr_elem_result = NULL;

uint8_t notifyData[BLE_INPUT_BUFFSIZE];
int8_t notifyDataLen = -1;
uint8_t sendData[BLE_INPUT_BUFFSIZE];
int8_t sendDataLen = 0;
esp_mqtt_client_handle_t mqttclient; 


//*** data ***
char MQTT_BASE_TOPIC[8];                    // r4s
char FND_NAME[16];                          // last founded device name
char REQ_NAME[16];                          // requested device name
char DEV_NAME[16];                          // connected device name
char MQTT_USER[16];                         // MQTT Server user
char MQTT_PASSWORD[16];                     // MQTT Server password
char MQTT_SERVER[16];                       // MQTT Server
char WIFI_SSID[33];                         // network SSID for ESP8266 to connect to
char WIFI_PASSWORD[65];                     // password for the network above

char tBLEAddr[16];                          //last ble text address
uint32_t NumConn = 0;                       //number of ble connection
uint8_t r4scounter = 0;                     //r4s message counter
uint8_t r4sConnErr = 0;
uint8_t R4SNUM = 0;                         //r4s number
//uint8_t r4ststat = 0;                     //timer?
uint8_t r4slpcom = 0;
uint8_t r4slppar = 0;
uint8_t r4slpres = 0;
int64_t t_last_us = 0;


//static uint8_t r4sAuth[8] = { 0xb6, 0x2c, 0x27, 0xb3, 0xb8, 0xac, 0x5a, 0xef };

//                       boilOrLight    scale_from rand  rgb1       scale_mid  rand   rgb_mid     scale_to   rand  rgb2
uint8_t nl_settings[] = {1,             0,         38,   0, 0, 0,   50,        38,    0, 0, 0,    100,       38,   0, 0, 0};

bool mqttConnected =false;

char  cprevStatus[cStatus_len];
char  cStatus[cStatus_len];
uint8_t bState = 0;
uint8_t bCtemp = 0;
uint8_t bHtemp = 0;
int   iRssi = 0;
uint8_t bprevState = 0;
uint8_t bprevCtemp = 0;
uint8_t bprevHtemp = 0;
int   iprevRssi = 0;
uint8_t DEV_CHK1 = 0; //for G200S, G211S, G240S
uint8_t DEV_CHK2 = 0; //for G211S, G240S 
uint8_t  TimeZone = 0;
// NightLight colours
uint8_t RgbR = 255;
uint8_t RgbG = 255;
uint8_t RgbB = 255;

//tft
bool tft_conn = 0;


char cssDatasheet[] = ""     // CSS
                        ""  // some CSS from ESP Easy Mega
                        "<style>* {font-family: sans-serif; font-size: 12pt; margin: 0px; padding: 0px; box-sizing: border-box; }"
                        "h1 {font-size: 16pt; color: #07D; margin: 8px 0; font-weight: bold; }"
                        "h2 {font-size: 11pt; margin: 0px; padding: 0px; font-weight: normal;}"
                        "h3 {font-size: 12pt; margin: 16px 0px 0px 0px; padding: 4px; background-color: #EEE; color: #444; font-weight: bold; }"
                        "input, select, textarea {margin: 4px; padding: 4px 8px; border-radius: 4px; background-color: #eee; border-style: solid; border-width: 1px; border-color: gray;}"
                        "table.normal th {padding: 6px; background-color: #444; color: #FFF; border-color: #888; font-weight: bold; }table.normal "
                        "td {padding: 4px; height: 30px;}table.normal "
                        "tr {padding: 4px; }table.normal {color: #000; width: 100%; min-width: 420px; border-collapse: collapse; } "
                        "td {background-color: #F8F8F8; }.note {color: #444; font-style: italic; }.headermenu "
                        "{position: fixed; top: 0; left: 0; right: 0; height: 90px; padding: 8px 12px; background-color: #F8F8F8; border-bottom: 1px solid #DDD; z-index: 1;}.apheader "
                        "{padding: 8px 12px; background-color: #F8F8F8;}.bodymenu {margin-top: 96px;}.menubar "
                        "{position: inherit; top: 55px; }.menu "
                        "{float: left; padding: 4px 16px 8px 16px; color: #444; white-space: nowrap; border: solid transparent; border-width: 4px 1px 1px; border-radius: 4px 4px 0 0; text-decoration: none; }.menu.active "
                        "{color: #000; background-color: #FFF; border-color: #07D #DDD #FFF; }.menu:hover "
                        "{color: #000; background: #DEF; }.menu_button "
                        "section{overflow-x: auto; width: 100%; }@media screen and (max-width: 960px) {span.showmenulabel { display: none; }.menu { max-width: 11vw; max-width: 48px; }"
                        "</style>";
//update form
const char* serverIndex = "<form method='POST' action='/updating' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
