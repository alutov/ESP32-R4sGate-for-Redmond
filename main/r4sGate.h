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
#include "driver/timer.h"
#include "driver/ledc.h"
#include "mbedtls/aes.h"
#include "esp_tls.h"
#include "esp_crt_bundle.h"

//*** define ***
//*** common ***
#define AP_TAG "R4S"

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
// update

//00001800-0000-1000-8000-00805f9b34fb
//#define XREMOTE_SERVICE3_UUID {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00}
//#define XREMOTE_SERVICE316_UUID 0x1800
//00002a00-0000-1000-8000-00805f9b34fb
//#define XREMOTE_UPDATE_UUID  {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x14, 0x000, 0x00, 0x00}


//0000fee8-0000-1000-8000-00805f9b34fb
#define XREMOTE_SERVICE3_UUID {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xe8, 0xfe, 0x00, 0x00}
#define XREMOTE_SERVICE316_UUID 0xfee8
//013784cf-f7e3-55b4-6c4c-9fd140100a16
#define XREMOTE_UPDATE_UUID  {0x16, 0x0a, 0x10, 0x40, 0xd1, 0x9f, 0x4c, 0x6c, 0xb4, 0x55, 0xe3, 0xf7, 0xcf, 0x84, 0x37, 0x01}

//galcon
// GLREMOTE_SERVICE_UUID  "e8680100-9c4b-11e4-b5f7-0002a5d5c51b"
#define GLREMOTE_SERVICE_UUID  {0x1b, 0xc5, 0xd5, 0xa5, 0x02, 0x00, 0xf7, 0xb5, 0xe4, 0x11, 0x4b, 0x9c, 0x00, 0x01, 0x68, 0xe8}
// GLREMOTE_SERVICE1_UUID "e8680200-9c4b-11e4-b5f7-0002a5d5c51b"
#define GLREMOTE_SERVICE1_UUID {0x1b, 0xc5, 0xd5, 0xa5, 0x02, 0x00, 0xf7, 0xb5, 0xe4, 0x11, 0x4b, 0x9c, 0x00, 0x02, 0x68, 0xe8}
// GLREMOTE_SERVICE2_UUID "e8680400-9c4b-11e4-b5f7-0002a5d5c51b"
#define GLREMOTE_SERVICE2_UUID {0x1b, 0xc5, 0xd5, 0xa5, 0x02, 0x00, 0xf7, 0xb5, 0xe4, 0x11, 0x4b, 0x9c, 0x00, 0x04, 0x68, 0xe8}
// GLREMOTE_SETUP_UUID     "e8680401-9c4b-11e4-b5f7-0002a5d5c51b"
#define GLREMOTE_SETUP_UUID    {0x1b, 0xc5, 0xd5, 0xa5, 0x02, 0x00, 0xf7, 0xb5, 0xe4, 0x11, 0x4b, 0x9c, 0x01, 0x04, 0x68, 0xe8}
// GLREMOTE_AUTH_UUID     "e8680201-9c4b-11e4-b5f7-0002a5d5c51b"
#define GLREMOTE_AUTH_UUID     {0x1b, 0xc5, 0xd5, 0xa5, 0x02, 0x00, 0xf7, 0xb5, 0xe4, 0x11, 0x4b, 0x9c, 0x01, 0x02, 0x68, 0xe8}
// GLREMOTE_TIME_UUID     "e8680203-9c4b-11e4-b5f7-0002a5d5c51b"
#define GLREMOTE_TIME_UUID     {0x1b, 0xc5, 0xd5, 0xa5, 0x02, 0x00, 0xf7, 0xb5, 0xe4, 0x11, 0x4b, 0x9c, 0x03, 0x02, 0x68, 0xe8}
// GLREMOTE_TXCHAR_UUID   "e8680103-9c4b-11e4-b5f7-0002a5d5c51b"
#define GLREMOTE_TXCHAR_UUID   {0x1b, 0xc5, 0xd5, 0xa5, 0x02, 0x00, 0xf7, 0xb5, 0xe4, 0x11, 0x4b, 0x9c, 0x03, 0x01, 0x68, 0xe8}
// GLREMOTE_RXCHAR_UUID   "e8680102-9c4b-11e4-b5f7-0002a5d5c51b"
#define GLREMOTE_RXCHAR_UUID   {0x1b, 0xc5, 0xd5, 0xa5, 0x02, 0x00, 0xf7, 0xb5, 0xe4, 0x11, 0x4b, 0x9c, 0x02, 0x01, 0x68, 0xe8}

