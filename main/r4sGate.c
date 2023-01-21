/* 
*************************************************************
	ESP32 r4sGate for Redmond+ main
	Lutov Andrey  Donetsk
Use for compilation ESP-IDF Programming Guide:
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/
*************************************************************
*/
#define AP_VER "2023.01.21"
#define NVS_VER 6  //NVS config version (even only)

// Init WIFI setting
#define INIT_WIFI_SSID "r4s"
#define INIT_WIFI_PASSWORD "12345678"

// If use ili9341 320*240 tft
#define USE_TFT
// If use IR TX
#define USE_IRTX

// define max pin + 1
#ifdef CONFIG_IDF_TARGET_ESP32C3
#define MxPOutP 22 
//#define MxPInP 22 
#else
#define MxPOutP 34 
//#define MxPInP 40 
#endif

#include "r4sGate.h"
//************** my common proc ***************
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
// convert uint32 as decimal point 1 (0.0) & strcat 
void u32_strcat_p1 (uint32_t val, char* cout)
{
	char tmp[32] = {0}; 
	uint32_t var1 = val;
	itoa(var1 / 10,tmp,10);
	strcat(cout,tmp);
	var1 = var1 % 10;
	if (var1) {
	strcat(cout,".");
	itoa(var1,tmp,10);
	strcat(cout,tmp);
	}
}
// convert uint32 as decimal point 2 (0.00) & strcat 
void u32_strcat_p2 (uint32_t val, char* cout)
{
	char tmp[32] = {0}; 
	uint32_t var1 = val;
	itoa(var1 / 100,tmp,10);
	strcat(cout,tmp);
	var1 = var1 % 100;
	if (var1) {
	strcat(cout,".");
	if (var1 % 10) {
	if (var1 < 10) strcat (cout,"0");
	} else var1 = var1 / 10;
	itoa(var1,tmp,10);
	strcat(cout,tmp);
	}
}
// convert uint32 as decimal point 3 (0.000) & strcat 
void u32_strcat_p3 (uint32_t val, char* cout)
{
	char tmp[32];
	uint32_t var1 = val;
	itoa(var1 / 1000,tmp,10);
	strcat (cout,tmp);
	var1 = var1 % 1000;
	if (var1) {
	strcat (cout,".");
	if (var1 % 100) {
	if (var1 < 100) strcat (cout,"0");
	if (var1 % 10) {
	if (var1 < 10) strcat (cout,"0");
	} else var1 = var1 / 10;
	} else var1 = var1 / 100;
	itoa(var1,tmp,10);
	strcat (cout,tmp);
	}
}

// 18b20 uint * 10 / 16 to string cat 
void s18b20_strcat (uint16_t val, char* cout)
{
	char tmp[32] = {0}; 
	uint16_t var1 = val;
	if (var1 & 0x8000) {
	var1 = (var1 ^ 0x0ffff);  
	if (var1) var1++;     // ffff = -0 (sensor not connected)
	strcat(cout,"-");
	}
	var1 = (var1 * 10 + 8) >> 4;
	itoa(var1 / 10, tmp, 10);
	strcat(cout, tmp);
	var1 = var1 % 10;
	if (var1) {
	strcat(cout,".");
	itoa(var1,tmp,10);
	strcat(cout,tmp);
	}
}
// dht22 uint to string cat 
void 	sdht22strcat(uint16_t val, char* cout)
{
	char tmp[32] = {0}; 
	uint16_t var1 = val;
	if (var1 & 0x8000) {
	var1 = (var1  & 0x07fff);  
	if (!(var1 ^ 0x07fff)) var1 = 0; // ffff = -0 (sensor not connected)
	strcat(cout,"-");
	}
	itoa(var1 / 10, tmp, 10);
	strcat(cout, tmp);
	var1 = var1 % 10;
	if (var1) {
	strcat(cout,".");
	itoa(var1,tmp,10);
	strcat(cout,tmp);
	}
}
// bme280 uint / 16 to string cat 
void sbme280_strcat (uint16_t val, char* cout)
{
	char tmp[32] = {0}; 
	uint16_t var1 = val;
	if (var1 & 0x8000) {
	var1 = (var1 ^ 0x0ffff) + 1;
	strcat(cout,"-");
	}
	var1 >>= 4;
	itoa(var1 / 10, tmp, 10);
	strcat(cout, tmp);
	var1 = var1 % 10;
	if (var1) {
	strcat(cout,".");
	itoa(var1,tmp,10);
	strcat(cout,tmp);
	}
}
// convert int16 as decimal point 2 (0.00) & strcat 
void s16_strcat_p2 (uint16_t val, char* cout)
{
	char tmp[32] = {0}; 
	uint16_t var1 = val;
	if (var1 & 0x8000) {
	var1 = (var1 ^ 0x0ffff) + 1;
	strcat(cout,"-");
	}
	itoa(var1 / 100, tmp, 10);
	strcat(cout,tmp);
	var1 = var1 % 100;
	if (var1) {
	strcat(cout,".");
	if (var1 % 10) {
	if (var1 < 10) strcat (cout,"0");
	} else var1 = var1 / 10;
	itoa(var1,tmp,10);
	strcat(cout,tmp);
	}
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

// hex char lowercase string to bin byte
bool hex2bin(char *in, uint8_t *out,  size_t len)
{
	if (in == NULL || out == NULL || len == 0) return 0;
	bool result = 1;
	size_t i = 0;
	char a;
	char b;
	uint8_t c;
	while ((i < len) && (result)) {
	a = in[i*2];
	b =  (a <= '9') ? (a - '0') : (a - 'a' + 10);
	if (b > 0x0f) result = 0;
	b = (b & 0x0f) * 16;
	a = in[i*2 + 1];
	c =  (a <= '9') ? (a - '0') : (a - 'a' + 10);
	if (c > 0x0f) result = 0;
	c = (c & 0x0f) + b;
	out[i] = c;
	i++;
	}	
	return result;
}

// parse uri par=value string like this:
// swfid=wifiname&swfpsw=wifipassword&smqsrv=192.168.1.10&smqid=esp&
// smqpsw=esp&devnam=&rlight=255&glight=255&blight=255&chk2=2
// find par *key in string *cin(isize) & copy value to *out buffer(osize) 
void parsuri(char *cin, char *cout, char *ckey, int isize, int osize, uint8_t fmac)
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
	if (fmac && (c == 0x3a)) j--;
	else cout[j] = c;
	} else if (a == 0x2b) cout[j] = 0x20;
	else cout[j] = cin[i];
	i++; j++;
	cout[j] = 0;
	} else found = 0;
	}
}
// parse certificate string
void parscertstr(char *cin, char *cout, char *ckey, int isize, int osize)
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

	if (i < isize) a = cin[i];
	else a = 0;
	if (a == 0x2d) {                               //remove begin certificate line if exist
	c = 0;
	b = 0;
	while (c < 2) {
	if (a > 0x2b) b = a;
	i++;
	if (i < isize) a = cin[i];
	else a = 0;
	if (a < 0x20) { 
	c = 2;
	i = isize;
	} else if ((a > 0x2d) && (b == 0x2d)) c++;	
	}
	}
	while ((i < isize) && (j < osize) && (found)) {
	a = cin[i];
	if ((a > 0x1f) && (a != 0x26) && (a != 0x2d)) {       //0x26-> &  2d -> remove end certificate line
	if (a == 0x25) {
	i++;
	if (i < isize) a = cin[i];
	else found = 0;
	b =  (a <= '9') ? (a - '0') : (a - 'A' + 10);
	b = (b & 0xf) * 16;
	i++;
	if (i < isize) a = cin[i];
	else found = 0;
	c =  (a <= '9') ? (a - '0') : (a - 'A' + 10);
	c = (c & 0xf) + b;
	cout[j] = c;
	} else if (a == 0x2b) cout[j] = 0x0a;
	else cout[j] = cin[i];
	i++; j++;
	cout[j] = 0;
	} else found = 0;
	}
	if (j) j--;                     //remove \n
	while (j) {
	if (cout[j] == 0x0a) {
	cout[j] = 0;
	j--;	
	} else j = 0;
	}
}
//my cert copy include dest buf limit
void mycertcpy(char *cout, char *cin, int osize)
{
	int i = 0;
	char a;
	while (i < osize) {
	a = cin[i];
	if  (a) {
	if  (a < 0x20) a = 0x20;
	cout[i] = a;
	i++;
	cout[i] = 0;
	} else {
	i = osize;
	}		
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
//my value copy include dest buf limit
void myvalcpy(char *cout, char *cin, int osize)
{
	int i = 0;
	char a;
	while (cin[i] == 0x20) i++;
	while (i < osize) {
	a = cin[i];
	if  ((a > 0x2d) && (a < 0x3a)) {
	cout[i] = a;
	i++;
	cout[i] = 0;
	} else {
	i = osize;
	}		
	}
}
//my string copy include dest buf limit
void myurlcpy(char *cout, char *cin, int osize)
{
	int i = 0;
	int j = 0;
	int f = 0;
	uint8_t g = 0;
	uint8_t a;
	cout[j] = 0;
	while (j < osize) {
	a = cin[i];
	i++;
	if  ((a > 0x20) && (a < 0x80)) {
	if (f && (a == 0x2f)) f = 0;
	if (f && ((a < 0x30) || (a > 0x39))) j = osize;
	else {
	if ((a == 0x3a) && (i > 7) && !g) f = j + 6;
	if ((a == 0x3d) || (a == 0x3f)) g = 1;
	cout[j] = a;
	j++;
	cout[j] = 0;
	}
	if (j == f) j = osize;
	} else if  (!f && ((a == 0x20) || (a > 0x7f))) {
	cout[j] = 0x25;
	j++;
	cout[j] = 0;
	if (j < osize) {
	cout[j]   = "0123456789abcdef"[a >> 4];
	j++;
	cout[j] = 0;
	}
	if (j < osize) {
	cout[j]   = "0123456789abcdef"[a & 0x0F];
	j++;
	cout[j] = 0;
	}
	} else {
	j = osize;
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
//******************* io_mux ******************
void mygp_iomux_out (uint8_t gpio)
{
	gpio_iomux_out(gpio, PIN_FUNC_GPIO, false);

}
//******************* xiaomi ******************
//https://github.com/aprosvetova/xiaomi-kettle
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
//******************* smart tag ***************
bool vstsign(uint8_t *datin, uint8_t *keyin)
{
	bool result = 0;
	uint8_t datout[16];
	uint8_t iv[16];
	mbedtls_aes_context aes;
	memcpy(iv, &keyin[16], 16);
	mbedtls_aes_init(&aes);
	mbedtls_aes_setkey_enc(&aes, keyin, 128);
	mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, 16, iv, datin, datout);
	if (!memcmp(datout, &datin[16], 4)) result = 1;
	return result;
}

//****************** IR TX ********************
/*
----------------------------------------------------------------
https://www.sbprojects.net/knowledge/ir/index.php
----------------------------------------------------------------
code topic: r4sx/ir6code
data: 8 hex digits(0-9,a-f), 2-protocol(01-nec,02-necx16,03-rc5,
04-rc6,05-samsung,6-sircx12,07-sircx15,08-sircx20,09-panasonic),
4-addr, 2-cmd 
----------------------------------------------------------------
tested:
nec pioneer vsx-830 pwr: addr 165, cmd 28, code 0100a51c
necx16 lg dvd dks-2000h, pwr: addr 11565, cmd 48, code 022d2d30 
rc6 philips 40pfs6609, pwr: addr 0, cmd 12, code 0400000c
samsung ue32n5300, pwr: addr 7, cmd 2, code 05000702
sircx12 sony cmtsx7, pwr: addr:16, cmd: 21, code 06001015
sircx20 sony ubp-x800 pwr: addr 7258, cmd 21, code 081c5a15
panasonic sa-pm20 pwr: addr 2588, cmd 61, code 090a1c3d
panasonic dmp ub900 pwr: addr 2816, cmd 61, code 090b00c3d
----------------------------------------------------------------
freq/div:
36004: 101*22
37002: 94*23
38222: 161*13
38095: 84*25
38022: 263*8
38004: 421*5
37986: 81*26
*/

#ifdef USE_IRTX
bool rmtir_init (uint8_t idx, uint8_t  gpio_num)
{
	bool result = 0;
	uint8_t tidx;
#ifdef CONFIG_IDF_TARGET_ESP32C3
	if (idx > 1) return result;
	tidx = idx;
#else
	if (idx > 2) return result;
	tidx = idx << 1;
#endif
//rmt tx init
	rmt_config_t rmt_tx = {0};
	rmt_tx.channel = tidx;
	rmt_tx.gpio_num = gpio_num;
	rmt_tx.mem_block_num = 1;
	rmt_tx.clk_div = 80;
	rmt_tx.tx_config.loop_en = false;
	rmt_tx.tx_config.carrier_en = true;
	rmt_tx.tx_config.carrier_duty_percent = 33;
	rmt_tx.tx_config.carrier_freq_hz = 38000;
	rmt_tx.tx_config.carrier_level = 1;
	rmt_tx.tx_config.idle_level = 0;
	rmt_tx.tx_config.idle_output_en = true;
	rmt_tx.rmt_mode = RMT_MODE_TX;
	if (rmt_config(&rmt_tx) == ESP_OK) {  //1
	rmt_set_source_clk(rmt_tx.channel, RMT_BASECLK_APB);
	if (rmt_driver_install(rmt_tx.channel, 0, ESP_INTR_FLAG_LOWMED
			| ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_SHARED) == ESP_OK) result = 1;
//end of init	
	}  //1
	return result;
}
bool rmtir_send (uint8_t idx,  uint16_t* ptxcmd, uint16_t* pprtxcmd, uint16_t* ptaddr)
{
	bool result = 0;
	uint16_t txcmd;
	txcmd = *ptxcmd;
	if (!(txcmd & 0xff00)) return result;
	uint8_t tidx;
	uint16_t prtxcmd, taddr;
	prtxcmd = *pprtxcmd;
	taddr = *ptaddr;
	if (fdebug) ESP_LOGI(AP_TAG, "IR Tx code: %02X%04x%02x", ((txcmd >> 8) & 0xff), taddr, (txcmd & 0xff));
#ifdef CONFIG_IDF_TARGET_ESP32C3
	if (idx > 1) return result;
	tidx = idx;
#else
	if (idx > 2) return result;
	tidx = idx << 1;
#endif
	switch ((txcmd >> 8) & 0xff) {
	case 0x01:                        //nec
	if ((rmt_set_clk_div(tidx, 81) == ESP_OK) && 
	(rmt_set_tx_carrier(tidx, true, 9 * 81, 17 * 81,  1) == ESP_OK)) {
//12.5ns * 81(= 1.0125us) * 26 = 26.325us(37986.7Hz)  T = 556 (~562.5us/1.0125)
	uint16_t T = 556;                 //~562.5us/1.0125
	uint8_t tnum = 24;
	uint8_t out;
	rmt_item32_t tx_items[36] = {0};
	rmt_item32_t tx_ritems[4] = {0};
//first
	tx_items[0].level0 = 1;           //sb
	tx_items[0].duration0 = 16 * T;
	tx_items[0].level1 = 0;
	tx_items[0].duration1 = 8 * T;
	out = taddr & 0xff;
	for (int i = 1; i < 9; i++) {     //a0-a7 1
	tx_items[i].level0 = 1;
	tx_items[i].level1 = 0;
	tx_items[i].duration0 = T;
	if (out & 0x01) {
	tx_items[i].duration1 = 3 * T;
	tnum += 2;
	} else tx_items[i].duration1 = T;
	tnum += 2;
	out >>= 1;
	}
	out = ~taddr & 0xff;
	for (int i = 9; i < 17; i++) {    //~a0-a7 2
	tx_items[i].level0 = 1;
	tx_items[i].level1 = 0;
	tx_items[i].duration0 = T;
	if (out & 0x01) {
	tx_items[i].duration1 = 3 * T;
	tnum += 2;
	} else tx_items[i].duration1 = T;
	tnum += 2;
	out >>= 1;
	}
	out = txcmd & 0xff;
	for (int i = 17; i < 25; i++) {   //d0-d7 1
	tx_items[i].level0 = 1;
	tx_items[i].level1 = 0;
	tx_items[i].duration0 = T;
	if (out & 0x01) {
	tx_items[i].duration1 = 3 * T;
	tnum += 2;
	} else tx_items[i].duration1 = T;
	tnum += 2;
	out >>= 1;
	}
	out = ~txcmd & 0xff;
	for (int i = 25; i < 33; i++) {   //~d0-d7 2
	tx_items[i].level0 = 1;
	tx_items[i].level1 = 0;
	tx_items[i].duration0 = T;
	if (out & 0x01) {
	tx_items[i].duration1 = 3 * T;
	tnum += 2;
	} else tx_items[i].duration1 = T;
	tnum += 2;
	out >>= 1;
	}
	tx_items[33].level0 = 1;          //end bit
	tx_items[33].duration0 = T;
	tx_items[33].level1 = 0;
	tx_items[33].duration1 = T;
	tnum += 2;
	T = 54320 - ((tnum >> 1) * T);    //s2s 110ms, 110000/1.0125/2=54320
 	tx_items[34].level0 = 0;
	tx_items[34].duration0 = T;
	tx_items[34].level1 = 0;
	tx_items[34].duration1 = T;
	tx_items[35].level0 = 0;          //end
	tx_items[35].duration0 = 0;
	tx_items[35].level1 = 0;
	tx_items[35].duration1 = 0;
//repeat
	tx_ritems[0].level0 = 1;          //sb
	tx_ritems[0].duration0 = 16 * T;
	tx_ritems[0].level1 = 0;
	tx_ritems[0].duration1 = 4 * T;
	tx_ritems[1].level0 = 1;          //end bit
	tx_ritems[1].duration0 = T;
	tx_ritems[1].level1 = 0;
	tx_ritems[1].duration1 = T;
 	tx_ritems[2].level0 = 0;          //sp 96ms
	tx_ritems[2].duration0 = 32500;
	tx_ritems[2].level1 = 0;
	tx_ritems[2].duration1 = 32500;
	tx_ritems[3].level0 = 0;
	tx_ritems[3].duration0 = 31408;
	tx_ritems[3].level1 = 0;          //end
	tx_ritems[3].duration1 = 0;
	out = 0;
	if (rmt_write_items(tidx, tx_items, 36, true) != ESP_OK) out = 255;
	while (out < 3) {
	if (rmt_write_items(tidx, tx_ritems, 4, true) != ESP_OK) out = 255;
	out++;
	}
	if (fdebug) {
	if (out != 255) ESP_LOGI(AP_TAG, "IR NEC Tx addr: 0x%X, cmd: 0x%X", (taddr & 0xff), (txcmd & 0xff));
	else  ESP_LOGI(AP_TAG, "IR NEC Tx error");
	}
	if (out != 255) result = 1;
	}
	break;
	case 0x02:                        //necx16
	if ((rmt_set_clk_div(tidx, 81) == ESP_OK) && 
	(rmt_set_tx_carrier(tidx, true, 9 * 81, 17 * 81,  1) == ESP_OK)) {
//12.5ns * 81(= 1.0125us) * 26 = 26.325us(37986.7Hz)  T = 556 (~562.5us/1.0125)
	uint16_t T = 556;                 //~562.5us/1.0125
	uint8_t tnum = 24;
	uint8_t out;
	rmt_item32_t tx_items[36] = {0};
	rmt_item32_t tx_ritems[4] = {0};
//first
	tx_items[0].level0 = 1;           //sb
	tx_items[0].duration0 = 16 * T;
	tx_items[0].level1 = 0;
	tx_items[0].duration1 = 8 * T;
	out = taddr & 0xff;
	for (int i = 1; i < 9; i++) {     //a0-a7 1
	tx_items[i].level0 = 1;
	tx_items[i].level1 = 0;
	tx_items[i].duration0 = T;
	if (out & 0x01) {
	tx_items[i].duration1 = 3 * T;
	tnum += 2;
	} else tx_items[i].duration1 = T;
	tnum += 2;
	out >>= 1;
	}
	out = (taddr >> 8) & 0xff;
	for (int i = 9; i < 17; i++) {    //a8-a15 2
	tx_items[i].level0 = 1;
	tx_items[i].level1 = 0;
	tx_items[i].duration0 = T;
	if (out & 0x01) {
	tx_items[i].duration1 = 3 * T;
	tnum += 2;
	} else tx_items[i].duration1 = T;
	tnum += 2;
	out >>= 1;
	}
	out = txcmd & 0xff;
	for (int i = 17; i < 25; i++) {   //d0-d7 1
	tx_items[i].level0 = 1;
	tx_items[i].level1 = 0;
	tx_items[i].duration0 = T;
	if (out & 0x01) {
	tx_items[i].duration1 = 3 * T;
	tnum += 2;
	} else tx_items[i].duration1 = T;
	tnum += 2;
	out >>= 1;
	}
	out = ~txcmd & 0xff;
	for (int i = 25; i < 33; i++) {   //~d0-d7 2
	tx_items[i].level0 = 1;
	tx_items[i].level1 = 0;
	tx_items[i].duration0 = T;
	if (out & 0x01) {
	tx_items[i].duration1 = 3 * T;
	tnum += 2;
	} else tx_items[i].duration1 = T;
	tnum += 2;
	out >>= 1;
	}
	tx_items[33].level0 = 1;          //end bit
	tx_items[33].duration0 = T;
	tx_items[33].level1 = 0;
	tx_items[33].duration1 = T;
	tnum += 2;
	T = 54320 - ((tnum >> 1) * T);    //s2s 110ms, 110000/1.0125/2=54320
 	tx_items[34].level0 = 0;
	tx_items[34].duration0 = T;
	tx_items[34].level1 = 0;
	tx_items[34].duration1 = T;
	tx_items[35].level0 = 0;          //end
	tx_items[35].duration0 = 0;
	tx_items[35].level1 = 0;
	tx_items[35].duration1 = 0;
//repeat
	tx_ritems[0].level0 = 1;          //sb
	tx_ritems[0].duration0 = 16 * T;
	tx_ritems[0].level1 = 0;
	tx_ritems[0].duration1 = 4 * T;
	tx_ritems[1].level0 = 1;          //end bit
	tx_ritems[1].duration0 = T;
	tx_ritems[1].level1 = 0;
	tx_ritems[1].duration1 = T;
 	tx_ritems[2].level0 = 0;          //sp 96ms
	tx_ritems[2].duration0 = 32500;
	tx_ritems[2].level1 = 0;
	tx_ritems[2].duration1 = 32500;
	tx_ritems[3].level0 = 0;
	tx_ritems[3].duration0 = 31408;
	tx_ritems[3].level1 = 0;          //end
	tx_ritems[3].duration1 = 0;
	out = 0;
	if (rmt_write_items(tidx, tx_items, 36, true) != ESP_OK) out = 255;
	while (out < 3) {
	if (rmt_write_items(tidx, tx_ritems, 4, true) != ESP_OK) out = 255;
	out++;
	}
	if (fdebug) {
	if (out != 255) ESP_LOGI(AP_TAG, "IR NECx16 Tx addr: 0x%X, cmd: 0x%X", taddr, (txcmd & 0xff));
	else  ESP_LOGI(AP_TAG, "IR NECx16 Tx error");
	}
	if (out != 255) result = 1;
	}
	break;
	case 0x03:                        //rc5
	if ((rmt_set_clk_div(tidx, 101) == ESP_OK) && 
	(rmt_set_tx_carrier(tidx, true, 7 * 101, 15 * 101,  1) == ESP_OK)) {
//https://www.st.com/resource/en/application_note/an2957-implementing-an-rc5-infrared-transmitter-using-the-ir-timer-modulator-of-the-stm8l10x-microcontroller-stmicroelectronics.pdf
//12.5ns * 101(= 1.2625us) * 22 = 27.775us(36003.6Hz) T = 704 (22*32)
	uint16_t T = 704;                 //888.8us(22 * 32)
	uint8_t out = 0;                  //frame 14 * 2T = 24.886ms
	rmt_item32_t tx_items[15] = {0};

	tx_items[0].level0 = 0;           //sb 1
	tx_items[0].duration0 = T;
	tx_items[0].level1 = 1;
	tx_items[0].duration1 = T;
	tx_items[1].duration0 = T;        //sb 2
	tx_items[1].duration1 = T;
	if (txcmd & 0x40) {               //if rc5ext
	tx_items[1].level0 = 1;           //cmd ~bit6 
	tx_items[1].level1 = 0;
	} else {
	tx_items[1].level0 = 0;           
	tx_items[1].level1 = 1;
	}
	tx_items[2].level0 = 1;           //tb
	tx_items[2].duration0 = T;
	tx_items[2].level1 = 0;
	tx_items[2].duration1 = T;
	out = (taddr << 3) & 0xff;
	for (int i = 2; i < 7; i++) {     //a4-a0
	tx_items[i].duration0 = T;
	tx_items[i].duration1 = T;
	if (out & 0x80) {
	tx_items[i].level0 = 0;
	tx_items[i].level1 = 1;
	} else {
	tx_items[i].level0 = 1;
	tx_items[i].level1 = 0;
	}
	out <<= 1;
	}
	out = (txcmd << 2) & 0xff;
	for (int i = 7; i < 13; i++) {    //d5-d0
	tx_items[i].duration0 = T;
	tx_items[i].duration1 = T;
	if (out & 0x80) {
	tx_items[i].level0 = 0;
	tx_items[i].level1 = 1;
	} else {
	tx_items[i].level0 = 1;
	tx_items[i].level1 = 0;
	}
	out <<= 1;
	}
	tx_items[13].level0 = 0;          //sp 88.880ms  88880/1.2625=70400
	tx_items[13].duration0 = 30000;
	tx_items[13].level1 = 0;
	tx_items[13].duration1 = 30000;
	tx_items[14].level0 = 0;
	tx_items[14].duration0 = 10400;
	tx_items[14].level1 = 0;
	tx_items[14].duration1 = 0;       //end
	out = 0;
	while (out < 3) {
	if (rmt_write_items(tidx, tx_items, 15, true) != ESP_OK) out = 255;
	out++;
	}
	if (fdebug) {
	if (out != 255) ESP_LOGI(AP_TAG, "IR RC5 Tx addr: 0x%X, cmd: 0x%X", (taddr & 0x1f), (txcmd & 0x7f));
	else  ESP_LOGI(AP_TAG, "IR RC5 Tx error");
	}
	if (out != 255) result = 1;
	}
	break;
	case 0x04:                        //rc6
	if ((rmt_set_clk_div(tidx, 101) == ESP_OK) && 
	(rmt_set_tx_carrier(tidx, true, 7 * 101, 15 * 101,  1) == ESP_OK)) {
//12.5ns * 101(= 1.2625us) * 22 = 27.775us(36003.6Hz)  T = 352 (22*16)
	uint16_t T = 352;                 //444.4us(22 * 16)
	uint8_t out = 0;
	rmt_item32_t tx_items[24] = {0};

	tx_items[0].level0 = 1;           //ls
	tx_items[0].duration0 = 6 * T;
	tx_items[0].level1 = 0;
	tx_items[0].duration1 = 2 * T;
	tx_items[1].level0 = 1;           //sb
	tx_items[1].duration0 = T;
	tx_items[1].level1 = 0;
	tx_items[1].duration1 = T;
	out = taddr >> 3;
	for (int i = 2; i < 5; i++) {     //mb2-mb0
	tx_items[i].duration0 = T;
	tx_items[i].duration1 = T;
	if (out & 0x80) {
	tx_items[i].level0 = 1;
	tx_items[i].level1 = 0;
	} else {
	tx_items[i].level0 = 0;
	tx_items[i].level1 = 1;
	}
	out <<= 1;
	}
	tx_items[5].level0 = 0;           //tr
	tx_items[5].duration0 = 2 * T;
	tx_items[5].level1 = 1;
	tx_items[5].duration1 = 2 * T;
	out = taddr & 0xff;
	for (int i = 6; i < 14; i++) {    //a7-a0
	tx_items[i].duration0 = T;
	tx_items[i].duration1 = T;
	if (out & 0x80) {
	tx_items[i].level0 = 1;
	tx_items[i].level1 = 0;
	} else {
	tx_items[i].level0 = 0;
	tx_items[i].level1 = 1;
	}
	out <<= 1;
	}
	out = txcmd & 0xff;
	for (int i = 14; i < 22; i++) {   //d7-d0
	tx_items[i].duration0 = T;
	tx_items[i].duration1 = T;
	if (out & 0x80) {
	tx_items[i].level0 = 1;
	tx_items[i].level1 = 0;
	} else {
	tx_items[i].level0 = 0;
	tx_items[i].level1 = 1;
	}
	out <<= 1;
	}
	tx_items[22].level0 = 0;          //sp 88.880ms  88880/1.2625=70400
	tx_items[22].duration0 = 30000;
	tx_items[22].level1 = 0;
	tx_items[22].duration1 = 30000;
	tx_items[23].level0 = 0;
	tx_items[23].duration0 = 10400;
	tx_items[23].level1 = 0;
	tx_items[23].duration1 = 0;       //end
	out = 0;
	while (out < 3) {
	if (rmt_write_items(tidx, tx_items, 24, true) != ESP_OK) out = 255;
	out++;
	}
	if (fdebug) {
	if (out != 255) ESP_LOGI(AP_TAG, "IR RC6 Tx addr: 0x%X, cmd: 0x%X", (taddr & 0x3ff), (txcmd & 0xff));
	else  ESP_LOGI(AP_TAG, "IR RC6 Tx error");
	}
	if (out != 255) result = 1;
	}
	break;
	case 0x05:                        //samsung
	if ((rmt_set_clk_div(tidx, 81) == ESP_OK) && 
	(rmt_set_tx_carrier(tidx, true, 9 * 81, 17 * 81,  1) == ESP_OK)) {
//http://elektrolab.wz.cz/katalog/samsung_protocol.pdf
//12.5ns * 81(= 1.0125us) * 26 = 26.325us(37986.7Hz)  T = 556 (~562.5us/1.0125)
	uint16_t T = 556;                 //~562.5us/1.0125
	uint8_t tnum = 16;
	uint8_t out;
	rmt_item32_t tx_items[36] = {0};

	tx_items[0].level0 = 1;           //sb
	tx_items[0].duration0 = 8 * T;
	tx_items[0].level1 = 0;
	tx_items[0].duration1 = 8 * T;
	out = taddr & 0xff;
	for (int i = 1; i < 9; i++) {     //a0-a7 1
	tx_items[i].level0 = 1;
	tx_items[i].level1 = 0;
	tx_items[i].duration0 = T;
	if (out & 0x01) {
	tx_items[i].duration1 = 3 * T;
	tnum += 2;
	} else tx_items[i].duration1 = T;
	tnum += 2;
	out >>= 1;
	}
	out = taddr & 0xff;
	for (int i = 9; i < 17; i++) {    //a0-a7 2
	tx_items[i].level0 = 1;
	tx_items[i].level1 = 0;
	tx_items[i].duration0 = T;
	if (out & 0x01) {
	tx_items[i].duration1 = 3 * T;
	tnum += 2;
	} else tx_items[i].duration1 = T;
	tnum += 2;
	out >>= 1;
	}
	out = txcmd & 0xff;
	for (int i = 17; i < 25; i++) {   //d0-d7 1
	tx_items[i].level0 = 1;
	tx_items[i].level1 = 0;
	tx_items[i].duration0 = T;
	if (out & 0x01) {
	tx_items[i].duration1 = 3 * T;
	tnum += 2;
	} else tx_items[i].duration1 = T;
	tnum += 2;
	out >>= 1;
	}
	out = ~txcmd & 0xff;
	for (int i = 25; i < 33; i++) {   //~d0-d7 2
	tx_items[i].level0 = 1;
	tx_items[i].level1 = 0;
	tx_items[i].duration0 = T;
	if (out & 0x01) {
	tx_items[i].duration1 = 3 * T;
	tnum += 2;
	} else tx_items[i].duration1 = T;
	tnum += 2;
	out >>= 1;
	}
	tx_items[33].level0 = 1;          //end bit
	tx_items[33].duration0 = T;
	tx_items[33].level1 = 0;
	tx_items[33].duration1 = T;
	tnum += 2;
	T = 53333 - ((tnum >> 1) * T);    //s2s 108ms, 108000/1.0125/2=53333
 	tx_items[34].level0 = 0;
	tx_items[34].duration0 = T;
	tx_items[34].level1 = 0;
	tx_items[34].duration1 = T;
	tx_items[35].level0 = 0;          //end
	tx_items[35].duration0 = 0;
	tx_items[35].level1 = 0;
	tx_items[35].duration1 = 0;
	out = 0;
	while (out < 3) {
	if (rmt_write_items(tidx, tx_items, 36, true) != ESP_OK) out = 255;
	out++;
	}
	if (fdebug) {
	if (out != 255) ESP_LOGI(AP_TAG, "IR SAMSUNG Tx addr: 0x%X, cmd: 0x%X", (taddr & 0xff), (txcmd & 0xff));
	else  ESP_LOGI(AP_TAG, "IR SAMSUNG Tx error");
	}
	if (out != 255) result = 1;
	}
	break;
	case 0x06:                        //sony sirc 12 bit
	if ((rmt_set_clk_div(tidx, 80) == ESP_OK) && 
	(rmt_set_tx_carrier(tidx, true, 8 * 80, 17 * 80,  1) == ESP_OK)) {
//12.5ns * 80(= 1us) * 25 = 25us(40000Hz)  T = 600 (600us)
	uint16_t T = 600;                 //600us
	uint8_t tnum = 5;
	uint16_t out;
	rmt_item32_t tx_items[14] = {0};
	tx_items[0].level0 = 1;           //sb
	tx_items[0].duration0 = 4 * T;
	tx_items[0].level1 = 0;
	tx_items[0].duration1 = T;
	out = txcmd & 0xff;
	for (int i = 1; i < 8; i++) {     //d0-d6
	tx_items[i].level0 = 1;
	tx_items[i].level1 = 0;
	tx_items[i].duration1 = T;
	if (out & 0x01) {
	tx_items[i].duration0 = 2 * T;
	tnum++;
	} else tx_items[i].duration0 = T;
	tnum += 2;
	out >>= 1;
	}
	out = taddr & 0xff;
	for (int i = 8; i < 13; i++) {    //a0-a4
	tx_items[i].level0 = 1;
	tx_items[i].level1 = 0;
	tx_items[i].duration1 = T;
	if (out & 0x01) {
	tx_items[i].duration0 = 2 * T;
	tnum++;
	} else tx_items[i].duration0 = T;
	tnum += 2;
	out >>= 1;
	}
	T = 45000 - (T * tnum);
 	tx_items[13].level0 = 0;          //start to start 45ms
	tx_items[13].duration0 = T;
	tx_items[13].level1 = 0;          //end
	tx_items[13].duration1 = 0;
	out = 0;
	while (out < 3) {
	if (rmt_write_items(tidx, tx_items, 14, true) != ESP_OK) out = 255;
	out++;
	}
	if (fdebug) {
	if (out != 255) ESP_LOGI(AP_TAG, "IR SIRCx12 Tx addr: 0x%X, cmd: 0x%X", (taddr & 0x1f), (txcmd & 0x7f));
	else  ESP_LOGI(AP_TAG, "IR SIRCx12 Tx error");
	}
	if (out != 255) result = 1;
	}
	break;
	case 0x07:                        //sony sirc 15 bit
	if ((rmt_set_clk_div(tidx, 80) == ESP_OK) && 
	(rmt_set_tx_carrier(tidx, true, 8 * 80, 17 * 80,  1) == ESP_OK)) {
//12.5ns * 80(= 1us) * 25 = 25us(40000Hz)  T = 600 (600us)
	uint16_t T = 600;                 //600us
	uint8_t tnum = 5;
	uint16_t out;
	rmt_item32_t tx_items[17] = {0};
	tx_items[0].level0 = 1;           //sb
	tx_items[0].duration0 = 4 * T;
	tx_items[0].level1 = 0;
	tx_items[0].duration1 = T;
	out = txcmd & 0xff;
	for (int i = 1; i < 8; i++) {     //d0-d6
	tx_items[i].level0 = 1;
	tx_items[i].level1 = 0;
	tx_items[i].duration1 = T;
	if (out & 0x01) {
	tx_items[i].duration0 = 2 * T;
	tnum++;
	} else tx_items[i].duration0 = T;
	tnum += 2;
	out >>= 1;
	}
	out = taddr & 0xff;
	for (int i = 8; i < 16; i++) {    //a0-a7
	tx_items[i].level0 = 1;
	tx_items[i].level1 = 0;
	tx_items[i].duration1 = T;
	if (out & 0x01) {
	tx_items[i].duration0 = 2 * T;
	tnum++;
	} else tx_items[i].duration0 = T;
	tnum += 2;
	out >>= 1;
	}
	T = 45000 - (T * tnum);
 	tx_items[16].level0 = 0;          //start to start 45ms
	tx_items[16].duration0 = T;
	tx_items[16].level1 = 0;          //end
	tx_items[16].duration1 = 0;
	out = 0;
	while (out < 3) {
	if (rmt_write_items(tidx, tx_items, 17, true) != ESP_OK) out = 255;
	out++;
	}
	if (fdebug) {
	if (out != 255) ESP_LOGI(AP_TAG, "IR SIRCx15 Tx addr: 0x%X, cmd: 0x%X", (taddr & 0xff), (txcmd & 0x7f));
	else  ESP_LOGI(AP_TAG, "IR SIRCx15 Tx error");
	}
	if (out != 255) result = 1;
	}
	break;
	case 0x08:                        //sony sirc 20 bit
	if ((rmt_set_clk_div(tidx, 80) == ESP_OK) && 
	(rmt_set_tx_carrier(tidx, true, 8 * 80, 17 * 80,  1) == ESP_OK)) {
//12.5ns * 80(= 1us) * 25 = 25us(40000Hz)  T = 600 (600us)
	uint16_t T = 600;                 //600us
	uint8_t tnum = 5;
	uint16_t out;
	rmt_item32_t tx_items[22] = {0};
	tx_items[0].level0 = 1;           //sb
	tx_items[0].duration0 = 4 * T;
	tx_items[0].level1 = 0;
	tx_items[0].duration1 = T;
	out = txcmd & 0xff;
	for (int i = 1; i < 8; i++) {     //d0-d6
	tx_items[i].level0 = 1;
	tx_items[i].level1 = 0;
	tx_items[i].duration1 = T;
	if (out & 0x01) {
	tx_items[i].duration0 = 2 * T;
	tnum++;
	} else tx_items[i].duration0 = T;
	tnum += 2;
	out >>= 1;
	}
	out = taddr;
	for (int i = 8; i < 21; i++) {    //a0-a12
	tx_items[i].level0 = 1;
	tx_items[i].level1 = 0;
	tx_items[i].duration1 = T;
	if (out & 0x01) {
	tx_items[i].duration0 = 2 * T;
	tnum++;
	} else tx_items[i].duration0 = T;
	tnum += 2;
	out >>= 1;
	}
	T = 45000 - (T * tnum);
 	tx_items[21].level0 = 0;          //start to start 45ms
	tx_items[21].duration0 = T;
	tx_items[21].level1 = 0;          //end
	tx_items[21].duration1 = 0;
	out = 0;
	while (out < 3) {
	if (rmt_write_items(tidx, tx_items, 22, true) != ESP_OK) out = 255;
	out++;
	}
	if (fdebug) {
	if (out != 255) ESP_LOGI(AP_TAG, "IR SIRCx20 Tx addr: 0x%X, cmd: 0x%X", (taddr & 0x1fff), (txcmd & 0x7f));
	else  ESP_LOGI(AP_TAG, "IR SIRCx20 Tx error");
	}
	if (out != 255) result = 1;
	}
	break;
	case 0x09:                        //panasonic
	if ((rmt_set_clk_div(tidx, 94) == ESP_OK) && 
	(rmt_set_tx_carrier(tidx, true, 7 * 94, 16 * 94,  1) == ESP_OK)) {
//12.5ns * 94(= 1.175us) * 23 = 27.025(37002.7Hz)  T = 368 (23*16)
	uint16_t T = 368;                 //432.4us
	uint16_t out;
	uint8_t  xor;
	rmt_item32_t tx_items[51] = {0};
	tx_items[0].level0 = 1;           //lsb
	tx_items[0].duration0 = 8 * T;
	tx_items[0].level1 = 0;
	tx_items[0].duration1 = 4 * T;
	out = 0x2002;                     //vendor
	for (int i = 1; i < 17; i++) {    //v0-v15
	tx_items[i].level0 = 1;
	tx_items[i].level1 = 0;
	tx_items[i].duration0 = T;
	if (out & 0x01) tx_items[i].duration1 = 3 * T;
	else tx_items[i].duration1 = T;
	out >>= 1;
	}
	xor = (out & 0xff) ^ ((out >> 8) & 0xff);
	xor = xor ^ (xor >> 4);
	out = xor;
	for (int i = 17; i < 21; i++) {   //4 xor bits
	tx_items[i].level0 = 1;
	tx_items[i].level1 = 0;
	tx_items[i].duration0 = T;
	if (out & 0x01) tx_items[i].duration1 = 3 * T;
	else tx_items[i].duration1 = T;
	out >>= 1;
	}
	out = taddr >> 8;
	xor = (xor & 0x0f) ^ ((taddr >> 4) & 0xf0);
	for (int i = 21; i < 25; i++) {   //4 bit system code high addr
	tx_items[i].level0 = 1;
	tx_items[i].level1 = 0;
	tx_items[i].duration0 = T;
	if (out & 0x01) tx_items[i].duration1 = 3 * T;
	else tx_items[i].duration1 = T;
	out >>= 1;
	}
	out = taddr;
	for (int i = 25; i < 33; i++) {   //a0-a7
	tx_items[i].level0 = 1;
	tx_items[i].level1 = 0;
	tx_items[i].duration0 = T;
	if (out & 0x01) tx_items[i].duration1 = 3 * T;
	else tx_items[i].duration1 = T;
	out >>= 1;
	}
	out = txcmd;
	xor = xor ^ (taddr & 0xff) ^ (txcmd & 0xff);
	for (int i = 33; i < 41; i++) {   //d0-d7
	tx_items[i].level0 = 1;
	tx_items[i].level1 = 0;
	tx_items[i].duration0 = T;
	if (out & 0x01) tx_items[i].duration1 = 3 * T;
	else tx_items[i].duration1 = T;
	out >>= 1;
	}
	out = xor;
	for (int i = 41; i < 49; i++) {   //c0-c7
	tx_items[i].level0 = 1;
	tx_items[i].level1 = 0;
	tx_items[i].duration0 = T;
	if (out & 0x01) tx_items[i].duration1 = 3 * T;
	else tx_items[i].duration1 = T;
	out >>= 1;
	}
	tx_items[49].level0 = 1;          //end bit
	tx_items[49].duration0 = T;
	tx_items[49].level1 = 0;
	tx_items[49].duration1 = 30000;   //sp 42.2ms
 	tx_items[50].level0 = 0;          //42200/1.175=35915
	tx_items[50].duration0 = 5915;
	tx_items[50].level1 = 0;          //end
	tx_items[50].duration1 = 0;
	out = 0;
	while (out < 3) {
	if (rmt_write_items(tidx, tx_items, 51, true) != ESP_OK) out = 255;
	out++;
	}
	if (fdebug) {
	if (out != 255) ESP_LOGI(AP_TAG, "IR PANASONIC Tx addr: 0x%X, cmd: 0x%X", (taddr & 0xfff), (txcmd & 0xff));
	else  ESP_LOGI(AP_TAG, "IR PANASONIC Tx error");
	}
	if (out != 255) result = 1;
	}
	break;
	}
	txcmd &= 0xff;
	*ptxcmd = txcmd;
	*pprtxcmd = (prtxcmd & 0xff) | 0xff00;
	return result;
}
#endif

//***************** Hx711 *********************

bool    rmthx_ssck(uint8_t idx, uint16_t usec)
{
	bool result = 0;
	uint8_t tidx;
#ifdef CONFIG_IDF_TARGET_ESP32C3
	if (idx > 1) return result;
	tidx = idx;
#else
	if (idx > 2) return result;
	tidx = idx << 1;
#endif
	rmt_item32_t tx_items[2] = {0};
	tx_items[0].level0 = 0;
	tx_items[0].duration0 = 5;
	tx_items[0].level1 = 1;
	tx_items[0].duration1 = usec & 0x7fff;
	tx_items[1].level0 = 0;
	tx_items[1].duration0 = 5;
	tx_items[1].level1 = 0;
	tx_items[1].duration1 = 0;
	if (rmt_write_items(tidx, tx_items, 2, true) == ESP_OK) result = 1;
	return result;
}
bool rmthx_init (uint8_t idx, uint8_t  gpio_num)
{
	bool result = 0;
	uint8_t tidx;
#ifdef CONFIG_IDF_TARGET_ESP32C3
	if (idx > 1) return result;
	tidx = idx;
#else
	if (idx > 2) return result;
	tidx = idx << 1;
#endif
//rmt tx init
	rmt_config_t rmt_tx = {0};
	rmt_tx.channel = tidx;
	rmt_tx.gpio_num = gpio_num;
	rmt_tx.mem_block_num = 1;
	rmt_tx.clk_div = 80;
	rmt_tx.tx_config.loop_en = false;
	rmt_tx.tx_config.carrier_en = false;
	rmt_tx.tx_config.carrier_level = 1;
	rmt_tx.tx_config.idle_level = 0;
	rmt_tx.tx_config.idle_output_en = true;
	rmt_tx.rmt_mode = RMT_MODE_TX;
	if (rmt_config(&rmt_tx) == ESP_OK) {  //1
	rmt_set_source_clk(rmt_tx.channel, RMT_BASECLK_APB);
	if (rmt_driver_install(rmt_tx.channel, 0, ESP_INTR_FLAG_LOWMED
			| ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_SHARED) == ESP_OK) result = rmthx_ssck(idx, 1000);
//end of init	
	}  //1
	return result;
}
bool readHx(uint8_t idx,  uint32_t* data) 
{
	bool result = 0;
	uint32_t in = 0;
	if (gpio_get_level(bgpio5 & 0x3f)) {
	in = 0xffffffff;
	} else {
	for(int i = 0; i < 24; ++i) {
	if (rmthx_ssck(idx, 20)) in |= gpio_get_level(bgpio5 & 0x3f) << (31 - i);
	else return result;
	}
        if (!rmthx_ssck(idx, 20)) return result;
	result = gpio_get_level(bgpio5 & 0x3f);
	}
	if (result) *data = in;
	else {
	*data = 0xffffffff;
	rmthx_ssck(idx, 1000);
	}
//ESP_LOGI(AP_TAG, "Hx read: 0x%X, end: %d", in, result);
	return result;
}

//***************** 1w 18b20 ******************
//https://github.com/DavidAntliff/esp32-owb
bool rmt1w_init (uint8_t idx, uint8_t  gpio_num, RingbufHandle_t RmtRgHd)
{
	bool result = 0;
	uint8_t tidx, ridx;
#ifdef CONFIG_IDF_TARGET_ESP32C3
	if (idx > 1) return result;
	tidx = idx;
	ridx = idx + 2;	
#else
	if (idx > 2) return result;
	tidx = idx << 1;
	ridx = (idx << 1) + 1;	
#endif
//rmt tx init
	rmt_config_t rmt_tx = {0};
	rmt_tx.channel = tidx;
	rmt_tx.gpio_num = gpio_num;
	rmt_tx.mem_block_num = 1;
	rmt_tx.clk_div = 80;
	rmt_tx.tx_config.loop_en = false;
	rmt_tx.tx_config.carrier_en = false;
	rmt_tx.tx_config.idle_level = 1;
	rmt_tx.tx_config.idle_output_en = true;
	rmt_tx.rmt_mode = RMT_MODE_TX;
	if (rmt_config(&rmt_tx) == ESP_OK) {  //1
	rmt_set_source_clk(rmt_tx.channel, RMT_BASECLK_APB);
	if (rmt_driver_install(rmt_tx.channel, 0, ESP_INTR_FLAG_LOWMED
			| ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_SHARED) == ESP_OK) {  //2
//rmt rx init
	rmt_config_t rmt_rx = {0};
	rmt_rx.channel = ridx;
	rmt_rx.gpio_num = gpio_num;
	rmt_rx.clk_div = 80;
	rmt_rx.mem_block_num = 1;
	rmt_rx.rmt_mode = RMT_MODE_RX;
	rmt_rx.rx_config.filter_en = true;
	rmt_rx.rx_config.filter_ticks_thresh = 30;
	rmt_rx.rx_config.idle_threshold = 77;  // slot(75us) + 2
	if (rmt_config(&rmt_rx) == ESP_OK) { //3
	rmt_set_source_clk(rmt_rx.channel, RMT_BASECLK_APB);
	if (rmt_driver_install(rmt_rx.channel, 512, ESP_INTR_FLAG_LOWMED
			| ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_SHARED) == ESP_OK)	{  //4
	rmt_get_ringbuf_handle(rmt_rx.channel, RmtRgHd);
//pin
#ifdef CONFIG_IDF_TARGET_ESP32C3
	GPIO.enable_w1ts.val = (0x1 << gpio_num);
#else
	if (gpio_num < 32) GPIO.enable_w1ts = (0x1 << gpio_num);
	else GPIO.enable1_w1ts.data = (0x1 << (gpio_num - 32));
#endif
//set pin for rx first since gpio_output_disable() will remove rmt output signal in matrix!
	rmt_set_gpio(rmt_rx.channel, RMT_MODE_RX, gpio_num,0);
	rmt_set_gpio(rmt_tx.channel, RMT_MODE_TX, gpio_num,0);
// force pin direction to input to enable path to RX channel
	PIN_INPUT_ENABLE(GPIO_PIN_MUX_REG[gpio_num]);
// enable open drain
	GPIO.pin[gpio_num].pad_driver = 1;
//end of init	
	result = 1;
	} else rmt_driver_uninstall(rmt_tx.channel); //4
	} else rmt_driver_uninstall(rmt_tx.channel); //3
	}  //2
	}  //1
	return result;
}
void rmt_flush__rxbuf (RingbufHandle_t RmtRgHd)
{
	void * ptr = NULL;
	size_t sz = 0;
	while ((ptr = xRingbufferReceive(RmtRgHd, &sz, 0))) vRingbufferReturnItem(RmtRgHd, ptr);
}
bool    rmt1w_dsres(uint8_t idx, RingbufHandle_t RmtRgHd)
{
	bool result = 0;
	uint8_t tidx, ridx;
#ifdef CONFIG_IDF_TARGET_ESP32C3
	if (idx > 1) return result;
	tidx = idx;
	ridx = idx + 2;	
#else
	if (idx > 2) return result;
	tidx = idx << 1;
	ridx = (idx << 1) + 1;	
#endif
	rmt_item32_t tx_items[1] = {0};
	uint16_t old_rx_thresh = 0;
	tx_items[0].duration0 = 500; // 500us
	tx_items[0].level0 = 0;
	tx_items[0].duration1 = 0;   //end
	tx_items[0].level1 = 1;
	rmt_get_rx_idle_thresh(ridx, &old_rx_thresh);
	rmt_set_rx_idle_thresh(ridx, 500 + 60);
	rmt_flush__rxbuf (RmtRgHd);
	rmt_rx_start(ridx, true);
	if (rmt_write_items(tidx, tx_items, 1, true) == ESP_OK) {  //1
	size_t rx_size = 0;
	rmt_item32_t * rx_items = (rmt_item32_t *)xRingbufferReceive(RmtRgHd, &rx_size, 100 / portTICK_PERIOD_MS);
	if (rx_items) { //2
	if (rx_size >= (1 * sizeof(rmt_item32_t))) {  //3
// parse signal and search for presence pulse
	if ((rx_items[0].level0 == 0) && (rx_items[0].duration0 >= 500 - 2)) { //4
		if ((rx_items[0].level1 == 1) && (rx_items[0].duration1 > 0)) { //5
			if (rx_items[1].level0 == 0) result = 1;
			}  //5
		} //4
	} //3
	vRingbufferReturnItem(RmtRgHd, (void *)rx_items);
	} //2
	} //1
	rmt_rx_stop(ridx);
	rmt_set_rx_idle_thresh(ridx, old_rx_thresh);
// end
	return result;
}
bool    rmt1w_dscomm(uint8_t idx, uint8_t comm)
{
	bool result = 0;
	uint8_t tidx;
#ifdef CONFIG_IDF_TARGET_ESP32C3
	if (idx > 1) return result;
	tidx = idx;
#else
	if (idx > 2) return result;
	tidx = idx << 1;
#endif
	uint8_t out = comm;
	rmt_item32_t tx_items[9] = {0};   //8bits + end marker
	rmt_item32_t item = {0};
	item.level0 = 0;
	item.level1 = 1;
	for (int i = 0; i < 8; i++) {
	if (out & 0x01) {
// write "1" slot
	item.duration0 = 5;               //1_LOW;
	item.duration1 = 70;              //1_HIGH;
	} else {
// write "0" slot
        item.duration0 = 65;              //0_LOW;
        item.duration1 = 10;              //0_HIGH;
	}
        tx_items[i] = item;
	out >>= 1;
	}
// end marker
	tx_items[8].level0 = 1;
	tx_items[8].duration0 = 0;
	if (rmt_write_items(tidx, tx_items, 9, true) == ESP_OK) result = 1;
	return result;
}
bool    rmt1w_dsread(uint8_t idx, uint8_t* data, RingbufHandle_t RmtRgHd)
{
	bool result = 0;
	uint8_t in = 0;
	uint8_t tidx, ridx;
#ifdef CONFIG_IDF_TARGET_ESP32C3
	if (idx > 1) return result;
	tidx = idx;
	ridx = idx + 2;	
#else
	if (idx > 2) return result;
	tidx = idx << 1;
	ridx = (idx << 1) + 1;	
#endif
	rmt_item32_t tx_items[9] = {0};   //8bits + end marker
	rmt_item32_t item = {0};
	item.level0 = 0;
	item.level1 = 1;
	item.duration0 = 5;               //1_LOW;
	item.duration1 = 70;              //1_HIGH;
	for (int i = 0; i < 8; i++) tx_items[i] = item;
// end marker
	tx_items[8].level0 = 1;
	tx_items[8].duration0 = 0;
	rmt_flush__rxbuf (RmtRgHd);
	rmt_rx_start(ridx, true);
	if (rmt_write_items(tidx, tx_items, 9, true) == ESP_OK) { //1
	size_t rx_size = 0;
	rmt_item32_t *rx_items = (rmt_item32_t *)xRingbufferReceive(RmtRgHd, &rx_size, 100 / portTICK_PERIOD_MS);
	if (rx_items) { //2
	if (rx_size >= 8 * sizeof(rmt_item32_t)) { //3
	for (int i = 0; i < 8; i++) { //4
	in >>= 1;
// parse signal and identify logical bit
	if (rx_items[i].level1 == 1) { //5
	if ((rx_items[i].level0 == 0) && (rx_items[i].duration0 < 13)) { //6  15us-2
// rising edge occured before 15us -> bit 1
	in |= 0x80;
	} //6
	} //5
	} //4
	result = 1;
	} //3
	vRingbufferReturnItem(RmtRgHd, (void *)rx_items);
	} //2
	} //1
	rmt_rx_stop(ridx);
	*data = in;
	return result;
}
void rmt1w_readds (uint8_t idx, uint8_t* f_rmds, uint16_t* temp, RingbufHandle_t RmtRgHd)
{
#ifdef CONFIG_IDF_TARGET_ESP32C3
	if (idx > 1) return;
#else
	if (idx > 2) return;
#endif
	uint8_t rmbits = *f_rmds;
	uint8_t ld = 0xff;
	uint8_t hd = 0xff;
	if (rmbits & (0x10 << idx)) {  //1 if convert t command sended before
	if (rmt1w_dsres(idx, RmtRgHd)) { //2 if reset w1 bus ok
	if (rmt1w_dscomm(idx, 0xcc)) { //3 skip rom command
	if (rmt1w_dscomm(idx, 0xbe)) { //4 read command
	if (rmt1w_dsread(idx, &ld, RmtRgHd)) { //5
	if (!rmt1w_dsread(idx, &hd, RmtRgHd)) { //6
	ld = 0xff;
	hd = 0xff;
	} //6
	} else ld = 0xff;//5
	} //4
	} //3
	} //2
	*temp = (hd << 8) + ld;
	ld = (0x10 << idx);
	ld = ~ld;
	rmbits = rmbits & ld;
	} //1
	if (rmt1w_dsres(idx, RmtRgHd)) { //1
	if (rmt1w_dscomm(idx, 0xcc)) { //2 skip rom command
	if (rmt1w_dscomm(idx, 0x44)) { //3 convert t command
	rmbits = rmbits | (0x10 << idx);
	} else *temp = 0xffff;  //3
	} else *temp = 0xffff;  //2
	} else *temp = 0xffff;  //1
	*f_rmds = rmbits;
}


void rmt1w_readdht (uint8_t idx, uint8_t* f_rmds, uint16_t* temp, uint16_t* humid, RingbufHandle_t RmtRgHd)
{
	rmt_item32_t tx_items[1] = {0};
	uint8_t tidx, ridx;
#ifdef CONFIG_IDF_TARGET_ESP32C3
	if (idx > 1) return;
	tidx = idx;
	ridx = idx + 2;	
#else
	if (idx > 2) return;
	tidx = idx << 1;
	ridx = (idx << 1) + 1;	
#endif
	uint32_t in = 0;
	tx_items[0].duration0 = 5000; // 5ms
	uint16_t old_rx_thresh = 0;
	tx_items[0].level0 = 0;
	tx_items[0].duration1 = 0;   //end
	tx_items[0].level1 = 1;
	rmt_get_rx_idle_thresh(ridx, &old_rx_thresh);
	rmt_set_rx_idle_thresh(ridx, 5000 + 100);
	rmt_flush__rxbuf (RmtRgHd);
	rmt_rx_start(ridx, true);
	if (rmt_write_items(tidx, tx_items, 1, true) == ESP_OK) {  //1
	size_t rx_size = 0;
	rmt_item32_t *rx_items = (rmt_item32_t *)xRingbufferReceive(RmtRgHd, &rx_size, 100 / portTICK_PERIOD_MS);
	if (rx_items) { //2
	if (rx_size >= 34 * sizeof(rmt_item32_t)) { //3
	for (int i = 0; i < 34; i++) { //4
	in <<= 1;
// parse signal and identify logical bit
	if ((rx_items[i].level1 == 1) && (rx_items[i].duration1 > 50) && 
	(rx_items[i].level0 == 0) && (rx_items[i].duration0 < 90)) in |= 0x01;
	} //4
// all data bits readed
	}  else in = 0xffff; //3
	vRingbufferReturnItem(RmtRgHd, (void *)rx_items);
	} else in = 0xffff; //2
	} else in = 0xffff; //1
	rmt_rx_stop(ridx);
	rmt_set_rx_idle_thresh(ridx, old_rx_thresh);
	*temp = in & 0xffff;
	*humid = in >> 16;
// end
}

//****************** i2c **********************
esp_err_t i2c_init_bus() 
{
/*
timeout:  10us(1bit @100kbit) * 10 bit(~byte+start/stop/ack) * 256 bytes(max) = 25.6ms
assume 80ms for read, 40ms for write/check(2/1 bytes)
*/
	esp_err_t err = -1;
	if ((bgpio9 < 192) | (bgpio10 < 192) | (bgpio9 > 225) | (bgpio10 > 225)) return err;
	i2c_config_t i2c_config = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = (bgpio10 & 0x3f),
		.scl_io_num = (bgpio9 & 0x3f),
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
		.master.clk_speed = 100000
	};
	i2c_param_config(I2C_NUM_0, &i2c_config);
	err = i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, ESP_INTR_FLAG_IRAM);
	return err;
}
esp_err_t i2c_check(uint8_t addr)
{
//no count errors
	esp_err_t err = -1;
	i2c_cmd_handle_t CmdHd;
	CmdHd = i2c_cmd_link_create();
	i2c_master_start(CmdHd);
	i2c_master_write_byte(CmdHd, addr << 1, 1);
	i2c_master_stop(CmdHd);
	err = i2c_master_cmd_begin(I2C_NUM_0, CmdHd, 40);
	i2c_cmd_link_delete(CmdHd);
	if (err == ESP_ERR_TIMEOUT) { // if i2c bus busy errcnt++
	i2c_errcnt++;
	if (!i2c_errcnt) i2c_errcnt--;
	}
	return err;
}
esp_err_t i2c_devnum(uint32_t* f_i2cdev, uint32_t* s_i2cdev, uint8_t* i2cdevnum)
{
	esp_err_t err = -1;
	uint32_t i2cbits = * f_i2cdev;
	uint32_t i2csbits = 0;
	uint8_t i,j;
	if (!(i2cbits & 0x80000000)) return err;
	uint8_t devnum = 0;
	for (i = 1; i < 128; i++) {
	err = i2c_check(i);
	if (!err) {
	if (fdebug) ESP_LOGI(AP_TAG, "Found i2c addr: 0x%X", i);
	for (j = 0; j < 28; j++) {
	if (i == i2c_addr[j]) i2csbits = i2csbits | 0x01 << j;
	}
	devnum++;
	} else if (err == ESP_ERR_TIMEOUT) {
	*i2cdevnum = 0;                 // if i2c bus busy stop scanning
	*s_i2cdev = 0;
	return err;
	}
	}
	*i2cdevnum = devnum;
	*s_i2cdev = i2csbits;
//	if (fdebug) ESP_LOGI(AP_TAG, "Found i2c bits: 0x%X", i2csbits);
	err = 0;
	return err;
}
esp_err_t i2c_read0_data(uint8_t addr, uint8_t* data, uint16_t len)
{
	esp_err_t err = -1;
	i2c_cmd_handle_t CmdHd;
	if (len) {
	CmdHd = i2c_cmd_link_create();
	i2c_master_start(CmdHd);
	i2c_master_write_byte(CmdHd, (addr << 1) + 1, I2C_MASTER_ACK);
	if (len > 1) i2c_master_read(CmdHd, data, len, I2C_MASTER_ACK);
	i2c_master_read_byte(CmdHd, data+len-1, I2C_MASTER_NACK);
	i2c_master_stop(CmdHd);
	err = i2c_master_cmd_begin(I2C_NUM_0, CmdHd, 80);
	i2c_cmd_link_delete(CmdHd);
	}
	if (err) {
	i2c_errcnt++;
	if (!i2c_errcnt) i2c_errcnt--;
	}
	return err;
}
esp_err_t i2c_read_data(uint8_t addr, uint8_t reg, uint8_t* data, uint16_t len)
{
	esp_err_t err = -1;
	i2c_cmd_handle_t CmdHd;
	CmdHd = i2c_cmd_link_create();
	i2c_master_start(CmdHd);
	i2c_master_write_byte(CmdHd, addr << 1, I2C_MASTER_ACK);
	i2c_master_write_byte(CmdHd, reg, I2C_MASTER_ACK);
	if (len) {
	i2c_master_start(CmdHd);
	i2c_master_write_byte(CmdHd, (addr << 1) + 1, I2C_MASTER_ACK);
	if (len > 1) i2c_master_read(CmdHd, data, len, I2C_MASTER_ACK);
	i2c_master_read_byte(CmdHd, data+len-1, I2C_MASTER_NACK);
	}
	i2c_master_stop(CmdHd);
	err = i2c_master_cmd_begin(I2C_NUM_0, CmdHd, 80);
	i2c_cmd_link_delete(CmdHd);
	if (err) {
	i2c_errcnt++;
	if (!i2c_errcnt) i2c_errcnt--;
	}
	return err;
}
esp_err_t i2c_read2_data(uint8_t addr, uint8_t reg, uint8_t cmd, uint8_t* data, uint16_t len)
{
	esp_err_t err = -1;
	i2c_cmd_handle_t CmdHd;
	CmdHd = i2c_cmd_link_create();
	i2c_master_start(CmdHd);
	i2c_master_write_byte(CmdHd, addr << 1, I2C_MASTER_ACK);
	i2c_master_write_byte(CmdHd, reg, I2C_MASTER_ACK);
	i2c_master_write_byte(CmdHd, cmd, I2C_MASTER_ACK);
	if (len) {
	i2c_master_start(CmdHd);
	i2c_master_write_byte(CmdHd, (addr << 1) + 1, I2C_MASTER_ACK);
	if (len > 1) i2c_master_read(CmdHd, data, len, I2C_MASTER_ACK);
	i2c_master_read_byte(CmdHd, data+len-1, I2C_MASTER_NACK);
	}
	i2c_master_stop(CmdHd);
	err = i2c_master_cmd_begin(I2C_NUM_0, CmdHd, 80);
	i2c_cmd_link_delete(CmdHd);
	if (err) {
	i2c_errcnt++;
	if (!i2c_errcnt) i2c_errcnt--;
	}
	return err;
}
esp_err_t i2c_read3_data(uint8_t addr, uint8_t reg, uint8_t cmd, uint8_t par, uint8_t* data, uint16_t len)
{
	esp_err_t err = -1;
	i2c_cmd_handle_t CmdHd;
	CmdHd = i2c_cmd_link_create();
	i2c_master_start(CmdHd);
	i2c_master_write_byte(CmdHd, addr << 1, I2C_MASTER_ACK);
	i2c_master_write_byte(CmdHd, reg, I2C_MASTER_ACK);
	i2c_master_write_byte(CmdHd, cmd, I2C_MASTER_ACK);
	i2c_master_write_byte(CmdHd, par, I2C_MASTER_ACK);
	if (len) {
	i2c_master_start(CmdHd);
	i2c_master_write_byte(CmdHd, (addr << 1) + 1, I2C_MASTER_ACK);
	if (len > 1) i2c_master_read(CmdHd, data, len, I2C_MASTER_ACK);
	i2c_master_read_byte(CmdHd, data+len-1, I2C_MASTER_NACK);
	}
	i2c_master_stop(CmdHd);
	err = i2c_master_cmd_begin(I2C_NUM_0, CmdHd, 80);
	i2c_cmd_link_delete(CmdHd);
	if (err) {
	i2c_errcnt++;
	if (!i2c_errcnt) i2c_errcnt--;
	}
	return err;
}
esp_err_t i2c_write_byte(uint8_t addr, uint8_t reg, uint8_t value)
{
	esp_err_t err = -1;
	i2c_cmd_handle_t CmdHd;
	CmdHd = i2c_cmd_link_create();
	i2c_master_start(CmdHd);
	i2c_master_write_byte(CmdHd, addr << 1, I2C_MASTER_ACK);
	i2c_master_write_byte(CmdHd, reg, I2C_MASTER_ACK);
	i2c_master_write_byte(CmdHd, value, I2C_MASTER_ACK);
	i2c_master_stop(CmdHd);
	err = i2c_master_cmd_begin(I2C_NUM_0, CmdHd, 40);
	i2c_cmd_link_delete(CmdHd);
	if (err) {
	i2c_errcnt++;
	if (!i2c_errcnt) i2c_errcnt--;
	}
	return err;
}
esp_err_t i2c_write_data(uint8_t addr, uint8_t reg, uint8_t* data, uint16_t len)
{
	esp_err_t err = -1;
	i2c_cmd_handle_t CmdHd;
	CmdHd = i2c_cmd_link_create();
	i2c_master_start(CmdHd);
	i2c_master_write_byte(CmdHd, addr << 1, I2C_MASTER_ACK);
	i2c_master_write_byte(CmdHd, reg, I2C_MASTER_ACK);
	if (len) i2c_master_write(CmdHd, data, len, I2C_MASTER_ACK);
	i2c_master_stop(CmdHd);
	err = i2c_master_cmd_begin(I2C_NUM_0, CmdHd, 40);
	i2c_cmd_link_delete(CmdHd);
	if (err) {
	i2c_errcnt++;
	if (!i2c_errcnt) i2c_errcnt--;
	}
	return err;
}
//************* ip5306 & axp192 ***************
//https://sharvielectronics.com/wp-content/uploads/2021/07/IP5306-I2C-registers.pdf
//https://mobile.twitter.com/lovyan03/status/1104548868199337984
//https://docs.nanoframework.net/devicesdetails/Ip5306/README.html
esp_err_t i2c_init_pwr (uint32_t* f_i2cdev) 
{
	esp_err_t err = -1;
	uint32_t i2cbits = * f_i2cdev;
	if (!(i2cbits & 0x80000000)) return err;
	uint8_t addr;
	if (!i2c_check(0x75)) {           //ip5306
	addr = 0x75;
	uint8_t var = 0;   
//ip5306 setting for m5stack or ttgo-t4
	const uint8_t ip5306_addr[8] = {0x00, 0x01, 0x02, 0x20, 0x21, 0x22, 0x23, 0x24};
	const uint8_t ip5306_data[8] = {0x35, 0x1d, 0x64, 0x01, 0x89, 0x02, 0xbf, 0xd6};
//detect ip5306 by register 0x77
	err = i2c_read_data(addr, 0x77, &var, 1);
	if (err || ((var & 0xf8) != 0x80)) return err;
	err =i2c_write_byte(addr, 0x77, 0x87);
	if (err) return err;
	err = i2c_read_data(addr, 0x77, &var, 1);
	if (err || (var ^ 0x80)) return err;
//verify all settings for m5stack or ttgo-t4
	for (int i = 0; i < 8; i++) {
	err = i2c_read_data(addr, ip5306_addr[i], &var, 1);
	if (err) return err;
	if (var != ip5306_data[i]) {
	err = i2c_write_byte(addr, ip5306_addr[i], ip5306_data[i]);
	if (err) return err;
	err = i2c_read_data(addr, ip5306_addr[i], &var, 1);
	if (err || (var != ip5306_data[i])) return err;
	}
	}
	err = 0;
	i2cbits = i2cbits | 0x40000000;
	*f_i2cdev = i2cbits;
	} else if (!i2c_check(0x34)) {    //axp192
	addr = 0x34;
	uint8_t var;
	uint8_t buf[8];
	memset(buf, 0xff, sizeof(buf));
	err = i2c_read_data(addr, 0x00, buf, 8);
	if (err) return err;
	if (!(buf[0] & 0xfb) || buf[2] || buf[4] || buf[5]) return err;
//https://github.com/m5stack/M5Tough
	err =  i2c_read_data(addr, 0x30, &var, 1);
	if (err) return err;
	var = (var & 0x04) | 0x02;  //vbus V-limit off, curr 0.5A
	err = i2c_write_byte(addr, 0x30, var);	
	if (err) return err;
	err =  i2c_read_data(addr, 0x35, &var, 1);
	if (err) return err;
	var = (var & 0x1c) | 0xa2;  //rtc charging
	err = i2c_write_byte(addr, 0x35, var);	
	if (err) return err;
	err =  i2c_read_data(addr, 0x26, &var, 1);
	if (err) return err;
	var = (var & 0x80) | 0x6a;  //dc1(esp32) voltage 3.35v
	err = i2c_write_byte(addr, 0x26, var);	
	if (err) return err;
	err =  i2c_read_data(addr, 0x27, &var, 1);
	if (err) return err;
	var = (var & 0x80) | 0x68;  //dc3 voltage 3.3v
	err = i2c_write_byte(addr, 0x27, var);	
	if (err) return err;
	err =  i2c_read_data(addr, 0x12, &var, 1);
	if (err) return err;
	var = var | 0x01;  //dc1 enable
	err = i2c_write_byte(addr, 0x12, var);	
	if (err) return err;
	err =  i2c_read_data(addr, 0x33, &var, 1);
	if (err) return err;
	var = var & 0xf0;  //0 - charge current 100ma
	err = i2c_write_byte(addr, 0x33, var);	
	if (err) return err;
	err = i2c_write_byte(addr, 0x82, 0xff); //all adc enable
	if (err) return err;
	err = i2c_write_byte(addr, 0x36, 0x4c); //pwr switch delays
	if (err) return err;
// vbus for tough
	err =  i2c_read_data(addr, 0x00, &var, 1);
	if (err) return err;
	if (var & 0x08) {     //use vbus in
	err =  i2c_read_data(addr, 0x30, &var, 1);
	if (err) return err;
	var |= 0x80;  //enable vbus
	err = i2c_write_byte(addr, 0x30, var);	
	if (err) return err;
	err =  i2c_read_data(addr, 0x12, &var, 1);
	if (err) return err;
	var = var & 0xbf;                       //set EXTEN to disable 5v boost
	err = i2c_write_byte(addr, 0x12, var);	
	if (err) return err;
	vTaskDelay(200 / portTICK_RATE_MS);
	err =  i2c_read_data(addr, 0x90, &var, 1);
	if (err) return err;
	var = (var & 0xf8) | 0x01;              //set GPIO0 to float , using enternal pulldown resistor to enable supply from BUS_5VS
	err = i2c_write_byte(addr, 0x90, var);	
	if (err) return err;
	} else {              //enable 5v out
	err =  i2c_read_data(addr, 0x91, &var, 1);
	if (err) return err;
	var = (var & 0x0f) | 0xf0;
	err = i2c_write_byte(addr, 0x91, var);	
	if (err) return err;
	err =  i2c_read_data(addr, 0x90, &var, 1);
	if (err) return err;
	var = (var & 0xf8) | 0x02;              //set GPIO0 to LDO OUTPUT , pullup N_VBUSEN to disable supply from BUS_5V
	err = i2c_write_byte(addr, 0x90, var);	
	if (err) return err;
	err =  i2c_read_data(addr, 0x12, &var, 1);
	if (err) return err;
	var = var | 0x40;
	err = i2c_write_byte(addr, 0x12, var);	
	if (err) return err;
	}
	err = 0;
	i2cbits = i2cbits | 0x20000000;
	*f_i2cdev = i2cbits;
	}
	return err;
}
esp_err_t i2c_read_pwr (uint32_t* f_i2cdev, uint8_t* pwr_batmode, uint8_t* pwr_batlevp, uint16_t* pwr_batlevv, uint16_t* pwr_batlevc) 
{
	esp_err_t err = -1;
	uint32_t i2cbits = * f_i2cdev;
	uint8_t pwrbatmode = 0;
	uint8_t pwrbatlevp = 0;
	uint16_t pwrbatlevv = 0;
	uint16_t pwrbatlevc = 0;
	if (!(i2cbits & 0x80000000)) return err;
	uint8_t data[8] = {0};   
	uint8_t addr;
	if (i2cbits & 0x40000000) {
	addr = 0x75;
	err = i2c_read_data(addr, 0x70, data, 1);
	if (!err) {  //70
	err = i2c_read_data(addr, 0x71, &data[1], 1);
	if (!err) {  //71
	err = i2c_read_data(addr, 0x72, &data[2], 1);
	if (!err) {  //72
	err = i2c_read_data(addr, 0x77, &data[3], 1);
	if (!err) {  //77
	err = i2c_read_data(addr, 0x78, &data[4], 1);
	if (!err) {  //78
	switch (data[4] & 0xf0) {
	case 0xe0:
        pwrbatlevp = 25;
	break;
	case 0xc0:
        pwrbatlevp = 50;
	break;
	case 0x80:
        pwrbatlevp = 75;
	break;
	case 0x00:
        pwrbatlevp = 100;
	break;
	default:
        pwrbatlevp = 0;
	break;
	}
	if (data[0] & 0x08) pwrbatmode = pwrbatmode | 0x02;
	if (data[1] & 0x08) pwrbatmode = pwrbatmode | 0x01;
	err = 0;
	*pwr_batmode = pwrbatmode;
	*pwr_batlevp = pwrbatlevp;
	*pwr_batlevv = pwrbatlevv;
	*pwr_batlevc = pwrbatlevc;
	} //78
	} //77
	} //72
	} //71
	} //70
	} else if (i2cbits & 0x20000000) {
	addr = 0x34;
	err = i2c_read_data(addr, 0x00, data, 4);
	if (!err) {  //00
/*
	if (data[0] & 0x04) pwrbatmode |= 0x02;
	if (!(data[1] & 0x40)) pwrbatmode |= 0x01;
*/
	if (!(data[1] & 0x20)) pwrbatmode |= 0x07;
	if (pwrbatmode & 0x04) {
	*pwr_batmode = pwrbatmode;
	*pwr_batlevp = 0;
	*pwr_batlevv = 0;
	*pwr_batlevc = 0;
	} else {
	err = i2c_read_data(addr, 0x78, data, 8);
	if (!err) {  //78
        pwrbatlevv = (data[0] << 4) | (data[1] & 0x0f);
	pwrbatlevv = pwrbatlevv + ((pwrbatlevv / 5) >> 1);
	pwrbatlevc = (((data[2] << 5) | (data[3] & 0x1f)) >> 1) -
			(((data[4] << 5) | (data[5] & 0x1f)) >> 1);
	if (pwrbatlevv < 3248) pwrbatlevp = 0;
	else pwrbatlevp = (pwrbatlevv - 3120) / 10;
	if (pwrbatlevp > 100) pwrbatlevp = 100;
	if (!pwrbatlevc) pwrbatmode |= 0x03;
	else if (!(pwrbatlevc & 0x8000)) pwrbatmode |= 0x02;
	err = 0;
	*pwr_batmode = pwrbatmode;
	*pwr_batlevp = pwrbatlevp;
	*pwr_batlevv = pwrbatlevv;
	*pwr_batlevc = pwrbatlevc;
	} //78
	}
	} //00
	}
	if (err) {
	*pwr_batmode = 4;
	*pwr_batlevp = 0;
	*pwr_batlevv = 0;
	*pwr_batlevc = 0;
	}
	return err;
}
esp_err_t i2c_axpin_set (uint32_t* f_i2cdev, uint8_t gpio, uint8_t val) 
{
	esp_err_t err = -1;
	uint32_t i2cbits = * f_i2cdev;
	if (!(i2cbits & 0x80000000) || !(i2cbits & 0x20000000) || (gpio < 40) || (gpio > 48)) return err;
	uint8_t buf[4] = {0};
	uint8_t addr;
	uint8_t val1;
	addr = 0x34;
	switch (gpio) {
/*
	case 40:     //gpio 0
	if (!i2c_read_data(addr, 0x90, &buf[0], 1) && !i2c_read_data(addr, 0x94, &buf[1], 1)) {
	err = 0;
	if (buf[0] & 0x07) err = i2c_write_byte(addr, 0x90, (buf[0] & 0xf8));
	if (val) {
	if (!err && !(buf[1] & 0x01)) err = i2c_write_byte(addr, 0x94, (buf[1] | 0x01));
	} else {
	if (!err && (buf[1] & 0x01)) err = i2c_write_byte(addr, 0x94, (buf[1] & ~0x01));
	}
	}
	break;
*/
	case 41:     //gpio 1
	if (!i2c_read_data(addr, 0x92, &buf[0], 1) && !i2c_read_data(addr, 0x94, &buf[1], 1)) {
	err = 0;
	if (buf[0] & 0x07) err = i2c_write_byte(addr, 0x92, (buf[0] & 0xf8));
	if (val) {
	if (!err && !(buf[1] & 0x02)) err = i2c_write_byte(addr, 0x94, (buf[1] | 0x02));
	} else {
	if (!err && (buf[1] & 0x02)) err = i2c_write_byte(addr, 0x94, (buf[1] & ~0x02));
	}
	}
	break;
	case 42:     //gpio 2
	if (!i2c_read_data(addr, 0x93, &buf[0], 1) && !i2c_read_data(addr, 0x94, &buf[1], 1)) {
	err = 0;
	if (buf[0] & 0x07) err = i2c_write_byte(addr, 0x93, (buf[0] & 0xf8));
	if (val) {
	if (!err && !(buf[1] & 0x04)) err = i2c_write_byte(addr, 0x94, (buf[1] | 0x04));
	} else {
	if (!err && (buf[1] & 0x04)) err = i2c_write_byte(addr, 0x94, (buf[1] & ~0x04));
	}
	}
	break;
	case 43:     //gpio 3
	if (!i2c_read_data(addr, 0x95, &buf[0], 3)) {
	err = 0;
	if ((buf[0] & 0x83) ^ 0x81) err = i2c_write_byte(addr, 0x95, ((buf[0] & 0xfc) | 0x81));
	if (val) {
	if (!err && !(buf[1] & 0x01)) err = i2c_write_byte(addr, 0x96, (buf[1] | 0x01));
	} else {
	if (!err && (buf[1] & 0x01)) err = i2c_write_byte(addr, 0x96, (buf[1] & ~0x01));
	}
	}
	break;
	case 44:     //gpio 4
	if (!i2c_read_data(addr, 0x95, &buf[0], 3)) {
	err = 0;
	if ((buf[0] & 0x8c) ^ 0x84) err = i2c_write_byte(addr, 0x95, ((buf[0] & 0xf3) | 0x84));
	if (val) {
	if (!err && !(buf[1] & 0x02)) err = i2c_write_byte(addr, 0x96, (buf[1] | 0x02));
	} else {
	if (!err && (buf[1] & 0x02)) err = i2c_write_byte(addr, 0x96, (buf[1] & ~0x02));
	}
	}
	break;
	case 45:     //gpio 5
	if (!i2c_read_data(addr, 0x9e, &buf[0], 1)) {
	err = 0;
	if (val) {
        if ((buf[0] & 0xe0) ^ 0xa0) err = i2c_write_byte(addr, 0x9e, ((buf[0] & 0x1f) | 0xa0));
	} else {
        if ((buf[0] & 0xe0) ^ 0x80) err = i2c_write_byte(addr, 0x9e, ((buf[0] & 0x1f) | 0x80));
	}
	}
	break;
	case 46:     //ldo2
	val1 = (val / 28 + 6) << 4;
	if (!i2c_read_data(addr, 0x12, &buf[0], 1) && !i2c_read_data(addr, 0x28, &buf[1], 1)) {
	err = 0;
	if (val) {
	if ((val1 & 0xf0) ^ (buf[1] & 0xf0)) {
	buf[1] = (buf[1] & 0x0f) | (val1 & 0xf0);
	err = i2c_write_byte(addr, 0x28, buf[1]);
	}
	if (!err && !(buf[0] & 0x04)) {
	buf[0] |= 0x04;
	err = i2c_write_byte(addr, 0x12, buf[0]);
	}
	} else {
	if (buf[0] & 0x04) {
	buf[0] &= ~0x04;
	err = i2c_write_byte(addr, 0x12, buf[0]);
	}
	}
	}
	break;
	case 47:     //ldo3
	val1 = (val / 28 + 6) << 4;
	if (!i2c_read_data(addr, 0x12, &buf[0], 1) && !i2c_read_data(addr, 0x28, &buf[1], 1)) {
	err = 0;
	if (val) {
	if ((val1 & 0xf0) ^ ((buf[1] << 4) & 0xf0)) {
	buf[1] = (buf[1] & 0xf0) | ((val1 >> 4) & 0x0f);
	err = i2c_write_byte(addr, 0x28, buf[1]);
	}
	if (!err && !(buf[0] & 0x08)) {
	buf[0] |= 0x08;
	err = i2c_write_byte(addr, 0x12, buf[0]);
	}
	} else {
	if (buf[0] & 0x08) {
	buf[0] &= ~0x08;
	err = i2c_write_byte(addr, 0x12, buf[0]);
	}
	}
	}
	break;
	default:
        err = -1;
	break;
	}
	return err;
}


//***************** bme280 ********************
//https://github.com/BoschSensortec/BME280_driver
//https://cdn.sparkfun.com/assets/e/7/3/b/1/BME280_Datasheet.pdf
//https://github.com/BoschSensortec/BME68x-Sensor-API
//https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme688-ds000.pdf
esp_err_t i2c_init_bme280(uint8_t idx,uint32_t* f_i2cdev)
{
	esp_err_t err = -1;
	uint32_t i2cbits = * f_i2cdev;
	if (!(i2cbits & 0x80000000) || (idx > 1)) return err;
	uint8_t id = 0;
	uint8_t idv = 0;
	uint8_t buf = 0;
	uint8_t addr;
	addr = i2c_addr[idx];
	err = i2c_check(addr);
	if (err) return err;
	err = i2c_read_data(addr, 0xd0, &id, 1);
	if (err) return err;
	if ((id == 0x60) || (id == 0x58)) { //[d0]=60/bme280, [d0]=58/bmp280 
// init 280
	err = i2c_write_byte(addr, 0xe0, 0xb6);  //write reset command
	if (err) return err; 
	vTaskDelay(10 / portTICK_PERIOD_MS);      //2ms bme280 ready time after reset
	err = i2c_read_data(addr, 0xf3, &buf, 1);
	if (err || (buf & 0x01)) return err;
	if (id == 0x60) err = i2c_write_byte(addr, 0xf2, 0x05); //h*16
	if (err) return err; 
	err = i2c_write_byte(addr, 0xf5, 0xa8); //config: 101 0.10 00 inactivity 1sec, IIR 4(a0-off)
	if (err) return err; 
	err = i2c_write_byte(addr, 0xf4, 0xb5);  //ctrl_meas: 101 1.01 11 t*16 p*16 forced mode
	if (err) return err; 
	i2cbits = i2cbits | (0x0001 << idx);
	*f_i2cdev = i2cbits;
	} else if (id == 0x61) {  //[d0]=61/bme680
	err = i2c_read_data(addr, 0xf0, &idv, 1);
	if (err) return err; 
// init data 68x
	uint16_t amb_temp = 25;
	uint16_t temp = 320;
	uint16_t dur = 100;
// init 68x
	int32_t var1;
	int32_t var2;
	int32_t var3;
	int32_t var4;
	int32_t var5;
	int16_t gh2;
	int8_t  gh1, gh3, htval;
	uint8_t htrng, htres;
	uint8_t clb[8] = {0};   
	err = i2c_write_byte(addr, 0xe0, 0xb6); //write reset command
	if (err) return err; 
	vTaskDelay(10 / portTICK_PERIOD_MS);      //2ms bme68x ready time after reset
	err = i2c_read_data(addr, 0x1d, &buf, 1);
	if (err || (buf & 0xe0)) return err; //eas_status_0: no meas
	err = i2c_write_byte(addr, 0x72, 0x05); //h*16
	if (err) return err; 
	err = i2c_write_byte(addr, 0x75, 0x08); //config: 000 0.10 00 IIR 4(0-off)
	if (err) return err; 
	err = i2c_write_byte(addr, 0x74, 0xb4); //ctrl_meas: 101 1.01 00 t*16 p*16 sleep mode
	if (err) return err; 
// BME68X_REG_COEFF2 e1/14  BME68X_REG_COEFF3 00/5 19 all
	err = i2c_read_data(addr, 0xeb, clb, 4);     //calibration 2
	if (err) return err; 
	err = i2c_read_data(addr, 0x00, &clb[4], 3); //calibration 3
	if (err) return err; 
	gh1 = clb[2];
	gh2 = (clb[1] << 8) + clb[0];
	gh3 = clb[3];
	htrng = (clb[6] & 0x30) >> 4;
	htval = clb[4];
// calculate the heater resistance value
	var1 = (((int32_t)amb_temp * gh3) / 1000) * 256;
	var2 = (gh1 + 784) * (((((gh2 + 154009) * temp * 5) / 100) + 3276800) / 10);
	var3 = var1 + (var2 / 2);
	var4 = (var3 / (htrng + 4));
	var5 = (131 * htval) + 65536;
	htres = (uint8_t)((((var4 / var5) - 250) * 34) / 100);
	err = i2c_write_byte(addr, 0x5a, htres);
	if (err) return err; 
// calculate the gas wait
	uint8_t factor = 0;
	if (dur >= 0xfc0) htres = 0xff; // Max duration
	else {
	while (dur > 0x3F) {
	dur = dur / 4;
	factor += 1;
	}
	htres = (uint8_t)(dur + (factor * 64));
	}
	err = i2c_write_byte(addr, 0x64, htres);
	if (err) return err; 
	if (!idv) buf = 0x10;
	else buf = 0x20;
	err = i2c_write_byte(addr, 0x71, buf); //10 Ctrl_gas_1: run_gas
	if (err) return err; 
	err = i2c_write_byte(addr, 0x70, 0x00); //00 Ctrl_gas_0: heat_off
	if (err) return err; 
	err = i2c_write_byte(addr, 0x74, 0xb5); //ctrl_meas: 101 1.01 01 t*16 p*16 forced mode
	if (err) return err; 
	i2cbits = i2cbits | (0x0001 << idx);
	*f_i2cdev = i2cbits;
	} //id
	return err;
}
esp_err_t i2c_read_bme280 (uint8_t idx, uint32_t* f_i2cdev, uint16_t* temp, uint16_t* humid, uint16_t* press, uint32_t* res)
{
	esp_err_t err = -1;
	uint32_t i2cbits = * f_i2cdev;
	if (!(i2cbits & 0x80000000) || (idx > 1)) return err;
	uint8_t id = 0;
	uint8_t idv = 0;
	uint8_t clb[42] = {0};   
	uint8_t data[24] = {0};   
	uint8_t addr;            
	addr = i2c_addr[idx];
	err = i2c_check(addr);
	if (err == ESP_FAIL) err = ESP_ERR_TIMEOUT; 
	if (!err) err = i2c_read_data(addr, 0xd0, &id, 1);
	if (!err && ((id == 0x60) || (id == 0x58))) { //[d0]=60/bme280, =58/bmp280 
//bme280
	int32_t adc_t, var1, var2, fine,  var3, var4, var5;
	uint32_t p;
	int16_t t2, t3, p2, p3, p4, p5, p6, p7, p8, p9, h2, h4, h5;
	uint16_t t1, p1;
	int8_t h6;
	uint8_t h1, h3;
//read data
	err = i2c_read_data(addr, 0xf3, &data[8], 3);
	if (!err && ((data[9] & 0xfc) == 0xb4)) {   //f3 & verify ctrl_meas(f4)
	err = i2c_read_data(addr, 0x88, clb, 0x19);
	if (!err) {       //calibration 1
	if (id == 0x60) {                                 //bme
	err = i2c_read_data(addr, 0xe1, &clb[0x1a], 7);  //humid calibration 
	if (!err) {   //calibration 2
	err = i2c_read_data(addr, 0xf7, data, 8);        //read data bme
	}
	} else {                                          //bmp
	err = i2c_read_data(addr, 0xf7, data, 6);        //read data bmp
	}
	if (!err) err = i2c_write_byte(addr, 0xf4, 0xb5); //start next measurement
	} //88
	} else if (!err) err = -1; //f3
//esp_log_buffer_hex(AP_TAG, clb, 40);
//esp_log_buffer_hex(AP_TAG, data, 16);
	if (!err) {
//temperature  res -> 0.1degC
	t1 = (clb[1] << 8) + clb[0];
	t2 = (clb[3] << 8) + clb[2];
	t3 = (clb[5] << 8) + clb[4];
	adc_t = (data[3] << 16) + (data[4] << 8) + data[5];
	if (adc_t != 0x800000) {
	adc_t >>= 4;
	var1 =  (((adc_t >> 3) - ((int32_t)t1 << 1)) * (int32_t)t2) >> 11;
	var2 = (((((adc_t >> 4) - (int32_t)t1) * ((adc_t >> 4) - (int32_t)t1)) >> 12) * (int32_t)t3) >> 14;
	fine = var1 + var2;
	var1 = (fine  + 256) >> 5;               //+ >> 4 in publish proc (res 0.1degC)
	*temp = var1;
//humidity  res -> 0.1%
	h1 = clb[0x18];
	h2 = (clb[0x1b] << 8) | clb[0x1a];
	h3 = clb[0x1c];
	h4 = (clb[0x1d] << 4) | (clb[0x1e] & 0x0f);
	h5 = (clb[0x1f] << 4) | (clb[0x1e] >> 4);
	h6 = clb[0x20];
	adc_t = (data[6] << 8) + data[7];
	if ((id == 0x60) && (adc_t != 0x8000)) {
	var1 = fine - ((int32_t)76800);
	var2 = (int32_t)(adc_t * 16384);
	var3 = (int32_t)(((int32_t)h4) * 1048576);
	var4 = ((int32_t)h5) * var1;
	var5 = (((var2 - var3) - var4) + (int32_t)16384) / 32768;
	var2 = (var1 * ((int32_t)h6)) / 1024;
	var3 = (var1 * ((int32_t)h3)) / 2048;
	var4 = ((var2 * (var3 + (int32_t)32768)) / 1024) + (int32_t)2097152;
	var2 = ((var4 * ((int32_t)h2)) + 8192) / 16384;
	var3 = var5 * var2;
	var4 = ((var3 / 32768) * (var3 / 32768)) / 128;
	var5 = var3 - ((var4 * ((int32_t)h1)) / 16);
	var5 = (var5 < 0 ? 0 : var5);
	var5 = (var5 > 419430400 ? 419430400 : var5);
	var1 = (var5 + 131072) >> 18;
	var1 = (var1 * 10) >> 4;
	*humid = var1;
//ESP_LOGI(AP_TAG, "h1 0x%X, h2 0x%X, h3 0x%X, h4 0x%X, h5 0x%X, h6 0x%X,  adc 0x%X, hum %d", h1,h2,h3,h4,h5,h6,adc_t,var1);
	} else {
	*humid = 0;
	}
//pressure 32bit  res -> 0.1 hPa
	p1 = (clb[0x07] << 8) + clb[0x06];
	p2 = (clb[0x09] << 8) + clb[0x08];
	p3 = (clb[0x0b] << 8) + clb[0x0a];
	p4 = (clb[0x0d] << 8) + clb[0x0c];
	p5 = (clb[0x0f] << 8) + clb[0x0e];
	p6 = (clb[0x11] << 8) + clb[0x10];
	p7 = (clb[0x13] << 8) + clb[0x12];
	p8 = (clb[0x15] << 8) + clb[0x14];
	p9 = (clb[0x17] << 8) + clb[0x16];
	adc_t = (data[0] << 16) + (data[1] << 8) + data[2];
	if (adc_t != 0x800000) {
	adc_t >>= 4;
	var1 = ((fine) >> 1) - 64000;
	var2 = (((var1 >> 2) * (var1 >> 2)) >> 11 ) * ((int32_t) p6);
	var2 = var2 + ((var1 * ((int32_t) p5)) << 1);
	var2 = (var2 >> 2) + (((int32_t) p4) << 16);
	var1 = ((((int32_t)p3 * (((var1 >> 2) * (var1 >> 2)) >> 13 )) >> 3) + ((((int32_t)p2) * var1) >> 1)) >> 18;
	var1 =((((32768 + var1)) * ((int32_t)p1)) >> 15);
	if (var1 != 0) {
	p = (((uint32_t)(((int32_t)1048576) - adc_t) - (var2 >> 12))) * 3125;
	if (p < 0x80000000) p = (p << 1) / ((uint32_t)var1);
	else p = (p / (uint32_t)var1) * 2;
	var1 = (((int32_t) p9) * ((int32_t)(((p >> 3) * (p >> 3)) >> 13))) >> 12;
	var2 = (((int32_t)(p >> 2)) * ((int32_t) p8)) >> 13;
	p = (uint32_t)((int32_t)p + ((var1 + var2 + (int32_t)p7) >> 4));
	p = (p / 5 + 1) >> 1;
        *press = p;
	} else *press = 0;
	*res = 0xffffffff;
	} else {
	*press = 0;
	*res = 0xffffffff;
	}
	} else {      //if no temp no other calc because variable fine not defined
	*temp = 0xffff;
	*humid = 0;
	*press = 0;
	*res = 0xffffffff;
	}
	} else {     //result
	*temp = 0xffff;
	*humid = 0;
	*press = 0;
	*res = 0xffffffff;
	}
	} else if (id == 0x61) {
//bme68x
// BME68X_REG_COEFF1 8a/23 BME68X_REG_COEFF2 e1/14  BME68X_REG_COEFF3 00/5 42 all
	int32_t adc_t, var1, var2, fine,  var3, var4, var5, finesc;
	uint32_t p, grs;
	int16_t t2, p2, p4, p5, p8, p9;
	uint16_t t1, h1, h2, p1;
	int8_t t3, p3, p6, p7, h3, h4, h5, h7, swer;
	uint8_t p10, h6, grng;
//read data
	err = i2c_read_data(addr, 0xf0, &idv, 1);
	if (!err) { //f0
	err  = i2c_read_data(addr, 0x74, &data[0], 1);
	if (!err && ((data[0] & 0xfc) == 0xb4)) {   //74 & verify ctrl_meas(f4)
	err = i2c_read_data(addr, 0x8a, clb, 23);
	if (!err) {       //calibration 1
	err = i2c_read_data(addr, 0xe1, &clb[23], 14);
	if (!err) {  //calibration 2
	err = i2c_read_data(addr, 0x00, &clb[37], 5);
	if (!err) {   //calibration 3
	err = i2c_read_data(addr, 0x1d, data, 17);       //read data
	if (!err) err = i2c_write_byte(addr, 0x74, 0xb5); //start next measurement
	} //00
	} //e1
	} //8a
	} else if (!err) err = -1;//74
	} //f0
//esp_log_buffer_hex(AP_TAG, clb, 42);
//esp_log_buffer_hex(AP_TAG, data, 16);
	if (!err) {
// 1d 1e 1f 20 21 22 23 24 25 26 27 28 29 2a 2b 2c 2d
//  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16
//        p        t        h             gl    gh
//temperature  res -> 0.1degC
	t1 = (clb[32] << 8) + clb[31];
	t2 = (clb[1] << 8) + clb[0];
	t3 =  clb[2];
	adc_t = (data[5] << 16) + (data[6] << 8) + data[7];
	if (adc_t != 0x800000) {
	adc_t >>= 4;
	var1 =  (((adc_t >> 3) - ((int32_t)t1 << 1)) * (int32_t)t2) >> 11;
	var2 = (((((adc_t >> 4) - (int32_t)t1) * ((adc_t >> 4) - (int32_t)t1)) >> 12) * (int32_t)t3) >> 14;
	fine = var1 + var2;
	var1 = (fine  + 256) >> 5;               //+ >> 4 in publish proc (res 0.1degC)
	*temp = var1;
//humidity  res -> 0.1%
	h1 = (clb[25] << 4) | (clb[24] & 0x0f);
	h2 = (clb[23] << 4) | (clb[24] >> 4);
	h3 = clb[26];
	h4 = clb[27];
	h5 = clb[28];
	h6 = clb[29];
	h7 = clb[30];
	adc_t = (data[8] << 8) + data[9];
	if ((id == 0x61) && (adc_t != 0x8000)) {
	finesc = (((int32_t)fine * 5) + 128) >> 8;
	var1 = (int32_t)(adc_t - ((int32_t)((int32_t)h1 * 16))) - (((finesc * (int32_t)h3) / ((int32_t)100)) >> 1);
	var2 = ((int32_t)h2 * (((finesc * (int32_t)h4) / ((int32_t)100)) +
          (((finesc * ((finesc * (int32_t)h5) / ((int32_t)100))) >> 6) / ((int32_t)100)) +
          (int32_t)(1 << 14))) >> 10;
	var3 = var1 * var2;
	var4 = (int32_t)h6 << 7;
	var4 = ((var4) + ((finesc * (int32_t)h7) / ((int32_t)100))) >> 4;
	var5 = ((var3 >> 14) * (var3 >> 14)) >> 10;
	var5 = (var4 * var5) >> 1;
	var5 = (((var3 + var5) >> 10) * ((int32_t)1000)) >> 12;
	if (var5 > 100000) var5 = 100000; // Cap at 100%rH /
	else if (var5 < 0) var5 = 0;
	var1 = (var5 / 25 + 2) >> 2;
	*humid = var1;
//ESP_LOGI(AP_TAG, "h1 0x%X, h2 0x%X, h3 0x%X, h4 0x%X, h5 0x%X, h6 0x%X,  adc 0x%X, hum %d", h1,h2,h3,h4,h5,h6,adc_t,var1);
	} else {
	*humid = 0;
	}
//pressure 32bit  res -> 0.1 hPa
	p1 = (clb[5] << 8) + clb[4];
	p2 = (clb[7] << 8) + clb[6];
	p3 = clb[8];
	p4 = (clb[11] << 8) + clb[10];
	p5 = (clb[13] << 8) + clb[12];
	p6 = clb[15];
	p7 = clb[14];
	p8 = (clb[19] << 8) + clb[18];
	p9 = (clb[21] << 8) + clb[20];
	p10 = clb[22];
	adc_t = (data[2] << 16) + (data[3] << 8) + data[4];
	if (adc_t != 0x800000) {
	adc_t >>= 4;
	var1 = (((int32_t)fine) >> 1) - 64000;
	var2 = ((((var1 >> 2) * (var1 >> 2)) >> 11) * (int32_t)p6) >> 2;
	var2 = var2 + ((var1 * (int32_t)p5) << 1);
	var2 = (var2 >> 2) + ((int32_t)p4 << 16);
	var1 = (((((var1 >> 2) * (var1 >> 2)) >> 13) * ((int32_t)p3 << 5)) >> 3) + (((int32_t)p2 * var1) >> 1);
	var1 = var1 >> 18;
	var1 = ((32768 + var1) * (int32_t)p1) >> 15;
	var4 = 1048576 - adc_t;
	var4 = (int32_t)((var4 - (var2 >> 12)) * ((uint32_t)3125));
	if (var4 >= (int32_t)0x40000000) var4 = ((var4 / var1) << 1);
	else var4 = ((var4 << 1) / var1);
	var1 = ((int32_t)p9 * (int32_t)(((var4 >> 3) * (var4 >> 3)) >> 13)) >> 12;
	var2 = ((int32_t)(var4 >> 2) * (int32_t)p8) >> 13;
	var3 = ((int32_t)(var4 >> 8) * (int32_t)(var4 >> 8) * (int32_t)(var4 >> 8) * (int32_t)p10) >> 17;
	var4 = (int32_t)(var4) + ((var1 + var2 + var3 + ((int32_t)p7 << 7)) >> 4);
	p = (var4 / 5 + 1) >> 1; 
        *press = p;
	} else {
	*press = 0;
	}
//gas resistance
	swer = (clb[41] & 0xf0) >> 4;
	if (!idv) {
//bme680
	int64_t dvr1;
	uint64_t dvr2;
	int64_t dvr3;
	uint32_t lookup_table1[16] = {
		UINT32_C(2147483647), UINT32_C(2147483647), UINT32_C(2147483647), UINT32_C(2147483647), UINT32_C(2147483647),
		UINT32_C(2126008810), UINT32_C(2147483647), UINT32_C(2130303777), UINT32_C(2147483647), UINT32_C(2147483647),
		UINT32_C(2143188679), UINT32_C(2136746228), UINT32_C(2147483647), UINT32_C(2126008810), UINT32_C(2147483647),
		UINT32_C(2147483647)
		};
	uint32_t lookup_table2[16] = {
		UINT32_C(4096000000), UINT32_C(2048000000), UINT32_C(1024000000), UINT32_C(512000000), UINT32_C(255744255),
		UINT32_C(127110228), UINT32_C(64000000), UINT32_C(32258064), UINT32_C(16016016), UINT32_C(8000000), UINT32_C(
		4000000), UINT32_C(2000000), UINT32_C(1000000), UINT32_C(500000), UINT32_C(250000), UINT32_C(125000)
		};
	adc_t = (data[13] << 2) + (data[14] >> 6);
	grng = data[14] & 0x0f;
	dvr1 = (int64_t)((1340 + (5 * (int64_t)swer)) * ((int64_t)lookup_table1[grng])) >> 16;
	dvr2 = (((int64_t)((int64_t)adc_t << 15) - (int64_t)(16777216)) + dvr1);
	dvr3 = (((int64_t)lookup_table2[grng] * (int64_t)dvr1) >> 9);
	grs = (uint32_t)((dvr3 + ((int64_t)dvr2 >> 1)) / (int64_t)dvr2);
	*res = grs;
	} else if (idv == 1) {
//bme688
	adc_t = (data[15] << 2) + (data[16] >> 6);
	grng = data[16] & 0x0f;
	grs = UINT32_C(262144) >> grng;
	var1 = (int32_t)adc_t - INT32_C(512);
	var1 *= INT32_C(3);
	var1 = INT32_C(4096) + var1;
	grs = (UINT32_C(10000) * grs) / (uint32_t)var1;
	grs = grs * 100;
	*res = grs;
	} else {      //unsupported
	*res = 0xffffffff;
	}
	} else {      //if no temp no other calc because variable fine not defined
	*temp = 0xffff;
	*humid = 0;
	*press = 0;
	*res = 0xffffffff;
	}
	} else {     //result
	*temp = 0xffff;
	*humid = 0;
	*press = 0;
	*res = 0xffffffff;
	}
	} else {     //id
	*temp = 0xffff;
	*humid = 0;
	*press = 0;
	*res = 0xffffffff;
	} //d0
	return err;
}

//***************** sht3x *********************
esp_err_t i2c_init_sht3x(uint8_t idx,uint32_t* f_i2cdev)
{
	esp_err_t err = -1;
	uint32_t i2cbits = * f_i2cdev;
	if (!(i2cbits & 0x80000000) || (idx > 1)) return err;
	uint8_t buf[4] = {0};
	uint8_t addr;
	addr = i2c_addr[idx + 2];
	err = i2c_check(addr);
	if (err) return err;
	err = i2c_write_byte(addr, 0x30, 0xa2);  //write reset command
	if (err) return err; 
	vTaskDelay(10 / portTICK_PERIOD_MS);      //1ms sht3x ready time after reset
	err = i2c_read2_data(addr, 0xf3, 0x2d, buf, 3); //read status
	if (err) return err;
	err = -1;
	if (buf[0] & 0x20) return err;           //check heater off after reset
	err = i2c_write_byte(addr, 0x30, 0x41);  //write clear status
	if (err) return err;
	vTaskDelay(10 / portTICK_PERIOD_MS);      //1ms sht3x ready time
	err = i2c_read2_data(addr, 0xf3, 0x2d, buf, 3); //read status
	if (err) return err;
	if ((buf[0] & 0x20) || (buf[1] & 0x10)) return -1;
	i2cbits = i2cbits | (0x0004 << idx);
	*f_i2cdev = i2cbits;
	return err;
}
esp_err_t i2c_read_sht3x(uint8_t idx,uint32_t* f_i2cdev, uint16_t* temp, uint16_t* humid)
{
	esp_err_t err = -1;
	uint32_t i2cbits = * f_i2cdev;
	uint32_t tmp, hum;
	if (!(i2cbits & 0x80000000) || (idx > 1)) return err;
	uint8_t buf[8] = {0};
	uint8_t addr;
	addr = i2c_addr[idx + 2];
	err = i2c_check(addr);
	if (err == ESP_FAIL) err = ESP_ERR_TIMEOUT; 
	if (!err) {
	err = i2c_write_byte(addr, 0x24, 0x00);  //Start Measurement Command for Single Shot Mode
	vTaskDelay(30 / portTICK_PERIOD_MS);      //15ms sht3x ready in Single Shot Mode
	if (!err) {
	err = i2c_read0_data(addr, buf, 6); //read data
	if (!err) {
	tmp = ((((buf[0] << 8) + buf[1]) * 1750 ) >> 12) - 7200; 
	*temp = tmp;
	hum = (((buf[3] << 8) + buf[4]) * 1000) >> 16;
	*humid = hum;
	}
	}
	}
	if (err) {
	*temp = 0xffff;
	*humid = 0;
	}
	return err;
}
//***************** aht2x *********************
esp_err_t i2c_init_aht2x(uint8_t idx,uint32_t* f_i2cdev)
{
	esp_err_t err = -1;
	uint32_t i2cbits = * f_i2cdev;
	if (!(i2cbits & 0x80000000) || idx) return err;
	uint8_t buf[4] = {0};
	uint8_t addr;
	addr = i2c_addr[idx + 4];
	err = i2c_check(addr);
	if (err) return err;
	err = i2c_write_data(addr, 0xba, buf, 0);  //write reset command
	if (err) return err; 
	vTaskDelay(25 / portTICK_PERIOD_MS);      //20ms sht2x ready time after reset
	err = i2c_read_data(addr, 0x71, buf, 1); //read status
	if (err) return err;
	buf[0] = 0x08;
	buf[1] = 0;
	if ((buf[0] & 0x18) != 0x18) err = i2c_write_data(addr, 0xbe, buf, 2);  //calibration command
	vTaskDelay(25 / portTICK_PERIOD_MS);      //10ms sht2x ready time after reset
	err = i2c_read_data(addr, 0x71, buf, 1); //read status
	if (err) return err;
	if ((buf[0] & 0x18) != 0x18) return -1;  //if not calibrated - error;
	i2cbits = i2cbits | (0x0010 << idx);
	*f_i2cdev = i2cbits;
	return err;
}
esp_err_t i2c_read_aht2x(uint8_t idx,uint32_t* f_i2cdev, uint16_t* temp, uint16_t* humid)
{
	esp_err_t err = -1;
	uint32_t i2cbits = * f_i2cdev;
	uint32_t tmp, hum;
	if (!(i2cbits & 0x80000000) || idx) return err;
	uint8_t buf[8] = {0};
	uint8_t addr;
	addr = i2c_addr[idx + 4];
	err = i2c_check(addr);
	if (err == ESP_FAIL) err = ESP_ERR_TIMEOUT; 
	if (!err) {
	buf[0] = 0x33;
	buf[1] = 0;
	err = i2c_write_data(addr, 0xac, buf, 2);  //Start Measurement Command
	vTaskDelay(100 / portTICK_PERIOD_MS);      //80ms aht2x ready
	if (!err) {
	err = i2c_read0_data(addr, buf, 7); //read data
	if (!err) {
	if (!(buf[0] & 0x80)) {
	tmp = (((((buf[3] & 0x0f) << 16) | (buf[4] << 8) | buf[5]) * 1000) >> 15) - 8000; 
	*temp = tmp;
	hum = (((buf[1] << 12) | (buf[2] << 4) | (buf[3] >> 4)) * 1000) >> 20;
	*humid = hum;
	} else err = -1;
	}
	}
	}
	if (err) {
	*temp = 0xffff;
	*humid = 0;
	}
	return err;
}
//***************** htu21 *********************
esp_err_t i2c_init_htu21(uint8_t idx,uint32_t* f_i2cdev)
{
	esp_err_t err = -1;
	uint32_t i2cbits = * f_i2cdev;
	if (!(i2cbits & 0x80000000) || idx) return err;
	uint8_t buf = 0xff;
	uint8_t addr;
	addr = i2c_addr[idx + 5];
	err = i2c_check(addr);
	if (err) return err;
	err = i2c_write_data(addr, 0xfe, &buf, 0);  //write reset command
	if (err) return err; 
	vTaskDelay(30 / portTICK_PERIOD_MS);      //15ms htu21 ready time after reset
	err = i2c_read_data(addr, 0xe7, &buf, 1); //read status
	if (err) return err;
	if (buf & 0x38) return -1;
	if (buf & 0x04) {
	buf &= 0xfb;
	err = i2c_write_byte(addr, 0xe6, buf);  //disable heater
	if (err) return err;
	err = i2c_read_data(addr, 0xe7, &buf, 1); //read status
	if (err) return err;
	if (buf & 0x3c) return -1;
	}
	i2cbits = i2cbits | (0x0020 << idx);
	*f_i2cdev = i2cbits;
	return err;
}
esp_err_t i2c_read_htu21(uint8_t idx,uint32_t* f_i2cdev, uint16_t* temp, uint16_t* humid)
{
	esp_err_t err = -1;
	uint32_t i2cbits = * f_i2cdev;
	uint32_t tmp, hum;
	if (!(i2cbits & 0x80000000) || idx) return err;
	uint8_t buf[4] = {0};
	uint8_t addr;
	addr = i2c_addr[idx + 5];
	err = i2c_check(addr);
	if (err == ESP_FAIL) err = ESP_ERR_TIMEOUT; 
	if (!err) {  //1
	err = i2c_write_data(addr, 0xf3, buf, 0);  //Start Measurement Command for temperature
	if (!err) {  //2
	vTaskDelay(100 / portTICK_PERIOD_MS);      //50ms ready temperature
	err = i2c_read0_data(addr, buf, 3); //read data
	if (!err) { //3
	tmp = ((((buf[0] << 8) + buf[1]) * 1757 ) >> 12) - 7488; 
	*temp = tmp;
	err = i2c_write_data(addr, 0xf5, buf, 0);  //Start Measurement Command for humidity
	if (!err) { //4
	vTaskDelay(50 / portTICK_PERIOD_MS);       //16ms ready humidity
	err = i2c_read0_data(addr, buf, 3);        //read data
	if (!err) { //5
	hum = ((((buf[0] << 8) + buf[1]) * 1250 ) >> 16) - 60; 
	*humid = hum;
	} //5
	} //4
	} //3
	} //2
	} //1

	if (err) {
	*temp = 0xffff;
	*humid = 0;
	}
	return err;
}

//********* rtc: ds3231(0)/pcf8563(1) *********
esp_err_t i2c_init_rtc(uint8_t idx,uint32_t* f_i2cdev)
{
	esp_err_t err = -1;
	uint32_t i2cbits = * f_i2cdev;
	if (!(i2cbits & 0x80000000) || (idx > 1)) return err;
	uint8_t data[32];
	uint8_t addr;
	addr = i2c_addr[idx + 6];
	err = i2c_check(addr);
	if (err) return err;
	memset(data, 0xff, sizeof(data));
	if (!idx) {                                //ds3231
	err = i2c_read_data(addr, 0x00, data, 20);
	if (err) return err;
	if ((data[0x00] & 0x80) || (data[0x01] & 0x80) || (data[0x02] & 0x80) || (data[0x03] & 0xf8)
	 || (data[0x04] & 0xc0) || (data[0x05] & 0x60) || (data[0x0f] & 0x70) || (data[0x12] & 0x3f)) return -1;
	if (!(data[0x0e] & 0x20) && !(data[0x0f] & 0x04)) err = i2c_write_byte(addr, 0x0e, 0x3c);
	else err = i2c_write_byte(addr, 0x0e, 0x1c);
	} else {                                   //pcf8563
	err = i2c_read_data(addr, 0x00, data, 16);
	if (err) return err;
	if ((data[0x00] & 0x57) || (data[0x01] & 0xe0) || (data[0x03] & 0x80) || (data[0x04] & 0xc0)
	 || (data[0x05] & 0xc0) || (data[0x06] & 0xf8) || (data[0x07] & 0x60)) return -1;
	if (data[0x00]) err = i2c_write_byte(addr, 0x00, 0x00);
	if (err) return err;
	if (data[0x01]) err = i2c_write_byte(addr, 0x01, 0x00);
	if (err) return err;
	}
	if (!err) {
	i2cbits = i2cbits | (0x0040 << idx);
	*f_i2cdev = i2cbits;
	}
	return err;
}
uint8_t b2bcd(uint8_t x)
{
	uint8_t y;
	y = ((x / 10) << 4) | (x % 10);
	return y;
}
uint8_t bcd2b(uint8_t x)
{
	uint8_t y;
	y = (x & 0x0f) + ((x & 0xf0) >> 1) + ((x & 0xf0) >> 3);
	return y;
}
esp_err_t i2c_read_rtc(uint8_t idx,uint32_t* f_i2cdev, uint16_t* temp)
{
	esp_err_t err = -1;
	uint32_t i2cbits = * f_i2cdev;
	if (!(i2cbits & 0x80000000) || (idx > 1)) return err;
	struct tm timeinfo;
	time_t now;
	int16_t itmp;
	uint8_t data[32];
	uint8_t addr;
	addr = i2c_addr[idx + 6];
	memset(data, 0xff, sizeof(data));
	err = i2c_check(addr);
	if (err == ESP_FAIL) err = ESP_ERR_TIMEOUT; 
	if (!err) {  //1
	if (!idx) {                                //ds3231
	err = i2c_read_data(addr, 0x00, data, 20);
	itmp =  (data[0x11]<< 8) | data[0x12];
	itmp = (itmp >> 1) + (itmp >> 3) + 8;
	if (!err) {  //2
	time(&now);
	localtime_r(&now, &timeinfo);
	if (timeinfo.tm_year > 100) {  //3
	data[20] = b2bcd(timeinfo.tm_sec);
	data[21] = b2bcd(timeinfo.tm_min);
	data[22] = b2bcd(timeinfo.tm_hour);
	data[23] = timeinfo.tm_wday + 1;
	data[24] = b2bcd(timeinfo.tm_mday);
	data[25] = b2bcd(timeinfo.tm_mon + 1);
	data[26] = b2bcd(timeinfo.tm_year % 100);
        if (sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED) {
	if ((data[0x0f] & 0x80) || ((data[0] ^ data[20]) & 0xf0) || memcmp(&data[21], &data[1], 6)) {
	err = i2c_write_data(addr, 0x00, &data[20], 7);  //write data
	if (!err) err = i2c_write_byte(addr, 0x0f, (data[0x0f] & 0x7f)); //clear OSF
	if (!err && fdebug) ESP_LOGI(AP_TAG, "Write to RTC%d Ok", idx);
	}
	}
	} else if (!(data[0x0f] & 0x80)) {
	timeinfo.tm_sec = bcd2b(data[0]);
	timeinfo.tm_min = bcd2b(data[1]);
	timeinfo.tm_hour = bcd2b(data[2]);
	timeinfo.tm_mday = bcd2b(data[4]);
	timeinfo.tm_mon = bcd2b(data[5] & 0x1f) - 1;
	timeinfo.tm_year = bcd2b(data[6]) + 100;
	now = mktime(&timeinfo);
	struct timeval stnow = { .tv_sec = now};
	settimeofday(&stnow, NULL);
	if (fdebug) ESP_LOGI(AP_TAG, "Read from RTC%d Ok", idx);
	} else {
	if (fdebug) ESP_LOGI(AP_TAG, "RTC%d Oscillator Stop detected", idx);
	} //3
	} //2
	if (!err && !(data[0x0e] & 0x20) && !(data[0x0f] & 0x04))  {
	data[0x0e] |= 0x20;
	err = i2c_write_byte(addr, 0x0e, data[0x0e]);
	}
	} else {                                   //pcf8563
	err = i2c_read_data(addr, 0x00, data, 16);
	itmp = 0;
	if (!err) {  //2
	time(&now);
	localtime_r(&now, &timeinfo);
	if (timeinfo.tm_year > 100) {  //3
	data[20] = b2bcd(timeinfo.tm_sec);
	data[21] = b2bcd(timeinfo.tm_min);
	data[22] = b2bcd(timeinfo.tm_hour);
	data[23] = b2bcd(timeinfo.tm_mday);
	data[24] = timeinfo.tm_wday;
	data[25] = b2bcd(timeinfo.tm_mon + 1);
	data[26] = b2bcd(timeinfo.tm_year % 100);
        if (sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED) {
	if ((data[0x02] & 0x80) || ((data[0x02] ^ data[20]) & 0xf0) || memcmp(&data[21], &data[0x03], 6)) {
	err = i2c_write_data(addr, 0x02, &data[20], 7);  //write data
	if (!err && fdebug) ESP_LOGI(AP_TAG, "Write to RTC%d Ok", idx);
	}
	}
	} else if (!(data[0x02] & 0x80)) {
	timeinfo.tm_sec = bcd2b(data[0x02]);
	timeinfo.tm_min = bcd2b(data[0x03]);
	timeinfo.tm_hour = bcd2b(data[0x04]);
	timeinfo.tm_mday = bcd2b(data[0x05]);
	timeinfo.tm_mon = bcd2b(data[0x07] & 0x1f) - 1;
	timeinfo.tm_year = bcd2b(data[0x08]) + 100;
	now = mktime(&timeinfo);
	struct timeval stnow = { .tv_sec = now};
	settimeofday(&stnow, NULL);
	if (fdebug) ESP_LOGI(AP_TAG, "Read from RTC%d Ok", idx);
	} else {
	if (fdebug) ESP_LOGI(AP_TAG, "RTC%d Oscillator Stop detected", idx);
	} //3
	} //2
	}
	} //1
	if (!idx) {
	if (err) {
	*temp = 0xffff;
	} else {
	*temp = itmp;
	}
	}
	return err;
}

//*********************************************


static struct BleDevSt BleDevStA;
static struct BleDevSt BleDevStB;
static struct BleDevSt BleDevStC;
static struct BleDevSt BleDevStD;
static struct BleDevSt BleDevStE;
static int wf_retry_cnt;

static struct BleMonRec BleMR[BleMonNum];
static struct BleMonExt BleMX[BleMonNum];
static struct SnPari2c SnPi2c[28];

#ifdef USE_TFT
#include "tft/tft.c"
#endif

//******************* timer *********************
static intr_handle_t s_timer_handle;
static void hw_timer_callback(void *arg)
{
//Reset irq and set for next time
#ifdef CONFIG_IDF_TARGET_ESP32C3
	TIMERG0.int_clr_timers.t0_int_clr = 1;
	TIMERG0.hw_timer[0].config.tx_alarm_en = 1;
#else
	TIMERG0.int_clr_timers.t0 = 1;
	TIMERG0.hw_timer[0].config.alarm_en = 1;
#endif
//
	if (!hwtdiv) {
	if (BleDevStA.t_rspdel) BleDevStA.t_rspdel--;
	if (BleDevStB.t_rspdel) BleDevStB.t_rspdel--;
	if (BleDevStC.t_rspdel) BleDevStC.t_rspdel--;
	if (BleDevStD.t_rspdel) BleDevStD.t_rspdel--;
	if (BleDevStE.t_rspdel) BleDevStE.t_rspdel--;
	} else if (hwtdiv == 1) {
	if (BleDevStA.r4sppcom) BleDevStA.r4sppcom--;
	if (BleDevStB.r4sppcom) BleDevStB.r4sppcom--;
	if (BleDevStC.r4sppcom) BleDevStC.r4sppcom--;
	if (BleDevStD.r4sppcom) BleDevStD.r4sppcom--;
	if (BleDevStE.r4sppcom) BleDevStE.r4sppcom--;
	} else if (hwtdiv == 2) {
	if (BleDevStA.t_ppcon) BleDevStA.t_ppcon--;
	if (BleDevStB.t_ppcon) BleDevStB.t_ppcon--;
	if (BleDevStC.t_ppcon) BleDevStC.t_ppcon--;
	if (BleDevStD.t_ppcon) BleDevStD.t_ppcon--;
	if (BleDevStE.t_ppcon) BleDevStE.t_ppcon--;
	} else if (hwtdiv == 3) {
	if (t_clock) t_clock--;
	if (t_lasts) t_lasts--;
	if (t_ppcons) t_ppcons--;
#ifdef USE_TFT
	if (t_jpg) t_jpg--;
#endif
	if (t_tinc) t_tinc--;
	if (r4sppcoms) r4sppcoms--;
	}

	if (hwtdiv > 3) {
	for (int i = 0; i < BleMonNum; i++) {
	if (BleMX[i].ttick) {
	BleMX[i].ttick--;
	if (!BleMX[i].ttick) {
	t_lasts = 0;
	if (BleMR[i].sto) {
	BleMX[i].advdatlen = 0;
	BleMX[i].scrsplen = 0;
	BleMX[i].state = 0;
	if (BleMR[i].id != 2) {
	BleMX[i].rssi = 0;
	BleMX[i].par1 = 0;
	BleMX[i].par2 = 0;
	BleMX[i].par3 = 0;
	BleMX[i].par4 = 0;
	BleMX[i].par5 = 0;
	BleMX[i].par7 = 0;
	}
	BleMX[i].par6 = 0;
	} else {
	memset(&BleMR[i],0,sizeof(BleMR[i]));
	memset(&BleMX[i],0,sizeof(BleMX[i]));
	}
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
	t_lasts = 0;
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
	t_lasts = 0;
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
	} else if ((BleDevStA.DEV_TYP > 15) && (BleDevStA.DEV_TYP < 62)) { 
	if (!BleDevStA.bState) {
	BleDevStA.r4slppar1 = 1;
	BleDevStA.r4slpcom = 10;
	} else {
	BleDevStA.r4slppar1 = 0;
	BleDevStA.r4slpcom = 10;
	}
	} else if ((BleDevStA.DEV_TYP == 62)) { 
	if (!BleDevStA.bState) {
	BleDevStA.bState = 1;
	BleDevStA.r4slppar1 = 0;
	BleDevStA.r4slpcom = 62;
	}
	} else if ((BleDevStA.DEV_TYP == 63)) { 
	if (!BleDevStA.bState) {
	BleDevStA.bState = 1;
	BleDevStA.r4slppar1 = 0;
	BleDevStA.r4slpcom = 63;
	}
	} else if ((BleDevStA.DEV_TYP > 63) && (BleDevStA.DEV_TYP < 73)) { 
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
	t_lasts = 0;
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
	t_lasts = 0;
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
	} else if ((BleDevStB.DEV_TYP > 15) && (BleDevStB.DEV_TYP < 62)) { 
	if (!BleDevStB.bState) {
	BleDevStB.r4slppar1 = 1;
	BleDevStB.r4slpcom = 10;
	} else {
	BleDevStB.r4slppar1 = 0;
	BleDevStB.r4slpcom = 10;
	}
	} else if ((BleDevStB.DEV_TYP == 62)) { 
	if (!BleDevStB.bState) {
	BleDevStB.bState = 1;
	BleDevStB.r4slppar1 = 0;
	BleDevStB.r4slpcom = 62;
	}
	} else if ((BleDevStB.DEV_TYP == 63)) { 
	if (!BleDevStB.bState) {
	BleDevStB.bState = 1;
	BleDevStB.r4slppar1 = 0;
	BleDevStB.r4slpcom = 63;
	}
	} else if ((BleDevStB.DEV_TYP > 63) && (BleDevStB.DEV_TYP < 73)) { 
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
	t_lasts = 0;
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
	t_lasts = 0;
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
	} else if ((BleDevStC.DEV_TYP > 15) && (BleDevStC.DEV_TYP < 62)) { 
	if (!BleDevStC.bState) {
	BleDevStC.r4slppar1 = 1;
	BleDevStC.r4slpcom = 10;
	} else {
	BleDevStC.r4slppar1 = 0;
	BleDevStC.r4slpcom = 10;
	}
	} else if ((BleDevStC.DEV_TYP == 62)) { 
	if (!BleDevStC.bState) {
	BleDevStC.bState = 1;
	BleDevStC.r4slppar1 = 0;
	BleDevStC.r4slpcom = 62;
	}
	} else if ((BleDevStC.DEV_TYP == 63)) { 
	if (!BleDevStC.bState) {
	BleDevStC.bState = 1;
	BleDevStC.r4slppar1 = 0;
	BleDevStC.r4slpcom = 63;
	}
	} else if ((BleDevStC.DEV_TYP > 63) && (BleDevStC.DEV_TYP < 73)) { 
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
	t_lasts = 0;
	cntgpio4 = 0;
	}
	} else cntgpio4 = 0;
	break;
	case 192:
	if ((gpio_get_level(bgpio4 & 0x3f)) != lvgpio4) {
        cntgpio4++;
	if (cntgpio4 > 5) {
	fgpio4 = 1;
	lvgpio4 = lvgpio4 ^ 1;
	t_lasts = 0;
	cntgpio4 = 0;
	t_jpg = 0;	
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
	t_lasts = 0;
	cntgpio5 = 0;
	}
	} else cntgpio5 = 0;
	break;

	}
}

//******************** ble hass discovery data **********************
void MqttPubSub (uint8_t blenum, bool mqtttst) {
	uint8_t blenum1 = blenum + 1;
	char tbuff[16]; 
	if (blenum > 4) return;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	if (!mqtdel && ptr->tBLEAddr[0] && ptr->btauthoriz && mqtttst && ptr->DEV_TYP) {
	char *bufd = NULL;
	bufd = malloc(2048);
	if (bufd == NULL) {
	if (fdebug) ESP_LOGE(AP_TAG, "MqttPubSub: No memory");
	MemErr++;
	if (!MemErr) MemErr--;
	} else {	
	memset (bufd,0,2048);
	char buft[64];
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

	if ((ptr->DEV_TYP < 10) || ((ptr->DEV_TYP > 63) && (ptr->DEV_TYP < 73))) {
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
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	strcat(bufd,".Kettle.temp\",\"icon\":\"mdi:kettle\",\"uniq_id\":\"temp_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Kettle_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Kettle\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	strcat(bufd,"/heat\",\"temp_step\":\"5\",\"modes\":[\"off\",\"auto\",\"heat\"");
	if ((ptr->DEV_TYP > 3) && (ptr->DEV_TYP < 10)) strcat(bufd,",\"cool\"]}");
	else strcat(bufd,"]}");
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
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	strcat(bufd,"Redmond");
	strcat(bufd,"\"},\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/volume\",\"unit_of_meas\":\"");
	if (volperc) strcat(bufd,"\x25");
	else strcat(bufd,"l");
	strcat(bufd,"\",\"state_class\":\"measurement\",\"availability_topic\":\"");
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
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"");
	strcat(bufd,"Redmond");
	strcat(bufd,"\"},\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/volume_last\",\"unit_of_meas\":\"");
	if (volperc) strcat(bufd,"\x25");
	else strcat(bufd,"l");
	strcat(bufd,"\",\"state_class\":\"measurement\",\"availability_topic\":\"");
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
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/keep");
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
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	strcat(bufd,".Power.lock\",\"icon\":\"mdi:lock\",\"uniq_id\":\"lock_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Power_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Power\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	strcat(bufd,".Power.keep\",\"icon\":\"mdi:memory\",\"uniq_id\":\"keep_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Power_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Power\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/keep\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/keep\",\"availability_topic\":\"");
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
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	strcat(buft,"/keep");
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
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	strcat(bufd,".Heater.lock\",\"icon\":\"mdi:lock\",\"uniq_id\":\"lock_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Heater_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Heater\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	strcat(bufd,".Coffee.lock\",\"icon\":\"mdi:lock\",\"uniq_id\":\"lock_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Coffee_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Coffee\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	} else if ( ptr->DEV_TYP < 61) {
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
	if (ptr->DEV_TYP != 48) {
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
	}
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
	strcat(bufd,".Cooker.switch\",\"icon\":\"mdi:clock-start\",\"uniq_id\":\"switch_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Cooker\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	strcat(bufd,".Cooker.auto.warming\",\"icon\":\"mdi:pot-steam-outline\",\"uniq_id\":\"autowarming_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Cooker\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	if (ptr->DEV_TYP != 48) {
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
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	strcat(bufd,"/temp\",\"mode\":\"box\",\"min\":\"0\",\"max\":\"230\",\"unit_of_meas\":\"\xc2\xb0\x43\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
	}
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
	strcat(bufd,".Cooker.s.hour\",\"icon\":\"mdi:clock-outline\",\"uniq_id\":\"shour_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Cooker\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	strcat(bufd,"/set_hour\",\"mode\":\"box\",\"min\":\"0\",\"max\":\"23\",\"unit_of_meas\":\"h\",\"availability_topic\":\"");
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
	strcat(bufd,".Cooker.s.min\",\"icon\":\"mdi:clock-outline\",\"uniq_id\":\"smin_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Cooker\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	strcat(bufd,"/set_min\",\"mode\":\"box\",\"min\":\"0\",\"max\":\"59\",\"unit_of_meas\":\"m\",\"availability_topic\":\"");
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
	strcat(bufd,".Cooker.d.hour\",\"icon\":\"mdi:clock-plus-outline\",\"uniq_id\":\"dhour_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Cooker\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	strcat(bufd,"/delay_hour\",\"mode\":\"box\",\"min\":\"0\",\"max\":\"23\",\"unit_of_meas\":\"h\",\"availability_topic\":\"");
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
	strcat(bufd,".Cooker.d.min\",\"icon\":\"mdi:clock-plus-outline\",\"uniq_id\":\"dmin_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Cooker\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	strcat(bufd,"/delay_min\",\"mode\":\"box\",\"min\":\"0\",\"max\":\"59\",\"unit_of_meas\":\"m\",\"availability_topic\":\"");
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
	strcat(bufd,".Cooker.state\",\"icon\":\"mdi:pot-steam-outline\",\"uniq_id\":\"stat_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Cooker\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	strcat(bufd,".Cooker.hour\",\"icon\":\"mdi:clock-outline\",\"uniq_id\":\"hour_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Cooker\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/hour\",\"unit_of_meas\":\"h\",\"availability_topic\":\"");
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
	strcat(bufd,".Cooker.min\",\"icon\":\"mdi:clock-outline\",\"uniq_id\":\"min_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Cooker_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Cooker\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/min\",\"unit_of_meas\":\"m\",\"availability_topic\":\"");
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
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	} else if ( ptr->DEV_TYP == 61) {
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
	strcat(bufd,".Iron.switch\",\"icon\":\"mdi:iron\",\"uniq_id\":\"switch_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Iron_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Iron\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	strcat(bufd,".Iron.lock\",\"icon\":\"mdi:lock\",\"uniq_id\":\"lock_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Iron_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Iron\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Iron_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Power\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	} else if ( ptr->DEV_TYP == 62) {
/*
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/clear");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
*/
	if (FDHass) {
/*
	strcpy(buft,"homeassistant/switch/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Smoke.clear\",\"icon\":\"mdi:mdi-check-circle-outline\",\"uniq_id\":\"clear_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Smoke_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Smoke\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/clear\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/clear\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
*/
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
	strcat(bufd,".Smoke.rssi\",\"icon\":\"mdi:bluetooth\",\"uniq_id\":\"rssi_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Smoke_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Smoke\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	strcat(bufd,".Smoke.temperature\",\"icon\":\"mdi:thermometer\",\"uniq_id\":\"temp_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Smoke_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Smoke\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	strcat(bufd,".Smoke.battery\",\"icon\":\"mdi:battery-bluetooth\",\"uniq_id\":\"battery_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Smoke_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Smoke\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"device_class\":\"battery\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/battery\",\"unit_of_meas\":\"\x25\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	strcpy(buft,"homeassistant/binary_sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Smoke.alarm\",\"icon\":\"mdi:smoke-detector\",\"uniq_id\":\"alarm_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Smoke_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Smoke\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Redmond\"},\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/smoke\",\"availability_topic\":\"");
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
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
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

	} else if ( ptr->DEV_TYP == 73) {
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
	strcat(buft,"/set_hour");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/set_min");
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
	strcat(bufd,".Galcon.state\",\"icon\":\"mdi:water-pump\",\"uniq_id\":\"state_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Galcon_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Galcon\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Galcon\"},\"command_topic\":\"");
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
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Galcon.rssi\",\"icon\":\"mdi:bluetooth\",\"uniq_id\":\"rssi_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Galcon_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Galcon\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Galcon\"},\"device_class\":\"signal_strength\",\"state_class\":\"measurement\",\"state_topic\":\"");
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
/*
	strcpy(buft,"homeassistant/sensor/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/2x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Galcon.battery\",\"icon\":\"mdi:battery-bluetooth\",\"uniq_id\":\"battery_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Galcon_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Galcon\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Galcon\"},\"device_class\":\"battery\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/battery\",\"unit_of_meas\":\"\x25\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
*/
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
	strcat(bufd,".Galcon.hour\",\"icon\":\"mdi:timer\",\"uniq_id\":\"hour_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Galcon_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Galcon\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Galcon\"},\"state_topic\":\"");
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
	strcat(buft,"/4x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Galcon.min\",\"icon\":\"mdi:timer\",\"uniq_id\":\"min_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Galcon_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Galcon\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Galcon\"},\"state_topic\":\"");
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
	strcat(buft,"/5x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Galcon.sec\",\"icon\":\"mdi:timer\",\"uniq_id\":\"sec_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Galcon_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Galcon\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Galcon\"},\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/sec\",\"availability_topic\":\"");
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
	strcat(bufd,".Galcon.shour\",\"icon\":\"mdi:timer\",\"uniq_id\":\"shour_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Galcon_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Galcon\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Galcon\"},\"command_topic\":\"");
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
	strcat(buft,"/2x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Galcon.smin\",\"icon\":\"mdi:timer\",\"uniq_id\":\"smin_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"Galcon_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".Galcon\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"Galcon\"},\"command_topic\":\"");
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
	}

	} else if ( ptr->DEV_TYP == 74) {
	strcpy(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/");
	strcat(buft,ptr->tBLEAddr);
	if (!fcommtp) strcat(buft,"/cmd");
	strcat(buft,"/set");
	esp_mqtt_client_subscribe(mqttclient, buft, 0);
	if (FDHass) {
	strcpy(buft,"homeassistant/cover/");
	strcat(buft,MQTT_BASE_TOPIC);
	strcat(buft,"/1x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".AM43.ctrl\",\"icon\":\"mdi:blinds-horizontal\",\"uniq_id\":\"ctrl_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"AM43_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".AM43\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"A-OK\"},\"command_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/set\",\"set_position_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/cmd");
	strcat(bufd,"/set\",\"position_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/position\",\"availability_topic\":\"");
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
	strcat(bufd,".AM43.position\",\"icon\":\"mdi:blinds-horizontal\",\"uniq_id\":\"position_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"AM43_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".AM43\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"A-OK\"},\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/position\",\"unit_of_meas\":\"\x25\",\"availability_topic\":\"");
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
	strcat(bufd,".AM43.illuminance\",\"icon\":\"mdi:sun-wireless\",\"uniq_id\":\"illuminance_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"AM43_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".AM43\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"A-OK\"},\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/illuminance\",\"unit_of_meas\":\"\x25\",\"availability_topic\":\"");
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
	strcat(bufd,".AM43.rssi\",\"icon\":\"mdi:bluetooth\",\"uniq_id\":\"rssi_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"AM43_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".AM43\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"A-OK\"},\"device_class\":\"signal_strength\",\"state_class\":\"measurement\",\"state_topic\":\"");
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
	strcat(buft,"/4x");
	strcat(buft,ptr->tBLEAddr);
	strcat(buft,"/config");
	strcpy(bufd,"{\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".AM43.battery\",\"icon\":\"mdi:battery-bluetooth\",\"uniq_id\":\"battery_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\",\"device\":{\"identifiers\":[\"AM43_");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"\"],\"name\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	itoa(blenum1,tbuff,10);
	strcat(bufd,tbuff);
	strcat(bufd,".AM43\",\"model\":\"");
	strcat(bufd,ptr->DEV_NAME);
	if (ptr->sVer[0] > 0x20) {
	strcat(bufd,"\",\"sw_version\":\"");
	strcat(bufd, ptr->sVer);
	}
	strcat(bufd,"\",\"via_device\":\"ESP32_");
	strcat(bufd,tESP32Addr);
	strcat(bufd,"\",\"manufacturer\":\"A-OK\"},\"device_class\":\"battery\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	if (!fcommtp) strcat(bufd,"/rsp");
	strcat(bufd,"/battery\",\"unit_of_meas\":\"\x25\",\"availability_topic\":\"");
	strcat(bufd,MQTT_BASE_TOPIC);
	strcat(bufd,"/");
	strcat(bufd,ptr->tBLEAddr);
	strcat(bufd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, buft, bufd, 0, 1, 1);
//
	}


	}
	free(bufd);
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

//galcon
static esp_bt_uuid_t glremote_filter_service_uuid = {
    .len = ESP_UUID_LEN_128,
    .uuid = {.uuid128 = GLREMOTE_SERVICE_UUID,},
};
static esp_bt_uuid_t glremote_filter_service1_uuid = {
    .len = ESP_UUID_LEN_128,
    .uuid = {.uuid128 = GLREMOTE_SERVICE1_UUID,},
};
static esp_bt_uuid_t glremote_filter_service2_uuid = {
    .len = ESP_UUID_LEN_128,
    .uuid = {.uuid128 = GLREMOTE_SERVICE2_UUID,},
};
static esp_bt_uuid_t glremote_filter_txchar_uuid = {
    .len = ESP_UUID_LEN_128,
    .uuid = {.uuid128 = GLREMOTE_TXCHAR_UUID,},
};
static esp_bt_uuid_t glremote_filter_rxchar_uuid = {
    .len = ESP_UUID_LEN_128,
    .uuid = {.uuid128 = GLREMOTE_RXCHAR_UUID,},
};
static esp_bt_uuid_t glremote_filter_auth_uuid = {
    .len = ESP_UUID_LEN_128,
    .uuid = {.uuid128 = GLREMOTE_AUTH_UUID,},
};
static esp_bt_uuid_t glremote_filter_time_uuid = {
    .len = ESP_UUID_LEN_128,
    .uuid = {.uuid128 = GLREMOTE_TIME_UUID,},
};
static esp_bt_uuid_t glremote_filter_setup_uuid = {
    .len = ESP_UUID_LEN_128,
    .uuid = {.uuid128 = GLREMOTE_SETUP_UUID,},
};

//am43
static esp_bt_uuid_t amremote_filter_service_uuid = {
    .len = ESP_UUID_LEN_16,
    .uuid = {.uuid16 = AMREMOTE_SERVICE_UUID,},
};
static esp_bt_uuid_t amremote_filter_char_uuid = {
    .len = ESP_UUID_LEN_16,
    .uuid = {.uuid16 = AMREMOTE_CHAR_UUID,},
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

//connection profile D handle
    [PROFILE_D_APP_ID] = {
        .gattc_if = ESP_GATT_IF_NONE,       // Not get the gatt_if, so initial is ESP_GATT_IF_NONE
    },

//connection profile E handle
    [PROFILE_E_APP_ID] = {
        .gattc_if = ESP_GATT_IF_NONE,       // Not get the gatt_if, so initial is ESP_GATT_IF_NONE
    },

};

static void start_scan(void)
{
	esp_err_t scan_ret = 0;

//step 1: set scan only if not update, if not already starting scan or if no connections is opening   
	if (!f_update && !StartStopScanReq && (!BleDevStA.btopenreq || BleDevStA.btopen) && (!BleDevStB.btopenreq || BleDevStB.btopen) && (!BleDevStC.btopenreq || BleDevStC.btopen) && (!BleDevStD.btopenreq || BleDevStD.btopen) && (!BleDevStE.btopenreq || BleDevStE.btopen)) {
	if (!ble_mon || (ble_mon == 3)) {
	if ((BleDevStA.REQ_NAME[0] && !BleDevStA.btopenreq) || (BleDevStB.REQ_NAME[0] && !BleDevStB.btopenreq) || (BleDevStC.REQ_NAME[0] && !BleDevStC.btopenreq) || (BleDevStD.REQ_NAME[0] && !BleDevStD.btopenreq) || (BleDevStE.REQ_NAME[0] && !BleDevStE.btopenreq)) {
	if (!Isscanning) {
	scan_ret = esp_ble_gap_set_scan_params(&ble_scan_params);
	IsPassiveScan = false;
	} else if (IsPassiveScan) {
	esp_ble_gap_stop_scanning();
	StartStopScanReq = true;
	}
	} else if (ble_mon) {
	if (!Isscanning) {
	scan_ret = esp_ble_gap_set_scan_params(&ble_pscan_params);
	IsPassiveScan = true;
	} else if (!IsPassiveScan) {
	esp_ble_gap_stop_scanning();
	StartStopScanReq = true;
	}
	}
	} else if (ble_mon == 2) {
	if (!Isscanning) {
	scan_ret = esp_ble_gap_set_scan_params(&ble_scan_params);
	IsPassiveScan = false;
	} else if (IsPassiveScan) {
	esp_ble_gap_stop_scanning();
	StartStopScanReq = true;
	}
	} else if (ble_mon == 1) {
	if (!Isscanning) {
	scan_ret = esp_ble_gap_set_scan_params(&ble_pscan_params);
	IsPassiveScan = true;
	} else if (!IsPassiveScan) {
	esp_ble_gap_stop_scanning();
	StartStopScanReq = true;
	}
	}
	if (scan_ret){
	if (fdebug) ESP_LOGE(AP_TAG, "Set scan params error, error code = 0x%X", scan_ret);
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
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
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
	if (ptr->btopenreq && !ptr->btopen) {
	if (ptr->r4sAuthCount > 2) {
	if (fdebug) ESP_LOGI(AP_TAG, "CONNECT_EVT %d, set Bond and Encryption", blenum1);
	esp_ble_auth_req_t auth_req = ESP_LE_AUTH_REQ_SC_MITM_BOND;     //bonding with peer device after authentication
	esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;           //set the IO capability to No output No input
	if (ptr->DEV_TYP == 73)	iocap = ESP_IO_CAP_IO;  // ESP_IO_CAP_OUT; ESP_IO_CAP_IO; set the IO capability
	esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t));
	esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
	esp_ble_set_encryption(param->connect.remote_bda, ESP_BLE_SEC_ENCRYPT_MITM);
	} else if ((ptr->DEV_TYP > 61) && (ptr->DEV_TYP < 64)) {
	if (fdebug) ESP_LOGI(AP_TAG, "CONNECT_EVT %d, set no Bond and Encryption", blenum1);
	esp_ble_auth_req_t auth_req = ESP_LE_AUTH_REQ_SC_ONLY;     //default authentication
	esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;           //set the IO capability to No output No input
	esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t));
	esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
	esp_ble_set_encryption(param->connect.remote_bda, 1);
	} else if (ptr->DEV_TYP == 73) {
	if (fdebug) ESP_LOGI(AP_TAG, "CONNECT_EVT %d, set no Bond and Encryption", blenum1);
	esp_ble_auth_req_t auth_req = ESP_LE_AUTH_REQ_SC_ONLY;     //default authentication
	esp_ble_io_cap_t iocap = ESP_IO_CAP_IO; // ESP_IO_CAP_OUT; ESP_IO_CAP_IO; set the IO capability
	esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t));
	esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
	esp_ble_set_encryption(param->connect.remote_bda, 1);
	} else if (ptr->DEV_TYP == 74) {
	if (fdebug) ESP_LOGI(AP_TAG, "CONNECT_EVT %d, set no Bond and Encryption", blenum1);
	esp_ble_auth_req_t auth_req = ESP_LE_AUTH_REQ_SC_ONLY;     //default authentication
	esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;           //set the IO capability to No output No input
	esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t));
	esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
	esp_ble_set_encryption(param->connect.remote_bda, 1);
	} else {
	if (fdebug) ESP_LOGI(AP_TAG, "CONNECT_EVT %d, set no Bond and no Encryption", blenum1);
	}

	}
	break;
    case ESP_GATTC_OPEN_EVT:
	ptr->xbtauth = 0;
        char bd_addr[20];
	if (param->open.status != ESP_GATT_OK) {
	if (fdebug) ESP_LOGE(AP_TAG, "Open %d failed, status %d", blenum1, p_data->open.status);
	ptr->btopen = false;
        ptr->btopenreq = false;
	start_scan();
	break;
	}
	memcpy(gl_profile_tab[blenum].remote_bda, p_data->open.remote_bda, 6);
        gl_profile_tab[blenum].conn_id = p_data->open.conn_id;
	if (fdebug) {
	ESP_LOGI(AP_TAG, "Open %d success, conn_id %d, if %d, status %d, mtu %d", blenum1, p_data->open.conn_id, gattc_if, p_data->open.status, p_data->open.mtu);
	bin2hex(p_data->open.remote_bda, bd_addr,6,0x3a);
        ESP_LOGI(AP_TAG, "Remote BD_ADDR: %s", bd_addr);
	}
        esp_err_t mtu_ret = esp_ble_gattc_send_mtu_req (gattc_if, p_data->open.conn_id);
	ptr->btopen = true;
	if (mtu_ret){
	if (fdebug) ESP_LOGE(AP_TAG, "Config MTU %d error, error code = 0x%X", blenum1, mtu_ret);
	}
	if (ptr->DEV_TYP == 64) ptr->MiKettleID = 275;
	if (ptr->DEV_TYP == 65) ptr->MiKettleID = 131;
	if (ptr->DEV_TYP == 66) ptr->MiKettleID = 1116;
        break;

    case ESP_GATTC_DIS_SRVC_CMPL_EVT:
	if (param->dis_srvc_cmpl.status != ESP_GATT_OK){
	conerr = 1;
	if (fdebug) ESP_LOGE(AP_TAG, "Discover service %d failed, status %d", blenum1, param->dis_srvc_cmpl.status);
	break;
	}
	if (fdebug) ESP_LOGI(AP_TAG, "Discover service %d complete conn_id %d", blenum1, param->dis_srvc_cmpl.conn_id);
	if (ptr->DEV_TYP < 64) esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id, &remote_filter_service_uuid);
	else if (ptr->DEV_TYP < 73) {
	esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id, &xremote_filter_service_uuid);
	esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id, &xremote_filter_service1_uuid);
	esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id, &xremote_filter_service2_uuid);
	esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id, &xremote_filter_service3_uuid);
	} else if (ptr->DEV_TYP == 73) {
	esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id, &glremote_filter_service_uuid);
	esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id, &glremote_filter_service1_uuid);
	esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id, &glremote_filter_service2_uuid);
	} else if (ptr->DEV_TYP == 74) {
	esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id, &amremote_filter_service_uuid);
	}
        break;
    case ESP_GATTC_CFG_MTU_EVT:
	if (param->cfg_mtu.status != ESP_GATT_OK){
	conerr = 1;
	if (fdebug) ESP_LOGE(AP_TAG,"Config mtu %d failed, error status = 0x%X", blenum1, param->cfg_mtu.status);
	}
	if (fdebug) ESP_LOGI(AP_TAG, "ESP_GATTC_CFG_MTU_EVT, Status %d, MTU %d, conn_id %d", param->cfg_mtu.status, param->cfg_mtu.mtu, param->cfg_mtu.conn_id);
	break;
    case ESP_GATTC_SEARCH_RES_EVT: {
	if (fdebug) {
	ESP_LOGI(AP_TAG, "SEARCH RES %d: conn_id = 0x%X is primary service %d", blenum1, p_data->search_res.conn_id, p_data->search_res.is_primary);
        ESP_LOGI(AP_TAG, "start handle 0x%X end handle 0x%X current handle value 0x%X", p_data->search_res.start_handle, p_data->search_res.end_handle, p_data->search_res.srvc_id.inst_id);
	}
/*
	if (fdebug) {
	esp_log_buffer_hex(AP_TAG,p_data->search_res.srvc_id.uuid.uuid.uuid128,16);
	esp_log_buffer_hex(AP_TAG,amremote_filter_service_uuid.uuid.uuid128,16);
	}
*/
	if ((ptr->DEV_TYP < 64) && (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_128) && (bt_compare_UUID128(p_data->search_res.srvc_id.uuid.uuid.uuid128, remote_filter_service_uuid.uuid.uuid128))) {

	if (fdebug) ESP_LOGI(AP_TAG, "Redmond Service %d found", blenum1);
            ptr->get_server = true;
            gl_profile_tab[blenum].service_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[blenum].service_end_handle = p_data->search_res.end_handle;
	} else if ((ptr->DEV_TYP > 63) && (ptr->DEV_TYP < 73) && (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_128) && (bt_compare_UUID128(p_data->search_res.srvc_id.uuid.uuid.uuid128, xremote_filter_service_uuid.uuid.uuid128))) {

	if (fdebug) ESP_LOGI(AP_TAG, "Xiaomi Service1 %d found", blenum1);
            gl_profile_tab[blenum].service_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[blenum].service_end_handle = p_data->search_res.end_handle;
	} else if ((ptr->DEV_TYP > 63) && (ptr->DEV_TYP < 73) && (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_16) && (p_data->search_res.srvc_id.uuid.uuid.uuid16 == XREMOTE_SERVICE116_UUID)) {

	if (fdebug) ESP_LOGI(AP_TAG, "Xiaomi Service2 %d found", blenum1);
            gl_profile_tab[blenum].service1_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[blenum].service1_end_handle = p_data->search_res.end_handle;
	} else if ((ptr->DEV_TYP > 63) && (ptr->DEV_TYP < 73) && (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_16) && (p_data->search_res.srvc_id.uuid.uuid.uuid16 == XREMOTE_SERVICE216_UUID)) {

	if (fdebug) ESP_LOGI(AP_TAG, "Xiaomi Service3 %d found", blenum1);
            gl_profile_tab[blenum].service2_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[blenum].service2_end_handle = p_data->search_res.end_handle;
	} else if ((ptr->DEV_TYP > 63) && (ptr->DEV_TYP < 73) && (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_16) && (p_data->search_res.srvc_id.uuid.uuid.uuid16 == XREMOTE_SERVICE316_UUID)) {

	if (fdebug) ESP_LOGI(AP_TAG, "Xiaomi Service4 %d found", blenum1);
            ptr->get_server = true;
            gl_profile_tab[blenum].service3_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[blenum].service3_end_handle = p_data->search_res.end_handle;
	} else if ((ptr->DEV_TYP == 73) && (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_128) && (bt_compare_UUID128(p_data->search_res.srvc_id.uuid.uuid.uuid128, glremote_filter_service_uuid.uuid.uuid128))) {

	if (fdebug) ESP_LOGI(AP_TAG, "Galcon Service1 %d found", blenum1);
            gl_profile_tab[blenum].service_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[blenum].service_end_handle = p_data->search_res.end_handle;
	} else if ((ptr->DEV_TYP == 73) && (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_128) && (bt_compare_UUID128(p_data->search_res.srvc_id.uuid.uuid.uuid128, glremote_filter_service1_uuid.uuid.uuid128))) {

	if (fdebug) ESP_LOGI(AP_TAG, "Galcon Service2 %d found", blenum1);
            gl_profile_tab[blenum].service1_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[blenum].service1_end_handle = p_data->search_res.end_handle;
	} else if ((ptr->DEV_TYP == 73) && (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_128) && (bt_compare_UUID128(p_data->search_res.srvc_id.uuid.uuid.uuid128, glremote_filter_service2_uuid.uuid.uuid128))) {

	if (fdebug) ESP_LOGI(AP_TAG, "Galcon Service3 %d found", blenum1);
            ptr->get_server = true;
            gl_profile_tab[blenum].service2_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[blenum].service2_end_handle = p_data->search_res.end_handle;
	} else if ((ptr->DEV_TYP == 74) && (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_16) && (p_data->search_res.srvc_id.uuid.uuid.uuid16 == amremote_filter_service_uuid.uuid.uuid16)) {
	if (fdebug) ESP_LOGI(AP_TAG, "AM43 Service %d found", blenum1);
            ptr->get_server = true;
            gl_profile_tab[blenum].service_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[blenum].service_end_handle = p_data->search_res.end_handle;
	}
        break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT:
	if (p_data->search_cmpl.status != ESP_GATT_OK){
	conerr = 1;
	if (fdebug) ESP_LOGE(AP_TAG, "Search service %d failed, error status = 0x%X", blenum1, p_data->search_cmpl.status);
            break;
	}
	if(p_data->search_cmpl.searched_service_source == ESP_GATT_SERVICE_FROM_REMOTE_DEVICE) {
//	if (fdebug) ESP_LOGI(AP_TAG, "Get service information from remote device");
	} else if (p_data->search_cmpl.searched_service_source == ESP_GATT_SERVICE_FROM_NVS_FLASH) {
//	if (fdebug) ESP_LOGI(AP_TAG, "Get service information from flash");
	} else {
//	if (fdebug) ESP_LOGI(AP_TAG, "Unknown service source");
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
		if (fdebug) ESP_LOGE(AP_TAG, "esp_ble_gattc_get_attr_count error");
    	}

	if ((ptr->DEV_TYP > 63) && (ptr->DEV_TYP < 74)) {
            status = esp_ble_gattc_get_attr_count( gattc_if,
                                                                     p_data->search_cmpl.conn_id,
                                                                     ESP_GATT_DB_CHARACTERISTIC,
                                                                     gl_profile_tab[blenum].service1_start_handle,
                                                                     gl_profile_tab[blenum].service1_end_handle,
                                                                     INVALID_HANDLE,
                                                                     &count1);
    	if (status != ESP_GATT_OK){
		conerr = 1;
	if (fdebug) ESP_LOGE(AP_TAG, "esp_ble_gattc_get_attr1_count error");
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
	if (fdebug) ESP_LOGE(AP_TAG, "esp_ble_gattc_get_attr2_count error");
    	}
	count = count + count1;
	count1 = 0;		
	}
	if ((ptr->DEV_TYP > 63) && (ptr->DEV_TYP < 73)) {
            status = esp_ble_gattc_get_attr_count( gattc_if,
                                                                     p_data->search_cmpl.conn_id,
                                                                     ESP_GATT_DB_CHARACTERISTIC,
                                                                     gl_profile_tab[blenum].service3_start_handle,
                                                                     gl_profile_tab[blenum].service3_end_handle,
                                                                     INVALID_HANDLE,
                                                                     &count1);
    	if (status != ESP_GATT_OK){
		conerr = 1;
	if (fdebug) ESP_LOGE(AP_TAG, "esp_ble_gattc_get_attr3_count error");
    	}
	count = count + count1;
	}

    	if (count > 0) {
		char_elem_result = (esp_gattc_char_elem_t *)malloc(sizeof(esp_gattc_char_elem_t) * count);
        	if (!char_elem_result){
		conerr = 1;
	if (fdebug) ESP_LOGE(AP_TAG, "Gattc no mem");
        	} else {
		if ((ptr->DEV_TYP < 64) && (count > 1)) {    //if dev_type
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
		if (fdebug) ESP_LOGE(AP_TAG, "Get_rxchar_by_uuid %d error", blenum1);
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
		if (fdebug) ESP_LOGE(AP_TAG, "Get_txchar_by_uuid %d error", blenum1);
            	}
                    /*  Every service have only one char in our 'ESP_GATTS_DEMO' demo, so we used first 'char_elem_result' */
		int i = 0; 
                while (count1 && (i < count1)) {
		if (char_elem_result[i].properties & ESP_GATT_CHAR_PROP_BIT_NOTIFY) {	
                        gl_profile_tab[blenum].rxchar_handle = char_elem_result[i].char_handle;
                        esp_ble_gattc_register_for_notify (gattc_if, gl_profile_tab[blenum].remote_bda, char_elem_result[i].char_handle);
			if (fdebug) ESP_LOGI(AP_TAG, "Register_for_notify %d", blenum1);
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



	if (fdebug) {
	ESP_LOGI(AP_TAG, "Rx char count = %d, handle = 0x%X", count1, gl_profile_tab[blenum].rxchar_handle);
	ESP_LOGI(AP_TAG, "Tx char count = %d, handle = 0x%X", count2, gl_profile_tab[blenum].txchar_handle);
		}
		} else if ((ptr->DEV_TYP > 63) && (ptr->DEV_TYP < 73) && (count > 8)) {
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[blenum].service_start_handle,
                                                             gl_profile_tab[blenum].service_end_handle,
                                                             xremote_filter_status_uuid,
                                                             (char_elem_result),
                                                             &count);
            	if (status != ESP_GATT_OK){
			conerr = 1;
            		if (fdebug) ESP_LOGE(AP_TAG, "Get_status_by_uuid %d error", blenum1);
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
			if (fdebug) ESP_LOGE(AP_TAG, "Get_authinit_by_uuid %d error", blenum1);
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
			if (fdebug) ESP_LOGE(AP_TAG, "Get_auth_by_uuid %d error", blenum1);
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
			if (fdebug) ESP_LOGE(AP_TAG, "Get_ver_by_uuid %d error", blenum1);
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
			if (fdebug) ESP_LOGE(AP_TAG, "Get_setup_by_uuid %d error", blenum1);
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
			if (fdebug) ESP_LOGE(AP_TAG, "Get_time_by_uuid %d error", blenum1);
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
			if (fdebug) ESP_LOGE(AP_TAG, "Get_boil_by_uuid %d error", blenum1);
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
			if (fdebug) ESP_LOGE(AP_TAG, "Get_mcuver_by_uuid %d error", blenum1);
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
			if (fdebug) ESP_LOGE(AP_TAG, "Get_update_by_uuid %d error", blenum1);
            	}

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
	if (fdebug) {
		ESP_LOGI(AP_TAG, "Update handle = 0x%X", gl_profile_tab[blenum].update_handle);
		ESP_LOGI(AP_TAG, "Register_for_notify %d", blenum1);
		}
            	}

		} else if ((ptr->DEV_TYP == 73) && (count > 4)) {
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[blenum].service_start_handle,
                                                             gl_profile_tab[blenum].service_end_handle,
                                                             glremote_filter_rxchar_uuid,
                                                             (char_elem_result),
                                                             &count);
            	if (status != ESP_GATT_OK) {
			conerr = 1;
			if (fdebug) ESP_LOGE(AP_TAG, "Get_rxchar_by_uuid %d error", blenum1);
            	}
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
                                                             p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[blenum].service_start_handle,
                                                             gl_profile_tab[blenum].service_end_handle,
                                                             glremote_filter_txchar_uuid,
                                                             (char_elem_result+1),
                                                             &count);

            	if (status != ESP_GATT_OK) {
			conerr = 1;
			if (fdebug) ESP_LOGE(AP_TAG, "Get_txchar_by_uuid %d error", blenum1);
            	}
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
                                                             p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[blenum].service1_start_handle,
                                                             gl_profile_tab[blenum].service1_end_handle,
                                                             glremote_filter_auth_uuid,
                                                             (char_elem_result+2),
                                                             &count);

            	if (status != ESP_GATT_OK) {
			conerr = 1;
			if (fdebug) ESP_LOGE(AP_TAG, "Get_auth_by_uuid %d error", blenum1);
            	}
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
                                                             p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[blenum].service1_start_handle,
                                                             gl_profile_tab[blenum].service1_end_handle,
                                                             glremote_filter_time_uuid,
                                                             (char_elem_result+3),
                                                             &count);

            	if (status != ESP_GATT_OK) {
			conerr = 1;
			if (fdebug) ESP_LOGE(AP_TAG, "Get_time_by_uuid %d error", blenum1);
            	}
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
                                                             p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[blenum].service2_start_handle,
                                                             gl_profile_tab[blenum].service2_end_handle,
                                                             glremote_filter_setup_uuid,
                                                             (char_elem_result+4),
                                                             &count);

            	if (status != ESP_GATT_OK) {
			conerr = 1;
			if (fdebug) ESP_LOGE(AP_TAG, "Get_setup_by_uuid %d error", blenum1);
            	}

            	if ((count > 0) && (char_elem_result[0].properties & ESP_GATT_CHAR_PROP_BIT_NOTIFY)){
                        gl_profile_tab[blenum].rxchar_handle = char_elem_result[0].char_handle;
                        esp_ble_gattc_register_for_notify (gattc_if, gl_profile_tab[blenum].remote_bda, char_elem_result[0].char_handle);
                        gl_profile_tab[blenum].txchar_handle = char_elem_result[1].char_handle;
                        gl_profile_tab[blenum].auth_handle = char_elem_result[2].char_handle;
                        gl_profile_tab[blenum].time_handle = char_elem_result[3].char_handle;
                        gl_profile_tab[blenum].setup_handle = char_elem_result[4].char_handle;
		if (fdebug) {
		ESP_LOGI(AP_TAG, "Rx char handle = 0x%X", gl_profile_tab[blenum].rxchar_handle);
		ESP_LOGI(AP_TAG, "Tx char handle = 0x%X",  gl_profile_tab[blenum].txchar_handle);
		ESP_LOGI(AP_TAG, "Auth handle = 0x%X", gl_profile_tab[blenum].auth_handle);
		ESP_LOGI(AP_TAG, "Time handle = 0x%X", gl_profile_tab[blenum].time_handle);
		ESP_LOGI(AP_TAG, "Setup handle = 0x%X", gl_profile_tab[blenum].setup_handle);
		ESP_LOGI(AP_TAG, "Register_for_notify %d", blenum1);
		}
		}

		} else if ((ptr->DEV_TYP == 74) && (count > 0)) {
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[blenum].service_start_handle,
                                                             gl_profile_tab[blenum].service_end_handle,
                                                             amremote_filter_char_uuid,
                                                             (char_elem_result),
                                                             &count);
            	if (status != ESP_GATT_OK) {
			conerr = 1;
			if (fdebug) ESP_LOGE(AP_TAG, "Get_rxchar_by_uuid %d error", blenum1);
            	}
            	if ((count > 0) && (char_elem_result[0].properties & ESP_GATT_CHAR_PROP_BIT_NOTIFY)){
                        gl_profile_tab[blenum].rxchar_handle = char_elem_result[0].char_handle;
                        esp_ble_gattc_register_for_notify (gattc_if, gl_profile_tab[blenum].remote_bda, char_elem_result[0].char_handle);
		if (fdebug) {
		ESP_LOGI(AP_TAG, "Rx/Tx char handle = 0x%X", gl_profile_tab[blenum].rxchar_handle);
		ESP_LOGI(AP_TAG, "Register_for_notify %d", blenum1);
		}
		}

		} else conerr = 1;      //if dev_type

        	}
                // free char_elem_result
                free(char_elem_result);
    	} else {
		conerr = 1;
		if (fdebug) ESP_LOGE(AP_TAG, "No char %d found", blenum1);
    	}
	}
         break;
    case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
	if (p_data->reg_for_notify.status != ESP_GATT_OK){
	conerr = 1;
	if (fdebug) ESP_LOGE(AP_TAG, "REG FOR NOTIFY %d failed: error status = %d", blenum1, p_data->reg_for_notify.status);
	} else {
            uint16_t count = 0;
            uint16_t notify_en = 1; 
	if (!ptr->xbtauth) {
            esp_gatt_status_t ret_status = esp_ble_gattc_get_attr_count( gattc_if,
                                                                         gl_profile_tab[blenum].conn_id,
                                                                         ESP_GATT_DB_DESCRIPTOR,
                                                                         gl_profile_tab[blenum].service_start_handle,
                                                                         gl_profile_tab[blenum].service_end_handle,
                                                                         gl_profile_tab[blenum].rxchar_handle,
                                                                         &count);
    	if (ret_status != ESP_GATT_OK){
		conerr = 1;
		if (fdebug) ESP_LOGE(AP_TAG, "Get_attr_count %d error", blenum1);
    	}
    	if (count > 0){
                descr_elem_result = malloc(sizeof(esp_gattc_descr_elem_t) * count);
        	if (!descr_elem_result){
        		if (fdebug) ESP_LOGE(AP_TAG, "malloc error, gattc no mem");
        	} else {
                    ret_status = esp_ble_gattc_get_descr_by_char_handle( gattc_if,
                                                                         gl_profile_tab[blenum].conn_id,
                                                                         p_data->reg_for_notify.handle,
                                                                         notify_descr_uuid,
                                                                         descr_elem_result,
                                                                         &count);
            	if (ret_status != ESP_GATT_OK){
			conerr = 1;
			if (fdebug) ESP_LOGE(AP_TAG, "Get_descr_by_char_handle %d error", blenum1);
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
			if (fdebug) ESP_LOGE(AP_TAG, "Write_char_descr %d error", blenum1);
            	}
                    /* free descr_elem_result */
                    free(descr_elem_result);
        	}
    	} else {
		conerr = 1;
		if (fdebug) ESP_LOGE(AP_TAG, "Decsr %d not found", blenum1);
    	}
	} else if ((ptr->DEV_TYP > 63) && (ptr->DEV_TYP < 73) && ptr->xbtauth) {
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
		if (fdebug) ESP_LOGE(AP_TAG, "Get_attr_count1 %d error", blenum1);
    	}
    	if (count > 0){
                descr_elem_result = malloc(sizeof(esp_gattc_descr_elem_t) * count);
        	if (!descr_elem_result){
		conerr = 1;
		if (fdebug) ESP_LOGE(AP_TAG, "malloc error, gattc no mem");
        	} else {
                    ret_status = esp_ble_gattc_get_descr_by_char_handle( gattc_if,
                                                                         gl_profile_tab[blenum].conn_id,
                                                                         p_data->reg_for_notify.handle,
                                                                         notify_descr_uuid,
                                                                         descr_elem_result,
                                                                         &count);
            	if (ret_status != ESP_GATT_OK){
			conerr = 1;
			if (fdebug) ESP_LOGE(AP_TAG, "Get_descr_by_char_handle %d error", blenum1);
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
			if (fdebug) ESP_LOGE(AP_TAG, "Write_char_descr1 %d error", blenum1);
            	}
                    /* free descr_elem_result */
                    free(descr_elem_result);
        	}
    	} else {
		conerr = 1;
    		if (fdebug) ESP_LOGE(AP_TAG, "Decsr1 %d not found", blenum1);
    	}
	}	
	}
        break;
    }

    case ESP_GATTC_READ_CHAR_EVT:
{
/*
	if (fdebug) {
	ESP_LOGI(AP_TAG, "ESP_GATTC_READ_CHAR_EVT, receive read value:");
	esp_log_buffer_hex(AP_TAG, p_data->read.value, p_data->read.value_len);
	ESP_LOGI(AP_TAG, "Read char handle = 0x%X", p_data->read.handle);
	}
*/
	int length = p_data->read.value_len;
	if (length > BLE_INPUT_BUFFSIZE)
	length = BLE_INPUT_BUFFSIZE;
	if (length > 0) {
	memcpy(ptr->readData, p_data->read.value, length);
	}
	ptr->readDataLen = length;
	ptr->readDataHandle = p_data->read.handle;
	if ((ptr->DEV_TYP > 63) && (ptr->DEV_TYP < 73) && (p_data->read.handle == gl_profile_tab[blenum].mcuver_handle)) {
	if (length && (length < sizeof(ptr->sVer))) memcpy(ptr->sVer, p_data->read.value, length);
	MqttPubSub(blenum, mqttConnected);	
	}
}
	break;
    case ESP_GATTC_NOTIFY_EVT:
	if ((p_data->notify.is_notify) && (p_data->notify.handle == gl_profile_tab[blenum].rxchar_handle)) {
	if (ptr->btauthoriz) {
	int length = p_data->notify.value_len;
	if (length > BLE_INPUT_BUFFSIZE)
	length = BLE_INPUT_BUFFSIZE;
	if (length > 0) {
/*
	if (fdebug) {
	ESP_LOGI(AP_TAG, "Notify %d:", blenum1);
	esp_log_buffer_hex(AP_TAG, p_data->notify.value, p_data->notify.value_len);
	}
*/
	if (ptr->DEV_TYP == 74) {
	if (ptr->LstCmd && (p_data->notify.value[1] == ptr->LstCmd)) {
	memcpy(ptr->readData, p_data->notify.value, length);
	ptr->readDataLen = length;
	ptr->t_rspdel = 0;
	} else if (p_data->notify.value[1] == 0xa1) {
	memcpy(ptr->notifyData, p_data->notify.value, length);
	ptr->notifyDataLen = length;
	ptr->t_rspdel = 0;
	} else if (ptr->t_rspdel > 40) ptr->t_rspdel = 40;



	} else {
	memcpy(ptr->notifyData, p_data->notify.value, length);
	ptr->notifyDataLen = length;
	}
	if (ptr->DEV_TYP == 73) ptr->t_rspdel = 0;
	}
	} else if (ptr->DEV_TYP < 64) {
	if (fdebug) {
	ESP_LOGI(AP_TAG, "Read_auth %d:", blenum1);
	esp_log_buffer_hex(AP_TAG, p_data->notify.value, p_data->notify.value_len);
	}
	if ((p_data->notify.value_len == 5) && (p_data->notify.value[0] = 0x55) && (p_data->notify.value[2] = 0xff) && (p_data->notify.value[4] = 0xaa)) {
	if (p_data->notify.value[3]) {
	if (fdebug) ESP_LOGI(AP_TAG, "Authorize %d Redmond ok", blenum1);
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	ptr->t_ppcon = 40;
	memset(ptr->sVer, 0, sizeof(ptr->sVer));
	ptr->btauthoriz = true;
	ptr->r4sConnErr = 0;
	ptr->r4sAuthCount = 0;
	ptr->r4scounter = p_data->notify.value[1] + 1;	
	ptr->f_Sync = 16;
	ptr->NumConn++;
	if (!ptr->NumConn) ptr->NumConn--;
	bin2hex(gl_profile_tab[blenum].remote_bda, ptr->tBLEAddr,6,0);
	strcpy(ptr->DEV_NAME,ptr->RQC_NAME);
//	MqttPubSub(blenum, mqttConnected);	
	} else {
	if (fdebug) ESP_LOGI(AP_TAG, "Authorize %d Redmond error", blenum1);
	ptr->r4sAuthCount++;
	if (ptr->r4sAuthCount > 6) ptr->r4sAuthCount = 0;
	conerr = 1;
	}
	}

	} else if (ptr->DEV_TYP == 74) {
	if (fdebug && (!memcmp(&p_data->notify.value[0],"\x9a\x17",2))) {
	ESP_LOGI(AP_TAG, "Read_auth %d:", blenum1);
	esp_log_buffer_hex(AP_TAG, p_data->notify.value, p_data->notify.value_len);
	}
	if ((p_data->notify.value_len == 5) && (!memcmp(&p_data->notify.value[0],"\x9a\x17\x01\x5a\x31",5))) {
	if (fdebug) ESP_LOGI(AP_TAG, "Authorize %d AM43 ok", blenum1);
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	ptr->t_ppcon = 40;
	memset(ptr->sVer, 0, sizeof(ptr->sVer));
	ptr->btauthoriz = true;
	ptr->r4sConnErr = 0;
	ptr->r4sAuthCount = 0;
	ptr->f_Sync = 16;
	ptr->NumConn++;
	if (!ptr->NumConn) ptr->NumConn--;
	bin2hex(gl_profile_tab[blenum].remote_bda, ptr->tBLEAddr,6,0);
	strcpy(ptr->DEV_NAME,ptr->RQC_NAME);
	MqttPubSub(blenum, mqttConnected);	
	} else if ((p_data->notify.value_len == 5) && (!memcmp(&p_data->notify.value[0],"\x9a\x17\x01\xa5\xce",5))) {
	if (fdebug) ESP_LOGI(AP_TAG, "Authorize %d AM43 error, Passkey %d invalid", blenum1, ptr->PassKey);
	conerr = 1;
	}
/*
	} else {
	if (fdebug) {
	ESP_LOGI(AP_TAG, "Read_auth %d:", blenum1);
	esp_log_buffer_hex(AP_TAG, p_data->notify.value, p_data->notify.value_len);
	}
*/
	}
	} else if (!ptr->btauthoriz && (p_data->notify.is_notify) && (p_data->notify.handle == gl_profile_tab[blenum].auth_handle)) {
	if (fdebug) {
	ESP_LOGI(AP_TAG, "Read_auth %d:", blenum1);
	esp_log_buffer_hex(AP_TAG, p_data->notify.value, p_data->notify.value_len);
	}
	if ((p_data->notify.value_len == 12) && (ptr->xbtauth == 2)) {
	uint8_t buff2[16];
        uint8_t xiv_char_data[12] = { 0x55,0x00,0xff,0xb6,0x2c,0x27,0xb3,0xb8,0xac,0x5a,0xef,0xaa};  //auth string
	xiv_char_data[3] = xiv_char_data[3] + blenum;  //for each position number different auth id
	xiv_char_data[5] = xiv_char_data[5] + R4SNUM;  //for each gate number different auth id
	if (macauth) {                                 // for each esp32 different auth id
        xiv_char_data[4] = binblemac [0];
        xiv_char_data[6] = binblemac [1];
        xiv_char_data[7] = binblemac [2];
        xiv_char_data[8] = binblemac [3];
        xiv_char_data[9] = binblemac [4];
        xiv_char_data[10] = binblemac [5];
	}
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
	if (fdebug) {
	ESP_LOGI(AP_TAG, "Write_auth_xi_ack %d:", blenum1);
	esp_log_buffer_hex(AP_TAG, buff1, 4);
	}
        esp_gatt_status_t ret_status = esp_ble_gattc_write_char( gattc_if,
                                  gl_profile_tab[blenum].conn_id,
                                  gl_profile_tab[blenum].auth_handle,
                                  4,
                                  buff1,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
            	if (ret_status != ESP_GATT_OK){
			conerr = 1;
            		if (fdebug) ESP_LOGE(AP_TAG, "Write_auth_xi_ack %d error", blenum1);
            	}
	buff2[0] = 0x03;
	buff2[1] = 0x01;
	if (fdebug) {
	ESP_LOGI(AP_TAG, "Write_setup_xi %d:", blenum1);
	esp_log_buffer_hex(AP_TAG, buff2, 2);
	}
        ret_status = esp_ble_gattc_write_char( gattc_if,
                                  gl_profile_tab[blenum].conn_id,
                                  gl_profile_tab[blenum].setup_handle,
                                  2,
                                  buff2,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
            	if (ret_status != ESP_GATT_OK){
            		if (fdebug) ESP_LOGE(AP_TAG, "Write_setup_xi %d error", blenum1);
            	}
	buff2[0] = 0x00;
	if (fdebug) {
	ESP_LOGI(AP_TAG, "Write_boil_xi %d:", blenum1);
	esp_log_buffer_hex(AP_TAG, buff2, 1);
	}
        ret_status = esp_ble_gattc_write_char( gattc_if,
                                  gl_profile_tab[blenum].conn_id,
                                  gl_profile_tab[blenum].boil_handle,
                                  1,
                                  buff2,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
            	if (ret_status != ESP_GATT_OK){
            		if (fdebug) ESP_LOGE(AP_TAG, "Write_boil_xi %d error", blenum1);
            	}
	buff2[0] = 0x17;
	if (fdebug) {
	ESP_LOGI(AP_TAG, "Write_time_xi %d:", blenum1);
	esp_log_buffer_hex(AP_TAG, buff2, 1);
	}
        ret_status = esp_ble_gattc_write_char( gattc_if,
                                  gl_profile_tab[blenum].conn_id,
                                  gl_profile_tab[blenum].time_handle,
                                  1,
                                  buff2,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
            	if (ret_status != ESP_GATT_OK){
            		if (fdebug) ESP_LOGE(AP_TAG, "Write_time_xi %d error", blenum1);
            	}

        ret_status = esp_ble_gattc_read_char( gattc_if,
                                  gl_profile_tab[blenum].conn_id,
                                  gl_profile_tab[blenum].mcuver_handle,
                                  ESP_GATT_AUTH_REQ_NONE);
            	if (ret_status != ESP_GATT_OK){
			conerr = 1;
            		if (fdebug) ESP_LOGE(AP_TAG, "Read_ver_req %d error", blenum1);
            	}

	if (fdebug) ESP_LOGI(AP_TAG, "Authorize %d Xiaomi ok", blenum1);
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	ptr->t_ppcon = 40;
	memset(ptr->sVer, 0, sizeof(ptr->sVer));
	ptr->btauthoriz = true;
	ptr->r4sConnErr = 0;
	ptr->NumConn++;
	if (!ptr->NumConn) ptr->NumConn--;
	bin2hex(gl_profile_tab[blenum].remote_bda, ptr->tBLEAddr,6,0);
	strcpy(ptr->DEV_NAME,ptr->RQC_NAME);
//	MqttPubSub(blenum, mqttConnected);	
	esp_ble_gattc_register_for_notify (gattc_if, gl_profile_tab[blenum].remote_bda, gl_profile_tab[blenum].rxchar_handle);

	} else {
	conerr = 1;
	if (fdebug) ESP_LOGI(AP_TAG, "Invalid %d Xiaomi product Id", blenum1);
	if ((ptr->DEV_TYP == 72) && (ptr->MiKettleID < 10000)) ptr->MiKettleID++;
	}
	}
	} if (p_data->notify.is_notify) {
//	if (fdebug) ESP_LOGI(AP_TAG, "GATTC_NOTIFY_EVT_0, Handle = 0x%X, Notify value:",p_data->notify.handle);
	} else {
//	if (fdebug) ESP_LOGI(AP_TAG, "GATTC_NOTIFY_EVT_0, Indicate value:");
	}
//	if (fdebug) esp_log_buffer_hex(AP_TAG, p_data->notify.value, p_data->notify.value_len);

        break;



    case ESP_GATTC_WRITE_DESCR_EVT:
	if (p_data->write.status != ESP_GATT_OK){
	conerr = 1;
	if (fdebug) ESP_LOGE(AP_TAG, "Write descr %d failed, error status = 0x%X", blenum1, p_data->write.status);
	conerr = 1;
            break;
	}
	uint8_t  write_char_crypt_data[16];
	int  write_char_data_len = 12;
        uint8_t write_char_data[12] = { 0x55,0x00,0xff,0xb6,0x2c,0x27,0xb3,0xb8,0xac,0x5a,0xef,0xaa};  //auth string

	write_char_data[3] = write_char_data[3] + blenum;  //for each position number different auth id
	write_char_data[5] = write_char_data[5] + R4SNUM;  //for each gate number different auth id
	if (macauth) {                                 // for each esp32 different auth id
        write_char_data[4] = binblemac [0];
        write_char_data[6] = binblemac [1];
        write_char_data[7] = binblemac [2];
        write_char_data[8] = binblemac [3];
        write_char_data[9] = binblemac [4];
        write_char_data[10] = binblemac [5];
	}
	if ((ptr->DEV_TYP < 73) && !ptr->xbtauth) {
	if ((ptr->DEV_TYP > 63) && (ptr->DEV_TYP < 73)) {
        write_char_data[0] = 0x90;
        write_char_data[1] = 0xca;
        write_char_data[2] = 0x85;
        write_char_data[3] = 0xde;
	write_char_data_len = 4;
	}
	if (fdebug) {
	ESP_LOGI(AP_TAG, "Write_auth %d:", blenum1);
	esp_log_buffer_hex(AP_TAG, write_char_data, write_char_data_len);
	}
        esp_gatt_status_t ret_status = esp_ble_gattc_write_char( gattc_if,
                                  gl_profile_tab[blenum].conn_id,
                                  gl_profile_tab[blenum].txchar_handle,
                                  write_char_data_len,
                                  write_char_data,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
            	if (ret_status != ESP_GATT_OK){
            		if (fdebug) ESP_LOGE(AP_TAG, "Write_auth %d error", blenum1);
			conerr = 1;
            	}
	if ((ptr->DEV_TYP > 63) && (ptr->DEV_TYP < 73)) {
	ptr->xbtauth = 1;
	esp_ble_gattc_register_for_notify (gattc_if, gl_profile_tab[blenum].remote_bda, gl_profile_tab[blenum].auth_handle);
	}
	} else if ((ptr->DEV_TYP > 63) && (ptr->DEV_TYP < 73) && (ptr->xbtauth == 1)) {
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
    		if (fdebug) ESP_LOGE(AP_TAG, "Write_auth_mi %d error", blenum1);
			conerr = 1;
            	}  else if (fdebug) ESP_LOGI(AP_TAG, "Write_auth_mi %d Ok", blenum1);

	} else if (ptr->DEV_TYP == 73) {
	ptr->xbtauth = 1;
//
        write_char_data[0] = 0xff;
        write_char_data[1] = 0xff;
	write_char_data_len = 2;
	if (fdebug) {
	ESP_LOGI(AP_TAG, "Write_auth %d:", blenum1);
	esp_log_buffer_hex(AP_TAG, write_char_data, write_char_data_len);
	}
        esp_gatt_status_t ret_status = esp_ble_gattc_write_char( gattc_if,
                                  gl_profile_tab[blenum].conn_id,
                                  gl_profile_tab[blenum].auth_handle,
                                  write_char_data_len,
                                  write_char_data,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
//
/*
        write_char_data[0] = 0x01;
        write_char_data[1] = 0x02;
        write_char_data[2] = 0x03;
        write_char_data[3] = 0x04;
	write_char_data_len = 4;
	if (fdebug) ESP_LOGI(AP_TAG, "Write_auth %d:", blenum1);
        esp_gatt_status_t ret_status = esp_ble_gattc_write_char( gattc_if,
                                  gl_profile_tab[blenum].conn_id,
                                  gl_profile_tab[blenum].setup_handle,
                                  write_char_data_len,
                                  write_char_data,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);

*/
            	if (ret_status != ESP_GATT_OK){
		if (fdebug) ESP_LOGE(AP_TAG, "Write_auth %d error", blenum1);
		conerr = 1;
            	} else {
	ptr->t_rspdel = 0;
	ptr->t_rspcnt = 5;
	ptr->t_ppcon = 40;
	memset(ptr->sVer, 0, sizeof(ptr->sVer));
	ptr->btauthoriz = true;
	ptr->r4sConnErr = 0;
	ptr->r4sAuthCount = 0;
	ptr->f_Sync = 16;
	ptr->NumConn++;
	if (!ptr->NumConn) ptr->NumConn--;
	bin2hex(gl_profile_tab[blenum].remote_bda, ptr->tBLEAddr,6,0);
	strcpy(ptr->DEV_NAME,ptr->RQC_NAME);
	MqttPubSub(blenum, mqttConnected);	
	}
	} else if (ptr->DEV_TYP == 74) {
        write_char_data[0] = 0x00;
        write_char_data[1] = 0xff;
        write_char_data[2] = 0x00;
        write_char_data[3] = 0x00;
        write_char_data[4] = 0x9a;
        write_char_data[5] = 0x17;
        write_char_data[6] = 0x02;
        write_char_data[7] = (ptr->PassKey >> 8) & 0xff;
        write_char_data[8] = ptr->PassKey & 0xff;
        write_char_data[9] = 0x00;
	for (int i = 0; i < 9; i++) write_char_data[9] = write_char_data[9] ^ write_char_data[i];
	write_char_data[9] = write_char_data[9] ^ 0xff;
	write_char_data_len = 10;
	if (fdebug) {
	ESP_LOGI(AP_TAG, "Write_auth %d, Passkey %d:", blenum1, ptr->PassKey);
	esp_log_buffer_hex(AP_TAG, write_char_data, write_char_data_len);
	}
        esp_gatt_status_t ret_status = esp_ble_gattc_write_char( gattc_if,
                                  gl_profile_tab[blenum].conn_id,
                                  gl_profile_tab[blenum].rxchar_handle,
                                  write_char_data_len,
                                  write_char_data,
                                  ESP_GATT_WRITE_TYPE_NO_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);

            	if (ret_status != ESP_GATT_OK){
		if (fdebug) ESP_LOGE(AP_TAG, "Write_auth %d error", blenum1);
		conerr = 1;
		}
	}
        break;
    case ESP_GATTC_SRVC_CHG_EVT: {
        esp_bd_addr_t bda;
	memcpy(bda, p_data->srvc_chg.remote_bda, sizeof(esp_bd_addr_t));
	if (fdebug) {
	ESP_LOGI(AP_TAG, "ESP_GATTC_SRVC_CHG_EVT, bd_addr:");
        esp_log_buffer_hex(AP_TAG, bda, sizeof(esp_bd_addr_t));
	}
        break;
    }
    case ESP_GATTC_WRITE_CHAR_EVT:
	if (p_data->write.status != ESP_GATT_OK){
	if (fdebug) ESP_LOGE(AP_TAG, "Write char %d failed, error status = 0x%X", blenum1, p_data->write.status);
	if (p_data->write.status == 5) ptr->r4sAuthCount++;
	if ((ptr->DEV_TYP > 61) && (ptr->DEV_TYP < 64)) {
	if (ptr->r4sAuthCount > 10) {
//	esp_ble_remove_bond_device(gl_profile_tab[blenum].remote_bda);
	ptr->r4sAuthCount = 3;
	}
	} else if (ptr->r4sAuthCount > 6) ptr->r4sAuthCount = 0;
	conerr = 1;
	} else start_scan();
        break;

    case ESP_GATTC_DISCONNECT_EVT:
	if (memcmp(p_data->disconnect.remote_bda, gl_profile_tab[blenum].remote_bda, 6) == 0){
	ptr->btopen = false;
	ptr->btopenreq = false;
	memset(ptr->sVer, 0, sizeof(ptr->sVer));
	ptr->btauthoriz = false;
	ptr->get_server = false;
	ptr->xbtauth = 0;
	if (fdebug) ESP_LOGI(AP_TAG, "ESP_GATTC_DISCONNECT_EVT, reason = %d", p_data->disconnect.reason);
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
//	if ((conerr || f_update) && ptr->btopen) esp_ble_gattc_close(gl_profile_tab[blenum].gattc_if,gl_profile_tab[blenum].conn_id);
	if ((conerr || f_update) && ptr->btopen) esp_ble_gap_disconnect(gl_profile_tab[blenum].remote_bda);

}

//*** Gattc event handler end ********************

//*** Gap event handler **************************
static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
	uint8_t *adv_name = NULL;
	uint8_t adv_name_len = 0;
	uint16_t SHandle = 0;
	uint8_t blenum = 255;
	uint8_t blenum1 = 255;
        struct BleDevSt *ptr;
    switch (event) {

    case ESP_GAP_BLE_READ_RSSI_COMPLETE_EVT:
//	if (fdebug) esp_log_buffer_hex(AP_TAG, param->read_rssi_cmpl.remote_addr, 6);
	if (!memcmp(gl_profile_tab[PROFILE_A_APP_ID].remote_bda, param->read_rssi_cmpl.remote_addr, 6)) blenum = 0;
	else if (!memcmp(gl_profile_tab[PROFILE_B_APP_ID].remote_bda, param->read_rssi_cmpl.remote_addr, 6)) blenum = 1;
	else if (!memcmp(gl_profile_tab[PROFILE_C_APP_ID].remote_bda, param->read_rssi_cmpl.remote_addr, 6)) blenum = 2;
	else if (!memcmp(gl_profile_tab[PROFILE_D_APP_ID].remote_bda, param->read_rssi_cmpl.remote_addr, 6)) blenum = 3;
	else if (!memcmp(gl_profile_tab[PROFILE_E_APP_ID].remote_bda, param->read_rssi_cmpl.remote_addr, 6)) blenum = 4;
	blenum1 = blenum + 1;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	if (blenum < 5) {
	ptr->iRssi = param->read_rssi_cmpl.rssi;
//if (fdebug) ESP_LOGI(AP_TAG,"read %d-RSSI: %d", blenum1, param->read_rssi_cmpl.rssi); //test #2 with data from read_rssi_cmpl
	if ((ptr->r4sConnErr < 6 ) && (ptr->btauthoriz)) {
	if ((ptr->sendDataLen > 0) && (ptr->sendDataLen < BLE_INPUT_BUFFSIZE)) {
	SHandle = gl_profile_tab[blenum].txchar_handle;
	if ((ptr->DEV_TYP > 63) && (ptr->DEV_TYP < 73)) {
	switch (ptr->sendDataHandle) {
	case 0:
	SHandle = gl_profile_tab[blenum].rxchar_handle;
	break;
	case 2:
	SHandle = gl_profile_tab[blenum].auth_handle;
	break;
	case 3:
	SHandle = gl_profile_tab[blenum].ver_handle;
	break;
	case 4:
	SHandle = gl_profile_tab[blenum].setup_handle;
	break;
	case 5:
	SHandle = gl_profile_tab[blenum].time_handle;
	break;
	case 6:
	SHandle = gl_profile_tab[blenum].boil_handle;
	break;
	case 7:
	SHandle = gl_profile_tab[blenum].mcuver_handle;
	break;
	case 8:
	SHandle = gl_profile_tab[blenum].update_handle;
	break;
	}
	} else if (ptr->DEV_TYP == 73) {
	switch (ptr->sendDataHandle) {
	case 2:
	SHandle = gl_profile_tab[blenum].auth_handle;
	break;
	case 3:
	SHandle = gl_profile_tab[blenum].time_handle;
	break;
	}
	}
	esp_gatt_status_t ret_status;
	if (ptr->DEV_TYP == 74) {
	SHandle = gl_profile_tab[blenum].rxchar_handle;
	ret_status = esp_ble_gattc_write_char( gl_profile_tab[blenum].gattc_if,
                                  gl_profile_tab[blenum].conn_id,
                                  SHandle,
                                  ptr->sendDataLen,
                                  ptr->sendData,
                                  ESP_GATT_WRITE_TYPE_NO_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
	} else {
	ret_status = esp_ble_gattc_write_char( gl_profile_tab[blenum].gattc_if,
                                  gl_profile_tab[blenum].conn_id,
                                  SHandle,
                                  ptr->sendDataLen,
                                  ptr->sendData,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
	}
	if (ret_status != ESP_GATT_OK){
	if (fdebug) ESP_LOGE(AP_TAG, "Write_char_data %d error", blenum1);
	ptr->r4sConnErr++;
	}  else {
if (fdebug) {
//	ESP_LOGI(AP_TAG, "Connection Error: %d", ptr->r4sConnErr);
	ESP_LOGI(AP_TAG, "Send %d, Handle 0x%X, Data:", blenum1, SHandle);
	esp_log_buffer_hex(AP_TAG, ptr->sendData, ptr->sendDataLen);
	}
	}
	ptr->sendDataLen = 0;
	} else if ((ptr->DEV_TYP == 73) && !ptr->sendDataLen) {
        esp_gatt_status_t ret_status = esp_ble_gattc_read_char( gl_profile_tab[blenum].gattc_if,
                                  gl_profile_tab[blenum].conn_id,
                                  gl_profile_tab[blenum].rxchar_handle,
                                  ESP_GATT_AUTH_REQ_NONE);
            	if (ret_status != ESP_GATT_OK){
	ptr->r4sConnErr++;
            		if (fdebug) ESP_LOGE(AP_TAG, "Read_char_data_req %d error", blenum1);
            	}  else {
//if (fdebug) ESP_LOGI(AP_TAG, "Read_char_data_req %d  Ok", blenum1);
	}
	}


	} else if (ptr->btauthoriz) esp_ble_gattc_close(gl_profile_tab[blenum].gattc_if,gl_profile_tab[blenum].conn_id);	
	}
	break;

    case ESP_GAP_BLE_SET_LOCAL_PRIVACY_COMPLETE_EVT:
	if (param->local_privacy_cmpl.status != ESP_BT_STATUS_SUCCESS){
	if (fdebug) ESP_LOGE(AP_TAG, "Config local privacy failed, error code =0x%X", param->local_privacy_cmpl.status);
            break;
	}
        esp_err_t scan_ret = ESP_GATT_OK;
	if (!ble_mon || (ble_mon == 3)) {
	if ((BleDevStA.REQ_NAME[0] && !BleDevStA.btopenreq) || (BleDevStB.REQ_NAME[0] && !BleDevStB.btopenreq) || (BleDevStC.REQ_NAME[0] && !BleDevStC.btopenreq) || (BleDevStD.REQ_NAME[0] && !BleDevStD.btopenreq) || (BleDevStE.REQ_NAME[0] && !BleDevStE.btopenreq)) {
	scan_ret = esp_ble_gap_set_scan_params(&ble_scan_params);
	IsPassiveScan = false;
	} else if (ble_mon) {
	scan_ret = esp_ble_gap_set_scan_params(&ble_pscan_params);
	IsPassiveScan = true;
	}
	} else if (ble_mon == 2) {
	scan_ret = esp_ble_gap_set_scan_params(&ble_scan_params);
	IsPassiveScan = false;
	} else if (ble_mon == 1) {
	scan_ret = esp_ble_gap_set_scan_params(&ble_pscan_params);
	IsPassiveScan = true;
	}
	if (scan_ret){
	if (fdebug) ESP_LOGE(AP_TAG, "Set scan params error, error code = 0x%X", scan_ret);
	}
        break;


    case ESP_GAP_BLE_PASSKEY_REQ_EVT:                           /* passkey request event */
        /* Call the following function to input the passkey which is displayed on the remote device */
	if (fdebug) ESP_LOGI(AP_TAG, "ESP_GAP_BLE_PASSKEY_REQ_EVT");
	if (!memcmp(gl_profile_tab[PROFILE_A_APP_ID].remote_bda, param->ble_security.ble_req.bd_addr, 6)) blenum = 0;
	else if (!memcmp(gl_profile_tab[PROFILE_B_APP_ID].remote_bda, param->ble_security.ble_req.bd_addr, 6)) blenum = 1;
	else if (!memcmp(gl_profile_tab[PROFILE_C_APP_ID].remote_bda, param->ble_security.ble_req.bd_addr, 6)) blenum = 2;
	else if (!memcmp(gl_profile_tab[PROFILE_D_APP_ID].remote_bda, param->ble_security.ble_req.bd_addr, 6)) blenum = 3;
	else if (!memcmp(gl_profile_tab[PROFILE_E_APP_ID].remote_bda, param->ble_security.ble_req.bd_addr, 6)) blenum = 4;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
        esp_ble_passkey_reply(param->ble_security.ble_req.bd_addr, true, ptr->PassKey);
	if (fdebug) ESP_LOGI(AP_TAG, "Passkey reply: %d", ptr->PassKey);
        break;
    case ESP_GAP_BLE_OOB_REQ_EVT: {
	if (fdebug) ESP_LOGI(AP_TAG, "ESP_GAP_BLE_OOB_REQ_EVT");
        uint8_t tk[16] = {1}; //If you paired with OOB, both devices need to use the same tk
        esp_ble_oob_req_reply(param->ble_security.ble_req.bd_addr, tk, sizeof(tk));
        break;
    }
    case ESP_GAP_BLE_LOCAL_IR_EVT:                               /* BLE local IR event */
	if (fdebug) ESP_LOGI(AP_TAG, "ESP_GAP_BLE_LOCAL_IR_EVT");
        break;
    case ESP_GAP_BLE_LOCAL_ER_EVT:                               /* BLE local ER event */
	if (fdebug) ESP_LOGI(AP_TAG, "ESP_GAP_BLE_LOCAL_ER_EVT");
        break;
    case ESP_GAP_BLE_SEC_REQ_EVT:
        /* send the positive(true) security response to the peer device to accept the security request.
	if not accept the security request, should send the security response with negative(false) accept value*/
        esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
        break;
    case ESP_GAP_BLE_NC_REQ_EVT:
        /* The app will receive this evt when the IO has DisplayYesNO capability and the peer device IO also has DisplayYesNo capability.
        show the passkey number to the user to confirm it with the number displayed by peer device. */
        esp_ble_confirm_reply(param->ble_security.ble_req.bd_addr, true);
	if (fdebug) ESP_LOGI(AP_TAG, "ESP_GAP_BLE_NC_REQ_EVT, the passkey Notify number:%d", param->ble_security.key_notif.passkey);
        break;
    case ESP_GAP_BLE_PASSKEY_NOTIF_EVT:  ///the app will receive this evt when the IO  has Output capability and the peer device IO has Input capability.
        ///show the passkey number to the user to input it in the peer device.
	if (fdebug) ESP_LOGI(AP_TAG, "The passkey Notify number:%06d", param->ble_security.key_notif.passkey);
        break;
    case ESP_GAP_BLE_KEY_EVT:
        //shows the ble key info share with peer device to the user.
	if (fdebug) ESP_LOGI(AP_TAG, "key type = %s", esp_key_type_to_str(param->ble_security.ble_key.key_type));
        break;
    case ESP_GAP_BLE_AUTH_CMPL_EVT: {
        char bd_addr[20];
	if (!memcmp(gl_profile_tab[PROFILE_A_APP_ID].remote_bda, param->ble_security.ble_req.bd_addr, 6)) blenum = 0;
	else if (!memcmp(gl_profile_tab[PROFILE_B_APP_ID].remote_bda, param->ble_security.ble_req.bd_addr, 6)) blenum = 1;
	else if (!memcmp(gl_profile_tab[PROFILE_C_APP_ID].remote_bda, param->ble_security.ble_req.bd_addr, 6)) blenum = 2;
	else if (!memcmp(gl_profile_tab[PROFILE_D_APP_ID].remote_bda, param->ble_security.ble_req.bd_addr, 6)) blenum = 3;
	else if (!memcmp(gl_profile_tab[PROFILE_E_APP_ID].remote_bda, param->ble_security.ble_req.bd_addr, 6)) blenum = 4;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	bin2hex(param->ble_security.auth_cmpl.bd_addr, bd_addr,6,0x3a);
	if (fdebug) {
	ESP_LOGI(AP_TAG, "Remote BD_ADDR: %s", bd_addr);
        ESP_LOGI(AP_TAG, "Address type = %d", param->ble_security.auth_cmpl.addr_type);
        ESP_LOGI(AP_TAG, "Pair status = %s",param->ble_security.auth_cmpl.success ? "success" : "fail");
	}
	if (!param->ble_security.auth_cmpl.success) {
	if (fdebug) ESP_LOGI(AP_TAG, "Fail reason = 0x%X",param->ble_security.auth_cmpl.fail_reason);
	if (ptr->DEV_TYP == 73) ptr->r4sAuthCount = 3;
//	if (param->ble_security.auth_cmpl.fail_reason == 0x52) esp_ble_remove_bond_device(param->ble_security.auth_cmpl.bd_addr);
	} else {
	if (ptr->DEV_TYP == 73) ptr->r4sAuthCount = 0;
	if (fdebug) ESP_LOGI(AP_TAG, "Auth mode = %s",esp_auth_req_to_str(param->ble_security.auth_cmpl.auth_mode));
	}
        break;
	}


    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT: 
//step 1: start scan only if not update, if not scanning, if not already starting scan or if no connections is opening   
	if (!f_update && !Isscanning && !StartStopScanReq && (!BleDevStA.btopenreq || BleDevStA.btopen) && (!BleDevStB.btopenreq || BleDevStB.btopen) && (!BleDevStC.btopenreq || BleDevStC.btopen) && (!BleDevStD.btopenreq || BleDevStD.btopen) && (!BleDevStE.btopenreq || BleDevStE.btopen)) {
//step 2: start scan if defined but not open connection present
//	if ((BleDevStA.REQ_NAME[0] && !BleDevStA.btopenreq) || (BleDevStB.REQ_NAME[0] && !BleDevStB.btopenreq) || (BleDevStC.REQ_NAME[0] && !BleDevStC.btopenreq) || (BleDevStD.REQ_NAME[0] && !BleDevStD.btopenreq) || (BleDevStE.REQ_NAME[0] && !BleDevStE.btopenreq)) {
	uint32_t duration = 0; //30
	FND_NAME[0] = 0;
	FND_ADDR[0] = 0;
	FND_ADDRx[0] = 0;
        esp_ble_gap_start_scanning(duration);
	StartStopScanReq = true;
	if (fdebug) ESP_LOGI(AP_TAG, "Scan starting");
//		}
	}
        break;

    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
        //scan start complete event to indicate scan start successfully or failed
	StartStopScanReq = false;
	if (param->scan_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
	if (fdebug) ESP_LOGE(AP_TAG, "scan start failed, error status = 0x%X", param->scan_start_cmpl.status);
//	if (ble_mon && !f_update && !Isscanning) esp_restart();
//	if (!f_update && !Isscanning) esp_restart();
	break;
	} 
	if (fdebug) ESP_LOGI(AP_TAG, "Scan start success");
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
//	if (fdebug) ESP_LOGI(AP_TAG, "searched Adv Data Len %d, Scan Response Len %d", scan_result->scan_rst.adv_data_len, scan_result->scan_rst.scan_rsp_len);
	adv_name = esp_ble_resolve_adv_data(scan_result->scan_rst.ble_adv,
                                                ESP_BLE_AD_TYPE_NAME_CMPL, &adv_name_len);
	memset(devname,0,32);
	if (adv_name_len && (adv_name_len < 32)) memcpy(devname,adv_name, adv_name_len);

//	if ((scan_result->scan_rst.adv_data_len > 16) && !memcmp(&scan_result->scan_rst.ble_adv[0],"\x02\x01\x04\x03\x02",5)) {

	if (fdebug) {
	ESP_LOGI(AP_TAG, "Remote BD_ADDR: %s", FND_ADDRx);
	ESP_LOGI(AP_TAG, "Rssi %d dBm, Device Name: %s", scan_result->scan_rst.rssi, devname);
	}
//#if CONFIG_EXAMPLE_DUMP_ADV_DATA_AND_SCAN_RESP
    	if (scan_result->scan_rst.adv_data_len > 0) {
    		if (fdebug) {
		ESP_LOGI(AP_TAG, "Adv data:");
                esp_log_buffer_hex(AP_TAG, &scan_result->scan_rst.ble_adv[0], scan_result->scan_rst.adv_data_len);
		}
    	}
    	if (scan_result->scan_rst.scan_rsp_len > 0) {
    		if (fdebug) {
		ESP_LOGI(AP_TAG, "Scan resp:");
                esp_log_buffer_hex(AP_TAG, &scan_result->scan_rst.ble_adv[scan_result->scan_rst.adv_data_len], scan_result->scan_rst.scan_rsp_len);
		}
    	}
//#endif
	if (fdebug) ESP_LOGI(AP_TAG, "\n");
//}


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
	switch (scan_result->scan_rst.adv_data_len) {
	case 17:
	if (!memcmp(&scan_result->scan_rst.ble_adv[0],"\x10\x16\x1a\x18",4)) id = 3;
	break;
	case 19:
	if (!memcmp(&scan_result->scan_rst.ble_adv[0],"\x12\x16\x1a\x18",4)) id = 3;
	break;
	case 24:
// 02 01 06 03 02 1b 18 10 16 1b 18 02 a4 e6 07 0a 12 08 26 27 fd ff c0 3f
//  0        3    5                 11                17          21
	if (!memcmp(&scan_result->scan_rst.ble_adv[3],"\x03\x02\x1b\x18\x10\x16\x1b\x18",8)) id = 2;
	break;
	case 27:
	if (!memcmp(&scan_result->scan_rst.ble_adv[0],"\x1a\xff\x4c\x00\x02\x15",6)) id = 0x42;
	break;
	case 31:
// 02 01 06 03 02 1d 18 09 ff 57 01 c8 47 8c ef 9c 4c 0d 16 1d 18 02 20 44 b2 07 01 01 0d 0f 0e
//  0        3    5                 11                17          21
	if (!memcmp(&scan_result->scan_rst.ble_adv[3],"\x03\x02\x1d\x18\x09\xff\x57\x01",8) && 
		!memcmp(&scan_result->scan_rst.ble_adv[17],"\x0d\x16\x1d\x18",4)) id = 2;
	else if (!memcmp(&scan_result->scan_rst.ble_adv[3],"\x03\x02\x5a\xfd\x17\x16\x5a\xfd",8)) id = 0x44;
	break;
	}

	while ((i < BleMonNum) && (!found)) {
	if (BleMR[i].id == id) {
      	if (((id < 0x40) && !memcmp(BleMR[i].mac, scan_result->scan_rst.bda, 6)) ||
 		((id == 0x42) && !memcmp(BleMR[i].mac, &scan_result->scan_rst.ble_adv[6], 16)) ||
 		((id == 0x44) && vstsign(&scan_result->scan_rst.ble_adv[11],BleMR[i].mac))) {
	BleMX[i].state = 1;
	if (!BleMX[i].ttick) t_lasts = 0;
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
	if (id == 2) {
	if (BleMX[i].advdat[5] == 0x1b) {
	if ((BleMX[i].advdat[12] & 0xa0) == 0x20) {
	if (BleMX[i].advdat[11] & 0x01) BleMX[i].par1 = BleMX[i].advdat[22] + (BleMX[i].advdat[23] << 8);
	else BleMX[i].par1 = (BleMX[i].advdat[22] >> 1) + (BleMX[i].advdat[23] << 7);
	BleMX[i].par2 = BleMX[i].advdat[13] + (BleMX[i].advdat[14] << 8);
	BleMX[i].par3 = BleMX[i].advdat[15] + (BleMX[i].advdat[16] << 8);
	BleMX[i].par4 = BleMX[i].advdat[17] + (BleMX[i].advdat[18] << 8);
	BleMX[i].par5 = BleMX[i].advdat[19] + (BleMX[i].advdat[11] << 8);
	if (BleMX[i].advdat[12] & 0x02) BleMX[i].par7 = BleMX[i].advdat[20] + (BleMX[i].advdat[21] << 8);
	} else if (!(BleMX[i].advdat[12] & 0x80)) {
	if (BleMX[i].advdat[11] & 0x01) BleMX[i].par6 = BleMX[i].advdat[22] + (BleMX[i].advdat[23] << 8);
	else BleMX[i].par6 = (BleMX[i].advdat[22] >> 1) + (BleMX[i].advdat[23] << 7);
	}
	} else {
	if ((BleMX[i].advdat[21] & 0x22) == 0x22) {
	if (BleMX[i].advdat[21] & 0x01) BleMX[i].par1 = BleMX[i].advdat[22] + (BleMX[i].advdat[23] << 8);
	else BleMX[i].par1 = (BleMX[i].advdat[22] >> 1) + (BleMX[i].advdat[23] << 7);
	BleMX[i].par2 = BleMX[i].advdat[24] + (BleMX[i].advdat[25] << 8);
	BleMX[i].par3 = BleMX[i].advdat[26] + (BleMX[i].advdat[27] << 8);
	BleMX[i].par4 = BleMX[i].advdat[28] + (BleMX[i].advdat[29] << 8);
	BleMX[i].par5 = BleMX[i].advdat[30] + (BleMX[i].advdat[21] << 8);
	BleMX[i].par7 = 0;
	} else {
	if (BleMX[i].advdat[21] & 0x01) BleMX[i].par6 = BleMX[i].advdat[22] + (BleMX[i].advdat[23] << 8);
	else BleMX[i].par6 = (BleMX[i].advdat[22] >> 1) + (BleMX[i].advdat[23] << 7);
	BleMX[i].par7 = 0;
	}
	}	

	} else if (id == 3) {
	if (BleMX[i].advdat[0] < 0x12) {
	BleMX[i].par1 = BleMX[i].advdat[11] + (BleMX[i].advdat[10] << 8);
	if (BleMX[i].par1 & 0x8000) {
	BleMX[i].par1 = (BleMX[i].par1  ^ 0x0ffff) + 1;
	BleMX[i].par1 = BleMX[i].par1 * 10;
	BleMX[i].par1 = (BleMX[i].par1  ^ 0x0ffff) + 1;
	} else BleMX[i].par1 = BleMX[i].par1 * 10;
	BleMX[i].par2 = BleMX[i].advdat[12] * 100;
	BleMX[i].par3 = BleMX[i].advdat[15] + (BleMX[i].advdat[14] << 8);
	BleMX[i].par4 = BleMX[i].advdat[13];
	BleMX[i].par5 = BleMX[i].advdat[16];
	} else {
	BleMX[i].par1 = BleMX[i].advdat[10] + (BleMX[i].advdat[11] << 8);
	BleMX[i].par2 = BleMX[i].advdat[12] + (BleMX[i].advdat[13] << 8);
	BleMX[i].par3 = BleMX[i].advdat[14] + (BleMX[i].advdat[15] << 8);
	BleMX[i].par4 = BleMX[i].advdat[16];
	BleMX[i].par5 = BleMX[i].advdat[17];
	BleMX[i].par6 = BleMX[i].advdat[18];
	}
	} else if (id == 0x44) {
	BleMX[i].par1 = BleMX[i].advdat[11];
	BleMX[i].par2 = BleMX[i].advdat[23];
	}
	found = 1;
	}
	}
	i++;
	if ((id == 0x44) && !found && (i == BleMonNum)) {
	id = 0x04;
	i = 0;
	}
	}
	i = 0;
	while ((i < BleMonNum) && (!found)) {
      	if (!BleMR[i].sto && !BleMX[i].ttick) {
	BleMX[i].state = 1;
	t_lasts = 0;
	BleMX[i].mto = 0;
	BleMX[i].ttick = BleMonDefTO;
	BleMX[i].rssi = scan_result->scan_rst.rssi;
	if (id < 0x40) memcpy(BleMR[i].mac, scan_result->scan_rst.bda, 6);
	else if (id == 0x42) memcpy(BleMR[i].mac, &scan_result->scan_rst.ble_adv[6], 16);
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
	if (id == 2) {
	if ((BleMX[i].advdat[21] & 0x22) == 0x22) {
	if (BleMX[i].advdat[21] & 0x01) BleMX[i].par1 = BleMX[i].advdat[22] + (BleMX[i].advdat[23] << 8);
	else BleMX[i].par1 = (BleMX[i].advdat[22] >> 1) + (BleMX[i].advdat[23] << 7);
	BleMX[i].par2 = BleMX[i].advdat[24] + (BleMX[i].advdat[25] << 8);
	BleMX[i].par3 = BleMX[i].advdat[26] + (BleMX[i].advdat[27] << 8);
	BleMX[i].par4 = BleMX[i].advdat[28] + (BleMX[i].advdat[29] << 8);
	BleMX[i].par5 = BleMX[i].advdat[30] + (BleMX[i].advdat[21] << 8);
	} else {
	if (BleMX[i].advdat[21] & 0x01) BleMX[i].par6 = BleMX[i].advdat[22] + (BleMX[i].advdat[23] << 8);
	else BleMX[i].par6 = (BleMX[i].advdat[22] >> 1) + (BleMX[i].advdat[23] << 7);
	}
	} else if (id == 3) {
	if (BleMX[i].advdat[0] < 0x12) {
	BleMX[i].par1 = BleMX[i].advdat[11] + (BleMX[i].advdat[10] << 8);
	if (BleMX[i].par1 & 0x8000) {
	BleMX[i].par1 = (BleMX[i].par1  ^ 0x0ffff) + 1;
	BleMX[i].par1 = BleMX[i].par1 * 10;
	BleMX[i].par1 = (BleMX[i].par1  ^ 0x0ffff) + 1;
	} else BleMX[i].par1 = BleMX[i].par1 * 10;
	BleMX[i].par2 = BleMX[i].advdat[12] * 100;
	BleMX[i].par3 = BleMX[i].advdat[15] + (BleMX[i].advdat[14] << 8);
	BleMX[i].par4 = BleMX[i].advdat[13];
	BleMX[i].par5 = BleMX[i].advdat[16];
	} else {
	BleMX[i].par1 = BleMX[i].advdat[10] + (BleMX[i].advdat[11] << 8);
	BleMX[i].par2 = BleMX[i].advdat[12] + (BleMX[i].advdat[13] << 8);
	BleMX[i].par3 = BleMX[i].advdat[14] + (BleMX[i].advdat[15] << 8);
	BleMX[i].par4 = BleMX[i].advdat[16];
	BleMX[i].par5 = BleMX[i].advdat[17];
	BleMX[i].par6 = BleMX[i].advdat[18];
	}
	} else if (id == 0x44) {
	BleMX[i].par1 = BleMX[i].advdat[11];
	BleMX[i].par2 = BleMX[i].advdat[23];
	}
	}
	i++;
	}
	}
//
    	if ((BleDevStA.btopen || !BleDevStA.btopenreq) && (BleDevStB.btopen || !BleDevStB.btopenreq) && (BleDevStC.btopen || !BleDevStC.btopenreq) && (BleDevStD.btopen || !BleDevStD.btopenreq) && (BleDevStE.btopen || !BleDevStE.btopenreq)) {

        	if ((!BleDevStA.btopenreq && BleDevStA.REQ_NAME[0] &&  strlen(BleDevStA.REQ_NAME) == adv_name_len && !strncmp((char *)adv_name, BleDevStA.REQ_NAME, adv_name_len)) ||
                (!BleDevStA.btopenreq && BleDevStA.REQ_NAME[0] &&  strlen(BleDevStA.REQ_NAME) == 12 && !incascmp(BleDevStA.REQ_NAME,FND_ADDR,12))) {
			if (FND_NAME[0]) strcpy (BleDevStA.RQC_NAME,FND_NAME);
			else strcpy (BleDevStA.RQC_NAME,BleDevStA.REQ_NAME);
	if (fdebug) ESP_LOGI(AP_TAG, "Searched 1 device %s\n", BleDevStA.RQC_NAME);
			BleDevStA.btopen = false;
                	BleDevStA.btopenreq = true;
                	memcpy(&(scan_rsta), scan_result, sizeof(esp_ble_gap_cb_param_t));
                	if (Isscanning) {
			esp_ble_gap_stop_scanning();
			StartStopScanReq = true;
			}
        	}

        	else if ((!BleDevStB.btopenreq && BleDevStB.REQ_NAME[0] &&  strlen(BleDevStB.REQ_NAME) == adv_name_len && !strncmp((char *)adv_name, BleDevStB.REQ_NAME, adv_name_len)) ||
                (!BleDevStB.btopenreq && BleDevStB.REQ_NAME[0] &&  strlen(BleDevStB.REQ_NAME) == 12 && !incascmp(BleDevStB.REQ_NAME,FND_ADDR,12))) {
			if (FND_NAME[0]) strcpy (BleDevStB.RQC_NAME,FND_NAME);
			else strcpy (BleDevStB.RQC_NAME,BleDevStB.REQ_NAME);
	if (fdebug) ESP_LOGI(AP_TAG, "Searched 2 device %s\n", BleDevStB.RQC_NAME);
			BleDevStB.btopen = false;
                	BleDevStB.btopenreq = true;
                	memcpy(&(scan_rstb), scan_result, sizeof(esp_ble_gap_cb_param_t));
                	if (Isscanning) {
			esp_ble_gap_stop_scanning();
			StartStopScanReq = true;
			}
        	}

        	else if ((!BleDevStC.btopenreq && BleDevStC.REQ_NAME[0] &&  strlen(BleDevStC.REQ_NAME) == adv_name_len && !strncmp((char *)adv_name, BleDevStC.REQ_NAME, adv_name_len)) ||
                (!BleDevStC.btopenreq && BleDevStC.REQ_NAME[0] &&  strlen(BleDevStC.REQ_NAME) == 12 && !incascmp(BleDevStC.REQ_NAME,FND_ADDR,12))) {
			if (FND_NAME[0]) strcpy (BleDevStC.RQC_NAME,FND_NAME);
			else strcpy (BleDevStC.RQC_NAME,BleDevStC.REQ_NAME);
	if (fdebug) ESP_LOGI(AP_TAG, "Searched 3 device %s\n", BleDevStC.RQC_NAME);
			BleDevStC.btopen = false;
                	BleDevStC.btopenreq = true;
                	memcpy(&(scan_rstc), scan_result, sizeof(esp_ble_gap_cb_param_t));
                	if (Isscanning) {
			esp_ble_gap_stop_scanning();
			StartStopScanReq = true;
			}
        	}

        	else if ((!BleDevStD.btopenreq && BleDevStD.REQ_NAME[0] &&  strlen(BleDevStD.REQ_NAME) == adv_name_len && !strncmp((char *)adv_name, BleDevStD.REQ_NAME, adv_name_len)) ||
                (!BleDevStD.btopenreq && BleDevStD.REQ_NAME[0] &&  strlen(BleDevStD.REQ_NAME) == 12 && !incascmp(BleDevStD.REQ_NAME,FND_ADDR,12))) {
			if (FND_NAME[0]) strcpy (BleDevStD.RQC_NAME,FND_NAME);
			else strcpy (BleDevStD.RQC_NAME,BleDevStD.REQ_NAME);
	if (fdebug) ESP_LOGI(AP_TAG, "Searched 4 device %s\n", BleDevStD.RQC_NAME);
			BleDevStD.btopen = false;
                	BleDevStD.btopenreq = true;
                	memcpy(&(scan_rstd), scan_result, sizeof(esp_ble_gap_cb_param_t));
                	if (Isscanning) {
			esp_ble_gap_stop_scanning();
			StartStopScanReq = true;
			}
        	}

        	else if ((!BleDevStE.btopenreq && BleDevStE.REQ_NAME[0] &&  strlen(BleDevStE.REQ_NAME) == adv_name_len && !strncmp((char *)adv_name, BleDevStE.REQ_NAME, adv_name_len)) ||
                (!BleDevStE.btopenreq && BleDevStE.REQ_NAME[0] &&  strlen(BleDevStE.REQ_NAME) == 12 && !incascmp(BleDevStE.REQ_NAME,FND_ADDR,12))) {
			if (FND_NAME[0]) strcpy (BleDevStE.RQC_NAME,FND_NAME);
			else strcpy (BleDevStE.RQC_NAME,BleDevStE.REQ_NAME);
	if (fdebug) ESP_LOGI(AP_TAG, "Searched 5 device %s\n", BleDevStE.RQC_NAME);
			BleDevStE.btopen = false;
                	BleDevStE.btopenreq = true;
                	memcpy(&(scan_rste), scan_result, sizeof(esp_ble_gap_cb_param_t));
                	if (Isscanning) {
			esp_ble_gap_stop_scanning();
			StartStopScanReq = true;
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
	StartStopScanReq = false;
	if (param->scan_stop_cmpl.status != ESP_BT_STATUS_SUCCESS){
	if (fdebug) ESP_LOGE(AP_TAG, "Scan stop failed, error status = 0x%X", param->scan_stop_cmpl.status);
	} else {
	Isscanning = false;
	if (fdebug) ESP_LOGI(AP_TAG, "Scan stop successfully");
	if (!BleDevStA.btopen && BleDevStA.btopenreq) {
	if (fdebug) ESP_LOGI(AP_TAG, "Connect 1 to the remote device");
	esp_ble_gattc_open(gl_profile_tab[PROFILE_A_APP_ID].gattc_if, scan_rsta.scan_rst.bda, scan_rsta.scan_rst.ble_addr_type, true);
	} else if (!BleDevStB.btopen && BleDevStB.btopenreq) {
	if (fdebug) ESP_LOGI(AP_TAG, "Connect 2 to the remote device");
	esp_ble_gattc_open(gl_profile_tab[PROFILE_B_APP_ID].gattc_if, scan_rstb.scan_rst.bda, scan_rstb.scan_rst.ble_addr_type, true);
	} else if (!BleDevStC.btopen && BleDevStC.btopenreq) {
	if (fdebug) ESP_LOGI(AP_TAG, "Connect 3 to the remote device");
	esp_ble_gattc_open(gl_profile_tab[PROFILE_C_APP_ID].gattc_if, scan_rstc.scan_rst.bda, scan_rstc.scan_rst.ble_addr_type, true);
	} else if (!BleDevStD.btopen && BleDevStD.btopenreq) {
	if (fdebug) ESP_LOGI(AP_TAG, "Connect 4 to the remote device");
	esp_ble_gattc_open(gl_profile_tab[PROFILE_D_APP_ID].gattc_if, scan_rstd.scan_rst.bda, scan_rstd.scan_rst.ble_addr_type, true);
	} else if (!BleDevStE.btopen && BleDevStE.btopenreq) {
	if (fdebug) ESP_LOGI(AP_TAG, "Connect 5 to the remote device");
	esp_ble_gattc_open(gl_profile_tab[PROFILE_E_APP_ID].gattc_if, scan_rste.scan_rst.bda, scan_rste.scan_rst.ble_addr_type, true);
	} else {
	start_scan();
	}
	}
	break;

    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
	if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS){
	if (fdebug) ESP_LOGE(AP_TAG, "Adv stop failed, error status = 0x%X", param->adv_stop_cmpl.status);
            break;
	}
	if (fdebug) ESP_LOGI(AP_TAG, "Stop adv successfully");
        break;
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
 	if (fdebug) ESP_LOGI(AP_TAG, "update connection params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
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
//	if (fdebug) ESP_LOGI(AP_TAG, "EVT %d, gattc if %d, app_id %d", event, gattc_if, param->reg.app_id);

    /* If event is register event, store the gattc_if for each profile */
	if (event == ESP_GATTC_REG_EVT) {
	if (param->reg.status == ESP_GATT_OK) {
            gl_profile_tab[param->reg.app_id].gattc_if = gattc_if;
	} else {
	if (fdebug) ESP_LOGI(AP_TAG, "reg app failed, app_id %04x, status %d",
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
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
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
	uint8_t timeout = 200; 	// 200*10 = 2 seconds
	while (--timeout && ((ptr->notifyDataLen == -1) || (ptr->notifyData[0] != 3) || (ptr->notifyData[4] != 40))) {
	vTaskDelay(10 / portTICK_PERIOD_MS);
	}
	if ((ptr->notifyData[0] == 3) && (ptr->notifyData[1] == 2) && (ptr->notifyData[4] == 40)) return true;
	else {
	return false;
	}
	} else ptr->iRssi = 0;
	return false;
}

bool mMiOfft(uint8_t blenum) {
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
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
	uint8_t timeout = 200; 	// 200*10 = 2 seconds
	while (--timeout && ((ptr->notifyDataLen == -1) || (ptr->notifyData[0] != 3) || (ptr->notifyData[4] != 40))) {
	vTaskDelay(10 / portTICK_PERIOD_MS);
	}
	if ((ptr->notifyData[0] == 3) && (ptr->notifyData[1] == 2) && (ptr->notifyData[4] == 40)) return true;
	else {
	return false;
	}
	} else ptr->iRssi = 0;
	return false;
}

bool mMiBoil(uint8_t blenum) {
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
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
	uint8_t timeout = 200; 	// 200*10 = 2 seconds
	while (--timeout && ((ptr->notifyDataLen == -1) || (ptr->notifyData[0] != 1))) {
	vTaskDelay(10 / portTICK_PERIOD_MS);
	}
	if ((ptr->notifyData[0] == 1) && (ptr->notifyData[1] == 2)) return true;
	else {
	return false;
	}
	} else ptr->iRssi = 0;
	return true;
}

bool mMiHeat(uint8_t blenum, uint8_t temp) {
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
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
	uint8_t timeout = 200; 	// 200*10 = 2 seconds
	while (--timeout && ((ptr->notifyDataLen == -1) || (ptr->notifyData[0] != 3) || (ptr->notifyData[4] != temp))) {
	vTaskDelay(10 / portTICK_PERIOD_MS);
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
	uint8_t timeout = 200; 	// 200*10 = 2 seconds
	while (--timeout && ((ptr->notifyDataLen == -1) || (ptr->notifyData[0] != 3) || (ptr->notifyData[4] != ptr->bCtemp))) {
	vTaskDelay(10 / portTICK_PERIOD_MS);
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
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
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
	uint8_t timeout = 200; 	// 200*10 = 2 seconds
	while (--timeout && ((ptr->notifyDataLen == -1) || (ptr->notifyData[4] != temp))) {
	vTaskDelay(10 / portTICK_PERIOD_MS);
	}
	if (ptr->notifyData[4] == temp) return true;
	else return false;
	} else ptr->iRssi = 0;
	return true;
}

bool mMiRewarm(uint8_t blenum) {
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
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
	uint8_t timeout = 200; 	// 200*10 = 2 seconds
	while (--timeout && ((ptr->notifyDataLen == -1) || (ptr->notifyData[0] != 1))) {
	vTaskDelay(10 / portTICK_PERIOD_MS);
	}
	if (ptr->notifyData[0] != 1) retc = false;
	ptr->sendDataHandle = 4;  //setup
	ptr->sendData[0] = 1;
	ptr->sendData[1] = ptr->bHtemp;
	ptr->sendDataLen = 2;
	esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_A_APP_ID].remote_bda);
	ptr->notifyDataLen = -1;
	timeout = 200; 	// 200*10 = 2 seconds
	while (--timeout && ((ptr->notifyDataLen == -1) || (ptr->notifyData[0] != 3))) {
	vTaskDelay(10 / portTICK_PERIOD_MS);
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
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
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
	uint8_t timeout = 200; 	// 200*10 = 2 seconds
	while (--timeout && ((ptr->notifyDataLen == -1) || (ptr->notifyData[10] != 0x18))) {
	vTaskDelay(10 / portTICK_PERIOD_MS);
	}
	if (ptr->notifyData[10] != 0x18) retc = false;
	} else {
	ptr->iRssi = 0;
	retc = false;
	}
	return retc;
}

//******************** am43 *********************
uint8_t am43Write(uint8_t blenum, uint8_t cmd, uint8_t* data, size_t len) {
	uint8_t retc = 0;
	if (blenum > 4) return retc;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	ptr->sendData[0] = 0x00;
	ptr->sendData[1] = 0xff;
	ptr->sendData[2] = 0x00;
	ptr->sendData[3] = 0x00;
	ptr->sendData[4] = 0x9a;
	ptr->sendData[5] = cmd;
	ptr->sendData[6] = len;
	if (len > 0) {
	memcpy(&ptr->sendData[7], data, len);
		}
	for (int i = 0; i < (len + 7); i++) retc = retc ^ ptr->sendData[i];
	ptr->sendData[len + 7] = retc ^ 0xff;
	ptr->sendDataLen = len + 8;
	retc = 1;
//  ble_gap_read_rssi event return rssi and start sending data
	if (ptr->btauthoriz) esp_ble_gap_read_rssi(gl_profile_tab[blenum].remote_bda);
	else {
	ptr->iRssi = 0;
	retc = 0;
	}
	return retc;
}

int8_t am43Command(uint8_t blenum, uint8_t cmd, uint8_t* data, size_t len) {
	uint8_t blenum1 = blenum + 1;
	if (blenum > 4) return -1;
	uint8_t retc;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	ptr->readDataLen = -1;
	ptr->LstCmd = cmd;
	if (ptr->btauthoriz) {
	uint8_t timeout = 200; 	// 200*10 = 2 seconds
	retc = am43Write(blenum, cmd, data, len);
	while (--timeout && retc && (ptr->readDataLen == -1)) {
	vTaskDelay(10 / portTICK_PERIOD_MS);
	}
//
	if (retc && (ptr->readDataLen < BLE_INPUT_BUFFSIZE) && (ptr->readDataLen > 3) && (ptr->readData[0] == 0x9a) && (ptr->readData[1] == cmd)) {
	ptr->r4sConnErr = 0;	
	if (fdebug) {
	ESP_LOGI(AP_TAG, "Receive Data %d: ", blenum1);
	esp_log_buffer_hex(AP_TAG, ptr->readData, ptr->readDataLen);
	}
	} else {
	ptr->readDataLen = -1;
	ptr->r4sConnErr++;	
	}
	}
	ptr->LstCmd = 0;
	return ptr->readDataLen;
}

//******************** r4s **********************

//[I][R4S.cpp:24]   r4sWriteA(): >> 55 59 06 aa
//                         offset:  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19
//[I][R4S.cpp:43] r4sCommandA(): << 55 59 06 00 00 00 00 00 02 00 00 00 00 39 00 00 00 00 00 aa

uint8_t r4sWrite(uint8_t blenum, uint8_t cmd, uint8_t* data, size_t len) {
	if (blenum > 4) return -1;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
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
	if (blenum > 4) return -1;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	ptr->notifyDataLen = -1;
	if (ptr->btauthoriz) {
	uint8_t timeout = 200; 	// 200*10 = 2 seconds
	uint8_t cnt = r4sWrite(blenum, cmd, data, len);
	while (--timeout && (ptr->notifyDataLen == -1)) {
	vTaskDelay(10 / portTICK_PERIOD_MS);
	}
//
	if ((ptr->notifyDataLen < BLE_INPUT_BUFFSIZE) && (ptr->notifyDataLen > 1)) {
	ptr->r4sConnErr = 0;	
	if (fdebug) {
	ESP_LOGI(AP_TAG, "Receive Data %d: ", blenum1);
	esp_log_buffer_hex(AP_TAG, ptr->notifyData, ptr->notifyDataLen);
	}
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
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
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
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	if (r4sCommand(blenum, 0x03, 0, 0) != 5) return false;
	return ptr->notifyData[3] == 1;
}

bool m103sPon(uint8_t blenum, uint8_t pwr) {
	if (blenum > 4) return false;
	uint8_t pwrc = 100;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
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
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
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
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
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
bool m103sKon(uint8_t blenum) {
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	uint8_t data[] = { 1 };
	if (r4sCommand(blenum, 0x44, data, sizeof(data)) != 5)
	return false;
	return ptr->notifyData[3] == 1;
}

bool m103sKoff(uint8_t blenum) {
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	uint8_t data[] = { 0 };
	if (r4sCommand(blenum, 0x44, data, sizeof(data)) != 5)
	return false;
	return ptr->notifyData[3] == 1;
}
bool m103sTon(uint8_t blenum) {
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
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
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
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
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
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
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
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
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
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
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
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
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
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
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	if (ptr->DEV_TYP > 2)  return m171sOn(blenum, 2, temp);
	else return m171sOn(blenum, 0, temp);
}

bool m171s_NLOn(uint8_t blenum) {
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
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
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
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
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
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
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
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
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
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
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
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
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
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
	if ( ptr->DEV_TYP < 24 ) {
	uint8_t data[] = { prog, 0, 0, 0, 0, dhour, dmin, warm};
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
	} else if ( ptr->DEV_TYP == 24 ) {
	uint8_t data[] = { prog , 0};
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
	data[0] = warm;
	if (r4sCommand(blenum, 0x16, data, 1) != 5) return false;
	if (ptr->notifyData[3] != 1) return false;
	return true;
	} else if ( ptr->DEV_TYP == 48 ) {
	uint8_t data[] = { prog, phour, pmin, 0,  dhour, dmin, 0, warm,  0, 0, 0, 0,  0, 0, 0, 0};
	if (r4sCommand(blenum, 0x05, data, sizeof(data)) != 5) return false;
	if (ptr->notifyData[3] != 1) return false;
	return true;
	return true;
	} else return false;
}

bool rm800sProg(uint8_t blenum, uint8_t prog) {
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
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
	if ( ptr->DEV_TYP < 24 ) {
	uint8_t data[] = { prog, 0, 0, 0, 0, ptr->bDHour, ptr->bDMin, ptr->bAwarm};
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
	} else if (( ptr->DEV_TYP == 24 ) || ( ptr->DEV_TYP == 48 )) {
	uint8_t data[] = { prog };
	if (r4sCommand(blenum, 0x09, data, 1) != 5) return false;
	if (ptr->notifyData[3] != 1) return false;
	return true;
	} else return false;
}

bool rm800sMod(uint8_t blenum, uint8_t mod) {
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
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
	} else if ( ptr->DEV_TYP == 24 ) {
	data[0] = mod;
	if (r4sCommand(blenum, 0x0a, data, 1) != 5) return false;
	if (ptr->notifyData[3] != 1) return false;
	return true;
	} else if ( ptr->DEV_TYP == 48 ) return true;
	else return false;
}

bool rm800sTemp(uint8_t blenum, uint8_t temp) {
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
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
	} else if ( ptr->DEV_TYP == 24 ) {
	uint16_t ttemp = temp * 10;
	data[0] = ttemp & 0xff;
	data[1] = ((ttemp)>>8 & 0xff) | 0xf0;
	if (r4sCommand(blenum, 0x0b, data, 2) != 5) return false;
	if (ptr->notifyData[3] != 1) return false;
	return true;
	} else if ( ptr->DEV_TYP == 48 ) return true;
	else return false;
}

bool rm800sPhour(uint8_t blenum, uint8_t hour) {
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	if ( ptr->DEV_TYP < 48 ) {
	uint8_t data[] = { hour, ptr->bPMin};
	if (r4sCommand(blenum, 0x0c, data, sizeof(data)) != 5) return false;
	if (ptr->notifyData[3] != 1) return false;
	return true;
	} else if ( ptr->DEV_TYP == 48 ) {
	uint8_t data[] = { hour, ptr->bPMin, 0 };
	if (r4sCommand(blenum, 0x0c, data, sizeof(data)) != 5) return false;
	if (ptr->notifyData[3] != 1) return false;
	return true;
	} else return false;
}

bool rm800sPmin(uint8_t blenum, uint8_t min) {
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	if ( ptr->DEV_TYP < 48 ) {
	uint8_t data[] = { ptr->bPHour, min};
	if (r4sCommand(blenum, 0x0c, data, sizeof(data)) != 5) return false;
	if (ptr->notifyData[3] != 1) return false;
	return true;
	} else if ( ptr->DEV_TYP == 48 ) {
	uint8_t data[] = { ptr->bPHour, min, 0};
	if (r4sCommand(blenum, 0x0c, data, sizeof(data)) != 5) return false;
	if (ptr->notifyData[3] != 1) return false;
	return true;
	} else return false;
}

bool rm800sDhour(uint8_t blenum, uint8_t hour) {
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	uint8_t data[] = { hour, ptr->bDMin};
	if (r4sCommand(blenum, 0x14, data, sizeof(data)) != 5) return false;
	if (ptr->notifyData[3] != 1) return false;
	return true;
}

bool rm800sDmin(uint8_t blenum, uint8_t min) {
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	uint8_t data[] = { ptr->bDHour, min};
	if (r4sCommand(blenum, 0x14, data, sizeof(data)) != 5) return false;
	if (ptr->notifyData[3] != 1) return false;
	return true;
}

bool rm800sAwarm(uint8_t blenum, uint8_t warm) {
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	if ( ptr->DEV_TYP < 24 ) {
	uint8_t data[] = { ptr->bProg, ptr->bModProg, ptr->bHtemp, ptr->bPHour, ptr->bPMin, ptr->bDHour, ptr->bDMin, warm};
	if (r4sCommand(blenum, 0x05, data, sizeof(data)) != 5)
	return false;
	if (ptr->notifyData[3] != 1)
	return false;
	return true;
	} else if (( ptr->DEV_TYP == 24 ) || ( ptr->DEV_TYP == 48 )) {
	uint8_t data[] = { warm };
	if (r4sCommand(blenum, 0x16, data, 1) != 5) return false;
	if (ptr->notifyData[3] != 1) return false;
	return true;
	} else return false;
}

bool m51sCalibrate(uint8_t blenum) {
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	if (r4sCommand(blenum, 0x7b, 0, 0) != 5) return false;
	return ptr->notifyData[3] == 1;
}

bool m61sClear(uint8_t blenum) {
	return false;

	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	if (r4sCommand(blenum, 0x7b, 0, 0) != 5) return false;
	return ptr->notifyData[3] == 1;
}

bool mgl90sw(uint8_t blenum, uint8_t state, uint8_t  phour, uint8_t  pmin) {
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	ptr->sendDataLen = 0;
	if (ptr->btauthoriz) {
	ptr->sendDataHandle = 2;  //auth
	ptr->sendData[0] = 0xff;
	ptr->sendDataLen = 1;
	esp_ble_gap_read_rssi(gl_profile_tab[blenum].remote_bda);
	ptr->sendDataLen = 0;
	ptr->sendDataHandle = 0;  //txchar
	if (state) {
	ptr->sendData[0] = 0x00;
	if (phour || pmin) ptr->sendData[1] = 0x03;
	else ptr->sendData[1] = 0x01;
	ptr->sendData[2] = 0x00;
	ptr->sendData[3] = phour;
	ptr->sendData[4] = pmin;
	ptr->sendData[5] = 0x00;
	ptr->sendData[6] = 0x00;
	} else {
	ptr->sendData[0] = 0x01;
	ptr->sendData[1] = 0x00;
	ptr->sendData[2] = 0x00;
	ptr->sendData[3] = 0x00;
	ptr->sendData[4] = 0x00;
	ptr->sendData[5] = 0x00;
	ptr->sendData[6] = 0x00;
	}
	ptr->sendDataLen = 7;
	esp_ble_gap_read_rssi(gl_profile_tab[blenum].remote_bda);
	return true;
	} else ptr->iRssi = 0;
	return false;
}


bool mgl90st(uint8_t blenum) {
//	uint8_t blenum1 = blenum + 1;
	if (blenum > 4) return false;
	uint8_t timeout = 200; 	// 200*10 = 2 seconds
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	ptr->sendDataLen = 0;
	if (ptr->btauthoriz) {
	ptr->sendDataHandle = 2;  //auth
	ptr->sendData[0] = 0xff;
	ptr->sendDataLen = 1;
	esp_ble_gap_read_rssi(gl_profile_tab[blenum].remote_bda);
	ptr->sendDataLen = 0;
	ptr->sendDataHandle = 0;  //txchar
	ptr->readDataLen = -1;
	esp_ble_gap_read_rssi(gl_profile_tab[blenum].remote_bda);
	while (--timeout && (ptr->readDataLen == -1)) {
	vTaskDelay(10 / portTICK_PERIOD_MS);
	}
	if ((ptr->readDataLen < BLE_INPUT_BUFFSIZE) && (ptr->readDataLen > 1)) {
	ptr->r4sConnErr = 0;	
/*
	if (fdebug) {
	ESP_LOGI(AP_TAG, "Receive Data %d: ", blenum1);
	esp_log_buffer_hex(AP_TAG, ptr->readData, ptr->readDataLen);
	}
*/
	return true;
	} else return false;
	} else ptr->iRssi = 0;
	return false;
}


bool m43sPos(uint8_t blenum, uint8_t pos) {
	bool result = 0;
	if (blenum > 4) return result;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
        uint8_t data;
	if (pos == 254) {     //stop
	data = 0xcc;
	if ((am43Command(blenum, 0x0a, &data, 1) == 5) && (ptr->readData[3] == 0x5a)) result = 1;
	} else if (pos == 0) {       //close
	data = 0xee;
	if ((am43Command(blenum, 0x0a, &data, 1) == 5) && (ptr->readData[3] == 0x5a)) result = 1;
	} else if (pos == 100) {     //open
	data = 0xdd;
	if ((am43Command(blenum, 0x0a, &data, 1) == 5) && (ptr->readData[3] == 0x5a)) result = 1;
	} else if ((ptr->bModProg & 0x0c) == 0x0c) {              //set position if set limits
	if (ptr->bModProg & 0x01) {                               //if invert mode
	if (pos > 100) data = 0;	
	else data = 100 - pos;	
	} else {
	if (pos > 100) data = 100;	
	else data = pos;	
	}
	if ((am43Command(blenum, 0x0d, &data, 1) == 5) && (ptr->readData[3] == 0x5a)) result = 1;
	}
	return result;
}


bool mkSync(uint8_t blenum) {
	if (blenum > 4) return false;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
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
/*
	if (fdebug) {
	ESP_LOGI(AP_TAG, "Time %d: %ld,Timezone: %d", blenum, now, tz);
	esp_log_buffer_hex(AP_TAG, data, sizeof(data));
	}
*/
	if (!data[3]) return false; //if data correct?
	if (r4sCommand(blenum, 0x6e, data, sizeof(data)) != 5)
	return false;
	if (ptr->notifyData[3] != 0)
	return false;
	} else if (ptr->btauthoriz && (ptr->DEV_TYP == 73)) {
	uint8_t data[] = { 0, 0, 0, 0, 0, 0, 0, 0};
	struct tm timeinfo;
	time_t now;
	time(&now);
	localtime_r(&now, &timeinfo);
	data[0] = timeinfo.tm_year / 100 + 19;
	data[1] = timeinfo.tm_year % 100; 	
	data[2] = timeinfo.tm_mon + 1;
	data[3] = timeinfo.tm_mday;
	data[4] = timeinfo.tm_hour;
	data[5] = timeinfo.tm_min;
	data[6] = timeinfo.tm_sec;
	data[7] = TimeZone;
	if (data[0] < 20) return false; //if data correct?
	ptr->sendDataLen = 0;
	ptr->sendDataHandle = 2;  //auth
	ptr->sendData[0] = 0xff;
	ptr->sendDataLen = 1;
	esp_ble_gap_read_rssi(gl_profile_tab[blenum].remote_bda);
	memcpy(&ptr->sendData, &data[0], 8);
	ptr->sendDataLen = 8;
	ptr->sendDataHandle = 3;  //time
	esp_ble_gap_read_rssi(gl_profile_tab[blenum].remote_bda);
	} else if (ptr->btauthoriz && (ptr->DEV_TYP == 74)) {
	uint8_t data[] = { 0, 0, 0, 0};
	struct tm timeinfo;
	time_t now;
	time(&now);
	if (now < 0x1000000) return false; //if data correct?
	localtime_r(&now, &timeinfo);
	data[0] = timeinfo.tm_wday;
	data[1] = timeinfo.tm_hour;
	data[2] = timeinfo.tm_min;
	data[3] = timeinfo.tm_sec;
	if ((am43Command(blenum, 0x14, data, 4) != 5) || (ptr->readData[3] != 0x5a)) return false;
	} else return false;
	ptr->f_Sync = 0;
	return true;
}

//******************************************************************

void msStatus(uint8_t blenum) {
	if (blenum > 4) return;
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
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}

	if (ptr->btauthoriz && (ptr->DEV_TYP < 64)) {
	if (!ptr->sVer[0]) {
	char buff[4];
        retc = r4sCommand(blenum, 0x01, 0, 0);	
        if (retc == 6) {
	itoa(ptr->notifyData[3],ptr->sVer,10);
	strcat (ptr->sVer,".");
	if (ptr->notifyData[4] < 10) strcat (ptr->sVer,"0");
	itoa(ptr->notifyData[4],buff,10);
	strcat (ptr->sVer,buff);
	MqttPubSub(blenum, mqttConnected);	
	}
	if (!ptr->sVer[0]) ptr->sVer[0] = 0x20;
	}
        retc = r4sCommand(blenum, 0x06, 0, 0);	
	if ((ptr->notifyData[2] == 6) && (ptr->DEV_TYP == 1) && (retc == 13)) {
	ptr->bCtemp = ptr->notifyData[5];
	ptr->bState = ptr->notifyData[11];
	if (fkpmd && !ptr->bCtemp && ptr->bHeat && (ptr->notifyData[11] == 4)) ptr->bKeep  = 1;
	ptr->bHeat = ptr->notifyData[3];
	if (ptr->bState == 4) ptr->bState = 0;
	ptr->bProg = ptr->notifyData[3];
	if (ptr->bProg) ptr->bState = 0;
	ptr->bStNl = 0;
	ptr->bStBl = 0;
	ptr->bStBp = 0;
	if ((ptr->bState) && (ptr->bProg != 2)) ptr->bHtemp = 100;
	else {
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
	ptr->bState = ptr->notifyData[11];
	ptr->bProg = ptr->notifyData[3];
	if ((ptr->bState == 4) || (ptr->bProg && (ptr->bProg != 2))) ptr->bState = 0;
	if ((ptr->bState)  && (ptr->bProg != 2)) ptr->bHtemp = 100;
	else ptr->bHtemp = ptr->notifyData[5];
	if (ptr->DEV_TYP > 3) {
	ptr->bCtemp = ptr->notifyData[8];
	ptr->bHeat = ptr->notifyData[11];
	if ((ptr->bHeat == 4) || ((ptr->bProg != 1) && (ptr->bProg != 2))) ptr->bHeat = 0;
	if (ptr->bState && !ptr->bProg  && !ptr->bHeat && ptr->bStNl) ptr->bKeep = 3;
	ptr->bStNl = ptr->notifyData[11];
	if ((ptr->bStNl == 4) || (ptr->bProg != 3)) ptr->bStNl = 0;
	if ((ptr->bKeep == 3) && ptr->bHeat) ptr->bKeep = 0;
	ptr->bStBp = ptr->notifyData[7];
	ptr->bBlTime = ptr->notifyData[16] + 128;
	} else {
	ptr->bCtemp = ptr->notifyData[13];
	if (fkpmd && !ptr->bCtemp && ptr->bHeat && (ptr->notifyData[11] == 4)) ptr->bKeep  = 1;
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
	itoa(ptr->bHtemp,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"state\":");
	itoa(ptr->notifyData[11],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"prog\":");
	itoa(ptr->notifyData[3],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	if (fkpmd) {
	strcat(ptr->cStatus,",\"keep\":");
	itoa(ptr->bKeep,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	}
	strcat(ptr->cStatus,",\"beep\":");
	itoa(ptr->bStBp,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"error\":");
	if (ptr->DEV_TYP > 3) itoa(ptr->notifyData[18],tmpvar,10);
	else itoa(ptr->notifyData[12],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	if (ptr->DEV_TYP > 3) {
	strcat(ptr->cStatus,",\"boil\":");
	itoa(ptr->bBlTime,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"volume\":[");
	if (ptr->bCVol < 250) {
	if (!volperc) {
	itoa(ptr->bCVol / 10,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,".");	
	itoa(ptr->bCVol % 10,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	} else {
	itoa(ptr->bCVol,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	}
	} else if (ptr->bCVol == 253) strcat(ptr->cStatus,"\"???\"");
	else if (ptr->bC1temp && ptr->bS1Energy) strcat(ptr->cStatus,"\"??\"");
	else strcat(ptr->cStatus,"\"?\"");
	}
	strcat(ptr->cStatus,",");
	if (!volperc) {
	itoa(ptr->bCVoll / 10,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,".");	
	itoa(ptr->bCVoll % 10,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	} else {
	itoa(ptr->bCVoll,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	}
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
	if (!BleDevStA.r4slpcom && !BleDevStB.r4slpcom && !BleDevStC.r4slpcom && !BleDevStD.r4slpcom && !BleDevStE.r4slpcom && !ptr->r4slpres) {
	data[0] = 0x00;
	if (r4sCommand(blenum, 0x47, data, sizeof(data)) == 20) {
	ptr->bSEnergy = (ptr->notifyData[12] << 24) + (ptr->notifyData[11] << 16) + (ptr->notifyData[10] << 8) + ptr->notifyData[9];
	ptr->bSTime = (ptr->notifyData[8] << 24) + (ptr->notifyData[7] << 16) + (ptr->notifyData[6] << 8) + ptr->notifyData[5];
	if (ptr->bState || (ptr->bHeat && (ptr->bHtemp > 84))) {
	if (((ptr->bCtemp < 65) || (ptr->bCtemp > 89)) && (ptr->bC1temp || ptr->bS1Energy)) { 
        ptr->bS1Energy = 0;
        ptr->bC1temp = 0;
	} else if ((ptr->bCtemp > 64) && (ptr->bCtemp > ptr->bCStemp) && (ptr->bCtemp < 76) && !ptr->bC1temp && !ptr->bS1Energy) { 
	ptr->bS1Energy = ptr->bSEnergy;
	ptr->bC1temp = ptr->bCtemp;
	if (ptr->bCVol < 250) ptr->bCVol = 254;
	} else if ((ptr->bCtemp > 84) && ptr->bC1temp && ptr->bS1Energy) {
	ptr->bS1Energy = ptr->bSEnergy - ptr->bS1Energy;
	ptr->bC1temp = ptr->bCtemp - ptr->bC1temp;
	if (ptr->bCVol == 253) {     //1l calibration
	if (volperc) ptr->bCVol = (210 * ptr->bC1temp) / (ptr->bS1Energy);
	else ptr->bCVol = (350 * ptr->bC1temp) / (ptr->bS1Energy * 3);
	if ((ptr->bCVol > 50) && (ptr->bCVol < 100)) ptr->bEfficiency = ptr->bCVol;
	if (volperc) ptr->bCVol = 100;
	else ptr->bCVol = 10;
	ptr->bCVoll =ptr->bCVol;
	} else {
	if (volperc) {
	ptr->bCVol = (ptr->bS1Energy * 10 * ptr->bEfficiency) / (21 * ptr->bC1temp); //assume 18(1.8l) = 100% so if n=0.8 then L*10 * 100/18 = 1000/18*(En*3600*0.8)/(4200*dT); 	
	if ((ptr->bCVol > 100) && (ptr->bCVol < 120)) ptr->bCVol = 100;
	} else ptr->bCVol = (ptr->bS1Energy * 3 * ptr->bEfficiency) / (35 * ptr->bC1temp); //if n=0.8 then L*10 = 10*(En*3600*0.8)/(4200*dT); 	
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
	ptr->bModProg = ptr->notifyData[13];                //keep
	strcpy(ptr->cStatus,"{\"state\":");
	itoa(ptr->notifyData[11],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"lock\":");
	itoa(ptr->notifyData[10],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"keep\":");
	itoa(ptr->notifyData[13],tmpvar,10);
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
	itoa(ptr->notifyData[18],tmpvar,10);
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
	itoa(ptr->notifyData[18],tmpvar,10);
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
	if (!ptr->bState || (ptr->bState > 10)) ptr->bProg = 255; 
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
	if (!ptr->bState || (ptr->bState > 10)) ptr->bProg = 255; 
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
	if (!ptr->bState || (ptr->bState > 10)) ptr->bProg = 255; 
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
	} else if ((ptr->notifyData[2] == 6) && ( ptr->DEV_TYP == 48 ) && (retc == 20)) {
	ptr->bProg = ptr->notifyData[3];
	ptr->bModProg = 0;
	ptr->bPHour = ptr->notifyData[4];
	ptr->bPMin = ptr->notifyData[5];
//sec 6
	ptr->bHtemp = ptr->notifyData[6];
	ptr->bDHour = 0;
	ptr->bDMin = 0;
	ptr->bCHour = ptr->notifyData[7];
	ptr->bCMin = ptr->notifyData[8];
//sec 9
	ptr->bCtemp = ptr->notifyData[9];
	ptr->bHeat = ptr->notifyData[10];
	ptr->bState = ptr->notifyData[11];
	ptr->bAwarm = ptr->notifyData[12];
//	ptr->bLock = ptr->notifyData[16];                   //lock
        retc = r4sCommand(blenum, 0x15, 0, 0);	
	if (retc == 6) {
	ptr->bDHour = ptr->notifyData[3];
	ptr->bDMin = ptr->notifyData[4];
	}
	strcpy(ptr->cStatus,"{\"prog\":");
	itoa(ptr->bProg,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"sethours\":");
	itoa(ptr->bPHour,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"setmins\":");
	itoa(ptr->bPMin,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"setsecs\":");
	itoa(ptr->bModProg,tmpvar,10);
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
	strcat(ptr->cStatus,",\"secs\":");
	itoa(ptr->bHtemp,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"warm\":");
	itoa(ptr->bAwarm,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
/*
	strcat(ptr->cStatus,",\"lock\":");
	itoa(ptr->bLock,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
*/
	strcat(ptr->cStatus,"}");    
	} else if ((ptr->notifyData[2] == 6) && ( ptr->DEV_TYP == 61 ) && (retc == 20)) {
	ptr->bState = ptr->notifyData[11];
	if (ptr->bState == 7) ptr->bState = 0;     //state
	ptr->bLock = ptr->notifyData[10];          //lock
	ptr->bModProg = ptr->notifyData[5];        //position
	strcpy(ptr->cStatus,"{\"state\":");
	itoa(ptr->notifyData[11],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"lock\":");
	itoa(ptr->notifyData[10],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"prog\":");
	itoa(ptr->notifyData[3],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"position\":");
	itoa(ptr->notifyData[5],tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,"}");    
	} else if ((ptr->notifyData[2] == 6) && ( ptr->DEV_TYP == 62 ) && (retc == 20)) {
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
	ptr->bProg = ptr->notifyData[12];                   //smoke alarm
	ptr->bCtemp = ptr->notifyData[6];                   //battery
	strcpy(ptr->cStatus,"{\"temperature\":");
	if (ptr->bSEnergy & 0x80000000) strcat(ptr->cStatus,"-");
	itoa((ptr->bSEnergy & 0x7fffffff) / 10,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,".");
	itoa((ptr->bSEnergy & 0x7fffffff) % 10,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"smoke\":");
	itoa(ptr->bProg,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"battery\":");
	itoa(ptr->bCtemp,tmpvar,10);
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
	if (fkpmd && (ptr->DEV_TYP > 3) && (ptr->DEV_TYP < 10)) {
	if (ptr->bHeat && (ptr->bHeat != 254)) ptr->bKeep  = 1;
	else if (ptr->bStNl && (ptr->bStNl != 254)) ptr->bKeep  = 2;
	}
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
	if ((ptr->DEV_TYP > 15) && (ptr->DEV_TYP < 61)) ptr->bProg = 255;
	else ptr->bProg = 0;
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
	} else if (ptr->btauthoriz && (ptr->DEV_TYP > 63) && (ptr->DEV_TYP < 73)) {
	ptr->sendDataLen = 0;
	esp_ble_gap_read_rssi(gl_profile_tab[blenum].remote_bda);
	if (ptr->notifyDataLen == 12) {
	if (fdebug) {
	ESP_LOGI(AP_TAG, "Notify Data %d: ", blenum);
        esp_log_buffer_hex(AP_TAG, ptr->notifyData, ptr->notifyDataLen);
	}
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

	} else if (ptr->btauthoriz && (ptr->DEV_TYP == 73)) {
	if ((ptr->notifyDataLen == 7) && ((ptr->notifyData[0] & 0xfe)== 0x98)){
	if (fdebug) {
	ESP_LOGI(AP_TAG, "Notify Data %d: ", blenum + 1);
        esp_log_buffer_hex(AP_TAG, ptr->notifyData, ptr->notifyDataLen);
	}
	ptr->bState = ptr->notifyData[0] & 0x01;
	ptr->bCHour = ptr->notifyData[2];
	ptr->bCMin = ptr->notifyData[3];
	ptr->bDMin = ptr->notifyData[4];
	ptr->bCtemp = ptr->notifyData[5];
	strcpy(ptr->cStatus,"{\"state\":");
	itoa(ptr->bState,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"sethour\":");
	itoa(ptr->bPHour,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"setmin\":");
	itoa(ptr->bPMin,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"hour\":");
	itoa(ptr->bCHour,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"min\":");
	itoa(ptr->bCMin,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"sec\":");
	itoa(ptr->bDMin,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"battery\":");
	itoa(ptr->bCtemp,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,"}");    
	ptr->notifyDataLen = -1;
        ptr->notifyData[0] = 0;
	} else if (mgl90st(blenum) && (ptr->readDataLen == 7) && ((ptr->readData[0] & 0xfe)== 0x98)){
	if (fdebug) {
	ESP_LOGI(AP_TAG, "Read Data %d: ", blenum + 1);
        esp_log_buffer_hex(AP_TAG, ptr->readData, ptr->readDataLen);
	}
	ptr->bState = ptr->readData[0] & 0x01;
	ptr->bCHour = ptr->readData[2];
	ptr->bCMin = ptr->readData[3];
	ptr->bDMin = ptr->readData[4];
	ptr->bCtemp = ptr->readData[5];
	strcpy(ptr->cStatus,"{\"state\":");
	itoa(ptr->bState,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"sethour\":");
	itoa(ptr->bPHour,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"setmin\":");
	itoa(ptr->bPMin,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"hour\":");
	itoa(ptr->bCHour,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"min\":");
	itoa(ptr->bCMin,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"sec\":");
	itoa(ptr->bDMin,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"battery\":");
	itoa(ptr->bCtemp,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,"}");    
	ptr->notifyDataLen = -1;
        ptr->notifyData[0] = 0;
	} else {
	ptr->r4sConnErr++;
	if (ptr->r4sConnErr > 2) { 
	ptr->cStatus[0] = 0;
	if (FDHass || !foffln) {
	ptr->bState = 0;
	} else {
	ptr->bState = 254;
	}
	}
	}

	} else if (ptr->btauthoriz && (ptr->DEV_TYP == 74)) {
	uint8_t data;
	if ((ptr->bModProg & 0x80) && (ptr->notifyDataLen > 3) && (ptr->notifyData[0] == 0x9a)){
	if (fdebug) {
	ESP_LOGI(AP_TAG, "Notify Data %d: ", blenum + 1);
        esp_log_buffer_hex(AP_TAG, ptr->notifyData, ptr->notifyDataLen);
	}
	if (!memcmp(&ptr->notifyData[1],"\xa2\x05",2)) { //battery
	ptr->bCtemp = ptr->notifyData[7];
	data = 0x5a;
	am43Write(blenum, 0xa2, &data, 1); //ack
	} else if (!memcmp(&ptr->notifyData[1],"\xaa\x02",2)) {  //illuminance
        ptr->bSEnergy = ptr->notifyData[4] * 100 / 9;
	data = 0x5a;
	am43Write(blenum, 0xaa, &data, 1); //ack
	} else if (!memcmp(&ptr->notifyData[1],"\xa1\x04",2)) {  //position
	if (ptr->bModProg & 0x01) ptr->bProg = 100 - ptr->notifyData[4]; //if invert mode
	else ptr->bProg = ptr->notifyData[4];
	data = 0x5a;
	am43Write(blenum, 0xa1, &data, 1); //ack
	}
	strcpy(ptr->cStatus,"{\"position\":");
	itoa(ptr->bProg,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"illuminance\":");
	itoa(ptr->bSEnergy,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"battery\":");
	itoa(ptr->bCtemp,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,"}");    
	ptr->notifyDataLen = -1;
        ptr->notifyData[0] = 0;
	} else {
	ptr->bState = 0;
	data = 0x01;
	if (am43Command(blenum, 0xa7, &data, 1) == 11) { //position
        ptr->bModProg = ptr->readData[3] | 0x80;
	if (ptr->bModProg & 0x01) ptr->bProg = 100 - ptr->readData[5]; //if invert mode
	else ptr->bProg = ptr->readData[5];
	data = 0x5a;
	am43Write(blenum, 0xa7, &data, 1); //ack
	}
	data = 0x01;
	if (am43Command(blenum, 0xaa, &data, 1) == 6) { //illuminance
        ptr->bSEnergy = ptr->readData[4] * 100 / 9;
	data = 0x5a;
	am43Write(blenum, 0xaa, &data, 1); //ack
	}
	data = 0x01;
	if (am43Command(blenum, 0xa2, &data, 1) == 9) { //battery
	ptr->bCtemp = ptr->readData[7];
	data = 0x5a;
	am43Write(blenum, 0xa2, &data, 1); //ack
	}
	strcpy(ptr->cStatus,"{\"position\":");
	itoa(ptr->bProg,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"illuminance\":");
	itoa(ptr->bSEnergy,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,",\"battery\":");
	itoa(ptr->bCtemp,tmpvar,10);
	strcat(ptr->cStatus,tmpvar);
	strcat(ptr->cStatus,"}");    
	ptr->notifyDataLen = -1;
        ptr->notifyData[0] = 0;
	}
	} else {
	ptr->cStatus[0]=0;
	if (fkpmd && (ptr->DEV_TYP > 3) && (ptr->DEV_TYP < 10)) {
	if (ptr->bHeat && (ptr->bHeat != 254)) ptr->bKeep  = 1;
	else if (ptr->bStNl && (ptr->bStNl != 254)) ptr->bKeep  = 2;
	}
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
	if ((ptr->DEV_TYP > 15) && (ptr->DEV_TYP < 61)) ptr->bProg = 255;
	else ptr->bProg = 0;
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
	uint8_t tgtnum;
	char ldata[128];
	char tmpvar[64]; 
	char tmpvar1[32]; 
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

	if (ble_mon) for (int i = 0; i < BleMonNum; i++) {
	tgtnum = BleMX[i].gtnum;
	if  ((mqttConnected) && BleMR[i].sto && (BleMX[i].state != BleMX[i].prstate)) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	if ((BleMR[i].id > 0x40) && (BleMR[i].id < 0x50)) bin2hex(BleMR[i].mac,tmpvar,16,0); 
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
	if ((BleMR[i].id > 0x40) && (BleMR[i].id < 0x50)) bin2hex(BleMR[i].mac,tmpvar,16,0); 
	else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(ldata,tmpvar);
	strcat(ldata,"/rssi");
	itoa(BleMX[i].rssi,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	BleMX[i].prrssi = BleMX[i].rssi;
	}
	if (mqttConnected && BleMR[i].sto) {
	if (BleMX[i].gttmo) BleMX[i].gttmo--;
	int8_t trssi = BleMX[i].rssi;
	if (!trssi) trssi = -126;
	int8_t tcmrssi = BleMX[i].cmrssi;
	if (!tcmrssi) tcmrssi = -126;
	if (BleMX[i].gtnum == R4SNUM) {
	strcpy(ldata,"r4s/");
	if ((BleMR[i].id > 0x40) && (BleMR[i].id < 0x50)) bin2hex(BleMR[i].mac,tmpvar,16,0); 
	else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(ldata,tmpvar);
	strcat(ldata,"/rssi");
	itoa(BleMX[i].rssi,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	} else if ((trssi > tcmrssi) || !BleMX[i].gttmo || (BleMX[i].gtnum > 99)) {
	BleMX[i].gttmo = 5;
	strcpy(ldata,"r4s/");
	if ((BleMR[i].id > 0x40) && (BleMR[i].id < 0x50)) bin2hex(BleMR[i].mac,tmpvar,16,0); 
	else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(ldata,tmpvar);
	strcat(ldata,"/rssi");
	itoa(BleMX[i].rssi,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	strcpy(ldata,"r4s/");
	if ((BleMR[i].id > 0x40) && (BleMR[i].id < 0x50)) bin2hex(BleMR[i].mac,tmpvar,16,0); 
	else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(ldata,tmpvar);
	strcat(ldata,"/gtnum");
	itoa(R4SNUM,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	}
	}
	if (BleMR[i].id == 2) {
	if  ((mqttConnected) && BleMR[i].sto && (BleMX[i].par1 || (tgtnum > 99)) && (BleMX[i].par1 != BleMX[i].ppar1)) {
	uint16_t var1;
	strcpy(ldata,MQTT_BASE_TOPIC);           //weight
	strcat(ldata,"/");
	bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(ldata,tmpvar);
	strcat(ldata,"/weight");
	tmpvar[0] = 0;
	u32_strcat_p2 (BleMX[i].par1, tmpvar);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);

	strcpy(ldata,MQTT_BASE_TOPIC);           //date
	strcat(ldata,"/");
	bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(ldata,tmpvar);
	strcat(ldata,"/date");
	tmpvar[0] = 0;
	itoa(BleMX[i].par2,tmpvar,10);
	strcat(tmpvar,"/");
	var1 = BleMX[i].par3 & 0xff;
	if (var1 < 10) strcat (tmpvar,"0");
	itoa(var1,tmpvar1,10);
	strcat(tmpvar,tmpvar1);
	strcat(tmpvar,"/");
	var1 = (BleMX[i].par3 >> 8) & 0xff;
	if (var1 < 10) strcat (tmpvar,"0");
	itoa(var1,tmpvar1,10);
	strcat(tmpvar,tmpvar1);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);

	strcpy(ldata,MQTT_BASE_TOPIC);           //time
	strcat(ldata,"/");
	bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(ldata,tmpvar);
	strcat(ldata,"/time");
	tmpvar[0] = 0;
	var1 = BleMX[i].par4 & 0xff;
	if (var1 < 10) strcat (tmpvar,"0");
	itoa(var1,tmpvar1,10);
	strcat(tmpvar,tmpvar1);
	strcat(tmpvar,":");
	var1 = (BleMX[i].par4 >> 8) & 0xff;
	if (var1 < 10) strcat (tmpvar,"0");
	itoa(var1,tmpvar1,10);
	strcat(tmpvar,tmpvar1);
	strcat(tmpvar,":");
	var1 = BleMX[i].par5 & 0xff;
	if (var1 < 10) strcat (tmpvar,"0");
	itoa(var1,tmpvar1,10);
	strcat(tmpvar,tmpvar1);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	BleMX[i].ppar1 = BleMX[i].par1;
	BleMX[i].ppar2 = BleMX[i].par2;
	BleMX[i].ppar3 = BleMX[i].par3;
	BleMX[i].ppar4 = BleMX[i].par4;
	if (FDHass && ((BleMX[i].ppar5 ^ BleMX[i].par5) & 0x100)) {
	char llwtt[128];
	char llwtd[1024];
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
	strcat(llwtd,".weight\",\"icon\":\"mdi:scale\",\"uniq_id\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"_");
	bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"_weight\",\"device\":{\"identifiers\":[\"r4s_");
	bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"\"],\"name\":\"r4s.");
	bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"\",\"model\":\"");
	strcat(llwtd,"Mi Scale");
	strcat(llwtd,"\",\"manufacturer\":\"");
	strcat(llwtd,"Xiaomi");
	strcat(llwtd,"\",\"via_device\":\"ESP32_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\"},\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/");
	bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"/weight\",\"unit_of_meas\":\"");
	if (BleMX[i].par5 & 0x100) strcat(llwtd,"lbs");
	else strcat(llwtd,"kg");
	strcat(llwtd,"\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
//
	strcpy(llwtt,"homeassistant/sensor/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/5x");
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
	strcat(llwtd,".prov_weight\",\"icon\":\"mdi:scale\",\"uniq_id\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"_");
	bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"_prov_weight\",\"device\":{\"identifiers\":[\"r4s_");
	bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"\"],\"name\":\"r4s.");
	bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"\",\"model\":\"");
	strcat(llwtd,"Mi Scale");
	strcat(llwtd,"\",\"manufacturer\":\"");
	strcat(llwtd,"Xiaomi");
	strcat(llwtd,"\",\"via_device\":\"ESP32_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\"},\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/");
	bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"/prov_weight\",\"unit_of_meas\":\"");
	if (BleMX[i].par5 & 0x100) strcat(llwtd,"lbs");
	else strcat(llwtd,"kg");
	strcat(llwtd,"\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
//
	}
	BleMX[i].ppar5 = BleMX[i].par5;
	}
	if  ((mqttConnected) && BleMR[i].sto && (BleMX[i].par6 != BleMX[i].ppar6)) {
	strcpy(ldata,MQTT_BASE_TOPIC);           //weight
	strcat(ldata,"/");
	bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(ldata,tmpvar);
	strcat(ldata,"/prov_weight");
	tmpvar[0] = 0;
	u32_strcat_p2 (BleMX[i].par6, tmpvar);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	BleMX[i].ppar6 = BleMX[i].par6;
	}
//
	if  ((mqttConnected) && BleMR[i].sto && (BleMX[i].par7 != BleMX[i].ppar7)) {
	strcpy(ldata,MQTT_BASE_TOPIC);           //impedance
	strcat(ldata,"/");
	bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(ldata,tmpvar);
	strcat(ldata,"/impedance");
	itoa(BleMX[i].par7,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	BleMX[i].ppar7 = BleMX[i].par7;
	}
	} else if (BleMR[i].id == 3) {
	if  ((mqttConnected) && BleMR[i].sto && (BleMX[i].par1 != BleMX[i].ppar1)) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(ldata,tmpvar);
	strcat(ldata,"/temperature");
	tmpvar[0] = 0;
	s16_strcat_p2 (BleMX[i].par1, tmpvar);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	BleMX[i].ppar1 = BleMX[i].par1;
	}
	if  ((mqttConnected) && BleMR[i].sto && (BleMX[i].par2 != BleMX[i].ppar2)) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(ldata,tmpvar);
	strcat(ldata,"/humidity");
	tmpvar[0] = 0;
	u32_strcat_p2 (BleMX[i].par2, tmpvar);
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

	} else if (BleMR[i].id == 0x44) {
	if  ((mqttConnected) && BleMR[i].sto && (BleMX[i].par1 != BleMX[i].ppar1)) {
	uint8_t tgst = BleMX[i].par1;
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	bin2hex(BleMR[i].mac,tmpvar,16,0);
	strcat(ldata,tmpvar);
	strcat(ldata,"/tgstate");
	bin2hex(&tgst,tmpvar,1,0);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	BleMX[i].ppar1 = BleMX[i].par1;
	}
	if  ((mqttConnected) && BleMR[i].sto && (BleMX[i].par2 != BleMX[i].ppar2)) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	bin2hex(BleMR[i].mac,tmpvar,16,0);
	strcat(ldata,tmpvar);
	strcat(ldata,"/battery");
	switch (BleMX[i].par2 & 0x03) {
	case 3:
	strcpy(tmpvar,"100");
	break;
	case 2:
	strcpy(tmpvar,"15");
	break;
	case 1:
	strcpy(tmpvar,"5");
	break;
	case 0:
	strcpy(tmpvar,"0");
	break;
	}
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	BleMX[i].ppar2 = BleMX[i].par2;
	}

	}
	}

	if  ((mqttConnected) && (bStateS != bprevStateS)) {
#ifdef USE_TFT
	uint8_t duty = bStateS;
	if (tft_conn && PIN_NUM_BCKL) {
	if (PIN_NUM_BCKL < MxPOutP) {	
	if ((f_i2cdev & 0x60000000) && !(pwr_batmode & 0x06)) duty >>= 4;
	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
	} else {
	if ((f_i2cdev & 0x60000000) && !(pwr_batmode & 0x06)) duty >>= 2;
	if (bStateS) i2c_axpin_set (&f_i2cdev, PIN_NUM_BCKL, duty | 1);
	else i2c_axpin_set (&f_i2cdev, PIN_NUM_BCKL, 0);
	}
	}
#endif
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/screen");
	itoa(bStateS,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcoms = 30;
	bprevStateS = bStateS;
	}

	if  ((mqttConnected) && (bgpio1 > 63) && (bgpio1 < 192) && fgpio1) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/gpio1");
	if (!lvgpio1) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
	else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcoms = 30;
	fgpio1 = 0;
	}
	if  ((mqttConnected) && (bgpio2 > 63) && (bgpio2 < 192) && fgpio2) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/gpio2");
	if (!lvgpio2) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
	else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcoms = 30;
	fgpio2 = 0;
	}
	if  ((mqttConnected) && (bgpio3 > 63) && (bgpio3 < 192) && fgpio3) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/gpio3");
	if (!lvgpio3) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
	else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcoms = 30;
	fgpio3 = 0;
	}
	if  ((mqttConnected) && (bgpio4 > 63) && (bgpio4 < 192) && fgpio4) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/gpio4");
	if (!lvgpio4) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
	else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcoms = 30;
	fgpio4 = 0;
	}
	if  ((mqttConnected) && (bgpio5 > 63) && (bgpio5 < 192) && fgpio5) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/gpio5");
	if (!lvgpio5) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
	else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	r4sppcoms = 30;
	fgpio5 = 0;
	}
	if  ((mqttConnected) && (bStatHx6 != bprevStatHx6) && (bgpio6 > 127) && (bgpio6 < 192) && (bgpio5 > 191)) {
	uint32_t var;
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/Hx6");
	tmpvar[0] = 0;
	if (bStatHx6 & 0xff) strcpy (tmpvar,"-0");
	else {
	var = bStatHx6 - bZeroHx6;
	if (var & 0x80000000) {
	var = (var ^ 0xffffffff) + 1;
	strcpy (tmpvar,"-");
	}
	if (bDivHx6 & 0x7fffffff) var = var / (bDivHx6 & 0x7fffffff);
//ESP_LOGI(AP_TAG, "Hx pub: st: %d, zr: %d, diff: %d, div: %d, var: %d", bStatHx6, bZeroHx6, (bStatHx6 - bZeroHx6),bDivHx6, var);
	if(!var) tmpvar[0] = 0;
	if (bDivHx6 & 0x80000000) u32_strcat_p1(var,tmpvar);
	else u32_strcat_p3(var,tmpvar);
	}

	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevStatHx6 = bStatHx6;
	}
	if  ((mqttConnected) && (bStatG6 != bprevStatG6) && (bgpio6 > 63)) {
	if (bgpio6 < 128) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/gpio6");
	itoa(bStatG6,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcoms = 30;
	} else if (bgpio6 < 192) {
	if (bgpio5 < 192) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/1w6");
	tmpvar[0] = 0;
	s18b20_strcat (bStatG6, tmpvar);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	}
	} else if (bgpio6 < (MxPOutP + 192)) {
#ifdef USE_IRTX
	r4sppcoms = 30;
	if ((bStatG6 ^ bprevStatG6) & 0xff00) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/ir6tx");
	tmpvar[0] = 0;
	switch ((bStatG6 >> 8) & 0xff) {
	case 0x01:
	strcpy(tmpvar,"NEC");
	break;
	case 0x02:
	strcpy(tmpvar,"NECx16");
	break;
	case 0x03:
	strcpy(tmpvar,"RC5");
	break;
	case 0x04:
	strcpy(tmpvar,"RC6");
	break;
	case 0x05:
	strcpy(tmpvar,"SAMSUNG");
	break;
	case 0x06:
	strcpy(tmpvar,"SIRCx12");
	break;
	case 0x07:
	strcpy(tmpvar,"SIRCx15");
	break;
	case 0x08:
	strcpy(tmpvar,"SIRCx20");
	break;
	case 0x09:
	strcpy(tmpvar,"PANASONIC");
	break;
	default:
        strcpy(tmpvar,"OFF");
	break;
	}
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	}
	if ((bStatG6 ^ bprevStatG6) & 0xff) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/ir6cmd");
	itoa((bStatG6 & 0xff),tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	}
#else
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/dht6t");
	tmpvar[0] = 0;
	sdht22strcat(bStatG6, tmpvar);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
#endif
	}
	bprevStatG6 = bStatG6;
	}
	if  ((mqttConnected) && (bStatG7 != bprevStatG7) && (bgpio7 > 63)) {
	if (bgpio7 < 128) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/gpio7");
	itoa(bStatG7,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcoms = 30;
	} else if (bgpio7 < 192) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/1w7");
	tmpvar[0] = 0;
	s18b20_strcat (bStatG7, tmpvar);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	} else if (bgpio7 < (MxPOutP + 192)) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/dht7t");
	tmpvar[0] = 0;
	sdht22strcat(bStatG7, tmpvar);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	}
	bprevStatG7 = bStatG7;
	}
	if  ((mqttConnected) && (bStatG8 != bprevStatG8) && (bgpio8 > 63)) {
	if (bgpio8 < 128) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/gpio8");
	itoa(bStatG8,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	r4sppcoms = 30;
	} else if (bgpio8 < 192) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/1w8");
	tmpvar[0] = 0;
	s18b20_strcat (bStatG8, tmpvar);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	} else if (bgpio8 < (MxPOutP + 192)) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/dht8t");
	tmpvar[0] = 0;
	sdht22strcat(bStatG8, tmpvar);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	}
	bprevStatG8 = bStatG8;
	}
	if  ((mqttConnected) && (bStatG6h != bprevStatG6h) && (bgpio6 > 191) && (bgpio6 < (MxPOutP + 192))) {
#ifdef USE_IRTX
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/ir6addr");
	itoa(bStatG6h,tmpvar,10);
	r4sppcoms = 30;
#else
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/dht6h");
	tmpvar[0] = 0;
	u32_strcat_p1 (bStatG6h,tmpvar);
#endif
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevStatG6h = bStatG6h;
	}
	if  ((mqttConnected) && (bStatG7h != bprevStatG7h) && (bgpio7 > 191) && (bgpio7 < (MxPOutP + 192))) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/dht7h");
	tmpvar[0] = 0;
	u32_strcat_p1 (bStatG7h,tmpvar);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevStatG7h = bStatG7h;
	}
	if  ((mqttConnected) && (bStatG8h != bprevStatG8h) && (bgpio8 > 191) && (bgpio8 < (MxPOutP + 192))) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/dht8h");
	tmpvar[0] = 0;
	u32_strcat_p1 (bStatG8h,tmpvar);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	bprevStatG8h = bStatG8h;
	}

	if (f_i2cdev & 0x80000000) {
//i2c
	for (int i = 0; i < 28; i++) {
	if (f_i2cdev & (1 << i)) {
	if (i2c_bits[i] & 0x01) {
//i2c temp
	if ((mqttConnected) && (SnPi2c[i].ppar1 != SnPi2c[i].par1)) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/i2c");
	bin2hex(&i2c_addr[i],tmpvar,1,0);
	strcat(ldata,tmpvar);
	strcat(ldata,"temp");
	tmpvar[0] = 0;
	sbme280_strcat (SnPi2c[i].par1, tmpvar);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	SnPi2c[i].ppar1 = SnPi2c[i].par1;
	}
	}
	if (i2c_bits[i] & 0x02) {
//i2c humid
	if ((mqttConnected) && (SnPi2c[i].ppar2 != SnPi2c[i].par2)) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/i2c");
	bin2hex(&i2c_addr[i],tmpvar,1,0);
	strcat(ldata,tmpvar);
	strcat(ldata,"humid");
	tmpvar[0] = 0;
	u32_strcat_p1 (SnPi2c[i].par2,tmpvar);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	SnPi2c[i].ppar2 = SnPi2c[i].par2;
	}
	}
	if (i2c_bits[i] & 0x04) {
//i2c press
	if ((mqttConnected) && (SnPi2c[i].ppar3 != SnPi2c[i].par3)) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/i2c");
	bin2hex(&i2c_addr[i],tmpvar,1,0);
	strcat(ldata,tmpvar);
	strcat(ldata,"pressp");
	tmpvar[0] = 0;
	u32_strcat_p1 (SnPi2c[i].par3,tmpvar);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/i2c");
	bin2hex(&i2c_addr[i],tmpvar,1,0);
	strcat(ldata,tmpvar);
	strcat(ldata,"pressm");
	tmpvar[0] = 0;
	u32_strcat_p1 ((SnPi2c[i].par3 * 3) >> 2,tmpvar);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	SnPi2c[i].ppar3 = SnPi2c[i].par3;
	}
	}
	if (i2c_bits[i] & 0x08) {
//i2c gas resistance
	if ((mqttConnected) && (SnPi2c[i].ppar4 != SnPi2c[i].par4)) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/i2c");
	bin2hex(&i2c_addr[i],tmpvar,1,0);
	strcat(ldata,tmpvar);
	strcat(ldata,"gsresist");
	if (SnPi2c[i].par4 == 0xffffffff) strcpy(tmpvar,"-0");
	else itoa(SnPi2c[i].par4,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	SnPi2c[i].ppar4 = SnPi2c[i].par4;
	}
	}
//
	} //bit i
	} //for i
	if  (f_i2cdev & 0x60000000) {
	if  ((mqttConnected) && (pwr_batprevlevp != pwr_batlevp)) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	if (f_i2cdev & 0x40000000) strcat(ldata,"/i2c75battery");
	else if (f_i2cdev & 0x20000000) strcat(ldata,"/i2c34battery");
	itoa(pwr_batlevp,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	pwr_batprevlevp = pwr_batlevp;
	}
	if  ((mqttConnected) && (pwr_batprevmode != pwr_batmode)) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	if (f_i2cdev & 0x40000000) strcat(ldata,"/i2c75batmode");
	else if (f_i2cdev & 0x20000000) strcat(ldata,"/i2c34batmode");
	if (pwr_batmode & 0x04) strcpy(tmpvar,"Disconnected");
	else if (!(pwr_batmode & 0x02)) strcpy(tmpvar,"Discharging");
	else if (!(pwr_batmode & 0x01)) strcpy(tmpvar,"Charging");
	else strcpy(tmpvar,"Charged");
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	pwr_batprevmode = pwr_batmode;
	}
	if  (f_i2cdev & 0x20000000) {
	if  ((mqttConnected) && (pwr_batprevlevv != pwr_batlevv)) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/i2c34batvoltage");
	tmpvar[0]= 0;
	u32_strcat_p3(pwr_batlevv,tmpvar);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	pwr_batprevlevv = pwr_batlevv;
	}
	if  ((mqttConnected) && (pwr_batprevlevc != pwr_batlevc)) {
	uint16_t var = pwr_batlevc;
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/i2c34batcurrent");
	tmpvar[0]= 0;
	if (var & 0x8000) {
	var = (var ^ 0x0ffff) + 1;  
	strcat(tmpvar,"-");
	}
	u32_strcat_p3(var,tmpvar);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	pwr_batprevlevc = pwr_batlevc;
	}

	}
	}
	}



}

void MqState(uint8_t blenum) {
	if (mqtdel) return;
	if (blenum > 4) return;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	char ldata[64];
	char tmpvar[32]; 
	char tmpvar1[16]; 

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
	if ((ptr->DEV_TYP < 10) || ((ptr->DEV_TYP > 63) && (ptr->DEV_TYP < 73))) {
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
	if  ((ptr->bprevState != ptr->bState) || (ptr->bprevHeat != ptr->bHeat) || (ptr->bprevStNl != ptr->bStNl) || (ptr->bprevHtemp != ptr->bHtemp)) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/hstate");
	if (ptr->bState && (ptr->bState != 254)) esp_mqtt_client_publish(mqttclient, ldata, "auto", 0, 1, 1);
	else if (ptr->bHeat && (ptr->bHeat != 254)) esp_mqtt_client_publish(mqttclient, ldata, "heat", 0, 1, 1);
	else if (ptr->bStNl && (ptr->bStNl != 254)) esp_mqtt_client_publish(mqttclient, ldata, "cool", 0, 1, 1);
	else esp_mqtt_client_publish(mqttclient, ldata, "off", 0, 1, 1);
	}
	if  (ptr->bprevHtemp != ptr->bHtemp) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/heat_temp");
	itoa(ptr->bHtemp,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->r4sppcom = 30;
	if ((ptr->bHtemp > 34) && (ptr->bHtemp < 100)) ptr->bLtemp = ptr->bHtemp;
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
	ptr->r4sppcom = 30;
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
	ptr->r4sppcom = 30;
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
	ptr->r4sppcom = 30;
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
	ptr->r4sppcom = 30;
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
	ptr->r4sppcom = 30;
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
	ptr->r4sppcom = 30;
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
	ptr->r4sppcom = 30;
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
	ptr->r4sppcom = 30;
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
	ptr->r4sppcom = 30;
	ptr->bprevStBp = ptr->bStBp;
	}
	if  (ptr->bSEnergy != ptr->bprevSEnergy) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/total_energy");
	tmpvar[0] = 0;
	u32_strcat_p3(ptr->bSEnergy,tmpvar);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->r4sppcom = 30;
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
	ptr->r4sppcom = 30;
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
	ptr->r4sppcom = 30;
	ptr->bprevSCount = ptr->bSCount;
	}
	if  (ptr->bCVol != ptr->bprevCVol) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/volume");
	if (ptr->bCVol < 250) {
	if (!volperc) {
	itoa(ptr->bCVol / 10,tmpvar1,10);
	strcpy(tmpvar,tmpvar1);
	strcat(tmpvar,".");	
	itoa(ptr->bCVol % 10,tmpvar1,10);
	strcat(tmpvar,tmpvar1);
	} else 	itoa(ptr->bCVol,tmpvar,10);
	} else {
	if (!volperc) strcpy(tmpvar,"-0.0");
	else strcpy(tmpvar,"-0");
	}
	if (ptr->DEV_TYP > 3) esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->r4sppcom = 30;
	ptr->bprevCVol = ptr->bCVol;
	}
	if  (ptr->bCVoll != ptr->bprevCVoll) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/volume_last");
	if (!volperc) {
	itoa(ptr->bCVoll / 10,tmpvar1,10);
	strcpy(tmpvar,tmpvar1);
	strcat(tmpvar,".");	
	itoa(ptr->bCVoll % 10,tmpvar1,10);
	strcat(tmpvar,tmpvar1);
	} else 	itoa(ptr->bCVoll,tmpvar,10);
	if (ptr->DEV_TYP > 3) esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->r4sppcom = 30;
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
	ptr->r4sppcom = 30;
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
	ptr->r4sppcom = 30;
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
	ptr->r4sppcom = 30;
	ptr->bprevLock = ptr->bLock;
	}
	if  (ptr->bprevModProg != ptr->bModProg) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/keep");
	if (!ptr->bModProg) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
	else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	ptr->r4sppcom = 30;
	ptr->bprevModProg = ptr->bModProg;
	}
	if  ((ptr->DEV_TYP == 11) && (ptr->bprevHtemp != ptr->bHtemp)) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/power");
	itoa(ptr->bHtemp,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->r4sppcom = 30;
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
	ptr->r4sppcom = 30;
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
	ptr->r4sppcom = 30;
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
	ptr->r4sppcom = 30;
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
	ptr->r4sppcom = 30;
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
	} else if (ptr->DEV_TYP < 61) {
	if  (ptr->bprevState != ptr->bState) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/state");
	if (ptr->DEV_TYP == 16) {
	if (!ptr->bState) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
	else if (ptr->bState == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
	else if (ptr->bState == 1) esp_mqtt_client_publish(mqttclient, ldata, "SETTING", 0, 1, 1);
	else if (ptr->bState == 2) esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	else if (ptr->bState == 3) esp_mqtt_client_publish(mqttclient, ldata, "WAIT_PRODUCT", 0, 1, 1);
	else if (ptr->bState == 4) esp_mqtt_client_publish(mqttclient, ldata, "WARMING", 0, 1, 1);
	else if (ptr->bState == 5) esp_mqtt_client_publish(mqttclient, ldata, "DELAYED_START", 0, 1, 1);
	else if (ptr->bState == 6) esp_mqtt_client_publish(mqttclient, ldata, "PREHEATING", 0, 1, 1);
	} else if (ptr->DEV_TYP == 48) {
	if (!ptr->bState) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
	else if (ptr->bState == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
	else if (ptr->bState == 1) esp_mqtt_client_publish(mqttclient, ldata, "UPDATE", 0, 1, 1);
	else if (ptr->bState == 2) esp_mqtt_client_publish(mqttclient, ldata, "DELAYED_START", 0, 1, 1);
	else if (ptr->bState == 3) esp_mqtt_client_publish(mqttclient, ldata, "PREHEATING", 0, 1, 1);
	else if (ptr->bState == 4) esp_mqtt_client_publish(mqttclient, ldata, "WAIT_PRODUCT", 0, 1, 1);
	else if (ptr->bState == 5) esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	else if (ptr->bState == 6) esp_mqtt_client_publish(mqttclient, ldata, "WARMING", 0, 1, 1);
	else if (ptr->bState == 255) esp_mqtt_client_publish(mqttclient, ldata, "ERROR", 0, 1, 1);
	} else {
	if (!ptr->bState) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
	else if (ptr->bState == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
	else if (ptr->bState == 1) esp_mqtt_client_publish(mqttclient, ldata, "SETTING", 0, 1, 1);
	else if (ptr->bState == 2) esp_mqtt_client_publish(mqttclient, ldata, "DELAYED_START", 0, 1, 1);
	else if (ptr->bState == 3) esp_mqtt_client_publish(mqttclient, ldata, "PREHEATING", 0, 1, 1);
	else if (ptr->bState == 4) esp_mqtt_client_publish(mqttclient, ldata, "WAIT_PRODUCT", 0, 1, 1);
	else if (ptr->bState == 5) esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	else if (ptr->bState == 6) esp_mqtt_client_publish(mqttclient, ldata, "WARMING", 0, 1, 1);
	else if (ptr->bState == 7) esp_mqtt_client_publish(mqttclient, ldata, "ERROR", 0, 1, 1);
	else if (ptr->bState == 8) esp_mqtt_client_publish(mqttclient, ldata, "WAIT_CONFIRM", 0, 1, 1);
	else if (ptr->bState == 9) esp_mqtt_client_publish(mqttclient, ldata, "STOP_POWER_LOSS", 0, 1, 1);
	}
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/hstate");
	if (ptr->bState < 2) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
	else if (ptr->bState == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
	else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	ptr->r4sppcom = 30;
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
	} else if ( ptr->DEV_TYP == 48 ) {
// for RMB-658
	switch (ptr->bProg) {
	case 255:
	esp_mqtt_client_publish(mqttclient, ldata, "OFF", 0, 1, 1);
	break;
	case 0:
	esp_mqtt_client_publish(mqttclient, ldata, "Manual", 0, 1, 1);
	break;
	case 1:
	esp_mqtt_client_publish(mqttclient, ldata, "Fry", 0, 1, 1);
	break;
	case 2:
	esp_mqtt_client_publish(mqttclient, ldata, "Heating", 0, 1, 1);
	break;
	}
	}
	ptr->r4sppcom = 30;
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
	ptr->r4sppcom = 30;
	ptr->bprevProg = ptr->bProg;
	}
	if (ptr->DEV_TYP != 48) {
	if  (ptr->bprevModProg != ptr->bModProg) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/mode");
	itoa(ptr->bModProg,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->r4sppcom = 30;
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
	ptr->r4sppcom = 30;
	ptr->bprevHtemp = ptr->bHtemp;
	}
	}
	if  (ptr->bprevPHour != ptr->bPHour) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/set_hour");
	itoa(ptr->bPHour,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->r4sppcom = 30;
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
	ptr->r4sppcom = 30;
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
	ptr->r4sppcom = 30;
	ptr->bprevAwarm = ptr->bAwarm;
	}

	} else if (ptr->DEV_TYP == 61) {
	if  (ptr->bprevState != ptr->bState) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/state");
	if (!ptr->bState) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
	else if (ptr->bState == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
	else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	ptr->r4sppcom = 30;
	ptr->bprevState = ptr->bState;
	}
	if  (ptr->bprevLock != ptr->bLock) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/lock");
	if (!ptr->bLock) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
	else if (ptr->bLock == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
	else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	ptr->r4sppcom = 30;
	ptr->bprevLock = ptr->bLock;
	}
	if  (ptr->bprevModProg != ptr->bModProg) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/position");
	switch (ptr->bModProg) {
	case 0:
	esp_mqtt_client_publish(mqttclient, ldata, "Work", 0, 1, 1);  //work
	break;
	case 1:
	esp_mqtt_client_publish(mqttclient, ldata, "Stand", 0, 1, 1);   //base
	break;
	case 2:
	esp_mqtt_client_publish(mqttclient, ldata, "Offside", 0, 1, 1); //
	break;
	case 3:
	esp_mqtt_client_publish(mqttclient, ldata, "Upside", 0, 1, 1);  //up
	break;
	case 254:
	esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
	break;
	}
	ptr->bprevModProg = ptr->bModProg;
	}
	} else if (ptr->DEV_TYP == 62) {
/*
	if  (ptr->bprevState != ptr->bState) {
	if (ptr->r4slpcom != 62) ptr->bState = 0;
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/clear");
	if (!ptr->bState) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
	else if (ptr->bState == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
	else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	ptr->r4sppcom = 30;
	ptr->bprevState = ptr->bState;
	}
*/
	if  (ptr->bprevProg != ptr->bProg) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/smoke");
	if (!ptr->bProg) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
	else if (ptr->bProg == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
	else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	ptr->r4sppcom = 30;
	ptr->bprevProg = ptr->bProg;
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
	ptr->r4sppcom = 30;
	ptr->bprevSEnergy = ptr->bSEnergy;
	}
	if  (ptr->bprevCtemp != ptr->bCtemp) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/battery");
	itoa(ptr->bCtemp,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->bprevCtemp = ptr->bCtemp;
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
	ptr->r4sppcom = 30;
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
	ptr->r4sppcom = 30;
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
	ptr->r4sppcom = 30;
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
	ptr->r4sppcom = 30;
	ptr->bprevSCount = ptr->bSCount;
	}

	} else if (ptr->DEV_TYP == 73) {
	if  (ptr->bprevState != ptr->bState) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/state");
	if (!ptr->bState) esp_mqtt_client_publish(mqttclient, ldata, strOFF, 0, 1, 1);
	else if (ptr->bState == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 1);
	else esp_mqtt_client_publish(mqttclient, ldata, strON, 0, 1, 1);
	ptr->r4sppcom = 30;
	ptr->bprevState = ptr->bState;
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
	if  (ptr->bprevDMin != ptr->bDMin) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/sec");
	itoa(ptr->bDMin,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->bprevDMin = ptr->bDMin;
	}
	if  (ptr->bprevCtemp != ptr->bCtemp) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/battery");
	itoa(ptr->bCtemp,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->bprevCtemp = ptr->bCtemp;
	}
	} else if (ptr->DEV_TYP == 74) {
	if  (ptr->bprevProg != ptr->bProg) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/position");
	itoa(ptr->bProg,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->bprevProg = ptr->bProg;
	}
	if  (ptr->bprevSEnergy != ptr->bSEnergy) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/illuminance");
	itoa(ptr->bSEnergy,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->bprevSEnergy = ptr->bSEnergy;
	}
	if  (ptr->bprevCtemp != ptr->bCtemp) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,ptr->tBLEAddr);
	if (!fcommtp) strcat(ldata,"/rsp");
	strcat(ldata,"/battery");
	itoa(ptr->bCtemp,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 1);
	ptr->bprevCtemp = ptr->bCtemp;
	}
	}
}
}


void BleMqtPr(uint8_t blenum, int topoff, char *topic, int topic_len, char *data, int data_len) {
	if ((blenum > 4) || (data_len > 63)) return;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
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
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
	if ((!incascmp("1",data,data_len)) || (!incascmp("on",data,data_len))
		|| (!incascmp("true",data,data_len))) {
	if ((!ptr->bState) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strON,data,data_len))) {
	if (ptr->bHtemp) {
	if (ptr->DEV_TYP == 1) {	
	if (ptr->bHtemp < 35) ptr->r4slppar1 = 0;
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
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_BOIL_ON");
	} else if ((!incascmp("0",data,data_len)) || (!incascmp("off",data,data_len))
		|| (!incascmp("false",data,data_len))) {
	if ((ptr->bState)  || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strOFF,data,data_len))) {
	if (ptr->bHtemp) {
	if (ptr->DEV_TYP == 1) {	
	if (ptr->bHtemp < 35) ptr->r4slppar1 = 0;
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
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_BOIL_OFF");
		} 

	} else if (!memcmp(topic+topoff, "heat", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
	if ((!incascmp("1",data,data_len)) || (!incascmp("on",data,data_len))
		|| (!incascmp("true",data,data_len)) || (!incascmp("heat",data,data_len))) {
	if ((!ptr->bHeat) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strON,data,data_len))) {
	uint8_t temp = ptr->bHtemp;
	if ((temp < 35) || (temp > 98)) temp = ptr->bLtemp;
	if ((temp < 35) || (temp > 98)) temp = 40;

	if (ptr->DEV_TYP == 1) {	
	if (temp < 35) ptr->r4slppar1 = 0;
	else if (temp < 41) ptr->r4slppar1 = 1;
	else if (temp < 56) ptr->r4slppar1 = 2;
	else if (temp < 71) ptr->r4slppar1 = 3;
	else if (temp < 86) ptr->r4slppar1 = 4;
	else ptr->r4slppar1 = 5;
	} else ptr->r4slppar1 = temp;
	ptr->r4slppar2 = 1;
	ptr->r4slpcom = 3;
	}
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_HEAT_ON");
	} else if ((!incascmp("0",data,data_len)) || (!incascmp("off",data,data_len))
		|| (!incascmp("false",data,data_len))) {
	if ((ptr->bHeat)  || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strOFF,data,data_len))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 1;
	}	
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_HEAT_OFF");
	} else if (!incascmp("auto",data,data_len)) {
	if ((!ptr->bState) || (!fcommtp) || (!ptr->r4sppcom)) {
	if (ptr->bHtemp) {
	if (ptr->DEV_TYP == 1) {	
	if (ptr->bHtemp < 35) ptr->r4slppar1 = 0;
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

//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_AUTO_ON");
	} else if (!incascmp("cool",data,data_len)) {
	if ((!ptr->bStNl) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strON,data,data_len))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 5;
	}
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_NIGHTLIGHT_ON");
		} 

	} else if (!memcmp(topic+topoff, "heat_temp", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
	int tempsz = data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, data, tempsz);
	uint8_t temp = atoi(tbuff);
	if (temp && (temp < 40) && (ptr->DEV_TYP == 2)) temp = 0;
	if (temp && (temp < 35)) temp = 0;
	if  ((!fcommtp) || (!ptr->r4sppcom) || ((temp != ptr->bHtemp) && temp && ptr->bHtemp)) {
	if (!ptr->bState && (temp == 100)) {
	if (ptr->bHtemp && (ptr->bHtemp < 100)) {
	if (ptr->DEV_TYP == 1) {	
	if (ptr->bHtemp < 35) ptr->r4slppar1 = 0;
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
	} else {
	if (ptr->DEV_TYP == 1) {	
	if (temp < 35) ptr->r4slppar1 = 0;
	else if (temp < 41) ptr->r4slppar1 = 1;
	else if (temp < 56) ptr->r4slppar1 = 2;
	else if (temp < 71) ptr->r4slppar1 = 3;
	else if (temp < 86) ptr->r4slppar1 = 4;
	else ptr->r4slppar1 = 5;
	} else ptr->r4slppar1 = temp;
	ptr->r4slppar2 = 1;
	ptr->r4slpcom = 3;
	}
	} else if ((!fcommtp) || (!ptr->r4sppcom) || (temp != ptr->bHtemp)) ptr->bprevHtemp = 255;

//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_HEAT_TEMP");

	} else if (!memcmp(topic+topoff, "backlight", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
	if ((!incascmp("1",data,data_len)) || (!incascmp("on",data,data_len))
		|| (!incascmp("true",data,data_len))) {
	if ((!ptr->bStBl) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strON,data,data_len))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 22;
	}
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_BACKLIGHT_ON");
	} else if ((!incascmp("0",data,data_len)) || (!incascmp("off",data,data_len))
		|| (!incascmp("false",data,data_len))) {
	if ((ptr->bStBl) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strOFF,data,data_len))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 21;
	}
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_BACKLIGHT_OFF");
	}

	} else if (!memcmp(topic+topoff, "beep", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
	if ((!incascmp("1",data,data_len)) || (!incascmp("on",data,data_len))
		|| (!incascmp("true",data,data_len))) {
	if ((!ptr->bStBp) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strON,data,data_len))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 24;
	}
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_BEEP_ON");
	} else if ((!incascmp("0",data,data_len)) || (!incascmp("off",data,data_len))
		|| (!incascmp("false",data,data_len))) {
	if ((ptr->bStBp) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strOFF,data,data_len))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 23;
	}
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_BEEP_OFF");
	}

	} else if (!memcmp(topic+topoff, "nightlight", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
	if ((!incascmp("1",data,data_len)) || (!incascmp("on",data,data_len))
		|| (!incascmp("true",data,data_len))) {
	if ((!ptr->bStNl) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strON,data,data_len))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 5;
	}
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_NIGHTLIGHT_ON");
	} else if ((!incascmp("0",data,data_len)) || (!incascmp("off",data,data_len))
		|| (!incascmp("false",data,data_len))) {
	if ((ptr->bStNl) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strOFF,data,data_len))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 1;
	}
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_NIGHTLIGHT_OFF");
	}

	} else if (!memcmp(topic+topoff, "nightlight_rgb", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
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
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	}
	}

	} else if (!memcmp(topic+topoff, "nightlight_red", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
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
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	}
	} else if (!memcmp(topic+topoff, "nightlight_green", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
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
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	}
	} else if (!memcmp(topic+topoff, "nightlight_blue", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
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
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	}
	} else if (!memcmp(topic+topoff, "boil_time", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
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
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	}
	}
	} else if ( ptr->DEV_TYP < 12) {
	//power
	if (!memcmp(topic+topoff, "state", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
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
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_CMD_ON");
	} else if ((!incascmp("0",data,data_len)) || (!incascmp("off",data,data_len))
		|| (!incascmp("false",data,data_len))) {
	if ((ptr->bState) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strOFF,data,data_len))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 1;
	}	
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_CMD_OFF");
	}

	} else if (!memcmp(topic+topoff, "power", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
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
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_POWER");

	} else 	if (!memcmp(topic+topoff, "lock", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
	if ((!incascmp("1",data,data_len)) || (!incascmp("on",data,data_len))
		|| (!incascmp("true",data,data_len))) {
	if ((!ptr->bLock) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strON,data,data_len))) {	
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 8;
	}
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_lock_ON");
	} else if ((!incascmp("0",data,data_len)) || (!incascmp("off",data,data_len))
		|| (!incascmp("false",data,data_len))) {
	if ((ptr->bLock) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strOFF,data,data_len))) {	
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 7;
	}	
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_lock_OFF");
	}
	} else 	if (!memcmp(topic+topoff, "keep", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
	if ((!incascmp("1",data,data_len)) || (!incascmp("on",data,data_len))
		|| (!incascmp("true",data,data_len))) {
	if ((!ptr->bModProg) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strON,data,data_len))) {	
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 22;
	}
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_keep_ON");
	} else if ((!incascmp("0",data,data_len)) || (!incascmp("off",data,data_len))
		|| (!incascmp("false",data,data_len))) {
	if ((ptr->bModProg) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strOFF,data,data_len))) {	
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 21;
	}	
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_keep_OFF");
	}
	}
	} else if ( ptr->DEV_TYP < 16) {
	//coffee
	if (!memcmp(topic+topoff, "state", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
	if ((!incascmp("1",data,data_len)) || (!incascmp("on",data,data_len))
		|| (!incascmp("true",data,data_len))) {
	if ((!ptr->bState) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strON,data,data_len))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 6;
	}
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_CMD_ON");
	} else if ((!incascmp("0",data,data_len)) || (!incascmp("off",data,data_len))
		|| (!incascmp("false",data,data_len))) {
	if ((ptr->bState) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strOFF,data,data_len))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 1;
	}	
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_CMD_OFF");
	}
	} else if (!memcmp(topic+topoff, "delay", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
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
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_delay_ON");
	} else if ((!incascmp("0",data,data_len)) || (!incascmp("off",data,data_len))
		|| (!incascmp("false",data,data_len))) {
	if ((ptr->bStNl) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strOFF,data,data_len))) {	
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 1;
	}	
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_delay_OFF");
	}
	} else if (!memcmp(topic+topoff, "delay_hour", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
	int tempsz = data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, data, tempsz);
	uint8_t hour = atoi(tbuff);
	if (hour < 24) {
	ptr->bDHour = hour;
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	}
	} else if (!memcmp(topic+topoff, "delay_min", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
	int tempsz = data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, data, tempsz);
	uint8_t min = atoi(tbuff);
	if (min < 60) {
	ptr->bDMin = min;
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	}
	} else 	if (!memcmp(topic+topoff, "lock", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
	if ((!incascmp("1",data,data_len)) || (!incascmp("on",data,data_len))
		|| (!incascmp("true",data,data_len))) {
	if ((!ptr->bLock) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strON,data,data_len))) {	
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 8;
	}
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_lock_ON");
	} else if ((!incascmp("0",data,data_len)) || (!incascmp("off",data,data_len))
		|| (!incascmp("false",data,data_len))) {
	if ((ptr->bLock) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strOFF,data,data_len))) {	
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 7;
	}	
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_lock_OFF");
	}
	} else 	if (!memcmp(topic+topoff, "strength", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
	if ((!incascmp("1",data,data_len)) || (!incascmp("on",data,data_len))
		|| (!incascmp("true",data,data_len))) {
	if ((!ptr->bProg) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strON,data,data_len))) {	
	ptr->r4slppar1 = 1;
	ptr->r4slpcom = 9;
	}
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_strength_ON");
	} else if ((!incascmp("0",data,data_len)) || (!incascmp("off",data,data_len))
		|| (!incascmp("false",data,data_len))) {
	if ((ptr->bProg) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strOFF,data,data_len))) {	
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 9;
	}	
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_strength_OFF");
	}
	}

	} else if (ptr->DEV_TYP < 61) {
	//cooker
	if (!memcmp(topic+topoff, "state", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
	if ((!incascmp("1",data,data_len)) || (!incascmp("on",data,data_len))
		|| (!incascmp("true",data,data_len)) || (!incascmp("heat",data,data_len))) {
	if ((ptr->bState < 2) || (!ptr->r4sppcom) || (!fcommtp)) {
	ptr->r4slppar1 = 1;
	ptr->r4slpcom = 10;
	}
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_CMD_ON");
	} else if ((!incascmp("0",data,data_len)) || (!incascmp("off",data,data_len))
		|| (!incascmp("false",data,data_len))) {
	if ((ptr->bState > 1) || (!ptr->r4sppcom) || (!fcommtp)) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 10;
	}	
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_CMD_OFF");
	}
	} else if (!memcmp(topic+topoff, "prname", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
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
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
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
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
	int tempsz = data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, data, tempsz);
	uint8_t mod = atoi(tbuff);
	if ((mod < 4) && ((!fcommtp) || (!ptr->r4sppcom) || ( mod != ptr->bModProg))) {
	ptr->r4slppar1 = mod;
	ptr->r4slpcom = 12;
	}
	} else if (!memcmp(topic+topoff, "temp", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
	int tempsz = data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, data, tempsz);
	uint8_t temp = atoi(tbuff);
	if ((!fcommtp) || (!ptr->r4sppcom) || (temp != ptr->bHtemp)) {
	ptr->r4slppar1 = temp;
	ptr->r4slpcom = 13;
	}
	} else if (!memcmp(topic+topoff, "set_hour", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
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
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
	int tempsz = data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, data, tempsz);
	uint8_t min = atoi(tbuff);
	if ((min < 60) && ((!fcommtp) || (!ptr->r4sppcom) || (min != ptr->bPMin))) {
	ptr->r4slppar1 = min;
	ptr->r4slpcom = 15;
	}
	} else if (!memcmp(topic+topoff, "delay_hour", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
	int tempsz = data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, data, tempsz);
	uint8_t cval = atoi(tbuff);
	if ( ptr->DEV_TYP < 24 ) {
	ptr->bDHour = cval;
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	} else {
	ptr->r4slppar1 = cval;
	ptr->bDHour = cval;
	ptr->r4slpcom = 19;
	}
	} else if (!memcmp(topic+topoff, "delay_min", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
	int tempsz = data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, data, tempsz);
	uint8_t cval = atoi(tbuff);
	if ( ptr->DEV_TYP < 24 ) {
	ptr->bDMin = cval;
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	} else {
	ptr->r4slppar1 = cval;
	ptr->r4slpcom = 20;
	}
	} else if (!memcmp(topic+topoff, "warm", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
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
	} else if (ptr->DEV_TYP == 61) {
	//iron
	if (!memcmp(topic+topoff, "state", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
	if ((!incascmp("1",data,data_len)) || (!incascmp("on",data,data_len))
		|| (!incascmp("true",data,data_len)) || (!incascmp("heat",data,data_len))) {
	if ((!ptr->bState) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strON,data,data_len))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 6;
	}
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_CMD_ON");
	} else if ((!incascmp("0",data,data_len)) || (!incascmp("off",data,data_len))
		|| (!incascmp("false",data,data_len))) {
	if ((ptr->bState) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strOFF,data,data_len))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 1;
	}	
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_CMD_OFF");
	}
	} else if (!memcmp(topic+topoff, "lock", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
	if ((!incascmp("1",data,data_len)) || (!incascmp("on",data,data_len))
		|| (!incascmp("true",data,data_len)) || (!incascmp("heat",data,data_len))) {
	if ((!ptr->bState) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strON,data,data_len))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 8;
	}
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_CMD_LOCK_ON");
	} else if ((!incascmp("0",data,data_len)) || (!incascmp("off",data,data_len))
		|| (!incascmp("false",data,data_len))) {
	if ((ptr->bState) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strOFF,data,data_len))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 7;
	}	
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_CMD_LOCK_OFF");
	}
	}
	} else if (ptr->DEV_TYP == 62) {
	//weather station
	if (!memcmp(topic+topoff, "clear", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
	if ((!incascmp("1",data,data_len)) || (!incascmp("on",data,data_len))
		|| (!incascmp("true",data,data_len))) {
	if ((!ptr->bState) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strON,data,data_len))) {
	ptr->bState = 1;
	ptr->bprevState = 255;
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 62;
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	}
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_CLEAR_ON");
	} else if ((!incascmp("0",data,data_len)) || (!incascmp("off",data,data_len))
		|| (!incascmp("false",data,data_len))) {
	if ((ptr->bState)  || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strOFF,data,data_len))) {
	ptr->bState = 0;
	ptr->bprevState = 255;
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	}
	}
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_CLEAR_OFF");
	}

	} else if (ptr->DEV_TYP == 63) {
	//weather station
	if (!memcmp(topic+topoff, "calibration", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
	if ((!incascmp("1",data,data_len)) || (!incascmp("on",data,data_len))
		|| (!incascmp("true",data,data_len))) {
	if ((!ptr->bState) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strON,data,data_len))) {
	ptr->bState = 1;
	ptr->bprevState = 255;
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 63;
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	}
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_CALIBRATION_ON");
	} else if ((!incascmp("0",data,data_len)) || (!incascmp("off",data,data_len))
		|| (!incascmp("false",data,data_len))) {
	if ((ptr->bState)  || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strOFF,data,data_len))) {
	ptr->bState = 0;
	ptr->bprevState = 255;
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	}
	}
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_CALIBRATION_OFF");
	}

	} else if ((ptr->DEV_TYP > 63) && (ptr->DEV_TYP < 73)) {
	//mikettle
	if (!memcmp(topic+topoff, "boil", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
	if ((!incascmp("1",data,data_len)) || (!incascmp("on",data,data_len))
		|| (!incascmp("true",data,data_len)) || (!incascmp("heat",data,data_len))) {
	if ((!ptr->bState) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strON,data,data_len))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 65;
	}
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_BOIL_ON");
	} else if ((!incascmp("0",data,data_len)) || (!incascmp("off",data,data_len))
		|| (!incascmp("false",data,data_len))) {
	if ((ptr->bState)  || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strOFF,data,data_len))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 64;
	}	
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_BOIL_OFF");
	}
	} else if (!memcmp(topic+topoff, "heat", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
	if ((!incascmp("1",data,data_len)) || (!incascmp("on",data,data_len))
		|| (!incascmp("true",data,data_len)) || (!incascmp("heat",data,data_len))) {
	if ((!ptr->bHeat) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strON,data,data_len))) {
	ptr->r4slppar1 = 40;
	ptr->r4slpcom = 66;
	}
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_HEAT_ON");
	} else if ((!incascmp("0",data,data_len)) || (!incascmp("off",data,data_len))
		|| (!incascmp("false",data,data_len))) {
// boil off only heat still on
	if ((ptr->bState)  || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strOFF,data,data_len))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 64;
/*      
	} else if ((ptr->bHeat)  || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strOFF,data,data_len))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 66;
*/
	}	
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_HEAT_OFF");
	} else if (!incascmp("auto",data,data_len)) {
	if ((!ptr->bState) || (!fcommtp) || (!ptr->r4sppcom)) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 65;
	}
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_AUTO_ON");
	}
	} else if (!memcmp(topic+topoff, "heat_temp", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
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
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_HEAT_TEMP");
	} else if (ptr->DEV_TYP == 73) {
	//galcon
	if (!memcmp(topic+topoff, "state", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
	if ((!incascmp("1",data,data_len)) || (!incascmp("on",data,data_len))
		|| (!incascmp("true",data,data_len)) || (!incascmp("heat",data,data_len))) {
	if ((!ptr->bState) || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strON,data,data_len))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 73;
	}
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_SWITCH_ON");
	} else if ((!incascmp("0",data,data_len)) || (!incascmp("off",data,data_len))
		|| (!incascmp("false",data,data_len))) {
	if ((ptr->bState)  || (!fcommtp) || (!ptr->r4sppcom) || (inccmp(strOFF,data,data_len))) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 72;
	}	
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_SWITCH_OFF");
	}
	} else if (!memcmp(topic+topoff, "set_hour", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
	int tempsz = data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, data, tempsz);
	uint8_t hour = atoi(tbuff);
	if (hour < 24) {
	ptr->bPHour = hour;
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	}
	} else if (!memcmp(topic+topoff, "set_min", topic_len-topoff)) {
	if (!fcommtp) esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
	int tempsz = data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(tbuff, data, tempsz);
	uint8_t min = atoi(tbuff);
	if (min < 60) {
	ptr->bPMin = min;
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	}
	}
	} else if (ptr->DEV_TYP == 74) {
	if (!memcmp(topic+topoff, "set", topic_len-topoff)) {
	esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
	if (!incascmp("open",data,data_len)) {
	ptr->r4slppar1 = 100;
	ptr->r4slpcom = 26;
	} else if (!incascmp("close",data,data_len)) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 26;
	} else if (!incascmp("stop",data,data_len)) {
	ptr->r4slppar1 = 254;
	ptr->r4slpcom = 26;
	} else if (data_len && (data_len < 5) && (data[0] > 0x2f) && (data[0] < 0x3a)) {
	uint8_t tsetpos = 255;
	mystrcpy(tbuff, data, data_len);
	tsetpos = atoi(tbuff);
	if (tsetpos < 101){
	ptr->r4slppar1 = tsetpos;
	ptr->r4slpcom = 26;
	}
	} 

	} // open...
	} // if set 74

} // if dev_typ

void HDiscBlemon(bool mqtttst)
{
	if (mqtttst && ble_mon) {
	char llwtt[128];
	char llwtd[1024];
	char tmpvar[64];
	for (int i = 0; i < BleMonNum; i++) {
	if(BleMR[i].sto) {
//
	strcpy(llwtt,"r4s/");
	if ((BleMR[i].id > 0x40) && (BleMR[i].id < 0x50)) bin2hex(BleMR[i].mac,tmpvar,16,0); 
	else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtt,tmpvar);
	strcat(llwtt,"/#");
	esp_mqtt_client_subscribe(mqttclient, llwtt, 0);
	if (FDHass) {
	strcpy(llwtt,"homeassistant/device_tracker/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/1x");
	if ((BleMR[i].id > 0x40) && (BleMR[i].id < 0x50)) bin2hex(BleMR[i].mac,tmpvar,16,0); 
	else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtt,tmpvar);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".");
	if ((BleMR[i].id > 0x40) && (BleMR[i].id < 0x50)) bin2hex(BleMR[i].mac,tmpvar,16,0); 
	else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,".state\",\"icon\":\"mdi:tag\",\"uniq_id\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"_");
	if ((BleMR[i].id > 0x40) && (BleMR[i].id < 0x50)) bin2hex(BleMR[i].mac,tmpvar,16,0); 
	else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"_state\",\"device\":{\"identifiers\":[\"r4s_");
	if ((BleMR[i].id > 0x40) && (BleMR[i].id < 0x50)) bin2hex(BleMR[i].mac,tmpvar,16,0); 
	else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"\"],\"name\":\"r4s.");
	if ((BleMR[i].id > 0x40) && (BleMR[i].id < 0x50)) bin2hex(BleMR[i].mac,tmpvar,16,0); 
	else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"\",\"model\":\"");
	if (BleMR[i].id == 2) strcat(llwtd,"Mi Scale");
	else if (BleMR[i].id == 3) strcat(llwtd,"ATC_MiThermometer LYWSD03MMC");
	else if (BleMR[i].id == 0x42) strcat(llwtd,"HA iBeacon");
	else if (BleMR[i].id == 0x44) strcat(llwtd,"Smart Tag");
	else strcat(llwtd,"Unknown");
	strcat(llwtd,"\",\"manufacturer\":\"");
	if (BleMR[i].id == 2) strcat(llwtd,"Xiaomi");
	else if (BleMR[i].id == 3) strcat(llwtd,"Xiaomi & pvvx & atc1441");
	else if (BleMR[i].id == 0x42) strcat(llwtd,"Android / iOS");
	else if (BleMR[i].id == 0x44) strcat(llwtd,"Samsung Electronics");
	else strcat(llwtd,"Unknown");
	strcat(llwtd,"\",\"via_device\":\"ESP32_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\"},\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/");
	if ((BleMR[i].id > 0x40) && (BleMR[i].id < 0x50)) bin2hex(BleMR[i].mac,tmpvar,16,0); 
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
	if ((BleMR[i].id > 0x40) && (BleMR[i].id < 0x50)) bin2hex(BleMR[i].mac,tmpvar,16,0); 
	else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtt,tmpvar);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".");
	if ((BleMR[i].id > 0x40) && (BleMR[i].id < 0x50)) bin2hex(BleMR[i].mac,tmpvar,16,0); 
	else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,".rssi\",\"icon\":\"mdi:bluetooth\",\"uniq_id\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"_");
	if ((BleMR[i].id > 0x40) && (BleMR[i].id < 0x50)) bin2hex(BleMR[i].mac,tmpvar,16,0); 
	else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"_rssi\",\"device\":{\"identifiers\":[\"r4s_");
	if ((BleMR[i].id > 0x40) && (BleMR[i].id < 0x50)) bin2hex(BleMR[i].mac,tmpvar,16,0); 
	else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"\"],\"name\":\"r4s.");
	if ((BleMR[i].id > 0x40) && (BleMR[i].id < 0x50)) bin2hex(BleMR[i].mac,tmpvar,16,0); 
	else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"\",\"model\":\"");
	if (BleMR[i].id == 2) strcat(llwtd,"Mi Scale");
	else if (BleMR[i].id == 3) strcat(llwtd,"ATC_MiThermometer LYWSD03MMC");
	else if (BleMR[i].id == 0x42) strcat(llwtd,"HA iBeacon");
	else if (BleMR[i].id == 0x44) strcat(llwtd,"Smart Tag");
	else strcat(llwtd,"Unknown");
	strcat(llwtd,"\",\"manufacturer\":\"");
	if (BleMR[i].id == 2) strcat(llwtd,"Xiaomi");
	else if (BleMR[i].id == 3) strcat(llwtd,"Xiaomi & pvvx & atc1441");
	else if (BleMR[i].id == 0x42) strcat(llwtd,"Android / iOS");
	else if (BleMR[i].id == 0x44) strcat(llwtd,"Samsung Electronics");
	else strcat(llwtd,"Unknown");
	strcat(llwtd,"\",\"via_device\":\"ESP32_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\"},\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/");
	if ((BleMR[i].id > 0x40) && (BleMR[i].id < 0x50)) bin2hex(BleMR[i].mac,tmpvar,16,0); 
	else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"/rssi\",\"unit_of_meas\":\"dBm\",\"device_class\":\"signal_strength\",\"state_class\":\"measurement\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
//
// common topics
	strcpy(llwtt,"homeassistant/sensor/");
	strcat(llwtt,"r4s/1x");
	if ((BleMR[i].id > 0x40) && (BleMR[i].id < 0x50)) bin2hex(BleMR[i].mac,tmpvar,16,0); 
	else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtt,tmpvar);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,"r4s");
	strcat(llwtd,".");
	if ((BleMR[i].id > 0x40) && (BleMR[i].id < 0x50)) bin2hex(BleMR[i].mac,tmpvar,16,0); 
	else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,".rssi\",\"icon\":\"mdi:bluetooth\",\"uniq_id\":\"");
	strcat(llwtd,"r4s");
	strcat(llwtd,"_");
	if ((BleMR[i].id > 0x40) && (BleMR[i].id < 0x50)) bin2hex(BleMR[i].mac,tmpvar,16,0); 
	else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"_rssi\",\"device\":{\"identifiers\":[\"r4s_");
	if ((BleMR[i].id > 0x40) && (BleMR[i].id < 0x50)) bin2hex(BleMR[i].mac,tmpvar,16,0); 
	else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"\"],\"name\":\"r4s.");
	if ((BleMR[i].id > 0x40) && (BleMR[i].id < 0x50)) bin2hex(BleMR[i].mac,tmpvar,16,0); 
	else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"\",\"model\":\"");
	if (BleMR[i].id == 2) strcat(llwtd,"Mi Scale");
	else if (BleMR[i].id == 3) strcat(llwtd,"ATC_MiThermometer LYWSD03MMC");
	else if (BleMR[i].id == 0x42) strcat(llwtd,"HA iBeacon");
	else if (BleMR[i].id == 0x44) strcat(llwtd,"Smart Tag");
	else strcat(llwtd,"Unknown");
	strcat(llwtd,"\",\"manufacturer\":\"");
	if (BleMR[i].id == 2) strcat(llwtd,"Xiaomi");
	else if (BleMR[i].id == 3) strcat(llwtd,"Xiaomi & pvvx & atc1441");
	else if (BleMR[i].id == 0x42) strcat(llwtd,"Android / iOS");
	else if (BleMR[i].id == 0x44) strcat(llwtd,"Samsung Electronics");
	else strcat(llwtd,"Unknown");
	strcat(llwtd,"\",\"via_device\":\"ESP32_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\"},\"state_topic\":\"");
	strcat(llwtd,"r4s/");
	if ((BleMR[i].id > 0x40) && (BleMR[i].id < 0x50)) bin2hex(BleMR[i].mac,tmpvar,16,0); 
	else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"/rssi\",\"unit_of_meas\":\"dBm\",\"device_class\":\"signal_strength\",\"state_class\":\"measurement\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
//
	strcpy(llwtt,"homeassistant/sensor/");
	strcat(llwtt,"r4s/2x");
	if ((BleMR[i].id > 0x40) && (BleMR[i].id < 0x50)) bin2hex(BleMR[i].mac,tmpvar,16,0); 
	else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtt,tmpvar);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,"r4s");
	strcat(llwtd,".");
	if ((BleMR[i].id > 0x40) && (BleMR[i].id < 0x50)) bin2hex(BleMR[i].mac,tmpvar,16,0); 
	else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,".gtnum\",\"icon\":\"mdi:router-wireless\",\"uniq_id\":\"");
	strcat(llwtd,"r4s");
	strcat(llwtd,"_");
	if ((BleMR[i].id > 0x40) && (BleMR[i].id < 0x50)) bin2hex(BleMR[i].mac,tmpvar,16,0); 
	else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"_gtnum\",\"device\":{\"identifiers\":[\"r4s_");
	if ((BleMR[i].id > 0x40) && (BleMR[i].id < 0x50)) bin2hex(BleMR[i].mac,tmpvar,16,0); 
	else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"\"],\"name\":\"r4s.");
	if ((BleMR[i].id > 0x40) && (BleMR[i].id < 0x50)) bin2hex(BleMR[i].mac,tmpvar,16,0); 
	else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"\",\"model\":\"");
	if (BleMR[i].id == 2) strcat(llwtd,"Mi Scale");
	else if (BleMR[i].id == 3) strcat(llwtd,"ATC_MiThermometer LYWSD03MMC");
	else if (BleMR[i].id == 0x42) strcat(llwtd,"HA iBeacon");
	else if (BleMR[i].id == 0x44) strcat(llwtd,"Smart Tag");
	else strcat(llwtd,"Unknown");
	strcat(llwtd,"\",\"manufacturer\":\"");
	if (BleMR[i].id == 2) strcat(llwtd,"Xiaomi");
	else if (BleMR[i].id == 3) strcat(llwtd,"Xiaomi & pvvx & atc1441");
	else if (BleMR[i].id == 0x42) strcat(llwtd,"Android / iOS");
	else if (BleMR[i].id == 0x44) strcat(llwtd,"Samsung Electronics");
	else strcat(llwtd,"Unknown");
	strcat(llwtd,"\",\"via_device\":\"ESP32_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\"},\"state_topic\":\"");
	strcat(llwtd,"r4s/");
	if ((BleMR[i].id > 0x40) && (BleMR[i].id < 0x50)) bin2hex(BleMR[i].mac,tmpvar,16,0); 
	else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"/gtnum\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
//
	if (BleMR[i].id == 2) {
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
	strcat(llwtd,".weight\",\"icon\":\"mdi:scale\",\"uniq_id\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"_");
	bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"_weight\",\"device\":{\"identifiers\":[\"r4s_");
	bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"\"],\"name\":\"r4s.");
	bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"\",\"model\":\"");
	strcat(llwtd,"Mi Scale");
	strcat(llwtd,"\",\"manufacturer\":\"");
	strcat(llwtd,"Xiaomi");
	strcat(llwtd,"\",\"via_device\":\"ESP32_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\"},\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/");
	bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"/weight\",\"unit_of_meas\":\"");
	if (BleMX[i].par5 & 0x100) strcat(llwtd,"lbs");
	else strcat(llwtd,"kg");
	strcat(llwtd,"\",\"availability_topic\":\"");
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
	strcat(llwtd,".date\",\"icon\":\"mdi:calendar\",\"uniq_id\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"_");
	bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"_date\",\"device\":{\"identifiers\":[\"r4s_");
	bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"\"],\"name\":\"r4s.");
	bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"\",\"model\":\"");
	strcat(llwtd,"Mi Scale");
	strcat(llwtd,"\",\"manufacturer\":\"");
	strcat(llwtd,"Xiaomi");
	strcat(llwtd,"\",\"via_device\":\"ESP32_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\"},\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/");
	bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"/date\",\"availability_topic\":\"");
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
	strcat(llwtd,".time\",\"icon\":\"mdi:clock\",\"uniq_id\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"_");
	bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"_time\",\"device\":{\"identifiers\":[\"r4s_");
	bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"\"],\"name\":\"r4s.");
	bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"\",\"model\":\"");
	strcat(llwtd,"Mi Scale");
	strcat(llwtd,"\",\"manufacturer\":\"");
	strcat(llwtd,"Xiaomi");
	strcat(llwtd,"\",\"via_device\":\"ESP32_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\"},\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/");
	bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"/time\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
//
	strcpy(llwtt,"homeassistant/sensor/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/5x");
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
	strcat(llwtd,".prov_weight\",\"icon\":\"mdi:scale\",\"uniq_id\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"_");
	bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"_prov_weight\",\"device\":{\"identifiers\":[\"r4s_");
	bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"\"],\"name\":\"r4s.");
	bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"\",\"model\":\"");
	strcat(llwtd,"Mi Scale");
	strcat(llwtd,"\",\"manufacturer\":\"");
	strcat(llwtd,"Xiaomi");
	strcat(llwtd,"\",\"via_device\":\"ESP32_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\"},\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/");
	bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"/prov_weight\",\"unit_of_meas\":\"");
	if (BleMX[i].par5 & 0x100) strcat(llwtd,"lbs");
	else strcat(llwtd,"kg");
	strcat(llwtd,"\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
//
	strcpy(llwtt,"homeassistant/sensor/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/6x");
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
	strcat(llwtd,".impedance\",\"icon\":\"mdi:resistor\",\"uniq_id\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"_");
	bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"_impedance\",\"device\":{\"identifiers\":[\"r4s_");
	bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"\"],\"name\":\"r4s.");
	bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"\",\"model\":\"");
	strcat(llwtd,"Mi Scale");
	strcat(llwtd,"\",\"manufacturer\":\"");
	strcat(llwtd,"Xiaomi");
	strcat(llwtd,"\",\"via_device\":\"ESP32_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\"},\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/");
	bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"/impedance\",\"unit_of_meas\":\"\xce\xa9\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
//
	} else if (BleMR[i].id == 3) {
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
	strcat(llwtd,"ATC_MiThermometer LYWSD03MMC");
	strcat(llwtd,"\",\"manufacturer\":\"");
	strcat(llwtd,"Xiaomi & pvvx & atc1441");
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
	strcat(llwtd,"ATC_MiThermometer LYWSD03MMC");
	strcat(llwtd,"\",\"manufacturer\":\"");
	strcat(llwtd,"Xiaomi & pvvx & atc1441");
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
	strcat(llwtd,"ATC_MiThermometer LYWSD03MMC");
	strcat(llwtd,"\",\"manufacturer\":\"");
	strcat(llwtd,"Xiaomi & pvvx & atc1441");
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
	} else 	if (BleMR[i].id == 0x44) {
	strcpy(llwtt,"homeassistant/sensor/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/2x");
	bin2hex(BleMR[i].mac,tmpvar,16,0);
	strcat(llwtt,tmpvar);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".");
	bin2hex(BleMR[i].mac,tmpvar,16,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,".tgstate\",\"icon\":\"mdi:tag-check\",\"uniq_id\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"_");
	bin2hex(BleMR[i].mac,tmpvar,16,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"_tgstate\",\"device\":{\"identifiers\":[\"r4s_");
	bin2hex(BleMR[i].mac,tmpvar,16,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"\"],\"name\":\"r4s.");
	bin2hex(BleMR[i].mac,tmpvar,16,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"\",\"model\":\"");
	strcat(llwtd,"Smart Tag");
	strcat(llwtd,"\",\"manufacturer\":\"");
	strcat(llwtd,"Samsung Electronics");
	strcat(llwtd,"\",\"via_device\":\"ESP32_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\"},\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/");
	bin2hex(BleMR[i].mac,tmpvar,16,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"/tgstate\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
//
	strcpy(llwtt,"homeassistant/sensor/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/3x");
	bin2hex(BleMR[i].mac,tmpvar,16,0);
	strcat(llwtt,tmpvar);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".");
	bin2hex(BleMR[i].mac,tmpvar,16,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,".battery\",\"icon\":\"mdi:battery-bluetooth\",\"uniq_id\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"_");
	bin2hex(BleMR[i].mac,tmpvar,16,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"_battery\",\"device\":{\"identifiers\":[\"r4s_");
	bin2hex(BleMR[i].mac,tmpvar,16,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"\"],\"name\":\"r4s.");
	bin2hex(BleMR[i].mac,tmpvar,16,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"\",\"model\":\"");
	strcat(llwtd,"Smart Tag");
	strcat(llwtd,"\",\"manufacturer\":\"");
	strcat(llwtd,"Samsung Electronics");
	strcat(llwtd,"\",\"via_device\":\"ESP32_");
	strcat(llwtd,tESP32Addr);
	strcat(llwtd,"\"},\"device_class\":\"battery\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/");
	bin2hex(BleMR[i].mac,tmpvar,16,0);
	strcat(llwtd,tmpvar);
	strcat(llwtd,"/battery\",\"unit_of_meas\":\"\x25\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
//



	}                 //fdhass
	}
	}
	}
	}
}

bool HDisci2c(uint32_t* f_i2cdev)
{
	bool result = 0;
	uint32_t i2cbits = * f_i2cdev;
	if (!(i2cbits & 0x80000000)) return result;
	char llwtt[128];
	char llwtd[512];
	char tbuff[8];
	tcpip_adapter_ip_info_t ipInfo;
	char wbuff[256];
	memset(wbuff,0,32);
	tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo);
	sprintf(wbuff, "%d.%d.%d.%d", IP2STR(&ipInfo.ip));
	if (FDHass && tESP32Addr[0]) {
	for (int i = 0; i < 28; i++) {
	if (i2cbits & (1 << i)) {
	if (i2c_bits[i] & 0x01) {
//i2c temp
	strcpy(llwtt,"homeassistant/sensor/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/i2c");
	bin2hex(&i2c_addr[i],tbuff,1,0);
	strcat(llwtt,tbuff);
	strcat(llwtt,"tx");
	strcat(llwtt,tESP32Addr);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate.i2c");
	bin2hex(&i2c_addr[i],tbuff,1,0);
	strcat(llwtd,tbuff);
	strcat(llwtd,".temp\",\"icon\":\"mdi:thermometer\",\"uniq_id\":\"i2c");
	bin2hex(&i2c_addr[i],tbuff,1,0);
	strcat(llwtd,tbuff);
	strcat(llwtd,"_temp_");
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
	strcat(llwtd,"\",\"connections\":[[\"mac\",\"");
	strcat(llwtd,tESP32Addr1);
//	if (wbuff[0]) {
//	strcat(llwtd,"\"],[\"ip\",\"");
//	strcat(llwtd,wbuff);
//	}
	strcat(llwtd,"\"]],\"manufacturer\":\"Espressif\"},\"device_class\":\"temperature\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/i2c");
	bin2hex(&i2c_addr[i],tbuff,1,0);
	strcat(llwtd,tbuff);
	strcat(llwtd,"temp\",\"unit_of_meas\":\"\xc2\xb0\x43\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	}
	if (i2c_bits[i] & 0x02) {
//i2c hum
	strcpy(llwtt,"homeassistant/sensor/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/i2c");
	bin2hex(&i2c_addr[i],tbuff,1,0);
	strcat(llwtt,tbuff);
	strcat(llwtt,"hx");
	strcat(llwtt,tESP32Addr);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate.i2c");
	bin2hex(&i2c_addr[i],tbuff,1,0);
	strcat(llwtd,tbuff);
	strcat(llwtd,".humid\",\"icon\":\"mdi:water-percent\",\"uniq_id\":\"i2c");
	bin2hex(&i2c_addr[i],tbuff,1,0);
	strcat(llwtd,tbuff);
	strcat(llwtd,"_humid_");
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
	strcat(llwtd,"\",\"connections\":[[\"mac\",\"");
	strcat(llwtd,tESP32Addr1);
//	if (wbuff[0]) {
//	strcat(llwtd,"\"],[\"ip\",\"");
//	strcat(llwtd,wbuff);
//	}
	strcat(llwtd,"\"]],\"manufacturer\":\"Espressif\"},\"device_class\":\"humidity\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/i2c");
	bin2hex(&i2c_addr[i],tbuff,1,0);
	strcat(llwtd,tbuff);
	strcat(llwtd,"humid\",\"unit_of_meas\":\"\x25\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	}
	if (i2c_bits[i] & 0x04) {
//i2c pressp
	strcpy(llwtt,"homeassistant/sensor/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/i2c");
	bin2hex(&i2c_addr[i],tbuff,1,0);
	strcat(llwtt,tbuff);
	strcat(llwtt,"px");
	strcat(llwtt,tESP32Addr);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate.i2c");
	bin2hex(&i2c_addr[i],tbuff,1,0);
	strcat(llwtd,tbuff);
	strcat(llwtd,".pressp\",\"icon\":\"mdi:gauge\",\"uniq_id\":\"i2c");
	bin2hex(&i2c_addr[i],tbuff,1,0);
	strcat(llwtd,tbuff);
	strcat(llwtd,"_pressp_");
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
	strcat(llwtd,"\",\"connections\":[[\"mac\",\"");
	strcat(llwtd,tESP32Addr1);
//	if (wbuff[0]) {
//	strcat(llwtd,"\"],[\"ip\",\"");
//	strcat(llwtd,wbuff);
//	}
	strcat(llwtd,"\"]],\"manufacturer\":\"Espressif\"},\"device_class\":\"pressure\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/i2c");
	bin2hex(&i2c_addr[i],tbuff,1,0);
	strcat(llwtd,tbuff);
	strcat(llwtd,"pressp\",\"unit_of_meas\":\"hPa\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
//i2c pressm
	strcpy(llwtt,"homeassistant/sensor/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/i2c");
	bin2hex(&i2c_addr[i],tbuff,1,0);
	strcat(llwtt,tbuff);
	strcat(llwtt,"mx");
	strcat(llwtt,tESP32Addr);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate.i2c");
	bin2hex(&i2c_addr[i],tbuff,1,0);
	strcat(llwtd,tbuff);
	strcat(llwtd,".pressm\",\"icon\":\"mdi:gauge\",\"uniq_id\":\"i2c");
	bin2hex(&i2c_addr[i],tbuff,1,0);
	strcat(llwtd,tbuff);
	strcat(llwtd,"_pressm_");
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
	strcat(llwtd,"\",\"connections\":[[\"mac\",\"");
	strcat(llwtd,tESP32Addr1);
//	if (wbuff[0]) {
//	strcat(llwtd,"\"],[\"ip\",\"");
//	strcat(llwtd,wbuff);
//	}
	strcat(llwtd,"\"]],\"manufacturer\":\"Espressif\"},\"device_class\":\"pressure\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/i2c");
	bin2hex(&i2c_addr[i],tbuff,1,0);
	strcat(llwtd,tbuff);
	strcat(llwtd,"pressm\",\"unit_of_meas\":\"mmHg\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	}
	if (i2c_bits[i] & 0x08) {
	strcpy(llwtt,"homeassistant/sensor/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/i2c");
	bin2hex(&i2c_addr[i],tbuff,1,0);
	strcat(llwtt,tbuff);
	strcat(llwtt,"grx");
	strcat(llwtt,tESP32Addr);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate.i2c");
	bin2hex(&i2c_addr[i],tbuff,1,0);
	strcat(llwtd,tbuff);
	strcat(llwtd,".gas.resistance\",\"icon\":\"mdi:resistor\",\"uniq_id\":\"i2c");
	bin2hex(&i2c_addr[i],tbuff,1,0);
	strcat(llwtd,tbuff);
	strcat(llwtd,"_gas_resistance_");
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
	strcat(llwtd,"\",\"connections\":[[\"mac\",\"");
	strcat(llwtd,tESP32Addr1);
//	if (wbuff[0]) {
//	strcat(llwtd,"\"],[\"ip\",\"");
//	strcat(llwtd,wbuff);
//	}
	strcat(llwtd,"\"]],\"manufacturer\":\"Espressif\"},\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/i2c");
	bin2hex(&i2c_addr[i],tbuff,1,0);
	strcat(llwtd,tbuff);
	strcat(llwtd,"gsresist\",\"unit_of_meas\":\"\xce\xa9\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	}
	} //bit i
	} //for i
//ip5306 battery level
	if (i2cbits & 0x60000000) {
	strcpy(llwtt,"homeassistant/sensor/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	if (i2cbits & 0x40000000) strcat(llwtt,"/i2c75");
	else if (i2cbits & 0x20000000) strcat(llwtt,"/i2c34");
	strcat(llwtt,"blvx");
	strcat(llwtt,tESP32Addr);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	if (i2cbits & 0x40000000) strcat(llwtd,".Gate.i2c75.battery\",\"icon\":\"mdi:battery\",\"uniq_id\":\"i2c75");
	else if (i2cbits & 0x20000000) strcat(llwtd,".Gate.i2c34.battery\",\"icon\":\"mdi:battery\",\"uniq_id\":\"i2c34");
	strcat(llwtd,"_battery_");
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
	strcat(llwtd,"\",\"connections\":[[\"mac\",\"");
	strcat(llwtd,tESP32Addr1);
//	if (wbuff[0]) {
//	strcat(llwtd,"\"],[\"ip\",\"");
//	strcat(llwtd,wbuff);
//	}
	strcat(llwtd,"\"]],\"manufacturer\":\"Espressif\"},\"device_class\":\"battery\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	if (i2cbits & 0x40000000) strcat(llwtd,"/i2c75");
	else if (i2cbits & 0x20000000) strcat(llwtd,"/i2c34");
	strcat(llwtd,"battery\",\"unit_of_meas\":\"\x25\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
//ip5306 battery state
	strcpy(llwtt,"homeassistant/sensor/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	if (i2cbits & 0x40000000) strcat(llwtt,"/i2c75");
	else if (i2cbits & 0x20000000) strcat(llwtt,"/i2c34");
	strcat(llwtt,"bstx");
	strcat(llwtt,tESP32Addr);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	if (i2cbits & 0x40000000) strcat(llwtd,".Gate.i2c75.batmode\",\"icon\":\"mdi:battery\",\"uniq_id\":\"i2c75");
	else if (i2cbits & 0x20000000) strcat(llwtd,".Gate.i2c34.batmode\",\"icon\":\"mdi:battery\",\"uniq_id\":\"i2c34");
	strcat(llwtd,"_batmode_");
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
	strcat(llwtd,"\",\"connections\":[[\"mac\",\"");
	strcat(llwtd,tESP32Addr1);
//	if (wbuff[0]) {
//	strcat(llwtd,"\"],[\"ip\",\"");
//	strcat(llwtd,wbuff);
//	}
	strcat(llwtd,"\"]],\"manufacturer\":\"Espressif\"},\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	if (i2cbits & 0x40000000) strcat(llwtd,"/i2c75");
	else if (i2cbits & 0x20000000) strcat(llwtd,"/i2c34");
	strcat(llwtd,"batmode\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
//
	if (i2cbits & 0x20000000) {
	strcpy(llwtt,"homeassistant/sensor/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/i2c34");
	strcat(llwtt,"bvvx");
	strcat(llwtt,tESP32Addr);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate.i2c34.batvoltage\",\"icon\":\"mdi:battery\",\"uniq_id\":\"i2c34");
	strcat(llwtd,"_batvoltage_");
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
	strcat(llwtd,"\",\"connections\":[[\"mac\",\"");
	strcat(llwtd,tESP32Addr1);
//	if (wbuff[0]) {
//	strcat(llwtd,"\"],[\"ip\",\"");
//	strcat(llwtd,wbuff);
	}
	strcat(llwtd,"\"]],\"manufacturer\":\"Espressif\"},\"device_class\":\"voltage\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/i2c34");
	strcat(llwtd,"batvoltage\",\"unit_of_meas\":\"V\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
//
	strcpy(llwtt,"homeassistant/sensor/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/i2c34");
	strcat(llwtt,"bvcx");
	strcat(llwtt,tESP32Addr);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate.i2c34.batcurrent\",\"icon\":\"mdi:battery\",\"uniq_id\":\"i2c34");
	strcat(llwtd,"_batcurrent_");
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
	strcat(llwtd,"\",\"connections\":[[\"mac\",\"");
	strcat(llwtd,tESP32Addr1);
//	if (wbuff[0]) {
//	strcat(llwtd,"\"],[\"ip\",\"");
//	strcat(llwtd,wbuff);
//	}
	strcat(llwtd,"\"]],\"manufacturer\":\"Espressif\"},\"device_class\":\"current\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/i2c34");
	strcat(llwtd,"batcurrent\",\"unit_of_meas\":\"A\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
//
//	}
	}
	result = 1;
	} //fdhass
	return result;
}

//******************* Mqtt **********************
static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
//    int msg_id;
    // your_context_t *context = event->context;
//uint32_t fdeltmp = 0;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_EVENT_CONNECTED");
	BleDevStA.t_ppcon = 40;
	BleDevStB.t_ppcon = 40;
	BleDevStC.t_ppcon = 40;
	BleDevStD.t_ppcon = 40;
	BleDevStE.t_ppcon = 40;
	t_ppcons = 30;
	iprevRssiESP = 0;

	BleDevStA.iprevRssi = 0;
	BleDevStB.iprevRssi = 0;
	BleDevStC.iprevRssi = 0;
	BleDevStD.iprevRssi = 0;
	BleDevStE.iprevRssi = 0;
	BleDevStA.cprevStatus[0] = 0;
	BleDevStB.cprevStatus[0] = 0;
	BleDevStC.cprevStatus[0] = 0;
	BleDevStD.cprevStatus[0] = 0;
	BleDevStE.cprevStatus[0] = 0;
	BleDevStA.bprevSEnergy = ~BleDevStA.bSEnergy;
	BleDevStB.bprevSEnergy = ~BleDevStB.bSEnergy;
	BleDevStC.bprevSEnergy = ~BleDevStC.bSEnergy;
	BleDevStD.bprevSEnergy = ~BleDevStD.bSEnergy;
	BleDevStE.bprevSEnergy = ~BleDevStE.bSEnergy;
	BleDevStA.bprevSTime = ~BleDevStA.bSTime;
	BleDevStB.bprevSTime = ~BleDevStB.bSTime;
	BleDevStC.bprevSTime = ~BleDevStC.bSTime;
	BleDevStD.bprevSTime = ~BleDevStD.bSTime;
	BleDevStE.bprevSTime = ~BleDevStE.bSTime;
	BleDevStA.bprevSHum = ~BleDevStA.bSHum;
	BleDevStB.bprevSHum = ~BleDevStB.bSHum;
	BleDevStC.bprevSHum = ~BleDevStC.bSHum;
	BleDevStD.bprevSHum = ~BleDevStD.bSHum;
	BleDevStE.bprevSHum = ~BleDevStE.bSHum;
	BleDevStA.bprevSCount = ~BleDevStA.bSCount;
	BleDevStB.bprevSCount = ~BleDevStB.bSCount;
	BleDevStC.bprevSCount = ~BleDevStC.bSCount;
	BleDevStD.bprevSCount = ~BleDevStD.bSCount;
	BleDevStE.bprevSCount = ~BleDevStE.bSCount;
	BleDevStA.bprevBlTime = 128;
	BleDevStB.bprevBlTime = 128;
	BleDevStC.bprevBlTime = 128;
	BleDevStD.bprevBlTime = 128;
	BleDevStE.bprevBlTime = 128;
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
	BleDevStD.bprevLock = 255;
	BleDevStD.bprevState = 255;
	BleDevStD.bprevHeat = 255;
	BleDevStD.bprevStNl = 255;
	BleDevStD.bprevCtemp = 255;
	BleDevStD.bprevHtemp = 255;
	BleDevStD.bprevProg = 254;
	BleDevStD.bprevModProg = 255;
	BleDevStD.bprevPHour = 255;
	BleDevStD.bprevPMin = 255;
	BleDevStD.bprevCHour = 255;
	BleDevStD.bprevCMin = 255;
	BleDevStD.bprevDHour = 255;
	BleDevStD.bprevDMin = 255;
	BleDevStD.bprevAwarm = 255;
	BleDevStD.PRgbR = ~BleDevStD.RgbR;
	BleDevStD.PRgbG = ~BleDevStD.RgbG;
	BleDevStD.PRgbB = ~BleDevStD.RgbB;
	BleDevStD.bprevStBl = 255;
	BleDevStD.bprevStBp = 255;
	BleDevStD.bprevCVol = 255;
	BleDevStD.bprevCVoll = 255;
	BleDevStE.bprevLock = 255;
	BleDevStE.bprevState = 255;
	BleDevStE.bprevHeat = 255;
	BleDevStE.bprevStNl = 255;
	BleDevStE.bprevCtemp = 255;
	BleDevStE.bprevHtemp = 255;
	BleDevStE.bprevProg = 254;
	BleDevStE.bprevModProg = 255;
	BleDevStE.bprevPHour = 255;
	BleDevStE.bprevPMin = 255;
	BleDevStE.bprevCHour = 255;
	BleDevStE.bprevCMin = 255;
	BleDevStE.bprevDHour = 255;
	BleDevStE.bprevDMin = 255;
	BleDevStE.bprevAwarm = 255;
	BleDevStE.PRgbR = ~BleDevStE.RgbR;
	BleDevStE.PRgbG = ~BleDevStE.RgbG;
	BleDevStE.PRgbB = ~BleDevStE.RgbB;
	BleDevStE.bprevStBl = 255;
	BleDevStE.bprevStBp = 255;
	BleDevStE.bprevCVol = 255;
	BleDevStE.bprevCVoll = 255;

	bprevStateS = ~bStateS;
	bprevStatHx6 = ~bStatHx6;
	bprevStatG6 = ~bStatG6;
	bprevStatG7 = ~bStatG7;
	bprevStatG8 = ~bStatG8;
	bprevStatG6h = ~bStatG6h;
	bprevStatG7h = ~bStatG7h;
	bprevStatG8h = ~bStatG8h;
	pwr_batprevlevp = 255;
	pwr_batprevlevv = ~pwr_batlevv;
	pwr_batprevlevc = ~pwr_batlevc;
	pwr_batprevmode = 255;

	fgpio1 = 1;
	fgpio2 = 1;
	fgpio3 = 1;
	fgpio4 = 1;
	fgpio5 = 1;
	for (int i = 0; i < BleMonNum; i++) {
	BleMX[i].prstate =255;
	BleMX[i].prrssi = 255;
	BleMX[i].cmrssi = 0;
	BleMX[i].gtnum = 255;
	BleMX[i].ppar1 = ~BleMX[i].par1;
	BleMX[i].ppar2 = ~BleMX[i].par2;
	BleMX[i].ppar3 = ~BleMX[i].par3;
	BleMX[i].ppar4 = ~BleMX[i].par4;
	BleMX[i].ppar5 = ~BleMX[i].par5;
	BleMX[i].ppar6 = ~BleMX[i].par6;
	BleMX[i].ppar7 = ~BleMX[i].par7;
	}
	for (int i = 0; i < 28; i++) {
	SnPi2c[i].ppar1 = ~SnPi2c[i].par1;
	SnPi2c[i].ppar2 = ~SnPi2c[i].par2;
	SnPi2c[i].ppar3 = ~SnPi2c[i].par3;
	SnPi2c[i].ppar4 = ~SnPi2c[i].par4;
/*
	SnPi2c[i].ppar5 = ~SnPi2c[i].par5;
	SnPi2c[i].ppar6 = ~SnPi2c[i].par6;
	SnPi2c[i].ppar7 = ~SnPi2c[i].par7;
	SnPi2c[i].ppar8 = ~SnPi2c[i].par8;
*/
	}

	char llwtt[128];
	char llwtd[512];
	strcpy(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/status");
	esp_mqtt_client_publish(client, llwtt, "online", 0, 1, 1);
//	msg_id = esp_mqtt_client_publish(client, llwtt, "online", 0, 1, 1);
//	if (fdebug) ESP_LOGI(AP_TAG,"sent publish successful, msg_id=%d", msg_id);
	tcpip_adapter_ip_info_t ipInfo;
	char wbuff[256];
	memset(wbuff,0,32);
	tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo);
	sprintf(wbuff, "%d.%d.%d.%d", IP2STR(&ipInfo.ip));
	strcpy(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/ip");
	esp_mqtt_client_publish(client, llwtt, wbuff, 0, 1, 1);
#ifdef USE_TFT
	if (MQTT_TOPP1[0]) esp_mqtt_client_subscribe(client, MQTT_TOPP1, 0);
	if (MQTT_TOPP2[0]) esp_mqtt_client_subscribe(client, MQTT_TOPP2, 0);
	if (MQTT_TOPP3[0]) esp_mqtt_client_subscribe(client, MQTT_TOPP3, 0);
	if (MQTT_TOPP4[0]) esp_mqtt_client_subscribe(client, MQTT_TOPP4, 0);
	if (MQTT_TOPP5[0]) esp_mqtt_client_subscribe(client, MQTT_TOPP5, 0);
	if (MQTT_TOPP6[0]) esp_mqtt_client_subscribe(client, MQTT_TOPP6, 0);
	if (MQTT_TOPP7[0]) esp_mqtt_client_subscribe(client, MQTT_TOPP7, 0);

	if (tft_conn) {
	strcpy(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/jpg_url");
	esp_mqtt_client_subscribe(client, llwtt, 0);
	strcpy(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/jpg_time");
	esp_mqtt_client_subscribe(client, llwtt, 0);
	}
#endif
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
	strcat(llwtd,"\",\"connections\":[[\"mac\",\"");
	strcat(llwtd,tESP32Addr1);
//	if (wbuff[0]) {
//	strcat(llwtd,"\"],[\"ip\",\"");
//	strcat(llwtd,wbuff);
//	}
	strcat(llwtd,"\"]],\"manufacturer\":\"Espressif\"},\"device_class\":\"signal_strength\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/rssi\",\"unit_of_meas\":\"dBm\"");
	strcat(llwtd,",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
#ifdef USE_TFT
	if (tft_conn) {
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
	strcat(llwtd,"\",\"connections\":[[\"mac\",\"");
	strcat(llwtd,tESP32Addr1);
//	if (wbuff[0]) {
//	strcat(llwtd,"\"],[\"ip\",\"");
//	strcat(llwtd,wbuff);
//	}
	strcat(llwtd,"\"]],\"manufacturer\":\"Espressif\"},\"command_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/screen\",\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/screen\",\"min\":\"0\",\"max\":\"255\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
//
	strcpy(llwtt,"homeassistant/number/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/jtx");
	strcat(llwtt,tESP32Addr);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate.jpg.time\",\"icon\":\"mdi:image-check-outline\",\"uniq_id\":\"jpgtime_");
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
	strcat(llwtd,"\",\"connections\":[[\"mac\",\"");
	strcat(llwtd,tESP32Addr1);
//	if (wbuff[0]) {
//	strcat(llwtd,"\"],[\"ip\",\"");
//	strcat(llwtd,wbuff);
//	}
	strcat(llwtd,"\"]],\"manufacturer\":\"Espressif\"},\"command_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/jpg_time\",\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/jpg_time\",\"min\":\"0\",\"max\":\"65535\",\"retain\":\"true\",\"unit_of_meas\":\"s\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
//
	strcpy(llwtt,"homeassistant/text/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/jux");
	strcat(llwtt,tESP32Addr);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate.jpg.url\",\"icon\":\"mdi:image-edit-outline\",\"uniq_id\":\"jpgurl_");
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
	strcat(llwtd,"\",\"connections\":[[\"mac\",\"");
	strcat(llwtd,tESP32Addr1);
//	if (wbuff[0]) {
//	strcat(llwtd,"\"],[\"ip\",\"");
//	strcat(llwtd,wbuff);
//	}
	strcat(llwtd,"\"]],\"manufacturer\":\"Espressif\"},\"command_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/jpg_url\",\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/jpg_url\",\"retain\":\"true\",\"min\":\"0\",\"max\":\"255\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
//
	}
	strcpy(llwtt,"homeassistant/button/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/2x");
	strcat(llwtt,tESP32Addr);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate.jpg.refresh\",\"icon\":\"mdi:image-auto-adjust\",\"uniq_id\":\"jpgrefr_");
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
	strcat(llwtd,"\",\"connections\":[[\"mac\",\"");
	strcat(llwtd,tESP32Addr1);
//	if (wbuff[0]) {
//	strcat(llwtd,"\"],[\"ip\",\"");
//	strcat(llwtd,wbuff);
//	}
	strcat(llwtd,"\"]],\"manufacturer\":\"Espressif\"},\"command_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/jpg_time\",\"payload_press\":\"65536\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
#endif
	strcpy(llwtt,"homeassistant/button/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/1x");
	strcat(llwtt,tESP32Addr);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate.restart\",\"icon\":\"mdi:restart\",\"uniq_id\":\"restart_");
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
	strcat(llwtd,"\",\"connections\":[[\"mac\",\"");
	strcat(llwtd,tESP32Addr1);
//	if (wbuff[0]) {
//	strcat(llwtd,"\"],[\"ip\",\"");
//	strcat(llwtd,wbuff);
//	}
	strcat(llwtd,"\"]],\"manufacturer\":\"Espressif\"},\"command_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/screen\",\"payload_press\":\"restart\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);



//gpio 1 - 5
	for (int i = 1; i < 6; i++) {
	uint8_t tbgpio;
	char tbuff[16];
	switch (i) {
	break;
	case 2:
	tbgpio = bgpio2;
	break;
	case 3:
	tbgpio = bgpio3;
	break;
	case 4:
	tbgpio = bgpio4;
	break;
	case 5:
	tbgpio = bgpio5;
	break;
	default:
	tbgpio = bgpio1;
	}
	if ((tbgpio > 63) && (tbgpio < (MxPOutP + 64))) {
	strcpy(llwtt,"homeassistant/switch/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/");
	itoa(i + 2,tbuff,10);
	strcat(llwtt,tbuff);
	strcat(llwtt,"x");
	strcat(llwtt,tESP32Addr);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate.gpio");
	itoa(i,tbuff,10);
	strcat(llwtd,tbuff);
	strcat(llwtd,"\",\"icon\":\"mdi:electric-switch\",\"uniq_id\":\"gpio");
	itoa(i,tbuff,10);
	strcat(llwtd,tbuff);
	strcat(llwtd,"_");
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
	strcat(llwtd,"\",\"connections\":[[\"mac\",\"");
	strcat(llwtd,tESP32Addr1);
//	if (wbuff[0]) {
//	strcat(llwtd,"\"],[\"ip\",\"");
//	strcat(llwtd,wbuff);
//	}
	strcat(llwtd,"\"]],\"manufacturer\":\"Espressif\"},\"command_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/gpio");
	itoa(i,tbuff,10);
	strcat(llwtd,tbuff);
	strcat(llwtd,"\",\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/gpio");
	itoa(i,tbuff,10);
	strcat(llwtd,tbuff);
	strcat(llwtd,"\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	} else if ((tbgpio > 127) && (tbgpio < 192)) {
	strcpy(llwtt,"homeassistant/binary_sensor/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/");
	itoa(i + 2,tbuff,10);
	strcat(llwtt,tbuff);
	strcat(llwtt,"x");
	strcat(llwtt,tESP32Addr);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate.gpio");
	itoa(i,tbuff,10);
	strcat(llwtd,tbuff);
	strcat(llwtd,"\",\"icon\":\"mdi:electric-switch\",\"uniq_id\":\"gpio");
	itoa(i,tbuff,10);
	strcat(llwtd,tbuff);
	strcat(llwtd,"_");
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
	strcat(llwtd,"\",\"connections\":[[\"mac\",\"");
	strcat(llwtd,tESP32Addr1);
//	if (wbuff[0]) {
//	strcat(llwtd,"\"],[\"ip\",\"");
//	strcat(llwtd,wbuff);
//	}
	strcat(llwtd,"\"]],\"manufacturer\":\"Espressif\"},\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/gpio");
	itoa(i,tbuff,10);
	strcat(llwtd,tbuff);
	strcat(llwtd,"\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	}
//
	} //for i1-5

//gpio 6 - 8
	for (int i = 6; i < 9; i++) {
	uint8_t tbgpio;
	char tbuff[16];
	switch (i) {
	break;
	case 7:
	tbgpio = bgpio7;
	break;
	case 8:
	tbgpio = bgpio8;
	break;
	default:
	tbgpio = bgpio6;
	}
	if ((tbgpio > 63) && (tbgpio < (MxPOutP + 64))) {
	strcpy(llwtt,"homeassistant/number/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/");
	itoa(i + 2,tbuff,10);
	strcat(llwtt,tbuff);
	strcat(llwtt,"x");
	strcat(llwtt,tESP32Addr);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate.pwm");
	itoa(i,tbuff,10);
	strcat(llwtd,tbuff);
	strcat(llwtd,"\",\"icon\":\"mdi:pulse\",\"uniq_id\":\"pwm");
	itoa(i,tbuff,10);
	strcat(llwtd,tbuff);
	strcat(llwtd,"_");
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
	strcat(llwtd,"\",\"connections\":[[\"mac\",\"");
	strcat(llwtd,tESP32Addr1);
//	if (wbuff[0]) {
//	strcat(llwtd,"\"],[\"ip\",\"");
//	strcat(llwtd,wbuff);
//	}
	strcat(llwtd,"\"]],\"manufacturer\":\"Espressif\"},\"command_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/gpio");
	itoa(i,tbuff,10);
	strcat(llwtd,tbuff);
	strcat(llwtd,"\",\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/gpio");
	itoa(i,tbuff,10);
	strcat(llwtd,tbuff);
	strcat(llwtd,"\",\"min\":\"0\",\"max\":\"255\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	} else if ((tbgpio > 127) && (tbgpio < 192)) {
	strcpy(llwtt,"homeassistant/sensor/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/");
	itoa(i + 2,tbuff,10);
	strcat(llwtt,tbuff);
	strcat(llwtt,"x");
	strcat(llwtt,tESP32Addr);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	if ((i == 6) && (bgpio5 > 191)) {
	strcat(llwtd,".Gate.Hx");
	itoa(i,tbuff,10);
	strcat(llwtd,tbuff);
	strcat(llwtd,".weight\",\"icon\":\"mdi:weight-kilogram\",\"uniq_id\":\"Hx");
	itoa(i,tbuff,10);
	strcat(llwtd,tbuff);
	strcat(llwtd,"_weight_");
	} else {
	strcat(llwtd,".Gate.1w");
	itoa(i,tbuff,10);
	strcat(llwtd,tbuff);
	strcat(llwtd,".temp\",\"icon\":\"mdi:thermometer\",\"uniq_id\":\"1w");
	itoa(i,tbuff,10);
	strcat(llwtd,tbuff);
	strcat(llwtd,"_temp_");
	}
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
	strcat(llwtd,"\",\"connections\":[[\"mac\",\"");
	strcat(llwtd,tESP32Addr1);
//	if (wbuff[0]) {
//	strcat(llwtd,"\"],[\"ip\",\"");
//	strcat(llwtd,wbuff);
//	}
	strcat(llwtd,"\"]],\"manufacturer\":\"Espressif\"},\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	if ((i == 6) && (bgpio5 > 191)) {
	strcat(llwtd,"/Hx");
	itoa(i,tbuff,10);
	strcat(llwtd,tbuff);
	strcat(llwtd,"\",\"device_class\":\"weight\",\"unit_of_meas\":\"");
	if (bDivHx6 & 0x80000000) strcat(llwtd,"\x25\"");
	else strcat(llwtd,"kg\"");
	} else {
	strcat(llwtd,"/1w");
	itoa(i,tbuff,10);
	strcat(llwtd,tbuff);
	strcat(llwtd,"\",\"device_class\":\"temperature\",\"unit_of_meas\":\"\xc2\xb0\x43\"");
	}
	strcat(llwtd,",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
#ifdef USE_IRTX
	} else if ((i == 6) && (tbgpio > 191) &&  (tbgpio < (MxPOutP + 192))) {
	strcpy(llwtt,"homeassistant/select/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/");
	itoa(i + 2,tbuff,10);
	strcat(llwtt,tbuff);
	strcat(llwtt,"itx");
	strcat(llwtt,tESP32Addr);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate.ir");
	itoa(i,tbuff,10);
	strcat(llwtd,tbuff);
	strcat(llwtd,"tx\",\"icon\":\"mdi:remote\",\"uniq_id\":\"ir");
	itoa(i,tbuff,10);
	strcat(llwtd,tbuff);
	strcat(llwtd,"tx_");
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
	strcat(llwtd,"\",\"connections\":[[\"mac\",\"");
	strcat(llwtd,tESP32Addr1);
//	if (wbuff[0]) {
//	strcat(llwtd,"\"],[\"ip\",\"");
//	strcat(llwtd,wbuff);
//	}
	strcat(llwtd,"\"]],\"manufacturer\":\"Espressif\"},\"command_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/ir");
	itoa(i,tbuff,10);
	strcat(llwtd,tbuff);
	strcat(llwtd,"tx\",\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/ir");
	itoa(i,tbuff,10);
	strcat(llwtd,tbuff);
	strcat(llwtd,"tx\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\",\"options\":");
	strcat(llwtd,"[\"OFF\",\"NEC\",\"NECx16\",\"RC5\",\"RC6\",\"SAMSUNG\",\"SIRCx12\",\"SIRCx15\",\"SIRCx20\",\"PANASONIC\"]}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
//
	strcpy(llwtt,"homeassistant/number/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/");
	itoa(i + 2,tbuff,10);
	strcat(llwtt,tbuff);
	strcat(llwtt,"iax");
	strcat(llwtt,tESP32Addr);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate.ir");
	itoa(i,tbuff,10);
	strcat(llwtd,tbuff);
	strcat(llwtd,"addr\",\"icon\":\"mdi:directions-fork\",\"uniq_id\":\"ir");
	itoa(i,tbuff,10);
	strcat(llwtd,tbuff);
	strcat(llwtd,"addr_");
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
	strcat(llwtd,"\",\"connections\":[[\"mac\",\"");
	strcat(llwtd,tESP32Addr1);
//	if (wbuff[0]) {
//	strcat(llwtd,"\"],[\"ip\",\"");
//	strcat(llwtd,wbuff);
//	}
	strcat(llwtd,"\"]],\"manufacturer\":\"Espressif\"},\"command_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/ir");
	itoa(i,tbuff,10);
	strcat(llwtd,tbuff);
	strcat(llwtd,"addr\",\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/ir");
	itoa(i,tbuff,10);
	strcat(llwtd,tbuff);
	strcat(llwtd,"addr\",\"mode\":\"box\",\"min\":\"0\",\"max\":\"65535\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
//
	strcpy(llwtt,"homeassistant/number/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/");
	itoa(i + 2,tbuff,10);
	strcat(llwtt,tbuff);
	strcat(llwtt,"icx");
	strcat(llwtt,tESP32Addr);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate.ir");
	itoa(i,tbuff,10);
	strcat(llwtd,tbuff);
	strcat(llwtd,"cmd\",\"icon\":\"mdi:keyboard\",\"uniq_id\":\"ir");
	itoa(i,tbuff,10);
	strcat(llwtd,tbuff);
	strcat(llwtd,"cmd_");
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
	strcat(llwtd,"\",\"connections\":[[\"mac\",\"");
	strcat(llwtd,tESP32Addr1);
//	if (wbuff[0]) {
//	strcat(llwtd,"\"],[\"ip\",\"");
//	strcat(llwtd,wbuff);
//	}
	strcat(llwtd,"\"]],\"manufacturer\":\"Espressif\"},\"command_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/ir");
	itoa(i,tbuff,10);
	strcat(llwtd,tbuff);
	strcat(llwtd,"cmd\",\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/ir");
	itoa(i,tbuff,10);
	strcat(llwtd,tbuff);
	strcat(llwtd,"cmd\",\"mode\":\"box\",\"min\":\"0\",\"max\":\"255\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
//
#endif
	} else if ((tbgpio > 191) &&  (tbgpio < (MxPOutP + 192))) {
	strcpy(llwtt,"homeassistant/sensor/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/");
	itoa(i + 2,tbuff,10);
	strcat(llwtt,tbuff);
	strcat(llwtt,"tx");
	strcat(llwtt,tESP32Addr);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate.dht");
	itoa(i,tbuff,10);
	strcat(llwtd,tbuff);
	strcat(llwtd,".temp\",\"icon\":\"mdi:thermometer\",\"uniq_id\":\"dht");
	itoa(i,tbuff,10);
	strcat(llwtd,tbuff);
	strcat(llwtd,"_temp_");
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
	strcat(llwtd,"\",\"connections\":[[\"mac\",\"");
	strcat(llwtd,tESP32Addr1);
//	if (wbuff[0]) {
//	strcat(llwtd,"\"],[\"ip\",\"");
//	strcat(llwtd,wbuff);
//	}
	strcat(llwtd,"\"]],\"manufacturer\":\"Espressif\"},\"device_class\":\"temperature\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/dht");
	itoa(i,tbuff,10);
	strcat(llwtd,tbuff);
	strcat(llwtd,"t\",\"unit_of_meas\":\"\xc2\xb0\x43\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
//
	strcpy(llwtt,"homeassistant/sensor/");
	strcat(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/");
	itoa(i + 2,tbuff,10);
	strcat(llwtt,tbuff);
	strcat(llwtt,"hx");
	strcat(llwtt,tESP32Addr);
	strcat(llwtt,"/config");
	llwtd[0] = 0;
//	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
	strcpy(llwtd,"{\"name\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,".Gate.dht");
	itoa(i,tbuff,10);
	strcat(llwtd,tbuff);
	strcat(llwtd,".humid\",\"icon\":\"mdi:water-percent\",\"uniq_id\":\"dht");
	itoa(i,tbuff,10);
	strcat(llwtd,tbuff);
	strcat(llwtd,"_humid_");
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
	strcat(llwtd,"\",\"connections\":[[\"mac\",\"");
	strcat(llwtd,tESP32Addr1);
//	if (wbuff[0]) {
//	strcat(llwtd,"\"],[\"ip\",\"");
//	strcat(llwtd,wbuff);
//	}
	strcat(llwtd,"\"]],\"manufacturer\":\"Espressif\"},\"device_class\":\"humidity\",\"state_class\":\"measurement\",\"state_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/dht");
	itoa(i,tbuff,10);
	strcat(llwtd,tbuff);
	strcat(llwtd,"h\",\"unit_of_meas\":\"\x25\",\"availability_topic\":\"");
	strcat(llwtd,MQTT_BASE_TOPIC);
	strcat(llwtd,"/status\"}");
	esp_mqtt_client_publish(mqttclient, llwtt, llwtd, 0, 1, 1);
//
	}
	} //for i6-8
	} //hass

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
	if ((bgpio6 > 63) && (bgpio6 < (MxPOutP + 64))) {
	strcpy(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/gpio6");
	esp_mqtt_client_subscribe(mqttclient, llwtt, 0);
#ifdef USE_IRTX
	} else if ((bgpio6 > 191) && (bgpio6 < (MxPOutP + 192))) {
	strcpy(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/ir6tx");
	esp_mqtt_client_subscribe(mqttclient, llwtt, 0);
	strcpy(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/ir6addr");
	esp_mqtt_client_subscribe(mqttclient, llwtt, 0);
	strcpy(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/ir6cmd");
	esp_mqtt_client_subscribe(mqttclient, llwtt, 0);
	strcpy(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/ir6code");
	esp_mqtt_client_subscribe(mqttclient, llwtt, 0);
#endif
	}
	if ((bgpio7 > 63) && (bgpio7 < (MxPOutP + 64))) {
	strcpy(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/gpio7");
	esp_mqtt_client_subscribe(mqttclient, llwtt, 0);
	}
	if ((bgpio8 > 63) && (bgpio8 < (MxPOutP + 64))) {
	strcpy(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/gpio8");
	esp_mqtt_client_subscribe(mqttclient, llwtt, 0);
	}
	MqttPubSub(0, true);	
	MqttPubSub(1, true);	
	MqttPubSub(2, true);	
	MqttPubSub(3, true);	
	MqttPubSub(4, true);	
	HDiscBlemon(true);
	if (HDisci2c(&f_i2cdev)) i2cdevnumo = i2cdevnum;
	mqttConnected = true;
	NumMqConn++;
	if (!NumMqConn) NumMqConn--;
	break;

	case MQTT_EVENT_DISCONNECTED:
	mqttConnected = false;
	BleDevStA.t_ppcon = 40;
	BleDevStB.t_ppcon = 40;
	BleDevStC.t_ppcon = 40;
	BleDevStD.t_ppcon = 40;
	BleDevStE.t_ppcon = 40;

	BleDevStA.iprevRssi = 0;
	BleDevStB.iprevRssi = 0;
	BleDevStC.iprevRssi = 0;
	BleDevStD.iprevRssi = 0;
	BleDevStE.iprevRssi = 0;

	BleDevStA.cprevStatus[0] = 0;
	BleDevStB.cprevStatus[0] = 0;
	BleDevStC.cprevStatus[0] = 0;
	BleDevStD.cprevStatus[0] = 0;
	BleDevStE.cprevStatus[0] = 0;

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
	BleDevStD.bprevLock = 255;
	BleDevStD.bprevState = 255;
	BleDevStD.bprevHeat = 255;
	BleDevStD.bprevStNl = 255;
	BleDevStD.bprevCtemp = 255;
	BleDevStD.bprevHtemp = 255;
	BleDevStD.bprevProg = 254;
	BleDevStD.bprevModProg = 255;
	BleDevStD.bprevPHour = 255;
	BleDevStD.bprevPMin = 255;
	BleDevStD.bprevCHour = 255;
	BleDevStD.bprevCMin = 255;
	BleDevStD.bprevDHour = 255;
	BleDevStD.bprevDMin = 255;
	BleDevStD.bprevAwarm = 255;
	BleDevStD.PRgbR = ~BleDevStD.RgbR;
	BleDevStD.PRgbG = ~BleDevStD.RgbG;
	BleDevStD.PRgbB = ~BleDevStD.RgbB;
	BleDevStD.bprevStBl = 255;
	BleDevStD.bprevStBp = 255;
	BleDevStE.bprevLock = 255;
	BleDevStE.bprevState = 255;
	BleDevStE.bprevHeat = 255;
	BleDevStE.bprevStNl = 255;
	BleDevStE.bprevCtemp = 255;
	BleDevStE.bprevHtemp = 255;
	BleDevStE.bprevProg = 254;
	BleDevStE.bprevModProg = 255;
	BleDevStE.bprevPHour = 255;
	BleDevStE.bprevPMin = 255;
	BleDevStE.bprevCHour = 255;
	BleDevStE.bprevCMin = 255;
	BleDevStE.bprevDHour = 255;
	BleDevStE.bprevDMin = 255;
	BleDevStE.bprevAwarm = 255;
	BleDevStE.PRgbR = ~BleDevStE.RgbR;
	BleDevStE.PRgbG = ~BleDevStE.RgbG;
	BleDevStE.PRgbB = ~BleDevStE.RgbB;
	BleDevStE.bprevStBl = 255;
	BleDevStE.bprevStBp = 255;

	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_EVENT_DISCONNECTED");
	break;

        case MQTT_EVENT_SUBSCRIBED:
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
	break;
	case MQTT_EVENT_UNSUBSCRIBED:
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_EVENT_UNSUBSCRIBED, msg_id=%d\n", event->msg_id);
	break;
	case MQTT_EVENT_PUBLISHED:
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
	break;
	case MQTT_EVENT_DATA:
/*
	if (fdebug) {
	ESP_LOGI(AP_TAG,"MQTT_EVENT_DATA");
	ESP_LOGI(AP_TAG,"TOPIC=%.*s", event->topic_len, event->topic);
	ESP_LOGI(AP_TAG,"DATA=%.*s", event->data_len, event->data);
	}
*/
       	if (mqtdel && (event->topic_len) && (event->topic_len < 128)) {
	char ttopic[128];
	memset(ttopic,0,128);
	memcpy(ttopic, event->topic, event->topic_len);
        mqtdel = 20;
	if (event->data_len) esp_mqtt_client_publish(mqttclient, ttopic, "", 0, 1, 1);
	} else if (event->data_len && (event->data[0] != 0x2e) && (event->data_len < 384) && (event->topic_len) && (event->topic_len < 64)) {
	int topoffa = 0;
	int topoffb = 0;
	int topoffc = 0;
	int topoffd = 0;
	int topoffe = 0;
	int topoffs = 0;
#ifdef USE_IRTX
	int topoffi = 0;
#endif
#ifdef USE_TFT
	int topoffj = 0;
#endif
	char tbuff[64];
	char ttopic[64];
	memset(ttopic,0,64);
	memcpy(ttopic, event->topic, event->topic_len);
//if (fdebug) ESP_LOGI(AP_TAG,"mqtopic=%s", ttopic);
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
	if (BleDevStD.tBLEAddr[0]) {
	strcpy(tbuff,MQTT_BASE_TOPIC);
	strcat(tbuff,"/");
	strcat(tbuff,BleDevStD.tBLEAddr);
	if (!fcommtp) strcat(tbuff,"/cmd");
	strcat(tbuff,"/");
	topoffd = parsoff(event->topic,tbuff, event->topic_len);
	}
	if (BleDevStE.tBLEAddr[0]) {
	strcpy(tbuff,MQTT_BASE_TOPIC);
	strcat(tbuff,"/");
	strcat(tbuff,BleDevStE.tBLEAddr);
	if (!fcommtp) strcat(tbuff,"/cmd");
	strcat(tbuff,"/");
	topoffe = parsoff(event->topic,tbuff, event->topic_len);
	}
	strcpy(tbuff,MQTT_BASE_TOPIC);
	strcat(tbuff,"/gpio");
	topoffs = parsoff(event->topic,tbuff, event->topic_len);
#ifdef USE_IRTX
	strcpy(tbuff,MQTT_BASE_TOPIC);
	strcat(tbuff,"/ir6");
	topoffi = parsoff(event->topic,tbuff, event->topic_len);
#endif
#ifdef USE_TFT
	strcpy(tbuff,MQTT_BASE_TOPIC);
	strcat(tbuff,"/jpg_");
	topoffj = parsoff(event->topic,tbuff, event->topic_len);
#endif
//
	strcpy(tbuff,MQTT_BASE_TOPIC);
	strcat(tbuff,"/screen");
	if (!memcmp(event->topic, tbuff, event->topic_len)) {
	if ((!incascmp("restart",event->data,event->data_len)) || (!incascmp("reset",event->data,event->data_len))
		|| (!incascmp("reboot",event->data,event->data_len))) {
	esp_restart();
#ifdef USE_TFT
	} else if (event->data_len && (event->data_len < 5)){
	uint16_t duty = 255;
	mystrcpy(tbuff, event->data, event->data_len);
	duty = atoi(tbuff);
	if (duty > 255) duty = 255;
	if ((duty != bStateS) || (!r4sppcoms)) { 
	if (tft_conn) bStateS = duty;
	else if (!bStateS) bStateS = duty;
	bprevStateS = ~bStateS;
	t_lasts = 0;
	}
#endif
	}

	} else if (topoffs) {
	if ((!memcmp(event->topic+topoffs, "1", event->topic_len-topoffs)) && (bgpio1 > 63)) {
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!lvgpio1) || (!r4sppcoms) || (inccmp(strON,event->data,event->data_len))) {
	if (bgpio1 < (MxPOutP + 64)) {
	gpio_set_level((bgpio1 & 0x3f), 1);
	lvgpio1 = 1;
			}
	fgpio1 = 1;
	t_lasts = 0;
		}
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((lvgpio1)  || (!r4sppcoms) || (inccmp(strOFF,event->data,event->data_len))) {
	if (bgpio1 < (MxPOutP + 64)) {
	gpio_set_level((bgpio1 & 0x3f), 0);
	lvgpio1 = 0;
			}
	fgpio1 = 1;
	t_lasts = 0;
		}	
	}
	} else if ((!memcmp(event->topic+topoffs, "2", event->topic_len-topoffs)) && (bgpio2 > 63)) {
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!lvgpio2) || (!r4sppcoms) || (inccmp(strON,event->data,event->data_len))) {
	if (bgpio2 < (MxPOutP + 64)) {
	gpio_set_level((bgpio2 & 0x3f), 1);
	lvgpio2 = 1;
			}
	fgpio2 = 1;
	t_lasts = 0;
		}
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((lvgpio2)  || (!r4sppcoms) || (inccmp(strOFF,event->data,event->data_len))) {
	if (bgpio2 < (MxPOutP + 64)) {
	gpio_set_level((bgpio2 & 0x3f), 0);
	lvgpio2 = 0;
			}
	fgpio2 = 1;
	t_lasts = 0;
		}	
	}
	} else if ((!memcmp(event->topic+topoffs, "3", event->topic_len-topoffs)) && (bgpio3 > 63)) {
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!lvgpio3) || (!r4sppcoms) || (inccmp(strON,event->data,event->data_len))) {
	if (bgpio3 < (MxPOutP + 64)) {
	gpio_set_level((bgpio3 & 0x3f), 1);
	lvgpio3 = 1;
			}
	fgpio3 = 1;
	t_lasts = 0;
		}
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((lvgpio3)  || (!r4sppcoms) || (inccmp(strOFF,event->data,event->data_len))) {
	if (bgpio3 < (MxPOutP + 64)) {
	gpio_set_level((bgpio3 & 0x3f), 0);
	lvgpio3 = 0;
			}
	fgpio3 = 1;
	t_lasts = 0;
		}	
	}
	} else if ((!memcmp(event->topic+topoffs, "4", event->topic_len-topoffs)) && (bgpio4 > 63)) {
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!lvgpio4) || (!r4sppcoms) || (inccmp(strON,event->data,event->data_len))) {
	if (bgpio4 < (MxPOutP + 64)) {
	gpio_set_level((bgpio4 & 0x3f), 1);
	lvgpio4 = 1;
			}
	fgpio4 = 1;
	t_lasts = 0;
		}
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((lvgpio4)  || (!r4sppcoms) || (inccmp(strOFF,event->data,event->data_len))) {
	if (bgpio4 < (MxPOutP + 64)) {
	gpio_set_level((bgpio4 & 0x3f), 0);
	lvgpio4 = 0;
			}
	fgpio4 = 1;
	t_lasts = 0;
		}	
	}
	} else if ((!memcmp(event->topic+topoffs, "5", event->topic_len-topoffs)) && (bgpio5 > 63)) {
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	if ((!lvgpio5) || (!r4sppcoms) || (inccmp(strON,event->data,event->data_len))) {
	if (bgpio5 < (MxPOutP + 64)) {
	gpio_set_level((bgpio5 & 0x3f), 1);
	lvgpio5 = 1;
			}
	fgpio5 = 1;
	t_lasts = 0;
		}
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	if ((lvgpio5)  || (!r4sppcoms) || (inccmp(strOFF,event->data,event->data_len))) {
	if (bgpio5 < (MxPOutP + 64)) {
	gpio_set_level((bgpio5 & 0x3f), 0);
	lvgpio5 = 0;
			}
	fgpio5 = 1;
	t_lasts = 0;
		}	
	}
	} else if ((!memcmp(event->topic+topoffs, "6", event->topic_len-topoffs)) && (bgpio6 > 63)) {
	if (bgpio6 < (MxPOutP + 64)) {
	if (event->data_len && (event->data_len < 5)){
	uint16_t duty = 255;
	mystrcpy(tbuff, event->data, event->data_len);
	duty = atoi(tbuff);
	if (duty > 255) duty = 255;
	if ((duty != bStatG6) || (!r4sppcoms)) { 
	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, duty);
	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
        bStatG6 = duty;
	}
	}
	t_lasts = 0;
	}
	} else if ((!memcmp(event->topic+topoffs, "7", event->topic_len-topoffs)) && (bgpio7 > 63)) {
	if (bgpio7 < (MxPOutP + 64)) {
	if (event->data_len && (event->data_len < 5)){
	uint16_t duty = 255;
	mystrcpy(tbuff, event->data, event->data_len);
	duty = atoi(tbuff);
	if (duty > 255) duty = 255;
	if ((duty != bStatG7) || (!r4sppcoms)) { 
	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, duty);
	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);
        bStatG7 = duty;
	}
	}
	t_lasts = 0;
	}
	} else if ((!memcmp(event->topic+topoffs, "8", event->topic_len-topoffs)) && (bgpio8 > 63)) {
	if (bgpio8 < (MxPOutP + 64)) {
	if (event->data_len && (event->data_len < 5)){
	uint16_t duty = 255;
	mystrcpy(tbuff, event->data, event->data_len);
	duty = atoi(tbuff);
	if (duty > 255) duty = 255;
	if ((duty != bStatG8) || (!r4sppcoms)) { 
	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_3, duty);
	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_3);
        bStatG8 = duty;
	}
	}
	t_lasts = 0;
	}
	} //gpio
	} else if (topoffa && BleDevStA.DEV_TYP) {
//if (fdebug) ESP_LOGI(AP_TAG,"topoffa=%d", topoffa);
	if (!BleDevStA.t_ppcon) BleMqtPr(0,  topoffa, event->topic, event->topic_len, event->data, event->data_len);

	} else if (topoffb && BleDevStB.DEV_TYP) {
//if (fdebug) ESP_LOGI(AP_TAG,"topoffb=%d", topoffb);
	if (!BleDevStB.t_ppcon) BleMqtPr(1,  topoffb, event->topic, event->topic_len, event->data, event->data_len);

	} else if (topoffc && BleDevStC.DEV_TYP) {
//if (fdebug) ESP_LOGI(AP_TAG,"topoffc=%d", topoffc);
	if (!BleDevStC.t_ppcon) BleMqtPr(2,  topoffc, event->topic, event->topic_len, event->data, event->data_len);

	} else if (topoffd && BleDevStD.DEV_TYP) {
//if (fdebug) ESP_LOGI(AP_TAG,"topoffd=%d", topoffd);
	if (!BleDevStD.t_ppcon) BleMqtPr(3,  topoffd, event->topic, event->topic_len, event->data, event->data_len);

	} else if (topoffe && BleDevStE.DEV_TYP) {
//if (fdebug) ESP_LOGI(AP_TAG,"topoffe=%d", topoffe);
	if (!BleDevStE.t_ppcon) BleMqtPr(4,  topoffe, event->topic, event->topic_len, event->data, event->data_len);
#ifdef USE_TFT
	} else if ((MQTT_TOPP1[0]) && (!memcmp(event->topic, MQTT_TOPP1, event->topic_len))) {
	int tempsz = event->data_len;
	if  (tempsz > 15) tempsz = 15;
	mystrcpy(MQTT_VALP1, event->data, tempsz);
	} else if ((MQTT_TOPP2[0] && !memcmp(event->topic, MQTT_TOPP2, event->topic_len)) ||
		(MQTT_TOPP3[0] && !memcmp(event->topic, MQTT_TOPP3, event->topic_len))) {
	if ((MQTT_TOPP2[0]) && (!memcmp(event->topic, MQTT_TOPP2, event->topic_len))) {
	if  (event->data_len < 10) mystrcpy(MQTT_VALP2, event->data, event->data_len);
	else {
	int datoff = 0;
	datoff = parsoff(event->data,"\"Voltage\":", event->data_len);
	myvalcpy(MQTT_VALP2, event->data + datoff, 15);
	}
	} 
	if ((MQTT_TOPP3[0]) && (!memcmp(event->topic, MQTT_TOPP3, event->topic_len))) {
	if  (event->data_len < 10) mystrcpy(MQTT_VALP3, event->data, event->data_len);
	else {
	int datoff = 0;
	datoff = parsoff(event->data,"\"Current\":", event->data_len);
	myvalcpy(MQTT_VALP3, event->data + datoff, 15);
	}
	}
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
#ifdef USE_IRTX
	} else if (topoffi) {
	if (!memcmp(event->topic+topoffi, "tx", event->topic_len-topoffi)) {
	uint16_t tmp = 0;
	if (!incascmp("off", event->data, event->data_len)) tmp = 0; 
	else if (!incascmp("nec", event->data, event->data_len)) tmp = 1;
	else if (!incascmp("necx16", event->data, event->data_len)) tmp = 2;
	else if (!incascmp("rc5", event->data, event->data_len)) tmp = 3;
	else if (!incascmp("rc6", event->data, event->data_len)) tmp = 4;
	else if (!incascmp("samsung", event->data, event->data_len)) tmp = 5;
	else if (!incascmp("sircx12", event->data, event->data_len)) tmp = 6;
	else if (!incascmp("sircx15", event->data, event->data_len)) tmp = 7;
	else if (!incascmp("sircx20", event->data, event->data_len)) tmp = 8;
	else if (!incascmp("panasonic", event->data, event->data_len)) tmp = 9;
	if (tmp && !(bStatG6 & 0xff00)) {
	bStatG6 = (bStatG6 & 0xff) | ((tmp << 8) & 0xff00);
	bprevStatG6 = bprevStatG6 | 0xff00;
	t_lasts = 0;
	} else if (!r4sppcoms) {
	bprevStatG6 = bprevStatG6 | 0xff00;
	t_lasts = 0;
	}
	} else if (!memcmp(event->topic+topoffi, "addr", event->topic_len-topoffi)) {
	if (event->data_len && (event->data_len < 6) && (event->data[0] > 0x2f) && (event->data[0] < 0x3a)) {
	uint32_t tmp = 0;
	mystrcpy(tbuff, event->data, event->data_len);
	tmp = atoi(tbuff);
	if (tmp < 65536) {
	if ((tmp ^ bStatG6h) || (!r4sppcoms)) {
	bStatG6h = tmp;
	t_lasts = 0;
	}
	} else {
	bprevStatG6h = ~bStatG6h;
	t_lasts = 0;
	}
	} else {
	bprevStatG6h = ~bStatG6h;
	t_lasts = 0;
	}
	} else if (!memcmp(event->topic+topoffi, "cmd", event->topic_len-topoffi)) {
	if (event->data_len && (event->data_len < 4) && (event->data[0] > 0x2f) && (event->data[0] < 0x3a)) {
	uint16_t tmp = 0;
	mystrcpy(tbuff, event->data, event->data_len);
	tmp = atoi(tbuff);
	if (tmp < 256) {
	if ((tmp ^ (bStatG6 & 0xff)) || (!r4sppcoms)) {
	bStatG6 = (bStatG6 & 0xff00) | tmp;
	t_lasts = 0;
	}
	} else {
	bprevStatG6 = bStatG6 ^ 0x00ff;
	t_lasts = 0;
	}
	} else {
	bprevStatG6 = bStatG6 ^ 0x00ff;
	t_lasts = 0;
	}
	} else if (!memcmp(event->topic+topoffi, "code", event->topic_len-topoffi)) {
	esp_mqtt_client_publish(mqttclient, ttopic, ".", 0, 1, 1);
	if (event->data_len && (event->data_len == 8)) {
	uint8_t buf[4];
	mystrcpy(tbuff, event->data, event->data_len);
	if (hex2bin(tbuff, buf, 4) && buf[0] && (buf[0] < 16) && !(bStatG6 & 0xff00)) {
	bStatG6 = (buf[0] << 8) + buf[3];	
	bStatG6h = (buf[1] << 8) + buf[2];	
	t_lasts = 0;
	} //h2b
	} //<9
	} //code
#endif
	} else if (topoffj) {
	if (!memcmp(event->topic+topoffj, "url", event->topic_len-topoffj)) {
	if (event->data_len > 0) {
	uint16_t tmp = jpg_time;
	jpg_time = 0;
	vTaskDelay(40 / portTICK_PERIOD_MS);
	mystrcpy(MyHttpUri, event->data, event->data_len);
	jpg_time = tmp;
	t_jpg = 0;	
	MyHttpMqtt = MyHttpMqtt | 0x41;
//if (fdebug) ESP_LOGI(AP_TAG,"MQTT_EVENT_JPG_URL");
	}
	} else if (!memcmp(event->topic+topoffj, "time", event->topic_len-topoffj)) {
	uint32_t var = 0;
	tbuff[0] = 0;
	if (event->data_len < 7) {
	mystrcpy(tbuff, event->data, event->data_len);
	if (tbuff[0]) {
	var = atoi(tbuff);
	if  (var < 65536) {
	if (var != jpg_time) MyHttpMqtt = MyHttpMqtt | 0x40;
	jpg_time = var;
	MyHttpMqtt = MyHttpMqtt | 0x02;
	} else {
	itoa(jpg_time,tbuff,10);
	esp_mqtt_client_publish(mqttclient, ttopic, tbuff, 0, 1, 1);
	}
	if (!jpg_time) 	MyHttpMqtt = MyHttpMqtt | 0x80;
	t_jpg = 0;	
//if (fdebug) ESP_LOGI(AP_TAG,"MQTT_EVENT_JPG_TIME");
	}
	}
	}
#endif
	} else {
	char tmpvar[64]; 
	uint8_t i = 0;
	int topoffm = 0;
	while (i < BleMonNum) {
	if (BleMR[i].sto) {
	strcpy(tbuff,"r4s/");
	if ((BleMR[i].id > 0x40) && (BleMR[i].id < 0x50)) bin2hex(BleMR[i].mac,tmpvar,16,0); 
	else bin2hex(BleMR[i].mac,tmpvar,6,0);
	strcat(tbuff,tmpvar);
	strcat(tbuff,"/");
	topoffm = parsoff(event->topic,tbuff, event->topic_len);
	if (topoffm) {
	if (!memcmp(event->topic+topoffm, "rssi", event->topic_len-topoffm)) {
	if (event->data_len && (event->data_len < 6)){
	int8_t rssi = 0;
	mystrcpy(tmpvar, event->data, event->data_len);
	rssi = atoi(tmpvar);
	BleMX[i].cmrssi = rssi;
	BleMX[i].gttmo = 5;
        i = BleMonNum;
	}
	} else if (!memcmp(event->topic+topoffm, "gtnum", event->topic_len-topoffm)) {
	if (event->data_len && (event->data_len < 5)){
	uint8_t gtn = 0;
	mystrcpy(tmpvar, event->data, event->data_len);
	gtn = atoi(tmpvar);
	BleMX[i].gtnum = gtn;
	BleMX[i].gttmo = 5;
        i = BleMonNum;
	}
	}                        //cmp
	}                        //topoffs
	}                        //sto
	i++;
	}                        //while

	}                        // a b c mqtt topic

	}                        //datalen,topiclen
	break;




	case MQTT_EVENT_ERROR:
//	if (fdebug) ESP_LOGI(AP_TAG,"MQTT_EVENT_ERROR");
	break;
	default:
//	if (fdebug) ESP_LOGI(AP_TAG,"Other event id:%d", event->event_id);
	break;
	}
	return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
//	if (fdebug) ESP_LOGI(AP_TAG,"Event dispatched from event loop base=%s, event_id=%d\n", base, event_id);
    mqtt_event_handler_cb(event_data);
}

static void mqtt_app_start(void)
{
	char luri[128];
	char llwtt[16];
	if (fmwss) {
	(fmssl)? strcpy(luri,"wss://") : strcpy(luri,"ws://");
	} else {
	(fmssl)? strcpy(luri,"mqtts://") : strcpy(luri,"mqtt://");
	}
	strcat(luri,MQTT_SERVER);
	strcpy(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/status");
//
	esp_mqtt_client_config_t mqtt_cfg = {
	.uri = luri,
	.username = MQTT_USER,
	.password = MQTT_PASSWORD,
	.port = mqtt_port,
	.lwt_topic = llwtt,
	.lwt_msg = "offline",
	.keepalive = 60,
	.client_id = MQTT_BASE_TOPIC,
	.buffer_size = 2048,
	};
	if (fdebug) ESP_LOGI(AP_TAG,"Mqtt url: %s, port: %d, login: %s, password: %s", luri, mqtt_port, MQTT_USER, MQTT_PASSWORD);
	if (fmssl && (bufcert[0] || fmsslbundle)) {
	if (!fmsslhost) mqtt_cfg.skip_cert_common_name_check = 1;
	if (fmsslbundle) {
	mqtt_cfg.crt_bundle_attach = esp_crt_bundle_attach;
	} else if (!fmsslbundle && bufcert[0]) {
	mqtt_cfg.cert_pem = bufcert;
	if (fdebug) ESP_LOGI(AP_TAG,"Mqtt certificate: %s", mqtt_cfg.cert_pem);
	}
	}
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



static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
	esp_wifi_connect();
	} else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
	if (fdebug) ESP_LOGI(AP_TAG,"AP disconnected");
	if ((!floop && (wf_retry_cnt < WIFI_MAXIMUM_RETRY)) || (floop && (wf_retry_cnt < (WIFI_MAXIMUM_RETRY << 4)))) {
		esp_wifi_connect();
	if (fdebug) ESP_LOGI(AP_TAG, "Retry %d to connect to the AP",wf_retry_cnt);
		wf_retry_cnt++;
	} else {
	xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
	if (fdebug) ESP_LOGI(AP_TAG,"Connect to the AP fail");
	}
	} else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
	ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
	if (fdebug) ESP_LOGI(AP_TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
	wf_retry_cnt = 0;
	xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
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

	wf_retry_cnt = 1;
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
#ifdef USE_TFT
	if (tft_conn) tfststr("Connecting to AP ",WIFI_SSID," ..."); 
#endif
	if (fdebug) {
	ESP_LOGI(AP_TAG,"Wifi_init_sta finished.");
	ESP_LOGI(AP_TAG,"Connecting to Wifi: '%s' with password '%s'", WIFI_SSID, WIFI_PASSWORD);
	}

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
	if (fdebug) ESP_LOGI(AP_TAG,"Connected to ap SSID:'%s' password:'%s'",
		WIFI_SSID, WIFI_PASSWORD);
	} else if (bits & WIFI_FAIL_BIT) {
	if (fdebug) ESP_LOGI(AP_TAG,"Failed to connect to SSID:'%s', password:'%s'",
		WIFI_SSID, WIFI_PASSWORD);

	ESP_ERROR_CHECK(esp_wifi_stop() );
	char DEFWFSSID[33];
	char DEFWFPSW[65];
	wf_retry_cnt = 1;
	strcpy(DEFWFSSID, INIT_WIFI_SSID);
	strcpy(DEFWFPSW, INIT_WIFI_PASSWORD);
	memcpy(wifi_config.sta.ssid, DEFWFSSID, sizeof(wifi_config.sta.ssid));
	memcpy(wifi_config.sta.password, DEFWFPSW, sizeof(wifi_config.sta.password));
	bits = xEventGroupClearBits(s_wifi_event_group,
		WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);

	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
	ESP_ERROR_CHECK(esp_wifi_start() );
#ifdef USE_TFT
	if (tft_conn) tfststr("Connecting to AP ",DEFWFSSID," ..."); 
#endif
	if (fdebug) ESP_LOGI(AP_TAG,"Connecting to ap SSID:'%s' password:'%s'", DEFWFSSID, DEFWFPSW);

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
	if (fdebug) ESP_LOGI(AP_TAG,"Connected to ap SSID:'%s' password:'%s'", DEFWFSSID, DEFWFPSW);
	} else if (bits & WIFI_FAIL_BIT) {
#ifdef USE_TFT
	if (tft_conn) tfststr("Connection to AP ",DEFWFSSID," failed. Restarting ..."); 
#endif
	if (fdebug) {
	ESP_LOGI(AP_TAG,"Failed to connect to SSID:'%s', password:'%s'", DEFWFSSID, DEFWFPSW);
	ESP_LOGI(AP_TAG,"Restarting now...");
	}
	vTaskDelay(2000 / portTICK_PERIOD_MS);
	fflush(stdout);
	esp_restart();
	}
	} else {
	if (fdebug) ESP_LOGE(AP_TAG, "UNEXPECTED EVENT");
	}
#ifdef USE_TFT
	if (tft_conn) pushImage(0, 52, 320, 240, wallpaper);
#endif
}
// **************** NVS ********************
uint8_t ReadNVS(){
	uint8_t f_nvs = 1;  //if f_nvs format & init & write nvs
	uint16_t defnvs = NVS_VER;
//
#ifdef USE_TFT
	defnvs++;             //odd if use tft
#endif
//
	nvs_handle_t my_handle;
	uint8_t ret = nvs_open("storage", NVS_READWRITE, &my_handle);
	if (ret == ESP_OK) {
	uint64_t nvtemp = 0xffffffffffffffff;
	uint16_t nvsver = 0;
#ifdef USE_TFT
	nvs_get_u16(my_handle, "sjpgtim", &jpg_time);
	nvs_get_u16(my_handle, "sjpgbuf", &MyJPGbuflen);
	if (MyJPGbuflen < 20000) MyJPGbuflen = 20000;
#endif
	nvs_get_u16(my_handle, "smqprt", &mqtt_port);
//
	ret = nvs_get_u64(my_handle,  "sreqtp", &nvtemp);
	if ((ret == ESP_OK) && (nvtemp != 0xffffffffffffffff)) {
	uint8_t tmpbuf[32] = {0};
	f_nvs = 0;
	BleDevStA.DEV_TYP = nvtemp & 0xff;	
	BleDevStB.DEV_TYP = (nvtemp >> 8) & 0xff;	
	BleDevStC.DEV_TYP = (nvtemp >> 16) & 0xff;	
	BleDevStD.DEV_TYP = (nvtemp >> 24) & 0xff;	
	BleDevStE.DEV_TYP = (nvtemp >> 32) & 0xff;	
	nvtemp = 0xffffffffffffffff;
	nvs_get_u64(my_handle, "rlight", &nvtemp);
	BleDevStA.RgbR = nvtemp & 0xff;	
	BleDevStB.RgbR = (nvtemp >> 8) & 0xff;	
	BleDevStC.RgbR = (nvtemp >> 16) & 0xff;	
	BleDevStD.RgbR = (nvtemp >> 24) & 0xff;	
	BleDevStE.RgbR = (nvtemp >> 32) & 0xff;	
	nvtemp = 0xffffffffffffffff;
	nvs_get_u64(my_handle, "glight", &nvtemp);
	BleDevStA.RgbG = nvtemp & 0xff;	
	BleDevStB.RgbG = (nvtemp >> 8) & 0xff;	
	BleDevStC.RgbG = (nvtemp >> 16) & 0xff;	
	BleDevStD.RgbG = (nvtemp >> 24) & 0xff;	
	BleDevStE.RgbG = (nvtemp >> 32) & 0xff;	
	nvtemp = 0xffffffffffffffff;
	nvs_get_u64(my_handle, "blight", &nvtemp);
	BleDevStA.RgbB = nvtemp & 0xff;	
	BleDevStB.RgbB = (nvtemp >> 8) & 0xff;	
	BleDevStC.RgbB = (nvtemp >> 16) & 0xff;	
	BleDevStD.RgbB = (nvtemp >> 24) & 0xff;	
	BleDevStE.RgbB = (nvtemp >> 32) & 0xff;	
	nvtemp = 0;
	nvs_get_u64(my_handle, "ltemp", &nvtemp);
	BleDevStA.bLtemp = nvtemp & 0xff;	
	BleDevStB.bLtemp = (nvtemp >> 8) & 0xff;	
	BleDevStC.bLtemp = (nvtemp >> 16) & 0xff;	
	BleDevStD.bLtemp = (nvtemp >> 24) & 0xff;	
	BleDevStE.bLtemp = (nvtemp >> 32) & 0xff;	
	nvtemp = 0x5050505050505050;
	nvs_get_u64(my_handle, "effic", &nvtemp);
	BleDevStA.bEfficiency = nvtemp & 0xff;	
	BleDevStB.bEfficiency = (nvtemp >> 8) & 0xff;	
	BleDevStC.bEfficiency = (nvtemp >> 16) & 0xff;	
	BleDevStD.bEfficiency = (nvtemp >> 24) & 0xff;	
	BleDevStE.bEfficiency = (nvtemp >> 32) & 0xff;	
#ifdef USE_TFT
	nvtemp = 0x2115121110131719;
	nvs_get_u64(my_handle, "ppci", &nvtemp);
	PIN_NUM_MISO = nvtemp & 0xff;	
	PIN_NUM_MOSI = (nvtemp >> 8) & 0xff;	
	PIN_NUM_CLK = (nvtemp >> 16) & 0xff;	
	PIN_NUM_CS = (nvtemp >> 24) & 0xff;	
	PIN_NUM_DC = (nvtemp >> 32) & 0xff;	
	PIN_NUM_RST = (nvtemp >> 40) & 0xff;	
	PIN_NUM_BCKL = (nvtemp >> 48) & 0xff;	
	PIN_NUM_PWR = (nvtemp >> 56) & 0xff;	
#endif
	nvtemp = 0;
	nvs_get_u64(my_handle, "bgpio", &nvtemp);
	bgpio1 = nvtemp & 0xff;	
	bgpio2 = (nvtemp >> 8) & 0xff;	
	bgpio3 = (nvtemp >> 16) & 0xff;	
	bgpio4 = (nvtemp >> 24) & 0xff;	
	bgpio5 = (nvtemp >> 32) & 0xff;	
	bgpio6 = (nvtemp >> 40) & 0xff;	
	bgpio7 = (nvtemp >> 48) & 0xff;	
	bgpio8 = (nvtemp >> 56) & 0xff;	
#ifdef CONFIG_IDF_TARGET_ESP32C3
	if (bgpio8 > 127) bgpio8 = 0;
#endif
	nvtemp = 0;
	nvs_get_u64(my_handle, "bgpi1", &nvtemp);
	bgpio9 = nvtemp & 0xff;	
	bgpio10 = (nvtemp >> 8) & 0xff;	
	nvtemp = 0;
	nvs_get_u64(my_handle, "cmbits", &nvtemp);
	FDHass = 0;
	fcommtp = 0;
	ftrufal = 0;
	foffln = 0;
	macauth = 0;
	volperc = 0;
	fkpmd = 0;
	fmssl = 0;
	fmsslbundle = 0;
	fmsslhost = 0;
	fmwss = 0;
	fdebug = 0;
        ble_mon =  nvtemp & 0x03;
	if (nvtemp & 0x04) FDHass = 1;
	if (nvtemp & 0x08) fcommtp = 1;
	if (nvtemp & 0x10) ftrufal = 1;
	if (nvtemp & 0x20) foffln = 1;
	if (nvtemp & 0x40) macauth = 1;
	if (nvtemp & 0x80) volperc = 1;
#ifdef USE_TFT
	tft_conf = 0;
	tft_flip = 0;
	if (nvtemp & 0x100) tft_conf = 1;
	if (nvtemp & 0x200) tft_flip = 1;
#endif
	if (nvtemp & 0x400) fmssl = 1;
	if (nvtemp & 0x800) fmsslbundle = 1;
	if (nvtemp & 0x1000) fmsslhost = 1;
	if (nvtemp & 0x2000) fmwss = 1;
	if (nvtemp & 0x4000) fdebug = 1;
	if (nvtemp & 0x8000) fkpmd = 1;
	nvtemp = 0;
	nvs_get_u64(my_handle, "bhx1", &nvtemp);
	bZeroHx6 = nvtemp & 0xffffffff;
	bDivHx6 =  nvtemp >> 32;
	size_t nvsize = 20;
	nvs_get_blob(my_handle,"pnpsw", tmpbuf, &nvsize);
	nvtemp = tmpbuf[3];
	nvtemp = (nvtemp << 8) | tmpbuf[2];
	nvtemp = (nvtemp << 8) | tmpbuf[1];
	nvtemp = (nvtemp << 8) | tmpbuf[0];
	BleDevStA.PassKey = nvtemp;
	nvtemp = tmpbuf[7];
	nvtemp = (nvtemp << 8) | tmpbuf[6];
	nvtemp = (nvtemp << 8) | tmpbuf[5];
	nvtemp = (nvtemp << 8) | tmpbuf[4];
	BleDevStB.PassKey = nvtemp;
	nvtemp = tmpbuf[11];
	nvtemp = (nvtemp << 8) | tmpbuf[10];
	nvtemp = (nvtemp << 8) | tmpbuf[9];
	nvtemp = (nvtemp << 8) | tmpbuf[8];
	BleDevStC.PassKey = nvtemp;
	nvtemp = tmpbuf[15];
	nvtemp = (nvtemp << 8) | tmpbuf[14];
	nvtemp = (nvtemp << 8) | tmpbuf[13];
	nvtemp = (nvtemp << 8) | tmpbuf[12];
	BleDevStD.PassKey = nvtemp;
	nvtemp = tmpbuf[19];
	nvtemp = (nvtemp << 8) | tmpbuf[18];
	nvtemp = (nvtemp << 8) | tmpbuf[17];
	nvtemp = (nvtemp << 8) | tmpbuf[16];
	BleDevStE.PassKey = nvtemp;
	nvs_get_u16(my_handle, "nvsid", &nvsver); //read & check nvs version
	if (nvsver != defnvs) f_nvs = 1;          //if != 1 format & init & write
	} else {
//old NVS
	f_nvs = 1;                                //if old format & init & write
	nvs_get_u8(my_handle,  "sreqtpa", &BleDevStA.DEV_TYP);
	nvs_get_u8(my_handle,  "sreqtpb", &BleDevStB.DEV_TYP);
	nvs_get_u8(my_handle,  "sreqtpc", &BleDevStC.DEV_TYP);
	nvs_get_u8(my_handle,  "sreqtpd", &BleDevStD.DEV_TYP);
	nvs_get_u8(my_handle,  "sreqtpe", &BleDevStE.DEV_TYP);
	nvs_get_u8(my_handle, "rlighta", &BleDevStA.RgbR);
	nvs_get_u8(my_handle, "glighta", &BleDevStA.RgbG);
	nvs_get_u8(my_handle, "blighta", &BleDevStA.RgbB);
	nvs_get_u8(my_handle, "rlightb", &BleDevStB.RgbR);
	nvs_get_u8(my_handle, "glightb", &BleDevStB.RgbG);
	nvs_get_u8(my_handle, "blightb", &BleDevStB.RgbB);
	nvs_get_u8(my_handle, "rlightc", &BleDevStC.RgbR);
	nvs_get_u8(my_handle, "glightc", &BleDevStC.RgbG);
	nvs_get_u8(my_handle, "blightc", &BleDevStC.RgbB);
	nvs_get_u8(my_handle, "rlightd", &BleDevStD.RgbR);
	nvs_get_u8(my_handle, "glightd", &BleDevStD.RgbG);
	nvs_get_u8(my_handle, "blightd", &BleDevStD.RgbB);
	nvs_get_u8(my_handle, "rlighte", &BleDevStE.RgbR);
	nvs_get_u8(my_handle, "glighte", &BleDevStE.RgbG);
	nvs_get_u8(my_handle, "blighte", &BleDevStE.RgbB);
	nvs_get_u8(my_handle, "ltempa", &BleDevStA.bLtemp);
	nvs_get_u8(my_handle, "ltempb", &BleDevStB.bLtemp);
	nvs_get_u8(my_handle, "ltempc", &BleDevStC.bLtemp);
	nvs_get_u8(my_handle, "ltempd", &BleDevStD.bLtemp);
	nvs_get_u8(my_handle, "ltempe", &BleDevStE.bLtemp);
	nvs_get_u8(my_handle, "effica", &BleDevStA.bEfficiency);
	nvs_get_u8(my_handle, "efficb", &BleDevStB.bEfficiency);
	nvs_get_u8(my_handle, "efficc", &BleDevStC.bEfficiency);
	nvs_get_u8(my_handle, "efficd", &BleDevStD.bEfficiency);
	nvs_get_u8(my_handle, "effice", &BleDevStE.bEfficiency);
#ifdef USE_TFT
	nvs_get_u8(my_handle, "pnmiso", &PIN_NUM_MISO);
	nvs_get_u8(my_handle, "pnmosi", &PIN_NUM_MOSI);
	nvs_get_u8(my_handle, "pnclk", &PIN_NUM_CLK);
	nvs_get_u8(my_handle, "pncs", &PIN_NUM_CS);
	nvs_get_u8(my_handle, "pndc", &PIN_NUM_DC);
	nvs_get_u8(my_handle, "pnrst", &PIN_NUM_RST);
	nvs_get_u8(my_handle, "pnbckl", &PIN_NUM_BCKL);
	nvs_get_u8(my_handle, "pnpwr", &PIN_NUM_PWR);
#endif
	nvs_get_u8(my_handle, "bgpio1", &bgpio1);
	nvs_get_u8(my_handle, "bgpio2", &bgpio2);
	nvs_get_u8(my_handle, "bgpio3", &bgpio3);
	nvs_get_u8(my_handle, "bgpio4", &bgpio4);
	nvs_get_u8(my_handle, "bgpio5", &bgpio5);
	nvs_get_u8(my_handle, "chk1",   &FDHass);
	nvs_get_u8(my_handle, "chk2",   &fcommtp);
	nvs_get_u8(my_handle, "chk3",   &ftrufal);
#ifdef USE_TFT
	nvs_get_u8(my_handle, "chk4",   &tft_flip);
#endif
	nvs_get_u8(my_handle, "chk5",   &ble_mon);
#ifdef USE_TFT
	nvs_get_u8(my_handle, "chk6",   &tft_conf);
#endif
	nvs_get_u8(my_handle, "chk7",   &foffln);
	nvs_get_u8(my_handle, "chk8",   &macauth);
	nvs_get_u8(my_handle, "chk9",   &volperc);
	}
	BleDevStA.PRgbR = ~BleDevStA.RgbR;
	BleDevStA.PRgbG = ~BleDevStA.RgbG;
	BleDevStA.PRgbB = ~BleDevStA.RgbB;
	BleDevStB.PRgbR = ~BleDevStB.RgbR;
	BleDevStB.PRgbG = ~BleDevStB.RgbG;
	BleDevStB.PRgbB = ~BleDevStB.RgbB;
	BleDevStC.PRgbR = ~BleDevStC.RgbR;
	BleDevStC.PRgbG = ~BleDevStC.RgbG;
	BleDevStC.PRgbB = ~BleDevStC.RgbB;
	BleDevStD.PRgbR = ~BleDevStD.RgbR;
	BleDevStD.PRgbG = ~BleDevStD.RgbG;
	BleDevStD.PRgbB = ~BleDevStD.RgbB;
	BleDevStE.PRgbR = ~BleDevStE.RgbR;
	BleDevStE.PRgbG = ~BleDevStE.RgbG;
	BleDevStE.PRgbB = ~BleDevStE.RgbB;
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
	nvsize = 16;
	nvs_get_str(my_handle,"sreqnmd", BleDevStD.REQ_NAME,&nvsize);
	nvsize = 16;
	nvs_get_str(my_handle,"sreqnme", BleDevStE.REQ_NAME,&nvsize);
	nvsize = sizeof(BleMR);
	nvs_get_blob(my_handle,"sblemd",  BleMR,&nvsize);
	if (nvsize != sizeof(BleMR)) {
	memset (BleMR,0,sizeof(BleMR));
	f_nvs = 1;
	}
	nvsize = 50;
	nvs_get_str(my_handle, "auth", AUTH_BASIC, &nvsize);
	nvsize = 32;
	nvs_get_str(my_handle,"sntp", NTP_SERVER,&nvsize);
#ifdef USE_TFT
	nvsize = 32;
	nvs_get_str(my_handle,"smtopp1", MQTT_TOPP1,&nvsize);
	nvsize = 32;
	nvs_get_str(my_handle,"smtopp2", MQTT_TOPP2,&nvsize);
	nvsize = 32;
	nvs_get_str(my_handle,"smtopp3", MQTT_TOPP3,&nvsize);
	nvsize = 32;
	nvs_get_str(my_handle,"smtopp4", MQTT_TOPP4,&nvsize);
	nvsize = 32;
	nvs_get_str(my_handle,"smtopp5", MQTT_TOPP5,&nvsize);
	nvsize = 32;
	nvs_get_str(my_handle,"smtopp6", MQTT_TOPP6,&nvsize);
	nvsize = 32;
	nvs_get_str(my_handle,"smtopp7", MQTT_TOPP7,&nvsize);
	nvsize = 128;
	nvs_get_str(my_handle,"smjpuri", MyHttpUri,&nvsize);
#endif
	memset (bufcert,0,sizeof(bufcert));
	strcpy(bufcert,"-----BEGIN CERTIFICATE-----\n");
	bcertofs = strlen(bufcert);
/*
	nvsize = 960;   //64 bytes for begin & end headers
	nvs_get_blob(my_handle,"bmcer1",  bufcert + bcertofs,&nvsize);
	if (nvsize == 960) nvs_get_blob(my_handle,"bmcer2",  bufcert + bcertofs + 960,&nvsize);
	if (nvsize == 960) {
*/
	nvsize = 1920;   //64 bytes for begin & end headers
	nvs_get_blob(my_handle,"bmcrt",  bufcert + bcertofs,&nvsize);
	if (nvsize == 1920) {
	bcertsz = strlen(bufcert) - bcertofs;
	if (bcertsz > 16) strcat(bufcert,"\n-----END CERTIFICATE-----\n");
	else {
	bcertofs = 0;
	bcertsz = 0;
	memset (bufcert,0,sizeof(bufcert));
	}
	} else {
	bcertofs = 0;
	bcertsz = 0;
	memset (bufcert,0,sizeof(bufcert));
	}
// Close nvs
	nvs_close(my_handle);
	if (fdebug) ESP_LOGI(AP_TAG, "Read NVS done");
	}
	return f_nvs;
}

void FinitNVS () {
	esp_phy_erase_cal_data_in_nvs();
	nvs_flash_erase();
	nvs_flash_init();
	if (fdebug) ESP_LOGI(AP_TAG, "Format and Init NVS done");
	}



void WriteNVS () {
	nvs_handle_t my_handle;
	uint16_t defnvs = NVS_VER;
	uint8_t tmpbuf[32] = {0};
#ifdef USE_TFT
	defnvs++;             //odd if use tft
#endif
	uint8_t ret = nvs_open("storage", NVS_READWRITE, &my_handle);
	if (ret == ESP_OK) {
	uint64_t nvtemp = 0;
	nvs_set_u16(my_handle, "nvsid", defnvs);
#ifdef USE_TFT
	nvs_set_u16(my_handle, "sjpgtim", jpg_time);
	nvs_set_u16(my_handle, "sjpgbuf", MyJPGbuflen);
#endif
	nvs_set_u16(my_handle, "smqprt", mqtt_port);

	nvtemp = BleDevStE.DEV_TYP;
	nvtemp = nvtemp << 32;
	nvtemp = nvtemp | BleDevStA.DEV_TYP | (BleDevStB.DEV_TYP << 8) | (BleDevStC.DEV_TYP << 16) | (BleDevStD.DEV_TYP << 24);	
	nvs_set_u64(my_handle,  "sreqtp", nvtemp);
	nvtemp = BleDevStE.RgbR;
	nvtemp = nvtemp << 32;
	nvtemp = nvtemp | BleDevStA.RgbR | (BleDevStB.RgbR << 8) | (BleDevStC.RgbR << 16) | (BleDevStD.RgbR << 24);	
	nvs_set_u64(my_handle,  "rlight", nvtemp);
	nvtemp = BleDevStE.RgbG;
	nvtemp = nvtemp << 32;
	nvtemp = nvtemp | BleDevStA.RgbG | (BleDevStB.RgbG << 8) | (BleDevStC.RgbG << 16) | (BleDevStD.RgbG << 24);	
	nvs_set_u64(my_handle,  "glight", nvtemp);
	nvtemp = BleDevStE.RgbB;
	nvtemp = nvtemp << 32;
	nvtemp = nvtemp | BleDevStA.RgbB | (BleDevStB.RgbB << 8) | (BleDevStC.RgbB << 16) | (BleDevStD.RgbB << 24);	
	nvs_set_u64(my_handle,  "blight", nvtemp);
	nvtemp = BleDevStE.bLtemp;
	nvtemp = nvtemp << 32;
	nvtemp = nvtemp | BleDevStA.bLtemp | (BleDevStB.bLtemp << 8) | (BleDevStC.bLtemp << 16) | (BleDevStD.bLtemp << 24);	
	nvs_set_u64(my_handle,  "ltemp", nvtemp);
	nvtemp = BleDevStE.bEfficiency;
	nvtemp = nvtemp << 32;
	nvtemp = nvtemp | BleDevStA.bEfficiency | (BleDevStB.bEfficiency << 8) | (BleDevStC.bEfficiency << 16) | (BleDevStD.bEfficiency << 24);	
	nvs_set_u64(my_handle,  "effic", nvtemp);
#ifdef USE_TFT
	nvtemp = PIN_NUM_PWR;
	nvtemp = (nvtemp << 8) | PIN_NUM_BCKL;
	nvtemp = (nvtemp << 8) | PIN_NUM_RST;
	nvtemp = (nvtemp << 8) | PIN_NUM_DC;
	nvtemp = (nvtemp << 8) | PIN_NUM_CS;
	nvtemp = (nvtemp << 8) | PIN_NUM_CLK;
	nvtemp = (nvtemp << 8) | PIN_NUM_MOSI;
	nvtemp = (nvtemp << 8) | PIN_NUM_MISO;
	nvs_set_u64(my_handle,  "ppci", nvtemp);
#endif
	nvtemp = bgpio8;
	nvtemp = (nvtemp << 8) | bgpio7;
	nvtemp = (nvtemp << 8) | bgpio6;
	nvtemp = (nvtemp << 8) | bgpio5;
	nvtemp = (nvtemp << 8) | bgpio4;
	nvtemp = (nvtemp << 8) | bgpio3;
	nvtemp = (nvtemp << 8) | bgpio2;
	nvtemp = (nvtemp << 8) | bgpio1;
	nvs_set_u64(my_handle,  "bgpio", nvtemp);
	nvtemp = bgpio10;
	nvtemp = (nvtemp << 8) | bgpio9;
	nvs_set_u64(my_handle,  "bgpi1", nvtemp);
	nvtemp = ble_mon & 0x03;
	if (FDHass) nvtemp = nvtemp | 0x04;
	if (fcommtp) nvtemp = nvtemp | 0x08;
	if (ftrufal) nvtemp = nvtemp | 0x10;
	if (foffln) nvtemp = nvtemp | 0x20;
	if (macauth) nvtemp = nvtemp | 0x40;
	if (volperc) nvtemp = nvtemp | 0x80;
#ifdef USE_TFT
	if (tft_conf) nvtemp = nvtemp | 0x100;
	if (tft_flip) nvtemp = nvtemp | 0x200;
#endif
	if (fmssl) nvtemp = nvtemp | 0x400;
	if (fmsslbundle) nvtemp = nvtemp | 0x800;
	if (fmsslhost) nvtemp = nvtemp | 0x1000;
	if (fmwss) nvtemp = nvtemp | 0x2000;
	if (fdebug) nvtemp = nvtemp | 0x4000;
	if (fkpmd) nvtemp = nvtemp | 0x8000;
	nvs_set_u64(my_handle, "cmbits", nvtemp);
	nvtemp = bDivHx6;
	nvtemp = (nvtemp << 32) | bZeroHx6;
	nvs_set_u64(my_handle, "bhx1", nvtemp);
	nvs_set_u8(my_handle,  "timzon", TimeZone);
	nvs_set_u8(my_handle,  "r4snum", R4SNUM);
	nvs_set_str(my_handle, "swfid", WIFI_SSID);
	nvs_set_str(my_handle, "swfpsw", WIFI_PASSWORD);
	nvs_set_str(my_handle, "smqsrv", MQTT_SERVER);
	nvs_set_str(my_handle, "smqid", MQTT_USER);
	nvs_set_str(my_handle, "smqpsw", MQTT_PASSWORD);
	nvs_set_str(my_handle, "sreqnma", BleDevStA.REQ_NAME);
	nvs_set_str(my_handle, "sreqnmb", BleDevStB.REQ_NAME);
	nvs_set_str(my_handle, "sreqnmc", BleDevStC.REQ_NAME);
	nvs_set_str(my_handle, "sreqnmd", BleDevStD.REQ_NAME);
	nvs_set_str(my_handle, "sreqnme", BleDevStE.REQ_NAME);
	nvs_set_str(my_handle, "auth", AUTH_BASIC);
	nvs_set_str(my_handle, "sntp", NTP_SERVER);
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
	nvtemp = BleDevStA.PassKey;
	tmpbuf[0] = nvtemp & 0xff;
	tmpbuf[1] = (nvtemp >> 8) & 0xff;
	tmpbuf[2] = (nvtemp >> 16) & 0xff;
	tmpbuf[3] = (nvtemp >> 24) & 0xff;
	nvtemp = BleDevStB.PassKey;
	tmpbuf[4] = nvtemp & 0xff;
	tmpbuf[5] = (nvtemp >> 8) & 0xff;
	tmpbuf[6] = (nvtemp >> 16) & 0xff;
	tmpbuf[7] = (nvtemp >> 24) & 0xff;
	nvtemp = BleDevStC.PassKey;
	tmpbuf[8] = nvtemp & 0xff;
	tmpbuf[9] = (nvtemp >> 8) & 0xff;
	tmpbuf[10] = (nvtemp >> 16) & 0xff;
	tmpbuf[11] = (nvtemp >> 24) & 0xff;
	nvtemp = BleDevStD.PassKey;
	tmpbuf[12] = nvtemp & 0xff;
	tmpbuf[13] = (nvtemp >> 8) & 0xff;
	tmpbuf[14] = (nvtemp >> 16) & 0xff;
	tmpbuf[15] = (nvtemp >> 24) & 0xff;
	nvtemp = BleDevStE.PassKey;
	tmpbuf[16] = nvtemp & 0xff;
	tmpbuf[17] = (nvtemp >> 8) & 0xff;
	tmpbuf[18] = (nvtemp >> 16) & 0xff;
	tmpbuf[19] = (nvtemp >> 24) & 0xff;
	nvs_set_blob(my_handle,"pnpsw", tmpbuf, 20);
	ret = nvs_commit(my_handle);
	if (ret != ESP_OK) {
	if (fdebug) ESP_LOGE(AP_TAG, "NVS write 1 error");
	} else {
	nvs_set_blob(my_handle,"sblemd",  BleMR, sizeof(BleMR));
/*
	nvs_set_blob(my_handle,"bmcer1",  bufcert, 960);
	nvs_set_blob(my_handle,"bmcer2",  bufcert + 960, 960);
*/
	nvs_set_blob(my_handle,"bmcrt",  bufcert, 1920);
	ret = nvs_commit(my_handle);
	if (ret != ESP_OK) {
	if (fdebug) ESP_LOGE(AP_TAG, "NVS write error");
	}
	}
// Close nvs
	nvs_close(my_handle);
	if (fdebug) ESP_LOGI(AP_TAG, "Write NVS done");
	}
}


void MnHtpBleSt(uint8_t blenum, char* bsend) {
	uint8_t blenum1 = blenum + 1;
	char buff[16]; 
	if (blenum > 4) return;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
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
	if ((ptr->DEV_TYP < 10) || ((ptr->DEV_TYP > 63) && (ptr->DEV_TYP < 73))) strcat(bsend,"Kettle, ");
	else if (ptr->DEV_TYP < 11) strcat(bsend,"Power, ");
	else if (ptr->DEV_TYP < 12) strcat(bsend,"Heater, ");
	else if (ptr->DEV_TYP < 16) strcat(bsend,"Coffee, ");
	else if (ptr->DEV_TYP < 24) strcat(bsend,"Cooker, ");
	else if (ptr->DEV_TYP < 48) strcat(bsend,"Oven, ");
	else if (ptr->DEV_TYP < 61) strcat(bsend,"Baker, ");
	else if (ptr->DEV_TYP == 61) strcat(bsend,"Iron, ");
	else if (ptr->DEV_TYP == 62) strcat(bsend,"Smoke, ");
	else if (ptr->DEV_TYP == 63) strcat(bsend,"Weather, ");
	else if (ptr->DEV_TYP == 73) strcat(bsend,"Galcon, ");
	else if (ptr->DEV_TYP == 74) strcat(bsend,"Blinds, ");
	strcat(bsend,"MAC: ");
	strcat(bsend,ptr->tBLEAddr);
	strcat(bsend,", Name: ");
	if (ptr->DEV_NAME[0]) strcat(bsend,ptr->DEV_NAME);
	if (ptr->sVer[0] > 0x20) {
	strcat(bsend,", Ver: ");
	strcat(bsend, ptr->sVer);
	}
	if ((ptr->DEV_TYP < 10) || ((ptr->DEV_TYP > 63) && (ptr->DEV_TYP < 73))) {
	strcat(bsend,", State: ");
	if ((!ptr->bState) && (!ptr->bHeat || (ptr->DEV_TYP > 63)) && (!ptr->bStNl)) strcat(bsend,"Off");
 	else if (ptr->bState == 254) strcat(bsend,"Offline");
 	else strcat(bsend,"On");
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
	strcat(bsend,"&deg;C, Target: ");
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
	strcat(bsend,", Keep: ");
	if (!ptr->bModProg) strcat(bsend,"Off");
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
	} else if (ptr->DEV_TYP < 61) {
	strcat(bsend,", State: ");
	if (ptr->DEV_TYP == 16) {	
	if (!ptr->bState) strcat(bsend,"Off");
	else if (ptr->bState == 1) strcat(bsend,"Setting");
	else if (ptr->bState == 2) strcat(bsend,"On");
	else if (ptr->bState == 3) strcat(bsend,"Wait Product");
	else if (ptr->bState == 4) strcat(bsend,"Warming");
	else if (ptr->bState == 5) strcat(bsend,"Delayed Start");
	else if (ptr->bState == 6) strcat(bsend,"PreHeating");
 	else if (ptr->bState == 254) strcat(bsend,"Offline");
	} else if (ptr->DEV_TYP == 48) {
	if (!ptr->bState) strcat(bsend,"Off");
	else if (ptr->bState == 1) strcat(bsend,"Update");
	else if (ptr->bState == 2) strcat(bsend,"Delayed Start");
	else if (ptr->bState == 3) strcat(bsend,"PreHeating");
	else if (ptr->bState == 4) strcat(bsend,"Wait Product");
	else if (ptr->bState == 5) strcat(bsend,"On");
	else if (ptr->bState == 6) strcat(bsend,"Warming");
 	else if (ptr->bState == 254) strcat(bsend,"Offline");
 	else if (ptr->bState == 255) strcat(bsend,"Error");
 	} else {
	if (!ptr->bState) strcat(bsend,"Off");
	else if (ptr->bState == 1) strcat(bsend,"Setting");
	else if (ptr->bState == 2) strcat(bsend,"Delayed Start");
	else if (ptr->bState == 3) strcat(bsend,"PreHeating");
	else if (ptr->bState == 4) strcat(bsend,"Wait Product");
	else if (ptr->bState == 5) strcat(bsend,"On");
	else if (ptr->bState == 6) strcat(bsend,"Warming");
	else if (ptr->bState == 7) strcat(bsend,"Error");
	else if (ptr->bState == 8) strcat(bsend,"Wait Confirm");
	else if (ptr->bState == 9) strcat(bsend,"Stop Power Loss");
 	else if (ptr->bState == 254) strcat(bsend,"Offline");
	}
	strcat(bsend,", Prog: ");
	itoa(ptr->bProg,buff,10);
	strcat(bsend,buff);
	if (ptr->DEV_TYP != 48) {	
	strcat(bsend,", Mode: ");
	itoa(ptr->bModProg,buff,10);
	strcat(bsend,buff);
	strcat(bsend,", Temp: ");
	itoa(ptr->bHtemp,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"&deg;C");
	}
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
	} else if (ptr->DEV_TYP == 61) {
	strcat(bsend,", State: ");
	if (!ptr->bState) strcat(bsend,"Off");
 	else if (ptr->bState == 254) strcat(bsend,"Offline");
 	else strcat(bsend,"On");
	strcat(bsend,", Lock: ");
	if (!ptr->bLock) strcat(bsend,"Off");
 	else strcat(bsend,"On");
	strcat(bsend,", ");
	} else if (ptr->DEV_TYP == 62) {
	strcat(bsend,", Temperature: ");
	if (ptr->bSEnergy & 0x80000000) strcat(bsend,"-");
	itoa((ptr->bSEnergy & 0x7fffffff) / 10,buff,10);
	strcat(bsend,buff);
	strcat(bsend,".");
	itoa((ptr->bSEnergy & 0x7fffffff) % 10,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"&deg;C, Smoke: ");
	if (!ptr->bProg) strcat(bsend,"Off");
 	else if (ptr->bProg == 254) strcat(bsend,"Offline");
 	else strcat(bsend,"On");
	strcat(bsend,", Battery: ");
	itoa(ptr->bCtemp,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"&#37;, ");
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
	} else if (ptr->DEV_TYP == 73) {
	strcat(bsend,", State: ");
	if (!ptr->bState) strcat(bsend,"Off");
 	else if (ptr->bState == 254) strcat(bsend,"Offline");
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
	strcat(bsend,":");
	if (ptr->bDMin < 10) strcat(bsend,"0");
	itoa(ptr->bCMin,buff,10);
	strcat(bsend,buff);
	strcat(bsend,", Battery: ");
	itoa(ptr->bCtemp,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"&#37;, ");
	} else if (ptr->DEV_TYP == 74) {
	strcat(bsend,", Position: ");
	itoa(ptr->bProg,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"&#37;, Illuminance: ");
	itoa(ptr->bSEnergy,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"&#37, Battery: ");
	itoa(ptr->bCtemp,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"&#37;, ");
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
	} else {
	strcat(bsend,"offline\"");
	if (fkpmd && (ptr->DEV_TYP < 10)) {
	strcat(bsend,",\"keep\":");
	itoa(ptr->bKeep,buff,10);
	strcat(bsend,buff);
	}
	}
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
	ble_mon_refr = ble_mon_refr & 0xfd;
	char *bsend = NULL;
	bsend = malloc(14000);
	if (bsend == NULL) {
	if (fdebug) ESP_LOGE(AP_TAG, "Http main: No memory");
	MemErr++;
	if (!MemErr) MemErr--;
	httpd_resp_sendstr(req, "<meta http-equiv=\"refresh\" content=\"5\">");
	} else {	
	int FreeMem = esp_get_free_heap_size();
	char bufip[32] = {0};
	time_t now;
	char strftime_buf[64];
	struct tm timeinfo;
	time(&now);
	localtime_r(&now, &timeinfo);
	strftime(strftime_buf, sizeof(strftime_buf), "%a %Y %b %d / %X", &timeinfo);
	int buf_len;
	buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
	if (buf_len > 1) {
	if (buf_len >31) buf_len = 31;
        /* Copy null terminated value string into buffer */
	if (httpd_req_get_hdr_value_str(req, "Host", bufip, buf_len) == ESP_OK) {
	}
	}
        char buff[64];
	strcpy(bsend,"<!DOCTYPE html><html>");
	strcat(bsend,"<head><title>r4sGate</title>");
	strcat(bsend,"<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
	strcat(bsend,"<meta http-equiv=\"refresh\" content=\"5\">");
	strcat(bsend,cssDatasheet);
	strcat(bsend,"</head><body class='bodymenu'><header class='headermenu'><h1>ESP32 r4sGate");
	itoa(R4SNUM,buff,10);
	strcat(bsend, buff);
	strcat(bsend," for Redmond +</h1>");
	strcat(bsend,"<div class='menubar'><a class='menu active' href='.'>&#8962;<span class='showmenulabel'>Main");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev1'>&#128246;<span class='showmenulabel'> 1 ");
	(BleDevStA.RQC_NAME[0])? strcat(bsend,BleDevStA.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev2'>&#128246;<span class='showmenulabel'> 2 ");
	(BleDevStB.RQC_NAME[0])? strcat(bsend,BleDevStB.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev3'>&#128246;<span class='showmenulabel'> 3 ");
	(BleDevStC.RQC_NAME[0])? strcat(bsend,BleDevStC.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev4'>&#128246;<span class='showmenulabel'> 4 ");
	(BleDevStD.RQC_NAME[0])? strcat(bsend,BleDevStD.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev5'>&#128246;<span class='showmenulabel'> 5 ");
	(BleDevStE.RQC_NAME[0])? strcat(bsend,BleDevStE.RQC_NAME) : strcat(bsend,"Not defined");
	if (ble_mon) strcat(bsend,"</span></a><a class='menu' href='blemon'>&#128246;<span class='showmenulabel'>BLE monitor</span></a>");
	strcat(bsend,"</span></a><a class='menu' href='setting'>&#9881;<span class='showmenulabel'>Setting</span></a>");
	strcat(bsend,"<a class='menu' href='restart'>&#128259;<span class='showmenulabel'>Reboot</span></a>");
	strcat(bsend,"<a class='menu' href='update'>&#10548;<span class='showmenulabel'>Load firmware</span></a></div>");
	strcat(bsend,"</header><table class='normal'>");

	MnHtpBleSt(0, bsend);
	MnHtpBleSt(1, bsend);
	MnHtpBleSt(2, bsend);
	MnHtpBleSt(3, bsend);
	MnHtpBleSt(4, bsend);
#ifdef USE_IRTX
	if ((bgpio9 & 0xc0) || (bgpio10 & 0xc0) || (bgpio1 & 0xc0) || (bgpio2 & 0xc0) || (bgpio3 & 0xc0) || (bgpio4 & 0xc0) || ((bgpio5 & 0xc0) && (bgpio5  < 192)) || ((bgpio6  > 63) && (bgpio6  < 192)) || (bgpio7 & 0xc0) || (bgpio8 & 0xc0)) {
#else
	if ((bgpio9 & 0xc0) || (bgpio10 & 0xc0) || (bgpio1 & 0xc0) || (bgpio2 & 0xc0) || (bgpio3 & 0xc0) || (bgpio4 & 0xc0) || ((bgpio5 & 0xc0) && (bgpio5  < 192)) || (bgpio6 & 0xc0) || (bgpio7 & 0xc0) || (bgpio8 & 0xc0)) {
#endif
	strcat(bsend,"<h3>Ports & Sensors:</h3><h2>");
	if (bgpio1 & 0xc0) {
	strcat(bsend,"&emsp;Port1: ");
	(lvgpio1)? strcat(bsend,"On") : strcat(bsend,"Off");
	}
	if (bgpio2 & 0xc0) {
	strcat(bsend,"&emsp;Port2: ");
	(lvgpio2)? strcat(bsend,"On") : strcat(bsend,"Off");
	}
	if (bgpio3 & 0xc0) {
	strcat(bsend,"&emsp;Port3: ");
	(lvgpio3)? strcat(bsend,"On") : strcat(bsend,"Off");
	}
	if (bgpio4 & 0xc0) {
	strcat(bsend,"&emsp;Port4: ");
	(lvgpio4)? strcat(bsend,"On") : strcat(bsend,"Off");
	}
	if ((bgpio5 & 0xc0) && (bgpio5 < 192)){
	strcat(bsend,"&emsp;Port5: ");
	(lvgpio5)? strcat(bsend,"On") : strcat(bsend,"Off");
	}
	if (bgpio6 & 0xc0) {
	if (bgpio6 < (MxPOutP + 64)) {
	strcat(bsend,"&emsp;Pwm6: ");
	itoa(bStatG6,buff,10);
	strcat(bsend,buff);
	} else if ((bgpio6 > 127) && (bgpio6 < 192)) {
	if (bgpio5 > 191) {
	uint32_t var;
	strcat(bsend,"&emsp;Hx6: ");
	buff[0] = 0;
	if (bStatHx6 & 0xff) strcpy (buff,"-0");
	else {
	var = bStatHx6 - bZeroHx6;
	if (var & 0x80000000) {
	var = (var ^ 0xffffffff) + 1;
	strcpy (buff,"-");
	}
	if (bDivHx6 & 0x7fffffff) var = var / (bDivHx6 & 0x7fffffff);
	if(!var) buff[0] = 0;
	if (bDivHx6 & 0x80000000) u32_strcat_p1(var,buff);
	else u32_strcat_p3(var,buff);
	}
	if (bDivHx6 & 0x80000000) strcat(buff,"%");
	else strcat(buff,"kg");
	strcat(bsend,buff);
	} else {
	strcat(bsend,"&emsp;1w6: ");
	s18b20_strcat (bStatG6, bsend);
	strcat(bsend,"&deg;C");
	}
#ifdef USE_IRTX
	}
#else
	} else if ((bgpio6 > 191) && (bgpio6 < (MxPOutP + 192))) {
	strcat(bsend,"&emsp;dht6: ");
	sdht22strcat (bStatG6, bsend);
	strcat(bsend,"&deg;C, ");
	u32_strcat_p1 (bStatG6h, bsend);
	strcat(bsend,"%");
	}
#endif
	}
	if (bgpio7 & 0xc0) {
	if (bgpio7 < (MxPOutP + 64)) {
	strcat(bsend,"&emsp;Pwm7: ");
	itoa(bStatG7,buff,10);
	strcat(bsend,buff);
	} else if ((bgpio7 > 127) && (bgpio7 < 192)) {
	strcat(bsend,"&emsp;1w7: ");
	s18b20_strcat (bStatG7, bsend);
	strcat(bsend,"&deg;C");
	} else if ((bgpio7 > 191) && (bgpio7 < (MxPOutP + 192))) {
	strcat(bsend,"&emsp;dht7: ");
	sdht22strcat (bStatG7, bsend);
	strcat(bsend,"&deg;C, ");
	u32_strcat_p1 (bStatG7h, bsend);
	strcat(bsend,"%");
	}
	}
	if (bgpio8 & 0xc0) {
	if (bgpio8 < (MxPOutP + 64)) {
	strcat(bsend,"&emsp;Pwm8: ");
	itoa(bStatG8,buff,10);
	strcat(bsend,buff);
	} else if ((bgpio8 > 127) && (bgpio8 < 192)) {
	strcat(bsend,"&emsp;1w8: ");
	s18b20_strcat (bStatG8, bsend);
	strcat(bsend,"&deg;C");
	} else if ((bgpio8 > 191) && (bgpio8 < (MxPOutP + 192))) {
	strcat(bsend,"&emsp;dht8: ");
	sdht22strcat (bStatG8, bsend);
	strcat(bsend,"&deg;C, ");
	u32_strcat_p1 (bStatG8h, bsend);
	strcat(bsend,"%");
	}
	}
	if (f_i2cdev & 0x80000000) {
	strcat(bsend,"&emsp;I2C> Dev: ");
	itoa(i2cdevnum,buff,10);
	strcat(bsend,buff);
	strcat(bsend,", Err: ");
	itoa(i2c_errcnt,buff,10);
	strcat(bsend,buff);
	if (f_i2cdev & 0x60000000) {
	if (f_i2cdev & 0x40000000) strcat(bsend,"&emsp;I2C.75> Bat: ");
	else if (f_i2cdev & 0x20000000) strcat(bsend,"&emsp;I2C.34> Bat: ");
	if (pwr_batmode & 0x04) {
	strcat(bsend,"Disconnected");
	} else {
	itoa(pwr_batlevp,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"%");
	if (f_i2cdev & 0x20000000) {
	uint16_t var = pwr_batlevc;
	strcat(bsend,", ");
	u32_strcat_p3(pwr_batlevv, bsend);
	strcat(bsend,"V, ");
	if (var & 0x8000) {
	var = (var ^ 0x0ffff) + 1;  
	strcat(bsend,"-");
	}
	u32_strcat_p3(var, bsend);
	strcat(bsend,"A");
	}
	if (!(pwr_batmode & 0x02)) strcat(bsend,", Discharging");
	else if (!(pwr_batmode & 0x01)) strcat(bsend,", Charging");
	else strcat(bsend,", Charged");
	}
	}
	for (int i = 0; i < 28; i++) {
	if (f_i2cdev & (1 << i)) {
	if (i2c_bits[i] & 0x0f) {
	strcat(bsend,"&emsp;I2C.");
	bin2hex(&i2c_addr[i],buff,1,0);
	strcat(bsend,buff);
	strcat(bsend,"> ");
	}
	if (i2c_bits[i] & 0x01) {
	sbme280_strcat (SnPi2c[i].par1, bsend);
	strcat(bsend,"&deg;C");
	if (i2c_bits[i] & 0x0e) strcat(bsend,", ");
	}
	if (i2c_bits[i] & 0x02) {
	u32_strcat_p1 (SnPi2c[i].par2, bsend);
	strcat(bsend,"%");
	if (i2c_bits[i] & 0x0c) strcat(bsend,", ");
	}
	if (i2c_bits[i] & 0x04) {
	u32_strcat_p1 (SnPi2c[i].par3, bsend);
	strcat(bsend,"hPa");
	if (i2c_bits[i] & 0x08) strcat(bsend,", ");
	}
	if (i2c_bits[i] & 0x08) {
	if (SnPi2c[i].par4 == 0xffffffff) strcat(bsend,"-0");
	else {
	itoa(SnPi2c[i].par4,buff,10);
	strcat(bsend,buff);
	}
	strcat(bsend,"&#x3A9;");
	if (i2c_bits[i] & 0x10) strcat(bsend,", ");
	}

	} //bits
	} //i

	}
	strcat(bsend,"</h2>");
	}
	strcat(bsend,"<h3>System Info</h3><table class='normal'>");
	strcat(bsend,"<tr><td style='min-width:150px;'>Version App / IDF / CPU Clock");
#ifdef CONFIG_IDF_TARGET_ESP32
	if (REG_GET_BIT(EFUSE_BLK0_RDATA3_REG, EFUSE_RD_CHIP_CPU_FREQ_RATED)) strcat(bsend," / CPU Rated");
#endif
	strcat(bsend,"</td><td style='width:80%;'>");
	strcat(bsend,AP_VER);
	strcat(bsend," / ");
	strcat(bsend,IDF_VER);
	strcat(bsend," / ");
#ifdef CONFIG_IDF_TARGET_ESP32C3
	itoa(CONFIG_ESP32C3_DEFAULT_CPU_FREQ_MHZ,buff,10);
#else
	itoa(CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ,buff,10);
#endif
	strcat(bsend,buff);
#ifdef CONFIG_IDF_TARGET_ESP32
	if (REG_GET_BIT(EFUSE_BLK0_RDATA3_REG, EFUSE_RD_CHIP_CPU_FREQ_RATED)) {
	strcat(bsend,"MHz / ");
        (REG_GET_BIT(EFUSE_BLK0_RDATA3_REG, EFUSE_RD_CHIP_CPU_FREQ_LOW))? strcat(bsend,"160") : strcat(bsend,"240");
	}
#endif
	uptime_string_exp(buff);
	strcat(bsend,"MHz</td></tr><tr><td>Local date / time</td><td>");
	strcat(bsend,strftime_buf);
	strcat(bsend,"</td></tr><tr><td>Uptime</td><td>");
	strcat(bsend,buff);
	strcat(bsend,"</td></tr><tr><td>NoMem count / Free memory</td><td>");
	itoa(MemErr,buff,10);
	strcat(bsend,buff);
	strcat(bsend," / ");
	itoa(FreeMem,buff,10);
	strcat(bsend,buff);
	strcat(bsend," bytes");
	if (Isscanning || (BleDevStA.REQ_NAME[0] && !BleDevStA.btauthoriz) || (BleDevStB.REQ_NAME[0] && !BleDevStB.btauthoriz) ||(BleDevStC.REQ_NAME[0] && !BleDevStC.btauthoriz)) {
	strcat(bsend,"</td></tr><tr><td>BLE found Name / MAC / RSSI</td><td>");
	strcat(bsend,FND_NAME);
	strcat(bsend," / ");
	strcat(bsend,FND_ADDR);
	strcat(bsend," / ");
	itoa(FND_RSSI,buff,10);
	strcat(bsend,buff);
	strcat(bsend," dB");
	strcat(bsend,"</td></tr><tr><td>BLE activity</td><td>");
	if (BleDevStA.btopenreq && !BleDevStA.btauthoriz) {
	strcat(bsend,"Connecting ");
	strcat(bsend, BleDevStA.RQC_NAME);
	if (!BleDevStA.btopen) strcat(bsend," (Open");
	else {
	strcat(bsend," (Auth");
	if ((BleDevStA.DEV_TYP > 63) && (BleDevStA.DEV_TYP < 73)) {
	strcat(bsend,"/Id:");
	itoa(BleDevStA.MiKettleID,buff,10);
	strcat(bsend,buff);
	}
	}
	strcat(bsend,")");
	} else if (BleDevStB.btopenreq && !BleDevStB.btauthoriz) {
	strcat(bsend,"Connecting ");
	strcat(bsend, BleDevStB.RQC_NAME);
	if (!BleDevStB.btopen) strcat(bsend," (Open");
	else {
	strcat(bsend," (Auth");
	if ((BleDevStB.DEV_TYP > 63) && (BleDevStB.DEV_TYP < 73)) {
	strcat(bsend," / Id:");
	itoa(BleDevStB.MiKettleID,buff,10);
	strcat(bsend,buff);
	}
	}
	strcat(bsend,")");
	} else if (BleDevStC.btopenreq && !BleDevStC.btauthoriz) {
	strcat(bsend,"Connecting ");
	strcat(bsend, BleDevStC.RQC_NAME);
	if (!BleDevStC.btopen) strcat(bsend," (Open");
	else {
	strcat(bsend," (Auth");
	if ((BleDevStC.DEV_TYP > 63) && (BleDevStC.DEV_TYP < 73)) {
	strcat(bsend," / Id:");
	itoa(BleDevStC.MiKettleID,buff,10);
	strcat(bsend,buff);
	}
	}
	strcat(bsend,")");
	} else if (BleDevStD.btopenreq && !BleDevStD.btauthoriz) {
	strcat(bsend,"Connecting ");
	strcat(bsend, BleDevStD.RQC_NAME);
	if (!BleDevStD.btopen) strcat(bsend," (Open");
	else {
	strcat(bsend," (Auth");
	if ((BleDevStD.DEV_TYP > 63) && (BleDevStD.DEV_TYP < 73)) {
	strcat(bsend," / Id:");
	itoa(BleDevStD.MiKettleID,buff,10);
	strcat(bsend,buff);
	}
	}
	strcat(bsend,")");
	} else if (BleDevStE.btopenreq && !BleDevStE.btauthoriz) {
	strcat(bsend,"Connecting ");
	strcat(bsend, BleDevStE.RQC_NAME);
	if (!BleDevStE.btopen) strcat(bsend," (Open");
	else {
	strcat(bsend," (Auth");
	if ((BleDevStE.DEV_TYP > 63) && (BleDevStE.DEV_TYP < 73)) {
	strcat(bsend," / Id:");
	itoa(BleDevStE.MiKettleID,buff,10);
	strcat(bsend,buff);
	}
	}
	strcat(bsend,")");
	} else if (Isscanning) strcat(bsend,"Scanning");
	else strcat(bsend,"Idle");
	}

	strcat(bsend,"</td></tr><tr><td>BLE 1 connection count / state / RSSI</td><td>");
	itoa(BleDevStA.NumConn,buff,10);
	strcat(bsend,buff);
        (BleDevStA.btauthoriz)? strcat(bsend," / Connected") : strcat(bsend," / Disconnected");
	if ((BleDevStA.btauthoriz) && (BleDevStA.iRssi)) {
	strcat(bsend," / ");
	itoa(BleDevStA.iRssi,buff,10);
	strcat(bsend,buff);
	strcat(bsend," dB");
	}

	strcat(bsend,"</td></tr><tr><td>BLE 2 connection count / state / RSSI</td><td>");
	itoa(BleDevStB.NumConn,buff,10);
	strcat(bsend,buff);
        (BleDevStB.btauthoriz)? strcat(bsend," / Connected") : strcat(bsend," / Disconnected");
	if ((BleDevStB.btauthoriz) && (BleDevStB.iRssi)) {
	strcat(bsend," / ");
	itoa(BleDevStB.iRssi,buff,10);
	strcat(bsend,buff);
	strcat(bsend," dB");
	}

	strcat(bsend,"</td></tr><tr><td>BLE 3 connection count / state / RSSI</td><td>");
	itoa(BleDevStC.NumConn,buff,10);
	strcat(bsend,buff);
        (BleDevStC.btauthoriz)? strcat(bsend," / Connected") : strcat(bsend," / Disconnected");
	if ((BleDevStC.btauthoriz) && (BleDevStC.iRssi)) {
	strcat(bsend," / ");
	itoa(BleDevStC.iRssi,buff,10);
	strcat(bsend,buff);
	strcat(bsend," dB");
	}

	strcat(bsend,"</td></tr><tr><td>BLE 4 connection count / state / RSSI</td><td>");
	itoa(BleDevStD.NumConn,buff,10);
	strcat(bsend,buff);
        (BleDevStD.btauthoriz)? strcat(bsend," / Connected") : strcat(bsend," / Disconnected");
	if ((BleDevStD.btauthoriz) && (BleDevStD.iRssi)) {
	strcat(bsend," / ");
	itoa(BleDevStD.iRssi,buff,10);
	strcat(bsend,buff);
	strcat(bsend," dB");
	}

	strcat(bsend,"</td></tr><tr><td>BLE 5 connection count / state / RSSI</td><td>");
	itoa(BleDevStE.NumConn,buff,10);
	strcat(bsend,buff);
        (BleDevStE.btauthoriz)? strcat(bsend," / Connected") : strcat(bsend," / Disconnected");
	if ((BleDevStE.btauthoriz) && (BleDevStE.iRssi)) {
	strcat(bsend," / ");
	itoa(BleDevStE.iRssi,buff,10);
	strcat(bsend,buff);
	strcat(bsend," dB");
	}

	strcat(bsend,"</td></tr>");
	wifi_ap_record_t wifidata;
        memset(wifidata.ssid,0,31);
	if (esp_wifi_sta_get_ap_info(&wifidata)==0){
	strcat(bsend,"<tr><td>WiFi connection count / RSSI / Name</td><td>");
	itoa(NumWfConn,buff,10);
	strcat(bsend,buff);
	strcat(bsend," / ");
	itoa(wifidata.rssi,buff,10);
	strcat(bsend,buff);
	strcat(bsend," dB / ");
	memcpy(buff,wifidata.ssid,31);
	strcat(bsend,buff);
	strcat(bsend,"</td></tr><tr><td>WiFi IP / MAC</td><td>");
	strcat(bsend,bufip);
	strcat(bsend," / ");
	strcat(bsend,tESP32Addr1);
	strcat(bsend,"</td></tr>");
	}
	strcat(bsend,"<tr><td>MQTT server:port</td><td>");
	if (MQTT_SERVER[0]) strcat(bsend,MQTT_SERVER);
	strcat(bsend,":");
	itoa(mqtt_port,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"<tr><td>MQTT connection count / state</td><td>");
	itoa(NumMqConn,buff,10);
	strcat(bsend,buff);
	strcat(bsend," / ");
        (mqttConnected)? strcat(bsend,"Connected") : strcat(bsend,"Disconnected");
#ifdef USE_TFT
	strcat(bsend,"</td></tr>");
	strcat(bsend,"<tr><td>TFT 320*240 LCD</td><td>");
	if (!tft_conf) strcat(bsend,"Not defined");
	else switch (tft_conn) {
	case 0:
	strcat(bsend,"Disconnected");
	break;
	case 1:
	strcat(bsend,"ILI9341 connected");
	break;
	case 2:
	strcat(bsend,"ILI9342 connected");
	break;
	}
#endif
	strcat(bsend,"</td></tr></table><footer><h6>More info on <a href='https://github.com/alutov/ESP32-R4sGate-for-Redmond' style='font-size: 15px; text-decoration: none'>github.com/alutov</a></h6>");

//itoa(strlen(bsend),buff,10);
//strcat(bsend,buff);

	strcat(bsend,"</body></html>");
	httpd_resp_sendstr(req, bsend);
	free(bsend);
	}
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
	if (blenum > 4) return;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
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
	strcat(bsend," ");
	if ((ptr->DEV_TYP > 0) && (ptr->DEV_TYP < 10)) strcat(bsend,"Kettle");
	if ((ptr->DEV_TYP > 9) && (ptr->DEV_TYP < 11)) strcat(bsend,"Power");
	if ((ptr->DEV_TYP > 10) && (ptr->DEV_TYP < 12)) strcat(bsend,"Heater");
	if ((ptr->DEV_TYP > 11) && (ptr->DEV_TYP < 16)) strcat(bsend,"Coffee");
	if ((ptr->DEV_TYP > 15) && (ptr->DEV_TYP < 24)) strcat(bsend,"Cooker");
	if ((ptr->DEV_TYP > 23) && (ptr->DEV_TYP < 48)) strcat(bsend,"Oven");
	if ((ptr->DEV_TYP > 47) && (ptr->DEV_TYP < 61)) strcat(bsend,"Baker");
	if (ptr->DEV_TYP == 61) strcat(bsend,"Iron");
	if (ptr->DEV_TYP == 62) strcat(bsend,"Smoke");
	if (ptr->DEV_TYP == 63) strcat(bsend,"Weather");
	if ((ptr->DEV_TYP > 63) && (ptr->DEV_TYP < 73)) strcat(bsend,"Kettle");
	if (ptr->DEV_TYP == 73) strcat(bsend,"Galcon");
	if (ptr->DEV_TYP == 74) strcat(bsend,"Blinds");
	strcat(bsend," Control</h3><br/>");
	if (!ptr->REQ_NAME[0] || !ptr->DEV_TYP || !ptr->btauthoriz) {
	if (ptr->REQ_NAME[0] && ptr->DEV_TYP && (ptr->DEV_TYP < 128) && !ptr->btauthoriz) {
	if ((ptr->DEV_TYP > 0) && (ptr->DEV_TYP < 10)) strcat(bsend,"Kettle");
	if ((ptr->DEV_TYP > 9) && (ptr->DEV_TYP < 11)) strcat(bsend,"Power");
	if ((ptr->DEV_TYP > 10) && (ptr->DEV_TYP < 12)) strcat(bsend,"Heater");
	if ((ptr->DEV_TYP > 11) && (ptr->DEV_TYP < 16)) strcat(bsend,"Coffee");
	if ((ptr->DEV_TYP > 15) && (ptr->DEV_TYP < 24)) strcat(bsend,"Cooker");
	if ((ptr->DEV_TYP > 23) && (ptr->DEV_TYP < 48)) strcat(bsend,"Oven");
	if ((ptr->DEV_TYP > 47) && (ptr->DEV_TYP < 61)) strcat(bsend,"Baker");
	if (ptr->DEV_TYP == 61) strcat(bsend,"Iron");
	if (ptr->DEV_TYP == 62) strcat(bsend,"Smoke");
	if (ptr->DEV_TYP == 63) strcat(bsend,"Weather");
	if ((ptr->DEV_TYP > 63) && (ptr->DEV_TYP < 73)) strcat(bsend,"Kettle");
	if (ptr->DEV_TYP == 73) strcat(bsend,"Galcon");
	if (ptr->DEV_TYP == 74) strcat(bsend,"Blinds");
	strcat(bsend," ");
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
	strcat(bsend,"value=\"25\">Boil ");
	if (volperc) strcat(bsend,"100%");
	else strcat(bsend,"1l");
	strcat(bsend," On</option></select>Select state</br>");
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
	} else if ( ptr->DEV_TYP == 61) {
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
	} else if ( ptr->DEV_TYP == 62) {
/*
	strcat(bsend,"<body><form method=\"POST\" action=\"/cfgdev");
	itoa(blenum1,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"ok\">");
	strcat(bsend,"<select name=\"sstate\"><option ");
	strcat(bsend,"value=\"62\">Clear Smoke</option></select>Select state</br>");
*/
	strcat(bsend,"No control available</br>");
	strcat(bsend,"<body><form method=\"POST\" action=\"/setignore\">");
	} else if ( ptr->DEV_TYP == 63) {
	strcat(bsend,"<body><form method=\"POST\" action=\"/cfgdev");
	itoa(blenum1,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"ok\">");
	strcat(bsend,"<select name=\"sstate\"><option ");
	strcat(bsend,"value=\"63\">Start Calibration</option></select>Select state</br>");
//	strcat(bsend,"No control available</br>");
//	strcat(bsend,"<body><form method=\"POST\" action=\"/setignore\">");
	} else if ( ptr->DEV_TYP == 73) {
	strcat(bsend,"<body><form method=\"POST\" action=\"/cfgdev");
	itoa(blenum1,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"ok\">");
	strcat(bsend,"<select name=\"sstate\"><option ");
	strcat(bsend,"value=\"72\">Switch Off</option><option ");
	if (!ptr->bState) strcat(bsend,"selected ");
	strcat(bsend,"value=\"73\">Switch On</option></select>Select state</br>");
	strcat(bsend,"<input name=\"sphour\" type=\"number\" value=\"");
	itoa(ptr->bPHour,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"23\" size=\"2\">Set Hour</br>");
	strcat(bsend,"<input name=\"spmin\" type=\"number\" value=\"");
	itoa(ptr->bPMin,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"59\" size=\"2\">Set Min</br>");
	} else if (( ptr->DEV_TYP > 63) && ( ptr->DEV_TYP < 73)){
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
	} else if ( ptr->DEV_TYP == 74) {
	strcat(bsend,"<body><form method=\"POST\" action=\"/cfgdev");
	itoa(blenum1,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"ok\">");
	strcat(bsend,"<select name=\"sstate\"><option ");
	if ((ptr->bModProg & 0x0c) == 0x0c) strcat(bsend,"value=\"26\">Set position</option><option ");
	if (!ptr->bState) strcat(bsend,"selected ");
	strcat(bsend,"value=\"1\">Stop</option><option ");
	strcat(bsend,"value=\"2\">Open</option><option ");
	strcat(bsend,"value=\"3\">Close</option></select>Select state</br>");
	strcat(bsend,"<input name=\"stemp\" type=\"number\" value=\"0\" min=\"0\" max=\"100\" size=\"3\">Position 0-100%</br>");
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
	strcat(bsend,"value=\"8\">Lock On</option><option ");
	strcat(bsend,"value=\"21\">Keep Off</option><option ");
	strcat(bsend,"value=\"22\">Keep On</option></select>Select state</br>");
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
	} else if ( ptr->DEV_TYP < 61) {
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
	strcat(bsend,"value=\"0\">Multicooker / &#1052;&#1091;&#1083;&#1100;&#1090;&#1080;&#1087;&#1086;&#1074;&#1072;&#1088;</option><option ");
	if (ptr->bProg == 1) strcat(bsend,"selected ");
	strcat(bsend,"value=\"1\">Rice / &#1056;&#1080;&#1089; &#1050;&#1088;&#1091;&#1087;&#1099; (0)</option><option ");
	if (ptr->bProg == 2) strcat(bsend,"selected ");
	strcat(bsend,"value=\"2\">Slow cooking / &#1058;&#1086;&#1084;&#1083;&#1077;&#1085;&#1080;&#1077; (0)</option><option ");
	if (ptr->bProg == 3) strcat(bsend,"selected ");
	strcat(bsend,"value=\"3\">Pilaf / &#1055;&#1083;&#1086;&#1074; (0)</option><option ");
	if (ptr->bProg == 4) strcat(bsend,"selected ");
	strcat(bsend,"value=\"4\">Frying / &#1046;&#1072;&#1088;&#1082;&#1072; (1-3)</option><option ");
	if (ptr->bProg == 5) strcat(bsend,"selected ");
	strcat(bsend,"value=\"5\">Stewing / &#1058;&#1091;&#1096;&#1077;&#1085;&#1080;&#1077; (1-3)</option><option ");
	if (ptr->bProg == 6) strcat(bsend,"selected ");
	strcat(bsend,"value=\"6\">Pasta / &#1052;&#1072;&#1082;&#1072;&#1088;&#1086;&#1085;&#1099; (0)</option><option ");
	if (ptr->bProg == 7) strcat(bsend,"selected ");
	strcat(bsend,"value=\"7\">Milk porridge / &#1052;&#1086;&#1083;&#1086;&#1095;&#1085;&#1072;&#1103; &#1082;&#1072;&#1096;&#1072; (0)</option><option ");
	if (ptr->bProg == 8) strcat(bsend,"selected ");
	strcat(bsend,"value=\"8\">Soup / &#1057;&#1091;&#1087; (0)</option><option ");
	if (ptr->bProg == 9) strcat(bsend,"selected ");
	strcat(bsend,"value=\"9\">Yogurt / &#1049;&#1086;&#1075;&#1091;&#1088;&#1090; (0)</option><option ");
	if (ptr->bProg == 10) strcat(bsend,"selected ");
	strcat(bsend,"value=\"10\">Baking / &#1042;&#1099;&#1087;&#1077;&#1095;&#1082;&#1072; (0)</option><option ");
	if (ptr->bProg == 11) strcat(bsend,"selected ");
	strcat(bsend,"value=\"11\">Steam / &#1055;&#1072;&#1088; (1-3)</option><option ");
	if (ptr->bProg == 12) strcat(bsend,"selected ");
	strcat(bsend,"value=\"12\">Hot / &#1042;&#1072;&#1088;&#1082;&#1072; &#1041;&#1086;&#1073;&#1086;&#1074;&#1099;&#1077; (0)</option></select>Select program</br>");
	strcat(bsend,"<select name=\"smod\"><option ");
	if (ptr->bModProg == 0) strcat(bsend,"selected ");
	strcat(bsend,"value=\"0\">Not defined (0)</option><option ");
	if (ptr->bModProg == 1) strcat(bsend,"selected ");
	strcat(bsend,"value=\"1\">Vegetables / &#1054;&#1074;&#1086;&#1097;&#1080; (1)</option><option ");
	if (ptr->bModProg == 2) strcat(bsend,"selected ");
	strcat(bsend,"value=\"2\">Fish / &#1056;&#1099;&#1073;&#1072; (2)<option ");
	if (ptr->bModProg == 3) strcat(bsend,"selected ");
	strcat(bsend,"value=\"3\">Meat / &#1052;&#1103;&#1089;&#1086; (3)</option></select>Select program mode</br>");
	} else if ( ptr->DEV_TYP == 17 ) {
// for RMC-903s
	if (ptr->bProg == 0) strcat(bsend,"selected ");
	strcat(bsend,"value=\"0\">Multicooker / &#1052;&#1091;&#1083;&#1100;&#1090;&#1080;&#1087;&#1086;&#1074;&#1072;&#1088;</option><option ");
	if (ptr->bProg == 1) strcat(bsend,"selected ");
	strcat(bsend,"value=\"1\">Milk porridge / &#1052;&#1086;&#1083;&#1086;&#1095;&#1085;&#1072;&#1103; &#1082;&#1072;&#1096;&#1072;</option><option ");
	if (ptr->bProg == 2) strcat(bsend,"selected ");
	strcat(bsend,"value=\"2\">Stewing / &#1058;&#1091;&#1096;&#1077;&#1085;&#1080;&#1077;</option><option  ");
	if (ptr->bProg == 3) strcat(bsend,"selected ");
	strcat(bsend,"value=\"3\">Frying / &#1046;&#1072;&#1088;&#1082;&#1072;</option><option ");
	if (ptr->bProg == 4) strcat(bsend,"selected ");
	strcat(bsend,"value=\"4\">Soup / &#1057;&#1091;&#1087;</option><option ");
	if (ptr->bProg == 5) strcat(bsend,"selected ");
	strcat(bsend,"value=\"5\">Steam / &#1055;&#1072;&#1088;</option><option ");
	if (ptr->bProg == 6) strcat(bsend,"selected ");
	strcat(bsend,"value=\"6\">Pasta / &#1052;&#1072;&#1082;&#1072;&#1088;&#1086;&#1085;&#1099;</option><option ");
	if (ptr->bProg == 7) strcat(bsend,"selected ");
	strcat(bsend,"value=\"7\">Slow cooking / &#1058;&#1086;&#1084;&#1083;&#1077;&#1085;&#1080;&#1077;</option><option ");
	if (ptr->bProg == 8) strcat(bsend,"selected ");
	strcat(bsend,"value=\"8\">Hot / &#1042;&#1072;&#1088;&#1082;&#1072;</option><option ");
	if (ptr->bProg == 9) strcat(bsend,"selected ");
	strcat(bsend,"value=\"9\">Baking / &#1042;&#1099;&#1087;&#1077;&#1095;&#1082;&#1072;</option><option ");
	if (ptr->bProg == 10) strcat(bsend,"selected ");
	strcat(bsend,"value=\"10\">Groats / &#1050;&#1088;&#1091;&#1087;&#1099;</option><option ");
	if (ptr->bProg == 11) strcat(bsend,"selected ");
	strcat(bsend,"value=\"11\">Pilaf / &#1055;&#1083;&#1086;&#1074;</option><option ");
	if (ptr->bProg == 12) strcat(bsend,"selected ");
	strcat(bsend,"value=\"12\">Yogurt / &#1049;&#1086;&#1075;&#1091;&#1088;&#1090;</option><option ");
	if (ptr->bProg == 13) strcat(bsend,"selected ");
	strcat(bsend,"value=\"13\">Pizza / &#1055;&#1080;&#1094;&#1094;&#1072;</option><option ");
	if (ptr->bProg == 14) strcat(bsend,"selected ");
	strcat(bsend,"value=\"14\">Bread / &#1061;&#1083;&#1077;&#1073;</option><option ");
	if (ptr->bProg == 15) strcat(bsend,"selected ");
	strcat(bsend,"value=\"15\">Desserts / &#1044;&#1077;&#1089;&#1077;&#1088;&#1090;&#1099;</option><option ");
	if (ptr->bProg == 16) strcat(bsend,"selected ");
	strcat(bsend,"value=\"16\">Express / &#1069;&#1082;&#1089;&#1087;&#1088;&#1077;&#1089;&#1089;</option></select>Select program</br>");
	strcat(bsend,"<select name=\"smod\"><option ");
	strcat(bsend,"selected ");
	strcat(bsend,"value=\"0\">Not defined</option></select>Select program mode</br>");
	} else if ( ptr->DEV_TYP == 18 ) {
// for RMC-224s
	if (ptr->bProg == 0) strcat(bsend,"selected ");
	strcat(bsend,"value=\"0\">Frying / &#1046;&#1072;&#1088;&#1082;&#1072;</option><option ");
	if (ptr->bProg == 1) strcat(bsend,"selected ");
	strcat(bsend,"value=\"1\">Groats / &#1050;&#1088;&#1091;&#1087;&#1099;</option><option ");
	if (ptr->bProg == 2) strcat(bsend,"selected ");
	strcat(bsend,"value=\"2\">Multicooker / &#1052;&#1091;&#1083;&#1100;&#1090;&#1080;&#1087;&#1086;&#1074;&#1072;&#1088;</option><option ");
	if (ptr->bProg == 3) strcat(bsend,"selected ");
	strcat(bsend,"value=\"3\">Pilaf / &#1055;&#1083;&#1086;&#1074;</option><option ");
	if (ptr->bProg == 4) strcat(bsend,"selected ");
	strcat(bsend,"value=\"4\">Steam / &#1055;&#1072;&#1088;</option><option ");
	if (ptr->bProg == 5) strcat(bsend,"selected ");
	strcat(bsend,"value=\"5\">Baking / &#1042;&#1099;&#1087;&#1077;&#1095;&#1082;&#1072;</option><option ");
	if (ptr->bProg == 6) strcat(bsend,"selected ");
	strcat(bsend,"value=\"6\">Stewing / &#1058;&#1091;&#1096;&#1077;&#1085;&#1080;&#1077;</option><option  ");
	if (ptr->bProg == 7) strcat(bsend,"selected ");
	strcat(bsend,"value=\"7\">Soup / &#1057;&#1091;&#1087;</option><option ");
	if (ptr->bProg == 8) strcat(bsend,"selected ");
	strcat(bsend,"value=\"8\">Milk porridge / &#1052;&#1086;&#1083;&#1086;&#1095;&#1085;&#1072;&#1103; &#1082;&#1072;&#1096;&#1072;</option><option ");
	if (ptr->bProg == 9) strcat(bsend,"selected ");
	strcat(bsend,"value=\"9\">Yogurt / &#1049;&#1086;&#1075;&#1091;&#1088;&#1090;</option><option ");
	if (ptr->bProg == 10) strcat(bsend,"selected ");
	strcat(bsend,"value=\"10\">Express / &#1069;&#1082;&#1089;&#1087;&#1088;&#1077;&#1089;&#1089;</option></select>Select program</br>");
	strcat(bsend,"<select name=\"smod\"><option ");
	strcat(bsend,"selected ");
	strcat(bsend,"value=\"0\">Not defined</option></select>Select program mode</br>");
	} else if ( ptr->DEV_TYP == 19 ) {
// for RMC-961s
	if (ptr->bProg == 0) strcat(bsend,"selected ");
	strcat(bsend,"value=\"0\">Groats / &#1050;&#1088;&#1091;&#1087;&#1099;</option><option ");
	if (ptr->bProg == 1) strcat(bsend,"selected ");
	strcat(bsend,"value=\"1\">Frying / &#1046;&#1072;&#1088;&#1082;&#1072;</option><option ");
	if (ptr->bProg == 2) strcat(bsend,"selected ");
	strcat(bsend,"value=\"2\">Steam / &#1055;&#1072;&#1088;</option><option ");
	if (ptr->bProg == 3) strcat(bsend,"selected ");
	strcat(bsend,"value=\"3\">Baking / &#1042;&#1099;&#1087;&#1077;&#1095;&#1082;&#1072;</option><option ");
	if (ptr->bProg == 4) strcat(bsend,"selected ");
	strcat(bsend,"value=\"4\">Stewing / &#1058;&#1091;&#1096;&#1077;&#1085;&#1080;&#1077;</option><option  ");
	if (ptr->bProg == 5) strcat(bsend,"selected ");
	strcat(bsend,"value=\"5\">Multicooker / &#1052;&#1091;&#1083;&#1100;&#1090;&#1080;&#1087;&#1086;&#1074;&#1072;&#1088;</option><option ");
	if (ptr->bProg == 6) strcat(bsend,"selected ");
	strcat(bsend,"value=\"6\">Pilaf / &#1055;&#1083;&#1086;&#1074;</option><option ");
	if (ptr->bProg == 7) strcat(bsend,"selected ");
	strcat(bsend,"value=\"7\">Soup / &#1057;&#1091;&#1087;</option><option ");
	if (ptr->bProg == 8) strcat(bsend,"selected ");
	strcat(bsend,"value=\"8\">Milk porridge / &#1052;&#1086;&#1083;&#1086;&#1095;&#1085;&#1072;&#1103; &#1082;&#1072;&#1096;&#1072;</option><option ");
	if (ptr->bProg == 9) strcat(bsend,"selected ");
	strcat(bsend,"value=\"9\">Yogurt / &#1049;&#1086;&#1075;&#1091;&#1088;&#1090;</option></select>Select program</br>");
	strcat(bsend,"<select name=\"smod\"><option ");
	strcat(bsend,"selected ");
	strcat(bsend,"value=\"0\">Not defined</option></select>Select program mode</br>");
	} else if ( ptr->DEV_TYP == 20 ) {
// for RMC-92s
	if (ptr->bProg == 0) strcat(bsend,"selected ");
	strcat(bsend,"value=\"0\">Multicooker / &#1052;&#1091;&#1083;&#1100;&#1090;&#1080;&#1087;&#1086;&#1074;&#1072;&#1088;</option><option ");
	if (ptr->bProg == 1) strcat(bsend,"selected ");
	strcat(bsend,"value=\"1\">Milk porridge / &#1052;&#1086;&#1083;&#1086;&#1095;&#1085;&#1072;&#1103; &#1082;&#1072;&#1096;&#1072;</option><option ");
	if (ptr->bProg == 2) strcat(bsend,"selected ");
	strcat(bsend,"value=\"2\">Stewing / &#1058;&#1091;&#1096;&#1077;&#1085;&#1080;&#1077;</option><option  ");
	if (ptr->bProg == 3) strcat(bsend,"selected ");
	strcat(bsend,"value=\"3\">Frying / &#1046;&#1072;&#1088;&#1082;&#1072;</option><option ");
	if (ptr->bProg == 4) strcat(bsend,"selected ");
	strcat(bsend,"value=\"4\">Soup / &#1057;&#1091;&#1087;</option><option ");
	if (ptr->bProg == 5) strcat(bsend,"selected ");
	strcat(bsend,"value=\"5\">Steam / &#1055;&#1072;&#1088;</option><option ");
	if (ptr->bProg == 6) strcat(bsend,"selected ");
	strcat(bsend,"value=\"6\">Pasta / &#1052;&#1072;&#1082;&#1072;&#1088;&#1086;&#1085;&#1099;</option><option ");
	if (ptr->bProg == 7) strcat(bsend,"selected ");
	strcat(bsend,"value=\"7\">Slow cooking / &#1058;&#1086;&#1084;&#1083;&#1077;&#1085;&#1080;&#1077;</option><option ");
	if (ptr->bProg == 8) strcat(bsend,"selected ");
	strcat(bsend,"value=\"8\">Hot / &#1042;&#1072;&#1088;&#1082;&#1072;</option><option ");
	if (ptr->bProg == 9) strcat(bsend,"selected ");
	strcat(bsend,"value=\"9\">Baking / &#1042;&#1099;&#1087;&#1077;&#1095;&#1082;&#1072;</option><option ");
	if (ptr->bProg == 10) strcat(bsend,"selected ");
	strcat(bsend,"value=\"10\">Groats / &#1050;&#1088;&#1091;&#1087;&#1099;</option><option ");
	if (ptr->bProg == 11) strcat(bsend,"selected ");
	strcat(bsend,"value=\"11\">Pilaf / &#1055;&#1083;&#1086;&#1074;</option><option ");
	if (ptr->bProg == 12) strcat(bsend,"selected ");
	strcat(bsend,"value=\"12\">Yogurt / &#1049;&#1086;&#1075;&#1091;&#1088;&#1090;</option><option ");
	if (ptr->bProg == 13) strcat(bsend,"selected ");
	strcat(bsend,"value=\"13\">Pizza / &#1055;&#1080;&#1094;&#1094;&#1072;</option><option ");
	if (ptr->bProg == 14) strcat(bsend,"selected ");
	strcat(bsend,"value=\"14\">Bread / &#1061;&#1083;&#1077;&#1073;</option><option ");
	if (ptr->bProg == 15) strcat(bsend,"selected ");
	strcat(bsend,"value=\"15\">Desserts / &#1044;&#1077;&#1089;&#1077;&#1088;&#1090;&#1099;</option><option ");
	if (ptr->bProg == 16) strcat(bsend,"selected ");
	strcat(bsend,"value=\"16\">Express / &#1069;&#1082;&#1089;&#1087;&#1088;&#1077;&#1089;&#1089;</option><option ");
	if (ptr->bProg == 17) strcat(bsend,"selected ");
	strcat(bsend,"value=\"17\">Warming / &#1056;&#1072;&#1079;&#1086;&#1075;&#1088;&#1077;&#1074;</option></select>Select program</br>");
	strcat(bsend,"<select name=\"smod\"><option ");
	strcat(bsend,"selected ");
	strcat(bsend,"value=\"0\">Not defined</option></select>Select program mode</br>");
	} else if ( ptr->DEV_TYP == 24 ) {
	if (ptr->bProg == 0) strcat(bsend,"selected ");
	strcat(bsend,"value=\"0\">Multicooker / &#1052;&#1091;&#1083;&#1100;&#1090;&#1080;&#1087;&#1086;&#1074;&#1072;&#1088;</option><option ");
	if (ptr->bProg == 1) strcat(bsend,"selected ");
	strcat(bsend,"value=\"1\">Omelet / &#1054;&#1084;&#1083;&#1077;&#1090;</option><option ");
	if (ptr->bProg == 2) strcat(bsend,"selected ");
	strcat(bsend,"value=\"2\">Slow cooking meat / &#1058;&#1086;&#1084;&#1083;&#1077;&#1085;&#1080;&#1077; &#1052;&#1103;&#1089;&#1086;</option><option  ");
	if (ptr->bProg == 3) strcat(bsend,"selected ");
	strcat(bsend,"value=\"3\">Slow cooking bird / &#1058;&#1086;&#1084;&#1083;&#1077;&#1085;&#1080;&#1077; &#1055;&#1090;&#1080;&#1094;&#1072;</option><option ");
	if (ptr->bProg == 4) strcat(bsend,"selected ");
	strcat(bsend,"value=\"4\">Slow cooking fish / &#1058;&#1086;&#1084;&#1083;&#1077;&#1085;&#1080;&#1077;  &#1056;&#1099;&#1073;&#1072;</option><option ");
	if (ptr->bProg == 5) strcat(bsend,"selected ");
	strcat(bsend,"value=\"5\">Slow cooking vegetables / &#1058;&#1086;&#1084;&#1083;&#1077;&#1085;&#1080;&#1077; &#1054;&#1074;&#1086;&#1097;&#1080;</option><option ");
	if (ptr->bProg == 6) strcat(bsend,"selected ");
	strcat(bsend,"value=\"6\">Bread / &#1061;&#1083;&#1077;&#1073;</option><option ");
	if (ptr->bProg == 7) strcat(bsend,"selected ");
	strcat(bsend,"value=\"7\">Pizza / &#1055;&#1080;&#1094;&#1094;&#1072;</option><option ");
	if (ptr->bProg == 8) strcat(bsend,"selected ");
	strcat(bsend,"value=\"8\">Charlotte / &#1064;&#1072;&#1088;&#1083;&#1086;&#1090;&#1082;&#1072;</option><option ");
	if (ptr->bProg == 9) strcat(bsend,"selected ");
	strcat(bsend,"value=\"9\">Baking meat in pot / &#1047;&#1072;&#1087;&#1077;&#1082;&#1072;&#1085;&#1080;&#1077; &#1074; &#1075;&#1086;&#1088;&#1096;&#1086;&#1095;&#1082;&#1077; &#1052;&#1103;&#1089;&#1086;</option><option ");
	if (ptr->bProg == 10) strcat(bsend,"selected ");
	strcat(bsend,"value=\"10\">Baking bird in pot / &#1047;&#1072;&#1087;&#1077;&#1082;&#1072;&#1085;&#1080;&#1077; &#1074; &#1075;&#1086;&#1088;&#1096;&#1086;&#1095;&#1082;&#1077; &#1055;&#1090;&#1080;&#1094;&#1072;</option><option ");
	if (ptr->bProg == 11) strcat(bsend,"selected ");
	strcat(bsend,"value=\"11\">Baking fish in pot / &#1047;&#1072;&#1087;&#1077;&#1082;&#1072;&#1085;&#1080;&#1077; &#1074; &#1075;&#1086;&#1088;&#1096;&#1086;&#1095;&#1082;&#1077; &#1056;&#1099;&#1073;&#1072;</option><option ");
	if (ptr->bProg == 12) strcat(bsend,"selected ");
	strcat(bsend,"value=\"12\">Baking vegetables in pot / &#1047;&#1072;&#1087;&#1077;&#1082;&#1072;&#1085;&#1080;&#1077; &#1074; &#1075;&#1086;&#1088;&#1096;&#1086;&#1095;&#1082;&#1077; &#1054;&#1074;&#1086;&#1097;&#1080;</option><option ");
	if (ptr->bProg == 13) strcat(bsend,"selected ");
	strcat(bsend,"value=\"13\">Roast / &#1046;&#1072;&#1088;&#1082;&#1086;&#1077;</option><option ");
	if (ptr->bProg == 14) strcat(bsend,"selected ");
	strcat(bsend,"value=\"14\">Cake / &#1050;&#1077;&#1082;&#1089;</option><option ");
	if (ptr->bProg == 15) strcat(bsend,"selected ");
	strcat(bsend,"value=\"15\">Baking meat / &#1047;&#1072;&#1087;&#1077;&#1082;&#1072;&#1085;&#1080;&#1077; &#1052;&#1103;&#1089;&#1086;</option><option ");
	if (ptr->bProg == 16) strcat(bsend,"selected ");
	strcat(bsend,"value=\"16\">Baking bird / &#1047;&#1072;&#1087;&#1077;&#1082;&#1072;&#1085;&#1080;&#1077; &#1055;&#1090;&#1080;&#1094;&#1072;</option><option ");
	if (ptr->bProg == 17) strcat(bsend,"selected ");
	strcat(bsend,"value=\"17\">Baking fish / &#1047;&#1072;&#1087;&#1077;&#1082;&#1072;&#1085;&#1080;&#1077; &#1056;&#1099;&#1073;&#1072;</option><option ");
	if (ptr->bProg == 18) strcat(bsend,"selected ");
	strcat(bsend,"value=\"18\">Baking vegetables / &#1047;&#1072;&#1087;&#1077;&#1082;&#1072;&#1085;&#1080;&#1077; &#1054;&#1074;&#1086;&#1097;&#1080;</option><option ");
	if (ptr->bProg == 19) strcat(bsend,"selected ");
	strcat(bsend,"value=\"19\">Boiled pork / &#1041;&#1091;&#1078;&#1077;&#1085;&#1080;&#1085;&#1072;</option><option ");
	if (ptr->bProg == 20) strcat(bsend,"selected ");
	strcat(bsend,"value=\"20\">Warming / &#1056;&#1072;&#1079;&#1086;&#1075;&#1088;&#1077;&#1074;</option></select>Select program</br>");
	strcat(bsend,"<select name=\"smod\"><option ");
	if (ptr->bModProg == 0) strcat(bsend,"selected ");
	strcat(bsend,"value=\"0\">Top heater</option><option ");
	if (ptr->bModProg == 1) strcat(bsend,"selected ");
	strcat(bsend,"value=\"1\">Bottom heater<option ");
	if (ptr->bModProg == 2) strcat(bsend,"selected ");
	strcat(bsend,"value=\"2\">Top and bottom heaters</option></select>Select heating mode</br>");
	} else if ( ptr->DEV_TYP == 48 ) {
	if (ptr->bProg == 0) strcat(bsend,"selected ");
	strcat(bsend,"value=\"0\">Manual</option><option ");
	if (ptr->bProg == 1) strcat(bsend,"selected ");
	strcat(bsend,"value=\"1\">Fry</option><option ");
	if (ptr->bProg == 2) strcat(bsend,"selected ");
	strcat(bsend,"value=\"2\">Heating</option></select>Select program</br>");
	}
	if ( ptr->DEV_TYP != 48 ) {
	strcat(bsend,"<input name=\"stemp\" type=\"number\" value=\"");
	itoa(ptr->bHtemp,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"230\" size=\"3\">Set Temp 0-230&deg;C</br>");
	}
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
	ble_mon_refr = ble_mon_refr & 0xfd;
	char *bsend = NULL;
	bsend = malloc(13000);
	if (bsend == NULL) {
	if (fdebug) ESP_LOGE(AP_TAG, "Http devh1: No memory");
	MemErr++;
	if (!MemErr) MemErr--;
	httpd_resp_set_status(req, "303 See Other");
	httpd_resp_set_hdr(req, "Location", "/");
	httpd_resp_send(req, NULL, 0);  // Response body can be empty
	} else {	
        char buff[64];
	strcpy(bsend,"<!DOCTYPE html><html>");
	strcat(bsend,"<head><title>r4sGate</title>");
	strcat(bsend,"<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
	strcat(bsend,cssDatasheet);
	strcat(bsend,"</head><body class='bodymenu'><header class='headermenu'><h1>ESP32 r4sGate");
	itoa(R4SNUM,buff,10);
	strcat(bsend, buff);
	strcat(bsend," for Redmond +</h1>");
	strcat(bsend,"<div class='menubar'><a class='menu' href='.'>&#8962;<span class='showmenulabel'>Main");
	strcat(bsend,"</span></a><a class='menu active' href='cfgdev1'>&#128246;<span class='showmenulabel'> 1 ");
	(BleDevStA.RQC_NAME[0])? strcat(bsend,BleDevStA.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev2'>&#128246;<span class='showmenulabel'> 2 ");
	(BleDevStB.RQC_NAME[0])? strcat(bsend,BleDevStB.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev3'>&#128246;<span class='showmenulabel'> 3 ");
	(BleDevStC.RQC_NAME[0])? strcat(bsend,BleDevStC.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev4'>&#128246;<span class='showmenulabel'> 4 ");
	(BleDevStD.RQC_NAME[0])? strcat(bsend,BleDevStD.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev5'>&#128246;<span class='showmenulabel'> 5 ");
	(BleDevStE.RQC_NAME[0])? strcat(bsend,BleDevStE.RQC_NAME) : strcat(bsend,"Not defined");
	if (ble_mon) strcat(bsend,"</span></a><a class='menu' href='blemon'>&#128246;<span class='showmenulabel'>BLE monitor</span></a>");
	strcat(bsend,"</span></a><a class='menu' href='setting'>&#9881;<span class='showmenulabel'>Setting</span></a>");
	strcat(bsend,"<a class='menu' href='restart'>&#128259;<span class='showmenulabel'>Reboot</span></a>");
	strcat(bsend,"<a class='menu' href='update'>&#10548;<span class='showmenulabel'>Load firmware</span></a></div>");

	strcat(bsend,"</header>");
        HtpDeVHandle(0, bsend);

//itoa(strlen(bsend),buff,10);
//strcat(bsend,buff);

	httpd_resp_sendstr(req, bsend);
	free(bsend);
	}
	return ESP_OK;
}
static const httpd_uri_t pcfgdev1 = {
	.uri       = "/cfgdev1",
	.method    = HTTP_GET,
	.handler   = pcfgdev1_get_handler,
	.user_ctx  = NULL
};


uint8_t pcfgdev (uint8_t blenum, char *buf1, size_t len)
{
	if (blenum > 4) return 1;
        struct BleDevSt *ptr;
	char buf2[16] = {0};
	char buf3[16] = {0};
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	int8_t bnewBlTime = ptr->bBlTime;
	int  cm_done = ptr->bProg;
	buf3[0] = 0;
	strcpy(buf2,"stemp");
	parsuri(buf1,buf3,buf2,len,4,0);
	uint8_t temp = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"sstate");
	parsuri(buf1,buf3,buf2,len,4,0);
	uint8_t state = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"sprog");
	parsuri(buf1,buf3,buf2,len,4,0);
	uint8_t pprog = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"smod");
	parsuri(buf1,buf3,buf2,len,4,0);
	uint8_t pmod = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"sphour");
	parsuri(buf1,buf3,buf2,len,4,0);
	uint8_t phour = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"spmin");
	parsuri(buf1,buf3,buf2,len,4,0);
	uint8_t pmin = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"sdhour");
	parsuri(buf1,buf3,buf2,len,4,0);
	uint8_t dhour = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"sdmin");
	parsuri(buf1,buf3,buf2,len,4,0);
	uint8_t dmin = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"swarm");
	parsuri(buf1,buf3,buf2,len,4,0);
	uint8_t warm = atoi(buf3);
	if (ptr->DEV_TYP < 10) {
	buf3[0] = 0;
	strcpy(buf2,"rlight");
	parsuri(buf1,buf3,buf2,len,4,0);
	ptr->RgbR = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"glight");
	parsuri(buf1,buf3,buf2,len,4,0);
	ptr->RgbG = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"blight");
	parsuri(buf1,buf3,buf2,len,4,0);
	ptr->RgbB = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"sbltim");
	parsuri(buf1,buf3,buf2,len,3,0);
	bnewBlTime = atoi(buf3);
	} else if (ptr->DEV_TYP == 74) {
	switch (state) {
	case 1:
	state = 26;
	temp = 254;
	break;
	case 2:
	state = 26;
	temp = 100;
	break;
	case 3:
	state = 26;
	temp = 0;
	break;
	}
	}
	switch (state) {    //kettle
	case 1:             //off
	cm_done = 1;
	ptr->r4slppar1 = pmod;		
	ptr->r4slpcom = 1;
	break;
	case 2:             //boil(on)
	if (bnewBlTime != ptr->bBlTime) {
	cm_done = 0;
	ptr->r4slppar1 = bnewBlTime;
	ptr->r4slpcom = 25;
	} else {
	cm_done = 1;
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 2;
	}
	break;
	case 3:             //heat(on)
	cm_done = 1;
	if (ptr->DEV_TYP == 1) {	
	if (temp < 30) ptr->r4slppar1 = 0;
	else if (temp < 41) ptr->r4slppar1 = 1;
	else if (temp < 56) ptr->r4slppar1 = 2;
	else if (temp < 71) ptr->r4slppar1 = 3;
	else if (temp < 86) ptr->r4slppar1 = 4;
	else ptr->r4slppar1 = 5;
	} else ptr->r4slppar1 = temp;
	if (temp > 29) {
	ptr->r4slppar2 = 1;
	ptr->r4slpcom = 3;
	} else {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 1;
	}
	break;
	case 4:             //boil&heat(on)
	cm_done = 1;
	if (ptr->DEV_TYP == 1) {	
	if (temp < 30) ptr->r4slppar1 = 0;
	else if (temp < 41) ptr->r4slppar1 = 1;
	else if (temp < 56) ptr->r4slppar1 = 2;
	else if (temp < 71) ptr->r4slppar1 = 3;
	else if (temp < 86) ptr->r4slppar1 = 4;
	else ptr->r4slppar1 = 5;
	} else ptr->r4slppar1 = temp;
	ptr->r4slpcom = 4;
	break;
	case 5:             //nightlight(on)
	cm_done = 1;
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 5;
	break;
	case 6:             //power coffee on
	cm_done = 1;
	ptr->r4slppar2 = temp;
	ptr->r4slppar1 = pmod;
	ptr->r4slpcom = 6;
	break;
	case 7:             //lock off
	cm_done = 1;
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 7;
	break;
	case 8:             //lock on
	cm_done = 1;
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 8;
	break;
	case 11:             //cooker off
	cm_done = 1;
	ptr->r4slppar1 = 0;		
	ptr->r4slpcom = 10;
	break;
	case 12:             //cooker warming
	cm_done = 1;
	ptr->r4slppar1 = 1;		
	ptr->r4slpcom = 10;
	break;
	case 13:             //cooker set prog
	cm_done = 0;
	ptr->r4slppar1 = pprog;		
	ptr->r4slppar2 = pmod;		
	ptr->r4slppar3 = temp;		
	ptr->r4slppar4 = phour;		
	ptr->r4slppar5 = pmin;		
	ptr->r4slppar6 = warm;		
	ptr->r4slppar7 = dhour;		
	ptr->r4slppar8 = dmin;
	ptr->r4slpcom = 17;
	break;
	case 14:             //cooker warming
	cm_done = 1;
	ptr->r4slppar1 = 1;		
	ptr->r4slpcom = 10;
	break;
	case 18:             //coffee delay start
	cm_done = 1;
	ptr->r4slppar1 = pmod;
	ptr->r4slppar2 = phour;
	ptr->r4slppar3 = pmin;
	ptr->r4slpcom = 18;
	break;
	case 21:             //blight keep off
	cm_done = 1;
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 21;
	break;
	case 22:             //blight keep on
	cm_done = 1;
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 22;
	break;
	case 23:             //beep off
	cm_done = 1;
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 23;
	break;
	case 24:             //beep on
	cm_done = 1;
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 24;
	break;
	case 25:             //boil 1l(on)
	cm_done = 1;
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 2;
	if (ptr->DEV_TYP > 3) ptr->bCVol = 252;
	break;
	case 26:             //set blinds
	cm_done = 1;
	ptr->r4slppar1 = temp;
	ptr->r4slpcom = 26;
	break;
	case 62:             //clear smoke
	cm_done = 1;
	ptr->r4slppar1 = 0;		
	ptr->r4slpcom = 62;
	break;
	case 63:             //weather calibration
	cm_done = 1;
	ptr->r4slppar1 = 0;		
	ptr->r4slpcom = 63;
	break;
	case 64:             //mi off
	cm_done = 1;
	ptr->r4slppar1 = 0;		
	ptr->r4slpcom = 64;
	break;
	case 65:             //mi boil(on)
	cm_done = 1;
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 65;
	break;
	case 66:             //mi heat(on)
	cm_done = 1;
	ptr->r4slppar1 = temp;
	ptr->r4slpcom = 66;
	break;
	case 72:             //9001 off
	cm_done = 1;
	ptr->bPHour = phour;
	ptr->bPMin = pmin;
	ptr->r4slppar1 = 0;		
	ptr->r4slpcom = 72;
	break;
	case 73:             //9001 on
	cm_done = 1;
	ptr->bPHour = phour;
	ptr->bPMin = pmin;
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 73;
	break;
	}
	ptr->r4slpres = 1;
	uint8_t timeout = 35; 	// 100*35 = 3.5 seconds
	while (--timeout && (ptr->r4slpres)) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
return cm_done;
}

static esp_err_t pcfgdev1ok_get_handler(httpd_req_t *req)
{
	char buf1[512] = {0};
	int  ret;
	ret = httpd_req_recv(req,buf1,512);
/*

in buf1 after httpd_req_recv string like below
swfid=wifiname&swfpsw=wifipassword&smqsrv=192.168.1.10&smqid=esp&
smqpsw=esp&devnam=&rlight=255&glight=255&blight=255&chk2=2

*/
	if ( ret > 0 ) ret = pcfgdev (0, buf1, 512);
	httpd_resp_set_status(req, "303 See Other");
	if (!ret) httpd_resp_set_hdr(req, "Location", "/cfgdev1");
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
	ble_mon_refr = ble_mon_refr & 0xfd;
	char *bsend = NULL;
	bsend = malloc(13000);
	if (bsend == NULL) {
	if (fdebug) ESP_LOGE(AP_TAG, "Http devh2: No memory");
	MemErr++;
	if (!MemErr) MemErr--;
	httpd_resp_set_status(req, "303 See Other");
	httpd_resp_set_hdr(req, "Location", "/");
	httpd_resp_send(req, NULL, 0);  // Response body can be empty
	} else {	
        char buff[64];
	strcpy(bsend,"<!DOCTYPE html><html>");
	strcat(bsend,"<head><title>r4sGate</title>");
	strcat(bsend,"<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
	strcat(bsend,cssDatasheet);
	strcat(bsend,"</head><body class='bodymenu'><header class='headermenu'><h1>ESP32 r4sGate");
	itoa(R4SNUM,buff,10);
	strcat(bsend, buff);
	strcat(bsend," for Redmond +</h1>");
	strcat(bsend,"<div class='menubar'><a class='menu' href='.'>&#8962;<span class='showmenulabel'>Main");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev1'>&#128246;<span class='showmenulabel'> 1 ");
	(BleDevStA.RQC_NAME[0])? strcat(bsend,BleDevStA.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu active' href='cfgdev2'>&#128246;<span class='showmenulabel'> 2 ");
	(BleDevStB.RQC_NAME[0])? strcat(bsend,BleDevStB.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev3'>&#128246;<span class='showmenulabel'> 3 ");
	(BleDevStC.RQC_NAME[0])? strcat(bsend,BleDevStC.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev4'>&#128246;<span class='showmenulabel'> 4 ");
	(BleDevStD.RQC_NAME[0])? strcat(bsend,BleDevStD.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev5'>&#128246;<span class='showmenulabel'> 5 ");
	(BleDevStE.RQC_NAME[0])? strcat(bsend,BleDevStE.RQC_NAME) : strcat(bsend,"Not defined");
	if (ble_mon) strcat(bsend,"</span></a><a class='menu' href='blemon'>&#128246;<span class='showmenulabel'>BLE monitor</span></a>");
	strcat(bsend,"</span></a><a class='menu' href='setting'>&#9881;<span class='showmenulabel'>Setting</span></a>");
	strcat(bsend,"<a class='menu' href='restart'>&#128259;<span class='showmenulabel'>Reboot</span></a>");
	strcat(bsend,"<a class='menu' href='update'>&#10548;<span class='showmenulabel'>Load firmware</span></a></div>");

	strcat(bsend,"</header>");
        HtpDeVHandle(1, bsend);

//itoa(strlen(bsend),buff,10);
//strcat(bsend,buff);

	httpd_resp_sendstr(req, bsend);
	free(bsend);
	}
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
	int  ret;
	ret = httpd_req_recv(req,buf1,512);
/*

in buf1 after httpd_req_recv string like below
swfid=wifiname&swfpsw=wifipassword&smqsrv=192.168.1.10&smqid=esp&
smqpsw=esp&devnam=&rlight=255&glight=255&blight=255&chk2=2

*/
	if ( ret > 0 ) ret = pcfgdev (1, buf1, 512);
	httpd_resp_set_status(req, "303 See Other");
	if (!ret) httpd_resp_set_hdr(req, "Location", "/cfgdev2");
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
	ble_mon_refr = ble_mon_refr & 0xfd;
	char *bsend = NULL;
	bsend = malloc(13000);
	if (bsend == NULL) {
	if (fdebug) ESP_LOGE(AP_TAG, "Http devh3: No memory");
	MemErr++;
	if (!MemErr) MemErr--;
	httpd_resp_set_status(req, "303 See Other");
	httpd_resp_set_hdr(req, "Location", "/");
	httpd_resp_send(req, NULL, 0);  // Response body can be empty
	} else {	
        char buff[32];
	strcpy(bsend,"<!DOCTYPE html><html>");
	strcat(bsend,"<head><title>r4sGate</title>");
	strcat(bsend,"<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
	strcat(bsend,cssDatasheet);
	strcat(bsend,"</head><body class='bodymenu'><header class='headermenu'><h1>ESP32 r4sGate");
	itoa(R4SNUM,buff,10);
	strcat(bsend, buff);
	strcat(bsend," for Redmond +</h1>");
	strcat(bsend,"<div class='menubar'><a class='menu' href='.'>&#8962;<span class='showmenulabel'>Main");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev1'>&#128246;<span class='showmenulabel'> 1 ");
	(BleDevStA.RQC_NAME[0])? strcat(bsend,BleDevStA.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev2'>&#128246;<span class='showmenulabel'> 2 ");
	(BleDevStB.RQC_NAME[0])? strcat(bsend,BleDevStB.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu active' href='cfgdev3'>&#128246;<span class='showmenulabel'> 3 ");
	(BleDevStC.RQC_NAME[0])? strcat(bsend,BleDevStC.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev4'>&#128246;<span class='showmenulabel'> 4 ");
	(BleDevStD.RQC_NAME[0])? strcat(bsend,BleDevStD.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev5'>&#128246;<span class='showmenulabel'> 5 ");
	(BleDevStE.RQC_NAME[0])? strcat(bsend,BleDevStE.RQC_NAME) : strcat(bsend,"Not defined");
	if (ble_mon) strcat(bsend,"</span></a><a class='menu' href='blemon'>&#128246;<span class='showmenulabel'>BLE monitor</span></a>");
	strcat(bsend,"</span></a><a class='menu' href='setting'>&#9881;<span class='showmenulabel'>Setting</span></a>");
	strcat(bsend,"<a class='menu' href='restart'>&#128259;<span class='showmenulabel'>Reboot</span></a>");
	strcat(bsend,"<a class='menu' href='update'>&#10548;<span class='showmenulabel'>Load firmware</span></a></div>");

	strcat(bsend,"</header>");
        HtpDeVHandle(2, bsend);

//itoa(strlen(bsend),buff,10);
//strcat(bsend,buff);

	httpd_resp_sendstr(req, bsend);
	free(bsend);
	}
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
	int  ret;
	ret = httpd_req_recv(req,buf1,512);
/*

in buf1 after httpd_req_recv string like below
swfid=wifiname&swfpsw=wifipassword&smqsrv=192.168.1.10&smqid=esp&
smqpsw=esp&devnam=&rlight=255&glight=255&blight=255&chk2=2

*/
	if ( ret > 0 ) ret = pcfgdev (2, buf1, 512);
	httpd_resp_set_status(req, "303 See Other");
	if (!ret) httpd_resp_set_hdr(req, "Location", "/cfgdev3");
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



/* HTTP GET device4 handler */
static esp_err_t pcfgdev4_get_handler(httpd_req_t *req)
{
	if (!test_auth(req)) {
		return ESP_OK;
	}
	ble_mon_refr = ble_mon_refr & 0xfd;
	char *bsend = NULL;
	bsend = malloc(13000);
	if (bsend == NULL) {
	if (fdebug) ESP_LOGE(AP_TAG, "Http devh4: No memory");
	MemErr++;
	if (!MemErr) MemErr--;
	httpd_resp_set_status(req, "303 See Other");
	httpd_resp_set_hdr(req, "Location", "/");
	httpd_resp_send(req, NULL, 0);  // Response body can be empty
	} else {	
        char buff[32];
	strcpy(bsend,"<!DOCTYPE html><html>");
	strcat(bsend,"<head><title>r4sGate</title>");
	strcat(bsend,"<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
	strcat(bsend,cssDatasheet);
	strcat(bsend,"</head><body class='bodymenu'><header class='headermenu'><h1>ESP32 r4sGate");
	itoa(R4SNUM,buff,10);
	strcat(bsend, buff);
	strcat(bsend," for Redmond +</h1>");
	strcat(bsend,"<div class='menubar'><a class='menu' href='.'>&#8962;<span class='showmenulabel'>Main");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev1'>&#128246;<span class='showmenulabel'> 1 ");
	(BleDevStA.RQC_NAME[0])? strcat(bsend,BleDevStA.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev2'>&#128246;<span class='showmenulabel'> 2 ");
	(BleDevStB.RQC_NAME[0])? strcat(bsend,BleDevStB.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev3'>&#128246;<span class='showmenulabel'> 3 ");
	(BleDevStC.RQC_NAME[0])? strcat(bsend,BleDevStC.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu active' href='cfgdev4'>&#128246;<span class='showmenulabel'> 4 ");
	(BleDevStD.RQC_NAME[0])? strcat(bsend,BleDevStD.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev5'>&#128246;<span class='showmenulabel'> 5 ");
	(BleDevStE.RQC_NAME[0])? strcat(bsend,BleDevStE.RQC_NAME) : strcat(bsend,"Not defined");
	if (ble_mon) strcat(bsend,"</span></a><a class='menu' href='blemon'>&#128246;<span class='showmenulabel'>BLE monitor</span></a>");
	strcat(bsend,"</span></a><a class='menu' href='setting'>&#9881;<span class='showmenulabel'>Setting</span></a>");
	strcat(bsend,"<a class='menu' href='restart'>&#128259;<span class='showmenulabel'>Reboot</span></a>");
	strcat(bsend,"<a class='menu' href='update'>&#10548;<span class='showmenulabel'>Load firmware</span></a></div>");

	strcat(bsend,"</header>");
        HtpDeVHandle(3, bsend);

//itoa(strlen(bsend),buff,10);
//strcat(bsend,buff);

	httpd_resp_sendstr(req, bsend);
	free(bsend);
	}
	return ESP_OK;
}
static const httpd_uri_t pcfgdev4 = {
	.uri       = "/cfgdev4",
	.method    = HTTP_GET,
	.handler   = pcfgdev4_get_handler,
	.user_ctx  = NULL
};


static esp_err_t pcfgdev4ok_get_handler(httpd_req_t *req)
{
	char buf1[512] = {0};
	int  ret;
	ret = httpd_req_recv(req,buf1,512);
/*

in buf1 after httpd_req_recv string like below
swfid=wifiname&swfpsw=wifipassword&smqsrv=192.168.1.10&smqid=esp&
smqpsw=esp&devnam=&rlight=255&glight=255&blight=255&chk2=2

*/
	if ( ret > 0 ) ret = pcfgdev (3, buf1, 512);
	httpd_resp_set_status(req, "303 See Other");
	if (!ret) httpd_resp_set_hdr(req, "Location", "/cfgdev4");
	else httpd_resp_set_hdr(req, "Location", "/");
	httpd_resp_send(req, NULL, 0);  // Response body can be empty
	return ESP_OK;
}

static const httpd_uri_t pcfgdev4ok = {
	.uri       = "/cfgdev4ok",
	.method    = HTTP_POST,
	.handler   = pcfgdev4ok_get_handler,
	.user_ctx  = NULL
};



/* HTTP GET device5 handler */
static esp_err_t pcfgdev5_get_handler(httpd_req_t *req)
{
	if (!test_auth(req)) {
		return ESP_OK;
	}
	ble_mon_refr = ble_mon_refr & 0xfd;
	char *bsend = NULL;
	bsend = malloc(13000);
	if (bsend == NULL) {
	if (fdebug) ESP_LOGE(AP_TAG, "Http devh5: No memory");
	MemErr++;
	if (!MemErr) MemErr--;
	httpd_resp_set_status(req, "303 See Other");
	httpd_resp_set_hdr(req, "Location", "/");
	httpd_resp_send(req, NULL, 0);  // Response body can be empty
	} else {	
        char buff[32];
	strcpy(bsend,"<!DOCTYPE html><html>");
	strcat(bsend,"<head><title>r4sGate</title>");
	strcat(bsend,"<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
	strcat(bsend,cssDatasheet);
	strcat(bsend,"</head><body class='bodymenu'><header class='headermenu'><h1>ESP32 r4sGate");
	itoa(R4SNUM,buff,10);
	strcat(bsend, buff);
	strcat(bsend," for Redmond +</h1>");
	strcat(bsend,"<div class='menubar'><a class='menu' href='.'>&#8962;<span class='showmenulabel'>Main");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev1'>&#128246;<span class='showmenulabel'> 1 ");
	(BleDevStA.RQC_NAME[0])? strcat(bsend,BleDevStA.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev2'>&#128246;<span class='showmenulabel'> 2 ");
	(BleDevStB.RQC_NAME[0])? strcat(bsend,BleDevStB.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev3'>&#128246;<span class='showmenulabel'> 3 ");
	(BleDevStC.RQC_NAME[0])? strcat(bsend,BleDevStC.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev4'>&#128246;<span class='showmenulabel'> 4 ");
	(BleDevStD.RQC_NAME[0])? strcat(bsend,BleDevStD.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu active' href='cfgdev5'>&#128246;<span class='showmenulabel'> 5 ");
	(BleDevStE.RQC_NAME[0])? strcat(bsend,BleDevStE.RQC_NAME) : strcat(bsend,"Not defined");
	if (ble_mon) strcat(bsend,"</span></a><a class='menu' href='blemon'>&#128246;<span class='showmenulabel'>BLE monitor</span></a>");
	strcat(bsend,"</span></a><a class='menu' href='setting'>&#9881;<span class='showmenulabel'>Setting</span></a>");
	strcat(bsend,"<a class='menu' href='restart'>&#128259;<span class='showmenulabel'>Reboot</span></a>");
	strcat(bsend,"<a class='menu' href='update'>&#10548;<span class='showmenulabel'>Load firmware</span></a></div>");

	strcat(bsend,"</header>");
        HtpDeVHandle(4, bsend);

//itoa(strlen(bsend),buff,10);
//strcat(bsend,buff);

	httpd_resp_sendstr(req, bsend);
	free(bsend);
	}
	return ESP_OK;
}
static const httpd_uri_t pcfgdev5 = {
	.uri       = "/cfgdev5",
	.method    = HTTP_GET,
	.handler   = pcfgdev5_get_handler,
	.user_ctx  = NULL
};


static esp_err_t pcfgdev5ok_get_handler(httpd_req_t *req)
{
	char buf1[512] = {0};
	int  ret;
	ret = httpd_req_recv(req,buf1,512);
/*

in buf1 after httpd_req_recv string like below
swfid=wifiname&swfpsw=wifipassword&smqsrv=192.168.1.10&smqid=esp&
smqpsw=esp&devnam=&rlight=255&glight=255&blight=255&chk2=2

*/
	if ( ret > 0 ) ret = pcfgdev (4, buf1, 512);
	httpd_resp_set_status(req, "303 See Other");
	if (!ret) httpd_resp_set_hdr(req, "Location", "/cfgdev5");
	else httpd_resp_set_hdr(req, "Location", "/");
	httpd_resp_send(req, NULL, 0);  // Response body can be empty
	return ESP_OK;
}

static const httpd_uri_t pcfgdev5ok = {
	.uri       = "/cfgdev5ok",
	.method    = HTTP_POST,
	.handler   = pcfgdev5ok_get_handler,
	.user_ctx  = NULL
};



/* HTTP blemon handler */
static esp_err_t pblemon_get_handler(httpd_req_t *req)
{
	if (!test_auth(req)) {
		return ESP_OK;
	}
	char *bsend = NULL;
	bsend = malloc(18000);
	if (bsend == NULL) {
	if (fdebug) ESP_LOGE(AP_TAG, "Http blemon: No memory");
	MemErr++;
	if (!MemErr) MemErr--;
	httpd_resp_set_status(req, "303 See Other");
	httpd_resp_set_hdr(req, "Location", "/");
	httpd_resp_send(req, NULL, 0);  // Response body can be empty
	} else {	
        char buff[128];
	int  bmofs = 0;
	strcpy(bsend,"<!DOCTYPE html><html>");
	strcat(bsend,"<head><title>r4sGate</title>");
	strcat(bsend,"<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
	if (ble_mon_refr & 1) strcat(bsend,"<meta http-equiv=\"refresh\" content=\"5\">");
	strcat(bsend,cssDatasheet);
	strcat(bsend,"</head><body class='bodymenu'><header class='headermenu'><h1>ESP32 r4sGate");
	itoa(R4SNUM,buff,10);
	strcat(bsend, buff);
	strcat(bsend," for Redmond +</h1>");
	strcat(bsend,"<div class='menubar'><a class='menu' href='.'>&#8962;<span class='showmenulabel'>Main");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev1'>&#128246;<span class='showmenulabel'> 1 ");
	(BleDevStA.RQC_NAME[0])? strcat(bsend,BleDevStA.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev2'>&#128246;<span class='showmenulabel'> 2 ");
	(BleDevStB.RQC_NAME[0])? strcat(bsend,BleDevStB.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev3'>&#128246;<span class='showmenulabel'> 3 ");
	(BleDevStC.RQC_NAME[0])? strcat(bsend,BleDevStC.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev4'>&#128246;<span class='showmenulabel'> 4 ");
	(BleDevStD.RQC_NAME[0])? strcat(bsend,BleDevStD.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev5'>&#128246;<span class='showmenulabel'> 5 ");
	(BleDevStE.RQC_NAME[0])? strcat(bsend,BleDevStE.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu active' href='blemon'>&#128246;<span class='showmenulabel'>BLE monitor</span></a>");
	strcat(bsend,"</span></a><a class='menu' href='setting'>&#9881;<span class='showmenulabel'>Setting</span></a>");
	strcat(bsend,"<a class='menu' href='restart'>&#128259;<span class='showmenulabel'>Reboot</span></a>");
	strcat(bsend,"<a class='menu' href='update'>&#10548;<span class='showmenulabel'>Load firmware</span></a></div>");
	strcat(bsend,"</header><body><form method=\"POST\" action=\"/blemonok\"  id=\"frm1\"><table class='normal' width='80%'>");
	strcat(bsend,"<tr class=\"header\"><th width='30px' align='left'>Pos</th><th width='210px' align='left'>ID</th><th width='140px' align='left'>Name / Type</th><th width='80px' align='left''>RSSI</th><th width='70px' align='left''>Gap</th><th width='70px' align='left''>Last</th><th width='720px' align='left'>Advanced Data / Scan Response</th><th width='150px' align='left''>Timeout etc.</tr>");
	if (ble_mon_refr & 2) bmofs = BleMonNum/2;
	for (int i = bmofs; i < (BleMonNum/2 + bmofs); i++) {
	(i & 1)? strcat(bsend,"<tr><td class='xbg'>") : strcat(bsend,"<tr><td>");
	itoa(i+1,buff,10);
	strcat(bsend,buff);	
	(i & 1)? strcat(bsend,"</td><td class='xbg'>") : strcat(bsend,"</td><td>");
	buff[0] = 0;
	if (BleMR[i].id == 0x42) {
	strcat(bsend,"uuid: ");	
	bin2hex(BleMR[i].mac,buff,4,0);
	strcat(bsend,buff);	
	strcat(bsend,"-");	
	bin2hex(&BleMR[i].mac[4],buff,2,0);
	strcat(bsend,buff);	
	strcat(bsend,"-");	
	} else if (BleMR[i].id == 0x44) {
	strcat(bsend,"id: ");	
	bin2hex(BleMR[i].mac,buff,8,0);
	strcat(bsend,buff);	
	} else if (BleMR[i].id) {
	strcat(bsend,"mac: ");	
	bin2hex(BleMR[i].mac,buff,6,0x3a);
	strcat(bsend,buff);	
	}
	(i & 1)? strcat(bsend,"</td><td class='xbg'>") : strcat(bsend,"</td><td>");
	memset(buff,0,16);
	if (BleMR[i].id) {
	mystrcpy(buff,BleMX[i].name,15);
	strcat(bsend,buff);	
	}
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
	if (BleMR[i].sto) tmp = BleMR[i].sto - BleMX[i].ttick;
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

	if (BleMR[i].id == 0x04) {
	strcat(bsend,"<input name=\"blmkey");
	itoa(i,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" type=\"text\" maxlength=\"64\" size=\"6\" placeholder=\"Key\">");
	} else if (BleMR[i].id) {
	strcat(bsend,"<input name=\"blmto");
	itoa(i,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" type=\"number");
	if (BleMR[i].sto) {
	strcat(bsend,"\" value=\"");
	itoa(BleMR[i].sto / 10,buff,10);
	strcat(bsend,buff);
	}
	strcat(bsend,"\" min=\"0\" max=\"6500\" size=\"6\" placeholder=\"Timeout\">s");
	}
	(i & 1)? strcat(bsend,"</td></tr><tr><td class='xbg'></td><td class='xbg'>") : strcat(bsend,"</td></tr><tr><td></td><td>");
	if (BleMR[i].id == 0x42) {
	bin2hex(&BleMR[i].mac[6],buff,2,0);
	strcat(bsend,buff);	
	strcat(bsend,"-");	
	bin2hex(&BleMR[i].mac[8],buff,2,0);
	strcat(bsend,buff);	
	strcat(bsend,"-");	
	bin2hex(&BleMR[i].mac[10],buff,6,0);
	strcat(bsend,buff);	
	} else if (BleMR[i].id == 0x44) {
	strcat(bsend,"&emsp; ");	
	bin2hex(&BleMR[i].mac[8],buff,8,0);
	strcat(bsend,buff);	
	}
	(i & 1)? strcat(bsend,"</td><td class='xbg'>") : strcat(bsend,"</td><td>");
	if (BleMR[i].id == 2) {
	strcat(bsend,"Mi Scale");	
	} else if (BleMR[i].id == 3) {
	strcat(bsend,"LYWSD03MMC");	
	} else if (BleMR[i].id == 0x42) {
	strcat(bsend,"HA iBeacon");	
	} else if (BleMR[i].id == 0x44) {
	strcat(bsend,"Smart Tag");	
	} else if (BleMR[i].id == 0x04) {
	strcat(bsend,"Smart Tag");	
	}
	(i & 1)? strcat(bsend,"</td><td class='xbg'>") : strcat(bsend,"</td><td>");
	if (BleMX[i].cmrssi) {
	itoa(BleMX[i].cmrssi,buff,10);
	strcat(bsend,buff);	
	strcat(bsend," dB/");
	itoa(BleMX[i].gtnum,buff,10);
	strcat(bsend,buff);	
	}
	(i & 1)? strcat(bsend,"</td><td class='xbg'></td><td class='xbg'></td><td class='xbg'>") : strcat(bsend,"</td><td></td><td></td><td>");
	if(BleMX[i].scrsplen) {
	bin2hex(BleMX[i].scrsp,buff,BleMX[i].scrsplen & 0x1f,0x20);
	strcat(bsend,buff);	
	}
	strcat(bsend,"</td></tr>");
	}
	strcat(bsend,"<tr class=\"header\"><th align='left'></th><th align='left'><input type=\"checkbox\" name=\"blmarfr\" value=\"1\"");
	if (ble_mon_refr & 1) strcat(bsend,"checked");
	strcat(bsend,">Auto Refresh");
	strcat(bsend,"</th><th align='left'>");
        (ble_mon_refr & 2)?  strcat(bsend,"Page 2") : strcat(bsend,"Page 1");
	strcat(bsend,"</th><th align='left'></th><th align='left'></th><th align='left'></th><th align='left'>");
        (IsPassiveScan)? strcat(bsend,"Passive mode") : strcat(bsend,"Active mode");

	strcat(bsend,"</th><th align='left''></tr>");
	strcat(bsend,"</table></br>");

	strcat(bsend,"</form><form method=\"POST\" action=\"/setignore\">");
	strcat(bsend,"<input type=\"button\" value =\"Page\" onclick=\"window.location.reload(true);\" />");
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
	ble_mon_refr = ble_mon_refr ^ 2;
	free(bsend);
	}
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
	char buf3[96] = {0};
	uint8_t buf4[48] = {0};
	uint8_t fsave = 0;
	uint8_t fble_mon = ble_mon;
	uint16_t var = 0;
	int  ret;
	ble_mon = 0;
	ret = httpd_req_recv(req,buf1,512);
	if ( ret > 0 ) {
//if (fdebug) ESP_LOGI(AP_TAG, "Buf: '%s'", buf1);
/*

in buf1 after httpd_req_recv string like below
swfid=wifiname&swfpsw=wifipassword&smqsrv=192.168.1.10&smqid=esp&
smqpsw=esp&devnam=&rlight=255&glight=255&blight=255&chk2=2

*/
	vTaskDelay(100 / portTICK_PERIOD_MS);
	ble_mon_refr = ble_mon_refr ^ 2;
	buf3[0] = 0;
	strcpy(buf2,"blmarfr");
	parsuri(buf1,buf3,buf2,512,2,0);
	if (buf3[0] == 0x31) ble_mon_refr = ble_mon_refr | 1;
	else ble_mon_refr = ble_mon_refr & 0xfe;
	for (int i = 0; i < BleMonNum; i++) {
	strcpy(buf2,"blmto");
	itoa(i,buf3,10);
	strcat(buf2,buf3);
	buf3[0] = 0;
	parsuri(buf1,buf3,buf2,512,5,0);
	if (buf3[0]) {
	var = atoi(buf3) * 10;
	if (BleMR[i].sto != var) fsave = 1;
	BleMR[i].sto = var;
	if (var && (BleMX[i].ttick > var)) BleMX[i].ttick = var;
	}
	strcpy(buf2,"blmkey");
	itoa(i,buf3,10);
	strcat(buf2,buf3);
	buf3[0] = 0;
	parsuri(buf1,buf3,buf2,512,65,0);
	if (buf3[0] && BleMX[i].advdatlen && (strlen(buf3) == 64)) {
	if((hex2bin(buf3, buf4, 32)) && (vstsign(&BleMX[i].advdat[11], buf4))) {
	memcpy(BleMR[i].mac, buf4, 32);
	BleMR[i].id = 0x44;
	fsave = 0;
	}
	}

	}
	}
	if (fsave) {
// write nvs
	nvs_handle_t my_handle;
	ret = nvs_open("storage", NVS_READWRITE, &my_handle);
	if (ret == ESP_OK) {
	for (int i = 0; i < BleMonNum; i++) {
	if (!BleMR[i].sto) {
	memset(&BleMR[i],0,sizeof(BleMR[i]));
	memset(&BleMX[i],0,sizeof(BleMX[i]));
	}
	}
	nvs_set_blob(my_handle,"sblemd",  BleMR, sizeof(BleMR));
        ret = nvs_commit(my_handle);
	if (ret != ESP_OK) {
	if (fdebug) ESP_LOGE(AP_TAG, "NVS write error");
	}
// Close nvs
	nvs_close(my_handle);
	}
	}
	ble_mon = fble_mon;
	if (fsave) HDiscBlemon(mqttConnected);
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



uint8_t pinvalid(uint8_t pin) {
	uint8_t retc = 7;
#ifdef CONFIG_IDF_TARGET_ESP32C3
	if (pin > 21) retc = 0;
#else
	if (pin > 39) retc = 0;
	if (pin > 33) retc = 3;
	if ((pin > 5) && (pin < 11)) retc = 0;
	if ((pin > 27) && (pin < 32)) retc = 0;
	if ((pin == 20) ||(pin == 24)) retc = 0;
#endif
	if ((pin > 39) && (pin < 49)) retc = 1;
	return retc;
}


void HtpDeVSett(uint8_t blenum, char* bsend) {
	uint8_t blenum1 = blenum + 1;
	char buff[16]; 
	if (blenum > 4) return;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
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
	strcat(bsend,"\" size=\"15\">Name/MAC &emsp;<input name=\"pnpsw");
	strcpy(buff,"a");
	buff[0] = buff[0] + blenum;
	strcat(bsend,buff);
	strcat(bsend,"\" type=\"number\" value=\"");
	itoa(ptr->PassKey,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"9999\" size=\"4\">Passkey &emsp;<select name=\"sreqtp");
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
	if (ptr->DEV_TYP == 48) strcat(bsend,"selected ");
	strcat(bsend,"value=\"48\">RMB-M658S</option><option ");
	if (ptr->DEV_TYP == 61) strcat(bsend,"selected ");
	strcat(bsend,"value=\"61\">RI-C273S</option><option ");
	if (ptr->DEV_TYP == 62) strcat(bsend,"selected ");
	strcat(bsend,"value=\"62\">RSS-61S</option><option ");
	if (ptr->DEV_TYP == 63) strcat(bsend,"selected ");
	strcat(bsend,"value=\"63\">RSC-51S</option><option ");
	if (ptr->DEV_TYP == 64) strcat(bsend,"selected ");
	strcat(bsend,"value=\"64\">YM-K1501(Int)</option><option ");
	if (ptr->DEV_TYP == 65) strcat(bsend,"selected ");
	strcat(bsend,"value=\"65\">YM-K1501(HK)</option><option ");
	if (ptr->DEV_TYP == 66) strcat(bsend,"selected ");
	strcat(bsend,"value=\"66\">V-SK152(Int)</option><option ");
	if (fdebug) {
	if (ptr->DEV_TYP == 72) strcat(bsend,"selected ");
	strcat(bsend,"value=\"72\">MiKettle ID</option><option ");
	}
	if (ptr->DEV_TYP == 73) strcat(bsend,"selected ");
	strcat(bsend,"value=\"73\">GL9001A</option><option ");
	if (ptr->DEV_TYP == 74) strcat(bsend,"selected ");
	strcat(bsend,"value=\"74\">AM43 Blinds</option></select>Type/");
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
	ble_mon_refr = ble_mon_refr & 0xfd;
	char *bsend = NULL;
	bsend = malloc(24000);
	if (bsend == NULL) {
	if (fdebug) ESP_LOGE(AP_TAG, "Http setting: No memory");
	MemErr++;
	if (!MemErr) MemErr--;
	httpd_resp_set_status(req, "303 See Other");
	httpd_resp_set_hdr(req, "Location", "/");
	httpd_resp_send(req, NULL, 0);  // Response body can be empty
	} else {	
        char buff[130];
	strcpy(bsend,"<!DOCTYPE html><html>");
	strcat(bsend,"<head><title>r4sGate</title>");
	strcat(bsend,"<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
	strcat(bsend,cssDatasheet);
	strcat(bsend,"</head><body class='bodymenu'><header class='headermenu'><h1>ESP32 r4sGate");
	itoa(R4SNUM,buff,10);
	strcat(bsend, buff);
	strcat(bsend," for Redmond +</h1>");
	strcat(bsend,"<div class='menubar'><a class='menu' href='.'>&#8962;<span class='showmenulabel'>Main");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev1'>&#128246;<span class='showmenulabel'> 1 ");
	(BleDevStA.RQC_NAME[0])? strcat(bsend,BleDevStA.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev2'>&#128246;<span class='showmenulabel'> 2 ");
	(BleDevStB.RQC_NAME[0])? strcat(bsend,BleDevStB.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev3'>&#128246;<span class='showmenulabel'> 3 ");
	(BleDevStC.RQC_NAME[0])? strcat(bsend,BleDevStC.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev4'>&#128246;<span class='showmenulabel'> 4 ");
	(BleDevStD.RQC_NAME[0])? strcat(bsend,BleDevStD.RQC_NAME) : strcat(bsend,"Not defined");
	strcat(bsend,"</span></a><a class='menu' href='cfgdev5'>&#128246;<span class='showmenulabel'> 5 ");
	(BleDevStE.RQC_NAME[0])? strcat(bsend,BleDevStE.RQC_NAME) : strcat(bsend,"Not defined");
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
	strcat(bsend,"\"size=\"20\">Server &emsp;<input name=\"smqprt\" type=\"number\" value=\"");
	itoa(mqtt_port,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"65535\" size=\"5\">Port &emsp;<input name=\"smqid\" value=\"");
	if (MQTT_USER[0]) strcat(bsend,MQTT_USER);
	strcat(bsend,"\"size=\"15\">Login &emsp;<input type=\"password\" input name=\"smqpsw\" value=\"");
	if (MQTT_PASSWORD[0]) strcat(bsend,MQTT_PASSWORD);
	strcat(bsend,"\"size=\"19\">Password &emsp;<input name=\"smcert\" value=\"");
	if (bcertsz) mycertcpy(bsend + strlen(bsend), bufcert + bcertofs, bcertsz);
	strcat(bsend,"\"size=\"20\" maxlength=\"1920\">Certificate</br><input type=\"checkbox\" name=\"smssl\" value=\"1\"");
	if (fmssl) strcat(bsend,"checked");
	strcat(bsend,"> Use SSL / TLS&emsp;<input type=\"checkbox\" name=\"smwss\" value=\"4\"");
	if (fmwss) strcat(bsend,"checked");
	strcat(bsend,"> Use WS&emsp;<input type=\"checkbox\" name=\"smsslb\" value=\"2\"");
	if (fmsslbundle) strcat(bsend,"checked");
	strcat(bsend,"> Use x509 Bundle&emsp;<input type=\"checkbox\" name=\"smsslh\" value=\"3\"");
	if (fmsslhost) strcat(bsend,"checked");
	strcat(bsend,"> Hostname Verify&emsp;<input type=\"checkbox\" name=\"chk1\" value=\"1\"");
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
	strcat(bsend,"\"size=\"20\">Indoor Temp&ensp;&emsp;<input name=\"smtopp2\" value=\"");
	if (MQTT_TOPP2[0]) strcat(bsend,MQTT_TOPP2);
	strcat(bsend,"\"size=\"20\">Voltage&ensp;&emsp;&emsp;<input name=\"smtopp3\" value=\"");
	if (MQTT_TOPP3[0]) strcat(bsend,MQTT_TOPP3);
	strcat(bsend,"\"size=\"20\">Current</br><input name=\"smtopp4\" value=\"");
	if (MQTT_TOPP4[0]) strcat(bsend,MQTT_TOPP4);
	strcat(bsend,"\"size=\"20\">Boiler Temp &ensp;&emsp;<input name=\"smtopp5\" value=\"");
	if (MQTT_TOPP5[0]) strcat(bsend,MQTT_TOPP5);
	strcat(bsend,"\"size=\"20\">Boiler State&thinsp;&ensp;<input name=\"smtopp6\" value=\"");
	if (MQTT_TOPP6[0]) strcat(bsend,MQTT_TOPP6);
	strcat(bsend,"\"size=\"20\">Outdoor Temp &ensp;<input name=\"smtopp7\" value=\"");
	if (MQTT_TOPP7[0]) strcat(bsend,MQTT_TOPP7);
	strcat(bsend,"\"size=\"20\">Outdoor Humidity</br>");
#endif
	strcat(bsend,"<h3>BLE Devices Setting</h3></br>");

	HtpDeVSett(0, bsend);
	HtpDeVSett(1, bsend);
	HtpDeVSett(2, bsend);
	HtpDeVSett(3, bsend);
	HtpDeVSett(4, bsend);

	strcat(bsend,"<input type=\"checkbox\" name=\"chk8\" value=\"8\"");
	if (macauth) strcat(bsend,"checked");
	strcat(bsend,"> Use MAC in BLE Authentication &emsp;<input type=\"checkbox\" name=\"chk9\" value=\"9\"");
	if (volperc) strcat(bsend,"checked");
	strcat(bsend,"> Volume in Percent &emsp;<input type=\"checkbox\" name=\"skpmd\" value=\"1\"");
	if (fkpmd) strcat(bsend,"checked");
	strcat(bsend,"> Keep kettle mode</br>");
	strcat(bsend, "<h3>System Setting</h3></br>");

	strcat(bsend, "<input name=\"auth\" type=\"text\" value=\"");
	if (AUTH_BASIC[0]) strcat(bsend, AUTH_BASIC);
	strcat(bsend, "\" size=\"15\">Basic Auth &emsp;");

	strcat(bsend, "<input name=\"r4snum\" type=\"number\" value=\"");
	itoa(R4SNUM,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"99\" size=\"3\">Gate Number &emsp;<input name=\"timzon\" type=\"number\" value=\"");
	uint8_t TmZn = TimeZone;
	if (TmZn >127) {
	TmZn = ~TmZn;
	TmZn++;
	strcat(bsend,"-");
	}
	itoa(TmZn,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"-12\" max=\"12\" size=\"3\">Timezone &emsp;");


	strcat(bsend,"<select name=\"chk5\"><option ");
	if (ble_mon == 0) strcat(bsend,"selected ");
	strcat(bsend,"value=\"0\">Off</option><option ");
	if (ble_mon == 1) strcat(bsend,"selected ");
	strcat(bsend,"value=\"1\">Passive</option><option ");
	if (ble_mon == 2) strcat(bsend,"selected ");
	strcat(bsend,"value=\"2\">Active</option><option ");
	if (ble_mon == 3) strcat(bsend,"selected ");
	strcat(bsend,"value=\"3\">Auto</option></select> BLE Monitoring &emsp;");
	strcat(bsend,"<input type=\"checkbox\" name=\"mqtdel\" value=\"1\"");
	strcat(bsend,"> Delete Mqtt Topics &emsp;");
	strcat(bsend,"<input type=\"checkbox\" name=\"chk0\" value=\"0\"");
	strcat(bsend,"> Format NVS &emsp;");
	strcat(bsend,"<input type=\"checkbox\" name=\"chkd\" value=\"1\"");
	if (fdebug) strcat(bsend,"checked");
	strcat(bsend,"> Uart Debug</br>");
	strcat(bsend,"<input name=\"sntp\" value=\"");
	if (NTP_SERVER[0]) strcat(bsend,NTP_SERVER);
	strcat(bsend,"\"size=\"15\">NTP&emsp;");
#ifdef USE_TFT
// read nvs
	nvs_handle_t my_handle;
	memset (buff,0,sizeof(buff));
	if (nvs_open("storage", NVS_READWRITE, &my_handle) == ESP_OK) {
	size_t nvsize = 128;
	nvs_get_str(my_handle,"smjpuri", buff,&nvsize);
// Close nvs
	nvs_close(my_handle);
	}
	strcat(bsend,"<input name=\"smjpuri\" value=\"");
	if (buff[0]) strcat(bsend,buff);
	strcat(bsend,"\"size=\"60\">JPEG Url&emsp;<input name=\"sjpgtim\" type=\"number\" value=\"");
	itoa(jpg_time,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"65535\" size=\"5\">JPEG Time(s)&emsp;<input name=\"sjpgbuf\" type=\"number\" value=\"");
	itoa(MyJPGbuflen,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"20000\" max=\"65535\" size=\"5\">JPEG Memory</br>");
	strcat(bsend,"<input type=\"checkbox\" name=\"chk6\" value=\"6\"");
	if (tft_conf) strcat(bsend,"checked");
	strcat(bsend,"> TFT: <input type=\"checkbox\" name=\"chk4\" value=\"4\"");
	if (tft_flip) strcat(bsend,"checked");
	strcat(bsend,"> flip 180&deg; &ensp;GPIO<input name=\"pnmiso\" type=\"number\" value=\"");
	itoa(PIN_NUM_MISO,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"39\" size=\"2\">MISO <input name=\"pnmosi\" type=\"number\" value=\"");
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
	strcat(bsend,"\" min=\"0\" max=\"63\" size=\"2\">RES <input name=\"pnbckl\" type=\"number\" value=\"");
	itoa(PIN_NUM_BCKL,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"63\" size=\"2\">LED <input name=\"pnpwr\" type=\"number\" value=\"");
	itoa(PIN_NUM_PWR,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"63\" size=\"2\">PWR");
#endif
	strcat(bsend,"</br><input name=\"ppin1\" type=\"number\" value=\"");
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
	strcat(bsend,"value=\"128\">In</option><option ");
	if ((bgpio4 & 0xc0) == 0xc0) strcat(bsend,"selected ");
	strcat(bsend,"value=\"192\">Jpg</option></select>&emsp;<input name=\"ppin5\" type=\"number\" value=\"");
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
	strcat(bsend,"value=\"192\">Hx D</option></select>");
	if ((bgpio5 > 192) && (bgpio6 > 128) && (bgpio6 < 192)) {
	strcat(bsend,"&emsp;<select name=\"bhx1\"><option ");
	strcat(bsend,"value=\"0\">Hx setting</option><option ");
	strcat(bsend,"value=\"1\">\x3e 0.0 \x3c</option><option ");
	strcat(bsend,"value=\"2\">\x3e 0.9kg \x3c</option><option ");
	strcat(bsend,"value=\"3\">\x3e 1.0kg \x3c</option><option ");
	strcat(bsend,"value=\"4\">\x3e 1.5kg \x3c</option><option ");
	strcat(bsend,"value=\"5\">\x3e 1.6kg \x3c</option><option ");
	strcat(bsend,"value=\"6\">\x3e 1.7kg \x3c</option><option ");
	strcat(bsend,"value=\"7\">\x3e 1.8kg \x3c</option><option ");
	strcat(bsend,"value=\"8\">\x3e 2.0kg \x3c</option><option ");
	strcat(bsend,"value=\"9\">\x3e 2.2kg \x3c</option><option ");
	strcat(bsend,"value=\"10\">\x3e 2.4kg \x3c</option><option ");
	strcat(bsend,"value=\"11\">\x3e 100% \x3c</option></select>");
	}
	strcat(bsend,"<br><input name=\"ppin6\" type=\"number\" value=\"");
	itoa((bgpio6 & 0x3f),buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"39\" size=\"2\">Port6<select name=\"popt6\"><option ");
	if (!(bgpio6 & 0xc0)) strcat(bsend,"selected ");
	strcat(bsend,"value=\"0\">Off</option><option ");
	if ((bgpio6 & 0xc0) == 0x40) strcat(bsend,"selected ");
	strcat(bsend,"value=\"64\">Pwm</option><option ");
	if ((bgpio6 & 0xc0) == 0x80) strcat(bsend,"selected ");
	strcat(bsend,"value=\"128\">18b20/Hx C</option><option ");
	if ((bgpio6 & 0xc0) == 0xc0) strcat(bsend,"selected ");
#ifdef USE_IRTX
	strcat(bsend,"value=\"192\">IR Tx</option></select>&emsp;<input name=\"ppin7\" type=\"number\" value=\"");
#else
	strcat(bsend,"value=\"192\">Dht22</option></select>&emsp;<input name=\"ppin7\" type=\"number\" value=\"");
#endif
	itoa((bgpio7 & 0x3f),buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"39\" size=\"2\">Port7<select name=\"popt7\"><option ");
	if (!(bgpio7 & 0xc0)) strcat(bsend,"selected ");
	strcat(bsend,"value=\"0\">Off</option><option ");
	if ((bgpio7 & 0xc0) == 0x40) strcat(bsend,"selected ");
	strcat(bsend,"value=\"64\">Pwm</option><option ");
	if ((bgpio7 & 0xc0) == 0x80) strcat(bsend,"selected ");
	strcat(bsend,"value=\"128\">18b20</option><option ");
	if ((bgpio7 & 0xc0) == 0xc0) strcat(bsend,"selected ");
	strcat(bsend,"value=\"192\">Dht22</option></select>&emsp;<input name=\"ppin8\" type=\"number\" value=\"");
	itoa((bgpio8 & 0x3f),buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"39\" size=\"2\">Port8<select name=\"popt8\"><option ");
	if (!(bgpio8 & 0xc0)) strcat(bsend,"selected ");
	strcat(bsend,"value=\"0\">Off</option><option ");
	if ((bgpio8 & 0xc0) == 0x40) strcat(bsend,"selected ");
#ifdef CONFIG_IDF_TARGET_ESP32C3
	strcat(bsend,"value=\"64\">Pwm</option></select>&emsp;<input name=\"ppin9\" type=\"number\" value=\"");
#else
	strcat(bsend,"value=\"64\">Pwm</option><option ");
	if ((bgpio8 & 0xc0) == 0x80) strcat(bsend,"selected ");
	strcat(bsend,"value=\"128\">18b20</option><option ");
	if ((bgpio8 & 0xc0) == 0xc0) strcat(bsend,"selected ");
	strcat(bsend,"value=\"192\">Dht22</option></select>&emsp;<input name=\"ppin9\" type=\"number\" value=\"");
#endif
	itoa((bgpio9 & 0x3f),buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"39\" size=\"2\">Port9<select name=\"popt9\"><option ");
	if (!(bgpio9 & 0xc0)) strcat(bsend,"selected ");
	strcat(bsend,"value=\"0\">Off</option><option ");
	if ((bgpio9 & 0xc0) == 0xc0) strcat(bsend,"selected ");
	strcat(bsend,"value=\"192\">I2C SCL</option></select>&emsp;<input name=\"ppina\" type=\"number\" value=\"");
	itoa((bgpio10 & 0x3f),buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"39\" size=\"2\">Port10<select name=\"popta\"><option ");
	if (!(bgpio10 & 0xc0)) strcat(bsend,"selected ");
	strcat(bsend,"value=\"0\">Off</option><option ");
	if ((bgpio10 & 0xc0) == 0xc0) strcat(bsend,"selected ");
	strcat(bsend,"value=\"192\">I2C SDA</option></select><br>");

	strcat(bsend,"<h3>Store setting then press SAVE. ESP32 r4sGate will restart</h3></br>");
	strcat(bsend,"<input type=SUBMIT value=\"Save settings\"></form><form method=\"POST\" action=\"/setignore\">");
	strcat(bsend,"<input type=SUBMIT value=\"Cancel\"></form></body></html>");

//itoa(strlen(bsend),buff,10);
//strcat(bsend,buff);

	httpd_resp_sendstr(req, bsend);
	free(bsend);
	}
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

	char *buf1 = NULL;
	buf1 = malloc(4128);
	if (buf1 == NULL) {
	if (fdebug) ESP_LOGE(AP_TAG, "Http save setting: No memory");
	MemErr++;
	if (!MemErr) MemErr--;
	httpd_resp_set_status(req, "303 See Other");
	httpd_resp_set_hdr(req, "Location", "/");
	httpd_resp_send(req, NULL, 0);  // Response body can be empty
	} else {	
	memset (buf1,0,4128);
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
	int  rdoffs;
	int  rdcnt;
	rdoffs = 0;
	while (rdoffs < 4096) {
	rdcnt = 4096 - rdoffs;
	if (rdcnt > 1024) rdcnt = 1024;
	ret = httpd_req_recv(req, buf1 + rdoffs, rdcnt);
	rdcnt = 0;
	if (ret < 0) rdoffs = 4096;
	else if (ret > 0) rdoffs += ret;
	else if (ret == 0) {
	rdcnt = rdoffs;
	rdoffs = 4096;
	}
	}
	if (rdcnt > 0) {
/*
in buf1 after httpd_req_recv string like below
swfid=wifiname&swfpsw=wifipassword&smqsrv=192.168.1.10&smqid=esp&
smqpsw=esp&devnam=&rlight=255&glight=255&blight=255&chk2=2
*/
/*
	if (fdebug) {
	ESP_LOGI(AP_TAG, "Buflen: %d", strlen(buf1));
	ESP_LOGI(AP_TAG, "Buf: %s", buf1);
	}
*/
	strcpy(buf2,"swfid");
	parsuri(buf1,WIFI_SSID,buf2,4096,33,0);
	strcpy(buf2,"swfpsw");
	parsuri(buf1,WIFI_PASSWORD,buf2,4096,65,0);
	strcpy(buf2,"smqsrv");
	parsuri(buf1,MQTT_SERVER,buf2,4096,32,0);
	strcpy(buf2,"smqid");
	parsuri(buf1,MQTT_USER,buf2,4096,16,0);
	strcpy(buf2,"smqpsw");
	parsuri(buf1,MQTT_PASSWORD,buf2,4096,20,0);
#ifdef USE_TFT
	strcpy(buf2,"smtopp1");
	parsuri(buf1,MQTT_TOPP1,buf2,4096,32,0);
	strcpy(buf2,"smtopp2");
	parsuri(buf1,MQTT_TOPP2,buf2,4096,32,0);
	strcpy(buf2,"smtopp3");
	parsuri(buf1,MQTT_TOPP3,buf2,4096,32,0);
	strcpy(buf2,"smtopp4");
	parsuri(buf1,MQTT_TOPP4,buf2,4096,32,0);
	strcpy(buf2,"smtopp5");
	parsuri(buf1,MQTT_TOPP5,buf2,4096,32,0);
	strcpy(buf2,"smtopp6");
	parsuri(buf1,MQTT_TOPP6,buf2,4096,32,0);
	strcpy(buf2,"smtopp7");
	parsuri(buf1,MQTT_TOPP7,buf2,4096,32,0);
#endif
	strcpy(buf2,"sreqnma");
	parsuri(buf1,BleDevStA.REQ_NAME,buf2,4096,16,1);
	strcpy(buf2,"sreqnmb");
	parsuri(buf1,BleDevStB.REQ_NAME,buf2,4096,16,1);
	strcpy(buf2,"sreqnmc");
	parsuri(buf1,BleDevStC.REQ_NAME,buf2,4096,16,1);
	strcpy(buf2,"sreqnmd");
	parsuri(buf1,BleDevStD.REQ_NAME,buf2,4096,16,1);
	strcpy(buf2,"sreqnme");
	parsuri(buf1,BleDevStE.REQ_NAME,buf2,4096,16,1);
	buf3[0] = 0;
	strcpy(buf2,"pnpswa");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) BleDevStA.PassKey = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"pnpswb");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) BleDevStB.PassKey = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"pnpswc");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) BleDevStC.PassKey = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"pnpswd");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) BleDevStD.PassKey = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"pnpswe");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) BleDevStE.PassKey = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"rlighta");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) BleDevStA.RgbR = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"glighta");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) BleDevStA.RgbG = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"blighta");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) BleDevStA.RgbB = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"rlightb");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) BleDevStB.RgbR = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"glightb");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) BleDevStB.RgbG = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"blightb");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) BleDevStB.RgbB = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"rlightc");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) BleDevStC.RgbR = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"glightc");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) BleDevStC.RgbG = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"blightc");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) BleDevStC.RgbB = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"rlightd");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) BleDevStD.RgbR = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"glightd");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) BleDevStD.RgbG = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"blightd");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) BleDevStD.RgbB = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"rlighte");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) BleDevStE.RgbR = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"glighte");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) BleDevStE.RgbG = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"blighte");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) BleDevStE.RgbB = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"ltempa");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) BleDevStA.bLtemp = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"ltempb");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) BleDevStB.bLtemp = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"ltempc");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) BleDevStC.bLtemp = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"ltempd");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) BleDevStD.bLtemp = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"ltempe");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) BleDevStE.bLtemp = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"sreqtpa");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) BleDevStA.DEV_TYP = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"sreqtpb");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) BleDevStB.DEV_TYP = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"sreqtpc");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) BleDevStC.DEV_TYP = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"sreqtpd");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) BleDevStD.DEV_TYP = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"sreqtpe");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) BleDevStE.DEV_TYP = atoi(buf3);
#ifdef USE_TFT
	buf3[0] = 0;
	strcpy(buf2,"sjpgtim");
	parsuri(buf1,buf3,buf2,4096,8,0);
	if (buf3[0]) jpg_time = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"sjpgbuf");
	parsuri(buf1,buf3,buf2,4096,8,0);
	if (buf3[0]) MyJPGbuflen = atoi(buf3);
#endif
	buf3[0] = 0;
	strcpy(buf2,"smqprt");
	parsuri(buf1,buf3,buf2,4096,8,0);
	if (buf3[0]) mqtt_port = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"timzon");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) TimeZone = atoi(buf3);
	strcpy(buf2, "auth");
	parsuri(buf1, AUTH_BASIC, buf2, 4096, 50,0);
	strcpy(buf2, "sntp");
	parsuri(buf1, NTP_SERVER, buf2, 4096, 33,0);
	buf3[0] = 0;
	strcpy(buf2,"r4snum");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) R4SNUM = atoi(buf3);
	memset (bufcert,0,sizeof(bufcert));
	strcpy(buf2,"smcert");
	parscertstr(buf1,bufcert,buf2,4096,1920);
#ifdef USE_TFT
	strcpy(buf2,"smjpuri");
	parsuri(buf1,MyHttpUri,buf2,4096,128,0);
#endif
	buf3[0] = 0;
	strcpy(buf2,"chk1");
	parsuri(buf1,buf3,buf2,4096,2,0);
	FDHass = 0;
	if (buf3[0] == 0x31) FDHass = 1;
	buf3[0] = 0;
	strcpy(buf2,"chk2");
	parsuri(buf1,buf3,buf2,4096,2,0);
	fcommtp = 0;
	if (buf3[0] == 0x32) fcommtp = 1;
	buf3[0] = 0;
	strcpy(buf2,"chk3");
	parsuri(buf1,buf3,buf2,4096,2,0);
	ftrufal = 0;
	if ((buf3[0] == 0x33) && !FDHass) ftrufal = 1;
	buf3[0] = 0;
#ifdef USE_TFT
	strcpy(buf2,"chk4");
	parsuri(buf1,buf3,buf2,4096,2,0);
	tft_flip = 0;
	if (buf3[0] == 0x34) tft_flip = 1;
	buf3[0] = 0;
#endif
	strcpy(buf2,"chk5");
	parsuri(buf1,buf3,buf2,4096,2,0);
	ble_mon = atoi(buf3);
#ifdef USE_TFT
	buf3[0] = 0;
	strcpy(buf2,"chk6");
	parsuri(buf1,buf3,buf2,4096,2,0);
	tft_conf = 0;
	if (buf3[0] == 0x36) tft_conf = 1;
#endif
	buf3[0] = 0;
	strcpy(buf2,"chk7");
	parsuri(buf1,buf3,buf2,4096,2,0);
	foffln = 0;
	if (buf3[0] == 0x37) foffln = 1;
	buf3[0] = 0;
	strcpy(buf2,"chk8");
	parsuri(buf1,buf3,buf2,4096,2,0);
	macauth = 0;
	if (buf3[0] == 0x38) macauth = 1;
	buf3[0] = 0;
	strcpy(buf2,"chk9");
	parsuri(buf1,buf3,buf2,4096,2,0);
	volperc = 0;
	if (buf3[0] == 0x39) volperc = 1;
	buf3[0] = 0;
	strcpy(buf2,"smssl");
	parsuri(buf1,buf3,buf2,4096,2,0);
	fmssl = 0;
	if (buf3[0] == 0x31) fmssl = 1;
	buf3[0] = 0;
	strcpy(buf2,"smsslb");
	parsuri(buf1,buf3,buf2,4096,2,0);
	fmsslbundle = 0;
	if (buf3[0] == 0x32) fmsslbundle = 1;
	buf3[0] = 0;
	strcpy(buf2,"smsslh");
	parsuri(buf1,buf3,buf2,4096,2,0);
	fmsslhost = 0;
	if (buf3[0] == 0x33) fmsslhost = 1;
	buf3[0] = 0;
	strcpy(buf2,"smwss");
	parsuri(buf1,buf3,buf2,4096,2,0);
	fmwss = 0;
	if (buf3[0] == 0x34) fmwss = 1;
	buf3[0] = 0;
	strcpy(buf2,"chkd");
	parsuri(buf1,buf3,buf2,4096,2,0);
	fdebug = 0;
	if (buf3[0] == 0x31) fdebug = 1;
	buf3[0] = 0;
	strcpy(buf2,"skpmd");
	parsuri(buf1,buf3,buf2,4096,2,0);
	fkpmd = 0;
	if (buf3[0] == 0x31) fkpmd = 1;
	buf3[0] = 0;
	strcpy(buf2,"chk0");
	parsuri(buf1,buf3,buf2,4096,2,0);
	if (buf3[0] == 0x30) FinitNVS();
	if (FDHass) {
	ftrufal = 0;
	foffln = 0;
	}
#ifdef USE_TFT
	tft_conn = 0;
	buf3[0] = 0;
	strcpy(buf2,"pnmiso");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) {
	pintemp = atoi(buf3);
	if (pinvalid(pintemp) & 0x02) PIN_NUM_MISO = pintemp;
	else tft_conf = 0;
	}
	buf3[0] = 0;
	strcpy(buf2,"pnmosi");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) {
	pintemp = atoi(buf3);
	if (pinvalid(pintemp) & 0x04) PIN_NUM_MOSI = pintemp;
	else tft_conf = 0;
	}
	buf3[0] = 0;
	strcpy(buf2,"pnclk");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) {
	pintemp = atoi(buf3);
	if (pinvalid(pintemp) & 0x04) PIN_NUM_CLK = pintemp;
	else tft_conf = 0;
	}
	buf3[0] = 0;
	strcpy(buf2,"pncs");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) {
	pintemp = atoi(buf3);
	if (pinvalid(pintemp) & 0x04) PIN_NUM_CS = pintemp;
	else tft_conf = 0;
	}
	buf3[0] = 0;
	strcpy(buf2,"pndc");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) {
	pintemp = atoi(buf3);
	if (pinvalid(pintemp) & 0x04) PIN_NUM_DC = pintemp;
	else tft_conf = 0;
	}
	buf3[0] = 0;
	strcpy(buf2,"pnrst");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) {
	pintemp = atoi(buf3);
	if (pinvalid(pintemp)) PIN_NUM_RST = pintemp;
	else tft_conf = 0;
	}
	buf3[0] = 0;
	strcpy(buf2,"pnbckl");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) {
	pintemp = atoi(buf3);
	if (pinvalid(pintemp)) PIN_NUM_BCKL = pintemp;
	else tft_conf = 0;
	}
	buf3[0] = 0;
	strcpy(buf2,"pnpwr");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) {
	pintemp = atoi(buf3);
	if (pinvalid(pintemp)) PIN_NUM_PWR = pintemp;
	else tft_conf = 0;
	}
#endif
	buf3[0] = 0;
	strcpy(buf2,"ppin1");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) {
	pintemp = atoi(buf3);
	if (pinvalid(pintemp) & 0x02) {
	bgpio1 = pintemp;
	buf3[0] = 0;
	strcpy(buf2,"popt1");
	parsuri(buf1,buf3,buf2,4096,4,0);
	pintemp = atoi(buf3);
	bgpio1 = bgpio1 | pintemp;
	} else bgpio1 = 0;
	}
	buf3[0] = 0;
	strcpy(buf2,"ppin2");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) {
	pintemp = atoi(buf3);
	if (pinvalid(pintemp) & 0x02) {
	bgpio2 = pintemp;
	buf3[0] = 0;
	strcpy(buf2,"popt2");
	parsuri(buf1,buf3,buf2,4096,4,0);
	pintemp = atoi(buf3);
	bgpio2 = bgpio2 | pintemp;
	} else bgpio2 = 0;
	}
	buf3[0] = 0;
	strcpy(buf2,"ppin3");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) {
	pintemp = atoi(buf3);
	if (pinvalid(pintemp) & 0x02) {
	bgpio3 = pintemp;
	buf3[0] = 0;
	strcpy(buf2,"popt3");
	parsuri(buf1,buf3,buf2,4096,4,0);
	pintemp = atoi(buf3);
	bgpio3 = bgpio3 | pintemp;
	} else bgpio3 = 0;
	}
	buf3[0] = 0;
	strcpy(buf2,"ppin4");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) {
	pintemp = atoi(buf3);
	if (pinvalid(pintemp) & 0x02) {
	bgpio4 = pintemp;
	buf3[0] = 0;
	strcpy(buf2,"popt4");
	parsuri(buf1,buf3,buf2,4096,4,0);
	pintemp = atoi(buf3);
	bgpio4 = bgpio4 | pintemp;
	} else bgpio4 = 0;
	}
	buf3[0] = 0;
	strcpy(buf2,"ppin5");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) {
	pintemp = atoi(buf3);
	if (pinvalid(pintemp) & 0x02) {
	bgpio5 = pintemp;
	buf3[0] = 0;
	strcpy(buf2,"popt5");
	parsuri(buf1,buf3,buf2,4096,4,0);
	pintemp = atoi(buf3);
	bgpio5 = bgpio5 | pintemp;
	} else bgpio5 = 0;
	}
	buf3[0] = 0;
	strcpy(buf2,"ppin6");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) {
	pintemp = atoi(buf3);
	if (pinvalid(pintemp) & 0x02) {
	bgpio6 = pintemp;
	buf3[0] = 0;
	strcpy(buf2,"popt6");
	parsuri(buf1,buf3,buf2,4096,4,0);
	pintemp = atoi(buf3);
	bgpio6 = bgpio6 | pintemp;
	} else bgpio6 = 0;
	}
	buf3[0] = 0;
	strcpy(buf2,"ppin7");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) {
	pintemp = atoi(buf3);
	if (pinvalid(pintemp) & 0x02) {
	bgpio7 = pintemp;
	buf3[0] = 0;
	strcpy(buf2,"popt7");
	parsuri(buf1,buf3,buf2,4096,4,0);
	pintemp = atoi(buf3);
	bgpio7 = bgpio7 | pintemp;
	} else bgpio7 = 0;
	}
	buf3[0] = 0;
	strcpy(buf2,"ppin8");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) {
	pintemp = atoi(buf3);
	if (pinvalid(pintemp) & 0x02) {
	buf3[0] = 0;
	bgpio8 = pintemp;
	strcpy(buf2,"popt8");
	parsuri(buf1,buf3,buf2,4096,4,0);
	pintemp = atoi(buf3);
	bgpio8 = bgpio8 | pintemp;
	} else bgpio8 = 0;
	}
	buf3[0] = 0;
	strcpy(buf2,"ppin9");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) {
	pintemp = atoi(buf3);
	if (pinvalid(pintemp) & 0x04) {
	bgpio9 = pintemp;
	buf3[0] = 0;
	strcpy(buf2,"popt9");
	parsuri(buf1,buf3,buf2,4096,4,0);
	pintemp = atoi(buf3);
	bgpio9 = bgpio9 | pintemp;
	} else bgpio9 = 0;
	}
	buf3[0] = 0;
	strcpy(buf2,"ppina");
	parsuri(buf1,buf3,buf2,4096,4,0);
	if (buf3[0]) {
	pintemp = atoi(buf3);
	if (pinvalid(pintemp) & 0x04) {
	bgpio10 = pintemp;
	buf3[0] = 0;
	strcpy(buf2,"popta");
	parsuri(buf1,buf3,buf2,4096,4,0);
	pintemp = atoi(buf3);
	bgpio10 = bgpio10 | pintemp;
	} else bgpio10 = 0;
	}
	buf3[0] = 0;
	strcpy(buf2,"bhx1");
	parsuri(buf1,buf3,buf2,4096,4,0);
	pintemp = atoi(buf3);
	switch (pintemp) {
	case 1:
	bZeroHx6 = bStatHx6;
	break;
	case 2:
	bDivHx6 =  (bStatHx6 - bZeroHx6) / 900;
	break;
	case 3:
	bDivHx6 =  (bStatHx6 - bZeroHx6) / 1000;
	break;
	case 4:
	bDivHx6 =  (bStatHx6 - bZeroHx6) / 1500;
	break;
	case 5:
	bDivHx6 =  (bStatHx6 - bZeroHx6) / 1600;
	break;
	case 6:
	bDivHx6 =  (bStatHx6 - bZeroHx6) / 1700;
	break;
	case 7:
	bDivHx6 =  (bStatHx6 - bZeroHx6) / 1800;
	break;
	case 8:
	bDivHx6 =  (bStatHx6 - bZeroHx6) / 2000;
	break;
	case 9:
	bDivHx6 =  (bStatHx6 - bZeroHx6) / 2200;
	break;
	case 10:
	bDivHx6 =  (bStatHx6 - bZeroHx6) / 2400;
	break;
	case 11:
	bDivHx6 =  ((bStatHx6 - bZeroHx6) / 1000) | 0x80000000;
	break;
	}
//ESP_LOGI(AP_TAG, "Hx set: st: %d, zr: %d, diff: %d, div: %d", bStatHx6, bZeroHx6, (bStatHx6 - bZeroHx6),bDivHx6);

	if (mqttConnected) {
	buf3[0] = 0;
	strcpy(buf2,"mqtdel");
	parsuri(buf1,buf3,buf2,4096,2,0);
	if (buf3[0] == 0x31) {
	if (fdebug) ESP_LOGI(AP_TAG, "Delete Mqtt topics");

	strcpy(buf2, "r4s");
	itoa(R4SNUM,buf3,10);
	strcat(buf2, buf3);
	f_update = true;
	BleDevStA.t_rspdel = 0;
	BleDevStB.t_rspdel = 0;
	BleDevStC.t_rspdel = 0;
	BleDevStD.t_rspdel = 0;
	BleDevStE.t_rspdel = 0;
	while (BleDevStA.btauthoriz || BleDevStB.btauthoriz || BleDevStC.btauthoriz || BleDevStD.btauthoriz || BleDevStE.btauthoriz) vTaskDelay(200 / portTICK_PERIOD_MS);
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
	strcpy(buf1,"homeassistant/button/");
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
	strcpy(buf1,"homeassistant/cover/");
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
	strcpy(buf1,"homeassistant/text/");
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


//***** Delete common topics *********************************
	if (ble_mon) {
	strcpy(buf2, "r4s");
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
	strcpy(buf1,"homeassistant/button/");
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
	strcpy(buf1,"homeassistant/cover/");
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
//************************************************************


	if (R4SNUM !=R4SNUMO) {
	strcpy(buf2, "r4s");
	itoa(R4SNUMO,buf3,10);
	strcat(buf2, buf3);
	f_update = true;
	while (BleDevStA.btauthoriz || BleDevStB.btauthoriz || BleDevStC.btauthoriz || BleDevStD.btauthoriz || BleDevStE.btauthoriz) vTaskDelay(200 / portTICK_PERIOD_MS);
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
	strcpy(buf1,"homeassistant/text/");
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
	}    //mqttconn

	for (int i = 0; i < BleMonNum; i++) {
	if (!BleMR[i].sto) {
	memset(&BleMR[i],0,sizeof(BleMR[i]));
	}
	}
	WriteNVS();
	}
	strcpy(buf1,"<!DOCTYPE html><html><head><title>r4sGate</title><meta name='viewport' content='width=device-width, initial-scale=1.0'>");
	strcat(buf1, "<meta http-equiv=\"refresh\" content=\"3;URL=http://");
	strcat(buf1, bufip);
	strcat(buf1,"\">");
	strcat(buf1, "</head><body>Setting saved. Rebooting...</body></html>");
	httpd_resp_sendstr(req, buf1);
	free(buf1);

	if (fdebug) ESP_LOGI(AP_TAG, "Prepare to restart system!");
	vTaskDelay(1000 / portTICK_PERIOD_MS);
	esp_restart();
	}
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
	ble_mon_refr = ble_mon_refr & 0xfd;

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
	BleDevStA.t_rspdel = 0;
	BleDevStB.t_rspdel = 0;
	BleDevStC.t_rspdel = 0;
	BleDevStD.t_rspdel = 0;
	BleDevStE.t_rspdel = 0;
	t_tinc = 0;	
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
	if (fdebug) ESP_LOGI(AP_TAG, "Starting OTA");
	const esp_partition_t *pupdate = esp_ota_get_next_update_partition(NULL);
//info about running
	const esp_partition_t *running = esp_ota_get_running_partition();
	if (fdebug) ESP_LOGI(AP_TAG, "Running partition type %d subtype %d (offset 0x%08x)",
             running->type, running->subtype, running->address);

	/*deal with all receive packet*/
// read loop
	while (ota_running) {
	otabufoffs = 0;
//read max 2048 bytes
	int data_read = httpd_req_recv(req, otabuf, otabufsize);
	if (data_read < 0) {
	if (fdebug) ESP_LOGE(AP_TAG, "Error: data read error");
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
	if (fdebug) {
	esp_log_buffer_hex(AP_TAG, otabuf, 256);
	ESP_LOGI(AP_TAG, "Buff: %s",otabuf);
	}
//check "Content-Disposition" string in received data
	otabufoffs = parsoff(otabuf,"Content-Disposition", otabufsize);
	if (!otabufoffs) {
	if (fdebug) ESP_LOGE(AP_TAG, "Content-Disposition not found");
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
	if (fdebug) ESP_LOGE(AP_TAG, "application/octet-stream or application/macbinary not found");
	ota_running = false;
	otabufoffs = 0;
	data_read = 0;
	}

//if (fdebug) esp_log_buffer_hex(AP_TAG, otabuf, 128);
	if (fdebug) ESP_LOGI(AP_TAG, "Loading filename: %s",filnam);

	image_header_was_checked = true;
	err = esp_ota_begin(pupdate, OTA_SIZE_UNKNOWN, &update_handle);
	if (err != ESP_OK) {
	if (fdebug) ESP_LOGE(AP_TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
	ota_running = false;
	otabufoffs = 0;
	data_read = 0;
	}
//	if (fdebug) ESP_LOGI(AP_TAG, "esp_ota_begin succeeded");
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
//if (fdebug) ESP_LOGI(AP_TAG, "Written image length %d", binary_file_length);
	} else if (data_read == 0) {
	ota_running = false;
	otabufoffs = 0;
	data_read = 0;
	}
}
//
	if (fdebug) ESP_LOGI(AP_TAG, "Total Write binary data length: 0x%X", binary_file_length);
	
	err = esp_ota_end(update_handle);
	strcpy (otabuf,"<!DOCTYPE html><html><head><title>r4sGate</title><meta name='viewport' content='width=device-width, initial-scale=1.0'> ");
	strcat(otabuf,"<meta http-equiv=\"refresh\" content=\"3;URL=http://");
	strcat(otabuf, bufip);
	strcat(otabuf,"\">");
	strcat(otabuf,"</head><body>Update ");
	if (err != ESP_OK) {
	if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
	if (fdebug) ESP_LOGE(AP_TAG, "Image validation failed, image is corrupted");
	}
	if (fdebug) ESP_LOGE(AP_TAG, "esp_ota_end failed (%s)!", esp_err_to_name(err));
	strcat(otabuf,"failed");
	} else {
	if (fdebug) ESP_LOGI(AP_TAG, "esp_ota_end ok!");
	strcat(otabuf,"ok");
	}
	strcat(otabuf,". Rebooting...</body></html>");
	httpd_resp_sendstr(req, otabuf);

	err = esp_ota_set_boot_partition(pupdate);
	if (err != ESP_OK) {
	if (fdebug) ESP_LOGE(AP_TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
	ota_running = false;
	}
	if (fdebug) ESP_LOGI(AP_TAG, "Prepare to restart system!");
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
	config.max_uri_handlers = 19;
//	config.max_resp_headers = 16;
	config.stack_size = 10240;
	// Start the httpd server
/*
	if (fdebug) {
	ESP_LOGI(AP_TAG, "Starting server on port: '%d'", config.server_port);
	ESP_LOGI(AP_TAG, "Max URI handlers: '%d'", config.max_uri_handlers);
	ESP_LOGI(AP_TAG, "Max Open Sessions: '%d'", config.max_open_sockets);
	ESP_LOGI(AP_TAG, "Max Header Length: '%d'", HTTPD_MAX_REQ_HDR_LEN);
	ESP_LOGI(AP_TAG, "Max URI Length: '%d'", HTTPD_MAX_URI_LEN);
	ESP_LOGI(AP_TAG, "Max Stack Size: '%d'", config.stack_size);
	}
*/
	if (httpd_start(&server, &config) == ESP_OK) {
// Set URI handlers
//	if (fdebug) ESP_LOGI(AP_TAG, "Registering URI handlers");
	httpd_register_uri_handler(server, &pmain);
	httpd_register_uri_handler(server, &pcfgdev1);
	httpd_register_uri_handler(server, &pcfgdev1ok);
	httpd_register_uri_handler(server, &pcfgdev2);
	httpd_register_uri_handler(server, &pcfgdev2ok);
	httpd_register_uri_handler(server, &pcfgdev3);
	httpd_register_uri_handler(server, &pcfgdev3ok);
	httpd_register_uri_handler(server, &pcfgdev4);
	httpd_register_uri_handler(server, &pcfgdev4ok);
	httpd_register_uri_handler(server, &pcfgdev5);
	httpd_register_uri_handler(server, &pcfgdev5ok);
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
	if (fdebug) ESP_LOGI(AP_TAG, "Error starting server!");
	return NULL;
}

/*
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
//	if (fdebug) ESP_LOGI(AP_TAG, "Stopping webserver");
	stop_webserver(*server);
	*server = NULL;
	}
}

static void connect_handler(void* arg, esp_event_base_t event_base, 
                            int32_t event_id, void* event_data)
{
	httpd_handle_t* server = (httpd_handle_t*) arg;
	if (*server == NULL) {
//	if (fdebug) ESP_LOGI(AP_TAG, "Starting webserver");
	*server = start_webserver();
	}
}
*/

void lpcomstat(uint8_t blenum) {
	if (blenum > 4) return;
        struct BleDevSt *ptr;
	switch (blenum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}

	if (fkpmd && ptr->bKeep && ptr->bCtemp && (ptr->DEV_TYP < 10) && ptr->btauthoriz && !ptr->r4slpcom) {
	switch (ptr->bKeep) {
	case 1:
	ptr->bKeep = 0;
	if (!ptr->bState) {
	uint8_t temp = ptr->bHtemp;
	if ((temp < 35) || (temp > 98)) temp = ptr->bLtemp;
	if ((temp < 35) || (temp > 98)) temp = 40;
	if (ptr->DEV_TYP == 1) {	
	if (temp < 35) ptr->r4slppar1 = 0;
	else if (temp < 41) ptr->r4slppar1 = 1;
	else if (temp < 56) ptr->r4slppar1 = 2;
	else if (temp < 71) ptr->r4slppar1 = 3;
	else if (temp < 86) ptr->r4slppar1 = 4;
	else ptr->r4slppar1 = 5;
	} else ptr->r4slppar1 = temp;
	ptr->r4slppar2 = 1;
	ptr->r4slpcom = 3;
	}
	break;
	case 2:
	ptr->bKeep = 0;
	if (!ptr->bState) {
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 5;
	}
	break;
	case 3:
	if ((!ptr->bState && !ptr->bProg) || (ptr->bProg == 3)) {
	ptr->bKeep = 0;
	ptr->r4slppar1 = 0;
	ptr->r4slpcom = 5;
	}
	break;
	default:
	ptr->bKeep = 0;
	break;
	}
	}
	if (ptr->r4slpcom) {
#ifdef USE_TFT
	if (jpg_time) t_jpg = jpg_time * 10;	
#endif
	switch (ptr->r4slpcom) {
	case 1:             //off
	ptr->r4slpres = 1;
	if (ptr->DEV_TYP < 10) {
	if (ptr->bHtemp && (ptr->bHtemp < 91)) m171sHeat(blenum, 0);
	if (ptr->bProg || ptr->bHeat) {
	m171sOff(blenum);
	m171s_ModOff(blenum);
//	ptr->bKeep = 0;
	}
	}
	if ((ptr->DEV_TYP > 11) && (ptr->DEV_TYP < 16)) {
	if (ptr->r4slppar1 == 1) m103sToff(blenum);
	else if (ptr->r4slppar1 == 2) m103sTon(blenum);
	ptr->bprevProg = 254;
	}
	m171sOff(blenum);
	ptr->bprevHtemp = 255;
	ptr->bprevState = 255;
	ptr->bprevHeat = 255;
	ptr->bprevStNl = 255;
	ptr->r4slpcom = 0;
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
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
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	break;
	case 3:             //heat
//	ptr->bKeep = 0;

	ptr->r4slpres = 1;
	m171sOff(blenum);
	ptr->bprevHtemp = 255;
	ptr->bprevState = 255;
	ptr->bprevHeat = 255;
	ptr->bprevStNl = 255;
	if (ptr->r4slppar1 && (ptr->r4slppar1 < 99)) m171sHeat(blenum, ptr->r4slppar1);
	else {
	m171s_ModOff(blenum);
	}
	ptr->r4slpcom = 0;
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	break;
	case 4:             //boil&heat
//	ptr->bKeep = 0;

	ptr->r4slpres = 1;
	m171sOff(blenum);
	if (ptr->r4slppar1 && (ptr->r4slppar1 < 99)) m171sBoilAndHeat(blenum, ptr->r4slppar1);
	else m171sBoil(blenum);
	ptr->bprevHtemp = 255;
	ptr->bprevState = 255;
	ptr->bprevHeat = 255;
	ptr->bprevStNl = 255;
	ptr->r4slpcom = 0;
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
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
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
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
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	break;
	case 7:             //power lock off
	ptr->r4slpres = 1;
	if (( ptr->DEV_TYP > 11 ) && ( ptr->DEV_TYP < 16 )) m151sLoff(blenum);
	else if (( ptr->DEV_TYP > 9) && ( ptr->DEV_TYP < 12 )) m103sLoff(blenum);
	else if ( ptr->DEV_TYP == 61) m103sLoff(blenum);
	ptr->bprevState = 255;
	ptr->bprevLock = 255;
	ptr->r4slpcom = 0;
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	break;
	case 8:             //power lock on
	ptr->r4slpres = 1;
	if (( ptr->DEV_TYP > 11 ) && ( ptr->DEV_TYP < 16 )) m151sLon(blenum);
	else if (( ptr->DEV_TYP > 9) && ( ptr->DEV_TYP < 12 )) m103sLon(blenum);
	else if ( ptr->DEV_TYP == 61) m103sLon(blenum);
	ptr->bprevState = 255;
	ptr->bprevLock = 255;
	ptr->r4slpcom = 0;
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	break;
	case 9:             //coffee strength
	ptr->r4slpres = 1;
	if (ptr->r4slppar1 == 0) m103sToff(blenum);
	else if (ptr->r4slppar1 == 1) m103sTon(blenum);
	ptr->bprevAwarm = 255;
	ptr->r4slpcom = 0;
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
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
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
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
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	break;

	case 12:	//800 mode
	ptr->r4slpres = 1;
	ptr->bprevModProg = 255;
        rm800sMod(blenum, ptr->r4slppar1);
	ptr->r4slpcom = 0;
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	break;

	case 13:	//800 temp
	ptr->r4slpres = 1;
	ptr->bprevHtemp = 255;
        rm800sTemp(blenum, ptr->r4slppar1);
	ptr->r4slpcom = 0;
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	break;

	case 14:	//800 phour
	ptr->r4slpres = 1;
	ptr->bprevPHour = 255;
        rm800sPhour(blenum, ptr->r4slppar1);
	ptr->r4slpcom = 0;
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	break;

	case 15:	//800 pmin
	ptr->r4slpres = 1;
	ptr->bprevPMin = 255;
        rm800sPmin(blenum, ptr->r4slppar1);
	ptr->r4slpcom = 0;
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	break;

	case 16:	//800 awarm
	ptr->r4slpres = 1;
	ptr->bprevAwarm = 255;
        rm800sAwarm(blenum, ptr->r4slppar1);
	ptr->r4slpcom = 0;
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
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
	} else rm800sPall(blenum, ptr->r4slppar1, ptr->r4slppar2, ptr->r4slppar3, ptr->r4slppar4, ptr->r4slppar5, ptr->r4slppar7, ptr->r4slppar8, ptr->r4slppar6);
	ptr->r4slpcom = 0;
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
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
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	break;

	case 19:	//800 dhour
	ptr->r4slpres = 1;
	ptr->bprevDHour = 255;
        rm800sDhour(blenum, ptr->r4slppar1);
	ptr->r4slpcom = 0;
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	break;

	case 20:	//800 dmin
	ptr->r4slpres = 1;
	ptr->bprevDMin = 255;
        rm800sDmin(blenum, ptr->r4slppar1);
	ptr->r4slpcom = 0;
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	break;

	case 21:	//bklight or keep off
	ptr->r4slpres = 1;
	if (ptr->DEV_TYP < 10) {
	m171Bl(blenum, 0);
	ptr->bprevStBl = 255;
	} else if (ptr->DEV_TYP < 12) {
	m103sKoff(blenum);;
	ptr->bprevModProg = 255;
	}
	ptr->r4slpcom = 0;
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	break;

	case 22:	//bklight or keep on
	ptr->r4slpres = 1;
	if (ptr->DEV_TYP < 10) {
	m171Bl(blenum, 1);
	ptr->bprevStBl = 255;
	} else if (ptr->DEV_TYP < 12) {
	m103sKon(blenum);
	ptr->bprevModProg = 255;
	}
	ptr->r4slpcom = 0;
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	break;

	case 23:	//beep off
	ptr->r4slpres = 1;
        ptr->bprevStBp = 255;
	m171Bp(blenum, 0);
	ptr->r4slpcom = 0;
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	break;

	case 24:	//beep on
	ptr->r4slpres = 1;
        ptr->bprevStBp = 255;
	m171Bp(blenum, 1);
	ptr->r4slpcom = 0;
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	break;

	case 25:	//boil time
	ptr->r4slpres = 1;
        ptr->bprevBlTime = 128;
	m171sBlTm(blenum, ptr->r4slppar1);
	ptr->r4slpcom = 0;
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	break;

	case 26:	//blinds position
        MqState(blenum);
	ptr->r4slpres = 1;
	ptr->bprevState = 255;
	m43sPos(blenum, ptr->r4slppar1);
	ptr->r4slpcom = 0;
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	break;


	case 62:	//smoke reset
        MqState(blenum);
	ptr->r4slpres = 1;
	ptr->bprevState = 255;
	m61sClear(blenum);
	ptr->r4slpcom = 0;
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	break;

	case 63:	//weather calibrate
        MqState(blenum);
	ptr->r4slpres = 1;
	ptr->bprevState = 255;
	m51sCalibrate(blenum);
	ptr->r4slpcom = 0;
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	break;

	case 64:	//mi off
	ptr->r4slpres = 1;
	ptr->bprevHtemp = 255;
	ptr->bprevState = 255;
	ptr->bprevHeat = 255;
	mMiOff(blenum);
	ptr->r4slpcom = 0;
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	break;

	case 65:	//mi boil
	ptr->r4slpres = 1;
	ptr->bprevHtemp = 255;
	ptr->bprevState = 255;
	ptr->bprevHeat = 255;
	mMiBoil(blenum);
	ptr->r4slpcom = 0;
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	break;

	case 66:	//mi temp
	ptr->r4slpres = 1;
	ptr->bprevHtemp = 255;
	ptr->bprevState = 255;
	ptr->bprevHeat = 255;
	mMiHeat(blenum, ptr->r4slppar1);
	ptr->r4slpcom = 0;
	ptr->t_rspdel = 0;
//	ptr->t_rspcnt = 1;
	break;

	case 72:	//9001 off
	ptr->r4slpres = 1;
	ptr->bprevHtemp = 255;
	ptr->bprevState = 255;
	ptr->bprevHeat = 255;
	mgl90sw(blenum, 0, ptr->bPHour, ptr->bPMin);
	ptr->r4slpcom = 0;
	ptr->t_rspdel = 0;
	ptr->t_rspcnt = 5;
	break;

	case 73:	//9001 on
	ptr->r4slpres = 1;
	ptr->bprevHtemp = 255;
	ptr->bprevState = 255;
	ptr->bprevHeat = 255;
	mgl90sw(blenum, 1, ptr->bPHour, ptr->bPMin);
	ptr->r4slpcom = 0;
	ptr->t_rspdel = 0;
	ptr->t_rspcnt = 5;
	break;
	}
	}

	vTaskDelay(100 / portTICK_PERIOD_MS);
// every 4s get kettle state
	if (!ptr->t_rspdel) {
        MqState(blenum);
	ptr->r4slpres = 0;
	ptr->f_Sync++;
	if (ptr->f_Sync > 15) mkSync(blenum);	
#ifdef USE_TFT
	if ((tft_conn) && (t_tinc < 41)) tfblestate();
#endif
	if (ptr->t_rspcnt) {
	ptr->t_rspcnt = (ptr->t_rspcnt & 7) - 1;
	if (ptr->DEV_TYP == 73) ptr->t_rspdel = 7;
	else ptr->t_rspdel = 10;
	} else if ((ptr->DEV_TYP > 61) && (ptr->DEV_TYP < 64)) ptr->t_rspdel = 200;
	else if (ptr->DEV_TYP == 73) ptr->t_rspdel = 200;
	else if (ptr->DEV_TYP == 74) ptr->t_rspdel = 1800;
	else ptr->t_rspdel = 40;
	}
// every 1s display time and date
#ifdef USE_TFT
	if ((tft_conn) && !t_clock) {
	tftclock();
	t_clock = 10;	
	}
#endif

}



//******************* Main **********************
void app_main(void)
{
//	static httpd_handle_t server = NULL;
	printf("Starting r4sGate...\n");
	fdebug = 0;
	floop = 0;
	NumWfConn = 0;
	NumMqConn = 0;
	MemErr = 0;
#ifdef USE_TFT
	tft_conf = 0;
	tft_conn = 0;
	blstnum  = 5;
	PIN_NUM_MISO = 25;	//MISO
	PIN_NUM_MOSI = 23;	//MOSI
	PIN_NUM_CLK  = 19;	//CLK
	PIN_NUM_CS   = 16;	// Chip select control pin
	PIN_NUM_DC   = 17;	// Data Command control pin
	PIN_NUM_RST  = 18;	// Reset pin (could connect to RST pin)
	PIN_NUM_BCKL = 21;	// TFT_BACKLIGHT
	PIN_NUM_PWR = 33;	// lcd pwr pin always high
#endif
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
	f_update = false;
	mqtdel  = 0;
	macauth  = 0;
	volperc  = 0;
	fkpmd  = 0;
	R4SNUM = 0;
	R4SNUMO = 0;
	memset (&BleDevStA,0,sizeof(BleDevStA));
	memset (&BleDevStB,0,sizeof(BleDevStB));
	memset (&BleDevStC,0,sizeof(BleDevStC));
	memset (&BleDevStD,0,sizeof(BleDevStD));
	memset (&BleDevStE,0,sizeof(BleDevStE));
	BleDevStA.MiKettleID = 1;
	BleDevStB.MiKettleID = 1;
	BleDevStC.MiKettleID = 1;
	BleDevStD.MiKettleID = 1;
	BleDevStE.MiKettleID = 1;
	BleDevStA.notifyDataLen = -1;
	BleDevStB.notifyDataLen = -1;
	BleDevStC.notifyDataLen = -1;
	BleDevStD.notifyDataLen = -1;
	BleDevStE.notifyDataLen = -1;
	BleDevStA.readDataLen = -1;
	BleDevStB.readDataLen = -1;
	BleDevStC.readDataLen = -1;
	BleDevStD.readDataLen = -1;
	BleDevStE.readDataLen = -1;
	BleDevStA.readDataHandle = -1;
	BleDevStB.readDataHandle = -1;
	BleDevStC.readDataHandle = -1;
	BleDevStD.readDataHandle = -1;
	BleDevStE.readDataHandle = -1;
	BleDevStA.RgbR = 255;
	BleDevStA.RgbG = 255;
	BleDevStA.RgbB = 255;
	BleDevStB.RgbR = 255;
	BleDevStB.RgbG = 255;
	BleDevStB.RgbB = 255;
	BleDevStC.RgbR = 255;
	BleDevStC.RgbG = 255;
	BleDevStC.RgbB = 255;
	BleDevStD.RgbR = 255;
	BleDevStD.RgbG = 255;
	BleDevStD.RgbB = 255;
	BleDevStE.RgbR = 255;
	BleDevStE.RgbG = 255;
	BleDevStE.RgbB = 255;
	memset (FND_NAME,0,sizeof(FND_NAME));
	memset (MQTT_USER,0,sizeof(MQTT_USER));
	memset (MQTT_PASSWORD,0,sizeof(MQTT_PASSWORD));
	memset (MQTT_SERVER,0,sizeof(MQTT_SERVER));
	memset (WIFI_SSID,0,sizeof(WIFI_SSID));
	memset (WIFI_PASSWORD,0,sizeof(WIFI_PASSWORD));
	memset (NTP_SERVER,0,sizeof(NTP_SERVER));
	memset (AUTH_BASIC,0,sizeof(AUTH_BASIC));
#ifdef USE_TFT
	memset (MQTT_TOPP1,0,sizeof(MQTT_TOPP1));
	memset (MQTT_TOPP2,0,sizeof(MQTT_TOPP2));
	memset (MQTT_TOPP3,0,sizeof(MQTT_TOPP3));
	memset (MQTT_TOPP4,0,sizeof(MQTT_TOPP4));
	memset (MQTT_TOPP5,0,sizeof(MQTT_TOPP5));
	memset (MQTT_TOPP6,0,sizeof(MQTT_TOPP6));
	memset (MQTT_TOPP7,0,sizeof(MQTT_TOPP7));
	memset (MQTT_VALP1,0,sizeof(MQTT_VALP1));
	memset (MQTT_VALP2,0,sizeof(MQTT_VALP2));
	memset (MQTT_VALP3,0,sizeof(MQTT_VALP3));
	memset (MQTT_VALP4,0,sizeof(MQTT_VALP4));
	memset (MQTT_VALP5,0,sizeof(MQTT_VALP5));
	memset (MQTT_VALP6,0,sizeof(MQTT_VALP6));
	memset (MQTT_VALP7,0,sizeof(MQTT_VALP7));
	memset (MyHttpUri,0,sizeof(MyHttpUri));
	MyHttpMqtt = 0;
	MyJPGbuflen  = 32768;
#endif
	ble_mon = 0;
	ble_mon_refr = 0;
	BleDevStA.bEfficiency = 80;
	BleDevStB.bEfficiency = 80;
	BleDevStC.bEfficiency = 80;
	BleDevStD.bEfficiency = 80;
	BleDevStE.bEfficiency = 80;
	BleDevStA.bCVol = 254;
	BleDevStB.bCVol = 254;
	BleDevStC.bCVol = 254;
	BleDevStD.bCVol = 254;
	BleDevStE.bCVol = 254;
	memset (BleMR,0,sizeof(BleMR));
	memset (BleMX,0,sizeof(BleMX));
	memset (SnPi2c,0,sizeof(SnPi2c));
	for (int i = 0; i < 28; i++) {
        SnPi2c[i].par1 = 0xffff;
	}
	memset (bufcert,0,sizeof(bufcert));
	FDHass = 0;
	fmssl = 0;
	fmsslbundle = 0;
	fmsslhost = 0;
	fmwss = 0;
	bcertofs = 0;
	bcertsz = 0;
	foffln = 0;
	bgpio1 = 0;
	bgpio2 = 0;
	bgpio3 = 0;
	bgpio4 = 0;
	bgpio5 = 0;
	bgpio6 = 0;
	bgpio7 = 0;
	bgpio8 = 0;
	bgpio9 = 0;
	bgpio10 = 0;
	f_rmds = 0;
	f_i2cdev = 0;
	i2c_errcnt = 0;
	pwr_batmode = 0;
	pwr_batpscrmode = 255;
	pwr_batprevmode = 255;
	pwr_batlevp = 0;
	i2cdevnum = 0;
	i2cdevnumo = 0;
	strcpy(strON,"ON");
	strcpy(strOFF,"OFF");
	ret = ReadNVS();
	if (ret) {
	FinitNVS();
	WriteNVS();
	}
	strcpy(BleDevStA.RQC_NAME, BleDevStA.REQ_NAME);
	strcpy(BleDevStB.RQC_NAME, BleDevStB.REQ_NAME);
	strcpy(BleDevStC.RQC_NAME, BleDevStC.REQ_NAME);
	strcpy(BleDevStD.RQC_NAME, BleDevStD.REQ_NAME);
	strcpy(BleDevStE.RQC_NAME, BleDevStE.REQ_NAME);
	if ((BleDevStA.DEV_TYP > 15) && (BleDevStA.DEV_TYP < 61)) BleDevStA.bProg = 255;
	if ((BleDevStB.DEV_TYP > 15) && (BleDevStB.DEV_TYP < 61)) BleDevStB.bProg = 255;
	if ((BleDevStC.DEV_TYP > 15) && (BleDevStC.DEV_TYP < 61)) BleDevStC.bProg = 255;
	if ((BleDevStD.DEV_TYP > 15) && (BleDevStD.DEV_TYP < 61)) BleDevStD.bProg = 255;
	if ((BleDevStE.DEV_TYP > 15) && (BleDevStE.DEV_TYP < 61)) BleDevStE.bProg = 255;
#ifdef USE_TFT
	if ((PIN_NUM_MISO > 39) || (PIN_NUM_MOSI > 33) || (PIN_NUM_CLK > 33) || 
	(PIN_NUM_CS > 33) || (PIN_NUM_DC > 33)) tft_conf = 0;
#endif

	if (!WIFI_SSID[0]) {
	strcpy(WIFI_SSID, INIT_WIFI_SSID);
	strcpy(WIFI_PASSWORD, INIT_WIFI_PASSWORD);
	}
// fill basic parameters
	R4SNUMO = R4SNUM;
	BleDevStA.tBLEAddr[0] = 0;
	char tzbuff[8];
	strcpy(MQTT_BASE_TOPIC, "r4s");
	itoa(R4SNUM,tzbuff,10);
	strcat(MQTT_BASE_TOPIC, tzbuff);
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
	if (bgpio1 < (MxPOutP + 64)) {
	gpio_set_direction((bgpio1 & 0x3f), GPIO_MODE_OUTPUT);
	mygp_iomux_out(bgpio1 & 0x3f);
	gpio_set_level((bgpio1 & 0x3f), 0);
	lvgpio1 = 0;
	} else {
	gpio_set_direction((bgpio1 & 0x3f), GPIO_MODE_INPUT);
	lvgpio1 = 0;
	if ((bgpio1 & 0x3f) < MxPOutP) gpio_set_pull_mode((bgpio1 & 0x3f), GPIO_PULLUP_ONLY);
	}
	}
	if (bgpio2 > 63) {
	if (bgpio2 < (MxPOutP + 64)) {
	gpio_set_direction((bgpio2 & 0x3f), GPIO_MODE_OUTPUT);
	mygp_iomux_out(bgpio2 & 0x3f);
	gpio_set_level((bgpio2 & 0x3f), 0);
	lvgpio2 = 0;
	} else {
	gpio_set_direction((bgpio2 & 0x3f), GPIO_MODE_INPUT);
	lvgpio2 = 0;
	if ((bgpio2 & 0x3f) < MxPOutP) gpio_set_pull_mode((bgpio2 & 0x3f), GPIO_PULLUP_ONLY);
	}
	}
	if (bgpio3 > 63) {
	if (bgpio3 < (MxPOutP + 64)) {
	gpio_set_direction((bgpio3 & 0x3f), GPIO_MODE_OUTPUT);
	mygp_iomux_out(bgpio3 & 0x3f);
	gpio_set_level((bgpio3 & 0x3f), 0);
	lvgpio3 = 0;
	} else {
	gpio_set_direction((bgpio3 & 0x3f), GPIO_MODE_INPUT);
	lvgpio3 = 0;
	if ((bgpio3 & 0x3f) < MxPOutP) gpio_set_pull_mode((bgpio3 & 0x3f), GPIO_PULLUP_ONLY);
	}
	}
	if (bgpio4 > 63) {
	if (bgpio4 < (MxPOutP + 64)) {
	gpio_set_direction((bgpio4 & 0x3f), GPIO_MODE_OUTPUT);
	mygp_iomux_out(bgpio4 & 0x3f);
	gpio_set_level((bgpio4 & 0x3f), 0);
	lvgpio4 = 0;
	} else {
	gpio_set_direction((bgpio4 & 0x3f), GPIO_MODE_INPUT);
	lvgpio4 = 0;
	if ((bgpio4 & 0x3f) < MxPOutP) gpio_set_pull_mode((bgpio4 & 0x3f), GPIO_PULLUP_ONLY);
	}
	}
	if (bgpio5 > 63) {
	if (bgpio5 < (MxPOutP + 64)) {
	gpio_set_direction((bgpio5 & 0x3f), GPIO_MODE_OUTPUT);
	mygp_iomux_out(bgpio5 & 0x3f);
	gpio_set_level((bgpio5 & 0x3f), 0);
	lvgpio5 = 0;
	} else {
	gpio_set_direction((bgpio5 & 0x3f), GPIO_MODE_INPUT);
	lvgpio5 = 0;
	if ((bgpio5 & 0x3f) < MxPOutP) gpio_set_pull_mode((bgpio5 & 0x3f), GPIO_PULLUP_ONLY);
	}
	}
//setup pwm timer
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_8_BIT, // resolution of PWM duty
        .freq_hz = 3136,                      // frequency of PWM signal may be used for beep
        .speed_mode = LEDC_LOW_SPEED_MODE,   // timer mode
        .timer_num = LEDC_TIMER_1,            // timer index
        .clk_cfg = LEDC_AUTO_CLK,             // Auto select the source clock
    };
	ledc_timer_config(&ledc_timer);
//
	if ((bgpio6 > 63) && (bgpio6 < 128)) {
	bStatG6 = 0;
	if (bgpio6 < (MxPOutP + 64)) {
    ledc_channel_config_t ledc_channel = {
            .channel    = LEDC_CHANNEL_1,
            .duty       = 0,
            .gpio_num   = (bgpio6 & 0x3f),
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .hpoint     = 0,
            .timer_sel  = LEDC_TIMER_1
    };
        ledc_channel_config(&ledc_channel);
	}
	} else if ((bgpio6 > 127) && (bgpio6 < 192)) {
	bStatG6 = 0xffff; //-0 if error
	if (bgpio6 < (MxPOutP + 128)) {
        if (bgpio5 > 191) {
	rmthx_init (0, (bgpio6 & 0x3f));
	} else if (rmt1w_init (0, (bgpio6 & 0x3f), &RmtRgHd0)) {
	f_rmds = (f_rmds & 0xef) | 0x01;
	rmt1w_readds(0, &f_rmds, &bStatG6, RmtRgHd0);    //44 start conversion
	}
	}
	} else if (bgpio6 > 191) {
        bStatG6 = 0xffff; //-0
        bStatG6h = 0;
	if (bgpio6 < (MxPOutP + 192)) {
#ifdef USE_IRTX
        bStatG6 = 0;
        bStatG6h = 0;
        if (!rmtir_init (0, (bgpio6 & 0x3f))) bgpio6 = bgpio6 & 0x3f;
#else
	if (rmt1w_init (0, (bgpio6 & 0x3f), &RmtRgHd0)) rmt1w_readdht(0, &f_rmds, &bStatG6, &bStatG6h, RmtRgHd0);
#endif
	}
	}

	if ((bgpio7 > 63) && (bgpio7 < 128)) {
	bStatG7 = 0;
	if (bgpio7 < (MxPOutP + 64)) {
    ledc_channel_config_t ledc_channel = {
            .channel    = LEDC_CHANNEL_2,
            .duty       = 0,
            .gpio_num   = (bgpio7 & 0x3f),
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .hpoint     = 0,
            .timer_sel  = LEDC_TIMER_1
    };
        ledc_channel_config(&ledc_channel);
	}
	} else if ((bgpio7 > 127) && (bgpio7 < 192)) {
	bStatG7 = 0xffff; //-0 if error
	if (bgpio7 < (MxPOutP + 128)) {
	if (rmt1w_init (1, (bgpio7 & 0x3f), &RmtRgHd1)) {
	f_rmds = (f_rmds & 0xdf) | 0x02;
	rmt1w_readds(1, &f_rmds, &bStatG7, RmtRgHd1);    //44 start conversion
	}
	}
	} else if (bgpio7 > 191) {
        bStatG7 = 0xffff; //-0
        bStatG7h = 0;
	if (bgpio7 < (MxPOutP + 192)) {
	if (rmt1w_init (1, (bgpio7 & 0x3f), &RmtRgHd1)) rmt1w_readdht(1, &f_rmds, &bStatG7, &bStatG7h, RmtRgHd1);
	}
	}

	if ((bgpio8 > 63) && (bgpio8 < 128)) {
	bStatG8 = 0;
	if (bgpio8 < (MxPOutP + 64)) {
    ledc_channel_config_t ledc_channel = {
            .channel    = LEDC_CHANNEL_3,
            .duty       = 0,
            .gpio_num   = (bgpio8 & 0x3f),
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .hpoint     = 0,
            .timer_sel  = LEDC_TIMER_1
    };
        ledc_channel_config(&ledc_channel);
	}
#ifdef CONFIG_IDF_TARGET_ESP32C3
	} else bgpio8 = 0;
#else
	} else if ((bgpio8 > 127) && (bgpio8 < 192)) {
	bStatG8 = 0xffff; //-0 if error
	if (bgpio8 < (MxPOutP + 128)) {
	if (rmt1w_init (2, (bgpio8 & 0x3f), &RmtRgHd2)) {
	f_rmds = (f_rmds & 0xbf) | 0x04;
	rmt1w_readds(2, &f_rmds, &bStatG8, RmtRgHd2);    //44 start conversion
	}
	}
	} else if (bgpio8 > 191) {
        bStatG8 = 0xffff; //-0
        bStatG8h = 0;
	if (bgpio8 < (MxPOutP + 192)) {
	if (rmt1w_init (2, (bgpio8 & 0x3f), &RmtRgHd2)) rmt1w_readdht(2, &f_rmds, &bStatG8, &bStatG8h, RmtRgHd2);
	}
	}
#endif
	if ((bgpio9 > 191) && (bgpio10 > 191) && (bgpio9 < (MxPOutP + 192)) && (bgpio10 < (MxPOutP + 192))) { // if i2c
	if (!i2c_init_bus ()) {                            //if i2c init
        f_i2cdev = f_i2cdev | 0x80000000;
	i2c_devnum(&f_i2cdev, &s_i2cdev, &i2cdevnum);
	i2c_init_pwr(&f_i2cdev);
	i2c_init_rtc(0, &f_i2cdev);
	i2c_init_rtc(1, &f_i2cdev);
	i2c_init_bme280(0, &f_i2cdev);
	i2c_init_bme280(1, &f_i2cdev);
	i2c_init_sht3x(0, &f_i2cdev);
	i2c_init_sht3x(1, &f_i2cdev);
	i2c_init_aht2x(0, &f_i2cdev);
	i2c_init_htu21(0, &f_i2cdev);
	if (f_i2cdev & 0x60000000) i2c_read_pwr(&f_i2cdev, &pwr_batmode, &pwr_batlevp, &pwr_batlevv, &pwr_batlevc);
	} //if i2c init
	} //if i2c

// 
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
#else
	vTaskDelay(700 / portTICK_RATE_MS);     //delay fo ds18b20 conversion
#endif
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
	sntp_servermode_dhcp(1);
	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	if(!NTP_SERVER[0]) strcpy (NTP_SERVER, "pool.ntp.org");
	sntp_setservername(0, NTP_SERVER);
	sntp_init();
	if (fdebug) ESP_LOGI(AP_TAG,"NTP server: %s", NTP_SERVER);
//Initialize Wifi
	wifi_init_sta();
//init bt
        Isscanning = false;
        StartStopScanReq = false;
	ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
	esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
	ret = esp_bt_controller_init(&bt_cfg);
	if (ret) {
	if (fdebug) ESP_LOGI(AP_TAG,"%s init controller failed: %s\n", __func__, esp_err_to_name(ret));
	fflush(stdout);
	esp_restart();
	}
	ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
	if (ret) {
	if (fdebug) ESP_LOGI(AP_TAG,"%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
	fflush(stdout);
	esp_restart();
	}
	if (fdebug) ESP_LOGI(AP_TAG,"%s init bluetooth\n", __func__);
	ret = esp_bluedroid_init();
	if (ret) {
	if (fdebug) ESP_LOGI(AP_TAG,"%s init bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
	fflush(stdout);
	esp_restart();
	}
	ret = esp_bluedroid_enable();
	if (ret) {
	if (fdebug) ESP_LOGI(AP_TAG,"%s enable bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
	fflush(stdout);
	esp_restart();
	}

	ESP_LOGI(AP_TAG,"Basic Auth string: %s",AUTH_BASIC);
	esp_read_mac(binblemac, ESP_MAC_BT);
	if (fdebug) {
	ESP_LOGI(AP_TAG,"esp32 BLE MAC:");
        esp_log_buffer_hex(AP_TAG, binblemac, 6);
	}
//register the  callback function to the gap module
	ret = esp_ble_gap_register_callback(esp_gap_cb);
	if (ret){
	if (fdebug) ESP_LOGI(AP_TAG,"%s gap register failed, error code = 0x%X\n", __func__, ret);
	fflush(stdout);
	esp_restart();
	}
//register the callback function to the gattc module
	ret = esp_ble_gattc_register_callback(esp_gattc_cb);
	if(ret){
	if (fdebug) ESP_LOGI(AP_TAG,"%s gattc register failed, error code = 0x%X\n", __func__, ret);
	fflush(stdout);
	esp_restart();
	}
	ret = esp_ble_gattc_app_register(PROFILE_A_APP_ID);
	if (ret){
	if (fdebug) ESP_LOGI(AP_TAG,"%s gattc app register failed, error code = 0x%X\n", __func__, ret);
	fflush(stdout);
	esp_restart();
	}
	ret = esp_ble_gattc_app_register(PROFILE_B_APP_ID);
	if (ret){
	if (fdebug) ESP_LOGI(AP_TAG,"%s gattc app register failed, error code = 0x%X\n", __func__, ret);
	fflush(stdout);
	esp_restart();
	}
	ret = esp_ble_gattc_app_register(PROFILE_C_APP_ID);
	if (ret){
	if (fdebug) ESP_LOGI(AP_TAG,"%s gattc app register failed, error code = 0x%X\n", __func__, ret);
	fflush(stdout);
	esp_restart();
	}
	ret = esp_ble_gattc_app_register(PROFILE_D_APP_ID);
	if (ret){
	if (fdebug) ESP_LOGI(AP_TAG,"%s gattc app register failed, error code = 0x%X\n", __func__, ret);
	fflush(stdout);
	esp_restart();
	}
	ret = esp_ble_gattc_app_register(PROFILE_E_APP_ID);
	if (ret){
	if (fdebug) ESP_LOGI(AP_TAG,"%s gattc app register failed, error code = 0x%X\n", __func__, ret);
	fflush(stdout);
	esp_restart();
	}
	esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(BLE_INPUT_BUFFSIZE);
	if (local_mtu_ret){
	if (fdebug) ESP_LOGI(AP_TAG,"Set local  MTU failed, error code = 0x%X\n", local_mtu_ret);
	}
// read ds & bme & dht sensors before mqtt init
	if (f_rmds & 0x01) rmt1w_readds(0, &f_rmds, &bStatG6, RmtRgHd0);
	else if ((bgpio5 > 191) && (bgpio6 > 127) && (bgpio6 < (MxPOutP + 128))) {
	readHx(0, &bStatHx6);
	}
	if (f_rmds & 0x02) rmt1w_readds(1, &f_rmds, &bStatG7, RmtRgHd1);
	if (f_rmds & 0x04) rmt1w_readds(2, &f_rmds, &bStatG8, RmtRgHd2);
#ifdef USE_IRTX

#else
	if ((bgpio6 > 191) && (bgpio6 < (MxPOutP + 192))) rmt1w_readdht(0, &f_rmds, &bStatG6, &bStatG6h, RmtRgHd0);
#endif
	if ((bgpio7 > 191) && (bgpio7 < (MxPOutP + 192))) rmt1w_readdht(1, &f_rmds, &bStatG7, &bStatG7h, RmtRgHd1);
	if ((bgpio8 > 191) && (bgpio8 < (MxPOutP + 192))) rmt1w_readdht(2, &f_rmds, &bStatG8, &bStatG8h, RmtRgHd2);
	if (f_i2cdev & 0x80000000) {
	if (f_i2cdev & 0x01) {
	if (i2c_read_bme280(0, &f_i2cdev, &SnPi2c[0].par1, &SnPi2c[0].par2, &SnPi2c[0].par3, &SnPi2c[0].par4)) i2c_init_bme280(0, &f_i2cdev);
	}
	if (f_i2cdev & 0x02) {
	if (i2c_read_bme280(1, &f_i2cdev, &SnPi2c[1].par1, &SnPi2c[1].par2, &SnPi2c[1].par3, &SnPi2c[1].par4)) i2c_init_bme280(1, &f_i2cdev);
	}
	if (f_i2cdev & 0x04) {
	if (i2c_read_sht3x(0, &f_i2cdev, &SnPi2c[2].par1, &SnPi2c[2].par2)) i2c_init_sht3x(0, &f_i2cdev);
	}
	if (f_i2cdev & 0x08) {
	if (i2c_read_sht3x(1, &f_i2cdev, &SnPi2c[3].par1, &SnPi2c[3].par2)) i2c_init_sht3x(1, &f_i2cdev);
	}
	if (f_i2cdev & 0x10) {
	if (i2c_read_aht2x(0, &f_i2cdev, &SnPi2c[4].par1, &SnPi2c[4].par2)) i2c_init_aht2x(0, &f_i2cdev);
	}
	if (f_i2cdev & 0x20) {
	if (i2c_read_htu21(0, &f_i2cdev, &SnPi2c[5].par1, &SnPi2c[5].par2)) i2c_init_htu21(0, &f_i2cdev);
	}
	if (f_i2cdev & 0x40) {
	if (i2c_read_rtc(0, &f_i2cdev, &SnPi2c[6].par1)) i2c_init_rtc(0, &f_i2cdev);
	}
	if (f_i2cdev & 0x80) {
	if (i2c_read_rtc(1, &f_i2cdev, &SnPi2c[7].par1)) i2c_init_rtc(1, &f_i2cdev);
	}
	}
//Initialize Mqtt
	if (MQTT_SERVER[0]) mqtt_app_start();
// get esp mac addr 
	tESP32Addr[0] = 0;
	tESP32Addr1[0] = 0;
	esp_read_mac(binwfmac,0);
	bin2hex(binwfmac, tESP32Addr,6,0);
	bin2hex(binwfmac, tESP32Addr1,6,0x3a);
	itoa(R4SNUM,tzbuf,10);
	strcat (tESP32Addr,tzbuf);

//Initialize http server
//	ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
//	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));
/* Start the server for the first time */
//	server = start_webserver();
	start_webserver();


// mark as valid boot for prevent rollback after ota
	esp_ota_mark_app_valid_cancel_rollback();	
	if (fdebug) ESP_LOGI(AP_TAG,"Init end free heap: %d\n", esp_get_free_heap_size());
	floop = 32;
#ifdef USE_TFT
	t_jpg = jpg_time * 10;	
	blstnum_inc();
	t_tinc = 40;	
	MyHttpMqtt |= 0x40;
#endif

//r4s state nonitoring and command execution loop
	while (floop) {

	lpcomstat(0);
	lpcomstat(1);
	lpcomstat(2);
	lpcomstat(3);
	lpcomstat(4);
#ifdef USE_IRTX
	if ((bgpio6 > 191) && (bgpio6 < (MxPOutP + 192))) {
	rmtir_send(0, &bStatG6, &bprevStatG6, &bStatG6h);
	if (bStatG6 != bprevStatG6) t_lasts = 0;
	}
#endif
	if (!t_lasts && !t_ppcons) {
	MqSState();
	t_lasts = 60;
	}

//if no command
	if (!BleDevStA.r4slpcom && !BleDevStB.r4slpcom && !BleDevStC.r4slpcom && !BleDevStD.r4slpcom && !BleDevStE.r4slpcom) {
#ifdef USE_TFT
	if (!f_update && tft_conn && jpg_time && !t_jpg) {
	ret = tftjpg();
	if (ret) {
	if (!(f_rmds & 0x01) && (bgpio5 > 191) && (bgpio6 > 127) && (bgpio6 < (MxPOutP + 128))) {
	readHx(0, &bStatHx6);
	if (bStatHx6 != bprevStatHx6) t_lasts = 0;
	}
	t_tinc = 60;
	}
	if (((MyHttpMqtt & 0x03) ^ 3) && !t_ppcons && MyHttpUri[0] && mqttConnected) {
	char ldata[32]; 
	char tmpvar[16]; 
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/jpg_url");
	esp_mqtt_client_publish (mqttclient, ldata, MyHttpUri, 0, 1, 1);
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/jpg_time");
	itoa(jpg_time,tmpvar,10);
	esp_mqtt_client_publish (mqttclient, ldata, tmpvar, 0, 1, 1);
	MyHttpMqtt |= 0x40;
	}
	t_jpg = jpg_time * 10;	
	} else if (!f_update && tft_conn && !jpg_time && ((MyHttpMqtt & 0x80) == 0x80)) {
	pushImage(0, 52, 320, 240, wallpaper);
	MyHttpMqtt = MyHttpMqtt & 0x7f;
	}
#endif
	if (!t_tinc) {
#ifdef USE_TFT
	blstnum_inc();
	if (tft_conn) tfblestate();
	if (tft_conn) tftclock();
	t_clock = 10;	
	if (f_update && tft_conn) tfblestate();
#endif
	if (!f_update) {
	esp_err_t err = 0;
	if(RmtDsNum > 2) RmtDsNum = 0;
	if (!(f_rmds & 0x01) && (bgpio5 > 191) && (bgpio6 > 127) && (bgpio6 < (MxPOutP + 128))) {
	readHx(0, &bStatHx6);
	if (bStatHx6 != bprevStatHx6) t_lasts = 0;
	}
	switch (RmtDsNum) {
	case 1:
	if (f_rmds & 0x02) rmt1w_readds(1, &f_rmds, &bStatG7, RmtRgHd1);
	if ((bgpio7 > 191) && (bgpio7 < (MxPOutP + 192))) rmt1w_readdht(1, &f_rmds, &bStatG7, &bStatG7h, RmtRgHd1);
	if ((bStatG7 != bprevStatG7) || (bStatG7h != bprevStatG7h)) t_lasts = 0;
	if (f_i2cdev & 0x80000000) {
	if (f_i2cdev & 0x02) {
	err = i2c_read_bme280(1, &f_i2cdev, &SnPi2c[1].par1, &SnPi2c[1].par2, &SnPi2c[1].par3, &SnPi2c[1].par4);
	if (err && (err != ESP_ERR_TIMEOUT)) i2c_init_bme280(1, &f_i2cdev);
	if ((SnPi2c[1].ppar1 != SnPi2c[1].par1) || (SnPi2c[1].ppar2 != SnPi2c[1].par2) || (SnPi2c[1].ppar3 != SnPi2c[1].par3) || (SnPi2c[1].ppar4 != SnPi2c[1].par4)) t_lasts = 0;
	} else if (s_i2cdev & 0x02) i2c_init_bme280(1, &f_i2cdev);
	if (f_i2cdev & 0x40) {
	if (i2c_read_rtc(0, &f_i2cdev, &SnPi2c[6].par1)) i2c_init_rtc(0, &f_i2cdev);
	if (SnPi2c[6].ppar1 != SnPi2c[6].par1) t_lasts = 0;
	}  else if (s_i2cdev & 0x40) i2c_init_rtc(0, &f_i2cdev);
	}
	if (f_i2cdev & 0x08) {
	err = i2c_read_sht3x(1, &f_i2cdev, &SnPi2c[3].par1, &SnPi2c[3].par2);
	if (err && (err != ESP_ERR_TIMEOUT)) i2c_init_sht3x(1, &f_i2cdev);
	if ((SnPi2c[3].ppar1 != SnPi2c[3].par1) || (SnPi2c[3].ppar2 != SnPi2c[3].par2)) t_lasts = 0;
	} else if (s_i2cdev & 0x08)  i2c_init_sht3x(1, &f_i2cdev);
	break;
	case 2:
	if (f_rmds & 0x04) rmt1w_readds(2, &f_rmds, &bStatG8, RmtRgHd2);
	if ((bgpio8 > 191) && (bgpio8 < (MxPOutP + 192))) rmt1w_readdht(2, &f_rmds, &bStatG8, &bStatG8h, RmtRgHd2);
	if (f_i2cdev & 0x80000000) {
	if (f_i2cdev & 0x10) {
	err = i2c_read_aht2x(0, &f_i2cdev, &SnPi2c[4].par1, &SnPi2c[4].par2);
	if (err && (err != ESP_ERR_TIMEOUT)) i2c_init_aht2x(0, &f_i2cdev);
	if ((SnPi2c[4].ppar1 != SnPi2c[4].par1) || (SnPi2c[4].ppar2 != SnPi2c[4].par2)) t_lasts = 0;
	} else if (s_i2cdev & 0x10) i2c_init_aht2x(0, &f_i2cdev);
	if (f_i2cdev & 0x80) {
	if (i2c_read_rtc(1, &f_i2cdev, &SnPi2c[7].par1)) i2c_init_rtc(1, &f_i2cdev);
	}  else if (s_i2cdev & 0x80) i2c_init_rtc(1, &f_i2cdev);
	if (f_i2cdev & 0x60000000) {
	if (i2c_read_pwr(&f_i2cdev, &pwr_batmode, &pwr_batlevp, &pwr_batlevv, &pwr_batlevc)) i2c_init_pwr(&f_i2cdev);
	if (pwr_batpscrmode != pwr_batmode) {
#ifdef USE_TFT
	if (tft_conn && PIN_NUM_BCKL) {	
	if (PIN_NUM_BCKL < MxPOutP) {	
	if (pwr_batmode & 0x06) ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, bStateS);
	else ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, bStateS >> 4);
	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
	} else {
	if (pwr_batmode & 0x06) i2c_axpin_set (&f_i2cdev, PIN_NUM_BCKL, bStateS); 
	else {
	if (bStateS) i2c_axpin_set (&f_i2cdev, PIN_NUM_BCKL, (bStateS >> 2) | 1);
	else i2c_axpin_set (&f_i2cdev, PIN_NUM_BCKL, 0);
	}
	}
	}
#endif
	pwr_batprevmode = 255;
	pwr_batpscrmode = pwr_batmode;
	}
	}
	i2c_devnum(&f_i2cdev, &s_i2cdev, &i2cdevnum);
	if (FDHass && (i2cdevnum > i2cdevnumo)) i2cdevnumo =255;
	else if ((i2cdevnumo == 255) && mqttConnected && HDisci2c(&f_i2cdev)) i2cdevnumo = i2cdevnum;
	else if (i2cdevnumo > i2cdevnum)  i2cdevnumo = i2cdevnum;
	}
	if ((bStatG8 != bprevStatG8) || (bStatG8h != bprevStatG8h) || (pwr_batlevp != pwr_batprevlevp) || (pwr_batmode != pwr_batprevmode)) t_lasts = 0;
	break;
	default:
	if (f_rmds & 0x01) rmt1w_readds(0, &f_rmds, &bStatG6, RmtRgHd0);
#ifdef USE_IRTX

#else
	if ((bgpio6 > 191) && (bgpio6 < (MxPOutP + 192))) rmt1w_readdht(0, &f_rmds, &bStatG6, &bStatG6h, RmtRgHd0);
	if ((bStatG6 != bprevStatG6) || (bStatG6h != bprevStatG6h)) t_lasts = 0;
#endif
	if (f_i2cdev & 0x80000000) {
	if (f_i2cdev & 0x01) {
	err = i2c_read_bme280(0, &f_i2cdev, &SnPi2c[0].par1, &SnPi2c[0].par2, &SnPi2c[0].par3, &SnPi2c[0].par4);
	if (err && (err != ESP_ERR_TIMEOUT)) i2c_init_bme280(0, &f_i2cdev);
	if ((SnPi2c[0].ppar1 != SnPi2c[0].par1) || (SnPi2c[0].ppar2 != SnPi2c[0].par2) || (SnPi2c[0].ppar3 != SnPi2c[0].par3) || (SnPi2c[0].ppar4 != SnPi2c[0].par4)) t_lasts = 0;
	} else if (s_i2cdev & 0x01) i2c_init_bme280(0, &f_i2cdev);
	if (f_i2cdev & 0x04) {
	err = i2c_read_sht3x(0, &f_i2cdev, &SnPi2c[2].par1, &SnPi2c[2].par2);
	if (err && (err != ESP_ERR_TIMEOUT)) i2c_init_sht3x(0, &f_i2cdev);
	if ((SnPi2c[2].ppar1 != SnPi2c[2].par1) || (SnPi2c[2].ppar2 != SnPi2c[2].par2))  t_lasts = 0;
	} else if (s_i2cdev & 0x04) i2c_init_sht3x(0, &f_i2cdev);
	if (f_i2cdev & 0x20) {
	err = i2c_read_htu21(0, &f_i2cdev, &SnPi2c[5].par1, &SnPi2c[5].par2);
	if (err && (err != ESP_ERR_TIMEOUT)) i2c_init_htu21(0, &f_i2cdev);
	if ((SnPi2c[5].ppar1 != SnPi2c[5].par1) || (SnPi2c[5].ppar2 != SnPi2c[5].par2))  t_lasts = 0;
	} else if (s_i2cdev & 0x20) i2c_init_htu21(0, &f_i2cdev);
	}
	break;
	}
	RmtDsNum++;
	}
	t_tinc = 40;
	if (wf_retry_cnt >= (WIFI_MAXIMUM_RETRY << 4)) {
	if (fdebug) ESP_LOGI(AP_TAG,"Wifi disconnected. Restarting now...");
#ifdef USE_TFT
	blstnum = 253;
	if (tft_conn) tfststr("Wifi disconnected. Restarting ...", NULL, NULL); 
#endif
	fflush(stdout);
	vTaskDelay(1000 / portTICK_PERIOD_MS);
	floop = 0;
	} else floop = 16;

	} //inc
	} //no command
} //main loop
	esp_restart();
}
