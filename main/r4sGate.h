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
#include <esp_http_client.h>
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
//redmond
// REMOTE_SERVICE_UUID "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define REMOTE_SERVICE_UUID {0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x01, 0x00, 0x40, 0x6e}
// REMOTE_TXCHAR_UUID "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
#define REMOTE_TXCHAR_UUID {0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x02, 0x00, 0x40, 0x6e}
// REMOTE_RXCHAR_UUID "6e400003-b5a3-f393-e0a9-e50e24dcca9e"
#define REMOTE_RXCHAR_UUID {0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x03, 0x00, 0x40, 0x6e}
//xiaomi
//01344736-0000-1000-8000-262837236156
#define XREMOTE_SERVICE_UUID {0x56, 0x61, 0x23, 0x37, 0x28, 0x26, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x36, 0x47, 0x34, 0x01}
//0000fe95-0000-1000-8000-00805f9b34fb
#define XREMOTE_SERVICE1_UUID {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x95, 0xfe, 0x00, 0x00}
#define XREMOTE_SERVICE116_UUID 0xfe95
//0000180a-0000-1000-8000-00805f9b34fb
#define XREMOTE_SERVICE2_UUID {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x0a, 0x18, 0x00, 0x00}
#define XREMOTE_SERVICE216_UUID 0x180a
//0000aa02-0000-1000-8000-00805f9b34fb
#define XREMOTE_STATUS_UUID  {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x02, 0xaa, 0x00, 0x00}
//00000010-0000-1000-8000-00805f9b34fb
#define XREMOTE_AUTHINIT_UUID  {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00}
//00000001-0000-1000-8000-00805f9b34fb
#define XREMOTE_AUTH_UUID  {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00}
//00000004-0000-1000-8000-00805f9b34fb
#define XREMOTE_VER_UUID  {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00}
//0000aa01-0000-1000-8000-00805f9b34fb
#define XREMOTE_SETUP_UUID  {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x01, 0xaa, 0x00, 0x00}
//0000aa04-0000-1000-8000-00805f9b34fb
#define XREMOTE_TIME_UUID  {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x04, 0xaa, 0x00, 0x00}
//0000aa05-0000-1000-8000-00805f9b34fb
#define XREMOTE_BOIL_UUID  {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x05, 0xaa, 0x00, 0x00}
//00002a28-0000-1000-8000-00805f9b34fb
#define XREMOTE_MCUVER_UUID  {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x28, 0x2a, 0x00, 0x00}
int MiAKettleID = 275;
int MiBKettleID = 275;
int MiCKettleID = 275;

#define PROFILE_NUM      3
#define PROFILE_A_APP_ID 0
#define PROFILE_B_APP_ID 1
#define PROFILE_C_APP_ID 2
#define INVALID_HANDLE   0
#define BLE_INPUT_BUFFSIZE 64
#define cStatus_len 128
#define otabufsize 2048

static bool btconnecta  = false;
static bool btconnectb  = false;
static bool btconnectc  = false;
static bool btopena  = false;
static bool btopenb  = false;
static bool btopenc  = false;
static bool btauthoriza = false;
static bool btauthorizb = false;
static bool btauthorizc = false;
static bool get_servera = false;
static bool get_serverb = false;
static bool get_serverc = false;

static bool Isconnecting    = false;
static bool Isscanning      = false;
static bool stop_scan_done  = false;



static esp_gattc_char_elem_t *char_elem_result_a   = NULL;
static esp_gattc_descr_elem_t *descr_elem_result_a = NULL;
static esp_gattc_char_elem_t *char_elem_result_b   = NULL;
static esp_gattc_descr_elem_t *descr_elem_result_b = NULL;
static esp_gattc_char_elem_t *char_elem_result_c   = NULL;
static esp_gattc_descr_elem_t *descr_elem_result_c = NULL;
static esp_ble_gap_cb_param_t scan_rsta;
static esp_ble_gap_cb_param_t scan_rstb;
static esp_ble_gap_cb_param_t scan_rstc;