#define PROFILE_NUM      5
#define PROFILE_A_APP_ID 0
#define PROFILE_B_APP_ID 1
#define PROFILE_C_APP_ID 2
#define PROFILE_D_APP_ID 3
#define PROFILE_E_APP_ID 4
#define INVALID_HANDLE   0
#define BLE_INPUT_BUFFSIZE 64
#define cStatus_len 128
#define otabufsize 2048
#define BleMonNum 24
#define BleMonDefTO 3000


//uint8_t DEV_TYPA = 0; 
//uint8_t DEV_TYPB = 0; 
//uint8_t DEV_TYPC = 0; 


struct BleDevSt {
int      MiKettleID;
bool     btopenreq;
bool     btopen;
bool     btauthoriz;
bool     get_server;
uint8_t  notifyData[BLE_INPUT_BUFFSIZE];
int8_t   notifyDataLen;
uint8_t  readData[BLE_INPUT_BUFFSIZE];
int8_t   readDataLen;
int8_t   readDataHandle;
uint8_t  sendData[BLE_INPUT_BUFFSIZE];
int8_t   sendDataLen;
int8_t   sendDataHandle;
uint8_t  DEV_TYP; 
char     REQ_NAME[16];
char     RQC_NAME[16];
char     DEV_NAME[16];
char     tBLEAddr[16];
uint32_t NumConn;
uint32_t PassKey;
uint8_t  r4scounter;
uint8_t  r4sConnErr;
uint8_t  r4sAuthCount;
uint8_t  xbtauth;
uint8_t  xshedcom;
uint8_t  r4slpcom;
uint8_t  r4sppcom;
uint8_t  r4slppar1;
uint8_t  r4slppar2;
uint8_t  r4slppar3;
uint8_t  r4slppar4;
uint8_t  r4slppar5;
uint8_t  r4slppar6;
uint8_t  r4slppar7;
uint8_t  r4slppar8;
uint8_t  r4slpres;
uint8_t  t_ppcon;
uint8_t  t_rspdel;
uint8_t  t_rspcnt;
uint8_t  f_Sync;
char     cprevStatus[cStatus_len];
char     cStatus[cStatus_len];
int      iRssi;
int      iprevRssi;
uint32_t bSEnergy;
uint32_t bprevSEnergy;
uint32_t bSTime;
uint32_t bprevSTime;
uint32_t bSCount;
uint32_t bprevSCount;
uint32_t bSHum;
uint32_t bprevSHum;
uint8_t  bLock;
uint8_t  bProg;
uint8_t  bModProg;
uint8_t  bHeat;
uint8_t  bPHour;
uint8_t  bPMin;
uint8_t  bCHour;
uint8_t  bCMin;
uint8_t  bDHour;
uint8_t  bDMin;
uint8_t  bState;
uint8_t  bStNl;
uint8_t  bStBl;
uint8_t  bStBp;
uint8_t  bCtemp;
uint8_t  bHtemp;
uint8_t  bLtemp;
uint8_t  bAwarm;
int8_t   bBlTime;
uint8_t  RgbR;
uint8_t  RgbG;
uint8_t  RgbB;
uint8_t  bprevLock;
uint8_t  bprevProg;
uint8_t  bprevModProg;
uint8_t  bprevHeat;
uint8_t  bprevPHour;
uint8_t  bprevPMin;
uint8_t  bprevCHour;
uint8_t  bprevCMin;
uint8_t  bprevDHour;
uint8_t  bprevDMin;
uint8_t  bprevState;
uint8_t  bprevStNl;
uint8_t  bprevStBl;
uint8_t  bprevStBp;
uint8_t  bprevCtemp;
uint8_t  bprevHtemp;
uint8_t  bprevAwarm;
int8_t   bprevBlTime;
uint8_t  PRgbR;
uint8_t  PRgbG;
uint8_t  PRgbB;

uint32_t bS1Energy;
uint8_t  bC1temp;
uint8_t  bCStemp;
uint8_t  bCVol;
uint8_t  bCVoll;
uint8_t  bprevCVol;
uint8_t  bprevCVoll;
uint8_t  bEfficiency;

};
struct BleMonRec {
uint16_t sto;
uint8_t  mac[20];
uint8_t  id;
uint8_t  res;
};

