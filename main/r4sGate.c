/* 
****************************************************************
	ESP32 r4sGate for Redmond+ main
	Lutov Andrey  Donetsk
Use for compilation ESP-IDF Programming Guide:
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/
****************************************************************
*/
#define AP_VER "2021.11.14"

// If use ili9341 320*240 tft
#define USE_TFT

#include "r4sGate.h"


//**************** my common proc *******************
//compare uuid 128 bit
int bt_compare_UUID128(uint8_t uuid1[ESP_UUID_LEN_128],
		uint8_t uuid2[ESP_UUID_LEN_128]) {
	for (int i = 0; i < ESP_UUID_LEN_128; i++) {
		if (uuid1[i] != uuid2[i]) return 0;
	}
	return 1;
}
// int32 to hh/mm/ss
void itoat (uint32_t val, char* cout, size_t len)
{
	char buff[16];
	uint32_t seconds = val;
	uint32_t minutes = seconds / 60;
	uint32_t hours = minutes / 60;
	seconds = seconds % 60;
	minutes = minutes % 60;
       	itoa(hours,buff,10);
	strcpy (cout,buff);
	strcat (cout,":");
	if (minutes < 10) strcat (cout,"0");
       	itoa(minutes,buff,10);
	strcat (cout,buff);
	strcat (cout,":");
	if (seconds < 10) strcat (cout,"0");
       	itoa(seconds,buff,10);
	strcat (cout,buff);
}
// int32 to kwh
void itoakw (uint32_t val, char* cout, size_t len)
{
	char buff[16];
	uint32_t watt = val;
	uint32_t kwatt = watt / 1000;
	watt = watt % 1000;
       	itoa(kwatt,buff,10);
	strcpy (cout,buff);
	strcat (cout,".");
	if (watt < 100) strcat (cout,"0");
	if (watt < 10) strcat (cout,"0");
       	itoa(watt,buff,10);
	strcat (cout,buff);
}

// bin byte to hex char string
void bin2hex(const unsigned char *bin, char *out, size_t len)
{
	size_t  i;
	if (bin == NULL || out == NULL || len == 0) return;
	for (i=0; i<len; i++) {
		out[i*2]   = "0123456789abcdef"[bin[i] >> 4];
		out[i*2+1] = "0123456789abcdef"[bin[i] & 0x0F];
	}
	out[len*2] = '\0';
}

// parse uri par=value string like this:
// swfid=wifiname&swfpsw=wifipassword&smqsrv=192.168.1.10&smqid=esp&
// smqpsw=esp&devnam=&rlight=255&glight=255&blight=255&chk2=2
// find par *key in string *cin(isize) & copy value to *out buffer(osize) 
void parsuri(char *cin, char *cout, char *ckey, int isize, int osize)
{
	int i = 0;
	int j = 0;
	char found = 0;
	char a;
	char b;
	char c;
	while ((i < isize) && (!found)) {
	a = cin[i];
	b = ckey[j];
	i++;
	j++;
	if (a != b) {
	if ((a == 0x3d) && (b == 0)) {                 //0x3d-> =
	found = 1;
	cout[0] = 0;
	}
	if (a < 0x20) i = isize;
	j = 0;
	}
	}	
	j = 0;
	while ((i < isize) && (j < osize) && (found)) {
	a = cin[i];
	if ((a > 0x1f) && (a != 0x26)) {       //0x26-> &
	if (a == 0x25) {
	i++;
	a = cin[i];
	b =  (a <= '9') ? (a - '0') : (a - 'A' + 10);
	b = (b & 0xf)*16;
	i++;
	a = cin[i];
	c =  (a <= '9') ? (a - '0') : (a - 'A' + 10);
	c = (c & 0xf) + b;
	cout[j] = c;
	} else if (a == 0x2b) cout[j] = 0x20;
	else cout[j] = cin[i];
	i++; j++;
	cout[j] = 0;
	} else found = 0;
	}
}

//return offset after founded string 0 if not found
int parsoff(char *cin, char *ckey, int isize)
{
	int i = 0;
	int j = 0;
	char found = 0;
	char a;
	char b;
	while ((i < isize) && (!found)) {
	a = cin[i];
	b = ckey[j];
	i++;
	j++;
	if (a != b) {
	if (b == 0) {
	found = 1;
	i--;
	}
	j = 0;
	}
	}	
	if (!found) i = 0;
	return i;
}

//my string copy include dest buf limit
void mystrcpy(char *cout, char *cin, int osize)
{
	int i = 0;
	char a;
	while (i < osize) {
	a = cin[i];
	if  (a > 0x1f) {
	cout[i] = a;
	i++;
	cout[i] = 0;
	} else {
	i = osize;
	}		
	}
}


// size control case insensitive strings compare 
bool incascmp(char *cain, char *cbin, int size)
{
	int i = 0;
	char ca;
	char cb;
	bool result = 0;
	while ((i < size) && (!result)) {
	ca = cain[i];
	cb = cbin[i];
	i++;
	if (ca >= 'A' && ca <= 'Z')
	ca = 'a' + (ca - 'A');
	if (cb >= 'A' && cb <= 'Z')
	cb = 'a' + (cb - 'A');
	if ((ca==0) || (ca != cb)) result = 1;
	}
	if (cain[i] != 0) result = 1;
	return result;
}

// size control case sensitive strings compare 
bool inccmp(char *cain, char *cbin, int size)
{
	int i = 0;
	char ca;
	char cb;
	bool result = 0;
	while ((i < size) && (!result)) {
	ca = cain[i];
	cb = cbin[i];
	i++;
	if ((ca==0) || (ca != cb)) result = 1;
	}
	if (cain[i] != 0) result = 1;
	return result;
}

// store uptime string in *cout buffer
void uptime_string_exp(char *cout)
{
	char buff[16];
	int64_t t_before_us = esp_timer_get_time();
	int minutes = t_before_us * 0.001 / 60000;
	int days = minutes / 1440;
	minutes = minutes % 1440;
	int hrs = minutes / 60;
	minutes = minutes % 60;
       	itoa(days,buff,10);
	strcpy (cout,buff);
	strcat (cout," days ");
       	itoa(hrs,buff,10);
	strcat (cout,buff);
	strcat (cout," hours ");
       	itoa(minutes,buff,10);
	strcat (cout,buff);
	strcat (cout," minutes");
}
//xiaomi
void mixA(uint8_t  *cin, uint8_t  *cout, int prid)
{
	if (cin == NULL || cout == NULL) return;
	cout[0] = cin[5];
	cout[1] = cin[3];
	cout[2] = cin[0];
	cout[3] = prid & 0xff;
	cout[4] = prid & 0xff;
	cout[5] = cin[1];
	cout[6] = cin[0];
	cout[7] = cin[4];
}
void mixB(uint8_t  *cin, uint8_t  *cout, int prid)
{
	if (cin == NULL || cout == NULL) return;
	cout[0] = cin[5];
	cout[1] = cin[3];
	cout[2] = cin[0];
	cout[3] = (prid  >> 8) & 0xff;
	cout[4] = cin[1];
	cout[5] = cin[5];
	cout[6] = cin[0];
	cout[7] = prid & 0xff;
}
void cipherInit(uint8_t  *cin, uint8_t  *ctab, int keysize)
{
	if (cin == NULL || ctab == NULL || keysize == 0) return;
	int i;
	int j = 0;
	char a;
	char b;
	for (i = 0; i < 256; i++) ctab[i] = i;
	for (i = 0; i < 256; i++) {
	j += ctab[i] + cin[i%keysize];	
	j = j & 0xff;
	a = ctab[i];
	b = ctab[j];
	ctab[j] = a;
	ctab[i] = b;
	}
}
void cipherCrypt(uint8_t  *cin, uint8_t  *cout, uint8_t  *ctab, int size)
{
	if (cin == NULL || cout == NULL || cout == NULL || size == 0) return;
	int i;
	int idx;
	int idx1 = 0;
	int idx2 = 0;
	char a;
	char b;
	for (i = 0; i < size; i++) {
	idx1++;
	idx1 = idx1 & 0xff;
	idx2 += ctab[idx1];
	idx2 = idx2 & 0xff;
	a = ctab[idx1];
	b = ctab[idx2];	
	ctab[idx2] = a;
	ctab[idx1] = b;
	idx = ctab[idx1] + ctab[idx2];
	idx = idx & 0xff;
	cout[i] = cin[i] ^ ctab[idx];
	}
}

#ifdef USE_TFT
#include "tft/tft.c"
#endif

static intr_handle_t s_timer_handle;
//******************* timer *********************
static void hw_timer_callback(void *arg)
{
//Reset irq and set for next time
	TIMERG0.int_clr_timers.t0 = 1;
	TIMERG0.hw_timer[0].config.alarm_en = 1;
//
	switch (bgpio1 & 0xc0) {
	case 128:
	if ((gpio_get_level(bgpio1 & 0x3f)) != lvgpio1) {
        cntgpio1++;
	if (cntgpio1 > 5) {
	fgpio1 = 1;
	lvgpio1 = lvgpio1 ^ 1;
	cntgpio1 = 0;
	}
	} else cntgpio1 = 0;
	break;
	case 192:
	if ((gpio_get_level(bgpio1 & 0x3f)) != lvgpio1) {
        cntgpio1++;
	if (cntgpio1 > 5) {
	fgpio1 = 1;
	lvgpio1 = lvgpio1 ^ 1;
	t_lasts_us = ~t_lasts_us;
	cntgpio1 = 0;
	if (!lvgpio1 && btauthoriza) {
	if ((DEV_TYPA > 0) && (DEV_TYPA < 10)) { 
	if (!bStateA) {
	r4slppar1a = 0;
	r4slpcoma = 2;
	} else {
	r4slppar1a = 0;
	r4slpcoma = 1;
	}
	} else if ((DEV_TYPA > 9) && (DEV_TYPA < 16)) { 
	if (!bStateA) {
	r4slppar1a = 0;
	r4slpcoma = 6;
	} else {
	r4slppar1a = 0;
	r4slpcoma = 1;
	}
	} else if ((DEV_TYPA > 15) && (DEV_TYPA < 64)) { 
	if (!bStateA) {
	r4slppar1a = 1;
	r4slpcoma = 10;
	} else {
	r4slppar1a = 0;
	r4slpcoma = 10;
	}
	} else if ((DEV_TYPA > 63) && (DEV_TYPA < 70)) { 
	if (!bStateA) {
	r4slppar1a = 0;
	r4slpcoma = 65;
	} else {
	r4slppar1a = 0;
	r4slpcoma = 64;
	}
	}

	}
	}
	} else cntgpio1 = 0;
	break;
	}

	switch (bgpio2 & 0xc0) {
	case 128:
	if ((gpio_get_level(bgpio2 & 0x3f)) != lvgpio2) {
        cntgpio2++;
	if (cntgpio2 > 5) {
	fgpio2 = 1;
	lvgpio2 = lvgpio2 ^ 1;
	cntgpio2 = 0;
	}
	} else cntgpio2 = 0;
	break;
	case 192:
	if ((gpio_get_level(bgpio2 & 0x3f)) != lvgpio2) {
        cntgpio2++;
	if (cntgpio2 > 5) {
	fgpio2 = 1;
	lvgpio2 = lvgpio2 ^ 1;
	t_lasts_us = ~t_lasts_us;
	cntgpio2 = 0;
	if (!lvgpio2 && btauthorizb) {
	if ((DEV_TYPB > 0) && (DEV_TYPB < 10)) { 
	if (!bStateB) {
	r4slppar1b = 0;
	r4slpcomb = 2;
	} else {
	r4slppar1b = 0;
	r4slpcomb = 1;
	}
	} else if ((DEV_TYPB > 9) && (DEV_TYPB < 16)) { 
	if (!bStateB) {
	r4slppar1b = 0;
	r4slpcomb = 6;
	} else {
	r4slppar1b = 0;
	r4slpcomb = 1;
	}
	} else if ((DEV_TYPB > 15) && (DEV_TYPB < 64)) { 
	if (!bStateB) {
	r4slppar1b = 1;
	r4slpcomb = 10;
	} else {
	r4slppar1b = 0;
	r4slpcomb = 10;
	}
	} else if ((DEV_TYPB > 63) && (DEV_TYPB < 70)) { 
	if (!bStateB) {
	r4slppar1b = 0;
	r4slpcomb = 65;
	} else {
	r4slppar1b = 0;
	r4slpcomb = 64;
	}
	}

	}
	}
	} else cntgpio2 = 0;
	break;

	}
	switch (bgpio3 & 0xc0) {
	case 128:
	if ((gpio_get_level(bgpio3 & 0x3f)) != lvgpio3) {
        cntgpio3++;
	if (cntgpio3 > 5) {
	fgpio3 = 1;
	lvgpio3 = lvgpio3 ^ 1;
	t_lasts_us = ~t_lasts_us;
	cntgpio3 = 0;
	}
	} else cntgpio3 = 0;
	break;
	case 192:
	if ((gpio_get_level(bgpio3 & 0x3f)) != lvgpio3) {
        cntgpio3++;
	if (cntgpio3 > 5) {
	fgpio3 = 1;
	lvgpio3 = lvgpio3 ^ 1;
	t_lasts_us = ~t_lasts_us;
	cntgpio3 = 0;
	if (!lvgpio3 && btauthorizc) {
	if ((DEV_TYPC > 0) && (DEV_TYPC < 10)) { 
	if (!bStateC) {
	r4slppar1c = 0;
	r4slpcomc = 2;
	} else {
	r4slppar1c = 0;
	r4slpcomc = 1;
	}
	} else if ((DEV_TYPC > 9) && (DEV_TYPC < 16)) { 
	if (!bStateC) {
	r4slppar1c = 0;
	r4slpcomc = 6;
	} else {
	r4slppar1c = 0;
	r4slpcomc = 1;
	}
	} else if ((DEV_TYPC > 15) && (DEV_TYPC < 64)) { 
	if (!bStateC) {
	r4slppar1c = 1;
	r4slpcomc = 10;
	} else {
	r4slppar1c = 0;
	r4slpcomc = 10;
	}
	} else if ((DEV_TYPC > 63) && (DEV_TYPC < 70)) { 
	if (!bStateC) {
	r4slppar1c = 0;
	r4slpcomc = 65;
	} else {
	r4slppar1c = 0;
	r4slpcomc = 64;
	}
	}

	}
	}
	} else cntgpio3 = 0;
	break;

	}


	switch (bgpio4 & 0xc0) {
	case 128:
	if ((gpio_get_level(bgpio4 & 0x3f)) != lvgpio4) {
        cntgpio4++;
	if (cntgpio4 > 5) {
	fgpio4 = 1;
	lvgpio4 = lvgpio4 ^ 1;
	t_lasts_us = ~t_lasts_us;
	cntgpio4 = 0;
	}
	} else cntgpio4 = 0;
	break;
	}

	switch (bgpio5 & 0xc0) {
	case 128:
	if ((gpio_get_level(bgpio5 & 0x3f)) != lvgpio5) {
        cntgpio5++;
	if (cntgpio5 > 5) {
	fgpio5 = 1;
	lvgpio5 = lvgpio5 ^ 1;
	t_lasts_us = ~t_lasts_us;
	cntgpio5 = 0;
	}
	} else cntgpio5 = 0;
	break;
	}
}

//******************** ble **********************
/* Declare static functions */
static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
static void esp_gattc_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);
static void gattc_profile_a_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);
static void gattc_profile_b_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);
static void gattc_profile_c_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);

//redmond
static esp_bt_uuid_t remote_filter_service_uuid = {
    .len = ESP_UUID_LEN_128,
    .uuid = {.uuid128 = REMOTE_SERVICE_UUID,},
};
static esp_bt_uuid_t remote_filter_txchar_uuid = {
    .len = ESP_UUID_LEN_128,
    .uuid = {.uuid128 = REMOTE_TXCHAR_UUID,},
};
static esp_bt_uuid_t remote_filter_rxchar_uuid = {
    .len = ESP_UUID_LEN_128,
    .uuid = {.uuid128 = REMOTE_RXCHAR_UUID,},
};

static esp_bt_uuid_t notify_descr_uuid = {
    .len = ESP_UUID_LEN_16,
    .uuid = {.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG,},
};

//xiaomi
static esp_bt_uuid_t xremote_filter_service_uuid = {
    .len = ESP_UUID_LEN_128,
    .uuid = {.uuid128 = XREMOTE_SERVICE_UUID,},
};
static esp_bt_uuid_t xremote_filter_service1_uuid = {
    .len = ESP_UUID_LEN_128,
    .uuid = {.uuid128 = XREMOTE_SERVICE1_UUID,},
};
static esp_bt_uuid_t xremote_filter_service2_uuid = {
    .len = ESP_UUID_LEN_128,
    .uuid = {.uuid128 = XREMOTE_SERVICE2_UUID,},
};
static esp_bt_uuid_t xremote_filter_service3_uuid = {
    .len = ESP_UUID_LEN_128,
    .uuid = {.uuid128 = XREMOTE_SERVICE3_UUID,},
};

static esp_bt_uuid_t xremote_filter_status_uuid = {
    .len = ESP_UUID_LEN_128,
    .uuid = {.uuid128 = XREMOTE_STATUS_UUID,},
};
static esp_bt_uuid_t xremote_filter_authinit_uuid = {
    .len = ESP_UUID_LEN_128,
    .uuid = {.uuid128 = XREMOTE_AUTHINIT_UUID,},
};
static esp_bt_uuid_t xremote_filter_auth_uuid = {
    .len = ESP_UUID_LEN_128,
    .uuid = {.uuid128 = XREMOTE_AUTH_UUID,},
};
static esp_bt_uuid_t xremote_filter_ver_uuid = {
    .len = ESP_UUID_LEN_128,
    .uuid = {.uuid128 = XREMOTE_VER_UUID,},
};
static esp_bt_uuid_t xremote_filter_setup_uuid = {
    .len = ESP_UUID_LEN_128,
    .uuid = {.uuid128 = XREMOTE_SETUP_UUID,},
};
static esp_bt_uuid_t xremote_filter_time_uuid = {
    .len = ESP_UUID_LEN_128,
    .uuid = {.uuid128 = XREMOTE_TIME_UUID,},
};
static esp_bt_uuid_t xremote_filter_boil_uuid = {
    .len = ESP_UUID_LEN_128,
    .uuid = {.uuid128 = XREMOTE_BOIL_UUID,},
};
static esp_bt_uuid_t xremote_filter_mcuver_uuid = {
    .len = ESP_UUID_LEN_128,
    .uuid = {.uuid128 = XREMOTE_MCUVER_UUID,},
};
static esp_bt_uuid_t xremote_filter_update_uuid = {
    .len = ESP_UUID_LEN_128,
    .uuid = {.uuid128 = XREMOTE_UPDATE_UUID,},
};


static esp_ble_scan_params_t ble_scan_params = {
    .scan_type              = BLE_SCAN_TYPE_ACTIVE,
    .own_addr_type          = BLE_ADDR_TYPE_PUBLIC,
    .scan_filter_policy     = BLE_SCAN_FILTER_ALLOW_ALL,
    .scan_interval          = 0x50,
    .scan_window            = 0x30,
    .scan_duplicate         = BLE_SCAN_DUPLICATE_DISABLE
};

struct gattc_profile_inst {
    esp_gattc_cb_t gattc_cb;
    uint16_t gattc_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_start_handle;
    uint16_t service_end_handle;
    uint16_t service1_start_handle;
    uint16_t service1_end_handle;
    uint16_t service2_start_handle;
    uint16_t service2_end_handle;
    uint16_t service3_start_handle;
    uint16_t service3_end_handle;
    uint16_t rxchar_handle;      //0 redm rxchar, xiaomi status
    uint16_t txchar_handle;      //1 redm txchar, xiaomi authinit
    uint16_t auth_handle;        //2
    uint16_t ver_handle;         //3
    uint16_t setup_handle;       //4
    uint16_t time_handle;        //5
    uint16_t boil_handle;        //6
    uint16_t mcuver_handle;      //7
    uint16_t update_handle;      //8
    esp_bd_addr_t remote_bda;
};

/* One gatt-based profile one app_id and one gattc_if, this array will store the gattc_if returned by ESP_GATTS_REG_EVT */
static struct gattc_profile_inst gl_profile_tab[PROFILE_NUM] = {

//connection profile A handle
    [PROFILE_A_APP_ID] = {
        .gattc_cb = gattc_profile_a_event_handler,
        .gattc_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
    },

//connection profile B handle
    [PROFILE_B_APP_ID] = {
        .gattc_cb = gattc_profile_b_event_handler,
        .gattc_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
    },

//connection profile C handle
    [PROFILE_C_APP_ID] = {
        .gattc_cb = gattc_profile_c_event_handler,
        .gattc_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
    },

};

static void start_scan(void)
{
	uint32_t duration = 0; //30
//step 1: start scan only if not update, if not scanning, if not already starting scan or if no connections is opening   
	if (!f_update && !Isscanning && !StartingScan && (!btconnecta || btopena) && (!btconnectb || btopenb) && (!btconnectc || btopenc)) {
//	if (!f_update && !Isscanning && !StartingScan && (!btconnecta || btauthoriza) && (!btconnectb || btauthorizb) && (!btconnectc || btauthorizc)) {
//step 2: start scan if defined but not open connection present
        if ((REQ_NAMEA[0] && !btconnecta) || (REQ_NAMEB[0] && !btconnectb) ||(REQ_NAMEC[0] && !btconnectc)) {
	FND_NAME[0] = 0;
        esp_ble_gap_start_scanning(duration);
	StartingScan = true;
	ESP_LOGI(AP_TAG, "Scan starting");
		}
	}
}

void MqttPubSubA (bool mqtttst) {
	if (!mqtdel && tBLEAddrA[0] && mqtttst && DEV_TYPA) {
	char buft[64];
	char bufd[2048];
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrA);
	strcat(buft,"/status");
	esp_mqtt_client_publish(mqttclient, buft, "online", 0, 1, 1);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrA);
	strcat(buft,"/name");
	esp_mqtt_client_publish(mqttclient, buft, DEV_NAMEA, 0, 1, 1);

	if ((DEV_TYPA < 10) || (DEV_TYPA > 63)) {
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrA);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/boil");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrA);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/heat");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrA);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/heat_temp");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	if (DEV_TYPA < 10) {
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrA);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/backlight");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrA);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/beep");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrA);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/nightlight");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrA);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/nightlight_red");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrA);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/nightlight_green");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrA);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/nightlight_blue");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrA);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/nightlight_rgb");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	}
	if (FDHass) {
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,tBLEAddrA);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Kettle.boil\",\"icon\":\"mdi:kettle\",\"uniq_id\":\"boil_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Kettle\",\"model\":\"");
	strcat(bufd,DEV_NAMEA);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	if (DEV_TYPA < 64 ) strcat(bufd,"Redmond");
	else strcat(bufd,"Xiaomi");
	strcat(bufd,"\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/boil\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/boil\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/4x");
	strcat(buft,tBLEAddrA);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Kettle.heat\",\"icon\":\"mdi:kettle\",\"uniq_id\":\"heat_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Kettle\",\"model\":\"");
	strcat(bufd,DEV_NAMEA);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	if (DEV_TYPA < 64 ) strcat(bufd,"Redmond");
	else strcat(bufd,"Xiaomi");
	strcat(bufd,"\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/heat\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/heat\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/climate/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,tBLEAddrA);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Kettle.temp\",\"uniq_id\":\"temp_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Kettle\",\"model\":\"");
	strcat(bufd,DEV_NAMEA);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	if (DEV_TYPA < 64 ) strcat(bufd,"Redmond");
	else strcat(bufd,"Xiaomi");
	strcat(bufd,"\"},\"temperature_command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/heat_temp\",\"temperature_state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/heat_temp\",\"current_temperature_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/temp\",\"min_temp\":\"0\",\"max_temp\":\"100\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"/status\",\"mode_state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/hstate\",\"mode_command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/heat\"");
	strcat(bufd,",\"temp_step\":\"5\",\"modes\":[\"off\",\"heat\"]}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,tBLEAddrA);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Kettle.rssi\",\"icon\":\"mdi:bluetooth\",\"uniq_id\":\"rssi_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Kettle\",\"model\":\"");
	strcat(bufd,DEV_NAMEA);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	if (DEV_TYPA < 64 ) strcat(bufd,"Redmond");
	else strcat(bufd,"Xiaomi");
	strcat(bufd,"\"},\"device_class\":\"signal_strength\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"/rssi\",\"unit_of_meas\":\"dBm\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	if ((DEV_TYPA < 10) && (DEV_TYPA > 3)) {
	strcpy(buft,"homeassistant/light/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,tBLEAddrA);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Kettle.light\",\"uniq_id\":\"light_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Kettle\",\"model\":\"");
	strcat(bufd,DEV_NAMEA);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/nightlight\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/nightlight\",\"rgb_cmd_t\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/nightlight_rgb\",\"rgb_stat_t\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/nightlight_rgb\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/2x");
	strcat(buft,tBLEAddrA);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Kettle.beep\",\"icon\":\"mdi:speaker\",\"uniq_id\":\"beep_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Kettle\",\"model\":\"");
	strcat(bufd,DEV_NAMEA);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	strcat(bufd,"Redmond");
	strcat(bufd,"\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/beep\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/beep\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/3x");
	strcat(buft,tBLEAddrA);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Kettle.backlight\",\"icon\":\"mdi:thermometer\",\"uniq_id\":\"backlight_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Kettle\",\"model\":\"");
	strcat(bufd,DEV_NAMEA);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	strcat(bufd,"Redmond");
	strcat(bufd,"\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/backlight\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/backlight\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/2x");
	strcat(buft,tBLEAddrA);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Kettle.energy\",\"icon\":\"mdi:counter\",\"uniq_id\":\"energy_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Kettle\",\"model\":\"");
	strcat(bufd,DEV_NAMEA);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	strcat(bufd,"Redmond");
	strcat(bufd,"\"},\"device_class\":\"energy\",\"state_class\":\"total_increasing\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/total_energy\",\"unit_of_meas\":\"kWh\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/3x");
	strcat(buft,tBLEAddrA);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Kettle.ontime\",\"icon\":\"mdi:timer\",\"uniq_id\":\"ontime_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Kettle\",\"model\":\"");
	strcat(bufd,DEV_NAMEA);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	strcat(bufd,"Redmond");
	strcat(bufd,"\"},\"state_class\":\"total_increasing\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/working_time\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/4x");
	strcat(buft,tBLEAddrA);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Kettle.oncount\",\"icon\":\"mdi:counter\",\"uniq_id\":\"oncount_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Kettle\",\"model\":\"");
	strcat(bufd,DEV_NAMEA);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	strcat(bufd,"Redmond");
	strcat(bufd,"\"},\"state_class\":\"total_increasing\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/working_count\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	}
	}
	} else if ( DEV_TYPA < 12) {
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrA);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/state");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrA);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/lock");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	if (FDHass) {
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,tBLEAddrA);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Power.switch\",\"icon\":\"mdi:power-plug\",\"uniq_id\":\"switch_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Power_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Power\",\"model\":\"");
	strcat(bufd,DEV_NAMEA);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/state\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/state\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/2x");
	strcat(buft,tBLEAddrA);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Power.lock\",\"icon\":\"mdi:power-plug\",\"uniq_id\":\"lock_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Power_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Power\",\"model\":\"");
	strcat(bufd,DEV_NAMEA);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/lock\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/lock\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,tBLEAddrA);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Power.rssi\",\"icon\":\"mdi:bluetooth\",\"uniq_id\":\"rssi_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Power_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Power\",\"model\":\"");
	strcat(bufd,DEV_NAMEA);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"device_class\":\"signal_strength\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"/rssi\",\"unit_of_meas\":\"dBm\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	}
	} else if ( DEV_TYPA < 16) {
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrA);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/state");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrA);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/lock");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrA);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/strength");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrA);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/delay");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrA);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/delay_hour");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrA);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/delay_min");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	if (FDHass) {
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,tBLEAddrA);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Coffee.switch\",\"icon\":\"mdi:coffee\",\"uniq_id\":\"switch_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Coffee_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Coffee\",\"model\":\"");
	strcat(bufd,DEV_NAMEA);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/state\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/state\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/2x");
	strcat(buft,tBLEAddrA);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Coffee.lock\",\"icon\":\"mdi:coffee\",\"uniq_id\":\"lock_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Coffee_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Coffee\",\"model\":\"");
	strcat(bufd,DEV_NAMEA);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/lock\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/lock\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/3x");
	strcat(buft,tBLEAddrA);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Coffee.strength\",\"icon\":\"mdi:coffee\",\"uniq_id\":\"strength_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Coffee_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Coffee\",\"model\":\"");
	strcat(bufd,DEV_NAMEA);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/strength\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/strength\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,tBLEAddrA);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Coffee.rssi\",\"icon\":\"mdi:bluetooth\",\"uniq_id\":\"rssi_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Coffee_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Coffee\",\"model\":\"");
	strcat(bufd,DEV_NAMEA);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"device_class\":\"signal_strength\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"/rssi\",\"unit_of_meas\":\"dBm\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//

	}
	} else if ( DEV_TYPA < 64) {
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrA);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/state");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrA);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/prname");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrA);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/prog");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrA);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/mode");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrA);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/temp");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrA);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/set_hour");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrA);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/set_min");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrA);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/delay_hour");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrA);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/delay_min");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrA);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/warm");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	if (FDHass) {
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,tBLEAddrA);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Cooker.switch\",\"icon\":\"mdi:chef-hat\",\"uniq_id\":\"switch_");
//	strcat(bufd,"1.Cooker.switch\",\"icon\":\"mdi:pot-steam\",\"uniq_id\":\"switch_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Cooker\",\"model\":\"");
	strcat(bufd,DEV_NAMEA);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/state\",\"pl_on\":\"heat\",\"pl_off\":\"off\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/hstate\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/2x");
	strcat(buft,tBLEAddrA);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
//	strcat(bufd,"1.Cooker.warming\",\"icon\":\"mdi:chef-hat\",\"uniq_id\":\"warm_");
	strcat(bufd,"1.Cooker.warming\",\"icon\":\"mdi:pot-steam\",\"uniq_id\":\"warm_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Cooker\",\"model\":\"");
	strcat(bufd,DEV_NAMEA);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/warm\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/warm\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/climate/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,tBLEAddrA);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Cooker.temp\",\"uniq_id\":\"temp_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Cooker\",\"model\":\"");
	strcat(bufd,DEV_NAMEA);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"temperature_command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/temp\",\"temperature_state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/temp\",\"current_temperature_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/temp\",\"min_temp\":\"0\",\"max_temp\":\"100\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"/status\",\"mode_state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/hstate\",\"mode_command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/state\"");
	strcat(bufd,",\"hold_state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/prname\"");
	strcat(bufd,",\"hold_command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/prname\"");
	strcat(bufd,",\"temp_lo_stat_t\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/set_hour\"");
	strcat(bufd,",\"temp_hi_stat_t\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/set_min\"");
	strcat(bufd,",\"temp_lo_cmd_t\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/set_hour\"");
	strcat(bufd,",\"temp_hi_cmd_t\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/set_min\"");
        if ( DEV_TYPA == 16 ) {
	strcat(bufd,",\"temp_step\":\"1\",\"modes\":[\"off\",\"heat\"],\"hold_modes\":[\"OFF\",\"Multicooker\",\"Rice\",\"Slow_cooking\",\"Pilaf\",\"Frying_vegetables\"");
	strcat(bufd,",\"Frying_fish\",\"Frying_meat\",\"Stewing_vegetables\",\"Stewing_fish\",\"Stewing_meat\"");
	strcat(bufd,",\"Pasta\",\"Milk_porridge\",\"Soup\",\"Yogurt\",\"Baking\",\"Steam_vegetables\"");
	strcat(bufd,",\"Steam_fish\",\"Steam_meat\",\"Hot\"]}");
	} else if ( DEV_TYPA == 17 ) {
	strcat(bufd,",\"temp_step\":\"1\",\"modes\":[\"off\",\"heat\"],\"hold_modes\":[\"OFF\",\"Multicooker\",\"Milk_porridge\",\"Stewing\",\"Frying\",\"Soup\"");
	strcat(bufd,",\"Steam\",\"Pasta\",\"Slow_cooking\",\"Hot\",\"Baking\"");
	strcat(bufd,",\"Groats\",\"Pilaf\",\"Yogurt\",\"Pizza\",\"Bread\",\"Desserts\",\"Express\"]}");
	} else if ( DEV_TYPA == 18 ) {
	strcat(bufd,",\"temp_step\":\"1\",\"modes\":[\"off\",\"heat\"],\"hold_modes\":[\"OFF\",\"Frying\",\"Groats\",\"Multicooker\",\"Pilaf\",\"Steam\"");
	strcat(bufd,",\"Baking\",\"Stewing\",\"Soup\",\"Milk_porridge\",\"Yogurt\",\"Express\"]}");
	} else if ( DEV_TYPA == 24 ) {
	strcat(bufd,",\"temp_step\":\"1\",\"modes\":[\"off\",\"heat\"],\"hold_modes\":[\"OFF\",\"Multicooker\",\"Omelet\"");
	strcat(bufd,",\"Slow_cooking_meat\",\"Slow_cooking_bird\",\"Slow_cooking_fish\",\"Slow_cooking_vegetables\"");
	strcat(bufd,",\"Bread\",\"Pizza\",\"Charlotte\",\"Baking_meat_in_pot\",\"Baking_bird_in_pot\"");
	strcat(bufd,",\"Baking_fish_in_pot\",\"Baking_vegetables_in_pot\",\"Roast\",\"Cake\",\"Baking_meat\"");
	strcat(bufd,",\"Baking_bird\",\"Baking_fish\",\"Baking_vegetables\",\"Boiled_pork\",\"Warming\"]}");
	}
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,tBLEAddrA);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Cooker.prog\",\"icon\":\"mdi:pot-steam\",\"uniq_id\":\"prog_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Cooker\",\"model\":\"");
	strcat(bufd,DEV_NAMEA);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/prname\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/2x");
	strcat(buft,tBLEAddrA);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Cooker.state\",\"icon\":\"mdi:pot-steam\",\"uniq_id\":\"stat_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Cooker\",\"model\":\"");
	strcat(bufd,DEV_NAMEA);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/state\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/3x");
	strcat(buft,tBLEAddrA);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Cooker.hour\",\"icon\":\"mdi:timer\",\"uniq_id\":\"phour_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Cooker\",\"model\":\"");
	strcat(bufd,DEV_NAMEA);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/hour\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/4x");
	strcat(buft,tBLEAddrA);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Cooker.min\",\"icon\":\"mdi:timer\",\"uniq_id\":\"pmin_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Cooker\",\"model\":\"");
	strcat(bufd,DEV_NAMEA);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/min\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/5x");
	strcat(buft,tBLEAddrA);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Cooker.rssi\",\"icon\":\"mdi:bluetooth\",\"uniq_id\":\"rssi_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"1.Cooker\",\"model\":\"");
	strcat(bufd,DEV_NAMEA);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"device_class\":\"signal_strength\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"/rssi\",\"unit_of_meas\":\"dBm\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrA);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	}

	}
	}
}

void MqttPubSubB (bool mqtttst) {
	if (!mqtdel && tBLEAddrB[0] && mqtttst && DEV_TYPB) {
	char buft[64];
	char bufd[2048];
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrB);
	strcat(buft,"/status");
	esp_mqtt_client_publish(mqttclient, buft, "online", 0, 1, 1);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrB);
	strcat(buft,"/name");
	esp_mqtt_client_publish(mqttclient, buft, DEV_NAMEB, 0, 1, 1);

	if ((DEV_TYPB < 10) || (DEV_TYPB > 63)) {
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrB);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/boil");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrB);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/heat");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrB);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/heat_temp");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	if (DEV_TYPB < 10) {
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrB);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/backlight");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrB);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/beep");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrB);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/nightlight");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrB);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/nightlight_red");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrB);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/nightlight_green");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrB);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/nightlight_blue");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrB);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/nightlight_rgb");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	}
	if (FDHass) {
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,tBLEAddrB);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Kettle.boil\",\"icon\":\"mdi:kettle\",\"uniq_id\":\"boil_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Kettle\",\"model\":\"");
	strcat(bufd,DEV_NAMEB);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	if (DEV_TYPB < 64 ) strcat(bufd,"Redmond");
	else strcat(bufd,"Xiaomi");
	strcat(bufd,"\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/boil\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/boil\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/4x");
	strcat(buft,tBLEAddrB);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Kettle.heat\",\"icon\":\"mdi:kettle\",\"uniq_id\":\"heat_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Kettle\",\"model\":\"");
	strcat(bufd,DEV_NAMEB);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	if (DEV_TYPB < 64 ) strcat(bufd,"Redmond");
	else strcat(bufd,"Xiaomi");
	strcat(bufd,"\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/heat\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/heat\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/climate/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,tBLEAddrB);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Kettle.temp\",\"uniq_id\":\"temp_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Kettle\",\"model\":\"");
	strcat(bufd,DEV_NAMEB);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	if (DEV_TYPB < 64 ) strcat(bufd,"Redmond");
	else strcat(bufd,"Xiaomi");
	strcat(bufd,"\"},\"temperature_command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/heat_temp\",\"temperature_state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/heat_temp\",\"current_temperature_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/temp\",\"min_temp\":\"0\",\"max_temp\":\"100\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"/status\",\"mode_state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/hstate\",\"mode_command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/heat\"");
	strcat(bufd,",\"temp_step\":\"5\",\"modes\":[\"off\",\"heat\"]}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,tBLEAddrB);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Kettle.rssi\",\"icon\":\"mdi:bluetooth\",\"uniq_id\":\"rssi_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Kettle\",\"model\":\"");
	strcat(bufd,DEV_NAMEB);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	if (DEV_TYPB < 64 ) strcat(bufd,"Redmond");
	else strcat(bufd,"Xiaomi");
	strcat(bufd,"\"},\"device_class\":\"signal_strength\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"/rssi\",\"unit_of_meas\":\"dBm\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	if ((DEV_TYPB < 10) && (DEV_TYPB > 3)) {
	strcpy(buft,"homeassistant/light/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,tBLEAddrB);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Kettle.light\",\"uniq_id\":\"light_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Kettle\",\"model\":\"");
	strcat(bufd,DEV_NAMEB);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/nightlight\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/nightlight\",\"rgb_cmd_t\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/nightlight_rgb\",\"rgb_stat_t\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/nightlight_rgb\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/2x");
	strcat(buft,tBLEAddrB);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Kettle.beep\",\"icon\":\"mdi:speaker\",\"uniq_id\":\"beep_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Kettle\",\"model\":\"");
	strcat(bufd,DEV_NAMEB);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	strcat(bufd,"Redmond");
	strcat(bufd,"\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/beep\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/beep\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/3x");
	strcat(buft,tBLEAddrB);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Kettle.backlight\",\"icon\":\"mdi:thermometer\",\"uniq_id\":\"backlight_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Kettle\",\"model\":\"");
	strcat(bufd,DEV_NAMEB);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	strcat(bufd,"Redmond");
	strcat(bufd,"\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/backlight\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/backlight\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/2x");
	strcat(buft,tBLEAddrB);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Kettle.energy\",\"icon\":\"mdi:counter\",\"uniq_id\":\"energy_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Kettle\",\"model\":\"");
	strcat(bufd,DEV_NAMEB);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	strcat(bufd,"Redmond");
	strcat(bufd,"\"},\"device_class\":\"energy\",\"state_class\":\"total_increasing\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/total_energy\",\"unit_of_meas\":\"kWh\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/3x");
	strcat(buft,tBLEAddrB);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Kettle.ontime\",\"icon\":\"mdi:timer\",\"uniq_id\":\"ontime_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Kettle\",\"model\":\"");
	strcat(bufd,DEV_NAMEB);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	strcat(bufd,"Redmond");
	strcat(bufd,"\"},\"state_class\":\"total_increasing\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/working_time\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/4x");
	strcat(buft,tBLEAddrB);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Kettle.oncount\",\"icon\":\"mdi:counter\",\"uniq_id\":\"oncount_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Kettle\",\"model\":\"");
	strcat(bufd,DEV_NAMEB);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	strcat(bufd,"Redmond");
	strcat(bufd,"\"},\"state_class\":\"total_increasing\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/working_count\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	}
	}
	} else if ( DEV_TYPB < 12) {
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrB);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/state");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrB);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/lock");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	if (FDHass) {
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,tBLEAddrB);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Power.switch\",\"icon\":\"mdi:power-plug\",\"uniq_id\":\"switch_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Power_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Power\",\"model\":\"");
	strcat(bufd,DEV_NAMEB);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/state\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/state\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/2x");
	strcat(buft,tBLEAddrB);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Power.lock\",\"icon\":\"mdi:power-plug\",\"uniq_id\":\"lock_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Power_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Power\",\"model\":\"");
	strcat(bufd,DEV_NAMEB);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/lock\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/lock\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,tBLEAddrB);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Power.rssi\",\"icon\":\"mdi:bluetooth\",\"uniq_id\":\"rssi_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Power_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Power\",\"model\":\"");
	strcat(bufd,DEV_NAMEB);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"device_class\":\"signal_strength\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"/rssi\",\"unit_of_meas\":\"dBm\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//

	}
	} else if ( DEV_TYPB < 16) {
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrB);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/state");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrB);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/lock");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrB);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/strength");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrB);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/delay");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrB);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/delay_hour");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrB);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/delay_min");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	if (FDHass) {
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,tBLEAddrB);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Coffee.switch\",\"icon\":\"mdi:coffee\",\"uniq_id\":\"switch_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Coffee_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Coffee\",\"model\":\"");
	strcat(bufd,DEV_NAMEB);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/state\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/state\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/2x");
	strcat(buft,tBLEAddrB);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Coffee.lock\",\"icon\":\"mdi:coffee\",\"uniq_id\":\"lock_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Coffee_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Coffee\",\"model\":\"");
	strcat(bufd,DEV_NAMEB);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/lock\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/lock\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/3x");
	strcat(buft,tBLEAddrB);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Coffee.strength\",\"icon\":\"mdi:coffee\",\"uniq_id\":\"strength_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Coffee_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Coffee\",\"model\":\"");
	strcat(bufd,DEV_NAMEB);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/strength\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/strength\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,tBLEAddrB);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Coffee.rssi\",\"icon\":\"mdi:bluetooth\",\"uniq_id\":\"rssi_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Coffee_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Coffee\",\"model\":\"");
	strcat(bufd,DEV_NAMEB);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"device_class\":\"signal_strength\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"/rssi\",\"unit_of_meas\":\"dBm\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//

	}
	} else if ( DEV_TYPB < 64) {
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrB);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/state");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrB);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/prname");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrB);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/prog");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrB);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/mode");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrB);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/temp");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrB);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/set_hour");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrB);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/set_min");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrB);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/delay_hour");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrB);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/delay_min");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrB);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/warm");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	if (FDHass) {
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,tBLEAddrB);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Cooker.switch\",\"icon\":\"mdi:chef-hat\",\"uniq_id\":\"switch_");
//	strcat(bufd,"2.Cooker.switch\",\"icon\":\"mdi:pot-steam\",\"uniq_id\":\"switch_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Cooker\",\"model\":\"");
	strcat(bufd,DEV_NAMEB);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/state\",\"pl_on\":\"heat\",\"pl_off\":\"off\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/hstate\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/2x");
	strcat(buft,tBLEAddrB);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
//	strcat(bufd,"2.Cooker.warming\",\"icon\":\"mdi:chef-hat\",\"uniq_id\":\"warm_");
	strcat(bufd,"2.Cooker.warming\",\"icon\":\"mdi:pot-steam\",\"uniq_id\":\"warm_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Cooker\",\"model\":\"");
	strcat(bufd,DEV_NAMEB);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/warm\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/warm\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/climate/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,tBLEAddrB);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Cooker.temp\",\"uniq_id\":\"temp_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Cooker\",\"model\":\"");
	strcat(bufd,DEV_NAMEB);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"temperature_command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/temp\",\"temperature_state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/temp\",\"current_temperature_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/temp\",\"min_temp\":\"0\",\"max_temp\":\"100\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"/status\",\"mode_state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/hstate\",\"mode_command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/state\"");
	strcat(bufd,",\"hold_state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/prname\"");
	strcat(bufd,",\"hold_command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/prname\"");
	strcat(bufd,",\"temp_lo_stat_t\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/set_hour\"");
	strcat(bufd,",\"temp_hi_stat_t\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/set_min\"");
	strcat(bufd,",\"temp_lo_cmd_t\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/set_hour\"");
	strcat(bufd,",\"temp_hi_cmd_t\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/set_min\"");
        if ( DEV_TYPB == 16 ) {
	strcat(bufd,",\"temp_step\":\"1\",\"modes\":[\"off\",\"heat\"],\"hold_modes\":[\"OFF\",\"Multicooker\",\"Rice\",\"Slow_cooking\",\"Pilaf\",\"Frying_vegetables\"");
	strcat(bufd,",\"Frying_fish\",\"Frying_meat\",\"Stewing_vegetables\",\"Stewing_fish\",\"Stewing_meat\"");
	strcat(bufd,",\"Pasta\",\"Milk_porridge\",\"Soup\",\"Yogurt\",\"Baking\",\"Steam_vegetables\"");
	strcat(bufd,",\"Steam_fish\",\"Steam_meat\",\"Hot\"]}");
	} else if ( DEV_TYPB == 17 ) {
	strcat(bufd,",\"temp_step\":\"1\",\"modes\":[\"off\",\"heat\"],\"hold_modes\":[\"OFF\",\"Multicooker\",\"Milk_porridge\",\"Stewing\",\"Frying\",\"Soup\"");
	strcat(bufd,",\"Steam\",\"Pasta\",\"Slow_cooking\",\"Hot\",\"Baking\"");
	strcat(bufd,",\"Groats\",\"Pilaf\",\"Yogurt\",\"Pizza\",\"Bread\",\"Desserts\",\"Express\"]}");
	} else if ( DEV_TYPB == 18 ) {
	strcat(bufd,",\"temp_step\":\"1\",\"modes\":[\"off\",\"heat\"],\"hold_modes\":[\"OFF\",\"Frying\",\"Groats\",\"Multicooker\",\"Pilaf\",\"Steam\"");
	strcat(bufd,",\"Baking\",\"Stewing\",\"Soup\",\"Milk_porridge\",\"Yogurt\",\"Express\"]}");
	} else if ( DEV_TYPB == 24 ) {
	strcat(bufd,",\"temp_step\":\"1\",\"modes\":[\"off\",\"heat\"],\"hold_modes\":[\"OFF\",\"Multicooker\",\"Omelet\"");
	strcat(bufd,",\"Slow_cooking_meat\",\"Slow_cooking_bird\",\"Slow_cooking_fish\",\"Slow_cooking_vegetables\"");
	strcat(bufd,",\"Bread\",\"Pizza\",\"Charlotte\",\"Baking_meat_in_pot\",\"Baking_bird_in_pot\"");
	strcat(bufd,",\"Baking_fish_in_pot\",\"Baking_vegetables_in_pot\",\"Roast\",\"Cake\",\"Baking_meat\"");
	strcat(bufd,",\"Baking_bird\",\"Baking_fish\",\"Baking_vegetables\",\"Boiled_pork\",\"Warming\"]}");
	}
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,tBLEAddrB);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Cooker.prog\",\"icon\":\"mdi:pot-steam\",\"uniq_id\":\"prog_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Cooker\",\"model\":\"");
	strcat(bufd,DEV_NAMEB);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/prname\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/2x");
	strcat(buft,tBLEAddrB);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Cooker.state\",\"icon\":\"mdi:pot-steam\",\"uniq_id\":\"stat_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Cooker\",\"model\":\"");
	strcat(bufd,DEV_NAMEB);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/state\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/3x");
	strcat(buft,tBLEAddrB);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Cooker.hour\",\"icon\":\"mdi:timer\",\"uniq_id\":\"phour_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Cooker\",\"model\":\"");
	strcat(bufd,DEV_NAMEB);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/hour\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/4x");
	strcat(buft,tBLEAddrB);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Cooker.min\",\"icon\":\"mdi:timer\",\"uniq_id\":\"pmin_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Cooker\",\"model\":\"");
	strcat(bufd,DEV_NAMEB);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/min\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/5x");
	strcat(buft,tBLEAddrB);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Cooker.rssi\",\"icon\":\"mdi:bluetooth\",\"uniq_id\":\"rssi_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"2.Cooker\",\"model\":\"");
	strcat(bufd,DEV_NAMEB);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"device_class\":\"signal_strength\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"/rssi\",\"unit_of_meas\":\"dBm\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrB);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	}

	}
	}
}

void MqttPubSubC (bool mqtttst) {
	if (!mqtdel && tBLEAddrC[0] && mqtttst && DEV_TYPC) {
	char buft[64];
	char bufd[2048];
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrC);
	strcat(buft,"/status");
	esp_mqtt_client_publish(mqttclient, buft, "online", 0, 1, 1);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrC);
	strcat(buft,"/name");
	esp_mqtt_client_publish(mqttclient, buft, DEV_NAMEC, 0, 1, 1);

	if ((DEV_TYPC < 10) || (DEV_TYPC > 63)) {
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrC);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/boil");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrC);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/heat");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrC);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/heat_temp");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	if (DEV_TYPC < 10) {
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrC);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/backlight");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrC);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/beep");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrC);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/nightlight");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrC);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/nightlight_red");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrC);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/nightlight_green");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrC);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/nightlight_blue");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrC);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/nightlight_rgb");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	}
	if (FDHass) {
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,tBLEAddrC);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Kettle.boil\",\"icon\":\"mdi:kettle\",\"uniq_id\":\"boil_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Kettle\",\"model\":\"");
	strcat(bufd,DEV_NAMEC);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	if (DEV_TYPC < 64 ) strcat(bufd,"Redmond");
	else strcat(bufd,"Xiaomi");
	strcat(bufd,"\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/boil\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/boil\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/4x");
	strcat(buft,tBLEAddrC);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Kettle.heat\",\"icon\":\"mdi:kettle\",\"uniq_id\":\"heat_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Kettle\",\"model\":\"");
	strcat(bufd,DEV_NAMEC);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	if (DEV_TYPC < 64 ) strcat(bufd,"Redmond");
	else strcat(bufd,"Xiaomi");
	strcat(bufd,"\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/heat\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/heat\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/climate/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,tBLEAddrC);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Kettle.temp\",\"uniq_id\":\"temp_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Kettle\",\"model\":\"");
	strcat(bufd,DEV_NAMEC);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	if (DEV_TYPC < 64 ) strcat(bufd,"Redmond");
	else strcat(bufd,"Xiaomi");
	strcat(bufd,"\"},\"temperature_command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/heat_temp\",\"temperature_state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/heat_temp\",\"current_temperature_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/temp\",\"min_temp\":\"0\",\"max_temp\":\"100\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"/status\",\"mode_state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/hstate\",\"mode_command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/heat\"");
	strcat(bufd,",\"temp_step\":\"5\",\"modes\":[\"off\",\"heat\"]}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,tBLEAddrC);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Kettle.rssi\",\"icon\":\"mdi:bluetooth\",\"uniq_id\":\"rssi_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Kettle\",\"model\":\"");
	strcat(bufd,DEV_NAMEC);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	if (DEV_TYPC < 64 ) strcat(bufd,"Redmond");
	else strcat(bufd,"Xiaomi");
	strcat(bufd,"\"},\"device_class\":\"signal_strength\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"/rssi\",\"unit_of_meas\":\"dBm\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	if ((DEV_TYPC < 10) && (DEV_TYPC > 3)) {
	strcpy(buft,"homeassistant/light/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,tBLEAddrC);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Kettle.light\",\"uniq_id\":\"light_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Kettle\",\"model\":\"");
	strcat(bufd,DEV_NAMEC);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/nightlight\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/nightlight\",\"rgb_cmd_t\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/nightlight_rgb\",\"rgb_stat_t\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/nightlight_rgb\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/2x");
	strcat(buft,tBLEAddrC);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Kettle.beep\",\"icon\":\"mdi:speaker\",\"uniq_id\":\"beep_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Kettle\",\"model\":\"");
	strcat(bufd,DEV_NAMEC);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	strcat(bufd,"Redmond");
	strcat(bufd,"\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/beep\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/beep\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/3x");
	strcat(buft,tBLEAddrC);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Kettle.backlight\",\"icon\":\"mdi:thermometer\",\"uniq_id\":\"backlight_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Kettle\",\"model\":\"");
	strcat(bufd,DEV_NAMEC);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	strcat(bufd,"Redmond");
	strcat(bufd,"\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/backlight\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/backlight\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/2x");
	strcat(buft,tBLEAddrC);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Kettle.energy\",\"icon\":\"mdi:counter\",\"uniq_id\":\"energy_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Kettle\",\"model\":\"");
	strcat(bufd,DEV_NAMEC);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	strcat(bufd,"Redmond");
	strcat(bufd,"\"},\"device_class\":\"energy\",\"state_class\":\"total_increasing\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/total_energy\",\"unit_of_meas\":\"kWh\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/3x");
	strcat(buft,tBLEAddrC);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Kettle.ontime\",\"icon\":\"mdi:timer\",\"uniq_id\":\"ontime_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Kettle\",\"model\":\"");
	strcat(bufd,DEV_NAMEC);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	strcat(bufd,"Redmond");
	strcat(bufd,"\"},\"state_class\":\"total_increasing\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/working_time\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/4x");
	strcat(buft,tBLEAddrC);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Kettle.oncount\",\"icon\":\"mdi:counter\",\"uniq_id\":\"oncount_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Kettle\",\"model\":\"");
	strcat(bufd,DEV_NAMEC);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	strcat(bufd,"Redmond");
	strcat(bufd,"\"},\"state_class\":\"total_increasing\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/working_count\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	}
	}
	} else if ( DEV_TYPC < 12) {
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrC);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/state");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrC);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/lock");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	if (FDHass) {
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,tBLEAddrC);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Power.switch\",\"icon\":\"mdi:power-plug\",\"uniq_id\":\"switch_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Power_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Power\",\"model\":\"");
	strcat(bufd,DEV_NAMEC);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/state\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/state\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/2x");
	strcat(buft,tBLEAddrC);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Power.lock\",\"icon\":\"mdi:power-plug\",\"uniq_id\":\"lock_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Power_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Power\",\"model\":\"");
	strcat(bufd,DEV_NAMEC);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/lock\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/lock\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,tBLEAddrC);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Power.rssi\",\"icon\":\"mdi:bluetooth\",\"uniq_id\":\"rssi_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Power_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Power\",\"model\":\"");
	strcat(bufd,DEV_NAMEC);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"device_class\":\"signal_strength\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"/rssi\",\"unit_of_meas\":\"dBm\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//

	}
	} else if ( DEV_TYPC < 16) {
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrC);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/state");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrC);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/lock");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrC);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/strength");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrC);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/delay");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrC);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/delay_hour");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrC);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/delay_min");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	if (FDHass) {
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,tBLEAddrC);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Coffee.switch\",\"icon\":\"mdi:coffee\",\"uniq_id\":\"switch_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Coffee_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Coffee\",\"model\":\"");
	strcat(bufd,DEV_NAMEC);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/state\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/state\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/2x");
	strcat(buft,tBLEAddrC);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Coffee.lock\",\"icon\":\"mdi:coffee\",\"uniq_id\":\"lock_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Coffee_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Coffee\",\"model\":\"");
	strcat(bufd,DEV_NAMEC);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/lock\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/lock\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/3x");
	strcat(buft,tBLEAddrC);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Coffee.strength\",\"icon\":\"mdi:coffee\",\"uniq_id\":\"strength_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Coffee_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Coffee\",\"model\":\"");
	strcat(bufd,DEV_NAMEC);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/strength\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/strength\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,tBLEAddrC);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Coffee.rssi\",\"icon\":\"mdi:bluetooth\",\"uniq_id\":\"rssi_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Coffee_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Coffee\",\"model\":\"");
	strcat(bufd,DEV_NAMEC);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"device_class\":\"signal_strength\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"/rssi\",\"unit_of_meas\":\"dBm\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//

	}
	} else if ( DEV_TYPC < 64) {
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrC);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/state");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrC);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/prname");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrC);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/prog");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrC);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/mode");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrC);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/temp");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrC);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/set_hour");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrC);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/set_min");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrC);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/delay_hour");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrC);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/delay_min");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,tBLEAddrC);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/warm");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	if (FDHass) {
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,tBLEAddrC);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Cooker.switch\",\"icon\":\"mdi:chef-hat\",\"uniq_id\":\"switch_");
//	strcat(bufd,"3.Cooker.switch\",\"icon\":\"mdi:pot-steam\",\"uniq_id\":\"switch_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Cooker\",\"model\":\"");
	strcat(bufd,DEV_NAMEC);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/state\",\"pl_on\":\"heat\",\"pl_off\":\"off\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/hstate\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/2x");
	strcat(buft,tBLEAddrC);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
//	strcat(bufd,"3.Cooker.warming\",\"icon\":\"mdi:chef-hat\",\"uniq_id\":\"warm_");
	strcat(bufd,"3.Cooker.warming\",\"icon\":\"mdi:pot-steam\",\"uniq_id\":\"warm_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Cooker\",\"model\":\"");
	strcat(bufd,DEV_NAMEC);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/warm\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/warm\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/climate/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,tBLEAddrC);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Cooker.temp\",\"uniq_id\":\"temp_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Cooker\",\"model\":\"");
	strcat(bufd,DEV_NAMEC);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"temperature_command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/temp\",\"temperature_state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/temp\",\"current_temperature_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/temp\",\"min_temp\":\"0\",\"max_temp\":\"100\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"/status\",\"mode_state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/hstate\",\"mode_command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/state\"");
	strcat(bufd,",\"hold_state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/prname\"");
	strcat(bufd,",\"hold_command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/prname\"");
	strcat(bufd,",\"temp_lo_stat_t\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/set_hour\"");
	strcat(bufd,",\"temp_hi_stat_t\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/set_min\"");
	strcat(bufd,",\"temp_lo_cmd_t\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/set_hour\"");
	strcat(bufd,",\"temp_hi_cmd_t\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/set_min\"");
        if ( DEV_TYPC == 16 ) {
	strcat(bufd,",\"temp_step\":\"1\",\"modes\":[\"off\",\"heat\"],\"hold_modes\":[\"OFF\",\"Multicooker\",\"Rice\",\"Slow_cooking\",\"Pilaf\",\"Frying_vegetables\"");
	strcat(bufd,",\"Frying_fish\",\"Frying_meat\",\"Stewing_vegetables\",\"Stewing_fish\",\"Stewing_meat\"");
	strcat(bufd,",\"Pasta\",\"Milk_porridge\",\"Soup\",\"Yogurt\",\"Baking\",\"Steam_vegetables\"");
	strcat(bufd,",\"Steam_fish\",\"Steam_meat\",\"Hot\"]}");
	} else if ( DEV_TYPC == 17 ) {
	strcat(bufd,",\"temp_step\":\"1\",\"modes\":[\"off\",\"heat\"],\"hold_modes\":[\"OFF\",\"Multicooker\",\"Milk_porridge\",\"Stewing\",\"Frying\",\"Soup\"");
	strcat(bufd,",\"Steam\",\"Pasta\",\"Slow_cooking\",\"Hot\",\"Baking\"");
	strcat(bufd,",\"Groats\",\"Pilaf\",\"Yogurt\",\"Pizza\",\"Bread\",\"Desserts\",\"Express\"]}");
	} else if ( DEV_TYPC == 18 ) {
	strcat(bufd,",\"temp_step\":\"1\",\"modes\":[\"off\",\"heat\"],\"hold_modes\":[\"OFF\",\"Frying\",\"Groats\",\"Multicooker\",\"Pilaf\",\"Steam\"");
	strcat(bufd,",\"Baking\",\"Stewing\",\"Soup\",\"Milk_porridge\",\"Yogurt\",\"Express\"]}");
	} else if ( DEV_TYPC == 24 ) {
	strcat(bufd,",\"temp_step\":\"1\",\"modes\":[\"off\",\"heat\"],\"hold_modes\":[\"OFF\",\"Multicooker\",\"Omelet\"");
	strcat(bufd,",\"Slow_cooking_meat\",\"Slow_cooking_bird\",\"Slow_cooking_fish\",\"Slow_cooking_vegetables\"");
	strcat(bufd,",\"Bread\",\"Pizza\",\"Charlotte\",\"Baking_meat_in_pot\",\"Baking_bird_in_pot\"");
	strcat(bufd,",\"Baking_fish_in_pot\",\"Baking_vegetables_in_pot\",\"Roast\",\"Cake\",\"Baking_meat\"");
	strcat(bufd,",\"Baking_bird\",\"Baking_fish\",\"Baking_vegetables\",\"Boiled_pork\",\"Warming\"]}");
	}
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,tBLEAddrC);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Cooker.prog\",\"icon\":\"mdi:pot-steam\",\"uniq_id\":\"prog_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Cooker\",\"model\":\"");
	strcat(bufd,DEV_NAMEC);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/prname\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/2x");
	strcat(buft,tBLEAddrC);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Cooker.state\",\"icon\":\"mdi:pot-steam\",\"uniq_id\":\"stat_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Cooker\",\"model\":\"");
	strcat(bufd,DEV_NAMEC);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/state\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/3x");
	strcat(buft,tBLEAddrC);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Cooker.hour\",\"icon\":\"mdi:timer\",\"uniq_id\":\"phour_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Cooker\",\"model\":\"");
	strcat(bufd,DEV_NAMEC);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/hour\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/4x");
	strcat(buft,tBLEAddrC);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Cooker.min\",\"icon\":\"mdi:timer\",\"uniq_id\":\"pmin_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Cooker\",\"model\":\"");
	strcat(bufd,DEV_NAMEC);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/min\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/5x");
	strcat(buft,tBLEAddrC);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Cooker.rssi\",\"icon\":\"mdi:bluetooth\",\"uniq_id\":\"rssi_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"3.Cooker\",\"model\":\"");
	strcat(bufd,DEV_NAMEC);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"device_class\":\"signal_strength\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"/rssi\",\"unit_of_meas\":\"dBm\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,tBLEAddrC);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	}
	}
	}
}


//*** A ********************************************
static void gattc_profile_a_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    esp_ble_gattc_cb_param_t *p_data = (esp_ble_gattc_cb_param_t *)param;
	uint8_t  conerr = 0;
	uint8_t  buff1[16];
	uint8_t  bufftab[256];

    switch (event) {
    case ESP_GATTC_REG_EVT:
//	ESP_LOGI(AP_TAG, "REG_EVT");

	esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV,ESP_PWR_LVL_P9); // for more power???
	esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT,ESP_PWR_LVL_P9); // for more power???

        esp_err_t scan_ret = esp_ble_gap_set_scan_params(&ble_scan_params);
        if (scan_ret){
	ESP_LOGE(AP_TAG, "Set scan params error, error code = %x", scan_ret);
        }
	break;
    case ESP_GATTC_CONNECT_EVT:
	break;
    case ESP_GATTC_OPEN_EVT:
	xbtautha = 0;
        if (param->open.status != ESP_GATT_OK) {
	ESP_LOGE(AP_TAG, "Open A failed, status %d", p_data->open.status);
	btopena = false;
        btconnecta = false;
//	Isscanning = false;
	start_scan();
	break;
        }
        memcpy(gl_profile_tab[PROFILE_A_APP_ID].remote_bda, p_data->open.remote_bda, 6);
        gl_profile_tab[PROFILE_A_APP_ID].conn_id = p_data->open.conn_id;
        ESP_LOGI(AP_TAG, "Open A success, conn_id %d, if %d, status %d, mtu %d", p_data->open.conn_id, gattc_if, p_data->open.status, p_data->open.mtu);
        ESP_LOGI(AP_TAG, "REMOTE BDA A:");
        esp_log_buffer_hex(AP_TAG, p_data->open.remote_bda, sizeof(esp_bd_addr_t));
        esp_err_t mtu_ret = esp_ble_gattc_send_mtu_req (gattc_if, p_data->open.conn_id);
	btopena = true;
        if (mtu_ret){
            ESP_LOGE(AP_TAG, "config MTU A error, error code = %x", mtu_ret);
        }
	if (DEV_TYPA == 64) MiAKettleID = 275;
	if (DEV_TYPA == 65) MiAKettleID = 131;
	if (DEV_TYPA == 66) MiAKettleID = 1116;
        break;

    case ESP_GATTC_DIS_SRVC_CMPL_EVT:
        if (param->dis_srvc_cmpl.status != ESP_GATT_OK){
	conerr = 1;
	ESP_LOGE(AP_TAG, "Discover service A failed, status %d", param->dis_srvc_cmpl.status);
	break;
        }
        ESP_LOGI(AP_TAG, "Discover service A complete conn_id %d", param->dis_srvc_cmpl.conn_id);
        if (DEV_TYPA < 64) esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id, &remote_filter_service_uuid);
	else {
	esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id, &xremote_filter_service_uuid);
	esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id, &xremote_filter_service1_uuid);
	esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id, &xremote_filter_service2_uuid);
	esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id, &xremote_filter_service3_uuid);
	}
        break;
    case ESP_GATTC_CFG_MTU_EVT:
        if (param->cfg_mtu.status != ESP_GATT_OK){
	conerr = 1;
	ESP_LOGE(AP_TAG,"Config mtu failed, error status = %x", param->cfg_mtu.status);
        }
//	ESP_LOGI(AP_TAG, "ESP_GATTC_CFG_MTU_EVT, Status %d, MTU %d, conn_id %d", param->cfg_mtu.status, param->cfg_mtu.mtu, param->cfg_mtu.conn_id);
	break;
    case ESP_GATTC_SEARCH_RES_EVT: {
        ESP_LOGI(AP_TAG, "SEARCH RES A: conn_id = %x is primary service %d", p_data->search_res.conn_id, p_data->search_res.is_primary);
        ESP_LOGI(AP_TAG, "start handle %d end handle %d current handle value %d", p_data->search_res.start_handle, p_data->search_res.end_handle, p_data->search_res.srvc_id.inst_id);

//esp_log_buffer_hex(AP_TAG,p_data->search_res.srvc_id.uuid.uuid.uuid128,16);
//esp_log_buffer_hex(AP_TAG,xremote_filter_service11_uuid.uuid.uuid128,16);

        if ((DEV_TYPA < 64) && (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_128) && (bt_compare_UUID128(p_data->search_res.srvc_id.uuid.uuid.uuid128, remote_filter_service_uuid.uuid.uuid128))) {

            ESP_LOGI(AP_TAG, "Redmond Service A found");
            get_servera = true;
            gl_profile_tab[PROFILE_A_APP_ID].service_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[PROFILE_A_APP_ID].service_end_handle = p_data->search_res.end_handle;
        } else if ((DEV_TYPA > 63) && (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_128) && (bt_compare_UUID128(p_data->search_res.srvc_id.uuid.uuid.uuid128, xremote_filter_service_uuid.uuid.uuid128))) {

            ESP_LOGI(AP_TAG, "Xiaomi Service1 A found");
            gl_profile_tab[PROFILE_A_APP_ID].service_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[PROFILE_A_APP_ID].service_end_handle = p_data->search_res.end_handle;
        } else if ((DEV_TYPA > 63) && (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_16) && (p_data->search_res.srvc_id.uuid.uuid.uuid16 == XREMOTE_SERVICE116_UUID)) {

            ESP_LOGI(AP_TAG, "Xiaomi Service2 A found");
            gl_profile_tab[PROFILE_A_APP_ID].service1_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[PROFILE_A_APP_ID].service1_end_handle = p_data->search_res.end_handle;
        } else if ((DEV_TYPA > 63) && (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_16) && (p_data->search_res.srvc_id.uuid.uuid.uuid16 == XREMOTE_SERVICE216_UUID)) {

            ESP_LOGI(AP_TAG, "Xiaomi Service3 A found");
//            get_servera = true;
            gl_profile_tab[PROFILE_A_APP_ID].service2_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[PROFILE_A_APP_ID].service2_end_handle = p_data->search_res.end_handle;
        } else if ((DEV_TYPA > 63) && (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_16) && (p_data->search_res.srvc_id.uuid.uuid.uuid16 == XREMOTE_SERVICE316_UUID)) {

            ESP_LOGI(AP_TAG, "Xiaomi Service4 A found");
            get_servera = true;
            gl_profile_tab[PROFILE_A_APP_ID].service3_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[PROFILE_A_APP_ID].service3_end_handle = p_data->search_res.end_handle;
	}
        break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT:
        if (p_data->search_cmpl.status != ESP_GATT_OK){
	conerr = 1;
            ESP_LOGE(AP_TAG, "Search service failed, error status = %x", p_data->search_cmpl.status);
            break;
        }
        if(p_data->search_cmpl.searched_service_source == ESP_GATT_SERVICE_FROM_REMOTE_DEVICE) {
//            ESP_LOGI(AP_TAG, "Get service information from remote device");
        } else if (p_data->search_cmpl.searched_service_source == ESP_GATT_SERVICE_FROM_NVS_FLASH) {
//            ESP_LOGI(AP_TAG, "Get service information from flash");
        } else {
//            ESP_LOGI(AP_TAG, "Unknown service source");
        }
        if (get_servera){
            uint16_t count = 0;
            uint16_t count1 = 0;
            esp_gatt_status_t status = esp_ble_gattc_get_attr_count( gattc_if,
                                                                     p_data->search_cmpl.conn_id,
                                                                     ESP_GATT_DB_CHARACTERISTIC,
                                                                     gl_profile_tab[PROFILE_A_APP_ID].service_start_handle,
                                                                     gl_profile_tab[PROFILE_A_APP_ID].service_end_handle,
                                                                     INVALID_HANDLE,
                                                                     &count);
            if (status != ESP_GATT_OK){
		conerr = 1;
                ESP_LOGE(AP_TAG, "esp_ble_gattc_get_attr_count error");
            }

	if (DEV_TYPA > 63) {
            status = esp_ble_gattc_get_attr_count( gattc_if,
                                                                     p_data->search_cmpl.conn_id,
                                                                     ESP_GATT_DB_CHARACTERISTIC,
                                                                     gl_profile_tab[PROFILE_A_APP_ID].service1_start_handle,
                                                                     gl_profile_tab[PROFILE_A_APP_ID].service1_end_handle,
                                                                     INVALID_HANDLE,
                                                                     &count1);
            if (status != ESP_GATT_OK){
		conerr = 1;
                ESP_LOGE(AP_TAG, "esp_ble_gattc_get_attr1_count error");
            }
	count = count + count1;
	count1 = 0;		
            status = esp_ble_gattc_get_attr_count( gattc_if,
                                                                     p_data->search_cmpl.conn_id,
                                                                     ESP_GATT_DB_CHARACTERISTIC,
                                                                     gl_profile_tab[PROFILE_A_APP_ID].service2_start_handle,
                                                                     gl_profile_tab[PROFILE_A_APP_ID].service2_end_handle,
                                                                     INVALID_HANDLE,
                                                                     &count1);
            if (status != ESP_GATT_OK){
		conerr = 1;
                ESP_LOGE(AP_TAG, "esp_ble_gattc_get_attr2_count error");
            }
	count = count + count1;
	count1 = 0;		
            status = esp_ble_gattc_get_attr_count( gattc_if,
                                                                     p_data->search_cmpl.conn_id,
                                                                     ESP_GATT_DB_CHARACTERISTIC,
                                                                     gl_profile_tab[PROFILE_A_APP_ID].service3_start_handle,
                                                                     gl_profile_tab[PROFILE_A_APP_ID].service3_end_handle,
                                                                     INVALID_HANDLE,
                                                                     &count1);
            if (status != ESP_GATT_OK){
		conerr = 1;
                ESP_LOGE(AP_TAG, "esp_ble_gattc_get_attr3_count error");
            }
	count = count + count1;
	}

            if (count > 0){
		char_elem_result_a = (esp_gattc_char_elem_t *)malloc(sizeof(esp_gattc_char_elem_t) * count);
                if (!char_elem_result_a){
		conerr = 1;
                    ESP_LOGE(AP_TAG, "Gattc no mem");
                }else{
		if (DEV_TYPA < 64) {
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_A_APP_ID].service_start_handle,
                                                             gl_profile_tab[PROFILE_A_APP_ID].service_end_handle,
                                                             remote_filter_rxchar_uuid,
                                                             (char_elem_result_a),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_rxchar_by_uuid A error");
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
                                                             p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_A_APP_ID].service_start_handle,
                                                             gl_profile_tab[PROFILE_A_APP_ID].service_end_handle,
                                                             remote_filter_txchar_uuid,
                                                             (char_elem_result_a+1),
                                                             &count);

                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_txchar_by_uuid A error");
                    }
		} else {
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_A_APP_ID].service_start_handle,
                                                             gl_profile_tab[PROFILE_A_APP_ID].service_end_handle,
                                                             xremote_filter_status_uuid,
                                                             (char_elem_result_a),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_status_by_uuid A error");
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_A_APP_ID].service1_start_handle,
                                                             gl_profile_tab[PROFILE_A_APP_ID].service1_end_handle,
                                                             xremote_filter_authinit_uuid,
                                                             (char_elem_result_a+1),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_authinit_by_uuid A error");
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_A_APP_ID].service1_start_handle,
                                                             gl_profile_tab[PROFILE_A_APP_ID].service1_end_handle,
                                                             xremote_filter_auth_uuid,
                                                             (char_elem_result_a+2),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_auth_by_uuid A error");
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_A_APP_ID].service1_start_handle,
                                                             gl_profile_tab[PROFILE_A_APP_ID].service1_end_handle,
                                                             xremote_filter_ver_uuid,
                                                             (char_elem_result_a+3),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_ver_by_uuid A error");
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_A_APP_ID].service_start_handle,
                                                             gl_profile_tab[PROFILE_A_APP_ID].service_end_handle,
                                                             xremote_filter_setup_uuid,
                                                             (char_elem_result_a+4),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_setup_by_uuid A error");
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_A_APP_ID].service_start_handle,
                                                             gl_profile_tab[PROFILE_A_APP_ID].service_end_handle,
                                                             xremote_filter_time_uuid,
                                                             (char_elem_result_a+5),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_time_by_uuid A error");
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_A_APP_ID].service_start_handle,
                                                             gl_profile_tab[PROFILE_A_APP_ID].service_end_handle,
                                                             xremote_filter_boil_uuid,
                                                             (char_elem_result_a+6),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_boil_by_uuid A error");
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_A_APP_ID].service2_start_handle,
                                                             gl_profile_tab[PROFILE_A_APP_ID].service2_end_handle,
                                                             xremote_filter_mcuver_uuid,
                                                             (char_elem_result_a+7),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_mcuver_by_uuid A error");
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_A_APP_ID].service3_start_handle,
                                                             gl_profile_tab[PROFILE_A_APP_ID].service3_end_handle,
                                                             xremote_filter_update_uuid,
                                                             (char_elem_result_a+8),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_update_by_uuid A error");
                    }

		}

                    /*  Every service have only one char in our 'ESP_GATTS_DEMO' demo, so we used first 'char_elem_result' */
                    if (count > 0 && (char_elem_result_a[0].properties & ESP_GATT_CHAR_PROP_BIT_NOTIFY)){
                        gl_profile_tab[PROFILE_A_APP_ID].rxchar_handle = char_elem_result_a[0].char_handle;
                        esp_ble_gattc_register_for_notify (gattc_if, gl_profile_tab[PROFILE_A_APP_ID].remote_bda, char_elem_result_a[0].char_handle);
                        gl_profile_tab[PROFILE_A_APP_ID].txchar_handle = char_elem_result_a[1].char_handle;
		if (DEV_TYPA > 63) {
                        gl_profile_tab[PROFILE_A_APP_ID].auth_handle = char_elem_result_a[2].char_handle;
                        gl_profile_tab[PROFILE_A_APP_ID].ver_handle = char_elem_result_a[3].char_handle;
                        gl_profile_tab[PROFILE_A_APP_ID].setup_handle = char_elem_result_a[4].char_handle;
                        gl_profile_tab[PROFILE_A_APP_ID].time_handle = char_elem_result_a[5].char_handle;
                        gl_profile_tab[PROFILE_A_APP_ID].boil_handle = char_elem_result_a[6].char_handle;
                        gl_profile_tab[PROFILE_A_APP_ID].mcuver_handle = char_elem_result_a[7].char_handle;
                        gl_profile_tab[PROFILE_A_APP_ID].update_handle = char_elem_result_a[8].char_handle;
ESP_LOGI(AP_TAG, "Update char handle = %x", gl_profile_tab[PROFILE_A_APP_ID].update_handle);
		}
			ESP_LOGI(AP_TAG, "Register_for_notify A");
                    }
                }
                /* free char_elem_result */
                free(char_elem_result_a);
            }else{
		conerr = 1;
                ESP_LOGE(AP_TAG, "No char A found");
            }
        }
         break;
    case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
        if (p_data->reg_for_notify.status != ESP_GATT_OK){
	conerr = 1;
            ESP_LOGE(AP_TAG, "REG FOR NOTIFY A failed: error status = %d", p_data->reg_for_notify.status);
        } else {
            uint16_t count = 0;
            uint16_t notify_en = 1; 
	if ((DEV_TYPA < 64) || (xbtautha != 1)) {
            esp_gatt_status_t ret_status = esp_ble_gattc_get_attr_count( gattc_if,
                                                                         gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                                                         ESP_GATT_DB_DESCRIPTOR,
                                                                         gl_profile_tab[PROFILE_A_APP_ID].service_start_handle,
                                                                         gl_profile_tab[PROFILE_A_APP_ID].service_end_handle,
                                                                         gl_profile_tab[PROFILE_A_APP_ID].rxchar_handle,
                                                                         &count);
            if (ret_status != ESP_GATT_OK){
		conerr = 1;
                ESP_LOGE(AP_TAG, "Get_attr_count A error");
            }
            if (count > 0){
                descr_elem_result_a = malloc(sizeof(esp_gattc_descr_elem_t) * count);
                if (!descr_elem_result_a){
                    ESP_LOGE(AP_TAG, "malloc error, gattc no mem");
                }else{
                    ret_status = esp_ble_gattc_get_descr_by_char_handle( gattc_if,
                                                                         gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                                                         p_data->reg_for_notify.handle,
                                                                         notify_descr_uuid,
                                                                         descr_elem_result_a,
                                                                         &count);
                    if (ret_status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_descr_by_char_handle A error");
                    }
                    /* Every char has only one descriptor in our 'ESP_GATTS_DEMO' demo, so we used first 'descr_elem_result' */
                    if (count > 0 && descr_elem_result_a[0].uuid.len == ESP_UUID_LEN_16 && descr_elem_result_a[0].uuid.uuid.uuid16 == ESP_GATT_UUID_CHAR_CLIENT_CONFIG){
                        ret_status = esp_ble_gattc_write_char_descr( gattc_if,
                                                                     gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                                                     descr_elem_result_a[0].handle,
                                                                     sizeof(notify_en),
                                                                     (uint8_t *)&notify_en,
                                                                     ESP_GATT_WRITE_TYPE_RSP,
                                                                     ESP_GATT_AUTH_REQ_NONE);
                    }
                    if (ret_status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Write_char_descr A error");
                    }
                    /* free descr_elem_result */
                    free(descr_elem_result_a);
                }
            } else {
		conerr = 1;
                ESP_LOGE(AP_TAG, "Decsr A not found");
            }
	} else if ((DEV_TYPA > 63) && xbtautha) {
            count = 0;
            notify_en = 1; 
            esp_gatt_status_t ret_status = esp_ble_gattc_get_attr_count( gattc_if,
                                                                         gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                                                         ESP_GATT_DB_DESCRIPTOR,
                                                                         gl_profile_tab[PROFILE_A_APP_ID].service1_start_handle,
                                                                         gl_profile_tab[PROFILE_A_APP_ID].service1_end_handle,
                                                                         gl_profile_tab[PROFILE_A_APP_ID].auth_handle,
                                                                         &count);
            if (ret_status != ESP_GATT_OK){
		conerr = 1;
                ESP_LOGE(AP_TAG, "Get_attr_count1 A error");
            }
            if (count > 0){
                descr_elem_result_a = malloc(sizeof(esp_gattc_descr_elem_t) * count);
                if (!descr_elem_result_a){
		conerr = 1;
                    ESP_LOGE(AP_TAG, "malloc error, gattc no mem");
                }else{
                    ret_status = esp_ble_gattc_get_descr_by_char_handle( gattc_if,
                                                                         gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                                                         p_data->reg_for_notify.handle,
                                                                         notify_descr_uuid,
                                                                         descr_elem_result_a,
                                                                         &count);
                    if (ret_status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_descr_by_char_handle A error");
                    }
                    /* Every char has only one descriptor in our 'ESP_GATTS_DEMO' demo, so we used first 'descr_elem_result' */
                    if (count > 0 && descr_elem_result_a[0].uuid.len == ESP_UUID_LEN_16 && descr_elem_result_a[0].uuid.uuid.uuid16 == ESP_GATT_UUID_CHAR_CLIENT_CONFIG){
                        ret_status = esp_ble_gattc_write_char_descr( gattc_if,
                                                                     gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                                                     descr_elem_result_a[0].handle,
                                                                     sizeof(notify_en),
                                                                     (uint8_t *)&notify_en,
                                                                     ESP_GATT_WRITE_TYPE_RSP,
                                                                     ESP_GATT_AUTH_REQ_NONE);
                    }
                    if (ret_status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Write_char_descr1 A error");
                    }
                    /* free descr_elem_result */
                    free(descr_elem_result_a);
                }
            } else {
		conerr = 1;
                ESP_LOGE(AP_TAG, "Decsr1 A not found");
            }
	}	
        }
        break;
    }

    case ESP_GATTC_READ_CHAR_EVT:
ESP_LOGI(AP_TAG, "ESP_GATTC_READ_CHAR_EVT, receive read value:");
esp_log_buffer_hex(AP_TAG, p_data->read.value, p_data->read.value_len);
ESP_LOGI(AP_TAG, "Read char handle = %x", p_data->read.handle);

	int length = p_data->read.value_len;
	if (length > BLE_INPUT_BUFFSIZE)
	length = BLE_INPUT_BUFFSIZE;
	if (length > 0) {
        memcpy(readDataA, p_data->read.value, length);
	}
	readDataALen = length;
	readDataHandleA = p_data->read.handle;

/*
if (p_data->read.handle == 0x2a) {
        esp_gatt_status_t ret_status = esp_ble_gattc_read_char( gattc_if,
                                  gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                  gl_profile_tab[PROFILE_A_APP_ID].mcuver_handle,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Read_mcuver_req A error");
			conerr = 1;
                    }  ESP_LOGI(AP_TAG, "Read_read_mcuver_req A Ok");
	}
*/
	break;
    case ESP_GATTC_NOTIFY_EVT:
        if ((p_data->notify.is_notify) && (p_data->notify.handle == gl_profile_tab[PROFILE_A_APP_ID].rxchar_handle)) {
	if (btauthoriza) {
	int length = p_data->notify.value_len;
	if (length > BLE_INPUT_BUFFSIZE)
	length = BLE_INPUT_BUFFSIZE;
	if (length > 0) {
        memcpy(notifyDataA, p_data->notify.value, length);
	}
	notifyDataALen = length;
	} else {
	if ((!memcmp(p_data->notify.value, "\x55\x00\xff\x01\xaa", 5))||(!memcmp(p_data->notify.value, "\x55\x00\xff\x02\xaa", 5))) {
	ESP_LOGI(AP_TAG, "Authorize A Redmond ok");
	t_lasta_us = ~t_lasta_us;
	t_ppcona_us = esp_timer_get_time();
	btauthoriza = true;
	r4sAConnErr = 0;
	r4sAcounter = 1;	
	f_SyncA = 1;
	NumConnA++;
	if (!NumConnA) NumConnA--;
	bin2hex(gl_profile_tab[PROFILE_A_APP_ID].remote_bda, tBLEAddrA,6);
	strcpy(DEV_NAMEA,REQ_NAMEA);
        MqttPubSubA(mqttConnected);	
	}
	}
	} else if (!btauthoriza && (p_data->notify.is_notify) && (p_data->notify.handle == gl_profile_tab[PROFILE_A_APP_ID].auth_handle)) {
	if ((p_data->notify.value_len == 12) && (xbtautha == 2)) {
	uint8_t buff2[16];
        uint8_t xiv_char_data[12] = { 0x55,0x00,0xff,0xb6,0x2c,0x27,0xb3,0xb8,0xac,0x5a,0xef,0xaa};  //auth string
	xiv_char_data[5] = xiv_char_data[5] + R4SNUM;  //for each gate number different auth id

	mixA(gl_profile_tab[PROFILE_A_APP_ID].remote_bda, buff1, MiAKettleID);
	cipherInit(buff1, bufftab, 8);
	cipherCrypt(p_data->notify.value, buff2, bufftab, 12);
	mixB(gl_profile_tab[PROFILE_A_APP_ID].remote_bda, buff1, MiAKettleID);
	cipherInit(buff1, bufftab, 8);
	cipherCrypt(buff2, buff1, bufftab, 12);
	if (!memcmp(xiv_char_data, buff1, 12)) {
	buff2[0] = 0x92;
	buff2[1] = 0xab;
	buff2[2] = 0x54;
	buff2[3] = 0xfa;
	cipherInit(xiv_char_data, bufftab, 12);
	cipherCrypt(buff2, buff1, bufftab, 4);
        esp_gatt_status_t ret_status = esp_ble_gattc_write_char( gattc_if,
                                  gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                  gl_profile_tab[PROFILE_A_APP_ID].auth_handle,
                                  4,
                                  buff1,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Write_auth_xi_ack A error");
                    }  ESP_LOGI(AP_TAG, "Write_auth_xi_ack A Ok");
	buff2[0] = 0x03;
	buff2[1] = 0x01;
        ret_status = esp_ble_gattc_write_char( gattc_if,
                                  gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                  gl_profile_tab[PROFILE_A_APP_ID].setup_handle,
                                  2,
                                  buff2,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
                        ESP_LOGE(AP_TAG, "Write_setup_xi A error");
                    }  ESP_LOGI(AP_TAG, "Write_setup_xi A Ok");
	buff2[0] = 0x00;
        ret_status = esp_ble_gattc_write_char( gattc_if,
                                  gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                  gl_profile_tab[PROFILE_A_APP_ID].boil_handle,
                                  1,
                                  buff2,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
                        ESP_LOGE(AP_TAG, "Write_boil_xi A error");
                    }  ESP_LOGI(AP_TAG, "Write_boil_xi A Ok");
	buff2[0] = 0x17;
        ret_status = esp_ble_gattc_write_char( gattc_if,
                                  gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                  gl_profile_tab[PROFILE_A_APP_ID].time_handle,
                                  1,
                                  buff2,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
                        ESP_LOGE(AP_TAG, "Write_time_xi A error");
                    }  ESP_LOGI(AP_TAG, "Write_time_xi A Ok");

///*
        ret_status = esp_ble_gattc_read_char( gattc_if,
                                  gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                  gl_profile_tab[PROFILE_A_APP_ID].update_handle,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
//			conerr = 1;
                        ESP_LOGE(AP_TAG, "Read_ver_req A error");
                    }  ESP_LOGI(AP_TAG, "Read_read_ver_req A Ok");
//*/

	ESP_LOGI(AP_TAG, "Authorize A Xiaomi ok");
	t_lasta_us = ~t_lasta_us;
	t_ppcona_us = esp_timer_get_time();
	btauthoriza = true;
	r4sAConnErr = 0;
	NumConnA++;
	if (!NumConnA) NumConnA--;
	bin2hex(gl_profile_tab[PROFILE_A_APP_ID].remote_bda, tBLEAddrA,6);
	strcpy(DEV_NAMEA,REQ_NAMEA);
        MqttPubSubA(mqttConnected);	
	esp_ble_gattc_register_for_notify (gattc_if, gl_profile_tab[PROFILE_A_APP_ID].remote_bda, gl_profile_tab[PROFILE_A_APP_ID].rxchar_handle);

	} else {
	conerr = 1;
	ESP_LOGI(AP_TAG, "Invalid A Xiaomi product Id");
	if ((DEV_TYPA == 95) && (MiAKettleID < 10000)) MiAKettleID++;
	}
	}
        } if (p_data->notify.is_notify) {
//            ESP_LOGI(AP_TAG, "GATTC_NOTIFY_EVT_A, Handle = %x, Notify value:",p_data->notify.handle);
        }else{
//            ESP_LOGI(AP_TAG, "GATTC_NOTIFY_EVT_A, Indicate value:");
        }
//        esp_log_buffer_hex(AP_TAG, p_data->notify.value, p_data->notify.value_len);

        break;



    case ESP_GATTC_WRITE_DESCR_EVT:
        if (p_data->write.status != ESP_GATT_OK){
	conerr = 1;
            ESP_LOGE(AP_TAG, "Write descr A failed, error status = %x", p_data->write.status);
	conerr = 1;
            break;
        }
	uint8_t  write_char_crypt_data[16];
	int  write_char_data_len = 12;
        uint8_t write_char_data[12] = { 0x55,0x00,0xff,0xb6,0x2c,0x27,0xb3,0xb8,0xac,0x5a,0xef,0xaa};  //auth string
	write_char_data[5] = write_char_data[5] + R4SNUM;  //for each gate number different auth id
	if ((DEV_TYPA < 64) || !xbtautha) {
	if (DEV_TYPA > 63) {
        write_char_data[0] = 0x90;
        write_char_data[1] = 0xca;
        write_char_data[2] = 0x85;
        write_char_data[3] = 0xde;
	write_char_data_len = 4;
	}
        esp_gatt_status_t ret_status = esp_ble_gattc_write_char( gattc_if,
                                  gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                  gl_profile_tab[PROFILE_A_APP_ID].txchar_handle,
                                  write_char_data_len,
                                  write_char_data,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
                        ESP_LOGE(AP_TAG, "Write_auth A error");
			conerr = 1;
                    }  ESP_LOGI(AP_TAG, "Write_auth A Ok");
	if (DEV_TYPA > 63) {
	xbtautha = 1;
	esp_ble_gattc_register_for_notify (gattc_if, gl_profile_tab[PROFILE_A_APP_ID].remote_bda, gl_profile_tab[PROFILE_A_APP_ID].auth_handle);
	}
	} else if ((DEV_TYPA > 63) && (xbtautha == 1)) {
	xbtautha = 2;
	mixA(gl_profile_tab[PROFILE_A_APP_ID].remote_bda, buff1, MiAKettleID);
	cipherInit(buff1, bufftab, 8);
	cipherCrypt(write_char_data, write_char_crypt_data, bufftab, 12);
        esp_gatt_status_t ret_status = esp_ble_gattc_write_char( gattc_if,
                                  gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                  gl_profile_tab[PROFILE_A_APP_ID].auth_handle,
                                  write_char_data_len,
                                  write_char_crypt_data,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
                        ESP_LOGE(AP_TAG, "Write_auth_mi A error");
			conerr = 1;
                    }  ESP_LOGI(AP_TAG, "Write_auth_mi A Ok");
	}
        break;
    case ESP_GATTC_SRVC_CHG_EVT: {
        esp_bd_addr_t bda;
        memcpy(bda, p_data->srvc_chg.remote_bda, sizeof(esp_bd_addr_t));
        ESP_LOGI(AP_TAG, "ESP_GATTC_SRVC_CHG_EVT, bd_addr:");
        esp_log_buffer_hex(AP_TAG, bda, sizeof(esp_bd_addr_t));
        break;
    }
    case ESP_GATTC_WRITE_CHAR_EVT:
        if (p_data->write.status != ESP_GATT_OK){
	conerr = 1;
            ESP_LOGE(AP_TAG, "Write char A failed, error status = %x", p_data->write.status);
            break;
        }
//	if (btauthoriza) start_scan();
	start_scan();
        break;

    case ESP_GATTC_DISCONNECT_EVT:
        if (memcmp(p_data->disconnect.remote_bda, gl_profile_tab[PROFILE_A_APP_ID].remote_bda, 6) == 0){
	btopena = false;
        btconnecta = false;
        btauthoriza = false;
        get_servera = false;
        xbtautha = 0;
        ESP_LOGI(AP_TAG, "ESP_GATTC_DISCONNECT_EVT, reason = %d", p_data->disconnect.reason);
	if ((mqttConnected) &&(tBLEAddrA[0])) {
            char ldata[32];
            strcpy(ldata,MQTT_BASE_TOPIC);
            strcat(ldata,"/");
            strcat(ldata,tBLEAddrA);
            strcat(ldata,"/status");
            esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
	}
//	Isscanning = false;
	start_scan();
	}
        break;
    default:
        break;
    }
//	if (conerr && btopena) esp_ble_gattc_close(gl_profile_tab[PROFILE_A_APP_ID].gattc_if,gl_profile_tab[PROFILE_A_APP_ID].conn_id);
	if ((conerr || f_update) && btopena) esp_ble_gap_disconnect(gl_profile_tab[PROFILE_A_APP_ID].remote_bda);

}
//*** A ************************

//*** B ********************************************
static void gattc_profile_b_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    esp_ble_gattc_cb_param_t *p_data = (esp_ble_gattc_cb_param_t *)param;
	uint8_t  conerr = 0;
	uint8_t  buff1[16];
	uint8_t  bufftab[256];

    switch (event) {
    case ESP_GATTC_REG_EVT:
//	ESP_LOGI(AP_TAG, "REG_EVT");

	esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV,ESP_PWR_LVL_P9); // for more power???
	esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT,ESP_PWR_LVL_P9); // for more power???

        esp_err_t scan_ret = esp_ble_gap_set_scan_params(&ble_scan_params);
        if (scan_ret){
	ESP_LOGE(AP_TAG, "Set scan params error, error code = %x", scan_ret);
        }
	break;
    case ESP_GATTC_CONNECT_EVT:
	break;
    case ESP_GATTC_OPEN_EVT:
	xbtauthb = 0;
        if (param->open.status != ESP_GATT_OK) {
	ESP_LOGE(AP_TAG, "Open B failed, status %d", p_data->open.status);
	btopenb = false;
        btconnectb = false;
//	Isscanning = false;
	start_scan();
	break;
        }
        memcpy(gl_profile_tab[PROFILE_B_APP_ID].remote_bda, p_data->open.remote_bda, 6);
        gl_profile_tab[PROFILE_B_APP_ID].conn_id = p_data->open.conn_id;
        ESP_LOGI(AP_TAG, "Open B success, conn_id %d, if %d, status %d, mtu %d", p_data->open.conn_id, gattc_if, p_data->open.status, p_data->open.mtu);
        ESP_LOGI(AP_TAG, "REMOTE BDA B:");
        esp_log_buffer_hex(AP_TAG, p_data->open.remote_bda, sizeof(esp_bd_addr_t));
        esp_err_t mtu_ret = esp_ble_gattc_send_mtu_req (gattc_if, p_data->open.conn_id);
	btopenb = true;
        if (mtu_ret){
            ESP_LOGE(AP_TAG, "config MTU B error, error code = %x", mtu_ret);
        }
	if (DEV_TYPB == 64) MiBKettleID = 275;
	if (DEV_TYPB == 65) MiBKettleID = 131;
	if (DEV_TYPB == 66) MiBKettleID = 1116;
        break;

    case ESP_GATTC_DIS_SRVC_CMPL_EVT:
        if (param->dis_srvc_cmpl.status != ESP_GATT_OK){
	conerr = 1;
	ESP_LOGE(AP_TAG, "Discover service B failed, status %d", param->dis_srvc_cmpl.status);
	break;
        }
        ESP_LOGI(AP_TAG, "Discover service B complete conn_id %d", param->dis_srvc_cmpl.conn_id);
        if (DEV_TYPB < 64) esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id, &remote_filter_service_uuid);
	else {
	esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id, &xremote_filter_service_uuid);
	esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id, &xremote_filter_service1_uuid);
	esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id, &xremote_filter_service2_uuid);
	esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id, &xremote_filter_service3_uuid);
	}
        break;
    case ESP_GATTC_CFG_MTU_EVT:
        if (param->cfg_mtu.status != ESP_GATT_OK){
	conerr = 1;
	ESP_LOGE(AP_TAG,"Config mtu failed, error status = %x", param->cfg_mtu.status);
        }
//	ESP_LOGI(AP_TAG, "ESP_GATTC_CFG_MTU_EVT, Status %d, MTU %d, conn_id %d", param->cfg_mtu.status, param->cfg_mtu.mtu, param->cfg_mtu.conn_id);
	break;
    case ESP_GATTC_SEARCH_RES_EVT: {
        ESP_LOGI(AP_TAG, "SEARCH RES B: conn_id = %x is primary service %d", p_data->search_res.conn_id, p_data->search_res.is_primary);
        ESP_LOGI(AP_TAG, "start handle %d end handle %d current handle value %d", p_data->search_res.start_handle, p_data->search_res.end_handle, p_data->search_res.srvc_id.inst_id);

//esp_log_buffer_hex(AP_TAG,p_data->search_res.srvc_id.uuid.uuid.uuid128,16);
//esp_log_buffer_hex(AP_TAG,xremote_filter_service11_uuid.uuid.uuid128,16);

        if ((DEV_TYPB < 64) && (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_128) && (bt_compare_UUID128(p_data->search_res.srvc_id.uuid.uuid.uuid128, remote_filter_service_uuid.uuid.uuid128))) {

            ESP_LOGI(AP_TAG, "Redmond Service B found");
            get_serverb = true;
            gl_profile_tab[PROFILE_B_APP_ID].service_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[PROFILE_B_APP_ID].service_end_handle = p_data->search_res.end_handle;
        } else if ((DEV_TYPB > 63) && (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_128) && (bt_compare_UUID128(p_data->search_res.srvc_id.uuid.uuid.uuid128, xremote_filter_service_uuid.uuid.uuid128))) {

            ESP_LOGI(AP_TAG, "Xiaomi Service1 B found");
            gl_profile_tab[PROFILE_B_APP_ID].service_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[PROFILE_B_APP_ID].service_end_handle = p_data->search_res.end_handle;
        } else if ((DEV_TYPB > 63) && (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_16) && (p_data->search_res.srvc_id.uuid.uuid.uuid16 == XREMOTE_SERVICE116_UUID)) {

            ESP_LOGI(AP_TAG, "Xiaomi Service2 B found");
            gl_profile_tab[PROFILE_B_APP_ID].service1_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[PROFILE_B_APP_ID].service1_end_handle = p_data->search_res.end_handle;
        } else if ((DEV_TYPB > 63) && (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_16) && (p_data->search_res.srvc_id.uuid.uuid.uuid16 == XREMOTE_SERVICE216_UUID)) {

            ESP_LOGI(AP_TAG, "Xiaomi Service3 B found");
//            get_serverb = true;
            gl_profile_tab[PROFILE_B_APP_ID].service2_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[PROFILE_B_APP_ID].service2_end_handle = p_data->search_res.end_handle;
        } else if ((DEV_TYPB > 63) && (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_16) && (p_data->search_res.srvc_id.uuid.uuid.uuid16 == XREMOTE_SERVICE316_UUID)) {

            ESP_LOGI(AP_TAG, "Xiaomi Service4 B found");
            get_serverb = true;
            gl_profile_tab[PROFILE_B_APP_ID].service3_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[PROFILE_B_APP_ID].service3_end_handle = p_data->search_res.end_handle;
	}
        break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT:
        if (p_data->search_cmpl.status != ESP_GATT_OK){
	conerr = 1;
            ESP_LOGE(AP_TAG, "Search service failed, error status = %x", p_data->search_cmpl.status);
            break;
        }
        if(p_data->search_cmpl.searched_service_source == ESP_GATT_SERVICE_FROM_REMOTE_DEVICE) {
//            ESP_LOGI(AP_TAG, "Get service information from remote device");
        } else if (p_data->search_cmpl.searched_service_source == ESP_GATT_SERVICE_FROM_NVS_FLASH) {
//            ESP_LOGI(AP_TAG, "Get service information from flash");
        } else {
//            ESP_LOGI(AP_TAG, "Unknown service source");
        }
        if (get_serverb){
            uint16_t count = 0;
            uint16_t count1 = 0;
            esp_gatt_status_t status = esp_ble_gattc_get_attr_count( gattc_if,
                                                                     p_data->search_cmpl.conn_id,
                                                                     ESP_GATT_DB_CHARACTERISTIC,
                                                                     gl_profile_tab[PROFILE_B_APP_ID].service_start_handle,
                                                                     gl_profile_tab[PROFILE_B_APP_ID].service_end_handle,
                                                                     INVALID_HANDLE,
                                                                     &count);
            if (status != ESP_GATT_OK){
		conerr = 1;
                ESP_LOGE(AP_TAG, "esp_ble_gattc_get_attr_count error");
            }

	if (DEV_TYPB > 63) {
            status = esp_ble_gattc_get_attr_count( gattc_if,
                                                                     p_data->search_cmpl.conn_id,
                                                                     ESP_GATT_DB_CHARACTERISTIC,
                                                                     gl_profile_tab[PROFILE_B_APP_ID].service1_start_handle,
                                                                     gl_profile_tab[PROFILE_B_APP_ID].service1_end_handle,
                                                                     INVALID_HANDLE,
                                                                     &count1);
            if (status != ESP_GATT_OK){
		conerr = 1;
                ESP_LOGE(AP_TAG, "esp_ble_gattc_get_attr1_count error");
            }
	count = count + count1;
	count1 = 0;		
            status = esp_ble_gattc_get_attr_count( gattc_if,
                                                                     p_data->search_cmpl.conn_id,
                                                                     ESP_GATT_DB_CHARACTERISTIC,
                                                                     gl_profile_tab[PROFILE_B_APP_ID].service2_start_handle,
                                                                     gl_profile_tab[PROFILE_B_APP_ID].service2_end_handle,
                                                                     INVALID_HANDLE,
                                                                     &count1);
            if (status != ESP_GATT_OK){
		conerr = 1;
                ESP_LOGE(AP_TAG, "esp_ble_gattc_get_attr2_count error");
            }
	count = count + count1;
	count1 = 0;		
            status = esp_ble_gattc_get_attr_count( gattc_if,
                                                                     p_data->search_cmpl.conn_id,
                                                                     ESP_GATT_DB_CHARACTERISTIC,
                                                                     gl_profile_tab[PROFILE_B_APP_ID].service3_start_handle,
                                                                     gl_profile_tab[PROFILE_B_APP_ID].service3_end_handle,
                                                                     INVALID_HANDLE,
                                                                     &count1);
            if (status != ESP_GATT_OK){
		conerr = 1;
                ESP_LOGE(AP_TAG, "esp_ble_gattc_get_attr3_count error");
            }
	count = count + count1;
	}

            if (count > 0){
		char_elem_result_b = (esp_gattc_char_elem_t *)malloc(sizeof(esp_gattc_char_elem_t) * count);
                if (!char_elem_result_b){
		conerr = 1;
                    ESP_LOGE(AP_TAG, "Gattc no mem");
                }else{
		if (DEV_TYPB < 64) {
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_B_APP_ID].service_start_handle,
                                                             gl_profile_tab[PROFILE_B_APP_ID].service_end_handle,
                                                             remote_filter_rxchar_uuid,
                                                             (char_elem_result_b),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_rxchar_by_uuid B error");
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
                                                             p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_B_APP_ID].service_start_handle,
                                                             gl_profile_tab[PROFILE_B_APP_ID].service_end_handle,
                                                             remote_filter_txchar_uuid,
                                                             (char_elem_result_b+1),
                                                             &count);

                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_txchar_by_uuid B error");
                    }
		} else {
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_B_APP_ID].service_start_handle,
                                                             gl_profile_tab[PROFILE_B_APP_ID].service_end_handle,
                                                             xremote_filter_status_uuid,
                                                             (char_elem_result_b),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_status_by_uuid B error");
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_B_APP_ID].service1_start_handle,
                                                             gl_profile_tab[PROFILE_B_APP_ID].service1_end_handle,
                                                             xremote_filter_authinit_uuid,
                                                             (char_elem_result_b+1),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_authinit_by_uuid B error");
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_B_APP_ID].service1_start_handle,
                                                             gl_profile_tab[PROFILE_B_APP_ID].service1_end_handle,
                                                             xremote_filter_auth_uuid,
                                                             (char_elem_result_b+2),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_auth_by_uuid B error");
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_B_APP_ID].service1_start_handle,
                                                             gl_profile_tab[PROFILE_B_APP_ID].service1_end_handle,
                                                             xremote_filter_ver_uuid,
                                                             (char_elem_result_b+3),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_ver_by_uuid B error");
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_B_APP_ID].service_start_handle,
                                                             gl_profile_tab[PROFILE_B_APP_ID].service_end_handle,
                                                             xremote_filter_setup_uuid,
                                                             (char_elem_result_b+4),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_setup_by_uuid B error");
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_B_APP_ID].service_start_handle,
                                                             gl_profile_tab[PROFILE_B_APP_ID].service_end_handle,
                                                             xremote_filter_time_uuid,
                                                             (char_elem_result_b+5),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_time_by_uuid B error");
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_B_APP_ID].service_start_handle,
                                                             gl_profile_tab[PROFILE_B_APP_ID].service_end_handle,
                                                             xremote_filter_boil_uuid,
                                                             (char_elem_result_b+6),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_boil_by_uuid B error");
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_B_APP_ID].service2_start_handle,
                                                             gl_profile_tab[PROFILE_B_APP_ID].service2_end_handle,
                                                             xremote_filter_mcuver_uuid,
                                                             (char_elem_result_b+7),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_mcuver_by_uuid B error");
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_B_APP_ID].service3_start_handle,
                                                             gl_profile_tab[PROFILE_B_APP_ID].service3_end_handle,
                                                             xremote_filter_update_uuid,
                                                             (char_elem_result_b+8),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_update_by_uuid B error");
                    }

		}

                    /*  Every service have only one char in our 'ESP_GATTS_DEMO' demo, so we used first 'char_elem_result' */
                    if (count > 0 && (char_elem_result_b[0].properties & ESP_GATT_CHAR_PROP_BIT_NOTIFY)){
                        gl_profile_tab[PROFILE_B_APP_ID].rxchar_handle = char_elem_result_b[0].char_handle;
                        esp_ble_gattc_register_for_notify (gattc_if, gl_profile_tab[PROFILE_B_APP_ID].remote_bda, char_elem_result_b[0].char_handle);
                        gl_profile_tab[PROFILE_B_APP_ID].txchar_handle = char_elem_result_b[1].char_handle;
		if (DEV_TYPB > 63) {
                        gl_profile_tab[PROFILE_B_APP_ID].auth_handle = char_elem_result_b[2].char_handle;
                        gl_profile_tab[PROFILE_B_APP_ID].ver_handle = char_elem_result_b[3].char_handle;
                        gl_profile_tab[PROFILE_B_APP_ID].setup_handle = char_elem_result_b[4].char_handle;
                        gl_profile_tab[PROFILE_B_APP_ID].time_handle = char_elem_result_b[5].char_handle;
                        gl_profile_tab[PROFILE_B_APP_ID].boil_handle = char_elem_result_b[6].char_handle;
                        gl_profile_tab[PROFILE_B_APP_ID].mcuver_handle = char_elem_result_b[7].char_handle;
                        gl_profile_tab[PROFILE_B_APP_ID].update_handle = char_elem_result_b[8].char_handle;
		}
			ESP_LOGI(AP_TAG, "Register_for_notify B");
                    }
                }
                /* free char_elem_result */
                free(char_elem_result_b);
            }else{
		conerr = 1;
                ESP_LOGE(AP_TAG, "No char B found");
            }
        }
         break;
    case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
        if (p_data->reg_for_notify.status != ESP_GATT_OK){
	conerr = 1;
            ESP_LOGE(AP_TAG, "REG FOR NOTIFY B failed: error status = %d", p_data->reg_for_notify.status);
        } else {
            uint16_t count = 0;
            uint16_t notify_en = 1; 
	if ((DEV_TYPB < 64) || (xbtauthb != 1)) {
            esp_gatt_status_t ret_status = esp_ble_gattc_get_attr_count( gattc_if,
                                                                         gl_profile_tab[PROFILE_B_APP_ID].conn_id,
                                                                         ESP_GATT_DB_DESCRIPTOR,
                                                                         gl_profile_tab[PROFILE_B_APP_ID].service_start_handle,
                                                                         gl_profile_tab[PROFILE_B_APP_ID].service_end_handle,
                                                                         gl_profile_tab[PROFILE_B_APP_ID].rxchar_handle,
                                                                         &count);
            if (ret_status != ESP_GATT_OK){
		conerr = 1;
                ESP_LOGE(AP_TAG, "Get_attr_count B error");
            }
            if (count > 0){
                descr_elem_result_b = malloc(sizeof(esp_gattc_descr_elem_t) * count);
                if (!descr_elem_result_b){
                    ESP_LOGE(AP_TAG, "malloc error, gattc no mem");
                }else{
                    ret_status = esp_ble_gattc_get_descr_by_char_handle( gattc_if,
                                                                         gl_profile_tab[PROFILE_B_APP_ID].conn_id,
                                                                         p_data->reg_for_notify.handle,
                                                                         notify_descr_uuid,
                                                                         descr_elem_result_b,
                                                                         &count);
                    if (ret_status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_descr_by_char_handle B error");
                    }
                    /* Every char has only one descriptor in our 'ESP_GATTS_DEMO' demo, so we used first 'descr_elem_result' */
                    if (count > 0 && descr_elem_result_b[0].uuid.len == ESP_UUID_LEN_16 && descr_elem_result_b[0].uuid.uuid.uuid16 == ESP_GATT_UUID_CHAR_CLIENT_CONFIG){
                        ret_status = esp_ble_gattc_write_char_descr( gattc_if,
                                                                     gl_profile_tab[PROFILE_B_APP_ID].conn_id,
                                                                     descr_elem_result_b[0].handle,
                                                                     sizeof(notify_en),
                                                                     (uint8_t *)&notify_en,
                                                                     ESP_GATT_WRITE_TYPE_RSP,
                                                                     ESP_GATT_AUTH_REQ_NONE);
                    }
                    if (ret_status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Write_char_descr B error");
                    }
                    /* free descr_elem_result */
                    free(descr_elem_result_b);
                }
            } else {
		conerr = 1;
                ESP_LOGE(AP_TAG, "Decsr B not found");
            }
	} else if ((DEV_TYPB > 63) && xbtauthb) {
            count = 0;
            notify_en = 1; 
            esp_gatt_status_t ret_status = esp_ble_gattc_get_attr_count( gattc_if,
                                                                         gl_profile_tab[PROFILE_B_APP_ID].conn_id,
                                                                         ESP_GATT_DB_DESCRIPTOR,
                                                                         gl_profile_tab[PROFILE_B_APP_ID].service1_start_handle,
                                                                         gl_profile_tab[PROFILE_B_APP_ID].service1_end_handle,
                                                                         gl_profile_tab[PROFILE_B_APP_ID].auth_handle,
                                                                         &count);
            if (ret_status != ESP_GATT_OK){
		conerr = 1;
                ESP_LOGE(AP_TAG, "Get_attr_count1 B error");
            }
            if (count > 0){
                descr_elem_result_b = malloc(sizeof(esp_gattc_descr_elem_t) * count);
                if (!descr_elem_result_b){
		conerr = 1;
                    ESP_LOGE(AP_TAG, "malloc error, gattc no mem");
                }else{
                    ret_status = esp_ble_gattc_get_descr_by_char_handle( gattc_if,
                                                                         gl_profile_tab[PROFILE_B_APP_ID].conn_id,
                                                                         p_data->reg_for_notify.handle,
                                                                         notify_descr_uuid,
                                                                         descr_elem_result_b,
                                                                         &count);
                    if (ret_status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_descr_by_char_handle B error");
                    }
                    /* Every char has only one descriptor in our 'ESP_GATTS_DEMO' demo, so we used first 'descr_elem_result' */
                    if (count > 0 && descr_elem_result_b[0].uuid.len == ESP_UUID_LEN_16 && descr_elem_result_b[0].uuid.uuid.uuid16 == ESP_GATT_UUID_CHAR_CLIENT_CONFIG){
                        ret_status = esp_ble_gattc_write_char_descr( gattc_if,
                                                                     gl_profile_tab[PROFILE_B_APP_ID].conn_id,
                                                                     descr_elem_result_b[0].handle,
                                                                     sizeof(notify_en),
                                                                     (uint8_t *)&notify_en,
                                                                     ESP_GATT_WRITE_TYPE_RSP,
                                                                     ESP_GATT_AUTH_REQ_NONE);
                    }
                    if (ret_status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Write_char_descr1 B error");
                    }
                    /* free descr_elem_result */
                    free(descr_elem_result_b);
                }
            } else {
		conerr = 1;
                ESP_LOGE(AP_TAG, "Decsr1 B not found");
            }
	}	
        }
        break;
    }

    case ESP_GATTC_READ_CHAR_EVT:
ESP_LOGI(AP_TAG, "ESP_GATTC_READ_CHAR_EVT, receive read value:");
esp_log_buffer_hex(AP_TAG, p_data->read.value, p_data->read.value_len);
ESP_LOGI(AP_TAG, "Read char handle = %x", p_data->read.handle);

	int length = p_data->read.value_len;
	if (length > BLE_INPUT_BUFFSIZE)
	length = BLE_INPUT_BUFFSIZE;
	if (length > 0) {
        memcpy(readDataB, p_data->read.value, length);
	}
	readDataBLen = length;
	readDataHandleB = p_data->read.handle;

/*
if (p_data->read.handle == 0x2a) {
        esp_gatt_status_t ret_status = esp_ble_gattc_read_char( gattc_if,
                                  gl_profile_tab[PROFILE_B_APP_ID].conn_id,
                                  gl_profile_tab[PROFILE_B_APP_ID].mcuver_handle,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Read_mcuver_req B error");
			conerr = 1;
                    }  ESP_LOGI(AP_TAG, "Read_read_mcuver_req B Ok");
	}
*/
	break;
    case ESP_GATTC_NOTIFY_EVT:
        if ((p_data->notify.is_notify) && (p_data->notify.handle == gl_profile_tab[PROFILE_B_APP_ID].rxchar_handle)) {
	if (btauthorizb) {
	int length = p_data->notify.value_len;
	if (length > BLE_INPUT_BUFFSIZE)
	length = BLE_INPUT_BUFFSIZE;
	if (length > 0) {
        memcpy(notifyDataB, p_data->notify.value, length);
	}
	notifyDataBLen = length;
	} else {
	if ((!memcmp(p_data->notify.value, "\x55\x00\xff\x01\xaa", 5))||(!memcmp(p_data->notify.value, "\x55\x00\xff\x02\xaa", 5))) {
	ESP_LOGI(AP_TAG, "Authorize B Redmond ok");
	t_lastb_us = ~t_lastb_us;
	t_ppconb_us = esp_timer_get_time();
	btauthorizb = true;
	r4sBConnErr = 0;
	r4sBcounter = 1;	
	f_SyncB = 1;
	NumConnB++;
	if (!NumConnB) NumConnB--;
	bin2hex(gl_profile_tab[PROFILE_B_APP_ID].remote_bda, tBLEAddrB,6);
	strcpy(DEV_NAMEB,REQ_NAMEB);
        MqttPubSubB(mqttConnected);	
	}
	}
	} else if (!btauthorizb && (p_data->notify.is_notify) && (p_data->notify.handle == gl_profile_tab[PROFILE_B_APP_ID].auth_handle)) {
	if ((p_data->notify.value_len == 12) && (xbtauthb == 2)) {
	uint8_t buff2[16];
        uint8_t xiv_char_data[12] = { 0x55,0x00,0xff,0xb7,0x2c,0x27,0xb3,0xb8,0xac,0x5a,0xef,0xaa};  //auth string
	xiv_char_data[5] = xiv_char_data[5] + R4SNUM;  //for each gate number different auth id

	mixA(gl_profile_tab[PROFILE_B_APP_ID].remote_bda, buff1, MiBKettleID);
	cipherInit(buff1, bufftab, 8);
	cipherCrypt(p_data->notify.value, buff2, bufftab, 12);
	mixB(gl_profile_tab[PROFILE_B_APP_ID].remote_bda, buff1, MiBKettleID);
	cipherInit(buff1, bufftab, 8);
	cipherCrypt(buff2, buff1, bufftab, 12);
	if (!memcmp(xiv_char_data, buff1, 12)) {
	buff2[0] = 0x92;
	buff2[1] = 0xab;
	buff2[2] = 0x54;
	buff2[3] = 0xfa;
	cipherInit(xiv_char_data, bufftab, 12);
	cipherCrypt(buff2, buff1, bufftab, 4);
        esp_gatt_status_t ret_status = esp_ble_gattc_write_char( gattc_if,
                                  gl_profile_tab[PROFILE_B_APP_ID].conn_id,
                                  gl_profile_tab[PROFILE_B_APP_ID].auth_handle,
                                  4,
                                  buff1,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Write_auth_xi_ack B error");
                    }  ESP_LOGI(AP_TAG, "Write_auth_xi_ack B Ok");
	buff2[0] = 0x03;
	buff2[1] = 0x01;
        ret_status = esp_ble_gattc_write_char( gattc_if,
                                  gl_profile_tab[PROFILE_B_APP_ID].conn_id,
                                  gl_profile_tab[PROFILE_B_APP_ID].setup_handle,
                                  2,
                                  buff2,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
                        ESP_LOGE(AP_TAG, "Write_setup_xi B error");
                    }  ESP_LOGI(AP_TAG, "Write_setup_xi B Ok");
	buff2[0] = 0x00;
        ret_status = esp_ble_gattc_write_char( gattc_if,
                                  gl_profile_tab[PROFILE_B_APP_ID].conn_id,
                                  gl_profile_tab[PROFILE_B_APP_ID].boil_handle,
                                  1,
                                  buff2,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
                        ESP_LOGE(AP_TAG, "Write_boil_xi B error");
                    }  ESP_LOGI(AP_TAG, "Write_boil_xi B Ok");
	buff2[0] = 0x17;
        ret_status = esp_ble_gattc_write_char( gattc_if,
                                  gl_profile_tab[PROFILE_B_APP_ID].conn_id,
                                  gl_profile_tab[PROFILE_B_APP_ID].time_handle,
                                  1,
                                  buff2,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
                        ESP_LOGE(AP_TAG, "Write_time_xi B error");
                    }  ESP_LOGI(AP_TAG, "Write_time_xi B Ok");

/*
        ret_status = esp_ble_gattc_read_char( gattc_if,
                                  gl_profile_tab[PROFILE_B_APP_ID].conn_id,
                                  gl_profile_tab[PROFILE_B_APP_ID].ver_handle,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Read_ver_req B error");
                    }  ESP_LOGI(AP_TAG, "Read_read_ver_req B Ok");
*/

	ESP_LOGI(AP_TAG, "Authorize B Xiaomi ok");
	t_lastb_us = ~t_lastb_us;
	t_ppconb_us = esp_timer_get_time();
	btauthorizb = true;
	r4sBConnErr = 0;
	NumConnB++;
	if (!NumConnB) NumConnB--;
	bin2hex(gl_profile_tab[PROFILE_B_APP_ID].remote_bda, tBLEAddrB,6);
	strcpy(DEV_NAMEB,REQ_NAMEB);
        MqttPubSubB(mqttConnected);	
	esp_ble_gattc_register_for_notify (gattc_if, gl_profile_tab[PROFILE_B_APP_ID].remote_bda, gl_profile_tab[PROFILE_B_APP_ID].rxchar_handle);

	} else {
	conerr = 1;
	ESP_LOGI(AP_TAG, "Invalid B Xiaomi product Id");
	if ((DEV_TYPB == 95) && (MiBKettleID < 10000)) MiBKettleID++;
	}
	}
        } if (p_data->notify.is_notify) {
//            ESP_LOGI(AP_TAG, "GATTC_NOTIFY_EVT_B, Handle = %x, Notify value:",p_data->notify.handle);
        }else{
//            ESP_LOGI(AP_TAG, "GATTC_NOTIFY_EVT_B, Indicate value:");
        }
//        esp_log_buffer_hex(AP_TAG, p_data->notify.value, p_data->notify.value_len);

        break;



    case ESP_GATTC_WRITE_DESCR_EVT:
        if (p_data->write.status != ESP_GATT_OK){
	conerr = 1;
            ESP_LOGE(AP_TAG, "Write descr B failed, error status = %x", p_data->write.status);
	conerr = 1;
            break;
        }
	uint8_t  write_char_crypt_data[16];
	int  write_char_data_len = 12;
        uint8_t write_char_data[12] = { 0x55,0x00,0xff,0xb7,0x2c,0x27,0xb3,0xb8,0xac,0x5a,0xef,0xaa};  //auth string
	write_char_data[5] = write_char_data[5] + R4SNUM;  //for each gate number different auth id
	if ((DEV_TYPB < 64) || !xbtauthb) {
	if (DEV_TYPB > 63) {
        write_char_data[0] = 0x90;
        write_char_data[1] = 0xca;
        write_char_data[2] = 0x85;
        write_char_data[3] = 0xde;
	write_char_data_len = 4;
	}
        esp_gatt_status_t ret_status = esp_ble_gattc_write_char( gattc_if,
                                  gl_profile_tab[PROFILE_B_APP_ID].conn_id,
                                  gl_profile_tab[PROFILE_B_APP_ID].txchar_handle,
                                  write_char_data_len,
                                  write_char_data,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
                        ESP_LOGE(AP_TAG, "Write_auth B error");
			conerr = 1;
                    }  ESP_LOGI(AP_TAG, "Write_auth B Ok");
	if (DEV_TYPB > 63) {
	xbtauthb = 1;
	esp_ble_gattc_register_for_notify (gattc_if, gl_profile_tab[PROFILE_B_APP_ID].remote_bda, gl_profile_tab[PROFILE_B_APP_ID].auth_handle);
	}
	} else if ((DEV_TYPB > 63) && (xbtauthb == 1)) {
	xbtauthb = 2;
	mixA(gl_profile_tab[PROFILE_B_APP_ID].remote_bda, buff1, MiBKettleID);
	cipherInit(buff1, bufftab, 8);
	cipherCrypt(write_char_data, write_char_crypt_data, bufftab, 12);
        esp_gatt_status_t ret_status = esp_ble_gattc_write_char( gattc_if,
                                  gl_profile_tab[PROFILE_B_APP_ID].conn_id,
                                  gl_profile_tab[PROFILE_B_APP_ID].auth_handle,
                                  write_char_data_len,
                                  write_char_crypt_data,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
                        ESP_LOGE(AP_TAG, "Write_auth_mi B error");
			conerr = 1;
                    }  ESP_LOGI(AP_TAG, "Write_auth_mi B Ok");
	}
        break;
    case ESP_GATTC_SRVC_CHG_EVT: {
        esp_bd_addr_t bda;
        memcpy(bda, p_data->srvc_chg.remote_bda, sizeof(esp_bd_addr_t));
        ESP_LOGI(AP_TAG, "ESP_GATTC_SRVC_CHG_EVT, bd_addr:");
        esp_log_buffer_hex(AP_TAG, bda, sizeof(esp_bd_addr_t));
        break;
    }
    case ESP_GATTC_WRITE_CHAR_EVT:
        if (p_data->write.status != ESP_GATT_OK){
	conerr = 1;
            ESP_LOGE(AP_TAG, "Write char B failed, error status = %x", p_data->write.status);
            break;
        }
//	if (btauthorizb) start_scan();
	start_scan();
        break;

    case ESP_GATTC_DISCONNECT_EVT:
        if (memcmp(p_data->disconnect.remote_bda, gl_profile_tab[PROFILE_B_APP_ID].remote_bda, 6) == 0){
	btopenb = false;
        btconnectb = false;
        btauthorizb = false;
        get_serverb = false;
        xbtauthb = 0;
        ESP_LOGI(AP_TAG, "ESP_GATTC_DISCONNECT_EVT, reason = %d", p_data->disconnect.reason);
	if ((mqttConnected) &&(tBLEAddrB[0])) {
            char ldata[32];
            strcpy(ldata,MQTT_BASE_TOPIC);
            strcat(ldata,"/");
            strcat(ldata,tBLEAddrB);
            strcat(ldata,"/status");
            esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
	}
//	Isscanning = false;
	start_scan();
	}
        break;
    default:
        break;
    }
//	if (conerr && btopenb) esp_ble_gattc_close(gl_profile_tab[PROFILE_B_APP_ID].gattc_if,gl_profile_tab[PROFILE_B_APP_ID].conn_id);
	if ((conerr || f_update) && btopenb) esp_ble_gap_disconnect(gl_profile_tab[PROFILE_B_APP_ID].remote_bda);
}
//*** B ************************

//*** C ********************************************
static void gattc_profile_c_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    esp_ble_gattc_cb_param_t *p_data = (esp_ble_gattc_cb_param_t *)param;
	uint8_t  conerr = 0;
	uint8_t  buff1[16];
	uint8_t  bufftab[256];

    switch (event) {
    case ESP_GATTC_REG_EVT:
//	ESP_LOGI(AP_TAG, "REG_EVT");

	esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV,ESP_PWR_LVL_P9); // for more power???
	esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT,ESP_PWR_LVL_P9); // for more power???

        esp_err_t scan_ret = esp_ble_gap_set_scan_params(&ble_scan_params);
        if (scan_ret){
	ESP_LOGE(AP_TAG, "Set scan params error, error code = %x", scan_ret);
        }
	break;
    case ESP_GATTC_CONNECT_EVT:
	break;
    case ESP_GATTC_OPEN_EVT:
	xbtauthc = 0;
        if (param->open.status != ESP_GATT_OK) {
	ESP_LOGE(AP_TAG, "Open C failed, status %d", p_data->open.status);
	btopenc = false;
        btconnectc = false;
//	Isscanning = false;
	start_scan();
	break;
        }
        memcpy(gl_profile_tab[PROFILE_C_APP_ID].remote_bda, p_data->open.remote_bda, 6);
        gl_profile_tab[PROFILE_C_APP_ID].conn_id = p_data->open.conn_id;
        ESP_LOGI(AP_TAG, "Open C success, conn_id %d, if %d, status %d, mtu %d", p_data->open.conn_id, gattc_if, p_data->open.status, p_data->open.mtu);
        ESP_LOGI(AP_TAG, "REMOTE BDA C:");
        esp_log_buffer_hex(AP_TAG, p_data->open.remote_bda, sizeof(esp_bd_addr_t));
        esp_err_t mtu_ret = esp_ble_gattc_send_mtu_req (gattc_if, p_data->open.conn_id);
	btopenc = true;
        if (mtu_ret){
            ESP_LOGE(AP_TAG, "config MTU C error, error code = %x", mtu_ret);
	if (DEV_TYPC == 64) MiCKettleID = 275;
	if (DEV_TYPC == 65) MiCKettleID = 131;
	if (DEV_TYPC == 66) MiCKettleID = 1116;
        }
        break;

    case ESP_GATTC_DIS_SRVC_CMPL_EVT:
        if (param->dis_srvc_cmpl.status != ESP_GATT_OK){
	conerr = 1;
	ESP_LOGE(AP_TAG, "Discover service C failed, status %d", param->dis_srvc_cmpl.status);
	break;
        }
        ESP_LOGI(AP_TAG, "Discover service C complete conn_id %d", param->dis_srvc_cmpl.conn_id);
        if (DEV_TYPC < 64) esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id, &remote_filter_service_uuid);
	else {
	esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id, &xremote_filter_service_uuid);
	esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id, &xremote_filter_service1_uuid);
	esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id, &xremote_filter_service2_uuid);
	esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id, &xremote_filter_service3_uuid);
	}
        break;
    case ESP_GATTC_CFG_MTU_EVT:
        if (param->cfg_mtu.status != ESP_GATT_OK){
	conerr = 1;
	ESP_LOGE(AP_TAG,"Config mtu failed, error status = %x", param->cfg_mtu.status);
        }
//	ESP_LOGI(AP_TAG, "ESP_GATTC_CFG_MTU_EVT, Status %d, MTU %d, conn_id %d", param->cfg_mtu.status, param->cfg_mtu.mtu, param->cfg_mtu.conn_id);
	break;
    case ESP_GATTC_SEARCH_RES_EVT: {
        ESP_LOGI(AP_TAG, "SEARCH RES C: conn_id = %x is primary service %d", p_data->search_res.conn_id, p_data->search_res.is_primary);
        ESP_LOGI(AP_TAG, "start handle %d end handle %d current handle value %d", p_data->search_res.start_handle, p_data->search_res.end_handle, p_data->search_res.srvc_id.inst_id);

//esp_log_buffer_hex(AP_TAG,p_data->search_res.srvc_id.uuid.uuid.uuid128,16);
//esp_log_buffer_hex(AP_TAG,xremote_filter_service11_uuid.uuid.uuid128,16);

        if ((DEV_TYPC < 64) && (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_128) && (bt_compare_UUID128(p_data->search_res.srvc_id.uuid.uuid.uuid128, remote_filter_service_uuid.uuid.uuid128))) {

            ESP_LOGI(AP_TAG, "Redmond Service C found");
            get_serverc = true;
            gl_profile_tab[PROFILE_C_APP_ID].service_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[PROFILE_C_APP_ID].service_end_handle = p_data->search_res.end_handle;
        } else if ((DEV_TYPC > 63) && (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_128) && (bt_compare_UUID128(p_data->search_res.srvc_id.uuid.uuid.uuid128, xremote_filter_service_uuid.uuid.uuid128))) {

            ESP_LOGI(AP_TAG, "Xiaomi Service1 C found");
            gl_profile_tab[PROFILE_C_APP_ID].service_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[PROFILE_C_APP_ID].service_end_handle = p_data->search_res.end_handle;
        } else if ((DEV_TYPC > 63) && (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_16) && (p_data->search_res.srvc_id.uuid.uuid.uuid16 == XREMOTE_SERVICE116_UUID)) {

            ESP_LOGI(AP_TAG, "Xiaomi Service2 C found");
            gl_profile_tab[PROFILE_C_APP_ID].service1_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[PROFILE_C_APP_ID].service1_end_handle = p_data->search_res.end_handle;
        } else if ((DEV_TYPC > 63) && (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_16) && (p_data->search_res.srvc_id.uuid.uuid.uuid16 == XREMOTE_SERVICE216_UUID)) {

            ESP_LOGI(AP_TAG, "Xiaomi Service3 C found");
            get_serverc = true;
            gl_profile_tab[PROFILE_C_APP_ID].service2_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[PROFILE_C_APP_ID].service2_end_handle = p_data->search_res.end_handle;
        } else if ((DEV_TYPC > 63) && (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_16) && (p_data->search_res.srvc_id.uuid.uuid.uuid16 == XREMOTE_SERVICE316_UUID)) {

            ESP_LOGI(AP_TAG, "Xiaomi Service4 C found");
            get_serverc = true;
            gl_profile_tab[PROFILE_C_APP_ID].service3_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[PROFILE_C_APP_ID].service3_end_handle = p_data->search_res.end_handle;
	}
        break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT:
        if (p_data->search_cmpl.status != ESP_GATT_OK){
	conerr = 1;
            ESP_LOGE(AP_TAG, "Search service failed, error status = %x", p_data->search_cmpl.status);
            break;
        }
        if(p_data->search_cmpl.searched_service_source == ESP_GATT_SERVICE_FROM_REMOTE_DEVICE) {
//            ESP_LOGI(AP_TAG, "Get service information from remote device");
        } else if (p_data->search_cmpl.searched_service_source == ESP_GATT_SERVICE_FROM_NVS_FLASH) {
//            ESP_LOGI(AP_TAG, "Get service information from flash");
        } else {
//            ESP_LOGI(AP_TAG, "Unknown service source");
        }
        if (get_serverc){
            uint16_t count = 0;
            uint16_t count1 = 0;
            esp_gatt_status_t status = esp_ble_gattc_get_attr_count( gattc_if,
                                                                     p_data->search_cmpl.conn_id,
                                                                     ESP_GATT_DB_CHARACTERISTIC,
                                                                     gl_profile_tab[PROFILE_C_APP_ID].service_start_handle,
                                                                     gl_profile_tab[PROFILE_C_APP_ID].service_end_handle,
                                                                     INVALID_HANDLE,
                                                                     &count);
            if (status != ESP_GATT_OK){
		conerr = 1;
                ESP_LOGE(AP_TAG, "esp_ble_gattc_get_attr_count error");
            }

	if (DEV_TYPC > 63) {
            status = esp_ble_gattc_get_attr_count( gattc_if,
                                                                     p_data->search_cmpl.conn_id,
                                                                     ESP_GATT_DB_CHARACTERISTIC,
                                                                     gl_profile_tab[PROFILE_C_APP_ID].service1_start_handle,
                                                                     gl_profile_tab[PROFILE_C_APP_ID].service1_end_handle,
                                                                     INVALID_HANDLE,
                                                                     &count1);
            if (status != ESP_GATT_OK){
		conerr = 1;
                ESP_LOGE(AP_TAG, "esp_ble_gattc_get_attr1_count error");
            }
	count = count + count1;
	count1 = 0;		
            status = esp_ble_gattc_get_attr_count( gattc_if,
                                                                     p_data->search_cmpl.conn_id,
                                                                     ESP_GATT_DB_CHARACTERISTIC,
                                                                     gl_profile_tab[PROFILE_C_APP_ID].service2_start_handle,
                                                                     gl_profile_tab[PROFILE_C_APP_ID].service2_end_handle,
                                                                     INVALID_HANDLE,
                                                                     &count1);
            if (status != ESP_GATT_OK){
		conerr = 1;
                ESP_LOGE(AP_TAG, "esp_ble_gattc_get_attr2_count error");
            }
	count = count + count1;
	count1 = 0;		
            status = esp_ble_gattc_get_attr_count( gattc_if,
                                                                     p_data->search_cmpl.conn_id,
                                                                     ESP_GATT_DB_CHARACTERISTIC,
                                                                     gl_profile_tab[PROFILE_C_APP_ID].service3_start_handle,
                                                                     gl_profile_tab[PROFILE_C_APP_ID].service3_end_handle,
                                                                     INVALID_HANDLE,
                                                                     &count1);
            if (status != ESP_GATT_OK){
		conerr = 1;
                ESP_LOGE(AP_TAG, "esp_ble_gattc_get_attr3_count error");
            }
	count = count + count1;
	}

            if (count > 0){
		char_elem_result_c = (esp_gattc_char_elem_t *)malloc(sizeof(esp_gattc_char_elem_t) * count);
                if (!char_elem_result_c){
		conerr = 1;
                    ESP_LOGE(AP_TAG, "Gattc no mem");
                }else{
		if (DEV_TYPC < 64) {
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_C_APP_ID].service_start_handle,
                                                             gl_profile_tab[PROFILE_C_APP_ID].service_end_handle,
                                                             remote_filter_rxchar_uuid,
                                                             (char_elem_result_c),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_rxchar_by_uuid C error");
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
                                                             p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_C_APP_ID].service_start_handle,
                                                             gl_profile_tab[PROFILE_C_APP_ID].service_end_handle,
                                                             remote_filter_txchar_uuid,
                                                             (char_elem_result_c+1),
                                                             &count);

                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_txchar_by_uuid C error");
                    }
		} else {
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_C_APP_ID].service_start_handle,
                                                             gl_profile_tab[PROFILE_C_APP_ID].service_end_handle,
                                                             xremote_filter_status_uuid,
                                                             (char_elem_result_c),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_status_by_uuid C error");
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_C_APP_ID].service1_start_handle,
                                                             gl_profile_tab[PROFILE_C_APP_ID].service1_end_handle,
                                                             xremote_filter_authinit_uuid,
                                                             (char_elem_result_c+1),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_authinit_by_uuid C error");
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_C_APP_ID].service1_start_handle,
                                                             gl_profile_tab[PROFILE_C_APP_ID].service1_end_handle,
                                                             xremote_filter_auth_uuid,
                                                             (char_elem_result_c+2),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_auth_by_uuid C error");
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_C_APP_ID].service1_start_handle,
                                                             gl_profile_tab[PROFILE_C_APP_ID].service1_end_handle,
                                                             xremote_filter_ver_uuid,
                                                             (char_elem_result_c+3),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_ver_by_uuid C error");
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_C_APP_ID].service_start_handle,
                                                             gl_profile_tab[PROFILE_C_APP_ID].service_end_handle,
                                                             xremote_filter_setup_uuid,
                                                             (char_elem_result_c+4),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_setup_by_uuid C error");
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_C_APP_ID].service_start_handle,
                                                             gl_profile_tab[PROFILE_C_APP_ID].service_end_handle,
                                                             xremote_filter_time_uuid,
                                                             (char_elem_result_c+5),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_time_by_uuid C error");
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_C_APP_ID].service_start_handle,
                                                             gl_profile_tab[PROFILE_C_APP_ID].service_end_handle,
                                                             xremote_filter_boil_uuid,
                                                             (char_elem_result_c+6),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_boil_by_uuid C error");
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_C_APP_ID].service2_start_handle,
                                                             gl_profile_tab[PROFILE_C_APP_ID].service2_end_handle,
                                                             xremote_filter_mcuver_uuid,
                                                             (char_elem_result_c+7),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_mcuver_by_uuid C error");
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_C_APP_ID].service3_start_handle,
                                                             gl_profile_tab[PROFILE_C_APP_ID].service3_end_handle,
                                                             xremote_filter_update_uuid,
                                                             (char_elem_result_c+8),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_update_by_uuid C error");
                    }

		}

                    /*  Every service have only one char in our 'ESP_GATTS_DEMO' demo, so we used first 'char_elem_result' */
                    if (count > 0 && (char_elem_result_c[0].properties & ESP_GATT_CHAR_PROP_BIT_NOTIFY)){
                        gl_profile_tab[PROFILE_C_APP_ID].rxchar_handle = char_elem_result_c[0].char_handle;
                        esp_ble_gattc_register_for_notify (gattc_if, gl_profile_tab[PROFILE_C_APP_ID].remote_bda, char_elem_result_c[0].char_handle);
                        gl_profile_tab[PROFILE_C_APP_ID].txchar_handle = char_elem_result_c[1].char_handle;
		if (DEV_TYPC > 63) {
                        gl_profile_tab[PROFILE_C_APP_ID].auth_handle = char_elem_result_c[2].char_handle;
                        gl_profile_tab[PROFILE_C_APP_ID].ver_handle = char_elem_result_c[3].char_handle;
                        gl_profile_tab[PROFILE_C_APP_ID].setup_handle = char_elem_result_c[4].char_handle;
                        gl_profile_tab[PROFILE_C_APP_ID].time_handle = char_elem_result_c[5].char_handle;
                        gl_profile_tab[PROFILE_C_APP_ID].boil_handle = char_elem_result_c[6].char_handle;
                        gl_profile_tab[PROFILE_C_APP_ID].mcuver_handle = char_elem_result_c[7].char_handle;
                        gl_profile_tab[PROFILE_C_APP_ID].update_handle = char_elem_result_c[8].char_handle;
		}
			ESP_LOGI(AP_TAG, "Register_for_notify C");
                    }
                }
                /* free char_elem_result */
                free(char_elem_result_c);
            }else{
		conerr = 1;
                ESP_LOGE(AP_TAG, "No char C found");
            }
        }
         break;
    case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
        if (p_data->reg_for_notify.status != ESP_GATT_OK){
	conerr = 1;
            ESP_LOGE(AP_TAG, "REG FOR NOTIFY C failed: error status = %d", p_data->reg_for_notify.status);
        } else {
            uint16_t count = 0;
            uint16_t notify_en = 1; 
	if ((DEV_TYPC < 64) || (xbtauthc != 1)) {
            esp_gatt_status_t ret_status = esp_ble_gattc_get_attr_count( gattc_if,
                                                                         gl_profile_tab[PROFILE_C_APP_ID].conn_id,
                                                                         ESP_GATT_DB_DESCRIPTOR,
                                                                         gl_profile_tab[PROFILE_C_APP_ID].service_start_handle,
                                                                         gl_profile_tab[PROFILE_C_APP_ID].service_end_handle,
                                                                         gl_profile_tab[PROFILE_C_APP_ID].rxchar_handle,
                                                                         &count);
            if (ret_status != ESP_GATT_OK){
		conerr = 1;
                ESP_LOGE(AP_TAG, "Get_attr_count C error");
            }
            if (count > 0){
                descr_elem_result_c = malloc(sizeof(esp_gattc_descr_elem_t) * count);
                if (!descr_elem_result_c){
                    ESP_LOGE(AP_TAG, "malloc error, gattc no mem");
                }else{
                    ret_status = esp_ble_gattc_get_descr_by_char_handle( gattc_if,
                                                                         gl_profile_tab[PROFILE_C_APP_ID].conn_id,
                                                                         p_data->reg_for_notify.handle,
                                                                         notify_descr_uuid,
                                                                         descr_elem_result_c,
                                                                         &count);
                    if (ret_status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_descr_by_char_handle C error");
                    }
                    /* Every char has only one descriptor in our 'ESP_GATTS_DEMO' demo, so we used first 'descr_elem_result' */
                    if (count > 0 && descr_elem_result_c[0].uuid.len == ESP_UUID_LEN_16 && descr_elem_result_c[0].uuid.uuid.uuid16 == ESP_GATT_UUID_CHAR_CLIENT_CONFIG){
                        ret_status = esp_ble_gattc_write_char_descr( gattc_if,
                                                                     gl_profile_tab[PROFILE_C_APP_ID].conn_id,
                                                                     descr_elem_result_c[0].handle,
                                                                     sizeof(notify_en),
                                                                     (uint8_t *)&notify_en,
                                                                     ESP_GATT_WRITE_TYPE_RSP,
                                                                     ESP_GATT_AUTH_REQ_NONE);
                    }
                    if (ret_status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Write_char_descr C error");
                    }
                    /* free descr_elem_result */
                    free(descr_elem_result_c);
                }
            } else {
		conerr = 1;
                ESP_LOGE(AP_TAG, "Decsr C not found");
            }
	} else if ((DEV_TYPC > 63) && xbtauthc) {
            count = 0;
            notify_en = 1; 
            esp_gatt_status_t ret_status = esp_ble_gattc_get_attr_count( gattc_if,
                                                                         gl_profile_tab[PROFILE_C_APP_ID].conn_id,
                                                                         ESP_GATT_DB_DESCRIPTOR,
                                                                         gl_profile_tab[PROFILE_C_APP_ID].service1_start_handle,
                                                                         gl_profile_tab[PROFILE_C_APP_ID].service1_end_handle,
                                                                         gl_profile_tab[PROFILE_C_APP_ID].auth_handle,
                                                                         &count);
            if (ret_status != ESP_GATT_OK){
		conerr = 1;
                ESP_LOGE(AP_TAG, "Get_attr_count1 C error");
            }
            if (count > 0){
                descr_elem_result_c = malloc(sizeof(esp_gattc_descr_elem_t) * count);
                if (!descr_elem_result_c){
		conerr = 1;
                    ESP_LOGE(AP_TAG, "malloc error, gattc no mem");
                }else{
                    ret_status = esp_ble_gattc_get_descr_by_char_handle( gattc_if,
                                                                         gl_profile_tab[PROFILE_C_APP_ID].conn_id,
                                                                         p_data->reg_for_notify.handle,
                                                                         notify_descr_uuid,
                                                                         descr_elem_result_c,
                                                                         &count);
                    if (ret_status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_descr_by_char_handle C error");
                    }
                    /* Every char has only one descriptor in our 'ESP_GATTS_DEMO' demo, so we used first 'descr_elem_result' */
                    if (count > 0 && descr_elem_result_c[0].uuid.len == ESP_UUID_LEN_16 && descr_elem_result_c[0].uuid.uuid.uuid16 == ESP_GATT_UUID_CHAR_CLIENT_CONFIG){
                        ret_status = esp_ble_gattc_write_char_descr( gattc_if,
                                                                     gl_profile_tab[PROFILE_C_APP_ID].conn_id,
                                                                     descr_elem_result_c[0].handle,
                                                                     sizeof(notify_en),
                                                                     (uint8_t *)&notify_en,
                                                                     ESP_GATT_WRITE_TYPE_RSP,
                                                                     ESP_GATT_AUTH_REQ_NONE);
                    }
                    if (ret_status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Write_char_descr1 C error");
                    }
                    /* free descr_elem_result */
                    free(descr_elem_result_c);
                }
            } else {
		conerr = 1;
                ESP_LOGE(AP_TAG, "Decsr1 C not found");
            }
	}	
        }
        break;
    }

    case ESP_GATTC_READ_CHAR_EVT:
ESP_LOGI(AP_TAG, "ESP_GATTC_READ_CHAR_EVT, receive read value:");
esp_log_buffer_hex(AP_TAG, p_data->read.value, p_data->read.value_len);
ESP_LOGI(AP_TAG, "Read char handle = %x", p_data->read.handle);

	int length = p_data->read.value_len;
	if (length > BLE_INPUT_BUFFSIZE)
	length = BLE_INPUT_BUFFSIZE;
	if (length > 0) {
        memcpy(readDataC, p_data->read.value, length);
	}
	readDataCLen = length;
	readDataHandleC = p_data->read.handle;

/*
if (p_data->read.handle == 0x2a) {
        esp_gatt_status_t ret_status = esp_ble_gattc_read_char( gattc_if,
                                  gl_profile_tab[PROFILE_C_APP_ID].conn_id,
                                  gl_profile_tab[PROFILE_C_APP_ID].mcuver_handle,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Read_mcuver_req C error");
			conerr = 1;
                    }  ESP_LOGI(AP_TAG, "Read_read_mcuver_req C Ok");
	}
*/
	break;
    case ESP_GATTC_NOTIFY_EVT:
        if ((p_data->notify.is_notify) && (p_data->notify.handle == gl_profile_tab[PROFILE_C_APP_ID].rxchar_handle)) {
	if (btauthorizc) {
	int length = p_data->notify.value_len;
	if (length > BLE_INPUT_BUFFSIZE)
	length = BLE_INPUT_BUFFSIZE;
	if (length > 0) {
        memcpy(notifyDataC, p_data->notify.value, length);
	}
	notifyDataCLen = length;
	} else {
	if ((!memcmp(p_data->notify.value, "\x55\x00\xff\x01\xaa", 5))||(!memcmp(p_data->notify.value, "\x55\x00\xff\x02\xaa", 5))) {
	ESP_LOGI(AP_TAG, "Authorize C Redmond ok");
	t_lastc_us = ~t_lastc_us;
	t_ppconc_us = esp_timer_get_time();
	btauthorizc = true;
	r4sCConnErr = 0;
	r4sCcounter = 1;	
	f_SyncC = 1;
	NumConnC++;
	if (!NumConnC) NumConnC--;
	bin2hex(gl_profile_tab[PROFILE_C_APP_ID].remote_bda, tBLEAddrC,6);
	strcpy(DEV_NAMEC,REQ_NAMEC);
        MqttPubSubC(mqttConnected);	
	}
	}
	} else if (!btauthorizc && (p_data->notify.is_notify) && (p_data->notify.handle == gl_profile_tab[PROFILE_C_APP_ID].auth_handle)) {
	if ((p_data->notify.value_len == 12) && (xbtauthc == 2)) {
	uint8_t buff2[16];
        uint8_t xiv_char_data[12] = { 0x55,0x00,0xff,0xb8,0x2c,0x27,0xb3,0xb8,0xac,0x5a,0xef,0xaa};  //auth string
	xiv_char_data[5] = xiv_char_data[5] + R4SNUM;  //for each gate number different auth id

	mixA(gl_profile_tab[PROFILE_C_APP_ID].remote_bda, buff1, MiCKettleID);
	cipherInit(buff1, bufftab, 8);
	cipherCrypt(p_data->notify.value, buff2, bufftab, 12);
	mixB(gl_profile_tab[PROFILE_C_APP_ID].remote_bda, buff1, MiCKettleID);
	cipherInit(buff1, bufftab, 8);
	cipherCrypt(buff2, buff1, bufftab, 12);
	if (!memcmp(xiv_char_data, buff1, 12)) {
	buff2[0] = 0x92;
	buff2[1] = 0xab;
	buff2[2] = 0x54;
	buff2[3] = 0xfa;
	cipherInit(xiv_char_data, bufftab, 12);
	cipherCrypt(buff2, buff1, bufftab, 4);
        esp_gatt_status_t ret_status = esp_ble_gattc_write_char( gattc_if,
                                  gl_profile_tab[PROFILE_C_APP_ID].conn_id,
                                  gl_profile_tab[PROFILE_C_APP_ID].auth_handle,
                                  4,
                                  buff1,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Write_auth_xi_ack C error");
                    }  ESP_LOGI(AP_TAG, "Write_auth_xi_ack C Ok");
	buff2[0] = 0x03;
	buff2[1] = 0x01;
        ret_status = esp_ble_gattc_write_char( gattc_if,
                                  gl_profile_tab[PROFILE_C_APP_ID].conn_id,
                                  gl_profile_tab[PROFILE_C_APP_ID].setup_handle,
                                  2,
                                  buff2,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
                        ESP_LOGE(AP_TAG, "Write_setup_xi C error");
                    }  ESP_LOGI(AP_TAG, "Write_setup_xi C Ok");
	buff2[0] = 0x00;
        ret_status = esp_ble_gattc_write_char( gattc_if,
                                  gl_profile_tab[PROFILE_C_APP_ID].conn_id,
                                  gl_profile_tab[PROFILE_C_APP_ID].boil_handle,
                                  1,
                                  buff2,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
                        ESP_LOGE(AP_TAG, "Write_boil_xi C error");
                    }  ESP_LOGI(AP_TAG, "Write_boil_xi C Ok");
	buff2[0] = 0x17;
        ret_status = esp_ble_gattc_write_char( gattc_if,
                                  gl_profile_tab[PROFILE_C_APP_ID].conn_id,
                                  gl_profile_tab[PROFILE_C_APP_ID].time_handle,
                                  1,
                                  buff2,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
                        ESP_LOGE(AP_TAG, "Write_time_xi C error");
                    }  ESP_LOGI(AP_TAG, "Write_time_xi C Ok");

/*
        ret_status = esp_ble_gattc_read_char( gattc_if,
                                  gl_profile_tab[PROFILE_C_APP_ID].conn_id,
                                  gl_profile_tab[PROFILE_C_APP_ID].ver_handle,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Read_ver_req C error");
                    }  ESP_LOGI(AP_TAG, "Read_read_ver_req C Ok");
*/

	ESP_LOGI(AP_TAG, "Authorize C Xiaomi ok");
	t_lastc_us = ~t_lastc_us;
	t_ppconc_us = esp_timer_get_time();
	btauthorizc = true;
	r4sCConnErr = 0;
	NumConnC++;
	if (!NumConnC) NumConnC--;
	bin2hex(gl_profile_tab[PROFILE_C_APP_ID].remote_bda, tBLEAddrC,6);
	strcpy(DEV_NAMEC,REQ_NAMEC);
        MqttPubSubC(mqttConnected);	
	esp_ble_gattc_register_for_notify (gattc_if, gl_profile_tab[PROFILE_C_APP_ID].remote_bda, gl_profile_tab[PROFILE_C_APP_ID].rxchar_handle);

	} else {
	conerr = 1;
	ESP_LOGI(AP_TAG, "Invalid C Xiaomi product Id");
	if ((DEV_TYPC == 95) && (MiCKettleID < 10000)) MiCKettleID++;
	}
	}
        } if (p_data->notify.is_notify) {
//            ESP_LOGI(AP_TAG, "GATTC_NOTIFY_EVT_C, Handle = %x, Notify value:",p_data->notify.handle);
        }else{
//            ESP_LOGI(AP_TAG, "GATTC_NOTIFY_EVT_C, Indicate value:");
        }
//        esp_log_buffer_hex(AP_TAG, p_data->notify.value, p_data->notify.value_len);

        break;



    case ESP_GATTC_WRITE_DESCR_EVT:
        if (p_data->write.status != ESP_GATT_OK){
	conerr = 1;
            ESP_LOGE(AP_TAG, "Write descr C failed, error status = %x", p_data->write.status);
	conerr = 1;
            break;
        }
	uint8_t  write_char_crypt_data[16];
	int  write_char_data_len = 12;
        uint8_t write_char_data[12] = { 0x55,0x00,0xff,0xb8,0x2c,0x27,0xb3,0xb8,0xac,0x5a,0xef,0xaa};  //auth string
	write_char_data[5] = write_char_data[5] + R4SNUM;  //for each gate number different auth id
	if ((DEV_TYPC < 64) || !xbtauthc) {
	if (DEV_TYPC > 63) {
        write_char_data[0] = 0x90;
        write_char_data[1] = 0xca;
        write_char_data[2] = 0x85;
        write_char_data[3] = 0xde;
	write_char_data_len = 4;
	}
        esp_gatt_status_t ret_status = esp_ble_gattc_write_char( gattc_if,
                                  gl_profile_tab[PROFILE_C_APP_ID].conn_id,
                                  gl_profile_tab[PROFILE_C_APP_ID].txchar_handle,
                                  write_char_data_len,
                                  write_char_data,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
                        ESP_LOGE(AP_TAG, "Write_auth C error");
			conerr = 1;
                    }  ESP_LOGI(AP_TAG, "Write_auth C Ok");
	if (DEV_TYPC > 63) {
	xbtauthc = 1;
	esp_ble_gattc_register_for_notify (gattc_if, gl_profile_tab[PROFILE_C_APP_ID].remote_bda, gl_profile_tab[PROFILE_C_APP_ID].auth_handle);
	}
	} else if ((DEV_TYPC > 63) && (xbtauthc == 1)) {
	xbtauthc = 2;
	mixA(gl_profile_tab[PROFILE_C_APP_ID].remote_bda, buff1, MiCKettleID);
	cipherInit(buff1, bufftab, 8);
	cipherCrypt(write_char_data, write_char_crypt_data, bufftab, 12);
        esp_gatt_status_t ret_status = esp_ble_gattc_write_char( gattc_if,
                                  gl_profile_tab[PROFILE_C_APP_ID].conn_id,
                                  gl_profile_tab[PROFILE_C_APP_ID].auth_handle,
                                  write_char_data_len,
                                  write_char_crypt_data,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
                        ESP_LOGE(AP_TAG, "Write_auth_mi C error");
			conerr = 1;
                    }  ESP_LOGI(AP_TAG, "Write_auth_mi C Ok");
	}
        break;
    case ESP_GATTC_SRVC_CHG_EVT: {
        esp_bd_addr_t bda;
        memcpy(bda, p_data->srvc_chg.remote_bda, sizeof(esp_bd_addr_t));
        ESP_LOGI(AP_TAG, "ESP_GATTC_SRVC_CHG_EVT, bd_addr:");
        esp_log_buffer_hex(AP_TAG, bda, sizeof(esp_bd_addr_t));
        break;
    }
    case ESP_GATTC_WRITE_CHAR_EVT:
        if (p_data->write.status != ESP_GATT_OK){
	conerr = 1;
            ESP_LOGE(AP_TAG, "Write char C failed, error status = %x", p_data->write.status);
            break;
        }
//	if (btauthorizc) start_scan();
	start_scan();
        break;

    case ESP_GATTC_DISCONNECT_EVT:
        if (memcmp(p_data->disconnect.remote_bda, gl_profile_tab[PROFILE_C_APP_ID].remote_bda, 6) == 0){
	btopenc = false;
        btconnectc = false;
        btauthorizc = false;
        get_serverc = false;
        xbtauthc = 0;
        ESP_LOGI(AP_TAG, "ESP_GATTC_DISCONNECT_EVT, reason = %d", p_data->disconnect.reason);
	if ((mqttConnected) &&(tBLEAddrC[0])) {
            char ldata[32];
            strcpy(ldata,MQTT_BASE_TOPIC);
            strcat(ldata,"/");
            strcat(ldata,tBLEAddrC);
            strcat(ldata,"/status");
            esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
	}
//	Isscanning = false;
	start_scan();
	}
        break;
    default:
        break;
    }
//	if (conerr && btopenc) esp_ble_gattc_close(gl_profile_tab[PROFILE_C_APP_ID].gattc_if,gl_profile_tab[PROFILE_C_APP_ID].conn_id);
	if ((conerr || f_update) && btopenc) esp_ble_gap_disconnect(gl_profile_tab[PROFILE_C_APP_ID].remote_bda);
}
//*** C ************************


static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    uint8_t *adv_name = NULL;
    uint8_t adv_name_len = 0;
    uint16_t SHandle = 0;
    switch (event) {

    case ESP_GAP_BLE_READ_RSSI_COMPLETE_EVT:

//        esp_log_buffer_hex(AP_TAG, param->read_rssi_cmpl.remote_addr, 6);

      if (!memcmp(gl_profile_tab[PROFILE_A_APP_ID].remote_bda, param->read_rssi_cmpl.remote_addr, 6)) {
	iRssiA = param->read_rssi_cmpl.rssi;
//ESP_LOGI(AP_TAG,"read A-RSSI: %d", param->read_rssi_cmpl.rssi); //test #2 with data from read_rssi_cmpl
	if ((r4sAConnErr < 6 ) && (btauthoriza)) {
	if ((sendDataALen > 0) && (sendDataALen < BLE_INPUT_BUFFSIZE)) {
	SHandle = gl_profile_tab[PROFILE_A_APP_ID].txchar_handle;
	if (DEV_TYPA > 63) {
	switch (sendDataHandleA) {
	case 0:
	SHandle = gl_profile_tab[PROFILE_A_APP_ID].rxchar_handle;
	break;
	case 2:
	SHandle = gl_profile_tab[PROFILE_A_APP_ID].auth_handle;
	break;
	case 3:
	SHandle = gl_profile_tab[PROFILE_A_APP_ID].ver_handle;
	break;
	case 4:
	SHandle = gl_profile_tab[PROFILE_A_APP_ID].setup_handle;
	break;
	case 5:
	SHandle = gl_profile_tab[PROFILE_A_APP_ID].time_handle;
	break;
	case 6:
	SHandle = gl_profile_tab[PROFILE_A_APP_ID].boil_handle;
	break;
	case 7:
	SHandle = gl_profile_tab[PROFILE_A_APP_ID].mcuver_handle;
	break;
	case 8:
	SHandle = gl_profile_tab[PROFILE_A_APP_ID].update_handle;
	break;
	}
	}
        esp_gatt_status_t ret_status = esp_ble_gattc_write_char( gl_profile_tab[PROFILE_A_APP_ID].gattc_if,
                                  gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                  SHandle,
                                  sendDataALen,
                                  sendDataA,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
                        ESP_LOGE(AP_TAG, "Write_char_data A error");
	r4sAConnErr++;
	}  else {
//ESP_LOGI(AP_TAG, "r4sAConnErr: %d", r4sAConnErr);
ESP_LOGI(AP_TAG, "Send A, Handle %X, Data:", SHandle);
esp_log_buffer_hex(AP_TAG, sendDataA, sendDataALen);
	}
	sendDataALen = 0;
	}
	} else if (btauthoriza) esp_ble_gattc_close(gl_profile_tab[PROFILE_A_APP_ID].gattc_if,gl_profile_tab[PROFILE_A_APP_ID].conn_id);	
	}

      else if (!memcmp(gl_profile_tab[PROFILE_B_APP_ID].remote_bda, param->read_rssi_cmpl.remote_addr, 6)) {
	iRssiB = param->read_rssi_cmpl.rssi;
//ESP_LOGI(AP_TAG,"read B-RSSI: %d", param->read_rssi_cmpl.rssi); //test #2 with data from read_rssi_cmpl
	if ((r4sBConnErr < 6 ) && (btauthorizb)) {
	if ((sendDataBLen > 0) && (sendDataBLen < BLE_INPUT_BUFFSIZE)) {
	SHandle = gl_profile_tab[PROFILE_B_APP_ID].txchar_handle;
	if (DEV_TYPB > 63) {
	switch (sendDataHandleB) {
	case 0:
	SHandle = gl_profile_tab[PROFILE_B_APP_ID].rxchar_handle;
	break;
	case 2:
	SHandle = gl_profile_tab[PROFILE_B_APP_ID].auth_handle;
	break;
	case 3:
	SHandle = gl_profile_tab[PROFILE_B_APP_ID].ver_handle;
	break;
	case 4:
	SHandle = gl_profile_tab[PROFILE_B_APP_ID].setup_handle;
	break;
	case 5:
	SHandle = gl_profile_tab[PROFILE_B_APP_ID].time_handle;
	break;
	case 6:
	SHandle = gl_profile_tab[PROFILE_B_APP_ID].boil_handle;
	break;
	case 7:
	SHandle = gl_profile_tab[PROFILE_B_APP_ID].mcuver_handle;
	break;
	case 8:
	SHandle = gl_profile_tab[PROFILE_B_APP_ID].update_handle;
	break;
	}
	}
        esp_gatt_status_t ret_status = esp_ble_gattc_write_char( gl_profile_tab[PROFILE_B_APP_ID].gattc_if,
                                  gl_profile_tab[PROFILE_B_APP_ID].conn_id,
                                  SHandle,
                                  sendDataBLen,
                                  sendDataB,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
                        ESP_LOGE(AP_TAG, "Write_char_data B error");
	r4sBConnErr++;
	}  else {
//ESP_LOGI(AP_TAG, "r4sBConnErr: %d", r4sBConnErr);
ESP_LOGI(AP_TAG, "Send B, Handle %X, Data:", SHandle);
esp_log_buffer_hex(AP_TAG, sendDataB, sendDataBLen);
	}
	sendDataBLen = 0;
	}
	} else if (btauthorizb) esp_ble_gattc_close(gl_profile_tab[PROFILE_B_APP_ID].gattc_if,gl_profile_tab[PROFILE_B_APP_ID].conn_id);	
	}

      else if (!memcmp(gl_profile_tab[PROFILE_C_APP_ID].remote_bda, param->read_rssi_cmpl.remote_addr, 6)) {
	iRssiC = param->read_rssi_cmpl.rssi;
//ESP_LOGI(AP_TAG,"read C-RSSI: %d", param->read_rssi_cmpl.rssi); //test #2 with data from read_rssi_cmpl
	if ((r4sCConnErr < 6 ) && (btauthorizc)) {
	if ((sendDataCLen > 0) && (sendDataCLen < BLE_INPUT_BUFFSIZE)) {
	SHandle = gl_profile_tab[PROFILE_C_APP_ID].txchar_handle;
	if (DEV_TYPC > 63) {
	switch (sendDataHandleC) {
	case 0:
	SHandle = gl_profile_tab[PROFILE_C_APP_ID].rxchar_handle;
	break;
	case 2:
	SHandle = gl_profile_tab[PROFILE_C_APP_ID].auth_handle;
	break;
	case 3:
	SHandle = gl_profile_tab[PROFILE_C_APP_ID].ver_handle;
	break;
	case 4:
	SHandle = gl_profile_tab[PROFILE_C_APP_ID].setup_handle;
	break;
	case 5:
	SHandle = gl_profile_tab[PROFILE_C_APP_ID].time_handle;
	break;
	case 6:
	SHandle = gl_profile_tab[PROFILE_C_APP_ID].boil_handle;
	break;
	case 7:
	SHandle = gl_profile_tab[PROFILE_C_APP_ID].mcuver_handle;
	break;
	case 8:
	SHandle = gl_profile_tab[PROFILE_C_APP_ID].update_handle;
	break;
	}
	}
        esp_gatt_status_t ret_status = esp_ble_gattc_write_char( gl_profile_tab[PROFILE_C_APP_ID].gattc_if,
                                  gl_profile_tab[PROFILE_C_APP_ID].conn_id,
                                  SHandle,
                                  sendDataCLen,
                                  sendDataC,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
                        ESP_LOGE(AP_TAG, "Write_char_data C error");
	r4sCConnErr++;
	}  else {
//ESP_LOGI(AP_TAG, "r4sCConnErr: %d", r4sCConnErr);
ESP_LOGI(AP_TAG, "Send C, Handle %X, Data:", SHandle);
esp_log_buffer_hex(AP_TAG, sendDataC, sendDataCLen);
	}
	sendDataCLen = 0;
	}
	} else if (btauthorizc) esp_ble_gattc_close(gl_profile_tab[PROFILE_C_APP_ID].gattc_if,gl_profile_tab[PROFILE_C_APP_ID].conn_id);	
	}

	break;

    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT: {
        //the unit of the duration is second
        uint32_t duration = 0; //30;
        if (REQ_NAMEA[0] || REQ_NAMEB[0] || REQ_NAMEC[0]) {
//start scan only if not scanning, if not already starting scan or if not open in progress any connection 
	if (!Isscanning && !StartingScan && (!btconnecta || btopena) && (!btconnectb || btopenb) && (!btconnectc || btopenc)) {
        esp_ble_gap_start_scanning(duration);
        StartingScan = true;
	ESP_LOGI(AP_TAG, "Scan starting");
	}
	}
        break;
    }
    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
        //scan start complete event to indicate scan start successfully or failed
	StartingScan = false;
        if (param->scan_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(AP_TAG, "scan start failed, error status = %x", param->scan_start_cmpl.status);
	if (ble_mon && !f_update && !Isscanning) esp_restart();
	break;
        } 
	ESP_LOGI(AP_TAG, "Scan start success");
	Isscanning = true;
        break;

    case ESP_GAP_BLE_SCAN_RESULT_EVT: {
        esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *)param;
        switch (scan_result->scan_rst.search_evt) {
        case ESP_GAP_SEARCH_INQ_RES_EVT:
	esp_log_buffer_hex(AP_TAG, scan_result->scan_rst.bda, 6);
//	ESP_LOGI(AP_TAG, "searched Adv Data Len %d, Scan Response Len %d", scan_result->scan_rst.adv_data_len, scan_result->scan_rst.scan_rsp_len);
            adv_name = esp_ble_resolve_adv_data(scan_result->scan_rst.ble_adv,
                                                ESP_BLE_AD_TYPE_NAME_CMPL, &adv_name_len);
	ESP_LOGI(AP_TAG, "searched Device Name Len %d", adv_name_len);
	esp_log_buffer_char(AP_TAG, adv_name, adv_name_len);

#if CONFIG_EXAMPLE_DUMP_ADV_DATA_AND_SCAN_RESP
            if (scan_result->scan_rst.adv_data_len > 0) {
                ESP_LOGI(AP_TAG, "adv data:");
                esp_log_buffer_hex(AP_TAG, &scan_result->scan_rst.ble_adv[0], scan_result->scan_rst.adv_data_len);
            }
            if (scan_result->scan_rst.scan_rsp_len > 0) {
                ESP_LOGI(AP_TAG, "Scan resp:");
                esp_log_buffer_hex(AP_TAG, &scan_result->scan_rst.ble_adv[scan_result->scan_rst.adv_data_len], scan_result->scan_rst.scan_rsp_len);
            }
#endif
            ESP_LOGI(AP_TAG, "\n");

            if (adv_name != NULL)  {
            int fnd_namelen = adv_name_len;
	    if (fnd_namelen > 15) fnd_namelen = 15;		
            mystrcpy(FND_NAME, (char *)adv_name,  fnd_namelen);
	}

            if ((adv_name != NULL) && (btopena || !btconnecta) && (btopenb || !btconnectb) && (btopenc || !btconnectc)) {

                if ( (REQ_NAMEA[0]) &&  strlen(REQ_NAMEA) == adv_name_len && strncmp((char *)adv_name, REQ_NAMEA, adv_name_len) == 0) {
                    ESP_LOGI(AP_TAG, "Searched A device %s\n", REQ_NAMEA);
                    if (btconnecta == false) {
			btopena = false;
                        btconnecta = true;
                        memcpy(&(scan_rsta), scan_result, sizeof(esp_ble_gap_cb_param_t));
                        if (Isscanning) esp_ble_gap_stop_scanning();
                    }
                }

                else if ( (REQ_NAMEB[0]) &&  strlen(REQ_NAMEB) == adv_name_len && strncmp((char *)adv_name, REQ_NAMEB, adv_name_len) == 0) {
                    ESP_LOGI(AP_TAG, "Searched B device %s\n", REQ_NAMEB);
                    if (btconnectb == false) {
			btopenb = false;
                        btconnectb = true;
                        memcpy(&(scan_rstb), scan_result, sizeof(esp_ble_gap_cb_param_t));
                        if (Isscanning) esp_ble_gap_stop_scanning();
                    }
                }

                else if ( (REQ_NAMEC[0]) &&  strlen(REQ_NAMEC) == adv_name_len && strncmp((char *)adv_name, REQ_NAMEC, adv_name_len) == 0) {
                    ESP_LOGI(AP_TAG, "Searched C device %s\n", REQ_NAMEC);
                    if (btconnectc == false) {
			btopenc = false;
                        btconnectc = true;
                        memcpy(&(scan_rstc), scan_result, sizeof(esp_ble_gap_cb_param_t));
                        if (Isscanning) esp_ble_gap_stop_scanning();
                    }
                }

            }
            break;
        case ESP_GAP_SEARCH_INQ_CMPL_EVT:
            break;
        default:
            break;
        }
        break;
    }

    case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
	StartingScan = false;
	Isscanning = false;
        if (param->scan_stop_cmpl.status != ESP_BT_STATUS_SUCCESS){
            ESP_LOGE(AP_TAG, "Scan stop failed, error status = %x", param->scan_stop_cmpl.status);
        } else ESP_LOGI(AP_TAG, "Scan stop successfully");

	if (!btopena && btconnecta) {
                        ESP_LOGI(AP_TAG, "Connect A to the remote device");
                        esp_ble_gattc_open(gl_profile_tab[PROFILE_A_APP_ID].gattc_if, scan_rsta.scan_rst.bda, scan_rsta.scan_rst.ble_addr_type, true);
	} else if (!btopenb && btconnectb) {
                        ESP_LOGI(AP_TAG, "Connect B to the remote device");
                        esp_ble_gattc_open(gl_profile_tab[PROFILE_B_APP_ID].gattc_if, scan_rstb.scan_rst.bda, scan_rstb.scan_rst.ble_addr_type, true);
	} else if (!btopenc && btconnectc) {
                        ESP_LOGI(AP_TAG, "Connect C to the remote device");
                        esp_ble_gattc_open(gl_profile_tab[PROFILE_C_APP_ID].gattc_if, scan_rstc.scan_rst.bda, scan_rstc.scan_rst.ble_addr_type, true);
	} else {
	start_scan();
	}
	break;

    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS){
            ESP_LOGE(AP_TAG, "Adv stop failed, error status = %x", param->adv_stop_cmpl.status);
            break;
        }
        ESP_LOGI(AP_TAG, "Stop adv successfully");
        break;
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
         ESP_LOGI(AP_TAG, "update connection params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
                  param->update_conn_params.status,
                  param->update_conn_params.min_int,
                  param->update_conn_params.max_int,
                  param->update_conn_params.conn_int,
                  param->update_conn_params.latency,
                  param->update_conn_params.timeout);
        break;
    default:
        break;
    }
}

static void esp_gattc_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
//   ESP_LOGI(AP_TAG, "EVT %d, gattc if %d, app_id %d", event, gattc_if, param->reg.app_id);

    /* If event is register event, store the gattc_if for each profile */
    if (event == ESP_GATTC_REG_EVT) {
        if (param->reg.status == ESP_GATT_OK) {
            gl_profile_tab[param->reg.app_id].gattc_if = gattc_if;
        } else {
            ESP_LOGI(AP_TAG, "reg app failed, app_id %04x, status %d",
                    param->reg.app_id,
                    param->reg.status);
            return;
        }
    }

    /* If the gattc_if equal to profile A, call profile A cb handler,
     * so here call each profile's callback */
    do {
        int idx;
        for (idx = 0; idx < PROFILE_NUM; idx++) {
            if (gattc_if == ESP_GATT_IF_NONE || /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
                    gattc_if == gl_profile_tab[idx].gattc_if) {
                if (gl_profile_tab[idx].gattc_cb) {
                    gl_profile_tab[idx].gattc_cb(event, gattc_if, param);
                }
            }
        }
    } while (0);
}
//******************** mi **********************
bool mAMiOff() {
	sendDataALen = 0;
	sendDataHandleA = 4;  //setup
	sendDataA[0] = 1;
	sendDataA[1] = 40;
	sendDataALen = 2;
	if (btauthoriza) {
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_A_APP_ID].remote_bda);
	notifyDataALen = -1;
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	while (--timeout && ((notifyDataALen == -1) || (notifyDataA[0] != 3) || (notifyDataA[4] != 40))) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	if ((notifyDataA[0] == 3) && (notifyDataA[1] == 2) && (notifyDataA[4] == 40)) return true;
	else {
	return false;
	}
	} else iRssiA = 0;
	return false;
}

bool mAMiOfft() {
	sendDataALen = 0;
	sendDataHandleA = 4;  //setup
	sendDataA[0] = 1;
	sendDataA[1] = bHtempA;
	sendDataALen = 2;
	if (btauthoriza) {
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_A_APP_ID].remote_bda);
	notifyDataALen = -1;
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	while (--timeout && ((notifyDataALen == -1) || (notifyDataA[0] != 3) || (notifyDataA[4] != 40))) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	if ((notifyDataA[0] == 3) && (notifyDataA[1] == 2) && (notifyDataA[4] == 40)) return true;
	else {
	return false;
	}
	} else iRssiA = 0;
	return false;
}

bool mBMiOff() {
	sendDataBLen = 0;
	sendDataHandleB = 4;  //setup
	sendDataB[0] = 1;
	sendDataB[1] = 40;
	sendDataBLen = 2;
	if (btauthorizb) {
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_B_APP_ID].remote_bda);
	notifyDataBLen = -1;
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	while (--timeout && ((notifyDataBLen == -1) || (notifyDataB[0] != 3) || (notifyDataB[4] != 40))) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	if ((notifyDataB[0] == 3) && (notifyDataB[1] == 2) && (notifyDataB[4] == 40)) return true;
	else {
	return false;
	}
	} else iRssiB = 0;
	return false;
}

bool mBMiOfft() {
	sendDataBLen = 0;
	sendDataHandleB = 4;  //setup
	sendDataB[0] = 1;
	sendDataB[1] = bHtempB;
	sendDataBLen = 2;
	if (btauthorizb) {
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_B_APP_ID].remote_bda);
	notifyDataBLen = -1;
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	while (--timeout && ((notifyDataBLen == -1) || (notifyDataB[0] != 3) || (notifyDataB[4] != 40))) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	if ((notifyDataB[0] == 3) && (notifyDataB[1] == 2) && (notifyDataB[4] == 40)) return true;
	else {
	return false;
	}
	} else iRssiB = 0;
	return false;
}

bool mCMiOff() {
	sendDataCLen = 0;
	sendDataHandleC = 4;  //setup
	sendDataC[0] = 1;
	sendDataC[1] = 40;
	sendDataCLen = 2;
	if (btauthorizc) {
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_C_APP_ID].remote_bda);
	notifyDataCLen = -1;
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	while (--timeout && ((notifyDataCLen == -1) || (notifyDataC[0] != 3) || (notifyDataC[4] != 40))) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	if ((notifyDataC[0] == 3) && (notifyDataC[1] == 2) && (notifyDataC[4] == 40)) return true;
	else {
	return false;
	}
	} else iRssiC = 0;
	return false;
}

bool mCMiOfft() {
	sendDataCLen = 0;
	sendDataHandleC = 4;  //setup
	sendDataC[0] = 1;
	sendDataC[1] = bHtempC;
	sendDataCLen = 2;
	if (btauthorizc) {
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_C_APP_ID].remote_bda);
	notifyDataCLen = -1;
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	while (--timeout && ((notifyDataCLen == -1) || (notifyDataC[0] != 3) || (notifyDataC[4] != 40))) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	if ((notifyDataC[0] == 3) && (notifyDataC[1] == 2) && (notifyDataC[4] == 40)) return true;
	else {
	return false;
	}
	} else iRssiC = 0;
	return false;
}

bool mAMiBoil() {
	sendDataALen = 0;
	sendDataHandleA = 4;  //setup
	sendDataA[0] = 0;
	sendDataA[1] = bHtempA;
	sendDataALen = 2;
	if (btauthoriza) {
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_A_APP_ID].remote_bda);
	notifyDataALen = -1;
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	while (--timeout && ((notifyDataALen == -1) || (notifyDataA[0] != 1))) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	if ((notifyDataA[0] == 1) && (notifyDataA[1] == 2)) return true;
	else {
	return false;
	}
	} else iRssiA = 0;
	return true;
}

bool mBMiBoil() {
	sendDataBLen = 0;
	sendDataHandleB = 4;  //setup
	sendDataB[0] = 0;
	sendDataB[1] = bHtempB;
	sendDataBLen = 2;
	if (btauthorizb) {
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_B_APP_ID].remote_bda);
	notifyDataBLen = -1;
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	while (--timeout && ((notifyDataBLen == -1) || (notifyDataB[0] != 1))) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	if ((notifyDataB[0] == 1) && (notifyDataB[1] == 2)) return true;
	else {
	return false;
	}
	} else iRssiB = 0;
	return true;
}
bool mCMiBoil() {
	sendDataCLen = 0;
	sendDataHandleC = 4;  //setup
	sendDataC[0] = 0;
	sendDataC[1] = bHtempC;
	sendDataCLen = 2;
	if (btauthorizc) {
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_C_APP_ID].remote_bda);
	notifyDataCLen = -1;
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	while (--timeout && ((notifyDataCLen == -1) || (notifyDataC[0] != 1))) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	if ((notifyDataC[0] == 1) && (notifyDataC[1] == 2)) return true;
	else {
	return false;
	}
	} else iRssiC = 0;
	return true;
}

bool mAMiHeat(uint8_t temp) {
	if (btauthoriza) {
	if (temp) {
	sendDataALen = 0;
	sendDataHandleA = 4;  //setup
	sendDataA[0] = 1;
	sendDataA[1] = temp;
	sendDataALen = 2;
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_A_APP_ID].remote_bda);
	notifyDataALen = -1;
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	while (--timeout && ((notifyDataALen == -1) || (notifyDataA[0] != 3) || (notifyDataA[4] != temp))) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	if ((notifyDataA[0] == 3) && (notifyDataA[1] == 2) && (notifyDataA[4] == temp)) return true;
	else {
	return false;
	}
	} else {
	xshedcoma = 2;
	sendDataALen = 0;
	sendDataHandleA = 4;  //setup
	sendDataA[0] = 1;
	sendDataA[1] = bCtempA;
	sendDataALen = 2;
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_A_APP_ID].remote_bda);
	notifyDataALen = -1;
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	while (--timeout && ((notifyDataALen == -1) || (notifyDataA[0] != 3) || (notifyDataA[4] != bCtempA))) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	sendDataHandleA = 5;  //time
	sendDataA[0] = 0;
	sendDataALen = 1;
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_A_APP_ID].remote_bda);

	}
	} else iRssiA = 0;
	return true;
}

bool mBMiHeat(uint8_t temp) {
	if (btauthorizb) {
	if (temp) {
	sendDataBLen = 0;
	sendDataHandleB = 4;  //setup
	sendDataB[0] = 1;
	sendDataB[1] = temp;
	sendDataBLen = 2;
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_B_APP_ID].remote_bda);
	notifyDataBLen = -1;
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	while (--timeout && ((notifyDataBLen == -1) || (notifyDataB[0] != 3) || (notifyDataB[4] != temp))) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	if ((notifyDataB[0] == 3) && (notifyDataB[1] == 2) && (notifyDataB[4] == temp)) return true;
	else {
	return false;
	}
	} else {
	xshedcomb = 2;
	sendDataBLen = 0;
	sendDataHandleB = 4;  //setup
	sendDataB[0] = 1;
	sendDataB[1] = bCtempB;
	sendDataBLen = 2;
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_B_APP_ID].remote_bda);
	notifyDataBLen = -1;
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	while (--timeout && ((notifyDataBLen == -1) || (notifyDataB[0] != 3) || (notifyDataB[4] != bCtempB))) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	sendDataHandleB = 5;  //time
	sendDataB[0] = 0;
	sendDataBLen = 1;
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_B_APP_ID].remote_bda);

	}
	} else iRssiB = 0;
	return true;
}

bool mCMiHeat(uint8_t temp) {
	if (btauthorizc) {
	if (temp) {
	sendDataCLen = 0;
	sendDataHandleC = 4;  //setup
	sendDataC[0] = 1;
	sendDataC[1] = temp;
	sendDataCLen = 2;
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_C_APP_ID].remote_bda);
	notifyDataCLen = -1;
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	while (--timeout && ((notifyDataCLen == -1) || (notifyDataC[0] != 3) || (notifyDataC[4] != temp))) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	if ((notifyDataC[0] == 3) && (notifyDataC[1] == 2) && (notifyDataC[4] == temp)) return true;
	else {
	return false;
	}
	} else {
	xshedcomc = 2;
	sendDataCLen = 0;
	sendDataHandleC = 4;  //setup
	sendDataC[0] = 1;
	sendDataC[1] = bCtempC;
	sendDataCLen = 2;
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_C_APP_ID].remote_bda);
	notifyDataCLen = -1;
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	while (--timeout && ((notifyDataCLen == -1) || (notifyDataC[0] != 3) || (notifyDataC[4] != bCtempC))) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	sendDataHandleC = 5;  //time
	sendDataC[0] = 0;
	sendDataCLen = 1;
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_C_APP_ID].remote_bda);

	}
	} else iRssiC = 0;
	return true;
}


bool mAMiIdlTmp(uint8_t temp) {
	sendDataALen = 0;
	sendDataHandleA = 4;  //setup
	sendDataA[0] = 1;
	sendDataA[1] = temp;
	sendDataALen = 2;
	if (btauthoriza) {
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_A_APP_ID].remote_bda);
	notifyDataALen = -1;
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	while (--timeout && ((notifyDataALen == -1) || (notifyDataA[4] != temp))) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	if (notifyDataA[4] == temp) return true;
	else return false;
	} else iRssiA = 0;
	return true;
}

bool mBMiIdlTmp(uint8_t temp) {
	sendDataBLen = 0;
	sendDataHandleB = 4;  //setup
	sendDataB[0] = 1;
	sendDataB[1] = temp;
	sendDataBLen = 2;
	if (btauthorizb) {
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_B_APP_ID].remote_bda);
	notifyDataBLen = -1;
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	while (--timeout && ((notifyDataBLen == -1) || (notifyDataB[4] != temp))) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	if (notifyDataB[4] == temp) return true;
	else return false;
	} else iRssiB = 0;
	return true;
}

bool mCMiIdlTmp(uint8_t temp) {
	sendDataCLen = 0;
	sendDataHandleC = 4;  //setup
	sendDataC[0] = 1;
	sendDataC[1] = temp;
	sendDataCLen = 2;
	if (btauthorizc) {
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_C_APP_ID].remote_bda);
	notifyDataCLen = -1;
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	while (--timeout && ((notifyDataCLen == -1) || (notifyDataC[4] != temp))) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	if (notifyDataC[4] == temp) return true;
	else return false;
	} else iRssiC = 0;
	return true;
}

bool mAMiRewarm() {
	bool retc = true;
	if (btauthoriza) {
	xshedcoma = 1;
	sendDataALen = 0;
	sendDataHandleA = 4;  //setup
	sendDataA[0] = 0;
	sendDataA[1] = bHtempA;
	sendDataALen = 2;
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_A_APP_ID].remote_bda);
	notifyDataALen = -1;
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	while (--timeout && ((notifyDataALen == -1) || (notifyDataA[0] != 1))) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	if (notifyDataA[0] != 1) retc = false;
	sendDataHandleA = 4;  //setup
	sendDataA[0] = 1;
	sendDataA[1] = bHtempA;
	sendDataALen = 2;
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_A_APP_ID].remote_bda);
	notifyDataALen = -1;
	timeout = 30; 	// 100*30 = 3 seconds
	while (--timeout && ((notifyDataALen == -1) || (notifyDataA[0] != 3))) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	if ((notifyDataA[0] != 3) || (notifyDataA[1] != 2)) retc = false;
	} else {
	iRssiA = 0;
	retc = false;
	}
	if (retc) xshedcoma = 0;
	return retc;
}

bool mBMiRewarm() {
	bool retc = true;
	if (btauthorizb) {
	xshedcomb = 1;
	sendDataBLen = 0;
	sendDataHandleB = 4;  //setup
	sendDataB[0] = 0;
	sendDataB[1] = bHtempA;
	sendDataBLen = 2;
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_B_APP_ID].remote_bda);
	notifyDataBLen = -1;
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	while (--timeout && ((notifyDataBLen == -1) || (notifyDataB[0] != 1))) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	if (notifyDataB[0] != 1) retc = false;
	sendDataHandleB = 4;  //setup
	sendDataB[0] = 1;
	sendDataB[1] = bHtempA;
	sendDataBLen = 2;
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_B_APP_ID].remote_bda);
	notifyDataBLen = -1;
	timeout = 30; 	// 100*30 = 3 seconds
	while (--timeout && ((notifyDataBLen == -1) || (notifyDataB[0] != 3))) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	if ((notifyDataB[0] != 3) || (notifyDataB[1] != 2)) retc = false;
	} else {
	iRssiB = 0;
	retc = false;
	}
	if (retc) xshedcomb = 0;
	return retc;
}

bool mCMiRewarm() {
	bool retc = true;
	if (btauthorizc) {
	xshedcomc = 1;
	sendDataCLen = 0;
	sendDataHandleC = 4;  //setup
	sendDataC[0] = 0;
	sendDataC[1] = bHtempA;
	sendDataCLen = 2;
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_C_APP_ID].remote_bda);
	notifyDataCLen = -1;
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	while (--timeout && ((notifyDataCLen == -1) || (notifyDataC[0] != 1))) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	if (notifyDataC[0] != 1) retc = false;
	sendDataHandleC = 4;  //setup
	sendDataC[0] = 1;
	sendDataC[1] = bHtempA;
	sendDataCLen = 2;
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_C_APP_ID].remote_bda);
	notifyDataCLen = -1;
	timeout = 30; 	// 100*30 = 3 seconds
	while (--timeout && ((notifyDataCLen == -1) || (notifyDataC[0] != 3))) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	if ((notifyDataC[0] != 3) || (notifyDataC[1] != 2)) retc = false;
	} else {
	iRssiC = 0;
	retc = false;
	}
	if (retc) xshedcomc = 0;
	return retc;
}

bool mAMiSWtime() {
	bool retc = true;
	if (btauthoriza) {
	sendDataALen = 0;
	sendDataHandleA = 5;  //time
	sendDataA[0] = 0x18;
	sendDataALen = 1;
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_A_APP_ID].remote_bda);
	notifyDataALen = -1;
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	while (--timeout && ((notifyDataALen == -1) || (notifyDataA[10] != 0x18))) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	if (notifyDataA[10] != 0x18) retc = false;
	} else {
	iRssiA = 0;
	retc = false;
	}
	return retc;
}

bool mBMiSWtime() {
	bool retc = true;
	if (btauthorizb) {
	sendDataBLen = 0;
	sendDataHandleB = 5;  //time
	sendDataB[0] = 0x18;
	sendDataBLen = 1;
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_B_APP_ID].remote_bda);
	notifyDataBLen = -1;
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	while (--timeout && ((notifyDataBLen == -1) || (notifyDataB[10] != 0x18))) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	if (notifyDataB[10] != 0x18) retc = false;
	} else {
	iRssiB = 0;
	retc = false;
	}
	return retc;
}

bool mCMiSWtime() {
	bool retc = true;
	if (btauthorizc) {
	sendDataCLen = 0;
	sendDataHandleC = 5;  //time
	sendDataC[0] = 0x18;
	sendDataCLen = 1;
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_C_APP_ID].remote_bda);
	notifyDataCLen = -1;
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	while (--timeout && ((notifyDataCLen == -1) || (notifyDataC[10] != 0x18))) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	if (notifyDataC[10] != 0x18) retc = false;
	} else {
	iRssiC = 0;
	retc = false;
	}
	return retc;
}

//******************** r4s **********************

//[I][R4S.cpp:24]   r4sWriteA(): >> 55 59 06 aa
//                         offset:  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19
//[I][R4S.cpp:43] r4sCommandA(): << 55 59 06 00 00 00 00 00 02 00 00 00 00 39 00 00 00 00 00 aa

uint8_t r4sWriteA(uint8_t cmd, uint8_t* data, size_t len) {
	size_t sz = 4 + len; // 55, counter, cmd, AA
	sendDataA[0] = 0x55;
	sendDataA[1] = r4sAcounter;
	sendDataA[2] = cmd;
	sendDataA[sz - 1] = 0xAA;
	if (len > 0) {
	memcpy(&sendDataA[3], data, len);
		}
	sendDataALen = sz;
//  ble_gap_read_rssi event return rssi and start sending data
	if (btauthoriza) esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_A_APP_ID].remote_bda);
	else {
	iRssiA = 0;
	r4sAcounter = -1;
	}
	return r4sAcounter++;
}

uint8_t r4sWriteB(uint8_t cmd, uint8_t* data, size_t len) {
	size_t sz = 4 + len; // 55, counter, cmd, AA
	sendDataB[0] = 0x55;
	sendDataB[1] = r4sBcounter;
	sendDataB[2] = cmd;
	sendDataB[sz - 1] = 0xAA;
	if (len > 0) {
	memcpy(&sendDataB[3], data, len);
		}
	sendDataBLen = sz;
//  ble_gap_read_rssi event return rssi and start sending data
	if (btauthorizb) esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_B_APP_ID].remote_bda);
	else {
	iRssiB = 0;
	r4sBcounter = -1;
	}
	return r4sBcounter++;
}

uint8_t r4sWriteC(uint8_t cmd, uint8_t* data, size_t len) {
	size_t sz = 4 + len; // 55, counter, cmd, AA
	sendDataC[0] = 0x55;
	sendDataC[1] = r4sCcounter;
	sendDataC[2] = cmd;
	sendDataC[sz - 1] = 0xAA;
	if (len > 0) {
	memcpy(&sendDataC[3], data, len);
		}
	sendDataCLen = sz;
//  ble_gap_read_rssi event return rssi and start sending data
	if (btauthorizc) esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_C_APP_ID].remote_bda);
	else {
	iRssiC = 0;
	r4sCcounter = -1;
	}
	return r4sCcounter++;
}


int8_t r4sCommandA(uint8_t cmd, uint8_t* data, size_t len) {
	notifyDataALen = -1;
	if (btauthoriza) {
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	uint8_t cnt = r4sWriteA(cmd, data, len);
	while (--timeout && (notifyDataALen == -1)) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
//
	if ((notifyDataALen < BLE_INPUT_BUFFSIZE) && (notifyDataALen > 1)) {
	r4sAConnErr = 0;	
ESP_LOGI(AP_TAG, "Receive Data A:");
esp_log_buffer_hex(AP_TAG, notifyDataA, notifyDataALen);
	}
//
	if ((notifyDataALen > 1) && (notifyDataA[1] != cnt)) {
	notifyDataALen = -1;
	r4sAConnErr++;	
	}
	}
	return notifyDataALen;
}

int8_t r4sCommandB(uint8_t cmd, uint8_t* data, size_t len) {
	notifyDataBLen = -1;
	if (btauthorizb) {
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	uint8_t cnt = r4sWriteB(cmd, data, len);
	while (--timeout && (notifyDataBLen == -1)) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
//
	if ((notifyDataBLen < BLE_INPUT_BUFFSIZE) && (notifyDataBLen > 1)) {
	r4sBConnErr = 0;	
ESP_LOGI(AP_TAG, "Receive Data B:");
esp_log_buffer_hex(AP_TAG, notifyDataB, notifyDataBLen);
	}
//
	if ((notifyDataBLen > 1) && (notifyDataB[1] != cnt)) {
	notifyDataBLen = -1;
	r4sBConnErr++;	
	}
	}
	return notifyDataBLen;
}

int8_t r4sCommandC(uint8_t cmd, uint8_t* data, size_t len) {
	notifyDataCLen = -1;
	if (btauthorizc) {
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	uint8_t cnt = r4sWriteC(cmd, data, len);
	while (--timeout && (notifyDataCLen == -1)) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
//
	if ((notifyDataCLen < BLE_INPUT_BUFFSIZE) && (notifyDataCLen > 1)) {
	r4sCConnErr = 0;	
ESP_LOGI(AP_TAG, "Receive Data C:");
esp_log_buffer_hex(AP_TAG, notifyDataC, notifyDataCLen);
	}
//
	if ((notifyDataCLen > 1) && (notifyDataC[1] != cnt)) {
	notifyDataCLen = -1;
	r4sCConnErr++;	
	}
	}
	return notifyDataCLen;
}


bool mA171sOff() {
	if (r4sCommandA(0x04, 0, 0) != 5)
	return false;
	return notifyDataA[3] == 1;
}

bool mB171sOff() {
	if (r4sCommandB(0x04, 0, 0) != 5)
	return false;
	return notifyDataB[3] == 1;
}

bool mC171sOff() {
	if (r4sCommandC(0x04, 0, 0) != 5)
	return false;
	return notifyDataC[3] == 1;
}

bool mA103sOn() {
	if (r4sCommandA(0x03, 0, 0) != 5)
	return false;
	return notifyDataA[3] == 1;
}

bool mB103sOn() {
	if (r4sCommandB(0x03, 0, 0) != 5)
	return false;
	return notifyDataB[3] == 1;
}

bool mC103sOn() {
	if (r4sCommandC(0x03, 0, 0) != 5)
	return false;
	return notifyDataC[3] == 1;
}


bool mA103sLon() {
	uint8_t data[] = { 1 };
	if (r4sCommandA(0x16, data, sizeof(data)) != 5)
	return false;
	return notifyDataA[3] == 1;
}

bool mA103sLoff() {
	uint8_t data[] = { 0 };
	if (r4sCommandA(0x16, data, sizeof(data)) != 5)
	return false;
	return notifyDataA[3] == 1;
}

bool mB103sLon() {
	uint8_t data[] = { 1 };
	if (r4sCommandB(0x16, data, sizeof(data)) != 5)
	return false;
	return notifyDataB[3] == 1;
}

bool mB103sLoff() {
	uint8_t data[] = { 0 };
	if (r4sCommandB(0x16, data, sizeof(data)) != 5)
	return false;
	return notifyDataB[3] == 1;
}

bool mC103sLon() {
	uint8_t data[] = { 1 };
	if (r4sCommandC(0x16, data, sizeof(data)) != 5)
	return false;
	return notifyDataC[3] == 1;
}

bool mC103sLoff() {
	uint8_t data[] = { 0 };
	if (r4sCommandC(0x16, data, sizeof(data)) != 5)
	return false;
	return notifyDataC[3] == 1;
}


bool mA103sTon() {
	uint8_t data[] = { 1 };
	if (r4sCommandA(0x1b, data, sizeof(data)) != 5)
	return false;
	return notifyDataA[3] == 1;
}

bool mA103sToff() {
	uint8_t data[] = { 0 };
	if (r4sCommandA(0x1b, data, sizeof(data)) != 5)
	return false;
	return notifyDataA[3] == 1;
}

bool mB103sTon() {
	uint8_t data[] = { 1 };
	if (r4sCommandB(0x1b, data, sizeof(data)) != 5)
	return false;
	return notifyDataB[3] == 1;
}

bool mB103sToff() {
	uint8_t data[] = { 0 };
	if (r4sCommandB(0x1b, data, sizeof(data)) != 5)
	return false;
	return notifyDataB[3] == 1;
}

bool mC103sTon() {
	uint8_t data[] = { 1 };
	if (r4sCommandC(0x1b, data, sizeof(data)) != 5)
	return false;
	return notifyDataC[3] == 1;
}

bool mC103sToff() {
	uint8_t data[] = { 0 };
	if (r4sCommandC(0x1b, data, sizeof(data)) != 5)
	return false;
	return notifyDataC[3] == 1;
}

bool mA151sDon(uint8_t phour, uint8_t pmin) {
	uint8_t data[] = { 0, 0, 0, phour, pmin, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0};
	if (r4sCommandA(0x05, data, sizeof(data)) != 5)
	return false;
	return notifyDataA[3] == 1;

	if (r4sCommandA(0x03, 0, 0) != 5)
	return false;
	if (notifyDataA[3] != 1)
	return false;
    
	return true;
}

bool mB151sDon(uint8_t phour, uint8_t pmin) {
	uint8_t data[] = { 0, 0, 0, phour, pmin, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0};
	if (r4sCommandB(0x05, data, sizeof(data)) != 5)
	return false;
	return notifyDataB[3] == 1;

	if (r4sCommandB(0x03, 0, 0) != 5)
	return false;
	if (notifyDataB[3] != 1)
	return false;
    
	return true;
}

bool mC151sDon(uint8_t phour, uint8_t pmin) {
	uint8_t data[] = { 0, 0, 0, phour, pmin, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0};
	if (r4sCommandC(0x05, data, sizeof(data)) != 5)
	return false;
	return notifyDataC[3] == 1;

	if (r4sCommandC(0x03, 0, 0) != 5)
	return false;
	if (notifyDataC[3] != 1)
	return false;
    
	return true;
}

bool mA151sLon() {
	uint8_t data[] = { 1 };
	if (r4sCommandA(0x3e, data, sizeof(data)) != 5)
	return false;
	return notifyDataA[3] == 1;
}

bool mA151sLoff() {
	uint8_t data[] = { 0 };
	if (r4sCommandA(0x3e, data, sizeof(data)) != 5)
	return false;
	return notifyDataA[3] == 1;
}

bool mB151sLon() {
	uint8_t data[] = { 1 };
	if (r4sCommandB(0x3e, data, sizeof(data)) != 5)
	return false;
	return notifyDataB[3] == 1;
}

bool mB151sLoff() {
	uint8_t data[] = { 0 };
	if (r4sCommandB(0x3e, data, sizeof(data)) != 5)
	return false;
	return notifyDataB[3] == 1;
}

bool mC151sLon() {
	uint8_t data[] = { 1 };
	if (r4sCommandC(0x3e, data, sizeof(data)) != 5)
	return false;
	return notifyDataC[3] == 1;
}

bool mC151sLoff() {
	uint8_t data[] = { 0 };
	if (r4sCommandC(0x3e, data, sizeof(data)) != 5)
	return false;
	return notifyDataC[3] == 1;
}



bool mA171sOn(uint8_t prog, uint8_t temp) {
	if (DEV_TYPA > 3) {
	uint8_t data[] = { prog, 0, temp, 0, 1, temp, 30, 0, 0, 0, 0, 0, 0, 128, 0, 0};
	if (r4sCommandA(0x05, data, sizeof(data)) != 5) return false;
	if (notifyDataA[3] != 1) return false;
	if (r4sCommandA(0x03, 0, 0) != 5) return false;
	if (notifyDataA[3] != 1) return false;
	} else if (DEV_TYPA == 1) {	
	uint8_t data[] = { prog, temp, 0, 0 };
	if (r4sCommandA(0x05, data, sizeof(data)) != 5) return false;
	if (notifyDataA[3] == 1) return  false;
	} else {
	uint8_t data[] = { prog, 0, temp, 0 };
	if (r4sCommandA(0x05, data, sizeof(data)) != 5) return false;
	if (notifyDataA[3] == 1) return  false;
	if (DEV_TYPA > 2) {
	if (r4sCommandA(0x03, 0, 0) != 5) return false;
	if (notifyDataA[3] != 1) return false;
	}
	}
	return true;
}

bool mB171sOn(uint8_t prog, uint8_t temp) {
	if (DEV_TYPB > 3) {
	uint8_t data[] = { prog, 0, temp, 0, 1, temp, 30, 0, 0, 0, 0, 0, 0, 128, 0, 0};
	if (r4sCommandB(0x05, data, sizeof(data)) != 5) return false;
	if (notifyDataB[3] != 1) return false;
	if (r4sCommandB(0x03, 0, 0) != 5) return false;
	if (notifyDataB[3] != 1) return false;
	} else if (DEV_TYPB == 1) {	
	uint8_t data[] = { prog, temp, 0, 0 };
	if (r4sCommandB(0x05, data, sizeof(data)) != 5) return false;
	if (notifyDataB[3] == 1) return  false;
	} else {
	uint8_t data[] = { prog, 0, temp, 0 };
	if (r4sCommandB(0x05, data, sizeof(data)) != 5) return false;
	if (notifyDataB[3] == 1) return  false;
	if (DEV_TYPB > 2) {
	if (r4sCommandB(0x03, 0, 0) != 5) return false;
	if (notifyDataB[3] != 1) return false;
	}
	}
	return true;
}

bool mC171sOn(uint8_t prog, uint8_t temp) {
	if (DEV_TYPC > 3) {
	uint8_t data[] = { prog, 0, temp, 0, 1, temp, 30, 0, 0, 0, 0, 0, 0, 128, 0, 0};
	if (r4sCommandC(0x05, data, sizeof(data)) != 5) return false;
	if (notifyDataC[3] != 1) return false;
	if (r4sCommandC(0x03, 0, 0) != 5) return false;
	if (notifyDataC[3] != 1) return false;
	} else if (DEV_TYPC == 1) {	
	uint8_t data[] = { prog, temp, 0, 0 };
	if (r4sCommandC(0x05, data, sizeof(data)) != 5) return false;
	if (notifyDataC[3] == 1) return  false;
	} else {
	uint8_t data[] = { prog, 0, temp, 0 };
	if (r4sCommandC(0x05, data, sizeof(data)) != 5) return false;
	if (notifyDataC[3] == 1) return  false;
	if (DEV_TYPC > 2) {
	if (r4sCommandC(0x03, 0, 0) != 5) return false;
	if (notifyDataC[3] != 1) return false;
	}
	}
	return true;
}

bool mA171sBoil() {
	return mA171sOn(0, 0);
}

bool mB171sBoil() {
	return mB171sOn(0, 0);
}

bool mC171sBoil() {
	return mC171sOn(0, 0);
}

bool mA171sHeat(uint8_t temp) {
	return mA171sOn(1, temp);
}

bool mB171sHeat(uint8_t temp) {
	return mB171sOn(1, temp);
}

bool mC171sHeat(uint8_t temp) {
	return mC171sOn(1, temp);
}

bool mA171sBoilAndHeat(uint8_t temp) {
	if (DEV_TYPA > 2)  return mA171sOn(2, temp);
	else return mA171sOn(0, temp);
}

bool mB171sBoilAndHeat(uint8_t temp) {
	if (DEV_TYPB > 2)  return mB171sOn(2, temp);
	else return mB171sOn(0, temp);
}

bool mC171sBoilAndHeat(uint8_t temp) {
	if (DEV_TYPC > 2)  return mC171sOn(2, temp);
	else return mC171sOn(0, temp);
}

bool mA171s_NLOn() {
	if (r4sCommandA(0x32, nl_settings, sizeof(nl_settings)) != 5)
	return false;
	if (notifyDataA[3] == 1)
	return false;
    
	uint8_t data[] = { 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 0, 0};
	if (r4sCommandA(0x05, data, sizeof(data)) != 5)
	return false;
	if (notifyDataA[3] == 1)
	return false;
    
	if (r4sCommandA(0x03, 0, 0) != 5)
	return false;
	if (notifyDataA[3] != 1)
	return false;
    
	return true;
}

bool mB171s_NLOn() {
	if (r4sCommandB(0x32, nl_settings, sizeof(nl_settings)) != 5)
	return false;
	if (notifyDataB[3] == 1)
	return false;
    
	uint8_t data[] = { 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 0, 0};
	if (r4sCommandB(0x05, data, sizeof(data)) != 5)
	return false;
	if (notifyDataB[3] == 1)
	return false;
    
	if (r4sCommandB(0x03, 0, 0) != 5)
	return false;
	if (notifyDataB[3] != 1)
	return false;
    
	return true;
}

bool mC171s_NLOn() {
	if (r4sCommandC(0x32, nl_settings, sizeof(nl_settings)) != 5)
	return false;
	if (notifyDataC[3] == 1)
	return false;
    
	uint8_t data[] = { 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 0, 0};
	if (r4sCommandC(0x05, data, sizeof(data)) != 5)
	return false;
	if (notifyDataC[3] == 1)
	return false;
    
	if (r4sCommandC(0x03, 0, 0) != 5)
	return false;
	if (notifyDataC[3] != 1)
	return false;
    
	return true;
}

bool mA171s_ModOff() {
//	if (!bProgA) return true;
	if (DEV_TYPA > 3) {
	uint8_t data[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 0, 0};
	if (r4sCommandA(0x05, data, sizeof(data)) != 5) return false;
	if (notifyDataA[3] == 1) return false;
	} else if (DEV_TYPA == 1) {	
	uint8_t data[] = { 1, 0, 0, 0 };  // heat,temp,0,0
	if (r4sCommandA(0x05, data, sizeof(data)) != 5) return false;
	if (notifyDataA[3] == 1) return false;
	} else {
	uint8_t data[] = { 1, 0, 0, 0 };  // heat,0,temp,0
	if (r4sCommandA(0x05, data, sizeof(data)) != 5) return false;
	if (notifyDataA[3] == 1) return  false;
	}
	return true;
}

bool mB171s_ModOff() {
//	if (!bProgB) return true;
	if (DEV_TYPB > 3) {
	uint8_t data[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 0, 0};
	if (r4sCommandB(0x05, data, sizeof(data)) != 5) return false;
	if (notifyDataB[3] == 1) return false;
	} else if (DEV_TYPB == 1) {	
	uint8_t data[] = { 1, 0, 0, 0 };  // heat,temp,0,0
	if (r4sCommandB(0x05, data, sizeof(data)) != 5) return false;
	if (notifyDataB[3] == 1) return false;
	} else {
	uint8_t data[] = { 1, 0, 0, 0 };  // heat,0,temp,0
	if (r4sCommandB(0x05, data, sizeof(data)) != 5) return false;
	if (notifyDataB[3] == 1) return  false;
	}
	return true;
}

bool mC171s_ModOff() {
//	if (!bProgC) return true;
	if (DEV_TYPC > 3) {
	uint8_t data[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 0, 0};
	if (r4sCommandC(0x05, data, sizeof(data)) != 5) return false;
	if (notifyDataC[3] == 1) return false;
	} else if (DEV_TYPC == 1) {	
	uint8_t data[] = { 1, 0, 0, 0 };  // heat,temp,0,0
	if (r4sCommandC(0x05, data, sizeof(data)) != 5) return false;
	if (notifyDataC[3] == 1) return false;
	} else {
	uint8_t data[] = { 1, 0, 0, 0 };  // heat,0,temp,0
	if (r4sCommandC(0x05, data, sizeof(data)) != 5) return false;
	if (notifyDataC[3] == 1) return  false;
	}
	return true;
}


/*
bool mC171s_ModOff() {
	if (DEV_TYPC > 3) {
	uint8_t data[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 0, 0};
	if (r4sCommandC(0x05, data, sizeof(data)) != 5) return false;
	if (notifyDataC[3] == 1) return false;
        if (r4sCommandC(0x03, 0, 0) != 5) return false;
	if (notifyDataC[3] != 1) return false;
	} else {
	uint8_t data[] = { 0, 0, 0, 0 };
	if (r4sCommandC(0x05, data, sizeof(data)) != 5) return false;
	if (notifyDataC[3] == 1) return  false;
	if (DEV_TYPC > 2) {
	if (r4sCommandC(0x03, 0, 0) != 5) return false;
	if (notifyDataC[3] != 1) return false;
	}
	}
	return true;
}
*/

bool mA171Bl(uint8_t state) {
	uint8_t data[] = { 0xc8, 0xc8, state };
	if (DEV_TYPA > 3) {
	if (r4sCommandA(0x37, data, sizeof(data)) != 5) return false;
	if (notifyDataA[3] != 0) return false;
	}
	return true;
}

bool mA171Bp(uint8_t state) {
	uint8_t data[] = { state };
	if (DEV_TYPA > 3) {
	if (r4sCommandA(0x3c, data, sizeof(data)) != 5) return false;
	if (notifyDataA[3] != 1) return false;
	}
	return true;
}

bool mB171Bl(uint8_t state) {
	uint8_t data[] = { 0xc8, 0xc8, state };
	if (DEV_TYPB > 3) {
	if (r4sCommandB(0x37, data, sizeof(data)) != 5) return false;
	if (notifyDataB[3] != 0) return false;
	}
	return true;
}

bool mB171Bp(uint8_t state) {
	uint8_t data[] = { state };
	if (DEV_TYPB > 3) {
	if (r4sCommandB(0x3c, data, sizeof(data)) != 5) return false;
	if (notifyDataB[3] != 1) return false;
	}
	return true;
}

bool mC171Bl(uint8_t state) {
	uint8_t data[] = { 0xc8, 0xc8, state };
	if (DEV_TYPC > 3) {
	if (r4sCommandC(0x37, data, sizeof(data)) != 5) return false;
	if (notifyDataC[3] != 0) return false;
	}
	return true;
}

bool mC171Bp(uint8_t state) {
	uint8_t data[] = { state };
	if (DEV_TYPC > 3) {
	if (r4sCommandC(0x3c, data, sizeof(data)) != 5) return false;
	if (notifyDataC[3] != 1) return false;
	}
	return true;
}

bool rmA800sOn() {
	if (r4sCommandA(0x03, 0, 0) != 5)
	return false;
	return notifyDataA[3] == 1;
}

bool rmB800sOn() {
	if (r4sCommandB(0x03, 0, 0) != 5)
	return false;
	return notifyDataB[3] == 1;
}

bool rmC800sOn() {
	if (r4sCommandC(0x03, 0, 0) != 5)
	return false;
	return notifyDataC[3] == 1;
}

bool rmA800sOff() {
	if (r4sCommandA(0x04, 0, 0) != 5)
	return false;
	return notifyDataA[3] == 1;
}

bool rmB800sOff() {
	if (r4sCommandB(0x04, 0, 0) != 5)
	return false;
	return notifyDataB[3] == 1;
}

bool rmC800sOff() {
	if (r4sCommandC(0x04, 0, 0) != 5)
	return false;
	return notifyDataC[3] == 1;
}

bool rmA800sPall(uint8_t prog, uint8_t mod, uint8_t temp, uint8_t phour, uint8_t pmin, uint8_t dhour, uint8_t dmin, uint8_t warm) {
	if (( DEV_TYPA == 16 ) && (prog > 12)) return false;
	if (( DEV_TYPA == 17 ) && (prog > 16)) return false;
	uint8_t data[] = { prog, 0, 0, 0, 0, dhour, dmin, warm};
        if ( DEV_TYPA < 24 ) {
        if ( DEV_TYPA == 16 ) {
// for RMC-800s
	switch (prog) {
	case 0:		//multicooker мультиповар
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 30;		//smin
	break;

	case 1:		//rice рис/крупы
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 35;		//smin
	break;

	case 2:		//slow_cooking  томление
	data[1] = 0;     	//mode
	data[2] = 97;		//temp
	data[3] = 3;		//shour
	data[4] = 0;		//smin
	break;

	case 3:		//pilaf плов
	data[1] = 0;     	//mode
	data[2] = 110;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 4:		//frying_vegetables fish meat  жарка
	data[1] = 3;     	//mode
	data[2] = 180;		//temp
	data[3] = 0;		//shour
	data[4] = 15;		//smin
	break;

	case 5:		//stewing_vegetables fish meat тушение
	data[1] = 3;     	//mode
	data[2] = 100;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 6:		//pasta  паста
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 8;		//smin
	break;

	case 7:		//milk_porridge молочная каша
	data[1] = 0;     	//mode
	data[2] = 95;		//temp
	data[3] = 0;		//shour
	data[4] = 35;		//smin
	break;

	case 8:		//soup суп
	data[1] = 0;     	//mode
	data[2] = 99;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 9:		//yogurt йогурт
	data[1] = 0;     	//mode
	data[2] = 40;		//temp
	data[3] = 8;		//shour
	data[4] = 0;		//smin
	break;

	case 10:		//baking выпечка
	data[1] = 0;     	//mode
	data[2] = 145;		//temp
	data[3] = 0;		//shour
	data[4] = 45;		//smin
	break;

	case 11:		//steam_vegetables fish meat пар
	data[1] = 3;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 40;		//smin
	break;

	case 12:		//hot варка бобовые
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 40;		//smin
	break;
	}
	} else if ( DEV_TYPA == 17 ) {
// for RMC-903s
	switch (prog) {
	case 0:		//multicooker мультиповар
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 30;		//smin
	break;

	case 1:		//milk_porridge молочная каша
	data[1] = 0;     	//mode
	data[2] = 97;		//temp
	data[3] = 0;		//shour
	data[4] = 10;		//smin
	break;

	case 2:		//stewing тушение
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 3:		//frying  жарка
	data[1] = 0;     	//mode
	data[2] = 170;		//temp
	data[3] = 0;		//shour
	data[4] = 15;		//smin
	break;

	case 4:		//soup суп
	data[1] = 0;     	//mode
	data[2] = 99;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 5:		//steam пар
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 20;		//smin
	break;

	case 6:		//pasta  макароны
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 8;		//smin
	break;

	case 7:		//slow_cooking  томление
	data[1] = 0;     	//mode
	data[2] = 97;		//temp
	data[3] = 5;		//shour
	data[4] = 0;		//smin
	break;

	case 8:		//hot варка
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 40;		//smin
	break;

	case 9:		//baking выпечка
	data[1] = 0;     	//mode
	data[2] = 145;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 10:	//rice рис/крупы
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 35;		//smin
	break;

	case 11:		//pilaf плов
	data[1] = 0;     	//mode
	data[2] = 110;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 12:		//yogurt йогурт
	data[1] = 0;     	//mode
	data[2] = 38;		//temp
	data[3] = 8;		//shour
	data[4] = 0;		//smin
	break;

	case 13:		//pizza пицца
	data[1] = 0;     	//mode
	data[2] = 150;		//temp
	data[3] = 0;		//shour
	data[4] = 25;		//smin
	break;

	case 14:		//bread хлеб
	data[1] = 0;     	//mode
	data[2] = 150;		//temp
	data[3] = 3;		//shour
	data[4] = 0;		//smin
	break;

	case 15:		//desserts десерты
	data[1] = 0;     	//mode
	data[2] = 98;		//temp
	data[3] = 0;		//shour
	data[4] = 20;		//smin
	break;

	case 16:		//express экспресс
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 20;		//smin
	break;
	}
	} else if ( DEV_TYPA == 18 ) {
// for RMC-224s
	switch (prog) {
	case 0:		//frying  жарка
	data[1] = 0;     	//mode
	data[2] = 150;		//temp
	data[3] = 0;		//shour
	data[4] = 15;		//smin
	break;

	case 1: 	//rice рис/крупы
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 25;		//smin
	break;

	case 2:		//multicooker мультиповар
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 30;		//smin
	break;

	case 3:		//pilaf плов
	data[1] = 0;     	//mode
	data[2] = 110;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 4:		//steam пар
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 25;		//smin
	break;

	case 5:		//baking выпечка
	data[1] = 0;     	//mode
	data[2] = 140;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 6:		//stewing тушение
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 7:		//soup суп
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 8:		//milk_porridge молочная каша
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 30;		//smin
	break;

	case 9: 		//yogurt йогурт
	data[1] = 0;     	//mode
	data[2] = 40;		//temp
	data[3] = 8;		//shour
	data[4] = 0;		//smin
	break;

	case 10:		//express экспресс
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 20;		//smin
	break;
	}
	}
	if ((data[1]) && (mod) && (mod < 4)) data[1] = mod;
	if (temp) data[2] = temp;
	if (phour || pmin) {
	data[3] = phour;
	data[4] = pmin;
	}	

	if (r4sCommandA(0x05, data, sizeof(data)) != 5) return false;
	if (notifyDataA[3] != 1) return false;
	bDHourA = dhour;
	bDMinA = dmin;
	return true;
	} else {
	if (r4sCommandA(0x09, data, 1) != 5) return false;
	if (notifyDataA[3] != 1) return false;
	data[0] = mod;
	if (r4sCommandA(0x0a, data, 1) != 5) return false;
	if (notifyDataA[3] != 1) return false;
	data[0] = phour;
	data[1] = pmin;
	if (r4sCommandA(0x0c, data, 2) != 5) return false;
	if (notifyDataA[3] != 1) return false;
	data[0] = dhour;
	data[1] = dmin;
	if (r4sCommandA(0x14, data, 2) != 5) return false;
	if (notifyDataA[3] != 1) return false;
	uint16_t ttemp = temp * 10;
	data[0] = ttemp & 0xff;
	data[1] = ((ttemp)>>8 & 0xff) | 0xf0;
	if (r4sCommandA(0x0b, data, 2) != 5) return false;
	if (notifyDataA[3] != 1) return false;
	return true;
}
}

bool rmB800sPall(uint8_t prog, uint8_t mod, uint8_t temp, uint8_t phour, uint8_t pmin, uint8_t dhour, uint8_t dmin, uint8_t warm) {
	if (( DEV_TYPB == 16 ) && (prog > 12)) return false;
	if (( DEV_TYPB == 17 ) && (prog > 16)) return false;
	uint8_t data[] = { prog, 0, 0, 0, 0, dhour, dmin, warm};
        if ( DEV_TYPB < 24 ) {
        if ( DEV_TYPB == 16 ) {
// for RMC-800s
	switch (prog) {
	case 0:		//multicooker мультиповар
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 30;		//smin
	break;

	case 1:		//rice рис/крупы
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 35;		//smin
	break;

	case 2:		//slow_cooking  томление
	data[1] = 0;     	//mode
	data[2] = 97;		//temp
	data[3] = 3;		//shour
	data[4] = 0;		//smin
	break;

	case 3:		//pilaf плов
	data[1] = 0;     	//mode
	data[2] = 110;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 4:		//frying_vegetables fish meat  жарка
	data[1] = 3;     	//mode
	data[2] = 180;		//temp
	data[3] = 0;		//shour
	data[4] = 15;		//smin
	break;

	case 5:		//stewing_vegetables fish meat тушение
	data[1] = 3;     	//mode
	data[2] = 100;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 6:		//pasta  паста
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 8;		//smin
	break;

	case 7:		//milk_porridge молочная каша
	data[1] = 0;     	//mode
	data[2] = 95;		//temp
	data[3] = 0;		//shour
	data[4] = 35;		//smin
	break;

	case 8:		//soup суп
	data[1] = 0;     	//mode
	data[2] = 99;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 9:		//yogurt йогурт
	data[1] = 0;     	//mode
	data[2] = 40;		//temp
	data[3] = 8;		//shour
	data[4] = 0;		//smin
	break;

	case 10:		//baking выпечка
	data[1] = 0;     	//mode
	data[2] = 145;		//temp
	data[3] = 0;		//shour
	data[4] = 45;		//smin
	break;

	case 11:		//steam_vegetables fish meat пар
	data[1] = 3;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 40;		//smin
	break;

	case 12:		//hot варка бобовые
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 40;		//smin
	break;
	}
	} else if ( DEV_TYPB == 17 ) {
// for RMC-903s
	switch (prog) {
	case 0:		//multicooker мультиповар
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 30;		//smin
	break;

	case 1:		//milk_porridge молочная каша
	data[1] = 0;     	//mode
	data[2] = 97;		//temp
	data[3] = 0;		//shour
	data[4] = 10;		//smin
	break;

	case 2:		//stewing тушение
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 3:		//frying  жарка
	data[1] = 0;     	//mode
	data[2] = 170;		//temp
	data[3] = 0;		//shour
	data[4] = 15;		//smin
	break;

	case 4:		//soup суп
	data[1] = 0;     	//mode
	data[2] = 99;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 5:		//steam пар
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 20;		//smin
	break;

	case 6:		//pasta  макароны
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 8;		//smin
	break;

	case 7:		//slow_cooking  томление
	data[1] = 0;     	//mode
	data[2] = 97;		//temp
	data[3] = 5;		//shour
	data[4] = 0;		//smin
	break;

	case 8:		//hot варка
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 40;		//smin
	break;

	case 9:		//baking выпечка
	data[1] = 0;     	//mode
	data[2] = 145;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 10:	//rice рис/крупы
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 35;		//smin
	break;

	case 11:		//pilaf плов
	data[1] = 0;     	//mode
	data[2] = 110;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 12:		//yogurt йогурт
	data[1] = 0;     	//mode
	data[2] = 38;		//temp
	data[3] = 8;		//shour
	data[4] = 0;		//smin
	break;

	case 13:		//pizza пицца
	data[1] = 0;     	//mode
	data[2] = 150;		//temp
	data[3] = 0;		//shour
	data[4] = 25;		//smin
	break;

	case 14:		//bread хлеб
	data[1] = 0;     	//mode
	data[2] = 150;		//temp
	data[3] = 3;		//shour
	data[4] = 0;		//smin
	break;

	case 15:		//desserts десерты
	data[1] = 0;     	//mode
	data[2] = 98;		//temp
	data[3] = 0;		//shour
	data[4] = 20;		//smin
	break;

	case 16:		//express экспресс
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 20;		//smin
	break;
	}
	} else if ( DEV_TYPB == 18 ) {
// for RMC-224s
	switch (prog) {
	case 0:		//frying  жарка
	data[1] = 0;     	//mode
	data[2] = 150;		//temp
	data[3] = 0;		//shour
	data[4] = 15;		//smin
	break;

	case 1: 	//rice рис/крупы
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 25;		//smin
	break;

	case 2:		//multicooker мультиповар
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 30;		//smin
	break;

	case 3:		//pilaf плов
	data[1] = 0;     	//mode
	data[2] = 110;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 4:		//steam пар
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 25;		//smin
	break;

	case 5:		//baking выпечка
	data[1] = 0;     	//mode
	data[2] = 140;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 6:		//stewing тушение
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 7:		//soup суп
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 8:		//milk_porridge молочная каша
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 30;		//smin
	break;

	case 9: 		//yogurt йогурт
	data[1] = 0;     	//mode
	data[2] = 40;		//temp
	data[3] = 8;		//shour
	data[4] = 0;		//smin
	break;

	case 10:		//express экспресс
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 20;		//smin
	break;
	}
	}
	if ((data[1]) && (mod) && (mod < 4)) data[1] = mod;
	if (temp) data[2] = temp;
	if (phour || pmin) {
	data[3] = phour;
	data[4] = pmin;
	}	

	if (r4sCommandB(0x05, data, sizeof(data)) != 5) return false;
	if (notifyDataB[3] != 1) return false;
	bDHourB = dhour;
	bDMinB = dmin;
	return true;
	} else {
	if (r4sCommandB(0x09, data, 1) != 5) return false;
	if (notifyDataB[3] != 1) return false;
	data[0] = mod;
	if (r4sCommandB(0x0a, data, 1) != 5) return false;
	if (notifyDataB[3] != 1) return false;
	data[0] = phour;
	data[1] = pmin;
	if (r4sCommandB(0x0c, data, 2) != 5) return false;
	if (notifyDataB[3] != 1) return false;
	data[0] = dhour;
	data[1] = dmin;
	if (r4sCommandB(0x14, data, 2) != 5) return false;
	if (notifyDataB[3] != 1) return false;
	uint16_t ttemp = temp * 10;
	data[0] = ttemp & 0xff;
	data[1] = ((ttemp)>>8 & 0xff) | 0xf0;
	if (r4sCommandB(0x0b, data, 2) != 5) return false;
	if (notifyDataB[3] != 1) return false;
	return true;
}
}

bool rmC800sPall(uint8_t prog, uint8_t mod, uint8_t temp, uint8_t phour, uint8_t pmin, uint8_t dhour, uint8_t dmin, uint8_t warm) {
	if (( DEV_TYPC == 16 ) && (prog > 12)) return false;
	if (( DEV_TYPC == 17 ) && (prog > 16)) return false;
	uint8_t data[] = { prog, 0, 0, 0, 0, dhour, dmin, warm};
        if ( DEV_TYPC < 24 ) {
        if ( DEV_TYPC == 16 ) {
// for RMC-800s
	switch (prog) {
	case 0:		//multicooker мультиповар
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 30;		//smin
	break;

	case 1:		//rice рис/крупы
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 35;		//smin
	break;

	case 2:		//slow_cooking  томление
	data[1] = 0;     	//mode
	data[2] = 97;		//temp
	data[3] = 3;		//shour
	data[4] = 0;		//smin
	break;

	case 3:		//pilaf плов
	data[1] = 0;     	//mode
	data[2] = 110;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 4:		//frying_vegetables fish meat  жарка
	data[1] = 3;     	//mode
	data[2] = 180;		//temp
	data[3] = 0;		//shour
	data[4] = 15;		//smin
	break;

	case 5:		//stewing_vegetables fish meat тушение
	data[1] = 3;     	//mode
	data[2] = 100;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 6:		//pasta  паста
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 8;		//smin
	break;

	case 7:		//milk_porridge молочная каша
	data[1] = 0;     	//mode
	data[2] = 95;		//temp
	data[3] = 0;		//shour
	data[4] = 35;		//smin
	break;

	case 8:		//soup суп
	data[1] = 0;     	//mode
	data[2] = 99;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 9:		//yogurt йогурт
	data[1] = 0;     	//mode
	data[2] = 40;		//temp
	data[3] = 8;		//shour
	data[4] = 0;		//smin
	break;

	case 10:		//baking выпечка
	data[1] = 0;     	//mode
	data[2] = 145;		//temp
	data[3] = 0;		//shour
	data[4] = 45;		//smin
	break;

	case 11:		//steam_vegetables fish meat пар
	data[1] = 3;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 40;		//smin
	break;

	case 12:		//hot варка бобовые
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 40;		//smin
	break;
	}
	} else if ( DEV_TYPC == 17 ) {
// for RMC-903s
	switch (prog) {
	case 0:		//multicooker мультиповар
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 30;		//smin
	break;

	case 1:		//milk_porridge молочная каша
	data[1] = 0;     	//mode
	data[2] = 97;		//temp
	data[3] = 0;		//shour
	data[4] = 10;		//smin
	break;

	case 2:		//stewing тушение
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 3:		//frying  жарка
	data[1] = 0;     	//mode
	data[2] = 170;		//temp
	data[3] = 0;		//shour
	data[4] = 15;		//smin
	break;

	case 4:		//soup суп
	data[1] = 0;     	//mode
	data[2] = 99;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 5:		//steam пар
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 20;		//smin
	break;

	case 6:		//pasta  макароны
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 8;		//smin
	break;

	case 7:		//slow_cooking  томление
	data[1] = 0;     	//mode
	data[2] = 97;		//temp
	data[3] = 5;		//shour
	data[4] = 0;		//smin
	break;

	case 8:		//hot варка
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 40;		//smin
	break;

	case 9:		//baking выпечка
	data[1] = 0;     	//mode
	data[2] = 145;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 10:	//rice рис/крупы
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 35;		//smin
	break;

	case 11:		//pilaf плов
	data[1] = 0;     	//mode
	data[2] = 110;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 12:		//yogurt йогурт
	data[1] = 0;     	//mode
	data[2] = 38;		//temp
	data[3] = 8;		//shour
	data[4] = 0;		//smin
	break;

	case 13:		//pizza пицца
	data[1] = 0;     	//mode
	data[2] = 150;		//temp
	data[3] = 0;		//shour
	data[4] = 25;		//smin
	break;

	case 14:		//bread хлеб
	data[1] = 0;     	//mode
	data[2] = 150;		//temp
	data[3] = 3;		//shour
	data[4] = 0;		//smin
	break;

	case 15:		//desserts десерты
	data[1] = 0;     	//mode
	data[2] = 98;		//temp
	data[3] = 0;		//shour
	data[4] = 20;		//smin
	break;

	case 16:		//express экспресс
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 20;		//smin
	break;
	}
	} else if ( DEV_TYPC == 18 ) {
// for RMC-224s
	switch (prog) {
	case 0:		//frying  жарка
	data[1] = 0;     	//mode
	data[2] = 150;		//temp
	data[3] = 0;		//shour
	data[4] = 15;		//smin
	break;

	case 1: 	//rice рис/крупы
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 25;		//smin
	break;

	case 2:		//multicooker мультиповар
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 30;		//smin
	break;

	case 3:		//pilaf плов
	data[1] = 0;     	//mode
	data[2] = 110;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 4:		//steam пар
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 25;		//smin
	break;

	case 5:		//baking выпечка
	data[1] = 0;     	//mode
	data[2] = 140;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 6:		//stewing тушение
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 7:		//soup суп
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 8:		//milk_porridge молочная каша
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 30;		//smin
	break;

	case 9: 		//yogurt йогурт
	data[1] = 0;     	//mode
	data[2] = 40;		//temp
	data[3] = 8;		//shour
	data[4] = 0;		//smin
	break;

	case 10:		//express экспресс
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 20;		//smin
	break;
	}
	}
	if ((data[1]) && (mod) && (mod < 4)) data[1] = mod;
	if (temp) data[2] = temp;
	if (phour || pmin) {
	data[3] = phour;
	data[4] = pmin;
	}	

	if (r4sCommandC(0x05, data, sizeof(data)) != 5) return false;
	if (notifyDataC[3] != 1) return false;
	bDHourC = dhour;
	bDMinC = dmin;
	return true;
	} else {
	if (r4sCommandC(0x09, data, 1) != 5) return false;
	if (notifyDataC[3] != 1) return false;
	data[0] = mod;
	if (r4sCommandC(0x0a, data, 1) != 5) return false;
	if (notifyDataC[3] != 1) return false;
	data[0] = phour;
	data[1] = pmin;
	if (r4sCommandC(0x0c, data, 2) != 5) return false;
	if (notifyDataC[3] != 1) return false;
	data[0] = dhour;
	data[1] = dmin;
	if (r4sCommandC(0x14, data, 2) != 5) return false;
	if (notifyDataC[3] != 1) return false;
	uint16_t ttemp = temp * 10;
	data[0] = ttemp & 0xff;
	data[1] = ((ttemp)>>8 & 0xff) | 0xf0;
	if (r4sCommandC(0x0b, data, 2) != 5) return false;
	if (notifyDataC[3] != 1) return false;
	return true;
}
}



bool rmA800sProg(uint8_t prog) {
	if (( DEV_TYPA == 16 ) && (prog > 12)) return false;
	if (( DEV_TYPA == 17 ) && (prog > 16)) return false;
	uint8_t data[] = { prog, 0, 0, 0, 0, bDHourA, bDMinA, bAwarmA};
        if ( DEV_TYPA < 24 ) {
        if ( DEV_TYPA == 16 ) {
// for RMC-800s
	switch (prog) {
	case 0:		//multicooker мультиповар
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 30;		//smin
	break;

	case 1:		//rice рис/крупы
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 35;		//smin
	break;

	case 2:		//slow_cooking  томление
	data[1] = 0;     	//mode
	data[2] = 97;		//temp
	data[3] = 3;		//shour
	data[4] = 0;		//smin
	break;

	case 3:		//pilaf плов
	data[1] = 0;     	//mode
	data[2] = 110;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 4:		//frying_vegetables fish meat  жарка
	data[1] = 3;     	//mode
	data[2] = 180;		//temp
	data[3] = 0;		//shour
	data[4] = 15;		//smin
	break;

	case 5:		//stewing_vegetables fish meat тушение
	data[1] = 3;     	//mode
	data[2] = 100;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 6:		//pasta  паста
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 8;		//smin
	break;

	case 7:		//milk_porridge молочная каша
	data[1] = 0;     	//mode
	data[2] = 95;		//temp
	data[3] = 0;		//shour
	data[4] = 35;		//smin
	break;

	case 8:		//soup суп
	data[1] = 0;     	//mode
	data[2] = 99;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 9:		//yogurt йогурт
	data[1] = 0;     	//mode
	data[2] = 40;		//temp
	data[3] = 8;		//shour
	data[4] = 0;		//smin
	break;

	case 10:		//baking выпечка
	data[1] = 0;     	//mode
	data[2] = 145;		//temp
	data[3] = 0;		//shour
	data[4] = 45;		//smin
	break;

	case 11:		//steam_vegetables fish meat пар
	data[1] = 3;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 40;		//smin
	break;

	case 12:		//hot варка бобовые
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 40;		//smin
	break;
	}
	} else if ( DEV_TYPA == 17 ) {
// for RMC-903s
	switch (prog) {
	case 0:		//multicooker мультиповар
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 30;		//smin
	break;

	case 1:		//milk_porridge молочная каша
	data[1] = 0;     	//mode
	data[2] = 97;		//temp
	data[3] = 0;		//shour
	data[4] = 10;		//smin
	break;

	case 2:		//stewing тушение
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 3:		//frying  жарка
	data[1] = 0;     	//mode
	data[2] = 170;		//temp
	data[3] = 0;		//shour
	data[4] = 15;		//smin
	break;

	case 4:		//soup суп
	data[1] = 0;     	//mode
	data[2] = 99;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 5:		//steam пар
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 20;		//smin
	break;

	case 6:		//pasta  макароны
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 8;		//smin
	break;

	case 7:		//slow_cooking  томление
	data[1] = 0;     	//mode
	data[2] = 97;		//temp
	data[3] = 5;		//shour
	data[4] = 0;		//smin
	break;

	case 8:		//hot варка
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 40;		//smin
	break;

	case 9:		//baking выпечка
	data[1] = 0;     	//mode
	data[2] = 145;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 10:	//rice рис/крупы
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 35;		//smin
	break;

	case 11:		//pilaf плов
	data[1] = 0;     	//mode
	data[2] = 110;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 12:		//yogurt йогурт
	data[1] = 0;     	//mode
	data[2] = 38;		//temp
	data[3] = 8;		//shour
	data[4] = 0;		//smin
	break;

	case 13:		//pizza пицца
	data[1] = 0;     	//mode
	data[2] = 150;		//temp
	data[3] = 0;		//shour
	data[4] = 25;		//smin
	break;

	case 14:		//bread хлеб
	data[1] = 0;     	//mode
	data[2] = 150;		//temp
	data[3] = 3;		//shour
	data[4] = 0;		//smin
	break;

	case 15:		//desserts десерты
	data[1] = 0;     	//mode
	data[2] = 98;		//temp
	data[3] = 0;		//shour
	data[4] = 20;		//smin
	break;

	case 16:		//express экспресс
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 20;		//smin
	break;
	}
	} else if ( DEV_TYPA == 18 ) {
// for RMC-224s
	switch (prog) {
	case 0:		//frying  жарка
	data[1] = 0;     	//mode
	data[2] = 150;		//temp
	data[3] = 0;		//shour
	data[4] = 15;		//smin
	break;

	case 1: 	//rice рис/крупы
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 25;		//smin
	break;

	case 2:		//multicooker мультиповар
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 30;		//smin
	break;

	case 3:		//pilaf плов
	data[1] = 0;     	//mode
	data[2] = 110;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 4:		//steam пар
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 25;		//smin
	break;

	case 5:		//baking выпечка
	data[1] = 0;     	//mode
	data[2] = 140;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 6:		//stewing тушение
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 7:		//soup суп
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 8:		//milk_porridge молочная каша
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 30;		//smin
	break;

	case 9: 		//yogurt йогурт
	data[1] = 0;     	//mode
	data[2] = 40;		//temp
	data[3] = 8;		//shour
	data[4] = 0;		//smin
	break;

	case 10:		//express экспресс
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 20;		//smin
	break;
	}
	}
	if (r4sCommandA(0x05, data, sizeof(data)) != 5) return false;
	if (notifyDataA[3] != 1) return false;
	return true;
	} else {
	if (r4sCommandA(0x09, data, 1) != 5) return false;
	if (notifyDataA[3] != 1) return false;
	return true;
}
}

bool rmB800sProg(uint8_t prog) {
	if (( DEV_TYPB == 16 ) && (prog > 12)) return false;
	if (( DEV_TYPB == 17 ) && (prog > 16)) return false;
	uint8_t data[] = { prog, 0, 0, 0, 0, bDHourB, bDMinB, bAwarmB};
        if ( DEV_TYPB < 24 ) {
        if ( DEV_TYPB == 16 ) {
// for RMC-800s
	switch (prog) {
	case 0:		//multicooker мультиповар
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 30;		//smin
	break;

	case 1:		//rice рис/крупы
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 35;		//smin
	break;

	case 2:		//slow_cooking  томление
	data[1] = 0;     	//mode
	data[2] = 97;		//temp
	data[3] = 3;		//shour
	data[4] = 0;		//smin
	break;

	case 3:		//pilaf плов
	data[1] = 0;     	//mode
	data[2] = 110;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 4:		//frying_vegetables fish meat  жарка
	data[1] = 3;     	//mode
	data[2] = 180;		//temp
	data[3] = 0;		//shour
	data[4] = 15;		//smin
	break;

	case 5:		//stewing_vegetables fish meat тушение
	data[1] = 3;     	//mode
	data[2] = 100;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 6:		//pasta  паста
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 8;		//smin
	break;

	case 7:		//milk_porridge молочная каша
	data[1] = 0;     	//mode
	data[2] = 95;		//temp
	data[3] = 0;		//shour
	data[4] = 35;		//smin
	break;

	case 8:		//soup суп
	data[1] = 0;     	//mode
	data[2] = 99;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 9:		//yogurt йогурт
	data[1] = 0;     	//mode
	data[2] = 40;		//temp
	data[3] = 8;		//shour
	data[4] = 0;		//smin
	break;

	case 10:		//baking выпечка
	data[1] = 0;     	//mode
	data[2] = 145;		//temp
	data[3] = 0;		//shour
	data[4] = 45;		//smin
	break;

	case 11:		//steam_vegetables fish meat пар
	data[1] = 3;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 40;		//smin
	break;

	case 12:		//hot варка бобовые
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 40;		//smin
	break;
	}
	} else if ( DEV_TYPB == 17 ) {
// for RMC-903s
	switch (prog) {
	case 0:		//multicooker мультиповар
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 30;		//smin
	break;

	case 1:		//milk_porridge молочная каша
	data[1] = 0;     	//mode
	data[2] = 97;		//temp
	data[3] = 0;		//shour
	data[4] = 10;		//smin
	break;

	case 2:		//stewing тушение
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 3:		//frying  жарка
	data[1] = 0;     	//mode
	data[2] = 170;		//temp
	data[3] = 0;		//shour
	data[4] = 15;		//smin
	break;

	case 4:		//soup суп
	data[1] = 0;     	//mode
	data[2] = 99;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 5:		//steam пар
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 20;		//smin
	break;

	case 6:		//pasta  макароны
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 8;		//smin
	break;

	case 7:		//slow_cooking  томление
	data[1] = 0;     	//mode
	data[2] = 97;		//temp
	data[3] = 5;		//shour
	data[4] = 0;		//smin
	break;

	case 8:		//hot варка
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 40;		//smin
	break;

	case 9:		//baking выпечка
	data[1] = 0;     	//mode
	data[2] = 145;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 10:	//rice рис/крупы
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 35;		//smin
	break;

	case 11:		//pilaf плов
	data[1] = 0;     	//mode
	data[2] = 110;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 12:		//yogurt йогурт
	data[1] = 0;     	//mode
	data[2] = 38;		//temp
	data[3] = 8;		//shour
	data[4] = 0;		//smin
	break;

	case 13:		//pizza пицца
	data[1] = 0;     	//mode
	data[2] = 150;		//temp
	data[3] = 0;		//shour
	data[4] = 25;		//smin
	break;

	case 14:		//bread хлеб
	data[1] = 0;     	//mode
	data[2] = 150;		//temp
	data[3] = 3;		//shour
	data[4] = 0;		//smin
	break;

	case 15:		//desserts десерты
	data[1] = 0;     	//mode
	data[2] = 98;		//temp
	data[3] = 0;		//shour
	data[4] = 20;		//smin
	break;

	case 16:		//express экспресс
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 20;		//smin
	break;
	}
	} else if ( DEV_TYPB == 18 ) {
// for RMC-224s
	switch (prog) {
	case 0:		//frying  жарка
	data[1] = 0;     	//mode
	data[2] = 150;		//temp
	data[3] = 0;		//shour
	data[4] = 15;		//smin
	break;

	case 1: 	//rice рис/крупы
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 25;		//smin
	break;

	case 2:		//multicooker мультиповар
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 30;		//smin
	break;

	case 3:		//pilaf плов
	data[1] = 0;     	//mode
	data[2] = 110;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 4:		//steam пар
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 25;		//smin
	break;

	case 5:		//baking выпечка
	data[1] = 0;     	//mode
	data[2] = 140;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 6:		//stewing тушение
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 7:		//soup суп
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 8:		//milk_porridge молочная каша
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 30;		//smin
	break;

	case 9: 		//yogurt йогурт
	data[1] = 0;     	//mode
	data[2] = 40;		//temp
	data[3] = 8;		//shour
	data[4] = 0;		//smin
	break;

	case 10:		//express экспресс
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 20;		//smin
	break;
	}
	}
	if (r4sCommandB(0x05, data, sizeof(data)) != 5) return false;
	if (notifyDataB[3] != 1) return false;
	return true;
	} else {
	if (r4sCommandB(0x09, data, 1) != 5) return false;
	if (notifyDataB[3] != 1) return false;
	return true;
}
}

bool rmC800sProg(uint8_t prog) {
	if (( DEV_TYPC == 16 ) && (prog > 12)) return false;
	if (( DEV_TYPC == 17 ) && (prog > 16)) return false;
	uint8_t data[] = { prog, 0, 0, 0, 0, bDHourC, bDMinC, bAwarmC};
        if ( DEV_TYPC < 24 ) {
        if ( DEV_TYPC == 16 ) {
// for RMC-800s
	switch (prog) {
	case 0:		//multicooker мультиповар
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 30;		//smin
	break;

	case 1:		//rice рис/крупы
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 35;		//smin
	break;

	case 2:		//slow_cooking  томление
	data[1] = 0;     	//mode
	data[2] = 97;		//temp
	data[3] = 3;		//shour
	data[4] = 0;		//smin
	break;

	case 3:		//pilaf плов
	data[1] = 0;     	//mode
	data[2] = 110;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 4:		//frying_vegetables fish meat  жарка
	data[1] = 3;     	//mode
	data[2] = 180;		//temp
	data[3] = 0;		//shour
	data[4] = 15;		//smin
	break;

	case 5:		//stewing_vegetables fish meat тушение
	data[1] = 3;     	//mode
	data[2] = 100;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 6:		//pasta  паста
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 8;		//smin
	break;

	case 7:		//milk_porridge молочная каша
	data[1] = 0;     	//mode
	data[2] = 95;		//temp
	data[3] = 0;		//shour
	data[4] = 35;		//smin
	break;

	case 8:		//soup суп
	data[1] = 0;     	//mode
	data[2] = 99;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 9:		//yogurt йогурт
	data[1] = 0;     	//mode
	data[2] = 40;		//temp
	data[3] = 8;		//shour
	data[4] = 0;		//smin
	break;

	case 10:		//baking выпечка
	data[1] = 0;     	//mode
	data[2] = 145;		//temp
	data[3] = 0;		//shour
	data[4] = 45;		//smin
	break;

	case 11:		//steam_vegetables fish meat пар
	data[1] = 3;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 40;		//smin
	break;

	case 12:		//hot варка бобовые
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 40;		//smin
	break;
	}
	} else if ( DEV_TYPC == 17 ) {
// for RMC-903s
	switch (prog) {
	case 0:		//multicooker мультиповар
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 30;		//smin
	break;

	case 1:		//milk_porridge молочная каша
	data[1] = 0;     	//mode
	data[2] = 97;		//temp
	data[3] = 0;		//shour
	data[4] = 10;		//smin
	break;

	case 2:		//stewing тушение
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 3:		//frying  жарка
	data[1] = 0;     	//mode
	data[2] = 170;		//temp
	data[3] = 0;		//shour
	data[4] = 15;		//smin
	break;

	case 4:		//soup суп
	data[1] = 0;     	//mode
	data[2] = 99;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 5:		//steam пар
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 20;		//smin
	break;

	case 6:		//pasta  макароны
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 8;		//smin
	break;

	case 7:		//slow_cooking  томление
	data[1] = 0;     	//mode
	data[2] = 97;		//temp
	data[3] = 5;		//shour
	data[4] = 0;		//smin
	break;

	case 8:		//hot варка
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 40;		//smin
	break;

	case 9:		//baking выпечка
	data[1] = 0;     	//mode
	data[2] = 145;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 10:	//rice рис/крупы
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 35;		//smin
	break;

	case 11:		//pilaf плов
	data[1] = 0;     	//mode
	data[2] = 110;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 12:		//yogurt йогурт
	data[1] = 0;     	//mode
	data[2] = 38;		//temp
	data[3] = 8;		//shour
	data[4] = 0;		//smin
	break;

	case 13:		//pizza пицца
	data[1] = 0;     	//mode
	data[2] = 150;		//temp
	data[3] = 0;		//shour
	data[4] = 25;		//smin
	break;

	case 14:		//bread хлеб
	data[1] = 0;     	//mode
	data[2] = 150;		//temp
	data[3] = 3;		//shour
	data[4] = 0;		//smin
	break;

	case 15:		//desserts десерты
	data[1] = 0;     	//mode
	data[2] = 98;		//temp
	data[3] = 0;		//shour
	data[4] = 20;		//smin
	break;

	case 16:		//express экспресс
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 20;		//smin
	break;
	}
	} else if ( DEV_TYPC == 18 ) {
// for RMC-224s
	switch (prog) {
	case 0:		//frying  жарка
	data[1] = 0;     	//mode
	data[2] = 150;		//temp
	data[3] = 0;		//shour
	data[4] = 15;		//smin
	break;

	case 1: 	//rice рис/крупы
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 25;		//smin
	break;

	case 2:		//multicooker мультиповар
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 30;		//smin
	break;

	case 3:		//pilaf плов
	data[1] = 0;     	//mode
	data[2] = 110;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 4:		//steam пар
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 25;		//smin
	break;

	case 5:		//baking выпечка
	data[1] = 0;     	//mode
	data[2] = 140;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 6:		//stewing тушение
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 7:		//soup суп
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 8:		//milk_porridge молочная каша
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 30;		//smin
	break;

	case 9: 		//yogurt йогурт
	data[1] = 0;     	//mode
	data[2] = 40;		//temp
	data[3] = 8;		//shour
	data[4] = 0;		//smin
	break;

	case 10:		//express экспресс
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 20;		//smin
	break;
	}
	}
	if (r4sCommandC(0x05, data, sizeof(data)) != 5) return false;
	if (notifyDataC[3] != 1) return false;
	return true;
	} else {
	if (r4sCommandC(0x09, data, 1) != 5) return false;
	if (notifyDataC[3] != 1) return false;
	return true;
}
}

bool rmA800sMod(uint8_t mod) {
	if (mod > 3) return false;
	uint8_t data[] = { bProgA, mod, bHtempA, bPHourA, bPMinA, bDHourA, bDMinA, bAwarmA};
        if ( DEV_TYPA < 24 ) {
	switch (mod) {

	case 1:		//vegetables
	if (bProgA == 4) {
	data[2] = 180;		//temp
	data[3] = 0;		//shour
	data[4] = 18;		//smin
	} else if (bProgA == 5) {
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 40;		//smin
	} else if (bProgA == 11) {
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 30;		//smin
	}
	break;

	case 2:		//fish
	if (bProgA == 4) {
	data[2] = 180;		//temp
	data[3] = 0;		//shour
	data[4] = 12;		//smin
	} else if (bProgA == 5) {
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 35;		//smin
	} else if (bProgA == 11) {
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 25;		//smin
	}
	break;

	case 3:		//meat
	if (bProgA == 4) {
	data[2] = 180;		//temp
	data[3] = 0;		//shour
	data[4] = 15;		//smin
	} else if (bProgA == 5) {
	data[2] = 100;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	} else if (bProgA == 11) {
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 40;		//smin
	}
	break;
	}

	if (r4sCommandA(0x05, data, sizeof(data)) != 5) return false;
	if (notifyDataA[3] != 1) return false;
	return true;
	} else {
	data[0] = mod;
	if (r4sCommandA(0x0a, data, 1) != 5) return false;
	if (notifyDataA[3] != 1) return false;
	return true;
}
}

bool rmB800sMod(uint8_t mod) {
	if (mod > 3) return false;
	uint8_t data[] = { bProgB, mod, bHtempB, bPHourB, bPMinB, bDHourB, bDMinB, bAwarmB};
        if ( DEV_TYPB < 24 ) {
	switch (mod) {

	case 1:		//vegetables
	if (bProgB == 4) {
	data[2] = 180;		//temp
	data[3] = 0;		//shour
	data[4] = 18;		//smin
	} else if (bProgB == 5) {
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 40;		//smin
	} else if (bProgB == 11) {
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 30;		//smin
	}
	break;

	case 2:		//fish
	if (bProgB == 4) {
	data[2] = 180;		//temp
	data[3] = 0;		//shour
	data[4] = 12;		//smin
	} else if (bProgB == 5) {
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 35;		//smin
	} else if (bProgB == 11) {
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 25;		//smin
	}
	break;

	case 3:		//meat
	if (bProgB == 4) {
	data[2] = 180;		//temp
	data[3] = 0;		//shour
	data[4] = 15;		//smin
	} else if (bProgB == 5) {
	data[2] = 100;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	} else if (bProgB == 11) {
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 40;		//smin
	}
	break;
	}

	if (r4sCommandB(0x05, data, sizeof(data)) != 5) return false;
	if (notifyDataB[3] != 1) return false;
	return true;
	} else {
	data[0] = mod;
	if (r4sCommandB(0x0a, data, 1) != 5) return false;
	if (notifyDataB[3] != 1) return false;
	return true;
}
}

bool rmC800sMod(uint8_t mod) {
	if (mod > 3) return false;
	uint8_t data[] = { bProgC, mod, bHtempC, bPHourC, bPMinC, bDHourC, bDMinC, bAwarmC};
        if ( DEV_TYPC < 24 ) {
	switch (mod) {

	case 1:		//vegetables
	if (bProgC == 4) {
	data[2] = 180;		//temp
	data[3] = 0;		//shour
	data[4] = 18;		//smin
	} else if (bProgC == 5) {
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 40;		//smin
	} else if (bProgC == 11) {
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 30;		//smin
	}
	break;

	case 2:		//fish
	if (bProgC == 4) {
	data[2] = 180;		//temp
	data[3] = 0;		//shour
	data[4] = 12;		//smin
	} else if (bProgC == 5) {
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 35;		//smin
	} else if (bProgC == 11) {
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 25;		//smin
	}
	break;

	case 3:		//meat
	if (bProgC == 4) {
	data[2] = 180;		//temp
	data[3] = 0;		//shour
	data[4] = 15;		//smin
	} else if (bProgC == 5) {
	data[2] = 100;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	} else if (bProgC == 11) {
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 40;		//smin
	}
	break;
	}

	if (r4sCommandC(0x05, data, sizeof(data)) != 5) return false;
	if (notifyDataC[3] != 1) return false;
	return true;
	} else {
	data[0] = mod;
	if (r4sCommandC(0x0a, data, 1) != 5) return false;
	if (notifyDataC[3] != 1) return false;
	return true;
}
}


bool rmA800sTemp(uint8_t temp) {
	uint8_t data[] = { temp, 0 };
        if ( DEV_TYPA < 24 ) {
	if (r4sCommandA(0x0b, data, 1) != 5) return false;
	if (notifyDataA[3] != 1) return false;
	return true;
	} else {
	uint16_t ttemp = temp * 10;
	data[0] = ttemp & 0xff;
	data[1] = ((ttemp)>>8 & 0xff) | 0xf0;
	if (r4sCommandA(0x0b, data, 2) != 5) return false;
	if (notifyDataA[3] != 1) return false;
	return true;
	}
}

bool rmB800sTemp(uint8_t temp) {
	uint8_t data[] = { temp, 0 };
        if ( DEV_TYPB < 24 ) {
	if (r4sCommandB(0x0b, data, 1) != 5)	return false;
	if (notifyDataB[3] != 1) return false;
	return true;
	} else {
	uint16_t ttemp = temp * 10;
	data[0] = ttemp & 0xff;
	data[1] = ((ttemp)>>8 & 0xff) | 0xf0;
	if (r4sCommandB(0x0b, data, 2) != 5)	return false;
	if (notifyDataB[3] != 1) return false;
	return true;
	}
}

bool rmC800sTemp(uint8_t temp) {
	uint8_t data[] = { temp, 0 };
        if ( DEV_TYPC < 24 ) {
	if (r4sCommandC(0x0b, data, 1) != 5) return false;
	if (notifyDataC[3] != 1) return false;
	return true;
	} else {
	uint16_t ttemp = temp * 10;
	data[0] = ttemp & 0xff;
	data[1] = ((ttemp)>>8 & 0xff) | 0xf0;
	if (r4sCommandC(0x0b, data, 2) != 5) return false;
	if (notifyDataC[3] != 1) return false;
	return true;
	}
}


bool rmA800sPhour(uint8_t hour) {
	uint8_t data[] = { hour, bPMinA};
	if (r4sCommandA(0x0c, data, sizeof(data)) != 5)
	return false;
	if (notifyDataA[3] != 1)
	return false;
	return true;
}

bool rmA800sPmin(uint8_t min) {
	uint8_t data[] = { bPHourA, min};
	if (r4sCommandA(0x0c, data, sizeof(data)) != 5)
	return false;
	if (notifyDataA[3] != 1)
	return false;
	return true;
}

bool rmB800sPhour(uint8_t hour) {
	uint8_t data[] = { hour, bPMinB};
	if (r4sCommandB(0x0c, data, sizeof(data)) != 5)
	return false;
	if (notifyDataB[3] != 1)
	return false;
	return true;
}

bool rmB800sPmin(uint8_t min) {
	uint8_t data[] = { bPHourB, min};
	if (r4sCommandB(0x0c, data, sizeof(data)) != 5)
	return false;
	if (notifyDataB[3] != 1)
	return false;
	return true;
}

bool rmC800sPhour(uint8_t hour) {
	uint8_t data[] = { hour, bPMinC};
	if (r4sCommandC(0x0c, data, sizeof(data)) != 5)
	return false;
	if (notifyDataC[3] != 1)
	return false;
	return true;
}

bool rmC800sPmin(uint8_t min) {
	uint8_t data[] = { bPHourC, min};
	if (r4sCommandC(0x0c, data, sizeof(data)) != 5)
	return false;
	if (notifyDataC[3] != 1)
	return false;
	return true;
}

bool rmA800sDhour(uint8_t hour) {
	uint8_t data[] = { hour, bDMinA};
	if (r4sCommandA(0x14, data, sizeof(data)) != 5)
	return false;
	if (notifyDataA[3] != 1)
	return false;
	return true;
}

bool rmA800sDmin(uint8_t min) {
	uint8_t data[] = { bDHourA, min};
	if (r4sCommandA(0x14, data, sizeof(data)) != 5)
	return false;
	if (notifyDataA[3] != 1)
	return false;
	return true;
}

bool rmB800sDhour(uint8_t hour) {
	uint8_t data[] = { hour, bDMinB};
	if (r4sCommandB(0x14, data, sizeof(data)) != 5)
	return false;
	if (notifyDataB[3] != 1)
	return false;
	return true;
}

bool rmB800sDmin(uint8_t min) {
	uint8_t data[] = { bDHourB, min};
	if (r4sCommandB(0x14, data, sizeof(data)) != 5)
	return false;
	if (notifyDataB[3] != 1)
	return false;
	return true;
}

bool rmC800sDhour(uint8_t hour) {
	uint8_t data[] = { hour, bDMinC};
	if (r4sCommandC(0x14, data, sizeof(data)) != 5)
	return false;
	if (notifyDataC[3] != 1)
	return false;
	return true;
}

bool rmC800sDmin(uint8_t min) {
	uint8_t data[] = { bDHourC, min};
	if (r4sCommandC(0x14, data, sizeof(data)) != 5)
	return false;
	if (notifyDataC[3] != 1)
	return false;
	return true;
}


bool rmA800sAwarm(uint8_t warm) {
	uint8_t data[] = { bProgA, bModProgA, bHtempA, bPHourA, bPMinA, bDHourA, bDMinA, warm};

	if (r4sCommandA(0x05, data, sizeof(data)) != 5)
	return false;
	if (notifyDataA[3] != 1)
	return false;
	return true;
}

bool rmB800sAwarm(uint8_t warm) {
	uint8_t data[] = { bProgB, bModProgB, bHtempB, bPHourB, bPMinB, bDHourB, bDMinB, warm};

	if (r4sCommandB(0x05, data, sizeof(data)) != 5)
	return false;
	if (notifyDataB[3] != 1)
	return false;
	return true;
}

bool rmC800sAwarm(uint8_t warm) {
	uint8_t data[] = { bProgC, bModProgC, bHtempC, bPHourC, bPMinC, bDHourC, bDMinC, warm};

	if (r4sCommandC(0x05, data, sizeof(data)) != 5)
	return false;
	if (notifyDataC[3] != 1)
	return false;
	return true;
}


bool mkSyncA() {
	if (btauthoriza && (DEV_TYPA > 3) && (DEV_TYPA < 64) && ((DEV_TYPA < 16) || (DEV_TYPA > 23))) {
	uint8_t data[] = { 0, 0, 0, 0, 0, 0, 0, 0};
	time_t now;
	time(&now);
	uint8_t ttz;
	int32_t tz;
	if (TimeZone < 128) tz = TimeZone * 3600;
	else {
	ttz = ~TimeZone + 1;
	tz  = ttz * 3600 - 1;
	tz = ~tz;
	}
	memcpy(&data[0], &now, 4);
	memcpy(&data[4], &tz, 4);
//ESP_LOGI(AP_TAG, "Time A: %ld,Timezone A: %d", now, tz);
//esp_log_buffer_hex(AP_TAG, data, sizeof(data));
	if (r4sCommandA(0x6e, data, sizeof(data)) != 5)
	return false;
	if (notifyDataA[3] != 0)
	return false;
	}
	f_SyncA = 0;
	return true;
}

bool mkSyncB() {
	if (btauthorizb && (DEV_TYPB > 3) && (DEV_TYPB < 64) && ((DEV_TYPB < 16) || (DEV_TYPB > 23))) {
	uint8_t data[] = { 0, 0, 0, 0, 0, 0, 0, 0};
	time_t now;
	time(&now);
	uint8_t ttz;
	int32_t tz;
	if (TimeZone < 128) tz = TimeZone * 3600;
	else {
	ttz = ~TimeZone + 1;
	tz  = ttz * 3600 - 1;
	tz = ~tz;
	}
	memcpy(&data[0], &now, 4);
	memcpy(&data[4], &tz, 4);
//ESP_LOGI(AP_TAG, "Time B: %ld,Timezone B: %d", now, tz);
//esp_log_buffer_hex(AP_TAG, data, sizeof(data));
	if (r4sCommandB(0x6e, data, sizeof(data)) != 5)
	return false;
	if (notifyDataB[3] != 0)
	return false;
	}
	f_SyncB = 0;
	return true;
}

bool mkSyncC() {
	if (btauthorizc && (DEV_TYPC > 3) && (DEV_TYPC < 64) && ((DEV_TYPC < 16) || (DEV_TYPC > 23))) {
	uint8_t data[] = { 0, 0, 0, 0, 0, 0, 0, 0};
	time_t now;
	time(&now);
	uint8_t ttz;
	int32_t tz;
	if (TimeZone < 128) tz = TimeZone * 3600;
	else {
	ttz = ~TimeZone + 1;
	tz  = ttz * 3600 - 1;
	tz = ~tz;
	}
	memcpy(&data[0], &now, 4);
	memcpy(&data[4], &tz, 4);
//ESP_LOGI(AP_TAG, "Time C: %ld,Timezone C: %d", now, tz);
//esp_log_buffer_hex(AP_TAG, data, sizeof(data));
	if (r4sCommandC(0x6e, data, sizeof(data)) != 5)
	return false;
	if (notifyDataC[3] != 0)
	return false;
	}
	f_SyncC = 0;
	return true;
}

void mAsStatus() {
	char tmpvar[8]; 
	int  tmpint;
	int  retc;
	if (btauthoriza && (DEV_TYPA < 64)) {
        retc = r4sCommandA(0x06, 0, 0);	
	if ((DEV_TYPA == 1) && (retc == 13)) {
	bCtempA = notifyDataA[5];
	bHeatA = notifyDataA[3];
	bStateA = notifyDataA[11];
	if (bStateA == 4) bStateA = 0;
	bProgA = notifyDataA[3];
	if (bProgA) bStateA = 0;
	bStNlA = 0;
	bStBlA = 0;
	bStBpA = 0;
	bHtempA = 0;
	switch (notifyDataA[4]) {
	case 1:
        bHtempA = 40;
	break;
	case 2:
        bHtempA = 55;
	break;
	case 3:
        bHtempA = 70;
	break;
	case 4:
        bHtempA = 85;
	break;
	case 5:
        bHtempA = 95;
	break;
	}
	strcpy(cStatusA,"{\"temp\":");
	itoa(notifyDataA[5],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"target\":");
	itoa(bHtempA,tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"state\":");
	itoa(notifyDataA[11],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"prog\":");
	itoa(notifyDataA[3],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,"]}");    
	} else if ((DEV_TYPA > 1) && ( DEV_TYPA < 10 ) && (retc == 20)) {
	bHtempA = notifyDataA[5];
	bStateA = notifyDataA[11];
	bProgA = notifyDataA[3];
	if ((bStateA == 4) || (bProgA && (bProgA != 2))) bStateA = 0;
	if (DEV_TYPA > 3) {
	bCtempA = notifyDataA[8];
	bHeatA = notifyDataA[11];
	if ((bHeatA == 4) || ((bProgA != 1) && (bProgA != 2))) bHeatA = 0;
	bStNlA = notifyDataA[11];
	if ((bStNlA == 4) || (bProgA != 3)) bStNlA = 0;
	bStBpA = notifyDataA[7];
	} else {
	bCtempA = notifyDataA[13];
//	bHeatA = notifyDataA[10];
	bHeatA = notifyDataA[3];
	bStNlA = 0;
	bStBpA = 0;
	}
	strcpy(cStatusA,"{\"temp\":");
	(DEV_TYPA > 3)? itoa(notifyDataA[8],tmpvar,10) : itoa(notifyDataA[13],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"target\":");
	itoa(notifyDataA[5],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"state\":");
	itoa(notifyDataA[11],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"prog\":");
	itoa(notifyDataA[3],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"beep\":");
	itoa(bStBpA,tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"error\":");
	itoa(notifyDataA[12],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"rgb\":[");
	itoa(RgbRA,tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",");    
	itoa(RgbGA,tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",");    
	itoa(RgbBA,tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,"]}");    
	if (DEV_TYPA > 3) {
	uint8_t data[] = {0xc8};
	if (r4sCommandA(0x35, data, sizeof(data)) == 9) {
	bStBlA = notifyDataA[5];
        if (!r4slpcoma && !r4slpcomb && !r4slpcomc && !r4slpresa) {
	data[0] = 0x00;
	if (r4sCommandA(0x47, data, sizeof(data)) == 20) {
	bSEnergyA = (notifyDataA[12] << 24) + (notifyDataA[11] << 16) + (notifyDataA[10] << 8) + notifyDataA[9];
	bSTimeA = (notifyDataA[8] << 24) + (notifyDataA[7] << 16) + (notifyDataA[6] << 8) + notifyDataA[5];
	if (r4sCommandA(0x50, data, sizeof(data)) == 20) {
	bSCountA = (notifyDataA[9] << 24) + (notifyDataA[8] << 16) + (notifyDataA[7] << 8) + notifyDataA[6];
				}
			}
		}
	}
	}
	} else if (( DEV_TYPA > 9) && ( DEV_TYPA < 12 ) && (retc == 20)) {
	bHeatA = 0;
	bStateA = notifyDataA[11];
	bLockA = notifyDataA[10];                   //lock
	strcpy(cStatusA,"{\"state\":");
	itoa(notifyDataA[11],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"lock\":");
	itoa(notifyDataA[10],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"prog\":");
	itoa(notifyDataA[3],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"error\":");
	itoa(notifyDataA[12],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,"}");    
	} else if (( DEV_TYPA > 11 ) && ( DEV_TYPA < 16 ) && (retc == 20)) {
	bHeatA = 0;
	bStateA = notifyDataA[11];
	bLockA = notifyDataA[16];                   //lock
	bProgA = notifyDataA[14];                   //strength of coffee
	bPHourA = notifyDataA[6];
	bPMinA = notifyDataA[7];
	bCHourA = notifyDataA[8];                      //curr time
	bCMinA = notifyDataA[9];
	if (bStateA == 5) bStateA = 0;
	if (notifyDataA[11] == 5) bStNlA = 1;
	else bStNlA = 0;
	strcpy(cStatusA,"{\"state\":");
	itoa(notifyDataA[11],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"strength\":");
	itoa(notifyDataA[14],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"lock\":");
	itoa(notifyDataA[16],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"sethours\":");
	itoa(notifyDataA[6],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"setmins\":");
	itoa(notifyDataA[7],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"hours\":");
	itoa(notifyDataA[8],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"mins\":");
	itoa(notifyDataA[9],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"prog\":");
	itoa(notifyDataA[3],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"error\":");
	itoa(notifyDataA[12],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,"}");    
	} else if (( DEV_TYPA == 16 ) && (retc == 13)) {
	bHeatA = 0;
	bProgA = notifyDataA[3];
	bModProgA = notifyDataA[4];
	bHtempA = notifyDataA[5];
	bPHourA = notifyDataA[6];
	bPMinA = notifyDataA[7];
	bCHourA = notifyDataA[8];
	bCMinA = notifyDataA[9];
	bAwarmA = notifyDataA[10];
	bStateA = notifyDataA[11];
	if (!bStateA) bProgA = 255; 
	strcpy(cStatusA,"{\"prog\":");
	itoa(bProgA,tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"mode\":");
	itoa(notifyDataA[4],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"temp\":");
	itoa(notifyDataA[5],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"sethours\":");
	itoa(notifyDataA[6],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"setmins\":");
	itoa(notifyDataA[7],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"state\":");
	itoa(notifyDataA[11],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"hours\":");
	itoa(notifyDataA[8],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"mins\":");
	itoa(notifyDataA[9],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"warm\":");
	itoa(notifyDataA[10],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,"}");    
	} else if (( DEV_TYPA > 16 ) && ( DEV_TYPA < 24 ) && (retc == 20)) {
	bHeatA = 0;
	bProgA = notifyDataA[3];
	bModProgA = notifyDataA[4];
	bHtempA = notifyDataA[5];
	bPHourA = notifyDataA[6];
	bPMinA = notifyDataA[7];
	bCHourA = notifyDataA[8];
	bCMinA = notifyDataA[9];
	bAwarmA = notifyDataA[10];
	bStateA = notifyDataA[11];
	if (!bStateA) bProgA = 255; 
	strcpy(cStatusA,"{\"prog\":");
	itoa(bProgA,tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"mode\":");
	itoa(notifyDataA[4],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"temp\":");
	itoa(notifyDataA[5],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"sethours\":");
	itoa(notifyDataA[6],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"setmins\":");
	itoa(notifyDataA[7],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"state\":");
	itoa(notifyDataA[11],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"hours\":");
	itoa(notifyDataA[8],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"mins\":");
	itoa(notifyDataA[9],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"warm\":");
	itoa(notifyDataA[10],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,"}");    
	} else if (( DEV_TYPA == 24 ) && (retc == 20)) {
	bHeatA = 0;
	bProgA = notifyDataA[3];
	bModProgA = notifyDataA[4];
	bHtempA = notifyDataA[5];
	bPHourA = 0;
	bPMinA = 0;
	bDHourA = 0;
	bDMinA = 0;
	bCHourA = notifyDataA[9];
	bCMinA = notifyDataA[10];
	bStateA = notifyDataA[11];
	bAwarmA = notifyDataA[12];
	bLockA = notifyDataA[16];                   //lock
	if (!bStateA) bProgA = 255; 
        retc = r4sCommandA(0x10, 0, 0);	
	if (retc == 6) {
	bPHourA = notifyDataA[3];
	bPMinA = notifyDataA[4];
        retc = r4sCommandA(0x15, 0, 0);	
	if (retc == 6) {
	bDHourA = notifyDataA[3];
	bDMinA = notifyDataA[4];
	}
	}
	strcpy(cStatusA,"{\"prog\":");
	itoa(bProgA,tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"mode\":");
	itoa(bModProgA,tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"temp\":");
	itoa(bHtempA,tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"sethours\":");
	itoa(bPHourA,tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"setmins\":");
	itoa(bPMinA,tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"state\":");
	itoa(bStateA,tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"hours\":");
	itoa(bCHourA,tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"mins\":");
	itoa(bCMinA,tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"warm\":");
	itoa(bAwarmA,tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"lock\":");
	itoa(bLockA,tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,"}");    
	} else if (r4sAConnErr > 2) {
	cStatusA[0]=0;
	if (FDHass || !foffln) {
	bStateA = 0;
	bStNlA = 0;
	bStBlA = 0;
	bStBpA = 0;
	bAwarmA = 0;
	bHeatA =  0;
	} else {
	bStateA = 254;
	bStNlA = 254;
	bStBlA = 254;
	bStBpA = 254;
	bAwarmA = 254;
	bHeatA =  254;
	}
	bCtempA = 0;
	bHtempA = 0;
	bProgA = 0;
	bModProgA = 0;
	bLockA = 0;
	bPHourA = 0;
	bPMinA = 0;
	bCHourA = 0;
	bCMinA = 0;
	bDHourA = 0;
	bDMinA = 0;
	}
	} else if (btauthoriza && (DEV_TYPA > 63)) {
	sendDataALen = 0;
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_A_APP_ID].remote_bda);
	if (notifyDataALen == 12) {
        ESP_LOGI(AP_TAG, "Notify Data A:");
        esp_log_buffer_hex(AP_TAG, notifyDataA, notifyDataALen);
	bHtempA = notifyDataA[4];
	if ((notifyDataA[0] == 1) || (notifyDataA[1] == 1)) bStateA = 1;
	else bStateA = 0;
	if ((notifyDataA[1] != 2) || (xshedcoma == 2)) bHtempA = 0;
	if (bHtempA) bHeatA = 1;
	else bHeatA = 0;
        bModProgA = notifyDataA[1];
        bProgA = notifyDataA[0];
	bCtempA = notifyDataA[5];
	bStNlA = 0;
	strcpy(cStatusA,"{\"temp\":");
	itoa(notifyDataA[5],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"target\":");
	itoa(bHtempA,tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"heat\":");
	if (notifyDataA[1] == 2) strcat(cStatusA,"1");
	else strcat(cStatusA,"0");
	strcat(cStatusA,",\"state\":");
	itoa(bStateA,tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"action\":");
	itoa(notifyDataA[0],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"mode\":");
	itoa(notifyDataA[1],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"id\":");
	itoa(MiAKettleID,tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"warm_lock\":");
	itoa(notifyDataA[2],tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,",\"warm_min\":");
	tmpint = notifyDataA[7] + (notifyDataA[8]<<8 & 0xff00);
	itoa(tmpint,tmpvar,10);
	strcat(cStatusA,tmpvar);
	strcat(cStatusA,"}");
	if (!xshedcoma && (notifyDataA[0] == 0x02) && (notifyDataA[1] == 0x02) && (notifyDataA[6] == 0x00)) mAMiOfft();
	else if (tmpint > 255) mAMiRewarm();
	else if (!xshedcoma && (notifyDataA[10] != 0x18)) mAMiSWtime();
//	else if (!xshedcoma && (notifyDataA[0] == 0x03) && ((notifyDataA[10] != 0x18) || !notifyDataA[2])) mAMiSWtime();
	else if (xshedcoma == 2) {
	if (notifyDataA[1] == 0xff) {
	xshedcoma = 0;
        mAMiIdlTmp(40);
	mAMiSWtime();
	} else mAMiHeat(0);
        }
	notifyDataALen = -1;
	} else {
	r4sAConnErr++;
	if (r4sAConnErr > 2) { 
	cStatusA[0]=0;
	if (FDHass || !foffln) {
	bStateA = 0;
	bHeatA =  0;
	} else {
	bStateA = 254;
	bHeatA =  254;
	}
	bCtempA = 0;
	bHtempA = 0;
	}
	}

	} else {
	cStatusA[0]=0;
	if (FDHass || !foffln) {
	bStateA = 0;
	bStNlA = 0;
	bStBlA = 0;
	bStBpA = 0;
	bAwarmA = 0;
	bHeatA =  0;
	} else {
	bStateA = 254;
	bStNlA = 254;
	bStBlA = 254;
	bStBpA = 254;
	bAwarmA = 254;
	bHeatA =  254;
	}
	bCtempA = 0;
	bHtempA = 0;
	bProgA = 0;
	bModProgA = 0;
	bPHourA = 0;
	bPMinA = 0;
	bCHourA = 0;
	bCMinA = 0;
	bDHourA = 0;
	bDMinA = 0;
	}
}


void mBsStatus() {
	char tmpvar[8]; 
	int  tmpint;
	int  retc;
	if (btauthorizb && (DEV_TYPB < 64)) {
        retc = r4sCommandB(0x06, 0, 0);	
	if ((DEV_TYPB == 1) && (retc == 13)) {
	bCtempB = notifyDataB[5];
	bHeatB = notifyDataB[3];
	bStateB = notifyDataB[11];
	if (bStateB == 4) bStateB = 0;
	bProgB = notifyDataB[3];
	if (bProgB) bStateB = 0;
	bStNlB = 0;
	bStBlB = 0;
	bStBpB = 0;
	bHtempB = 0;
	switch (notifyDataB[4]) {
	case 1:
        bHtempB = 40;
	break;
	case 2:
        bHtempB = 55;
	break;
	case 3:
        bHtempB = 70;
	break;
	case 4:
        bHtempB = 85;
	break;
	case 5:
        bHtempB = 95;
	break;
	}
	strcpy(cStatusB,"{\"temp\":");
	itoa(notifyDataB[5],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"target\":");
	itoa(bHtempB,tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"state\":");
	itoa(notifyDataB[11],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"prog\":");
	itoa(notifyDataB[3],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,"]}");    
	} else if ((DEV_TYPB > 1) && ( DEV_TYPB < 10 ) && (retc == 20)) {
	bHtempB = notifyDataB[5];
	bStateB = notifyDataB[11];
	bProgB = notifyDataB[3];
	if ((bStateB == 4) || (bProgB && (bProgB != 2))) bStateB = 0;
	if (DEV_TYPB > 3) {
	bCtempB = notifyDataB[8];
	bHeatB = notifyDataB[11];
	if ((bHeatB == 4) || ((bProgB != 1) && (bProgB != 2))) bHeatB = 0;
	bStNlB = notifyDataB[11];
	if ((bStNlB == 4) || (bProgB != 3)) bStNlB = 0;
	bStBpB = notifyDataB[7];
	} else {
	bCtempB = notifyDataB[13];
//	bHeatB = notifyDataB[10];
	bHeatB = notifyDataB[3];
	bStNlB = 0;
	bStBpB = 0;
	}
	strcpy(cStatusB,"{\"temp\":");
	(DEV_TYPB > 3)? itoa(notifyDataB[8],tmpvar,10) : itoa(notifyDataB[13],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"target\":");
	itoa(notifyDataB[5],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"state\":");
	itoa(notifyDataB[11],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"prog\":");
	itoa(notifyDataB[3],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"beep\":");
	itoa(bStBpB,tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"error\":");
	itoa(notifyDataB[12],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"rgb\":[");
	itoa(RgbRB,tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",");    
	itoa(RgbGB,tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",");    
	itoa(RgbBB,tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,"]}");    
	if (DEV_TYPB > 3) {
	uint8_t data[] = {0xc8};
	if (r4sCommandB(0x35, data, sizeof(data)) == 9) {
	bStBlB = notifyDataB[5];
        if (!r4slpcoma && !r4slpcomb && !r4slpcomc && !r4slpresb) {
	data[0] = 0x00;
	if (r4sCommandB(0x47, data, sizeof(data)) == 20) {
	bSEnergyB = (notifyDataB[12] << 24) + (notifyDataB[11] << 16) + (notifyDataB[10] << 8) + notifyDataB[9];
	bSTimeB = (notifyDataB[8] << 24) + (notifyDataB[7] << 16) + (notifyDataB[6] << 8) + notifyDataB[5];
	if (r4sCommandB(0x50, data, sizeof(data)) == 20) {
	bSCountB = (notifyDataB[9] << 24) + (notifyDataB[8] << 16) + (notifyDataB[7] << 8) + notifyDataB[6];
				}
			}
		}
	}
	}
	} else if (( DEV_TYPB > 9) && ( DEV_TYPB < 12 ) && (retc == 20)) {
	bHeatB = 0;
	bStateB = notifyDataB[11];
	bLockB = notifyDataB[10];                   //lock
	strcpy(cStatusB,"{\"state\":");
	itoa(notifyDataB[11],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"lock\":");
	itoa(notifyDataB[10],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"prog\":");
	itoa(notifyDataB[3],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"error\":");
	itoa(notifyDataB[12],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,"}");    
	} else if (( DEV_TYPB > 11 ) && ( DEV_TYPB < 16 ) && (retc == 20)) {
	bHeatB = 0;
	bStateB = notifyDataB[11];
	bLockB = notifyDataB[16];                   //lock
	bProgB = notifyDataB[14];                   //strength of coffee
	bPHourB = notifyDataB[6];
	bPMinB = notifyDataB[7];
	bCHourB = notifyDataB[8];                      //curr time
	bCMinB = notifyDataB[9];
	if (bStateB == 5) bStateB = 0;
	if (notifyDataB[11] == 5) bStNlB = 1;
	else bStNlB = 0;
	strcpy(cStatusB,"{\"state\":");
	itoa(notifyDataB[11],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"strength\":");
	itoa(notifyDataB[14],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"lock\":");
	itoa(notifyDataB[16],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"sethours\":");
	itoa(notifyDataB[6],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"setmins\":");
	itoa(notifyDataB[7],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"hours\":");
	itoa(notifyDataB[8],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"mins\":");
	itoa(notifyDataB[9],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"prog\":");
	itoa(notifyDataB[3],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"error\":");
	itoa(notifyDataB[12],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,"}");    
	} else if (( DEV_TYPB == 16 ) && (retc == 13)) {
	bHeatB = 0;
	bProgB = notifyDataB[3];
	bModProgB = notifyDataB[4];
	bHtempB = notifyDataB[5];
	bPHourB = notifyDataB[6];
	bPMinB = notifyDataB[7];
	bCHourB = notifyDataB[8];
	bCMinB = notifyDataB[9];
	bAwarmB = notifyDataB[10];
	bStateB = notifyDataB[11];
	if (!bStateB) bProgB = 255; 
	strcpy(cStatusB,"{\"prog\":");
	itoa(bProgB,tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"mode\":");
	itoa(notifyDataB[4],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"temp\":");
	itoa(notifyDataB[5],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"sethours\":");
	itoa(notifyDataB[6],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"setmins\":");
	itoa(notifyDataB[7],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"state\":");
	itoa(notifyDataB[11],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"hours\":");
	itoa(notifyDataB[8],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"mins\":");
	itoa(notifyDataB[9],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"warm\":");
	itoa(notifyDataB[10],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,"}");    
	} else if (( DEV_TYPB > 16 ) && ( DEV_TYPB < 24 ) && (retc == 20)) {
	bHeatB = 0;
	bProgB = notifyDataB[3];
	bModProgB = notifyDataB[4];
	bHtempB = notifyDataB[5];
	bPHourB = notifyDataB[6];
	bPMinB = notifyDataB[7];
	bCHourB = notifyDataB[8];
	bCMinB = notifyDataB[9];
	bAwarmB = notifyDataB[10];
	bStateB = notifyDataB[11];
	if (!bStateB) bProgB = 255; 
	strcpy(cStatusB,"{\"prog\":");
	itoa(bProgB,tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"mode\":");
	itoa(notifyDataB[4],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"temp\":");
	itoa(notifyDataB[5],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"sethours\":");
	itoa(notifyDataB[6],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"setmins\":");
	itoa(notifyDataB[7],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"state\":");
	itoa(notifyDataB[11],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"hours\":");
	itoa(notifyDataB[8],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"mins\":");
	itoa(notifyDataB[9],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"warm\":");
	itoa(notifyDataB[10],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,"}");    
	} else if (( DEV_TYPB == 24 ) && (retc == 20)) {
	bHeatB = 0;
	bProgB = notifyDataB[3];
	bModProgB = notifyDataB[4];
	bHtempB = notifyDataB[5];
	bPHourB = 0;
	bPMinB = 0;
	bDHourB = 0;
	bDMinB = 0;
	bCHourB = notifyDataB[9];
	bCMinB = notifyDataB[10];
	bStateB = notifyDataB[11];
	bAwarmB = notifyDataB[12];
	bLockB = notifyDataB[16];                   //lock
	if (!bStateB) bProgB = 255; 
        retc = r4sCommandB(0x10, 0, 0);	
	if (retc == 6) {
	bPHourB = notifyDataB[3];
	bPMinB = notifyDataB[4];
        retc = r4sCommandB(0x15, 0, 0);	
	if (retc == 6) {
	bDHourB = notifyDataB[3];
	bDMinB = notifyDataB[4];
	}
	}
	strcpy(cStatusB,"{\"prog\":");
	itoa(bProgB,tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"mode\":");
	itoa(bModProgB,tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"temp\":");
	itoa(bHtempB,tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"sethours\":");
	itoa(bPHourB,tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"setmins\":");
	itoa(bPMinB,tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"state\":");
	itoa(bStateB,tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"hours\":");
	itoa(bCHourB,tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"mins\":");
	itoa(bCMinB,tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"warm\":");
	itoa(bAwarmB,tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"lock\":");
	itoa(bLockB,tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,"}");    
	} else if (r4sBConnErr > 2) {
	cStatusB[0]=0;
	if (FDHass || !foffln) {
	bStateB = 0;
	bStNlB = 0;
	bStBlB = 0;
	bStBpB = 0;
	bAwarmB = 0;
	bHeatB =  0;
	} else {
	bStateB = 254;
	bStNlB = 254;
	bStBlB = 254;
	bStBpB = 254;
	bAwarmB = 254;
	bHeatB =  254;
	}
	bCtempB = 0;
	bHtempB = 0;
	bProgB = 0;
	bModProgB = 0;
	bLockB = 0;
	bPHourB = 0;
	bPMinB = 0;
	bCHourB = 0;
	bCMinB = 0;
	bDHourB = 0;
	bDMinB = 0;
	}
	} else if (btauthorizb && (DEV_TYPB > 63)) {
	sendDataBLen = 0;
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_B_APP_ID].remote_bda);
	if (notifyDataBLen == 12) {
        ESP_LOGI(AP_TAG, "Notify Data B:");
        esp_log_buffer_hex(AP_TAG, notifyDataB, notifyDataBLen);
	bHtempB = notifyDataB[4];
	if ((notifyDataB[0] == 1) || (notifyDataB[1] == 1)) bStateB = 1;
	else bStateB = 0;
	if ((notifyDataB[1] != 2) || (xshedcomb == 2)) bHtempB = 0;
	if (bHtempB) bHeatB = 1;
	else bHeatB = 0;
        bModProgB = notifyDataB[1];
        bProgB = notifyDataB[0];
	bCtempB = notifyDataB[5];
	bStNlB = 0;
	strcpy(cStatusB,"{\"temp\":");
	itoa(notifyDataB[5],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"target\":");
	itoa(bHtempB,tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"heat\":");
	if (notifyDataB[1] == 2) strcat(cStatusB,"1");
	else strcat(cStatusB,"0");
	strcat(cStatusB,",\"state\":");
	itoa(bStateB,tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"action\":");
	itoa(notifyDataB[0],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"mode\":");
	itoa(notifyDataB[1],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"id\":");
	itoa(MiBKettleID,tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"warm_lock\":");
	itoa(notifyDataB[2],tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,",\"warm_min\":");
	tmpint = notifyDataB[7] + (notifyDataB[8]<<8 & 0xff00);
	itoa(tmpint,tmpvar,10);
	strcat(cStatusB,tmpvar);
	strcat(cStatusB,"}");
	if (!xshedcomb && (notifyDataB[0] == 0x02) && (notifyDataB[1] == 0x02) && (notifyDataB[6] == 0x00)) mBMiOfft();
	else if (tmpint > 255) mBMiRewarm();
	else if (!xshedcomb && (notifyDataB[10] != 0x18)) mBMiSWtime();
//	else if (!xshedcomb && (notifyDataB[0] == 0x03) && ((notifyDataB[10] != 0x18) || !notifyDataB[2])) mBMiSWtime();
	else if (xshedcomb == 2) {
	if (notifyDataB[1] == 0xff) {
	xshedcomb = 0;
        mBMiIdlTmp(40);
	mBMiSWtime();
	} else mBMiHeat(0);
        }
	notifyDataBLen = -1;
	} else {
	r4sBConnErr++;
	if (r4sBConnErr > 2) { 
	cStatusB[0]=0;
	if (FDHass || !foffln) {
	bStateB = 0;
	bHeatB =  0;
	} else {
	bStateB = 254;
	bHeatB =  254;
	}
	bCtempB = 0;
	bHtempB = 0;
	}
	}

	} else {
	cStatusB[0]=0;
	if (FDHass || !foffln) {
	bStateB = 0;
	bStNlB = 0;
	bStBlB = 0;
	bStBpB = 0;
	bAwarmB = 0;
	bHeatB =  0;
	} else {
	bStateB = 254;
	bStNlB = 254;
	bStBlB = 254;
	bStBpB = 254;
	bAwarmB = 254;
	bHeatB =  254;
	}
	bCtempB = 0;
	bHtempB = 0;
	bProgB = 0;
	bModProgB = 0;
	bPHourB = 0;
	bPMinB = 0;
	bCHourB = 0;
	bCMinB = 0;
	bDHourB = 0;
	bDMinB = 0;
	}
}


void mCsStatus() {
	char tmpvar[8]; 
	int  tmpint;
	int  retc;
	if (btauthorizc && (DEV_TYPC < 64)) {
        retc = r4sCommandC(0x06, 0, 0);	
	if ((DEV_TYPC == 1) && (retc == 13)) {
	bCtempC = notifyDataC[5];
	bHeatC = notifyDataC[3];
	bStateC = notifyDataC[11];
	if (bStateC == 4) bStateC = 0;
	bProgC = notifyDataC[3];
	if (bProgC) bStateC = 0;
	bStNlC = 0;
	bStBlC = 0;
	bStBpC = 0;
	bHtempC = 0;
	switch (notifyDataC[4]) {
	case 1:
        bHtempC = 40;
	break;
	case 2:
        bHtempC = 55;
	break;
	case 3:
        bHtempC = 70;
	break;
	case 4:
        bHtempC = 85;
	break;
	case 5:
        bHtempC = 95;
	break;
	}
	strcpy(cStatusC,"{\"temp\":");
	itoa(notifyDataC[5],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"target\":");
	itoa(bHtempC,tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"state\":");
	itoa(notifyDataC[11],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"prog\":");
	itoa(notifyDataC[3],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,"]}");    
	} else if ((DEV_TYPC > 1) && ( DEV_TYPC < 10 ) && (retc == 20)) {
	bHtempC = notifyDataC[5];
	bStateC = notifyDataC[11];
	bProgC = notifyDataC[3];
	if ((bStateC == 4) || (bProgC && (bProgC != 2))) bStateC = 0;
	if (DEV_TYPC > 3) {
	bCtempC = notifyDataC[8];
	bHeatC = notifyDataC[11];
	if ((bHeatC == 4) || ((bProgC != 1) && (bProgC != 2))) bHeatC = 0;
	bStNlC = notifyDataC[11];
	if ((bStNlC == 4) || (bProgC != 3)) bStNlC = 0;
	bStBpC = notifyDataC[7];
	} else {
	bCtempC = notifyDataC[13];
//	bHeatC = notifyDataC[10];
	bHeatC = notifyDataC[3];
	bStNlC = 0;
	bStBpC = 0;
	}
	strcpy(cStatusC,"{\"temp\":");
	(DEV_TYPC > 3)? itoa(notifyDataC[8],tmpvar,10) : itoa(notifyDataC[13],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"target\":");
	itoa(notifyDataC[5],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"state\":");
	itoa(notifyDataC[11],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"prog\":");
	itoa(notifyDataC[3],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"beep\":");
	itoa(bStBpC,tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"error\":");
	itoa(notifyDataC[12],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"rgb\":[");
	itoa(RgbRC,tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",");    
	itoa(RgbGC,tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",");    
	itoa(RgbBC,tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,"]}");    
	if (DEV_TYPC > 3) {
	uint8_t data[] = {0xc8};
	if (r4sCommandC(0x35, data, sizeof(data)) == 9) {
	bStBlC = notifyDataC[5];
        if (!r4slpcoma && !r4slpcomb && !r4slpcomc && !r4slpresc) {
	data[0] = 0x00;
	if (r4sCommandC(0x47, data, sizeof(data)) == 20) {
	bSEnergyC = (notifyDataC[12] << 24) + (notifyDataC[11] << 16) + (notifyDataC[10] << 8) + notifyDataC[9];
	bSTimeC = (notifyDataC[8] << 24) + (notifyDataC[7] << 16) + (notifyDataC[6] << 8) + notifyDataC[5];
	if (r4sCommandC(0x50, data, sizeof(data)) == 20) {
	bSCountC = (notifyDataC[9] << 24) + (notifyDataC[8] << 16) + (notifyDataC[7] << 8) + notifyDataC[6];
				}
			}
		}
	}
	}
	} else if (( DEV_TYPC > 9) && ( DEV_TYPC < 12 ) && (retc == 20)) {
	bHeatC = 0;
	bStateC = notifyDataC[11];
	bLockC = notifyDataC[10];                   //lock
	strcpy(cStatusC,"{\"state\":");
	itoa(notifyDataC[11],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"lock\":");
	itoa(notifyDataC[10],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"prog\":");
	itoa(notifyDataC[3],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"error\":");
	itoa(notifyDataC[12],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,"}");    
	} else if (( DEV_TYPC > 11 ) && ( DEV_TYPC < 16 ) && (retc == 20)) {
	bHeatC = 0;
	bStateC = notifyDataC[11];
	bLockC = notifyDataC[16];                   //lock
	bProgC = notifyDataC[14];                   //strength of coffee
	bPHourC = notifyDataC[6];
	bPMinC = notifyDataC[7];
	bCHourC = notifyDataC[8];                      //curr time
	bCMinC = notifyDataC[9];
	if (bStateC == 5) bStateC = 0;
	if (notifyDataC[11] == 5) bStNlC = 1;
	else bStNlC = 0;
	strcpy(cStatusC,"{\"state\":");
	itoa(notifyDataC[11],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"strength\":");
	itoa(notifyDataC[14],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"lock\":");
	itoa(notifyDataC[16],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"sethours\":");
	itoa(notifyDataC[6],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"setmins\":");
	itoa(notifyDataC[7],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"hours\":");
	itoa(notifyDataC[8],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"mins\":");
	itoa(notifyDataC[9],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"prog\":");
	itoa(notifyDataC[3],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"error\":");
	itoa(notifyDataC[12],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,"}");    
	} else if (( DEV_TYPC == 16 ) && (retc == 13)) {
	bHeatC = 0;
	bProgC = notifyDataC[3];
	bModProgC = notifyDataC[4];
	bHtempC = notifyDataC[5];
	bPHourC = notifyDataC[6];
	bPMinC = notifyDataC[7];
	bCHourC = notifyDataC[8];
	bCMinC = notifyDataC[9];
	bAwarmC = notifyDataC[10];
	bStateC = notifyDataC[11];
	if (!bStateC) bProgC = 255; 
	strcpy(cStatusC,"{\"prog\":");
	itoa(bProgC,tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"mode\":");
	itoa(notifyDataC[4],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"temp\":");
	itoa(notifyDataC[5],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"sethours\":");
	itoa(notifyDataC[6],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"setmins\":");
	itoa(notifyDataC[7],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"state\":");
	itoa(notifyDataC[11],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"hours\":");
	itoa(notifyDataC[8],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"mins\":");
	itoa(notifyDataC[9],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"warm\":");
	itoa(notifyDataC[10],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,"}");    
	} else if (( DEV_TYPC > 16 ) && ( DEV_TYPC < 24 ) && (retc == 20)) {
	bHeatC = 0;
	bProgC = notifyDataC[3];
	bModProgC = notifyDataC[4];
	bHtempC = notifyDataC[5];
	bPHourC = notifyDataC[6];
	bPMinC = notifyDataC[7];
	bCHourC = notifyDataC[8];
	bCMinC = notifyDataC[9];
	bAwarmC = notifyDataC[10];
	bStateC = notifyDataC[11];
	if (!bStateC) bProgC = 255; 
	strcpy(cStatusC,"{\"prog\":");
	itoa(bProgC,tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"mode\":");
	itoa(notifyDataC[4],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"temp\":");
	itoa(notifyDataC[5],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"sethours\":");
	itoa(notifyDataC[6],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"setmins\":");
	itoa(notifyDataC[7],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"state\":");
	itoa(notifyDataC[11],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"hours\":");
	itoa(notifyDataC[8],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"mins\":");
	itoa(notifyDataC[9],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"warm\":");
	itoa(notifyDataC[10],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,"}");    
	} else if (( DEV_TYPC == 24 ) && (retc == 20)) {
	bHeatC = 0;
	bProgC = notifyDataC[3];
	bModProgC = notifyDataC[4];
	bHtempC = notifyDataC[5];
	bPHourC = 0;
	bPMinC = 0;
	bDHourC = 0;
	bDMinC = 0;
	bCHourC = notifyDataC[9];
	bCMinC = notifyDataC[10];
	bStateC = notifyDataC[11];
	bAwarmC = notifyDataC[12];
	bLockC = notifyDataC[16];                   //lock
	if (!bStateC) bProgC = 255; 
        retc = r4sCommandC(0x10, 0, 0);	
	if (retc == 6) {
	bPHourC = notifyDataC[3];
	bPMinC = notifyDataC[4];
        retc = r4sCommandC(0x15, 0, 0);	
	if (retc == 6) {
	bDHourC = notifyDataC[3];
	bDMinC = notifyDataC[4];
	}
	}
	strcpy(cStatusC,"{\"prog\":");
	itoa(bProgC,tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"mode\":");
	itoa(bModProgC,tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"temp\":");
	itoa(bHtempC,tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"sethours\":");
	itoa(bPHourC,tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"setmins\":");
	itoa(bPMinC,tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"state\":");
	itoa(bStateC,tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"hours\":");
	itoa(bCHourC,tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"mins\":");
	itoa(bCMinC,tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"warm\":");
	itoa(bAwarmC,tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"lock\":");
	itoa(bLockC,tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,"}");    
	} else if (r4sCConnErr > 2) {
	cStatusC[0]=0;
	if (FDHass || !foffln) {
	bStateC = 0;
	bStNlC = 0;
	bStBlC = 0;
	bStBpC = 0;
	bAwarmC = 0;
	bHeatC =  0;
	} else {
	bStateC = 254;
	bStNlC = 254;
	bStBlC = 254;
	bStBpC = 254;
	bAwarmC = 254;
	bHeatC =  254;
	}
	bCtempC = 0;
	bHtempC = 0;
	bProgC = 0;
	bModProgC = 0;
	bLockC = 0;
	bPHourC = 0;
	bPMinC = 0;
	bCHourC = 0;
	bCMinC = 0;
	bDHourC = 0;
	bDMinC = 0;
	}
	} else if (btauthorizc && (DEV_TYPC > 63)) {
	sendDataCLen = 0;
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_C_APP_ID].remote_bda);
	if (notifyDataCLen == 12) {
        ESP_LOGI(AP_TAG, "Notify Data C:");
        esp_log_buffer_hex(AP_TAG, notifyDataC, notifyDataCLen);
	bHtempC = notifyDataC[4];
	if ((notifyDataC[0] == 1) || (notifyDataC[1] == 1)) bStateC = 1;
	else bStateC = 0;
	if ((notifyDataC[1] != 2) || (xshedcomc == 2)) bHtempC = 0;
	if (bHtempC) bHeatC = 1;
	else bHeatC = 0;
        bModProgC = notifyDataC[1];
        bProgC = notifyDataC[0];
	bCtempC = notifyDataC[5];
	bStNlC = 0;
	strcpy(cStatusC,"{\"temp\":");
	itoa(notifyDataC[5],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"target\":");
	itoa(bHtempC,tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"heat\":");
	if (notifyDataC[1] == 2) strcat(cStatusC,"1");
	else strcat(cStatusC,"0");
	strcat(cStatusC,",\"state\":");
	itoa(bStateC,tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"action\":");
	itoa(notifyDataC[0],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"mode\":");
	itoa(notifyDataC[1],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"id\":");
	itoa(MiCKettleID,tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"warm_lock\":");
	itoa(notifyDataC[2],tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,",\"warm_min\":");
	tmpint = notifyDataC[7] + (notifyDataC[8]<<8 & 0xff00);
	itoa(tmpint,tmpvar,10);
	strcat(cStatusC,tmpvar);
	strcat(cStatusC,"}");
	if (!xshedcomc && (notifyDataC[0] == 0x02) && (notifyDataC[1] == 0x02) && (notifyDataC[6] == 0x00)) mCMiOfft();
	else if (tmpint > 255) mCMiRewarm();
	else if (!xshedcomc && (notifyDataC[10] != 0x18)) mCMiSWtime();
//	else if (!xshedcomc && (notifyDataC[0] == 0x03) && ((notifyDataC[10] != 0x18) || !notifyDataC[2])) mCMiSWtime();
	else if (xshedcomc == 2) {
	if (notifyDataC[1] == 0xff) {
	xshedcomc = 0;
        mCMiIdlTmp(40);
	mCMiSWtime();
	} else mCMiHeat(0);
        }
	notifyDataCLen = -1;
	} else {
	r4sCConnErr++;
	if (r4sCConnErr > 2) { 
	cStatusC[0]=0;
	if (FDHass || !foffln) {
	bStateC = 0;
	bHeatC =  0;
	} else {
	bStateC = 254;
	bHeatC =  254;
	}
	bCtempC = 0;
	bHtempC = 0;
	}
	}

	} else {
	cStatusC[0]=0;
	if (FDHass || !foffln) {
	bStateC = 0;
	bStNlC = 0;
	bStBlC = 0;
	bStBpC = 0;
	bAwarmC = 0;
	bHeatC =  0;
	} else {
	bStateC = 254;
	bStNlC = 254;
	bStBlC = 254;
	bStBpC = 254;
	bAwarmC = 254;
	bHeatC =  254;
	}
	bCtempC = 0;
	bHtempC = 0;
	bProgC = 0;
	bModProgC = 0;
	bPHourC = 0;
	bPMinC = 0;
	bCHourC = 0;
	bCMinC = 0;
	bDHourC = 0;
	bDMinC = 0;
	}
}
void MqSState() {
	if (mqtdel) return;
	int64_t t_mqt_us = esp_timer_get_time();
	char ldata[64];
//	char tmpvar[32]; 

	if  ((mqttConnected) && (bStateS != bprevStateS)) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/screen");
	if (!bStateS) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcoms = 255;
	t_ppcoms_us = t_mqt_us;
	bprevStateS = bStateS;
	t_lasts_us = ~t_lasts_us;
	}

	if  ((mqttConnected) && (bgpio1 > 63) && (bgpio1 < 192) && fgpio1) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/gpio1");
	if (!lvgpio1) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcoms = 255;
	t_ppcoms_us = t_mqt_us;
	fgpio1 = 0;
	}
	if  ((mqttConnected) && (bgpio2 > 63) && (bgpio2 < 192) && fgpio2) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/gpio2");
	if (!lvgpio2) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcoms = 255;
	t_ppcoms_us = t_mqt_us;
	fgpio2 = 0;
	}
	if  ((mqttConnected) && (bgpio3 > 63) && (bgpio3 < 192) && fgpio3) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/gpio3");
	if (!lvgpio3) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcoms = 255;
	t_ppcoms_us = t_mqt_us;
	fgpio3 = 0;
	}
	if  ((mqttConnected) && (bgpio4 > 63) && (bgpio4 < 192) && fgpio4) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/gpio4");
	if (!lvgpio4) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcoms = 255;
	t_ppcoms_us = t_mqt_us;
	fgpio4 = 0;
	}
	if  ((mqttConnected) && (bgpio5 > 63) && (bgpio5 < 192) && fgpio5) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/gpio5");
	if (!lvgpio5) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcoms = 255;
	t_ppcoms_us = t_mqt_us;
	fgpio5 = 0;
	}
}


void MqAState() {
	if (mqtdel) return;
	int64_t t_mqt_us = esp_timer_get_time();
	char ldata[64];
	char tmpvar[32]; 
	wifi_ap_record_t wifidata;
	if (mqttConnected && (esp_wifi_sta_get_ap_info(&wifidata)==0)){
	iRssiESP = wifidata.rssi;
	if  (iprevRssiESP != iRssiESP) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/rssi");
        itoa(iRssiESP,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	iprevRssiESP = iRssiESP;
	}

	}
	mAsStatus();
	if ((mqttConnected) && (tBLEAddrA[0]) && (DEV_TYPA)) {
	if ((cStatusA[0] != 0) && (strcmp(cStatusA,cprevStatusA) != 0)) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/json");
	esp_mqtt_client_publish(mqttclient, ldata, cStatusA, 0, 1, 1);
	strcpy(cprevStatusA,cStatusA);
	} 
	if  (iprevRssiA != iRssiA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	strcat(ldata,"/rssi");
        itoa(iRssiA,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	iprevRssiA = iRssiA;
	}
	if ((DEV_TYPA < 10) || (DEV_TYPA > 63)) {
	if  (bprevCtempA != bCtempA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/temp");
	itoa(bCtempA,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevCtempA = bCtempA;
	}
	if  (bprevHtempA != bHtempA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/heat_temp");
	itoa(bHtempA,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcoma = 255;
	t_ppcoma_us = t_mqt_us;
	if (bHtempA > 29) bLtempA = bHtempA;
	bprevHtempA = bHtempA;
	}
	if  (bprevStateA != bStateA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/boil");
	if (!bStateA) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (bStateA == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcoma = 255;
	t_ppcoma_us = t_mqt_us;
	bprevStateA = bStateA;
	}
	if  (bprevHeatA != bHeatA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/heat");
	if (!bHeatA) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (bHeatA == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/hstate");
	if (!bHeatA) esp_mqtt_client_publish(mqttclient, ldata, "off", 0, 1, 1);
        else if (bHeatA == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, "heat", 0, 1, 1);
	r4sppcoma = 255;
	t_ppcoma_us = t_mqt_us;
	bprevHeatA = bHeatA;
	}
	if (DEV_TYPA < 10) {
	if  (bprevStNlA != bStNlA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/nightlight");
	if (!bStNlA) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (bStateA == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcoma = 255;
	t_ppcoma_us = t_mqt_us;
	bprevStNlA = bStNlA;
	}
	if  ((PRgbRA != RgbRA) || (PRgbGA != RgbGA) || (PRgbBA != RgbBA)) {
	ldata[0] = 0;
        itoa(RgbRA,ldata,10);
	strcpy(tmpvar,ldata);
        itoa(RgbGA,ldata,10);
	strcat(tmpvar,",");
	strcat(tmpvar,ldata);
        itoa(RgbBA,ldata,10);
	strcat(tmpvar,",");
	strcat(tmpvar,ldata);
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/nightlight_rgb");
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcoma = 255;
	t_ppcoma_us = t_mqt_us;
//	PRgbRA = RgbRA;
//	PRgbGA = RgbGA;
//	PRgbBA = RgbBA;
	}
	if  (PRgbRA != RgbRA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/nightlight_red");
        itoa(RgbRA,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcoma = 255;
	t_ppcoma_us = t_mqt_us;
	PRgbRA = RgbRA;
	}
	if  (PRgbGA != RgbGA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/nightlight_green");
        itoa(RgbGA,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcoma = 255;
	t_ppcoma_us = t_mqt_us;
	PRgbGA = RgbGA;
	}
	if  (PRgbBA != RgbBA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/nightlight_blue");
        itoa(RgbBA,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcoma = 255;
	t_ppcoma_us = t_mqt_us;
	PRgbBA = RgbBA;
	}
	if  (bprevStBlA != bStBlA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/backlight");
	if (!bStBlA) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (bStBlA == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcoma = 255;
	t_ppcoma_us = t_mqt_us;
	bprevStBlA = bStBlA;
	}
	if  (bprevStBpA != bStBpA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/beep");
	if (!bStBpA) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (bStBpA == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcoma = 255;
	t_ppcoma_us = t_mqt_us;
	bprevStBpA = bStBpA;
	}
	if  (bSEnergyA != bprevSEnergyA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/total_energy");
        itoakw(bSEnergyA,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcoma = 255;
	t_ppcoma_us = t_mqt_us;
	bprevSEnergyA = bSEnergyA;
	}
	if  (bSTimeA != bprevSTimeA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/working_time");
        itoat(bSTimeA,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcoma = 255;
	t_ppcoma_us = t_mqt_us;
	bprevSTimeA = bSTimeA;
	}
	if  (bSCountA != bprevSCountA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/working_count");
        itoa(bSCountA,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcoma = 255;
	t_ppcoma_us = t_mqt_us;
	bprevSCountA = bSCountA;
	}
	}
	} else if ( DEV_TYPA < 12) {
	if  (bprevStateA != bStateA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/state");
	if (!bStateA) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (bStateA == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcoma = 255;
	t_ppcoma_us = t_mqt_us;
	bprevStateA = bStateA;
	}
	if  (bprevLockA != bLockA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/lock");
	if (!bLockA) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcoma = 255;
	t_ppcoma_us = t_mqt_us;
	bprevLockA = bLockA;
	}
	} else if ( DEV_TYPA < 16) {
	if  (bprevStateA != bStateA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/state");
	if (!bStateA) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (bStateA == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcoma = 255;
	t_ppcoma_us = t_mqt_us;
	bprevStateA = bStateA;
	}
	if  (bprevStNlA != bStNlA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/delay");
	if (!bStNlA) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (bStNlA == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcoma = 255;
	t_ppcoma_us = t_mqt_us;
	bprevStNlA = bStNlA;
	}
	if  (bprevLockA != bLockA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/lock");
	if (!bLockA) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcoma = 255;
	t_ppcoma_us = t_mqt_us;
	bprevLockA = bLockA;
	}
	if  (bprevProgA != bProgA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/strength");
	if (!bProgA) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcoma = 255;
	t_ppcoma_us = t_mqt_us;
	bprevProgA = bProgA;
	}
	if  (bprevPHourA != bPHourA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/set_hour");
	itoa(bPHourA,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevPHourA = bPHourA;
	}
	if  (bprevPMinA != bPMinA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/set_min");
	itoa(bPMinA,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevPMinA = bPMinA;
	}
	if  (bprevDHourA != bDHourA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/delay_hour");
	itoa(bDHourA,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevDHourA = bDHourA;
	}
	if  (bprevDMinA != bDMinA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/delay_min");
	itoa(bDMinA,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevDMinA = bDMinA;
	}
	if  (bprevCHourA != bCHourA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/hour");
	itoa(bCHourA,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevCHourA = bCHourA;
	}
	if  (bprevCMinA != bCMinA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/min");
	itoa(bCMinA,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevCMinA = bCMinA;
	}
	} else if (DEV_TYPA < 64) {
	if  (bprevStateA != bStateA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/state");
	if (!bStateA) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (bStateA == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else if (bStateA == 1) esp_mqtt_client_publish(mqttclient, ldata, "SETTING", 0, 1, 1);
        else if (bStateA == 2) esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
        else if (bStateA == 4) esp_mqtt_client_publish(mqttclient, ldata, "WARMING", 0, 1, 1);
        else if (bStateA == 5) esp_mqtt_client_publish(mqttclient, ldata, "DELAYEDSTART", 0, 1, 1);
        else if (bStateA == 6) esp_mqtt_client_publish(mqttclient, ldata, "WAITPASTA", 0, 1, 1);
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/hstate");
	if (bStateA < 2) esp_mqtt_client_publish(mqttclient, ldata, "off", 0, 1, 1);
        else if (bStateA == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, "heat", 0, 1, 1);
	r4sppcoma = 255;
	t_ppcoma_us = t_mqt_us;
	bprevStateA = bStateA;
	}
	if  ((bprevProgA != bProgA) || (bprevModProgA != bModProgA)){
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/prname");
        if ( DEV_TYPA == 16 ) {
// for RMC-800s
	switch (bProgA) {
	case 255:
	esp_mqtt_client_publish(mqttclient, ldata, "OFF", 0, 1, 1);
	break;
	case 0:
	esp_mqtt_client_publish(mqttclient, ldata, "Multicooker", 0, 1, 1);
	break;
	case 1:
	esp_mqtt_client_publish(mqttclient, ldata, "Rice", 0, 1, 1);
	break;
	case 2:
	esp_mqtt_client_publish(mqttclient, ldata, "Slow_cooking", 0, 1, 1);
	break;
	case 3:
	esp_mqtt_client_publish(mqttclient, ldata, "Pilaf", 0, 1, 1);
	break;
	case 4:
	if (bModProgA == 1) esp_mqtt_client_publish(mqttclient, ldata, "Frying_vegetables", 0, 1, 1);
	else if (bModProgA == 2) esp_mqtt_client_publish(mqttclient, ldata, "Frying_fish", 0, 1, 1);
	else if (bModProgA == 3) esp_mqtt_client_publish(mqttclient, ldata, "Frying_meat", 0, 1, 1);
	else esp_mqtt_client_publish(mqttclient, ldata, "Invalid mode", 0, 1, 1);
	break;
	case 5:
	if (bModProgA == 1) esp_mqtt_client_publish(mqttclient, ldata, "Stewing_vegetables", 0, 1, 1);
	else if (bModProgA == 2) esp_mqtt_client_publish(mqttclient, ldata, "Stewing_fish", 0, 1, 1);
	else if (bModProgA == 3) esp_mqtt_client_publish(mqttclient, ldata, "Stewing_meat", 0, 1, 1);
	else esp_mqtt_client_publish(mqttclient, ldata, "Invalid mode", 0, 1, 1);
	break;
	case 6:
	esp_mqtt_client_publish(mqttclient, ldata, "Pasta", 0, 1, 1);
	break;
	case 7:
	esp_mqtt_client_publish(mqttclient, ldata, "Milk_porridge", 0, 1, 1);
	break;
	case 8:
	esp_mqtt_client_publish(mqttclient, ldata, "Soup", 0, 1, 1);
	break;
	case 9:
	esp_mqtt_client_publish(mqttclient, ldata, "Yogurt", 0, 1, 1);
	break;
	case 10:
	esp_mqtt_client_publish(mqttclient, ldata, "Baking", 0, 1, 1);
	break;
	case 11:
	if (bModProgA == 1) esp_mqtt_client_publish(mqttclient, ldata, "Steam_vegetables", 0, 1, 1);
	else if (bModProgA == 2) esp_mqtt_client_publish(mqttclient, ldata, "Steam_fish", 0, 1, 1);
	else if (bModProgA == 3) esp_mqtt_client_publish(mqttclient, ldata, "Steam_meat", 0, 1, 1);
	else esp_mqtt_client_publish(mqttclient, ldata, "Invalid mode", 0, 1, 1);
	break;
	case 12:
	esp_mqtt_client_publish(mqttclient, ldata, "Hot", 0, 1, 1);
	break;
	}
	} else if ( DEV_TYPA == 17 ) {
// for RMC-903s
	switch (bProgA) {
	case 255:
	esp_mqtt_client_publish(mqttclient, ldata, "OFF", 0, 1, 1);
	break;
	case 0:
	esp_mqtt_client_publish(mqttclient, ldata, "Multicooker", 0, 1, 1);
	break;
	case 1:
	esp_mqtt_client_publish(mqttclient, ldata, "Milk_porridge", 0, 1, 1);
	break;
	case 2:
	esp_mqtt_client_publish(mqttclient, ldata, "Stewing", 0, 1, 1);
	break;
	case 3:
	esp_mqtt_client_publish(mqttclient, ldata, "Frying", 0, 1, 1);
	break;
	case 4:
	esp_mqtt_client_publish(mqttclient, ldata, "Soup", 0, 1, 1);
	break;
	case 5:
	esp_mqtt_client_publish(mqttclient, ldata, "Steam", 0, 1, 1);
	break;
	case 6:
	esp_mqtt_client_publish(mqttclient, ldata, "Pasta", 0, 1, 1);
	break;
	case 7:
	esp_mqtt_client_publish(mqttclient, ldata, "Slow_cooking", 0, 1, 1);
	break;
	case 8:
	esp_mqtt_client_publish(mqttclient, ldata, "Hot", 0, 1, 1);
	break;
	case 9:
	esp_mqtt_client_publish(mqttclient, ldata, "Baking", 0, 1, 1);
	break;
	case 10:
	esp_mqtt_client_publish(mqttclient, ldata, "Groats", 0, 1, 1);
	break;
	case 11:
	esp_mqtt_client_publish(mqttclient, ldata, "Pilaf", 0, 1, 1);
	break;
	case 12:
	esp_mqtt_client_publish(mqttclient, ldata, "Yogurt", 0, 1, 1);
	break;
	case 13:
	esp_mqtt_client_publish(mqttclient, ldata, "Pizza", 0, 1, 1);
	break;
	case 14:
	esp_mqtt_client_publish(mqttclient, ldata, "Bread", 0, 1, 1);
	break;
	case 15:
	esp_mqtt_client_publish(mqttclient, ldata, "Desserts", 0, 1, 1);
	break;
	case 16:
	esp_mqtt_client_publish(mqttclient, ldata, "Express", 0, 1, 1);
	break;
	}
	} else if ( DEV_TYPA == 18 ) {
// for RMC-224s
	switch (bProgA) {
	case 255:
	esp_mqtt_client_publish(mqttclient, ldata, "OFF", 0, 1, 1);
	break;
	case 0:
	esp_mqtt_client_publish(mqttclient, ldata, "Frying", 0, 1, 1);
	break;
	case 1:
	esp_mqtt_client_publish(mqttclient, ldata, "Groats", 0, 1, 1);
	break;
	case 2:
	esp_mqtt_client_publish(mqttclient, ldata, "Multicooker", 0, 1, 1);
	break;
	case 3:
	esp_mqtt_client_publish(mqttclient, ldata, "Pilaf", 0, 1, 1);
	break;
	case 4:
	esp_mqtt_client_publish(mqttclient, ldata, "Steam", 0, 1, 1);
	break;
	case 5:
	esp_mqtt_client_publish(mqttclient, ldata, "Baking", 0, 1, 1);
	break;
	case 6:
	esp_mqtt_client_publish(mqttclient, ldata, "Stewing", 0, 1, 1);
	break;
	case 7:
	esp_mqtt_client_publish(mqttclient, ldata, "Soup", 0, 1, 1);
	break;
	case 8:
	esp_mqtt_client_publish(mqttclient, ldata, "Milk_porridge", 0, 1, 1);
	break;
	case 9:
	esp_mqtt_client_publish(mqttclient, ldata, "Yogurt", 0, 1, 1);
	break;
	case 10:
	esp_mqtt_client_publish(mqttclient, ldata, "Express", 0, 1, 1);
	break;
	}
	} else if ( DEV_TYPA == 24 ) {
// for RO-5707
	switch (bProgA) {
	case 255:
	esp_mqtt_client_publish(mqttclient, ldata, "OFF", 0, 1, 1);
	break;
	case 0:
	esp_mqtt_client_publish(mqttclient, ldata, "Multicooker", 0, 1, 1);
	break;
	case 1:
	esp_mqtt_client_publish(mqttclient, ldata, "Omelet", 0, 1, 1);
	break;
	case 2:
	esp_mqtt_client_publish(mqttclient, ldata, "Slow_cooking_meat", 0, 1, 1);
	break;
	case 3:
	esp_mqtt_client_publish(mqttclient, ldata, "Slow_cooking_bird", 0, 1, 1);
	break;
	case 4:
	esp_mqtt_client_publish(mqttclient, ldata, "Slow_cooking_fish", 0, 1, 1);
	break;
	case 5:
	esp_mqtt_client_publish(mqttclient, ldata, "Slow_cooking_vegetables", 0, 1, 1);
	break;
	case 6:
	esp_mqtt_client_publish(mqttclient, ldata, "Bread", 0, 1, 1);
	break;
	case 7:
	esp_mqtt_client_publish(mqttclient, ldata, "Pizza", 0, 1, 1);
	break;
	case 8:
	esp_mqtt_client_publish(mqttclient, ldata, "Charlotte", 0, 1, 1);
	break;
	case 9:
	esp_mqtt_client_publish(mqttclient, ldata, "Baking_meat_in_pot", 0, 1, 1);
	break;
	case 10:
	esp_mqtt_client_publish(mqttclient, ldata, "Baking_bird_in_pot", 0, 1, 1);
	break;
	case 11:
	esp_mqtt_client_publish(mqttclient, ldata, "Baking_fish_in_pot", 0, 1, 1);
	break;
	case 12:
	esp_mqtt_client_publish(mqttclient, ldata, "Baking_vegetables_in_pot", 0, 1, 1);
	break;
	case 13:
	esp_mqtt_client_publish(mqttclient, ldata, "Roast", 0, 1, 1);
	break;
	case 14:
	esp_mqtt_client_publish(mqttclient, ldata, "Cake", 0, 1, 1);
	break;
	case 15:
	esp_mqtt_client_publish(mqttclient, ldata, "Baking_meat", 0, 1, 1);
	break;
	case 16:
	esp_mqtt_client_publish(mqttclient, ldata, "Baking_bird", 0, 1, 1);
	break;
	case 17:
	esp_mqtt_client_publish(mqttclient, ldata, "Baking_fish", 0, 1, 1);
	break;
	case 18:
	esp_mqtt_client_publish(mqttclient, ldata, "Baking_vegetables", 0, 1, 1);
	break;
	case 19:
	esp_mqtt_client_publish(mqttclient, ldata, "Boiled_pork", 0, 1, 1);
	break;
	case 20:
	esp_mqtt_client_publish(mqttclient, ldata, "Warming", 0, 1, 1);
	break;
	}
	}
	r4sppcoma = 255;
	t_ppcoma_us = t_mqt_us;
//	bprevProgA = bProgA;
//	bprevModProgA = bModProgA;
	}
	if  (bprevProgA != bProgA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/prog");
	itoa(bProgA,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcoma = 255;
	t_ppcoma_us = t_mqt_us;
	bprevProgA = bProgA;
	}
	if  (bprevModProgA != bModProgA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/mode");
	itoa(bModProgA,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcoma = 255;
	t_ppcoma_us = t_mqt_us;
	bprevModProgA = bModProgA;
	}
	if  (bprevHtempA != bHtempA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/temp");
	itoa(bHtempA,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcoma = 255;
	t_ppcoma_us = t_mqt_us;
	bprevHtempA = bHtempA;
	}
	if  (bprevPHourA != bPHourA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/set_hour");
	itoa(bPHourA,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcoma = 255;
	t_ppcoma_us = t_mqt_us;
	bprevPHourA = bPHourA;
	}
	if  (bprevPMinA != bPMinA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/set_min");
	itoa(bPMinA,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcoma = 255;
	t_ppcoma_us = t_mqt_us;
	bprevPMinA = bPMinA;
	}
	if  (bprevCHourA != bCHourA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/hour");
	itoa(bCHourA,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevCHourA = bCHourA;
	}
	if  (bprevCMinA != bCMinA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/min");
	itoa(bCMinA,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevCMinA = bCMinA;
	}
	if  (bprevDHourA != bDHourA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/delay_hour");
	itoa(bDHourA,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevDHourA = bDHourA;
	}
	if  (bprevDMinA != bDMinA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/delay_min");
	itoa(bDMinA,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevDMinA = bDMinA;
	}
	if  (bprevAwarmA != bAwarmA) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrA);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/warm");
	if (!bAwarmA) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (bAwarmA == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcoma = 255;
	t_ppcoma_us = t_mqt_us;
	bprevAwarmA = bAwarmA;
	}

	}
}
}


void MqBState() {
	if (mqtdel) return;
	int64_t t_mqt_us = esp_timer_get_time();
	char ldata[64];
	char tmpvar[32]; 
	mBsStatus();
	if ((mqttConnected) && (tBLEAddrB[0]) && (DEV_TYPB)) {
	if ((cStatusB[0] != 0) && (strcmp(cStatusB,cprevStatusB) != 0)) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/json");
	esp_mqtt_client_publish(mqttclient, ldata, cStatusB, 0, 1, 1);
	strcpy(cprevStatusB,cStatusB);
	} 
	if  (iprevRssiB != iRssiB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	strcat(ldata,"/rssi");
        itoa(iRssiB,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	iprevRssiB = iRssiB;
	}
	if ((DEV_TYPB < 10) || (DEV_TYPB > 63)) {
	if  (bprevCtempB != bCtempB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/temp");
	itoa(bCtempB,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevCtempB = bCtempB;
	}
	if  (bprevHtempB != bHtempB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/heat_temp");
	itoa(bHtempB,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcomb = 255;
	t_ppcomb_us = t_mqt_us;
	if (bHtempB > 29) bLtempB = bHtempB;
	bprevHtempB = bHtempB;
	}
	if  (bprevStateB != bStateB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/boil");
	if (!bStateB) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (bStateB == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcomb = 255;
	t_ppcomb_us = t_mqt_us;
	bprevStateB = bStateB;
	}
	if  (bprevHeatB != bHeatB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/heat");
	if (!bHeatB) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (bHeatB == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/hstate");
	if (!bHeatB) esp_mqtt_client_publish(mqttclient, ldata, "off", 0, 1, 1);
        else if (bHeatB == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, "heat", 0, 1, 1);
	r4sppcomb = 255;
	t_ppcomb_us = t_mqt_us;
	bprevHeatB = bHeatB;
	}
	if (DEV_TYPB < 10) {
	if  (bprevStNlB != bStNlB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/nightlight");
	if (!bStNlB) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (bStateB == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcomb = 255;
	t_ppcomb_us = t_mqt_us;
	bprevStNlB = bStNlB;
	}
	if  ((PRgbRB != RgbRB) || (PRgbGB != RgbGB) || (PRgbBB != RgbBB)) {
	ldata[0] = 0;
        itoa(RgbRB,ldata,10);
	strcpy(tmpvar,ldata);
        itoa(RgbGB,ldata,10);
	strcat(tmpvar,",");
	strcat(tmpvar,ldata);
        itoa(RgbBB,ldata,10);
	strcat(tmpvar,",");
	strcat(tmpvar,ldata);
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/nightlight_rgb");
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcomb = 255;
	t_ppcomb_us = t_mqt_us;
//	PRgbRB = RgbRB;
//	PRgbGB = RgbGB;
//	PRgbBB = RgbBB;
	}
	if  (PRgbRB != RgbRB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/nightlight_red");
        itoa(RgbRB,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcomb = 255;
	t_ppcomb_us = t_mqt_us;
	PRgbRB = RgbRB;
	}
	if  (PRgbGB != RgbGB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/nightlight_green");
        itoa(RgbGB,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcomb = 255;
	t_ppcomb_us = t_mqt_us;
	PRgbGB = RgbGB;
	}
	if  (PRgbBB != RgbBB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/nightlight_blue");
        itoa(RgbBB,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcomb = 255;
	t_ppcomb_us = t_mqt_us;
	PRgbBB = RgbBB;
	}
	if  (bprevStBlB != bStBlB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/backlight");
	if (!bStBlB) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (bStBlB == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcomb = 255;
	t_ppcomb_us = t_mqt_us;
	bprevStBlB = bStBlB;
	}
	if  (bprevStBpB != bStBpB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/beep");
	if (!bStBpB) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (bStBpB == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcomb = 255;
	t_ppcomb_us = t_mqt_us;
	bprevStBpB = bStBpB;
	}
	if  (bSEnergyB != bprevSEnergyB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/total_energy");
        itoakw(bSEnergyB,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcomb = 255;
	t_ppcomb_us = t_mqt_us;
	bprevSEnergyB = bSEnergyB;
	}
	if  (bSTimeB != bprevSTimeB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/working_time");
        itoat(bSTimeB,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcomb = 255;
	t_ppcomb_us = t_mqt_us;
	bprevSTimeB = bSTimeB;
	}
	if  (bSCountB != bprevSCountB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/working_count");
        itoa(bSCountB,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcomb = 255;
	t_ppcomb_us = t_mqt_us;
	bprevSCountB = bSCountB;
	}
	}
	} else if ( DEV_TYPB < 12) {
	if  (bprevStateB != bStateB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/state");
	if (!bStateB) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (bStateB == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcomb = 255;
	t_ppcomb_us = t_mqt_us;
	bprevStateB = bStateB;
	}
	if  (bprevLockB != bLockB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/lock");
	if (!bLockB) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcomb = 255;
	t_ppcomb_us = t_mqt_us;
	bprevLockB = bLockB;
	}
	} else if ( DEV_TYPB < 16) {
	if  (bprevStateB != bStateB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/state");
	if (!bStateB) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (bStateB == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcomb = 255;
	t_ppcomb_us = t_mqt_us;
	bprevStateB = bStateB;
	}
	if  (bprevStNlB != bStNlB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/delay");
	if (!bStNlB) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (bStNlB == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcomb = 255;
	t_ppcomb_us = t_mqt_us;
	bprevStNlB = bStNlB;
	}
	if  (bprevLockB != bLockB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/lock");
	if (!bLockB) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcomb = 255;
	t_ppcomb_us = t_mqt_us;
	bprevLockB = bLockB;
	}
	if  (bprevProgB != bProgB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/strength");
	if (!bProgB) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcomb = 255;
	t_ppcomb_us = t_mqt_us;
	bprevProgB = bProgB;
	}
	if  (bprevPHourB != bPHourB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/set_hour");
	itoa(bPHourB,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevPHourB = bPHourB;
	}
	if  (bprevPMinB != bPMinB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/set_min");
	itoa(bPMinB,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevPMinB = bPMinB;
	}
	if  (bprevDHourB != bDHourB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/delay_hour");
	itoa(bDHourB,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevDHourB = bDHourB;
	}
	if  (bprevDMinB != bDMinB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/delay_min");
	itoa(bDMinB,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevDMinB = bDMinB;
	}
	if  (bprevCHourB != bCHourB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/hour");
	itoa(bCHourB,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevCHourB = bCHourB;
	}
	if  (bprevCMinB != bCMinB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/min");
	itoa(bCMinB,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevCMinB = bCMinB;
	}
	} else if (DEV_TYPB < 64) {
	if  (bprevStateB != bStateB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/state");
	if (!bStateB) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (bStateB == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else if (bStateB == 1) esp_mqtt_client_publish(mqttclient, ldata, "SETTING", 0, 1, 1);
        else if (bStateB == 2) esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
        else if (bStateB == 4) esp_mqtt_client_publish(mqttclient, ldata, "WARMING", 0, 1, 1);
        else if (bStateB == 5) esp_mqtt_client_publish(mqttclient, ldata, "DELAYEDSTART", 0, 1, 1);
        else if (bStateB == 6) esp_mqtt_client_publish(mqttclient, ldata, "WAITPASTA", 0, 1, 1);
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/hstate");
	if (bStateB < 2) esp_mqtt_client_publish(mqttclient, ldata, "off", 0, 1, 1);
        else if (bStateB == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, "heat", 0, 1, 1);
	r4sppcomb = 255;
	t_ppcomb_us = t_mqt_us;
	bprevStateB = bStateB;
	}
	if  ((bprevProgB != bProgB) || (bprevModProgB != bModProgB)){
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/prname");
        if ( DEV_TYPB == 16 ) {
// for RMC-800s
	switch (bProgB) {
	case 255:
	esp_mqtt_client_publish(mqttclient, ldata, "OFF", 0, 1, 1);
	break;
	case 0:
	esp_mqtt_client_publish(mqttclient, ldata, "Multicooker", 0, 1, 1);
	break;
	case 1:
	esp_mqtt_client_publish(mqttclient, ldata, "Rice", 0, 1, 1);
	break;
	case 2:
	esp_mqtt_client_publish(mqttclient, ldata, "Slow_cooking", 0, 1, 1);
	break;
	case 3:
	esp_mqtt_client_publish(mqttclient, ldata, "Pilaf", 0, 1, 1);
	break;
	case 4:
	if (bModProgB == 1) esp_mqtt_client_publish(mqttclient, ldata, "Frying_vegetables", 0, 1, 1);
	else if (bModProgB == 2) esp_mqtt_client_publish(mqttclient, ldata, "Frying_fish", 0, 1, 1);
	else if (bModProgB == 3) esp_mqtt_client_publish(mqttclient, ldata, "Frying_meat", 0, 1, 1);
	else esp_mqtt_client_publish(mqttclient, ldata, "Invalid mode", 0, 1, 1);
	break;
	case 5:
	if (bModProgB == 1) esp_mqtt_client_publish(mqttclient, ldata, "Stewing_vegetables", 0, 1, 1);
	else if (bModProgB == 2) esp_mqtt_client_publish(mqttclient, ldata, "Stewing_fish", 0, 1, 1);
	else if (bModProgB == 3) esp_mqtt_client_publish(mqttclient, ldata, "Stewing_meat", 0, 1, 1);
	else esp_mqtt_client_publish(mqttclient, ldata, "Invalid mode", 0, 1, 1);
	break;
	case 6:
	esp_mqtt_client_publish(mqttclient, ldata, "Pasta", 0, 1, 1);
	break;
	case 7:
	esp_mqtt_client_publish(mqttclient, ldata, "Milk_porridge", 0, 1, 1);
	break;
	case 8:
	esp_mqtt_client_publish(mqttclient, ldata, "Soup", 0, 1, 1);
	break;
	case 9:
	esp_mqtt_client_publish(mqttclient, ldata, "Yogurt", 0, 1, 1);
	break;
	case 10:
	esp_mqtt_client_publish(mqttclient, ldata, "Baking", 0, 1, 1);
	break;
	case 11:
	if (bModProgB == 1) esp_mqtt_client_publish(mqttclient, ldata, "Steam_vegetables", 0, 1, 1);
	else if (bModProgB == 2) esp_mqtt_client_publish(mqttclient, ldata, "Steam_fish", 0, 1, 1);
	else if (bModProgB == 3) esp_mqtt_client_publish(mqttclient, ldata, "Steam_meat", 0, 1, 1);
	else esp_mqtt_client_publish(mqttclient, ldata, "Invalid mode", 0, 1, 1);
	break;
	case 12:
	esp_mqtt_client_publish(mqttclient, ldata, "Hot", 0, 1, 1);
	break;
	}
	} else if ( DEV_TYPB == 17 ) {
// for RMC-903s
	switch (bProgB) {
	case 255:
	esp_mqtt_client_publish(mqttclient, ldata, "OFF", 0, 1, 1);
	break;
	case 0:
	esp_mqtt_client_publish(mqttclient, ldata, "Multicooker", 0, 1, 1);
	break;
	case 1:
	esp_mqtt_client_publish(mqttclient, ldata, "Milk_porridge", 0, 1, 1);
	break;
	case 2:
	esp_mqtt_client_publish(mqttclient, ldata, "Stewing", 0, 1, 1);
	break;
	case 3:
	esp_mqtt_client_publish(mqttclient, ldata, "Frying", 0, 1, 1);
	break;
	case 4:
	esp_mqtt_client_publish(mqttclient, ldata, "Soup", 0, 1, 1);
	break;
	case 5:
	esp_mqtt_client_publish(mqttclient, ldata, "Steam", 0, 1, 1);
	break;
	case 6:
	esp_mqtt_client_publish(mqttclient, ldata, "Pasta", 0, 1, 1);
	break;
	case 7:
	esp_mqtt_client_publish(mqttclient, ldata, "Slow_cooking", 0, 1, 1);
	break;
	case 8:
	esp_mqtt_client_publish(mqttclient, ldata, "Hot", 0, 1, 1);
	break;
	case 9:
	esp_mqtt_client_publish(mqttclient, ldata, "Baking", 0, 1, 1);
	break;
	case 10:
	esp_mqtt_client_publish(mqttclient, ldata, "Groats", 0, 1, 1);
	break;
	case 11:
	esp_mqtt_client_publish(mqttclient, ldata, "Pilaf", 0, 1, 1);
	break;
	case 12:
	esp_mqtt_client_publish(mqttclient, ldata, "Yogurt", 0, 1, 1);
	break;
	case 13:
	esp_mqtt_client_publish(mqttclient, ldata, "Pizza", 0, 1, 1);
	break;
	case 14:
	esp_mqtt_client_publish(mqttclient, ldata, "Bread", 0, 1, 1);
	break;
	case 15:
	esp_mqtt_client_publish(mqttclient, ldata, "Desserts", 0, 1, 1);
	break;
	case 16:
	esp_mqtt_client_publish(mqttclient, ldata, "Express", 0, 1, 1);
	break;
	}
	} else if ( DEV_TYPB == 18 ) {
// for RMC-224s
	switch (bProgB) {
	case 255:
	esp_mqtt_client_publish(mqttclient, ldata, "OFF", 0, 1, 1);
	break;
	case 0:
	esp_mqtt_client_publish(mqttclient, ldata, "Frying", 0, 1, 1);
	break;
	case 1:
	esp_mqtt_client_publish(mqttclient, ldata, "Groats", 0, 1, 1);
	break;
	case 2:
	esp_mqtt_client_publish(mqttclient, ldata, "Multicooker", 0, 1, 1);
	break;
	case 3:
	esp_mqtt_client_publish(mqttclient, ldata, "Pilaf", 0, 1, 1);
	break;
	case 4:
	esp_mqtt_client_publish(mqttclient, ldata, "Steam", 0, 1, 1);
	break;
	case 5:
	esp_mqtt_client_publish(mqttclient, ldata, "Baking", 0, 1, 1);
	break;
	case 6:
	esp_mqtt_client_publish(mqttclient, ldata, "Stewing", 0, 1, 1);
	break;
	case 7:
	esp_mqtt_client_publish(mqttclient, ldata, "Soup", 0, 1, 1);
	break;
	case 8:
	esp_mqtt_client_publish(mqttclient, ldata, "Milk_porridge", 0, 1, 1);
	break;
	case 9:
	esp_mqtt_client_publish(mqttclient, ldata, "Yogurt", 0, 1, 1);
	break;
	case 10:
	esp_mqtt_client_publish(mqttclient, ldata, "Express", 0, 1, 1);
	break;
	}
	} else if ( DEV_TYPB == 24 ) {
// for RO-5707
	switch (bProgB) {
	case 255:
	esp_mqtt_client_publish(mqttclient, ldata, "OFF", 0, 1, 1);
	break;
	case 0:
	esp_mqtt_client_publish(mqttclient, ldata, "Multicooker", 0, 1, 1);
	break;
	case 1:
	esp_mqtt_client_publish(mqttclient, ldata, "Omelet", 0, 1, 1);
	break;
	case 2:
	esp_mqtt_client_publish(mqttclient, ldata, "Slow_cooking_meat", 0, 1, 1);
	break;
	case 3:
	esp_mqtt_client_publish(mqttclient, ldata, "Slow_cooking_bird", 0, 1, 1);
	break;
	case 4:
	esp_mqtt_client_publish(mqttclient, ldata, "Slow_cooking_fish", 0, 1, 1);
	break;
	case 5:
	esp_mqtt_client_publish(mqttclient, ldata, "Slow_cooking_vegetables", 0, 1, 1);
	break;
	case 6:
	esp_mqtt_client_publish(mqttclient, ldata, "Bread", 0, 1, 1);
	break;
	case 7:
	esp_mqtt_client_publish(mqttclient, ldata, "Pizza", 0, 1, 1);
	break;
	case 8:
	esp_mqtt_client_publish(mqttclient, ldata, "Charlotte", 0, 1, 1);
	break;
	case 9:
	esp_mqtt_client_publish(mqttclient, ldata, "Baking_meat_in_pot", 0, 1, 1);
	break;
	case 10:
	esp_mqtt_client_publish(mqttclient, ldata, "Baking_bird_in_pot", 0, 1, 1);
	break;
	case 11:
	esp_mqtt_client_publish(mqttclient, ldata, "Baking_fish_in_pot", 0, 1, 1);
	break;
	case 12:
	esp_mqtt_client_publish(mqttclient, ldata, "Baking_vegetables_in_pot", 0, 1, 1);
	break;
	case 13:
	esp_mqtt_client_publish(mqttclient, ldata, "Roast", 0, 1, 1);
	break;
	case 14:
	esp_mqtt_client_publish(mqttclient, ldata, "Cake", 0, 1, 1);
	break;
	case 15:
	esp_mqtt_client_publish(mqttclient, ldata, "Baking_meat", 0, 1, 1);
	break;
	case 16:
	esp_mqtt_client_publish(mqttclient, ldata, "Baking_bird", 0, 1, 1);
	break;
	case 17:
	esp_mqtt_client_publish(mqttclient, ldata, "Baking_fish", 0, 1, 1);
	break;
	case 18:
	esp_mqtt_client_publish(mqttclient, ldata, "Baking_vegetables", 0, 1, 1);
	break;
	case 19:
	esp_mqtt_client_publish(mqttclient, ldata, "Boiled_pork", 0, 1, 1);
	break;
	case 20:
	esp_mqtt_client_publish(mqttclient, ldata, "Warming", 0, 1, 1);
	break;
	}
	}
	r4sppcomb = 255;
	t_ppcomb_us = t_mqt_us;
//	bprevProgB = bProgB;
//	bprevModProgB = bModProgB;
	}
	if  (bprevProgB != bProgB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/prog");
	itoa(bProgB,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcomb = 255;
	t_ppcomb_us = t_mqt_us;
	bprevProgB = bProgB;
	}
	if  (bprevModProgB != bModProgB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/mode");
	itoa(bModProgB,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcomb = 255;
	t_ppcomb_us = t_mqt_us;
	bprevModProgB = bModProgB;
	}
	if  (bprevHtempB != bHtempB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/temp");
	itoa(bHtempB,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcomb = 255;
	t_ppcomb_us = t_mqt_us;
	bprevHtempB = bHtempB;
	}
	if  (bprevPHourB != bPHourB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/set_hour");
	itoa(bPHourB,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcomb = 255;
	t_ppcomb_us = t_mqt_us;
	bprevPHourB = bPHourB;
	}
	if  (bprevPMinB != bPMinB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/set_min");
	itoa(bPMinB,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcomb = 255;
	t_ppcomb_us = t_mqt_us;
	bprevPMinB = bPMinB;
	}
	if  (bprevCHourB != bCHourB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/hour");
	itoa(bCHourB,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevCHourB = bCHourB;
	}
	if  (bprevCMinB != bCMinB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/min");
	itoa(bCMinB,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevCMinB = bCMinB;
	}
	if  (bprevDHourB != bDHourB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/delay_hour");
	itoa(bDHourB,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevDHourB = bDHourB;
	}
	if  (bprevDMinB != bDMinB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/delay_min");
	itoa(bDMinB,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevDMinB = bDMinB;
	}
	if  (bprevAwarmB != bAwarmB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrB);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/warm");
	if (!bAwarmB) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (bAwarmB == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcomb = 255;
	t_ppcomb_us = t_mqt_us;
	bprevAwarmB = bAwarmB;
	}

	}
}
}


void MqCState() {
	if (mqtdel) return;
	int64_t t_mqt_us = esp_timer_get_time();
	char ldata[64];
	char tmpvar[32]; 
	mCsStatus();
	if ((mqttConnected) && (tBLEAddrC[0]) && (DEV_TYPC)) {
	if ((cStatusC[0] != 0) && (strcmp(cStatusC,cprevStatusC) != 0)) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/json");
	esp_mqtt_client_publish(mqttclient, ldata, cStatusC, 0, 1, 1);
	strcpy(cprevStatusC,cStatusC);
	} 
	if  (iprevRssiC != iRssiC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	strcat(ldata,"/rssi");
        itoa(iRssiC,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	iprevRssiC = iRssiC;
	}
	if ((DEV_TYPC < 10) || (DEV_TYPC > 63)) {
	if  (bprevCtempC != bCtempC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/temp");
	itoa(bCtempC,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevCtempC = bCtempC;
	}
	if  (bprevHtempC != bHtempC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/heat_temp");
	itoa(bHtempC,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcomc = 255;
	t_ppcomc_us = t_mqt_us;
	if (bHtempC > 29) bLtempC = bHtempC;
	bprevHtempC = bHtempC;
	}
	if  (bprevStateC != bStateC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/boil");
	if (!bStateC) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (bStateC == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcomc = 255;
	t_ppcomc_us = t_mqt_us;
	bprevStateC = bStateC;
	}
	if  (bprevHeatC != bHeatC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/heat");
	if (!bHeatC) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (bHeatC == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/hstate");
	if (!bHeatC) esp_mqtt_client_publish(mqttclient, ldata, "off", 0, 1, 1);
        else if (bHeatC == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, "heat", 0, 1, 1);
	r4sppcomc = 255;
	t_ppcomc_us = t_mqt_us;
	bprevHeatC = bHeatC;
	}
	if (DEV_TYPC < 10) {
	if  (bprevStNlC != bStNlC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/nightlight");
	if (!bStNlC) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (bStateC == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcomc = 255;
	t_ppcomc_us = t_mqt_us;
	bprevStNlC = bStNlC;
	}
	if  ((PRgbRC != RgbRC) || (PRgbGC != RgbGC) || (PRgbBC != RgbBC)) {
	ldata[0] = 0;
        itoa(RgbRC,ldata,10);
	strcpy(tmpvar,ldata);
        itoa(RgbGC,ldata,10);
	strcat(tmpvar,",");
	strcat(tmpvar,ldata);
        itoa(RgbBC,ldata,10);
	strcat(tmpvar,",");
	strcat(tmpvar,ldata);
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/nightlight_rgb");
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcomc = 255;
	t_ppcomc_us = t_mqt_us;
//	PRgbRC = RgbRC;
//	PRgbGC = RgbGC;
//	PRgbBC = RgbBC;
	}
	if  (PRgbRC != RgbRC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/nightlight_red");
        itoa(RgbRC,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcomc = 255;
	t_ppcomc_us = t_mqt_us;
	PRgbRC = RgbRC;
	}
	if  (PRgbGC != RgbGC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/nightlight_green");
        itoa(RgbGC,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcomc = 255;
	t_ppcomc_us = t_mqt_us;
	PRgbGC = RgbGC;
	}
	if  (PRgbBC != RgbBC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/nightlight_blue");
        itoa(RgbBC,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcomc = 255;
	t_ppcomc_us = t_mqt_us;
	PRgbBC = RgbBC;
	}
	if  (bprevStBlC != bStBlC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/backlight");
	if (!bStBlC) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (bStBlC == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcomc = 255;
	t_ppcomc_us = t_mqt_us;
	bprevStBlC = bStBlC;
	}
	if  (bprevStBpC != bStBpC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/beep");
	if (!bStBpC) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (bStBpC == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcomc = 255;
	t_ppcomc_us = t_mqt_us;
	bprevStBpC = bStBpC;
	}
	if  (bSEnergyC != bprevSEnergyC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/total_energy");
        itoakw(bSEnergyC,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcomc = 255;
	t_ppcomc_us = t_mqt_us;
	bprevSEnergyC = bSEnergyC;
	}
	if  (bSTimeC != bprevSTimeC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/working_time");
        itoat(bSTimeC,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcomc = 255;
	t_ppcomc_us = t_mqt_us;
	bprevSTimeC = bSTimeC;
	}
	if  (bSCountC != bprevSCountC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/working_count");
        itoa(bSCountC,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcomc = 255;
	t_ppcomc_us = t_mqt_us;
	bprevSCountC = bSCountC;
	}
	}
	} else if ( DEV_TYPC < 12) {
	if  (bprevStateC != bStateC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/state");
	if (!bStateC) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (bStateC == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcomc = 255;
	t_ppcomc_us = t_mqt_us;
	bprevStateC = bStateC;
	}
	if  (bprevLockC != bLockC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/lock");
	if (!bLockC) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcomc = 255;
	t_ppcomc_us = t_mqt_us;
	bprevLockC = bLockC;
	}
	} else if ( DEV_TYPC < 16) {
	if  (bprevStateC != bStateC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/state");
	if (!bStateC) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (bStateC == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcomc = 255;
	t_ppcomc_us = t_mqt_us;
	bprevStateC = bStateC;
	}
	if  (bprevStNlC != bStNlC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/delay");
	if (!bStNlC) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (bStNlC == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcomc = 255;
	t_ppcomc_us = t_mqt_us;
	bprevStNlC = bStNlC;
	}
	if  (bprevLockC != bLockC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/lock");
	if (!bLockC) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcomc = 255;
	t_ppcomc_us = t_mqt_us;
	bprevLockC = bLockC;
	}
	if  (bprevProgC != bProgC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/strength");
	if (!bProgC) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcomc = 255;
	t_ppcomc_us = t_mqt_us;
	bprevProgC = bProgC;
	}
	if  (bprevPHourC != bPHourC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/set_hour");
	itoa(bPHourC,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevPHourC = bPHourC;
	}
	if  (bprevPMinC != bPMinC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/set_min");
	itoa(bPMinC,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevPMinC = bPMinC;
	}
	if  (bprevDHourC != bDHourC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/delay_hour");
	itoa(bDHourC,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevDHourC = bDHourC;
	}
	if  (bprevDMinC != bDMinC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/delay_min");
	itoa(bDMinC,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevDMinC = bDMinC;
	}
	if  (bprevCHourC != bCHourC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/hour");
	itoa(bCHourC,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevCHourC = bCHourC;
	}
	if  (bprevCMinC != bCMinC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/min");
	itoa(bCMinC,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevCMinC = bCMinC;
	}
	} else if (DEV_TYPC < 64) {
	if  (bprevStateC != bStateC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/state");
	if (!bStateC) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (bStateC == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else if (bStateC == 1) esp_mqtt_client_publish(mqttclient, ldata, "SETTING", 0, 1, 1);
        else if (bStateC == 2) esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
        else if (bStateC == 4) esp_mqtt_client_publish(mqttclient, ldata, "WARMING", 0, 1, 1);
        else if (bStateC == 5) esp_mqtt_client_publish(mqttclient, ldata, "DELAYEDSTART", 0, 1, 1);
        else if (bStateC == 6) esp_mqtt_client_publish(mqttclient, ldata, "WAITPASTA", 0, 1, 1);
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/hstate");
	if (bStateC < 2) esp_mqtt_client_publish(mqttclient, ldata, "off", 0, 1, 1);
        else if (bStateC == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, "heat", 0, 1, 1);
	r4sppcomc = 255;
	t_ppcomc_us = t_mqt_us;
	bprevStateC = bStateC;
	}
	if  ((bprevProgC != bProgC) || (bprevModProgC != bModProgC)){
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/prname");
        if ( DEV_TYPC == 16 ) {
// for RMC-800s
	switch (bProgC) {
	case 255:
	esp_mqtt_client_publish(mqttclient, ldata, "OFF", 0, 1, 1);
	break;
	case 0:
	esp_mqtt_client_publish(mqttclient, ldata, "Multicooker", 0, 1, 1);
	break;
	case 1:
	esp_mqtt_client_publish(mqttclient, ldata, "Rice", 0, 1, 1);
	break;
	case 2:
	esp_mqtt_client_publish(mqttclient, ldata, "Slow_cooking", 0, 1, 1);
	break;
	case 3:
	esp_mqtt_client_publish(mqttclient, ldata, "Pilaf", 0, 1, 1);
	break;
	case 4:
	if (bModProgC == 1) esp_mqtt_client_publish(mqttclient, ldata, "Frying_vegetables", 0, 1, 1);
	else if (bModProgC == 2) esp_mqtt_client_publish(mqttclient, ldata, "Frying_fish", 0, 1, 1);
	else if (bModProgC == 3) esp_mqtt_client_publish(mqttclient, ldata, "Frying_meat", 0, 1, 1);
	else esp_mqtt_client_publish(mqttclient, ldata, "Invalid mode", 0, 1, 1);
	break;
	case 5:
	if (bModProgC == 1) esp_mqtt_client_publish(mqttclient, ldata, "Stewing_vegetables", 0, 1, 1);
	else if (bModProgC == 2) esp_mqtt_client_publish(mqttclient, ldata, "Stewing_fish", 0, 1, 1);
	else if (bModProgC == 3) esp_mqtt_client_publish(mqttclient, ldata, "Stewing_meat", 0, 1, 1);
	else esp_mqtt_client_publish(mqttclient, ldata, "Invalid mode", 0, 1, 1);
	break;
	case 6:
	esp_mqtt_client_publish(mqttclient, ldata, "Pasta", 0, 1, 1);
	break;
	case 7:
	esp_mqtt_client_publish(mqttclient, ldata, "Milk_porridge", 0, 1, 1);
	break;
	case 8:
	esp_mqtt_client_publish(mqttclient, ldata, "Soup", 0, 1, 1);
	break;
	case 9:
	esp_mqtt_client_publish(mqttclient, ldata, "Yogurt", 0, 1, 1);
	break;
	case 10:
	esp_mqtt_client_publish(mqttclient, ldata, "Baking", 0, 1, 1);
	break;
	case 11:
	if (bModProgC == 1) esp_mqtt_client_publish(mqttclient, ldata, "Steam_vegetables", 0, 1, 1);
	else if (bModProgC == 2) esp_mqtt_client_publish(mqttclient, ldata, "Steam_fish", 0, 1, 1);
	else if (bModProgC == 3) esp_mqtt_client_publish(mqttclient, ldata, "Steam_meat", 0, 1, 1);
	else esp_mqtt_client_publish(mqttclient, ldata, "Invalid mode", 0, 1, 1);
	break;
	case 12:
	esp_mqtt_client_publish(mqttclient, ldata, "Hot", 0, 1, 1);
	break;
	}
	} else if ( DEV_TYPC == 17 ) {
// for RMC-903s
	switch (bProgC) {
	case 255:
	esp_mqtt_client_publish(mqttclient, ldata, "OFF", 0, 1, 1);
	break;
	case 0:
	esp_mqtt_client_publish(mqttclient, ldata, "Multicooker", 0, 1, 1);
	break;
	case 1:
	esp_mqtt_client_publish(mqttclient, ldata, "Milk_porridge", 0, 1, 1);
	break;
	case 2:
	esp_mqtt_client_publish(mqttclient, ldata, "Stewing", 0, 1, 1);
	break;
	case 3:
	esp_mqtt_client_publish(mqttclient, ldata, "Frying", 0, 1, 1);
	break;
	case 4:
	esp_mqtt_client_publish(mqttclient, ldata, "Soup", 0, 1, 1);
	break;
	case 5:
	esp_mqtt_client_publish(mqttclient, ldata, "Steam", 0, 1, 1);
	break;
	case 6:
	esp_mqtt_client_publish(mqttclient, ldata, "Pasta", 0, 1, 1);
	break;
	case 7:
	esp_mqtt_client_publish(mqttclient, ldata, "Slow_cooking", 0, 1, 1);
	break;
	case 8:
	esp_mqtt_client_publish(mqttclient, ldata, "Hot", 0, 1, 1);
	break;
	case 9:
	esp_mqtt_client_publish(mqttclient, ldata, "Baking", 0, 1, 1);
	break;
	case 10:
	esp_mqtt_client_publish(mqttclient, ldata, "Groats", 0, 1, 1);
	break;
	case 11:
	esp_mqtt_client_publish(mqttclient, ldata, "Pilaf", 0, 1, 1);
	break;
	case 12:
	esp_mqtt_client_publish(mqttclient, ldata, "Yogurt", 0, 1, 1);
	break;
	case 13:
	esp_mqtt_client_publish(mqttclient, ldata, "Pizza", 0, 1, 1);
	break;
	case 14:
	esp_mqtt_client_publish(mqttclient, ldata, "Bread", 0, 1, 1);
	break;
	case 15:
	esp_mqtt_client_publish(mqttclient, ldata, "Desserts", 0, 1, 1);
	break;
	case 16:
	esp_mqtt_client_publish(mqttclient, ldata, "Express", 0, 1, 1);
	break;
	}
	} else if ( DEV_TYPC == 18 ) {
// for RMC-224s
	switch (bProgC) {
	case 255:
	esp_mqtt_client_publish(mqttclient, ldata, "OFF", 0, 1, 1);
	break;
	case 0:
	esp_mqtt_client_publish(mqttclient, ldata, "Frying", 0, 1, 1);
	break;
	case 1:
	esp_mqtt_client_publish(mqttclient, ldata, "Groats", 0, 1, 1);
	break;
	case 2:
	esp_mqtt_client_publish(mqttclient, ldata, "Multicooker", 0, 1, 1);
	break;
	case 3:
	esp_mqtt_client_publish(mqttclient, ldata, "Pilaf", 0, 1, 1);
	break;
	case 4:
	esp_mqtt_client_publish(mqttclient, ldata, "Steam", 0, 1, 1);
	break;
	case 5:
	esp_mqtt_client_publish(mqttclient, ldata, "Baking", 0, 1, 1);
	break;
	case 6:
	esp_mqtt_client_publish(mqttclient, ldata, "Stewing", 0, 1, 1);
	break;
	case 7:
	esp_mqtt_client_publish(mqttclient, ldata, "Soup", 0, 1, 1);
	break;
	case 8:
	esp_mqtt_client_publish(mqttclient, ldata, "Milk_porridge", 0, 1, 1);
	break;
	case 9:
	esp_mqtt_client_publish(mqttclient, ldata, "Yogurt", 0, 1, 1);
	break;
	case 10:
	esp_mqtt_client_publish(mqttclient, ldata, "Express", 0, 1, 1);
	break;
	}
	} else if ( DEV_TYPC == 24 ) {
// for RO-5707
	switch (bProgC) {
	case 255:
	esp_mqtt_client_publish(mqttclient, ldata, "OFF", 0, 1, 1);
	break;
	case 0:
	esp_mqtt_client_publish(mqttclient, ldata, "Multicooker", 0, 1, 1);
	break;
	case 1:
	esp_mqtt_client_publish(mqttclient, ldata, "Omelet", 0, 1, 1);
	break;
	case 2:
	esp_mqtt_client_publish(mqttclient, ldata, "Slow_cooking_meat", 0, 1, 1);
	break;
	case 3:
	esp_mqtt_client_publish(mqttclient, ldata, "Slow_cooking_bird", 0, 1, 1);
	break;
	case 4:
	esp_mqtt_client_publish(mqttclient, ldata, "Slow_cooking_fish", 0, 1, 1);
	break;
	case 5:
	esp_mqtt_client_publish(mqttclient, ldata, "Slow_cooking_vegetables", 0, 1, 1);
	break;
	case 6:
	esp_mqtt_client_publish(mqttclient, ldata, "Bread", 0, 1, 1);
	break;
	case 7:
	esp_mqtt_client_publish(mqttclient, ldata, "Pizza", 0, 1, 1);
	break;
	case 8:
	esp_mqtt_client_publish(mqttclient, ldata, "Charlotte", 0, 1, 1);
	break;
	case 9:
	esp_mqtt_client_publish(mqttclient, ldata, "Baking_meat_in_pot", 0, 1, 1);
	break;
	case 10:
	esp_mqtt_client_publish(mqttclient, ldata, "Baking_bird_in_pot", 0, 1, 1);
	break;
	case 11:
	esp_mqtt_client_publish(mqttclient, ldata, "Baking_fish_in_pot", 0, 1, 1);
	break;
	case 12:
	esp_mqtt_client_publish(mqttclient, ldata, "Baking_vegetables_in_pot", 0, 1, 1);
	break;
	case 13:
	esp_mqtt_client_publish(mqttclient, ldata, "Roast", 0, 1, 1);
	break;
	case 14:
	esp_mqtt_client_publish(mqttclient, ldata, "Cake", 0, 1, 1);
	break;
	case 15:
	esp_mqtt_client_publish(mqttclient, ldata, "Baking_meat", 0, 1, 1);
	break;
	case 16:
	esp_mqtt_client_publish(mqttclient, ldata, "Baking_bird", 0, 1, 1);
	break;
	case 17:
	esp_mqtt_client_publish(mqttclient, ldata, "Baking_fish", 0, 1, 1);
	break;
	case 18:
	esp_mqtt_client_publish(mqttclient, ldata, "Baking_vegetables", 0, 1, 1);
	break;
	case 19:
	esp_mqtt_client_publish(mqttclient, ldata, "Boiled_pork", 0, 1, 1);
	break;
	case 20:
	esp_mqtt_client_publish(mqttclient, ldata, "Warming", 0, 1, 1);
	break;
	}
	}
	r4sppcomc = 255;
	t_ppcomc_us = t_mqt_us;
//	bprevProgC = bProgC;
//	bprevModProgC = bModProgC;
	}
	if  (bprevProgC != bProgC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/prog");
	itoa(bProgC,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcomc = 255;
	t_ppcomc_us = t_mqt_us;
	bprevProgC = bProgC;
	}
	if  (bprevModProgC != bModProgC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/mode");
	itoa(bModProgC,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcomc = 255;
	t_ppcomc_us = t_mqt_us;
	bprevModProgC = bModProgC;
	}
	if  (bprevHtempC != bHtempC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/temp");
	itoa(bHtempC,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcomc = 255;
	t_ppcomc_us = t_mqt_us;
	bprevHtempC = bHtempC;
	}
	if  (bprevPHourC != bPHourC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/set_hour");
	itoa(bPHourC,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcomc = 255;
	t_ppcomc_us = t_mqt_us;
	bprevPHourC = bPHourC;
	}
	if  (bprevPMinC != bPMinC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/set_min");
	itoa(bPMinC,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcomc = 255;
	t_ppcomc_us = t_mqt_us;
	bprevPMinC = bPMinC;
	}
	if  (bprevCHourC != bCHourC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/hour");
	itoa(bCHourC,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevCHourC = bCHourC;
	}
	if  (bprevCMinC != bCMinC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/min");
	itoa(bCMinC,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevCMinC = bCMinC;
	}
	if  (bprevDHourC != bDHourC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/delay_hour");
	itoa(bDHourC,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevDHourC = bDHourC;
	}
	if  (bprevDMinC != bDMinC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/delay_min");
	itoa(bDMinC,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevDMinC = bDMinC;
	}
	if  (bprevAwarmC != bAwarmC) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddrC);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/warm");
	if (!bAwarmC) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (bAwarmC == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcomc = 255;
	t_ppcomc_us = t_mqt_us;
	bprevAwarmC = bAwarmC;
	}

	}
}
}




//******************* Mqtt **********************
static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
//    int msg_id;
    // your_context_t *context = event->context;
	int64_t t_mqt_us = 0;
//uint32_t fdeltmp = 0;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
	ESP_LOGI(AP_TAG,"MQTT_EVENT_CONNECTED");
	t_mqt_us = esp_timer_get_time();
	t_ppcona_us = t_mqt_us;
	t_ppconb_us = t_mqt_us;
	t_ppconc_us = t_mqt_us;
	t_lasta_us = ~t_lasta_us;
	t_lastb_us = ~t_lastb_us;
	t_lastc_us = ~t_lastc_us;
	iprevRssiA = 0;
	iprevRssiB = 0;
	iprevRssiC = 0;
	iprevRssiESP = 0;

	cprevStatusA[0] = 0;
	cprevStatusB[0] = 0;
	cprevStatusC[0] = 0;
	bprevSEnergyA = ~bSEnergyA;
	bprevSEnergyB = ~bSEnergyB;
	bprevSEnergyC = ~bSEnergyC;
	bprevSTimeA = ~bSTimeA;
	bprevSTimeB = ~bSTimeB;
	bprevSTimeC = ~bSTimeC;
	bprevSCountA = ~bSCountA;
	bprevSCountB = ~bSCountB;
	bprevSCountC = ~bSCountC;

	bprevLockA = 255;
	bprevStateA = 255;
	bprevHeatA = 255;
	bprevStNlA = 255;
	bprevCtempA = 255;
	bprevHtempA = 255;
	bprevProgA = 254;
	bprevModProgA = 255;
	bprevPHourA = 255;
	bprevPMinA = 255;
	bprevCHourA = 255;
	bprevCMinA = 255;
	bprevDHourA = 255;
	bprevDMinA = 255;
	bprevAwarmA = 255;
	PRgbRA = ~RgbRA;
	PRgbGA = ~RgbGA;
	PRgbBA = ~RgbBA;
        bprevStBlA = 255;
	bprevStBpA = 255;
	bprevLockB = 255;
	bprevStateB = 255;
	bprevHeatB = 255;
	bprevStNlB = 255;
	bprevCtempB = 255;
	bprevHtempB = 255;
	bprevProgB = 254;
	bprevModProgB = 255;
	bprevPHourB = 255;
	bprevPMinB = 255;
	bprevCHourB = 255;
	bprevCMinB = 255;
	bprevDHourB = 255;
	bprevDMinB = 255;
	bprevAwarmB = 255;
	PRgbRB = ~RgbRB;
	PRgbGB = ~RgbGB;
	PRgbBB = ~RgbBB;
        bprevStBlB = 255;
	bprevStBpB = 255;
	bprevLockC = 255;
	bprevStateC = 255;
	bprevHeatC = 255;
	bprevStNlC = 255;
	bprevCtempC = 255;
	bprevHtempC = 255;
	bprevProgC = 254;
	bprevModProgC = 255;
	bprevPHourC = 255;
	bprevPMinC = 255;
	bprevCHourC = 255;
	bprevCMinC = 255;
	bprevDHourC = 255;
	bprevDMinC = 255;
	bprevAwarmC = 255;
	PRgbRC = ~RgbRC;
	PRgbGC = ~RgbGC;
	PRgbBC = ~RgbBC;
        bprevStBlC = 255;
	bprevStBpC = 255;

	bprevStateS = 255;

	fgpio1 = 1;
	fgpio2 = 1;
	fgpio3 = 1;
	fgpio4 = 1;
	fgpio5 = 1;
	char llwtt[64];
	char llwtd[512];
	strcpy(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/status");
	esp_mqtt_client_publish(client, llwtt, "online", 0, 1, 1);
//	msg_id = esp_mqtt_client_publish(client, llwtt, "online", 0, 1, 1);
//	ESP_LOGI(AP_TAG,"sent publish successful, msg_id=%d", msg_id);
#ifdef USE_TFT
	if (MQTT_TOPP1[0]) esp_mqtt_client_subscribe(client, MQTT_TOPP1, 0);
	if (MQTT_TOPP2[0]) esp_mqtt_client_subscribe(client, MQTT_TOPP2, 0);
	if (MQTT_TOPP3[0]) esp_mqtt_client_subscribe(client, MQTT_TOPP3, 0);
	if (MQTT_TOPP4[0]) esp_mqtt_client_subscribe(client, MQTT_TOPP4, 0);
	if (MQTT_TOPP5[0]) esp_mqtt_client_subscribe(client, MQTT_TOPP5, 0);
	if (MQTT_TOPP6[0]) esp_mqtt_client_subscribe(client, MQTT_TOPP6, 0);
	if (MQTT_TOPP7[0]) esp_mqtt_client_subscribe(client, MQTT_TOPP7, 0);
#endif
	if (FDHass && tESP32Addr[0]) {
	tcpip_adapter_ip_info_t ipInfo;
	char wbuff[256];
	memset(wbuff,0,32);
	tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo);
	sprintf(wbuff, "%d.%d.%d.%d", IP2STR(&ipInfo.ip));
	strcpy(llwtt,"homeassistant/sensor/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/1x");
	strcat(llwtt,tESP32Addr);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate.rssi\",\"icon\":\"mdi:wifi\",\"uniq_id\":\"rssi_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\",\"device\":{\"identifiers\":[\"ESP32_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\"],\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate\",\"model\":\"ESP32\",\"sw_version\":\"");
	strcat(llwtd,AP_VER);
	if (wbuff[0]) {
	strcat(llwtd,"\",\"configuration_url\":\"http://");
	strcat(llwtd,wbuff);
	}
	strcat(llwtd,"\",\"manufacturer\":\"Espressif\"},\"device_class\":\"signal_strength\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/rssi\",\"unit_of_meas\":\"dBm\"");
	strcat(llwtd,",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);

	strcpy(llwtt,"homeassistant/switch/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/2x");
	strcat(llwtt,tESP32Addr);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate.screen\",\"icon\":\"mdi:laptop\",\"uniq_id\":\"screen_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\",\"device\":{\"identifiers\":[\"ESP32_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\"],\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate\",\"model\":\"ESP32\",\"sw_version\":\"");
	strcat(llwtd,AP_VER);
	if (wbuff[0]) {
	strcat(llwtd,"\",\"configuration_url\":\"http://");
	strcat(llwtd,wbuff);
	}
	strcat(llwtd,"\",\"manufacturer\":\"Espressif\"},\"command_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/screen\",\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/screen\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
//
	if ((bgpio1 > 63) && (bgpio1 < 98)) {
	strcpy(llwtt,"homeassistant/switch/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/3x");
	strcat(llwtt,tESP32Addr);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate.gpio1\",\"icon\":\"mdi:electric-switch\",\"uniq_id\":\"gpio1_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\",\"device\":{\"identifiers\":[\"ESP32_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\"],\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate\",\"model\":\"ESP32\",\"sw_version\":\"");
	strcat(llwtd,AP_VER);
	if (wbuff[0]) {
	strcat(llwtd,"\",\"configuration_url\":\"http://");
	strcat(llwtd,wbuff);
	}
	strcat(llwtd,"\",\"manufacturer\":\"Espressif\"},\"command_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/gpio1\",\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/gpio1\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	} else if ((bgpio1 > 127) && (bgpio1 < 192)) {
	strcpy(llwtt,"homeassistant/binary_sensor/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/3x");
	strcat(llwtt,tESP32Addr);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate.gpio1\",\"icon\":\"mdi:electric-switch\",\"uniq_id\":\"gpio1_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\",\"device\":{\"identifiers\":[\"ESP32_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\"],\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate\",\"model\":\"ESP32\",\"sw_version\":\"");
	strcat(llwtd,AP_VER);
	if (wbuff[0]) {
	strcat(llwtd,"\",\"configuration_url\":\"http://");
	strcat(llwtd,wbuff);
	}
	strcat(llwtd,"\",\"manufacturer\":\"Espressif\"},\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/gpio1\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	}
//
	if ((bgpio2 > 63) && (bgpio2 < 98)) {
	strcpy(llwtt,"homeassistant/switch/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/4x");
	strcat(llwtt,tESP32Addr);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate.gpio2\",\"icon\":\"mdi:electric-switch\",\"uniq_id\":\"gpio2_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\",\"device\":{\"identifiers\":[\"ESP32_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\"],\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate\",\"model\":\"ESP32\",\"sw_version\":\"");
	strcat(llwtd,AP_VER);
	if (wbuff[0]) {
	strcat(llwtd,"\",\"configuration_url\":\"http://");
	strcat(llwtd,wbuff);
	}
	strcat(llwtd,"\",\"manufacturer\":\"Espressif\"},\"command_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/gpio2\",\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/gpio2\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	} else if ((bgpio2 > 127) && (bgpio2 < 192)) {
	strcpy(llwtt,"homeassistant/binary_sensor/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/4x");
	strcat(llwtt,tESP32Addr);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate.gpio2\",\"icon\":\"mdi:electric-switch\",\"uniq_id\":\"gpio2_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\",\"device\":{\"identifiers\":[\"ESP32_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\"],\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate\",\"model\":\"ESP32\",\"sw_version\":\"");
	strcat(llwtd,AP_VER);
	if (wbuff[0]) {
	strcat(llwtd,"\",\"configuration_url\":\"http://");
	strcat(llwtd,wbuff);
	}
	strcat(llwtd,"\",\"manufacturer\":\"Espressif\"},\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/gpio2\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	}
//
	if ((bgpio3 > 63) && (bgpio3 < 98)) {
	strcpy(llwtt,"homeassistant/switch/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/5x");
	strcat(llwtt,tESP32Addr);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate.gpio3\",\"icon\":\"mdi:electric-switch\",\"uniq_id\":\"gpio3_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\",\"device\":{\"identifiers\":[\"ESP32_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\"],\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate\",\"model\":\"ESP32\",\"sw_version\":\"");
	strcat(llwtd,AP_VER);
	if (wbuff[0]) {
	strcat(llwtd,"\",\"configuration_url\":\"http://");
	strcat(llwtd,wbuff);
	}
	strcat(llwtd,"\",\"manufacturer\":\"Espressif\"},\"command_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/gpio3\",\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/gpio3\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	} else if ((bgpio3 > 127) && (bgpio3 < 192)) {
	strcpy(llwtt,"homeassistant/binary_sensor/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/5x");
	strcat(llwtt,tESP32Addr);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate.gpio3\",\"icon\":\"mdi:electric-switch\",\"uniq_id\":\"gpio3_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\",\"device\":{\"identifiers\":[\"ESP32_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\"],\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate\",\"model\":\"ESP32\",\"sw_version\":\"");
	strcat(llwtd,AP_VER);
	if (wbuff[0]) {
	strcat(llwtd,"\",\"configuration_url\":\"http://");
	strcat(llwtd,wbuff);
	}
	strcat(llwtd,"\",\"manufacturer\":\"Espressif\"},\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/gpio3\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	}
//
	if ((bgpio4 > 63) && (bgpio4 < 98)) {
	strcpy(llwtt,"homeassistant/switch/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/6x");
	strcat(llwtt,tESP32Addr);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate.gpio4\",\"icon\":\"mdi:electric-switch\",\"uniq_id\":\"gpio4_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\",\"device\":{\"identifiers\":[\"ESP32_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\"],\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate\",\"model\":\"ESP32\",\"sw_version\":\"");
	strcat(llwtd,AP_VER);
	if (wbuff[0]) {
	strcat(llwtd,"\",\"configuration_url\":\"http://");
	strcat(llwtd,wbuff);
	}
	strcat(llwtd,"\",\"manufacturer\":\"Espressif\"},\"command_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/gpio4\",\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/gpio4\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	} else if ((bgpio4 > 127) && (bgpio4 < 192)) {
	strcpy(llwtt,"homeassistant/binary_sensor/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/6x");
	strcat(llwtt,tESP32Addr);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate.gpio4\",\"icon\":\"mdi:electric-switch\",\"uniq_id\":\"gpio4_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\",\"device\":{\"identifiers\":[\"ESP32_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\"],\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate\",\"model\":\"ESP32\",\"sw_version\":\"");
	strcat(llwtd,AP_VER);
	if (wbuff[0]) {
	strcat(llwtd,"\",\"configuration_url\":\"http://");
	strcat(llwtd,wbuff);
	}
	strcat(llwtd,"\",\"manufacturer\":\"Espressif\"},\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/gpio4\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	}
//
	if ((bgpio5 > 63) && (bgpio5 < 98)) {
	strcpy(llwtt,"homeassistant/switch/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/7x");
	strcat(llwtt,tESP32Addr);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate.gpio5\",\"icon\":\"mdi:electric-switch\",\"uniq_id\":\"gpio5_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\",\"device\":{\"identifiers\":[\"ESP32_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\"],\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate\",\"model\":\"ESP32\",\"sw_version\":\"");
	strcat(llwtd,AP_VER);
	if (wbuff[0]) {
	strcat(llwtd,"\",\"configuration_url\":\"http://");
	strcat(llwtd,wbuff);
	}
	strcat(llwtd,"\",\"manufacturer\":\"Espressif\"},\"command_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/gpio5\",\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/gpio5\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	} else if ((bgpio5 > 127) && (bgpio5 < 192)) {
	strcpy(llwtt,"homeassistant/binary_sensor/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/7x");
	strcat(llwtt,tESP32Addr);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate.gpio5\",\"icon\":\"mdi:electric-switch\",\"uniq_id\":\"gpio5_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\",\"device\":{\"identifiers\":[\"ESP32_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\"],\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate\",\"model\":\"ESP32\",\"sw_version\":\"");
	strcat(llwtd,AP_VER);
	if (wbuff[0]) {
	strcat(llwtd,"\",\"configuration_url\":\"http://");
	strcat(llwtd,wbuff);
	}
	strcat(llwtd,"\",\"manufacturer\":\"Espressif\"},\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/gpio5\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	}


	}
	strcpy(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/screen");
	esp_mqtt_client_subscribe(mqttclient, llwtt, 0);
	if (bgpio1 > 63) {
	strcpy(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/gpio1");
	esp_mqtt_client_subscribe(mqttclient, llwtt, 0);
	}
	if (bgpio2 > 63) {
	strcpy(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/gpio2");
	esp_mqtt_client_subscribe(mqttclient, llwtt, 0);
	}
	if (bgpio3 > 63) {
	strcpy(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/gpio3");
	esp_mqtt_client_subscribe(mqttclient, llwtt, 0);
	}
	if (bgpio4 > 63) {
	strcpy(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/gpio4");
	esp_mqtt_client_subscribe(mqttclient, llwtt, 0);
	}
	if (bgpio5 > 63) {
	strcpy(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/gpio5");
	esp_mqtt_client_subscribe(mqttclient, llwtt, 0);
	}
        MqttPubSubA(true);	
        MqttPubSubB(true);	
        MqttPubSubC(true);	
	mqttConnected = true;
	break;

	case MQTT_EVENT_DISCONNECTED:
	mqttConnected = false;
	t_mqt_us = esp_timer_get_time();
	t_ppcona_us = t_mqt_us;
	t_ppconb_us = t_mqt_us;
	t_ppconc_us = t_mqt_us;

	iprevRssiA = 0;
	iprevRssiB = 0;
	iprevRssiC = 0;

	cprevStatusA[0] = 0;
	cprevStatusB[0] = 0;
	cprevStatusC[0] = 0;

	bprevLockA = 255;
	bprevStateA = 255;
	bprevHeatA = 255;
	bprevStNlA = 255;
	bprevCtempA = 255;
	bprevHtempA = 255;
	bprevProgA = 254;
	bprevModProgA = 255;
	bprevPHourA = 255;
	bprevPMinA = 255;
	bprevCHourA = 255;
	bprevCMinA = 255;
	bprevDHourA = 255;
	bprevDMinA = 255;
	bprevAwarmA = 255;
	PRgbRA = ~RgbRA;
	PRgbGA = ~RgbGA;
	PRgbBA = ~RgbBA;
        bprevStBlA = 255;
	bprevStBpA = 255;
	bprevLockB = 255;
	bprevStateB = 255;
	bprevHeatB = 255;
	bprevStNlB = 255;
	bprevCtempB = 255;
	bprevHtempB = 255;
	bprevProgB = 254;
	bprevModProgB = 255;
	bprevPHourB = 255;
	bprevPMinB = 255;
	bprevCHourB = 255;
	bprevCMinB = 255;
	bprevDHourB = 255;
	bprevDMinB = 255;
	bprevAwarmB = 255;
	PRgbRB = ~RgbRB;
	PRgbGB = ~RgbGB;
	PRgbBB = ~RgbBB;
        bprevStBlB = 255;
	bprevStBpB = 255;
	bprevLockC = 255;
	bprevStateC = 255;
	bprevHeatC = 255;
	bprevStNlC = 255;
	bprevCtempC = 255;
	bprevHtempC = 255;
	bprevProgC = 254;
	bprevModProgC = 255;
	bprevPHourC = 255;
	bprevPMinC = 255;
	bprevCHourC = 255;
	bprevCMinC = 255;
	bprevDHourC = 255;
	bprevDMinC = 255;
	bprevAwarmC = 255;
	PRgbRC = ~RgbRC;
	PRgbGC = ~RgbGC;
	PRgbBC = ~RgbBC;
        bprevStBlC = 255;
	bprevStBpC = 255;

	ESP_LOGI(AP_TAG,"MQTT_EVENT_DISCONNECTED");
	break;

        case MQTT_EVENT_SUBSCRIBED:
//            ESP_LOGI(AP_TAG,"MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
	break;
	case MQTT_EVENT_UNSUBSCRIBED:
//            ESP_LOGI(AP_TAG,"MQTT_EVENT_UNSUBSCRIBED, msg_id=%d\n", event->msg_id);
	break;
	case MQTT_EVENT_PUBLISHED:
//            ESP_LOGI(AP_TAG,"MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
	break;
	case MQTT_EVENT_DATA:
//	ESP_LOGI(AP_TAG,"MQTT_EVENT_DATA");
//	ESP_LOGI(AP_TAG,"TOPIC=%.*s", event->topic_len, event->topic);
//	ESP_LOGI(AP_TAG,"DATA=%.*s", event->data_len, event->data);
       	if (mqtdel && (event->topic_len) && (event->topic_len < 128)) {
	char ttopic[128];
	memset(ttopic,0,128);
        memcpy(ttopic, event->topic, event->topic_len);
        mqtdel = 20;
	if (event->data_len) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 1);
	} else if ((event->data_len) && (event->data_len < 64) && (event->topic_len) && (event->topic_len < 64)) {
	t_mqt_us = esp_timer_get_time();
	int topoffa = 0;
	int topoffb = 0;
	int topoffc = 0;
	int topoffs = 0;
	char tbuff[64];
	char ttopic[64];
	memset(ttopic,0,64);
        memcpy(ttopic, event->topic, event->topic_len);
//	ttopic[event->topic_len] = 0;
//ESP_LOGI(AP_TAG,"mqtopic=%s", ttopic);
	if (r4sppcoma) {
        if ((t_mqt_us - t_ppcoma_us) > 3000000) r4sppcoma = 0;
//fdeltmp = t_mqt_us - t_ppcoma_us;
//ESP_LOGI(AP_TAG,"Mqtt_delay_A=%d", fdeltmp);
	}
	if (r4sppcomb) {
        if ((t_mqt_us - t_ppcomb_us) > 3000000) r4sppcomb = 0;
//fdeltmp = t_mqt_us - t_ppcomb_us;
//ESP_LOGI(AP_TAG,"Mqtt_delay_B=%d", fdeltmp);
	}
	if (r4sppcomc) {
        if ((t_mqt_us - t_ppcomc_us) > 3000000) r4sppcomc = 0;
//fdeltmp = t_mqt_us - t_ppcomc_us;
//ESP_LOGI(AP_TAG,"Mqtt_delay_C=%d", fdeltmp);
	}
	if (r4sppcoms) {
        if ((t_mqt_us - t_ppcoms_us) > 3000000) r4sppcoms = 0;
	}
	if (tBLEAddrA[0]) {
	strcpy(tbuff,MQTT_BASE_TOPIC);
	strcat(tbuff,"/");
	strcat(tbuff,tBLEAddrA);
	if (!fcommtp) strcat(tbuff,"/cmd");
	strcat(tbuff,"/");
	topoffa = parsoff(event->topic,tbuff, event->topic_len);
	}
	if (tBLEAddrB[0]) {
	strcpy(tbuff,MQTT_BASE_TOPIC);
	strcat(tbuff,"/");
	strcat(tbuff,tBLEAddrB);
	if (!fcommtp) strcat(tbuff,"/cmd");
	strcat(tbuff,"/");
	topoffb = parsoff(event->topic,tbuff, event->topic_len);
	}
	if (tBLEAddrC[0]) {
	strcpy(tbuff,MQTT_BASE_TOPIC);
	strcat(tbuff,"/");
	strcat(tbuff,tBLEAddrC);
	if (!fcommtp) strcat(tbuff,"/cmd");
	strcat(tbuff,"/");
	topoffc = parsoff(event->topic,tbuff, event->topic_len);
	}
	strcpy(tbuff,MQTT_BASE_TOPIC);
	strcat(tbuff,"/gpio");
	topoffs = parsoff(event->topic,tbuff, event->topic_len);
//
	strcpy(tbuff,MQTT_BASE_TOPIC);
	strcat(tbuff,"/screen");
	if (!memcmp(event->topic, tbuff, event->topic_len)) {

	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!bStateS) || (!r4sppcoms) || (inccmp(strON,event->data,event->data_len))) {
	if (tft_conn) {
	gpio_set_level(PIN_NUM_BCKL, 1);
	bStateS = 1;
	}
	bprevStateS = 255;
	t_lasts_us = ~t_lasts_us;
	}
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bStateS)  || (!r4sppcoms) || (inccmp(strOFF,event->data,event->data_len))) {
	if (tft_conn) {
	gpio_set_level(PIN_NUM_BCKL, 0);
	}
	bStateS = 0;
	bprevStateS = 255;
	t_lasts_us = ~t_lasts_us;
	}	
	} else if ((!incascmp("restart",event->data,event->data_len)) || (!incascmp("reset",event->data,event->data_len))
		|| (!incascmp("reboot",event->data,event->data_len))) {
	esp_restart();
	}

	} else if (topoffs) {
	if ((!memcmp(event->topic+topoffs, "1", event->topic_len-topoffs)) && (bgpio1 > 63)) {
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!lvgpio1) || (!r4sppcoms) || (inccmp(strON,event->data,event->data_len))) {
	if (bgpio1 < 98) {
	gpio_set_level((bgpio1 & 0x3f), 1);
	lvgpio1 = 1;
			}
	fgpio1 = 1;
	t_lasts_us = ~t_lasts_us;
		}
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((lvgpio1)  || (!r4sppcoms) || (inccmp(strOFF,event->data,event->data_len))) {
	if (bgpio1 < 98) {
	gpio_set_level((bgpio1 & 0x3f), 0);
	lvgpio1 = 0;
			}
	fgpio1 = 1;
	t_lasts_us = ~t_lasts_us;
		}	
	}
	} else if ((!memcmp(event->topic+topoffs, "2", event->topic_len-topoffs)) && (bgpio2 > 63)) {
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!lvgpio2) || (!r4sppcoms) || (inccmp(strON,event->data,event->data_len))) {
	if (bgpio2 < 98) {
	gpio_set_level((bgpio2 & 0x3f), 1);
	lvgpio2 = 1;
			}
	fgpio2 = 1;
	t_lasts_us = ~t_lasts_us;
		}
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((lvgpio2)  || (!r4sppcoms) || (inccmp(strOFF,event->data,event->data_len))) {
	if (bgpio2 < 98) {
	gpio_set_level((bgpio2 & 0x3f), 0);
	lvgpio2 = 0;
			}
	fgpio2 = 1;
	t_lasts_us = ~t_lasts_us;
		}	
	}
	} else if ((!memcmp(event->topic+topoffs, "3", event->topic_len-topoffs)) && (bgpio3 > 63)) {
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!lvgpio3) || (!r4sppcoms) || (inccmp(strON,event->data,event->data_len))) {
	if (bgpio3 < 98) {
	gpio_set_level((bgpio3 & 0x3f), 1);
	lvgpio3 = 1;
			}
	fgpio3 = 1;
	t_lasts_us = ~t_lasts_us;
		}
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((lvgpio3)  || (!r4sppcoms) || (inccmp(strOFF,event->data,event->data_len))) {
	if (bgpio3 < 98) {
	gpio_set_level((bgpio3 & 0x3f), 0);
	lvgpio3 = 0;
			}
	fgpio3 = 1;
	t_lasts_us = ~t_lasts_us;
		}	
	}
	} else if ((!memcmp(event->topic+topoffs, "4", event->topic_len-topoffs)) && (bgpio4 > 63)) {
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!lvgpio4) || (!r4sppcoms) || (inccmp(strON,event->data,event->data_len))) {
	if (bgpio4 < 98) {
	gpio_set_level((bgpio4 & 0x3f), 1);
	lvgpio4 = 1;
			}
	fgpio4 = 1;
	t_lasts_us = ~t_lasts_us;
		}
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((lvgpio4)  || (!r4sppcoms) || (inccmp(strOFF,event->data,event->data_len))) {
	if (bgpio4 < 98) {
	gpio_set_level((bgpio4 & 0x3f), 0);
	lvgpio4 = 0;
			}
	fgpio4 = 1;
	t_lasts_us = ~t_lasts_us;
		}	
	}
	} else if ((!memcmp(event->topic+topoffs, "5", event->topic_len-topoffs)) && (bgpio5 > 63)) {
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!lvgpio5) || (!r4sppcoms) || (inccmp(strON,event->data,event->data_len))) {
	if (bgpio5 < 98) {
	gpio_set_level((bgpio5 & 0x3f), 1);
	lvgpio5 = 1;
			}
	fgpio5 = 1;
	t_lasts_us = ~t_lasts_us;
		}
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((lvgpio5)  || (!r4sppcoms) || (inccmp(strOFF,event->data,event->data_len))) {
	if (bgpio5 < 98) {
	gpio_set_level((bgpio5 & 0x3f), 0);
	lvgpio5 = 0;
			}
	fgpio5 = 1;
	t_lasts_us = ~t_lasts_us;
		}	
	}
	}
	} else if (topoffa && DEV_TYPA && ((t_mqt_us - t_ppcona_us) > 3000000)) {
//ESP_LOGI(AP_TAG,"topoffa=%d", topoffa);
	if (DEV_TYPA < 10) {	
	//kettle
	if (!memcmp(event->topic+topoffa, "boil", event->topic_len-topoffa)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!bStateA) || (!fcommtp) || (!r4sppcoma) || (inccmp(strON,event->data,event->data_len))) {
	if (bHtempA) {
	if (DEV_TYPA == 1) {	
	if (bHtempA < 30) r4slppar1a = 0;
	else if (bHtempA < 41) r4slppar1a = 1;
	else if (bHtempA < 56) r4slppar1a = 2;
	else if (bHtempA < 71) r4slppar1a = 3;
	else if (bHtempA < 86) r4slppar1a = 4;
	else r4slppar1a = 5;
	} else r4slppar1a = bHtempA;
	r4slpcoma = 4;
	} else {
	r4slppar1a = 0;
	r4slpcoma = 2;
		}
	}
//	ESP_LOGI(AP_TAG,"MQTT_BOIL_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bStateA)  || (!fcommtp) || (!r4sppcoma) || (inccmp(strOFF,event->data,event->data_len))) {
	if (bHtempA) {
	if (DEV_TYPA == 1) {	
	if (bHtempA < 30) r4slppar1a = 0;
	else if (bHtempA < 41) r4slppar1a = 1;
	else if (bHtempA < 56) r4slppar1a = 2;
	else if (bHtempA < 71) r4slppar1a = 3;
	else if (bHtempA < 86) r4slppar1a = 4;
	else r4slppar1a = 5;
	} else r4slppar1a = bHtempA;
	r4slppar2a = 1;
	r4slpcoma = 3;
	} else {
	r4slppar1a = 0;
	r4slpcoma = 1;
		}
	}	
//	ESP_LOGI(AP_TAG,"MQTT_BOIL_OFF");
		} 

	} else if (!memcmp(event->topic+topoffa, "heat", event->topic_len-topoffa)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len)) || (!incascmp("heat",event->data,event->data_len))) {
	if ((!bHeatA) || (!fcommtp) || (!r4sppcoma) || (inccmp(strON,event->data,event->data_len))) {
	uint8_t temp = bHtempA;
        if ((temp < 30) || (temp > 98)) temp = bLtempA;
        if ((temp < 30) || (temp > 98)) temp = 40;

	if (DEV_TYPA == 1) {	
	if (temp < 30) r4slppar1a = 0;
	else if (temp < 41) r4slppar1a = 1;
	else if (temp < 56) r4slppar1a = 2;
	else if (temp < 71) r4slppar1a = 3;
	else if (temp < 86) r4slppar1a = 4;
	else r4slppar1a = 5;
	} else r4slppar1a = temp;
	r4slppar2a = 1;
	r4slpcoma = 3;
	}
//	ESP_LOGI(AP_TAG,"MQTT_HEAT_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bHeatA)  || (!fcommtp) || (!r4sppcoma) || (inccmp(strOFF,event->data,event->data_len))) {
	r4slppar1a = 0;
	r4slpcoma = 1;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_HEAT_OFF");
		} 

	} else if (!memcmp(event->topic+topoffa, "heat_temp", event->topic_len-topoffa)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t temp = atoi(tbuff);
	if ((!bHtempA) && (bLtempA) && (temp < 30)) temp = bLtempA;
	if  ((!fcommtp) || (!r4sppcoma) || ((temp != bHtempA) && temp && bHtempA)) {
	if (DEV_TYPA == 1) {	
	if (temp < 30) r4slppar1a = 0;
	else if (temp < 41) r4slppar1a = 1;
	else if (temp < 56) r4slppar1a = 2;
	else if (temp < 71) r4slppar1a = 3;
	else if (temp < 86) r4slppar1a = 4;
	else r4slppar1a = 5;
	} else r4slppar1a = temp;
	r4slppar2a = 1;
	r4slpcoma = 3;
	} else if ((temp < 30) && ((!fcommtp) || (!r4sppcoma) || (temp != bHtempA))) {
	r4slppar1a = 0;
	r4slpcoma = 1;
	} else if ((temp > 98) && ((!fcommtp) || (!r4sppcoma) || (temp != bHtempA))) {
	r4slppar1a = 0;
	r4slpcoma = 1;
	}
//	ESP_LOGI(AP_TAG,"MQTT_HEAT_TEMP");

	} else if (!memcmp(event->topic+topoffa, "backlight", event->topic_len-topoffa)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!bStBlA) || (!fcommtp) || (!r4sppcoma) || (inccmp(strON,event->data,event->data_len))) {
	r4slppar1a = 0;
	r4slpcoma = 22;
	}
//	ESP_LOGI(AP_TAG,"MQTT_BACKLIGHT_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bStBlA) || (!fcommtp) || (!r4sppcoma) || (inccmp(strOFF,event->data,event->data_len))) {
	r4slppar1a = 0;
	r4slpcoma = 21;
	}
//	ESP_LOGI(AP_TAG,"MQTT_BACKLIGHT_OFF");
	}

	} else if (!memcmp(event->topic+topoffa, "beep", event->topic_len-topoffa)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!bStBpA) || (!fcommtp) || (!r4sppcoma) || (inccmp(strON,event->data,event->data_len))) {
	r4slppar1a = 0;
	r4slpcoma = 24;
	}
//	ESP_LOGI(AP_TAG,"MQTT_BEEP_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bStBpA) || (!fcommtp) || (!r4sppcoma) || (inccmp(strOFF,event->data,event->data_len))) {
	r4slppar1a = 0;
	r4slpcoma = 23;
	}
//	ESP_LOGI(AP_TAG,"MQTT_BEEP_OFF");
	}

	} else if (!memcmp(event->topic+topoffa, "nightlight", event->topic_len-topoffa)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!bStNlA) || (!fcommtp) || (!r4sppcoma) || (inccmp(strON,event->data,event->data_len))) {
	r4slppar1a = 0;
	r4slpcoma = 5;
	}
//	ESP_LOGI(AP_TAG,"MQTT_NIGHTLIGHT_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bStNlA) || (!fcommtp) || (!r4sppcoma) || (inccmp(strOFF,event->data,event->data_len))) {
	r4slppar1a = 0;
	r4slpcoma = 1;
	}
//	ESP_LOGI(AP_TAG,"MQTT_NIGHTLIGHT_OFF");
	}

	} else if (!memcmp(event->topic+topoffa, "nightlight_rgb", event->topic_len-topoffa)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t i = 0;
	uint8_t of1 = 0;
	uint8_t of2 = 0;
	while (i < 12) {
	if (tbuff[i] == 0x2c) {
	tbuff[i] = 0;
	if (!of1) of1 = i+1;
	else if (!of2) of2 = i+1;
	else i = 20;
	} else	if (tbuff[i] == 0x00) {
	if ((of1) && (of1 < of2) && (of2 < i)) i = 12;
	else i = 20; 	
	} else if ((tbuff[i] < 0x30) || (tbuff[i] > 0x39)) i = 20;
	i++;
	}
	if (i < 20) {
	uint8_t rval = atoi(tbuff);
	uint8_t gval = atoi(tbuff+of1);
	uint8_t bval = atoi(tbuff+of2);
	if ((!fcommtp) || (!r4sppcoma) || (RgbRA != rval) || (RgbGA != gval) || (RgbBA != bval)) {
	RgbRA = rval;
	RgbGA = gval;
	RgbBA = bval;
	if (bStNlA) {	
	r4slppar1a = 0;
	r4slpcoma = 5;
	}
	t_lasta_us = ~t_lasta_us;
	}
	}

	} else if (!memcmp(event->topic+topoffa, "nightlight_red", event->topic_len-topoffa)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t cval = atoi(tbuff);
	if ((!fcommtp) || (!r4sppcoma) || (RgbRA != cval)) {
	RgbRA = cval;
	if (bStNlA) {	
	r4slppar1a = 0;
	r4slpcoma = 5;
	}
	t_lasta_us = ~t_lasta_us;
	}
	} else if (!memcmp(event->topic+topoffa, "nightlight_green", event->topic_len-topoffa)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t cval = atoi(tbuff);
	if ((!fcommtp) || (!r4sppcoma) || (RgbGA != cval)) {
	RgbGA = cval;
	if (bStNlA) {	
	r4slppar1a = 0;
	r4slpcoma = 5;
	}
	t_lasta_us = ~t_lasta_us;
	}
	} else if (!memcmp(event->topic+topoffa, "nightlight_blue", event->topic_len-topoffa)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t cval = atoi(tbuff);
	if ((!fcommtp) || (!r4sppcoma) || (RgbBA != cval)) {
	RgbBA = cval;
	if (bStNlA) {	
	r4slppar1a = 0;
	r4slpcoma = 5;
	}
	t_lasta_us = ~t_lasta_us;
	}
	}
	} else if ( DEV_TYPA < 12) {
	//power
	if (!memcmp(event->topic+topoffa, "state", event->topic_len-topoffa)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!bStateA) || (!fcommtp) || (!r4sppcoma) || (inccmp(strON,event->data,event->data_len))) {
	r4slppar1a = 0;
	r4slpcoma = 6;
	}
//	ESP_LOGI(AP_TAG,"MQTT_CMD_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bStateA) || (!fcommtp) || (!r4sppcoma) || (inccmp(strOFF,event->data,event->data_len))) {
	r4slppar1a = 0;
	r4slpcoma = 1;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_CMD_OFF");
	}
	} else 	if (!memcmp(event->topic+topoffa, "lock", event->topic_len-topoffa)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!bLockA) || (!fcommtp) || (!r4sppcoma) || (inccmp(strON,event->data,event->data_len))) {	
	r4slppar1a = 0;
	r4slpcoma = 8;
	}
//	ESP_LOGI(AP_TAG,"MQTT_lock_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bLockA) || (!fcommtp) || (!r4sppcoma) || (inccmp(strOFF,event->data,event->data_len))) {	
	r4slppar1a = 0;
	r4slpcoma = 7;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_lock_OFF");
	}
	}
	} else if ( DEV_TYPA < 16) {
	//coffee
	if (!memcmp(event->topic+topoffa, "state", event->topic_len-topoffa)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!bStateA) || (!fcommtp) || (!r4sppcoma) || (inccmp(strON,event->data,event->data_len))) {
	r4slppar1a = 0;
	r4slpcoma = 6;
	}
//	ESP_LOGI(AP_TAG,"MQTT_CMD_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bStateA) || (!fcommtp) || (!r4sppcoma) || (inccmp(strOFF,event->data,event->data_len))) {
	r4slppar1a = 0;
	r4slpcoma = 1;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_CMD_OFF");
	}
	} else if (!memcmp(event->topic+topoffa, "delay", event->topic_len-topoffa)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!bStNlA) || (!fcommtp) || (!r4sppcoma) || (inccmp(strON,event->data,event->data_len))) {	
	r4slppar1a = bProgA;
	r4slppar2a = bDHourA;
	r4slppar3a = bDMinA;
	bDHourA = 0;
	bDMinA = 0;
	r4slpcoma = 18;
	}
//	ESP_LOGI(AP_TAG,"MQTT_delay_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bStNlA) || (!fcommtp) || (!r4sppcoma) || (inccmp(strOFF,event->data,event->data_len))) {	
	r4slppar1a = 0;
	r4slpcoma = 1;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_delay_OFF");
	}
	} else if (!memcmp(event->topic+topoffa, "delay_hour", event->topic_len-topoffa)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t hour = atoi(tbuff);
	if (hour < 24) {
	bDHourA = hour;
	t_lasta_us = ~t_lasta_us;
	}
	} else if (!memcmp(event->topic+topoffa, "delay_min", event->topic_len-topoffa)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t min = atoi(tbuff);
	if (min < 60) {
	bDMinA = min;
	t_lasta_us = ~t_lasta_us;
	}
	} else 	if (!memcmp(event->topic+topoffa, "lock", event->topic_len-topoffa)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!bLockA) || (!fcommtp) || (!r4sppcoma) || (inccmp(strON,event->data,event->data_len))) {	
	r4slppar1a = 0;
	r4slpcoma = 8;
	}
//	ESP_LOGI(AP_TAG,"MQTT_lock_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bLockA) || (!fcommtp) || (!r4sppcoma) || (inccmp(strOFF,event->data,event->data_len))) {	
	r4slppar1a = 0;
	r4slpcoma = 7;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_lock_OFF");
	}
	} else 	if (!memcmp(event->topic+topoffa, "strength", event->topic_len-topoffa)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!bProgA) || (!fcommtp) || (!r4sppcoma) || (inccmp(strON,event->data,event->data_len))) {	
	r4slppar1a = 1;
	r4slpcoma = 9;
	}
//	ESP_LOGI(AP_TAG,"MQTT_strength_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bProgA) || (!fcommtp) || (!r4sppcoma) || (inccmp(strOFF,event->data,event->data_len))) {	
	r4slppar1a = 0;
	r4slpcoma = 9;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_strength_OFF");
	}
	}

	} else if (DEV_TYPA < 64) {
	//cooker
	if (!memcmp(event->topic+topoffa, "state", event->topic_len-topoffa)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len)) || (!incascmp("heat",event->data,event->data_len))) {
	if ((!bStateA) || (!r4sppcoma) || (!fcommtp)) {
	r4slppar1a = 1;
	r4slpcoma = 10;
	}
//	ESP_LOGI(AP_TAG,"MQTT_CMD_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bStateA) || (!r4sppcoma) || (!fcommtp)) {
	r4slppar1a = 0;
	r4slpcoma = 10;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_CMD_OFF");
	}
	} else if (!memcmp(event->topic+topoffa, "prname", event->topic_len-topoffa)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	r4slppar1a = 0;
	r4slppar2a = 0;
	r4slppar3a = 0;
	r4slppar4a = 0;
	r4slppar5a = 0;
	r4slppar6a = bAwarmA;
	r4slppar7a = bDHourA;
	r4slppar8a = bDMinA;
        if ( DEV_TYPA == 16 ) {
// for RMC-800s
	if (!incascmp("off",event->data,event->data_len)) r4slppar1a = 255; 
	else if (!incascmp("multicooker",event->data,event->data_len)) r4slppar1a = 0; 
	else if (!incascmp("rice",event->data,event->data_len)) r4slppar1a = 1; 
	else if (!incascmp("slow_cooking",event->data,event->data_len)) r4slppar1a = 2; 
	else if (!incascmp("pilaf",event->data,event->data_len)) r4slppar1a = 3; 
	else if (!incascmp("frying_vegetables",event->data,event->data_len)) { r4slppar1a = 4 ; r4slppar2a = 1; }
	else if (!incascmp("frying_fish",event->data,event->data_len)) { r4slppar1a = 4 ; r4slppar2a = 2; }
	else if (!incascmp("frying_meat",event->data,event->data_len)) { r4slppar1a = 4 ; r4slppar2a = 3; }
	else if (!incascmp("stewing_vegetables",event->data,event->data_len)) { r4slppar1a = 5 ; r4slppar2a = 1; }
	else if (!incascmp("stewing_fish",event->data,event->data_len)) { r4slppar1a = 5 ; r4slppar2a = 2; }
	else if (!incascmp("stewing_meat",event->data,event->data_len)) { r4slppar1a = 5 ; r4slppar2a = 3; }
	else if (!incascmp("pasta",event->data,event->data_len)) r4slppar1a = 6; 
	else if (!incascmp("milk_porridge",event->data,event->data_len)) r4slppar1a = 7; 
	else if (!incascmp("soup",event->data,event->data_len)) r4slppar1a = 8; 
	else if (!incascmp("yogurt",event->data,event->data_len)) r4slppar1a = 9; 
	else if (!incascmp("baking",event->data,event->data_len)) r4slppar1a = 10; 
	else if (!incascmp("steam_vegetables",event->data,event->data_len)) { r4slppar1a = 11 ; r4slppar2a = 1; }
	else if (!incascmp("steam_fish",event->data,event->data_len)) { r4slppar1a = 11 ; r4slppar2a = 2; }
	else if (!incascmp("steam_meat",event->data,event->data_len)) { r4slppar1a = 11 ; r4slppar2a = 3; }
	else if (!incascmp("hot",event->data,event->data_len)) r4slppar1a = 12; 
	} else if ( DEV_TYPA == 17 ) {
// for RMC-903s
	if (!incascmp("off",event->data,event->data_len)) r4slppar1a = 255; 
	else if (!incascmp("Multicooker",event->data,event->data_len)) r4slppar1a = 0; 
	else if (!incascmp("Milk_porridge",event->data,event->data_len)) r4slppar1a = 1; 
	else if (!incascmp("Stewing",event->data,event->data_len)) r4slppar1a = 2; 
	else if (!incascmp("Frying",event->data,event->data_len)) r4slppar1a = 3; 
	else if (!incascmp("Soup",event->data,event->data_len)) r4slppar1a = 4; 
	else if (!incascmp("Steam",event->data,event->data_len)) r4slppar1a = 5; 
	else if (!incascmp("Pasta",event->data,event->data_len)) r4slppar1a = 6; 
	else if (!incascmp("Slow_cooking",event->data,event->data_len)) r4slppar1a = 7; 
	else if (!incascmp("Hot",event->data,event->data_len)) r4slppar1a = 8; 
	else if (!incascmp("Baking",event->data,event->data_len)) r4slppar1a = 9; 
	else if (!incascmp("Groats",event->data,event->data_len)) r4slppar1a = 10; 
	else if (!incascmp("Pilaf",event->data,event->data_len)) r4slppar1a = 11; 
	else if (!incascmp("Yogurt",event->data,event->data_len)) r4slppar1a = 12; 
	else if (!incascmp("Pizza",event->data,event->data_len)) r4slppar1a = 13; 
	else if (!incascmp("Bread",event->data,event->data_len)) r4slppar1a = 14; 
	else if (!incascmp("Desserts",event->data,event->data_len)) r4slppar1a = 15; 
	else if (!incascmp("Express",event->data,event->data_len)) r4slppar1a = 16; 
	} else if ( DEV_TYPA == 18 ) {
// for RMC-224s
	if (!incascmp("off",event->data,event->data_len)) r4slppar1a = 255; 
	else if (!incascmp("Frying",event->data,event->data_len)) r4slppar1a = 0; 
	else if (!incascmp("Groats",event->data,event->data_len)) r4slppar1a = 1; 
	else if (!incascmp("Multicooker",event->data,event->data_len)) r4slppar1a = 2; 
	else if (!incascmp("Pilaf",event->data,event->data_len)) r4slppar1a = 3; 
	else if (!incascmp("Steam",event->data,event->data_len)) r4slppar1a = 4; 
	else if (!incascmp("Baking",event->data,event->data_len)) r4slppar1a = 5; 
	else if (!incascmp("Stewing",event->data,event->data_len)) r4slppar1a = 6; 
	else if (!incascmp("Soup",event->data,event->data_len)) r4slppar1a = 7; 
	else if (!incascmp("Milk_porridge",event->data,event->data_len)) r4slppar1a = 8; 
	else if (!incascmp("Yogurt",event->data,event->data_len)) r4slppar1a = 9; 
	else if (!incascmp("Express",event->data,event->data_len)) r4slppar1a = 10; 
	} else if ( DEV_TYPA == 24 ) {
// for RO-5707
	if (!incascmp("off",event->data,event->data_len)) r4slppar1a = 255; 
	else if (!incascmp("Multicooker",event->data,event->data_len)) r4slppar1a = 0; 
	else if (!incascmp("Omelet",event->data,event->data_len)) r4slppar1a = 1; 
	else if (!incascmp("Slow_cooking_meat",event->data,event->data_len)) r4slppar1a = 2; 
	else if (!incascmp("Slow_cooking_bird",event->data,event->data_len)) r4slppar1a = 3; 
	else if (!incascmp("Slow_cooking_fish",event->data,event->data_len)) r4slppar1a = 4; 
	else if (!incascmp("Slow_cooking_vegetables",event->data,event->data_len)) r4slppar1a = 5; 
	else if (!incascmp("Bread",event->data,event->data_len)) r4slppar1a = 6; 
	else if (!incascmp("Pizza",event->data,event->data_len)) r4slppar1a = 7; 
	else if (!incascmp("Charlotte",event->data,event->data_len)) r4slppar1a = 8; 
	else if (!incascmp("Baking_meat_in_pot",event->data,event->data_len)) r4slppar1a = 9; 
	else if (!incascmp("Baking_bird_in_pot",event->data,event->data_len)) r4slppar1a = 10; 
	else if (!incascmp("Baking_fish_in_pot",event->data,event->data_len)) r4slppar1a = 11; 
	else if (!incascmp("Baking_vegetables_in_pot",event->data,event->data_len)) r4slppar1a = 12; 
	else if (!incascmp("Roast",event->data,event->data_len)) r4slppar1a = 13; 
	else if (!incascmp("Cake",event->data,event->data_len)) r4slppar1a = 14; 
	else if (!incascmp("Baking_meat",event->data,event->data_len)) r4slppar1a = 15; 
	else if (!incascmp("Baking_bird",event->data,event->data_len)) r4slppar1a = 16; 
	else if (!incascmp("Baking_fish",event->data,event->data_len)) r4slppar1a = 17; 
	else if (!incascmp("Baking_vegetables",event->data,event->data_len)) r4slppar1a = 18; 
	else if (!incascmp("Boiled_pork",event->data,event->data_len)) r4slppar1a = 19; 
	else if (!incascmp("Warming",event->data,event->data_len)) r4slppar1a = 20; 
	r4slppar2a = bModProgA;
	}
	if ((!fcommtp) || (!r4sppcoma) || (r4slppar1a != bProgA) || (r4slppar2a != bModProgA)) r4slpcoma = 17;
	} else if (!memcmp(event->topic+topoffa, "prog", event->topic_len-topoffa)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t prog = atoi(tbuff);
	if (prog == 254) prog = 255;
	if ((!fcommtp) || (!r4sppcoma) || (prog != bProgA)) {
	r4slppar1a = prog;
	r4slpcoma = 11;
	}
	} else if (!memcmp(event->topic+topoffa, "mode", event->topic_len-topoffa)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t mod = atoi(tbuff);
	if ((mod < 4) && ((!fcommtp) || (!r4sppcoma) || ( mod != bModProgA))) {
	r4slppar1a = mod;
	r4slpcoma = 12;
	}
	} else if (!memcmp(event->topic+topoffa, "temp", event->topic_len-topoffa)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t temp = atoi(tbuff);
	if ((!fcommtp) || (!r4sppcoma) || (temp != bHtempA)) {
	r4slppar1a = temp;
	r4slpcoma = 13;
	}
	} else if (!memcmp(event->topic+topoffa, "set_hour", event->topic_len-topoffa)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t hour = atoi(tbuff);
	if ((hour < 24) && ((!fcommtp) || (!r4sppcoma) || (hour != bPHourA))) {
	r4slppar1a = hour;
	bPHourA = hour;
	r4slpcoma = 14;
	}
	} else if (!memcmp(event->topic+topoffa, "set_min", event->topic_len-topoffa)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t min = atoi(tbuff);
	if ((min < 60) && ((!fcommtp) || (!r4sppcoma) || (min != bPMinA))) {
	r4slppar1a = min;
	r4slpcoma = 15;
	}
	} else if (!memcmp(event->topic+topoffa, "delay_hour", event->topic_len-topoffa)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t cval = atoi(tbuff);
	if ( DEV_TYPA < 24 ) {
	bDHourA = cval;
	t_lasta_us = ~t_lasta_us;
	} else {
	r4slppar1a = cval;
	bDHourA = cval;
	r4slpcoma = 19;
	}
	} else if (!memcmp(event->topic+topoffa, "delay_min", event->topic_len-topoffa)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t cval = atoi(tbuff);
	if ( DEV_TYPA < 24 ) {
	bDMinA = cval;
	t_lasta_us = ~t_lasta_us;
	} else {
	r4slppar1a = cval;
	r4slpcoma = 20;
	}
	} else if (!memcmp(event->topic+topoffa, "warm", event->topic_len-topoffa)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!bAwarmA) || (!fcommtp) || (!r4sppcoma) || (inccmp(strON,event->data,event->data_len))) {	
	r4slppar1a = 1;
	r4slpcoma = 16;
	}
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bAwarmA) || (!fcommtp) || (!r4sppcoma) || (inccmp(strOFF,event->data,event->data_len))) {	
	r4slppar1a = 0;
	r4slpcoma = 16;
	}
	}
	}
	} else if (DEV_TYPA > 63) {
	//mikettle
	if (!memcmp(event->topic+topoffa, "boil", event->topic_len-topoffa)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len)) || (!incascmp("heat",event->data,event->data_len))) {
	if ((!bStateA) || (!fcommtp) || (!r4sppcoma) || (inccmp(strON,event->data,event->data_len))) {
	r4slppar1a = 0;
	r4slpcoma = 65;
	}
//	ESP_LOGI(AP_TAG,"MQTT_BOIL_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bStateA)  || (!fcommtp) || (!r4sppcoma) || (inccmp(strOFF,event->data,event->data_len))) {
	r4slppar1a = 0;
	r4slpcoma = 64;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_BOIL_OFF");
	}
	} else if (!memcmp(event->topic+topoffa, "heat", event->topic_len-topoffa)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len)) || (!incascmp("heat",event->data,event->data_len))) {
	if ((!bHeatA) || (!fcommtp) || (!r4sppcoma) || (inccmp(strON,event->data,event->data_len))) {
	r4slppar1a = 40;
	r4slpcoma = 66;
	}
//	ESP_LOGI(AP_TAG,"MQTT_HEAT_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bHeatA)  || (!fcommtp) || (!r4sppcoma) || (inccmp(strOFF,event->data,event->data_len))) {
	r4slppar1a = 0;
	r4slpcoma = 66;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_HEAT_OFF");
	}
	} else if (!memcmp(event->topic+topoffa, "heat_temp", event->topic_len-topoffa)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t temp = atoi(tbuff);
	if  ((!temp || (temp > 39)) && (temp < 91) && ((!fcommtp) || (!r4sppcoma) || (temp != bHtempA))) {
	r4slppar1a = temp;
	r4slpcoma = 66;
	} else if ((temp < 40) && ((!fcommtp) || (!r4sppcoma) || (temp != bHtempA))) {
	r4slppar1a = 0;
	r4slpcoma = 64;
	} else if ((temp > 90) && ((!fcommtp) || (!r4sppcoma) || (temp != bHtempA))) {
	r4slppar1a = 0;
	r4slpcoma = 65;
	}
	}
//	ESP_LOGI(AP_TAG,"MQTT_HEAT_TEMP");
	}
	} else if (topoffb && DEV_TYPB && ((t_mqt_us - t_ppconb_us) > 3000000)) {
//ESP_LOGI(AP_TAG,"topoffb=%d", topoffb);
	if (DEV_TYPB < 10) {	
	//kettle
	if (!memcmp(event->topic+topoffb, "boil", event->topic_len-topoffb)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!bStateB) || (!fcommtp) || (!r4sppcomb) || (inccmp(strON,event->data,event->data_len))) {
	if (bHtempB) {
	if (DEV_TYPB == 1) {	
	if (bHtempB < 30) r4slppar1b = 0;
	else if (bHtempB < 41) r4slppar1b = 1;
	else if (bHtempB < 56) r4slppar1b = 2;
	else if (bHtempB < 71) r4slppar1b = 3;
	else if (bHtempB < 86) r4slppar1b = 4;
	else r4slppar1b = 5;
	} else r4slppar1b = bHtempB;
	r4slpcomb = 4;
	} else {
	r4slppar1b = 0;
	r4slpcomb = 2;
		}
	}
//	ESP_LOGI(AP_TAG,"MQTT_BOIL_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bStateB) || (!fcommtp) || (!r4sppcomb) || (inccmp(strOFF,event->data,event->data_len))) {
	if (bHtempB) {
	if (DEV_TYPB == 1) {	
	if (bHtempB < 30) r4slppar1b = 0;
	else if (bHtempB < 41) r4slppar1b = 1;
	else if (bHtempB < 56) r4slppar1b = 2;
	else if (bHtempB < 71) r4slppar1b = 3;
	else if (bHtempB < 86) r4slppar1b = 4;
	else r4slppar1b = 5;
	} else r4slppar1b = bHtempB;
	r4slppar2b = 1;
	r4slpcomb = 3;
	} else {
	r4slppar1b = 0;
	r4slpcomb = 1;
		}
	}	
//	ESP_LOGI(AP_TAG,"MQTT_BOIL_OFF");
		}

	} else if (!memcmp(event->topic+topoffb, "heat", event->topic_len-topoffb)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len)) || (!incascmp("heat",event->data,event->data_len))) {
	if ((!bHeatB) || (!fcommtp) || (!r4sppcomb) || (inccmp(strON,event->data,event->data_len))) {
	uint8_t temp = bHtempB;
        if ((temp < 30) || (temp > 98)) temp = bLtempB;
        if ((temp < 30) || (temp > 98)) temp = 40;

	if (DEV_TYPB == 1) {	
	if (temp < 30) r4slppar1b = 0;
	else if (temp < 41) r4slppar1b = 1;
	else if (temp < 56) r4slppar1b = 2;
	else if (temp < 71) r4slppar1b = 3;
	else if (temp < 86) r4slppar1b = 4;
	else r4slppar1b = 5;
	} else r4slppar1b = temp;
	r4slppar2b = 1;
	r4slpcomb = 3;
	}
//	ESP_LOGI(AP_TAG,"MQTT_HEAT_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bHeatB)  || (!fcommtp) || (!r4sppcomb) || (inccmp(strOFF,event->data,event->data_len))) {
	r4slppar1b = 0;
	r4slpcomb = 1;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_HEAT_OFF");
		} 

	} else if (!memcmp(event->topic+topoffb, "heat_temp", event->topic_len-topoffb)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t temp = atoi(tbuff);
	if ((!bHtempB) && (bLtempB) && (temp < 30)) temp = bLtempB;
	if  ((!fcommtp) || (!r4sppcomb) || ((temp != bHtempB) && temp && bHtempB)) {
	if (DEV_TYPB == 1) {	
	if (temp < 30) r4slppar1b = 0;
	else if (temp < 41) r4slppar1b = 1;
	else if (temp < 56) r4slppar1b = 2;
	else if (temp < 71) r4slppar1b = 3;
	else if (temp < 86) r4slppar1b = 4;
	else r4slppar1b = 5;
	} else r4slppar1b = temp;
	r4slppar2b = 1;
	r4slpcomb = 3;
	} else if ((temp < 30) && ((!fcommtp) || (!r4sppcomb) || (temp != bHtempB))) {
	r4slppar1b = 0;
	r4slpcomb = 1;
	} else if ((temp > 98) && ((!fcommtp) || (!r4sppcomb) || (temp != bHtempB))) {
	r4slppar1b = 0;
	r4slpcomb = 1;
	}
//	ESP_LOGI(AP_TAG,"MQTT_HEAT_TEMP");

	} else if (!memcmp(event->topic+topoffb, "backlight", event->topic_len-topoffb)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!bStBlB) || (!fcommtp) || (!r4sppcomb) || (inccmp(strON,event->data,event->data_len))) {
	r4slppar1b = 0;
	r4slpcomb = 22;
	}
//	ESP_LOGI(AP_TAG,"MQTT_BACKLIGHT_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bStBlB) || (!fcommtp) || (!r4sppcomb) || (inccmp(strOFF,event->data,event->data_len))) {
	r4slppar1b = 0;
	r4slpcomb = 21;
	}
//	ESP_LOGI(AP_TAG,"MQTT_BACKLIGHT_OFF");
	}

	} else if (!memcmp(event->topic+topoffb, "beep", event->topic_len-topoffb)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!bStBpB) || (!fcommtp) || (!r4sppcomb) || (inccmp(strON,event->data,event->data_len))) {
	r4slppar1b = 0;
	r4slpcomb = 24;
	}
//	ESP_LOGI(AP_TAG,"MQTT_BEEP_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bStBpB) || (!fcommtp) || (!r4sppcomb) || (inccmp(strOFF,event->data,event->data_len))) {
	r4slppar1b = 0;
	r4slpcomb = 23;
	}
//	ESP_LOGI(AP_TAG,"MQTT_BEEP_OFF");
	}

	} else if (!memcmp(event->topic+topoffb, "nightlight", event->topic_len-topoffb)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!bStNlB) || (!fcommtp) || (!r4sppcomb) || (inccmp(strON,event->data,event->data_len))) {
	r4slppar1b = 0;
	r4slpcomb = 5;
	}
//	ESP_LOGI(AP_TAG,"MQTT_NIGHTLIGHT_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bStNlB) || (!fcommtp) || (!r4sppcomb) || (inccmp(strOFF,event->data,event->data_len))) {
	r4slppar1b = 0;
	r4slpcomb = 1;
	}
//	ESP_LOGI(AP_TAG,"MQTT_NIGHTLIGHT_OFF");
	}

	} else if (!memcmp(event->topic+topoffb, "nightlight_rgb", event->topic_len-topoffb)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t i = 0;
	uint8_t of1 = 0;
	uint8_t of2 = 0;
	while (i < 12) {
	if (tbuff[i] == 0x2c) {
	tbuff[i] = 0;
	if (!of1) of1 = i+1;
	else if (!of2) of2 = i+1;
	else i = 20;
	} else	if (tbuff[i] == 0x00) {
	if ((of1) && (of1 < of2) && (of2 < i)) i = 12;
	else i = 20; 	
	} else if ((tbuff[i] < 0x30) || (tbuff[i] > 0x39)) i = 20;
	i++;
	}
	if (i < 20) {
	uint8_t rval = atoi(tbuff);
	uint8_t gval = atoi(tbuff+of1);
	uint8_t bval = atoi(tbuff+of2);
	if ((!fcommtp) || (!r4sppcomb) || (RgbRB != rval) || (RgbGB != gval) || (RgbBB != bval)) {
	RgbRB = rval;
	RgbGB = gval;
	RgbBB = bval;
	if (bStNlB) {	
	r4slppar1b = 0;
	r4slpcomb = 5;
	}
	t_lastb_us = ~t_lastb_us;
	}
	}

	} else if (!memcmp(event->topic+topoffb, "nightlight_red", event->topic_len-topoffb)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t cval = atoi(tbuff);
	if ((!fcommtp) || (!r4sppcomb) || (RgbRB != cval)) {
	RgbRB = cval;
	if (bStNlB) {	
	r4slppar1b = 0;
	r4slpcomb = 5;
	}
	t_lastb_us = ~t_lastb_us;
	}
	} else if (!memcmp(event->topic+topoffb, "nightlight_green", event->topic_len-topoffb)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t cval = atoi(tbuff);
	if ((!fcommtp) || (!r4sppcomb) || (RgbGB != cval)) {
	RgbGB = cval;
	if (bStNlB) {	
	r4slppar1b = 0;
	r4slpcomb = 5;
	}
	t_lastb_us = ~t_lastb_us;
	}
	} else if (!memcmp(event->topic+topoffb, "nightlight_blue", event->topic_len-topoffb)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t cval = atoi(tbuff);
	if ((!fcommtp) || (!r4sppcomb) || (RgbBB != cval)) {
	RgbBB = cval;
	if (bStNlB) {	
	r4slppar1b = 0;
	r4slpcomb = 5;
	}
	t_lastb_us = ~t_lastb_us;
	}
	}
	} else if ( DEV_TYPB < 12) {
	//power
	if (!memcmp(event->topic+topoffb, "state", event->topic_len-topoffb)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!bStateB) || (!fcommtp) || (!r4sppcomb) || (inccmp(strON,event->data,event->data_len))) {
	r4slppar1b = 0;
	r4slpcomb = 6;
	}
//	ESP_LOGI(AP_TAG,"MQTT_CMD_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bStateB) || (!fcommtp) || (!r4sppcomb) || (inccmp(strOFF,event->data,event->data_len))) {
	r4slppar1b = 0;
	r4slpcomb = 1;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_CMD_OFF");
	}
	} else 	if (!memcmp(event->topic+topoffb, "lock", event->topic_len-topoffb)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!bLockB) || (!fcommtp) || (!r4sppcomb) || (inccmp(strON,event->data,event->data_len))) {	
	r4slppar1b = 0;
	r4slpcomb = 8;
	}
//	ESP_LOGI(AP_TAG,"MQTT_lock_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bLockB) || (!fcommtp) || (!r4sppcomb) || (inccmp(strOFF,event->data,event->data_len))) {	
	r4slppar1b = 0;
	r4slpcomb = 7;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_lock_OFF");
	}
	}
	} else if ( DEV_TYPB < 16) {
	//coffee
	if (!memcmp(event->topic+topoffb, "state", event->topic_len-topoffb)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!bStateB) || (!fcommtp) || (!r4sppcomb) || (inccmp(strON,event->data,event->data_len))) {
	r4slppar1b = 0;
	r4slpcomb = 6;
	}
//	ESP_LOGI(AP_TAG,"MQTT_CMD_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bStateB) || (!fcommtp) || (!r4sppcomb) || (inccmp(strOFF,event->data,event->data_len))) {
	r4slppar1b = 0;
	r4slpcomb = 1;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_CMD_OFF");
	}
	} else if (!memcmp(event->topic+topoffb, "delay", event->topic_len-topoffb)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!bStNlB) || (!fcommtp) || (!r4sppcomb) || (inccmp(strON,event->data,event->data_len))) {	
	r4slppar1b = bProgB;
	r4slppar2b = bDHourB;
	r4slppar3b = bDMinB;
	bDHourB = 0;
	bDMinB = 0;
	r4slpcomb = 18;
	}
//	ESP_LOGI(AP_TAG,"MQTT_delay_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bStNlB) || (!fcommtp) || (!r4sppcomb) || (inccmp(strOFF,event->data,event->data_len))) {	
	r4slppar1b = 0;
	r4slpcomb = 1;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_delay_OFF");
	}
	} else if (!memcmp(event->topic+topoffb, "delay_hour", event->topic_len-topoffb)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t hour = atoi(tbuff);
	if (hour < 24) {
	bDHourB = hour;
	t_lastb_us = ~t_lastb_us;
	}
	} else if (!memcmp(event->topic+topoffb, "delay_min", event->topic_len-topoffb)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t min = atoi(tbuff);
	if (min < 60) {
	bDMinB = min;
	t_lastb_us = ~t_lastb_us;
	}
	} else 	if (!memcmp(event->topic+topoffb, "lock", event->topic_len-topoffb)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!bLockB) || (!fcommtp) || (!r4sppcomb) || (inccmp(strON,event->data,event->data_len))) {	
	r4slppar1b = 0;
	r4slpcomb = 8;
	}
//	ESP_LOGI(AP_TAG,"MQTT_lock_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bLockB) || (!fcommtp) || (!r4sppcomb) || (inccmp(strOFF,event->data,event->data_len))) {	
	r4slppar1b = 0;
	r4slpcomb = 7;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_lock_OFF");
	}
	} else 	if (!memcmp(event->topic+topoffb, "strength", event->topic_len-topoffb)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!bProgB) || (!fcommtp) || (!r4sppcomb) || (inccmp(strON,event->data,event->data_len))) {	
	r4slppar1b = 1;
	r4slpcomb = 9;
	}
//	ESP_LOGI(AP_TAG,"MQTT_strength_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bProgB) || (!fcommtp) || (!r4sppcomb) || (inccmp(strOFF,event->data,event->data_len))) {	
	r4slppar1b = 0;
	r4slpcomb = 9;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_strength_OFF");
	}
	}

	} else if ( DEV_TYPB < 64) {
	//cooker
	if (!memcmp(event->topic+topoffb, "state", event->topic_len-topoffb)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len)) || (!incascmp("heat",event->data,event->data_len))) {
	if ((!bStateB) || (!r4sppcomb) || (!fcommtp)) {
	r4slppar1b = 1;
	r4slpcomb = 10;
	}
//	ESP_LOGI(AP_TAG,"MQTT_CMD_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bStateB) || (!r4sppcomb) || (!fcommtp)) {
	r4slppar1b = 0;
	r4slpcomb = 10;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_CMD_OFF");
	}

	} else if (!memcmp(event->topic+topoffb, "prname", event->topic_len-topoffb)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	r4slppar1b = 0;
	r4slppar2b = 0;
	r4slppar3b = 0;
	r4slppar4b = 0;
	r4slppar5b = 0;
	r4slppar6b = bAwarmB;
	r4slppar7b = bDHourB;
	r4slppar8b = bDMinB;
        if ( DEV_TYPB == 16 ) {
// for RMC-800s
	if (!incascmp("off",event->data,event->data_len)) r4slppar1b = 255; 
	else if (!incascmp("multicooker",event->data,event->data_len)) r4slppar1b = 0; 
	else if (!incascmp("rice",event->data,event->data_len)) r4slppar1b = 1; 
	else if (!incascmp("slow_cooking",event->data,event->data_len)) r4slppar1b = 2; 
	else if (!incascmp("pilaf",event->data,event->data_len)) r4slppar1b = 3; 
	else if (!incascmp("frying_vegetables",event->data,event->data_len)) { r4slppar1b = 4 ; r4slppar2b = 1; }
	else if (!incascmp("frying_fish",event->data,event->data_len)) { r4slppar1b = 4 ; r4slppar2b = 2; }
	else if (!incascmp("frying_meat",event->data,event->data_len)) { r4slppar1b = 4 ; r4slppar2b = 3; }
	else if (!incascmp("stewing_vegetables",event->data,event->data_len)) { r4slppar1b = 5 ; r4slppar2b = 1; }
	else if (!incascmp("stewing_fish",event->data,event->data_len)) { r4slppar1b = 5 ; r4slppar2b = 2; }
	else if (!incascmp("stewing_meat",event->data,event->data_len)) { r4slppar1b = 5 ; r4slppar2b = 3; }
	else if (!incascmp("pasta",event->data,event->data_len)) r4slppar1b = 6; 
	else if (!incascmp("milk_porridge",event->data,event->data_len)) r4slppar1b = 7; 
	else if (!incascmp("soup",event->data,event->data_len)) r4slppar1b = 8; 
	else if (!incascmp("yogurt",event->data,event->data_len)) r4slppar1b = 9; 
	else if (!incascmp("baking",event->data,event->data_len)) r4slppar1b = 10; 
	else if (!incascmp("steam_vegetables",event->data,event->data_len)) { r4slppar1b = 11 ; r4slppar2b = 1; }
	else if (!incascmp("steam_fish",event->data,event->data_len)) { r4slppar1b = 11 ; r4slppar2b = 2; }
	else if (!incascmp("steam_meat",event->data,event->data_len)) { r4slppar1b = 11 ; r4slppar2b = 3; }
	else if (!incascmp("hot",event->data,event->data_len)) r4slppar1b = 12; 
	} else if ( DEV_TYPB == 17 ) {
// for RMC-903s
	if (!incascmp("off",event->data,event->data_len)) r4slppar1b = 255; 
	else if (!incascmp("Multicooker",event->data,event->data_len)) r4slppar1b = 0; 
	else if (!incascmp("Milk_porridge",event->data,event->data_len)) r4slppar1b = 1; 
	else if (!incascmp("Stewing",event->data,event->data_len)) r4slppar1b = 2; 
	else if (!incascmp("Frying",event->data,event->data_len)) r4slppar1b = 3; 
	else if (!incascmp("Soup",event->data,event->data_len)) r4slppar1b = 4; 
	else if (!incascmp("Steam",event->data,event->data_len)) r4slppar1b = 5; 
	else if (!incascmp("Pasta",event->data,event->data_len)) r4slppar1b = 6; 
	else if (!incascmp("Slow_cooking",event->data,event->data_len)) r4slppar1b = 7; 
	else if (!incascmp("Hot",event->data,event->data_len)) r4slppar1b = 8; 
	else if (!incascmp("Baking",event->data,event->data_len)) r4slppar1b = 9; 
	else if (!incascmp("Groats",event->data,event->data_len)) r4slppar1b = 10; 
	else if (!incascmp("Pilaf",event->data,event->data_len)) r4slppar1b = 11; 
	else if (!incascmp("Yogurt",event->data,event->data_len)) r4slppar1b = 12; 
	else if (!incascmp("Pizza",event->data,event->data_len)) r4slppar1b = 13; 
	else if (!incascmp("Bread",event->data,event->data_len)) r4slppar1b = 14; 
	else if (!incascmp("Desserts",event->data,event->data_len)) r4slppar1b = 15; 
	else if (!incascmp("Express",event->data,event->data_len)) r4slppar1b = 16; 
	} else if ( DEV_TYPB == 18 ) {
// for RMC-224s
	if (!incascmp("off",event->data,event->data_len)) r4slppar1b = 255; 
	else if (!incascmp("Frying",event->data,event->data_len)) r4slppar1b = 0; 
	else if (!incascmp("Groats",event->data,event->data_len)) r4slppar1b = 1; 
	else if (!incascmp("Multicooker",event->data,event->data_len)) r4slppar1b = 2; 
	else if (!incascmp("Pilaf",event->data,event->data_len)) r4slppar1b = 3; 
	else if (!incascmp("Steam",event->data,event->data_len)) r4slppar1b = 4; 
	else if (!incascmp("Baking",event->data,event->data_len)) r4slppar1b = 5; 
	else if (!incascmp("Stewing",event->data,event->data_len)) r4slppar1b = 6; 
	else if (!incascmp("Soup",event->data,event->data_len)) r4slppar1b = 7; 
	else if (!incascmp("Milk_porridge",event->data,event->data_len)) r4slppar1b = 8; 
	else if (!incascmp("Yogurt",event->data,event->data_len)) r4slppar1b = 9; 
	else if (!incascmp("Express",event->data,event->data_len)) r4slppar1b = 10; 
	} else if ( DEV_TYPB == 24 ) {
// for RO-5707
	if (!incascmp("off",event->data,event->data_len)) r4slppar1b = 255; 
	else if (!incascmp("Multicooker",event->data,event->data_len)) r4slppar1b = 0; 
	else if (!incascmp("Omelet",event->data,event->data_len)) r4slppar1b = 1; 
	else if (!incascmp("Slow_cooking_meat",event->data,event->data_len)) r4slppar1b = 2; 
	else if (!incascmp("Slow_cooking_bird",event->data,event->data_len)) r4slppar1b = 3; 
	else if (!incascmp("Slow_cooking_fish",event->data,event->data_len)) r4slppar1b = 4; 
	else if (!incascmp("Slow_cooking_vegetables",event->data,event->data_len)) r4slppar1b = 5; 
	else if (!incascmp("Bread",event->data,event->data_len)) r4slppar1b = 6; 
	else if (!incascmp("Pizza",event->data,event->data_len)) r4slppar1b = 7; 
	else if (!incascmp("Charlotte",event->data,event->data_len)) r4slppar1b = 8; 
	else if (!incascmp("Baking_meat_in_pot",event->data,event->data_len)) r4slppar1b = 9; 
	else if (!incascmp("Baking_bird_in_pot",event->data,event->data_len)) r4slppar1b = 10; 
	else if (!incascmp("Baking_fish_in_pot",event->data,event->data_len)) r4slppar1b = 11; 
	else if (!incascmp("Baking_vegetables_in_pot",event->data,event->data_len)) r4slppar1b = 12; 
	else if (!incascmp("Roast",event->data,event->data_len)) r4slppar1b = 13; 
	else if (!incascmp("Cake",event->data,event->data_len)) r4slppar1b = 14; 
	else if (!incascmp("Baking_meat",event->data,event->data_len)) r4slppar1b = 15; 
	else if (!incascmp("Baking_bird",event->data,event->data_len)) r4slppar1b = 16; 
	else if (!incascmp("Baking_fish",event->data,event->data_len)) r4slppar1b = 17; 
	else if (!incascmp("Baking_vegetables",event->data,event->data_len)) r4slppar1b = 18; 
	else if (!incascmp("Boiled_pork",event->data,event->data_len)) r4slppar1b = 19; 
	else if (!incascmp("Warming",event->data,event->data_len)) r4slppar1b = 20; 
	r4slppar2b = bModProgB;
	}
	if ((!fcommtp) || (!r4sppcomb) || (r4slppar1b != bProgB) || (r4slppar2b != bModProgB)) r4slpcomb = 17;
	} else if (!memcmp(event->topic+topoffb, "prog", event->topic_len-topoffb)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t prog = atoi(tbuff);
	if (prog == 254) prog = 255;
	if ((!fcommtp) || (!r4sppcomb) || (prog != bProgB)) {
	r4slppar1b = prog;
	r4slpcomb = 11;
	}
	} else if (!memcmp(event->topic+topoffb, "mode", event->topic_len-topoffb)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t mod = atoi(tbuff);
	if ((mod < 4) && ((!fcommtp) || (!r4sppcomb) || ( mod != bModProgB))) {
	r4slppar1b = mod;
	r4slpcomb = 12;
	}
	} else if (!memcmp(event->topic+topoffb, "temp", event->topic_len-topoffb)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t temp = atoi(tbuff);
	if ((!fcommtp) || (!r4sppcomb) || (temp != bHtempB)) {
	r4slppar1b = temp;
	r4slpcomb = 13;
	}
	} else if (!memcmp(event->topic+topoffb, "set_hour", event->topic_len-topoffb)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t hour = atoi(tbuff);
	if ((hour < 24) && ((!fcommtp) || (!r4sppcomb) || (hour != bPHourB))) {
	r4slppar1b = hour;
	bPHourB = hour;
	r4slpcomb = 14;
	}
	} else if (!memcmp(event->topic+topoffb, "set_min", event->topic_len-topoffb)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t min = atoi(tbuff);
	if ((min < 60) && ((!fcommtp) || (!r4sppcomb) || (min != bPMinB))) {
	r4slppar1b = min;
	r4slpcomb = 15;
	}
	} else if (!memcmp(event->topic+topoffb, "delay_hour", event->topic_len-topoffb)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t cval = atoi(tbuff);
	if ( DEV_TYPB < 24 ) {
	bDHourB = cval;
	t_lastb_us = ~t_lastb_us;
	} else {
	r4slppar1b = cval;
	bDHourB = cval;
	r4slpcomb = 19;
	}
	} else if (!memcmp(event->topic+topoffb, "delay_min", event->topic_len-topoffb)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t cval = atoi(tbuff);
	if ( DEV_TYPB < 24 ) {
	bDMinB = cval;
	t_lastb_us = ~t_lastb_us;
	} else {
	r4slppar1b = cval;
	r4slpcomb = 20;
	}
	} else if (!memcmp(event->topic+topoffb, "warm", event->topic_len-topoffb)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!bAwarmB) || (!fcommtp) || (!r4sppcomb) || (inccmp(strON,event->data,event->data_len))) {	
	r4slppar1b = 1;
	r4slpcomb = 16;
	}
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bAwarmB) || (!fcommtp) || (!r4sppcomb) || (inccmp(strOFF,event->data,event->data_len))) {	
	r4slppar1b = 0;
	r4slpcomb = 16;
	}
	}
	}
	} else if (DEV_TYPB > 63) {
	//mikettle
	if (!memcmp(event->topic+topoffb, "boil", event->topic_len-topoffb)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len)) || (!incascmp("heat",event->data,event->data_len))) {
	if ((!bStateB) || (!fcommtp) || (!r4sppcomb) || (inccmp(strON,event->data,event->data_len))) {
	r4slppar1b = 0;
	r4slpcomb = 65;
	}
//	ESP_LOGI(AP_TAG,"MQTT_BOIL_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bStateB)  || (!fcommtp) || (!r4sppcomb) || (inccmp(strOFF,event->data,event->data_len))) {
	r4slppar1b = 0;
	r4slpcomb = 64;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_BOIL_OFF");
	}
	} else if (!memcmp(event->topic+topoffb, "heat", event->topic_len-topoffb)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len)) || (!incascmp("heat",event->data,event->data_len))) {
	if ((!bHeatB) || (!fcommtp) || (!r4sppcoma) || (inccmp(strON,event->data,event->data_len))) {
	r4slppar1b = 40;
	r4slpcomb = 66;
	}
//	ESP_LOGI(AP_TAG,"MQTT_HEAT_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bHeatB)  || (!fcommtp) || (!r4sppcoma) || (inccmp(strOFF,event->data,event->data_len))) {
	r4slppar1b = 0;
	r4slpcomb = 66;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_HEAT_OFF");
	}
	} else if (!memcmp(event->topic+topoffb, "heat_temp", event->topic_len-topoffb)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t temp = atoi(tbuff);
	if  ((!temp || (temp > 39)) && (temp < 91) && ((!fcommtp) || (!r4sppcomb) || (temp != bHtempB))) {
	r4slppar1b = temp;
	r4slpcomb = 66;
	} else if ((temp < 40) && ((!fcommtp) || (!r4sppcomb) || (temp != bHtempB))) {
	r4slppar1b = 0;
	r4slpcomb = 64;
	} else if ((temp > 90) && ((!fcommtp) || (!r4sppcomb) || (temp != bHtempB))) {
	r4slppar1b = 0;
	r4slpcomb = 65;
	}
	}
//	ESP_LOGI(AP_TAG,"MQTT_HEAT_TEMP");
	}
	} else if (topoffc && DEV_TYPC && ((t_mqt_us - t_ppconc_us) > 3000000)) {
//ESP_LOGI(AP_TAG,"topoffc=%d", topoffc);
	if (DEV_TYPC < 10) {	
	//kettle
	if (!memcmp(event->topic+topoffc, "boil", event->topic_len-topoffc)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!bStateC) || (!fcommtp) || (!r4sppcomc) || (inccmp(strON,event->data,event->data_len))) {
	if (bHtempC) {
	if (DEV_TYPC == 1) {	
	if (bHtempC < 30) r4slppar1c = 0;
	else if (bHtempC < 41) r4slppar1c = 1;
	else if (bHtempC < 56) r4slppar1c = 2;
	else if (bHtempC < 71) r4slppar1c = 3;
	else if (bHtempC < 86) r4slppar1c = 4;
	else r4slppar1c = 5;
	} else r4slppar1c = bHtempC;
	r4slpcomc = 4;
	} else {
	r4slppar1c = 0;
	r4slpcomc = 2;
		}
	}
//	ESP_LOGI(AP_TAG,"MQTT_BOIL_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bStateC) || (!fcommtp) || (!r4sppcomc) || (inccmp(strOFF,event->data,event->data_len))) {
	if (bHtempC) {
	if (DEV_TYPC == 1) {	
	if (bHtempC < 30) r4slppar1c = 0;
	else if (bHtempC < 41) r4slppar1c = 1;
	else if (bHtempC < 56) r4slppar1c = 2;
	else if (bHtempC < 71) r4slppar1c = 3;
	else if (bHtempC < 86) r4slppar1c = 4;
	else r4slppar1c = 5;
	} else r4slppar1c = bHtempC;
	r4slppar2c = 1;
	r4slpcomc = 3;
	} else {
	r4slppar1c = 0;
	r4slpcomc = 1;
		}
	}	
//	ESP_LOGI(AP_TAG,"MQTT_BOIL_OFF");
		}

	} else if (!memcmp(event->topic+topoffc, "heat", event->topic_len-topoffc)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len)) || (!incascmp("heat",event->data,event->data_len))) {
	if ((!bHeatC) || (!fcommtp) || (!r4sppcomc) || (inccmp(strON,event->data,event->data_len))) {
	uint8_t temp = bHtempC;
        if ((temp < 30) || (temp > 98)) temp = bLtempC;
        if ((temp < 30) || (temp > 98)) temp = 40;

	if (DEV_TYPC == 1) {	
	if (temp < 30) r4slppar1c = 0;
	else if (temp < 41) r4slppar1c = 1;
	else if (temp < 56) r4slppar1c = 2;
	else if (temp < 71) r4slppar1c = 3;
	else if (temp < 86) r4slppar1c = 4;
	else r4slppar1c = 5;
	} else r4slppar1c = temp;
	r4slppar2c = 1;
	r4slpcomc = 3;
	}
//	ESP_LOGI(AP_TAG,"MQTT_HEAT_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bHeatC)  || (!fcommtp) || (!r4sppcomc) || (inccmp(strOFF,event->data,event->data_len))) {
	r4slppar1c = 0;
	r4slpcomc = 1;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_HEAT_OFF");
		} 

	} else if (!memcmp(event->topic+topoffc, "heat_temp", event->topic_len-topoffc)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t temp = atoi(tbuff);
	if ((!bHtempC) && (bLtempC) && (temp < 30)) temp = bLtempC;
	if  ((!fcommtp) || (!r4sppcomc) || ((temp != bHtempC) && temp && bHtempC)) {
	if (DEV_TYPC == 1) {	
	if (temp < 30) r4slppar1c = 0;
	else if (temp < 41) r4slppar1c = 1;
	else if (temp < 56) r4slppar1c = 2;
	else if (temp < 71) r4slppar1c = 3;
	else if (temp < 86) r4slppar1c = 4;
	else r4slppar1c = 5;
	} else r4slppar1c = temp;
	r4slppar2c = 1;
	r4slpcomc = 3;
	} else if ((temp < 30) && ((!fcommtp) || (!r4sppcomc) || (temp != bHtempC))) {
	r4slppar1c = 0;
	r4slpcomc = 1;
	} else if ((temp > 98) && ((!fcommtp) || (!r4sppcomc) || (temp != bHtempC))) {
	r4slppar1c = 0;
	r4slpcomc = 1;
	}
//	ESP_LOGI(AP_TAG,"MQTT_HEAT_TEMP");

	} else if (!memcmp(event->topic+topoffc, "backlight", event->topic_len-topoffc)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!bStBlC) || (!fcommtp) || (!r4sppcomc) || (inccmp(strON,event->data,event->data_len))) {
	r4slppar1c = 0;
	r4slpcomc = 22;
	}
//	ESP_LOGI(AP_TAG,"MQTT_BACKLIGHT_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bStBlC) || (!fcommtp) || (!r4sppcomc) || (inccmp(strOFF,event->data,event->data_len))) {
	r4slppar1c = 0;
	r4slpcomc = 21;
	}
//	ESP_LOGI(AP_TAG,"MQTT_BACKLIGHT_OFF");
	}

	} else if (!memcmp(event->topic+topoffc, "beep", event->topic_len-topoffc)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!bStBpC) || (!fcommtp) || (!r4sppcomc) || (inccmp(strON,event->data,event->data_len))) {
	r4slppar1c = 0;
	r4slpcomc = 24;
	}
//	ESP_LOGI(AP_TAG,"MQTT_BEEP_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bStBpC) || (!fcommtp) || (!r4sppcomc) || (inccmp(strOFF,event->data,event->data_len))) {
	r4slppar1c = 0;
	r4slpcomc = 23;
	}
//	ESP_LOGI(AP_TAG,"MQTT_BEEP_OFF");
	}

	} else if (!memcmp(event->topic+topoffc, "nightlight", event->topic_len-topoffc)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!bStNlC) || (!fcommtp) || (!r4sppcomc) || (inccmp(strON,event->data,event->data_len))) {
	r4slppar1c = 0;
	r4slpcomc = 5;
	}
//	ESP_LOGI(AP_TAG,"MQTT_NIGHTLIGHT_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bStNlC) || (!fcommtp) || (!r4sppcomc) || (inccmp(strOFF,event->data,event->data_len))) {
	r4slppar1c = 0;
	r4slpcomc = 1;
	}
//	ESP_LOGI(AP_TAG,"MQTT_NIGHTLIGHT_OFF");
	}

	} else if (!memcmp(event->topic+topoffc, "nightlight_rgb", event->topic_len-topoffc)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t i = 0;
	uint8_t of1 = 0;
	uint8_t of2 = 0;
	while (i < 12) {
	if (tbuff[i] == 0x2c) {
	tbuff[i] = 0;
	if (!of1) of1 = i+1;
	else if (!of2) of2 = i+1;
	else i = 20;
	} else	if (tbuff[i] == 0x00) {
	if ((of1) && (of1 < of2) && (of2 < i)) i = 12;
	else i = 20; 	
	} else if ((tbuff[i] < 0x30) || (tbuff[i] > 0x39)) i = 20;
	i++;
	}
	if (i < 20) {
	uint8_t rval = atoi(tbuff);
	uint8_t gval = atoi(tbuff+of1);
	uint8_t bval = atoi(tbuff+of2);
	if ((!fcommtp) || (!r4sppcomc) || (RgbRC != rval) || (RgbGC != gval) || (RgbBC != bval)) {
	RgbRC = rval;
	RgbGC = gval;
	RgbBC = bval;
	if (bStNlC) {	
	r4slppar1c = 0;
	r4slpcomc = 5;
	}
	t_lastc_us = ~t_lastc_us;
	}
	}

	} else if (!memcmp(event->topic+topoffc, "nightlight_red", event->topic_len-topoffc)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t cval = atoi(tbuff);
	if ((!fcommtp) || (!r4sppcomc) || (RgbRC != cval)) {
	RgbRC = cval;
	if (bStNlC) {	
	r4slppar1c = 0;
	r4slpcomc = 5;
	}
	t_lastc_us = ~t_lastc_us;
	}
	} else if (!memcmp(event->topic+topoffc, "nightlight_green", event->topic_len-topoffc)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t cval = atoi(tbuff);
	if ((!fcommtp) || (!r4sppcomc) || (RgbGC != cval)) {
	RgbGC = cval;
	if (bStNlC) {	
	r4slppar1c = 0;
	r4slpcomc = 5;
	}
	t_lastc_us = ~t_lastc_us;
	}
	} else if (!memcmp(event->topic+topoffc, "nightlight_blue", event->topic_len-topoffc)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t cval = atoi(tbuff);
	if ((!fcommtp) || (!r4sppcomc) || (RgbBC != cval)) {
	RgbBC = cval;
	if (bStNlC) {	
	r4slppar1c = 0;
	r4slpcomc = 5;
	}
	t_lastc_us = ~t_lastc_us;
	}
	}
	} else if ( DEV_TYPC < 12) {
	//power
	if (!memcmp(event->topic+topoffc, "state", event->topic_len-topoffc)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!bStateC) || (!fcommtp) || (!r4sppcomc) || (inccmp(strON,event->data,event->data_len))) {
	r4slppar1c = 0;
	r4slpcomc = 6;
	}
//	ESP_LOGI(AP_TAG,"MQTT_CMD_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bStateC) || (!fcommtp) || (!r4sppcomc) || (inccmp(strOFF,event->data,event->data_len))) {
	r4slppar1c = 0;
	r4slpcomc = 1;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_CMD_OFF");
	}
	} else 	if (!memcmp(event->topic+topoffc, "lock", event->topic_len-topoffc)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!bLockC) || (!fcommtp) || (!r4sppcomc) || (inccmp(strON,event->data,event->data_len))) {	
	r4slppar1c = 0;
	r4slpcomc = 8;
	}
//	ESP_LOGI(AP_TAG,"MQTT_lock_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bLockC) || (!fcommtp) || (!r4sppcomc) || (inccmp(strOFF,event->data,event->data_len))) {	
	r4slppar1c = 0;
	r4slpcomc = 7;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_lock_OFF");
	}
	}
	} else if ( DEV_TYPC < 16) {
	//coffee
	if (!memcmp(event->topic+topoffc, "state", event->topic_len-topoffc)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!bStateC) || (!fcommtp) || (!r4sppcomc) || (inccmp(strON,event->data,event->data_len))) {
	r4slppar1c = 0;
	r4slpcomc = 6;
	}
//	ESP_LOGI(AP_TAG,"MQTT_CMD_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bStateC) || (!fcommtp) || (!r4sppcomc) || (inccmp(strOFF,event->data,event->data_len))) {
	r4slppar1c = 0;
	r4slpcomc = 1;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_CMD_OFF");
	}
	} else if (!memcmp(event->topic+topoffc, "delay", event->topic_len-topoffc)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!bStNlC) || (!fcommtp) || (!r4sppcomc) || (inccmp(strON,event->data,event->data_len))) {	
	r4slppar1c = bProgC;
	r4slppar2c = bDHourC;
	r4slppar3c = bDMinC;
	bDHourC = 0;
	bDMinC = 0;
	r4slpcomc = 18;
	}
//	ESP_LOGI(AP_TAG,"MQTT_delay_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bStNlC) || (!fcommtp) || (!r4sppcomc) || (inccmp(strOFF,event->data,event->data_len))) {	
	r4slppar1c = 0;
	r4slpcomc = 1;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_delay_OFF");
	}
	} else if (!memcmp(event->topic+topoffc, "delay_hour", event->topic_len-topoffc)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t hour = atoi(tbuff);
	if (hour < 24) {
	bDHourC = hour;
	t_lastc_us = ~t_lastc_us;
	}
	} else if (!memcmp(event->topic+topoffc, "delay_min", event->topic_len-topoffc)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t min = atoi(tbuff);
	if (min < 60) {
	bDMinC = min;
	t_lastc_us = ~t_lastc_us;
	}
	} else 	if (!memcmp(event->topic+topoffc, "lock", event->topic_len-topoffc)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!bLockC) || (!fcommtp) || (!r4sppcomc) || (inccmp(strON,event->data,event->data_len))) {	
	r4slppar1c = 0;
	r4slpcomc = 8;
	}
//	ESP_LOGI(AP_TAG,"MQTT_lock_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bLockC) || (!fcommtp) || (!r4sppcomc) || (inccmp(strOFF,event->data,event->data_len))) {	
	r4slppar1c = 0;
	r4slpcomc = 7;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_lock_OFF");
	}
	} else 	if (!memcmp(event->topic+topoffc, "strength", event->topic_len-topoffc)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!bProgC) || (!fcommtp) || (!r4sppcomc) || (inccmp(strON,event->data,event->data_len))) {	
	r4slppar1c = 1;
	r4slpcomc = 9;
	}
//	ESP_LOGI(AP_TAG,"MQTT_strength_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bProgC) || (!fcommtp) || (!r4sppcomc) || (inccmp(strOFF,event->data,event->data_len))) {	
	r4slppar1c = 0;
	r4slpcomc = 9;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_strength_OFF");
	}
	}

	} else if ( DEV_TYPC < 64) {
	//cooker
	if (!memcmp(event->topic+topoffc, "state", event->topic_len-topoffc)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len)) || (!incascmp("heat",event->data,event->data_len))) {
	if ((!bStateC) || (!r4sppcomc) || (!fcommtp)) {
	r4slppar1c = 1;
	r4slpcomc = 10;
	}
//	ESP_LOGI(AP_TAG,"MQTT_CMD_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bStateC) || (!r4sppcomc) || (!fcommtp)) {
	r4slppar1c = 0;
	r4slpcomc = 10;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_CMD_OFF");
	}
	} else if (!memcmp(event->topic+topoffc, "prname", event->topic_len-topoffc)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	r4slppar1c = 0;
	r4slppar2c = 0;
	r4slppar3c = 0;
	r4slppar4c = 0;
	r4slppar5c = 0;
	r4slppar6c = bAwarmC;
	r4slppar7c = bDHourC;
	r4slppar8c = bDMinC;
        if ( DEV_TYPC == 16 ) {
// for RMC-800s
	if (!incascmp("off",event->data,event->data_len)) r4slppar1c = 255; 
	else if (!incascmp("multicooker",event->data,event->data_len)) r4slppar1c = 0; 
	else if (!incascmp("rice",event->data,event->data_len)) r4slppar1c = 1; 
	else if (!incascmp("slow_cooking",event->data,event->data_len)) r4slppar1c = 2; 
	else if (!incascmp("pilaf",event->data,event->data_len)) r4slppar1c = 3; 
	else if (!incascmp("frying_vegetables",event->data,event->data_len)) { r4slppar1c = 4 ; r4slppar2c = 1; }
	else if (!incascmp("frying_fish",event->data,event->data_len)) { r4slppar1c = 4 ; r4slppar2c = 2; }
	else if (!incascmp("frying_meat",event->data,event->data_len)) { r4slppar1c = 4 ; r4slppar2c = 3; }
	else if (!incascmp("stewing_vegetables",event->data,event->data_len)) { r4slppar1c = 5 ; r4slppar2c = 1; }
	else if (!incascmp("stewing_fish",event->data,event->data_len)) { r4slppar1c = 5 ; r4slppar2c = 2; }
	else if (!incascmp("stewing_meat",event->data,event->data_len)) { r4slppar1c = 5 ; r4slppar2c = 3; }
	else if (!incascmp("pasta",event->data,event->data_len)) r4slppar1c = 6; 
	else if (!incascmp("milk_porridge",event->data,event->data_len)) r4slppar1c = 7; 
	else if (!incascmp("soup",event->data,event->data_len)) r4slppar1c = 8; 
	else if (!incascmp("yogurt",event->data,event->data_len)) r4slppar1c = 9; 
	else if (!incascmp("baking",event->data,event->data_len)) r4slppar1c = 10; 
	else if (!incascmp("steam_vegetables",event->data,event->data_len)) { r4slppar1c = 11 ; r4slppar2c = 1; }
	else if (!incascmp("steam_fish",event->data,event->data_len)) { r4slppar1c = 11 ; r4slppar2c = 2; }
	else if (!incascmp("steam_meat",event->data,event->data_len)) { r4slppar1c = 11 ; r4slppar2c = 3; }
	else if (!incascmp("hot",event->data,event->data_len)) r4slppar1c = 12; 
	} else if ( DEV_TYPC == 17 ) {
// for RMC-903s
	if (!incascmp("off",event->data,event->data_len)) r4slppar1c = 255; 
	else if (!incascmp("Multicooker",event->data,event->data_len)) r4slppar1c = 0; 
	else if (!incascmp("Milk_porridge",event->data,event->data_len)) r4slppar1c = 1; 
	else if (!incascmp("Stewing",event->data,event->data_len)) r4slppar1c = 2; 
	else if (!incascmp("Frying",event->data,event->data_len)) r4slppar1c = 3; 
	else if (!incascmp("Soup",event->data,event->data_len)) r4slppar1c = 4; 
	else if (!incascmp("Steam",event->data,event->data_len)) r4slppar1c = 5; 
	else if (!incascmp("Pasta",event->data,event->data_len)) r4slppar1c = 6; 
	else if (!incascmp("Slow_cooking",event->data,event->data_len)) r4slppar1c = 7; 
	else if (!incascmp("Hot",event->data,event->data_len)) r4slppar1c = 8; 
	else if (!incascmp("Baking",event->data,event->data_len)) r4slppar1c = 9; 
	else if (!incascmp("Groats",event->data,event->data_len)) r4slppar1c = 10; 
	else if (!incascmp("Pilaf",event->data,event->data_len)) r4slppar1c = 11; 
	else if (!incascmp("Yogurt",event->data,event->data_len)) r4slppar1c = 12; 
	else if (!incascmp("Pizza",event->data,event->data_len)) r4slppar1c = 13; 
	else if (!incascmp("Bread",event->data,event->data_len)) r4slppar1c = 14; 
	else if (!incascmp("Desserts",event->data,event->data_len)) r4slppar1c = 15; 
	else if (!incascmp("Express",event->data,event->data_len)) r4slppar1c = 16; 
	} else if ( DEV_TYPC == 18 ) {
// for RMC-224s
	if (!incascmp("off",event->data,event->data_len)) r4slppar1c = 255; 
	else if (!incascmp("Frying",event->data,event->data_len)) r4slppar1c = 0; 
	else if (!incascmp("Groats",event->data,event->data_len)) r4slppar1c = 1; 
	else if (!incascmp("Multicooker",event->data,event->data_len)) r4slppar1c = 2; 
	else if (!incascmp("Pilaf",event->data,event->data_len)) r4slppar1c = 3; 
	else if (!incascmp("Steam",event->data,event->data_len)) r4slppar1c = 4; 
	else if (!incascmp("Baking",event->data,event->data_len)) r4slppar1c = 5; 
	else if (!incascmp("Stewing",event->data,event->data_len)) r4slppar1c = 6; 
	else if (!incascmp("Soup",event->data,event->data_len)) r4slppar1c = 7; 
	else if (!incascmp("Milk_porridge",event->data,event->data_len)) r4slppar1c = 8; 
	else if (!incascmp("Yogurt",event->data,event->data_len)) r4slppar1c = 9; 
	else if (!incascmp("Express",event->data,event->data_len)) r4slppar1c = 10; 
	} else if ( DEV_TYPC == 24 ) {
// for RO-5707
	if (!incascmp("off",event->data,event->data_len)) r4slppar1c = 255; 
	else if (!incascmp("Multicooker",event->data,event->data_len)) r4slppar1c = 0; 
	else if (!incascmp("Omelet",event->data,event->data_len)) r4slppar1c = 1; 
	else if (!incascmp("Slow_cooking_meat",event->data,event->data_len)) r4slppar1c = 2; 
	else if (!incascmp("Slow_cooking_bird",event->data,event->data_len)) r4slppar1c = 3; 
	else if (!incascmp("Slow_cooking_fish",event->data,event->data_len)) r4slppar1c = 4; 
	else if (!incascmp("Slow_cooking_vegetables",event->data,event->data_len)) r4slppar1c = 5; 
	else if (!incascmp("Bread",event->data,event->data_len)) r4slppar1c = 6; 
	else if (!incascmp("Pizza",event->data,event->data_len)) r4slppar1c = 7; 
	else if (!incascmp("Charlotte",event->data,event->data_len)) r4slppar1c = 8; 
	else if (!incascmp("Baking_meat_in_pot",event->data,event->data_len)) r4slppar1c = 9; 
	else if (!incascmp("Baking_bird_in_pot",event->data,event->data_len)) r4slppar1c = 10; 
	else if (!incascmp("Baking_fish_in_pot",event->data,event->data_len)) r4slppar1c = 11; 
	else if (!incascmp("Baking_vegetables_in_pot",event->data,event->data_len)) r4slppar1c = 12; 
	else if (!incascmp("Roast",event->data,event->data_len)) r4slppar1c = 13; 
	else if (!incascmp("Cake",event->data,event->data_len)) r4slppar1c = 14; 
	else if (!incascmp("Baking_meat",event->data,event->data_len)) r4slppar1c = 15; 
	else if (!incascmp("Baking_bird",event->data,event->data_len)) r4slppar1c = 16; 
	else if (!incascmp("Baking_fish",event->data,event->data_len)) r4slppar1c = 17; 
	else if (!incascmp("Baking_vegetables",event->data,event->data_len)) r4slppar1c = 18; 
	else if (!incascmp("Boiled_pork",event->data,event->data_len)) r4slppar1c = 19; 
	else if (!incascmp("Warming",event->data,event->data_len)) r4slppar1c = 20; 
	r4slppar2c = bModProgC;
	}
	if ((!fcommtp) || (!r4sppcomc) || (r4slppar1c != bProgC) || (r4slppar2c != bModProgC)) r4slpcomc = 17;
	} else if (!memcmp(event->topic+topoffc, "prog", event->topic_len-topoffc)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t prog = atoi(tbuff);
	if (prog == 254) prog = 255;
	if ((!fcommtp) || (!r4sppcomc) || (prog != bProgC)) {
	r4slppar1c = prog;
	r4slpcomc = 11;
	}
	} else if (!memcmp(event->topic+topoffc, "mode", event->topic_len-topoffc)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t mod = atoi(tbuff);
	if ((mod < 4) && ((!fcommtp) || (!r4sppcomc) || ( mod != bModProgC))) {
	r4slppar1c = mod;
	r4slpcomc = 12;
	}
	} else if (!memcmp(event->topic+topoffc, "temp", event->topic_len-topoffc)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t temp = atoi(tbuff);
	if ((!fcommtp) || (!r4sppcomc) || (temp != bHtempC)) {
	r4slppar1c = temp;
	r4slpcomc = 13;
	}
	} else if (!memcmp(event->topic+topoffc, "set_hour", event->topic_len-topoffc)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t hour = atoi(tbuff);
	if ((hour < 24) && ((!fcommtp) || (!r4sppcomc) || (hour != bPHourC))) {
	r4slppar1c = hour;
	bPHourC = hour;
	r4slpcomc = 14;
	}
	} else if (!memcmp(event->topic+topoffc, "set_min", event->topic_len-topoffc)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t min = atoi(tbuff);
	if ((min < 60) && ((!fcommtp) || (!r4sppcomc) || (min != bPMinC))) {
	r4slppar1c = min;
	r4slpcomc = 15;
	}
	} else if (!memcmp(event->topic+topoffc, "delay_hour", event->topic_len-topoffc)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t cval = atoi(tbuff);
	if ( DEV_TYPC < 24 ) {
	bDHourC = cval;
	t_lastc_us = ~t_lastc_us;
	} else {
	r4slppar1c = cval;
	bDHourC = cval;
	r4slpcomc = 19;
	}
	} else if (!memcmp(event->topic+topoffc, "delay_min", event->topic_len-topoffc)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t cval = atoi(tbuff);
	if ( DEV_TYPC < 24 ) {
	bDMinC = cval;
	t_lastc_us = ~t_lastc_us;
	} else {
	r4slppar1c = cval;
	r4slpcomc = 20;
	}
	} else if (!memcmp(event->topic+topoffc, "warm", event->topic_len-topoffc)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!bAwarmC) || (!fcommtp) || (!r4sppcomc) || (inccmp(strON,event->data,event->data_len))) {	
	r4slppar1c = 1;
	r4slpcomc = 16;
	}
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bAwarmC) || (!fcommtp) || (!r4sppcomc) || (inccmp(strOFF,event->data,event->data_len))) {	
	r4slppar1c = 0;
	r4slpcomc = 16;
	}
	}
	}
	} else if (DEV_TYPC > 63) {
	//mikettle
	if (!memcmp(event->topic+topoffc, "boil", event->topic_len-topoffc)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len)) || (!incascmp("heat",event->data,event->data_len))) {
	if ((!bStateC) || (!fcommtp) || (!r4sppcomc) || (inccmp(strON,event->data,event->data_len))) {
	r4slppar1c = 0;
	r4slpcomc = 65;
	}
//	ESP_LOGI(AP_TAG,"MQTT_BOIL_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bStateC)  || (!fcommtp) || (!r4sppcomc) || (inccmp(strOFF,event->data,event->data_len))) {
	r4slppar1c = 0;
	r4slpcomc = 64;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_BOIL_OFF");
	}
	} else if (!memcmp(event->topic+topoffc, "heat", event->topic_len-topoffc)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len)) || (!incascmp("heat",event->data,event->data_len))) {
	if ((!bHeatC) || (!fcommtp) || (!r4sppcoma) || (inccmp(strON,event->data,event->data_len))) {
	r4slppar1c = 40;
	r4slpcomc = 66;
	}
//	ESP_LOGI(AP_TAG,"MQTT_HEAT_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((bHeatC)  || (!fcommtp) || (!r4sppcoma) || (inccmp(strOFF,event->data,event->data_len))) {
	r4slppar1c = 0;
	r4slpcomc = 66;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_HEAT_OFF");
	}
	} else if (!memcmp(event->topic+topoffc, "heat_temp", event->topic_len-topoffc)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, event->data, tempsz);
	uint8_t temp = atoi(tbuff);
	if  ((!temp || (temp > 39)) && (temp < 91) && ((!fcommtp) || (!r4sppcomc) || (temp != bHtempC))) {
	r4slppar1c = temp;
	r4slpcomc = 66;
	} else if ((temp < 40) && ((!fcommtp) || (!r4sppcomc) || (temp != bHtempC))) {
	r4slppar1c = 0;
	r4slpcomc = 64;
	} else if ((temp > 90) && ((!fcommtp) || (!r4sppcomc) || (temp != bHtempC))) {
	r4slppar1c = 0;
	r4slpcomc = 65;
	}
	}
//	ESP_LOGI(AP_TAG,"MQTT_HEAT_TEMP");
	}
	} else if ((MQTT_TOPP1[0]) && (!memcmp(event->topic, MQTT_TOPP1, event->topic_len))) {
	int tempsz = event->data_len;
	if  (tempsz > 15) tempsz = 15;
	mystrcpy(MQTT_VALP1, event->data, tempsz);
	} else if ((MQTT_TOPP2[0]) && (!memcmp(event->topic, MQTT_TOPP2, event->topic_len))) {
	int tempsz = event->data_len;
	if  (tempsz > 15) tempsz = 15;
	mystrcpy(MQTT_VALP2, event->data, tempsz);
	} else if ((MQTT_TOPP3[0]) && (!memcmp(event->topic, MQTT_TOPP3, event->topic_len))) {
	int tempsz = event->data_len;
	if  (tempsz > 15) tempsz = 15;
	mystrcpy(MQTT_VALP3, event->data, tempsz);
	} else if ((MQTT_TOPP4[0]) && (!memcmp(event->topic, MQTT_TOPP4, event->topic_len))) {
	int tempsz = event->data_len;
	if  (tempsz > 15) tempsz = 15;
	mystrcpy(MQTT_VALP4, event->data, tempsz);
	} else if ((MQTT_TOPP5[0]) && (!memcmp(event->topic, MQTT_TOPP5, event->topic_len))) {
	int tempsz = event->data_len;
	if  (tempsz > 15) tempsz = 15;
	mystrcpy(MQTT_VALP5, event->data, tempsz);
	} else if ((MQTT_TOPP6[0]) && (!memcmp(event->topic, MQTT_TOPP6, event->topic_len))) {
	int tempsz = event->data_len;
	if  (tempsz > 15) tempsz = 15;
	mystrcpy(MQTT_VALP6, event->data, tempsz);
	} else if ((MQTT_TOPP7[0]) && (!memcmp(event->topic, MQTT_TOPP7, event->topic_len))) {
	int tempsz = event->data_len;
	if  (tempsz > 15) tempsz = 15;
	mystrcpy(MQTT_VALP7, event->data, tempsz);
	}                        // a b c mqtt topic

	}                        //datalen,topiclen
	break;




	case MQTT_EVENT_ERROR:
//	ESP_LOGI(AP_TAG,"MQTT_EVENT_ERROR");
	break;
	default:
//	ESP_LOGI(AP_TAG,"Other event id:%d", event->event_id);
	break;
	}
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
//    ESP_LOGI(AP_TAG,"Event dispatched from event loop base=%s, event_id=%d\n", base, event_id);
    mqtt_event_handler_cb(event_data);
}

static void mqtt_app_start(void)
{
	char buff[16];
	char luri[128];
	char llwtt[16];
	strcpy(luri,"mqtt://");
	strcat(luri,MQTT_USER);
	strcat(luri,":");
	strcat(luri,MQTT_PASSWORD);
	strcat(luri,"@");
	strcat(luri,MQTT_SERVER);
	strcat(luri,":");
	itoa(mqtt_port,buff,10);
	strcat(luri,buff);
	strcpy(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/status");

	esp_mqtt_client_config_t mqtt_cfg = {
	.uri = luri,
	.lwt_topic = llwtt,
	.lwt_msg = "offline",
	.keepalive = 60,
	.client_id = MQTT_BASE_TOPIC,
	.buffer_size = 2048,
	};
//	ESP_LOGI(AP_TAG,"Mqtt url: %s", luri);
	mqttConnected = false;
	mqttclient = esp_mqtt_client_init(&mqtt_cfg);
	esp_mqtt_client_register_event(mqttclient, ESP_EVENT_ANY_ID, mqtt_event_handler, mqttclient);
	esp_mqtt_client_start(mqttclient);
}


//******************* WiFi **********************
/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;
/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */


static int s_retry_num = 0;

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
	esp_wifi_connect();
	} else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
	ESP_LOGI(AP_TAG,"AP disconnected");
	if (s_retry_num < WIFI_MAXIMUM_RETRY) {
		esp_wifi_connect();
		s_retry_num++;
	ESP_LOGI(AP_TAG, "Retry %d to connect to the AP",s_retry_num);
	} else {
	xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
	ESP_LOGI(AP_TAG,"Connect to the AP fail");
	}
	} else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
	ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
	ESP_LOGI(AP_TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
	s_retry_num = 0;
	xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
	}
}

void wifi_init_sta(void)
{
	char buff[32];
	s_wifi_event_group = xEventGroupCreate();

	ESP_ERROR_CHECK(esp_netif_init());

	ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_netif_create_default_wifi_sta();

	strcpy(buff,MQTT_BASE_TOPIC);
	strcat(buff,"Gate");
	esp_netif_t *esp_netif = NULL;
	esp_netif = esp_netif_next(esp_netif);
	esp_netif_set_hostname(esp_netif, buff);

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	esp_event_handler_instance_t instance_any_id;
	esp_event_handler_instance_t instance_got_ip;
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

	wifi_config_t wifi_config = {
	.sta = {
//	.ssid = WIFI_SSID,
//	.password = WIFI_PASSWORD,
	/* Setting a password implies station will connect to all security modes including WEP/WPA.
	* However these modes are deprecated and not advisable to be used. Incase your Access point
	* doesn't support WPA2, these mode can be enabled by commenting below line */
//		.threshold.authmode = WIFI_AUTH_WPA2_PSK,
		.threshold.authmode = WIFI_AUTH_WPA_PSK,

		.pmf_cfg = {
		.capable = true,
		.required = false
		},
	},
	};

	memcpy(wifi_config.sta.ssid, WIFI_SSID, sizeof(wifi_config.sta.ssid));
	memcpy(wifi_config.sta.password, WIFI_PASSWORD, sizeof(wifi_config.sta.password));

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
	ESP_ERROR_CHECK(esp_wifi_start() );

	ESP_LOGI(AP_TAG,"Wifi_init_sta finished.");
	ESP_LOGI(AP_TAG,"Connecting to Wifi: '%s' with password '%s'", WIFI_SSID, WIFI_PASSWORD);

	/* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
	* number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
	EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
		WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
		pdFALSE,
		pdFALSE,
		portMAX_DELAY);

	/* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
	* happened. */
	if (bits & WIFI_CONNECTED_BIT) {
	ESP_LOGI(AP_TAG,"Connected to ap SSID:'%s' password:'%s'",
		WIFI_SSID, WIFI_PASSWORD);
	} else if (bits & WIFI_FAIL_BIT) {
	ESP_LOGI(AP_TAG,"Failed to connect to SSID:'%s', password:'%s'",
		WIFI_SSID, WIFI_PASSWORD);

	ESP_ERROR_CHECK(esp_wifi_stop() );
	char DEFWFSSID[33];
	char DEFWFPSW[65];
	strcpy(DEFWFSSID,"r4s");
	strcpy(DEFWFPSW,"12345678");
	memcpy(wifi_config.sta.ssid, DEFWFSSID, sizeof(wifi_config.sta.ssid));
	memcpy(wifi_config.sta.password, DEFWFPSW, sizeof(wifi_config.sta.password));
	bits = xEventGroupClearBits(s_wifi_event_group,
		WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);

	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
	ESP_ERROR_CHECK(esp_wifi_start() );
	ESP_LOGI(AP_TAG,"Connecting to ap SSID:'r4s' password:'12345678'");

	/* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
	* number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
	bits = xEventGroupWaitBits(s_wifi_event_group,
		WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
		pdFALSE,
		pdFALSE,
		portMAX_DELAY);

	/* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
	* happened. */
	if (bits & WIFI_CONNECTED_BIT) {
	ESP_LOGI(AP_TAG,"Connected to ap SSID:'r4s' password:'12345678'");
	} else if (bits & WIFI_FAIL_BIT) {
	ESP_LOGI(AP_TAG,"Failed to connect to SSID:'r4s', password:'12345678'");
	ESP_LOGI(AP_TAG,"Restarting now...");
	fflush(stdout);
	esp_restart();
	}
	} else {
	ESP_LOGE(AP_TAG, "UNEXPECTED EVENT");
	}
}




//*************** http server *******************


/* HTTP GET main handler */
static esp_err_t pmain_get_handler(httpd_req_t *req)
{
	int FreeMem = esp_get_free_heap_size();
	char bufip[32] = {0};
	time_t now;
	char strftime_buf[64];
	struct tm timeinfo;
	time(&now);
	localtime_r(&now, &timeinfo);
	strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
	int buf_len;
	buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
	if (buf_len > 1) {
	if (buf_len >31) buf_len = 31;
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Host", bufip, buf_len) == ESP_OK) {
        }
    }
	char bsend[10000];
        char buff[64];
	strcpy(bsend,"<!DOCTYPE html><html>");
	strcat(bsend,"<head><title>r4sGate</title>");
        strcat(bsend,"<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
        strcat(bsend,cssDatasheet);
	strcat(bsend,"</head><body class='bodymenu'><header class='headermenu'><h1>ESP32 r4sGate");
	if (R4SNUM)  {
	itoa(R4SNUM,buff,10);
	strcat(bsend, buff);
	}
	strcat(bsend," for Redmond +</h1>");
	strcat(bsend,"<div class='menubar'><a class='menu active' href='.'>&#8962;<span class='showmenulabel'>Main");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev1'>&#128246;<span class='showmenulabel'> 1 ");
	(REQ_NAMEA[0])? strcat(bsend,REQ_NAMEA) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev2'>&#128246;<span class='showmenulabel'> 2 ");
	(REQ_NAMEB[0])? strcat(bsend,REQ_NAMEB) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev3'>&#128246;<span class='showmenulabel'> 3 ");
	(REQ_NAMEC[0])? strcat(bsend,REQ_NAMEC) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='setting'>&#9881;<span class='showmenulabel'>Setting</span></a>");
	strcat(bsend,"<a class='menu' href='restart'>&#128259;<span class='showmenulabel'>Reboot</span></a>");
	strcat(bsend,"<a class='menu' href='update'>&#10548;<span class='showmenulabel'>Load firmware</span></a></div>");
	strcat(bsend,"</header><table class='normal'>");

	strcat(bsend,"<h3>1 ");
       	if (tBLEAddrA[0] && DEV_TYPA) {
	if ((DEV_TYPA < 10) || (DEV_TYPA > 63)) strcat(bsend,"Kettle, ");
	else if (DEV_TYPA < 12) strcat(bsend,"Power, ");
	else if (DEV_TYPA < 16) strcat(bsend,"Coffee, ");
	else if (DEV_TYPA < 24) strcat(bsend,"Cooker, ");
	else if (DEV_TYPA < 32) strcat(bsend,"Oven, ");
	strcat(bsend,"Address: ");
	strcat(bsend,tBLEAddrA);
	strcat(bsend,", Name: ");
	if (DEV_NAMEA[0]) strcat(bsend,DEV_NAMEA);
	strcat(bsend,", State: ");
	if ((DEV_TYPA < 10) || (DEV_TYPA > 63)) {
        if ((!bStateA) && (!bHeatA || (DEV_TYPA > 63)) && (!bStNlA)) strcat(bsend,"Off");
 	else if (bStateA == 254) strcat(bsend,"Offline");
 	else strcat(bsend,"On");
//	if ((DEV_TYPA < 10) || (DEV_TYPA > 3)) {
	if (DEV_TYPA < 10) {
	strcat(bsend,", Mode: ");
	if (bProgA == 1) strcat(bsend,"Heat");
	else if (bProgA == 2) strcat(bsend,"Boil&Heat");
	else if (bProgA == 3) strcat(bsend,"Nightlight");
	else strcat(bsend,"Boil");
	}
	strcat(bsend,", Temp: ");
	itoa(bCtempA,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"&deg;C, Heat: ");
	itoa(bHtempA,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"&deg;C, ");
	} else if (DEV_TYPA < 12) {
        if (!bStateA) strcat(bsend,"Off");
 	else if (bStateA == 254) strcat(bsend,"Offline");
 	else strcat(bsend,"On");
	strcat(bsend,", Lock: ");
        if (!bLockA) strcat(bsend,"Off");
 	else strcat(bsend,"On");
	strcat(bsend,", ");
	} else if (DEV_TYPA < 16) {
        if (!bStateA) strcat(bsend,"Off");
 	else if (bStateA == 254) strcat(bsend,"Offline");
 	else strcat(bsend,"On");
	strcat(bsend,", Strength: ");
        if (!bProgA) strcat(bsend,"Off");
 	else strcat(bsend,"On");
	strcat(bsend,", Delay: ");
        if (!bStNlA) strcat(bsend,"Off");
 	else strcat(bsend,"On");
	strcat(bsend,", Lock: ");
        if (!bLockA) strcat(bsend,"Off");
 	else strcat(bsend,"On");
	strcat(bsend,", SetTime: ");
	if (bPHourA < 10) strcat(bsend,"0");
	itoa(bPHourA,buff,10);
	strcat(bsend,buff);
	strcat(bsend,":");
	if (bPMinA < 10) strcat(bsend,"0");
	itoa(bPMinA,buff,10);
	strcat(bsend,buff);
	strcat(bsend,", CurTime: ");
	if (bCHourA < 10) strcat(bsend,"0");
	itoa(bCHourA,buff,10);
	strcat(bsend,buff);
	strcat(bsend,":");
	if (bCMinA < 10) strcat(bsend,"0");
	itoa(bCMinA,buff,10);
	strcat(bsend,buff);
	strcat(bsend,", ");
	} else if (DEV_TYPA < 64) {
        if (!bStateA) strcat(bsend,"Off");
        else if (bStateA == 1) strcat(bsend,"Setting");
        else if (bStateA == 2) strcat(bsend,"On");
        else if (bStateA == 4) strcat(bsend,"Warming");
        else if (bStateA == 5) strcat(bsend,"DelayedStart");
        else if (bStateA == 6) strcat(bsend,"WaitPasta");
 	else if (bStateA == 254) strcat(bsend,"Offline");
	strcat(bsend,", Prog: ");
	itoa(bProgA,buff,10);
	strcat(bsend,buff);
	strcat(bsend,", Mode: ");
	itoa(bModProgA,buff,10);
	strcat(bsend,buff);
	strcat(bsend,", Temp: ");
	itoa(bHtempA,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"&deg;C, SetTime: ");
	if (bPHourA < 10) strcat(bsend,"0");
	itoa(bPHourA,buff,10);
	strcat(bsend,buff);
	strcat(bsend,":");
	if (bPMinA < 10) strcat(bsend,"0");
	itoa(bPMinA,buff,10);
	strcat(bsend,buff);
	strcat(bsend,", CurTime: ");
	if (bCHourA < 10) strcat(bsend,"0");
	itoa(bCHourA,buff,10);
	strcat(bsend,buff);
	strcat(bsend,":");
	if (bCMinA < 10) strcat(bsend,"0");
	itoa(bCMinA,buff,10);
	strcat(bsend,buff);
	strcat(bsend,", ");
	}
	} else strcat(bsend,"Not defined, ");
        strcat(bsend,"Json String:</h3><h2>{\"mqtt\":");
	itoa(mqttConnected,buff,10);
	strcat(bsend,buff);
        if (tBLEAddrA[0]) {
        strcat(bsend,",\"");
	strcat(bsend,tBLEAddrA);
        strcat(bsend,"\":{\"name\":\"");
	if (DEV_NAMEA[0]) strcat(bsend,DEV_NAMEA);
        strcat(bsend,"\",\"status\":\"");
        if (btauthoriza) {
        strcat(bsend,"online\",\"state\":");
        if (cStatusA[0]) strcat(bsend,cStatusA);
        } else strcat(bsend,"offline\"");
	}

	strcat(bsend,"}<h3>2 ");
       	if (tBLEAddrB[0] && DEV_TYPB) {
	if ((DEV_TYPB < 10) || (DEV_TYPB > 63)) strcat(bsend,"Kettle, ");
	else if (DEV_TYPB < 12) strcat(bsend,"Power, ");
	else if (DEV_TYPB < 16) strcat(bsend,"Coffee, ");
	else if (DEV_TYPB < 24) strcat(bsend,"Cooker, ");
	else if (DEV_TYPB < 32) strcat(bsend,"Oven, ");
	strcat(bsend,"Address: ");
	strcat(bsend,tBLEAddrB);
	strcat(bsend,", Name: ");
	if (DEV_NAMEB[0]) strcat(bsend,DEV_NAMEB);
	strcat(bsend,", State: ");
	if ((DEV_TYPB < 10) || (DEV_TYPB > 63)) {
        if ((!bStateB) && (!bHeatB || (DEV_TYPB > 63)) && (!bStNlB)) strcat(bsend,"Off");
 	else if (bStateB == 254) strcat(bsend,"Offline");
 	else strcat(bsend,"On");
//	if ((DEV_TYPB < 10) || (DEV_TYPB > 3)) {
	if (DEV_TYPB < 10) {
	strcat(bsend,", Mode: ");
	if (bProgB == 1) strcat(bsend,"Heat");
	else if (bProgB == 2) strcat(bsend,"Boil&Heat");
	else if (bProgB == 3) strcat(bsend,"Nightlight");
	else strcat(bsend,"Boil");
	}
	strcat(bsend,", Temp: ");
	itoa(bCtempB,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"&deg;C, Heat: ");
	itoa(bHtempB,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"&deg;C, ");
	} else if (DEV_TYPB < 12) {
        if (!bStateB) strcat(bsend,"Off");
 	else if (bStateB == 254) strcat(bsend,"Offline");
 	else strcat(bsend,"On");
	strcat(bsend,", Lock: ");
        if (!bLockB) strcat(bsend,"Off");
 	else strcat(bsend,"On");
	strcat(bsend,", ");
	} else if (DEV_TYPB < 16) {
        if (!bStateB) strcat(bsend,"Off");
 	else if (bStateB == 254) strcat(bsend,"Offline");
 	else strcat(bsend,"On");
	strcat(bsend,", Strength: ");
        if (!bProgB) strcat(bsend,"Off");
 	else strcat(bsend,"On");
	strcat(bsend,", Delay: ");
        if (!bStNlB) strcat(bsend,"Off");
 	else strcat(bsend,"On");
	strcat(bsend,", Lock: ");
        if (!bLockB) strcat(bsend,"Off");
 	else strcat(bsend,"On");
	strcat(bsend,", SetTime: ");
	if (bPHourB < 10) strcat(bsend,"0");
	itoa(bPHourB,buff,10);
	strcat(bsend,buff);
	strcat(bsend,":");
	if (bPMinB < 10) strcat(bsend,"0");
	itoa(bPMinB,buff,10);
	strcat(bsend,buff);
	strcat(bsend,", CurTime: ");
	if (bCHourB < 10) strcat(bsend,"0");
	itoa(bCHourB,buff,10);
	strcat(bsend,buff);
	strcat(bsend,":");
	if (bCMinB < 10) strcat(bsend,"0");
	itoa(bCMinB,buff,10);
	strcat(bsend,buff);
	strcat(bsend,", ");
	} else if (DEV_TYPB < 64) {
        if (!bStateB) strcat(bsend,"Off");
        else if (bStateB == 1) strcat(bsend,"Setting");
        else if (bStateB == 2) strcat(bsend,"On");
        else if (bStateB == 4) strcat(bsend,"Warming");
        else if (bStateB == 5) strcat(bsend,"DelayedStart");
        else if (bStateB == 6) strcat(bsend,"WaitPasta");
 	else if (bStateB == 254) strcat(bsend,"Offline");
	strcat(bsend,", Prog: ");
	itoa(bProgB,buff,10);
	strcat(bsend,buff);
	strcat(bsend,", Mode: ");
	itoa(bModProgB,buff,10);
	strcat(bsend,buff);
	strcat(bsend,", Temp: ");
	itoa(bHtempB,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"&deg;C, SetTime: ");
	if (bPHourB < 10) strcat(bsend,"0");
	itoa(bPHourB,buff,10);
	strcat(bsend,buff);
	strcat(bsend,":");
	if (bPMinB < 10) strcat(bsend,"0");
	itoa(bPMinB,buff,10);
	strcat(bsend,buff);
	strcat(bsend,", CurTime: ");
	if (bCHourB < 10) strcat(bsend,"0");
	itoa(bCHourB,buff,10);
	strcat(bsend,buff);
	strcat(bsend,":");
	if (bCMinB < 10) strcat(bsend,"0");
	itoa(bCMinB,buff,10);
	strcat(bsend,buff);
	strcat(bsend,", ");
	}
	} else strcat(bsend,"Not defined, ");
        strcat(bsend,"Json String:</h3><h2>{\"mqtt\":");
	itoa(mqttConnected,buff,10);
	strcat(bsend,buff);
        if (tBLEAddrB[0]) {
        strcat(bsend,",\"");
	strcat(bsend,tBLEAddrB);
        strcat(bsend,"\":{\"name\":\"");
	if (DEV_NAMEB[0]) strcat(bsend,DEV_NAMEB);
        strcat(bsend,"\",\"status\":\"");
        if (btauthorizb) {
        strcat(bsend,"online\",\"state\":");
        if (cStatusB[0]) strcat(bsend,cStatusB);
        } else strcat(bsend,"offline\"");
	}

	strcat(bsend,"}<h3>3 ");
       	if (tBLEAddrC[0] && DEV_TYPC) {
	if ((DEV_TYPC < 10) || (DEV_TYPC > 63)) strcat(bsend,"Kettle, ");
	else if (DEV_TYPC < 12) strcat(bsend,"Power, ");
	else if (DEV_TYPC < 16) strcat(bsend,"Coffee, ");
	else if (DEV_TYPC < 24) strcat(bsend,"Cooker, ");
	else if (DEV_TYPC < 32) strcat(bsend,"Oven, ");
	strcat(bsend,"Address: ");
	strcat(bsend,tBLEAddrC);
	strcat(bsend,", Name: ");
	if (DEV_NAMEC[0]) strcat(bsend,DEV_NAMEC);
	strcat(bsend,", State: ");
	if ((DEV_TYPC < 10) || (DEV_TYPC > 63)) {
        if ((!bStateC) && (!bHeatC || (DEV_TYPC > 63)) && (!bStNlC)) strcat(bsend,"Off");
 	else if (bStateC == 254) strcat(bsend,"Offline");
 	else strcat(bsend,"On");
//	if ((DEV_TYPC < 10) || (DEV_TYPC > 3)) {
	if (DEV_TYPC < 10) {
	strcat(bsend,", Mode: ");
	if (bProgC == 1) strcat(bsend,"Heat");
	else if (bProgC == 2) strcat(bsend,"Boil&Heat");
	else if (bProgC == 3) strcat(bsend,"Nightlight");
	else strcat(bsend,"Boil");
	}
	strcat(bsend,", Temp: ");
	itoa(bCtempC,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"&deg;C, Heat: ");
	itoa(bHtempC,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"&deg;C, ");
	} else if (DEV_TYPC < 12) {
        if (!bStateC) strcat(bsend,"Off");
 	else if (bStateC == 254) strcat(bsend,"Offline");
 	else strcat(bsend,"On");
	strcat(bsend,", Lock: ");
        if (!bLockC) strcat(bsend,"Off");
 	else strcat(bsend,"On");
	strcat(bsend,", ");
	} else if (DEV_TYPC < 16) {
        if (!bStateC) strcat(bsend,"Off");
 	else if (bStateC == 254) strcat(bsend,"Offline");
 	else strcat(bsend,"On");
	strcat(bsend,", Strength: ");
        if (!bProgC) strcat(bsend,"Off");
 	else strcat(bsend,"On");
	strcat(bsend,", Delay: ");
        if (!bStNlC) strcat(bsend,"Off");
 	else strcat(bsend,"On");
	strcat(bsend,", Lock: ");
        if (!bLockC) strcat(bsend,"Off");
 	else strcat(bsend,"On");
	strcat(bsend,", SetTime: ");
	if (bPHourC < 10) strcat(bsend,"0");
	itoa(bPHourC,buff,10);
	strcat(bsend,buff);
	strcat(bsend,":");
	if (bPMinC < 10) strcat(bsend,"0");
	itoa(bPMinC,buff,10);
	strcat(bsend,buff);
	strcat(bsend,", CurTime: ");
	if (bCHourC < 10) strcat(bsend,"0");
	itoa(bCHourC,buff,10);
	strcat(bsend,buff);
	strcat(bsend,":");
	if (bCMinC < 10) strcat(bsend,"0");
	itoa(bCMinC,buff,10);
	strcat(bsend,buff);
	strcat(bsend,", ");
	} else if (DEV_TYPC < 64) {
        if (!bStateC) strcat(bsend,"Off");
        else if (bStateC == 1) strcat(bsend,"Setting");
        else if (bStateC == 2) strcat(bsend,"On");
        else if (bStateC == 4) strcat(bsend,"Warming");
        else if (bStateC == 5) strcat(bsend,"DelayedStart");
        else if (bStateC == 6) strcat(bsend,"WaitPasta");
 	else if (bStateC == 254) strcat(bsend,"Offline");
	strcat(bsend,", Prog: ");
	itoa(bProgC,buff,10);
	strcat(bsend,buff);
	strcat(bsend,", Mode: ");
	itoa(bModProgC,buff,10);
	strcat(bsend,buff);
	strcat(bsend,", Temp: ");
	itoa(bHtempC,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"&deg;C, SetTime: ");
	if (bPHourC < 10) strcat(bsend,"0");
	itoa(bPHourC,buff,10);
	strcat(bsend,buff);
	strcat(bsend,":");
	if (bPMinC < 10) strcat(bsend,"0");
	itoa(bPMinC,buff,10);
	strcat(bsend,buff);
	strcat(bsend,", CurTime: ");
	if (bCHourC < 10) strcat(bsend,"0");
	itoa(bCHourC,buff,10);
	strcat(bsend,buff);
	strcat(bsend,":");
	if (bCHourC < 10) strcat(bsend,"0");
	itoa(bCMinC,buff,10);
	strcat(bsend,buff);
	strcat(bsend,", ");
	}
	} else strcat(bsend,"Not defined, ");
        strcat(bsend,"Json String:</h3><h2>{\"mqtt\":");
	itoa(mqttConnected,buff,10);
	strcat(bsend,buff);
        if (tBLEAddrC[0]) {
        strcat(bsend,",\"");
	strcat(bsend,tBLEAddrC);
        strcat(bsend,"\":{\"name\":\"");
	if (DEV_NAMEC[0]) strcat(bsend,DEV_NAMEC);
        strcat(bsend,"\",\"status\":\"");
        if (btauthorizc) {
        strcat(bsend,"online\",\"state\":");
        if (cStatusC[0]) strcat(bsend,cStatusC);
        } else strcat(bsend,"offline\"");
	}

        strcat(bsend,"}</h2><h3>System Info</h3><table class='normal'>");
        strcat(bsend,"<tr><td style='min-width:150px;'>Version</td><td style='width:80%;'>");
        strcat(bsend,AP_VER);
	uptime_string_exp(buff);
        strcat(bsend,"</td></tr><tr><td>ESP-IDF version</td><td>");
        strcat(bsend,IDF_VER);
        strcat(bsend,"</td></tr><tr><td>Local time and date</td><td>");
	strcat(bsend,strftime_buf);
        strcat(bsend,"</td></tr><tr><td>Uptime</td><td>");
	strcat(bsend,buff);
        strcat(bsend,"</td></tr><tr><td>Free memory</td><td>");
	itoa(FreeMem,buff,10);
	strcat(bsend,buff);
        strcat(bsend," bytes");
        if ((REQ_NAMEA[0] && !btauthoriza) || (REQ_NAMEB[0] && !btauthorizb) ||(REQ_NAMEC[0] && !btauthorizc)) {
        strcat(bsend,"</td></tr><tr><td>BLE last found device name</td><td>");
        strcat(bsend,FND_NAME);
        strcat(bsend,"</td></tr><tr><td>BLE activity</td><td>");
	if (btconnecta && !btauthoriza) {
	strcat(bsend,"Connecting ");
	strcat(bsend, REQ_NAMEA);
        if (!btopena) strcat(bsend," (Open");
	else {
	strcat(bsend," (Auth");
	if (DEV_TYPA > 63) {
	strcat(bsend,"/Id:");
	itoa(MiAKettleID,buff,10);
	strcat(bsend,buff);
	}
	}
        strcat(bsend,")");
	} else if (btconnectb && !btauthorizb) {
	strcat(bsend,"Connecting ");
	strcat(bsend, REQ_NAMEB);
        if (!btopenb) strcat(bsend," (Open");
	else {
	strcat(bsend," (Auth");
	if (DEV_TYPB > 63) {
	strcat(bsend,"/Id:");
	itoa(MiBKettleID,buff,10);
	strcat(bsend,buff);
	}
	}
        strcat(bsend,")");
	} else if (btconnectc && !btauthorizc) {
	strcat(bsend,"Connecting ");
	strcat(bsend, REQ_NAMEC);
        if (!btopenc) strcat(bsend," (Open");
	else {
	strcat(bsend," (Auth");
	if (DEV_TYPC > 63) {
	strcat(bsend,"/Id:");
	itoa(MiCKettleID,buff,10);
	strcat(bsend,buff);
	}
	}
        strcat(bsend,")");
	} else if (Isscanning) strcat(bsend,"Scanning");
	else strcat(bsend,"Idle");
	}
        strcat(bsend,"</td></tr><tr><td>BLE 1 connection count/state</td><td>");
        itoa(NumConnA,buff,10);
	strcat(bsend,buff);
        (btauthoriza)? strcat(bsend,"/Connected") : strcat(bsend,"/Disconnected");
	if ((btauthoriza) && (iRssiA)) {
        strcat(bsend,"</td></tr><tr><td>BLE 1 RSSI</td><td>");
        itoa(iRssiA,buff,10);
	strcat(bsend,buff);
        strcat(bsend," dB");
	}

        strcat(bsend,"</td></tr><tr><td>BLE 2 connection count/state</td><td>");
        itoa(NumConnB,buff,10);
	strcat(bsend,buff);
        (btauthorizb)? strcat(bsend,"/Connected") : strcat(bsend,"/Disconnected");
	if ((btauthorizb) && (iRssiB)) {
        strcat(bsend,"</td></tr><tr><td>BLE 2 RSSI</td><td>");
        itoa(iRssiB,buff,10);
	strcat(bsend,buff);
        strcat(bsend," dB");
	}

        strcat(bsend,"</td></tr><tr><td>BLE 3 connection count/state</td><td>");
        itoa(NumConnC,buff,10);
	strcat(bsend,buff);
        (btauthorizc)? strcat(bsend,"/Connected") : strcat(bsend,"/Disconnected");
	if ((btauthorizc) && (iRssiC)) {
        strcat(bsend,"</td></tr><tr><td>BLE 3 RSSI</td><td>");
        itoa(iRssiC,buff,10);
	strcat(bsend,buff);
        strcat(bsend," dB");
	}

        strcat(bsend,"</td></tr>");
	wifi_ap_record_t wifidata;
        memset(wifidata.ssid,0,31);
	if (esp_wifi_sta_get_ap_info(&wifidata)==0){
        strcat(bsend,"<tr><td>WiFi network</td><td>");
	memcpy(buff,wifidata.ssid,31);
        strcat(bsend,buff);
	strcat(bsend,"</td></tr><tr><td>WiFi RSSI</td><td>");
        itoa(wifidata.rssi,buff,10);
	strcat(bsend,buff);
        strcat(bsend," dB</td></tr><tr><td>WiFi IP address</td><td>");
        strcat(bsend,bufip);
	strcat(bsend,"</td></tr>");
	}
	strcat(bsend,"<tr><td>MQTT server:port/state</td><td>");
        if (MQTT_SERVER[0]) strcat(bsend,MQTT_SERVER);
	strcat(bsend,":");
	itoa(mqtt_port,buff,10);
	strcat(bsend,buff);
        (mqttConnected)? strcat(bsend,"/Connected") : strcat(bsend,"/Disconnected");
#ifdef USE_TFT
	strcat(bsend,"</td></tr>");
	strcat(bsend,"<tr><td>ILI9341 320*240 LCD</td><td>");
	if (!tft_conf) strcat(bsend,"Disabled");
	else (tft_conn)? strcat(bsend,"Connected") : strcat(bsend,"Disconnected");
#endif
	strcat(bsend,"</td></tr></table><br><footer><h6>More info on <a href='https://github.com/alutov/ESP32-R4sGate-for-Redmond' style='font-size: 15px; text-decoration: none'>github.com/alutov</a></h6>");

//itoa(strlen(bsend),buff,10);
//strcat(bsend,buff);
//	strcat(bsend,"<meta http-equiv=\"refresh\" content=\"5;URL=http://");
//	strcat(bsend,bufip);
//	strcat(bsend,"/\"</body></html>");

	strcat(bsend,"<meta http-equiv=\"refresh\" content=\"5\"></body></html>");
        httpd_resp_sendstr(req, bsend);
    return ESP_OK;
}
static const httpd_uri_t pmain = {
	.uri       = "/",
	.method    = HTTP_GET,
	.handler   = pmain_get_handler,
	.user_ctx  = NULL
};

/* HTTP GET device1 handler */
static esp_err_t pcfgdev1_get_handler(httpd_req_t *req)
{
	char bsend[12000];
        char buff[64];
	strcpy(bsend,"<!DOCTYPE html><html>");
	strcat(bsend,"<head><title>r4sGate</title>");
        strcat(bsend,"<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
        strcat(bsend,cssDatasheet);
	strcat(bsend,"</head><body class='bodymenu'><header class='headermenu'><h1>ESP32 r4sGate");
	if (R4SNUM)  {
	itoa(R4SNUM,buff,10);
	strcat(bsend, buff);
	}
	strcat(bsend," for Redmond +</h1>");
	strcat(bsend,"<div class='menubar'><a class='menu' href='.'>&#8962;<span class='showmenulabel'>Main");
	strcat(bsend,"</span></a><a class='menu active' href='cfgdev1'>&#128246;<span class='showmenulabel'> 1 ");
	(REQ_NAMEA[0])? strcat(bsend,REQ_NAMEA) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev2'>&#128246;<span class='showmenulabel'> 2 ");
	(REQ_NAMEB[0])? strcat(bsend,REQ_NAMEB) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev3'>&#128246;<span class='showmenulabel'> 3 ");
	(REQ_NAMEC[0])? strcat(bsend,REQ_NAMEC) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='setting'>&#9881;<span class='showmenulabel'>Setting</span></a>");
	strcat(bsend,"<a class='menu' href='restart'>&#128259;<span class='showmenulabel'>Reboot</span></a>");
	strcat(bsend,"<a class='menu' href='update'>&#10548;<span class='showmenulabel'>Load firmware</span></a></div>");

	strcat(bsend,"</header><table class='normal'><h3>1 ");
	(REQ_NAMEA[0])? strcat(bsend,REQ_NAMEA) : strcat(bsend,"Not defined");
	if ((DEV_TYPA > 0) && (DEV_TYPA < 10)) strcat(bsend," Kettle");
	if ((DEV_TYPA > 9) && (DEV_TYPA < 12)) strcat(bsend," Power");
	if ((DEV_TYPA > 11) && (DEV_TYPA < 16)) strcat(bsend," Coffee");
	if ((DEV_TYPA > 15) && (DEV_TYPA < 24)) strcat(bsend," Cooker");
	if ((DEV_TYPA > 23) && (DEV_TYPA < 64)) strcat(bsend," Oven");
	if ((DEV_TYPA > 63) && (DEV_TYPA < 70)) strcat(bsend," Kettle");
	strcat(bsend," Control</h3><br/>");
	if (!REQ_NAMEA[0] || !DEV_TYPA || !btauthoriza) {
	if (REQ_NAMEA[0] && DEV_TYPA && (DEV_TYPA < 128) && !btauthoriza) {
	if ((DEV_TYPA > 0) && (DEV_TYPA < 10)) strcat(bsend,"Kettle ");
	if ((DEV_TYPA > 9) && (DEV_TYPA < 12)) strcat(bsend,"Power ");
	if ((DEV_TYPA > 11) && (DEV_TYPA < 16)) strcat(bsend,"Coffee ");
	if ((DEV_TYPA > 15) && (DEV_TYPA < 24)) strcat(bsend,"Cooker ");
	if ((DEV_TYPA > 23) && (DEV_TYPA < 64)) strcat(bsend,"Oven ");
	if ((DEV_TYPA > 63) && (DEV_TYPA < 70)) strcat(bsend,"Kettle ");
	strcat(bsend,REQ_NAMEA);
	strcat(bsend," not connected</br>");
	} else strcat(bsend,"Device not defined</br>");
	strcat(bsend,"<body><form method=\"POST\" action=\"/setignore\">");
	} else if (DEV_TYPA < 10) {
	strcat(bsend,"<body><form method=\"POST\" action=\"/cfgdev1ok\">");
	strcat(bsend,"<select name=\"sstate\"><option ");
        strcat(bsend,"value=\"1\">Switch Off</option><option ");
	if (!bStateA) strcat(bsend,"selected ");
        strcat(bsend,"value=\"2\">Boil On</option><option ");
        strcat(bsend,"value=\"3\">Heat On</option><option ");
	strcat(bsend,"value=\"4\">Boil&Heat On</option><option ");
	strcat(bsend,"value=\"5\">NightLight On</option><option ");
	strcat(bsend,"value=\"21\">BackLight Off</option><option ");
	strcat(bsend,"value=\"22\">BackLight On</option><option ");
	strcat(bsend,"value=\"23\">Beep Off</option><option ");
	strcat(bsend,"value=\"24\">Beep On</option></select>Select state</br>");
	strcat(bsend,"<input name=\"stemp\" type=\"number\" value=\"0\" min=\"0\" max=\"95\" size=\"2\">Heat temp 0-95&deg;C, if 0 heat off or boil only</br>");
	strcat(bsend,"<input name=\"rlight\" type=\"number\" value=\"");
	itoa(RgbRA,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"255\" size=\"3\">NightLight Red</br><input name=\"glight\" type=\"number\" value=\"");
	itoa(RgbGA,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"255\" size=\"3\">NightLight Green</br><input name=\"blight\" type=\"number\" value=\"");
	itoa(RgbBA,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"255\" size=\"3\">NightLight Blue</br></br>");
	} else if ( DEV_TYPA > 63) {
	if ((bModProgA == 2) && (xshedcoma != 2)) {
	strcat(bsend,"<body><form method=\"POST\" action=\"/cfgdev1ok\">");
	strcat(bsend,"<select name=\"sstate\"><option ");
        strcat(bsend,"value=\"64\">Switch Off</option><option ");
	if (!bStateA) strcat(bsend,"selected ");
	strcat(bsend,"value=\"65\">Boil On</option><option ");
        strcat(bsend,"value=\"66\">Heat On</option></select>Select state</br>");
	strcat(bsend,"<input name=\"stemp\" type=\"number\" value=\"40\" min=\"0\" max=\"95\" size=\"2\">Heat temp 40-95&deg;C, if 0 heat off</br></br>");
	} else { strcat(bsend,"Press \"Warm\" on MiKettle to remote device control</br>");
	strcat(bsend,"<body><form method=\"POST\" action=\"/setignore\">");
	}
	} else if ( DEV_TYPA < 12) {
	strcat(bsend,"<body><form method=\"POST\" action=\"/cfgdev1ok\">");
	strcat(bsend,"<select name=\"sstate\"><option ");
	strcat(bsend,"value=\"1\">Switch Off</option><option ");
	if (!bStateA) strcat(bsend,"selected ");
	strcat(bsend,"value=\"6\">Switch On</option><option ");
	strcat(bsend,"value=\"7\">Lock Off</option><option ");
	strcat(bsend,"value=\"8\">Lock On</option></select>Select state</br>");
	} else if ( DEV_TYPA < 16) {
	strcat(bsend,"<body><form method=\"POST\" action=\"/cfgdev1ok\">");
	strcat(bsend,"<select name=\"sstate\"><option ");
        strcat(bsend,"value=\"1\">Switch Off</option><option ");
	if (!bStateA) strcat(bsend,"selected ");
	strcat(bsend,"value=\"6\">Switch On</option><option ");
	strcat(bsend,"value=\"18\">Delayed Start</option><option ");
	strcat(bsend,"value=\"7\">Lock Off</option><option ");
	strcat(bsend,"value=\"8\">Lock On</option></select>Select state</br>");
	strcat(bsend,"<select name=\"smod\"><option ");
	if (!bProgA) strcat(bsend,"selected ");
        strcat(bsend,"value=\"1\">Coffee Strength Off</option><option ");
	if (bProgA) strcat(bsend,"selected ");
	strcat(bsend,"value=\"2\">Coffee Strength On</option></select>Set coffee strength</br>");
	strcat(bsend,"<input name=\"sphour\" type=\"number\" value=\"");
	itoa(bPHourA,buff,10);
	strcat(bsend,buff);
        strcat(bsend,"\" min=\"0\" max=\"23\" size=\"2\">Set Hour</br>");
	strcat(bsend,"<input name=\"spmin\" type=\"number\" value=\"");
	itoa(bPMinA,buff,10);
	strcat(bsend,buff);
        strcat(bsend,"\" min=\"0\" max=\"59\" size=\"2\">Set Min</br>");
	} else if ( DEV_TYPA < 64) {
	if (bProgA < 24) {
	if ((bDHourA != bCHourA) || (bDMinA != bCMinA)) {
	bDHourA = 0;
	bDMinA = 0;
	}
	}
	strcat(bsend,"<body><form method=\"POST\" action=\"/cfgdev1ok\">");
	strcat(bsend,"<select name=\"sstate\"><option ");
        strcat(bsend,"value=\"11\">Switch Off</option><option ");
        if ((DEV_TYPA < 24) && (bProgA > 127)) strcat(bsend,"value=\"12\">Warming On</option><option ");
	if (!bStateA || (bStateA ==1)) strcat(bsend,"selected ");
        strcat(bsend,"value=\"13\">Set Program</option>");
	if (bProgA < 128) strcat(bsend,"<option value=\"14\">Start Program</option></select>Select state</br>");
	else strcat(bsend,"</select>Select state</br>");
	strcat(bsend,"<select name=\"sprog\"><option ");
	if (bProgA > 127) strcat(bsend,"selected ");
        strcat(bsend,"value=\"255\">Not defined</option><option ");
        if ( DEV_TYPA == 16 ) {
// for RMC-800s
	if (bProgA == 0) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Multicooker / Мультиповар</option><option ");
	if (bProgA == 1) strcat(bsend,"selected ");
        strcat(bsend,"value=\"1\">Rice / Рис Крупы (0)</option><option ");
	if (bProgA == 2) strcat(bsend,"selected ");
        strcat(bsend,"value=\"2\">Slow cooking / Томление (0)</option><option ");
	if (bProgA == 3) strcat(bsend,"selected ");
        strcat(bsend,"value=\"3\">Pilaf / Плов (0)</option><option ");
	if (bProgA == 4) strcat(bsend,"selected ");
        strcat(bsend,"value=\"4\">Frying / Жарка (1-3)</option><option ");
	if (bProgA == 5) strcat(bsend,"selected ");
        strcat(bsend,"value=\"5\">Stewing / Тушение (1-3)</option><option ");
	if (bProgA == 6) strcat(bsend,"selected ");
        strcat(bsend,"value=\"6\">Pasta / Макароны (0)</option><option ");
	if (bProgA == 7) strcat(bsend,"selected ");
        strcat(bsend,"value=\"7\">Milk porridge / Молочная каша (0)</option><option ");
	if (bProgA == 8) strcat(bsend,"selected ");
        strcat(bsend,"value=\"8\">Soup / Суп (0)</option><option ");
	if (bProgA == 9) strcat(bsend,"selected ");
        strcat(bsend,"value=\"9\">Yogurt / Йогурт (0)</option><option ");
	if (bProgA == 10) strcat(bsend,"selected ");
        strcat(bsend,"value=\"10\">Baking / Выпечка (0)</option><option ");
	if (bProgA == 11) strcat(bsend,"selected ");
        strcat(bsend,"value=\"11\">Steam / Пар (1-3)</option><option ");
	if (bProgA == 12) strcat(bsend,"selected ");
	strcat(bsend,"value=\"12\">Hot / Варка Бобовые (0)</option></select>Select program</br>");
	strcat(bsend,"<select name=\"smod\"><option ");
	if (bModProgA == 0) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Not defined (0)</option><option ");
	if (bModProgA == 1) strcat(bsend,"selected ");
        strcat(bsend,"value=\"1\">Vegetables / Овощи (1)</option><option ");
	if (bModProgA == 2) strcat(bsend,"selected ");
        strcat(bsend,"value=\"2\">Fish / Рыба (2)<option ");
	if (bModProgA == 3) strcat(bsend,"selected ");
	strcat(bsend,"value=\"3\">Meat / Мясо (3)</option></select>Select program mode</br>");
	} else if ( DEV_TYPA == 17 ) {
// for RMC-903s
	if (bProgA == 0) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Multicooker / Мультиповар</option><option ");
	if (bProgA == 1) strcat(bsend,"selected ");
        strcat(bsend,"value=\"1\">Milk porridge / Молочная каша</option><option ");
	if (bProgA == 2) strcat(bsend,"selected ");
        strcat(bsend,"value=\"2\">Stewing / Тушение</option><option  ");
	if (bProgA == 3) strcat(bsend,"selected ");
        strcat(bsend,"value=\"3\">Frying / Жарка</option><option ");
	if (bProgA == 4) strcat(bsend,"selected ");
        strcat(bsend,"value=\"4\">Soup / Суп</option><option ");
	if (bProgA == 5) strcat(bsend,"selected ");
        strcat(bsend,"value=\"5\">Steam / Пар</option><option ");
	if (bProgA == 6) strcat(bsend,"selected ");
        strcat(bsend,"value=\"6\">Pasta / Макароны</option><option ");
	if (bProgA == 7) strcat(bsend,"selected ");
        strcat(bsend,"value=\"7\">Slow cooking / Томление</option><option ");
	if (bProgA == 8) strcat(bsend,"selected ");
        strcat(bsend,"value=\"8\">Hot / Варка</option><option ");
	if (bProgA == 9) strcat(bsend,"selected ");
        strcat(bsend,"value=\"9\">Baking / Выпечка</option><option ");
	if (bProgA == 10) strcat(bsend,"selected ");
        strcat(bsend,"value=\"10\">Groats / Крупы</option><option ");
	if (bProgA == 11) strcat(bsend,"selected ");
        strcat(bsend,"value=\"11\">Pilaf / Плов</option><option ");
	if (bProgA == 12) strcat(bsend,"selected ");
        strcat(bsend,"value=\"12\">Yogurt / Йогурт</option><option ");
	if (bProgA == 13) strcat(bsend,"selected ");
        strcat(bsend,"value=\"13\">Pizza / Пицца</option><option ");
	if (bProgA == 14) strcat(bsend,"selected ");
        strcat(bsend,"value=\"14\">Bread / Хлеб</option><option ");
	if (bProgA == 15) strcat(bsend,"selected ");
	strcat(bsend,"value=\"15\">Desserts / Десерты</option><option ");
	if (bProgA == 16) strcat(bsend,"selected ");
	strcat(bsend,"value=\"16\">Express / Экспресс</option></select>Select program</br>");
	strcat(bsend,"<select name=\"smod\"><option ");
	strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Not defined</option></select>Select program mode</br>");
	} else if ( DEV_TYPA == 18 ) {
// for RMC-224s
	if (bProgA == 0) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Frying / Жарка</option><option ");
	if (bProgA == 1) strcat(bsend,"selected ");
        strcat(bsend,"value=\"1\">Groats / Крупы</option><option ");
	if (bProgA == 2) strcat(bsend,"selected ");
        strcat(bsend,"value=\"2\">Multicooker / Мультиповар</option><option ");
	if (bProgA == 3) strcat(bsend,"selected ");
        strcat(bsend,"value=\"3\">Pilaf / Плов</option><option ");
	if (bProgA == 4) strcat(bsend,"selected ");
        strcat(bsend,"value=\"4\">Steam / Пар</option><option ");
	if (bProgA == 5) strcat(bsend,"selected ");
        strcat(bsend,"value=\"5\">Baking / Выпечка</option><option ");
	if (bProgA == 6) strcat(bsend,"selected ");
        strcat(bsend,"value=\"6\">Stewing / Тушение</option><option  ");
	if (bProgA == 7) strcat(bsend,"selected ");
        strcat(bsend,"value=\"7\">Soup / Суп</option><option ");
	if (bProgA == 8) strcat(bsend,"selected ");
        strcat(bsend,"value=\"8\">Milk porridge / Молочная каша</option><option ");
	if (bProgA == 9) strcat(bsend,"selected ");
        strcat(bsend,"value=\"9\">Yogurt / Йогурт</option><option ");
	if (bProgA == 10) strcat(bsend,"selected ");
	strcat(bsend,"value=\"10\">Express / Экспресс</option></select>Select program</br>");
	strcat(bsend,"<select name=\"smod\"><option ");
	strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Not defined</option></select>Select program mode</br>");
	} else if ( DEV_TYPA == 24 ) {
	if (bProgA == 0) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Multicooker / Мультиповар</option><option ");
	if (bProgA == 1) strcat(bsend,"selected ");
        strcat(bsend,"value=\"1\">Omelet / Омлет</option><option ");
	if (bProgA == 2) strcat(bsend,"selected ");
        strcat(bsend,"value=\"2\">Slow cooking meat / Томление мясо</option><option  ");
	if (bProgA == 3) strcat(bsend,"selected ");
        strcat(bsend,"value=\"3\">Slow cooking bird / Томление птица</option><option ");
	if (bProgA == 4) strcat(bsend,"selected ");
        strcat(bsend,"value=\"4\">Slow cooking fish / Томление  рыба</option><option ");
	if (bProgA == 5) strcat(bsend,"selected ");
        strcat(bsend,"value=\"5\">Slow cooking vegetables / Томление овощи</option><option ");
	if (bProgA == 6) strcat(bsend,"selected ");
        strcat(bsend,"value=\"6\">Bread / Хлеб</option><option ");
	if (bProgA == 7) strcat(bsend,"selected ");
        strcat(bsend,"value=\"7\">Pizza / Пицца</option><option ");
	if (bProgA == 8) strcat(bsend,"selected ");
        strcat(bsend,"value=\"8\">Charlotte / Шарлотка</option><option ");
	if (bProgA == 9) strcat(bsend,"selected ");
        strcat(bsend,"value=\"9\">Baking meat in pot / Запекание в горшочке мясо</option><option ");
	if (bProgA == 10) strcat(bsend,"selected ");
        strcat(bsend,"value=\"10\">Baking bird in pot / Запекание в горшочке птица</option><option ");
	if (bProgA == 11) strcat(bsend,"selected ");
        strcat(bsend,"value=\"11\">Baking fish in pot / Запекание в горошочке рыба</option><option ");
	if (bProgA == 12) strcat(bsend,"selected ");
        strcat(bsend,"value=\"12\">Baking vegetables in pot / Запекание в горшочке овощи</option><option ");
	if (bProgA == 13) strcat(bsend,"selected ");
        strcat(bsend,"value=\"13\">Roast / Жаркое</option><option ");
	if (bProgA == 14) strcat(bsend,"selected ");
        strcat(bsend,"value=\"14\">Cake / Кекс</option><option ");
	if (bProgA == 15) strcat(bsend,"selected ");
	strcat(bsend,"value=\"15\">Baking meat / Запекание мясо</option><option ");
	if (bProgA == 16) strcat(bsend,"selected ");
	strcat(bsend,"value=\"16\">Baking bird / Запекание птица</option><option ");
	if (bProgA == 17) strcat(bsend,"selected ");
	strcat(bsend,"value=\"17\">Baking fish / Запекание рыба</option><option ");
	if (bProgA == 18) strcat(bsend,"selected ");
	strcat(bsend,"value=\"18\">Baking vegetables / Запекание овощи</option><option ");
	if (bProgA == 19) strcat(bsend,"selected ");
	strcat(bsend,"value=\"19\">Boiled pork / Буженина</option><option ");
	if (bProgA == 20) strcat(bsend,"selected ");
	strcat(bsend,"value=\"20\">Warming / Разогрев</option></select>Select program</br>");
	strcat(bsend,"<select name=\"smod\"><option ");
	if (bModProgA == 0) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Top heater</option><option ");
	if (bModProgA == 1) strcat(bsend,"selected ");
        strcat(bsend,"value=\"1\">Bottom heater<option ");
	if (bModProgA == 2) strcat(bsend,"selected ");
	strcat(bsend,"value=\"2\">Top and bottom heaters</option></select>Select heating mode</br>");
	}
	strcat(bsend,"<input name=\"stemp\" type=\"number\" value=\"");
	itoa(bHtempA,buff,10);
	strcat(bsend,buff);
        strcat(bsend,"\" min=\"0\" max=\"230\" size=\"3\">Set Temp 0-230&deg;C</br>");
	strcat(bsend,"<input name=\"sphour\" type=\"number\" value=\"");
	itoa(bPHourA,buff,10);
	strcat(bsend,buff);
        strcat(bsend,"\" min=\"0\" max=\"23\" size=\"2\">Set Hour</br>");
	strcat(bsend,"<input name=\"spmin\" type=\"number\" value=\"");
	itoa(bPMinA,buff,10);
	strcat(bsend,buff);
        strcat(bsend,"\" min=\"0\" max=\"59\" size=\"2\">Set Min</br>");
	strcat(bsend,"<input name=\"sdhour\" type=\"number\" value=\"");
	itoa(bDHourA,buff,10);
	strcat(bsend,buff);
        strcat(bsend,"\" min=\"0\" max=\"23\" size=\"2\">Set Delay Hour</br>");
	strcat(bsend,"<input name=\"sdmin\" type=\"number\" value=\"");
	itoa(bDMinA,buff,10);
	strcat(bsend,buff);
        strcat(bsend,"\" min=\"0\" max=\"59\" size=\"2\">Set Delay Min</br>");
	strcat(bsend,"<select name=\"swarm\"><option ");
	if (!bAwarmA) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Auto Warming Off</option><option ");
	if (bAwarmA) strcat(bsend,"selected ");
	strcat(bsend,"value=\"1\">Auto Warming On</option></select>Set auto warming</br>");
	}
	strcat(bsend,"<h3> Store values then press Ok. </h3><br/>");
	strcat(bsend,"<input type=SUBMIT value=\"Ok\">");
	strcat(bsend,"</form><form method=\"POST\" action=\"/setignore\">");
	strcat(bsend,"<input type=SUBMIT value=\"Cancel\"></form></body></html>");

//itoa(strlen(bsend),buff,10);
//strcat(bsend,buff);

        httpd_resp_sendstr(req, bsend);
    return ESP_OK;
}
static const httpd_uri_t pcfgdev1 = {
	.uri       = "/cfgdev1",
	.method    = HTTP_GET,
	.handler   = pcfgdev1_get_handler,
	.user_ctx  = NULL
};

static esp_err_t pcfgdev1ok_get_handler(httpd_req_t *req)
{
	char buf1[512] = {0};
	char buf2[16] = {0};
	char buf3[16] = {0};
	int  ret;
	int  cm_done = bProgA;
	ret = httpd_req_recv(req,buf1,512);
	if ( ret > 0 ) {
//ESP_LOGI(AP_TAG, "Buf: '%s'", buf1);
/*

in buf1 after httpd_req_recv string like below
swfid=wifiname&swfpsw=wifipassword&smqsrv=192.168.1.10&smqid=esp&
smqpsw=esp&devnam=&rlight=255&glight=255&blight=255&chk2=2

*/
	if (DEV_TYPA < 10) {
	buf3[0] = 0;
	strcpy(buf2,"rlight");
	parsuri(buf1,buf3,buf2,512,4);
	RgbRA = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"glight");
	parsuri(buf1,buf3,buf2,512,4);
	RgbGA = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"blight");
	parsuri(buf1,buf3,buf2,512,4);
	RgbBA = atoi(buf3);
	}
	buf3[0] = 0;
	strcpy(buf2,"stemp");
	parsuri(buf1,buf3,buf2,512,4);
	uint8_t temp = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"sstate");
	parsuri(buf1,buf3,buf2,512,4);
	uint8_t state = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"sprog");
	parsuri(buf1,buf3,buf2,512,4);
	uint8_t pprog = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"smod");
	parsuri(buf1,buf3,buf2,512,4);
	uint8_t pmod = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"sphour");
	parsuri(buf1,buf3,buf2,512,4);
	uint8_t phour = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"spmin");
	parsuri(buf1,buf3,buf2,512,4);
	uint8_t pmin = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"sdhour");
	parsuri(buf1,buf3,buf2,512,4);
	uint8_t dhour = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"sdmin");
	parsuri(buf1,buf3,buf2,512,4);
	uint8_t dmin = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"swarm");
	parsuri(buf1,buf3,buf2,512,4);
	uint8_t warm = atoi(buf3);
	switch (state) {    //kettle
	case 1:             //off
	cm_done = 1;
	r4slppar1a = pmod;		
	r4slpcoma = 1;
	break;
	case 2:             //boil(on)
	cm_done = 1;
	r4slppar1a = 0;
	r4slpcoma = 2;
	break;
	case 3:             //heat(on)
	cm_done = 1;
	if (DEV_TYPA == 1) {	
	if (temp < 30) r4slppar1a = 0;
	else if (temp < 41) r4slppar1a = 1;
	else if (temp < 56) r4slppar1a = 2;
	else if (temp < 71) r4slppar1a = 3;
	else if (temp < 86) r4slppar1a = 4;
	else r4slppar1a = 5;
	} else r4slppar1a = temp;
	if (temp > 29) {
	r4slppar2a = 1;
	r4slpcoma = 3;
	} else {
	r4slppar1a = 0;
	r4slpcoma = 1;
	}
	break;
	case 4:             //boil&heat(on)
	cm_done = 1;
	if (DEV_TYPA == 1) {	
	if (temp < 30) r4slppar1a = 0;
	else if (temp < 41) r4slppar1a = 1;
	else if (temp < 56) r4slppar1a = 2;
	else if (temp < 71) r4slppar1a = 3;
	else if (temp < 86) r4slppar1a = 4;
	else r4slppar1a = 5;
	} else r4slppar1a = temp;
	r4slpcoma = 4;
	break;
	case 5:             //nightlight(on)
	cm_done = 1;
	r4slppar1a = 0;
	r4slpcoma = 5;
	break;
	case 6:             //power coffee on
	cm_done = 1;
	r4slppar1a = pmod;
	r4slpcoma = 6;
	break;
	case 7:             //lock off
	cm_done = 1;
	r4slppar1a = 0;
	r4slpcoma = 7;
	break;
	case 8:             //lock on
	cm_done = 1;
	r4slppar1a = 0;
	r4slpcoma = 8;
	break;
	case 11:             //cooker off
	cm_done = 1;
	r4slppar1a = 0;		
	r4slpcoma = 10;
	break;
	case 12:             //cooker warming
	cm_done = 1;
	r4slppar1a = 1;		
	r4slpcoma = 10;
	break;
	case 13:             //cooker set prog
	cm_done = 0;
	r4slppar1a = pprog;		
	r4slppar2a = pmod;		
	r4slppar3a = temp;		
	r4slppar4a = phour;		
	r4slppar5a = pmin;		
	r4slppar6a = warm;		
	r4slppar7a = dhour;		
	r4slppar8a = dmin;
	r4slpcoma = 17;
	break;
	case 14:             //cooker warming
	cm_done = 1;
	r4slppar1a = 1;		
	r4slpcoma = 10;
	break;
	case 18:             //coffee delay start
	cm_done = 1;
	r4slppar1a = pmod;
	r4slppar2a = phour;
	r4slppar3a = pmin;
	r4slpcoma = 18;
	break;
	case 21:             //blight off
	cm_done = 1;
	r4slppar1a = 0;
	r4slpcoma = 21;
	break;
	case 22:             //blight on
	cm_done = 1;
	r4slppar1a = 0;
	r4slpcoma = 22;
	break;
	case 23:             //beep off
	cm_done = 1;
	r4slppar1a = 0;
	r4slpcoma = 23;
	break;
	case 24:             //beep on
	cm_done = 1;
	r4slppar1a = 0;
	r4slpcoma = 24;
	break;
	case 64:             //mi off
	cm_done = 1;
	r4slppar1a = 0;		
	r4slpcoma = 64;
	break;
	case 65:             //mi boil(on)
	cm_done = 1;
	r4slppar1a = 0;
	r4slpcoma = 65;
	break;
	case 66:             //mi heat(on)
	cm_done = 1;
	r4slppar1a = temp;
	r4slpcoma = 66;
	break;
	}

	}
	r4slpresa = 1;
	uint8_t timeout = 35; 	// 100*35 = 3.5 seconds
	while (--timeout && (r4slpresa)) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	httpd_resp_set_status(req, "303 See Other");
	if (!cm_done) httpd_resp_set_hdr(req, "Location", "/cfgdev1");
	else httpd_resp_set_hdr(req, "Location", "/");
	httpd_resp_send(req, NULL, 0);  // Response body can be empty
    return ESP_OK;
}
static const httpd_uri_t pcfgdev1ok = {
	.uri       = "/cfgdev1ok",
	.method    = HTTP_POST,
	.handler   = pcfgdev1ok_get_handler,
	.user_ctx  = NULL
};


/* HTTP GET device2 handler */
static esp_err_t pcfgdev2_get_handler(httpd_req_t *req)
{
	char bsend[12000];
        char buff[64];
	strcpy(bsend,"<!DOCTYPE html><html>");
	strcat(bsend,"<head><title>r4sGate</title>");
        strcat(bsend,"<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
        strcat(bsend,cssDatasheet);
	strcat(bsend,"</head><body class='bodymenu'><header class='headermenu'><h1>ESP32 r4sGate");
	if (R4SNUM)  {
	itoa(R4SNUM,buff,10);
	strcat(bsend, buff);
	}
	strcat(bsend," for Redmond +</h1>");
	strcat(bsend,"<div class='menubar'><a class='menu' href='.'>&#8962;<span class='showmenulabel'>Main");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev1'>&#128246;<span class='showmenulabel'> 1 ");
	(REQ_NAMEA[0])? strcat(bsend,REQ_NAMEA) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu active' href='cfgdev2'>&#128246;<span class='showmenulabel'> 2 ");
	(REQ_NAMEB[0])? strcat(bsend,REQ_NAMEB) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev3'>&#128246;<span class='showmenulabel'> 3 ");
	(REQ_NAMEC[0])? strcat(bsend,REQ_NAMEC) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='setting'>&#9881;<span class='showmenulabel'>Setting</span></a>");
	strcat(bsend,"<a class='menu' href='restart'>&#128259;<span class='showmenulabel'>Reboot</span></a>");
	strcat(bsend,"<a class='menu' href='update'>&#10548;<span class='showmenulabel'>Load firmware</span></a></div>");

	strcat(bsend,"</header><table class='normal'><h3>2 ");
	(REQ_NAMEB[0])? strcat(bsend,REQ_NAMEB) : strcat(bsend,"Not defined");
	if ((DEV_TYPB > 0) && (DEV_TYPB < 10)) strcat(bsend," Kettle");
	if ((DEV_TYPB > 9) && (DEV_TYPB < 12)) strcat(bsend," Power");
	if ((DEV_TYPB > 11) && (DEV_TYPB < 16)) strcat(bsend," Coffee");
	if ((DEV_TYPB > 15) && (DEV_TYPB < 24)) strcat(bsend," Cooker");
	if ((DEV_TYPB > 23) && (DEV_TYPB < 64)) strcat(bsend," Oven");
	if ((DEV_TYPB > 63) && (DEV_TYPB < 70)) strcat(bsend," Kettle");
	strcat(bsend," Control</h3><br/>");
	if (!REQ_NAMEB[0] || !DEV_TYPB || !btauthorizb) {
	if (REQ_NAMEB[0] && DEV_TYPB && (DEV_TYPB < 128) && !btauthorizb) {
	if ((DEV_TYPB > 0) && (DEV_TYPB < 10)) strcat(bsend,"Kettle ");
	if ((DEV_TYPB > 9) && (DEV_TYPB < 12)) strcat(bsend,"Power ");
	if ((DEV_TYPB > 11) && (DEV_TYPB < 16)) strcat(bsend,"Coffee ");
	if ((DEV_TYPB > 15) && (DEV_TYPB < 24)) strcat(bsend,"Cooker ");
	if ((DEV_TYPB > 23) && (DEV_TYPB < 64)) strcat(bsend,"Oven ");
	if ((DEV_TYPB > 63) && (DEV_TYPB < 70)) strcat(bsend,"Kettle ");
	strcat(bsend,REQ_NAMEB);
	strcat(bsend," not connected</br>");
	} else strcat(bsend,"Device not defined</br>");
	strcat(bsend,"<body><form method=\"POST\" action=\"/setignore\">");
	} else if (DEV_TYPB < 10) {
	strcat(bsend,"<body><form method=\"POST\" action=\"/cfgdev2ok\">");
	strcat(bsend,"<select name=\"sstate\"><option ");
        strcat(bsend,"value=\"1\">Switch Off</option><option ");
	if (!bStateB) strcat(bsend,"selected ");
        strcat(bsend,"value=\"2\">Boil On</option><option ");
        strcat(bsend,"value=\"3\">Heat On</option><option ");
	strcat(bsend,"value=\"4\">Boil&Heat On</option><option ");
	strcat(bsend,"value=\"5\">NightLight On</option><option ");
	strcat(bsend,"value=\"21\">BackLight Off</option><option ");
	strcat(bsend,"value=\"22\">BackLight On</option><option ");
	strcat(bsend,"value=\"23\">Beep Off</option><option ");
	strcat(bsend,"value=\"24\">Beep On</option></select>Select state</br>");
	strcat(bsend,"<input name=\"stemp\" type=\"number\" value=\"0\" min=\"0\" max=\"95\" size=\"2\">Heat temp 0-95&deg;C, if 0 heat off or boil only</br>");
	strcat(bsend,"<input name=\"rlight\" type=\"number\" value=\"");
	itoa(RgbRB,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"255\" size=\"3\">NightLight Red</br><input name=\"glight\" type=\"number\" value=\"");
	itoa(RgbGB,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"255\" size=\"3\">NightLight Green</br><input name=\"blight\" type=\"number\" value=\"");
	itoa(RgbBB,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"255\" size=\"3\">NightLight Blue</br></br>");
	} else if ( DEV_TYPB > 63) {
	if ((bModProgB == 2) && (xshedcomb != 2)) {
	strcat(bsend,"<body><form method=\"POST\" action=\"/cfgdev2ok\">");
	strcat(bsend,"<select name=\"sstate\"><option ");
        strcat(bsend,"value=\"64\">Switch Off</option><option ");
	if (!bStateB) strcat(bsend,"selected ");
	strcat(bsend,"value=\"65\">Boil On</option><option ");
        strcat(bsend,"value=\"66\">Heat On</option></select>Select state</br>");
	strcat(bsend,"<input name=\"stemp\" type=\"number\" value=\"40\" min=\"0\" max=\"95\" size=\"2\">Heat temp 40-95&deg;C, if 0 heat off</br></br>");
	} else { strcat(bsend,"Press \"Warm\" on MiKettle to remote device control</br>");
	strcat(bsend,"<body><form method=\"POST\" action=\"/setignore\">");
	}
	} else if ( DEV_TYPB < 12) {
	strcat(bsend,"<body><form method=\"POST\" action=\"/cfgdev2ok\">");
	strcat(bsend,"<select name=\"sstate\"><option ");
	strcat(bsend,"value=\"1\">Switch Off</option><option ");
	if (!bStateB) strcat(bsend,"selected ");
	strcat(bsend,"value=\"6\">Switch On</option><option ");
	strcat(bsend,"value=\"7\">Lock Off</option><option ");
	strcat(bsend,"value=\"8\">Lock On</option></select>Select state</br>");
	} else if ( DEV_TYPB < 16) {
	strcat(bsend,"<body><form method=\"POST\" action=\"/cfgdev2ok\">");
	strcat(bsend,"<select name=\"sstate\"><option ");
        strcat(bsend,"value=\"1\">Switch Off</option><option ");
	if (!bStateB) strcat(bsend,"selected ");
	strcat(bsend,"value=\"6\">Switch On</option><option ");
	strcat(bsend,"value=\"18\">Delayed Start</option><option ");
	strcat(bsend,"value=\"7\">Lock Off</option><option ");
	strcat(bsend,"value=\"8\">Lock On</option></select>Select state</br>");
	strcat(bsend,"<select name=\"smod\"><option ");
	if (!bProgB) strcat(bsend,"selected ");
        strcat(bsend,"value=\"1\">Coffee Strength Off</option><option ");
	if (bProgB) strcat(bsend,"selected ");
	strcat(bsend,"value=\"2\">Coffee Strength On</option></select>Set coffee strength</br>");
	strcat(bsend,"<input name=\"sphour\" type=\"number\" value=\"");
	itoa(bPHourB,buff,10);
	strcat(bsend,buff);
        strcat(bsend,"\" min=\"0\" max=\"23\" size=\"2\">Set Hour</br>");
	strcat(bsend,"<input name=\"spmin\" type=\"number\" value=\"");
	itoa(bPMinB,buff,10);
	strcat(bsend,buff);
        strcat(bsend,"\" min=\"0\" max=\"59\" size=\"2\">Set Min</br>");
	} else if ( DEV_TYPB < 64) {
	if (bProgB < 24) {
	if ((bDHourB != bCHourB) || (bDMinB != bCMinB)) {
	bDHourB = 0;
	bDMinB = 0;
	}
	}
	strcat(bsend,"<body><form method=\"POST\" action=\"/cfgdev2ok\">");
	strcat(bsend,"<select name=\"sstate\"><option ");
        strcat(bsend,"value=\"11\">Switch Off</option><option ");
        if ((DEV_TYPB < 24) && (bProgB > 127)) strcat(bsend,"value=\"12\">Warming On</option><option ");
	if (!bStateB || (bStateB ==1)) strcat(bsend,"selected ");
        strcat(bsend,"value=\"13\">Set Program</option>");
	if (bProgB < 128) strcat(bsend,"<option value=\"14\">Start Program</option></select>Select state</br>");
	else strcat(bsend,"</select>Select state</br>");
	strcat(bsend,"<select name=\"sprog\"><option ");
	if (bProgB > 127) strcat(bsend,"selected ");
        strcat(bsend,"value=\"255\">Not defined</option><option ");
        if ( DEV_TYPB == 16 ) {
// for RMC-800s
	if (bProgB == 0) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Multicooker / Мультиповар</option><option ");
	if (bProgB == 1) strcat(bsend,"selected ");
        strcat(bsend,"value=\"1\">Rice / Рис Крупы (0)</option><option ");
	if (bProgB == 2) strcat(bsend,"selected ");
        strcat(bsend,"value=\"2\">Slow cooking / Томление (0)</option><option ");
	if (bProgB == 3) strcat(bsend,"selected ");
        strcat(bsend,"value=\"3\">Pilaf / Плов (0)</option><option ");
	if (bProgB == 4) strcat(bsend,"selected ");
        strcat(bsend,"value=\"4\">Frying / Жарка (1-3)</option><option ");
	if (bProgB == 5) strcat(bsend,"selected ");
        strcat(bsend,"value=\"5\">Stewing / Тушение (1-3)</option><option ");
	if (bProgB == 6) strcat(bsend,"selected ");
        strcat(bsend,"value=\"6\">Pasta / Макароны (0)</option><option ");
	if (bProgB == 7) strcat(bsend,"selected ");
        strcat(bsend,"value=\"7\">Milk porridge / Молочная каша (0)</option><option ");
	if (bProgB == 8) strcat(bsend,"selected ");
        strcat(bsend,"value=\"8\">Soup / Суп (0)</option><option ");
	if (bProgB == 9) strcat(bsend,"selected ");
        strcat(bsend,"value=\"9\">Yogurt / Йогурт (0)</option><option ");
	if (bProgB == 10) strcat(bsend,"selected ");
        strcat(bsend,"value=\"10\">Baking / Выпечка (0)</option><option ");
	if (bProgB == 11) strcat(bsend,"selected ");
        strcat(bsend,"value=\"11\">Steam / Пар (1-3)</option><option ");
	if (bProgB == 12) strcat(bsend,"selected ");
	strcat(bsend,"value=\"12\">Hot / Варка Бобовые (0)</option></select>Select program</br>");
	strcat(bsend,"<select name=\"smod\"><option ");
	if (bModProgB == 0) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Not defined (0)</option><option ");
	if (bModProgB == 1) strcat(bsend,"selected ");
        strcat(bsend,"value=\"1\">Vegetables / Овощи (1)</option><option ");
	if (bModProgB == 2) strcat(bsend,"selected ");
        strcat(bsend,"value=\"2\">Fish / Рыба (2)<option ");
	if (bModProgB == 3) strcat(bsend,"selected ");
	strcat(bsend,"value=\"3\">Meat / Мясо (3)</option></select>Select program mode</br>");
	} else if ( DEV_TYPB == 17 ) {
// for RMC-903s
	if (bProgB == 0) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Multicooker / Мультиповар</option><option ");
	if (bProgB == 1) strcat(bsend,"selected ");
        strcat(bsend,"value=\"1\">Milk porridge / Молочная каша</option><option ");
	if (bProgB == 2) strcat(bsend,"selected ");
        strcat(bsend,"value=\"2\">Stewing / Тушение</option><option  ");
	if (bProgB == 3) strcat(bsend,"selected ");
        strcat(bsend,"value=\"3\">Frying / Жарка</option><option ");
	if (bProgB == 4) strcat(bsend,"selected ");
        strcat(bsend,"value=\"4\">Soup / Суп</option><option ");
	if (bProgB == 5) strcat(bsend,"selected ");
        strcat(bsend,"value=\"5\">Steam / Пар</option><option ");
	if (bProgB == 6) strcat(bsend,"selected ");
        strcat(bsend,"value=\"6\">Pasta / Макароны</option><option ");
	if (bProgB == 7) strcat(bsend,"selected ");
        strcat(bsend,"value=\"7\">Slow cooking / Томление</option><option ");
	if (bProgB == 8) strcat(bsend,"selected ");
        strcat(bsend,"value=\"8\">Hot / Варка</option><option ");
	if (bProgB == 9) strcat(bsend,"selected ");
        strcat(bsend,"value=\"9\">Baking / Выпечка</option><option ");
	if (bProgB == 10) strcat(bsend,"selected ");
        strcat(bsend,"value=\"10\">Groats / Крупы</option><option ");
	if (bProgB == 11) strcat(bsend,"selected ");
        strcat(bsend,"value=\"11\">Pilaf / Плов</option><option ");
	if (bProgB == 12) strcat(bsend,"selected ");
        strcat(bsend,"value=\"12\">Yogurt / Йогурт</option><option ");
	if (bProgB == 13) strcat(bsend,"selected ");
        strcat(bsend,"value=\"13\">Pizza / Пицца</option><option ");
	if (bProgB == 14) strcat(bsend,"selected ");
        strcat(bsend,"value=\"14\">Bread / Хлеб</option><option ");
	if (bProgB == 15) strcat(bsend,"selected ");
	strcat(bsend,"value=\"15\">Desserts / Десерты</option><option ");
	if (bProgB == 16) strcat(bsend,"selected ");
	strcat(bsend,"value=\"16\">Express / Экспресс</option></select>Select program</br>");
	strcat(bsend,"<select name=\"smod\"><option ");
	strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Not defined</option></select>Select program mode</br>");
	} else if ( DEV_TYPB == 18 ) {
// for RMC-224s
	if (bProgB == 0) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Frying / Жарка</option><option ");
	if (bProgB == 1) strcat(bsend,"selected ");
        strcat(bsend,"value=\"1\">Groats / Крупы</option><option ");
	if (bProgB == 2) strcat(bsend,"selected ");
        strcat(bsend,"value=\"2\">Multicooker / Мультиповар</option><option ");
	if (bProgB == 3) strcat(bsend,"selected ");
        strcat(bsend,"value=\"3\">Pilaf / Плов</option><option ");
	if (bProgB == 4) strcat(bsend,"selected ");
        strcat(bsend,"value=\"4\">Steam / Пар</option><option ");
	if (bProgB == 5) strcat(bsend,"selected ");
        strcat(bsend,"value=\"5\">Baking / Выпечка</option><option ");
	if (bProgB == 6) strcat(bsend,"selected ");
        strcat(bsend,"value=\"6\">Stewing / Тушение</option><option  ");
	if (bProgB == 7) strcat(bsend,"selected ");
        strcat(bsend,"value=\"7\">Soup / Суп</option><option ");
	if (bProgB == 8) strcat(bsend,"selected ");
        strcat(bsend,"value=\"8\">Milk porridge / Молочная каша</option><option ");
	if (bProgB == 9) strcat(bsend,"selected ");
        strcat(bsend,"value=\"9\">Yogurt / Йогурт</option><option ");
	if (bProgB == 10) strcat(bsend,"selected ");
	strcat(bsend,"value=\"10\">Express / Экспресс</option></select>Select program</br>");
	strcat(bsend,"<select name=\"smod\"><option ");
	strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Not defined</option></select>Select program mode</br>");
	} else if ( DEV_TYPB == 24 ) {
	if (bProgB == 0) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Multicooker / Мультиповар</option><option ");
	if (bProgB == 1) strcat(bsend,"selected ");
        strcat(bsend,"value=\"1\">Omelet / Омлет</option><option ");
	if (bProgB == 2) strcat(bsend,"selected ");
        strcat(bsend,"value=\"2\">Slow cooking meat / Томление мясо</option><option  ");
	if (bProgB == 3) strcat(bsend,"selected ");
        strcat(bsend,"value=\"3\">Slow cooking bird / Томление птица</option><option ");
	if (bProgB == 4) strcat(bsend,"selected ");
        strcat(bsend,"value=\"4\">Slow cooking fish / Томление  рыба</option><option ");
	if (bProgB == 5) strcat(bsend,"selected ");
        strcat(bsend,"value=\"5\">Slow cooking vegetables / Томление овощи</option><option ");
	if (bProgB == 6) strcat(bsend,"selected ");
        strcat(bsend,"value=\"6\">Bread / Хлеб</option><option ");
	if (bProgB == 7) strcat(bsend,"selected ");
        strcat(bsend,"value=\"7\">Pizza / Пицца</option><option ");
	if (bProgB == 8) strcat(bsend,"selected ");
        strcat(bsend,"value=\"8\">Charlotte / Шарлотка</option><option ");
	if (bProgB == 9) strcat(bsend,"selected ");
        strcat(bsend,"value=\"9\">Baking meat in pot / Запекание в горшочке мясо</option><option ");
	if (bProgB == 10) strcat(bsend,"selected ");
        strcat(bsend,"value=\"10\">Baking bird in pot / Запекание в горшочке птица</option><option ");
	if (bProgB == 11) strcat(bsend,"selected ");
        strcat(bsend,"value=\"11\">Baking fish in pot / Запекание в горошочке рыба</option><option ");
	if (bProgB == 12) strcat(bsend,"selected ");
        strcat(bsend,"value=\"12\">Baking vegetables in pot / Запекание в горшочке овощи</option><option ");
	if (bProgB == 13) strcat(bsend,"selected ");
        strcat(bsend,"value=\"13\">Roast / Жаркое</option><option ");
	if (bProgB == 14) strcat(bsend,"selected ");
        strcat(bsend,"value=\"14\">Cake / Кекс</option><option ");
	if (bProgB == 15) strcat(bsend,"selected ");
	strcat(bsend,"value=\"15\">Baking meat / Запекание мясо</option><option ");
	if (bProgB == 16) strcat(bsend,"selected ");
	strcat(bsend,"value=\"16\">Baking bird / Запекание птица</option><option ");
	if (bProgB == 17) strcat(bsend,"selected ");
	strcat(bsend,"value=\"17\">Baking fish / Запекание рыба</option><option ");
	if (bProgB == 18) strcat(bsend,"selected ");
	strcat(bsend,"value=\"18\">Baking vegetables / Запекание овощи</option><option ");
	if (bProgB == 19) strcat(bsend,"selected ");
	strcat(bsend,"value=\"19\">Boiled pork / Буженина</option><option ");
	if (bProgB == 20) strcat(bsend,"selected ");
	strcat(bsend,"value=\"20\">Warming / Разогрев</option></select>Select program</br>");
	strcat(bsend,"<select name=\"smod\"><option ");
	if (bModProgB == 0) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Top heater</option><option ");
	if (bModProgB == 1) strcat(bsend,"selected ");
        strcat(bsend,"value=\"1\">Bottom heater<option ");
	if (bModProgB == 2) strcat(bsend,"selected ");
	strcat(bsend,"value=\"2\">Top and bottom heaters</option></select>Select heating mode</br>");
	}
	strcat(bsend,"<input name=\"stemp\" type=\"number\" value=\"");
	itoa(bHtempB,buff,10);
	strcat(bsend,buff);
        strcat(bsend,"\" min=\"0\" max=\"230\" size=\"3\">Set Temp 0-230&deg;C</br>");
	strcat(bsend,"<input name=\"sphour\" type=\"number\" value=\"");
	itoa(bPHourB,buff,10);
	strcat(bsend,buff);
        strcat(bsend,"\" min=\"0\" max=\"23\" size=\"2\">Set Hour</br>");
	strcat(bsend,"<input name=\"spmin\" type=\"number\" value=\"");
	itoa(bPMinB,buff,10);
	strcat(bsend,buff);
        strcat(bsend,"\" min=\"0\" max=\"59\" size=\"2\">Set Min</br>");
	strcat(bsend,"<input name=\"sdhour\" type=\"number\" value=\"");
	itoa(bDHourB,buff,10);
	strcat(bsend,buff);
        strcat(bsend,"\" min=\"0\" max=\"23\" size=\"2\">Set Delay Hour</br>");
	strcat(bsend,"<input name=\"sdmin\" type=\"number\" value=\"");
	itoa(bDMinB,buff,10);
	strcat(bsend,buff);
        strcat(bsend,"\" min=\"0\" max=\"59\" size=\"2\">Set Delay Min</br>");
	strcat(bsend,"<select name=\"swarm\"><option ");
	if (!bAwarmB) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Auto Warming Off</option><option ");
	if (bAwarmB) strcat(bsend,"selected ");
	strcat(bsend,"value=\"1\">Auto Warming On</option></select>Set auto warming</br>");
	}
	strcat(bsend,"<h3> Store values then press Ok. </h3><br/>");
	strcat(bsend,"<input type=SUBMIT value=\"Ok\">");
	strcat(bsend,"</form><form method=\"POST\" action=\"/setignore\">");
	strcat(bsend,"<input type=SUBMIT value=\"Cancel\"></form></body></html>");

//itoa(strlen(bsend),buff,10);
//strcat(bsend,buff);

        httpd_resp_sendstr(req, bsend);
    return ESP_OK;
}
static const httpd_uri_t pcfgdev2 = {
	.uri       = "/cfgdev2",
	.method    = HTTP_GET,
	.handler   = pcfgdev2_get_handler,
	.user_ctx  = NULL
};

static esp_err_t pcfgdev2ok_get_handler(httpd_req_t *req)
{
	char buf1[512] = {0};
	char buf2[16] = {0};
	char buf3[16] = {0};
	int  ret;
	int  cm_done = bProgB;
	ret = httpd_req_recv(req,buf1,512);
	if ( ret > 0 ) {
//ESP_LOGI(AP_TAG, "Buf: '%s'", buf1);
/*

in buf1 after httpd_req_recv string like below
swfid=wifiname&swfpsw=wifipassword&smqsrv=192.168.1.10&smqid=esp&
smqpsw=esp&devnam=&rlight=255&glight=255&blight=255&chk2=2

*/
	if (DEV_TYPB < 10) {
	buf3[0] = 0;
	strcpy(buf2,"rlight");
	parsuri(buf1,buf3,buf2,512,4);
	RgbRB = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"glight");
	parsuri(buf1,buf3,buf2,512,4);
	RgbGB = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"blight");
	parsuri(buf1,buf3,buf2,512,4);
	RgbBB = atoi(buf3);
	}
	buf3[0] = 0;
	strcpy(buf2,"stemp");
	parsuri(buf1,buf3,buf2,512,4);
	uint8_t temp = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"sstate");
	parsuri(buf1,buf3,buf2,512,4);
	uint8_t state = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"sprog");
	parsuri(buf1,buf3,buf2,512,4);
	uint8_t pprog = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"smod");
	parsuri(buf1,buf3,buf2,512,4);
	uint8_t pmod = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"sphour");
	parsuri(buf1,buf3,buf2,512,4);
	uint8_t phour = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"spmin");
	parsuri(buf1,buf3,buf2,512,4);
	uint8_t pmin = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"sdhour");
	parsuri(buf1,buf3,buf2,512,4);
	uint8_t dhour = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"sdmin");
	parsuri(buf1,buf3,buf2,512,4);
	uint8_t dmin = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"swarm");
	parsuri(buf1,buf3,buf2,512,4);
	uint8_t warm = atoi(buf3);
	switch (state) {    //kettle
	case 1:             //off
	cm_done = 1;
	r4slppar1b = pmod;		
	r4slpcomb = 1;
	break;
	case 2:             //boil(on)
	cm_done = 1;
	r4slppar1b = 0;
	r4slpcomb = 2;
	break;
	case 3:             //heat(on)
	cm_done = 1;
	if (DEV_TYPB == 1) {	
	if (temp < 30) r4slppar1b = 0;
	else if (temp < 41) r4slppar1b = 1;
	else if (temp < 56) r4slppar1b = 2;
	else if (temp < 71) r4slppar1b = 3;
	else if (temp < 86) r4slppar1b = 4;
	else r4slppar1b = 5;
	} else r4slppar1b = temp;
	if (temp > 29) {
	r4slppar2b = 1;
	r4slpcomb = 3;
	} else {
	r4slppar1b = 0;
	r4slpcomb = 1;
	}
	break;
	case 4:             //boil&heat(on)
	cm_done = 1;
	if (DEV_TYPB == 1) {	
	if (temp < 30) r4slppar1b = 0;
	else if (temp < 41) r4slppar1b = 1;
	else if (temp < 56) r4slppar1b = 2;
	else if (temp < 71) r4slppar1b = 3;
	else if (temp < 86) r4slppar1b = 4;
	else r4slppar1b = 5;
	} else r4slppar1b = temp;
	r4slpcomb = 4;
	break;
	case 5:             //nightlight(on)
	cm_done = 1;
	r4slppar1b = 0;
	r4slpcomb = 5;
	break;
	case 6:             //power coffee on
	cm_done = 1;
	r4slppar1b = pmod;
	r4slpcomb = 6;
	break;
	case 7:             //lock off
	cm_done = 1;
	r4slppar1b = 0;
	r4slpcomb = 7;
	break;
	case 8:             //lock on
	cm_done = 1;
	r4slppar1b = 0;
	r4slpcomb = 8;
	break;
	case 11:             //cooker off
	cm_done = 1;
	r4slppar1b = 0;		
	r4slpcomb = 10;
	break;
	case 12:             //cooker warming
	cm_done = 1;
	r4slppar1b = 1;		
	r4slpcomb = 10;
	break;
	case 13:             //cooker set prog
	cm_done = 0;
	r4slppar1b = pprog;		
	r4slppar2b = pmod;		
	r4slppar3b = temp;		
	r4slppar4b = phour;		
	r4slppar5b = pmin;		
	r4slppar6b = warm;		
	r4slppar7b = dhour;		
	r4slppar8b = dmin;
	r4slpcomb = 17;
	break;
	case 14:             //cooker warming
	cm_done = 1;
	r4slppar1b = 1;		
	r4slpcomb = 10;
	break;
	case 18:             //coffee delay start
	cm_done = 1;
	r4slppar1b = pmod;
	r4slppar2b = phour;
	r4slppar3b = pmin;
	r4slpcomb = 18;
	break;
	case 21:             //blight off
	cm_done = 1;
	r4slppar1b = 0;
	r4slpcomb = 21;
	break;
	case 22:             //blight on
	cm_done = 1;
	r4slppar1b = 0;
	r4slpcomb = 22;
	break;
	case 23:             //beep off
	cm_done = 1;
	r4slppar1b = 0;
	r4slpcomb = 23;
	break;
	case 24:             //beep on
	cm_done = 1;
	r4slppar1b = 0;
	r4slpcomb = 24;
	break;
	case 64:             //mi off
	cm_done = 1;
	r4slppar1b = 0;		
	r4slpcomb = 64;
	break;
	case 65:             //mi boil(on)
	cm_done = 1;
	r4slppar1b = 0;
	r4slpcomb = 65;
	break;
	case 66:             //mi heat(on)
	cm_done = 1;
	r4slppar1b = temp;
	r4slpcomb = 66;
	break;
	}

	}
	r4slpresb = 1;
	uint8_t timeout = 35; 	// 100*35 = 3.5 seconds
	while (--timeout && (r4slpresb)) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	httpd_resp_set_status(req, "303 See Other");
	if (!cm_done) httpd_resp_set_hdr(req, "Location", "/cfgdev2");
	else httpd_resp_set_hdr(req, "Location", "/");
	httpd_resp_send(req, NULL, 0);  // Response body can be empty
    return ESP_OK;
}
static const httpd_uri_t pcfgdev2ok = {
	.uri       = "/cfgdev2ok",
	.method    = HTTP_POST,
	.handler   = pcfgdev2ok_get_handler,
	.user_ctx  = NULL
};


/* HTTP GET device3 handler */
static esp_err_t pcfgdev3_get_handler(httpd_req_t *req)
{
	char bsend[12000];
        char buff[32];
	strcpy(bsend,"<!DOCTYPE html><html>");
	strcat(bsend,"<head><title>r4sGate</title>");
        strcat(bsend,"<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
        strcat(bsend,cssDatasheet);
	strcat(bsend,"</head><body class='bodymenu'><header class='headermenu'><h1>ESP32 r4sGate");
	if (R4SNUM)  {
	itoa(R4SNUM,buff,10);
	strcat(bsend, buff);
	}
	strcat(bsend," for Redmond +</h1>");
	strcat(bsend,"<div class='menubar'><a class='menu' href='.'>&#8962;<span class='showmenulabel'>Main");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev1'>&#128246;<span class='showmenulabel'> 1 ");
	(REQ_NAMEA[0])? strcat(bsend,REQ_NAMEA) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev2'>&#128246;<span class='showmenulabel'> 2 ");
	(REQ_NAMEB[0])? strcat(bsend,REQ_NAMEB) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu active' href='cfgdev3'>&#128246;<span class='showmenulabel'> 3 ");
	(REQ_NAMEC[0])? strcat(bsend,REQ_NAMEC) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='setting'>&#9881;<span class='showmenulabel'>Setting</span></a>");
	strcat(bsend,"<a class='menu' href='restart'>&#128259;<span class='showmenulabel'>Reboot</span></a>");
	strcat(bsend,"<a class='menu' href='update'>&#10548;<span class='showmenulabel'>Load firmware</span></a></div>");

	strcat(bsend,"</header><table class='normal'><h3>3 ");
	(REQ_NAMEC[0])? strcat(bsend,REQ_NAMEC) : strcat(bsend,"Not defined");
	if ((DEV_TYPC > 0) && (DEV_TYPC < 10)) strcat(bsend," Kettle");
	if ((DEV_TYPC > 9) && (DEV_TYPC < 12)) strcat(bsend," Power");
	if ((DEV_TYPC > 11) && (DEV_TYPC < 16)) strcat(bsend," Coffee");
	if ((DEV_TYPC > 15) && (DEV_TYPC < 24)) strcat(bsend," Cooker");
	if ((DEV_TYPC > 23) && (DEV_TYPC < 64)) strcat(bsend," Oven");
	if ((DEV_TYPC > 63) && (DEV_TYPC < 70)) strcat(bsend," Kettle");
	strcat(bsend," Control</h3><br/>");
	if (!REQ_NAMEC[0] || !DEV_TYPC || !btauthorizc) {
	if (REQ_NAMEC[0] && DEV_TYPC && (DEV_TYPC < 128) && !btauthorizc) {
	if ((DEV_TYPC > 0) && (DEV_TYPC < 10)) strcat(bsend,"Kettle ");
	if ((DEV_TYPC > 9) && (DEV_TYPC < 12)) strcat(bsend,"Power ");
	if ((DEV_TYPC > 11) && (DEV_TYPC < 16)) strcat(bsend,"Coffee ");
	if ((DEV_TYPC > 15) && (DEV_TYPC < 24)) strcat(bsend,"Cooker ");
	if ((DEV_TYPC > 23) && (DEV_TYPC < 64)) strcat(bsend,"Oven ");
	if ((DEV_TYPC > 63) && (DEV_TYPC < 70)) strcat(bsend,"Kettle ");
	strcat(bsend,REQ_NAMEC);
	strcat(bsend," not connected</br>");
	} else strcat(bsend,"Device not defined</br>");
	strcat(bsend,"<body><form method=\"POST\" action=\"/setignore\">");
	} else if (DEV_TYPC < 10) {
	strcat(bsend,"<body><form method=\"POST\" action=\"/cfgdev3ok\">");
	strcat(bsend,"<select name=\"sstate\"><option ");
        strcat(bsend,"value=\"1\">Switch Off</option><option ");
	if (!bStateC) strcat(bsend,"selected ");
        strcat(bsend,"value=\"2\">Boil On</option><option ");
        strcat(bsend,"value=\"3\">Heat On</option><option ");
	strcat(bsend,"value=\"4\">Boil&Heat On</option><option ");
	strcat(bsend,"value=\"5\">NightLight On</option><option ");
	strcat(bsend,"value=\"21\">BackLight Off</option><option ");
	strcat(bsend,"value=\"22\">BackLight On</option><option ");
	strcat(bsend,"value=\"23\">Beep Off</option><option ");
	strcat(bsend,"value=\"24\">Beep On</option></select>Select state</br>");
	strcat(bsend,"<input name=\"stemp\" type=\"number\" value=\"0\" min=\"0\" max=\"95\" size=\"2\">Heat temp 0-95&deg;C, if 0 heat off or boil only</br>");
	strcat(bsend,"<input name=\"rlight\" type=\"number\" value=\"");
	itoa(RgbRC,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"255\" size=\"3\">NightLight Red</br><input name=\"glight\" type=\"number\" value=\"");
	itoa(RgbGC,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"255\" size=\"3\">NightLight Green</br><input name=\"blight\" type=\"number\" value=\"");
	itoa(RgbBC,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"255\" size=\"3\">NightLight Blue</br></br>");
	} else if ( DEV_TYPC > 63) {
	if ((bModProgC == 2) && (xshedcomc != 2)) {
	strcat(bsend,"<body><form method=\"POST\" action=\"/cfgdev3ok\">");
	strcat(bsend,"<select name=\"sstate\"><option ");
        strcat(bsend,"value=\"64\">Switch Off</option><option ");
	if (!bStateC) strcat(bsend,"selected ");
	strcat(bsend,"value=\"65\">Boil On</option><option ");
        strcat(bsend,"value=\"66\">Heat On</option></select>Select state</br>");
	strcat(bsend,"<input name=\"stemp\" type=\"number\" value=\"40\" min=\"0\" max=\"95\" size=\"2\">Heat temp 40-95&deg;C, if 0 heat off</br></br>");
	} else { strcat(bsend,"Press \"Warm\" on MiKettle to remote device control</br>");
	strcat(bsend,"<body><form method=\"POST\" action=\"/setignore\">");
	}
	} else if ( DEV_TYPC < 12) {
	strcat(bsend,"<body><form method=\"POST\" action=\"/cfgdev3ok\">");
	strcat(bsend,"<select name=\"sstate\"><option ");
	strcat(bsend,"value=\"1\">Switch Off</option><option ");
	if (!bStateC) strcat(bsend,"selected ");
	strcat(bsend,"value=\"6\">Switch On</option><option ");
	strcat(bsend,"value=\"7\">Lock Off</option><option ");
	strcat(bsend,"value=\"8\">Lock On</option></select>Select state</br>");
	} else if ( DEV_TYPC < 16) {
	strcat(bsend,"<body><form method=\"POST\" action=\"/cfgdev3ok\">");
	strcat(bsend,"<select name=\"sstate\"><option ");
        strcat(bsend,"value=\"1\">Switch Off</option><option ");
	if (!bStateC) strcat(bsend,"selected ");
	strcat(bsend,"value=\"6\">Switch On</option><option ");
	strcat(bsend,"value=\"18\">Delayed Start</option><option ");
	strcat(bsend,"value=\"7\">Lock Off</option><option ");
	strcat(bsend,"value=\"8\">Lock On</option></select>Select state</br>");
	strcat(bsend,"<select name=\"smod\"><option ");
	if (!bProgC) strcat(bsend,"selected ");
        strcat(bsend,"value=\"1\">Coffee Strength Off</option><option ");
	if (bProgC) strcat(bsend,"selected ");
	strcat(bsend,"value=\"2\">Coffee Strength On</option></select>Set coffee strength</br>");
	strcat(bsend,"<input name=\"sphour\" type=\"number\" value=\"");
	itoa(bPHourC,buff,10);
	strcat(bsend,buff);
        strcat(bsend,"\" min=\"0\" max=\"23\" size=\"2\">Set Hour</br>");
	strcat(bsend,"<input name=\"spmin\" type=\"number\" value=\"");
	itoa(bPMinC,buff,10);
	strcat(bsend,buff);
        strcat(bsend,"\" min=\"0\" max=\"59\" size=\"2\">Set Min</br>");
	} else if ( DEV_TYPC < 64) {
	if (bProgC < 24) {
	if ((bDHourC != bCHourC) || (bDMinC != bCMinC)) {
	bDHourC = 0;
	bDMinC = 0;
	}
	}
	strcat(bsend,"<body><form method=\"POST\" action=\"/cfgdev3ok\">");
	strcat(bsend,"<select name=\"sstate\"><option ");
        strcat(bsend,"value=\"11\">Switch Off</option><option ");
        if ((DEV_TYPC < 24) && (bProgC > 127)) strcat(bsend,"value=\"12\">Warming On</option><option ");
	if (!bStateC || (bStateC ==1)) strcat(bsend,"selected ");
        strcat(bsend,"value=\"13\">Set Program</option>");
	if (bProgC < 128) strcat(bsend,"<option value=\"14\">Start Program</option></select>Select state</br>");
	else strcat(bsend,"</select>Select state</br>");
	strcat(bsend,"<select name=\"sprog\"><option ");
	if (bProgC > 127) strcat(bsend,"selected ");
        strcat(bsend,"value=\"255\">Not defined</option><option ");
        if ( DEV_TYPC == 16 ) {
// for RMC-800s
	if (bProgC == 0) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Multicooker / Мультиповар</option><option ");
	if (bProgC == 1) strcat(bsend,"selected ");
        strcat(bsend,"value=\"1\">Rice / Рис Крупы (0)</option><option ");
	if (bProgC == 2) strcat(bsend,"selected ");
        strcat(bsend,"value=\"2\">Slow cooking / Томление (0)</option><option ");
	if (bProgC == 3) strcat(bsend,"selected ");
        strcat(bsend,"value=\"3\">Pilaf / Плов (0)</option><option ");
	if (bProgC == 4) strcat(bsend,"selected ");
        strcat(bsend,"value=\"4\">Frying / Жарка (1-3)</option><option ");
	if (bProgC == 5) strcat(bsend,"selected ");
        strcat(bsend,"value=\"5\">Stewing / Тушение (1-3)</option><option ");
	if (bProgC == 6) strcat(bsend,"selected ");
        strcat(bsend,"value=\"6\">Pasta / Макароны (0)</option><option ");
	if (bProgC == 7) strcat(bsend,"selected ");
        strcat(bsend,"value=\"7\">Milk porridge / Молочная каша (0)</option><option ");
	if (bProgC == 8) strcat(bsend,"selected ");
        strcat(bsend,"value=\"8\">Soup / Суп (0)</option><option ");
	if (bProgC == 9) strcat(bsend,"selected ");
        strcat(bsend,"value=\"9\">Yogurt / Йогурт (0)</option><option ");
	if (bProgC == 10) strcat(bsend,"selected ");
        strcat(bsend,"value=\"10\">Baking / Выпечка (0)</option><option ");
	if (bProgC == 11) strcat(bsend,"selected ");
        strcat(bsend,"value=\"11\">Steam / Пар (1-3)</option><option ");
	if (bProgC == 12) strcat(bsend,"selected ");
	strcat(bsend,"value=\"12\">Hot / Варка Бобовые (0)</option></select>Select program</br>");
	strcat(bsend,"<select name=\"smod\"><option ");
	if (bModProgC == 0) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Not defined (0)</option><option ");
	if (bModProgC == 1) strcat(bsend,"selected ");
        strcat(bsend,"value=\"1\">Vegetables / Овощи (1)</option><option ");
	if (bModProgC == 2) strcat(bsend,"selected ");
        strcat(bsend,"value=\"2\">Fish / Рыба (2)<option ");
	if (bModProgC == 3) strcat(bsend,"selected ");
	strcat(bsend,"value=\"3\">Meat / Мясо (3)</option></select>Select program mode</br>");
	} else if ( DEV_TYPC == 17 ) {
// for RMC-903s
	if (bProgC == 0) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Multicooker / Мультиповар</option><option ");
	if (bProgC == 1) strcat(bsend,"selected ");
        strcat(bsend,"value=\"1\">Milk porridge / Молочная каша</option><option ");
	if (bProgC == 2) strcat(bsend,"selected ");
        strcat(bsend,"value=\"2\">Stewing / Тушение</option><option  ");
	if (bProgC == 3) strcat(bsend,"selected ");
        strcat(bsend,"value=\"3\">Frying / Жарка</option><option ");
	if (bProgC == 4) strcat(bsend,"selected ");
        strcat(bsend,"value=\"4\">Soup / Суп</option><option ");
	if (bProgC == 5) strcat(bsend,"selected ");
        strcat(bsend,"value=\"5\">Steam / Пар</option><option ");
	if (bProgC == 6) strcat(bsend,"selected ");
        strcat(bsend,"value=\"6\">Pasta / Макароны</option><option ");
	if (bProgC == 7) strcat(bsend,"selected ");
        strcat(bsend,"value=\"7\">Slow cooking / Томление</option><option ");
	if (bProgC == 8) strcat(bsend,"selected ");
        strcat(bsend,"value=\"8\">Hot / Варка</option><option ");
	if (bProgC == 9) strcat(bsend,"selected ");
        strcat(bsend,"value=\"9\">Baking / Выпечка</option><option ");
	if (bProgC == 10) strcat(bsend,"selected ");
        strcat(bsend,"value=\"10\">Groats / Крупы</option><option ");
	if (bProgC == 11) strcat(bsend,"selected ");
        strcat(bsend,"value=\"11\">Pilaf / Плов</option><option ");
	if (bProgC == 12) strcat(bsend,"selected ");
        strcat(bsend,"value=\"12\">Yogurt / Йогурт</option><option ");
	if (bProgC == 13) strcat(bsend,"selected ");
        strcat(bsend,"value=\"13\">Pizza / Пицца</option><option ");
	if (bProgC == 14) strcat(bsend,"selected ");
        strcat(bsend,"value=\"14\">Bread / Хлеб</option><option ");
	if (bProgC == 15) strcat(bsend,"selected ");
	strcat(bsend,"value=\"15\">Desserts / Десерты</option><option ");
	if (bProgC == 16) strcat(bsend,"selected ");
	strcat(bsend,"value=\"16\">Express / Экспресс</option></select>Select program</br>");
	strcat(bsend,"<select name=\"smod\"><option ");
	strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Not defined</option></select>Select program mode</br>");
	} else if ( DEV_TYPC == 18 ) {
// for RMC-224s
	if (bProgC == 0) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Frying / Жарка</option><option ");
	if (bProgC == 1) strcat(bsend,"selected ");
        strcat(bsend,"value=\"1\">Groats / Крупы</option><option ");
	if (bProgC == 2) strcat(bsend,"selected ");
        strcat(bsend,"value=\"2\">Multicooker / Мультиповар</option><option ");
	if (bProgC == 3) strcat(bsend,"selected ");
        strcat(bsend,"value=\"3\">Pilaf / Плов</option><option ");
	if (bProgC == 4) strcat(bsend,"selected ");
        strcat(bsend,"value=\"4\">Steam / Пар</option><option ");
	if (bProgC == 5) strcat(bsend,"selected ");
        strcat(bsend,"value=\"5\">Baking / Выпечка</option><option ");
	if (bProgC == 6) strcat(bsend,"selected ");
        strcat(bsend,"value=\"6\">Stewing / Тушение</option><option  ");
	if (bProgC == 7) strcat(bsend,"selected ");
        strcat(bsend,"value=\"7\">Soup / Суп</option><option ");
	if (bProgC == 8) strcat(bsend,"selected ");
        strcat(bsend,"value=\"8\">Milk porridge / Молочная каша</option><option ");
	if (bProgC == 9) strcat(bsend,"selected ");
        strcat(bsend,"value=\"9\">Yogurt / Йогурт</option><option ");
	if (bProgC == 10) strcat(bsend,"selected ");
	strcat(bsend,"value=\"10\">Express / Экспресс</option></select>Select program</br>");
	strcat(bsend,"<select name=\"smod\"><option ");
	strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Not defined</option></select>Select program mode</br>");
	} else if ( DEV_TYPC == 24 ) {
	if (bProgC == 0) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Multicooker / Мультиповар</option><option ");
	if (bProgC == 1) strcat(bsend,"selected ");
        strcat(bsend,"value=\"1\">Omelet / Омлет</option><option ");
	if (bProgC == 2) strcat(bsend,"selected ");
        strcat(bsend,"value=\"2\">Slow cooking meat / Томление мясо</option><option  ");
	if (bProgC == 3) strcat(bsend,"selected ");
        strcat(bsend,"value=\"3\">Slow cooking bird / Томление птица</option><option ");
	if (bProgC == 4) strcat(bsend,"selected ");
        strcat(bsend,"value=\"4\">Slow cooking fish / Томление  рыба</option><option ");
	if (bProgC == 5) strcat(bsend,"selected ");
        strcat(bsend,"value=\"5\">Slow cooking vegetables / Томление овощи</option><option ");
	if (bProgC == 6) strcat(bsend,"selected ");
        strcat(bsend,"value=\"6\">Bread / Хлеб</option><option ");
	if (bProgC == 7) strcat(bsend,"selected ");
        strcat(bsend,"value=\"7\">Pizza / Пицца</option><option ");
	if (bProgC == 8) strcat(bsend,"selected ");
        strcat(bsend,"value=\"8\">Charlotte / Шарлотка</option><option ");
	if (bProgC == 9) strcat(bsend,"selected ");
        strcat(bsend,"value=\"9\">Baking meat in pot / Запекание в горшочке мясо</option><option ");
	if (bProgC == 10) strcat(bsend,"selected ");
        strcat(bsend,"value=\"10\">Baking bird in pot / Запекание в горшочке птица</option><option ");
	if (bProgC == 11) strcat(bsend,"selected ");
        strcat(bsend,"value=\"11\">Baking fish in pot / Запекание в горошочке рыба</option><option ");
	if (bProgC == 12) strcat(bsend,"selected ");
        strcat(bsend,"value=\"12\">Baking vegetables in pot / Запекание в горшочке овощи</option><option ");
	if (bProgC == 13) strcat(bsend,"selected ");
        strcat(bsend,"value=\"13\">Roast / Жаркое</option><option ");
	if (bProgC == 14) strcat(bsend,"selected ");
        strcat(bsend,"value=\"14\">Cake / Кекс</option><option ");
	if (bProgC == 15) strcat(bsend,"selected ");
	strcat(bsend,"value=\"15\">Baking meat / Запекание мясо</option><option ");
	if (bProgC == 16) strcat(bsend,"selected ");
	strcat(bsend,"value=\"16\">Baking bird / Запекание птица</option><option ");
	if (bProgC == 17) strcat(bsend,"selected ");
	strcat(bsend,"value=\"17\">Baking fish / Запекание рыба</option><option ");
	if (bProgC == 18) strcat(bsend,"selected ");
	strcat(bsend,"value=\"18\">Baking vegetables / Запекание овощи</option><option ");
	if (bProgC == 19) strcat(bsend,"selected ");
	strcat(bsend,"value=\"19\">Boiled pork / Буженина</option><option ");
	if (bProgC == 20) strcat(bsend,"selected ");
	strcat(bsend,"value=\"20\">Warming / Разогрев</option></select>Select program</br>");
	strcat(bsend,"<select name=\"smod\"><option ");
	if (bModProgC == 0) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Top heater</option><option ");
	if (bModProgC == 1) strcat(bsend,"selected ");
        strcat(bsend,"value=\"1\">Bottom heater<option ");
	if (bModProgC == 2) strcat(bsend,"selected ");
	strcat(bsend,"value=\"2\">Top and bottom heaters</option></select>Select heating mode</br>");
	}
	strcat(bsend,"<input name=\"stemp\" type=\"number\" value=\"");
	itoa(bHtempC,buff,10);
	strcat(bsend,buff);
        strcat(bsend,"\" min=\"0\" max=\"230\" size=\"3\">Set Temp 0-230&deg;C</br>");
	strcat(bsend,"<input name=\"sphour\" type=\"number\" value=\"");
	itoa(bPHourC,buff,10);
	strcat(bsend,buff);
        strcat(bsend,"\" min=\"0\" max=\"23\" size=\"2\">Set Hour</br>");
	strcat(bsend,"<input name=\"spmin\" type=\"number\" value=\"");
	itoa(bPMinC,buff,10);
	strcat(bsend,buff);
        strcat(bsend,"\" min=\"0\" max=\"59\" size=\"2\">Set Min</br>");
	strcat(bsend,"<input name=\"sdhour\" type=\"number\" value=\"");
	itoa(bDHourC,buff,10);
	strcat(bsend,buff);
        strcat(bsend,"\" min=\"0\" max=\"23\" size=\"2\">Set Delay Hour</br>");
	strcat(bsend,"<input name=\"sdmin\" type=\"number\" value=\"");
	itoa(bDMinC,buff,10);
	strcat(bsend,buff);
        strcat(bsend,"\" min=\"0\" max=\"59\" size=\"2\">Set Delay Min</br>");
	strcat(bsend,"<select name=\"swarm\"><option ");
	if (!bAwarmC) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Auto Warming Off</option><option ");
	if (bAwarmC) strcat(bsend,"selected ");
	strcat(bsend,"value=\"1\">Auto Warming On</option></select>Set auto warming</br>");
	}
	strcat(bsend,"<h3> Store values then press Ok. </h3><br/>");
	strcat(bsend,"<input type=SUBMIT value=\"Ok\">");
	strcat(bsend,"</form><form method=\"POST\" action=\"/setignore\">");
	strcat(bsend,"<input type=SUBMIT value=\"Cancel\"></form></body></html>");

//itoa(strlen(bsend),buff,10);
//strcat(bsend,buff);

        httpd_resp_sendstr(req, bsend);
    return ESP_OK;
}
static const httpd_uri_t pcfgdev3 = {
	.uri       = "/cfgdev3",
	.method    = HTTP_GET,
	.handler   = pcfgdev3_get_handler,
	.user_ctx  = NULL
};


static esp_err_t pcfgdev3ok_get_handler(httpd_req_t *req)
{
	char buf1[512] = {0};
	char buf2[16] = {0};
	char buf3[16] = {0};
	int  ret;
	int  cm_done = bProgC;
	ret = httpd_req_recv(req,buf1,512);
	if ( ret > 0 ) {
//ESP_LOGI(AP_TAG, "Buf: '%s'", buf1);
/*

in buf1 after httpd_req_recv string like below
swfid=wifiname&swfpsw=wifipassword&smqsrv=192.168.1.10&smqid=esp&
smqpsw=esp&devnam=&rlight=255&glight=255&blight=255&chk2=2

*/
	if (DEV_TYPC < 10) {
	buf3[0] = 0;
	strcpy(buf2,"rlight");
	parsuri(buf1,buf3,buf2,512,4);
	RgbRC = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"glight");
	parsuri(buf1,buf3,buf2,512,4);
	RgbGC = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"blight");
	parsuri(buf1,buf3,buf2,512,4);
	RgbBC = atoi(buf3);
	}
	buf3[0] = 0;
	strcpy(buf2,"stemp");
	parsuri(buf1,buf3,buf2,512,4);
	uint8_t temp = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"sstate");
	parsuri(buf1,buf3,buf2,512,4);
	uint8_t state = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"sprog");
	parsuri(buf1,buf3,buf2,512,4);
	uint8_t pprog = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"smod");
	parsuri(buf1,buf3,buf2,512,4);
	uint8_t pmod = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"sphour");
	parsuri(buf1,buf3,buf2,512,4);
	uint8_t phour = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"spmin");
	parsuri(buf1,buf3,buf2,512,4);
	uint8_t pmin = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"sdhour");
	parsuri(buf1,buf3,buf2,512,4);
	uint8_t dhour = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"sdmin");
	parsuri(buf1,buf3,buf2,512,4);
	uint8_t dmin = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"swarm");
	parsuri(buf1,buf3,buf2,512,4);
	uint8_t warm = atoi(buf3);
	switch (state) {    //kettle
	case 1:             //off
	cm_done = 1;
	r4slppar1c = pmod;		
	r4slpcomc = 1;
	break;
	case 2:             //boil(on)
	cm_done = 1;
	r4slppar1c = 0;
	r4slpcomc = 2;
	break;
	case 3:             //heat(on)
	cm_done = 1;
	if (DEV_TYPC == 1) {	
	if (temp < 30) r4slppar1c = 0;
	else if (temp < 41) r4slppar1c = 1;
	else if (temp < 56) r4slppar1c = 2;
	else if (temp < 71) r4slppar1c = 3;
	else if (temp < 86) r4slppar1c = 4;
	else r4slppar1c = 5;
	} else r4slppar1c = temp;
	if (temp > 29) {
	r4slppar2c = 1;
	r4slpcomc = 3;
	} else {
	r4slppar1c = 0;
	r4slpcomc = 1;
	}
	break;
	case 4:             //boil&heat(on)
	cm_done = 1;
	if (DEV_TYPC == 1) {	
	if (temp < 30) r4slppar1c = 0;
	else if (temp < 41) r4slppar1c = 1;
	else if (temp < 56) r4slppar1c = 2;
	else if (temp < 71) r4slppar1c = 3;
	else if (temp < 86) r4slppar1c = 4;
	else r4slppar1c = 5;
	} else r4slppar1c = temp;
	r4slpcomc = 4;
	break;
	case 5:             //nightlight(on)
	cm_done = 1;
	r4slppar1c = 0;
	r4slpcomc = 5;
	break;
	case 6:             //power coffee on
	cm_done = 1;
	r4slppar1c = pmod;
	r4slpcomc = 6;
	break;
	case 7:             //lock off
	cm_done = 1;
	r4slppar1c = 0;
	r4slpcomc = 7;
	break;
	case 8:             //lock on
	cm_done = 1;
	r4slppar1c = 0;
	r4slpcomc = 8;
	break;
	case 11:             //cooker off
	cm_done = 1;
	r4slppar1c = 0;		
	r4slpcomc = 10;
	break;
	case 12:             //cooker warming
	cm_done = 1;
	r4slppar1c = 1;		
	r4slpcomc = 10;
	break;
	case 13:             //cooker set prog
	cm_done = 0;
	r4slppar1c = pprog;		
	r4slppar2c = pmod;		
	r4slppar3c = temp;		
	r4slppar4c = phour;		
	r4slppar5c = pmin;		
	r4slppar6c = warm;		
	r4slppar7c = dhour;		
	r4slppar8c = dmin;
	r4slpcomc = 17;
	break;
	case 14:             //cooker warming
	cm_done = 1;
	r4slppar1c = 1;		
	r4slpcomc = 10;
	break;
	case 18:             //coffee delay start
	cm_done = 1;
	r4slppar1c = pmod;
	r4slppar2c = phour;
	r4slppar3c = pmin;
	r4slpcomc = 18;
	break;
	case 21:             //blight off
	cm_done = 1;
	r4slppar1c = 0;
	r4slpcomc = 21;
	break;
	case 22:             //blight on
	cm_done = 1;
	r4slppar1c = 0;
	r4slpcomc = 22;
	break;
	case 23:             //beep off
	cm_done = 1;
	r4slppar1c = 0;
	r4slpcomc = 23;
	break;
	case 24:             //beep on
	cm_done = 1;
	r4slppar1c = 0;
	r4slpcomc = 24;
	break;
	case 64:             //mi off
	cm_done = 1;
	r4slppar1c = 0;		
	r4slpcomc = 64;
	break;
	case 65:             //mi boil(on)
	cm_done = 1;
	r4slppar1c = 0;
	r4slpcomc = 65;
	break;
	case 66:             //mi heat(on)
	cm_done = 1;
	r4slppar1c = temp;
	r4slpcomc = 66;
	break;
	}

	}
	r4slpresc = 1;
	uint8_t timeout = 35; 	// 100*35 = 3.5 seconds
	while (--timeout && (r4slpresc)) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	httpd_resp_set_status(req, "303 See Other");
	if (!cm_done) httpd_resp_set_hdr(req, "Location", "/cfgdev3");
	else httpd_resp_set_hdr(req, "Location", "/");
	httpd_resp_send(req, NULL, 0);  // Response body can be empty
    return ESP_OK;
}
static const httpd_uri_t pcfgdev3ok = {
	.uri       = "/cfgdev3ok",
	.method    = HTTP_POST,
	.handler   = pcfgdev3ok_get_handler,
	.user_ctx  = NULL
};


bool pinvalid(uint8_t pin) {
	bool retc = true;
	if (pin > 39) retc = false;
	if ((pin > 5) && (pin < 11)) retc = false;
	if ((pin > 27) && (pin < 32)) retc = false;
	if ((pin == 20) ||(pin == 24)) retc = false;
	return retc;
}

/* HTTP GET setting handler */
static esp_err_t psetting_get_handler(httpd_req_t *req)
{
	char bsend[14000];
        char buff[32];
	strcpy(bsend,"<!DOCTYPE html><html>");
	strcat(bsend,"<head><title>r4sGate</title>");
        strcat(bsend,"<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
        strcat(bsend,cssDatasheet);
	strcat(bsend,"</head><body class='bodymenu'><header class='headermenu'><h1>ESP32 r4sGate");
	if (R4SNUM)  {
	itoa(R4SNUM,buff,10);
	strcat(bsend, buff);
	}
	strcat(bsend," for Redmond +</h1>");
	strcat(bsend,"<div class='menubar'><a class='menu' href='.'>&#8962;<span class='showmenulabel'>Main");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev1'>&#128246;<span class='showmenulabel'> 1 ");
	(REQ_NAMEA[0])? strcat(bsend,REQ_NAMEA) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev2'>&#128246;<span class='showmenulabel'> 2 ");
	(REQ_NAMEB[0])? strcat(bsend,REQ_NAMEB) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev3'>&#128246;<span class='showmenulabel'> 3 ");
	(REQ_NAMEC[0])? strcat(bsend,REQ_NAMEC) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu active' href='setting'>&#9881;<span class='showmenulabel'>Setting</span></a>");
	strcat(bsend,"<a class='menu' href='restart'>&#128259;<span class='showmenulabel'>Reboot</span></a>");
	strcat(bsend,"<a class='menu' href='update'>&#10548;<span class='showmenulabel'>Load firmware</span></a></div>");
	strcat(bsend,"</header><table class='normal'><h3>Wifi Setting</h3></br><body>");
	strcat(bsend,"<form method=\"POST\" action=\"/setsave\"><input name=\"swfid\" value=\"");
	if (WIFI_SSID[0]) strcat(bsend,WIFI_SSID);
	strcat(bsend,"\" size=\"15\">SSID &emsp;<input type=\"password\" input name=\"swfpsw\" value=\"");
	if (WIFI_PASSWORD[0]) strcat(bsend,WIFI_PASSWORD);
	strcat(bsend,"\"size=\"31\">Password</br><h3>MQTT Setting</h3><br/><input name=\"smqsrv\" value=\"");
	if (MQTT_SERVER[0]) strcat(bsend,MQTT_SERVER);
	strcat(bsend,"\"size=\"19\">Server &emsp;<input name=\"smqprt\" type=\"number\" value=\"");
	itoa(mqtt_port,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"65535\" size=\"5\">Port &emsp;<input name=\"smqid\" value=\"");
	if (MQTT_USER[0]) strcat(bsend,MQTT_USER);
	strcat(bsend,"\"size=\"15\">Login &emsp;<input type=\"password\" input name=\"smqpsw\" value=\"");
	if (MQTT_PASSWORD[0]) strcat(bsend,MQTT_PASSWORD);
	strcat(bsend,"\"size=\"19\">Password</br><input type=\"checkbox\" name=\"chk1\" value=\"1\"");
	if (FDHass) strcat(bsend,"checked");
	strcat(bsend,"> Hass Discovery&emsp;<input type=\"checkbox\" name=\"chk2\" value=\"2\"");
	if (fcommtp) strcat(bsend,"checked");
	strcat(bsend,"> Common Command/Response Topics&emsp;<input type=\"checkbox\" name=\"chk3\" value=\"3\"");
	if (ftrufal) strcat(bsend,"checked");
        strcat(bsend,"> \"true/false\" Response&emsp;<input type=\"checkbox\" name=\"chk7\" value=\"7\"");
	if (foffln) strcat(bsend,"checked");
        strcat(bsend,"> \"offline\" Response</br>");
#ifdef USE_TFT
	strcat(bsend,"<input name=\"smtopp1\" value=\"");
	if (MQTT_TOPP1[0]) strcat(bsend,MQTT_TOPP1);
	strcat(bsend,"\"size=\"32\">Indoor Temp&ensp;&emsp;<input name=\"smtopp2\" value=\"");
	if (MQTT_TOPP2[0]) strcat(bsend,MQTT_TOPP2);
	strcat(bsend,"\"size=\"24\">Voltage&ensp;&emsp;<input name=\"smtopp3\" value=\"");
	if (MQTT_TOPP3[0]) strcat(bsend,MQTT_TOPP3);
	strcat(bsend,"\"size=\"24\">Current</br><input name=\"smtopp4\" value=\"");
	if (MQTT_TOPP4[0]) strcat(bsend,MQTT_TOPP4);
	strcat(bsend,"\"size=\"32\">Boiler Temp &ensp;&emsp;<input name=\"smtopp5\" value=\"");
	if (MQTT_TOPP5[0]) strcat(bsend,MQTT_TOPP5);
	strcat(bsend,"\"size=\"32\">Boiler State</br><input name=\"smtopp6\" value=\"");
	if (MQTT_TOPP6[0]) strcat(bsend,MQTT_TOPP6);
	strcat(bsend,"\"size=\"32\">Outdoor Temp &ensp;<input name=\"smtopp7\" value=\"");
	if (MQTT_TOPP7[0]) strcat(bsend,MQTT_TOPP7);
	strcat(bsend,"\"size=\"32\">Outdoor Humidity</br>");
#endif
	strcat(bsend,"<h3>Devices Setting</h3><br/>");
	strcat(bsend,"1 <input name=\"sreqnma\" value=\"");
	if (REQ_NAMEA[0]) strcat(bsend,REQ_NAMEA);
	strcat(bsend,"\" size=\"15\">Name &emsp;<select name=\"sreqtpa\"><option ");
	if (DEV_TYPA == 0) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Not defined</option><option ");
	if (DEV_TYPA == 1) strcat(bsend,"selected ");
        strcat(bsend,"value=\"1\">RK-M170S</option><option ");
	if (DEV_TYPA == 2) strcat(bsend,"selected ");
        strcat(bsend,"value=\"2\">RK-M173S</option><option ");
	if (DEV_TYPA == 3) strcat(bsend,"selected ");
        strcat(bsend,"value=\"3\">RK-G200(old)</option><option ");
	if (DEV_TYPA == 4) strcat(bsend,"selected ");
	strcat(bsend,"value=\"4\">RK-G200S</option><option ");
	if (DEV_TYPA == 5) strcat(bsend,"selected ");
	strcat(bsend,"value=\"5\">RK-G240S</option><option ");
	if (DEV_TYPA == 10) strcat(bsend,"selected ");
	strcat(bsend,"value=\"10\">RSP-103S</option><option ");
	if (DEV_TYPA == 12) strcat(bsend,"selected ");
	strcat(bsend,"value=\"12\">RCM-M1519S</option><option ");
	if (DEV_TYPA == 16) strcat(bsend,"selected ");
	strcat(bsend,"value=\"16\">RMC-M800S</option><option ");
	if (DEV_TYPA == 17) strcat(bsend,"selected ");
	strcat(bsend,"value=\"17\">RMC-M903S</option><option ");
	if (DEV_TYPA == 18) strcat(bsend,"selected ");
	strcat(bsend,"value=\"18\">RMC-M224S</option><option ");
	if (DEV_TYPA == 24) strcat(bsend,"selected ");
	strcat(bsend,"value=\"24\">RO-5707S</option><option ");
	if (DEV_TYPA == 64) strcat(bsend,"selected ");
	strcat(bsend,"value=\"64\">YM-K1501(Int)</option><option ");
	if (DEV_TYPA == 65) strcat(bsend,"selected ");
	strcat(bsend,"value=\"65\">YM-K1501(HK)</option><option ");
	if (DEV_TYPA == 66) strcat(bsend,"selected ");
	strcat(bsend,"value=\"66\">V-SK152(Int)</option><option ");
	if (DEV_TYPA == 95) strcat(bsend,"selected ");
	strcat(bsend,"value=\"95\">Mi-Unknown</option></select>Type &emsp;NightLight");
//	strcat(bsend,"value=\"66\">V-SK152(Int)</option></select>Type &emsp;NightLight");
	strcat(bsend,"<input name=\"rlighta\" type=\"number\" value=\"");
	itoa(RgbRA,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"255\" size=\"3\">Red &emsp;<input name=\"glighta\" type=\"number\" value=\"");
	itoa(RgbGA,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"255\" size=\"3\">Green &emsp;<input name=\"blighta\" type=\"number\" value=\"");
	itoa(RgbBA,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"255\" size=\"3\">Blue &emsp;<input name=\"ltempa\" type=\"number\" value=\"");
	itoa(bLtempA,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"95\" size=\"3\">Heat Temp</br>");

	strcat(bsend,"2 <input name=\"sreqnmb\" value=\"");
	if (REQ_NAMEB[0]) strcat(bsend,REQ_NAMEB);
	strcat(bsend,"\" size=\"15\">Name &emsp;<select name=\"sreqtpb\"><option ");
	if (DEV_TYPB == 0) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Not defined</option><option ");
	if (DEV_TYPB == 1) strcat(bsend,"selected ");
        strcat(bsend,"value=\"1\">RK-M170S</option><option ");
	if (DEV_TYPB == 2) strcat(bsend,"selected ");
        strcat(bsend,"value=\"2\">RK-M173S</option><option ");
	if (DEV_TYPB == 3) strcat(bsend,"selected ");
        strcat(bsend,"value=\"3\">RK-G200(old)</option><option ");
	if (DEV_TYPB == 4) strcat(bsend,"selected ");
	strcat(bsend,"value=\"4\">RK-G200S</option><option ");
	if (DEV_TYPB == 5) strcat(bsend,"selected ");
	strcat(bsend,"value=\"5\">RK-G240S</option><option ");
	if (DEV_TYPB == 10) strcat(bsend,"selected ");
	strcat(bsend,"value=\"10\">RSP-103S</option><option ");
	if (DEV_TYPB == 12) strcat(bsend,"selected ");
	strcat(bsend,"value=\"12\">RCM-M1519S</option><option ");
	if (DEV_TYPB == 16) strcat(bsend,"selected ");
	strcat(bsend,"value=\"16\">RMC-M800S</option><option ");
	if (DEV_TYPB == 17) strcat(bsend,"selected ");
	strcat(bsend,"value=\"17\">RMC-M903S</option><option ");
	if (DEV_TYPB == 18) strcat(bsend,"selected ");
	strcat(bsend,"value=\"18\">RMC-M224S</option><option ");
	if (DEV_TYPB == 24) strcat(bsend,"selected ");
	strcat(bsend,"value=\"24\">RO-5707S</option><option ");
	if (DEV_TYPB == 64) strcat(bsend,"selected ");
	strcat(bsend,"value=\"64\">YM-K1501(Int)</option><option ");
	if (DEV_TYPB == 65) strcat(bsend,"selected ");
	strcat(bsend,"value=\"65\">YM-K1501(HK)</option><option ");
	if (DEV_TYPB == 66) strcat(bsend,"selected ");
	strcat(bsend,"value=\"66\">V-SK152(Int)</option><option ");
	if (DEV_TYPB == 95) strcat(bsend,"selected ");
	strcat(bsend,"value=\"95\">Mi-Unknown</option></select>Type &emsp;NightLight");
//	strcat(bsend,"value=\"66\">V-SK152(Int)</option></select>Type &emsp;NightLight");
	strcat(bsend,"<input name=\"rlightb\" type=\"number\" value=\"");
	itoa(RgbRB,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"255\" size=\"3\">Red &emsp;<input name=\"glightb\" type=\"number\" value=\"");
	itoa(RgbGB,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"255\" size=\"3\">Green &emsp;<input name=\"blightb\" type=\"number\" value=\"");
	itoa(RgbBB,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"255\" size=\"3\">Blue &emsp;<input name=\"ltempb\" type=\"number\" value=\"");
	itoa(bLtempB,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"95\" size=\"3\">Heat Temp</br>");

	strcat(bsend,"3 <input name=\"sreqnmc\" value=\"");
	if (REQ_NAMEC[0]) strcat(bsend,REQ_NAMEC);
	strcat(bsend,"\" size=\"15\">Name &emsp;<select name=\"sreqtpc\"><option ");
	if (DEV_TYPC == 0) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Not defined</option><option ");
	if (DEV_TYPC == 1) strcat(bsend,"selected ");
        strcat(bsend,"value=\"1\">RK-M1703S</option><option ");
	if (DEV_TYPC == 2) strcat(bsend,"selected ");
        strcat(bsend,"value=\"2\">RK-M173S</option><option ");
	if (DEV_TYPC == 3) strcat(bsend,"selected ");
        strcat(bsend,"value=\"3\">RK-G200(old)</option><option ");
	if (DEV_TYPC == 4) strcat(bsend,"selected ");
	strcat(bsend,"value=\"4\">RK-G200S</option><option ");
	if (DEV_TYPC == 5) strcat(bsend,"selected ");
	strcat(bsend,"value=\"5\">RK-G240S</option><option ");
	if (DEV_TYPC == 10) strcat(bsend,"selected ");
	strcat(bsend,"value=\"10\">RSP-103S</option><option ");
	if (DEV_TYPC == 12) strcat(bsend,"selected ");
	strcat(bsend,"value=\"12\">RCM-M1519S</option><option ");
	if (DEV_TYPC == 16) strcat(bsend,"selected ");
	strcat(bsend,"value=\"16\">RMC-M800S</option><option ");
	if (DEV_TYPC == 17) strcat(bsend,"selected ");
	strcat(bsend,"value=\"17\">RMC-M903S</option><option ");
	if (DEV_TYPC == 18) strcat(bsend,"selected ");
	strcat(bsend,"value=\"18\">RMC-M224S</option><option ");
	if (DEV_TYPC == 24) strcat(bsend,"selected ");
	strcat(bsend,"value=\"24\">RO-5707S</option><option ");
	if (DEV_TYPC == 64) strcat(bsend,"selected ");
	strcat(bsend,"value=\"64\">YM-K1501(Int)</option><option ");
	if (DEV_TYPC == 65) strcat(bsend,"selected ");
	strcat(bsend,"value=\"65\">YM-K1501(HK)</option><option ");
	if (DEV_TYPC == 66) strcat(bsend,"selected ");
	strcat(bsend,"value=\"66\">V-SK152(Int)</option><option ");
	if (DEV_TYPC == 95) strcat(bsend,"selected ");
	strcat(bsend,"value=\"95\">Mi-Unknown</option></select>Type &emsp;NightLight");
//	strcat(bsend,"value=\"66\">V-SK152(Int)</option></select>Type &emsp;NightLight");
	strcat(bsend,"<input name=\"rlightc\" type=\"number\" value=\"");
	itoa(RgbRC,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"255\" size=\"3\">Red &emsp;<input name=\"glightc\" type=\"number\" value=\"");
	itoa(RgbGC,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"255\" size=\"3\">Green &emsp;<input name=\"blightc\" type=\"number\" value=\"");
	itoa(RgbBC,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"255\" size=\"3\">Blue &emsp;<input name=\"ltempc\" type=\"number\" value=\"");
	itoa(bLtempC,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"95\" size=\"3\">Heat Temp</br>");
	strcat(bsend,"<h3>System Setting</h3><br/><input name=\"r4snum\" type=\"number\" value=\"");
	itoa(R4SNUM,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"255\" size=\"3\">r4sGate Number &emsp;<input name=\"timzon\" type=\"number\" value=\"");
	uint8_t TmZn = TimeZone;
	if (TmZn >127) {
	TmZn = ~TmZn;
	TmZn++;
	strcat(bsend,"-");
	}
	itoa(TmZn,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"-12\" max=\"12\" size=\"3\">GMT Timezone &emsp;");
	strcat(bsend,"<input type=\"checkbox\" name=\"chk5\" value=\"5\"");
	if (ble_mon) strcat(bsend,"checked");
	strcat(bsend,"> BLE monitoring &emsp;");
	strcat(bsend,"<input type=\"checkbox\" name=\"mqtdel\" value=\"1\"");
	strcat(bsend,"> Delete Mqtt topics &emsp;");
	strcat(bsend,"<input type=\"checkbox\" name=\"chk0\" value=\"0\"");
	strcat(bsend,"> Format NVS area</br>");
#ifdef USE_TFT
	strcat(bsend,"<input name=\"smjpuri\" value=\"");
	if (MyHttpUri[0]) strcat(bsend,MyHttpUri);
	strcat(bsend,"\"size=\"64\">320*176 JPEG Url &emsp;<input name=\"sjpgtim\" type=\"number\" value=\"");
	itoa(jpg_time,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"65535\" size=\"5\">JPEG refresh time (sec)</br>");
	strcat(bsend,"<input type=\"checkbox\" name=\"chk6\" value=\"6\"");
	if (tft_conf) strcat(bsend,"checked");
	strcat(bsend,"> TFT: <input type=\"checkbox\" name=\"chk4\" value=\"4\"");
	if (tft_flip) strcat(bsend,"checked");
	strcat(bsend,"> flip 180&deg; &ensp;GPIO<input name=\"pnmiso\" type=\"number\" value=\"");
	itoa(PIN_NUM_MISO,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"33\" size=\"2\">MISO <input name=\"pnmosi\" type=\"number\" value=\"");
	itoa(PIN_NUM_MOSI,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"33\" size=\"2\">MOSI <input name=\"pnclk\" type=\"number\" value=\"");
	itoa(PIN_NUM_CLK,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"33\" size=\"2\">CLK <input name=\"pncs\" type=\"number\" value=\"");
	itoa(PIN_NUM_CS,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"33\" size=\"2\">CS <input name=\"pndc\" type=\"number\" value=\"");
	itoa(PIN_NUM_DC,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"33\" size=\"2\">DC <input name=\"pnrst\" type=\"number\" value=\"");
	itoa(PIN_NUM_RST,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"33\" size=\"2\">RES <input name=\"pnbckl\" type=\"number\" value=\"");
	itoa(PIN_NUM_BCKL,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"33\" size=\"2\">LED <input name=\"pntouchcs\" type=\"number\" value=\"");
	itoa(PIN_TOUCH_CS,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"33\" size=\"2\">T_CS</br>");
#endif
        strcat(bsend,"Port GPIO/Mode<input name=\"ppin1\" type=\"number\" value=\"");
	itoa((bgpio1 & 0x3f),buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"39\" size=\"2\">Port1<select name=\"popt1\"><option ");
	if (!(bgpio1 & 0xc0)) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Off</option><option ");
	if ((bgpio1 & 0xc0) == 0x40) strcat(bsend,"selected ");
        strcat(bsend,"value=\"64\">Out</option><option ");
	if ((bgpio1 & 0xc0) == 0x80) strcat(bsend,"selected ");
        strcat(bsend,"value=\"128\">In</option><option ");
	if ((bgpio1 & 0xc0) == 0xc0) strcat(bsend,"selected ");
        strcat(bsend,"value=\"192\">Sw1</option></select>&emsp;<input name=\"ppin2\" type=\"number\" value=\"");
	itoa((bgpio2 & 0x3f),buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"39\" size=\"2\">Port2<select name=\"popt2\"><option ");
	if (!(bgpio2 & 0xc0)) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Off</option><option ");
	if ((bgpio2 & 0xc0) == 0x40) strcat(bsend,"selected ");
        strcat(bsend,"value=\"64\">Out</option><option ");
	if ((bgpio2 & 0xc0) == 0x80) strcat(bsend,"selected ");
        strcat(bsend,"value=\"128\">In</option><option ");
	if ((bgpio2 & 0xc0) == 0xc0) strcat(bsend,"selected ");
        strcat(bsend,"value=\"192\">Sw2</option></select>&emsp;<input name=\"ppin3\" type=\"number\" value=\"");
	itoa((bgpio3 & 0x3f),buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"39\" size=\"2\">Port3<select name=\"popt3\"><option ");
	if (!(bgpio3 & 0xc0)) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Off</option><option ");
	if ((bgpio3 & 0xc0) == 0x40) strcat(bsend,"selected ");
        strcat(bsend,"value=\"64\">Out</option><option ");
	if ((bgpio3 & 0xc0) == 0x80) strcat(bsend,"selected ");
        strcat(bsend,"value=\"128\">In</option><option ");
	if ((bgpio3 & 0xc0) == 0xc0) strcat(bsend,"selected ");
        strcat(bsend,"value=\"192\">Sw3</option></select>&emsp;<input name=\"ppin4\" type=\"number\" value=\"");
	itoa((bgpio4 & 0x3f),buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"39\" size=\"2\">Port4<select name=\"popt4\"><option ");
	if (!(bgpio4 & 0xc0)) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Off</option><option ");
	if ((bgpio4 & 0xc0) == 0x40) strcat(bsend,"selected ");
        strcat(bsend,"value=\"64\">Out</option><option ");
	if ((bgpio4 & 0xc0) == 0x80) strcat(bsend,"selected ");
        strcat(bsend,"value=\"128\">In</option></select>&emsp;<input name=\"ppin5\" type=\"number\" value=\"");
	itoa((bgpio5 & 0x3f),buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"39\" size=\"2\">Port5<select name=\"popt5\"><option ");
	if (!(bgpio5 & 0xc0)) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Off</option><option ");
	if ((bgpio5 & 0xc0) == 0x40) strcat(bsend,"selected ");
        strcat(bsend,"value=\"64\">Out</option><option ");
	if ((bgpio5 & 0xc0) == 0x80) strcat(bsend,"selected ");
        strcat(bsend,"value=\"128\">In</option></select><br>");

	strcat(bsend,"<h3>Store setting then press SAVE. ESP32 r4sGate will restart</h3></br>");
	strcat(bsend,"<input type=SUBMIT value=\"Save settings\"></form><form method=\"POST\" action=\"/setignore\">");
	strcat(bsend,"<input type=SUBMIT value=\"Cancel\"></form></body></html>");

//itoa(strlen(bsend),buff,10);
//strcat(bsend,buff);

        httpd_resp_sendstr(req, bsend);
    return ESP_OK;
}
static const httpd_uri_t psetting = {
	.uri       = "/setting",
	.method    = HTTP_GET,
	.handler   = psetting_get_handler,
	.user_ctx  = NULL
};

static esp_err_t psetsave_get_handler(httpd_req_t *req)
{
	char buf1[1024] = {0};
	char buf2[16] = {0};
	char buf3[16] = {0};
	uint8_t pintemp;
//save ip from header
	char bufip[32] = {0};
	int buf_len;
	buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
	if (buf_len > 1) {
	if (buf_len >31) buf_len = 31;
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Host", bufip, buf_len) == ESP_OK) {
        }
	}
	int  ret;
	ret = httpd_req_recv(req,buf1,1024);
	if ( ret > 0 ) {
//ESP_LOGI(AP_TAG, "Buf: '%s'", buf1);
/*
in buf1 after httpd_req_recv string like below
swfid=wifiname&swfpsw=wifipassword&smqsrv=192.168.1.10&smqid=esp&
smqpsw=esp&devnam=&rlight=255&glight=255&blight=255&chk2=2
*/
	strcpy(buf2,"swfid");
	parsuri(buf1,WIFI_SSID,buf2,1024,33);
	strcpy(buf2,"swfpsw");
	parsuri(buf1,WIFI_PASSWORD,buf2,1024,65);
	strcpy(buf2,"smqsrv");
	parsuri(buf1,MQTT_SERVER,buf2,1024,20);
	strcpy(buf2,"smqid");
	parsuri(buf1,MQTT_USER,buf2,1024,16);
	strcpy(buf2,"smqpsw");
	parsuri(buf1,MQTT_PASSWORD,buf2,1024,20);
	strcpy(buf2,"smtopp1");
	parsuri(buf1,MQTT_TOPP1,buf2,1024,32);
	strcpy(buf2,"smtopp2");
	parsuri(buf1,MQTT_TOPP2,buf2,1024,24);
	strcpy(buf2,"smtopp3");
	parsuri(buf1,MQTT_TOPP3,buf2,1024,24);
	strcpy(buf2,"smtopp4");
	parsuri(buf1,MQTT_TOPP4,buf2,1024,32);
	strcpy(buf2,"smtopp5");
	parsuri(buf1,MQTT_TOPP5,buf2,1024,32);
	strcpy(buf2,"smtopp6");
	parsuri(buf1,MQTT_TOPP6,buf2,1024,32);
	strcpy(buf2,"smtopp7");
	parsuri(buf1,MQTT_TOPP7,buf2,1024,32);
	strcpy(buf2,"sreqnma");
	parsuri(buf1,REQ_NAMEA,buf2,1024,16);
	strcpy(buf2,"sreqnmb");
	parsuri(buf1,REQ_NAMEB,buf2,1024,16);
	strcpy(buf2,"sreqnmc");
	parsuri(buf1,REQ_NAMEC,buf2,1024,16);
	strcpy(buf2,"rlighta");
	parsuri(buf1,buf3,buf2,1024,4);
	RgbRA = atoi(buf3);
	strcpy(buf2,"glighta");
	parsuri(buf1,buf3,buf2,1024,4);
	RgbGA = atoi(buf3);
	strcpy(buf2,"blighta");
	parsuri(buf1,buf3,buf2,1024,4);
	RgbBA = atoi(buf3);
	strcpy(buf2,"rlightb");
	parsuri(buf1,buf3,buf2,1024,4);
	RgbRB = atoi(buf3);
	strcpy(buf2,"glightb");
	parsuri(buf1,buf3,buf2,1024,4);
	RgbGB = atoi(buf3);
	strcpy(buf2,"blightb");
	parsuri(buf1,buf3,buf2,1024,4);
	RgbBB = atoi(buf3);
	strcpy(buf2,"rlightc");
	parsuri(buf1,buf3,buf2,1024,4);
	RgbRC = atoi(buf3);
	strcpy(buf2,"glightc");
	parsuri(buf1,buf3,buf2,1024,4);
	RgbGC = atoi(buf3);
	strcpy(buf2,"blightc");
	parsuri(buf1,buf3,buf2,1024,4);
	RgbBC = atoi(buf3);
	strcpy(buf2,"ltempa");
	parsuri(buf1,buf3,buf2,1024,4);
	bLtempA = atoi(buf3);
	strcpy(buf2,"ltempb");
	parsuri(buf1,buf3,buf2,1024,4);
	bLtempB = atoi(buf3);
	strcpy(buf2,"ltempc");
	parsuri(buf1,buf3,buf2,1024,4);
	bLtempC = atoi(buf3);
	strcpy(buf2,"sreqtpa");
	parsuri(buf1,buf3,buf2,1024,4);
	DEV_TYPA = atoi(buf3);
	strcpy(buf2,"sreqtpb");
	parsuri(buf1,buf3,buf2,1024,4);
	DEV_TYPB = atoi(buf3);
	strcpy(buf2,"sreqtpc");
	parsuri(buf1,buf3,buf2,1024,4);
	DEV_TYPC = atoi(buf3);
	strcpy(buf2,"sjpgtim");
	parsuri(buf1,buf3,buf2,1024,8);
	jpg_time = atoi(buf3);
	strcpy(buf2,"smqprt");
	parsuri(buf1,buf3,buf2,1024,8);
	mqtt_port = atoi(buf3);
	strcpy(buf2,"timzon");
	parsuri(buf1,buf3,buf2,1024,4);
	TimeZone = atoi(buf3);
	strcpy(buf2,"r4snum");
	parsuri(buf1,buf3,buf2,1024,4);
	R4SNUM = atoi(buf3);
	strcpy(buf2,"smjpuri");
	parsuri(buf1,MyHttpUri,buf2,1024,64);
	buf3[0] = 0;
	strcpy(buf2,"chk1");
	parsuri(buf1,buf3,buf2,1024,2);
	FDHass = 0;
	if (buf3[0] == 0x31) FDHass = 1;
	buf3[0] = 0;
	strcpy(buf2,"chk2");
	parsuri(buf1,buf3,buf2,1024,2);
	fcommtp = 0;
	if (buf3[0] == 0x32) fcommtp = 1;
	buf3[0] = 0;
	strcpy(buf2,"chk3");
	parsuri(buf1,buf3,buf2,1024,2);
	ftrufal = 0;
	if ((buf3[0] == 0x33) && !FDHass) ftrufal = 1;
	buf3[0] = 0;
	strcpy(buf2,"chk4");
	parsuri(buf1,buf3,buf2,1024,2);
	tft_flip = 0;
	if (buf3[0] == 0x34) tft_flip = 1;
	buf3[0] = 0;
	strcpy(buf2,"chk5");
	parsuri(buf1,buf3,buf2,1024,2);
	ble_mon = 0;
	if (buf3[0] == 0x35) ble_mon = 1;
	buf3[0] = 0;
	strcpy(buf2,"chk6");
	parsuri(buf1,buf3,buf2,1024,2);
	tft_conf = 0;
	if (buf3[0] == 0x36) tft_conf = 1;
	buf3[0] = 0;
	strcpy(buf2,"chk7");
	parsuri(buf1,buf3,buf2,1024,2);
	foffln = 0;
	if (buf3[0] == 0x37) foffln = 1;
	buf3[0] = 0;
	strcpy(buf2,"chk0");
	parsuri(buf1,buf3,buf2,1024,2);
	if (buf3[0] == 0x30) {
	ESP_LOGI(AP_TAG, "Format NVS");
	nvs_flash_erase();
	nvs_flash_init();
	}
	if (FDHass) {
	ftrufal = 0;
	foffln = 0;
	}
	tft_conn = 0;
	strcpy(buf2,"pnmiso");
	parsuri(buf1,buf3,buf2,1024,4);
        pintemp = atoi(buf3);
	if (pinvalid(pintemp)) PIN_NUM_MISO = pintemp;
	else tft_conf = 0;
	strcpy(buf2,"pnmosi");
	parsuri(buf1,buf3,buf2,1024,4);
        pintemp = atoi(buf3);
	if (pinvalid(pintemp)) PIN_NUM_MOSI = pintemp;
	else tft_conf = 0;
	strcpy(buf2,"pnclk");
	parsuri(buf1,buf3,buf2,1024,4);
        pintemp = atoi(buf3);
	if (pinvalid(pintemp)) PIN_NUM_CLK = pintemp;
	else tft_conf = 0;
	strcpy(buf2,"pncs");
	parsuri(buf1,buf3,buf2,1024,4);
        pintemp = atoi(buf3);
	if (pinvalid(pintemp)) PIN_NUM_CS = pintemp;
	else tft_conf = 0;
	strcpy(buf2,"pndc");
	parsuri(buf1,buf3,buf2,1024,4);
        pintemp = atoi(buf3);
	if (pinvalid(pintemp)) PIN_NUM_DC = pintemp;
	else tft_conf = 0;
	strcpy(buf2,"pnrst");
	parsuri(buf1,buf3,buf2,1024,4);
        pintemp = atoi(buf3);
	if (pinvalid(pintemp)) PIN_NUM_RST = pintemp;
	else tft_conf = 0;
	strcpy(buf2,"pnbckl");
	parsuri(buf1,buf3,buf2,1024,4);
        pintemp = atoi(buf3);
	if (pinvalid(pintemp)) PIN_NUM_BCKL = pintemp;
	else tft_conf = 0;
	strcpy(buf2,"pntouchcs");
	parsuri(buf1,buf3,buf2,1024,4);
        pintemp = atoi(buf3);
	if (pinvalid(pintemp)) PIN_TOUCH_CS = pintemp;
	else tft_conf = 0;

	strcpy(buf2,"ppin1");
	parsuri(buf1,buf3,buf2,1024,4);
        pintemp = atoi(buf3);
	if (pinvalid(pintemp)) {
	bgpio1 = pintemp;
	strcpy(buf2,"popt1");
	parsuri(buf1,buf3,buf2,1024,4);
        pintemp = atoi(buf3);
	bgpio1 = bgpio1  | pintemp;
	} else bgpio1 = 0;

	strcpy(buf2,"ppin2");
	parsuri(buf1,buf3,buf2,1024,4);
        pintemp = atoi(buf3);
	if (pinvalid(pintemp)) {
	bgpio2 = pintemp;
	strcpy(buf2,"popt2");
	parsuri(buf1,buf3,buf2,1024,4);
        pintemp = atoi(buf3);
	bgpio2 = bgpio2  | pintemp;
	} else bgpio2 = 0;

	strcpy(buf2,"ppin3");
	parsuri(buf1,buf3,buf2,1024,4);
        pintemp = atoi(buf3);
	if (pinvalid(pintemp)) {
	bgpio3 = pintemp;
	strcpy(buf2,"popt3");
	parsuri(buf1,buf3,buf2,1024,4);
        pintemp = atoi(buf3);
	bgpio3 = bgpio3  | pintemp;
	} else bgpio3 = 0;

	strcpy(buf2,"ppin4");
	parsuri(buf1,buf3,buf2,1024,4);
        pintemp = atoi(buf3);
	if (pinvalid(pintemp)) {
	bgpio4 = pintemp;
	strcpy(buf2,"popt4");
	parsuri(buf1,buf3,buf2,1024,4);
        pintemp = atoi(buf3);
	bgpio4 = bgpio4  | pintemp;
	} else bgpio4 = 0;

	strcpy(buf2,"ppin5");
	parsuri(buf1,buf3,buf2,1024,4);
        pintemp = atoi(buf3);
	if (pinvalid(pintemp)) {
	bgpio5 = pintemp;
	strcpy(buf2,"popt5");
	parsuri(buf1,buf3,buf2,1024,4);
        pintemp = atoi(buf3);
	bgpio5 = bgpio5  | pintemp;
	} else bgpio1 = 0;

	buf3[0] = 0;
	strcpy(buf2,"mqtdel");
	parsuri(buf1,buf3,buf2,1024,2);
	if (buf3[0] == 0x31) {
	ESP_LOGI(AP_TAG, "Delete Mqtt topics");
	strcpy(buf2, "r4s");
	if (R4SNUM)  {
	itoa(R4SNUM,buf3,10);
	strcat(buf2, buf3);
	}
	strcpy(buf1,buf2);
	strcat(buf1,"/#");
	f_update = true;
	while (btauthoriza || btauthorizb || btauthorizc) vTaskDelay(200 / portTICK_PERIOD_MS);
	mqtdel = 20;
	esp_mqtt_client_subscribe(mqttclient, buf1, 0);
	while (--mqtdel > 1) vTaskDelay(20 / portTICK_PERIOD_MS);

	if (FDHass) {
	strcpy(buf1,"homeassistant/sensor/");
	strcat(buf1,buf2);
	strcat(buf1,"/#");
	mqtdel = 20;
	esp_mqtt_client_subscribe(mqttclient, buf1, 0);
	while (--mqtdel > 1) vTaskDelay(20 / portTICK_PERIOD_MS);
	strcpy(buf1,"homeassistant/switch/");
	strcat(buf1,buf2);
	strcat(buf1,"/#");
	mqtdel = 20;
	esp_mqtt_client_subscribe(mqttclient, buf1, 0);
	while (--mqtdel > 1) vTaskDelay(20 / portTICK_PERIOD_MS);
	strcpy(buf1,"homeassistant/climate/");
	strcat(buf1,buf2);
	strcat(buf1,"/#");
	mqtdel = 20;
	esp_mqtt_client_subscribe(mqttclient, buf1, 0);
	while (--mqtdel > 1) vTaskDelay(20 / portTICK_PERIOD_MS);
	strcpy(buf1,"homeassistant/light/");
	strcat(buf1,buf2);
	strcat(buf1,"/#");
	mqtdel = 20;
	esp_mqtt_client_subscribe(mqttclient, buf1, 0);
	while (--mqtdel > 1) vTaskDelay(20 / portTICK_PERIOD_MS);
	strcpy(buf1,"homeassistant/binary_sensor/");
	strcat(buf1,buf2);
	strcat(buf1,"/#");
	mqtdel = 20;
	esp_mqtt_client_subscribe(mqttclient, buf1, 0);
	while (--mqtdel > 1) vTaskDelay(20 / portTICK_PERIOD_MS);
	}
	if (R4SNUM !=R4SNUMO) {
	strcpy(buf2, "r4s");
	if (R4SNUMO)  {
	itoa(R4SNUMO,buf3,10);
	strcat(buf2, buf3);
	}
	strcpy(buf1,buf2);
	strcat(buf1,"/#");
	f_update = true;
	while (btauthoriza || btauthorizb || btauthorizc) vTaskDelay(200 / portTICK_PERIOD_MS);
	mqtdel = 20;
	esp_mqtt_client_subscribe(mqttclient, buf1, 0);
	while (--mqtdel > 1) vTaskDelay(20 / portTICK_PERIOD_MS);

	if (FDHass) {
	strcpy(buf1,"homeassistant/sensor/");
	strcat(buf1,buf2);
	strcat(buf1,"/#");
	mqtdel = 20;
	esp_mqtt_client_subscribe(mqttclient, buf1, 0);
	while (--mqtdel > 1) vTaskDelay(20 / portTICK_PERIOD_MS);
	strcpy(buf1,"homeassistant/switch/");
	strcat(buf1,buf2);
	strcat(buf1,"/#");
	mqtdel = 20;
	esp_mqtt_client_subscribe(mqttclient, buf1, 0);
	while (--mqtdel > 1) vTaskDelay(20 / portTICK_PERIOD_MS);
	strcpy(buf1,"homeassistant/climate/");
	strcat(buf1,buf2);
	strcat(buf1,"/#");
	mqtdel = 20;
	esp_mqtt_client_subscribe(mqttclient, buf1, 0);
	while (--mqtdel > 1) vTaskDelay(20 / portTICK_PERIOD_MS);
	strcpy(buf1,"homeassistant/light/");
	strcat(buf1,buf2);
	strcat(buf1,"/#");
	mqtdel = 20;
	esp_mqtt_client_subscribe(mqttclient, buf1, 0);
	while (--mqtdel > 1) vTaskDelay(20 / portTICK_PERIOD_MS);
	strcpy(buf1,"homeassistant/binary_sensor/");
	strcat(buf1,buf2);
	strcat(buf1,"/#");
	mqtdel = 20;
	esp_mqtt_client_subscribe(mqttclient, buf1, 0);
	while (--mqtdel > 1) vTaskDelay(20 / portTICK_PERIOD_MS);
	}

	}
	}


// write nvs
	nvs_handle_t my_handle;
	ret = nvs_open("storage", NVS_READWRITE, &my_handle);
	if (ret == ESP_OK) {

	nvs_set_u16(my_handle, "sjpgtim", jpg_time);
	nvs_set_u16(my_handle, "smqprt", mqtt_port);
	nvs_set_u8(my_handle,  "rlighta", RgbRA);
	nvs_set_u8(my_handle,  "glighta", RgbGA);
	nvs_set_u8(my_handle,  "blighta", RgbBA);
	nvs_set_u8(my_handle,  "rlightb", RgbRB);
	nvs_set_u8(my_handle,  "glightb", RgbGB);
	nvs_set_u8(my_handle,  "blightb", RgbBB);
	nvs_set_u8(my_handle,  "rlightc", RgbRC);
	nvs_set_u8(my_handle,  "glightc", RgbGC);
	nvs_set_u8(my_handle,  "blightc", RgbBC);
	nvs_set_u8(my_handle,  "ltempa", bLtempA);
	nvs_set_u8(my_handle,  "ltempb", bLtempB);
	nvs_set_u8(my_handle,  "ltempc", bLtempC);
	nvs_set_u8(my_handle,  "sreqtpa", DEV_TYPA);
	nvs_set_u8(my_handle,  "sreqtpb", DEV_TYPB);
	nvs_set_u8(my_handle,  "sreqtpc", DEV_TYPC);
	nvs_set_u8(my_handle,  "timzon", TimeZone);
	nvs_set_u8(my_handle,  "r4snum", R4SNUM);
	nvs_set_u8(my_handle,  "chk1",  FDHass);
	nvs_set_u8(my_handle,  "chk2",  fcommtp);
	nvs_set_u8(my_handle,  "chk3",  ftrufal);
	nvs_set_u8(my_handle,  "chk4",  tft_flip);
	nvs_set_u8(my_handle,  "chk5",  ble_mon);
	nvs_set_u8(my_handle,  "chk6",  tft_conf);
	nvs_set_u8(my_handle,  "chk7",  foffln);
	nvs_set_u8(my_handle, "pnmiso", PIN_NUM_MISO);
	nvs_set_u8(my_handle, "pnmosi", PIN_NUM_MOSI);
	nvs_set_u8(my_handle, "pnclk", PIN_NUM_CLK);
	nvs_set_u8(my_handle, "pncs", PIN_NUM_CS);
	nvs_set_u8(my_handle, "pndc", PIN_NUM_DC);
	nvs_set_u8(my_handle, "pnrst", PIN_NUM_RST);
	nvs_set_u8(my_handle, "pnbckl", PIN_NUM_BCKL);
	nvs_set_u8(my_handle, "pntouchcs", PIN_TOUCH_CS);
	nvs_set_u8(my_handle, "bgpio1", bgpio1);
	nvs_set_u8(my_handle, "bgpio2", bgpio2);
	nvs_set_u8(my_handle, "bgpio3", bgpio3);
	nvs_set_u8(my_handle, "bgpio4", bgpio4);
	nvs_set_u8(my_handle, "bgpio5", bgpio5);
	nvs_set_str(my_handle, "swfid", WIFI_SSID);
	nvs_set_str(my_handle, "swfpsw", WIFI_PASSWORD);
	nvs_set_str(my_handle, "smqsrv", MQTT_SERVER);
	nvs_set_str(my_handle, "smqid", MQTT_USER);
	nvs_set_str(my_handle, "smqpsw", MQTT_PASSWORD);
	nvs_set_str(my_handle, "sreqnma", REQ_NAMEA);
	nvs_set_str(my_handle, "sreqnmb", REQ_NAMEB);
	nvs_set_str(my_handle, "sreqnmc", REQ_NAMEC);
#ifdef USE_TFT
	nvs_set_str(my_handle, "smtopp1", MQTT_TOPP1);
	nvs_set_str(my_handle, "smtopp2", MQTT_TOPP2);
	nvs_set_str(my_handle, "smtopp3", MQTT_TOPP3);
	nvs_set_str(my_handle, "smtopp4", MQTT_TOPP4);
	nvs_set_str(my_handle, "smtopp5", MQTT_TOPP5);
	nvs_set_str(my_handle, "smtopp6", MQTT_TOPP6);
	nvs_set_str(my_handle, "smtopp7", MQTT_TOPP7);
	nvs_set_str(my_handle, "smjpuri", MyHttpUri);
#endif
        ret = nvs_commit(my_handle);
	if (ret != ESP_OK) {
	ESP_LOGE(AP_TAG, "NVS write error");
	}
// Close nvs
	nvs_close(my_handle);
	}
	}
	strcpy(buf1,"<!DOCTYPE html><html><head><title>r4sGate</title><meta name='viewport' content='width=device-width, initial-scale=1.0'></head><body>Setting saved. Rebooting...</body></html><meta http-equiv=\"refresh\" content=\"3;URL=http://");
        strcat(buf1, bufip);
	strcat(buf1,"/\"</body></html>");
        httpd_resp_sendstr(req, buf1);

	ESP_LOGI(AP_TAG, "Prepare to restart system!");
	vTaskDelay(3000 / portTICK_PERIOD_MS);
	esp_restart();
    return ESP_OK;
}
static const httpd_uri_t psetsave = {
	.uri       = "/setsave",
	.method    = HTTP_POST,
	.handler   = psetsave_get_handler,
	.user_ctx  = NULL
};



static esp_err_t psetignore_get_handler(httpd_req_t *req)
{
	httpd_resp_set_status(req, "303 See Other");
	httpd_resp_set_hdr(req, "Location", "/");
	httpd_resp_send(req, NULL, 0);  // Response body can be empty
    return ESP_OK;
}


static const httpd_uri_t psetignore = {
	.uri       = "/setignore",
	.method    = HTTP_POST,
	.handler   = psetignore_get_handler,
	.user_ctx  = NULL
};

/* HTTP GET reboot handler */
static esp_err_t prestart_get_handler(httpd_req_t *req)
{
	char buf1[512] = {0};
//save ip from header
	char bufip[32] = {0};
	int buf_len;
	buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
	if (buf_len > 1) {
	if (buf_len >31) buf_len = 31;
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Host", bufip, buf_len) == ESP_OK) {
        }
	}
	strcpy(buf1,"<!DOCTYPE html><html><head><title>r4sGate</title><meta name='viewport' content='width=device-width, initial-scale=1.0'></head><body>Rebooting...</body></html><meta http-equiv=\"refresh\" content=\"3;URL=http://");
        strcat(buf1, bufip);
	strcat(buf1,"/\"</body></html>");
        httpd_resp_sendstr(req, buf1);

	vTaskDelay(1000 / portTICK_PERIOD_MS);
	esp_restart();
    return ESP_OK;
}
static const httpd_uri_t prestart = {
	.uri       = "/restart",
	.method    = HTTP_GET,
	.handler   = prestart_get_handler,
	.user_ctx  = NULL
};

/* HTTP GET update handler */
static esp_err_t pupdate_get_handler(httpd_req_t *req)
{
        httpd_resp_sendstr(req, serverIndex);
    return ESP_OK;
}
static const httpd_uri_t pupdate = {
	.uri       = "/update",
	.method    = HTTP_GET,
	.handler   = pupdate_get_handler,
	.user_ctx  = NULL
};


/* HTTP GET updating handler */
static esp_err_t pupdating_get_handler(httpd_req_t *req)
{
	f_update = true;
	t_tinc_us = ~t_tinc_us;	
	char otabuf[otabufsize] ={0};
	char filnam[128] ={0};
	int  otabufoffs = 0;
	esp_err_t err = 0;
	int binary_file_length = 0;
	bool image_header_was_checked = false;
	bool ota_running = true;
	esp_ota_handle_t update_handle = 0 ;
//save ip from header
	char bufip[32] = {0};
	int buf_len;
	buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
	if (buf_len > 1) {
	if (buf_len >31) buf_len = 31;
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Host", bufip, buf_len) == ESP_OK) {
        }
	}
	ESP_LOGI(AP_TAG, "Starting OTA");
	const esp_partition_t *pupdate = esp_ota_get_next_update_partition(NULL);
//info about running
	const esp_partition_t *running = esp_ota_get_running_partition();
	ESP_LOGI(AP_TAG, "Running partition type %d subtype %d (offset 0x%08x)",
             running->type, running->subtype, running->address);

	/*deal with all receive packet*/
// read loop
	while (ota_running) {
	otabufoffs = 0;
//read max 2048 bytes
        int data_read = httpd_req_recv(req, otabuf, otabufsize);
        if (data_read < 0) {
	ESP_LOGE(AP_TAG, "Error: data read error");
	ota_running = false;
	otabufoffs = 0;
        } else if (data_read > 0) {
	if (image_header_was_checked == false) {
// if first read check and remove POST header
/*
in otabuf after httpd_req_recv string like below
------WebKitFormBoundary2ZdUwM7CAu6TtDPq
Content-Disposition: form-data; name="update"; filename="r4sGate.bin"
Content-Type: application/octet-stream\r\n\r\n
*/
//esp_log_buffer_hex(AP_TAG, otabuf, 256);
//ESP_LOGI(AP_TAG, "Buff: %s",otabuf);

//check "Content-Disposition" string in received data
	otabufoffs = parsoff(otabuf,"Content-Disposition", otabufsize);
	if (!otabufoffs) {
	ESP_LOGE(AP_TAG, "Content-Disposition not found");
	ota_running = false;
	otabufoffs = 0;
	data_read = 0;
	}
// save filename
	otabufoffs = parsoff(otabuf,"filename=", otabufsize);
	mystrcpy(filnam, otabuf+otabufoffs, 127);
// search for data begin
	otabufoffs = parsoff(otabuf,"application/octet-stream\r\n\r\n", otabufsize);
	if (!otabufoffs) {
	ESP_LOGE(AP_TAG, "application/octet-stream not found");
	ota_running = false;
	otabufoffs = 0;
	data_read = 0;
	}

//esp_log_buffer_hex(AP_TAG, otabuf, 128);
	ESP_LOGI(AP_TAG, "Loading filename: %s",filnam);

	image_header_was_checked = true;
	err = esp_ota_begin(pupdate, OTA_SIZE_UNKNOWN, &update_handle);
	if (err != ESP_OK) {
	ESP_LOGE(AP_TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
	ota_running = false;
	otabufoffs = 0;
	data_read = 0;
	}
//	ESP_LOGI(AP_TAG, "esp_ota_begin succeeded");
	} 
	if (data_read > 0)  {
	err = esp_ota_write( update_handle, (const void *)otabuf+otabufoffs, data_read-otabufoffs);
	if (err != ESP_OK) {
	ota_running = false;
	otabufoffs = 0;
	data_read = 0;
	}
	}
	binary_file_length = binary_file_length  + data_read - otabufoffs; 
//ESP_LOGI(AP_TAG, "Written image length %d", binary_file_length);
	} else if (data_read == 0) {
	ota_running = false;
	otabufoffs = 0;
	data_read = 0;
	}
}
//
	ESP_LOGI(AP_TAG, "Total Write binary data length: %x", binary_file_length);
	
	err = esp_ota_end(update_handle);
	strcpy (otabuf,"<!DOCTYPE html><html><head><title>r4sGate</title><meta name='viewport' content='width=device-width, initial-scale=1.0'></head><body>Update ");
	if (err != ESP_OK) {
	if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
	ESP_LOGE(AP_TAG, "Image validation failed, image is corrupted");
	}
	ESP_LOGE(AP_TAG, "esp_ota_end failed (%s)!", esp_err_to_name(err));
	strcat(otabuf,"failed");
	} else {
	ESP_LOGI(AP_TAG, "esp_ota_end ok!");
	strcat(otabuf,"ok");
	}
	strcat(otabuf,". Rebooting...</body></html><meta http-equiv=\"refresh\" content=\"3;URL=http://");
        strcat(otabuf, bufip);
	strcat(otabuf,"/\"</body></html>");
        httpd_resp_sendstr(req, otabuf);

	err = esp_ota_set_boot_partition(pupdate);
	if (err != ESP_OK) {
	ESP_LOGE(AP_TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
	ota_running = false;
	}
	ESP_LOGI(AP_TAG, "Prepare to restart system!");
	vTaskDelay(1000 / portTICK_PERIOD_MS);
	esp_restart();
    return ESP_OK;
}
static const httpd_uri_t pupdating = {
	.uri       = "/updating",
	.method    = HTTP_POST,
	.handler   = pupdating_get_handler,
	.user_ctx  = NULL
};



//*************************************************

static httpd_handle_t start_webserver(void)
{
	httpd_handle_t server = NULL;
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	config.max_uri_handlers = 16;
//	config.max_resp_headers =16;
	config.stack_size = 20480;
	// Start the httpd server
//	ESP_LOGI(AP_TAG, "Starting server on port: '%d'", config.server_port);
//	ESP_LOGI(AP_TAG, "Max URI handlers: '%d'", config.max_uri_handlers);
//	ESP_LOGI(AP_TAG, "Max Open Sessions: '%d'", config.max_open_sockets);
//	ESP_LOGI(AP_TAG, "Max Header Length: '%d'", HTTPD_MAX_REQ_HDR_LEN);
//	ESP_LOGI(AP_TAG, "Max URI Length: '%d'", HTTPD_MAX_URI_LEN);
//	ESP_LOGI(AP_TAG, "Max Stack Size: '%d'", config.stack_size);

	if (httpd_start(&server, &config) == ESP_OK) {
// Set URI handlers
//	ESP_LOGI(AP_TAG, "Registering URI handlers");
	httpd_register_uri_handler(server, &pmain);
	httpd_register_uri_handler(server, &pcfgdev1);
	httpd_register_uri_handler(server, &pcfgdev1ok);
	httpd_register_uri_handler(server, &pcfgdev2);
	httpd_register_uri_handler(server, &pcfgdev2ok);
	httpd_register_uri_handler(server, &pcfgdev3);
	httpd_register_uri_handler(server, &pcfgdev3ok);
	httpd_register_uri_handler(server, &psetting);
	httpd_register_uri_handler(server, &psetsave);
	httpd_register_uri_handler(server, &psetignore);
	httpd_register_uri_handler(server, &prestart);
	httpd_register_uri_handler(server, &pupdate);
	httpd_register_uri_handler(server, &pupdating);
	return server;
	}
	ESP_LOGI(AP_TAG, "Error starting server!");
	return NULL;
}

static void stop_webserver(httpd_handle_t server)
{
// Stop the httpd server
	httpd_stop(server);
}

static void disconnect_handler(void* arg, esp_event_base_t event_base, 
                               int32_t event_id, void* event_data)
{
	httpd_handle_t* server = (httpd_handle_t*) arg;
	if (*server) {
	ESP_LOGI(AP_TAG, "Stopping webserver");
	stop_webserver(*server);
	*server = NULL;
	}
}

static void connect_handler(void* arg, esp_event_base_t event_base, 
                            int32_t event_id, void* event_data)
{
	httpd_handle_t* server = (httpd_handle_t*) arg;
	if (*server == NULL) {
	ESP_LOGI(AP_TAG, "Starting webserver");
	*server = start_webserver();
	}
}




//******************* Main **********************
void app_main(void)
{
	static httpd_handle_t server = NULL;
	printf("Starting r4sGate...\n");
	tft_conf = 0;
	tft_conn = false;
	PIN_NUM_MISO = 25;	//MISO
	PIN_NUM_MOSI = 23;	//MOSI
	PIN_NUM_CLK  = 19;	//CLK
	PIN_NUM_CS   = 16;	// Chip select control pin
	PIN_NUM_DC   = 17;	// Data Command control pin
	PIN_NUM_RST  = 18;	// Reset pin (could connect to RST pin)
	PIN_NUM_BCKL = 21;	// TFT_BACKLIGHT
	PIN_TOUCH_CS = 33;	// Chip select pin (T_CS) of touch screen not used always high
/* Print chip information */
	esp_chip_info_t chip_info;
	esp_chip_info(&chip_info);
	printf("This is %s chip with %d CPU cores, WiFi%s%s, ",
		CONFIG_IDF_TARGET,
		chip_info.cores,
		(chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
		(chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");
	printf("\nsilicon revision %d, ", chip_info.revision);
	printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
		(chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
	ESP_LOGI(AP_TAG,"Init start free heap: %d\n", esp_get_free_heap_size());
	printf("APP IDF version: %s\n", esp_get_idf_version());
	esp_err_t ret;
//Initialize NVS
	ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
	ESP_ERROR_CHECK(nvs_flash_erase());
	ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);
// empty string 
	s_retry_num = 0;
	f_update = false;
	mqtdel  = 0;
	R4SNUM = 0;
	R4SNUMO = 0;
	MiAKettleID = 1;
	MiBKettleID = 1;
	MiCKettleID = 1;
	FND_NAME[0] = 0;
	REQ_NAMEA[0] = 0;
	REQ_NAMEB[0] = 0;
	REQ_NAMEC[0] = 0;
	DEV_NAMEA[0] = 0;
	DEV_NAMEB[0] = 0;
	DEV_NAMEC[0] = 0;
	MQTT_USER[0] = 0;
	MQTT_PASSWORD[0] = 0;
	MQTT_SERVER[0] = 0;
	WIFI_SSID[0] = 0;
	WIFI_PASSWORD[0] = 0;
	tBLEAddrA[0] = 0;

	MQTT_TOPP1[0] = 0;
	MQTT_TOPP2[0] = 0;
	MQTT_TOPP3[0] = 0;
	MQTT_TOPP4[0] = 0;
	MQTT_TOPP5[0] = 0;
	MQTT_TOPP6[0] = 0;
	MQTT_TOPP7[0] = 0;
	MQTT_VALP1[0] = 0;
	MQTT_VALP2[0] = 0;
	MQTT_VALP3[0] = 0;
	MQTT_VALP4[0] = 0;
	MQTT_VALP5[0] = 0;
	MQTT_VALP6[0] = 0;
	MQTT_VALP7[0] = 0;
	MyHttpUri[0] = 0;
	xshedcoma = 0;
	xshedcomb = 0;
	xshedcomc = 0;
	NumConnA = 0;
	NumConnB = 0;
	NumConnC = 0;
	ble_mon = 0;
	bSEnergyA = 0;
	bSEnergyB = 0;
	bSEnergyC = 0;
	bSTimeA = 0;  
	bSTimeB = 0;  
	bSTimeC = 0;  
	bSCountA = 0; 
	bSCountB = 0; 
	bSCountC = 0; 
	bLtempA = 0;
	bLtempB = 0;
	bLtempC = 0;
	foffln = 0;
	bgpio1 = 0;
	bgpio2 = 0;
	bgpio3 = 0;
	bgpio4 = 0;
	bgpio5 = 0;
	strcpy(strON,"ON");
	strcpy(strOFF,"OFF");
// read nvs
	nvs_handle_t my_handle;
	ret = nvs_open("storage", NVS_READWRITE, &my_handle);
	if (ret == ESP_OK) {
	nvs_get_u16(my_handle, "sjpgtim", &jpg_time);
	nvs_get_u16(my_handle, "smqprt", &mqtt_port);
	nvs_get_u8(my_handle,  "sreqtpa", &DEV_TYPA);
	nvs_get_u8(my_handle,  "sreqtpb", &DEV_TYPB);
	nvs_get_u8(my_handle,  "sreqtpc", &DEV_TYPC);
	nvs_get_u8(my_handle, "rlighta", &RgbRA);
	nvs_get_u8(my_handle, "glighta", &RgbGA);
	nvs_get_u8(my_handle, "blighta", &RgbBA);
	nvs_get_u8(my_handle, "rlightb", &RgbRB);
	nvs_get_u8(my_handle, "glightb", &RgbGB);
	nvs_get_u8(my_handle, "blightb", &RgbBB);
	nvs_get_u8(my_handle, "rlightc", &RgbRC);
	nvs_get_u8(my_handle, "glightc", &RgbGC);
	nvs_get_u8(my_handle, "blightc", &RgbBC);
	nvs_get_u8(my_handle, "ltempa", &bLtempA);
	nvs_get_u8(my_handle, "ltempb", &bLtempB);
	nvs_get_u8(my_handle, "ltempc", &bLtempC);
	nvs_get_u8(my_handle, "pnmiso", &PIN_NUM_MISO);
	nvs_get_u8(my_handle, "pnmosi", &PIN_NUM_MOSI);
	nvs_get_u8(my_handle, "pnclk", &PIN_NUM_CLK);
	nvs_get_u8(my_handle, "pncs", &PIN_NUM_CS);
	nvs_get_u8(my_handle, "pndc", &PIN_NUM_DC);
	nvs_get_u8(my_handle, "pnrst", &PIN_NUM_RST);
	nvs_get_u8(my_handle, "pnbckl", &PIN_NUM_BCKL);
	nvs_get_u8(my_handle, "pntouchcs", &PIN_TOUCH_CS);
	nvs_get_u8(my_handle, "bgpio1", &bgpio1);
	nvs_get_u8(my_handle, "bgpio2", &bgpio2);
	nvs_get_u8(my_handle, "bgpio3", &bgpio3);
	nvs_get_u8(my_handle, "bgpio4", &bgpio4);
	nvs_get_u8(my_handle, "bgpio5", &bgpio5);
	PRgbRA = ~RgbRA;
	PRgbGA = ~RgbGA;
	PRgbBA = ~RgbBA;
	PRgbRB = ~RgbRB;
	PRgbGB = ~RgbGB;
	PRgbBB = ~RgbBB;
	PRgbRC = ~RgbRC;
	PRgbGC = ~RgbGC;
	PRgbBC = ~RgbBC;
	nvs_get_u8(my_handle, "chk1",   &FDHass);
	nvs_get_u8(my_handle, "chk2",   &fcommtp);
	nvs_get_u8(my_handle, "chk3",   &ftrufal);
	nvs_get_u8(my_handle, "chk4",   &tft_flip);
	nvs_get_u8(my_handle, "chk5",   &ble_mon);
	nvs_get_u8(my_handle, "chk6",   &tft_conf);
	nvs_get_u8(my_handle, "chk7",   &foffln);
	nvs_get_u8(my_handle, "timzon", &TimeZone);
	nvs_get_u8(my_handle, "r4snum", &R4SNUM);
	size_t nvsize = 32;
	nvs_get_str(my_handle,"swfid", WIFI_SSID,&nvsize);
	nvsize = 64;
	nvs_get_str(my_handle,"swfpsw", WIFI_PASSWORD,&nvsize);
	nvsize = 20;
	nvs_get_str(my_handle,"smqsrv", MQTT_SERVER,&nvsize);
	nvsize = 16;
	nvs_get_str(my_handle,"smqid", MQTT_USER,&nvsize);
	nvsize = 20;
	nvs_get_str(my_handle,"smqpsw", MQTT_PASSWORD,&nvsize);
	nvsize = 16;
	nvs_get_str(my_handle,"sreqnma", REQ_NAMEA,&nvsize);
	nvsize = 16;
	nvs_get_str(my_handle,"sreqnmb", REQ_NAMEB,&nvsize);
	nvsize = 16;
	nvs_get_str(my_handle,"sreqnmc", REQ_NAMEC,&nvsize);
#ifdef USE_TFT
	nvsize = 32;
	nvs_get_str(my_handle,"smtopp1", MQTT_TOPP1,&nvsize);
	nvsize = 24;
	nvs_get_str(my_handle,"smtopp2", MQTT_TOPP2,&nvsize);
	nvsize = 24;
	nvs_get_str(my_handle,"smtopp3", MQTT_TOPP3,&nvsize);
	nvsize = 32;
	nvs_get_str(my_handle,"smtopp4", MQTT_TOPP4,&nvsize);
	nvsize = 32;
	nvs_get_str(my_handle,"smtopp5", MQTT_TOPP5,&nvsize);
	nvsize = 32;
	nvs_get_str(my_handle,"smtopp6", MQTT_TOPP6,&nvsize);
	nvsize = 32;
	nvs_get_str(my_handle,"smtopp7", MQTT_TOPP7,&nvsize);
	nvsize = 64;
	nvs_get_str(my_handle,"smjpuri", MyHttpUri,&nvsize);
#endif
// Close nvs
	nvs_close(my_handle);
	}
	if ((PIN_NUM_MISO > 33) || (PIN_NUM_MISO > 33) || (PIN_NUM_MOSI > 33) || (PIN_NUM_CLK > 33) || 
	(PIN_NUM_CS > 33) || (PIN_NUM_DC > 33) || (PIN_NUM_RST > 33) || (PIN_NUM_BCKL > 33) || (PIN_TOUCH_CS > 33)) {
	PIN_NUM_MISO = 25;	//MISO
	PIN_NUM_MOSI = 23;	//MOSI
	PIN_NUM_CLK  = 19;	//CLK
	PIN_NUM_CS   = 16;	// Chip select control pin
	PIN_NUM_DC   = 17;	// Data Command control pin
	PIN_NUM_RST  = 18;	// Reset pin (could connect to RST pin)
	PIN_NUM_BCKL = 21;	// TFT_BACKLIGHT
	PIN_TOUCH_CS = 33;	// Chip select pin (T_CS) of touch screen not used always high
	}

	if (!WIFI_SSID[0]) {
	strcpy(WIFI_SSID, "r4s");
	strcpy(WIFI_PASSWORD, "12345678");
	}
// fill basic parameters
	R4SNUMO = R4SNUM;
	tBLEAddrA[0] = 0;
	char tzbuff[8];
	strcpy(MQTT_BASE_TOPIC, "r4s");
	if (R4SNUM)  {
	itoa(R4SNUM,tzbuff,10);
	strcat(MQTT_BASE_TOPIC, tzbuff);
	}
	if (FDHass) ftrufal = 0;
	if (ftrufal) {
	strcpy(strON,"true");
	strcpy(strOFF,"false");
	}
	cntgpio1 = 0;
	cntgpio2 = 0;
	cntgpio3 = 0;
	cntgpio4 = 0;
	cntgpio5 = 0;
	if (bgpio1 > 63) {
	if (bgpio1 < 98) {
	gpio_set_direction((bgpio1 & 0x3f), GPIO_MODE_OUTPUT);
	gpio_set_level((bgpio1 & 0x3f), 0);
	lvgpio1 = 0;
	} else {
	gpio_set_direction((bgpio1 & 0x3f), GPIO_MODE_INPUT);
	lvgpio1 = 0;
	}
	}
	if (bgpio2 > 63) {
	if (bgpio2 < 98) {
	gpio_set_direction((bgpio2 & 0x3f), GPIO_MODE_OUTPUT);
	gpio_set_level((bgpio2 & 0x3f), 0);
	lvgpio2 = 0;
	} else {
	gpio_set_direction((bgpio2 & 0x3f), GPIO_MODE_INPUT);
	lvgpio2 = 0;
	}
	}
	if (bgpio3 > 63) {
	if (bgpio3 < 98) {
	gpio_set_direction((bgpio3 & 0x3f), GPIO_MODE_OUTPUT);
	gpio_set_level((bgpio3 & 0x3f), 0);
	lvgpio3 = 0;
	} else {
	gpio_set_direction((bgpio3 & 0x3f), GPIO_MODE_INPUT);
	lvgpio3 = 0;
	}
	}
	if (bgpio4 > 63) {
	if (bgpio4 < 98) {
	gpio_set_direction((bgpio4 & 0x3f), GPIO_MODE_OUTPUT);
	gpio_set_level((bgpio4 & 0x3f), 0);
	lvgpio4 = 0;
	} else {
	gpio_set_direction((bgpio4 & 0x3f), GPIO_MODE_INPUT);
	lvgpio4 = 0;
	}
	}
	if (bgpio5 > 63) {
	if (bgpio5 < 98) {
	gpio_set_direction((bgpio5 & 0x3f), GPIO_MODE_OUTPUT);
	gpio_set_level((bgpio5 & 0x3f), 0);
	lvgpio5 = 0;
	} else {
	gpio_set_direction((bgpio5 & 0x3f), GPIO_MODE_INPUT);
	lvgpio5 = 0;
	}
	}


    timer_config_t config = {
            .alarm_en = true,				//Alarm Enable
            .counter_en = false,			//If the counter is enabled it will start incrementing / decrementing immediately after calling timer_init()
            .intr_type = TIMER_INTR_LEVEL,	//Is interrupt is triggered on timer's alarm (timer_intr_mode_t)
            .counter_dir = TIMER_COUNT_UP,	//Does counter increment or decrement (timer_count_dir_t)
            .auto_reload = true,			//If counter should auto_reload a specific initial value on the timer's alarm, or continue incrementing or decrementing.
            .divider = 80   				//Divisor of the incoming 80 MHz (12.5nS) APB_CLK clock. E.g. 80 = 1uS per timer tick
    };

    timer_init(TIMER_GROUP_0, TIMER_0, &config);
    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);
    timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, 20000);
    timer_enable_intr(TIMER_GROUP_0, TIMER_0);
    timer_isr_register(TIMER_GROUP_0, TIMER_0, &hw_timer_callback, NULL, 0, &s_timer_handle);

    timer_start(TIMER_GROUP_0, TIMER_0);


#ifdef USE_TFT
	if (tft_conf) tft_conn = tftinit();
#endif
//Initialize Wifi
	wifi_init_sta();
//init bt
        Isscanning = false;
        StartingScan = false;
	ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
	esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
	ret = esp_bt_controller_init(&bt_cfg);
	if (ret) {
	ESP_LOGI(AP_TAG,"%s init controller failed: %s\n", __func__, esp_err_to_name(ret));
	fflush(stdout);
	esp_restart();
	}
	ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
	if (ret) {
	ESP_LOGI(AP_TAG,"%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
	fflush(stdout);
	esp_restart();
	}
	ESP_LOGI(AP_TAG,"%s init bluetooth\n", __func__);
	ret = esp_bluedroid_init();
	if (ret) {
	ESP_LOGI(AP_TAG,"%s init bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
	fflush(stdout);
	esp_restart();
	}
	ret = esp_bluedroid_enable();
	if (ret) {
	ESP_LOGI(AP_TAG,"%s enable bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
	fflush(stdout);
	esp_restart();
	}
//register the  callback function to the gap module
	ret = esp_ble_gap_register_callback(esp_gap_cb);
	if (ret){
	ESP_LOGI(AP_TAG,"%s gap register failed, error code = %x\n", __func__, ret);
	fflush(stdout);
	esp_restart();
	}
//register the callback function to the gattc module
	ret = esp_ble_gattc_register_callback(esp_gattc_cb);
	if(ret){
	ESP_LOGI(AP_TAG,"%s gattc register failed, error code = %x\n", __func__, ret);
	fflush(stdout);
	esp_restart();
	}
	ret = esp_ble_gattc_app_register(PROFILE_A_APP_ID);
	if (ret){
	ESP_LOGI(AP_TAG,"%s gattc app register failed, error code = %x\n", __func__, ret);
	fflush(stdout);
	esp_restart();
	}
	ret = esp_ble_gattc_app_register(PROFILE_B_APP_ID);
	if (ret){
	ESP_LOGI(AP_TAG,"%s gattc app register failed, error code = %x\n", __func__, ret);
	fflush(stdout);
	esp_restart();
	}
	ret = esp_ble_gattc_app_register(PROFILE_C_APP_ID);
	if (ret){
	ESP_LOGI(AP_TAG,"%s gattc app register failed, error code = %x\n", __func__, ret);
	fflush(stdout);
	esp_restart();
	}
	esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(BLE_INPUT_BUFFSIZE);
//	esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(200);
	if (local_mtu_ret){
	ESP_LOGI(AP_TAG,"Set local  MTU failed, error code = %x\n", local_mtu_ret);
	}

//Initialize Mqtt
	mqtt_app_start();
//timezone
/*
to get MSK (GMT + 3) I need to write GMT-3
*/
	char tzbuf[16];
	uint8_t TimZn = TimeZone;
	strcpy(tzbuf,"GMT");
	if (TimZn > 127 ) {
	strcat (tzbuf,"+");
	TimZn = ~TimZn;
	TimZn++;
	} else strcat (tzbuf,"-");
	itoa(TimZn,tzbuff,10);
	strcat(tzbuf,tzbuff);
	setenv("TZ", tzbuf, 1);
	tzset();
//sntp
	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	sntp_setservername(0, "pool.ntp.org");
	sntp_init();
// get esp mac addr 
	uint8_t macbuf[8];
        tESP32Addr[0] = 0;
	esp_read_mac(macbuf,0);
	bin2hex(macbuf, tESP32Addr,6);
	itoa(R4SNUM,tzbuf,10);
	strcat (tESP32Addr,tzbuf);

//Initialize http server
	ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));
/* Start the server for the first time */
	server = start_webserver();





// mark as valid boot for prevent rollback after ota
	esp_ota_mark_app_valid_cancel_rollback();	
	ESP_LOGI(AP_TAG,"Init end free heap: %d\n", esp_get_free_heap_size());
	int floop = 32;
	int64_t t_before_us = 0;

//r4s state nonitoring and command execution loop
	while (floop) {
	if (r4slpcoma) {
	switch (r4slpcoma) {
	case 1:             //off
	r4slpresa = 1;
	if ((DEV_TYPA < 10) && (bProgA || bHeatA)) {
	mA171sOff();
	mA171s_ModOff();
	}
	if ((DEV_TYPA > 11) && (DEV_TYPA < 16)) {
	if (r4slppar1a == 1) mA103sToff();
	else if (r4slppar1a == 2) mA103sTon();
	bprevProgA = 254;
	}
	bprevHtempA = 255;
	bprevStateA = 255;
	bprevHeatA = 255;
	bprevStNlA = 255;
	mA171sOff();
	r4slpcoma = 0;
	t_lasta_us = ~t_lasta_us;
	break;
	case 2:             //boil
	r4slpresa = 1;
	if ((DEV_TYPA < 10) && bHtempA && (bHtempA < 91)) mA171sHeat(0);
	mA171sOff();
	bprevHtempA = 255;
	bprevStateA = 255;
	bprevHeatA = 255;
	bprevStNlA = 255;
	mA171sBoil();
	r4slpcoma = 0;
	t_lasta_us = ~t_lasta_us;
	break;
	case 3:             //heat
	r4slpresa = 1;
	mA171sOff();
	bprevHtempA = 255;
	bprevStateA = 255;
	bprevHeatA = 255;
	bprevStNlA = 255;
	mA171sHeat(r4slppar1a);
	r4slpcoma = 0;
	t_lasta_us = ~t_lasta_us;
	break;
	case 4:             //boil&heat
	r4slpresa = 1;
	mA171sOff();
	if (r4slppar1a == 0) mA171sBoil();
	else mA171sBoilAndHeat(r4slppar1a);
	bprevHtempA = 255;
	bprevStateA = 255;
	bprevHeatA = 255;
	bprevStNlA = 255;
	r4slpcoma = 0;
	t_lasta_us = ~t_lasta_us;
	break;
	case 5:             //nightlight
	r4slpresa = 1;
	if (DEV_TYPA > 3) {
	nl_settings[3]=RgbRA;
	nl_settings[4]=RgbGA;
	nl_settings[5]=RgbBA;
	nl_settings[8]=RgbRA;
	nl_settings[9]=RgbGA;
	nl_settings[10]=RgbBA;
	nl_settings[13]=RgbRA;
	nl_settings[14]=RgbGA;
	nl_settings[15]=RgbBA;    
	if (bStateA || bHeatA) mA171sOff();
	mA171s_NLOn();
	bprevStateA = 255;
	bprevHeatA = 255;
	bprevStNlA = 255;
	}
	r4slpcoma = 0;
	t_lasta_us = ~t_lasta_us;
	break;
	case 6:             //power coffee on
	r4slpresa = 1;
	if ((DEV_TYPA > 11) && (DEV_TYPA < 16)) {
	if (r4slppar1a == 1) mA103sToff();
	else if (r4slppar1a == 2) mA103sTon();
	bprevProgA = 254;
	}
	bprevStateA = 255;
	mA103sOn();
	r4slpcoma = 0;
	t_lasta_us = ~t_lasta_us;
	break;
	case 7:             //power lock off
	r4slpresa = 1;
	bprevStateA = 255;
	bprevLockA = 255;
	if (( DEV_TYPA > 9) && ( DEV_TYPA < 12 )) mA103sLoff();
	else if (( DEV_TYPA > 11 ) && ( DEV_TYPA < 16 )) mA151sLoff();
	r4slpcoma = 0;
	t_lasta_us = ~t_lasta_us;
	break;
	case 8:             //power lock on
	r4slpresa = 1;
	bprevStateA = 255;
	bprevLockA = 255;
	if (( DEV_TYPA > 9) && ( DEV_TYPA < 12 )) mA103sLon();
	else if (( DEV_TYPA > 11 ) && ( DEV_TYPA < 16 )) mA151sLon();
	r4slpcoma = 0;
	t_lasta_us = ~t_lasta_us;
	break;
	case 9:             //coffee strength
	r4slpresa = 1;
	if (r4slppar1a == 0) mA103sToff();
	else if (r4slppar1a == 1) mA103sTon();
	bprevAwarmA = 255;
	r4slpcoma = 0;
	t_lasta_us = ~t_lasta_us;
	break;

	case 10:	//800 on off		
	r4slpresa = 1;
	bprevStateA = 255;
	if (r4slppar1a) rmA800sOn();
	else rmA800sOff();
	r4slpcoma = 0;
	if (DEV_TYPA < 24) {
	bDHourA = 0;
	bDMinA = 0;
	}
	t_lasta_us = ~t_lasta_us;
	break;

	case 11:	//800 prog
	r4slpresa = 1;
	rmA800sOff();
	bprevProgA = 254;
	bprevModProgA = 255;
	bprevPHourA = 255;
	bprevPMinA = 255;
	bprevCHourA = 255;
	bprevCMinA = 255;
	bprevDHourA = 255;
	bprevDMinA = 255;
	bprevAwarmA = 255;
	if (r4slppar1a < 128) rmA800sProg(r4slppar1a);
	else if (DEV_TYPA < 24) {
	bDHourA = 0;
	bDMinA = 0;
	}
	r4slpcoma = 0;
	t_lasta_us = ~t_lasta_us;
	break;

	case 12:	//800 mode
	r4slpresa = 1;
	bprevModProgA = 255;
        rmA800sMod(r4slppar1a);
	r4slpcoma = 0;
	t_lasta_us = ~t_lasta_us;
	break;

	case 13:	//800 temp
	r4slpresa = 1;
	bprevHtempA = 255;
        rmA800sTemp(r4slppar1a);
	r4slpcoma = 0;
	t_lasta_us = ~t_lasta_us;
	break;

	case 14:	//800 phour
	r4slpresa = 1;
	bprevPHourA = 255;
        rmA800sPhour(r4slppar1a);
	r4slpcoma = 0;
	t_lasta_us = ~t_lasta_us;
	break;

	case 15:	//800 pmin
	r4slpresa = 1;
	bprevPMinA = 255;
        rmA800sPmin(r4slppar1a);
	r4slpcoma = 0;
	t_lasta_us = ~t_lasta_us;
	break;

	case 16:	//800 awarm
	r4slpresa = 1;
	bprevAwarmA = 255;
        rmA800sAwarm(r4slppar1a);
	r4slpcoma = 0;
	t_lasta_us = ~t_lasta_us;
	break;

	case 17:	//800 set all
	r4slpresa = 1;
	bprevProgA = 254;
	bprevModProgA = 255;
	if (r4slppar1a > 127) {
	rmA800sOff();
	if (DEV_TYPA < 24) {
	bDHourA = 0;
	bDMinA = 0;
	}
	}
	else rmA800sPall(r4slppar1a, r4slppar2a, r4slppar3a, r4slppar4a, r4slppar5a, r4slppar7a, r4slppar8a, r4slppar6a);
	r4slpcoma = 0;
	t_lasta_us = ~t_lasta_us;
	break;

	case 18:	//1519 delay start
	r4slpresa = 1;
	if (bStateA) mA171sOff();
	if (r4slppar1a == 1) mA103sToff();
	else if (r4slppar1a == 2) mA103sTon();
	mA151sDon(r4slppar2a, r4slppar3a);
	bprevProgA = 254;
	bprevStateA = 255;
	r4slpcoma = 0;
	t_lasta_us = ~t_lasta_us;
	break;

	case 19:	//800 dhour
	r4slpresa = 1;
	bprevDHourA = 255;
        rmA800sDhour(r4slppar1a);
	r4slpcoma = 0;
	t_lasta_us = ~t_lasta_us;
	break;

	case 20:	//800 dmin
	r4slpresa = 1;
	bprevDMinA = 255;
        rmA800sDmin(r4slppar1a);
	r4slpcoma = 0;
	t_lasta_us = ~t_lasta_us;
	break;

	case 21:	//bklight off
	r4slpresa = 1;
        bprevStBlA = 255;
	mA171Bl(0);
	r4slpcoma = 0;
	t_lasta_us = ~t_lasta_us;
	break;

	case 22:	//bklight on
	r4slpresa = 1;
        bprevStBlA = 255;
	mA171Bl(1);
	r4slpcoma = 0;
	t_lasta_us = ~t_lasta_us;
	break;

	case 23:	//beep off
	r4slpresa = 1;
        bprevStBpA = 255;
	mA171Bp(0);
	r4slpcoma = 0;
	t_lasta_us = ~t_lasta_us;
	break;

	case 24:	//beep on
	r4slpresa = 1;
        bprevStBpA = 255;
	mA171Bp(1);
	r4slpcoma = 0;
	t_lasta_us = ~t_lasta_us;
	break;

	case 64:	//mi off
	r4slpresa = 1;
	bprevHtempA = 255;
	bprevStateA = 255;
	bprevHeatA = 255;
	mAMiOff();
	r4slpcoma = 0;
	t_lasta_us = ~t_lasta_us;
	break;

	case 65:	//mi boil
	r4slpresa = 1;
	bprevHtempA = 255;
	bprevStateA = 255;
	bprevHeatA = 255;
	mAMiBoil();
	r4slpcoma = 0;
	t_lasta_us = ~t_lasta_us;
	break;

	case 66:	//mi temp
	r4slpresa = 1;
	bprevHtempA = 255;
	bprevStateA = 255;
	bprevHeatA = 255;
	mAMiHeat(r4slppar1a);
	r4slpcoma = 0;
	t_lasta_us = ~t_lasta_us;
	break;
	}
	}

        vTaskDelay(100 / portTICK_PERIOD_MS);
	t_before_us = esp_timer_get_time();
// every 4s get kettle state
	if (t_before_us - t_lasta_us > 4000000) {
        MqAState();
	r4slpresa = 0;
	if (f_SyncA) mkSyncA();	
#ifdef USE_TFT
	if (tft_conn) tfblestate();
#endif
	t_lasta_us = t_before_us;	
	}
// every 1s display time and date
	if ((tft_conn) && (t_before_us - t_clock_us > 1000000)) {
#ifdef USE_TFT
	tftclock();
#endif
	t_clock_us = t_before_us;	
	}


	if (r4slpcomb) {
	switch (r4slpcomb) {
	case 1:             //off
	r4slpresb = 1;
	if ((DEV_TYPB < 10) && (bProgB || bHeatB)) {
	mB171sOff();
	mB171s_ModOff();
	}
	if ((DEV_TYPB > 11) && (DEV_TYPB < 16)) {
	if (r4slppar1b == 1) mB103sToff();
	else if (r4slppar1b == 2) mB103sTon();
	bprevProgB = 254;
	}
	bprevHtempB = 255;
	bprevStateB = 255;
	bprevHeatB = 255;
	bprevStNlB = 255;
	mB171sOff();
	r4slpcomb = 0;
	t_lastb_us = ~t_lastb_us;
	break;
	case 2:             //boil
	r4slpresb = 1;
	if ((DEV_TYPB < 10) && bHtempB && (bHtempB < 91)) mB171sHeat(0);
	mB171sOff();
	bprevHtempB = 255;
	bprevStateB = 255;
	bprevHeatB = 255;
	bprevStNlB = 255;
	mB171sBoil();
	r4slpcomb = 0;
	t_lastb_us = ~t_lastb_us;
	break;
	case 3:             //heat
	r4slpresb = 1;
	mB171sOff();
	bprevHtempB = 255;
	bprevStateB = 255;
	bprevHeatB = 255;
	bprevStNlB = 255;
	mB171sHeat(r4slppar1b);
	r4slpcomb = 0;
	t_lastb_us = ~t_lastb_us;
	break;
	case 4:             //boil&heat
	r4slpresb = 1;
	mB171sOff();
	if (r4slppar1b == 0) mB171sBoil();
	else mB171sBoilAndHeat(r4slppar1b);
	bprevHtempB = 255;
	bprevStateB = 255;
	bprevHeatB = 255;
	bprevStNlB = 255;
	r4slpcomb = 0;
	t_lastb_us = ~t_lastb_us;
	break;
	case 5:             //nightlight
	r4slpresb = 1;
	if (DEV_TYPB > 3) {
	nl_settings[3]=RgbRB;
	nl_settings[4]=RgbGB;
	nl_settings[5]=RgbBB;
	nl_settings[8]=RgbRB;
	nl_settings[9]=RgbGB;
	nl_settings[10]=RgbBB;
	nl_settings[13]=RgbRB;
	nl_settings[14]=RgbGB;
	nl_settings[15]=RgbBB;    
	if (bStateB || bHeatB) mB171sOff();
	mB171s_NLOn();
	bprevStateB = 255;
	bprevHeatB = 255;
	bprevStNlB = 255;
	}
	r4slpcomb = 0;
	t_lastb_us = ~t_lastb_us;
	break;
	case 6:             //power coffee on
	r4slpresb = 1;
	if ((DEV_TYPB > 11) && (DEV_TYPB < 16)) {
	if (r4slppar1b == 1) mB103sToff();
	else if (r4slppar1b == 2) mB103sTon();
	bprevProgB = 254;
	}
	bprevStateB = 255;
	mB103sOn();
	r4slpcomb = 0;
	t_lastb_us = ~t_lastb_us;
	break;
	case 7:             //power lock off
	r4slpresb = 1;
	bprevStateB = 255;
	bprevLockB = 255;
	if (( DEV_TYPB > 9) && ( DEV_TYPB < 12 )) mB103sLoff();
	else if (( DEV_TYPB > 11 ) && ( DEV_TYPB < 16 )) mB151sLoff();
	r4slpcomb = 0;
	t_lastb_us = ~t_lastb_us;
	break;
	case 8:             //power lock on
	r4slpresb = 1;
	bprevStateB = 255;
	bprevLockB = 255;
	if (( DEV_TYPB > 9) && ( DEV_TYPB < 12 )) mB103sLon();
	else if (( DEV_TYPB > 11 ) && ( DEV_TYPB < 16 )) mB151sLon();
	r4slpcomb = 0;
	t_lastb_us = ~t_lastb_us;
	break;
	case 9:             //coffee strength
	r4slpresb = 1;
	if (r4slppar1b == 0) mB103sToff();
	else if (r4slppar1b == 1) mB103sTon();
	bprevAwarmB = 255;
	r4slpcomb = 0;
	t_lastb_us = ~t_lastb_us;
	break;

	case 10:	//800 on off		
	r4slpresb = 1;
	bprevStateB = 255;
	if (r4slppar1b) rmB800sOn();
	else rmB800sOff();
	r4slpcomb = 0;
	if (DEV_TYPB < 24) {
	bDHourB = 0;
	bDMinB = 0;
	}
	t_lastb_us = ~t_lastb_us;
	break;

	case 11:	//800 prog
	r4slpresb = 1;
	rmB800sOff();
	bprevProgB = 254;
	bprevModProgB = 255;
	bprevPHourB = 255;
	bprevPMinB = 255;
	bprevCHourB = 255;
	bprevCMinB = 255;
	bprevDHourB = 255;
	bprevDMinB = 255;
	bprevAwarmB = 255;
	if (r4slppar1b < 128) rmB800sProg(r4slppar1b);
	else if (DEV_TYPB < 24) {
	bDHourB = 0;
	bDMinB = 0;
	}
	r4slpcomb = 0;
	t_lastb_us = ~t_lastb_us;
	break;

	case 12:	//800 mode
	r4slpresb = 1;
	bprevModProgB = 255;
        rmB800sMod(r4slppar1b);
	r4slpcomb = 0;
	t_lastb_us = ~t_lastb_us;
	break;

	case 13:	//800 temp
	r4slpresb = 1;
	bprevHtempB = 255;
        rmB800sTemp(r4slppar1b);
	r4slpcomb = 0;
	t_lastb_us = ~t_lastb_us;
	break;

	case 14:	//800 phour
	r4slpresb = 1;
	bprevPHourB = 255;
        rmB800sPhour(r4slppar1b);
	r4slpcomb = 0;
	t_lastb_us = ~t_lastb_us;
	break;

	case 15:	//800 pmin
	r4slpresb = 1;
	bprevPMinB = 255;
        rmB800sPmin(r4slppar1b);
	r4slpcomb = 0;
	t_lastb_us = ~t_lastb_us;
	break;

	case 16:	//800 awarm
	r4slpresb = 1;
	bprevAwarmB = 255;
        rmB800sAwarm(r4slppar1b);
	r4slpcomb = 0;
	t_lastb_us = ~t_lastb_us;
	break;

	case 17:	//800 set all
	r4slpresb = 1;
	bprevProgB = 254;
	bprevModProgB = 255;
	if (r4slppar1b > 127) {
	rmB800sOff();
	if (DEV_TYPB < 24) {
	bDHourB = 0;
	bDMinB = 0;
	}
	}
	else rmB800sPall(r4slppar1b, r4slppar2b, r4slppar3b, r4slppar4b, r4slppar5b, r4slppar7b, r4slppar8b, r4slppar6b);
	r4slpcomb = 0;
	t_lastb_us = ~t_lastb_us;
	break;

	case 18:	//1519 delay start
	r4slpresb = 1;
	if (bStateB) mB171sOff();
	if (r4slppar1b == 1) mB103sToff();
	else if (r4slppar1b == 2) mB103sTon();
	mB151sDon(r4slppar2b, r4slppar3b);
	bprevProgB = 254;
	bprevStateB = 255;
	r4slpcomb = 0;
	t_lastb_us = ~t_lastb_us;
	break;

	case 19:	//800 dhour
	r4slpresb = 1;
	bprevDHourB = 255;
        rmB800sDhour(r4slppar1b);
	r4slpcomb = 0;
	t_lastb_us = ~t_lastb_us;
	break;

	case 20:	//800 dmin
	r4slpresb = 1;
	bprevDMinB = 255;
        rmB800sDmin(r4slppar1b);
	r4slpcomb = 0;
	t_lastb_us = ~t_lastb_us;
	break;

	case 21:	//bklight off
	r4slpresb = 1;
        bprevStBlB = 255;
	mB171Bl(0);
	r4slpcomb = 0;
	t_lastb_us = ~t_lastb_us;
	break;

	case 22:	//bklight on
	r4slpresb = 1;
        bprevStBlB = 255;
	mB171Bl(1);
	r4slpcomb = 0;
	t_lastb_us = ~t_lastb_us;
	break;

	case 23:	//beep off
	r4slpresb = 1;
        bprevStBpB = 255;
	mB171Bp(0);
	r4slpcomb = 0;
	t_lastb_us = ~t_lastb_us;
	break;

	case 24:	//beep on
	r4slpresb = 1;
        bprevStBpB = 255;
	mB171Bp(1);
	r4slpcomb = 0;
	t_lastb_us = ~t_lastb_us;
	break;

	case 64:	//mi off
	r4slpresb = 1;
	bprevHtempB = 255;
	bprevStateB = 255;
	bprevHeatB = 255;
	mBMiOff();
	r4slpcomb = 0;
	t_lastb_us = ~t_lastb_us;
	break;

	case 65:	//mi boil
	r4slpresb = 1;
	bprevHtempB = 255;
	bprevStateB = 255;
	bprevHeatB = 255;
	mBMiBoil();
	r4slpcomb = 0;
	t_lastb_us = ~t_lastb_us;
	break;

	case 66:	//mi temp
	r4slpresb = 1;
	bprevHtempB = 255;
	bprevStateB = 255;
	bprevHeatB = 255;
	mBMiHeat(r4slppar1b);
	r4slpcomb = 0;
	t_lastb_us = ~t_lastb_us;
	break;
	}
	}

        vTaskDelay(100 / portTICK_PERIOD_MS);
	t_before_us = esp_timer_get_time();
// every 4s get kettle state
	if (t_before_us - t_lastb_us > 4000000) {
        MqBState();
	r4slpresb = 0;
	if (f_SyncB) mkSyncB();	
#ifdef USE_TFT
	if (tft_conn) tfblestate();
#endif
	t_lastb_us = t_before_us;	
	}
// every 1s display time and date
	if ((tft_conn) && (t_before_us - t_clock_us > 1000000)) {
#ifdef USE_TFT
	tftclock();
#endif
	t_clock_us = t_before_us;	
	}


	if (r4slpcomc) {
	switch (r4slpcomc) {
	case 1:             //off
	r4slpresc = 1;
	if ((DEV_TYPC < 10) && (bProgC || bHeatC)) {
	mC171sOff();
	mC171s_ModOff();
	}
	if ((DEV_TYPC > 11) && (DEV_TYPC < 16)) {
	if (r4slppar1c == 1) mC103sToff();
	else if (r4slppar1c == 2) mC103sTon();
	bprevProgC = 254;
	}
	bprevHtempC = 255;
	bprevStateC = 255;
	bprevHeatC = 255;
	bprevStNlC = 255;
	mC171sOff();
	r4slpcomc = 0;
	t_lastc_us = ~t_lastc_us;
	break;
	case 2:             //boil
	r4slpresc = 1;
	if ((DEV_TYPC < 10) && bHtempC && (bHtempC < 91)) mC171sHeat(0);
	mC171sOff();
	bprevHtempC = 255;
	bprevStateC = 255;
	bprevHeatC = 255;
	bprevStNlC = 255;
	mC171sBoil();
	r4slpcomc = 0;
	t_lastc_us = ~t_lastc_us;
	break;
	case 3:             //heat
	r4slpresc = 1;
	mC171sOff();
	bprevHtempC = 255;
	bprevStateC = 255;
	bprevHeatC = 255;
	bprevStNlC = 255;
	mC171sHeat(r4slppar1c);
	r4slpcomc = 0;
	t_lastc_us = ~t_lastc_us;
	break;
	case 4:             //boil&heat
	r4slpresc = 1;
	mC171sOff();
	if (r4slppar1c == 0) mC171sBoil();
	else mC171sBoilAndHeat(r4slppar1c);
	bprevHtempC = 255;
	bprevStateC = 255;
	bprevHeatC = 255;
	bprevStNlC = 255;
	r4slpcomc = 0;
	t_lastc_us = ~t_lastc_us;
	break;
	case 5:             //nightlight
	r4slpresc = 1;
	if (DEV_TYPC > 3) {
	nl_settings[3]=RgbRC;
	nl_settings[4]=RgbGC;
	nl_settings[5]=RgbBC;
	nl_settings[8]=RgbRC;
	nl_settings[9]=RgbGC;
	nl_settings[10]=RgbBC;
	nl_settings[13]=RgbRC;
	nl_settings[14]=RgbGC;
	nl_settings[15]=RgbBC;    
	if (bStateC || bHeatC) mC171sOff();
	mC171s_NLOn();
	bprevStateC = 255;
	bprevHeatC = 255;
	bprevStNlC = 255;
	}
	r4slpcomc = 0;
	t_lastc_us = ~t_lastc_us;
	break;
	case 6:             //power coffee on
	r4slpresc = 1;
	if ((DEV_TYPC > 11) && (DEV_TYPC < 16)) {
	if (r4slppar1c == 1) mC103sToff();
	else if (r4slppar1c == 2) mC103sTon();
	bprevProgC = 254;
	}
	bprevStateC = 255;
	mC103sOn();
	r4slpcomc = 0;
	t_lastc_us = ~t_lastc_us;
	break;
	case 7:             //power lock off
	r4slpresc = 1;
	bprevStateC = 255;
	bprevLockC = 255;
	if (( DEV_TYPC > 9) && ( DEV_TYPC < 12 )) mC103sLoff();
	else if (( DEV_TYPC > 11 ) && ( DEV_TYPC < 16 )) mC151sLoff();
	r4slpcomc = 0;
	t_lastc_us = ~t_lastc_us;
	break;
	case 8:             //power lock on
	r4slpresc = 1;
	bprevStateC = 255;
	bprevLockC = 255;
	if (( DEV_TYPC > 9) && ( DEV_TYPC < 12 )) mC103sLon();
	else if (( DEV_TYPC > 11 ) && ( DEV_TYPC < 16 )) mC151sLon();
	r4slpcomc = 0;
	t_lastc_us = ~t_lastc_us;
	break;
	case 9:             //coffee strength
	r4slpresc = 1;
	if (r4slppar1c == 0) mC103sToff();
	else if (r4slppar1c == 1) mC103sTon();
	bprevAwarmC = 255;
	r4slpcomc = 0;
	t_lastc_us = ~t_lastc_us;
	break;

	case 10:	//800 on off		
	r4slpresc = 1;
	bprevStateC = 255;
	if (r4slppar1c) rmC800sOn();
	else rmC800sOff();
	r4slpcomc = 0;
	if (DEV_TYPC < 24) {
	bDHourC = 0;
	bDMinC = 0;
	}
	t_lastc_us = ~t_lastc_us;
	break;

	case 11:	//800 prog
	r4slpresc = 1;
	rmC800sOff();
	bprevProgC = 254;
	bprevModProgC = 255;
	bprevPHourC = 255;
	bprevPMinC = 255;
	bprevCHourC = 255;
	bprevCMinC = 255;
	bprevDHourC = 255;
	bprevDMinC = 255;
	bprevAwarmC = 255;
	if (r4slppar1c < 128) rmC800sProg(r4slppar1c);
	else if (DEV_TYPC < 24) {
	bDHourC = 0;
	bDMinC = 0;
	}
	r4slpcomc = 0;
	t_lastc_us = ~t_lastc_us;
	break;

	case 12:	//800 mode
	r4slpresc = 1;
	bprevModProgC = 255;
        rmC800sMod(r4slppar1c);
	r4slpcomc = 0;
	t_lastc_us = ~t_lastc_us;
	break;

	case 13:	//800 temp
	r4slpresc = 1;
	bprevHtempC = 255;
        rmC800sTemp(r4slppar1c);
	r4slpcomc = 0;
	t_lastc_us = ~t_lastc_us;
	break;

	case 14:	//800 phour
	r4slpresc = 1;
	bprevPHourC = 255;
        rmC800sPhour(r4slppar1c);
	r4slpcomc = 0;
	t_lastc_us = ~t_lastc_us;
	break;

	case 15:	//800 pmin
	r4slpresc = 1;
	bprevPMinC = 255;
        rmC800sPmin(r4slppar1c);
	r4slpcomc = 0;
	t_lastc_us = ~t_lastc_us;
	break;

	case 16:	//800 awarm
	r4slpresc = 1;
	bprevAwarmC = 255;
        rmC800sAwarm(r4slppar1c);
	r4slpcomc = 0;
	t_lastc_us = ~t_lastc_us;
	break;

	case 17:	//800 set all
	r4slpresc = 1;
	bprevProgC = 254;
	bprevModProgC = 255;
	if (r4slppar1c > 127) {
	rmC800sOff();
	if (DEV_TYPC < 24) {
	bDHourC = 0;
	bDMinC = 0;
	}
	}
	else rmC800sPall(r4slppar1c, r4slppar2c, r4slppar3c, r4slppar4c, r4slppar5c, r4slppar7c, r4slppar8c, r4slppar6c);
	r4slpcomc = 0;
	t_lastc_us = ~t_lastc_us;
	break;

	case 18:	//1519 delay start
	r4slpresc = 1;
	if (bStateC) mC171sOff();
	if (r4slppar1c == 1) mC103sToff();
	else if (r4slppar1c == 2) mC103sTon();
	mC151sDon(r4slppar2c, r4slppar3c);
	bprevProgC = 254;
	bprevStateC = 255;
	r4slpcomc = 0;
	t_lastc_us = ~t_lastc_us;
	break;

	case 19:	//800 dhour
	r4slpresc = 1;
	bprevDHourC = 255;
        rmC800sDhour(r4slppar1c);
	r4slpcomc = 0;
	t_lastc_us = ~t_lastc_us;
	break;

	case 20:	//800 dmin
	r4slpresc = 1;
	bprevDMinC = 255;
        rmC800sDmin(r4slppar1c);
	r4slpcomc = 0;
	t_lastc_us = ~t_lastc_us;
	break;

	case 21:	//bklight off
	r4slpresc = 1;
        bprevStBlC = 255;
	mC171Bl(0);
	r4slpcomc = 0;
	t_lastc_us = ~t_lastc_us;
	break;

	case 22:	//bklight on
	r4slpresc = 1;
        bprevStBlC = 255;
	mC171Bl(1);
	r4slpcomc = 0;
	t_lastc_us = ~t_lastc_us;
	break;

	case 23:	//beep off
	r4slpresc = 1;
        bprevStBpC = 255;
	mC171Bp(0);
	r4slpcomc = 0;
	t_lastc_us = ~t_lastc_us;
	break;

	case 24:	//beep on
	r4slpresc = 1;
        bprevStBpC = 255;
	mC171Bp(1);
	r4slpcomc = 0;
	t_lastc_us = ~t_lastc_us;
	break;

	case 64:	//mi off
	r4slpresc = 1;
	bprevHtempC = 255;
	bprevStateC = 255;
	bprevHeatC = 255;
	mCMiOff();
	r4slpcomc = 0;
	t_lastc_us = ~t_lastc_us;
	break;

	case 65:	//mi boil
	r4slpresc = 1;
	bprevHtempC = 255;
	bprevStateC = 255;
	bprevHeatC = 255;
	mCMiBoil();
	r4slpcomc = 0;
	t_lastc_us = ~t_lastc_us;
	break;

	case 66:	//mi temp
	r4slpresc = 1;
	bprevHtempC = 255;
	bprevStateC = 255;
	bprevHeatC = 255;
	mCMiHeat(r4slppar1c);
	r4slpcomc = 0;
	t_lastc_us = ~t_lastc_us;
	break;
	}
	}


        vTaskDelay(100 / portTICK_PERIOD_MS);
	t_before_us = esp_timer_get_time();
// every 4s get kettle state
	if (t_before_us - t_lastc_us > 4000000) {
        MqCState();
	r4slpresc = 0;
	if (f_SyncC) mkSyncC();	
#ifdef USE_TFT
	if (tft_conn) tfblestate();
#endif
	t_lastc_us = t_before_us;	
	}
// every 1s display time and date
	if ((tft_conn) && (t_before_us - t_clock_us > 1000000)) {
#ifdef USE_TFT
	tftclock();
#endif
	t_clock_us = t_before_us;	
	}

	if (t_before_us - t_lasts_us > 8000000) {
	MqSState();
	t_lasts_us = t_before_us;	
	}

	if (!f_update && !r4slpcoma && !r4slpcomb && !r4slpcomc && tft_conn && jpg_time && (t_before_us - t_jpg_us > jpg_time*1000000)) {
#ifdef USE_TFT
	ret = tftjpg();
	if (ret) {
	t_lasta_us = t_before_us;
	t_lastb_us = t_before_us;
	t_lastc_us = t_before_us;
	t_tinc_us = t_before_us;
	}
#endif
	t_jpg_us = t_before_us;	
	}

	if (t_before_us - t_tinc_us > 4000000) {
	blstnum_inc();
#ifdef USE_TFT
	if (tft_conn) tfblestate();
	if (tft_conn) tftclock();
	t_clock_us = t_before_us;	
#endif
	if (f_update && tft_conn) tfblestate();
	t_tinc_us = t_before_us;	
        if (ble_mon && !f_update && REQ_NAMEA[0] && NumConnA && !btauthoriza) {
	if (floop) floop--;
	} else if (ble_mon && !f_update && REQ_NAMEB[0] && NumConnB && !btauthorizb) {
	if (floop) floop--;
	} else if (ble_mon && !f_update && REQ_NAMEC[0] && NumConnC && !btauthorizc) {
	if (floop) floop--;
	} else if (s_retry_num >= WIFI_MAXIMUM_RETRY) {
	ESP_LOGI(AP_TAG,"Wifi disconnected. Restarting now...");
	fflush(stdout);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
	floop = 0;
	} else floop = 16;

	}

}
	esp_restart();
}