uint8_t notifyDataA[BLE_INPUT_BUFFSIZE];
uint8_t notifyDataB[BLE_INPUT_BUFFSIZE];
uint8_t notifyDataC[BLE_INPUT_BUFFSIZE];
int8_t notifyDataALen = -1;
int8_t notifyDataBLen = -1;
int8_t notifyDataCLen = -1;
uint8_t readDataA[BLE_INPUT_BUFFSIZE];
uint8_t readDataB[BLE_INPUT_BUFFSIZE];
uint8_t readDataC[BLE_INPUT_BUFFSIZE];
int8_t readDataALen = -1;
int8_t readDataBLen = -1;
int8_t readDataCLen = -1;
int8_t readDataHandleA = -1;
int8_t readDataHandleB = -1;
int8_t readDataHandleC = -1;
uint8_t sendDataA[BLE_INPUT_BUFFSIZE];
uint8_t sendDataB[BLE_INPUT_BUFFSIZE];
uint8_t sendDataC[BLE_INPUT_BUFFSIZE];
int8_t sendDataALen = 0;
int8_t sendDataBLen = 0;
int8_t sendDataCLen = 0;
int8_t sendDataHandleA = 0;
int8_t sendDataHandleB = 0;
int8_t sendDataHandleC = 0;

uint16_t mqtt_port  = 1883;
uint16_t jpg_time  = 32;

esp_mqtt_client_handle_t mqttclient; 

//*** data ***
char strOFF[8];
char strON[8];

char MQTT_BASE_TOPIC[8];                    // r4s
char FND_NAME[16];                           // last founded device name
char REQ_NAMEA[16];                          // requested device name
char REQ_NAMEB[16];                          // requested device name
char REQ_NAMEC[16];                          // requested device name
char DEV_NAMEA[16];                          // connected device name
char DEV_NAMEB[16];                          // connected device name
char DEV_NAMEC[16];                          // connected device name
char MQTT_USER[16];                         // MQTT Server user
char MQTT_PASSWORD[16];                     // MQTT Server password
char MQTT_SERVER[16];                       // MQTT Server
char WIFI_SSID[33];                         // network SSID for ESP32 to connect to
char WIFI_PASSWORD[65];                     // password for the network above

char MQTT_TOPP1[33];
char MQTT_TOPP2[33];
char MQTT_TOPP3[33];
char MQTT_TOPP4[33];
char MQTT_TOPP5[33];
char MQTT_TOPP6[33];
char MQTT_VALP1[16];
char MQTT_VALP2[16];
char MQTT_VALP3[16];
char MQTT_VALP4[16];
char MQTT_VALP5[16];
char MQTT_VALP6[16];

#define MyJPGbuflen 32768

char MyHttpUri[65];
char MyJPGbuf[MyJPGbuflen];

int32_t MyJPGbufidx = 0;
char tESP32Addr[16];                        //esp mac
char tBLEAddrA[16];                         //last ble text address
char tBLEAddrB[16];                         //last ble text address
char tBLEAddrC[16];                         //last ble text address
uint32_t NumConnA = 0;                      //number of ble connection
uint32_t NumConnB = 0;                      //number of ble connection
uint32_t NumConnC = 0;                      //number of ble connection
uint8_t r4sAcounter = 0;                     //r4s message counter
uint8_t r4sBcounter = 0;                     //r4s message counter
uint8_t r4sCcounter = 0;                     //r4s message counter
uint8_t r4sAConnErr = 0;
uint8_t r4sBConnErr = 0;
uint8_t r4sCConnErr = 0;

uint8_t xbtautha = 0;
uint8_t xbtauthb = 0;
uint8_t xbtauthc = 0;
uint8_t xshedcoma = 0;
uint8_t xshedcomb = 0;
uint8_t xshedcomc = 0;