struct BleMonExt {
uint16_t ttick;
uint16_t mto;
char     name[16];
uint8_t  advdat[32];
uint8_t  scrsp[32];
uint8_t  advdatlen;
uint8_t  scrsplen;
uint8_t  state;
uint8_t  prstate;
int8_t   rssi;
int8_t   prrssi;
int16_t  par1;
int16_t  par2;
int16_t  par3;
int16_t  par4;
int16_t  par5;
int16_t  par6;
int16_t  ppar1;
int16_t  ppar2;
int16_t  ppar3;
int16_t  ppar4;
int16_t  ppar5;
int16_t  ppar6;
int8_t   cmrssi;
uint8_t  gtnum;
uint8_t  gttmo;
};



static bool Isscanning = false;
static bool IsPassiveScan = false;
static bool StartStopScanReq = false;
static uint8_t hwtdiv = 0;

static esp_gattc_char_elem_t *char_elem_result   = NULL;
static esp_gattc_descr_elem_t *descr_elem_result = NULL;


static esp_ble_gap_cb_param_t scan_rsta;
static esp_ble_gap_cb_param_t scan_rstb;
static esp_ble_gap_cb_param_t scan_rstc;
static esp_ble_gap_cb_param_t scan_rstd;
static esp_ble_gap_cb_param_t scan_rste;
int FND_RSSI = 0;
uint32_t NumWfConn;
uint32_t NumMqConn;
uint32_t MemErr;

uint16_t mqtt_port  = 1883;

esp_mqtt_client_handle_t mqttclient; 

//*** data ***
char strOFF[8];
char strON[8];
uint8_t binblemac [8];
uint8_t binwfmac [8];
char MQTT_BASE_TOPIC[8];                    // r4s
char FND_NAME[16];                           // last founded device name
char FND_ADDR[16];                           // last founded device addr
char FND_ADDRx[32];                           // last founded device addr incl :
char MQTT_USER[16];                         // MQTT Server user
char MQTT_PASSWORD[20];                     // MQTT Server password
char MQTT_SERVER[33];                       // MQTT Server
char WIFI_SSID[33];                         // network SSID for ESP32 to connect to
char WIFI_PASSWORD[65];                     // password for the network above

#ifdef USE_TFT
//tft
#define MyJPGbuflen 32768
uint16_t t_ppcons = 0;
uint16_t jpg_time  = 32;
uint8_t tft_conn = 0;
uint8_t tft_conf = 0;
uint8_t tft_flip = 0; 
uint8_t  MyHttpMqtt = 0;
int32_t MyJPGbufidx = 0;
char MyHttpUri[260];
char *MyJPGbuf;

char MQTT_TOPP1[33];
char MQTT_TOPP2[25];
char MQTT_TOPP3[25];
char MQTT_TOPP4[33];
char MQTT_TOPP5[33];
char MQTT_TOPP6[33];
char MQTT_TOPP7[33];
char MQTT_VALP1[16];
char MQTT_VALP2[16];
char MQTT_VALP3[16];
char MQTT_VALP4[16];
char MQTT_VALP5[16];
char MQTT_VALP6[16];
char MQTT_VALP7[16];
#endif




char tESP32Addr[16];                        //esp mac
char tESP32Addr1[32];                       //esp mac

char AUTH_BASIC[51];

uint8_t R4SNUM = 0;                         //r4s number
uint8_t R4SNUMO = 0;                        //r4s number
uint8_t r4sppcoms = 0;
uint16_t bStateS = 0;
uint16_t bprevStateS = 0;

