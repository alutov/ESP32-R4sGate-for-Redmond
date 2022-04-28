
/* 
****************************************************************
	ESP32 r4sGate for Redmond+ main
	Lutov Andrey  Donetsk
Use for compilation ESP-IDF Programming Guide:
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/
****************************************************************
*/
#define AP_VER "2022.04.28"


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
void bin2hex(const unsigned char *bin, char *out, size_t len, uint8_t del)
{
	size_t  i;
	if (bin == NULL || out == NULL || len == 0) return;
	if(!del) {
	for (i=0; i<len; i++) {
		out[i*2]   = "0123456789abcdef"[bin[i] >> 4];
		out[i*2+1] = "0123456789abcdef"[bin[i] & 0x0F];
	}
	out[len*2] = '\0';
	} else {
	for (i=0; i<len; i++) {
		out[i*3]   = "0123456789abcdef"[bin[i] >> 4];
		out[i*3+1] = "0123456789abcdef"[bin[i] & 0x0F];
		out[i*3+2] = del;
	}
	out[len*3-1] = '\0';
	}
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

int64_t t_before_us = 0;

static struct BleDevSt BleDevStA;
static struct BleDevSt BleDevStB;
static struct BleDevSt BleDevStC;
/*
struct BleDevStA *pBleDevStA;
struct BleDevStB *pBleDevStB;
struct BleDevStC *pBleDevStC;
*/

static struct BleMonRec BleMR[BleMonNum];
static struct BleMonExt BleMX[BleMonNum];

#include "tft/tft.c"

//******************* timer *********************
static intr_handle_t s_timer_handle;
static void hw_timer_callback(void *arg)
{
//Reset irq and set for next time
	TIMERG0.int_clr_timers.t0 = 1;
	TIMERG0.hw_timer[0].config.alarm_en = 1;
//
        if (hwtdiv > 3) {
	for (int i = 0; i < BleMonNum; i++) {
	if (BleMX[i].ttick) {
	BleMX[i].ttick--;
	if (!BleMX[i].ttick) {
	t_lasts_us = ~t_lasts_us;
        BleMX[i].advdatlen = 0;
	BleMX[i].scrsplen = 0;
	BleMX[i].state = 0;
	BleMX[i].rssi = 0;
	BleMX[i].par1 = 0;
	BleMX[i].par2 = 0;
	BleMX[i].par3 = 0;
	BleMX[i].par4 = 0;
	BleMX[i].par5 = 0;
	BleMX[i].par6 = 0;
	}
	}
	}
	hwtdiv = 0;
	} else hwtdiv++;
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
	if (!lvgpio1 && BleDevStA.btauthoriz) {
	if ((BleDevStA.DEV_TYP > 0) && (BleDevStA.DEV_TYP < 10)) { 
	if (!BleDevStA.bState) {
	BleDevStA.r4slppar1 = 0;
	BleDevStA.r4slpcom = 2;
	} else {
	BleDevStA.r4slppar1 = 0;
	BleDevStA.r4slpcom = 1;
	}
	} else if ((BleDevStA.DEV_TYP > 9) && (BleDevStA.DEV_TYP < 16)) { 
	if (!BleDevStA.bState) {
	BleDevStA.r4slppar1 = 0;
	BleDevStA.r4slpcom = 6;
	} else {
	BleDevStA.r4slppar1 = 0;
	BleDevStA.r4slpcom = 1;
	}
	} else if ((BleDevStA.DEV_TYP > 15) && (BleDevStA.DEV_TYP < 63)) { 
	if (!BleDevStA.bState) {
	BleDevStA.r4slppar1 = 1;
	BleDevStA.r4slpcom = 10;
	} else {
	BleDevStA.r4slppar1 = 0;
	BleDevStA.r4slpcom = 10;
	}
	} else if ((BleDevStA.DEV_TYP == 63)) { 
	if (!BleDevStA.bState) {
	BleDevStA.bState = 1;
	BleDevStA.r4slppar1 = 0;
	BleDevStA.r4slpcom = 63;
	}
	} else if ((BleDevStA.DEV_TYP > 63) && (BleDevStA.DEV_TYP < 70)) { 
	if (!BleDevStA.bState) {
	BleDevStA.r4slppar1 = 0;
	BleDevStA.r4slpcom = 65;
	} else {
	BleDevStA.r4slppar1 = 0;
	BleDevStA.r4slpcom = 64;
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
	if (!lvgpio2 && BleDevStB.btauthoriz) {
	if ((BleDevStB.DEV_TYP > 0) && (BleDevStB.DEV_TYP < 10)) { 
	if (!BleDevStB.bState) {
	BleDevStB.r4slppar1 = 0;
	BleDevStB.r4slpcom = 2;
	} else {
	BleDevStB.r4slppar1 = 0;
	BleDevStB.r4slpcom = 1;
	}
	} else if ((BleDevStB.DEV_TYP > 9) && (BleDevStB.DEV_TYP < 16)) { 
	if (!BleDevStB.bState) {
	BleDevStB.r4slppar1 = 0;
	BleDevStB.r4slpcom = 6;
	} else {
	BleDevStB.r4slppar1 = 0;
	BleDevStB.r4slpcom = 1;
	}
	} else if ((BleDevStB.DEV_TYP > 15) && (BleDevStB.DEV_TYP < 63)) { 
	if (!BleDevStB.bState) {
	BleDevStB.r4slppar1 = 1;
	BleDevStB.r4slpcom = 10;
	} else {
	BleDevStB.r4slppar1 = 0;
	BleDevStB.r4slpcom = 10;
	}
	} else if ((BleDevStB.DEV_TYP == 63)) { 
	if (!BleDevStB.bState) {
	BleDevStB.bState = 1;
	BleDevStB.r4slppar1 = 0;
	BleDevStB.r4slpcom = 63;
	}
	} else if ((BleDevStB.DEV_TYP > 63) && (BleDevStB.DEV_TYP < 70)) { 
	if (!BleDevStB.bState) {
	BleDevStB.r4slppar1 = 0;
	BleDevStB.r4slpcom = 65;
	} else {
	BleDevStB.r4slppar1 = 0;
	BleDevStB.r4slpcom = 64;
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
	if (!lvgpio3 && BleDevStC.btauthoriz) {
	if ((BleDevStC.DEV_TYP > 0) && (BleDevStC.DEV_TYP < 10)) { 
	if (!BleDevStC.bState) {
	BleDevStC.r4slppar1 = 0;
	BleDevStC.r4slpcom = 2;
	} else {
	BleDevStC.r4slppar1 = 0;
	BleDevStC.r4slpcom = 1;
	}
	} else if ((BleDevStC.DEV_TYP > 9) && (BleDevStC.DEV_TYP < 16)) { 
	if (!BleDevStC.bState) {
	BleDevStC.r4slppar1 = 0;
	BleDevStC.r4slpcom = 6;
	} else {
	BleDevStC.r4slppar1 = 0;
	BleDevStC.r4slpcom = 1;
	}
	} else if ((BleDevStC.DEV_TYP > 15) && (BleDevStC.DEV_TYP < 63)) { 
	if (!BleDevStC.bState) {
	BleDevStC.r4slppar1 = 1;
	BleDevStC.r4slpcom = 10;
	} else {
	BleDevStC.r4slppar1 = 0;
	BleDevStC.r4slpcom = 10;
	}
	} else if ((BleDevStC.DEV_TYP == 63)) { 
	if (!BleDevStC.bState) {
	BleDevStC.bState = 1;
	BleDevStC.r4slppar1 = 0;
	BleDevStC.r4slpcom = 63;
	}
	} else if ((BleDevStC.DEV_TYP > 63) && (BleDevStC.DEV_TYP < 70)) { 
	if (!BleDevStC.bState) {
	BleDevStC.r4slppar1 = 0;
	BleDevStC.r4slpcom = 65;
	} else {
	BleDevStC.r4slppar1 = 0;
	BleDevStC.r4slpcom = 64;
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
	case 192:
	if ((gpio_get_level(bgpio5 & 0x3f)) != lvgpio5) {
        cntgpio5++;
	if (cntgpio5 > 5) {
	fgpio5 = 1;
	lvgpio5 = lvgpio5 ^ 1;
	t_lasts_us = ~t_lasts_us;
	cntgpio5 = 0;
	t_jpg_us = ~t_jpg_us;	
	}
	} else cntgpio5 = 0;
	break;

	}
}

//******************** ble hass discovery data **********************
void MqttPubSub (uint8_t blenum, bool mqtttst) {
	uint8_t blenum1 = blenum + 1;
	char tbuff[16]; 
	if (blenum > 2) return;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	if (!mqtdel && ptr->tBLEAddr[0] && mqtttst && ptr->DEV_TYP) {
	char buft[64];
	char bufd[2048];
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/status");
	esp_mqtt_client_publish(mqttclient, buft, "online", 0, 1, 1);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/name");
	esp_mqtt_client_publish(mqttclient, buft, ptr->DEV_NAME, 0, 1, 1);

	if ((ptr->DEV_TYP < 10) || (ptr->DEV_TYP > 63)) {
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/boil");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/heat");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/heat_temp");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	if (ptr->DEV_TYP < 10) {
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/backlight");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/beep");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/nightlight");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/nightlight_red");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/nightlight_green");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/nightlight_blue");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/nightlight_rgb");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/boil_time");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	}
	if (FDHass) {
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Kettle.boil\",\"icon\":\"mdi:kettle\",\"uniq_id\":\"boil_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Kettle\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	if (ptr->DEV_TYP < 64 ) strcat(bufd,"Redmond");
	else strcat(bufd,"Xiaomi");
	strcat(bufd,"\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/boil\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/boil\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/4x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Kettle.heat\",\"icon\":\"mdi:kettle\",\"uniq_id\":\"heat_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Kettle\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	if (ptr->DEV_TYP < 64 ) strcat(bufd,"Redmond");
	else strcat(bufd,"Xiaomi");
	strcat(bufd,"\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/heat\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/heat\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/climate/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Kettle.temp\",\"uniq_id\":\"temp_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Kettle\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	if (ptr->DEV_TYP < 64 ) strcat(bufd,"Redmond");
	else strcat(bufd,"Xiaomi");
	strcat(bufd,"\"},\"temperature_command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/heat_temp\",\"temperature_state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/heat_temp\",\"current_temperature_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/temp\",\"min_temp\":\"0\",\"max_temp\":\"100\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\",\"mode_state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/hstate\",\"mode_command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/heat\"");
	strcat(bufd,",\"temp_step\":\"5\",\"modes\":[\"off\",\"heat\"]}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Kettle.rssi\",\"icon\":\"mdi:bluetooth\",\"uniq_id\":\"rssi_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Kettle\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	if (ptr->DEV_TYP < 64 ) strcat(bufd,"Redmond");
	else strcat(bufd,"Xiaomi");
	strcat(bufd,"\"},\"device_class\":\"signal_strength\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/rssi\",\"unit_of_meas\":\"dBm\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	if ((ptr->DEV_TYP < 10) && (ptr->DEV_TYP > 3)) {
	strcpy(buft,"homeassistant/number/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Kettle.boiltime\",\"icon\":\"mdi:kettle\",\"uniq_id\":\"boiltime_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Kettle\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/boil_time\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/boil_time\",\"min\":\"-5\",\"max\":\"5\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/light/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Kettle.light\",\"uniq_id\":\"light_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Kettle\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/nightlight\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/nightlight\",\"rgb_cmd_t\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/nightlight_rgb\",\"rgb_stat_t\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/nightlight_rgb\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/2x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Kettle.beep\",\"icon\":\"mdi:speaker\",\"uniq_id\":\"beep_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Kettle\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	strcat(bufd,"Redmond");
	strcat(bufd,"\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/beep\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/beep\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/3x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Kettle.backlight\",\"icon\":\"mdi:thermometer\",\"uniq_id\":\"backlight_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Kettle\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	strcat(bufd,"Redmond");
	strcat(bufd,"\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/backlight\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/backlight\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/2x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Kettle.energy\",\"icon\":\"mdi:counter\",\"uniq_id\":\"energy_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Kettle\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	strcat(bufd,"Redmond");
	strcat(bufd,"\"},\"device_class\":\"energy\",\"state_class\":\"total_increasing\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/total_energy\",\"unit_of_meas\":\"kWh\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/3x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Kettle.ontime\",\"icon\":\"mdi:timer\",\"uniq_id\":\"ontime_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Kettle\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	strcat(bufd,"Redmond");
	strcat(bufd,"\"},\"state_class\":\"total_increasing\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/working_time\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/4x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Kettle.oncount\",\"icon\":\"mdi:counter\",\"uniq_id\":\"oncount_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Kettle\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	strcat(bufd,"Redmond");
	strcat(bufd,"\"},\"state_class\":\"total_increasing\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/working_count\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/5x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Kettle.volume\",\"icon\":\"mdi:water\",\"uniq_id\":\"volume_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Kettle\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	strcat(bufd,"Redmond");
	strcat(bufd,"\"},\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/volume\",\"unit_of_meas\":\"l\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/6x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Kettle.volumelast\",\"icon\":\"mdi:water\",\"uniq_id\":\"volumelast_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Kettle\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	strcat(bufd,"Redmond");
	strcat(bufd,"\"},\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/volume_last\",\"unit_of_meas\":\"l\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	}
	}
	} else if ( ptr->DEV_TYP < 11) {
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/state");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/lock");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	if (FDHass) {
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Power.switch\",\"icon\":\"mdi:power-plug\",\"uniq_id\":\"switch_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Power_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Power\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/state\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/state\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/2x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Power.lock\",\"icon\":\"mdi:power-plug\",\"uniq_id\":\"lock_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Power_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Power\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/lock\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/lock\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Power.rssi\",\"icon\":\"mdi:bluetooth\",\"uniq_id\":\"rssi_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Power_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Power\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"device_class\":\"signal_strength\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/rssi\",\"unit_of_meas\":\"dBm\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	}
	} else if ( ptr->DEV_TYP < 12) {
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/state");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/lock");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/power");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	if (FDHass) {
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Heater.switch\",\"icon\":\"mdi:radiator\",\"uniq_id\":\"switch_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Heater_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Heater\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/state\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/state\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/2x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Heater.lock\",\"icon\":\"mdi:radiator\",\"uniq_id\":\"lock_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Heater_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Heater\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/lock\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/lock\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/number/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Heater.power\",\"icon\":\"mdi:radiator\",\"uniq_id\":\"power_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Heater_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Heater\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/power\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/power\",\"unit_of_meas\":\"\x25\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Heater.rssi\",\"icon\":\"mdi:bluetooth\",\"uniq_id\":\"rssi_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Heater_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Heater\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"device_class\":\"signal_strength\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/rssi\",\"unit_of_meas\":\"dBm\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	}
	} else if ( ptr->DEV_TYP < 16) {
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/state");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/lock");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/strength");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/delay");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/delay_hour");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/delay_min");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	if (FDHass) {
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Coffee.switch\",\"icon\":\"mdi:coffee\",\"uniq_id\":\"switch_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Coffee_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Coffee\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/state\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/state\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/2x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Coffee.lock\",\"icon\":\"mdi:coffee\",\"uniq_id\":\"lock_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Coffee_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Coffee\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/lock\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/lock\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/3x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Coffee.strength\",\"icon\":\"mdi:coffee\",\"uniq_id\":\"strength_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Coffee_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Coffee\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/strength\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/strength\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Coffee.rssi\",\"icon\":\"mdi:bluetooth\",\"uniq_id\":\"rssi_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Coffee_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Coffee\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"device_class\":\"signal_strength\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/rssi\",\"unit_of_meas\":\"dBm\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//

	}
	} else if ( ptr->DEV_TYP < 63) {
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/state");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/prname");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/prog");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/mode");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/temp");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/set_hour");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/set_min");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/delay_hour");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/delay_min");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/warm");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	if (FDHass) {
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Cooker.switch\",\"icon\":\"mdi:chef-hat\",\"uniq_id\":\"switch_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Cooker\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/state\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/hstate\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/select/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Cooker.program\",\"icon\":\"mdi:form-select\",\"uniq_id\":\"program_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Cooker\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/prname\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/prname\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\",\"options\":");
        if ( ptr->DEV_TYP == 16 ) {
	strcat(bufd,"[\"OFF\",\"Multicooker\",\"Rice\",\"Slow_cooking\",\"Pilaf\",\"Frying_vegetables\"");
	strcat(bufd,",\"Frying_fish\",\"Frying_meat\",\"Stewing_vegetables\",\"Stewing_fish\",\"Stewing_meat\"");
	strcat(bufd,",\"Pasta\",\"Milk_porridge\",\"Soup\",\"Yogurt\",\"Baking\",\"Steam_vegetables\"");
	strcat(bufd,",\"Steam_fish\",\"Steam_meat\",\"Hot\"]}");
	} else if ( ptr->DEV_TYP == 17 ) {
	strcat(bufd,"[\"OFF\",\"Multicooker\",\"Milk_porridge\",\"Stewing\",\"Frying\",\"Soup\"");
	strcat(bufd,",\"Steam\",\"Pasta\",\"Slow_cooking\",\"Hot\",\"Baking\"");
	strcat(bufd,",\"Groats\",\"Pilaf\",\"Yogurt\",\"Pizza\",\"Bread\",\"Desserts\",\"Express\"]}");
	} else if ( ptr->DEV_TYP == 18 ) {
	strcat(bufd,"[\"OFF\",\"Frying\",\"Groats\",\"Multicooker\",\"Pilaf\",\"Steam\"");
	strcat(bufd,",\"Baking\",\"Stewing\",\"Soup\",\"Milk_porridge\",\"Yogurt\",\"Express\"]}");
	} else if ( ptr->DEV_TYP == 19 ) {
	strcat(bufd,"[\"OFF\",\"Groats\",\"Frying\",\"Steam\",\"Baking\",\"Stewing\"");
	strcat(bufd,",\"Multicooker\",\"Pilaf\",\"Soup\",\"Milk_porridge\",\"Yogurt\"]}");
	} else if ( ptr->DEV_TYP == 20 ) {
	strcat(bufd,"[\"OFF\",\"Multicooker\",\"Milk_porridge\",\"Stewing\",\"Frying\",\"Soup\"");
	strcat(bufd,",\"Steam\",\"Pasta\",\"Slow_cooking\",\"Hot\",\"Baking\"");
	strcat(bufd,",\"Groats\",\"Pilaf\",\"Yogurt\",\"Pizza\",\"Bread\",\"Desserts\",\"Express\",\"Warming\"]}");
	} else if ( ptr->DEV_TYP == 24 ) {
	strcat(bufd,"[\"OFF\",\"Multicooker\",\"Omelet\"");
	strcat(bufd,",\"Slow_cooking_meat\",\"Slow_cooking_bird\",\"Slow_cooking_fish\",\"Slow_cooking_vegetables\"");
	strcat(bufd,",\"Bread\",\"Pizza\",\"Charlotte\",\"Baking_meat_in_pot\",\"Baking_bird_in_pot\"");
	strcat(bufd,",\"Baking_fish_in_pot\",\"Baking_vegetables_in_pot\",\"Roast\",\"Cake\",\"Baking_meat\"");
	strcat(bufd,",\"Baking_bird\",\"Baking_fish\",\"Baking_vegetables\",\"Boiled_pork\",\"Warming\"]}");
	}
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/select/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/2x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Cooker.auto.warming\",\"icon\":\"mdi:pot-steam\",\"uniq_id\":\"autowarming_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Cooker\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/warm\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/warm\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\",\"options\":");
	strcat(bufd,"[\"OFF\",\"ON\"]}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/number/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Cooker.temp\",\"icon\":\"mdi:thermometer\",\"uniq_id\":\"temp_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Cooker\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/temp\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/temp\",\"min\":\"0\",\"max\":\"230\",\"unit_of_meas\":\"\xc2\xb0\x43\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/number/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/2x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Cooker.s.hour\",\"icon\":\"mdi:timer\",\"uniq_id\":\"shour_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Cooker\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/set_hour\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/set_hour\",\"min\":\"0\",\"max\":\"23\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/number/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/3x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Cooker.s.min\",\"icon\":\"mdi:timer\",\"uniq_id\":\"smin_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Cooker\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/set_min\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/set_min\",\"min\":\"0\",\"max\":\"59\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/number/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/4x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Cooker.d.hour\",\"icon\":\"mdi:timer\",\"uniq_id\":\"dhour_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Cooker\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/delay_hour\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/delay_hour\",\"min\":\"0\",\"max\":\"23\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/number/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/5x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Cooker.d.min\",\"icon\":\"mdi:timer\",\"uniq_id\":\"dmin_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Cooker\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/delay_min\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/delay_min\",\"min\":\"0\",\"max\":\"59\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Cooker.state\",\"icon\":\"mdi:pot-steam\",\"uniq_id\":\"stat_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Cooker\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/state\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/2x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Cooker.hour\",\"icon\":\"mdi:timer\",\"uniq_id\":\"hour_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Cooker\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/hour\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/3x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Cooker.min\",\"icon\":\"mdi:timer\",\"uniq_id\":\"min_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Cooker\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/min\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/4x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Cooker.rssi\",\"icon\":\"mdi:bluetooth\",\"uniq_id\":\"rssi_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Cooker\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"device_class\":\"signal_strength\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/rssi\",\"unit_of_meas\":\"dBm\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	}

	} else if ( ptr->DEV_TYP == 63) {
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/calibration");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	if (FDHass) {
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Weather.calibrate\",\"icon\":\"mdi:mdi-check-circle-outline\",\"uniq_id\":\"calibrate_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Weather_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Weather\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/calibration\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/calibration\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Weather.rssi\",\"icon\":\"mdi:bluetooth\",\"uniq_id\":\"rssi_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Weather_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Weather\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"device_class\":\"signal_strength\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/rssi\",\"unit_of_meas\":\"dBm\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/2x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Weather.temperature\",\"icon\":\"mdi:thermometer\",\"uniq_id\":\"temp_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Weather_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Weather\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"device_class\":\"temperature\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/temperature\",\"unit_of_meas\":\"\xc2\xb0\x43\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/3x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Weather.humidity\",\"icon\":\"mdi:water-percent\",\"uniq_id\":\"humid_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Weather_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Weather\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"device_class\":\"humidity\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/humidity\",\"unit_of_meas\":\"\x25\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/4x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Weather.pressure\",\"icon\":\"mdi:gauge\",\"uniq_id\":\"press_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Weather_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Weather\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"device_class\":\"pressure\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/pressure\",\"unit_of_meas\":\"hPa\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/5x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Weather.quality\",\"icon\":\"mdi:leaf\",\"uniq_id\":\"qual_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Weather_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Weather\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
//	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"device_class\":\"aqi\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/quality\",\"unit_of_meas\":\"mm3/m3\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/6x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Weather.pressurem\",\"icon\":\"mdi:gauge\",\"uniq_id\":\"pressm_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Weather_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
       	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Weather\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
//	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"device_class\":\"pressure\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/pressurem\",\"unit_of_meas\":\"mmHg\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	}

	}
	}
}


//******************** ble **********************
/* Declare static functions */
static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
static void esp_gattc_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);
static void gattc_profile_cm_event_handler(uint8_t blenum,esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);

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
static esp_ble_scan_params_t ble_pscan_params = {
//    .scan_type              = BLE_SCAN_TYPE_ACTIVE,
    .scan_type              = BLE_SCAN_TYPE_PASSIVE,
    .own_addr_type          = BLE_ADDR_TYPE_PUBLIC,
    .scan_filter_policy     = BLE_SCAN_FILTER_ALLOW_ALL,
    .scan_interval          = 0x50,
    .scan_window            = 0x30,
    .scan_duplicate         = BLE_SCAN_DUPLICATE_DISABLE
};

struct gattc_profile_inst {
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
        .gattc_if = ESP_GATT_IF_NONE,       // Not get the gatt_if, so initial is ESP_GATT_IF_NONE
    },

//connection profile B handle
    [PROFILE_B_APP_ID] = {
        .gattc_if = ESP_GATT_IF_NONE,       // Not get the gatt_if, so initial is ESP_GATT_IF_NONE
    },

//connection profile C handle
    [PROFILE_C_APP_ID] = {
        .gattc_if = ESP_GATT_IF_NONE,       // Not get the gatt_if, so initial is ESP_GATT_IF_NONE
    },

};

static void start_scan(void)
{
	esp_err_t scan_ret = 0;

//step 1: set scan only if not update, if not already starting scan or if no connections is opening   
	if (!f_update && !StartingScan && (!BleDevStA.btconnect || BleDevStA.btopen) && (!BleDevStB.btconnect || BleDevStB.btopen) && (!BleDevStC.btconnect || BleDevStC.btopen)) {
        if ((ble_mon > 1) || (BleDevStA.REQ_NAME[0] && !BleDevStA.btconnect) || (BleDevStB.REQ_NAME[0] && !BleDevStB.btconnect) ||(BleDevStC.REQ_NAME[0] && !BleDevStC.btconnect)) {
        if (!Isscanning) {
	scan_ret = esp_ble_gap_set_scan_params(&ble_scan_params);
	IsPassiveScan = false;
	} else if (IsPassiveScan) esp_ble_gap_stop_scanning();
	} else if (ble_mon == 1) {
        if (!Isscanning) {
	scan_ret = esp_ble_gap_set_scan_params(&ble_pscan_params);
	IsPassiveScan = true;
	} else if (!IsPassiveScan) esp_ble_gap_stop_scanning();
        }
	if (scan_ret){
	ESP_LOGE(AP_TAG, "Set scan params error, error code = 0x%X", scan_ret);
	        }
	}
}

static char *esp_key_type_to_str(esp_ble_key_type_t key_type)
{
   char *key_str = NULL;
   switch(key_type) {
    case ESP_LE_KEY_NONE:
        key_str = "ESP_LE_KEY_NONE";
        break;
    case ESP_LE_KEY_PENC:
        key_str = "ESP_LE_KEY_PENC";
        break;
    case ESP_LE_KEY_PID:
        key_str = "ESP_LE_KEY_PID";
        break;
    case ESP_LE_KEY_PCSRK:
        key_str = "ESP_LE_KEY_PCSRK";
        break;
    case ESP_LE_KEY_PLK:
        key_str = "ESP_LE_KEY_PLK";
        break;
    case ESP_LE_KEY_LLK:
        key_str = "ESP_LE_KEY_LLK";
        break;
    case ESP_LE_KEY_LENC:
        key_str = "ESP_LE_KEY_LENC";
        break;
    case ESP_LE_KEY_LID:
        key_str = "ESP_LE_KEY_LID";
        break;
    case ESP_LE_KEY_LCSRK:
        key_str = "ESP_LE_KEY_LCSRK";
        break;
    default:
        key_str = "INVALID BLE KEY TYPE";
        break;

   }

   return key_str;
}

static char *esp_auth_req_to_str(esp_ble_auth_req_t auth_req)
{
   char *auth_str = NULL;
   switch(auth_req) {
    case ESP_LE_AUTH_NO_BOND:
        auth_str = "ESP_LE_AUTH_NO_BOND";
        break;
    case ESP_LE_AUTH_BOND:
        auth_str = "ESP_LE_AUTH_BOND";
        break;
    case ESP_LE_AUTH_REQ_MITM:
        auth_str = "ESP_LE_AUTH_REQ_MITM";
        break;
    case ESP_LE_AUTH_REQ_BOND_MITM:
        auth_str = "ESP_LE_AUTH_REQ_BOND_MITM";
        break;
    case ESP_LE_AUTH_REQ_SC_ONLY:
        auth_str = "ESP_LE_AUTH_REQ_SC_ONLY";
        break;
    case ESP_LE_AUTH_REQ_SC_BOND:
        auth_str = "ESP_LE_AUTH_REQ_SC_BOND";
        break;
    case ESP_LE_AUTH_REQ_SC_MITM:
        auth_str = "ESP_LE_AUTH_REQ_SC_MITM";
        break;
    case ESP_LE_AUTH_REQ_SC_MITM_BOND:
        auth_str = "ESP_LE_AUTH_REQ_SC_MITM_BOND";
        break;
    default:
        auth_str = "INVALID BLE AUTH REQ";
        break;
   }

   return auth_str;
}



//*** Gattc event handler ************************
static void gattc_profile_cm_event_handler(uint8_t blenum, esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    esp_ble_gattc_cb_param_t *p_data = (esp_ble_gattc_cb_param_t *)param;
	uint8_t  conerr = 0;
	uint8_t  buff1[16];
	uint8_t  bufftab[256];
	uint8_t blenum1 = blenum + 1;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}

    switch (event) {
    case ESP_GATTC_REG_EVT:
	if (!blenum) {
	esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV,ESP_PWR_LVL_P9); // for more power???
	esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT,ESP_PWR_LVL_P9); // for more power???
        esp_ble_gap_config_local_privacy(true);
	}
	break;
    case ESP_GATTC_CONNECT_EVT:
        if (ptr->btconnect && !ptr->btopen) {
	if ((ptr->DEV_TYP != 63) || (ptr->r4sAuthCount > 2)) {
//	if (ptr->DEV_TYP != 63) {
        ESP_LOGI(AP_TAG, "CONNECT_EVT %d, set Bond and Encryption", blenum1);
	esp_ble_auth_req_t auth_req = ESP_LE_AUTH_REQ_SC_MITM_BOND;     //bonding with peer device after authentication
	esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;           //set the IO capability to No output No input
	esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t));
	esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
	esp_ble_set_encryption(param->connect.remote_bda, ESP_BLE_SEC_ENCRYPT_MITM);
	} else {
        ESP_LOGI(AP_TAG, "CONNECT_EVT %d, set no Bond and Encryption", blenum1);
	esp_ble_auth_req_t auth_req = ESP_LE_AUTH_REQ_SC_ONLY;     //default authentication
	esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;           //set the IO capability to No output No input
	esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t));
	esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
	esp_ble_set_encryption(param->connect.remote_bda, 1);
	}

	}
	break;
    case ESP_GATTC_OPEN_EVT:
	ptr->xbtauth = 0;
        char bd_addr[20];
        if (param->open.status != ESP_GATT_OK) {
	ESP_LOGE(AP_TAG, "Open %d failed, status %d", blenum1, p_data->open.status);
	ptr->btopen = false;
        ptr->btconnect = false;
	start_scan();
	break;
        }
        memcpy(gl_profile_tab[blenum].remote_bda, p_data->open.remote_bda, 6);
        gl_profile_tab[blenum].conn_id = p_data->open.conn_id;
        ESP_LOGI(AP_TAG, "Open %d success, conn_id %d, if %d, status %d, mtu %d", blenum1, p_data->open.conn_id, gattc_if, p_data->open.status, p_data->open.mtu);
	bin2hex(p_data->open.remote_bda, bd_addr,6,0x3a);
        ESP_LOGI(AP_TAG, "Remote BD_ADDR: %s", bd_addr);
        esp_err_t mtu_ret = esp_ble_gattc_send_mtu_req (gattc_if, p_data->open.conn_id);
	ptr->btopen = true;
        if (mtu_ret){
            ESP_LOGE(AP_TAG, "Config MTU %d error, error code = 0x%X", blenum1, mtu_ret);
        }
	if (ptr->DEV_TYP == 64) ptr->MiKettleID = 275;
	if (ptr->DEV_TYP == 65) ptr->MiKettleID = 131;
	if (ptr->DEV_TYP == 66) ptr->MiKettleID = 1116;
        break;

    case ESP_GATTC_DIS_SRVC_CMPL_EVT:
        if (param->dis_srvc_cmpl.status != ESP_GATT_OK){
	conerr = 1;
	ESP_LOGE(AP_TAG, "Discover service %d failed, status %d", blenum1, param->dis_srvc_cmpl.status);
	break;
        }
        ESP_LOGI(AP_TAG, "Discover service %d complete conn_id %d", blenum1, param->dis_srvc_cmpl.conn_id);
        if (ptr->DEV_TYP < 64) esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id, &remote_filter_service_uuid);
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
	ESP_LOGE(AP_TAG,"Config mtu %d failed, error status = 0x%X", blenum1, param->cfg_mtu.status);
        }
	ESP_LOGI(AP_TAG, "ESP_GATTC_CFG_MTU_EVT, Status %d, MTU %d, conn_id %d", param->cfg_mtu.status, param->cfg_mtu.mtu, param->cfg_mtu.conn_id);
	break;
    case ESP_GATTC_SEARCH_RES_EVT: {
        ESP_LOGI(AP_TAG, "SEARCH RES %d: conn_id = 0x%X is primary service %d", blenum1, p_data->search_res.conn_id, p_data->search_res.is_primary);
        ESP_LOGI(AP_TAG, "start handle 0x%X end handle 0x%X current handle value 0x%X", p_data->search_res.start_handle, p_data->search_res.end_handle, p_data->search_res.srvc_id.inst_id);

//esp_log_buffer_hex(AP_TAG,p_data->search_res.srvc_id.uuid.uuid.uuid128,16);
//esp_log_buffer_hex(AP_TAG,xremote_filter_service11_uuid.uuid.uuid128,16);

        if ((ptr->DEV_TYP < 64) && (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_128) && (bt_compare_UUID128(p_data->search_res.srvc_id.uuid.uuid.uuid128, remote_filter_service_uuid.uuid.uuid128))) {

            ESP_LOGI(AP_TAG, "Redmond Service %d found", blenum1);
            ptr->get_server = true;
            gl_profile_tab[blenum].service_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[blenum].service_end_handle = p_data->search_res.end_handle;
        } else if ((ptr->DEV_TYP > 63) && (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_128) && (bt_compare_UUID128(p_data->search_res.srvc_id.uuid.uuid.uuid128, xremote_filter_service_uuid.uuid.uuid128))) {

            ESP_LOGI(AP_TAG, "Xiaomi Service1 %d found", blenum1);
            gl_profile_tab[blenum].service_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[blenum].service_end_handle = p_data->search_res.end_handle;
        } else if ((ptr->DEV_TYP > 63) && (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_16) && (p_data->search_res.srvc_id.uuid.uuid.uuid16 == XREMOTE_SERVICE116_UUID)) {

            ESP_LOGI(AP_TAG, "Xiaomi Service2 %d found", blenum1);
            gl_profile_tab[blenum].service1_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[blenum].service1_end_handle = p_data->search_res.end_handle;
        } else if ((ptr->DEV_TYP > 63) && (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_16) && (p_data->search_res.srvc_id.uuid.uuid.uuid16 == XREMOTE_SERVICE216_UUID)) {

            ESP_LOGI(AP_TAG, "Xiaomi Service3 %d found", blenum1);
//            ptr->get_server = true;
            gl_profile_tab[blenum].service2_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[blenum].service2_end_handle = p_data->search_res.end_handle;
        } else if ((ptr->DEV_TYP > 63) && (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_16) && (p_data->search_res.srvc_id.uuid.uuid.uuid16 == XREMOTE_SERVICE316_UUID)) {

            ESP_LOGI(AP_TAG, "Xiaomi Service4 %d found", blenum1);
            ptr->get_server = true;
            gl_profile_tab[blenum].service3_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[blenum].service3_end_handle = p_data->search_res.end_handle;
	}
        break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT:
        if (p_data->search_cmpl.status != ESP_GATT_OK){
	conerr = 1;
            ESP_LOGE(AP_TAG, "Search service %d failed, error status = 0x%X", blenum1, p_data->search_cmpl.status);
            break;
        }
        if(p_data->search_cmpl.searched_service_source == ESP_GATT_SERVICE_FROM_REMOTE_DEVICE) {
//            ESP_LOGI(AP_TAG, "Get service information from remote device");
        } else if (p_data->search_cmpl.searched_service_source == ESP_GATT_SERVICE_FROM_NVS_FLASH) {
//            ESP_LOGI(AP_TAG, "Get service information from flash");
        } else {
//            ESP_LOGI(AP_TAG, "Unknown service source");
        }
        if (ptr->get_server){
            uint16_t count = 0;
            uint16_t count1 = 0;
            uint16_t count2 = 0;
            esp_gatt_status_t status = esp_ble_gattc_get_attr_count( gattc_if,
                                                                     p_data->search_cmpl.conn_id,
                                                                     ESP_GATT_DB_CHARACTERISTIC,
                                                                     gl_profile_tab[blenum].service_start_handle,
                                                                     gl_profile_tab[blenum].service_end_handle,
                                                                     INVALID_HANDLE,
                                                                     &count);
            if (status != ESP_GATT_OK){
		conerr = 1;
                ESP_LOGE(AP_TAG, "esp_ble_gattc_get_attr_count error");
            }

	if (ptr->DEV_TYP > 63) {
            status = esp_ble_gattc_get_attr_count( gattc_if,
                                                                     p_data->search_cmpl.conn_id,
                                                                     ESP_GATT_DB_CHARACTERISTIC,
                                                                     gl_profile_tab[blenum].service1_start_handle,
                                                                     gl_profile_tab[blenum].service1_end_handle,
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
                                                                     gl_profile_tab[blenum].service2_start_handle,
                                                                     gl_profile_tab[blenum].service2_end_handle,
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
                                                                     gl_profile_tab[blenum].service3_start_handle,
                                                                     gl_profile_tab[blenum].service3_end_handle,
                                                                     INVALID_HANDLE,
                                                                     &count1);
            if (status != ESP_GATT_OK){
		conerr = 1;
                ESP_LOGE(AP_TAG, "esp_ble_gattc_get_attr3_count error");
            }
	count = count + count1;
	}

            if (count > 0) {
		char_elem_result = (esp_gattc_char_elem_t *)malloc(sizeof(esp_gattc_char_elem_t) * count);
                if (!char_elem_result){
		conerr = 1;
                    ESP_LOGE(AP_TAG, "Gattc no mem");
                } else {
		if ((ptr->DEV_TYP < 64) && (count > 1)) {
		count1 = count;
		count2 = count;
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[blenum].service_start_handle,
                                                             gl_profile_tab[blenum].service_end_handle,
                                                             remote_filter_rxchar_uuid,
                                                             (char_elem_result),
                                                             &count1);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_rxchar_by_uuid %d error", blenum1);
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
                                                             p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[blenum].service_start_handle,
                                                             gl_profile_tab[blenum].service_end_handle,
                                                             remote_filter_txchar_uuid,
                                                             (char_elem_result+count1),
                                                             &count2);

                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_txchar_by_uuid %d error", blenum1);
                    }
                    /*  Every service have only one char in our 'ESP_GATTS_DEMO' demo, so we used first 'char_elem_result' */
		int i = 0; 
                while (count1 && (i < count1)) {
		if (char_elem_result[i].properties & ESP_GATT_CHAR_PROP_BIT_NOTIFY) {	
                        gl_profile_tab[blenum].rxchar_handle = char_elem_result[i].char_handle;
                        esp_ble_gattc_register_for_notify (gattc_if, gl_profile_tab[blenum].remote_bda, char_elem_result[i].char_handle);
			ESP_LOGI(AP_TAG, "Register_for_notify %d", blenum1);
			i = count1;
		}
		i++;
		}
		i = count1;
		count = count1 + count2;
                while (count2 && (i < count)) {
		if (char_elem_result[i].properties & ESP_GATT_CHAR_PROP_BIT_WRITE) {	
                        gl_profile_tab[blenum].txchar_handle = char_elem_result[i].char_handle;
			i = count;
		}
		i++;
		}



	ESP_LOGI(AP_TAG, "Rx char count = %d, handle = 0x%X", count1, gl_profile_tab[blenum].rxchar_handle);
	ESP_LOGI(AP_TAG, "Tx char count = %d, handle = 0x%X", count2, gl_profile_tab[blenum].txchar_handle);
		} else if ((ptr->DEV_TYP > 63) && (count > 8)) {
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[blenum].service_start_handle,
                                                             gl_profile_tab[blenum].service_end_handle,
                                                             xremote_filter_status_uuid,
                                                             (char_elem_result),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_status_by_uuid %d error", blenum1);
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[blenum].service1_start_handle,
                                                             gl_profile_tab[blenum].service1_end_handle,
                                                             xremote_filter_authinit_uuid,
                                                             (char_elem_result+1),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_authinit_by_uuid %d error", blenum1);
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[blenum].service1_start_handle,
                                                             gl_profile_tab[blenum].service1_end_handle,
                                                             xremote_filter_auth_uuid,
                                                             (char_elem_result+2),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_auth_by_uuid %d error", blenum1);
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[blenum].service1_start_handle,
                                                             gl_profile_tab[blenum].service1_end_handle,
                                                             xremote_filter_ver_uuid,
                                                             (char_elem_result+3),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_ver_by_uuid %d error", blenum1);
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[blenum].service_start_handle,
                                                             gl_profile_tab[blenum].service_end_handle,
                                                             xremote_filter_setup_uuid,
                                                             (char_elem_result+4),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_setup_by_uuid %d error", blenum1);
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[blenum].service_start_handle,
                                                             gl_profile_tab[blenum].service_end_handle,
                                                             xremote_filter_time_uuid,
                                                             (char_elem_result+5),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_time_by_uuid %d error", blenum1);
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[blenum].service_start_handle,
                                                             gl_profile_tab[blenum].service_end_handle,
                                                             xremote_filter_boil_uuid,
                                                             (char_elem_result+6),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_boil_by_uuid %d error", blenum1);
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[blenum].service2_start_handle,
                                                             gl_profile_tab[blenum].service2_end_handle,
                                                             xremote_filter_mcuver_uuid,
                                                             (char_elem_result+7),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_mcuver_by_uuid %d error", blenum1);
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[blenum].service3_start_handle,
                                                             gl_profile_tab[blenum].service3_end_handle,
                                                             xremote_filter_update_uuid,
                                                             (char_elem_result+8),
                                                             &count);
                    if (status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_update_by_uuid %d error", blenum1);
                    }

                    /*  Every service have only one char in our 'ESP_GATTS_DEMO' demo, so we used first 'char_elem_result' */
                    if ((count > 0) && (char_elem_result[0].properties & ESP_GATT_CHAR_PROP_BIT_NOTIFY)){
                        gl_profile_tab[blenum].rxchar_handle = char_elem_result[0].char_handle;
                        esp_ble_gattc_register_for_notify (gattc_if, gl_profile_tab[blenum].remote_bda, char_elem_result[0].char_handle);
                        gl_profile_tab[blenum].txchar_handle = char_elem_result[1].char_handle;
                        gl_profile_tab[blenum].auth_handle = char_elem_result[2].char_handle;
                        gl_profile_tab[blenum].ver_handle = char_elem_result[3].char_handle;
                        gl_profile_tab[blenum].setup_handle = char_elem_result[4].char_handle;
                        gl_profile_tab[blenum].time_handle = char_elem_result[5].char_handle;
                        gl_profile_tab[blenum].boil_handle = char_elem_result[6].char_handle;
                        gl_profile_tab[blenum].mcuver_handle = char_elem_result[7].char_handle;
                        gl_profile_tab[blenum].update_handle = char_elem_result[8].char_handle;
	ESP_LOGI(AP_TAG, "Update char handle = 0x%X", gl_profile_tab[blenum].update_handle);
			ESP_LOGI(AP_TAG, "Register_for_notify %d", blenum1);
                    }


		} else conerr = 1;

                }
                /* free char_elem_result */
                free(char_elem_result);
            } else {
		conerr = 1;
                ESP_LOGE(AP_TAG, "No char %d found", blenum1);
            }
        }
         break;
    case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
        if (p_data->reg_for_notify.status != ESP_GATT_OK){
	conerr = 1;
            ESP_LOGE(AP_TAG, "REG FOR NOTIFY %d failed: error status = %d", blenum1, p_data->reg_for_notify.status);
        } else {
            uint16_t count = 0;
            uint16_t notify_en = 1; 
	if ((ptr->DEV_TYP < 64) || (ptr->xbtauth != 1)) {
            esp_gatt_status_t ret_status = esp_ble_gattc_get_attr_count( gattc_if,
                                                                         gl_profile_tab[blenum].conn_id,
                                                                         ESP_GATT_DB_DESCRIPTOR,
                                                                         gl_profile_tab[blenum].service_start_handle,
                                                                         gl_profile_tab[blenum].service_end_handle,
                                                                         gl_profile_tab[blenum].rxchar_handle,
                                                                         &count);
            if (ret_status != ESP_GATT_OK){
		conerr = 1;
                ESP_LOGE(AP_TAG, "Get_attr_count %d error", blenum1);
            }
            if (count > 0){
                descr_elem_result = malloc(sizeof(esp_gattc_descr_elem_t) * count);
                if (!descr_elem_result){
                    ESP_LOGE(AP_TAG, "malloc error, gattc no mem");
                }else{
                    ret_status = esp_ble_gattc_get_descr_by_char_handle( gattc_if,
                                                                         gl_profile_tab[blenum].conn_id,
                                                                         p_data->reg_for_notify.handle,
                                                                         notify_descr_uuid,
                                                                         descr_elem_result,
                                                                         &count);
                    if (ret_status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_descr_by_char_handle %d error", blenum1);
                    }
                    /* Every char has only one descriptor in our 'ESP_GATTS_DEMO' demo, so we used first 'descr_elem_result' */
                    if (count > 0 && descr_elem_result[0].uuid.len == ESP_UUID_LEN_16 && descr_elem_result[0].uuid.uuid.uuid16 == ESP_GATT_UUID_CHAR_CLIENT_CONFIG){
                        ret_status = esp_ble_gattc_write_char_descr( gattc_if,
                                                                     gl_profile_tab[blenum].conn_id,
                                                                     descr_elem_result[0].handle,
                                                                     sizeof(notify_en),
                                                                     (uint8_t *)&notify_en,
                                                                     ESP_GATT_WRITE_TYPE_RSP,
                                                                     ESP_GATT_AUTH_REQ_NONE);
                    }
                    if (ret_status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Write_char_descr %d error", blenum1);
                    }
                    /* free descr_elem_result */
                    free(descr_elem_result);
                }
            } else {
		conerr = 1;
                ESP_LOGE(AP_TAG, "Decsr %d not found", blenum1);
            }
	} else if ((ptr->DEV_TYP > 63) && ptr->xbtauth) {
            count = 0;
            notify_en = 1; 
            esp_gatt_status_t ret_status = esp_ble_gattc_get_attr_count( gattc_if,
                                                                         gl_profile_tab[blenum].conn_id,
                                                                         ESP_GATT_DB_DESCRIPTOR,
                                                                         gl_profile_tab[blenum].service1_start_handle,
                                                                         gl_profile_tab[blenum].service1_end_handle,
                                                                         gl_profile_tab[blenum].auth_handle,
                                                                         &count);
            if (ret_status != ESP_GATT_OK){
		conerr = 1;
                ESP_LOGE(AP_TAG, "Get_attr_count1 %d error", blenum1);
            }
            if (count > 0){
                descr_elem_result = malloc(sizeof(esp_gattc_descr_elem_t) * count);
                if (!descr_elem_result){
		conerr = 1;
                    ESP_LOGE(AP_TAG, "malloc error, gattc no mem");
                }else{
                    ret_status = esp_ble_gattc_get_descr_by_char_handle( gattc_if,
                                                                         gl_profile_tab[blenum].conn_id,
                                                                         p_data->reg_for_notify.handle,
                                                                         notify_descr_uuid,
                                                                         descr_elem_result,
                                                                         &count);
                    if (ret_status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Get_descr_by_char_handle %d error", blenum1);
                    }
                    /* Every char has only one descriptor in our 'ESP_GATTS_DEMO' demo, so we used first 'descr_elem_result' */
                    if (count > 0 && descr_elem_result[0].uuid.len == ESP_UUID_LEN_16 && descr_elem_result[0].uuid.uuid.uuid16 == ESP_GATT_UUID_CHAR_CLIENT_CONFIG){
                        ret_status = esp_ble_gattc_write_char_descr( gattc_if,
                                                                     gl_profile_tab[blenum].conn_id,
                                                                     descr_elem_result[0].handle,
                                                                     sizeof(notify_en),
                                                                     (uint8_t *)&notify_en,
                                                                     ESP_GATT_WRITE_TYPE_RSP,
                                                                     ESP_GATT_AUTH_REQ_NONE);
                    }
                    if (ret_status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Write_char_descr1 %d error", blenum1);
                    }
                    /* free descr_elem_result */
                    free(descr_elem_result);
                }
            } else {
		conerr = 1;
                ESP_LOGE(AP_TAG, "Decsr1 %d not found", blenum1);
            }
	}	
        }
        break;
    }

    case ESP_GATTC_READ_CHAR_EVT:
ESP_LOGI(AP_TAG, "ESP_GATTC_READ_CHAR_EVT, receive read value:");
esp_log_buffer_hex(AP_TAG, p_data->read.value, p_data->read.value_len);
ESP_LOGI(AP_TAG, "Read char handle = 0x%X", p_data->read.handle);

	int length = p_data->read.value_len;
	if (length > BLE_INPUT_BUFFSIZE)
	length = BLE_INPUT_BUFFSIZE;
	if (length > 0) {
        memcpy(ptr->readData, p_data->read.value, length);
	}
	ptr->readDataLen = length;
	ptr->readDataHandle = p_data->read.handle;

/*
if (p_data->read.handle == 0x2a) {
        esp_gatt_status_t ret_status = esp_ble_gattc_read_char( gattc_if,
                                  gl_profile_tab[blenum].conn_id,
                                  gl_profile_tab[blenum].mcuver_handle,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Read_mcuver_req %d error", blenum1);
			conerr = 1;
                    }  else ESP_LOGI(AP_TAG, "Read_read_mcuver_req %d Ok", blenum1);
	}
*/
	break;
    case ESP_GATTC_NOTIFY_EVT:
        if ((p_data->notify.is_notify) && (p_data->notify.handle == gl_profile_tab[blenum].rxchar_handle)) {
	if (ptr->btauthoriz) {
	int length = p_data->notify.value_len;
	if (length > BLE_INPUT_BUFFSIZE)
	length = BLE_INPUT_BUFFSIZE;
	if (length > 0) {
        memcpy(ptr->notifyData, p_data->notify.value, length);
	}
	ptr->notifyDataLen = length;
	} else {
	ESP_LOGI(AP_TAG, "Read_auth %d:", blenum1);
	esp_log_buffer_hex(AP_TAG, p_data->notify.value, p_data->notify.value_len);

	if ((p_data->notify.value_len == 5) && (p_data->notify.value[0] = 0x55) && (p_data->notify.value[2] = 0xff) && p_data->notify.value[3] && (p_data->notify.value[4] = 0xaa)) {
	ESP_LOGI(AP_TAG, "Authorize %d Redmond ok", blenum1);
	ptr->t_last_us = ~ptr->t_last_us;
	ptr->t_ppcon_us = esp_timer_get_time();
	ptr->btauthoriz = true;
	ptr->r4sConnErr = 0;
	ptr->r4sAuthCount = 0;
	ptr->r4scounter = p_data->notify.value[1] + 1;	
	ptr->f_Sync = 16;
	ptr->NumConn++;
	if (!ptr->NumConn) ptr->NumConn--;
	bin2hex(gl_profile_tab[blenum].remote_bda, ptr->tBLEAddr,6,0);
	strcpy(ptr->DEV_NAME,ptr->RQC_NAME);
        MqttPubSub(blenum, mqttConnected);	
	}
	}
	} else if (!ptr->btauthoriz && (p_data->notify.is_notify) && (p_data->notify.handle == gl_profile_tab[blenum].auth_handle)) {
	ESP_LOGI(AP_TAG, "Read_auth %d:", blenum1);
	esp_log_buffer_hex(AP_TAG, p_data->notify.value, p_data->notify.value_len);
	if ((p_data->notify.value_len == 12) && (ptr->xbtauth == 2)) {
	uint8_t buff2[16];
        uint8_t xiv_char_data[12] = { 0x55,0x00,0xff,0xb6,0x2c,0x27,0xb3,0xb8,0xac,0x5a,0xef,0xaa};  //auth string
	xiv_char_data[3] = xiv_char_data[3] + blenum;  //for each position number different auth id
	xiv_char_data[5] = xiv_char_data[5] + R4SNUM;  //for each gate number different auth id

	mixA(gl_profile_tab[blenum].remote_bda, buff1, ptr->MiKettleID);
	cipherInit(buff1, bufftab, 8);
	cipherCrypt(p_data->notify.value, buff2, bufftab, 12);
	mixB(gl_profile_tab[blenum].remote_bda, buff1, ptr->MiKettleID);
	cipherInit(buff1, bufftab, 8);
	cipherCrypt(buff2, buff1, bufftab, 12);
	if (!memcmp(xiv_char_data, buff1, 12)) {
	buff2[0] = 0x92;
	buff2[1] = 0xab;
	buff2[2] = 0x54;
	buff2[3] = 0xfa;
	cipherInit(xiv_char_data, bufftab, 12);
	cipherCrypt(buff2, buff1, bufftab, 4);
	ESP_LOGI(AP_TAG, "Write_auth_xi_ack %d:", blenum1);
	esp_log_buffer_hex(AP_TAG, buff1, 4);
        esp_gatt_status_t ret_status = esp_ble_gattc_write_char( gattc_if,
                                  gl_profile_tab[blenum].conn_id,
                                  gl_profile_tab[blenum].auth_handle,
                                  4,
                                  buff1,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
			conerr = 1;
                        ESP_LOGE(AP_TAG, "Write_auth_xi_ack %d error", blenum1);
                    }
	buff2[0] = 0x03;
	buff2[1] = 0x01;
	ESP_LOGI(AP_TAG, "Write_setup_xi %d:", blenum1);
	esp_log_buffer_hex(AP_TAG, buff2, 2);
        ret_status = esp_ble_gattc_write_char( gattc_if,
                                  gl_profile_tab[blenum].conn_id,
                                  gl_profile_tab[blenum].setup_handle,
                                  2,
                                  buff2,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
                        ESP_LOGE(AP_TAG, "Write_setup_xi %d error", blenum1);
                    }
	buff2[0] = 0x00;
	ESP_LOGI(AP_TAG, "Write_boil_xi %d:", blenum1);
	esp_log_buffer_hex(AP_TAG, buff2, 1);
        ret_status = esp_ble_gattc_write_char( gattc_if,
                                  gl_profile_tab[blenum].conn_id,
                                  gl_profile_tab[blenum].boil_handle,
                                  1,
                                  buff2,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
                        ESP_LOGE(AP_TAG, "Write_boil_xi %d error", blenum1);
                    }
	buff2[0] = 0x17;
	ESP_LOGI(AP_TAG, "Write_time_xi %d:", blenum1);
	esp_log_buffer_hex(AP_TAG, buff2, 1);
        ret_status = esp_ble_gattc_write_char( gattc_if,
                                  gl_profile_tab[blenum].conn_id,
                                  gl_profile_tab[blenum].time_handle,
                                  1,
                                  buff2,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
                        ESP_LOGE(AP_TAG, "Write_time_xi %d error", blenum1);
                    }

///*
        ret_status = esp_ble_gattc_read_char( gattc_if,
                                  gl_profile_tab[blenum].conn_id,
                                  gl_profile_tab[blenum].update_handle,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
//			conerr = 1;
                        ESP_LOGE(AP_TAG, "Read_ver_req %d error", blenum1);
                    }  else ESP_LOGI(AP_TAG, "Read_read_ver_req %d Ok", blenum1);
//*/

	ESP_LOGI(AP_TAG, "Authorize %d Xiaomi ok", blenum1);
	ptr->t_last_us = ~ptr->t_last_us;
	ptr->t_ppcon_us = esp_timer_get_time();
	ptr->btauthoriz = true;
	ptr->r4sConnErr = 0;
	ptr->NumConn++;
	if (!ptr->NumConn) ptr->NumConn--;
	bin2hex(gl_profile_tab[blenum].remote_bda, ptr->tBLEAddr,6,0);
	strcpy(ptr->DEV_NAME,ptr->RQC_NAME);
        MqttPubSub(blenum, mqttConnected);	
	esp_ble_gattc_register_for_notify (gattc_if, gl_profile_tab[blenum].remote_bda, gl_profile_tab[blenum].rxchar_handle);

	} else {
	conerr = 1;
	ESP_LOGI(AP_TAG, "Invalid %d Xiaomi product Id", blenum1);
	if ((ptr->DEV_TYP == 95) && (ptr->MiKettleID < 10000)) ptr->MiKettleID++;
	}
	}
        } if (p_data->notify.is_notify) {
//            ESP_LOGI(AP_TAG, "GATTC_NOTIFY_EVT_0, Handle = 0x%X, Notify value:",p_data->notify.handle);
        }else{
//            ESP_LOGI(AP_TAG, "GATTC_NOTIFY_EVT_0, Indicate value:");
        }
//        esp_log_buffer_hex(AP_TAG, p_data->notify.value, p_data->notify.value_len);

        break;



    case ESP_GATTC_WRITE_DESCR_EVT:
        if (p_data->write.status != ESP_GATT_OK){
	conerr = 1;
            ESP_LOGE(AP_TAG, "Write descr %d failed, error status = 0x%X", blenum1, p_data->write.status);
	conerr = 1;
            break;
        }
	uint8_t  write_char_crypt_data[16];
	int  write_char_data_len = 12;
        uint8_t write_char_data[12] = { 0x55,0x00,0xff,0xb6,0x2c,0x27,0xb3,0xb8,0xac,0x5a,0xef,0xaa};  //auth string
	write_char_data[3] = write_char_data[3] + blenum;  //for each position number different auth id
	write_char_data[5] = write_char_data[5] + R4SNUM;  //for each gate number different auth id
	if ((ptr->DEV_TYP < 64) || !ptr->xbtauth) {
	if (ptr->DEV_TYP > 63) {
        write_char_data[0] = 0x90;
        write_char_data[1] = 0xca;
        write_char_data[2] = 0x85;
        write_char_data[3] = 0xde;
	write_char_data_len = 4;
	}
	ESP_LOGI(AP_TAG, "Write_auth %d:", blenum1);
	esp_log_buffer_hex(AP_TAG, write_char_data, write_char_data_len);
        esp_gatt_status_t ret_status = esp_ble_gattc_write_char( gattc_if,
                                  gl_profile_tab[blenum].conn_id,
                                  gl_profile_tab[blenum].txchar_handle,
                                  write_char_data_len,
                                  write_char_data,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
                        ESP_LOGE(AP_TAG, "Write_auth %d error", blenum1);
			conerr = 1;
                    }
	if (ptr->DEV_TYP > 63) {
	ptr->xbtauth = 1;
	esp_ble_gattc_register_for_notify (gattc_if, gl_profile_tab[blenum].remote_bda, gl_profile_tab[blenum].auth_handle);
	}
	} else if ((ptr->DEV_TYP > 63) && (ptr->xbtauth == 1)) {
	ptr->xbtauth = 2;
	mixA(gl_profile_tab[blenum].remote_bda, buff1, ptr->MiKettleID);
	cipherInit(buff1, bufftab, 8);
	cipherCrypt(write_char_data, write_char_crypt_data, bufftab, 12);
        esp_gatt_status_t ret_status = esp_ble_gattc_write_char( gattc_if,
                                  gl_profile_tab[blenum].conn_id,
                                  gl_profile_tab[blenum].auth_handle,
                                  write_char_data_len,
                                  write_char_crypt_data,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
                        ESP_LOGE(AP_TAG, "Write_auth_mi %d error", blenum1);
			conerr = 1;
                    }  else ESP_LOGI(AP_TAG, "Write_auth_mi %d Ok", blenum1);
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
	ESP_LOGE(AP_TAG, "Write char %d failed, error status = 0x%X", blenum1, p_data->write.status);
	if (p_data->write.status == 5) ptr->r4sAuthCount++;
	if (ptr->r4sAuthCount > 15) {
	esp_ble_remove_bond_device(gl_profile_tab[blenum].remote_bda);
	ptr->r4sAuthCount = 3;
	}
	conerr = 1;
        } else start_scan();
        break;

    case ESP_GATTC_DISCONNECT_EVT:
        if (memcmp(p_data->disconnect.remote_bda, gl_profile_tab[blenum].remote_bda, 6) == 0){
	ptr->btopen = false;
        ptr->btconnect = false;
        ptr->btauthoriz = false;
        ptr->get_server = false;
        ptr->xbtauth = 0;
        ESP_LOGI(AP_TAG, "ESP_GATTC_DISCONNECT_EVT, reason = %d", p_data->disconnect.reason);
	if ((mqttConnected) &&(ptr->tBLEAddr[0])) {
            char ldata[32];
            strcpy(ldata,MQTT_BASE_TOPIC);
            strcat(ldata,"/");
            strcat(ldata,ptr->tBLEAddr);
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
//	if (conerr && ptr->btopen) esp_ble_gattc_close(gl_profile_tab[blenum].gattc_if,gl_profile_tab[blenum].conn_id);
	if ((conerr || f_update) && ptr->btopen) esp_ble_gap_disconnect(gl_profile_tab[blenum].remote_bda);

}

//*** Gattc event handler end ********************

//*** Gap event handler **************************
static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    uint8_t *adv_name = NULL;
    uint8_t adv_name_len = 0;
    uint16_t SHandle = 0;
    switch (event) {

    case ESP_GAP_BLE_READ_RSSI_COMPLETE_EVT:

//        esp_log_buffer_hex(AP_TAG, param->read_rssi_cmpl.remote_addr, 6);

      if (!memcmp(gl_profile_tab[PROFILE_A_APP_ID].remote_bda, param->read_rssi_cmpl.remote_addr, 6)) {
	BleDevStA.iRssi = param->read_rssi_cmpl.rssi;
//ESP_LOGI(AP_TAG,"read 1-RSSI: %d", param->read_rssi_cmpl.rssi); //test #2 with data from read_rssi_cmpl
	if ((BleDevStA.r4sConnErr < 6 ) && (BleDevStA.btauthoriz)) {
	if ((BleDevStA.sendDataLen > 0) && (BleDevStA.sendDataLen < BLE_INPUT_BUFFSIZE)) {
	SHandle = gl_profile_tab[PROFILE_A_APP_ID].txchar_handle;
	if (BleDevStA.DEV_TYP > 63) {
	switch (BleDevStA.sendDataHandle) {
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
                                  BleDevStA.sendDataLen,
                                  BleDevStA.sendData,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
                        ESP_LOGE(AP_TAG, "Write_char_data 1 error");
	BleDevStA.r4sConnErr++;
	}  else {
//ESP_LOGI(AP_TAG, "BleDevStA.r4sConnErr: %d", BleDevStA.r4sConnErr);
ESP_LOGI(AP_TAG, "Send 1, Handle 0x%X, Data:", SHandle);
esp_log_buffer_hex(AP_TAG, BleDevStA.sendData, BleDevStA.sendDataLen);
	}
	BleDevStA.sendDataLen = 0;
	}
	} else if (BleDevStA.btauthoriz) esp_ble_gattc_close(gl_profile_tab[PROFILE_A_APP_ID].gattc_if,gl_profile_tab[PROFILE_A_APP_ID].conn_id);	
	}

      else if (!memcmp(gl_profile_tab[PROFILE_B_APP_ID].remote_bda, param->read_rssi_cmpl.remote_addr, 6)) {
	BleDevStB.iRssi = param->read_rssi_cmpl.rssi;
//ESP_LOGI(AP_TAG,"read 2-RSSI: %d", param->read_rssi_cmpl.rssi); //test #2 with data from read_rssi_cmpl
	if ((BleDevStB.r4sConnErr < 6 ) && (BleDevStB.btauthoriz)) {
	if ((BleDevStB.sendDataLen > 0) && (BleDevStB.sendDataLen < BLE_INPUT_BUFFSIZE)) {
	SHandle = gl_profile_tab[PROFILE_B_APP_ID].txchar_handle;
	if (BleDevStB.DEV_TYP > 63) {
	switch (BleDevStB.sendDataHandle) {
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
                                  BleDevStB.sendDataLen,
                                  BleDevStB.sendData,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
                        ESP_LOGE(AP_TAG, "Write_char_data 2 error");
	BleDevStB.r4sConnErr++;
	}  else {
//ESP_LOGI(AP_TAG, "BleDevStB.r4sConnErr: %d", BleDevStB.r4sConnErr);
ESP_LOGI(AP_TAG, "Send 2, Handle 0x%X, Data:", SHandle);
esp_log_buffer_hex(AP_TAG, BleDevStB.sendData, BleDevStB.sendDataLen);
	}
	BleDevStB.sendDataLen = 0;
	}
	} else if (BleDevStB.btauthoriz) esp_ble_gattc_close(gl_profile_tab[PROFILE_B_APP_ID].gattc_if,gl_profile_tab[PROFILE_B_APP_ID].conn_id);	
	}

      else if (!memcmp(gl_profile_tab[PROFILE_C_APP_ID].remote_bda, param->read_rssi_cmpl.remote_addr, 6)) {
	BleDevStC.iRssi = param->read_rssi_cmpl.rssi;
//ESP_LOGI(AP_TAG,"read 3-RSSI: %d", param->read_rssi_cmpl.rssi); //test #2 with data from read_rssi_cmpl
	if ((BleDevStC.r4sConnErr < 6 ) && (BleDevStC.btauthoriz)) {
	if ((BleDevStC.sendDataLen > 0) && (BleDevStC.sendDataLen < BLE_INPUT_BUFFSIZE)) {
	SHandle = gl_profile_tab[PROFILE_C_APP_ID].txchar_handle;
	if (BleDevStC.DEV_TYP > 63) {
	switch (BleDevStC.sendDataHandle) {
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
                                  BleDevStC.sendDataLen,
                                  BleDevStC.sendData,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
                        ESP_LOGE(AP_TAG, "Write_char_data 3 error");
	BleDevStC.r4sConnErr++;
	}  else {
//ESP_LOGI(AP_TAG, "BleDevStC.r4sConnErr: %d", BleDevStC.r4sConnErr);
ESP_LOGI(AP_TAG, "Send 3, Handle 0x%X, Data:", SHandle);
esp_log_buffer_hex(AP_TAG, BleDevStC.sendData, BleDevStC.sendDataLen);
	}
	BleDevStC.sendDataLen = 0;
	}
	} else if (BleDevStC.btauthoriz) esp_ble_gattc_close(gl_profile_tab[PROFILE_C_APP_ID].gattc_if,gl_profile_tab[PROFILE_C_APP_ID].conn_id);	
	}

	break;

    case ESP_GAP_BLE_SET_LOCAL_PRIVACY_COMPLETE_EVT:
        if (param->local_privacy_cmpl.status != ESP_BT_STATUS_SUCCESS){
            ESP_LOGE(AP_TAG, "Config local privacy failed, error code =0x%X", param->local_privacy_cmpl.status);
            break;
        }
        esp_err_t scan_ret = ESP_GATT_OK;
        if ((ble_mon > 1) || (BleDevStA.REQ_NAME[0] && !BleDevStA.btconnect) || (BleDevStB.REQ_NAME[0] && !BleDevStB.btconnect) ||(BleDevStC.REQ_NAME[0] && !BleDevStC.btconnect)) {
	scan_ret = esp_ble_gap_set_scan_params(&ble_scan_params);
	IsPassiveScan = false;
	} else if (ble_mon == 1) {
	scan_ret = esp_ble_gap_set_scan_params(&ble_pscan_params);
	IsPassiveScan = true;
	}
        if (scan_ret){
	ESP_LOGE(AP_TAG, "Set scan params error, error code = 0x%X", scan_ret);
        }
        break;


    case ESP_GAP_BLE_PASSKEY_REQ_EVT:                           /* passkey request event */
        /* Call the following function to input the passkey which is displayed on the remote device */
        //esp_ble_passkey_reply(gl_profile_tab[PROFILE_A_APP_ID].remote_bda, true, 0x00);
        ESP_LOGI(AP_TAG, "ESP_GAP_BLE_PASSKEY_REQ_EVT");
        break;
    case ESP_GAP_BLE_OOB_REQ_EVT: {
        ESP_LOGI(AP_TAG, "ESP_GAP_BLE_OOB_REQ_EVT");
        uint8_t tk[16] = {1}; //If you paired with OOB, both devices need to use the same tk
        esp_ble_oob_req_reply(param->ble_security.ble_req.bd_addr, tk, sizeof(tk));
        break;
    }
    case ESP_GAP_BLE_LOCAL_IR_EVT:                               /* BLE local IR event */
        ESP_LOGI(AP_TAG, "ESP_GAP_BLE_LOCAL_IR_EVT");
        break;
    case ESP_GAP_BLE_LOCAL_ER_EVT:                               /* BLE local ER event */
        ESP_LOGI(AP_TAG, "ESP_GAP_BLE_LOCAL_ER_EVT");
        break;
    case ESP_GAP_BLE_SEC_REQ_EVT:
        /* send the positive(true) security response to the peer device to accept the security request.
        If not accept the security request, should send the security response with negative(false) accept value*/
        esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
        break;
    case ESP_GAP_BLE_NC_REQ_EVT:
        /* The app will receive this evt when the IO has DisplayYesNO capability and the peer device IO also has DisplayYesNo capability.
        show the passkey number to the user to confirm it with the number displayed by peer device. */
        esp_ble_confirm_reply(param->ble_security.ble_req.bd_addr, true);
        ESP_LOGI(AP_TAG, "ESP_GAP_BLE_NC_REQ_EVT, the passkey Notify number:%d", param->ble_security.key_notif.passkey);
        break;
    case ESP_GAP_BLE_PASSKEY_NOTIF_EVT:  ///the app will receive this evt when the IO  has Output capability and the peer device IO has Input capability.
        ///show the passkey number to the user to input it in the peer device.
        ESP_LOGI(AP_TAG, "The passkey Notify number:%06d", param->ble_security.key_notif.passkey);
        break;
    case ESP_GAP_BLE_KEY_EVT:
        //shows the ble key info share with peer device to the user.
        ESP_LOGI(AP_TAG, "key type = %s", esp_key_type_to_str(param->ble_security.ble_key.key_type));
        break;
    case ESP_GAP_BLE_AUTH_CMPL_EVT: {
        char bd_addr[20];
	bin2hex(param->ble_security.auth_cmpl.bd_addr, bd_addr,6,0x3a);
        ESP_LOGI(AP_TAG, "Remote BD_ADDR: %s", bd_addr);
        ESP_LOGI(AP_TAG, "Address type = %d", param->ble_security.auth_cmpl.addr_type);
        ESP_LOGI(AP_TAG, "Pair status = %s",param->ble_security.auth_cmpl.success ? "success" : "fail");
        if (!param->ble_security.auth_cmpl.success) {
            ESP_LOGI(AP_TAG, "Fail reason = 0x%X",param->ble_security.auth_cmpl.fail_reason);
//	if (param->ble_security.auth_cmpl.fail_reason == 0x52) esp_ble_remove_bond_device(param->ble_security.auth_cmpl.bd_addr);
        } else {
	ESP_LOGI(AP_TAG, "Auth mode = %s",esp_auth_req_to_str(param->ble_security.auth_cmpl.auth_mode));
        }
        break;
	}


    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT: 
//step 1: start scan only if not update, if not scanning, if not already starting scan or if no connections is opening   
	if (!f_update && !Isscanning && !StartingScan && (!BleDevStA.btconnect || BleDevStA.btopen) && (!BleDevStB.btconnect || BleDevStB.btopen) && (!BleDevStC.btconnect || BleDevStC.btopen)) {
//step 2: start scan if defined but not open connection present
//        if ((BleDevStA.REQ_NAME[0] && !BleDevStA.btconnect) || (BleDevStB.REQ_NAME[0] && !BleDevStB.btconnect) ||(BleDevStC.REQ_NAME[0] && !BleDevStC.btconnect)) {
	uint32_t duration = 0; //30
	FND_NAME[0] = 0;
	FND_ADDR[0] = 0;
	FND_ADDRx[0] = 0;
        esp_ble_gap_start_scanning(duration);
	StartingScan = true;
	ESP_LOGI(AP_TAG, "Scan starting");
//		}
	}
        break;

    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
        //scan start complete event to indicate scan start successfully or failed
	StartingScan = false;
        if (param->scan_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(AP_TAG, "scan start failed, error status = 0x%X", param->scan_start_cmpl.status);
//	if (ble_mon && !f_update && !Isscanning) esp_restart();
//	if (!f_update && !Isscanning) esp_restart();
	break;
        } 
	ESP_LOGI(AP_TAG, "Scan start success");
	Isscanning = true;
        break;

    case ESP_GAP_BLE_SCAN_RESULT_EVT: {
        esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *)param;
	char devname[32];
        switch (scan_result->scan_rst.search_evt) {
        case ESP_GAP_SEARCH_INQ_RES_EVT:
	bin2hex(scan_result->scan_rst.bda, FND_ADDR,6,0);
	bin2hex(scan_result->scan_rst.bda, FND_ADDRx,6,0x3a);
	FND_RSSI = scan_result->scan_rst.rssi;
        ESP_LOGI(AP_TAG, "Remote BD_ADDR: %s", FND_ADDRx);
//	ESP_LOGI(AP_TAG, "searched Adv Data Len %d, Scan Response Len %d", scan_result->scan_rst.adv_data_len, scan_result->scan_rst.scan_rsp_len);
            adv_name = esp_ble_resolve_adv_data(scan_result->scan_rst.ble_adv,
                                                ESP_BLE_AD_TYPE_NAME_CMPL, &adv_name_len);
	memset(devname,0,32);
	if (adv_name_len && (adv_name_len < 32)) memcpy(devname,adv_name, adv_name_len);
	ESP_LOGI(AP_TAG, "Rssi %d dBm, Device Name: %s", scan_result->scan_rst.rssi, devname);

//#if CONFIG_EXAMPLE_DUMP_ADV_DATA_AND_SCAN_RESP
            if (scan_result->scan_rst.adv_data_len > 0) {
                ESP_LOGI(AP_TAG, "Adv data:");
                esp_log_buffer_hex(AP_TAG, &scan_result->scan_rst.ble_adv[0], scan_result->scan_rst.adv_data_len);
            }
            if (scan_result->scan_rst.scan_rsp_len > 0) {
                ESP_LOGI(AP_TAG, "Scan resp:");
                esp_log_buffer_hex(AP_TAG, &scan_result->scan_rst.ble_adv[scan_result->scan_rst.adv_data_len], scan_result->scan_rst.scan_rsp_len);
            }
//#endif
            ESP_LOGI(AP_TAG, "\n");

            if (adv_name_len)  {
            int fnd_namelen = adv_name_len;
	    if (fnd_namelen > 15) fnd_namelen = 15;		
            mystrcpy(FND_NAME, (char *)adv_name,  fnd_namelen);
	} else FND_NAME[0] = 0;
// blemon
	if (ble_mon) {
	uint8_t i = 0;
	uint8_t id = 1;
	uint8_t found = 0;
	if ((scan_result->scan_rst.adv_data_len > 21) && !memcmp(&scan_result->scan_rst.ble_adv[0],"\x1a\xff\x4c\x00\x02\x15",6)) id = 2;
	else if ((scan_result->scan_rst.adv_data_len > 17) && !memcmp(&scan_result->scan_rst.ble_adv[0],"\x12\x16\x1a\x18",4)) id = 3;
	else if ((scan_result->scan_rst.adv_data_len > 18) && !memcmp(&scan_result->scan_rst.ble_adv[0],"\x13\x16\x1a\x18",4)) id = 3;

	while ((i < BleMonNum) && (!found)) {
	if (BleMR[i].id == id) {
      	if (((id != 2) && !memcmp(BleMR[i].mac, scan_result->scan_rst.bda, 6)) ||
 		((id == 2) && !memcmp(BleMR[i].mac, &scan_result->scan_rst.ble_adv[6], 16))) {
	BleMX[i].state = 1;
	if (!BleMX[i].ttick) t_lasts_us = ~t_lasts_us;
        if (BleMR[i].sto) {
	BleMX[i].mto = BleMR[i].sto - BleMX[i].ttick;
	BleMX[i].ttick = BleMR[i].sto;
        } else {
	BleMX[i].mto = BleMonDefTO - BleMX[i].ttick;
        BleMX[i].ttick = BleMonDefTO;
	}
	BleMX[i].rssi = scan_result->scan_rst.rssi;
	if (adv_name_len) mystrcpy(BleMX[i].name, (char *)adv_name,  15);
        BleMX[i].advdatlen = scan_result->scan_rst.adv_data_len & 0x1f;
	if (BleMX[i].advdatlen) memcpy(BleMX[i].advdat, &scan_result->scan_rst.ble_adv[0], BleMX[i].advdatlen);
	else memset(BleMX[i].advdat,0,32);
        BleMX[i].scrsplen =  scan_result->scan_rst.scan_rsp_len & 0x1f;
	if (BleMX[i].scrsplen) memcpy(BleMX[i].scrsp, &scan_result->scan_rst.ble_adv[scan_result->scan_rst.adv_data_len], BleMX[i].scrsplen);
	else memset(BleMX[i].scrsp,0,32);
	if (id == 3) {
        BleMX[i].par1 = BleMX[i].advdat[10] + (BleMX[i].advdat[11] << 8);
        BleMX[i].par2 = BleMX[i].advdat[12] + (BleMX[i].advdat[13] << 8);
        BleMX[i].par3 = BleMX[i].advdat[14] + (BleMX[i].advdat[15] << 8);
        BleMX[i].par4 = BleMX[i].advdat[16];
        BleMX[i].par5 = BleMX[i].advdat[17];
        BleMX[i].par6 = BleMX[i].advdat[18];
	}
	found = 1;
	}
	}
	i++;
	}
	i = 0;
	while ((i < BleMonNum) && (!found)) {
      	if (!BleMR[i].sto && !BleMX[i].ttick) {
	BleMX[i].state = 1;
	t_lasts_us = ~t_lasts_us;
	BleMX[i].mto = 0;
        BleMX[i].ttick = BleMonDefTO;
	BleMX[i].rssi = scan_result->scan_rst.rssi;
        if (id != 2) memcpy(BleMR[i].mac, scan_result->scan_rst.bda, 6);
        else if (id == 2) memcpy(BleMR[i].mac, &scan_result->scan_rst.ble_adv[6], 16);
	BleMR[i].id = id;
	if (adv_name_len) mystrcpy(BleMX[i].name, (char *)adv_name,  15);
	else memset(BleMX[i].name,0,16);
	memset(BleMX[i].advdat,0,32);
        BleMX[i].advdatlen = scan_result->scan_rst.adv_data_len & 0x1f;
	if (BleMX[i].advdatlen) memcpy(BleMX[i].advdat, &scan_result->scan_rst.ble_adv[0], BleMX[i].advdatlen);
	memset(BleMX[i].scrsp,0,32);
        BleMX[i].scrsplen =  scan_result->scan_rst.scan_rsp_len & 0x1f;
	if (BleMX[i].scrsplen) memcpy(BleMX[i].scrsp, &scan_result->scan_rst.ble_adv[scan_result->scan_rst.adv_data_len], BleMX[i].scrsplen);
	found = 1;
	if (id == 3) {
        BleMX[i].par1 = BleMX[i].advdat[10] + (BleMX[i].advdat[11] << 8);
        BleMX[i].par2 = BleMX[i].advdat[12] + (BleMX[i].advdat[13] << 8);
        BleMX[i].par3 = BleMX[i].advdat[14] + (BleMX[i].advdat[15] << 8);
        BleMX[i].par4 = BleMX[i].advdat[16];
        BleMX[i].par5 = BleMX[i].advdat[17];
        BleMX[i].par6 = BleMX[i].advdat[18];
	}
	}
	i++;
	}
	}
//
            if ((adv_name != NULL) && (BleDevStA.btopen || !BleDevStA.btconnect) && (BleDevStB.btopen || !BleDevStB.btconnect) && (BleDevStC.btopen || !BleDevStC.btconnect)) {

                if (((BleDevStA.REQ_NAME[0]) &&  strlen(BleDevStA.REQ_NAME) == adv_name_len && !strncmp((char *)adv_name, BleDevStA.REQ_NAME, adv_name_len)) ||
//                ((BleDevStA.REQ_NAME[0]) &&  strlen(BleDevStA.REQ_NAME) == 17 && !incascmp(BleDevStA.REQ_NAME,FND_ADDRx,17)) ||
                ((BleDevStA.REQ_NAME[0]) &&  strlen(BleDevStA.REQ_NAME) == 12 && !incascmp(BleDevStA.REQ_NAME,FND_ADDR,12))) {
//                if ((BleDevStA.REQ_NAME[0]) &&  strlen(BleDevStA.REQ_NAME) == adv_name_len && !strncmp((char *)adv_name, BleDevStA.REQ_NAME, adv_name_len)) {
			if (FND_NAME[0]) strcpy (BleDevStA.RQC_NAME,FND_NAME);
			else strcpy (BleDevStA.RQC_NAME,BleDevStA.REQ_NAME);
                    ESP_LOGI(AP_TAG, "Searched 1 device %s\n", BleDevStA.RQC_NAME);
                    if (BleDevStA.btconnect == false) {
			BleDevStA.btopen = false;
                        BleDevStA.btconnect = true;
                        memcpy(&(scan_rsta), scan_result, sizeof(esp_ble_gap_cb_param_t));
                        if (Isscanning) esp_ble_gap_stop_scanning();
                    }
                }

                else if (((BleDevStB.REQ_NAME[0]) &&  strlen(BleDevStB.REQ_NAME) == adv_name_len && !strncmp((char *)adv_name, BleDevStB.REQ_NAME, adv_name_len)) ||
//                ((BleDevStB.REQ_NAME[0]) &&  strlen(BleDevStB.REQ_NAME) == 17 && !incascmp(BleDevStB.REQ_NAME,FND_ADDRx,17)) ||
                ((BleDevStB.REQ_NAME[0]) &&  strlen(BleDevStB.REQ_NAME) == 12 && !incascmp(BleDevStB.REQ_NAME,FND_ADDR,12))) {
//                else if ((BleDevStB.REQ_NAME[0]) &&  strlen(BleDevStB.REQ_NAME) == adv_name_len && !strncmp((char *)adv_name, BleDevStB.REQ_NAME, adv_name_len)) {
			if (FND_NAME[0]) strcpy (BleDevStB.RQC_NAME,FND_NAME);
			else strcpy (BleDevStB.RQC_NAME,BleDevStB.REQ_NAME);
                    ESP_LOGI(AP_TAG, "Searched 2 device %s\n", BleDevStB.RQC_NAME);
                    if (BleDevStB.btconnect == false) {
			BleDevStB.btopen = false;
                        BleDevStB.btconnect = true;
                        memcpy(&(scan_rstb), scan_result, sizeof(esp_ble_gap_cb_param_t));
                        if (Isscanning) esp_ble_gap_stop_scanning();
                    }
                }

                else if (((BleDevStC.REQ_NAME[0]) &&  strlen(BleDevStC.REQ_NAME) == adv_name_len && !strncmp((char *)adv_name, BleDevStC.REQ_NAME, adv_name_len)) ||
//                ((BleDevStC.REQ_NAME[0]) &&  strlen(BleDevStC.REQ_NAME) == 17 && !incascmp(BleDevStC.REQ_NAME,FND_ADDRx,17)) ||
                ((BleDevStC.REQ_NAME[0]) &&  strlen(BleDevStC.REQ_NAME) == 12 && !incascmp(BleDevStC.REQ_NAME,FND_ADDR,12))) {
//                else if ( (BleDevStC.REQ_NAME[0]) &&  strlen(BleDevStC.REQ_NAME) == adv_name_len && !strncmp((char *)adv_name, BleDevStC.REQ_NAME, adv_name_len)) {
			if (FND_NAME[0]) strcpy (BleDevStC.RQC_NAME,FND_NAME);
			else strcpy (BleDevStC.RQC_NAME,BleDevStC.REQ_NAME);
                    ESP_LOGI(AP_TAG, "Searched 3 device %s\n", BleDevStC.RQC_NAME);
                    if (BleDevStC.btconnect == false) {
			BleDevStC.btopen = false;
                        BleDevStC.btconnect = true;
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
            ESP_LOGE(AP_TAG, "Scan stop failed, error status = 0x%X", param->scan_stop_cmpl.status);
        } else ESP_LOGI(AP_TAG, "Scan stop successfully");

	if (!BleDevStA.btopen && BleDevStA.btconnect) {
                        ESP_LOGI(AP_TAG, "Connect 1 to the remote device");
                        esp_ble_gattc_open(gl_profile_tab[PROFILE_A_APP_ID].gattc_if, scan_rsta.scan_rst.bda, scan_rsta.scan_rst.ble_addr_type, true);
	} else if (!BleDevStB.btopen && BleDevStB.btconnect) {
                        ESP_LOGI(AP_TAG, "Connect 2 to the remote device");
                        esp_ble_gattc_open(gl_profile_tab[PROFILE_B_APP_ID].gattc_if, scan_rstb.scan_rst.bda, scan_rstb.scan_rst.ble_addr_type, true);
	} else if (!BleDevStC.btopen && BleDevStC.btconnect) {
                        ESP_LOGI(AP_TAG, "Connect 3 to the remote device");
                        esp_ble_gattc_open(gl_profile_tab[PROFILE_C_APP_ID].gattc_if, scan_rstc.scan_rst.bda, scan_rstc.scan_rst.ble_addr_type, true);
	} else {
	start_scan();
	}
	break;

    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS){
            ESP_LOGE(AP_TAG, "Adv stop failed, error status = 0x%X", param->adv_stop_cmpl.status);
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
//*** Gap event handler end **********************

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
        uint8_t idx;
        for (idx = 0; idx < PROFILE_NUM; idx++) {
            if (gattc_if == ESP_GATT_IF_NONE || /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
                    gattc_if == gl_profile_tab[idx].gattc_if) {

                    gattc_profile_cm_event_handler(idx,event, gattc_if, param);

            }
        }
    } while (0);
}

//******************** mi **********************
bool mMiOff(uint8_t blenum) {
	if (blenum > 2) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	ptr->sendDataLen = 0;
	ptr->sendDataHandle = 4;  //setup
	ptr->sendData[0] = 1;
	ptr->sendData[1] = 40;
	ptr->sendDataLen = 2;
	if (ptr->btauthoriz) {
	esp_ble_gap_read_rssi(gl_profile_tab[blenum].remote_bda);
	ptr->notifyDataLen = -1;
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	while (--timeout && ((ptr->notifyDataLen == -1) || (ptr->notifyData[0] != 3) || (ptr->notifyData[4] != 40))) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	if ((ptr->notifyData[0] == 3) && (ptr->notifyData[1] == 2) && (ptr->notifyData[4] == 40)) return true;
	else {
	return false;
	}
	} else ptr->iRssi = 0;
	return false;
}

bool mMiOfft(uint8_t blenum) {
	if (blenum > 2) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	ptr->sendDataLen = 0;
	ptr->sendDataHandle = 4;  //setup
	ptr->sendData[0] = 1;
	ptr->sendData[1] = ptr->bHtemp;
	ptr->sendDataLen = 2;
	if (ptr->btauthoriz) {
	esp_ble_gap_read_rssi(gl_profile_tab[blenum].remote_bda);
	ptr->notifyDataLen = -1;
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	while (--timeout && ((ptr->notifyDataLen == -1) || (ptr->notifyData[0] != 3) || (ptr->notifyData[4] != 40))) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	if ((ptr->notifyData[0] == 3) && (ptr->notifyData[1] == 2) && (ptr->notifyData[4] == 40)) return true;
	else {
	return false;
	}
	} else ptr->iRssi = 0;
	return false;
}

bool mMiBoil(uint8_t blenum) {
	if (blenum > 2) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	ptr->sendDataLen = 0;
	ptr->sendDataHandle = 4;  //setup
	ptr->sendData[0] = 0;
	ptr->sendData[1] = ptr->bHtemp;
	ptr->sendDataLen = 2;
	if (ptr->btauthoriz) {
	esp_ble_gap_read_rssi(gl_profile_tab[blenum].remote_bda);
	ptr->notifyDataLen = -1;
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	while (--timeout && ((ptr->notifyDataLen == -1) || (ptr->notifyData[0] != 1))) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	if ((ptr->notifyData[0] == 1) && (ptr->notifyData[1] == 2)) return true;
	else {
	return false;
	}
	} else ptr->iRssi = 0;
	return true;
}

bool mMiHeat(uint8_t blenum, uint8_t temp) {
	if (blenum > 2) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	if (ptr->btauthoriz) {
	if (temp) {
	ptr->sendDataLen = 0;
	ptr->sendDataHandle = 4;  //setup
	ptr->sendData[0] = 1;
	ptr->sendData[1] = temp;
	ptr->sendDataLen = 2;
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_A_APP_ID].remote_bda);
	ptr->notifyDataLen = -1;
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	while (--timeout && ((ptr->notifyDataLen == -1) || (ptr->notifyData[0] != 3) || (ptr->notifyData[4] != temp))) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	if ((ptr->notifyData[0] == 3) && (ptr->notifyData[1] == 2) && (ptr->notifyData[4] == temp)) return true;
	else {
	return false;
	}
	} else {
	ptr->xshedcom = 2;
	ptr->sendDataLen = 0;
	ptr->sendDataHandle = 4;  //setup
	ptr->sendData[0] = 1;
	ptr->sendData[1] = ptr->bCtemp;
	ptr->sendDataLen = 2;
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_A_APP_ID].remote_bda);
	ptr->notifyDataLen = -1;
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	while (--timeout && ((ptr->notifyDataLen == -1) || (ptr->notifyData[0] != 3) || (ptr->notifyData[4] != ptr->bCtemp))) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	ptr->sendDataHandle = 5;  //time
	ptr->sendData[0] = 0;
	ptr->sendDataLen = 1;
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_A_APP_ID].remote_bda);

	}
	} else ptr->iRssi = 0;
	return true;
}

bool mMiIdlTmp(uint8_t blenum, uint8_t temp) {
	if (blenum > 2) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	ptr->sendDataLen = 0;
	ptr->sendDataHandle = 4;  //setup
	ptr->sendData[0] = 1;
	ptr->sendData[1] = temp;
	ptr->sendDataLen = 2;
	if (ptr->btauthoriz) {
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_A_APP_ID].remote_bda);
	ptr->notifyDataLen = -1;
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	while (--timeout && ((ptr->notifyDataLen == -1) || (ptr->notifyData[4] != temp))) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	if (ptr->notifyData[4] == temp) return true;
	else return false;
	} else ptr->iRssi = 0;
	return true;
}

bool mMiRewarm(uint8_t blenum) {
	if (blenum > 2) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	bool retc = true;
	if (ptr->btauthoriz) {
	ptr->xshedcom = 1;
	ptr->sendDataLen = 0;
	ptr->sendDataHandle = 4;  //setup
	ptr->sendData[0] = 0;
	ptr->sendData[1] = ptr->bHtemp;
	ptr->sendDataLen = 2;
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_A_APP_ID].remote_bda);
	ptr->notifyDataLen = -1;
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	while (--timeout && ((ptr->notifyDataLen == -1) || (ptr->notifyData[0] != 1))) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	if (ptr->notifyData[0] != 1) retc = false;
	ptr->sendDataHandle = 4;  //setup
	ptr->sendData[0] = 1;
	ptr->sendData[1] = ptr->bHtemp;
	ptr->sendDataLen = 2;
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_A_APP_ID].remote_bda);
	ptr->notifyDataLen = -1;
	timeout = 30; 	// 100*30 = 3 seconds
	while (--timeout && ((ptr->notifyDataLen == -1) || (ptr->notifyData[0] != 3))) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	if ((ptr->notifyData[0] != 3) || (ptr->notifyData[1] != 2)) retc = false;
	} else {
	ptr->iRssi = 0;
	retc = false;
	}
	if (retc) ptr->xshedcom = 0;
	return retc;
}

bool mMiSWtime(uint8_t blenum) {
	if (blenum > 2) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	bool retc = true;
	if (ptr->btauthoriz) {
	ptr->sendDataLen = 0;
	ptr->sendDataHandle = 5;  //time
	ptr->sendData[0] = 0x18;
	ptr->sendDataLen = 1;
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_A_APP_ID].remote_bda);
	ptr->notifyDataLen = -1;
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	while (--timeout && ((ptr->notifyDataLen == -1) || (ptr->notifyData[10] != 0x18))) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	if (ptr->notifyData[10] != 0x18) retc = false;
	} else {
	ptr->iRssi = 0;
	retc = false;
	}
	return retc;
}


//******************** r4s **********************

//[I][R4S.cpp:24]   r4sWriteA(): >> 55 59 06 aa
//                         offset:  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19
//[I][R4S.cpp:43] r4sCommandA(): << 55 59 06 00 00 00 00 00 02 00 00 00 00 39 00 00 00 00 00 aa

uint8_t r4sWrite(uint8_t blenum, uint8_t cmd, uint8_t* data, size_t len) {
	if (blenum > 2) return -1;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	size_t sz = 4 + len; // 55, counter, cmd, AA
	ptr->sendData[0] = 0x55;
	ptr->sendData[1] = ptr->r4scounter;
	ptr->sendData[2] = cmd;
	ptr->sendData[sz - 1] = 0xAA;
	if (len > 0) {
	memcpy(&ptr->sendData[3], data, len);
		}
	ptr->sendDataLen = sz;
//  ble_gap_read_rssi event return rssi and start sending data
	if (ptr->btauthoriz) esp_ble_gap_read_rssi(gl_profile_tab[blenum].remote_bda);
	else {
	ptr->iRssi = 0;
	ptr->r4scounter = -1;
	}
	return ptr->r4scounter++;
}

int8_t r4sCommand(uint8_t blenum, uint8_t cmd, uint8_t* data, size_t len) {
	uint8_t blenum1 = blenum + 1;
	if (blenum > 2) return -1;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	ptr->notifyDataLen = -1;
	if (ptr->btauthoriz) {
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	uint8_t cnt = r4sWrite(blenum, cmd, data, len);
	while (--timeout && (ptr->notifyDataLen == -1)) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
//
	if ((ptr->notifyDataLen < BLE_INPUT_BUFFSIZE) && (ptr->notifyDataLen > 1)) {
	ptr->r4sConnErr = 0;	
ESP_LOGI(AP_TAG, "Receive Data %d: ", blenum1);
esp_log_buffer_hex(AP_TAG, ptr->notifyData, ptr->notifyDataLen);
	}
//
	if ((ptr->notifyDataLen > 1) && (ptr->notifyData[1] != cnt)) {
	ptr->notifyDataLen = -1;
	ptr->r4sConnErr++;	
	}
	}
	return ptr->notifyDataLen;
}
bool m171sOff(uint8_t blenum) {
	if (blenum > 2) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	if (r4sCommand(blenum, 0x04, 0, 0) != 5)
	return false;
	return ptr->notifyData[3] == 1;
}

bool m103sOn(uint8_t blenum) {
	if (blenum > 2) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	if (r4sCommand(blenum, 0x03, 0, 0) != 5) return false;
	return ptr->notifyData[3] == 1;
}

bool m103sPon(uint8_t blenum, uint8_t pwr) {
	if (blenum > 2) return false;
	uint8_t pwrc = 100;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	if (pwr < 25) pwrc = 25;
	else if (pwr < 50) pwrc = 50;
	else if (pwr < 75) pwrc = 75;
	uint8_t data[] = { 0, 0, pwrc, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	if (r4sCommand(blenum, 0x05, data, sizeof(data)) != 5) return false;
	return ptr->notifyData[3] == 1;
}

bool m103sLon(uint8_t blenum) {
	if (blenum > 2) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	uint8_t data[] = { 1 };
	if (r4sCommand(blenum, 0x16, data, sizeof(data)) != 5)
	return false;
	return ptr->notifyData[3] == 1;
}

bool m103sLoff(uint8_t blenum) {
	if (blenum > 2) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	uint8_t data[] = { 0 };
	if (r4sCommand(blenum, 0x16, data, sizeof(data)) != 5)
	return false;
	return ptr->notifyData[3] == 1;
}
bool m103sTon(uint8_t blenum) {
	if (blenum > 2) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	uint8_t data[] = { 1 };
	if (r4sCommand(blenum, 0x1b, data, sizeof(data)) != 5)
	return false;
	return ptr->notifyData[3] == 1;
}

bool m103sToff(uint8_t blenum) {
	if (blenum > 2) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	uint8_t data[] = { 0 };
	if (r4sCommand(blenum, 0x1b, data, sizeof(data)) != 5)
	return false;
	return ptr->notifyData[3] == 1;
}

bool m151sDon(uint8_t blenum, uint8_t phour, uint8_t pmin) {
	if (blenum > 2) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	uint8_t data[] = { 0, 0, 0, phour, pmin, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0};
	if (r4sCommand(blenum, 0x05, data, sizeof(data)) != 5)
	return false;
	return ptr->notifyData[3] == 1;

	if (r4sCommand(blenum, 0x03, 0, 0) != 5)
	return false;
	if (ptr->notifyData[3] != 1)
	return false;
    
	return true;
}
bool m151sLon(uint8_t blenum) {
	if (blenum > 2) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	uint8_t data[] = { 1 };
	if (r4sCommand(blenum, 0x3e, data, sizeof(data)) != 5)
	return false;
	return ptr->notifyData[3] == 1;
}

bool m151sLoff(uint8_t blenum) {
	if (blenum > 2) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	uint8_t data[] = { 0 };
	if (r4sCommand(blenum, 0x3e, data, sizeof(data)) != 5)
	return false;
	return ptr->notifyData[3] == 1;
}

bool m171sBlTm(uint8_t blenum, int8_t bltime) {
	if (blenum > 2) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	if ((ptr->DEV_TYP > 3) && (ptr->DEV_TYP < 10)) {
	uint8_t data[] = { ptr->bProg, 0, ptr->bHtemp, 0, 1, ptr->bHtemp, 30, 0, 0, 0, 0, 0, 0, (bltime + 128), 0, 0};
	if (r4sCommand(blenum, 0x05, data, sizeof(data)) != 5) return false;
	if (ptr->notifyData[3] != 1) return false;
	} else {
	return false;
	}
	return true;
}

bool m171sOn(uint8_t blenum, uint8_t prog, uint8_t temp) {
	if (blenum > 2) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	if (ptr->DEV_TYP > 3) {
	uint8_t data[] = { prog, 0, temp, 0, 1, temp, 30, 0, 0, 0, 0, 0, 0, (ptr->bBlTime + 128), 0, 0};
	if (r4sCommand(blenum, 0x05, data, sizeof(data)) != 5) return false;
	if (ptr->notifyData[3] != 1) return false;
	if (r4sCommand(blenum, 0x03, 0, 0) != 5) return false;
	if (ptr->notifyData[3] != 1) return false;
	} else if (ptr->DEV_TYP == 1) {	
	uint8_t data[] = { prog, temp, 0, 0 };
	if (r4sCommand(blenum, 0x05, data, sizeof(data)) != 5) return false;
	if (ptr->notifyData[3] == 1) return  false;
	} else {
	uint8_t data[] = { prog, 0, temp, 0 };
	if (r4sCommand(blenum, 0x05, data, sizeof(data)) != 5) return false;
	if (ptr->notifyData[3] == 1) return  false;
	if (ptr->DEV_TYP > 2) {
	if (r4sCommand(blenum, 0x03, 0, 0) != 5) return false;
	if (ptr->notifyData[3] != 1) return false;
	}
	}
	return true;
}

bool m171sBoil(uint8_t blenum) {
	return m171sOn(blenum, 0, 0);
}

bool m171sHeat(uint8_t blenum, uint8_t temp) {
	return m171sOn(blenum, 1, temp);
}

bool m171sBoilAndHeat(uint8_t blenum, uint8_t temp) {
	if (blenum > 2) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	if (ptr->DEV_TYP > 2)  return m171sOn(blenum, 2, temp);
	else return m171sOn(blenum, 0, temp);
}

bool m171s_NLOn(uint8_t blenum) {
	if (blenum > 2) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	if (r4sCommand(blenum, 0x32, nl_settings, sizeof(nl_settings)) != 5)
	return false;
	if (ptr->notifyData[3] == 1)
	return false;
    
	uint8_t data[] = { 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, (ptr->bBlTime + 128), 0, 0};
	if (r4sCommand(blenum, 0x05, data, sizeof(data)) != 5)
	return false;
	if (ptr->notifyData[3] == 1)
	return false;
    
	if (r4sCommand(blenum, 0x03, 0, 0) != 5)
	return false;
	if (ptr->notifyData[3] != 1)
	return false;
    
	return true;
}

bool m171s_ModOff(uint8_t blenum) {
	if (blenum > 2) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
//	if (!ptr->bProg) return true;
	if (ptr->DEV_TYP > 3) {
	uint8_t data[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, (ptr->bBlTime + 128), 0, 0};
	if (r4sCommand(blenum, 0x05, data, sizeof(data)) != 5) return false;
	if (ptr->notifyData[3] == 1) return false;
	} else if (ptr->DEV_TYP == 1) {	
	uint8_t data[] = { 1, 0, 0, 0 };  // heat,temp,0,0
	if (r4sCommand(blenum, 0x05, data, sizeof(data)) != 5) return false;
	if (ptr->notifyData[3] == 1) return false;
	} else {
	uint8_t data[] = { 1, 0, 0, 0 };  // heat,0,temp,0
	if (r4sCommand(blenum, 0x05, data, sizeof(data)) != 5) return false;
	if (ptr->notifyData[3] == 1) return  false;
	}
	return true;
}

bool m171Bl(uint8_t blenum, uint8_t state) {
	if (blenum > 2) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	uint8_t data[] = { 0xc8, 0xc8, state };
	if (ptr->DEV_TYP > 3) {
	if (r4sCommand(blenum, 0x37, data, sizeof(data)) != 5) return false;
	if (ptr->notifyData[3] != 0) return false;
	}
	return true;
}

bool m171Bp(uint8_t blenum, uint8_t state) {
	if (blenum > 2) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	uint8_t data[] = { state };
	if (ptr->DEV_TYP > 3) {
	if (r4sCommand(blenum, 0x3c, data, sizeof(data)) != 5) return false;
	if (ptr->notifyData[3] != 1) return false;
	}
	return true;
}

bool rm800sOn(uint8_t blenum) {
	if (blenum > 2) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	if (r4sCommand(blenum, 0x03, 0, 0) != 5)
	return false;
	return ptr->notifyData[3] == 1;
}

bool rm800sOff(uint8_t blenum) {
	if (blenum > 2) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	if (r4sCommand(blenum, 0x04, 0, 0) != 5)
	return false;
	return ptr->notifyData[3] == 1;
}

bool rm800sPall(uint8_t blenum, uint8_t prog, uint8_t mod, uint8_t temp, uint8_t phour, uint8_t pmin, uint8_t dhour, uint8_t dmin, uint8_t warm) {
	if (blenum > 2) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	if (( ptr->DEV_TYP == 16 ) && (prog > 12)) return false;
	if (( ptr->DEV_TYP == 17 ) && (prog > 16)) return false;
	if (( ptr->DEV_TYP == 18 ) && (prog > 10)) return false;
	if (( ptr->DEV_TYP == 19 ) && (prog > 9)) return false;
	if (( ptr->DEV_TYP == 20 ) && (prog > 17)) return false;
	uint8_t data[] = { prog, 0, 0, 0, 0, dhour, dmin, warm};
        if ( ptr->DEV_TYP < 24 ) {
        if ( ptr->DEV_TYP == 16 ) {
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
	} else if ( ptr->DEV_TYP == 17 ) {
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
	} else if ( ptr->DEV_TYP == 18 ) {
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
	} else if ( ptr->DEV_TYP == 19 ) {
// for RMC-961s
	switch (prog) {
	case 0:	//rice рис/крупы
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 10;		//smin
	break;

	case 1:		//frying  жарка
	data[1] = 0;     	//mode
	data[2] = 150;		//temp
	data[3] = 0;		//shour
	data[4] = 15;		//smin
	break;

	case 2:		//steam пар
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 25;		//smin
	break;

	case 3:		//baking выпечка
	data[1] = 0;     	//mode
	data[2] = 140;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 4:		//stewing тушение
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 5:		//multicooker мультиповар
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 30;		//smin
	break;

	case 6:			//pilaf плов
	data[1] = 0;     	//mode
	data[2] = 110;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 7:			//soup суп
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 8:			//milk_porridge молочная каша
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 30;		//smin
	break;

	case 9:			//yogurt йогурт
	data[1] = 0;     	//mode
	data[2] = 38;		//temp
	data[3] = 8;		//shour
	data[4] = 0;		//smin
	break;
	}
	} else if ( ptr->DEV_TYP == 20 ) {
// for RMC-92s
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

	case 17:		//warming разогрев
	data[1] = 0;     	//mode
	data[2] = 70;		//temp
	data[3] = 0;		//shour
	data[4] = 30;		//smin
	break;
	}
	}
	if ((data[1]) && (mod) && (mod < 4)) data[1] = mod;
	if (temp) data[2] = temp;
	if (phour || pmin) {
	data[3] = phour;
	data[4] = pmin;
	}	

	if (r4sCommand(blenum, 0x05, data, sizeof(data)) != 5) return false;
	if (ptr->notifyData[3] != 1) return false;
	ptr->bDHour = dhour;
	ptr->bDMin = dmin;
	return true;
	} else {
	if (r4sCommand(blenum, 0x09, data, 1) != 5) return false;
	if (ptr->notifyData[3] != 1) return false;
	data[0] = mod;
	if (r4sCommand(blenum, 0x0a, data, 1) != 5) return false;
	if (ptr->notifyData[3] != 1) return false;
	data[0] = phour;
	data[1] = pmin;
	if (r4sCommand(blenum, 0x0c, data, 2) != 5) return false;
	if (ptr->notifyData[3] != 1) return false;
	data[0] = dhour;
	data[1] = dmin;
	if (r4sCommand(blenum, 0x14, data, 2) != 5) return false;
	if (ptr->notifyData[3] != 1) return false;
	uint16_t ttemp = temp * 10;
	data[0] = ttemp & 0xff;
	data[1] = ((ttemp)>>8 & 0xff) | 0xf0;
	if (r4sCommand(blenum, 0x0b, data, 2) != 5) return false;
	if (ptr->notifyData[3] != 1) return false;
	return true;
}
}

bool rm800sProg(uint8_t blenum, uint8_t prog) {
	if (blenum > 2) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	if (( ptr->DEV_TYP == 16 ) && (prog > 12)) return false;
	if (( ptr->DEV_TYP == 17 ) && (prog > 16)) return false;
	if (( ptr->DEV_TYP == 18 ) && (prog > 10)) return false;
	if (( ptr->DEV_TYP == 19 ) && (prog > 9)) return false;
	if (( ptr->DEV_TYP == 20 ) && (prog > 17)) return false;
	uint8_t data[] = { prog, 0, 0, 0, 0, ptr->bDHour, ptr->bDMin, ptr->bAwarm};
        if ( ptr->DEV_TYP < 24 ) {
        if ( ptr->DEV_TYP == 16 ) {
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
	} else if ( ptr->DEV_TYP == 17 ) {
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
	} else if ( ptr->DEV_TYP == 18 ) {
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
	} else if ( ptr->DEV_TYP == 19 ) {
// for RMC-961s
	switch (prog) {
	case 0:	//rice рис/крупы
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 10;		//smin
	break;

	case 1:		//frying  жарка
	data[1] = 0;     	//mode
	data[2] = 150;		//temp
	data[3] = 0;		//shour
	data[4] = 15;		//smin
	break;

	case 2:		//steam пар
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 25;		//smin
	break;

	case 3:		//baking выпечка
	data[1] = 0;     	//mode
	data[2] = 140;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 4:		//stewing тушение
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 5:		//multicooker мультиповар
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 30;		//smin
	break;

	case 6:			//pilaf плов
	data[1] = 0;     	//mode
	data[2] = 110;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 7:			//soup суп
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	break;

	case 8:			//milk_porridge молочная каша
	data[1] = 0;     	//mode
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 30;		//smin
	break;

	case 9:			//yogurt йогурт
	data[1] = 0;     	//mode
	data[2] = 38;		//temp
	data[3] = 8;		//shour
	data[4] = 0;		//smin
	break;
	}
	} else if ( ptr->DEV_TYP == 20 ) {
// for RMC-92s
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

	case 17:		//warming разогрев
	data[1] = 0;     	//mode
	data[2] = 70;		//temp
	data[3] = 0;		//shour
	data[4] = 30;		//smin
	break;
	}
	}
	if (r4sCommand(blenum, 0x05, data, sizeof(data)) != 5) return false;
	if (ptr->notifyData[3] != 1) return false;
	return true;
	} else {
	if (r4sCommand(blenum, 0x09, data, 1) != 5) return false;
	if (ptr->notifyData[3] != 1) return false;
	return true;
}
}

bool rm800sMod(uint8_t blenum, uint8_t mod) {
	if (blenum > 2) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	if (mod > 3) return false;
	uint8_t data[] = { ptr->bProg, mod, ptr->bHtemp, ptr->bPHour, ptr->bPMin, ptr->bDHour, ptr->bDMin, ptr->bAwarm};
        if ( ptr->DEV_TYP < 24 ) {
	switch (mod) {

	case 1:		//vegetables
	if (ptr->bProg == 4) {
	data[2] = 180;		//temp
	data[3] = 0;		//shour
	data[4] = 18;		//smin
	} else if (ptr->bProg == 5) {
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 40;		//smin
	} else if (ptr->bProg == 11) {
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 30;		//smin
	}
	break;

	case 2:		//fish
	if (ptr->bProg == 4) {
	data[2] = 180;		//temp
	data[3] = 0;		//shour
	data[4] = 12;		//smin
	} else if (ptr->bProg == 5) {
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 35;		//smin
	} else if (ptr->bProg == 11) {
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 25;		//smin
	}
	break;

	case 3:		//meat
	if (ptr->bProg == 4) {
	data[2] = 180;		//temp
	data[3] = 0;		//shour
	data[4] = 15;		//smin
	} else if (ptr->bProg == 5) {
	data[2] = 100;		//temp
	data[3] = 1;		//shour
	data[4] = 0;		//smin
	} else if (ptr->bProg == 11) {
	data[2] = 100;		//temp
	data[3] = 0;		//shour
	data[4] = 40;		//smin
	}
	break;
	}

	if (r4sCommand(blenum, 0x05, data, sizeof(data)) != 5) return false;
	if (ptr->notifyData[3] != 1) return false;
	return true;
	} else {
	data[0] = mod;
	if (r4sCommand(blenum, 0x0a, data, 1) != 5) return false;
	if (ptr->notifyData[3] != 1) return false;
	return true;
}
}

bool rm800sTemp(uint8_t blenum, uint8_t temp) {
	if (blenum > 2) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	uint8_t data[] = { temp, 0 };
        if ( ptr->DEV_TYP < 24 ) {
	if (r4sCommand(blenum, 0x0b, data, 1) != 5) return false;
	if (ptr->notifyData[3] != 1) return false;
	return true;
	} else {
	uint16_t ttemp = temp * 10;
	data[0] = ttemp & 0xff;
	data[1] = ((ttemp)>>8 & 0xff) | 0xf0;
	if (r4sCommand(blenum, 0x0b, data, 2) != 5) return false;
	if (ptr->notifyData[3] != 1) return false;
	return true;
	}
}

bool rm800sPhour(uint8_t blenum, uint8_t hour) {
	if (blenum > 2) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	uint8_t data[] = { hour, ptr->bPMin};
	if (r4sCommand(blenum, 0x0c, data, sizeof(data)) != 5)
	return false;
	if (ptr->notifyData[3] != 1)
	return false;
	return true;
}

bool rm800sPmin(uint8_t blenum, uint8_t min) {
	if (blenum > 2) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	uint8_t data[] = { ptr->bPHour, min};
	if (r4sCommand(blenum, 0x0c, data, sizeof(data)) != 5)
	return false;
	if (ptr->notifyData[3] != 1)
	return false;
	return true;
}

bool rm800sDhour(uint8_t blenum, uint8_t hour) {
	if (blenum > 2) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	uint8_t data[] = { hour, ptr->bDMin};
	if (r4sCommand(blenum, 0x14, data, sizeof(data)) != 5)
	return false;
	if (ptr->notifyData[3] != 1)
	return false;
	return true;
}

bool rm800sDmin(uint8_t blenum, uint8_t min) {
	if (blenum > 2) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	uint8_t data[] = { ptr->bDHour, min};
	if (r4sCommand(blenum, 0x14, data, sizeof(data)) != 5)
	return false;
	if (ptr->notifyData[3] != 1)
	return false;
	return true;
}

bool rm800sAwarm(uint8_t blenum, uint8_t warm) {
	if (blenum > 2) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	uint8_t data[] = { ptr->bProg, ptr->bModProg, ptr->bHtemp, ptr->bPHour, ptr->bPMin, ptr->bDHour, ptr->bDMin, warm};
	if (r4sCommand(blenum, 0x05, data, sizeof(data)) != 5)
	return false;
	if (ptr->notifyData[3] != 1)
	return false;
	return true;
}

bool m51sCalibrate(uint8_t blenum) {
	if (blenum > 2) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	if (r4sCommand(blenum, 0x7b, 0, 0) != 5) return false;
	return ptr->notifyData[3] == 1;
}

bool mkSync(uint8_t blenum) {
	if (blenum > 2) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	if (ptr->btauthoriz && (ptr->DEV_TYP > 3) && (ptr->DEV_TYP < 64) && ((ptr->DEV_TYP < 16) || (ptr->DEV_TYP > 23))) {
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
//ESP_LOGI(AP_TAG, "Time %d: %ld,Timezone: %d", blenum, now, tz);
//esp_log_buffer_hex(AP_TAG, data, sizeof(data));
	if (!data[3]) return false; //if data correct?
	if (r4sCommand(blenum, 0x6e, data, sizeof(data)) != 5)
	return false;
	if (ptr->notifyData[3] != 0)
	return false;
	}
	ptr->f_Sync = 0;
	return true;
}

//******************************************************************

void msStatus(uint8_t blenum) {
	if (blenum > 2) return;
        struct BleDevSt *ptr;
	char tmpvar[8]; 
	int  tmpint;
	int  retc;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}

	if (ptr->btauthoriz && (ptr->DEV_TYP < 64)) {
        retc = r4sCommand(blenum, 0x06, 0, 0);	
	if ((ptr->notifyData[2] == 6) && (ptr->DEV_TYP == 1) && (retc == 13)) {
	ptr->bCtemp = ptr->notifyData[5];
	ptr->bHeat = ptr->notifyData[3];
	ptr->bState = ptr->notifyData[11];
	if (ptr->bState == 4) ptr->bState = 0;
	ptr->bProg = ptr->notifyData[3];
	if (ptr->bProg) ptr->bState = 0;
	ptr->bStNl = 0;
	ptr->bStBl = 0;
	ptr->bStBp = 0;
	ptr->bHtemp = 0;
	switch (ptr->notifyData[4]) {
	case 1:
        ptr->bHtemp = 40;
	break;
	case 2:
        ptr->bHtemp = 55;
	break;
	case 3:
        ptr->bHtemp = 70;
	break;
	case 4:
        ptr->bHtemp = 85;
	break;
	case 5:
        ptr->bHtemp = 95;
	break;
	}
	strcpy(ptr->cStatus,"{\"temp\":");
	itoa(ptr->notifyData[5],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"target\":");
	itoa(ptr->bHtemp,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"state\":");
	itoa(ptr->notifyData[11],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"prog\":");
	itoa(ptr->notifyData[3],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,"]}");    
	} else if ((ptr->notifyData[2] == 6) && (ptr->DEV_TYP > 1) && ( ptr->DEV_TYP < 10 ) && (retc == 20)) {
	ptr->bHtemp = ptr->notifyData[5];
	ptr->bState = ptr->notifyData[11];
	ptr->bProg = ptr->notifyData[3];
	if ((ptr->bState == 4) || (ptr->bProg && (ptr->bProg != 2))) ptr->bState = 0;
	if (ptr->DEV_TYP > 3) {
	ptr->bCtemp = ptr->notifyData[8];
	ptr->bHeat = ptr->notifyData[11];
	if ((ptr->bHeat == 4) || ((ptr->bProg != 1) && (ptr->bProg != 2))) ptr->bHeat = 0;
	ptr->bStNl = ptr->notifyData[11];
	if ((ptr->bStNl == 4) || (ptr->bProg != 3)) ptr->bStNl = 0;
	ptr->bStBp = ptr->notifyData[7];
	ptr->bBlTime = ptr->notifyData[16] + 128;
	} else {
	ptr->bCtemp = ptr->notifyData[13];
//	ptr->bHeat = ptr->notifyData[10];
	ptr->bHeat = ptr->notifyData[3];
	ptr->bStNl = 0;
	ptr->bStBp = 0;
	ptr->bBlTime = 0;
	}
	if (!ptr->bCtemp) ptr->bCVol = 254;
	if ((ptr->bCVol == 252) && ptr->bState) ptr->bCVol = 253;
	if ((ptr->bCVol == 253) && !ptr->bState) ptr->bCVol = 254;
	if (!ptr->bCStemp && ptr->bState) ptr->bCStemp = ptr->bCtemp + 3;
	if (ptr->bCStemp && !ptr->bState) ptr->bCStemp = 0;
	strcpy(ptr->cStatus,"{\"temp\":");
	(ptr->DEV_TYP > 3)? itoa(ptr->notifyData[8],tmpvar,10) : itoa(ptr->notifyData[13],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"target\":");
	itoa(ptr->notifyData[5],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"state\":");
	itoa(ptr->notifyData[11],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"prog\":");
	itoa(ptr->notifyData[3],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"beep\":");
	itoa(ptr->bStBp,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"error\":");
	itoa(ptr->notifyData[12],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	if (ptr->DEV_TYP > 3) {
	strcat(ptr->cStatus,",\"boil\":");
	itoa(ptr->bBlTime,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"volume\":[");
	if (ptr->bCVol < 250) {
	itoa(ptr->bCVol / 10,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,".");	
	itoa(ptr->bCVol % 10,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	} else if (ptr->bCVol == 253) strcat(ptr->cStatus,"\"???\"");
	else if (ptr->bC1temp && ptr->bS1Energy) strcat(ptr->cStatus,"\"??\"");
	else strcat(ptr->cStatus,"\"?\"");
	}
	strcat(ptr->cStatus,",");
	itoa(ptr->bCVoll / 10,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,".");	
	itoa(ptr->bCVoll % 10,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,"],\"rgb\":[");
	itoa(ptr->RgbR,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",");    
	itoa(ptr->RgbG,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",");    
	itoa(ptr->RgbB,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,"]}");    
	if (ptr->DEV_TYP > 3) {
	uint8_t data[] = {0xc8};
	if (r4sCommand(blenum, 0x35, data, sizeof(data)) == 9) {
	ptr->bStBl = ptr->notifyData[5];
        if (!ptr->r4slpcom && !BleDevStB.r4slpcom && !BleDevStC.r4slpcom && !ptr->r4slpres) {
	data[0] = 0x00;
	if (r4sCommand(blenum, 0x47, data, sizeof(data)) == 20) {
	ptr->bSEnergy = (ptr->notifyData[12] << 24) + (ptr->notifyData[11] << 16) + (ptr->notifyData[10] << 8) + ptr->notifyData[9];
	ptr->bSTime = (ptr->notifyData[8] << 24) + (ptr->notifyData[7] << 16) + (ptr->notifyData[6] << 8) + ptr->notifyData[5];
        if (ptr->bState) {
	if (((ptr->bCtemp < 71) || (ptr->bCtemp > 95)) && (ptr->bC1temp || ptr->bS1Energy)) { 
        ptr->bS1Energy = 0;
        ptr->bC1temp = 0;
	} else if ((ptr->bCtemp > 70) && (ptr->bCtemp > ptr->bCStemp) && (ptr->bCtemp < 80) && !ptr->bC1temp && !ptr->bS1Energy) { 
	ptr->bS1Energy = ptr->bSEnergy;
	ptr->bC1temp = ptr->bCtemp;
        if (ptr->bCVol < 250) ptr->bCVol = 254;
	} else if ((ptr->bCtemp > 90) && ptr->bC1temp && ptr->bS1Energy) {
	ptr->bS1Energy = ptr->bSEnergy - ptr->bS1Energy;
	ptr->bC1temp = ptr->bCtemp - ptr->bC1temp;
	if (ptr->bCVol == 253) {     //1l calibration
        ptr->bCVol = (4200 * ptr->bC1temp) / (ptr->bS1Energy * 36);
	if ((ptr->bCVol > 50) && (ptr->bCVol < 100)) ptr->bEfficiency = ptr->bCVol;
	ptr->bCVol = 10;
	} else {
	ptr->bCVol = (ptr->bS1Energy * 36 * ptr->bEfficiency) / (420 * ptr->bC1temp); //if n=0.8 then L*10 = 10*(En*3600*0.8)/(4200*dT); 	
        ptr->bCVoll = ptr->bCVol;
	}
        ptr->bS1Energy = 0;
        ptr->bC1temp = 0;
	}
	} else if (ptr->bC1temp || ptr->bS1Energy) {
        ptr->bS1Energy = 0;
        ptr->bC1temp = 0;
	}
	if (r4sCommand(blenum, 0x50, data, sizeof(data)) == 20) {
	ptr->bSCount = (ptr->notifyData[9] << 24) + (ptr->notifyData[8] << 16) + (ptr->notifyData[7] << 8) + ptr->notifyData[6];
				}
			}
		}
	}

	}
	} else if ((ptr->notifyData[2] == 6) && ( ptr->DEV_TYP > 9) && ( ptr->DEV_TYP < 12 ) && (retc == 20)) {
	ptr->bHeat = 0;
	ptr->bState = ptr->notifyData[11];
	if (ptr->DEV_TYP == 11) ptr->bHtemp = ptr->notifyData[5];
	else ptr->bHtemp = 0;
	ptr->bLock = ptr->notifyData[10];                   //lock
	strcpy(ptr->cStatus,"{\"state\":");
	itoa(ptr->notifyData[11],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"lock\":");
	itoa(ptr->notifyData[10],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"prog\":");
	itoa(ptr->notifyData[3],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	if (ptr->DEV_TYP == 11) {
	strcat(ptr->cStatus,",\"power\":");
	itoa(ptr->notifyData[5],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	}
	strcat(ptr->cStatus,",\"error\":");
	itoa(ptr->notifyData[12],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,"}");    
	} else if ((ptr->notifyData[2] == 6) && ( ptr->DEV_TYP > 11 ) && ( ptr->DEV_TYP < 16 ) && (retc == 20)) {
	ptr->bHeat = 0;
	ptr->bState = ptr->notifyData[11];
	ptr->bLock = ptr->notifyData[16];                   //lock
	ptr->bProg = ptr->notifyData[14];                   //strength of coffee
	ptr->bPHour = ptr->notifyData[6];
	ptr->bPMin = ptr->notifyData[7];
	ptr->bCHour = ptr->notifyData[8];                      //curr time
	ptr->bCMin = ptr->notifyData[9];
	if (ptr->bState == 5) ptr->bState = 0;
	if (ptr->notifyData[11] == 5) ptr->bStNl = 1;
	else ptr->bStNl = 0;
	strcpy(ptr->cStatus,"{\"state\":");
	itoa(ptr->notifyData[11],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"strength\":");
	itoa(ptr->notifyData[14],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"lock\":");
	itoa(ptr->notifyData[16],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"sethours\":");
	itoa(ptr->notifyData[6],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"setmins\":");
	itoa(ptr->notifyData[7],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"hours\":");
	itoa(ptr->notifyData[8],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"mins\":");
	itoa(ptr->notifyData[9],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"prog\":");
	itoa(ptr->notifyData[3],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"error\":");
	itoa(ptr->notifyData[12],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,"}");    
	} else if ((ptr->notifyData[2] == 6) && ( ptr->DEV_TYP == 16 ) && (retc == 13)) {
	ptr->bHeat = 0;
	ptr->bProg = ptr->notifyData[3];
	ptr->bModProg = ptr->notifyData[4];
	ptr->bHtemp = ptr->notifyData[5];
	ptr->bPHour = ptr->notifyData[6];
	ptr->bPMin = ptr->notifyData[7];
	ptr->bCHour = ptr->notifyData[8];
	ptr->bCMin = ptr->notifyData[9];
	ptr->bAwarm = ptr->notifyData[10];
	ptr->bState = ptr->notifyData[11];
	if (!ptr->bState) ptr->bProg = 255; 
	strcpy(ptr->cStatus,"{\"prog\":");
	itoa(ptr->bProg,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"mode\":");
	itoa(ptr->notifyData[4],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"temp\":");
	itoa(ptr->notifyData[5],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"sethours\":");
	itoa(ptr->notifyData[6],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"setmins\":");
	itoa(ptr->notifyData[7],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"state\":");
	itoa(ptr->notifyData[11],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"hours\":");
	itoa(ptr->notifyData[8],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"mins\":");
	itoa(ptr->notifyData[9],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"warm\":");
	itoa(ptr->notifyData[10],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,"}");    
	} else if ((ptr->notifyData[2] == 6) && ( ptr->DEV_TYP > 16 ) && ( ptr->DEV_TYP < 24 ) && (retc == 20)) {
	ptr->bHeat = 0;
	ptr->bProg = ptr->notifyData[3];
	ptr->bModProg = ptr->notifyData[4];
	ptr->bHtemp = ptr->notifyData[5];
	ptr->bPHour = ptr->notifyData[6];
	ptr->bPMin = ptr->notifyData[7];
	ptr->bCHour = ptr->notifyData[8];
	ptr->bCMin = ptr->notifyData[9];
	ptr->bAwarm = ptr->notifyData[10];
	ptr->bState = ptr->notifyData[11];
	if (!ptr->bState) ptr->bProg = 255; 
	strcpy(ptr->cStatus,"{\"prog\":");
	itoa(ptr->bProg,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"mode\":");
	itoa(ptr->notifyData[4],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"temp\":");
	itoa(ptr->notifyData[5],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"sethours\":");
	itoa(ptr->notifyData[6],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"setmins\":");
	itoa(ptr->notifyData[7],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"state\":");
	itoa(ptr->notifyData[11],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"hours\":");
	itoa(ptr->notifyData[8],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"mins\":");
	itoa(ptr->notifyData[9],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"warm\":");
	itoa(ptr->notifyData[10],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,"}");    
	} else if ((ptr->notifyData[2] == 6) && ( ptr->DEV_TYP == 24 ) && (retc == 20)) {
	ptr->bHeat = 0;
	ptr->bProg = ptr->notifyData[3];
	ptr->bModProg = ptr->notifyData[4];
	ptr->bHtemp = ptr->notifyData[5];
	ptr->bPHour = 0;
	ptr->bPMin = 0;
	ptr->bDHour = 0;
	ptr->bDMin = 0;
	ptr->bCHour = ptr->notifyData[9];
	ptr->bCMin = ptr->notifyData[10];
	ptr->bState = ptr->notifyData[11];
	ptr->bAwarm = ptr->notifyData[12];
	ptr->bLock = ptr->notifyData[16];                   //lock
	if (!ptr->bState) ptr->bProg = 255; 
        retc = r4sCommand(blenum, 0x10, 0, 0);	
	if (retc == 6) {
	ptr->bPHour = ptr->notifyData[3];
	ptr->bPMin = ptr->notifyData[4];
        retc = r4sCommand(blenum, 0x15, 0, 0);	
	if (retc == 6) {
	ptr->bDHour = ptr->notifyData[3];
	ptr->bDMin = ptr->notifyData[4];
	}
	}
	strcpy(ptr->cStatus,"{\"prog\":");
	itoa(ptr->bProg,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"mode\":");
	itoa(ptr->bModProg,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"temp\":");
	itoa(ptr->bHtemp,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"sethours\":");
	itoa(ptr->bPHour,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"setmins\":");
	itoa(ptr->bPMin,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"state\":");
	itoa(ptr->bState,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"hours\":");
	itoa(ptr->bCHour,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"mins\":");
	itoa(ptr->bCMin,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"warm\":");
	itoa(ptr->bAwarm,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"lock\":");
	itoa(ptr->bLock,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,"}");    
	} else if ((ptr->notifyData[2] == 6) && (ptr->notifyData[7] || ptr->notifyData[8]) && ( ptr->DEV_TYP == 63 ) && (retc == 20)) {
	uint8_t s = ptr->notifyData[5] & 0x08;
	uint8_t i = ((ptr->notifyData[5] >> 4) + 1) & 0x0f;
	if (i < 6) {
	ptr->bSEnergy = (ptr->notifyData[4] + (ptr->notifyData[5] << 8)) & 0x0fff; //temp C
	if (s) ptr->bSEnergy =  (ptr->bSEnergy ^ 0x0fff) + 1;
	while (i) {
        ptr->bSEnergy = ptr->bSEnergy * 10;
	i--;
	}
	if (s) ptr->bSEnergy =  ptr->bSEnergy | 0x80000000;
	} else if (i > 11) {
	ptr->bSEnergy = (ptr->notifyData[4] + (ptr->notifyData[5] << 8)) & 0x0fff; //temp C
	if (s) ptr->bSEnergy =  (ptr->bSEnergy ^ 0x0fff) + 1;
	while (i) {
        ptr->bSEnergy = ptr->bSEnergy / 10;
	i++;
	i = i & 0x0f;
	}
	if (s) ptr->bSEnergy =  ptr->bSEnergy | 0x80000000;
	}

	i = ((ptr->notifyData[10] >> 4) + 1) & 0x0f;
	if (i < 6) {
	ptr->bSHum = (ptr->notifyData[9] + (ptr->notifyData[10] << 8)) & 0x0fff;  //humidity
	while (i) {
        ptr->bSHum = ptr->bSHum * 10;
	i--;
	}
	} else if (i > 11) {
	ptr->bSHum = (ptr->notifyData[9] + (ptr->notifyData[10] << 8)) & 0x0fff;  //humidity
	while (i) {
        ptr->bSHum = ptr->bSHum / 10;
	i++;
	i = i & 0x0f;
	}
	}
	i = (ptr->notifyData[8] >> 4) & 0x0f;
	if (i < 5) {
	ptr->bSTime = (ptr->notifyData[7] + (ptr->notifyData[8] << 8)) & 0x0fff;  //pressure pa
	while (i) {
        ptr->bSTime = ptr->bSTime * 10;
	i--;
	}
	}
	i = (ptr->notifyData[14] >> 4) & 0x0f;
	if (i < 5) {
	ptr->bSCount = (ptr->notifyData[13] + (ptr->notifyData[14] << 8)) & 0x0fff;    //quality ppb
	while (i) {
        ptr->bSCount = ptr->bSCount * 10;
	i--;
	}
	}
	strcpy(ptr->cStatus,"{\"temperature\":");
	if (ptr->bSEnergy & 0x80000000) strcat(ptr->cStatus,"-");
	itoa((ptr->bSEnergy & 0x7fffffff) / 10,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,".");
	itoa((ptr->bSEnergy & 0x7fffffff) % 10,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"humidity\":");
	itoa(ptr->bSHum / 10,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,".");
	itoa(ptr->bSHum % 10,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"pressure\":");
	itoa(ptr->bSTime / 100,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"quality\":");
	itoa(ptr->bSCount,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,"}");    
	} else if (ptr->r4sConnErr > 2) {
	ptr->cStatus[0]=0;
	if (FDHass || !foffln) {
	ptr->bState = 0;
	ptr->bStNl = 0;
	ptr->bStBl = 0;
	ptr->bStBp = 0;
	ptr->bAwarm = 0;
	ptr->bHeat =  0;
	} else {
	ptr->bState = 254;
	ptr->bStNl = 254;
	ptr->bStBl = 254;
	ptr->bStBp = 254;
	ptr->bAwarm = 254;
	ptr->bHeat =  254;
	}
	ptr->bCtemp = 0;
	ptr->bHtemp = 0;
	ptr->bProg = 0;
	ptr->bModProg = 0;
	ptr->bLock = 0;
	ptr->bPHour = 0;
	ptr->bPMin = 0;
	ptr->bCHour = 0;
	ptr->bCMin = 0;
	ptr->bDHour = 0;
	ptr->bDMin = 0;
	ptr->bCVol = 254;
	}
	} else if (ptr->btauthoriz && (ptr->DEV_TYP > 63)) {
	ptr->sendDataLen = 0;
//	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_A_APP_ID].remote_bda);
	esp_ble_gap_read_rssi(gl_profile_tab[blenum].remote_bda);
	if (ptr->notifyDataLen == 12) {
        ESP_LOGI(AP_TAG, "Notify Data %d: ", blenum);
        esp_log_buffer_hex(AP_TAG, ptr->notifyData, ptr->notifyDataLen);
	ptr->bHtemp = ptr->notifyData[4];
	if ((ptr->notifyData[0] == 1) || (ptr->notifyData[1] == 1)) ptr->bState = 1;
	else ptr->bState = 0;
	if ((ptr->notifyData[1] != 2) || (ptr->xshedcom == 2)) ptr->bHtemp = 0;
	if (ptr->bHtemp) ptr->bHeat = 1;
	else ptr->bHeat = 0;
        ptr->bModProg = ptr->notifyData[1];
        ptr->bProg = ptr->notifyData[0];
	ptr->bCtemp = ptr->notifyData[5];
	ptr->bStNl = 0;
	strcpy(ptr->cStatus,"{\"temp\":");
	itoa(ptr->notifyData[5],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"target\":");
	itoa(ptr->bHtemp,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"heat\":");
	if (ptr->notifyData[1] == 2) strcat(ptr->cStatus,"1");
	else strcat(ptr->cStatus,"0");
	strcat(ptr->cStatus,",\"state\":");
	itoa(ptr->bState,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"action\":");
	itoa(ptr->notifyData[0],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"mode\":");
	itoa(ptr->notifyData[1],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"id\":");
	itoa(ptr->MiKettleID,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"warm_lock\":");
	itoa(ptr->notifyData[2],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"warm_min\":");
	tmpint = ptr->notifyData[7] + (ptr->notifyData[8]<<8 & 0xff00);
	itoa(tmpint,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,"}");
	if (!ptr->xshedcom && (ptr->notifyData[0] == 0x02) && (ptr->notifyData[1] == 0x02) && (ptr->notifyData[6] == 0x00)) mMiOfft(blenum);
	else if (tmpint > 255) mMiRewarm(blenum);
	else if (!ptr->xshedcom && (ptr->notifyData[10] != 0x18)) mMiSWtime(blenum);
//	else if (!ptr->xshedcom && (ptr->notifyData[0] == 0x03) && ((ptr->notifyData[10] != 0x18) || !ptr->notifyData[2])) mMiSWtime(blenum);
	else if (ptr->xshedcom == 2) {
	if (ptr->notifyData[1] == 0xff) {
	ptr->xshedcom = 0;
        mMiIdlTmp(blenum, 40);
	mMiSWtime(blenum);
	} else mMiHeat(blenum, 0);
        }
	ptr->notifyDataLen = -1;
	} else {
	ptr->r4sConnErr++;
	if (ptr->r4sConnErr > 2) { 
	ptr->cStatus[0]=0;
	if (FDHass || !foffln) {
	ptr->bState = 0;
	ptr->bHeat =  0;
	} else {
	ptr->bState = 254;
	ptr->bHeat =  254;
	}
	ptr->bCtemp = 0;
	ptr->bHtemp = 0;
	}
	}

	} else {
	ptr->cStatus[0]=0;
	if (FDHass || !foffln) {
	ptr->bState = 0;
	ptr->bStNl = 0;
	ptr->bStBl = 0;
	ptr->bStBp = 0;
	ptr->bAwarm = 0;
	ptr->bHeat =  0;
	} else {
	ptr->bState = 254;
	ptr->bStNl = 254;
	ptr->bStBl = 254;
	ptr->bStBp = 254;
	ptr->bAwarm = 254;
	ptr->bHeat =  254;
	}
	ptr->bCtemp = 0;
	ptr->bHtemp = 0;
	ptr->bProg = 0;
	ptr->bModProg = 0;
	ptr->bPHour = 0;
	ptr->bPMin = 0;
	ptr->bCHour = 0;
	ptr->bCMin = 0;
	ptr->bDHour = 0;
	ptr->bDMin = 0;
	ptr->bCVol = 254;
	}
}


void MqSState() {
	if (mqtdel) return;
	int64_t t_mqt_us = esp_timer_get_time();
	char ldata[128];
	char tmpvar[64]; 
	char tmpvar1[32]; 

	for (int i = 0; i < BleMonNum; i++) {
	if  ((mqttConnected) && BleMR[i].sto && (BleMX[i].state != BleMX[i].prstate)) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
        if (BleMR[i].id == 2) bin2hex(BleMR[i].mac,tmpvar,16,0); 
        else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(ldata,tmpvar);
	strcat(ldata,"/state");
	if (!BleMX[i].state) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	BleMX[i].prstate = BleMX[i].state;
	}
	if  ((mqttConnected) && BleMR[i].sto && (BleMX[i].rssi != BleMX[i].prrssi)) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
        if (BleMR[i].id == 2) bin2hex(BleMR[i].mac,tmpvar,16,0); 
        else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(ldata,tmpvar);
	strcat(ldata,"/rssi");
        itoa(BleMX[i].rssi,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	BleMX[i].prrssi = BleMX[i].rssi;
	}
        if (BleMR[i].id == 3) {
	if  ((mqttConnected) && BleMR[i].sto && (BleMX[i].par1 != BleMX[i].ppar1)) {
	uint16_t var1 =  BleMX[i].par1;
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
        bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(ldata,tmpvar);
	strcat(ldata,"/temperature");
	tmpvar[0] = 0;
	if (var1 & 0x8000) {
	var1 = (var1  ^ 0x0ffff) + 1;
	strcat(tmpvar,"-");
	}
        itoa(var1 / 100,tmpvar1,10);
	strcat(tmpvar,tmpvar1);
	var1 = var1 % 100;
	strcat(tmpvar,".");
	if (var1 < 10) strcat (tmpvar,"0");
        itoa(var1,tmpvar1,10);
	strcat(tmpvar,tmpvar1);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	BleMX[i].ppar1 = BleMX[i].par1;
	}
	if  ((mqttConnected) && BleMR[i].sto && (BleMX[i].par2 != BleMX[i].ppar2)) {
	uint16_t var1 =  BleMX[i].par2;
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
        bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(ldata,tmpvar);
	strcat(ldata,"/humidity");
        itoa(var1 / 100,tmpvar1,10);
	strcpy(tmpvar,tmpvar1);
	var1 = var1 % 100;
	strcat(tmpvar,".");
	if (var1 < 10) strcat (tmpvar,"0");
        itoa(var1,tmpvar1,10);
	strcat(tmpvar,tmpvar1);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	BleMX[i].ppar2 = BleMX[i].par2;
	}
	if  ((mqttConnected) && BleMR[i].sto && (BleMX[i].par4 != BleMX[i].ppar4)) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
        bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(ldata,tmpvar);
	strcat(ldata,"/battery");
        itoa(BleMX[i].par4,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	BleMX[i].ppar4 = BleMX[i].par4;
	}

	}
	}

	if  ((mqttConnected) && (bStateS != bprevStateS)) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/screen");
       	itoa(bStateS,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
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

void MqState(uint8_t blenum) {
	if (mqtdel) return;
	if (blenum > 2) return;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	int64_t t_mqt_us = esp_timer_get_time();
	char ldata[64];
	char tmpvar[32]; 
	char tmpvar1[16]; 
	wifi_ap_record_t wifidata;
	if (!blenum && mqttConnected && (esp_wifi_sta_get_ap_info(&wifidata)==0)){
	iRssiESP = wifidata.rssi;
	if  (iprevRssiESP != iRssiESP) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/rssi");
        itoa(iRssiESP,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	iprevRssiESP = iRssiESP;
	}

	}
	msStatus(blenum);
	if ((mqttConnected) && (ptr->tBLEAddr[0]) && (ptr->DEV_TYP)) {
	if ((ptr->cStatus[0] != 0) && (strcmp(ptr->cStatus,ptr->cprevStatus) != 0)) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/json");
	esp_mqtt_client_publish(mqttclient, ldata, ptr->cStatus, 0, 1, 1);
	strcpy(ptr->cprevStatus,ptr->cStatus);
	} 
	if  (ptr->iprevRssi != ptr->iRssi) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	strcat(ldata,"/rssi");
        itoa(ptr->iRssi,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->iprevRssi = ptr->iRssi;
	}
	if ((ptr->DEV_TYP < 10) || (ptr->DEV_TYP > 63)) {
	if  (ptr->bprevCtemp != ptr->bCtemp) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/temp");
	itoa(ptr->bCtemp,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->bprevCtemp = ptr->bCtemp;
	}
	if  (ptr->bprevHtemp != ptr->bHtemp) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/heat_temp");
	itoa(ptr->bHtemp,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->r4sppcom = 255;
	ptr->t_ppcom_us = t_mqt_us;
	if (ptr->bHtemp > 29) ptr->bLtemp = ptr->bHtemp;
	ptr->bprevHtemp = ptr->bHtemp;
	}
	if  (ptr->bprevState != ptr->bState) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/boil");
	if (!ptr->bState) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (ptr->bState == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	ptr->r4sppcom = 255;
	ptr->t_ppcom_us = t_mqt_us;
	ptr->bprevState = ptr->bState;
	}
	if  (ptr->bprevHeat != ptr->bHeat) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/heat");
	if (!ptr->bHeat) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (ptr->bHeat == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/hstate");
	if (!ptr->bHeat) esp_mqtt_client_publish(mqttclient, ldata, "off", 0, 1, 1);
        else if (ptr->bHeat == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, "heat", 0, 1, 1);
	ptr->r4sppcom = 255;
	ptr->t_ppcom_us = t_mqt_us;
	ptr->bprevHeat = ptr->bHeat;
	}
	if (ptr->DEV_TYP < 10) {
	if  (ptr->bprevStNl != ptr->bStNl) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/nightlight");
	if (!ptr->bStNl) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (ptr->bState == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	ptr->r4sppcom = 255;
	ptr->t_ppcom_us = t_mqt_us;
	ptr->bprevStNl = ptr->bStNl;
	}
	if  ((ptr->PRgbR != ptr->RgbR) || (ptr->PRgbG != ptr->RgbG) || (ptr->PRgbB != ptr->RgbB)) {
	ldata[0] = 0;
        itoa(ptr->RgbR,ldata,10);
	strcpy(tmpvar,ldata);
        itoa(ptr->RgbG,ldata,10);
	strcat(tmpvar,",");
	strcat(tmpvar,ldata);
        itoa(ptr->RgbB,ldata,10);
	strcat(tmpvar,",");
	strcat(tmpvar,ldata);
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/nightlight_rgb");
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->r4sppcom = 255;
	ptr->t_ppcom_us = t_mqt_us;
//	ptr->PRgbR = ptr->RgbR;
//	ptr->PRgbG = ptr->RgbG;
//	ptr->PRgbB = ptr->RgbB;
	}
	if  (ptr->PRgbR != ptr->RgbR) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/nightlight_red");
        itoa(ptr->RgbR,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->r4sppcom = 255;
	ptr->t_ppcom_us = t_mqt_us;
	ptr->PRgbR = ptr->RgbR;
	}
	if  (ptr->PRgbG != ptr->RgbG) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/nightlight_green");
        itoa(ptr->RgbG,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->r4sppcom = 255;
	ptr->t_ppcom_us = t_mqt_us;
	ptr->PRgbG = ptr->RgbG;
	}
	if  (ptr->PRgbB != ptr->RgbB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/nightlight_blue");
        itoa(ptr->RgbB,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->r4sppcom = 255;
	ptr->t_ppcom_us = t_mqt_us;
	ptr->PRgbB = ptr->RgbB;
	}
	if  (ptr->bprevStBl != ptr->bStBl) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/backlight");
	if (!ptr->bStBl) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (ptr->bStBl == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	ptr->r4sppcom = 255;
	ptr->t_ppcom_us = t_mqt_us;
	ptr->bprevStBl = ptr->bStBl;
	}
	if  (ptr->bprevStBp != ptr->bStBp) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/beep");
	if (!ptr->bStBp) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (ptr->bStBp == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	ptr->r4sppcom = 255;
	ptr->t_ppcom_us = t_mqt_us;
	ptr->bprevStBp = ptr->bStBp;
	}
	if  (ptr->bSEnergy != ptr->bprevSEnergy) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/total_energy");
        itoakw(ptr->bSEnergy,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->r4sppcom = 255;
	ptr->t_ppcom_us = t_mqt_us;
	ptr->bprevSEnergy = ptr->bSEnergy;
	}
	if  (ptr->bSTime != ptr->bprevSTime) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/working_time");
        itoat(ptr->bSTime,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->r4sppcom = 255;
	ptr->t_ppcom_us = t_mqt_us;
	ptr->bprevSTime = ptr->bSTime;
	}
	if  (ptr->bSCount != ptr->bprevSCount) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/working_count");
        itoa(ptr->bSCount,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->r4sppcom = 255;
	ptr->t_ppcom_us = t_mqt_us;
	ptr->bprevSCount = ptr->bSCount;
	}
	if  (ptr->bCVol != ptr->bprevCVol) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/volume");
	if (ptr->bCVol < 250) {
	itoa(ptr->bCVol / 10,tmpvar1,10);
	strcpy(tmpvar,tmpvar1);
	strcat(tmpvar,".");	
	itoa(ptr->bCVol % 10,tmpvar1,10);
	strcat(tmpvar,tmpvar1);
	} else strcpy(tmpvar,"?");
	if (ptr->DEV_TYP > 3) esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->r4sppcom = 255;
	ptr->t_ppcom_us = t_mqt_us;
	ptr->bprevCVol = ptr->bCVol;
	}
	if  (ptr->bCVoll != ptr->bprevCVoll) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/volume_last");
	itoa(ptr->bCVoll / 10,tmpvar1,10);
	strcpy(tmpvar,tmpvar1);
	strcat(tmpvar,".");	
	itoa(ptr->bCVoll % 10,tmpvar1,10);
	strcat(tmpvar,tmpvar1);
	if (ptr->DEV_TYP > 3) esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->r4sppcom = 255;
	ptr->t_ppcom_us = t_mqt_us;
	ptr->bprevCVoll = ptr->bCVoll;
	}
	if  (ptr->bBlTime != ptr->bprevBlTime) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/boil_time");
	itoa(ptr->bBlTime,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->r4sppcom = 255;
	ptr->t_ppcom_us = t_mqt_us;
	ptr->bprevBlTime = ptr->bBlTime;
	}
	}
	} else if ( ptr->DEV_TYP < 12) {
	if  (ptr->bprevState != ptr->bState) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/state");
	if (!ptr->bState) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (ptr->bState == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
/*
	if (ptr->DEV_TYP == 11) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/hstate");
	if (!ptr->bHeat) esp_mqtt_client_publish(mqttclient, ldata, "off", 0, 1, 1);
        else if (ptr->bHeat == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, "heat", 0, 1, 1);
	}
*/
	ptr->r4sppcom = 255;
	ptr->t_ppcom_us = t_mqt_us;
	ptr->bprevState = ptr->bState;
	}
	if  (ptr->bprevLock != ptr->bLock) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/lock");
	if (!ptr->bLock) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	ptr->r4sppcom = 255;
	ptr->t_ppcom_us = t_mqt_us;
	ptr->bprevLock = ptr->bLock;
	}
	if  ((ptr->DEV_TYP == 11) && (ptr->bprevHtemp != ptr->bHtemp)) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/power");
	itoa(ptr->bHtemp,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->r4sppcom = 255;
	ptr->t_ppcom_us = t_mqt_us;
	if (ptr->bHtemp > 9) ptr->bLtemp = ptr->bHtemp;
	ptr->bprevHtemp = ptr->bHtemp;
	}
	} else if ( ptr->DEV_TYP < 16) {
	if  (ptr->bprevState != ptr->bState) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/state");
	if (!ptr->bState) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (ptr->bState == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	ptr->r4sppcom = 255;
	ptr->t_ppcom_us = t_mqt_us;
	ptr->bprevState = ptr->bState;
	}
	if  (ptr->bprevStNl != ptr->bStNl) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/delay");
	if (!ptr->bStNl) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (ptr->bStNl == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	ptr->r4sppcom = 255;
	ptr->t_ppcom_us = t_mqt_us;
	ptr->bprevStNl = ptr->bStNl;
	}
	if  (ptr->bprevLock != ptr->bLock) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/lock");
	if (!ptr->bLock) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	ptr->r4sppcom = 255;
	ptr->t_ppcom_us = t_mqt_us;
	ptr->bprevLock = ptr->bLock;
	}
	if  (ptr->bprevProg != ptr->bProg) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/strength");
	if (!ptr->bProg) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	ptr->r4sppcom = 255;
	ptr->t_ppcom_us = t_mqt_us;
	ptr->bprevProg = ptr->bProg;
	}
	if  (ptr->bprevPHour != ptr->bPHour) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/set_hour");
	itoa(ptr->bPHour,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->bprevPHour = ptr->bPHour;
	}
	if  (ptr->bprevPMin != ptr->bPMin) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/set_min");
	itoa(ptr->bPMin,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->bprevPMin = ptr->bPMin;
	}
	if  (ptr->bprevDHour != ptr->bDHour) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/delay_hour");
	itoa(ptr->bDHour,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->bprevDHour = ptr->bDHour;
	}
	if  (ptr->bprevDMin != ptr->bDMin) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/delay_min");
	itoa(ptr->bDMin,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->bprevDMin = ptr->bDMin;
	}
	if  (ptr->bprevCHour != ptr->bCHour) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/hour");
	itoa(ptr->bCHour,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->bprevCHour = ptr->bCHour;
	}
	if  (ptr->bprevCMin != ptr->bCMin) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/min");
	itoa(ptr->bCMin,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->bprevCMin = ptr->bCMin;
	}
	} else if (ptr->DEV_TYP < 63) {
	if  (ptr->bprevState != ptr->bState) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/state");
	if (ptr->DEV_TYP == 20) {
	if (!ptr->bState) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (ptr->bState == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else if (ptr->bState == 1) esp_mqtt_client_publish(mqttclient, ldata, "SETTING", 0, 1, 1);
        else if (ptr->bState == 2) esp_mqtt_client_publish(mqttclient, ldata, "DELAYEDSTART", 0, 1, 1);
        else if (ptr->bState == 3) esp_mqtt_client_publish(mqttclient, ldata, "BOIL4PASTA", 0, 1, 1);
        else if (ptr->bState == 4) esp_mqtt_client_publish(mqttclient, ldata, "WAITPASTA", 0, 1, 1);
        else if (ptr->bState == 5) esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
        else if (ptr->bState == 6) esp_mqtt_client_publish(mqttclient, ldata, "WARMING", 0, 1, 1);
	} else {
	if (!ptr->bState) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (ptr->bState == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else if (ptr->bState == 1) esp_mqtt_client_publish(mqttclient, ldata, "SETTING", 0, 1, 1);
        else if (ptr->bState == 2) esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
        else if (ptr->bState == 3) esp_mqtt_client_publish(mqttclient, ldata, "WAITPASTA", 0, 1, 1);
        else if (ptr->bState == 4) esp_mqtt_client_publish(mqttclient, ldata, "WARMING", 0, 1, 1);
        else if (ptr->bState == 5) esp_mqtt_client_publish(mqttclient, ldata, "DELAYEDSTART", 0, 1, 1);
        else if (ptr->bState == 6) esp_mqtt_client_publish(mqttclient, ldata, "BOIL4PASTA", 0, 1, 1);
        }
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/hstate");
	if (ptr->bState < 2) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (ptr->bState == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	ptr->r4sppcom = 255;
	ptr->t_ppcom_us = t_mqt_us;
	ptr->bprevState = ptr->bState;
	}
	if  ((ptr->bprevProg != ptr->bProg) || (ptr->bprevModProg != ptr->bModProg)){
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/prname");
        if ( ptr->DEV_TYP == 16 ) {
// for RMC-800s
	switch (ptr->bProg) {
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
	if (ptr->bModProg == 1) esp_mqtt_client_publish(mqttclient, ldata, "Frying_vegetables", 0, 1, 1);
	else if (ptr->bModProg == 2) esp_mqtt_client_publish(mqttclient, ldata, "Frying_fish", 0, 1, 1);
	else if (ptr->bModProg == 3) esp_mqtt_client_publish(mqttclient, ldata, "Frying_meat", 0, 1, 1);
	else esp_mqtt_client_publish(mqttclient, ldata, "Invalid mode", 0, 1, 1);
	break;
	case 5:
	if (ptr->bModProg == 1) esp_mqtt_client_publish(mqttclient, ldata, "Stewing_vegetables", 0, 1, 1);
	else if (ptr->bModProg == 2) esp_mqtt_client_publish(mqttclient, ldata, "Stewing_fish", 0, 1, 1);
	else if (ptr->bModProg == 3) esp_mqtt_client_publish(mqttclient, ldata, "Stewing_meat", 0, 1, 1);
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
	if (ptr->bModProg == 1) esp_mqtt_client_publish(mqttclient, ldata, "Steam_vegetables", 0, 1, 1);
	else if (ptr->bModProg == 2) esp_mqtt_client_publish(mqttclient, ldata, "Steam_fish", 0, 1, 1);
	else if (ptr->bModProg == 3) esp_mqtt_client_publish(mqttclient, ldata, "Steam_meat", 0, 1, 1);
	else esp_mqtt_client_publish(mqttclient, ldata, "Invalid mode", 0, 1, 1);
	break;
	case 12:
	esp_mqtt_client_publish(mqttclient, ldata, "Hot", 0, 1, 1);
	break;
	}
	} else if ( ptr->DEV_TYP == 17 ) {
// for RMC-903s
	switch (ptr->bProg) {
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
	} else if ( ptr->DEV_TYP == 18 ) {
// for RMC-224s
	switch (ptr->bProg) {
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
	} else if ( ptr->DEV_TYP == 19 ) {
// for RMC-961s
	switch (BleDevStC.bProg) {
	case 255:
	esp_mqtt_client_publish(mqttclient, ldata, "OFF", 0, 1, 1);
	break;
	case 0:
	esp_mqtt_client_publish(mqttclient, ldata, "Groats", 0, 1, 1);
	break;
	case 1:
	esp_mqtt_client_publish(mqttclient, ldata, "Frying", 0, 1, 1);
	break;
	case 2:
	esp_mqtt_client_publish(mqttclient, ldata, "Steam", 0, 1, 1);
	break;
	case 3:
	esp_mqtt_client_publish(mqttclient, ldata, "Baking", 0, 1, 1);
	break;
	case 4:
	esp_mqtt_client_publish(mqttclient, ldata, "Stewing", 0, 1, 1);
	break;
	case 5:
	esp_mqtt_client_publish(mqttclient, ldata, "Multicooker", 0, 1, 1);
	break;
	case 6:
	esp_mqtt_client_publish(mqttclient, ldata, "Pilaf", 0, 1, 1);
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
	}
	} else if ( ptr->DEV_TYP == 20 ) {
// for RMC-92s
	switch (ptr->bProg) {
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
	case 17:
	esp_mqtt_client_publish(mqttclient, ldata, "Warming", 0, 1, 1);
	break;
	}
	} else if ( ptr->DEV_TYP == 24 ) {
// for RO-5707
	switch (ptr->bProg) {
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
	ptr->r4sppcom = 255;
	ptr->t_ppcom_us = t_mqt_us;
//	ptr->bprevProg = ptr->bProg;
//	ptr->bprevModProg = ptr->bModProg;
	}
	if  (ptr->bprevProg != ptr->bProg) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/prog");
	itoa(ptr->bProg,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->r4sppcom = 255;
	ptr->t_ppcom_us = t_mqt_us;
	ptr->bprevProg = ptr->bProg;
	}
	if  (ptr->bprevModProg != ptr->bModProg) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/mode");
	itoa(ptr->bModProg,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->r4sppcom = 255;
	ptr->t_ppcom_us = t_mqt_us;
	ptr->bprevModProg = ptr->bModProg;
	}
	if  (ptr->bprevHtemp != ptr->bHtemp) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/temp");
	itoa(ptr->bHtemp,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->r4sppcom = 255;
	ptr->t_ppcom_us = t_mqt_us;
	ptr->bprevHtemp = ptr->bHtemp;
	}
	if  (ptr->bprevPHour != ptr->bPHour) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/set_hour");
	itoa(ptr->bPHour,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->r4sppcom = 255;
	ptr->t_ppcom_us = t_mqt_us;
	ptr->bprevPHour = ptr->bPHour;
	}
	if  (ptr->bprevPMin != ptr->bPMin) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/set_min");
	itoa(ptr->bPMin,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->r4sppcom = 255;
	ptr->t_ppcom_us = t_mqt_us;
	ptr->bprevPMin = ptr->bPMin;
	}
	if  (ptr->bprevCHour != ptr->bCHour) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/hour");
	itoa(ptr->bCHour,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->bprevCHour = ptr->bCHour;
	}
	if  (ptr->bprevCMin != ptr->bCMin) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/min");
	itoa(ptr->bCMin,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->bprevCMin = ptr->bCMin;
	}
	if  (ptr->bprevDHour != ptr->bDHour) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/delay_hour");
	itoa(ptr->bDHour,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->bprevDHour = ptr->bDHour;
	}
	if  (ptr->bprevDMin != ptr->bDMin) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/delay_min");
	itoa(ptr->bDMin,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->bprevDMin = ptr->bDMin;
	}
	if  (ptr->bprevAwarm != ptr->bAwarm) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/warm");
	if (!ptr->bAwarm) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (ptr->bAwarm == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	ptr->r4sppcom = 255;
	ptr->t_ppcom_us = t_mqt_us;
	ptr->bprevAwarm = ptr->bAwarm;
	}

	} else if (ptr->DEV_TYP == 63) {
	if  (ptr->bprevState != ptr->bState) {
	if (ptr->r4slpcom != 63) ptr->bState = 0;
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/calibration");
	if (!ptr->bState) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
        else if (ptr->bState == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
        else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	ptr->r4sppcom = 255;
	ptr->t_ppcom_us = t_mqt_us;
	ptr->bprevState = ptr->bState;
	}
	if  (ptr->bSEnergy != ptr->bprevSEnergy) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/temperature");
        tmpvar[0] = 0;
	if (ptr->bSEnergy & 0x80000000) strcat(tmpvar,"-");
	itoa((ptr->bSEnergy & 0x7fffffff) / 10,tmpvar1,10);
	strcat(tmpvar,tmpvar1);
	strcat(tmpvar,".");	
	itoa((ptr->bSEnergy & 0x7fffffff) % 10,tmpvar1,10);
	strcat(tmpvar,tmpvar1);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->r4sppcom = 255;
	ptr->t_ppcom_us = t_mqt_us;
	ptr->bprevSEnergy = ptr->bSEnergy;
	}
	if  (ptr->bprevSHum != ptr->bSHum) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/humidity");
	itoa(ptr->bSHum / 10,tmpvar1,10);
	strcpy(tmpvar,tmpvar1);
	strcat(tmpvar,".");	
	itoa(ptr->bSHum % 10,tmpvar1,10);
	strcat(tmpvar,tmpvar1);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->bprevSHum = ptr->bSHum;
	}
	if  (ptr->bSTime != ptr->bprevSTime) {
	uint32_t tmp = 0;
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/pressure");
        itoa(ptr->bSTime / 100,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/pressurem");
	tmp = ptr->bSTime * 75 / 1000;
        itoa(tmp / 10,tmpvar1,10);
	strcpy(tmpvar,tmpvar1);
	strcat(tmpvar,".");
        itoa(tmp % 10,tmpvar1,10);
	strcat(tmpvar,tmpvar1);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->r4sppcom = 255;
	ptr->t_ppcom_us = t_mqt_us;
	ptr->bprevSTime = ptr->bSTime;
	}
	if  (ptr->bSCount != ptr->bprevSCount) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/quality");
        itoa(ptr->bSCount,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->r4sppcom = 255;
	ptr->t_ppcom_us = t_mqt_us;
	ptr->bprevSCount = ptr->bSCount;
	}

	}
}
}



void BleMqtPr(uint8_t blenum, int topoff, char *topic, int topic_len, char *data, int data_len) {
	if (blenum > 2) return;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	char tbuff[64];
	char ttopic[64];
	memset(ttopic,0,64);
        memcpy(ttopic, topic, topic_len);
	if (ptr->DEV_TYP < 10) {	
	//kettle
	if (!memcmp(topic+topoff, "boil", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",data,data_len)) || (!incascmp("on",data,data_len))
		|| (!incascmp("true",data,data_len))) {
	if ((!ptr->bState) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strON,data,data_len))) {
	if (ptr->bHtemp) {
	if (ptr->DEV_TYP == 1) {	
	if (ptr->bHtemp < 30) ptr->r4slppar1 = 0;
	else if (ptr->bHtemp < 41) ptr->r4slppar1 = 1;
	else if (ptr->bHtemp < 56) ptr->r4slppar1 = 2;
	else if (ptr->bHtemp < 71) ptr->r4slppar1 = 3;
	else if (ptr->bHtemp < 86) ptr->r4slppar1 = 4;
	else ptr->r4slppar1 = 5;
	} else ptr->r4slppar1 = ptr->bHtemp;
	ptr->r4slpcom = 4;
	} else {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 2;
		}
	}
//	ESP_LOGI(AP_TAG,"MQTT_BOIL_ON");
	} else if ((!incascmp("0",data,data_len)) || (!incascmp("off",data,data_len))
		|| (!incascmp("false",data,data_len))) {
	if ((ptr->bState)  || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strOFF,data,data_len))) {
	if (ptr->bHtemp) {
	if (ptr->DEV_TYP == 1) {	
	if (ptr->bHtemp < 30) ptr->r4slppar1 = 0;
	else if (ptr->bHtemp < 41) ptr->r4slppar1 = 1;
	else if (ptr->bHtemp < 56) ptr->r4slppar1 = 2;
	else if (ptr->bHtemp < 71) ptr->r4slppar1 = 3;
	else if (ptr->bHtemp < 86) ptr->r4slppar1 = 4;
	else ptr->r4slppar1 = 5;
	} else ptr->r4slppar1 = ptr->bHtemp;
	ptr->r4slppar2 = 1;
	ptr->r4slpcom = 3;
	} else {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 1;
		}
	}	
//	ESP_LOGI(AP_TAG,"MQTT_BOIL_OFF");
		} 

	} else if (!memcmp(topic+topoff, "heat", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",data,data_len)) || (!incascmp("on",data,data_len))
		|| (!incascmp("true",data,data_len)) || (!incascmp("heat",data,data_len))) {
	if ((!ptr->bHeat) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strON,data,data_len))) {
	uint8_t temp = ptr->bHtemp;
        if ((temp < 30) || (temp > 98)) temp = ptr->bLtemp;
        if ((temp < 30) || (temp > 98)) temp = 40;

	if (ptr->DEV_TYP == 1) {	
	if (temp < 30) ptr->r4slppar1 = 0;
	else if (temp < 41) ptr->r4slppar1 = 1;
	else if (temp < 56) ptr->r4slppar1 = 2;
	else if (temp < 71) ptr->r4slppar1 = 3;
	else if (temp < 86) ptr->r4slppar1 = 4;
	else ptr->r4slppar1 = 5;
	} else ptr->r4slppar1 = temp;
	ptr->r4slppar2 = 1;
	ptr->r4slpcom = 3;
	}
//	ESP_LOGI(AP_TAG,"MQTT_HEAT_ON");
	} else if ((!incascmp("0",data,data_len)) || (!incascmp("off",data,data_len))
		|| (!incascmp("false",data,data_len))) {
	if ((ptr->bHeat)  || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strOFF,data,data_len))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 1;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_HEAT_OFF");
		} 

	} else if (!memcmp(topic+topoff, "heat_temp", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, data, tempsz);
	uint8_t temp = atoi(tbuff);
	if ((!ptr->bHtemp) && (ptr->bLtemp) && (temp < 30)) temp = ptr->bLtemp;
	if  ((!fcommtp) || (!ptr->r4sppcom) || ((temp != ptr->bHtemp) && temp && ptr->bHtemp)) {
	if (ptr->DEV_TYP == 1) {	
	if (temp < 30) ptr->r4slppar1 = 0;
	else if (temp < 41) ptr->r4slppar1 = 1;
	else if (temp < 56) ptr->r4slppar1 = 2;
	else if (temp < 71) ptr->r4slppar1 = 3;
	else if (temp < 86) ptr->r4slppar1 = 4;
	else ptr->r4slppar1 = 5;
	} else ptr->r4slppar1 = temp;
	ptr->r4slppar2 = 1;
	ptr->r4slpcom = 3;
	} else if ((temp < 30) && ((!fcommtp) || (!ptr->r4sppcom) || (temp != ptr->bHtemp))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 1;
	} else if ((temp > 98) && ((!fcommtp) || (!ptr->r4sppcom) || (temp != ptr->bHtemp))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 1;
	}
//	ESP_LOGI(AP_TAG,"MQTT_HEAT_TEMP");

	} else if (!memcmp(topic+topoff, "backlight", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",data,data_len)) || (!incascmp("on",data,data_len))
		|| (!incascmp("true",data,data_len))) {
	if ((!ptr->bStBl) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strON,data,data_len))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 22;
	}
//	ESP_LOGI(AP_TAG,"MQTT_BACKLIGHT_ON");
	} else if ((!incascmp("0",data,data_len)) || (!incascmp("off",data,data_len))
		|| (!incascmp("false",data,data_len))) {
	if ((ptr->bStBl) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strOFF,data,data_len))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 21;
	}
//	ESP_LOGI(AP_TAG,"MQTT_BACKLIGHT_OFF");
	}

	} else if (!memcmp(topic+topoff, "beep", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",data,data_len)) || (!incascmp("on",data,data_len))
		|| (!incascmp("true",data,data_len))) {
	if ((!ptr->bStBp) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strON,data,data_len))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 24;
	}
//	ESP_LOGI(AP_TAG,"MQTT_BEEP_ON");
	} else if ((!incascmp("0",data,data_len)) || (!incascmp("off",data,data_len))
		|| (!incascmp("false",data,data_len))) {
	if ((ptr->bStBp) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strOFF,data,data_len))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 23;
	}
//	ESP_LOGI(AP_TAG,"MQTT_BEEP_OFF");
	}

	} else if (!memcmp(topic+topoff, "nightlight", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",data,data_len)) || (!incascmp("on",data,data_len))
		|| (!incascmp("true",data,data_len))) {
	if ((!ptr->bStNl) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strON,data,data_len))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 5;
	}
//	ESP_LOGI(AP_TAG,"MQTT_NIGHTLIGHT_ON");
	} else if ((!incascmp("0",data,data_len)) || (!incascmp("off",data,data_len))
		|| (!incascmp("false",data,data_len))) {
	if ((ptr->bStNl) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strOFF,data,data_len))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 1;
	}
//	ESP_LOGI(AP_TAG,"MQTT_NIGHTLIGHT_OFF");
	}

	} else if (!memcmp(topic+topoff, "nightlight_rgb", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, data, tempsz);
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
	if ((!fcommtp) || (!ptr->r4sppcom) || (ptr->RgbR != rval) || (ptr->RgbG != gval) || (ptr->RgbB != bval)) {
	ptr->RgbR = rval;
	ptr->RgbG = gval;
	ptr->RgbB = bval;
	if (ptr->bStNl) {	
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 5;
	}
	ptr->t_last_us = ~ptr->t_last_us;
	}
	}

	} else if (!memcmp(topic+topoff, "nightlight_red", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, data, tempsz);
	uint8_t cval = atoi(tbuff);
	if ((!fcommtp) || (!ptr->r4sppcom) || (ptr->RgbR != cval)) {
	ptr->RgbR = cval;
	if (ptr->bStNl) {	
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 5;
	}
	ptr->t_last_us = ~ptr->t_last_us;
	}
	} else if (!memcmp(topic+topoff, "nightlight_green", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, data, tempsz);
	uint8_t cval = atoi(tbuff);
	if ((!fcommtp) || (!ptr->r4sppcom) || (ptr->RgbG != cval)) {
	ptr->RgbG = cval;
	if (ptr->bStNl) {	
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 5;
	}
	ptr->t_last_us = ~ptr->t_last_us;
	}
	} else if (!memcmp(topic+topoff, "nightlight_blue", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, data, tempsz);
	uint8_t cval = atoi(tbuff);
	if ((!fcommtp) || (!ptr->r4sppcom) || (ptr->RgbB != cval)) {
	ptr->RgbB = cval;
	if (ptr->bStNl) {	
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 5;
	}
	ptr->t_last_us = ~ptr->t_last_us;
	}
	} else if (!memcmp(topic+topoff, "boil_time", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, data, tempsz);
	int8_t cval = atoi(tbuff);
	if ((!fcommtp) || (!ptr->r4sppcom) || (ptr->bBlTime != cval)) {
	if (cval > 5) cval = 5;
	else if (cval < -5) cval = -5;
	if (!ptr->bState && !ptr->bHeat) {	
	ptr->r4slppar1 = cval;
	ptr->r4slpcom = 25;
	} else ptr->bprevBlTime = 128;
	ptr->t_last_us = ~ptr->t_last_us;
	}
	}
	} else if ( ptr->DEV_TYP < 12) {
	//power
	if (!memcmp(topic+topoff, "state", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",data,data_len)) || (!incascmp("on",data,data_len))
		|| (!incascmp("true",data,data_len)) || (!incascmp("heat",data,data_len))) {
	if ((!ptr->bState) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strON,data,data_len))) {
	ptr->r4slppar2 = 0;
	ptr->r4slppar1 = 0;
	if ( ptr->DEV_TYP == 11) {
	uint8_t pwr = ptr->bHtemp;
        if (!pwr || (pwr > 100)) pwr = ptr->bLtemp;
        if (!pwr || (pwr > 100)) pwr = 100;
	ptr->r4slppar2 = pwr;
	}
	ptr->r4slpcom = 6;
	}
//	ESP_LOGI(AP_TAG,"MQTT_CMD_ON");
	} else if ((!incascmp("0",data,data_len)) || (!incascmp("off",data,data_len))
		|| (!incascmp("false",data,data_len))) {
	if ((ptr->bState) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strOFF,data,data_len))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 1;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_CMD_OFF");
	}

	} else if (!memcmp(topic+topoff, "power", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, data, tempsz);
	uint8_t pwr = atoi(tbuff);
	if  ((!fcommtp) || (!ptr->r4sppcom) || (pwr != ptr->bHtemp)) {
	if (pwr) {
	ptr->r4slppar2 = pwr;
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 6;
	} else {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 1;
	}
	}
//	ESP_LOGI(AP_TAG,"MQTT_POWER");

	} else 	if (!memcmp(topic+topoff, "lock", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",data,data_len)) || (!incascmp("on",data,data_len))
		|| (!incascmp("true",data,data_len))) {
	if ((!ptr->bLock) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strON,data,data_len))) {	
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 8;
	}
//	ESP_LOGI(AP_TAG,"MQTT_lock_ON");
	} else if ((!incascmp("0",data,data_len)) || (!incascmp("off",data,data_len))
		|| (!incascmp("false",data,data_len))) {
	if ((ptr->bLock) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strOFF,data,data_len))) {	
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 7;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_lock_OFF");
	}
	}
	} else if ( ptr->DEV_TYP < 16) {
	//coffee
	if (!memcmp(topic+topoff, "state", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",data,data_len)) || (!incascmp("on",data,data_len))
		|| (!incascmp("true",data,data_len))) {
	if ((!ptr->bState) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strON,data,data_len))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 6;
	}
//	ESP_LOGI(AP_TAG,"MQTT_CMD_ON");
	} else if ((!incascmp("0",data,data_len)) || (!incascmp("off",data,data_len))
		|| (!incascmp("false",data,data_len))) {
	if ((ptr->bState) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strOFF,data,data_len))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 1;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_CMD_OFF");
	}
	} else if (!memcmp(topic+topoff, "delay", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",data,data_len)) || (!incascmp("on",data,data_len))
		|| (!incascmp("true",data,data_len))) {
	if ((!ptr->bStNl) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strON,data,data_len))) {	
	ptr->r4slppar1 = ptr->bProg;
	ptr->r4slppar2 = ptr->bDHour;
	ptr->r4slppar3 = ptr->bDMin;
	ptr->bDHour = 0;
	ptr->bDMin = 0;
	ptr->r4slpcom = 18;
	}
//	ESP_LOGI(AP_TAG,"MQTT_delay_ON");
	} else if ((!incascmp("0",data,data_len)) || (!incascmp("off",data,data_len))
		|| (!incascmp("false",data,data_len))) {
	if ((ptr->bStNl) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strOFF,data,data_len))) {	
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 1;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_delay_OFF");
	}
	} else if (!memcmp(topic+topoff, "delay_hour", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, data, tempsz);
	uint8_t hour = atoi(tbuff);
	if (hour < 24) {
	ptr->bDHour = hour;
	ptr->t_last_us = ~ptr->t_last_us;
	}
	} else if (!memcmp(topic+topoff, "delay_min", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, data, tempsz);
	uint8_t min = atoi(tbuff);
	if (min < 60) {
	ptr->bDMin = min;
	ptr->t_last_us = ~ptr->t_last_us;
	}
	} else 	if (!memcmp(topic+topoff, "lock", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",data,data_len)) || (!incascmp("on",data,data_len))
		|| (!incascmp("true",data,data_len))) {
	if ((!ptr->bLock) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strON,data,data_len))) {	
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 8;
	}
//	ESP_LOGI(AP_TAG,"MQTT_lock_ON");
	} else if ((!incascmp("0",data,data_len)) || (!incascmp("off",data,data_len))
		|| (!incascmp("false",data,data_len))) {
	if ((ptr->bLock) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strOFF,data,data_len))) {	
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 7;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_lock_OFF");
	}
	} else 	if (!memcmp(topic+topoff, "strength", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",data,data_len)) || (!incascmp("on",data,data_len))
		|| (!incascmp("true",data,data_len))) {
	if ((!ptr->bProg) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strON,data,data_len))) {	
	ptr->r4slppar1 = 1;
	ptr->r4slpcom = 9;
	}
//	ESP_LOGI(AP_TAG,"MQTT_strength_ON");
	} else if ((!incascmp("0",data,data_len)) || (!incascmp("off",data,data_len))
		|| (!incascmp("false",data,data_len))) {
	if ((ptr->bProg) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strOFF,data,data_len))) {	
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 9;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_strength_OFF");
	}
	}

	} else if (ptr->DEV_TYP < 63) {
	//cooker
	if (!memcmp(topic+topoff, "state", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",data,data_len)) || (!incascmp("on",data,data_len))
		|| (!incascmp("true",data,data_len)) || (!incascmp("heat",data,data_len))) {
	if ((ptr->bState < 2) || (!ptr->r4sppcom) || (!fcommtp)) {
	ptr->r4slppar1 = 1;
	ptr->r4slpcom = 10;
	}
//	ESP_LOGI(AP_TAG,"MQTT_CMD_ON");
	} else if ((!incascmp("0",data,data_len)) || (!incascmp("off",data,data_len))
		|| (!incascmp("false",data,data_len))) {
	if ((ptr->bState > 1) || (!ptr->r4sppcom) || (!fcommtp)) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 10;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_CMD_OFF");
	}
	} else if (!memcmp(topic+topoff, "prname", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	ptr->r4slppar1 = 0;
	ptr->r4slppar2 = 0;
	ptr->r4slppar3 = 0;
	ptr->r4slppar4 = 0;
	ptr->r4slppar5 = 0;
	ptr->r4slppar6 = ptr->bAwarm;
	ptr->r4slppar7 = ptr->bDHour;
	ptr->r4slppar8 = ptr->bDMin;
        if ( ptr->DEV_TYP == 16 ) {
// for RMC-800s
	if (!incascmp("off",data,data_len)) ptr->r4slppar1 = 255; 
	else if (!incascmp("multicooker",data,data_len)) ptr->r4slppar1 = 0; 
	else if (!incascmp("rice",data,data_len)) ptr->r4slppar1 = 1; 
	else if (!incascmp("slow_cooking",data,data_len)) ptr->r4slppar1 = 2; 
	else if (!incascmp("pilaf",data,data_len)) ptr->r4slppar1 = 3; 
	else if (!incascmp("frying_vegetables",data,data_len)) { ptr->r4slppar1 = 4 ; ptr->r4slppar2 = 1; }
	else if (!incascmp("frying_fish",data,data_len)) { ptr->r4slppar1 = 4 ; ptr->r4slppar2 = 2; }
	else if (!incascmp("frying_meat",data,data_len)) { ptr->r4slppar1 = 4 ; ptr->r4slppar2 = 3; }
	else if (!incascmp("stewing_vegetables",data,data_len)) { ptr->r4slppar1 = 5 ; ptr->r4slppar2 = 1; }
	else if (!incascmp("stewing_fish",data,data_len)) { ptr->r4slppar1 = 5 ; ptr->r4slppar2 = 2; }
	else if (!incascmp("stewing_meat",data,data_len)) { ptr->r4slppar1 = 5 ; ptr->r4slppar2 = 3; }
	else if (!incascmp("pasta",data,data_len)) ptr->r4slppar1 = 6; 
	else if (!incascmp("milk_porridge",data,data_len)) ptr->r4slppar1 = 7; 
	else if (!incascmp("soup",data,data_len)) ptr->r4slppar1 = 8; 
	else if (!incascmp("yogurt",data,data_len)) ptr->r4slppar1 = 9; 
	else if (!incascmp("baking",data,data_len)) ptr->r4slppar1 = 10; 
	else if (!incascmp("steam_vegetables",data,data_len)) { ptr->r4slppar1 = 11 ; ptr->r4slppar2 = 1; }
	else if (!incascmp("steam_fish",data,data_len)) { ptr->r4slppar1 = 11 ; ptr->r4slppar2 = 2; }
	else if (!incascmp("steam_meat",data,data_len)) { ptr->r4slppar1 = 11 ; ptr->r4slppar2 = 3; }
	else if (!incascmp("hot",data,data_len)) ptr->r4slppar1 = 12; 
	} else if ( ptr->DEV_TYP == 17 ) {
// for RMC-903s
	if (!incascmp("off",data,data_len)) ptr->r4slppar1 = 255; 
	else if (!incascmp("Multicooker",data,data_len)) ptr->r4slppar1 = 0; 
	else if (!incascmp("Milk_porridge",data,data_len)) ptr->r4slppar1 = 1; 
	else if (!incascmp("Stewing",data,data_len)) ptr->r4slppar1 = 2; 
	else if (!incascmp("Frying",data,data_len)) ptr->r4slppar1 = 3; 
	else if (!incascmp("Soup",data,data_len)) ptr->r4slppar1 = 4; 
	else if (!incascmp("Steam",data,data_len)) ptr->r4slppar1 = 5; 
	else if (!incascmp("Pasta",data,data_len)) ptr->r4slppar1 = 6; 
	else if (!incascmp("Slow_cooking",data,data_len)) ptr->r4slppar1 = 7; 
	else if (!incascmp("Hot",data,data_len)) ptr->r4slppar1 = 8; 
	else if (!incascmp("Baking",data,data_len)) ptr->r4slppar1 = 9; 
	else if (!incascmp("Groats",data,data_len)) ptr->r4slppar1 = 10; 
	else if (!incascmp("Pilaf",data,data_len)) ptr->r4slppar1 = 11; 
	else if (!incascmp("Yogurt",data,data_len)) ptr->r4slppar1 = 12; 
	else if (!incascmp("Pizza",data,data_len)) ptr->r4slppar1 = 13; 
	else if (!incascmp("Bread",data,data_len)) ptr->r4slppar1 = 14; 
	else if (!incascmp("Desserts",data,data_len)) ptr->r4slppar1 = 15; 
	else if (!incascmp("Express",data,data_len)) ptr->r4slppar1 = 16; 
	} else if ( ptr->DEV_TYP == 18 ) {
// for RMC-224s
	if (!incascmp("off",data,data_len)) ptr->r4slppar1 = 255; 
	else if (!incascmp("Frying",data,data_len)) ptr->r4slppar1 = 0; 
	else if (!incascmp("Groats",data,data_len)) ptr->r4slppar1 = 1; 
	else if (!incascmp("Multicooker",data,data_len)) ptr->r4slppar1 = 2; 
	else if (!incascmp("Pilaf",data,data_len)) ptr->r4slppar1 = 3; 
	else if (!incascmp("Steam",data,data_len)) ptr->r4slppar1 = 4; 
	else if (!incascmp("Baking",data,data_len)) ptr->r4slppar1 = 5; 
	else if (!incascmp("Stewing",data,data_len)) ptr->r4slppar1 = 6; 
	else if (!incascmp("Soup",data,data_len)) ptr->r4slppar1 = 7; 
	else if (!incascmp("Milk_porridge",data,data_len)) ptr->r4slppar1 = 8; 
	else if (!incascmp("Yogurt",data,data_len)) ptr->r4slppar1 = 9; 
	else if (!incascmp("Express",data,data_len)) ptr->r4slppar1 = 10; 
	} else if ( ptr->DEV_TYP == 19 ) {
// for RMC-961s
	if (!incascmp("off",data,data_len)) ptr->r4slppar1 = 255; 
	else if (!incascmp("Groats",data,data_len)) ptr->r4slppar1 = 0; 
	else if (!incascmp("Frying",data,data_len)) ptr->r4slppar1 = 1; 
	else if (!incascmp("Steam",data,data_len)) ptr->r4slppar1 = 2; 
	else if (!incascmp("Baking",data,data_len)) ptr->r4slppar1 = 3; 
	else if (!incascmp("Stewing",data,data_len)) ptr->r4slppar1 = 4; 
	else if (!incascmp("Multicooker",data,data_len)) ptr->r4slppar1 = 5; 
	else if (!incascmp("Pilaf",data,data_len)) ptr->r4slppar1 = 6; 
	else if (!incascmp("Soup",data,data_len)) ptr->r4slppar1 = 7; 
	else if (!incascmp("Milk_porridge",data,data_len)) ptr->r4slppar1 = 8; 
	else if (!incascmp("Yogurt",data,data_len)) ptr->r4slppar1 = 9; 
	} else if ( ptr->DEV_TYP == 20 ) {
// for RMC-92s
	if (!incascmp("off",data,data_len)) ptr->r4slppar1 = 255; 
	else if (!incascmp("Multicooker",data,data_len)) ptr->r4slppar1 = 0; 
	else if (!incascmp("Milk_porridge",data,data_len)) ptr->r4slppar1 = 1; 
	else if (!incascmp("Stewing",data,data_len)) ptr->r4slppar1 = 2; 
	else if (!incascmp("Frying",data,data_len)) ptr->r4slppar1 = 3; 
	else if (!incascmp("Soup",data,data_len)) ptr->r4slppar1 = 4; 
	else if (!incascmp("Steam",data,data_len)) ptr->r4slppar1 = 5; 
	else if (!incascmp("Pasta",data,data_len)) ptr->r4slppar1 = 6; 
	else if (!incascmp("Slow_cooking",data,data_len)) ptr->r4slppar1 = 7; 
	else if (!incascmp("Hot",data,data_len)) ptr->r4slppar1 = 8; 
	else if (!incascmp("Baking",data,data_len)) ptr->r4slppar1 = 9; 
	else if (!incascmp("Groats",data,data_len)) ptr->r4slppar1 = 10; 
	else if (!incascmp("Pilaf",data,data_len)) ptr->r4slppar1 = 11; 
	else if (!incascmp("Yogurt",data,data_len)) ptr->r4slppar1 = 12; 
	else if (!incascmp("Pizza",data,data_len)) ptr->r4slppar1 = 13; 
	else if (!incascmp("Bread",data,data_len)) ptr->r4slppar1 = 14; 
	else if (!incascmp("Desserts",data,data_len)) ptr->r4slppar1 = 15; 
	else if (!incascmp("Express",data,data_len)) ptr->r4slppar1 = 16; 
	else if (!incascmp("Warming",data,data_len)) ptr->r4slppar1 = 17; 
	} else if ( ptr->DEV_TYP == 24 ) {
// for RO-5707
	if (!incascmp("off",data,data_len)) ptr->r4slppar1 = 255; 
	else if (!incascmp("Multicooker",data,data_len)) ptr->r4slppar1 = 0; 
	else if (!incascmp("Omelet",data,data_len)) ptr->r4slppar1 = 1; 
	else if (!incascmp("Slow_cooking_meat",data,data_len)) ptr->r4slppar1 = 2; 
	else if (!incascmp("Slow_cooking_bird",data,data_len)) ptr->r4slppar1 = 3; 
	else if (!incascmp("Slow_cooking_fish",data,data_len)) ptr->r4slppar1 = 4; 
	else if (!incascmp("Slow_cooking_vegetables",data,data_len)) ptr->r4slppar1 = 5; 
	else if (!incascmp("Bread",data,data_len)) ptr->r4slppar1 = 6; 
	else if (!incascmp("Pizza",data,data_len)) ptr->r4slppar1 = 7; 
	else if (!incascmp("Charlotte",data,data_len)) ptr->r4slppar1 = 8; 
	else if (!incascmp("Baking_meat_in_pot",data,data_len)) ptr->r4slppar1 = 9; 
	else if (!incascmp("Baking_bird_in_pot",data,data_len)) ptr->r4slppar1 = 10; 
	else if (!incascmp("Baking_fish_in_pot",data,data_len)) ptr->r4slppar1 = 11; 
	else if (!incascmp("Baking_vegetables_in_pot",data,data_len)) ptr->r4slppar1 = 12; 
	else if (!incascmp("Roast",data,data_len)) ptr->r4slppar1 = 13; 
	else if (!incascmp("Cake",data,data_len)) ptr->r4slppar1 = 14; 
	else if (!incascmp("Baking_meat",data,data_len)) ptr->r4slppar1 = 15; 
	else if (!incascmp("Baking_bird",data,data_len)) ptr->r4slppar1 = 16; 
	else if (!incascmp("Baking_fish",data,data_len)) ptr->r4slppar1 = 17; 
	else if (!incascmp("Baking_vegetables",data,data_len)) ptr->r4slppar1 = 18; 
	else if (!incascmp("Boiled_pork",data,data_len)) ptr->r4slppar1 = 19; 
	else if (!incascmp("Warming",data,data_len)) ptr->r4slppar1 = 20; 
	ptr->r4slppar2 = ptr->bModProg;
	}
	if ((!fcommtp) || (!ptr->r4sppcom) || (ptr->r4slppar1 != ptr->bProg) || (ptr->r4slppar2 != ptr->bModProg)) ptr->r4slpcom = 17;
	} else if (!memcmp(topic+topoff, "prog", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, data, tempsz);
	uint8_t prog = atoi(tbuff);
	if (prog == 254) prog = 255;
	if ((!fcommtp) || (!ptr->r4sppcom) || (prog != ptr->bProg)) {
	ptr->r4slppar1 = prog;
	ptr->r4slpcom = 11;
	}
	} else if (!memcmp(topic+topoff, "mode", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, data, tempsz);
	uint8_t mod = atoi(tbuff);
	if ((mod < 4) && ((!fcommtp) || (!ptr->r4sppcom) || ( mod != ptr->bModProg))) {
	ptr->r4slppar1 = mod;
	ptr->r4slpcom = 12;
	}
	} else if (!memcmp(topic+topoff, "temp", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, data, tempsz);
	uint8_t temp = atoi(tbuff);
	if ((!fcommtp) || (!ptr->r4sppcom) || (temp != ptr->bHtemp)) {
	ptr->r4slppar1 = temp;
	ptr->r4slpcom = 13;
	}
	} else if (!memcmp(topic+topoff, "set_hour", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, data, tempsz);
	uint8_t hour = atoi(tbuff);
	if ((hour < 24) && ((!fcommtp) || (!ptr->r4sppcom) || (hour != ptr->bPHour))) {
	ptr->r4slppar1 = hour;
	ptr->bPHour = hour;
	ptr->r4slpcom = 14;
	}
	} else if (!memcmp(topic+topoff, "set_min", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, data, tempsz);
	uint8_t min = atoi(tbuff);
	if ((min < 60) && ((!fcommtp) || (!ptr->r4sppcom) || (min != ptr->bPMin))) {
	ptr->r4slppar1 = min;
	ptr->r4slpcom = 15;
	}
	} else if (!memcmp(topic+topoff, "delay_hour", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, data, tempsz);
	uint8_t cval = atoi(tbuff);
	if ( ptr->DEV_TYP < 24 ) {
	ptr->bDHour = cval;
	ptr->t_last_us = ~ptr->t_last_us;
	} else {
	ptr->r4slppar1 = cval;
	ptr->bDHour = cval;
	ptr->r4slpcom = 19;
	}
	} else if (!memcmp(topic+topoff, "delay_min", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, data, tempsz);
	uint8_t cval = atoi(tbuff);
	if ( ptr->DEV_TYP < 24 ) {
	ptr->bDMin = cval;
	ptr->t_last_us = ~ptr->t_last_us;
	} else {
	ptr->r4slppar1 = cval;
	ptr->r4slpcom = 20;
	}
	} else if (!memcmp(topic+topoff, "warm", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",data,data_len)) || (!incascmp("on",data,data_len))
		|| (!incascmp("true",data,data_len))) {
	if ((!ptr->bAwarm) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strON,data,data_len))) {	
	ptr->r4slppar1 = 1;
	ptr->r4slpcom = 16;
	}
	} else if ((!incascmp("0",data,data_len)) || (!incascmp("off",data,data_len))
		|| (!incascmp("false",data,data_len))) {
	if ((ptr->bAwarm) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strOFF,data,data_len))) {	
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 16;
	}
	}
	}
	} else if (ptr->DEV_TYP == 63) {
	//weather station
	if (!memcmp(topic+topoff, "calibration", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",data,data_len)) || (!incascmp("on",data,data_len))
		|| (!incascmp("true",data,data_len))) {
	if ((!ptr->bState) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strON,data,data_len))) {
	ptr->bState = 1;
	ptr->bprevState = 255;
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 63;
	ptr->t_last_us = ~ptr->t_last_us;
	}
//	ESP_LOGI(AP_TAG,"MQTT_CALIBRATION_ON");
	} else if ((!incascmp("0",data,data_len)) || (!incascmp("off",data,data_len))
		|| (!incascmp("false",data,data_len))) {
	if ((ptr->bState)  || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strOFF,data,data_len))) {
	ptr->bState = 0;
	ptr->bprevState = 255;
	ptr->t_last_us = ~ptr->t_last_us;
	}
	}
//	ESP_LOGI(AP_TAG,"MQTT_CALIBRATION_OFF");
	}

	} else if (ptr->DEV_TYP > 63) {
	//mikettle
	if (!memcmp(topic+topoff, "boil", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",data,data_len)) || (!incascmp("on",data,data_len))
		|| (!incascmp("true",data,data_len)) || (!incascmp("heat",data,data_len))) {
	if ((!ptr->bState) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strON,data,data_len))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 65;
	}
//	ESP_LOGI(AP_TAG,"MQTT_BOIL_ON");
	} else if ((!incascmp("0",data,data_len)) || (!incascmp("off",data,data_len))
		|| (!incascmp("false",data,data_len))) {
	if ((ptr->bState)  || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strOFF,data,data_len))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 64;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_BOIL_OFF");
	}
	} else if (!memcmp(topic+topoff, "heat", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	if ((!incascmp("1",data,data_len)) || (!incascmp("on",data,data_len))
		|| (!incascmp("true",data,data_len)) || (!incascmp("heat",data,data_len))) {
	if ((!ptr->bHeat) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strON,data,data_len))) {
	ptr->r4slppar1 = 40;
	ptr->r4slpcom = 66;
	}
//	ESP_LOGI(AP_TAG,"MQTT_HEAT_ON");
	} else if ((!incascmp("0",data,data_len)) || (!incascmp("off",data,data_len))
		|| (!incascmp("false",data,data_len))) {
	if ((ptr->bHeat)  || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strOFF,data,data_len))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 66;
	}	
//	ESP_LOGI(AP_TAG,"MQTT_HEAT_OFF");
	}
	} else if (!memcmp(topic+topoff, "heat_temp", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 0);
	int tempsz = data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, data, tempsz);
	uint8_t temp = atoi(tbuff);
	if  ((!temp || (temp > 39)) && (temp < 91) && ((!fcommtp) || (!ptr->r4sppcom) || (temp != ptr->bHtemp))) {
	ptr->r4slppar1 = temp;
	ptr->r4slpcom = 66;
	} else if ((temp < 40) && ((!fcommtp) || (!ptr->r4sppcom) || (temp != ptr->bHtemp))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 64;
	} else if ((temp > 90) && ((!fcommtp) || (!ptr->r4sppcom) || (temp != ptr->bHtemp))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 65;
	}
	}
//	ESP_LOGI(AP_TAG,"MQTT_HEAT_TEMP");
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
	BleDevStA.t_ppcon_us = t_mqt_us;
	BleDevStB.t_ppcon_us = t_mqt_us;
	BleDevStC.t_ppcon_us = t_mqt_us;
	BleDevStA.t_last_us = ~BleDevStA.t_last_us;
	BleDevStB.t_last_us = ~BleDevStB.t_last_us;
	BleDevStC.t_last_us = ~BleDevStC.t_last_us;
	BleDevStA.iprevRssi = 0;
	BleDevStB.iprevRssi = 0;
	BleDevStC.iprevRssi = 0;
	iprevRssiESP = 0;

	BleDevStA.cprevStatus[0] = 0;
	BleDevStB.cprevStatus[0] = 0;
	BleDevStC.cprevStatus[0] = 0;
	BleDevStA.bprevSEnergy = ~BleDevStA.bSEnergy;
	BleDevStB.bprevSEnergy = ~BleDevStB.bSEnergy;
	BleDevStC.bprevSEnergy = ~BleDevStC.bSEnergy;
	BleDevStA.bprevSTime = ~BleDevStA.bSTime;
	BleDevStB.bprevSTime = ~BleDevStB.bSTime;
	BleDevStC.bprevSTime = ~BleDevStC.bSTime;
	BleDevStA.bprevSHum = ~BleDevStA.bSHum;
	BleDevStB.bprevSHum = ~BleDevStB.bSHum;
	BleDevStC.bprevSHum = ~BleDevStC.bSHum;
	BleDevStA.bprevSCount = ~BleDevStA.bSCount;
	BleDevStB.bprevSCount = ~BleDevStB.bSCount;
	BleDevStC.bprevSCount = ~BleDevStC.bSCount;
	BleDevStA.bprevBlTime = 128;
	BleDevStB.bprevBlTime = 128;
	BleDevStC.bprevBlTime = 128;
	BleDevStA.bprevLock = 255;
	BleDevStA.bprevState = 255;
	BleDevStA.bprevHeat = 255;
	BleDevStA.bprevStNl = 255;
	BleDevStA.bprevCtemp = 255;
	BleDevStA.bprevHtemp = 255;
	BleDevStA.bprevProg = 254;
	BleDevStA.bprevModProg = 255;
	BleDevStA.bprevPHour = 255;
	BleDevStA.bprevPMin = 255;
	BleDevStA.bprevCHour = 255;
	BleDevStA.bprevCMin = 255;
	BleDevStA.bprevDHour = 255;
	BleDevStA.bprevDMin = 255;
	BleDevStA.bprevAwarm = 255;
	BleDevStA.PRgbR = ~BleDevStA.RgbR;
	BleDevStA.PRgbG = ~BleDevStA.RgbG;
	BleDevStA.PRgbB = ~BleDevStA.RgbB;
        BleDevStA.bprevStBl = 255;
	BleDevStA.bprevStBp = 255;
	BleDevStA.bprevCVol = 255;
	BleDevStA.bprevCVoll = 255;
	BleDevStB.bprevLock = 255;
	BleDevStB.bprevState = 255;
	BleDevStB.bprevHeat = 255;
	BleDevStB.bprevStNl = 255;
	BleDevStB.bprevCtemp = 255;
	BleDevStB.bprevHtemp = 255;
	BleDevStB.bprevProg = 254;
	BleDevStB.bprevModProg = 255;
	BleDevStB.bprevPHour = 255;
	BleDevStB.bprevPMin = 255;
	BleDevStB.bprevCHour = 255;
	BleDevStB.bprevCMin = 255;
	BleDevStB.bprevDHour = 255;
	BleDevStB.bprevDMin = 255;
	BleDevStB.bprevAwarm = 255;
	BleDevStB.PRgbR = ~BleDevStB.RgbR;
	BleDevStB.PRgbG = ~BleDevStB.RgbG;
	BleDevStB.PRgbB = ~BleDevStB.RgbB;
        BleDevStB.bprevStBl = 255;
	BleDevStB.bprevStBp = 255;
	BleDevStB.bprevCVol = 255;
	BleDevStB.bprevCVoll = 255;
	BleDevStC.bprevLock = 255;
	BleDevStC.bprevState = 255;
	BleDevStC.bprevHeat = 255;
	BleDevStC.bprevStNl = 255;
	BleDevStC.bprevCtemp = 255;
	BleDevStC.bprevHtemp = 255;
	BleDevStC.bprevProg = 254;
	BleDevStC.bprevModProg = 255;
	BleDevStC.bprevPHour = 255;
	BleDevStC.bprevPMin = 255;
	BleDevStC.bprevCHour = 255;
	BleDevStC.bprevCMin = 255;
	BleDevStC.bprevDHour = 255;
	BleDevStC.bprevDMin = 255;
	BleDevStC.bprevAwarm = 255;
	BleDevStC.PRgbR = ~BleDevStC.RgbR;
	BleDevStC.PRgbG = ~BleDevStC.RgbG;
	BleDevStC.PRgbB = ~BleDevStC.RgbB;
        BleDevStC.bprevStBl = 255;
	BleDevStC.bprevStBp = 255;
	BleDevStC.bprevCVol = 255;
	BleDevStC.bprevCVoll = 255;

	bprevStateS = ~bStateS;

	fgpio1 = 1;
	fgpio2 = 1;
	fgpio3 = 1;
	fgpio4 = 1;
	fgpio5 = 1;

	for (int i = 0; i < BleMonNum; i++) {
        BleMX[i].prstate =255;
        BleMX[i].prrssi = 255;
	BleMX[i].ppar1 = ~BleMX[i].par1;
	BleMX[i].ppar2 = ~BleMX[i].par2;
	BleMX[i].ppar3 = ~BleMX[i].par3;
	BleMX[i].ppar4 = ~BleMX[i].par4;
	BleMX[i].ppar5 = ~BleMX[i].par5;
	BleMX[i].ppar6 = ~BleMX[i].par6;
	}

	char llwtt[128];
	char llwtd[512];
	strcpy(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/status");
	esp_mqtt_client_publish(client, llwtt, "online", 0, 1, 1);
//	msg_id = esp_mqtt_client_publish(client, llwtt, "online", 0, 1, 1);
//	ESP_LOGI(AP_TAG,"sent publish successful, msg_id=%d", msg_id);
	tcpip_adapter_ip_info_t ipInfo;
	char wbuff[256];
	memset(wbuff,0,32);
	tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo);
	sprintf(wbuff, "%d.%d.%d.%d", IP2STR(&ipInfo.ip));
	strcpy(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/ip");
	esp_mqtt_client_publish(client, llwtt, wbuff, 0, 1, 1);
	if (MQTT_TOPP1[0]) esp_mqtt_client_subscribe(client, MQTT_TOPP1, 0);
	if (MQTT_TOPP2[0]) esp_mqtt_client_subscribe(client, MQTT_TOPP2, 0);
	if (MQTT_TOPP3[0]) esp_mqtt_client_subscribe(client, MQTT_TOPP3, 0);
	if (MQTT_TOPP4[0]) esp_mqtt_client_subscribe(client, MQTT_TOPP4, 0);
	if (MQTT_TOPP5[0]) esp_mqtt_client_subscribe(client, MQTT_TOPP5, 0);
	if (MQTT_TOPP6[0]) esp_mqtt_client_subscribe(client, MQTT_TOPP6, 0);
	if (MQTT_TOPP7[0]) esp_mqtt_client_subscribe(client, MQTT_TOPP7, 0);
	if (FDHass && tESP32Addr[0]) {
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

	strcpy(llwtt,"homeassistant/number/");
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
	strcat(llwtd,"/screen\",\"min\":\"0\",\"max\":\"255\",\"availability_topic\":\"");
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

	if (ble_mon) {
	char tmpvar[64];
	for (int i = 0; i < BleMonNum; i++) {
	if(BleMR[i].sto) {
//
	strcpy(llwtt,"homeassistant/device_tracker/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/1x");
        if (BleMR[i].id == 2) bin2hex(BleMR[i].mac,tmpvar,16,0); 
        else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtt,tmpvar);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".");
        if (BleMR[i].id == 2) bin2hex(BleMR[i].mac,tmpvar,16,0); 
        else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,".state\",\"icon\":\"mdi:tag\",\"uniq_id\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"_");
        if (BleMR[i].id == 2) bin2hex(BleMR[i].mac,tmpvar,16,0); 
        else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"_state\",\"device\":{\"identifiers\":[\"r4s_");
        if (BleMR[i].id == 2) bin2hex(BleMR[i].mac,tmpvar,16,0); 
        else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"\"],\"name\":\"r4s.");
        if (BleMR[i].id == 2) bin2hex(BleMR[i].mac,tmpvar,16,0); 
        else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"\",\"model\":\"");
        if (BleMR[i].id == 3) strcat(llwtd,"ATC_Thermometer LYWSD03MMC");
        if (BleMR[i].id == 2) strcat(llwtd,"HA beacon");
        else strcat(llwtd,"Unknown");
	strcat(llwtd,"\",\"manufacturer\":\"");
        if (BleMR[i].id == 3) strcat(llwtd,"Xiaomi & pvvx");
        if (BleMR[i].id == 2) strcat(llwtd,"HA mobile");
        else strcat(llwtd,"Unknown");
	strcat(llwtd,"\",\"via_device\":\"ESP32_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\"},\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/");
        if (BleMR[i].id == 2) bin2hex(BleMR[i].mac,tmpvar,16,0); 
        else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"/state\",\"payload_home\":\"ON\",\"payload_not_home\":\"OFF\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
//
	strcpy(llwtt,"homeassistant/sensor/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/1x");
        if (BleMR[i].id == 2) bin2hex(BleMR[i].mac,tmpvar,16,0); 
        else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtt,tmpvar);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".");
        if (BleMR[i].id == 2) bin2hex(BleMR[i].mac,tmpvar,16,0); 
        else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,".rssi\",\"icon\":\"mdi:bluetooth\",\"uniq_id\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"_");
        if (BleMR[i].id == 2) bin2hex(BleMR[i].mac,tmpvar,16,0); 
        else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"_rssi\",\"device\":{\"identifiers\":[\"r4s_");
        if (BleMR[i].id == 2) bin2hex(BleMR[i].mac,tmpvar,16,0); 
        else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"\"],\"name\":\"r4s.");
        if (BleMR[i].id == 2) bin2hex(BleMR[i].mac,tmpvar,16,0); 
        else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"\",\"model\":\"");
        if (BleMR[i].id == 3) strcat(llwtd,"ATC_Thermometer LYWSD03MMC");
        if (BleMR[i].id == 2) strcat(llwtd,"HA beacon");
        else strcat(llwtd,"Unknown");
	strcat(llwtd,"\",\"manufacturer\":\"");
        if (BleMR[i].id == 3) strcat(llwtd,"Xiaomi & pvvx");
        if (BleMR[i].id == 2) strcat(llwtd,"HA mobile");
        else strcat(llwtd,"Unknown");
	strcat(llwtd,"\",\"via_device\":\"ESP32_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\"},\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/");
        if (BleMR[i].id == 2) bin2hex(BleMR[i].mac,tmpvar,16,0); 
        else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"/rssi\",\"unit_of_meas\":\"dBm\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
//
	if (BleMR[i].id == 3) {
	strcpy(llwtt,"homeassistant/sensor/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/2x");
        bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtt,tmpvar);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".");
        bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,".temp\",\"icon\":\"mdi:thermometer\",\"uniq_id\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"_");
        bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"_temp\",\"device\":{\"identifiers\":[\"r4s_");
        bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"\"],\"name\":\"r4s.");
        bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"\",\"model\":\"");
        strcat(llwtd,"ATC_Thermometer LYWSD03MMC");
	strcat(llwtd,"\",\"manufacturer\":\"");
        strcat(llwtd,"Xiaomi & pvvx");
	strcat(llwtd,"\",\"via_device\":\"ESP32_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\"},\"device_class\":\"temperature\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/");
        bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"/temperature\",\"unit_of_meas\":\"\xc2\xb0\x43\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
//
	strcpy(llwtt,"homeassistant/sensor/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/3x");
        bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtt,tmpvar);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".");
        bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,".humid\",\"icon\":\"mdi:water-percent\",\"uniq_id\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"_");
        bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"_humid\",\"device\":{\"identifiers\":[\"r4s_");
        bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"\"],\"name\":\"r4s.");
        bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"\",\"model\":\"");
        strcat(llwtd,"ATC_Thermometer LYWSD03MMC");
	strcat(llwtd,"\",\"manufacturer\":\"");
        strcat(llwtd,"Xiaomi & pvvx");
	strcat(llwtd,"\",\"via_device\":\"ESP32_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\"},\"device_class\":\"humidity\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/");
        bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"/humidity\",\"unit_of_meas\":\"\x25\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
//
	strcpy(llwtt,"homeassistant/sensor/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/4x");
        bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtt,tmpvar);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".");
        bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,".battery\",\"icon\":\"mdi:battery-bluetooth\",\"uniq_id\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"_");
        bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"_battery\",\"device\":{\"identifiers\":[\"r4s_");
        bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"\"],\"name\":\"r4s.");
        bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"\",\"model\":\"");
        strcat(llwtd,"ATC_Thermometer LYWSD03MMC");
	strcat(llwtd,"\",\"manufacturer\":\"");
        strcat(llwtd,"Xiaomi & pvvx");
	strcat(llwtd,"\",\"via_device\":\"ESP32_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\"},\"device_class\":\"battery\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/");
        bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"/battery\",\"unit_of_meas\":\"\x25\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
//




	}
	}
	}
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
        MqttPubSub(0, true);	
        MqttPubSub(1, true);	
        MqttPubSub(2, true);	
	mqttConnected = true;
	break;

	case MQTT_EVENT_DISCONNECTED:
	mqttConnected = false;
	t_mqt_us = esp_timer_get_time();
	BleDevStA.t_ppcon_us = t_mqt_us;
	BleDevStB.t_ppcon_us = t_mqt_us;
	BleDevStC.t_ppcon_us = t_mqt_us;

	BleDevStA.iprevRssi = 0;
	BleDevStB.iprevRssi = 0;
	BleDevStC.iprevRssi = 0;

	BleDevStA.cprevStatus[0] = 0;
	BleDevStB.cprevStatus[0] = 0;
	BleDevStC.cprevStatus[0] = 0;

	BleDevStA.bprevLock = 255;
	BleDevStA.bprevState = 255;
	BleDevStA.bprevHeat = 255;
	BleDevStA.bprevStNl = 255;
	BleDevStA.bprevCtemp = 255;
	BleDevStA.bprevHtemp = 255;
	BleDevStA.bprevProg = 254;
	BleDevStA.bprevModProg = 255;
	BleDevStA.bprevPHour = 255;
	BleDevStA.bprevPMin = 255;
	BleDevStA.bprevCHour = 255;
	BleDevStA.bprevCMin = 255;
	BleDevStA.bprevDHour = 255;
	BleDevStA.bprevDMin = 255;
	BleDevStA.bprevAwarm = 255;
	BleDevStA.PRgbR = ~BleDevStA.RgbR;
	BleDevStA.PRgbG = ~BleDevStA.RgbG;
	BleDevStA.PRgbB = ~BleDevStA.RgbB;
        BleDevStA.bprevStBl = 255;
	BleDevStA.bprevStBp = 255;
	BleDevStB.bprevLock = 255;
	BleDevStB.bprevState = 255;
	BleDevStB.bprevHeat = 255;
	BleDevStB.bprevStNl = 255;
	BleDevStB.bprevCtemp = 255;
	BleDevStB.bprevHtemp = 255;
	BleDevStB.bprevProg = 254;
	BleDevStB.bprevModProg = 255;
	BleDevStB.bprevPHour = 255;
	BleDevStB.bprevPMin = 255;
	BleDevStB.bprevCHour = 255;
	BleDevStB.bprevCMin = 255;
	BleDevStB.bprevDHour = 255;
	BleDevStB.bprevDMin = 255;
	BleDevStB.bprevAwarm = 255;
	BleDevStB.PRgbR = ~BleDevStB.RgbR;
	BleDevStB.PRgbG = ~BleDevStB.RgbG;
	BleDevStB.PRgbB = ~BleDevStB.RgbB;
        BleDevStB.bprevStBl = 255;
	BleDevStB.bprevStBp = 255;
	BleDevStC.bprevLock = 255;
	BleDevStC.bprevState = 255;
	BleDevStC.bprevHeat = 255;
	BleDevStC.bprevStNl = 255;
	BleDevStC.bprevCtemp = 255;
	BleDevStC.bprevHtemp = 255;
	BleDevStC.bprevProg = 254;
	BleDevStC.bprevModProg = 255;
	BleDevStC.bprevPHour = 255;
	BleDevStC.bprevPMin = 255;
	BleDevStC.bprevCHour = 255;
	BleDevStC.bprevCMin = 255;
	BleDevStC.bprevDHour = 255;
	BleDevStC.bprevDMin = 255;
	BleDevStC.bprevAwarm = 255;
	BleDevStC.PRgbR = ~BleDevStC.RgbR;
	BleDevStC.PRgbG = ~BleDevStC.RgbG;
	BleDevStC.PRgbB = ~BleDevStC.RgbB;
        BleDevStC.bprevStBl = 255;
	BleDevStC.bprevStBp = 255;

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
	if (BleDevStA.r4sppcom) {
        if ((t_mqt_us - BleDevStA.t_ppcom_us) > 3000000) BleDevStA.r4sppcom = 0;
//fdeltmp = t_mqt_us - BleDevStA.t_ppcom_us;
//ESP_LOGI(AP_TAG,"Mqtt_delay_A=%d", fdeltmp);
	}
	if (BleDevStB.r4sppcom) {
        if ((t_mqt_us - BleDevStB.t_ppcom_us) > 3000000) BleDevStB.r4sppcom = 0;
//fdeltmp = t_mqt_us - BleDevStB.t_ppcom_us;
//ESP_LOGI(AP_TAG,"Mqtt_delay_B=%d", fdeltmp);
	}
	if (BleDevStC.r4sppcom) {
        if ((t_mqt_us - BleDevStC.t_ppcom_us) > 3000000) BleDevStC.r4sppcom = 0;
//fdeltmp = t_mqt_us - BleDevStC.t_ppcom_us;
//ESP_LOGI(AP_TAG,"Mqtt_delay_C=%d", fdeltmp);
	}
	if (r4sppcoms) {
        if ((t_mqt_us - t_ppcoms_us) > 3000000) r4sppcoms = 0;
	}
	if (BleDevStA.tBLEAddr[0]) {
	strcpy(tbuff,MQTT_BASE_TOPIC);
	strcat(tbuff,"/");
	strcat(tbuff,BleDevStA.tBLEAddr);
	if (!fcommtp) strcat(tbuff,"/cmd");
	strcat(tbuff,"/");
	topoffa = parsoff(event->topic,tbuff, event->topic_len);
	}
	if (BleDevStB.tBLEAddr[0]) {
	strcpy(tbuff,MQTT_BASE_TOPIC);
	strcat(tbuff,"/");
	strcat(tbuff,BleDevStB.tBLEAddr);
	if (!fcommtp) strcat(tbuff,"/cmd");
	strcat(tbuff,"/");
	topoffb = parsoff(event->topic,tbuff, event->topic_len);
	}
	if (BleDevStC.tBLEAddr[0]) {
	strcpy(tbuff,MQTT_BASE_TOPIC);
	strcat(tbuff,"/");
	strcat(tbuff,BleDevStC.tBLEAddr);
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
	if ((!incascmp("restart",event->data,event->data_len)) || (!incascmp("reset",event->data,event->data_len))
		|| (!incascmp("reboot",event->data,event->data_len))) {
	esp_restart();
	} else if (event->data_len && (event->data_len < 5)){
	uint16_t duty = 255;
	mystrcpy(tbuff, event->data, event->data_len);
	duty = atoi(tbuff);
	if (duty > 255) duty = 255;
	if ((duty != bStateS) || (!r4sppcoms)) { 
	if (!tft_conn) duty = 0;
	ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty);
	ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
        bStateS = duty;
	t_lasts_us = ~t_lasts_us;
	}
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
	} else if (topoffa && BleDevStA.DEV_TYP && ((t_mqt_us - BleDevStA.t_ppcon_us) > 4000000)) {
//ESP_LOGI(AP_TAG,"topoffa=%d", topoffa);
//	BleMqtPr(uint8_t blenum, int topoff, char *topic, int topic_len, char *data, int data_len) {
	BleMqtPr(0,  topoffa, event->topic, event->topic_len, event->data, event->data_len);

	} else if (topoffb && BleDevStB.DEV_TYP && ((t_mqt_us - BleDevStB.t_ppcon_us) > 4000000)) {
//ESP_LOGI(AP_TAG,"topoffb=%d", topoffb);
	BleMqtPr(1,  topoffb, event->topic, event->topic_len, event->data, event->data_len);

	} else if (topoffc && BleDevStC.DEV_TYP && ((t_mqt_us - BleDevStC.t_ppcon_us) > 4000000)) {
//ESP_LOGI(AP_TAG,"topoffc=%d", topoffc);
	BleMqtPr(2,  topoffc, event->topic, event->topic_len, event->data, event->data_len);

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
//	if (floop && MQTT_SERVER[0]) esp_mqtt_client_stop(mqttclient);
//	if (floop && MQTT_SERVER[0]) esp_mqtt_client_disconnect(mqttclient);
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
//	if (floop && MQTT_SERVER[0]) esp_mqtt_client_start(mqttclient);
	if (floop && MQTT_SERVER[0]) esp_mqtt_client_reconnect(mqttclient);
	NumWfConn++;
	if (!NumWfConn) NumWfConn--;
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



void MnHtpBleSt(uint8_t blenum, char* bsend) {
	uint8_t blenum1 = blenum + 1;
	char buff[16]; 
	if (blenum > 2) return;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	strcat(bsend,"<h3>");
	itoa(blenum1,buff,10);
	strcat(bsend,buff);
	strcat(bsend," ");
       	if (ptr->tBLEAddr[0] && ptr->DEV_TYP) {
	if ((ptr->DEV_TYP < 10) || (ptr->DEV_TYP > 63)) strcat(bsend,"Kettle, ");
	else if (ptr->DEV_TYP < 11) strcat(bsend,"Power, ");
	else if (ptr->DEV_TYP < 12) strcat(bsend,"Heater, ");
	else if (ptr->DEV_TYP < 16) strcat(bsend,"Coffee, ");
	else if (ptr->DEV_TYP < 24) strcat(bsend,"Cooker, ");
	else if (ptr->DEV_TYP < 32) strcat(bsend,"Oven, ");
	else if (ptr->DEV_TYP == 63) strcat(bsend,"Weather, ");
	strcat(bsend,"Address: ");
	strcat(bsend,ptr->tBLEAddr);
	strcat(bsend,", Name: ");
	if (ptr->DEV_NAME[0]) strcat(bsend,ptr->DEV_NAME);
	if ((ptr->DEV_TYP < 10) || (ptr->DEV_TYP > 63)) {
	strcat(bsend,", State: ");
        if ((!ptr->bState) && (!ptr->bHeat || (ptr->DEV_TYP > 63)) && (!ptr->bStNl)) strcat(bsend,"Off");
 	else if (ptr->bState == 254) strcat(bsend,"Offline");
 	else strcat(bsend,"On");
//	if ((ptr->DEV_TYP < 10) || (ptr->DEV_TYP > 3)) {
	if (ptr->DEV_TYP < 10) {
	strcat(bsend,", Mode: ");
	if (ptr->bProg == 1) strcat(bsend,"Heat");
	else if (ptr->bProg == 2) strcat(bsend,"Boil&Heat");
	else if (ptr->bProg == 3) strcat(bsend,"Nightlight");
	else strcat(bsend,"Boil");
	}
	strcat(bsend,", Temp: ");
	itoa(ptr->bCtemp,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"&deg;C, Heat: ");
	itoa(ptr->bHtemp,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"&deg;C, ");
	} else if (ptr->DEV_TYP < 12) {
	strcat(bsend,", State: ");
        if (!ptr->bState) strcat(bsend,"Off");
 	else if (ptr->bState == 254) strcat(bsend,"Offline");
 	else strcat(bsend,"On");
	strcat(bsend,", Lock: ");
        if (!ptr->bLock) strcat(bsend,"Off");
 	else strcat(bsend,"On");
	if (ptr->DEV_TYP == 11) {
	strcat(bsend,", Power: ");
	itoa(ptr->bHtemp,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"&#x25;");
	}
	strcat(bsend,", ");
	} else if (ptr->DEV_TYP < 16) {
	strcat(bsend,", State: ");
        if (!ptr->bState) strcat(bsend,"Off");
 	else if (ptr->bState == 254) strcat(bsend,"Offline");
 	else strcat(bsend,"On");
	strcat(bsend,", Strength: ");
        if (!ptr->bProg) strcat(bsend,"Off");
 	else strcat(bsend,"On");
	strcat(bsend,", Delay: ");
        if (!ptr->bStNl) strcat(bsend,"Off");
 	else strcat(bsend,"On");
	strcat(bsend,", Lock: ");
        if (!ptr->bLock) strcat(bsend,"Off");
 	else strcat(bsend,"On");
	strcat(bsend,", SetTime: ");
	if (ptr->bPHour < 10) strcat(bsend,"0");
	itoa(ptr->bPHour,buff,10);
	strcat(bsend,buff);
	strcat(bsend,":");
	if (ptr->bPMin < 10) strcat(bsend,"0");
	itoa(ptr->bPMin,buff,10);
	strcat(bsend,buff);
	strcat(bsend,", CurTime: ");
	if (ptr->bCHour < 10) strcat(bsend,"0");
	itoa(ptr->bCHour,buff,10);
	strcat(bsend,buff);
	strcat(bsend,":");
	if (ptr->bCMin < 10) strcat(bsend,"0");
	itoa(ptr->bCMin,buff,10);
	strcat(bsend,buff);
	strcat(bsend,", ");
	} else if (ptr->DEV_TYP < 63) {
	strcat(bsend,", State: ");
	if (ptr->DEV_TYP == 20) {	
        if (!ptr->bState) strcat(bsend,"Off");
        else if (ptr->bState == 1) strcat(bsend,"Setting");
        else if (ptr->bState == 2) strcat(bsend,"DelayedStart");
        else if (ptr->bState == 3) strcat(bsend,"Boil4Pasta");
        else if (ptr->bState == 4) strcat(bsend,"WaitPasta");
        else if (ptr->bState == 5) strcat(bsend,"On");
        else if (ptr->bState == 6) strcat(bsend,"Warming");
 	else if (ptr->bState == 254) strcat(bsend,"Offline");
 	} else {
        if (!ptr->bState) strcat(bsend,"Off");
        else if (ptr->bState == 1) strcat(bsend,"Setting");
        else if (ptr->bState == 2) strcat(bsend,"On");
        else if (ptr->bState == 3) strcat(bsend,"WaitPasta");
        else if (ptr->bState == 4) strcat(bsend,"Warming");
        else if (ptr->bState == 5) strcat(bsend,"DelayedStart");
        else if (ptr->bState == 6) strcat(bsend,"Boil4Pasta");
 	else if (ptr->bState == 254) strcat(bsend,"Offline");
	}
	strcat(bsend,", Prog: ");
	itoa(ptr->bProg,buff,10);
	strcat(bsend,buff);
	strcat(bsend,", Mode: ");
	itoa(ptr->bModProg,buff,10);
	strcat(bsend,buff);
	strcat(bsend,", Temp: ");
	itoa(ptr->bHtemp,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"&deg;C, SetTime: ");
	if (ptr->bPHour < 10) strcat(bsend,"0");
	itoa(ptr->bPHour,buff,10);
	strcat(bsend,buff);
	strcat(bsend,":");
	if (ptr->bPMin < 10) strcat(bsend,"0");
	itoa(ptr->bPMin,buff,10);
	strcat(bsend,buff);
	strcat(bsend,", CurTime: ");
	if (ptr->bCHour < 10) strcat(bsend,"0");
	itoa(ptr->bCHour,buff,10);
	strcat(bsend,buff);
	strcat(bsend,":");
	if (ptr->bCMin < 10) strcat(bsend,"0");
	itoa(ptr->bCMin,buff,10);
	strcat(bsend,buff);
	strcat(bsend,", ");
	} else if (ptr->DEV_TYP == 63) {
	uint32_t tmp = 0;
	strcat(bsend,", Temperature: ");
	if (ptr->bSEnergy & 0x80000000) strcat(bsend,"-");
	itoa((ptr->bSEnergy & 0x7fffffff) / 10,buff,10);
	strcat(bsend,buff);
	strcat(bsend,".");
	itoa((ptr->bSEnergy & 0x7fffffff) % 10,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"&deg;C, Humidity: ");
	itoa(ptr->bSHum / 10,buff,10);
	strcat(bsend,buff);
	strcat(bsend,".");
	itoa(ptr->bSHum % 10,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"&#37;, Pressure: ");
	itoa(ptr->bSTime / 100,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"hPa / ");
	tmp = ptr->bSTime * 75 / 1000;
        itoa(tmp / 10,buff,10);
	strcat(bsend,buff);
	strcat(bsend,".");
        itoa(tmp % 10,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"mmHg, Quality: ");
	itoa(ptr->bSCount,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"mm3/m3, ");
	}
	} else strcat(bsend,"Not defined, ");
        strcat(bsend,"Json String:</h3><h2>{\"mqtt\":");
	itoa(mqttConnected,buff,10);
	strcat(bsend,buff);
        if (ptr->tBLEAddr[0]) {
        strcat(bsend,",\"");
	strcat(bsend,ptr->tBLEAddr);
        strcat(bsend,"\":{\"name\":\"");
	if (ptr->DEV_NAME[0]) strcat(bsend,ptr->DEV_NAME);
        strcat(bsend,"\",\"status\":\"");
        if (ptr->btauthoriz) {
        strcat(bsend,"online\",\"state\":");
        if (ptr->cStatus[0]) strcat(bsend,ptr->cStatus);
        } else strcat(bsend,"offline\"");
	}
        strcat(bsend,"}</h2>");
}



//*************** http server *******************

static bool test_auth(httpd_req_t *req) {
	if (!AUTH_BASIC[0])
		return true;

	char buf_auth[51] = {0};
	char auth[56] = {0};

	strcat(auth, "Basic ");
	strcat(auth, AUTH_BASIC);

	int buf_len;

	buf_len = httpd_req_get_hdr_value_len(req, "Authorization") + 1;
	if (buf_len > 1)
	{
		if (buf_len > 49)
			buf_len = 49;
		/* Copy null terminated value string into buffer */
		if (httpd_req_get_hdr_value_str(req, "Authorization", buf_auth, buf_len) == ESP_OK)
		{
			if (strcmp(auth, buf_auth) == 0) {
				return true;
			}
		}
	}

	httpd_resp_set_hdr(req, "WWW-Authenticate", "Basic realm=\"User Visible Realm\"");
	httpd_resp_send_err(req, HTTPD_401_UNAUTHORIZED, NULL);
	httpd_resp_sendstr(req, "");

	return false;
}


/* HTTP GET main handler */
static esp_err_t pmain_get_handler(httpd_req_t *req)
{
	if (!test_auth(req)) {
		return ESP_OK;
	}

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
        strcat(bsend,"<meta http-equiv=\"refresh\" content=\"5\">");
        strcat(bsend,cssDatasheet);
	strcat(bsend,"</head><body class='bodymenu'><header class='headermenu'><h1>ESP32 r4sGate");
	if (R4SNUM)  {
	itoa(R4SNUM,buff,10);
	strcat(bsend, buff);
	}
	strcat(bsend," for Redmond +</h1>");
	strcat(bsend,"<div class='menubar'><a class='menu active' href='.'>&#8962;<span class='showmenulabel'>Main");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev1'>&#128246;<span class='showmenulabel'> 1 ");
	(BleDevStA.RQC_NAME[0])? strcat(bsend,BleDevStA.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev2'>&#128246;<span class='showmenulabel'> 2 ");
	(BleDevStB.RQC_NAME[0])? strcat(bsend,BleDevStB.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev3'>&#128246;<span class='showmenulabel'> 3 ");
	(BleDevStC.RQC_NAME[0])? strcat(bsend,BleDevStC.RQC_NAME) : strcat(bsend,"Not defined");
	if (ble_mon) strcat(bsend,"</span></a><a class='menu' href='blemon'>&#128246;<span class='showmenulabel'>BLE monitor</span></a>");
	strcat(bsend,"</span></a><a class='menu' href='setting'>&#9881;<span class='showmenulabel'>Setting</span></a>");
	strcat(bsend,"<a class='menu' href='restart'>&#128259;<span class='showmenulabel'>Reboot</span></a>");
	strcat(bsend,"<a class='menu' href='update'>&#10548;<span class='showmenulabel'>Load firmware</span></a></div>");
	strcat(bsend,"</header><table class='normal'>");

	MnHtpBleSt(0, bsend);
	MnHtpBleSt(1, bsend);
	MnHtpBleSt(2, bsend);

        strcat(bsend,"<h3>System Info</h3><table class='normal'>");
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
        if (Isscanning || (BleDevStA.REQ_NAME[0] && !BleDevStA.btauthoriz) || (BleDevStB.REQ_NAME[0] && !BleDevStB.btauthoriz) ||(BleDevStC.REQ_NAME[0] && !BleDevStC.btauthoriz)) {
        strcat(bsend,"</td></tr><tr><td>BLE found name / address / RSSI</td><td>");
        strcat(bsend,FND_NAME);
	strcat(bsend," / ");
        strcat(bsend,FND_ADDR);
	strcat(bsend," / ");
	itoa(FND_RSSI,buff,10);
	strcat(bsend,buff);
        strcat(bsend," dB");
        strcat(bsend,"</td></tr><tr><td>BLE activity</td><td>");
	if (BleDevStA.btconnect && !BleDevStA.btauthoriz) {
	strcat(bsend,"Connecting ");
	strcat(bsend, BleDevStA.RQC_NAME);
        if (!BleDevStA.btopen) strcat(bsend," (Open");
	else {
	strcat(bsend," (Auth");
	if (BleDevStA.DEV_TYP > 63) {
	strcat(bsend,"/Id:");
	itoa(BleDevStA.MiKettleID,buff,10);
	strcat(bsend,buff);
	}
	}
        strcat(bsend,")");
	} else if (BleDevStB.btconnect && !BleDevStB.btauthoriz) {
	strcat(bsend,"Connecting ");
	strcat(bsend, BleDevStB.RQC_NAME);
        if (!BleDevStB.btopen) strcat(bsend," (Open");
	else {
	strcat(bsend," (Auth");
	if (BleDevStB.DEV_TYP > 63) {
	strcat(bsend," / Id:");
	itoa(BleDevStB.MiKettleID,buff,10);
	strcat(bsend,buff);
	}
	}
        strcat(bsend,")");
	} else if (BleDevStC.btconnect && !BleDevStC.btauthoriz) {
	strcat(bsend,"Connecting ");
	strcat(bsend, BleDevStC.RQC_NAME);
        if (!BleDevStC.btopen) strcat(bsend," (Open");
	else {
	strcat(bsend," (Auth");
	if (BleDevStC.DEV_TYP > 63) {
	strcat(bsend," / Id:");
	itoa(BleDevStC.MiKettleID,buff,10);
	strcat(bsend,buff);
	}
	}
        strcat(bsend,")");
	} else if (Isscanning) strcat(bsend,"Scanning");
	else strcat(bsend,"Idle");
	}
        strcat(bsend,"</td></tr><tr><td>BLE 1 connection count / state</td><td>");
        itoa(BleDevStA.NumConn,buff,10);
	strcat(bsend,buff);
        (BleDevStA.btauthoriz)? strcat(bsend," / Connected") : strcat(bsend," / Disconnected");
	if ((BleDevStA.btauthoriz) && (BleDevStA.iRssi)) {
        strcat(bsend,"</td></tr><tr><td>BLE 1 RSSI</td><td>");
        itoa(BleDevStA.iRssi,buff,10);
	strcat(bsend,buff);
        strcat(bsend," dB");
	}

        strcat(bsend,"</td></tr><tr><td>BLE 2 connection count / state</td><td>");
        itoa(BleDevStB.NumConn,buff,10);
	strcat(bsend,buff);
        (BleDevStB.btauthoriz)? strcat(bsend," / Connected") : strcat(bsend," / Disconnected");
	if ((BleDevStB.btauthoriz) && (BleDevStB.iRssi)) {
        strcat(bsend,"</td></tr><tr><td>BLE 2 RSSI</td><td>");
        itoa(BleDevStB.iRssi,buff,10);
	strcat(bsend,buff);
        strcat(bsend," dB");
	}

        strcat(bsend,"</td></tr><tr><td>BLE 3 connection count / state</td><td>");
        itoa(BleDevStC.NumConn,buff,10);
	strcat(bsend,buff);
        (BleDevStC.btauthoriz)? strcat(bsend," / Connected") : strcat(bsend," / Disconnected");
	if ((BleDevStC.btauthoriz) && (BleDevStC.iRssi)) {
        strcat(bsend,"</td></tr><tr><td>BLE 3 RSSI</td><td>");
        itoa(BleDevStC.iRssi,buff,10);
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
	strcat(bsend,"</td></tr><tr><td>WiFi connection count / RSSI</td><td>");
	itoa(NumWfConn,buff,10);
	strcat(bsend,buff);
	strcat(bsend," / ");
        itoa(wifidata.rssi,buff,10);
	strcat(bsend,buff);
        strcat(bsend," dB</td></tr><tr><td>WiFi IP / MAC address</td><td>");
        strcat(bsend,bufip);
	strcat(bsend," / ");
	strcat(bsend,tESP32Addr1);
	strcat(bsend,"</td></tr>");
	}
	strcat(bsend,"<tr><td>MQTT server:port / state</td><td>");
        if (MQTT_SERVER[0]) strcat(bsend,MQTT_SERVER);
	strcat(bsend,":");
	itoa(mqtt_port,buff,10);
	strcat(bsend,buff);
        (mqttConnected)? strcat(bsend," / Connected") : strcat(bsend," / Disconnected");
	strcat(bsend,"</td></tr>");
	strcat(bsend,"<tr><td>ILI9341 320*240 LCD</td><td>");
	if (!tft_conf) strcat(bsend,"Disabled");
	else (tft_conn)? strcat(bsend,"Connected") : strcat(bsend,"Disconnected");
	strcat(bsend,"</td></tr></table><br><footer><h6>More info on <a href='https://github.com/alutov/ESP32-R4sGate-for-Redmond' style='font-size: 15px; text-decoration: none'>github.com/alutov</a></h6>");

//itoa(strlen(bsend),buff,10);
//strcat(bsend,buff);
//	strcat(bsend,"<meta http-equiv=\"refresh\" content=\"5;URL=http://");
//	strcat(bsend,bufip);
//	strcat(bsend,"/\"</body></html>");

	strcat(bsend,"</body></html>");
        httpd_resp_sendstr(req, bsend);
    return ESP_OK;
}
static const httpd_uri_t pmain = {
	.uri       = "/",
	.method    = HTTP_GET,
	.handler   = pmain_get_handler,
	.user_ctx  = NULL
};


void HtpDeVHandle(uint8_t blenum, char* bsend) {
	uint8_t blenum1 = blenum + 1;
	char buff[16]; 
	if (blenum > 2) return;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	strcat(bsend,"<table class='normal'><h3>");
	itoa(blenum1,buff,10);
	strcat(bsend,buff);
	strcat(bsend," ");
	(ptr->RQC_NAME[0])? strcat(bsend,ptr->RQC_NAME) : strcat(bsend,"Not defined");
	if ((ptr->DEV_TYP > 0) && (ptr->DEV_TYP < 10)) strcat(bsend," Kettle");
	if ((ptr->DEV_TYP > 9) && (ptr->DEV_TYP < 11)) strcat(bsend," Power");
	if ((ptr->DEV_TYP > 10) && (ptr->DEV_TYP < 12)) strcat(bsend," Heater");
	if ((ptr->DEV_TYP > 11) && (ptr->DEV_TYP < 16)) strcat(bsend," Coffee");
	if ((ptr->DEV_TYP > 15) && (ptr->DEV_TYP < 24)) strcat(bsend," Cooker");
	if ((ptr->DEV_TYP > 23) && (ptr->DEV_TYP < 63)) strcat(bsend," Oven");
	if (ptr->DEV_TYP == 63) strcat(bsend," Weather");
	if ((ptr->DEV_TYP > 63) && (ptr->DEV_TYP < 70)) strcat(bsend," Kettle");
	strcat(bsend," Control</h3><br/>");
	if (!ptr->REQ_NAME[0] || !ptr->DEV_TYP || !ptr->btauthoriz) {
	if (ptr->REQ_NAME[0] && ptr->DEV_TYP && (ptr->DEV_TYP < 128) && !ptr->btauthoriz) {
	if ((ptr->DEV_TYP > 0) && (ptr->DEV_TYP < 10)) strcat(bsend,"Kettle ");
	if ((ptr->DEV_TYP > 9) && (ptr->DEV_TYP < 11)) strcat(bsend,"Power ");
	if ((ptr->DEV_TYP > 10) && (ptr->DEV_TYP < 12)) strcat(bsend,"Heater ");
	if ((ptr->DEV_TYP > 11) && (ptr->DEV_TYP < 16)) strcat(bsend,"Coffee ");
	if ((ptr->DEV_TYP > 15) && (ptr->DEV_TYP < 24)) strcat(bsend,"Cooker ");
	if ((ptr->DEV_TYP > 23) && (ptr->DEV_TYP < 63)) strcat(bsend,"Oven ");
	if (ptr->DEV_TYP == 63) strcat(bsend,"Weather ");
	if ((ptr->DEV_TYP > 63) && (ptr->DEV_TYP < 70)) strcat(bsend,"Kettle ");
	strcat(bsend,ptr->RQC_NAME);
	strcat(bsend," not connected</br>");
	} else strcat(bsend,"Device not defined</br>");
	strcat(bsend,"<body><form method=\"POST\" action=\"/setignore\">");
	} else if (ptr->DEV_TYP < 10) {
	strcat(bsend,"<body><form method=\"POST\" action=\"/cfgdev");
	itoa(blenum1,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"ok\">");
	strcat(bsend,"<select name=\"sstate\"><option ");
        strcat(bsend,"value=\"1\">Switch Off</option><option ");
	if (!ptr->bState) strcat(bsend,"selected ");
        strcat(bsend,"value=\"2\">Boil On</option><option ");
        strcat(bsend,"value=\"3\">Heat On</option><option ");
	strcat(bsend,"value=\"4\">Boil&Heat On</option><option ");
	strcat(bsend,"value=\"5\">NightLight On</option><option ");
	strcat(bsend,"value=\"21\">BackLight Off</option><option ");
	strcat(bsend,"value=\"22\">BackLight On</option><option ");
	strcat(bsend,"value=\"23\">Beep Off</option><option ");
	strcat(bsend,"value=\"24\">Beep On</option><option ");
        strcat(bsend,"value=\"25\">Boil 1l On</option></select>Select state</br>");
	strcat(bsend,"<input name=\"stemp\" type=\"number\" value=\"0\" min=\"0\" max=\"95\" size=\"2\">Heat temp 0-95&deg;C, if 0 heat off or boil only</br>");
	strcat(bsend,"<input name=\"rlight\" type=\"number\" value=\"");
	itoa(ptr->RgbR,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"255\" size=\"3\">Light Red</br><input name=\"glight\" type=\"number\" value=\"");
	itoa(ptr->RgbG,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"255\" size=\"3\">Light Green</br><input name=\"blight\" type=\"number\" value=\"");
	itoa(ptr->RgbB,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"255\" size=\"3\">Light Blue</br>");
	if ((ptr->DEV_TYP > 3) && !ptr->bState && !ptr->bHeat) {
	strcat(bsend,"<input name=\"sbltim\" type=\"number\" value=\"");
	itoa(ptr->bBlTime,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"-5\" max=\"5\" size=\"3\">Boil Time(Smart Boil)</br>");
	}
	} else if ( ptr->DEV_TYP == 63) {
	strcat(bsend,"<body><form method=\"POST\" action=\"/cfgdev");
	itoa(blenum1,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"ok\">");
	strcat(bsend,"<select name=\"sstate\"><option ");
        strcat(bsend,"value=\"63\">Start Calibration</option></select>Select state</br>");
//	strcat(bsend,"No control available</br>");
//	strcat(bsend,"<body><form method=\"POST\" action=\"/setignore\">");
	} else if ( ptr->DEV_TYP > 63) {
	if ((ptr->bModProg == 2) && (ptr->xshedcom != 2)) {
	strcat(bsend,"<body><form method=\"POST\" action=\"/cfgdev");
	itoa(blenum1,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"ok\">");
	strcat(bsend,"<select name=\"sstate\"><option ");
        strcat(bsend,"value=\"64\">Switch Off</option><option ");
	if (!ptr->bState) strcat(bsend,"selected ");
	strcat(bsend,"value=\"65\">Boil On</option><option ");
        strcat(bsend,"value=\"66\">Heat On</option></select>Select state</br>");
	strcat(bsend,"<input name=\"stemp\" type=\"number\" value=\"40\" min=\"0\" max=\"95\" size=\"2\">Heat temp 40-95&deg;C, if 0 heat off</br>");
	} else { strcat(bsend,"Press \"Warm\" on MiKettle to remote device control</br>");
	strcat(bsend,"<body><form method=\"POST\" action=\"/setignore\">");
	}
	} else if ( ptr->DEV_TYP < 12) {
	strcat(bsend,"<body><form method=\"POST\" action=\"/cfgdev");
	itoa(blenum1,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"ok\">");
	strcat(bsend,"<select name=\"sstate\"><option ");
	strcat(bsend,"value=\"1\">Switch Off</option><option ");
	if (!ptr->bState) strcat(bsend,"selected ");
	strcat(bsend,"value=\"6\">Switch On</option><option ");
	strcat(bsend,"value=\"7\">Lock Off</option><option ");
	strcat(bsend,"value=\"8\">Lock On</option></select>Select state</br>");
	if ( ptr->DEV_TYP == 11) {
	strcat(bsend,"<input name=\"stemp\" type=\"number\" value=\"");
	if (ptr->bLtemp > 9) itoa(ptr->bLtemp,buff,10);
	else itoa(100,buff,10);
	strcat(bsend,buff);
        strcat(bsend,"\" min=\"0\" max=\"100\" size=\"3\">Power 0-100&#x25;</br>");
	}
	} else if ( ptr->DEV_TYP < 16) {
	strcat(bsend,"<body><form method=\"POST\" action=\"/cfgdev");
	itoa(blenum1,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"ok\">");
	strcat(bsend,"<select name=\"sstate\"><option ");
        strcat(bsend,"value=\"1\">Switch Off</option><option ");
	if (!ptr->bState) strcat(bsend,"selected ");
	strcat(bsend,"value=\"6\">Switch On</option><option ");
	strcat(bsend,"value=\"18\">Delayed Start</option><option ");
	strcat(bsend,"value=\"7\">Lock Off</option><option ");
	strcat(bsend,"value=\"8\">Lock On</option></select>Select state</br>");
	strcat(bsend,"<select name=\"smod\"><option ");
	if (!ptr->bProg) strcat(bsend,"selected ");
        strcat(bsend,"value=\"1\">Coffee Strength Off</option><option ");
	if (ptr->bProg) strcat(bsend,"selected ");
	strcat(bsend,"value=\"2\">Coffee Strength On</option></select>Set coffee strength</br>");
	strcat(bsend,"<input name=\"sphour\" type=\"number\" value=\"");
	itoa(ptr->bPHour,buff,10);
	strcat(bsend,buff);
        strcat(bsend,"\" min=\"0\" max=\"23\" size=\"2\">Set Hour</br>");
	strcat(bsend,"<input name=\"spmin\" type=\"number\" value=\"");
	itoa(ptr->bPMin,buff,10);
	strcat(bsend,buff);
        strcat(bsend,"\" min=\"0\" max=\"59\" size=\"2\">Set Min</br>");
	} else if ( ptr->DEV_TYP < 64) {
	if (ptr->bProg < 24) {
	if ((ptr->bDHour != ptr->bCHour) || (ptr->bDMin != ptr->bCMin)) {
	ptr->bDHour = 0;
	ptr->bDMin = 0;
	}
	}
	strcat(bsend,"<body><form method=\"POST\" action=\"/cfgdev");
	itoa(blenum1,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"ok\">");
	strcat(bsend,"<select name=\"sstate\"><option ");
        strcat(bsend,"value=\"11\">Switch Off</option><option ");
        if ((ptr->DEV_TYP != 20) && (ptr->DEV_TYP < 24) && (ptr->bProg > 127)) strcat(bsend,"value=\"12\">Warming On</option><option ");
	if (!ptr->bState || (ptr->bState == 1)) strcat(bsend,"selected ");
        strcat(bsend,"value=\"13\">Set Program</option>");
	if (ptr->bProg < 128) strcat(bsend,"<option value=\"14\">Start Program</option></select>Select state</br>");
	else strcat(bsend,"</select>Select state</br>");
	strcat(bsend,"<select name=\"sprog\"><option ");
	if (ptr->bProg > 127) strcat(bsend,"selected ");
        strcat(bsend,"value=\"255\">Not defined</option><option ");
        if ( ptr->DEV_TYP == 16 ) {
// for RMC-800s
	if (ptr->bProg == 0) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Multicooker / Мультиповар</option><option ");
	if (ptr->bProg == 1) strcat(bsend,"selected ");
        strcat(bsend,"value=\"1\">Rice / Рис Крупы (0)</option><option ");
	if (ptr->bProg == 2) strcat(bsend,"selected ");
        strcat(bsend,"value=\"2\">Slow cooking / Томление (0)</option><option ");
	if (ptr->bProg == 3) strcat(bsend,"selected ");
        strcat(bsend,"value=\"3\">Pilaf / Плов (0)</option><option ");
	if (ptr->bProg == 4) strcat(bsend,"selected ");
        strcat(bsend,"value=\"4\">Frying / Жарка (1-3)</option><option ");
	if (ptr->bProg == 5) strcat(bsend,"selected ");
        strcat(bsend,"value=\"5\">Stewing / Тушение (1-3)</option><option ");
	if (ptr->bProg == 6) strcat(bsend,"selected ");
        strcat(bsend,"value=\"6\">Pasta / Макароны (0)</option><option ");
	if (ptr->bProg == 7) strcat(bsend,"selected ");
        strcat(bsend,"value=\"7\">Milk porridge / Молочная каша (0)</option><option ");
	if (ptr->bProg == 8) strcat(bsend,"selected ");
        strcat(bsend,"value=\"8\">Soup / Суп (0)</option><option ");
	if (ptr->bProg == 9) strcat(bsend,"selected ");
        strcat(bsend,"value=\"9\">Yogurt / Йогурт (0)</option><option ");
	if (ptr->bProg == 10) strcat(bsend,"selected ");
        strcat(bsend,"value=\"10\">Baking / Выпечка (0)</option><option ");
	if (ptr->bProg == 11) strcat(bsend,"selected ");
        strcat(bsend,"value=\"11\">Steam / Пар (1-3)</option><option ");
	if (ptr->bProg == 12) strcat(bsend,"selected ");
	strcat(bsend,"value=\"12\">Hot / Варка Бобовые (0)</option></select>Select program</br>");
	strcat(bsend,"<select name=\"smod\"><option ");
	if (ptr->bModProg == 0) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Not defined (0)</option><option ");
	if (ptr->bModProg == 1) strcat(bsend,"selected ");
        strcat(bsend,"value=\"1\">Vegetables / Овощи (1)</option><option ");
	if (ptr->bModProg == 2) strcat(bsend,"selected ");
        strcat(bsend,"value=\"2\">Fish / Рыба (2)<option ");
	if (ptr->bModProg == 3) strcat(bsend,"selected ");
	strcat(bsend,"value=\"3\">Meat / Мясо (3)</option></select>Select program mode</br>");
	} else if ( ptr->DEV_TYP == 17 ) {
// for RMC-903s
	if (ptr->bProg == 0) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Multicooker / Мультиповар</option><option ");
	if (ptr->bProg == 1) strcat(bsend,"selected ");
        strcat(bsend,"value=\"1\">Milk porridge / Молочная каша</option><option ");
	if (ptr->bProg == 2) strcat(bsend,"selected ");
        strcat(bsend,"value=\"2\">Stewing / Тушение</option><option  ");
	if (ptr->bProg == 3) strcat(bsend,"selected ");
        strcat(bsend,"value=\"3\">Frying / Жарка</option><option ");
	if (ptr->bProg == 4) strcat(bsend,"selected ");
        strcat(bsend,"value=\"4\">Soup / Суп</option><option ");
	if (ptr->bProg == 5) strcat(bsend,"selected ");
        strcat(bsend,"value=\"5\">Steam / Пар</option><option ");
	if (ptr->bProg == 6) strcat(bsend,"selected ");
        strcat(bsend,"value=\"6\">Pasta / Макароны</option><option ");
	if (ptr->bProg == 7) strcat(bsend,"selected ");
        strcat(bsend,"value=\"7\">Slow cooking / Томление</option><option ");
	if (ptr->bProg == 8) strcat(bsend,"selected ");
        strcat(bsend,"value=\"8\">Hot / Варка</option><option ");
	if (ptr->bProg == 9) strcat(bsend,"selected ");
        strcat(bsend,"value=\"9\">Baking / Выпечка</option><option ");
	if (ptr->bProg == 10) strcat(bsend,"selected ");
        strcat(bsend,"value=\"10\">Groats / Крупы</option><option ");
	if (ptr->bProg == 11) strcat(bsend,"selected ");
        strcat(bsend,"value=\"11\">Pilaf / Плов</option><option ");
	if (ptr->bProg == 12) strcat(bsend,"selected ");
        strcat(bsend,"value=\"12\">Yogurt / Йогурт</option><option ");
	if (ptr->bProg == 13) strcat(bsend,"selected ");
        strcat(bsend,"value=\"13\">Pizza / Пицца</option><option ");
	if (ptr->bProg == 14) strcat(bsend,"selected ");
        strcat(bsend,"value=\"14\">Bread / Хлеб</option><option ");
	if (ptr->bProg == 15) strcat(bsend,"selected ");
	strcat(bsend,"value=\"15\">Desserts / Десерты</option><option ");
	if (ptr->bProg == 16) strcat(bsend,"selected ");
	strcat(bsend,"value=\"16\">Express / Экспресс</option></select>Select program</br>");
	strcat(bsend,"<select name=\"smod\"><option ");
	strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Not defined</option></select>Select program mode</br>");
	} else if ( ptr->DEV_TYP == 18 ) {
// for RMC-224s
	if (ptr->bProg == 0) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Frying / Жарка</option><option ");
	if (ptr->bProg == 1) strcat(bsend,"selected ");
        strcat(bsend,"value=\"1\">Groats / Крупы</option><option ");
	if (ptr->bProg == 2) strcat(bsend,"selected ");
        strcat(bsend,"value=\"2\">Multicooker / Мультиповар</option><option ");
	if (ptr->bProg == 3) strcat(bsend,"selected ");
        strcat(bsend,"value=\"3\">Pilaf / Плов</option><option ");
	if (ptr->bProg == 4) strcat(bsend,"selected ");
        strcat(bsend,"value=\"4\">Steam / Пар</option><option ");
	if (ptr->bProg == 5) strcat(bsend,"selected ");
        strcat(bsend,"value=\"5\">Baking / Выпечка</option><option ");
	if (ptr->bProg == 6) strcat(bsend,"selected ");
        strcat(bsend,"value=\"6\">Stewing / Тушение</option><option  ");
	if (ptr->bProg == 7) strcat(bsend,"selected ");
        strcat(bsend,"value=\"7\">Soup / Суп</option><option ");
	if (ptr->bProg == 8) strcat(bsend,"selected ");
        strcat(bsend,"value=\"8\">Milk porridge / Молочная каша</option><option ");
	if (ptr->bProg == 9) strcat(bsend,"selected ");
        strcat(bsend,"value=\"9\">Yogurt / Йогурт</option><option ");
	if (ptr->bProg == 10) strcat(bsend,"selected ");
	strcat(bsend,"value=\"10\">Express / Экспресс</option></select>Select program</br>");
	strcat(bsend,"<select name=\"smod\"><option ");
	strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Not defined</option></select>Select program mode</br>");
	} else if ( ptr->DEV_TYP == 19 ) {
// for RMC-961s
	if (ptr->bProg == 0) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Groats / Крупы</option><option ");
	if (ptr->bProg == 1) strcat(bsend,"selected ");
        strcat(bsend,"value=\"1\">Frying / Жарка</option><option ");
	if (ptr->bProg == 2) strcat(bsend,"selected ");
        strcat(bsend,"value=\"2\">Steam / Пар</option><option ");
	if (ptr->bProg == 3) strcat(bsend,"selected ");
        strcat(bsend,"value=\"3\">Baking / Выпечка</option><option ");
	if (ptr->bProg == 4) strcat(bsend,"selected ");
        strcat(bsend,"value=\"4\">Stewing / Тушение</option><option  ");
	if (ptr->bProg == 5) strcat(bsend,"selected ");
        strcat(bsend,"value=\"5\">Multicooker / Мультиповар</option><option ");
	if (ptr->bProg == 6) strcat(bsend,"selected ");
        strcat(bsend,"value=\"6\">Pilaf / Плов</option><option ");
	if (ptr->bProg == 7) strcat(bsend,"selected ");
        strcat(bsend,"value=\"7\">Soup / Суп</option><option ");
	if (ptr->bProg == 8) strcat(bsend,"selected ");
        strcat(bsend,"value=\"8\">Milk porridge / Молочная каша</option><option ");
	if (ptr->bProg == 9) strcat(bsend,"selected ");
        strcat(bsend,"value=\"9\">Yogurt / Йогурт</option></select>Select program</br>");
	strcat(bsend,"<select name=\"smod\"><option ");
	strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Not defined</option></select>Select program mode</br>");
	} else if ( ptr->DEV_TYP == 20 ) {
// for RMC-92s
	if (ptr->bProg == 0) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Multicooker / Мультиповар</option><option ");
	if (ptr->bProg == 1) strcat(bsend,"selected ");
        strcat(bsend,"value=\"1\">Milk porridge / Молочная каша</option><option ");
	if (ptr->bProg == 2) strcat(bsend,"selected ");
        strcat(bsend,"value=\"2\">Stewing / Тушение</option><option  ");
	if (ptr->bProg == 3) strcat(bsend,"selected ");
        strcat(bsend,"value=\"3\">Frying / Жарка</option><option ");
	if (ptr->bProg == 4) strcat(bsend,"selected ");
        strcat(bsend,"value=\"4\">Soup / Суп</option><option ");
	if (ptr->bProg == 5) strcat(bsend,"selected ");
        strcat(bsend,"value=\"5\">Steam / Пар</option><option ");
	if (ptr->bProg == 6) strcat(bsend,"selected ");
        strcat(bsend,"value=\"6\">Pasta / Макароны</option><option ");
	if (ptr->bProg == 7) strcat(bsend,"selected ");
        strcat(bsend,"value=\"7\">Slow cooking / Томление</option><option ");
	if (ptr->bProg == 8) strcat(bsend,"selected ");
        strcat(bsend,"value=\"8\">Hot / Варка</option><option ");
	if (ptr->bProg == 9) strcat(bsend,"selected ");
        strcat(bsend,"value=\"9\">Baking / Выпечка</option><option ");
	if (ptr->bProg == 10) strcat(bsend,"selected ");
        strcat(bsend,"value=\"10\">Groats / Крупы</option><option ");
	if (ptr->bProg == 11) strcat(bsend,"selected ");
        strcat(bsend,"value=\"11\">Pilaf / Плов</option><option ");
	if (ptr->bProg == 12) strcat(bsend,"selected ");
        strcat(bsend,"value=\"12\">Yogurt / Йогурт</option><option ");
	if (ptr->bProg == 13) strcat(bsend,"selected ");
        strcat(bsend,"value=\"13\">Pizza / Пицца</option><option ");
	if (ptr->bProg == 14) strcat(bsend,"selected ");
        strcat(bsend,"value=\"14\">Bread / Хлеб</option><option ");
	if (ptr->bProg == 15) strcat(bsend,"selected ");
	strcat(bsend,"value=\"15\">Desserts / Десерты</option><option ");
	if (ptr->bProg == 16) strcat(bsend,"selected ");
	strcat(bsend,"value=\"16\">Express / Экспресс</option><option ");
	if (ptr->bProg == 17) strcat(bsend,"selected ");
	strcat(bsend,"value=\"17\">Warming / Разогрев</option></select>Select program</br>");
	strcat(bsend,"<select name=\"smod\"><option ");
	strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Not defined</option></select>Select program mode</br>");
	} else if ( ptr->DEV_TYP == 24 ) {
	if (ptr->bProg == 0) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Multicooker / Мультиповар</option><option ");
	if (ptr->bProg == 1) strcat(bsend,"selected ");
        strcat(bsend,"value=\"1\">Omelet / Омлет</option><option ");
	if (ptr->bProg == 2) strcat(bsend,"selected ");
        strcat(bsend,"value=\"2\">Slow cooking meat / Томление мясо</option><option  ");
	if (ptr->bProg == 3) strcat(bsend,"selected ");
        strcat(bsend,"value=\"3\">Slow cooking bird / Томление птица</option><option ");
	if (ptr->bProg == 4) strcat(bsend,"selected ");
        strcat(bsend,"value=\"4\">Slow cooking fish / Томление  рыба</option><option ");
	if (ptr->bProg == 5) strcat(bsend,"selected ");
        strcat(bsend,"value=\"5\">Slow cooking vegetables / Томление овощи</option><option ");
	if (ptr->bProg == 6) strcat(bsend,"selected ");
        strcat(bsend,"value=\"6\">Bread / Хлеб</option><option ");
	if (ptr->bProg == 7) strcat(bsend,"selected ");
        strcat(bsend,"value=\"7\">Pizza / Пицца</option><option ");
	if (ptr->bProg == 8) strcat(bsend,"selected ");
        strcat(bsend,"value=\"8\">Charlotte / Шарлотка</option><option ");
	if (ptr->bProg == 9) strcat(bsend,"selected ");
        strcat(bsend,"value=\"9\">Baking meat in pot / Запекание в горшочке мясо</option><option ");
	if (ptr->bProg == 10) strcat(bsend,"selected ");
        strcat(bsend,"value=\"10\">Baking bird in pot / Запекание в горшочке птица</option><option ");
	if (ptr->bProg == 11) strcat(bsend,"selected ");
        strcat(bsend,"value=\"11\">Baking fish in pot / Запекание в горошочке рыба</option><option ");
	if (ptr->bProg == 12) strcat(bsend,"selected ");
        strcat(bsend,"value=\"12\">Baking vegetables in pot / Запекание в горшочке овощи</option><option ");
	if (ptr->bProg == 13) strcat(bsend,"selected ");
        strcat(bsend,"value=\"13\">Roast / Жаркое</option><option ");
	if (ptr->bProg == 14) strcat(bsend,"selected ");
        strcat(bsend,"value=\"14\">Cake / Кекс</option><option ");
	if (ptr->bProg == 15) strcat(bsend,"selected ");
	strcat(bsend,"value=\"15\">Baking meat / Запекание мясо</option><option ");
	if (ptr->bProg == 16) strcat(bsend,"selected ");
	strcat(bsend,"value=\"16\">Baking bird / Запекание птица</option><option ");
	if (ptr->bProg == 17) strcat(bsend,"selected ");
	strcat(bsend,"value=\"17\">Baking fish / Запекание рыба</option><option ");
	if (ptr->bProg == 18) strcat(bsend,"selected ");
	strcat(bsend,"value=\"18\">Baking vegetables / Запекание овощи</option><option ");
	if (ptr->bProg == 19) strcat(bsend,"selected ");
	strcat(bsend,"value=\"19\">Boiled pork / Буженина</option><option ");
	if (ptr->bProg == 20) strcat(bsend,"selected ");
	strcat(bsend,"value=\"20\">Warming / Разогрев</option></select>Select program</br>");
	strcat(bsend,"<select name=\"smod\"><option ");
	if (ptr->bModProg == 0) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Top heater</option><option ");
	if (ptr->bModProg == 1) strcat(bsend,"selected ");
        strcat(bsend,"value=\"1\">Bottom heater<option ");
	if (ptr->bModProg == 2) strcat(bsend,"selected ");
	strcat(bsend,"value=\"2\">Top and bottom heaters</option></select>Select heating mode</br>");
	}
	strcat(bsend,"<input name=\"stemp\" type=\"number\" value=\"");
	itoa(ptr->bHtemp,buff,10);
	strcat(bsend,buff);
        strcat(bsend,"\" min=\"0\" max=\"230\" size=\"3\">Set Temp 0-230&deg;C</br>");
	strcat(bsend,"<input name=\"sphour\" type=\"number\" value=\"");
	itoa(ptr->bPHour,buff,10);
	strcat(bsend,buff);
        strcat(bsend,"\" min=\"0\" max=\"23\" size=\"2\">Set Hour</br>");
	strcat(bsend,"<input name=\"spmin\" type=\"number\" value=\"");
	itoa(ptr->bPMin,buff,10);
	strcat(bsend,buff);
        strcat(bsend,"\" min=\"0\" max=\"59\" size=\"2\">Set Min</br>");
	strcat(bsend,"<input name=\"sdhour\" type=\"number\" value=\"");
	itoa(ptr->bDHour,buff,10);
	strcat(bsend,buff);
        strcat(bsend,"\" min=\"0\" max=\"23\" size=\"2\">Set Delay Hour</br>");
	strcat(bsend,"<input name=\"sdmin\" type=\"number\" value=\"");
	itoa(ptr->bDMin,buff,10);
	strcat(bsend,buff);
        strcat(bsend,"\" min=\"0\" max=\"59\" size=\"2\">Set Delay Min</br>");
	strcat(bsend,"<select name=\"swarm\"><option ");
	if (!ptr->bAwarm) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Auto Warming Off</option><option ");
	if (ptr->bAwarm) strcat(bsend,"selected ");
	strcat(bsend,"value=\"1\">Auto Warming On</option></select>Set auto warming</br>");
	}
	strcat(bsend,"<h3> Store values then press Ok. </h3><br/>");
	strcat(bsend,"<input type=SUBMIT value=\"Ok\">");
	strcat(bsend,"</form><form method=\"POST\" action=\"/setignore\">");
	strcat(bsend,"<input type=SUBMIT value=\"Cancel\"></form></body></html>");
}




/* HTTP GET device1 handler */
static esp_err_t pcfgdev1_get_handler(httpd_req_t *req)
{
	if (!test_auth(req)) {
		return ESP_OK;
	}

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
	(BleDevStA.RQC_NAME[0])? strcat(bsend,BleDevStA.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev2'>&#128246;<span class='showmenulabel'> 2 ");
	(BleDevStB.RQC_NAME[0])? strcat(bsend,BleDevStB.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev3'>&#128246;<span class='showmenulabel'> 3 ");
	(BleDevStC.RQC_NAME[0])? strcat(bsend,BleDevStC.RQC_NAME) : strcat(bsend,"Not defined");
	if (ble_mon) strcat(bsend,"</span></a><a class='menu' href='blemon'>&#128246;<span class='showmenulabel'>BLE monitor</span></a>");
	strcat(bsend,"</span></a><a class='menu' href='setting'>&#9881;<span class='showmenulabel'>Setting</span></a>");
	strcat(bsend,"<a class='menu' href='restart'>&#128259;<span class='showmenulabel'>Reboot</span></a>");
	strcat(bsend,"<a class='menu' href='update'>&#10548;<span class='showmenulabel'>Load firmware</span></a></div>");

	strcat(bsend,"</header>");
        HtpDeVHandle(0, bsend);

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
	int8_t bnewBlTime = BleDevStA.bBlTime;
	int  ret;
	int  cm_done = BleDevStA.bProg;
	ret = httpd_req_recv(req,buf1,512);
	if ( ret > 0 ) {
//ESP_LOGI(AP_TAG, "Buf: '%s'", buf1);
/*

in buf1 after httpd_req_recv string like below
swfid=wifiname&swfpsw=wifipassword&smqsrv=192.168.1.10&smqid=esp&
smqpsw=esp&devnam=&rlight=255&glight=255&blight=255&chk2=2

*/
	if (BleDevStA.DEV_TYP < 10) {
	buf3[0] = 0;
	strcpy(buf2,"rlight");
	parsuri(buf1,buf3,buf2,512,4);
	BleDevStA.RgbR = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"glight");
	parsuri(buf1,buf3,buf2,512,4);
	BleDevStA.RgbG = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"blight");
	parsuri(buf1,buf3,buf2,512,4);
	BleDevStA.RgbB = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"sbltim");
	parsuri(buf1,buf3,buf2,512,3);
	bnewBlTime = atoi(buf3);
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
	BleDevStA.r4slppar1 = pmod;		
	BleDevStA.r4slpcom = 1;
	break;
	case 2:             //boil(on)
	if (bnewBlTime != BleDevStA.bBlTime) {
	cm_done = 0;
	BleDevStA.r4slppar1 = bnewBlTime;
	BleDevStA.r4slpcom = 25;
	} else {
	cm_done = 1;
	BleDevStA.r4slppar1 = 0;
	BleDevStA.r4slpcom = 2;
	}
	break;
	case 3:             //heat(on)
	cm_done = 1;
	if (BleDevStA.DEV_TYP == 1) {	
	if (temp < 30) BleDevStA.r4slppar1 = 0;
	else if (temp < 41) BleDevStA.r4slppar1 = 1;
	else if (temp < 56) BleDevStA.r4slppar1 = 2;
	else if (temp < 71) BleDevStA.r4slppar1 = 3;
	else if (temp < 86) BleDevStA.r4slppar1 = 4;
	else BleDevStA.r4slppar1 = 5;
	} else BleDevStA.r4slppar1 = temp;
	if (temp > 29) {
	BleDevStA.r4slppar2 = 1;
	BleDevStA.r4slpcom = 3;
	} else {
	BleDevStA.r4slppar1 = 0;
	BleDevStA.r4slpcom = 1;
	}
	break;
	case 4:             //boil&heat(on)
	cm_done = 1;
	if (BleDevStA.DEV_TYP == 1) {	
	if (temp < 30) BleDevStA.r4slppar1 = 0;
	else if (temp < 41) BleDevStA.r4slppar1 = 1;
	else if (temp < 56) BleDevStA.r4slppar1 = 2;
	else if (temp < 71) BleDevStA.r4slppar1 = 3;
	else if (temp < 86) BleDevStA.r4slppar1 = 4;
	else BleDevStA.r4slppar1 = 5;
	} else BleDevStA.r4slppar1 = temp;
	BleDevStA.r4slpcom = 4;
	break;
	case 5:             //nightlight(on)
	cm_done = 1;
	BleDevStA.r4slppar1 = 0;
	BleDevStA.r4slpcom = 5;
	break;
	case 6:             //power coffee on
	cm_done = 1;
	BleDevStA.r4slppar2 = temp;
	BleDevStA.r4slppar1 = pmod;
	BleDevStA.r4slpcom = 6;
	break;
	case 7:             //lock off
	cm_done = 1;
	BleDevStA.r4slppar1 = 0;
	BleDevStA.r4slpcom = 7;
	break;
	case 8:             //lock on
	cm_done = 1;
	BleDevStA.r4slppar1 = 0;
	BleDevStA.r4slpcom = 8;
	break;
	case 11:             //cooker off
	cm_done = 1;
	BleDevStA.r4slppar1 = 0;		
	BleDevStA.r4slpcom = 10;
	break;
	case 12:             //cooker warming
	cm_done = 1;
	BleDevStA.r4slppar1 = 1;		
	BleDevStA.r4slpcom = 10;
	break;
	case 13:             //cooker set prog
	cm_done = 0;
	BleDevStA.r4slppar1 = pprog;		
	BleDevStA.r4slppar2 = pmod;		
	BleDevStA.r4slppar3 = temp;		
	BleDevStA.r4slppar4 = phour;		
	BleDevStA.r4slppar5 = pmin;		
	BleDevStA.r4slppar6 = warm;		
	BleDevStA.r4slppar7 = dhour;		
	BleDevStA.r4slppar8 = dmin;
	BleDevStA.r4slpcom = 17;
	break;
	case 14:             //cooker warming
	cm_done = 1;
	BleDevStA.r4slppar1 = 1;		
	BleDevStA.r4slpcom = 10;
	break;
	case 18:             //coffee delay start
	cm_done = 1;
	BleDevStA.r4slppar1 = pmod;
	BleDevStA.r4slppar2 = phour;
	BleDevStA.r4slppar3 = pmin;
	BleDevStA.r4slpcom = 18;
	break;
	case 21:             //blight off
	cm_done = 1;
	BleDevStA.r4slppar1 = 0;
	BleDevStA.r4slpcom = 21;
	break;
	case 22:             //blight on
	cm_done = 1;
	BleDevStA.r4slppar1 = 0;
	BleDevStA.r4slpcom = 22;
	break;
	case 23:             //beep off
	cm_done = 1;
	BleDevStA.r4slppar1 = 0;
	BleDevStA.r4slpcom = 23;
	break;
	case 24:             //beep on
	cm_done = 1;
	BleDevStA.r4slppar1 = 0;
	BleDevStA.r4slpcom = 24;
	break;
	case 25:             //boil 1l(on)
	cm_done = 1;
	BleDevStA.r4slppar1 = 0;
	BleDevStA.r4slpcom = 2;
	if (BleDevStA.DEV_TYP > 3) BleDevStA.bCVol = 252;
	break;
	case 63:             //weather calibration
	cm_done = 1;
	BleDevStA.r4slppar1 = 0;		
	BleDevStA.r4slpcom = 63;
	break;
	case 64:             //mi off
	cm_done = 1;
	BleDevStA.r4slppar1 = 0;		
	BleDevStA.r4slpcom = 64;
	break;
	case 65:             //mi boil(on)
	cm_done = 1;
	BleDevStA.r4slppar1 = 0;
	BleDevStA.r4slpcom = 65;
	break;
	case 66:             //mi heat(on)
	cm_done = 1;
	BleDevStA.r4slppar1 = temp;
	BleDevStA.r4slpcom = 66;
	break;
	}

	}
	BleDevStA.r4slpres = 1;
	uint8_t timeout = 35; 	// 100*35 = 3.5 seconds
	while (--timeout && (BleDevStA.r4slpres)) {
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
	if (!test_auth(req)) {
		return ESP_OK;
	}

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
	(BleDevStA.RQC_NAME[0])? strcat(bsend,BleDevStA.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu active' href='cfgdev2'>&#128246;<span class='showmenulabel'> 2 ");
	(BleDevStB.RQC_NAME[0])? strcat(bsend,BleDevStB.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev3'>&#128246;<span class='showmenulabel'> 3 ");
	(BleDevStC.RQC_NAME[0])? strcat(bsend,BleDevStC.RQC_NAME) : strcat(bsend,"Not defined");
	if (ble_mon) strcat(bsend,"</span></a><a class='menu' href='blemon'>&#128246;<span class='showmenulabel'>BLE monitor</span></a>");
	strcat(bsend,"</span></a><a class='menu' href='setting'>&#9881;<span class='showmenulabel'>Setting</span></a>");
	strcat(bsend,"<a class='menu' href='restart'>&#128259;<span class='showmenulabel'>Reboot</span></a>");
	strcat(bsend,"<a class='menu' href='update'>&#10548;<span class='showmenulabel'>Load firmware</span></a></div>");

	strcat(bsend,"</header>");
        HtpDeVHandle(1, bsend);

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
	int8_t bnewBlTime = BleDevStB.bBlTime;
	int  ret;
	int  cm_done = BleDevStB.bProg;
	ret = httpd_req_recv(req,buf1,512);
	if ( ret > 0 ) {
//ESP_LOGI(AP_TAG, "Buf: '%s'", buf1);
/*

in buf1 after httpd_req_recv string like below
swfid=wifiname&swfpsw=wifipassword&smqsrv=192.168.1.10&smqid=esp&
smqpsw=esp&devnam=&rlight=255&glight=255&blight=255&chk2=2

*/
	if (BleDevStB.DEV_TYP < 10) {
	buf3[0] = 0;
	strcpy(buf2,"rlight");
	parsuri(buf1,buf3,buf2,512,4);
	BleDevStB.RgbR = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"glight");
	parsuri(buf1,buf3,buf2,512,4);
	BleDevStB.RgbG = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"blight");
	parsuri(buf1,buf3,buf2,512,4);
	BleDevStB.RgbB = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"sbltim");
	parsuri(buf1,buf3,buf2,512,3);
	bnewBlTime = atoi(buf3);
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
	BleDevStB.r4slppar1 = pmod;		
	BleDevStB.r4slpcom = 1;
	break;
	case 2:             //boil(on)
	cm_done = 1;
	if (bnewBlTime != BleDevStB.bBlTime) {
	cm_done = 0;
	BleDevStB.r4slppar1 = bnewBlTime;
	BleDevStB.r4slpcom = 25;
	} else {
	BleDevStB.r4slppar1 = 0;
	BleDevStB.r4slpcom = 2;
	}
	break;
	case 3:             //heat(on)
	cm_done = 1;
	if (BleDevStB.DEV_TYP == 1) {	
	if (temp < 30) BleDevStB.r4slppar1 = 0;
	else if (temp < 41) BleDevStB.r4slppar1 = 1;
	else if (temp < 56) BleDevStB.r4slppar1 = 2;
	else if (temp < 71) BleDevStB.r4slppar1 = 3;
	else if (temp < 86) BleDevStB.r4slppar1 = 4;
	else BleDevStB.r4slppar1 = 5;
	} else BleDevStB.r4slppar1 = temp;
	if (temp > 29) {
	BleDevStB.r4slppar2 = 1;
	BleDevStB.r4slpcom = 3;
	} else {
	BleDevStB.r4slppar1 = 0;
	BleDevStB.r4slpcom = 1;
	}
	break;
	case 4:             //boil&heat(on)
	cm_done = 1;
	if (BleDevStB.DEV_TYP == 1) {	
	if (temp < 30) BleDevStB.r4slppar1 = 0;
	else if (temp < 41) BleDevStB.r4slppar1 = 1;
	else if (temp < 56) BleDevStB.r4slppar1 = 2;
	else if (temp < 71) BleDevStB.r4slppar1 = 3;
	else if (temp < 86) BleDevStB.r4slppar1 = 4;
	else BleDevStB.r4slppar1 = 5;
	} else BleDevStB.r4slppar1 = temp;
	BleDevStB.r4slpcom = 4;
	break;
	case 5:             //nightlight(on)
	cm_done = 1;
	BleDevStB.r4slppar1 = 0;
	BleDevStB.r4slpcom = 5;
	break;
	case 6:             //power coffee on
	cm_done = 1;
	BleDevStB.r4slppar2 = temp;
	BleDevStB.r4slppar1 = pmod;
	BleDevStB.r4slpcom = 6;
	break;
	case 7:             //lock off
	cm_done = 1;
	BleDevStB.r4slppar1 = 0;
	BleDevStB.r4slpcom = 7;
	break;
	case 8:             //lock on
	cm_done = 1;
	BleDevStB.r4slppar1 = 0;
	BleDevStB.r4slpcom = 8;
	break;
	case 11:             //cooker off
	cm_done = 1;
	BleDevStB.r4slppar1 = 0;		
	BleDevStB.r4slpcom = 10;
	break;
	case 12:             //cooker warming
	cm_done = 1;
	BleDevStB.r4slppar1 = 1;		
	BleDevStB.r4slpcom = 10;
	break;
	case 13:             //cooker set prog
	cm_done = 0;
	BleDevStB.r4slppar1 = pprog;		
	BleDevStB.r4slppar2 = pmod;		
	BleDevStB.r4slppar3 = temp;		
	BleDevStB.r4slppar4 = phour;		
	BleDevStB.r4slppar5 = pmin;		
	BleDevStB.r4slppar6 = warm;		
	BleDevStB.r4slppar7 = dhour;		
	BleDevStB.r4slppar8 = dmin;
	BleDevStB.r4slpcom = 17;
	break;
	case 14:             //cooker warming
	cm_done = 1;
	BleDevStB.r4slppar1 = 1;		
	BleDevStB.r4slpcom = 10;
	break;
	case 18:             //coffee delay start
	cm_done = 1;
	BleDevStB.r4slppar1 = pmod;
	BleDevStB.r4slppar2 = phour;
	BleDevStB.r4slppar3 = pmin;
	BleDevStB.r4slpcom = 18;
	break;
	case 21:             //blight off
	cm_done = 1;
	BleDevStB.r4slppar1 = 0;
	BleDevStB.r4slpcom = 21;
	break;
	case 22:             //blight on
	cm_done = 1;
	BleDevStB.r4slppar1 = 0;
	BleDevStB.r4slpcom = 22;
	break;
	case 23:             //beep off
	cm_done = 1;
	BleDevStB.r4slppar1 = 0;
	BleDevStB.r4slpcom = 23;
	break;
	case 24:             //beep on
	cm_done = 1;
	BleDevStB.r4slppar1 = 0;
	BleDevStB.r4slpcom = 24;
	break;
	case 25:             //boil 1l(on)
	cm_done = 1;
	BleDevStB.r4slppar1 = 0;
	BleDevStB.r4slpcom = 2;
	if (BleDevStB.DEV_TYP > 3) BleDevStB.bCVol = 252;
	break;
	case 63:             //weather calibration
	cm_done = 1;
	BleDevStB.r4slppar1 = 0;		
	BleDevStB.r4slpcom = 63;
	break;
	case 64:             //mi off
	cm_done = 1;
	BleDevStB.r4slppar1 = 0;		
	BleDevStB.r4slpcom = 64;
	break;
	case 65:             //mi boil(on)
	cm_done = 1;
	BleDevStB.r4slppar1 = 0;
	BleDevStB.r4slpcom = 65;
	break;
	case 66:             //mi heat(on)
	cm_done = 1;
	BleDevStB.r4slppar1 = temp;
	BleDevStB.r4slpcom = 66;
	break;
	}

	}
	BleDevStB.r4slpres = 1;
	uint8_t timeout = 35; 	// 100*35 = 3.5 seconds
	while (--timeout && (BleDevStB.r4slpres)) {
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
	if (!test_auth(req)) {
		return ESP_OK;
	}

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
	(BleDevStA.RQC_NAME[0])? strcat(bsend,BleDevStA.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev2'>&#128246;<span class='showmenulabel'> 2 ");
	(BleDevStB.RQC_NAME[0])? strcat(bsend,BleDevStB.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu active' href='cfgdev3'>&#128246;<span class='showmenulabel'> 3 ");
	(BleDevStC.RQC_NAME[0])? strcat(bsend,BleDevStC.RQC_NAME) : strcat(bsend,"Not defined");
	if (ble_mon) strcat(bsend,"</span></a><a class='menu' href='blemon'>&#128246;<span class='showmenulabel'>BLE monitor</span></a>");
	strcat(bsend,"</span></a><a class='menu' href='setting'>&#9881;<span class='showmenulabel'>Setting</span></a>");
	strcat(bsend,"<a class='menu' href='restart'>&#128259;<span class='showmenulabel'>Reboot</span></a>");
	strcat(bsend,"<a class='menu' href='update'>&#10548;<span class='showmenulabel'>Load firmware</span></a></div>");

	strcat(bsend,"</header>");
        HtpDeVHandle(2, bsend);

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
	int8_t bnewBlTime = BleDevStC.bBlTime;
	int  ret;
	int  cm_done = BleDevStC.bProg;
	ret = httpd_req_recv(req,buf1,512);
	if ( ret > 0 ) {
//ESP_LOGI(AP_TAG, "Buf: '%s'", buf1);
/*

in buf1 after httpd_req_recv string like below
swfid=wifiname&swfpsw=wifipassword&smqsrv=192.168.1.10&smqid=esp&
smqpsw=esp&devnam=&rlight=255&glight=255&blight=255&chk2=2

*/
	if (BleDevStC.DEV_TYP < 10) {
	buf3[0] = 0;
	strcpy(buf2,"rlight");
	parsuri(buf1,buf3,buf2,512,4);
	BleDevStC.RgbR = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"glight");
	parsuri(buf1,buf3,buf2,512,4);
	BleDevStC.RgbG = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"blight");
	parsuri(buf1,buf3,buf2,512,4);
	BleDevStC.RgbB = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"sbltim");
	parsuri(buf1,buf3,buf2,512,3);
	bnewBlTime = atoi(buf3);
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
	BleDevStC.r4slppar1 = pmod;		
	BleDevStC.r4slpcom = 1;
	break;
	case 2:             //boil(on)
	if (bnewBlTime != BleDevStC.bBlTime) {
	cm_done = 0;
	BleDevStC.r4slppar1 = bnewBlTime;
	BleDevStC.r4slpcom = 25;
	} else {
	cm_done = 1;
	BleDevStC.r4slppar1 = 0;
	BleDevStC.r4slpcom = 2;
	}
	break;
	case 3:             //heat(on)
	cm_done = 1;
	if (BleDevStC.DEV_TYP == 1) {	
	if (temp < 30) BleDevStC.r4slppar1 = 0;
	else if (temp < 41) BleDevStC.r4slppar1 = 1;
	else if (temp < 56) BleDevStC.r4slppar1 = 2;
	else if (temp < 71) BleDevStC.r4slppar1 = 3;
	else if (temp < 86) BleDevStC.r4slppar1 = 4;
	else BleDevStC.r4slppar1 = 5;
	} else BleDevStC.r4slppar1 = temp;
	if (temp > 29) {
	BleDevStC.r4slppar2 = 1;
	BleDevStC.r4slpcom = 3;
	} else {
	BleDevStC.r4slppar1 = 0;
	BleDevStC.r4slpcom = 1;
	}
	break;
	case 4:             //boil&heat(on)
	cm_done = 1;
	if (BleDevStC.DEV_TYP == 1) {	
	if (temp < 30) BleDevStC.r4slppar1 = 0;
	else if (temp < 41) BleDevStC.r4slppar1 = 1;
	else if (temp < 56) BleDevStC.r4slppar1 = 2;
	else if (temp < 71) BleDevStC.r4slppar1 = 3;
	else if (temp < 86) BleDevStC.r4slppar1 = 4;
	else BleDevStC.r4slppar1 = 5;
	} else BleDevStC.r4slppar1 = temp;
	BleDevStC.r4slpcom = 4;
	break;
	case 5:             //nightlight(on)
	cm_done = 1;
	BleDevStC.r4slppar1 = 0;
	BleDevStC.r4slpcom = 5;
	break;
	case 6:             //power coffee on
	cm_done = 1;
	BleDevStC.r4slppar2 = temp;
	BleDevStC.r4slppar1 = pmod;
	BleDevStC.r4slpcom = 6;
	break;
	case 7:             //lock off
	cm_done = 1;
	BleDevStC.r4slppar1 = 0;
	BleDevStC.r4slpcom = 7;
	break;
	case 8:             //lock on
	cm_done = 1;
	BleDevStC.r4slppar1 = 0;
	BleDevStC.r4slpcom = 8;
	break;
	case 11:             //cooker off
	cm_done = 1;
	BleDevStC.r4slppar1 = 0;		
	BleDevStC.r4slpcom = 10;
	break;
	case 12:             //cooker warming
	cm_done = 1;
	BleDevStC.r4slppar1 = 1;		
	BleDevStC.r4slpcom = 10;
	break;
	case 13:             //cooker set prog
	cm_done = 0;
	BleDevStC.r4slppar1 = pprog;		
	BleDevStC.r4slppar2 = pmod;		
	BleDevStC.r4slppar3 = temp;		
	BleDevStC.r4slppar4 = phour;		
	BleDevStC.r4slppar5 = pmin;		
	BleDevStC.r4slppar6 = warm;		
	BleDevStC.r4slppar7 = dhour;		
	BleDevStC.r4slppar8 = dmin;
	BleDevStC.r4slpcom = 17;
	break;
	case 14:             //cooker warming
	cm_done = 1;
	BleDevStC.r4slppar1 = 1;		
	BleDevStC.r4slpcom = 10;
	break;
	case 18:             //coffee delay start
	cm_done = 1;
	BleDevStC.r4slppar1 = pmod;
	BleDevStC.r4slppar2 = phour;
	BleDevStC.r4slppar3 = pmin;
	BleDevStC.r4slpcom = 18;
	break;
	case 21:             //blight off
	cm_done = 1;
	BleDevStC.r4slppar1 = 0;
	BleDevStC.r4slpcom = 21;
	break;
	case 22:             //blight on
	cm_done = 1;
	BleDevStC.r4slppar1 = 0;
	BleDevStC.r4slpcom = 22;
	break;
	case 23:             //beep off
	cm_done = 1;
	BleDevStC.r4slppar1 = 0;
	BleDevStC.r4slpcom = 23;
	break;
	case 24:             //beep on
	cm_done = 1;
	BleDevStC.r4slppar1 = 0;
	BleDevStC.r4slpcom = 24;
	break;
	case 25:             //boil 1l(on)
	cm_done = 1;
	BleDevStC.r4slppar1 = 0;
	BleDevStC.r4slpcom = 2;
	if (BleDevStC.DEV_TYP > 3) BleDevStC.bCVol = 252;
	break;
	case 63:             //weather calibration
	cm_done = 1;
	BleDevStC.r4slppar1 = 0;		
	BleDevStC.r4slpcom = 63;
	break;
	case 64:             //mi off
	cm_done = 1;
	BleDevStC.r4slppar1 = 0;		
	BleDevStC.r4slpcom = 64;
	break;
	case 65:             //mi boil(on)
	cm_done = 1;
	BleDevStC.r4slppar1 = 0;
	BleDevStC.r4slpcom = 65;
	break;
	case 66:             //mi heat(on)
	cm_done = 1;
	BleDevStC.r4slppar1 = temp;
	BleDevStC.r4slpcom = 66;
	break;
	}

	}
	BleDevStC.r4slpres = 1;
	uint8_t timeout = 35; 	// 100*35 = 3.5 seconds
	while (--timeout && (BleDevStC.r4slpres)) {
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

/* HTTP blemon handler */
static esp_err_t pblemon_get_handler(httpd_req_t *req)
{
	char bsend[14000];
        char buff[128];
	strcpy(bsend,"<!DOCTYPE html><html>");
	strcat(bsend,"<head><title>r4sGate</title>");
        strcat(bsend,"<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
        if (ble_mon_refr) strcat(bsend,"<meta http-equiv=\"refresh\" content=\"5\">");
        strcat(bsend,cssDatasheet);
	strcat(bsend,"</head><body class='bodymenu'><header class='headermenu'><h1>ESP32 r4sGate");
	if (R4SNUM)  {
	itoa(R4SNUM,buff,10);
	strcat(bsend, buff);
	}
	strcat(bsend," for Redmond +</h1>");
	strcat(bsend,"<div class='menubar'><a class='menu' href='.'>&#8962;<span class='showmenulabel'>Main");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev1'>&#128246;<span class='showmenulabel'> 1 ");
	(BleDevStA.RQC_NAME[0])? strcat(bsend,BleDevStA.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev2'>&#128246;<span class='showmenulabel'> 2 ");
	(BleDevStB.RQC_NAME[0])? strcat(bsend,BleDevStB.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev3'>&#128246;<span class='showmenulabel'> 3 ");
	(BleDevStC.RQC_NAME[0])? strcat(bsend,BleDevStC.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu active' href='blemon'>&#128246;<span class='showmenulabel'>BLE monitor</span></a>");
	strcat(bsend,"</span></a><a class='menu' href='setting'>&#9881;<span class='showmenulabel'>Setting</span></a>");
	strcat(bsend,"<a class='menu' href='restart'>&#128259;<span class='showmenulabel'>Reboot</span></a>");
	strcat(bsend,"<a class='menu' href='update'>&#10548;<span class='showmenulabel'>Load firmware</span></a></div>");

	strcat(bsend,"</header><body><form method=\"POST\" action=\"/blemonok\"  id=\"frm1\"><table class='normal' width='80%'>");
	strcat(bsend,"<tr class=\"header\"><th width='50px' align='left'>Pos</th><th width='200px' align='left'>ID</th><th width='150px' align='left'>Name</th><th width='70px' align='left''>RSSI</th><th width='70px' align='left''>Gap</th><th width='70px' align='left''>Last</th><th width='700px' align='left'>Advanced Data / Scan Response</th><th width='150px' align='left''>Timeout</tr>");
	for (int i=0; i < BleMonNum; i++) {
	(i & 1)? strcat(bsend,"<tr><td class='xbg'>") : strcat(bsend,"<tr><td>");
	itoa(i+1,buff,10);
	strcat(bsend,buff);	
	(i & 1)? strcat(bsend,"</td><td class='xbg'>") : strcat(bsend,"</td><td>");
	buff[0] = 0;
	if (BleMR[i].id == 2) {
	strcat(bsend,"uuid: ");	
	bin2hex(BleMR[i].mac,buff,4,0);
	strcat(bsend,buff);	
	strcat(bsend,"-");	
	bin2hex(&BleMR[i].mac[4],buff,2,0);
	strcat(bsend,buff);	
	strcat(bsend,"-");	
	} else if (BleMR[i].id != 2) {
	strcat(bsend,"mac: ");	
	bin2hex(BleMR[i].mac,buff,6,0x3a);
	strcat(bsend,buff);	
	}
	(i & 1)? strcat(bsend,"</td><td class='xbg'>") : strcat(bsend,"</td><td>");
	memset(buff,0,16);
	mystrcpy(buff,BleMX[i].name,15);
	strcat(bsend,buff);	
	(i & 1)? strcat(bsend,"</td><td class='xbg'>") : strcat(bsend,"</td><td>");
	if (BleMX[i].rssi) {
	itoa(BleMX[i].rssi,buff,10);
	strcat(bsend,buff);	
	strcat(bsend," dB");
	}
	(i & 1)? strcat(bsend,"</td><td class='xbg'>") : strcat(bsend,"</td><td>");
	if (BleMX[i].rssi) {
	itoa(BleMX[i].mto / 10,buff,10);
	strcat(bsend,buff);	
	strcat(bsend,".");
	itoa(BleMX[i].mto % 10,buff,10);
	strcat(bsend,buff);	
	strcat(bsend," s");
	}
	(i & 1)? strcat(bsend,"</td><td class='xbg'>") : strcat(bsend,"</td><td>");
	if (BleMX[i].rssi) {
	uint16_t tmp = 0;
	if(BleMR[i].sto) tmp = BleMR[i].sto - BleMX[i].ttick;
        else tmp = BleMonDefTO - BleMX[i].ttick;
	itoa(tmp / 10,buff,10);
	strcat(bsend,buff);	
	strcat(bsend,".");
	itoa(tmp % 10,buff,10);
	strcat(bsend,buff);	
	strcat(bsend," s");
	}
	(i & 1)? strcat(bsend,"</td><td class='xbg'>") : strcat(bsend,"</td><td>");
	if(BleMX[i].advdatlen) {
	bin2hex(BleMX[i].advdat,buff,BleMX[i].advdatlen & 0x1f,0x20);
	strcat(bsend,buff);	
	}
	(i & 1)? strcat(bsend,"</td><td class='xbg' rowspan='2'>") : strcat(bsend,"</td><td rowspan='2'>");

	strcat(bsend,"<input name=\"blmto");
	itoa(i,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" type=\"number\" value=\"");
	itoa(BleMR[i].sto / 10,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"6500\" size=\"4\">s");

	(i & 1)? strcat(bsend,"</td></tr><tr><td class='xbg'></td><td class='xbg'>") : strcat(bsend,"</td></tr><tr><td></td><td>");
	if (BleMR[i].id == 3) {
	strcat(bsend,"LYWSD03MMC");	
	} else if (BleMR[i].id == 2) {
	bin2hex(&BleMR[i].mac[6],buff,2,0);
	strcat(bsend,buff);	
	strcat(bsend,"-");	
	bin2hex(&BleMR[i].mac[8],buff,2,0);
	strcat(bsend,buff);	
	strcat(bsend,"-");	
	bin2hex(&BleMR[i].mac[10],buff,6,0);
	strcat(bsend,buff);	
	}
	(i & 1)? strcat(bsend,"</td><td class='xbg'></td><td class='xbg'></td><td class='xbg'></td><td class='xbg'></td><td class='xbg'>") : strcat(bsend,"</td><td></td><td></td><td></td><td></td><td>");


	if(BleMX[i].scrsplen) {
	bin2hex(BleMX[i].scrsp,buff,BleMX[i].scrsplen & 0x1f,0x20);
	strcat(bsend,buff);	
	}
        strcat(bsend,"</td></tr>");
	}
	strcat(bsend,"<tr class=\"header\"><th align='left'></th><th align='left'>Auto Refresh</th><th align='left'>");
	strcat(bsend,"<select name=\"blmarfr\"><option ");
        strcat(bsend,"value=\"0\">Off</option><option ");
	if (ble_mon_refr) strcat(bsend,"selected ");
	strcat(bsend,"value=\"1\">On</option></select></th><th align='left'></th><th align='left'></th><th align='left'></th><th align='left'>");
        (IsPassiveScan)? strcat(bsend,"Passive mode") : strcat(bsend,"Active mode");

	strcat(bsend,"</th><th align='left''></tr>");
        strcat(bsend,"</table></br>");

	strcat(bsend,"</form><form method=\"POST\" action=\"/setignore\">");
        strcat(bsend,"<input type=\"button\" value =\"Refresh\" onclick=\"window.location.reload(true);\" />");
	strcat(bsend,"<input type=SUBMIT value=\"Ok\" form=\"frm1\"><input type=SUBMIT value=\"Cancel\"></form></body></html>");

//itoa(strlen(bsend),buff,10);
//strcat(bsend,buff);
/*
strcat(bsend,"\r\n");
itoa(sizeof(BleMR),buff,10);
strcat(bsend,buff);
strcat(bsend,"\r\n");
itoa(sizeof(BleMX),buff,10);
strcat(bsend,buff);
*/

        httpd_resp_sendstr(req, bsend);
    return ESP_OK;
}

static const httpd_uri_t pblemon = {
	.uri       = "/blemon",
	.method    = HTTP_GET,
	.handler   = pblemon_get_handler,
	.user_ctx  = NULL
};

static esp_err_t pblemonok_get_handler(httpd_req_t *req)
{
	char buf1[512] = {0};
	char buf2[16] = {0};
	char buf3[16] = {0};
	int  ret;
	ret = httpd_req_recv(req,buf1,512);
	if ( ret > 0 ) {
//ESP_LOGI(AP_TAG, "Buf: '%s'", buf1);
/*

in buf1 after httpd_req_recv string like below
swfid=wifiname&swfpsw=wifipassword&smqsrv=192.168.1.10&smqid=esp&
smqpsw=esp&devnam=&rlight=255&glight=255&blight=255&chk2=2

*/
	for (int i=0; i < BleMonNum; i++) {
	strcpy(buf2,"blmto");
	itoa(i,buf3,10);
	strcat(buf2,buf3);
	buf3[0] = 0;
	parsuri(buf1,buf3,buf2,512,5);
	if (buf3[0]) BleMR[i].sto = atoi(buf3) * 10;
	}
	buf3[0] = 0;
	strcpy(buf2,"blmarfr");
	parsuri(buf1,buf3,buf2,512,2);
	ble_mon_refr = atoi(buf3);
	}
	httpd_resp_set_status(req, "303 See Other");
	httpd_resp_set_hdr(req, "Location", "/blemon");
	httpd_resp_send(req, NULL, 0);  // Response body can be empty
    return ESP_OK;
}
static const httpd_uri_t pblemonok = {
	.uri       = "/blemonok",
	.method    = HTTP_POST,
	.handler   = pblemonok_get_handler,
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


void HtpDeVSett(uint8_t blenum, char* bsend) {
	uint8_t blenum1 = blenum + 1;
	char buff[16]; 
	if (blenum > 2) return;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	itoa(blenum1,buff,10);
	strcat(bsend,buff);
	strcat(bsend," <input name=\"sreqnm");
	strcpy(buff,"a");
	buff[0] = buff[0] + blenum;
	strcat(bsend,buff);
	strcat(bsend,"\" value=\"");
	if (ptr->REQ_NAME[0]) strcat(bsend,ptr->REQ_NAME);
	strcat(bsend,"\" size=\"15\">Name/Address &emsp;<select name=\"sreqtp");
	strcpy(buff,"a");
	buff[0] = buff[0] + blenum;
	strcat(bsend,buff);
	strcat(bsend,"\"><option ");
	if (ptr->DEV_TYP == 0) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Not defined</option><option ");
	if (ptr->DEV_TYP == 1) strcat(bsend,"selected ");
        strcat(bsend,"value=\"1\">RK-M170S</option><option ");
	if (ptr->DEV_TYP == 2) strcat(bsend,"selected ");
        strcat(bsend,"value=\"2\">RK-M173S</option><option ");
	if (ptr->DEV_TYP == 3) strcat(bsend,"selected ");
        strcat(bsend,"value=\"3\">RK-G200(old)</option><option ");
	if (ptr->DEV_TYP == 4) strcat(bsend,"selected ");
	strcat(bsend,"value=\"4\">RK-G200S</option><option ");
	if (ptr->DEV_TYP == 5) strcat(bsend,"selected ");
	strcat(bsend,"value=\"5\">RK-G240S</option><option ");
	if (ptr->DEV_TYP == 10) strcat(bsend,"selected ");
	strcat(bsend,"value=\"10\">RSP-103S</option><option ");
	if (ptr->DEV_TYP == 11) strcat(bsend,"selected ");
	strcat(bsend,"value=\"11\">RCH-7001S</option><option ");
	if (ptr->DEV_TYP == 12) strcat(bsend,"selected ");
	strcat(bsend,"value=\"12\">RCM-M1519S</option><option ");
	if (ptr->DEV_TYP == 16) strcat(bsend,"selected ");
	strcat(bsend,"value=\"16\">RMC-M800S</option><option ");
	if (ptr->DEV_TYP == 17) strcat(bsend,"selected ");
	strcat(bsend,"value=\"17\">RMC-M903S</option><option ");
	if (ptr->DEV_TYP == 18) strcat(bsend,"selected ");
	strcat(bsend,"value=\"18\">RMC-M224S</option><option ");
	if (ptr->DEV_TYP == 19) strcat(bsend,"selected ");
	strcat(bsend,"value=\"19\">RMC-M961S</option><option ");
	if (ptr->DEV_TYP == 20) strcat(bsend,"selected ");
	strcat(bsend,"value=\"20\">RMC-M92S</option><option ");
	if (ptr->DEV_TYP == 24) strcat(bsend,"selected ");
	strcat(bsend,"value=\"24\">RO-5707S</option><option ");
	if (ptr->DEV_TYP == 63) strcat(bsend,"selected ");
	strcat(bsend,"value=\"63\">RSC-51S</option><option ");
	if (ptr->DEV_TYP == 64) strcat(bsend,"selected ");
	strcat(bsend,"value=\"64\">YM-K1501(Int)</option><option ");
	if (ptr->DEV_TYP == 65) strcat(bsend,"selected ");
	strcat(bsend,"value=\"65\">YM-K1501(HK)</option><option ");
	if (ptr->DEV_TYP == 66) strcat(bsend,"selected ");
	strcat(bsend,"value=\"66\">V-SK152(Int)</option><option ");
	if (ptr->DEV_TYP == 95) strcat(bsend,"selected ");
	strcat(bsend,"value=\"95\">Mi-Unknown</option></select>Type/");
	if (ptr->bEfficiency < 10) strcat(bsend,"0");
	itoa(ptr->bEfficiency,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"&percnt;&emsp;Light<input name=\"rlight");
	strcpy(buff,"a");
	buff[0] = buff[0] + blenum;
	strcat(bsend,buff);
	strcat(bsend,"\" type=\"number\" value=\"");
	itoa(ptr->RgbR,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"255\" size=\"3\">Red &emsp;<input name=\"glight");
	strcpy(buff,"a");
	buff[0] = buff[0] + blenum;
	strcat(bsend,buff);
	strcat(bsend,"\" type=\"number\" value=\"");
	itoa(ptr->RgbG,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"255\" size=\"3\">Green &emsp;<input name=\"blight");
	strcpy(buff,"a");
	buff[0] = buff[0] + blenum;
	strcat(bsend,buff);
	strcat(bsend,"\" type=\"number\" value=\"");
	itoa(ptr->RgbB,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"255\" size=\"3\">Blue &emsp;<input name=\"ltemp");
	strcpy(buff,"a");
	buff[0] = buff[0] + blenum;
	strcat(bsend,buff);
	strcat(bsend,"\" type=\"number\" value=\"");
	itoa(ptr->bLtemp,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"95\" size=\"3\">Heat</br>");
}



/* HTTP GET setting handler */
static esp_err_t psetting_get_handler(httpd_req_t *req)
{
	if (!test_auth(req)) {
		return ESP_OK;
	}

	char bsend[14200];
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
	(BleDevStA.RQC_NAME[0])? strcat(bsend,BleDevStA.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev2'>&#128246;<span class='showmenulabel'> 2 ");
	(BleDevStB.RQC_NAME[0])? strcat(bsend,BleDevStB.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev3'>&#128246;<span class='showmenulabel'> 3 ");
	(BleDevStC.RQC_NAME[0])? strcat(bsend,BleDevStC.RQC_NAME) : strcat(bsend,"Not defined");
	if (ble_mon) strcat(bsend,"</span></a><a class='menu' href='blemon'>&#128246;<span class='showmenulabel'>BLE monitor</span></a>");
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
	strcat(bsend,"\"size=\"32\">Server &emsp;<input name=\"smqprt\" type=\"number\" value=\"");
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
	strcat(bsend,"<h3>Devices Setting</h3><br/>");

	HtpDeVSett(0, bsend);
	HtpDeVSett(1, bsend);
	HtpDeVSett(2, bsend);

	strcat(bsend, "<h3>System Setting</h3><br/>");

	strcat(bsend, "<input name=\"auth\" type=\"text\" value=\"");
	if (AUTH_BASIC[0])
		strcat(bsend, AUTH_BASIC);
	strcat(bsend, "\" size=\"15\">Basic Auth &emsp;");

	strcat(bsend, "<input name=\"r4snum\" type=\"number\" value=\"");
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


	strcat(bsend,"<select name=\"chk5\"><option ");
	if (ble_mon == 0) strcat(bsend,"selected ");
        strcat(bsend,"value=\"0\">Off</option><option ");
	if (ble_mon == 1) strcat(bsend,"selected ");
        strcat(bsend,"value=\"1\">Passive</option><option ");
	if (ble_mon > 1) strcat(bsend,"selected ");
        strcat(bsend,"value=\"2\">Active</option></select> BLE monitoring &emsp;");
	strcat(bsend,"<input type=\"checkbox\" name=\"mqtdel\" value=\"1\"");
	strcat(bsend,"> Delete Mqtt topics &emsp;");
	strcat(bsend,"<input type=\"checkbox\" name=\"chk0\" value=\"0\"");
	strcat(bsend,"> Format NVS area</br>");
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
        strcat(bsend,"value=\"128\">In</option><option ");
	if ((bgpio5 & 0xc0) == 0xc0) strcat(bsend,"selected ");
        strcat(bsend,"value=\"192\">Jpg</option></select><br>");

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
	if (!test_auth(req)) {
		return ESP_OK;
	}

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
	parsuri(buf1,MQTT_SERVER,buf2,1024,32);
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
	parsuri(buf1,BleDevStA.REQ_NAME,buf2,1024,16);
	strcpy(buf2,"sreqnmb");
	parsuri(buf1,BleDevStB.REQ_NAME,buf2,1024,16);
	strcpy(buf2,"sreqnmc");
	parsuri(buf1,BleDevStC.REQ_NAME,buf2,1024,16);
	strcpy(buf2,"rlighta");
	parsuri(buf1,buf3,buf2,1024,4);
	BleDevStA.RgbR = atoi(buf3);
	strcpy(buf2,"glighta");
	parsuri(buf1,buf3,buf2,1024,4);
	BleDevStA.RgbG = atoi(buf3);
	strcpy(buf2,"blighta");
	parsuri(buf1,buf3,buf2,1024,4);
	BleDevStA.RgbB = atoi(buf3);
	strcpy(buf2,"rlightb");
	parsuri(buf1,buf3,buf2,1024,4);
	BleDevStB.RgbR = atoi(buf3);
	strcpy(buf2,"glightb");
	parsuri(buf1,buf3,buf2,1024,4);
	BleDevStB.RgbG = atoi(buf3);
	strcpy(buf2,"blightb");
	parsuri(buf1,buf3,buf2,1024,4);
	BleDevStB.RgbB = atoi(buf3);
	strcpy(buf2,"rlightc");
	parsuri(buf1,buf3,buf2,1024,4);
	BleDevStC.RgbR = atoi(buf3);
	strcpy(buf2,"glightc");
	parsuri(buf1,buf3,buf2,1024,4);
	BleDevStC.RgbG = atoi(buf3);
	strcpy(buf2,"blightc");
	parsuri(buf1,buf3,buf2,1024,4);
	BleDevStC.RgbB = atoi(buf3);
	strcpy(buf2,"ltempa");
	parsuri(buf1,buf3,buf2,1024,4);
	BleDevStA.bLtemp = atoi(buf3);
	strcpy(buf2,"ltempb");
	parsuri(buf1,buf3,buf2,1024,4);
	BleDevStB.bLtemp = atoi(buf3);
	strcpy(buf2,"ltempc");
	parsuri(buf1,buf3,buf2,1024,4);
	BleDevStC.bLtemp = atoi(buf3);
	strcpy(buf2,"sreqtpa");
	parsuri(buf1,buf3,buf2,1024,4);
	BleDevStA.DEV_TYP = atoi(buf3);
	strcpy(buf2,"sreqtpb");
	parsuri(buf1,buf3,buf2,1024,4);
	BleDevStB.DEV_TYP = atoi(buf3);
	strcpy(buf2,"sreqtpc");
	parsuri(buf1,buf3,buf2,1024,4);
	BleDevStC.DEV_TYP = atoi(buf3);
	strcpy(buf2,"sjpgtim");
	parsuri(buf1,buf3,buf2,1024,8);
	jpg_time = atoi(buf3);
	strcpy(buf2,"smqprt");
	parsuri(buf1,buf3,buf2,1024,8);
	mqtt_port = atoi(buf3);
	strcpy(buf2,"timzon");
	parsuri(buf1,buf3,buf2,1024,4);
	TimeZone = atoi(buf3);
	strcpy(buf2, "auth");
	parsuri(buf1, AUTH_BASIC, buf2, 1024, 50);
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
	strcpy(buf2,"chk5");
	parsuri(buf1,buf3,buf2,1024,2);
	ble_mon = atoi(buf3);
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
	f_update = true;
	while (BleDevStA.btauthoriz || BleDevStB.btauthoriz || BleDevStC.btauthoriz) vTaskDelay(200 / portTICK_PERIOD_MS);
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
	strcpy(buf1,"homeassistant/select/");
	strcat(buf1,buf2);
	strcat(buf1,"/#");
	mqtdel = 20;
	esp_mqtt_client_subscribe(mqttclient, buf1, 0);
	while (--mqtdel > 1) vTaskDelay(20 / portTICK_PERIOD_MS);
	strcpy(buf1,"homeassistant/number/");
	strcat(buf1,buf2);
	strcat(buf1,"/#");
	mqtdel = 20;
	esp_mqtt_client_subscribe(mqttclient, buf1, 0);
	while (--mqtdel > 1) vTaskDelay(20 / portTICK_PERIOD_MS);
	strcpy(buf1,"homeassistant/device_tracker/");
	strcat(buf1,buf2);
	strcat(buf1,"/#");
	mqtdel = 20;
	esp_mqtt_client_subscribe(mqttclient, buf1, 0);
	while (--mqtdel > 1) vTaskDelay(20 / portTICK_PERIOD_MS);
	}
	strcpy(buf1,buf2);
	strcat(buf1,"/#");
	mqtdel = 20;
	esp_mqtt_client_subscribe(mqttclient, buf1, 0);
	while (--mqtdel > 1) vTaskDelay(20 / portTICK_PERIOD_MS);

	if (R4SNUM !=R4SNUMO) {
	strcpy(buf2, "r4s");
	if (R4SNUMO)  {
	itoa(R4SNUMO,buf3,10);
	strcat(buf2, buf3);
	}
	f_update = true;
	while (BleDevStA.btauthoriz || BleDevStB.btauthoriz || BleDevStC.btauthoriz) vTaskDelay(200 / portTICK_PERIOD_MS);
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
	strcpy(buf1,"homeassistant/select/");
	strcat(buf1,buf2);
	strcat(buf1,"/#");
	mqtdel = 20;
	esp_mqtt_client_subscribe(mqttclient, buf1, 0);
	while (--mqtdel > 1) vTaskDelay(20 / portTICK_PERIOD_MS);
	strcpy(buf1,"homeassistant/number/");
	strcat(buf1,buf2);
	strcat(buf1,"/#");
	mqtdel = 20;
	esp_mqtt_client_subscribe(mqttclient, buf1, 0);
	while (--mqtdel > 1) vTaskDelay(20 / portTICK_PERIOD_MS);
	strcpy(buf1,"homeassistant/device_tracker/");
	strcat(buf1,buf2);
	strcat(buf1,"/#");
	mqtdel = 20;
	esp_mqtt_client_subscribe(mqttclient, buf1, 0);
	while (--mqtdel > 1) vTaskDelay(20 / portTICK_PERIOD_MS);
	}
	strcpy(buf1,buf2);
	strcat(buf1,"/#");
	mqtdel = 20;
	esp_mqtt_client_subscribe(mqttclient, buf1, 0);
	while (--mqtdel > 1) vTaskDelay(20 / portTICK_PERIOD_MS);

	}
	}


// write nvs
	nvs_handle_t my_handle;
	ret = nvs_open("storage", NVS_READWRITE, &my_handle);
	if (ret == ESP_OK) {
	nvs_set_str(my_handle, "auth", AUTH_BASIC);
	nvs_set_u16(my_handle, "sjpgtim", jpg_time);
	nvs_set_u16(my_handle, "smqprt", mqtt_port);
	nvs_set_u8(my_handle,  "rlighta", BleDevStA.RgbR);
	nvs_set_u8(my_handle,  "glighta", BleDevStA.RgbG);
	nvs_set_u8(my_handle,  "blighta", BleDevStA.RgbB);
	nvs_set_u8(my_handle,  "rlightb", BleDevStB.RgbR);
	nvs_set_u8(my_handle,  "glightb", BleDevStB.RgbG);
	nvs_set_u8(my_handle,  "blightb", BleDevStB.RgbB);
	nvs_set_u8(my_handle,  "rlightc", BleDevStC.RgbR);
	nvs_set_u8(my_handle,  "glightc", BleDevStC.RgbG);
	nvs_set_u8(my_handle,  "blightc", BleDevStC.RgbB);
	nvs_set_u8(my_handle,  "ltempa", BleDevStA.bLtemp);
	nvs_set_u8(my_handle,  "ltempb", BleDevStB.bLtemp);
	nvs_set_u8(my_handle,  "ltempc", BleDevStC.bLtemp);
	nvs_set_u8(my_handle, "effica", BleDevStA.bEfficiency);
	nvs_set_u8(my_handle, "efficb", BleDevStB.bEfficiency);
	nvs_set_u8(my_handle, "efficc", BleDevStC.bEfficiency);
	nvs_set_u8(my_handle,  "sreqtpa", BleDevStA.DEV_TYP);
	nvs_set_u8(my_handle,  "sreqtpb", BleDevStB.DEV_TYP);
	nvs_set_u8(my_handle,  "sreqtpc", BleDevStC.DEV_TYP);
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
	nvs_set_str(my_handle, "sreqnma", BleDevStA.REQ_NAME);
	nvs_set_str(my_handle, "sreqnmb", BleDevStB.REQ_NAME);
	nvs_set_str(my_handle, "sreqnmc", BleDevStC.REQ_NAME);
	nvs_set_blob(my_handle,"sblemd",  BleMR, sizeof(BleMR));
	nvs_set_str(my_handle, "smtopp1", MQTT_TOPP1);
	nvs_set_str(my_handle, "smtopp2", MQTT_TOPP2);
	nvs_set_str(my_handle, "smtopp3", MQTT_TOPP3);
	nvs_set_str(my_handle, "smtopp4", MQTT_TOPP4);
	nvs_set_str(my_handle, "smtopp5", MQTT_TOPP5);
	nvs_set_str(my_handle, "smtopp6", MQTT_TOPP6);
	nvs_set_str(my_handle, "smtopp7", MQTT_TOPP7);
	nvs_set_str(my_handle, "smjpuri", MyHttpUri);
        ret = nvs_commit(my_handle);
	if (ret != ESP_OK) {
	ESP_LOGE(AP_TAG, "NVS write error");
	}
// Close nvs
	nvs_close(my_handle);
	}
	}
	strcpy(buf1,"<!DOCTYPE html><html><head><title>r4sGate</title><meta name='viewport' content='width=device-width, initial-scale=1.0'>");
        strcat(buf1, "<meta http-equiv=\"refresh\" content=\"3;URL=http://");
        strcat(buf1, bufip);
        strcat(buf1,"\">");
        strcat(buf1, "</head><body>Setting saved. Rebooting...</body></html>");
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
	if (!test_auth(req)) {
		return ESP_OK;
	}

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
	strcpy(buf1,"<!DOCTYPE html><html><head><title>r4sGate</title><meta name='viewport' content='width=device-width, initial-scale=1.0'><meta http-equiv=\"refresh\" content=\"3;URL=http://");
        strcat(buf1, bufip);
	strcat(buf1,"\"></head><body>Rebooting...</body></html>");
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
	if (!test_auth(req)) {
		return ESP_OK;
	}

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
	if (!test_auth(req)) {
		return ESP_OK;
	}

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
esp_log_buffer_hex(AP_TAG, otabuf, 256);
ESP_LOGI(AP_TAG, "Buff: %s",otabuf);

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
	if (!otabufoffs) otabufoffs = parsoff(otabuf,"application/macbinary\r\n\r\n", otabufsize);
	if (!otabufoffs) {
	ESP_LOGE(AP_TAG, "application/octet-stream or application/macbinary not found");
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
	ESP_LOGI(AP_TAG, "Total Write binary data length: 0x%X", binary_file_length);
	
	err = esp_ota_end(update_handle);
	strcpy (otabuf,"<!DOCTYPE html><html><head><title>r4sGate</title><meta name='viewport' content='width=device-width, initial-scale=1.0'> ");
	strcat(otabuf,"<meta http-equiv=\"refresh\" content=\"3;URL=http://");
	strcat(otabuf, bufip);
	strcat(otabuf,"\">");
	strcat(otabuf,"</head><body>Update ");
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
	strcat(otabuf,". Rebooting...</body></html>");
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
	httpd_register_uri_handler(server, &pblemon);
	httpd_register_uri_handler(server, &pblemonok);
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


void lpcomstat(uint8_t blenum) {
	if (blenum > 2) return;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	if (ptr->r4slpcom) {
	switch (ptr->r4slpcom) {
	case 1:             //off
	ptr->r4slpres = 1;
	if ((ptr->DEV_TYP < 10) && (ptr->bProg || ptr->bHeat)) {
	m171sOff(blenum);
	m171s_ModOff(blenum);
	}
	if ((ptr->DEV_TYP > 11) && (ptr->DEV_TYP < 16)) {
	if (ptr->r4slppar1 == 1) m103sToff(blenum);
	else if (ptr->r4slppar1 == 2) m103sTon(blenum);
	ptr->bprevProg = 254;
	}
	ptr->bprevHtemp = 255;
	ptr->bprevState = 255;
	ptr->bprevHeat = 255;
	ptr->bprevStNl = 255;
	m171sOff(blenum);
	ptr->r4slpcom = 0;
	ptr->t_last_us = ~ptr->t_last_us;
	break;
	case 2:             //boil
	ptr->r4slpres = 1;
	if ((ptr->DEV_TYP < 10) && ptr->bHtemp && (ptr->bHtemp < 91)) m171sHeat(blenum, 0);
	m171sOff(blenum);
	ptr->bprevHtemp = 255;
	ptr->bprevState = 255;
	ptr->bprevHeat = 255;
	ptr->bprevStNl = 255;
	m171sBoil(blenum);
	ptr->r4slpcom = 0;
	ptr->t_last_us = ~ptr->t_last_us;
	break;
	case 3:             //heat
	ptr->r4slpres = 1;
	m171sOff(blenum);
	ptr->bprevHtemp = 255;
	ptr->bprevState = 255;
	ptr->bprevHeat = 255;
	ptr->bprevStNl = 255;
	m171sHeat(blenum, ptr->r4slppar1);
	ptr->r4slpcom = 0;
	ptr->t_last_us = ~ptr->t_last_us;
	break;
	case 4:             //boil&heat
	ptr->r4slpres = 1;
	m171sOff(blenum);
	if (ptr->r4slppar1 == 0) m171sBoil(blenum);
	else m171sBoilAndHeat(blenum, ptr->r4slppar1);
	ptr->bprevHtemp = 255;
	ptr->bprevState = 255;
	ptr->bprevHeat = 255;
	ptr->bprevStNl = 255;
	ptr->r4slpcom = 0;
	ptr->t_last_us = ~ptr->t_last_us;
	break;
	case 5:             //nightlight
	ptr->r4slpres = 1;
	if (ptr->DEV_TYP > 3) {
	nl_settings[3]=ptr->RgbR;
	nl_settings[4]=ptr->RgbG;
	nl_settings[5]=ptr->RgbB;
	nl_settings[8]=ptr->RgbR;
	nl_settings[9]=ptr->RgbG;
	nl_settings[10]=ptr->RgbB;
	nl_settings[13]=ptr->RgbR;
	nl_settings[14]=ptr->RgbG;
	nl_settings[15]=ptr->RgbB;    
	if (ptr->bState || ptr->bHeat) m171sOff(blenum);
	m171s_NLOn(blenum);
	ptr->bprevState = 255;
	ptr->bprevHeat = 255;
	ptr->bprevStNl = 255;
	}
	ptr->r4slpcom = 0;
	ptr->t_last_us = ~ptr->t_last_us;
	break;
	case 6:             //power heater coffee on
	ptr->r4slpres = 1;
	if ((ptr->DEV_TYP > 11) && (ptr->DEV_TYP < 16)) {
	if (ptr->r4slppar1 == 1) m103sToff(blenum);
	else if (ptr->r4slppar1 == 2) m103sTon(blenum);
	ptr->bprevProg = 254;
	}
	ptr->bprevState = 255;
        if (ptr->DEV_TYP == 11) m103sPon(blenum, ptr->r4slppar2);
	else m103sOn(blenum);
	ptr->r4slpcom = 0;
	ptr->t_last_us = ~ptr->t_last_us;
	break;
	case 7:             //power lock off
	ptr->r4slpres = 1;
	ptr->bprevState = 255;
	ptr->bprevLock = 255;
	if (( ptr->DEV_TYP > 9) && ( ptr->DEV_TYP < 12 )) m103sLoff(blenum);
	else if (( ptr->DEV_TYP > 11 ) && ( ptr->DEV_TYP < 16 )) m151sLoff(blenum);
	ptr->r4slpcom = 0;
	ptr->t_last_us = ~ptr->t_last_us;
	break;
	case 8:             //power lock on
	ptr->r4slpres = 1;
	ptr->bprevState = 255;
	ptr->bprevLock = 255;
	if (( ptr->DEV_TYP > 9) && ( ptr->DEV_TYP < 12 )) m103sLon(blenum);
	else if (( ptr->DEV_TYP > 11 ) && ( ptr->DEV_TYP < 16 )) m151sLon(blenum);
	ptr->r4slpcom = 0;
	ptr->t_last_us = ~ptr->t_last_us;
	break;
	case 9:             //coffee strength
	ptr->r4slpres = 1;
	if (ptr->r4slppar1 == 0) m103sToff(blenum);
	else if (ptr->r4slppar1 == 1) m103sTon(blenum);
	ptr->bprevAwarm = 255;
	ptr->r4slpcom = 0;
	ptr->t_last_us = ~ptr->t_last_us;
	break;

	case 10:	//800 on off		
	ptr->r4slpres = 1;
	ptr->bprevState = 255;
	if (ptr->r4slppar1) rm800sOn(blenum);
	else rm800sOff(blenum);
	ptr->r4slpcom = 0;
	if (ptr->DEV_TYP < 24) {
	ptr->bDHour = 0;
	ptr->bDMin = 0;
	}
	ptr->t_last_us = ~ptr->t_last_us;
	break;

	case 11:	//800 prog
	ptr->r4slpres = 1;
	rm800sOff(blenum);
	ptr->bprevProg = 254;
	ptr->bprevModProg = 255;
	ptr->bprevPHour = 255;
	ptr->bprevPMin = 255;
	ptr->bprevCHour = 255;
	ptr->bprevCMin = 255;
	ptr->bprevDHour = 255;
	ptr->bprevDMin = 255;
	ptr->bprevAwarm = 255;
	if (ptr->r4slppar1 < 128) rm800sProg(blenum, ptr->r4slppar1);
	else if (ptr->DEV_TYP < 24) {
	ptr->bDHour = 0;
	ptr->bDMin = 0;
	}
	ptr->r4slpcom = 0;
	ptr->t_last_us = ~ptr->t_last_us;
	break;

	case 12:	//800 mode
	ptr->r4slpres = 1;
	ptr->bprevModProg = 255;
        rm800sMod(blenum, ptr->r4slppar1);
	ptr->r4slpcom = 0;
	ptr->t_last_us = ~ptr->t_last_us;
	break;

	case 13:	//800 temp
	ptr->r4slpres = 1;
	ptr->bprevHtemp = 255;
        rm800sTemp(blenum, ptr->r4slppar1);
	ptr->r4slpcom = 0;
	ptr->t_last_us = ~ptr->t_last_us;
	break;

	case 14:	//800 phour
	ptr->r4slpres = 1;
	ptr->bprevPHour = 255;
        rm800sPhour(blenum, ptr->r4slppar1);
	ptr->r4slpcom = 0;
	ptr->t_last_us = ~ptr->t_last_us;
	break;

	case 15:	//800 pmin
	ptr->r4slpres = 1;
	ptr->bprevPMin = 255;
        rm800sPmin(blenum, ptr->r4slppar1);
	ptr->r4slpcom = 0;
	ptr->t_last_us = ~ptr->t_last_us;
	break;

	case 16:	//800 awarm
	ptr->r4slpres = 1;
	ptr->bprevAwarm = 255;
        rm800sAwarm(blenum, ptr->r4slppar1);
	ptr->r4slpcom = 0;
	ptr->t_last_us = ~ptr->t_last_us;
	break;

	case 17:	//800 set all
	ptr->r4slpres = 1;
	ptr->bprevProg = 254;
	ptr->bprevModProg = 255;
	if (ptr->r4slppar1 > 127) {
	rm800sOff(blenum);
	if (ptr->DEV_TYP < 24) {
	ptr->bDHour = 0;
	ptr->bDMin = 0;
	}
	}
	else rm800sPall(blenum, ptr->r4slppar1, ptr->r4slppar2, ptr->r4slppar3, ptr->r4slppar4, ptr->r4slppar5, ptr->r4slppar7, ptr->r4slppar8, ptr->r4slppar6);
	ptr->r4slpcom = 0;
	ptr->t_last_us = ~ptr->t_last_us;
	break;

	case 18:	//1519 delay start
	ptr->r4slpres = 1;
	if (ptr->bState) m171sOff(blenum);
	if (ptr->r4slppar1 == 1) m103sToff(blenum);
	else if (ptr->r4slppar1 == 2) m103sTon(blenum);
	m151sDon(blenum, ptr->r4slppar2, ptr->r4slppar3);
	ptr->bprevProg = 254;
	ptr->bprevState = 255;
	ptr->r4slpcom = 0;
	ptr->t_last_us = ~ptr->t_last_us;
	break;

	case 19:	//800 dhour
	ptr->r4slpres = 1;
	ptr->bprevDHour = 255;
        rm800sDhour(blenum, ptr->r4slppar1);
	ptr->r4slpcom = 0;
	ptr->t_last_us = ~ptr->t_last_us;
	break;

	case 20:	//800 dmin
	ptr->r4slpres = 1;
	ptr->bprevDMin = 255;
        rm800sDmin(blenum, ptr->r4slppar1);
	ptr->r4slpcom = 0;
	ptr->t_last_us = ~ptr->t_last_us;
	break;

	case 21:	//bklight off
	ptr->r4slpres = 1;
        ptr->bprevStBl = 255;
	m171Bl(blenum, 0);
	ptr->r4slpcom = 0;
	ptr->t_last_us = ~ptr->t_last_us;
	break;

	case 22:	//bklight on
	ptr->r4slpres = 1;
        ptr->bprevStBl = 255;
	m171Bl(blenum, 1);
	ptr->r4slpcom = 0;
	ptr->t_last_us = ~ptr->t_last_us;
	break;

	case 23:	//beep off
	ptr->r4slpres = 1;
        ptr->bprevStBp = 255;
	m171Bp(blenum, 0);
	ptr->r4slpcom = 0;
	ptr->t_last_us = ~ptr->t_last_us;
	break;

	case 24:	//beep on
	ptr->r4slpres = 1;
        ptr->bprevStBp = 255;
	m171Bp(blenum, 1);
	ptr->r4slpcom = 0;
	ptr->t_last_us = ~ptr->t_last_us;
	break;

	case 25:	//boil time
	ptr->r4slpres = 1;
        ptr->bprevBlTime = 128;
	m171sBlTm(blenum, ptr->r4slppar1);
	ptr->r4slpcom = 0;
	ptr->t_last_us = ~ptr->t_last_us;
	break;

	case 63:	//weather calibrate
        MqState(blenum);
	ptr->r4slpres = 1;
	ptr->bprevState = 255;
	m51sCalibrate(blenum);
	ptr->r4slpcom = 0;
	ptr->t_last_us = ~ptr->t_last_us;
	break;

	case 64:	//mi off
	ptr->r4slpres = 1;
	ptr->bprevHtemp = 255;
	ptr->bprevState = 255;
	ptr->bprevHeat = 255;
	mMiOff(blenum);
	ptr->r4slpcom = 0;
	ptr->t_last_us = ~ptr->t_last_us;
	break;

	case 65:	//mi boil
	ptr->r4slpres = 1;
	ptr->bprevHtemp = 255;
	ptr->bprevState = 255;
	ptr->bprevHeat = 255;
	mMiBoil(blenum);
	ptr->r4slpcom = 0;
	ptr->t_last_us = ~ptr->t_last_us;
	break;

	case 66:	//mi temp
	ptr->r4slpres = 1;
	ptr->bprevHtemp = 255;
	ptr->bprevState = 255;
	ptr->bprevHeat = 255;
	mMiHeat(blenum, ptr->r4slppar1);
	ptr->r4slpcom = 0;
	ptr->t_last_us = ~ptr->t_last_us;
	break;
	}
	}

        vTaskDelay(100 / portTICK_PERIOD_MS);
	t_before_us = esp_timer_get_time();
// every 4s get kettle state
	if (t_before_us - ptr->t_last_us > 4000000) {
        MqState(blenum);
	ptr->r4slpres = 0;
	ptr->f_Sync++;
	if (ptr->f_Sync > 15) mkSync(blenum);	
	if (tft_conn) tfblestate();
	ptr->t_last_us = t_before_us;	
	}
// every 1s display time and date
	if ((tft_conn) && (t_before_us - t_clock_us > 1000000)) {
	tftclock();
	t_clock_us = t_before_us;	
	}

}



//******************* Main **********************
void app_main(void)
{
	static httpd_handle_t server = NULL;
	printf("Starting r4sGate...\n");
	tft_conf = 0;
	tft_conn = false;
	floop = 0;
	NumWfConn = 0;
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
	BleDevStA.r4sAuthCount = 0;
	BleDevStB.r4sAuthCount = 0;
	BleDevStC.r4sAuthCount = 0;
	BleDevStA.MiKettleID = 1;
	BleDevStB.MiKettleID = 1;
	BleDevStC.MiKettleID = 1;
	BleDevStA.notifyDataLen = -1;
	BleDevStB.notifyDataLen = -1;
	BleDevStC.notifyDataLen = -1;
	BleDevStA.readDataLen = -1;
	BleDevStB.readDataLen = -1;
	BleDevStC.readDataLen = -1;
	BleDevStA.readDataHandle = -1;
	BleDevStB.readDataHandle = -1;
	BleDevStC.readDataHandle = -1;
	BleDevStA.RgbR = 255;
	BleDevStA.RgbG = 255;
	BleDevStA.RgbB = 255;
	BleDevStB.RgbR = 255;
	BleDevStB.RgbG = 255;
	BleDevStB.RgbB = 255;
	BleDevStC.RgbR = 255;
	BleDevStC.RgbG = 255;
	BleDevStC.RgbB = 255;
	FND_NAME[0] = 0;
	BleDevStA.REQ_NAME[0] = 0;
	BleDevStB.REQ_NAME[0] = 0;
	BleDevStC.REQ_NAME[0] = 0;
	BleDevStA.RQC_NAME[0] = 0;
	BleDevStB.RQC_NAME[0] = 0;
	BleDevStC.RQC_NAME[0] = 0;
	BleDevStA.DEV_NAME[0] = 0;
	BleDevStB.DEV_NAME[0] = 0;
	BleDevStC.DEV_NAME[0] = 0;
	BleDevStA.tBLEAddr[0] = 0;
	BleDevStB.tBLEAddr[0] = 0;
	BleDevStC.tBLEAddr[0] = 0;

	MQTT_USER[0] = 0;
	MQTT_PASSWORD[0] = 0;
	MQTT_SERVER[0] = 0;
	WIFI_SSID[0] = 0;
	WIFI_PASSWORD[0] = 0;

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
	BleDevStA.xshedcom = 0;
	BleDevStB.xshedcom = 0;
	BleDevStC.xshedcom = 0;
	BleDevStA.NumConn = 0;
	BleDevStB.NumConn = 0;
	BleDevStC.NumConn = 0;
	ble_mon = 0;
	ble_mon_refr = 0;
	BleDevStA.bSEnergy = 0;
	BleDevStB.bSEnergy = 0;
	BleDevStC.bSEnergy = 0;
	BleDevStA.bSTime = 0;  
	BleDevStB.bSTime = 0;  
	BleDevStC.bSTime = 0;  
	BleDevStA.bSCount = 0; 
	BleDevStB.bSCount = 0; 
	BleDevStC.bSCount = 0; 
	BleDevStA.bSHum = 0;
	BleDevStB.bSHum = 0;
	BleDevStC.bSHum = 0;
	BleDevStA.bLtemp = 0;
	BleDevStB.bLtemp = 0;
	BleDevStC.bLtemp = 0;
	BleDevStA.bEfficiency = 80;
	BleDevStB.bEfficiency = 80;
	BleDevStC.bEfficiency = 80;
	BleDevStA.bCVol = 254;
	BleDevStB.bCVol = 254;
	BleDevStC.bCVol = 254;
	BleDevStA.bCVoll = 0;
	BleDevStB.bCVoll = 0;
	BleDevStC.bCVoll = 0;
	BleDevStA.bBlTime = 0;
	BleDevStB.bBlTime = 0;
	BleDevStC.bBlTime = 0;
	memset (BleMR,0,sizeof(BleMR));
	memset (BleMX,0,sizeof(BleMX));
	foffln = 0;
	bgpio1 = 0;
	bgpio2 = 0;
	bgpio3 = 0;
	bgpio4 = 0;
	bgpio5 = 0;
	strcpy(strON,"ON");
	strcpy(strOFF,"OFF");
	AUTH_BASIC[0] = 0;

// read nvs
	nvs_handle_t my_handle;
	ret = nvs_open("storage", NVS_READWRITE, &my_handle);
	if (ret == ESP_OK) {
	nvs_get_u16(my_handle, "sjpgtim", &jpg_time);
	nvs_get_u16(my_handle, "smqprt", &mqtt_port);
	nvs_get_u8(my_handle,  "sreqtpa", &BleDevStA.DEV_TYP);
	nvs_get_u8(my_handle,  "sreqtpb", &BleDevStB.DEV_TYP);
	nvs_get_u8(my_handle,  "sreqtpc", &BleDevStC.DEV_TYP);
	nvs_get_u8(my_handle, "rlighta", &BleDevStA.RgbR);
	nvs_get_u8(my_handle, "glighta", &BleDevStA.RgbG);
	nvs_get_u8(my_handle, "blighta", &BleDevStA.RgbB);
	nvs_get_u8(my_handle, "rlightb", &BleDevStB.RgbR);
	nvs_get_u8(my_handle, "glightb", &BleDevStB.RgbG);
	nvs_get_u8(my_handle, "blightb", &BleDevStB.RgbB);
	nvs_get_u8(my_handle, "rlightc", &BleDevStC.RgbR);
	nvs_get_u8(my_handle, "glightc", &BleDevStC.RgbG);
	nvs_get_u8(my_handle, "blightc", &BleDevStC.RgbB);
	nvs_get_u8(my_handle, "ltempa", &BleDevStA.bLtemp);
	nvs_get_u8(my_handle, "ltempb", &BleDevStB.bLtemp);
	nvs_get_u8(my_handle, "ltempc", &BleDevStC.bLtemp);
	nvs_get_u8(my_handle, "effica", &BleDevStA.bEfficiency);
	nvs_get_u8(my_handle, "efficb", &BleDevStB.bEfficiency);
	nvs_get_u8(my_handle, "efficc", &BleDevStC.bEfficiency);
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
	BleDevStA.PRgbR = ~BleDevStA.RgbR;
	BleDevStA.PRgbG = ~BleDevStA.RgbG;
	BleDevStA.PRgbB = ~BleDevStA.RgbB;
	BleDevStB.PRgbR = ~BleDevStB.RgbR;
	BleDevStB.PRgbG = ~BleDevStB.RgbG;
	BleDevStB.PRgbB = ~BleDevStB.RgbB;
	BleDevStC.PRgbR = ~BleDevStC.RgbR;
	BleDevStC.PRgbG = ~BleDevStC.RgbG;
	BleDevStC.PRgbB = ~BleDevStC.RgbB;
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
	nvsize = 32;
	nvs_get_str(my_handle,"smqsrv", MQTT_SERVER,&nvsize);
	nvsize = 16;
	nvs_get_str(my_handle,"smqid", MQTT_USER,&nvsize);
	nvsize = 20;
	nvs_get_str(my_handle,"smqpsw", MQTT_PASSWORD,&nvsize);
	nvsize = 16;
	nvs_get_str(my_handle,"sreqnma", BleDevStA.REQ_NAME,&nvsize);
	nvsize = 16;
	nvs_get_str(my_handle,"sreqnmb", BleDevStB.REQ_NAME,&nvsize);
	nvsize = 16;
	nvs_get_str(my_handle,"sreqnmc", BleDevStC.REQ_NAME,&nvsize);
	nvsize = sizeof(BleMR);
	nvs_get_blob(my_handle,"sblemd",  BleMR,&nvsize);
	if (nvsize != sizeof(BleMR)) memset (BleMR,0,sizeof(BleMR));
	nvsize = 50;
	nvs_get_str(my_handle, "auth", AUTH_BASIC, &nvsize);

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
// Close nvs
	nvs_close(my_handle);
	}
	strcpy(BleDevStA.RQC_NAME, BleDevStA.REQ_NAME);
	strcpy(BleDevStB.RQC_NAME, BleDevStB.REQ_NAME);
	strcpy(BleDevStC.RQC_NAME, BleDevStC.REQ_NAME);
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
	BleDevStA.tBLEAddr[0] = 0;
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
	if ((bgpio1 > 127) && ((bgpio1 & 0x3f) < 34)) gpio_set_pull_mode((bgpio1 & 0x3f), GPIO_PULLUP_ONLY);
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
	if ((bgpio2 > 127) && ((bgpio2 & 0x3f) < 34)) gpio_set_pull_mode((bgpio2 & 0x3f), GPIO_PULLUP_ONLY);
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
	if ((bgpio3 > 127) && ((bgpio3 & 0x3f) < 34)) gpio_set_pull_mode((bgpio3 & 0x3f), GPIO_PULLUP_ONLY);
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
	if ((bgpio4 > 127) && ((bgpio4 & 0x3f) < 34)) gpio_set_pull_mode((bgpio4 & 0x3f), GPIO_PULLUP_ONLY);
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
	if ((bgpio5 > 127) && ((bgpio5 & 0x3f) < 34)) gpio_set_pull_mode((bgpio5 & 0x3f), GPIO_PULLUP_ONLY);
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


	if (tft_conf) tft_conn = tftinit();
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

	ESP_LOGI(AP_TAG,"Basic Auth string: %s",AUTH_BASIC);
	ESP_LOGI(AP_TAG,"esp32 BLE MAC Address:");
	esp_read_mac(binblemac, ESP_MAC_BT);
        esp_log_buffer_hex(AP_TAG, binblemac, 6);

//register the  callback function to the gap module
	ret = esp_ble_gap_register_callback(esp_gap_cb);
	if (ret){
	ESP_LOGI(AP_TAG,"%s gap register failed, error code = 0x%X\n", __func__, ret);
	fflush(stdout);
	esp_restart();
	}
//register the callback function to the gattc module
	ret = esp_ble_gattc_register_callback(esp_gattc_cb);
	if(ret){
	ESP_LOGI(AP_TAG,"%s gattc register failed, error code = 0x%X\n", __func__, ret);
	fflush(stdout);
	esp_restart();
	}
	ret = esp_ble_gattc_app_register(PROFILE_A_APP_ID);
	if (ret){
	ESP_LOGI(AP_TAG,"%s gattc app register failed, error code = 0x%X\n", __func__, ret);
	fflush(stdout);
	esp_restart();
	}
	ret = esp_ble_gattc_app_register(PROFILE_B_APP_ID);
	if (ret){
	ESP_LOGI(AP_TAG,"%s gattc app register failed, error code = 0x%X\n", __func__, ret);
	fflush(stdout);
	esp_restart();
	}
	ret = esp_ble_gattc_app_register(PROFILE_C_APP_ID);
	if (ret){
	ESP_LOGI(AP_TAG,"%s gattc app register failed, error code = 0x%X\n", __func__, ret);
	fflush(stdout);
	esp_restart();
	}
	esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(BLE_INPUT_BUFFSIZE);
//	esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(200);
	if (local_mtu_ret){
	ESP_LOGI(AP_TAG,"Set local  MTU failed, error code = 0x%X\n", local_mtu_ret);
	}


//Initialize Mqtt
	if (MQTT_SERVER[0]) mqtt_app_start();
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
	bin2hex(macbuf, tESP32Addr,6,0);
	bin2hex(macbuf, tESP32Addr1,6,0x3a);
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
	floop = 32;
	t_before_us = 0;

//r4s state nonitoring and command execution loop
	while (floop) {

	lpcomstat(0);
	lpcomstat(1);
	lpcomstat(2);


	if (t_before_us - t_lasts_us > 6000000) {
	MqSState();
	t_lasts_us = t_before_us;	
	}

	if (!f_update && !BleDevStA.r4slpcom && !BleDevStB.r4slpcom && !BleDevStC.r4slpcom && tft_conn && jpg_time && (t_before_us - t_jpg_us > jpg_time*1000000)) {
	ret = tftjpg();
	if (ret) {
	BleDevStA.t_last_us = t_before_us;
	BleDevStB.t_last_us = t_before_us;
	BleDevStC.t_last_us = t_before_us;
	t_tinc_us = t_before_us;
	}
	t_jpg_us = t_before_us;	
	}

	if (t_before_us - t_tinc_us > 4000000) {
	blstnum_inc();
	if (tft_conn) tfblestate();
	if (tft_conn) tftclock();
	t_clock_us = t_before_us;	
	if (f_update && tft_conn) tfblestate();
	t_tinc_us = t_before_us;	
	if (s_retry_num >= WIFI_MAXIMUM_RETRY) {
	ESP_LOGI(AP_TAG,"Wifi disconnected. Restarting now...");
	fflush(stdout);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
	floop = 0;
	} else floop = 16;

	}

}
	esp_restart();
}