uint8_t R4SNUM = 0;                         //r4s number
//uint8_t r4ststat = 0;                     //timer?
uint8_t r4slpcoma = 0;
uint8_t r4slpcomb = 0;
uint8_t r4slpcomc = 0;
uint8_t r4sppcoms = 0;
uint8_t r4sppcoma = 0;
uint8_t r4sppcomb = 0;
uint8_t r4sppcomc = 0;
uint8_t r4slppar1a = 0;
uint8_t r4slppar2a = 0;
uint8_t r4slppar3a = 0;
uint8_t r4slppar4a = 0;
uint8_t r4slppar5a = 0;
uint8_t r4slppar6a = 0;
uint8_t r4slppar1b = 0;
uint8_t r4slppar2b = 0;
uint8_t r4slppar3b = 0;
uint8_t r4slppar4b = 0;
uint8_t r4slppar5b = 0;
uint8_t r4slppar6b = 0;
uint8_t r4slppar1c = 0;
uint8_t r4slppar2c = 0;
uint8_t r4slppar3c = 0;
uint8_t r4slppar4c = 0;
uint8_t r4slppar5c = 0;
uint8_t r4slppar6c = 0;
uint8_t r4slpresa = 0;
uint8_t r4slpresb = 0;
uint8_t r4slpresc = 0;
int64_t t_ppcoms_us = 0;
int64_t t_ppcoma_us = 0;
int64_t t_ppcomb_us = 0;
int64_t t_ppcomc_us = 0;
int64_t t_ppcona_us = 0;
int64_t t_ppconb_us = 0;
int64_t t_ppconc_us = 0;
int64_t t_lasta_us = 0;
int64_t t_lastb_us = 0;
int64_t t_lastc_us = 0;
int64_t t_clock_us = 0;
int64_t t_jpg_us = 0;
int64_t t_tinc_us = 0;



uint8_t t_SyncA  = 0;
uint8_t t_SyncB  = 0;
uint8_t t_SyncC  = 0;

//static uint8_t r4sAuth[8] = { 0xb6, 0x2c, 0x27, 0xb3, 0xb8, 0xac, 0x5a, 0xef };

//                       boilOrLight    scale_from rand  rgb1       scale_mid  rand   rgb_mid     scale_to   rand  rgb2
uint8_t nl_settings[] = {1,             0,         94,   0, 0, 0,   50,        94,    0, 0, 0,    100,       94,   0, 0, 0};

bool mqttConnected =false;

char  cprevStatusA[cStatus_len];
char  cprevStatusB[cStatus_len];
char  cprevStatusC[cStatus_len];
char  cStatusA[cStatus_len];
char  cStatusB[cStatus_len];
char  cStatusC[cStatus_len];

int   iRssiA = 0;
int   iRssiB = 0;
int   iRssiC = 0;
int   iRssiESP = 0;
int   iprevRssiA = 0;
int   iprevRssiB = 0;
int   iprevRssiC = 0;
int   iprevRssiESP = 0;

uint8_t bProgA = 0;
uint8_t bProgB = 0;
uint8_t bProgC = 0;

uint8_t bHeatA = 0;
uint8_t bHeatB = 0;
uint8_t bHeatC = 0;

uint8_t bModProgA = 0;
uint8_t bModProgB = 0;
uint8_t bModProgC = 0;

uint8_t bPHourA = 0;
uint8_t bPHourB = 0;
uint8_t bPHourC = 0;

uint8_t bPMinA = 0;
uint8_t bPMinB = 0;
uint8_t bPMinC = 0;

uint8_t bCHourA = 0;
uint8_t bCHourB = 0;
uint8_t bCHourC = 0;

uint8_t bCMinA = 0;
uint8_t bCMinB = 0;
uint8_t bCMinC = 0;

uint8_t bDHourA = 0;
uint8_t bDHourB = 0;
uint8_t bDHourC = 0;

uint8_t bDMinA = 0;
uint8_t bDMinB = 0;
uint8_t bDMinC = 0;

uint8_t bStateA = 0;
uint8_t bStateB = 0;
uint8_t bStateC = 0;
uint8_t bStateS = 0;

uint8_t bStNlA = 0;
uint8_t bStNlB = 0;
uint8_t bStNlC = 0;