uint8_t t_lasts = 0;
uint8_t  t_clock = 0;
uint16_t t_jpg = 0;
uint8_t  t_tinc = 0;

uint8_t foffln  = 0;
uint8_t mqtdel  = 0;
uint8_t macauth  = 0;
uint8_t volperc  = 0;
int floop = 0;

//                       boilOrLight    scale_from rand  rgb1       scale_mid  rand   rgb_mid     scale_to   rand  rgb2
uint8_t nl_settings[] = {1,             0,         94,   0, 0, 0,   50,        94,    0, 0, 0,    100,       94,   0, 0, 0};

bool mqttConnected =false;

int   iRssiESP = 0;
int   iprevRssiESP = 0;





uint8_t FDHass = 0; 
uint8_t fcommtp = 0; 
uint8_t ftrufal = 0; 
uint8_t ble_mon = 0; 
uint8_t ble_mon_refr = 0; 

uint8_t  TimeZone = 0;
// NightLight colours

uint8_t bgpio1 = 0;
uint8_t bgpio2 = 0;
uint8_t bgpio3 = 0;
uint8_t bgpio4 = 0;
uint8_t bgpio5 = 0;

uint8_t lvgpio1 = 0;
uint8_t lvgpio2 = 0;
uint8_t lvgpio3 = 0;
uint8_t lvgpio4 = 0;
uint8_t lvgpio5 = 0;
uint8_t fgpio1 = 0;
uint8_t fgpio2 = 0;
uint8_t fgpio3 = 0;
uint8_t fgpio4 = 0;
uint8_t fgpio5 = 0;
int  cntgpio1 = 0;
int  cntgpio2 = 0;
int  cntgpio3 = 0;
int  cntgpio4 = 0;
int  cntgpio5 = 0;



bool f_update = false;


const char cssDatasheet[] = ""     // CSS
                        ""  // some CSS from ESP Easy Mega
                        "<style>* {font-family: sans-serif; font-size: 12pt; margin: 0px; padding: 0px; box-sizing: border-box; }"
                        "h1 {font-size: 16pt; color: #07D; margin: 8px 0; font-weight: bold; }"
                        "h2 {font-size: 11pt; margin: 0px; padding: 0px; font-weight: normal;}"
                        "h3 {font-size: 12pt; margin: 16px 0px 0px 0px; padding: 4px; background-color: #EEE; color: #444; font-weight: bold; }"
                        "input, select, textarea {margin: 4px; padding: 4px 8px; border-radius: 4px; background-color: #eee; border-style: solid; border-width: 1px; border-color: gray;}"
			"input:hover {background-color: #ccc; }input.wide {max-width: 500px; width:80%; }input.widenumber {max-width: 500px; width:100px; }#selectwidth {max-width: 500px; width:80%; padding: 4px 8px;}select:hover {background-color: #ccc; }.container {display: block; padding-left: 35px; margin-left: 4px; margin-top: 0px; position: relative; cursor: pointer; font-size: 12pt; -webkit-user-select: none; -moz-user-select: none; -ms-user-select: none; user-select: none; }.container input {position: absolute; opacity: 0; cursor: pointer;  }.checkmark {position: absolute; top: 0; left: 0; height: 25px;  width: 25px;  background-color: #eee; border-style: solid; border-width: 1px; border-color: gray;  border-radius: 4px;}.container:hover input ~ .checkmark {background-color: #ccc; }.container input:checked ~ .checkmark { background-color: #07D; }.checkmark:after {content: ''; position: absolute; display: none; }.container input:checked ~ .checkmark:after {display: block; }.container .checkmark:after {left: 7px; top: 3px; width: 5px; height: 10px; border: solid white; border-width: 0 3px 3px 0; -webkit-transform: rotate(45deg); -ms-transform: rotate(45deg); transform: rotate(45deg); }.container2 {display: block; padding-left: 35px; margin-left: 9px; margin-bottom: 20px; position: relative; cursor: pointer; font-size: 12pt; -webkit-user-select: none; -moz-user-select: none; -ms-user-select: none; user-select: none; }.container2 input {position: absolute; opacity: 0; cursor: pointer;  }.dotmark {position: absolute; top: 0; left: 0; height: 26px;  width: 26px;  background-color: #eee; border-style: solid; border-width: 1px; border-color: gray; border-radius: 50%;}.container2:hover input ~ .dotmark {background-color: #ccc; }.container2 input:checked ~ .dotmark { background-color: #07D;}.dotmark:after {content: ''; position: absolute; display: none; }.container2 input:checked ~ .dotmark:after {display: block; }.container2 .dotmark:after {top: 8px; left: 8px; width: 8px; height: 8px; border-radius: 50%; background: white; }#toastmessage {visibility: hidden; min-width: 250px; margin-left: -125px; background-color: #07D;color: #fff;  text-align: center;  border-radius: 4px;  padding: 16px;  position: fixed;z-index: 1; left: 282px; bottom: 30%;  font-size: 17px;  border-style: solid; border-width: 1px; border-color: gray;}#toastmessage.show {visibility: visible; -webkit-animation: fadein 0.5s, fadeout 0.5s 2.5s; animation: fadein 0.5s, fadeout 0.5s 2.5s; }@-webkit-keyframes fadein {from {bottom: 20%; opacity: 0;} to {bottom: 30%; opacity: 0.9;} }@keyframes fadein {from {bottom: 20%; opacity: 0;} to {bottom: 30%; opacity: 0.9;} }@-webkit-keyframes fadeout {from {bottom: 30%; opacity: 0.9;} to {bottom: 0; opacity: 0;} }@keyframes fadeout {from {bottom: 30%; opacity: 0.9;} to {bottom: 0; opacity: 0;} }.level_0 { color: #F1F1F1; }.level_1 { color: #FCFF95; }.level_2 { color: #9DCEFE; }.level_3 { color: #A4FC79; }.level_4 { color: #F2AB39; }.level_9 { color: #FF5500; }.logviewer {  color: #F1F1F1; background-color: #272727;  font-family: 'Lucida Console', Monaco, monospace;  height:  530px; max-width: 1000px; width: 80%; padding: 4px 8px;  overflow: auto;   border-style: solid; border-color: gray; }textarea {max-width: 1000px; width:80%; padding: 4px 8px; font-family: 'Lucida Console', Monaco, monospace; }textarea:hover {background-color: #ccc; } "
//                        "table.normal th {padding: 6px; background-color: #444; color: #FFF; border-color: #888; font-weight: bold; }table.normal "
                        "table.normal th {padding: 6px; background-color: #EEE; color: #444; border-color: #888; font-weight: bold; }table.normal "
                        "td {padding: 4px; height: 30px;}table.normal "
                        "tr {padding: 4px; }table.normal {color: #000; width: 100%; min-width: 420px; border-collapse: collapse; } "
                        "td {background-color: #F8F8F8; }td.xbg {background-color: #F4F4F4; }.note {color: #444; font-style: italic; }.headermenu "
                        "{position: fixed; top: 0; left: 0; right: 0; height: 90px; padding: 8px 12px; background-color: #F8F8F8; border-bottom: 1px solid #DDD; z-index: 1;}.apheader "
                        "{padding: 8px 12px; background-color: #F8F8F8;}.bodymenu {margin-top: 96px;}.menubar "
                        "{position: inherit; top: 55px; }.menu "
                        "{float: left; padding: 4px 16px 8px 16px; color: #444; white-space: nowrap; border: solid transparent; border-width: 4px 1px 1px; border-radius: 4px 4px 0 0; text-decoration: none; }.menu.active "
                        "{color: #000; background-color: #FFF; border-color: #07D #DDD #FFF; }.menu:hover "
                        "{color: #000; background: #DEF; }.menu_button "
                        "section{overflow-x: auto; width: 100%; }@media screen and (max-width: 960px) {span.showmenulabel { display: none; }.menu { max-width: 11vw; max-width: 48px; }"
//                        ".blbox {display: inline-block;} "
			"</style>";
//update form
const char* serverIndex = "<form method='POST' action='/updating' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