uint8_t bCtempA = 0;
uint8_t bCtempB = 0;
uint8_t bCtempC = 0;

uint8_t bHtempA = 0;
uint8_t bHtempB = 0;
uint8_t bHtempC = 0;

uint8_t bAwarmA = 0;
uint8_t bAwarmB = 0;
uint8_t bAwarmC = 0;



uint8_t bprevProgA = 0;
uint8_t bprevProgB = 0;
uint8_t bprevProgC = 0;

uint8_t bprevModProgA = 0;
uint8_t bprevModProgB = 0;
uint8_t bprevModProgC = 0;

uint8_t bprevPHourA = 0;
uint8_t bprevPHourB = 0;
uint8_t bprevPHourC = 0;

uint8_t bprevPMinA = 0;
uint8_t bprevPMinB = 0;
uint8_t bprevPMinC = 0;

uint8_t bprevCHourA = 0;
uint8_t bprevCHourB = 0;
uint8_t bprevCHourC = 0;

uint8_t bprevCMinA = 0;
uint8_t bprevCMinB = 0;
uint8_t bprevCMinC = 0;

uint8_t bprevDHourA = 0;
uint8_t bprevDHourB = 0;
uint8_t bprevDHourC = 0;

uint8_t bprevDMinA = 0;
uint8_t bprevDMinB = 0;
uint8_t bprevDMinC = 0;

uint8_t bprevStateA = 0;
uint8_t bprevStateB = 0;
uint8_t bprevStateC = 0;
uint8_t bprevStateS = 0;

uint8_t bprevStNlA = 0;
uint8_t bprevStNlB = 0;
uint8_t bprevStNlC = 0;

uint8_t bprevCtempA = 0;
uint8_t bprevCtempB = 0;
uint8_t bprevCtempC = 0;

uint8_t bprevHtempA = 0;
uint8_t bprevHtempB = 0;
uint8_t bprevHtempC = 0;

uint8_t bprevAwarmA = 0;
uint8_t bprevAwarmB = 0;
uint8_t bprevAwarmC = 0;

uint8_t DEV_TYPA = 0; 
uint8_t DEV_TYPB = 0; 
uint8_t DEV_TYPC = 0; 

uint8_t FDHass = 0; 
uint8_t fcommtp = 0; 
uint8_t ftrufal = 0; 
uint8_t tft_flip = 0; 

//uint8_t DEV_CHK1 = 0; //for G200S, G211S, G240S
//uint8_t DEV_CHK2 = 0; //for G211S, G240S 

uint8_t  TimeZone = 0;
// NightLight colours
uint8_t RgbRA = 255;
uint8_t RgbGA = 255;
uint8_t RgbBA = 255;
uint8_t PRgbRA = 255;
uint8_t PRgbGA = 255;
uint8_t PRgbBA = 255;

uint8_t RgbRB = 255;
uint8_t RgbGB = 255;
uint8_t RgbBB = 255;
uint8_t PRgbRB = 255;
uint8_t PRgbGB = 255;
uint8_t PRgbBB = 255;

uint8_t RgbRC = 255;
uint8_t RgbGC = 255;
uint8_t RgbBC = 255;
uint8_t PRgbRC = 255;
uint8_t PRgbGC = 255;
uint8_t PRgbBC = 255;

bool f_update = false;
//tft
bool tft_conn = 0;


char cssDatasheet[] = ""     // CSS
                        ""  // some CSS from ESP Easy Mega
                        "<style>* {font-family: sans-serif; font-size: 12pt; margin: 0px; padding: 0px; box-sizing: border-box; }"
                        "h1 {font-size: 16pt; color: #07D; margin: 8px 0; font-weight: bold; }"
                        "h2 {font-size: 11pt; margin: 0px; padding: 0px; font-weight: normal;}"
                        "h3 {font-size: 12pt; margin: 16px 0px 0px 0px; padding: 4px; background-color: #EEE; color: #444; font-weight: bold; }"
                        "input, select, textarea {margin: 4px; padding: 4px 8px; border-radius: 4px; background-color: #eee; border-style: solid; border-width: 1px; border-color: gray;}"
			"input:hover {background-color: #ccc; }input.wide {max-width: 500px; width:80%; }input.widenumber {max-width: 500px; width:100px; }#selectwidth {max-width: 500px; width:80%; padding: 4px 8px;}select:hover {background-color: #ccc; }.container {display: block; padding-left: 35px; margin-left: 4px; margin-top: 0px; position: relative; cursor: pointer; font-size: 12pt; -webkit-user-select: none; -moz-user-select: none; -ms-user-select: none; user-select: none; }.container input {position: absolute; opacity: 0; cursor: pointer;  }.checkmark {position: absolute; top: 0; left: 0; height: 25px;  width: 25px;  background-color: #eee; border-style: solid; border-width: 1px; border-color: gray;  border-radius: 4px;}.container:hover input ~ .checkmark {background-color: #ccc; }.container input:checked ~ .checkmark { background-color: #07D; }.checkmark:after {content: ''; position: absolute; display: none; }.container input:checked ~ .checkmark:after {display: block; }.container .checkmark:after {left: 7px; top: 3px; width: 5px; height: 10px; border: solid white; border-width: 0 3px 3px 0; -webkit-transform: rotate(45deg); -ms-transform: rotate(45deg); transform: rotate(45deg); }.container2 {display: block; padding-left: 35px; margin-left: 9px; margin-bottom: 20px; position: relative; cursor: pointer; font-size: 12pt; -webkit-user-select: none; -moz-user-select: none; -ms-user-select: none; user-select: none; }.container2 input {position: absolute; opacity: 0; cursor: pointer;  }.dotmark {position: absolute; top: 0; left: 0; height: 26px;  width: 26px;  background-color: #eee; border-style: solid; border-width: 1px; border-color: gray; border-radius: 50%;}.container2:hover input ~ .dotmark {background-color: #ccc; }.container2 input:checked ~ .dotmark { background-color: #07D;}.dotmark:after {content: ''; position: absolute; display: none; }.container2 input:checked ~ .dotmark:after {display: block; }.container2 .dotmark:after {top: 8px; left: 8px; width: 8px; height: 8px; border-radius: 50%; background: white; }#toastmessage {visibility: hidden; min-width: 250px; margin-left: -125px; background-color: #07D;color: #fff;  text-align: center;  border-radius: 4px;  padding: 16px;  position: fixed;z-index: 1; left: 282px; bottom: 30%;  font-size: 17px;  border-style: solid; border-width: 1px; border-color: gray;}#toastmessage.show {visibility: visible; -webkit-animation: fadein 0.5s, fadeout 0.5s 2.5s; animation: fadein 0.5s, fadeout 0.5s 2.5s; }@-webkit-keyframes fadein {from {bottom: 20%; opacity: 0;} to {bottom: 30%; opacity: 0.9;} }@keyframes fadein {from {bottom: 20%; opacity: 0;} to {bottom: 30%; opacity: 0.9;} }@-webkit-keyframes fadeout {from {bottom: 30%; opacity: 0.9;} to {bottom: 0; opacity: 0;} }@keyframes fadeout {from {bottom: 30%; opacity: 0.9;} to {bottom: 0; opacity: 0;} }.level_0 { color: #F1F1F1; }.level_1 { color: #FCFF95; }.level_2 { color: #9DCEFE; }.level_3 { color: #A4FC79; }.level_4 { color: #F2AB39; }.level_9 { color: #FF5500; }.logviewer {  color: #F1F1F1; background-color: #272727;  font-family: 'Lucida Console', Monaco, monospace;  height:  530px; max-width: 1000px; width: 80%; padding: 4px 8px;  overflow: auto;   border-style: solid; border-color: gray; }textarea {max-width: 1000px; width:80%; padding: 4px 8px; font-family: 'Lucida Console', Monaco, monospace; }textarea:hover {background-color: #ccc; } "
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
