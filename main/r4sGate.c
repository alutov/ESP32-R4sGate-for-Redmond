/* 
****************************************************************
	ESP32 r4sGate for Redmond Kettle main
	Lutov Andrey  Donetsk 2020.07.17
Use for compilation ESP-IDF Programming Guide:
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/
****************************************************************
*/

// It's a good idea to connect ili9341 320*240 tft in the future
//#define USE_TFT

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
	} else cout[j] = cin[i];
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

#ifdef USE_TFT
#include "tft/tft.c"
#endif

//******************** ble **********************
/* Declare static functions */
static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
static void esp_gattc_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);
static void gattc_profile_a_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);


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
    uint16_t char_handle;
    esp_bd_addr_t remote_bda;
};

/* One gatt-based profile one app_id and one gattc_if, this array will store the gattc_if returned by ESP_GATTS_REG_EVT */
static struct gattc_profile_inst gl_profile_tab[PROFILE_NUM] = {

//connection profile and RX handle
    [PROFILE_A_APP_ID] = {
        .gattc_cb = gattc_profile_a_event_handler,
        .gattc_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
    },

//I use profile B for store TX handle only
    [PROFILE_B_APP_ID] = {
        .gattc_cb = gattc_profile_a_event_handler,
        .gattc_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
    },

};

static void gattc_profile_a_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    esp_ble_gattc_cb_param_t *p_data = (esp_ble_gattc_cb_param_t *)param;

    switch (event) {
    case ESP_GATTC_REG_EVT:
        ESP_LOGI(AP_TAG, "REG_EVT");

	esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV,ESP_PWR_LVL_P9); // for more power???
	esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT,ESP_PWR_LVL_P9); // for more power???

        esp_err_t scan_ret = esp_ble_gap_set_scan_params(&ble_scan_params);
        if (scan_ret){
            ESP_LOGE(AP_TAG, "Set scan params error, error code = %x", scan_ret);
        }
        break;
    case ESP_GATTC_CONNECT_EVT:{
        ESP_LOGI(AP_TAG, "ESP_GATTC_CONNECT_EVT conn_id %d, if %d", p_data->connect.conn_id, gattc_if);
        gl_profile_tab[PROFILE_A_APP_ID].conn_id = p_data->connect.conn_id;
        gl_profile_tab[PROFILE_B_APP_ID].conn_id = p_data->connect.conn_id;
        memcpy(gl_profile_tab[PROFILE_A_APP_ID].remote_bda, p_data->connect.remote_bda, sizeof(esp_bd_addr_t));
        memcpy(gl_profile_tab[PROFILE_B_APP_ID].remote_bda, p_data->connect.remote_bda, sizeof(esp_bd_addr_t));
        ESP_LOGI(AP_TAG, "REMOTE BDA:");
        esp_log_buffer_hex(AP_TAG, gl_profile_tab[PROFILE_A_APP_ID].remote_bda, sizeof(esp_bd_addr_t));
        esp_err_t mtu_ret = esp_ble_gattc_send_mtu_req (gattc_if, p_data->connect.conn_id);
        if (mtu_ret){
            ESP_LOGE(AP_TAG, "Config MTU error, error code = %x", mtu_ret);
        }
        break;
    }
    case ESP_GATTC_OPEN_EVT:
        if (param->open.status != ESP_GATT_OK){
            ESP_LOGE(AP_TAG, "Open failed, status %d", p_data->open.status);
            break;
        }
        ESP_LOGI(AP_TAG, "Open success");
        break;
    case ESP_GATTC_DIS_SRVC_CMPL_EVT:
        if (param->dis_srvc_cmpl.status != ESP_GATT_OK){
            ESP_LOGE(AP_TAG, "discover service failed, status %d", param->dis_srvc_cmpl.status);
            break;
        }
        ESP_LOGI(AP_TAG, "Discover service complete conn_id %d", param->dis_srvc_cmpl.conn_id);
        esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id, &remote_filter_service_uuid);
        break;
    case ESP_GATTC_CFG_MTU_EVT:
        if (param->cfg_mtu.status != ESP_GATT_OK){
            ESP_LOGE(AP_TAG,"Config mtu failed, error status = %x", param->cfg_mtu.status);
        }
        ESP_LOGI(AP_TAG, "ESP_GATTC_CFG_MTU_EVT, Status %d, MTU %d, conn_id %d", param->cfg_mtu.status, param->cfg_mtu.mtu, param->cfg_mtu.conn_id);
        break;
    case ESP_GATTC_SEARCH_RES_EVT: {
        ESP_LOGI(AP_TAG, "SEARCH RES: conn_id = %x is primary service %d", p_data->search_res.conn_id, p_data->search_res.is_primary);
        ESP_LOGI(AP_TAG, "start handle %d end handle %d current handle value %d", p_data->search_res.start_handle, p_data->search_res.end_handle, p_data->search_res.srvc_id.inst_id);
        if ((p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_128) && (bt_compare_UUID128(p_data->search_res.srvc_id.uuid.uuid.uuid128, remote_filter_service_uuid.uuid.uuid128))) {

            ESP_LOGI(AP_TAG, "Service found");
            get_server = true;
            gl_profile_tab[PROFILE_A_APP_ID].service_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[PROFILE_A_APP_ID].service_end_handle = p_data->search_res.end_handle;
            gl_profile_tab[PROFILE_B_APP_ID].service_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[PROFILE_B_APP_ID].service_end_handle = p_data->search_res.end_handle;
        }
        break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT:
        if (p_data->search_cmpl.status != ESP_GATT_OK){
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
        if (get_server){
            uint16_t count = 0;
            esp_gatt_status_t status = esp_ble_gattc_get_attr_count( gattc_if,
                                                                     p_data->search_cmpl.conn_id,
                                                                     ESP_GATT_DB_CHARACTERISTIC,
                                                                     gl_profile_tab[PROFILE_A_APP_ID].service_start_handle,
                                                                     gl_profile_tab[PROFILE_A_APP_ID].service_end_handle,
                                                                     INVALID_HANDLE,
                                                                     &count);
            if (status != ESP_GATT_OK){
                ESP_LOGE(AP_TAG, "esp_ble_gattc_get_attr_count error");
            }
            if (count > 0){
		char_elem_result = (esp_gattc_char_elem_t *)malloc(sizeof(esp_gattc_char_elem_t) * count);
                if (!char_elem_result){
                    ESP_LOGE(AP_TAG, "Gattc no mem");
                }else{

		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
							     p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_A_APP_ID].service_start_handle,
                                                             gl_profile_tab[PROFILE_A_APP_ID].service_end_handle,
                                                             remote_filter_rxchar_uuid,
                                                             (char_elem_result),
                                                             &count);
                    if (status != ESP_GATT_OK){
                        ESP_LOGE(AP_TAG, "Get_rxchar_by_uuid error");
                    }
		    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
                                                             p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_B_APP_ID].service_start_handle,
                                                             gl_profile_tab[PROFILE_B_APP_ID].service_end_handle,
                                                             remote_filter_txchar_uuid,
                                                             (char_elem_result+1),
                                                             &count);

                    if (status != ESP_GATT_OK){
                        ESP_LOGE(AP_TAG, "Get_txchar_by_uuid error");
                    }

//        esp_log_buffer_hex(AP_TAG, char_elem_result, 44);

                    /*  Every service have only one char in our 'ESP_GATTS_DEMO' demo, so we used first 'char_elem_result' */
                    if (count > 0 && (char_elem_result[0].properties & ESP_GATT_CHAR_PROP_BIT_NOTIFY)){
                        gl_profile_tab[PROFILE_A_APP_ID].char_handle = char_elem_result[0].char_handle;
                        esp_ble_gattc_register_for_notify (gattc_if, gl_profile_tab[PROFILE_A_APP_ID].remote_bda, char_elem_result[0].char_handle);

                        gl_profile_tab[PROFILE_B_APP_ID].char_handle = char_elem_result[1].char_handle;
			ESP_LOGI(AP_TAG, "Register_for_notify");
                    }
                }
                /* free char_elem_result */
                free(char_elem_result);
            }else{
                ESP_LOGE(AP_TAG, "No char found");
            }
        }
         break;
    case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
//        ESP_LOGI(AP_TAG, "ESP_GATTC_REG_FOR_NOTIFY_EVT");
        if (p_data->reg_for_notify.status != ESP_GATT_OK){
            ESP_LOGE(AP_TAG, "REG FOR NOTIFY failed: error status = %d", p_data->reg_for_notify.status);
        }else{
            uint16_t count = 0;
            uint16_t notify_en = 1; 
            esp_gatt_status_t ret_status = esp_ble_gattc_get_attr_count( gattc_if,
                                                                         gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                                                         ESP_GATT_DB_DESCRIPTOR,
                                                                         gl_profile_tab[PROFILE_A_APP_ID].service_start_handle,
                                                                         gl_profile_tab[PROFILE_A_APP_ID].service_end_handle,
                                                                         gl_profile_tab[PROFILE_A_APP_ID].char_handle,
                                                                         &count);
            if (ret_status != ESP_GATT_OK){
                ESP_LOGE(AP_TAG, "Get_attr_count error");
            }
            if (count > 0){
                descr_elem_result = malloc(sizeof(esp_gattc_descr_elem_t) * count);
                if (!descr_elem_result){
                    ESP_LOGE(AP_TAG, "malloc error, gattc no mem");
                }else{
                    ret_status = esp_ble_gattc_get_descr_by_char_handle( gattc_if,
                                                                         gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                                                         p_data->reg_for_notify.handle,
                                                                         notify_descr_uuid,
                                                                         descr_elem_result,
                                                                         &count);
                    if (ret_status != ESP_GATT_OK){
                        ESP_LOGE(AP_TAG, "Get_descr_by_char_handle error");
                    }
                    /* Every char has only one descriptor in our 'ESP_GATTS_DEMO' demo, so we used first 'descr_elem_result' */
                    if (count > 0 && descr_elem_result[0].uuid.len == ESP_UUID_LEN_16 && descr_elem_result[0].uuid.uuid.uuid16 == ESP_GATT_UUID_CHAR_CLIENT_CONFIG){
                        ret_status = esp_ble_gattc_write_char_descr( gattc_if,
                                                                     gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                                                     descr_elem_result[0].handle,
                                                                     sizeof(notify_en),
                                                                     (uint8_t *)&notify_en,
                                                                     ESP_GATT_WRITE_TYPE_RSP,
                                                                     ESP_GATT_AUTH_REQ_NONE);
                    }

                    if (ret_status != ESP_GATT_OK){
                        ESP_LOGE(AP_TAG, "Write_char_descr error");
                    }
                    /* free descr_elem_result */
                    free(descr_elem_result);
                }
            }
            else{
                ESP_LOGE(AP_TAG, "Decsr not found");
            }

        }
        break;
    }
    case ESP_GATTC_NOTIFY_EVT:
        if (p_data->notify.is_notify){
	if (btauthorize) {
	int length = p_data->notify.value_len;
	if (length > BLE_INPUT_BUFFSIZE)
	length = BLE_INPUT_BUFFSIZE;
	if (length > 0) {
        memcpy(notifyData, p_data->notify.value, length);
	}
	notifyDataLen = length;
	} else {
	if (!memcmp(p_data->notify.value, "\x55\x00\xff\x01\xaa", 5)) {
	btauthorize = true;
	r4sConnErr = 0;
	r4scounter = 1;	
	NumConn++;
	bin2hex(gl_profile_tab[PROFILE_A_APP_ID].remote_bda, tBLEAddr,6);
	strcpy(DEV_NAME,REQ_NAME);
	if (mqttConnected) {
	char ldata[64];
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddr);
	strcat(ldata,"/online");
	esp_mqtt_client_publish(mqttclient, ldata, "1", 0, 1, 0);
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddr);
	strcat(ldata,"/name");
	esp_mqtt_client_publish(mqttclient, ldata, DEV_NAME, 0, 1, 0);
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddr);
	strcat(ldata,"/cmd/state");
	esp_mqtt_client_subscribe(mqttclient, ldata, 0);
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddr);
	strcat(ldata,"/cmd/heat_temp");
	esp_mqtt_client_subscribe(mqttclient, ldata, 0);
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddr);
	strcat(ldata,"/cmd/nightlight");
	esp_mqtt_client_subscribe(mqttclient, ldata, 0);
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddr);
	strcat(ldata,"/cmd/nightlight_red");
	esp_mqtt_client_subscribe(mqttclient, ldata, 0);
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddr);
	strcat(ldata,"/cmd/nightlight_green");
	esp_mqtt_client_subscribe(mqttclient, ldata, 0);
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddr);
	strcat(ldata,"/cmd/nightlight_blue");
	esp_mqtt_client_subscribe(mqttclient, ldata, 0);

	ESP_LOGI(AP_TAG, "Authorize ok");
	}
//ESP_LOGI(AP_TAG, "Addr: %s",tBLEAddr);



	} else esp_ble_gattc_close(gl_profile_tab[PROFILE_A_APP_ID].gattc_if,gl_profile_tab[PROFILE_A_APP_ID].conn_id);	
	}
//            ESP_LOGI(AP_TAG, "ESP_GATTC_NOTIFY_EVT, receive notify value:");
        }else{
//            ESP_LOGI(AP_TAG, "ESP_GATTC_NOTIFY_EVT, receive indicate value:");
        }
//        esp_log_buffer_hex(AP_TAG, p_data->notify.value, p_data->notify.value_len);

//esp_log_buffer_hex(AP_TAG, gl_profile_tab[PROFILE_A_APP_ID].remote_bda,6);

        break;
    case ESP_GATTC_WRITE_DESCR_EVT:
        if (p_data->write.status != ESP_GATT_OK){
            ESP_LOGE(AP_TAG, "Write descr failed, error status = %x", p_data->write.status);
            break;
        }
        uint8_t write_char_data[12] = { 0x55,0x00,0xff,0xb6,0x2c,0x27,0xb3,0xb8,0xac,0x5a,0xef,0xaa};  //auth string
	write_char_data[5] = write_char_data[5] + R4SNUM;  //for each gate number different auth id
        esp_gatt_status_t ret_status = esp_ble_gattc_write_char( gattc_if,
                                  gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                  gl_profile_tab[PROFILE_B_APP_ID].char_handle,
                                  sizeof(write_char_data),
                                  write_char_data,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
                        ESP_LOGE(AP_TAG, "Write_char_auth error");
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
            ESP_LOGE(AP_TAG, "Write char failed, error status = %x", p_data->write.status);
            break;
        }
        break;

    case ESP_GATTC_DISCONNECT_EVT:
        btconnect = false;
        btauthorize = false;
        get_server = false;
        ESP_LOGI(AP_TAG, "ESP_GATTC_DISCONNECT_EVT, reason = %d", p_data->disconnect.reason);
	if ((mqttConnected) &&(tBLEAddr[0])) {
            char ldata[32];
            strcpy(ldata,MQTT_BASE_TOPIC);
            strcat(ldata,"/");
            strcat(ldata,tBLEAddr);
            strcat(ldata,"/online");
            esp_mqtt_client_publish(mqttclient, ldata, "0", 0, 1, 0);
	}

        //the unit of the duration is second
        uint32_t duration = 0; //30;
        esp_ble_gap_start_scanning(duration);
        break;
    default:
        break;
    }
}

static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    uint8_t *adv_name = NULL;
    uint8_t adv_name_len = 0;
    switch (event) {

    case ESP_GAP_BLE_READ_RSSI_COMPLETE_EVT:
	iRssi = param->read_rssi_cmpl.rssi;
//ESP_LOGI(AP_TAG,"read-RSSI: %d", param->read_rssi_cmpl.rssi); //test #2 with data from read_rssi_cmpl
	if ((r4sConnErr < 6 ) && (btauthorize)) {
	if ((sendDataLen > 0) && (sendDataLen < BLE_INPUT_BUFFSIZE)) {
        esp_gatt_status_t ret_status = esp_ble_gattc_write_char( gl_profile_tab[PROFILE_A_APP_ID].gattc_if,
                                  gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                  gl_profile_tab[PROFILE_B_APP_ID].char_handle,
                                  sendDataLen,
                                  sendData,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
                    if (ret_status != ESP_GATT_OK){
                        ESP_LOGE(AP_TAG, "Write_char_data error");
	r4sConnErr++;
	}  else {
//ESP_LOGI(AP_TAG, "r4sConnErr: %d", r4sConnErr);
ESP_LOGI(AP_TAG, "Send Data:");
esp_log_buffer_hex(AP_TAG, sendData, sendDataLen);
	}
	sendDataLen = 0;
	}
	} else if (btauthorize) esp_ble_gattc_close(gl_profile_tab[PROFILE_A_APP_ID].gattc_if,gl_profile_tab[PROFILE_A_APP_ID].conn_id);	
	break;

    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT: {
        //the unit of the duration is second
        uint32_t duration = 0; //30;
        esp_ble_gap_start_scanning(duration);
        break;
    }
    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
        //scan start complete event to indicate scan start successfully or failed
        if (param->scan_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(AP_TAG, "scan start failed, error status = %x", param->scan_start_cmpl.status);
            break;
        }
        ESP_LOGI(AP_TAG, "Scan start success");

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
	    if (fnd_namelen >15) fnd_namelen = 15;		
            mystrcpy(FND_NAME, (char *)adv_name,  fnd_namelen);
	}
            if ((adv_name != NULL) && (REQ_NAME[0])) {
                if (strlen(REQ_NAME) == adv_name_len && strncmp((char *)adv_name, REQ_NAME, adv_name_len) == 0) {
                    ESP_LOGI(AP_TAG, "Searched device %s\n", REQ_NAME);
                    if (btconnect == false) {
                        btconnect = true;
                        ESP_LOGI(AP_TAG, "Connect to the remote device.");
                        esp_ble_gap_stop_scanning();
                        esp_ble_gattc_open(gl_profile_tab[PROFILE_A_APP_ID].gattc_if, scan_result->scan_rst.bda, scan_result->scan_rst.ble_addr_type, true);
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
        if (param->scan_stop_cmpl.status != ESP_BT_STATUS_SUCCESS){
            ESP_LOGE(AP_TAG, "Scan stop failed, error status = %x", param->scan_stop_cmpl.status);
            break;
        }
        ESP_LOGI(AP_TAG, "stop scan successfully");
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


//******************** r4s **********************

//[I][R4S.cpp:24]   r4sWrite(): >> 55 59 06 aa
//                         offset:  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19
//[I][R4S.cpp:43] r4sCommand(): << 55 59 06 00 00 00 00 00 02 00 00 00 00 39 00 00 00 00 00 aa

uint8_t r4sWrite(uint8_t cmd, uint8_t* data, size_t len) {
	size_t sz = 4 + len; // 55, counter, cmd, AA
	sendData[0] = 0x55;
	sendData[1] = r4scounter;
	sendData[2] = cmd;
	sendData[sz - 1] = 0xAA;
	if (len > 0) {
	memcpy(&sendData[3], data, len);
		}
	sendDataLen = sz;
//  ble_gap_read_rssi event return rssi and start sending data
	if (btauthorize) esp_ble_gap_read_rssi(gl_profile_tab[PROFILE_A_APP_ID].remote_bda);
	else {
	iRssi = 0;
	r4scounter = -1;
	}
	return r4scounter++;
}


int8_t r4sCommand(uint8_t cmd, uint8_t* data, size_t len) {
	notifyDataLen = -1;
	if (btauthorize) {
	uint8_t timeout = 30; 	// 100*30 = 3 seconds
	uint8_t cnt = r4sWrite(cmd, data, len);
	while (--timeout && (notifyDataLen == -1)) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
//
	if ((notifyDataLen < BLE_INPUT_BUFFSIZE) && (notifyDataLen > 1)) {
	r4sConnErr = 0;	
ESP_LOGI(AP_TAG, "Receive Data:");
esp_log_buffer_hex(AP_TAG, notifyData, notifyDataLen);
	}
//
	if ((notifyDataLen > 1) && (notifyData[1] != cnt)) {
	notifyDataLen = -1;
	r4sConnErr++;	
	}
	}
	return notifyDataLen;
}


bool m171sOff() {
	if (r4sCommand(0x04, 0, 0) != 5)
	return false;
	return notifyData[3] == 1;
}

bool m171sOn(uint8_t prog, uint8_t temp) {
	if (DEV_CHK2) {
	uint8_t data[] = { prog, 0, temp, 0, 1, temp, 30, 0, 0, 0, 0, 0, 0, 128, 0, 0};
	if (r4sCommand(0x05, data, sizeof(data)) != 5) return false;
	if (notifyData[3] != 1) return false;
	if (r4sCommand(0x03, 0, 0) != 5) return false;
	if (notifyData[3] != 1) return false;
	} else {
	uint8_t data[] = { prog, 0, temp, 0 };
	if (r4sCommand(0x05, data, sizeof(data)) != 5) return false;
	if (notifyData[3] == 1) return  false;
	if (DEV_CHK1) {
	if (r4sCommand(0x03, 0, 0) != 5) return false;
	if (notifyData[3] != 1) return false;
	}
	}
	return true;
}

bool m171sBoil() {
	return m171sOn(0, 0);
}

bool m171sHeat(uint8_t temp) {
	return m171sOn(1, temp);
}

bool m171sBoilAndHeat(uint8_t temp) {
	if ((DEV_CHK1)||(DEV_CHK2))  return m171sOn(2, temp);
	else return m171sOn(0, temp);
}

bool m171s_NLOn() {
	if (r4sCommand(0x32, nl_settings, sizeof(nl_settings)) != 5)
	return false;
	if (notifyData[3] == 1)
	return false;
    
	uint8_t data[] = { 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 0, 0};
	if (r4sCommand(0x05, data, sizeof(data)) != 5)
	return false;
	if (notifyData[3] == 1)
	return false;
    
	if (r4sCommand(0x03, 0, 0) != 5)
	return false;
	if (notifyData[3] != 1)
	return false;
    
	return true;
}

void m171sStatus() {
	char tmpvar[8]; 
	if (btauthorize) {
	if (r4sCommand(0x06, 0, 0) == 20) {
	if (DEV_CHK2) bCtemp = notifyData[8];
	else bCtemp = notifyData[13];
	bHtemp = notifyData[5];
	bState = notifyData[11];
	if (bState == 4) bState = 0;
	strcpy(cStatus,"{\"temp\":");
	(DEV_CHK2)? itoa(notifyData[8],tmpvar,10) : itoa(notifyData[13],tmpvar,10);
	strcat(cStatus,tmpvar);
	strcat(cStatus,",\"target\":");
	itoa(notifyData[5],tmpvar,10);
	strcat(cStatus,tmpvar);
	strcat(cStatus,",\"heat\":");
	(notifyData[10])? strcat(cStatus,"1") : strcat(cStatus,"0");
	strcat(cStatus,",\"state\":");
	itoa(notifyData[11],tmpvar,10);
	strcat(cStatus,tmpvar);
	strcat(cStatus,",\"hours\":");
	(DEV_CHK2)? itoa(notifyData[7],tmpvar,10) : itoa(notifyData[8],tmpvar,10);
	strcat(cStatus,tmpvar);
	strcat(cStatus,",\"mins\":");
	itoa(notifyData[9],tmpvar,10);
	strcat(cStatus,tmpvar);
	strcat(cStatus,",\"prog\":");
	itoa(notifyData[3],tmpvar,10);
	strcat(cStatus,tmpvar);
	strcat(cStatus,",\"error\":");
	itoa(notifyData[12],tmpvar,10);
	strcat(cStatus,tmpvar);
	strcat(cStatus,"}");    
	} else if (r4sConnErr > 2) {
	cStatus[0]=0;
	bState = 254;
	bCtemp = 0;
	bHtemp = 0;
	}
	} else {
	cStatus[0]=0;
	bState = 254;
	bCtemp = 0;
	bHtemp = 0;
	}
}

void MqState() {
	char ldata[64];
	char tmpvar[8]; 
	m171sStatus();
	if ((mqttConnected) && (tBLEAddr[0])) {
	if ((cStatus[0] != 0) && (strcmp(cStatus,cprevStatus) != 0)) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddr);
	strcat(ldata,"/rsp/json");
	esp_mqtt_client_publish(mqttclient, ldata, cStatus, 0, 1, 0);
	strcpy(cprevStatus,cStatus);
	} 
	if  (bprevCtemp != bCtemp) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddr);
	strcat(ldata,"/rsp/temp");
	itoa(bCtemp,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 0);
	bprevCtemp = bCtemp;
	}
	if  (bprevHtemp != bHtemp) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddr);
	strcat(ldata,"/rsp/heat_temp");
	itoa(bHtemp,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 0);
	bprevHtemp = bHtemp;
	}
	if  (bprevState != bState) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddr);
	strcat(ldata,"/rsp/state");
	if (!bState) esp_mqtt_client_publish(mqttclient, ldata, "false", 0, 1, 0);
        else if (bState == 254) esp_mqtt_client_publish(mqttclient, ldata, "offline", 0, 1, 0);
        else esp_mqtt_client_publish(mqttclient, ldata, "true", 0, 1, 0);
	bprevState = bState;
	}
	if  (iprevRssi != iRssi) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddr);
	strcat(ldata,"/rssi");
        itoa(iRssi,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 0);
	iprevRssi = iRssi;
	}
	if  (PRgbR != RgbR) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddr);
	strcat(ldata,"/rsp/nightlight_red");
        itoa(RgbR,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 0);
	PRgbR = RgbR;
	}
	if  (PRgbG != RgbG) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddr);
	strcat(ldata,"/rsp/nightlight_green");
        itoa(RgbG,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 0);
	PRgbG = RgbG;
	}
	if  (PRgbB != RgbB) {
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddr);
	strcat(ldata,"/rsp/nightlight_blue");
        itoa(RgbB,tmpvar,10);
	esp_mqtt_client_publish(mqttclient, ldata, tmpvar, 0, 1, 0);
	PRgbB = RgbB;
	}
}
}


//******************* Mqtt **********************
static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
	mqttConnected =true;
	ESP_LOGI(AP_TAG,"MQTT_EVENT_CONNECTED");
	bprevState = 255;
	bprevCtemp = 255;
	bprevHtemp = 255;
	iprevRssi = 0;

	char llwtt[16];
	strcpy(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/online");
	msg_id = esp_mqtt_client_publish(client, llwtt, "1", 0, 1, 0);
	ESP_LOGI(AP_TAG,"sent publish successful, msg_id=%d", msg_id);

	if (tBLEAddr[0]) {
	char ldata[64];
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddr);
	strcat(ldata,"/online");
	msg_id = esp_mqtt_client_publish(mqttclient, ldata, "1", 0, 1, 0);
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddr);
	strcat(ldata,"/name");
	msg_id = esp_mqtt_client_publish(client, ldata, DEV_NAME, 0, 1, 0);
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddr);
	strcat(ldata,"/cmd/state");
	msg_id = esp_mqtt_client_subscribe(mqttclient, ldata, 0);
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddr);
	strcat(ldata,"/cmd/heat_temp");
	msg_id = esp_mqtt_client_subscribe(mqttclient, ldata, 0);
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddr);
	strcat(ldata,"/cmd/nightlight");
	msg_id = esp_mqtt_client_subscribe(mqttclient, ldata, 0);
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddr);
	strcat(ldata,"/cmd/nightlight_red");
	msg_id = esp_mqtt_client_subscribe(mqttclient, ldata, 0);
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddr);
	strcat(ldata,"/cmd/nightlight_green");
	msg_id = esp_mqtt_client_subscribe(mqttclient, ldata, 0);
	strcpy(ldata,MQTT_BASE_TOPIC);
	strcat(ldata,"/");
	strcat(ldata,tBLEAddr);
	strcat(ldata,"/cmd/nightlight_blue");
	msg_id = esp_mqtt_client_subscribe(mqttclient, ldata, 0);
	}

	break;
	case MQTT_EVENT_DISCONNECTED:
	mqttConnected =false;
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
	if((event->data_len)&&(event->topic_len)) {
	char ldata1[64];
	char ldata2[64];
	char ldata3[64];
	char ldata4[64];
	char ldata5[64];
	char ldata6[64];
	strcpy(ldata1,MQTT_BASE_TOPIC);
	strcat(ldata1,"/");
	strcat(ldata1,tBLEAddr);
	strcat(ldata1,"/cmd/");
	strcpy(ldata2,ldata1);
	strcpy(ldata3,ldata1);
	strcpy(ldata4,ldata1);
	strcpy(ldata5,ldata1);
	strcpy(ldata6,ldata1);
	strcat(ldata1,"state");
	strcat(ldata2,"heat_temp");
	strcat(ldata3,"nightlight");
	strcat(ldata4,"nightlight_red");
	strcat(ldata5,"nightlight_green");
	strcat(ldata6,"nightlight_blue");
	if (!memcmp(event->topic, ldata1, event->topic_len)) {
	esp_mqtt_client_publish(mqttclient, ldata1, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {
	
	r4slppar = 0;
	r4slpcom = 2;

//	ESP_LOGI(AP_TAG,"MQTT_CMD_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {

	r4slppar = 0;
	r4slpcom = 1;
	
//	ESP_LOGI(AP_TAG,"MQTT_CMD_OFF");
	} else {
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(ldata1, event->data, tempsz);
	uint8_t temp = atoi(ldata1);
	if (temp < 91) {
	r4slppar = temp;
	r4slpcom = 4;
			}
		}
	} else if (!memcmp(event->topic, ldata2, event->topic_len)) {
	esp_mqtt_client_publish(mqttclient, ldata2, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(ldata1, event->data, tempsz);
	uint8_t temp = atoi(ldata1);


	r4slppar = temp;
	r4slpcom = 3;
//	ESP_LOGI(AP_TAG,"MQTT_HEAT_TEMP");
	} else if (!memcmp(event->topic, ldata3, event->topic_len)) {
	esp_mqtt_client_publish(mqttclient, ldata3, "", 0, 1, 0);
	if ((!incascmp("1",event->data,event->data_len)) || (!incascmp("on",event->data,event->data_len))
		|| (!incascmp("true",event->data,event->data_len))) {

	r4slppar = 0;
	r4slpcom = 5;
//	ESP_LOGI(AP_TAG,"MQTT_NIGHTLIGHT_ON");
	} else if ((!incascmp("0",event->data,event->data_len)) || (!incascmp("off",event->data,event->data_len))
		|| (!incascmp("false",event->data,event->data_len))) {
	r4slppar = 0;
	r4slpcom = 1;
//	ESP_LOGI(AP_TAG,"MQTT_NIGHTLIGHT_OFF");
	}
	} else if (!memcmp(event->topic, ldata4, event->topic_len)) {
	esp_mqtt_client_publish(mqttclient, ldata4, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(ldata1, event->data, tempsz);
	uint8_t cval = atoi(ldata1);
	RgbR = cval;
	t_last_us = ~t_last_us;
	} else if (!memcmp(event->topic, ldata5, event->topic_len)) {
	esp_mqtt_client_publish(mqttclient, ldata5, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(ldata1, event->data, tempsz);
	uint8_t cval = atoi(ldata1);
	RgbG = cval;
	t_last_us = ~t_last_us;
	} else if (!memcmp(event->topic, ldata6, event->topic_len)) {
	esp_mqtt_client_publish(mqttclient, ldata6, "", 0, 1, 0);
	int tempsz = event->data_len;
	if  (tempsz > 60) tempsz = 60;
	mystrcpy(ldata1, event->data, tempsz);
	uint8_t cval = atoi(ldata1);
	RgbB = cval;
	t_last_us = ~t_last_us;
	}
	} 
	break;
	case MQTT_EVENT_ERROR:
//	ESP_LOGI(AP_TAG,"MQTT_EVENT_ERROR");
	break;
	default:
	ESP_LOGI(AP_TAG,"Other event id:%d", event->event_id);
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
	char luri[128];
	char llwtt[16];
	strcpy(luri,"mqtt://");
	strcat(luri,MQTT_USER);
	strcat(luri,":");
	strcat(luri,MQTT_PASSWORD);
	strcat(luri,"@");
	strcat(luri,MQTT_SERVER);
	strcat(luri,":1883");
	strcpy(llwtt,MQTT_BASE_TOPIC);
	strcat(llwtt,"/online");

	esp_mqtt_client_config_t mqtt_cfg = {
	.uri = luri,
	.lwt_topic = llwtt,
	.lwt_msg = "0",
	.keepalive = 60,
	.client_id = MQTT_BASE_TOPIC,
	};
	mqttConnected =false;
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

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
	esp_wifi_connect();
	} else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
	if (s_retry_num < WIFI_MAXIMUM_RETRY) {
		esp_wifi_connect();
		s_retry_num++;
	ESP_LOGI(AP_TAG, "retry to connect to the AP");
	} else {
	xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
	}
	ESP_LOGI(AP_TAG,"connect to the AP fail");
	} else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
	ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
	ESP_LOGI(AP_TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
	s_retry_num = 0;
	xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
	}
}

void wifi_init_sta(void)
{
	s_wifi_event_group = xEventGroupCreate();

	ESP_ERROR_CHECK(esp_netif_init());

	ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_netif_create_default_wifi_sta();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	esp_event_handler_instance_t instance_any_id;
	esp_event_handler_instance_t instance_got_ip;
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
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
	ESP_LOGI(AP_TAG,"Connected to ap SSID:%s password:%s",
		WIFI_SSID, WIFI_PASSWORD);
	} else if (bits & WIFI_FAIL_BIT) {
	ESP_LOGI(AP_TAG,"Failed to connect to SSID:%s, password:%s",
		WIFI_SSID, WIFI_PASSWORD);
	ESP_LOGI(AP_TAG,"Restarting now...");
	fflush(stdout);
	esp_restart();
	} else {
	ESP_LOGE(AP_TAG, "UNEXPECTED EVENT");
	}

	/* The event will not be processed after unregister */
	ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
	ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
	vEventGroupDelete(s_wifi_event_group);
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
	char bsend[4500];
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
	strcat(bsend," for Redmond Kettle</h1>");
	strcat(bsend,"<div class='menubar'><a class='menu active' href='.'>&#8962;<span class='showmenulabel'>Main</span></a>");
	strcat(bsend,"<a class='menu' href='switchoff'>&#10006;<span class='showmenulabel'>SwitchOff</span></a>");
	strcat(bsend,"<a class='menu' href='switchon'>&#9832;<span class='showmenulabel'>SwitchOn</span></a>");
	strcat(bsend,"<a class='menu' href='heaton'>&#9832;<span class='showmenulabel'>HeatOn</span></a>");
	strcat(bsend,"<a class='menu' href='bheaton'>&#9832;<span class='showmenulabel'>Boil&HeatOn</span></a>");
	strcat(bsend,"<a class='menu' href='lighton'>&#127752;<span class='showmenulabel'>NightLightOn</span></a>");
	strcat(bsend,"<a class='menu' href='setting'>&#128295;<span class='showmenulabel'>Setting</span></a>");
	strcat(bsend,"<a class='menu' href='restart'>&#128295;<span class='showmenulabel'>Reboot</span></a>");
	strcat(bsend,"<a class='menu' href='update'>&#128295;<span class='showmenulabel'>Load firmware</span></a></div>");
	strcat(bsend,"</header><table class='normal'><h3>");
       	if (tBLEAddr[0]) {
	strcat(bsend,"Device address: ");
	strcat(bsend,tBLEAddr);
	strcat(bsend,", Name: ");
	if (DEV_NAME[0]) strcat(bsend,DEV_NAME);
	strcat(bsend,", State: ");
        if (!bState) strcat(bsend,"Off");
 	else if (bState == 254) strcat(bsend,"Offline");
 	else strcat(bsend,"On");
	strcat(bsend,", Temp: ");
	itoa(bCtemp,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"&deg;C, Heat: ");
	itoa(bHtemp,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"&deg;C, ");
	}
        strcat(bsend,"Json String:</h3><br><h2>{\"mqtt\":");
	itoa(mqttConnected,buff,10);
	strcat(bsend,buff);
	strcat(bsend,",\"heap\":");
	itoa(FreeMem,buff,10);
	strcat(bsend,buff);
#ifdef USE_TFT
	strcat(bsend,",\"tft\":");
        (tft_conn)? strcat(bsend,"1") : strcat(bsend,"0");
#endif
        if (tBLEAddr[0]) {
        strcat(bsend,",\"");
	strcat(bsend,tBLEAddr);
        strcat(bsend,"\":{\"name\":\"");
	if (DEV_NAME[0]) strcat(bsend,DEV_NAME);
        strcat(bsend,"\",\"status\":\"");
        if (btauthorize) {
        strcat(bsend,"online\",\"rssi\":");
        itoa(iRssi,buff,10);
	strcat(bsend,buff);
        strcat(bsend,",\"state\":");
        if (cStatus[0]) strcat(bsend,cStatus);
        } else strcat(bsend,"offline\"");
	}
        strcat(bsend,"}</h2><br><h3>System Info</h3><table class='normal'>");
        strcat(bsend,"<tr><td style='min-width:150px;'>Version</td><td style='width:80%;'>2020.07.17</td></tr>");
	uptime_string_exp(buff);
        strcat(bsend,"<tr><td>Local time and date</td><td>");
	strcat(bsend,strftime_buf);
        strcat(bsend,"<tr><td>Uptime</td><td>");
	strcat(bsend,buff);
        strcat(bsend,"</td></tr><tr><td>Free memory</td><td>");
	itoa(FreeMem,buff,10);
	strcat(bsend,buff);
        strcat(bsend,"</td></tr><tr><td>BLE connection state</td><td>");
        (btauthorize)? strcat(bsend,"Connected") : strcat(bsend,"Disconnected");
	if (iRssi) {
        strcat(bsend,"</td></tr><tr><td>BLE RSSI</td><td>");
        itoa(iRssi,buff,10);
	strcat(bsend,buff);
        strcat(bsend," dB");
	}
	if (!btauthorize) {
        strcat(bsend,"</td></tr><tr><td>Last found device name</td><td>");
        strcat(bsend,FND_NAME);
	}
        strcat(bsend,"</td></tr><tr><td>BLE connection count</td><td>");
        itoa(NumConn,buff,10);
	strcat(bsend,buff);
        strcat(bsend,"</td></tr>");
	wifi_ap_record_t wifidata;
	if (esp_wifi_sta_get_ap_info(&wifidata)==0){
        strcat(bsend,"<tr><td>WiFi network</td><td>");
        buff[31] = 0;
	memcpy(buff,wifidata.ssid,31);
        strcat(bsend,buff);
	strcat(bsend,"</td></tr><tr><td>WiFi RSSI</td><td>");
        itoa(wifidata.rssi,buff,10);
	strcat(bsend,buff);
        strcat(bsend," dB</td></tr><tr><td>WiFi IP address</td><td>");
        strcat(bsend,bufip);
	strcat(bsend,"</td></tr>");
	}
	strcat(bsend,"<tr><td>MQTT server and port</td><td>");
        if (MQTT_SERVER[0]) strcat(bsend,MQTT_SERVER);
	strcat(bsend,":1883</td></tr><tr><td>MQTT connection state</td><td>");
        (mqttConnected)? strcat(bsend,"Connected") : strcat(bsend,"Disconnected");
        strcat(bsend,"</td></tr>");
	strcat(bsend,"<meta http-equiv=\"refresh\" content=\"5;URL=http://");
	strcat(bsend,bufip);
	strcat(bsend,"/\"</body></html>");
        httpd_resp_sendstr(req, bsend);
    return ESP_OK;
}
static const httpd_uri_t pmain = {
	.uri       = "/",
	.method    = HTTP_GET,
	.handler   = pmain_get_handler,
	.user_ctx  = NULL
};

/* HTTP GET switch off handler */
static esp_err_t pswitchoff_get_handler(httpd_req_t *req)
{
	r4slppar = 0;
	r4slpcom = 1;
	r4slpres = 1;
	uint8_t timeout = 35; 	// 100*35 = 3.5 seconds
	while (--timeout && (r4slpres)) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	httpd_resp_set_status(req, "303 See Other");
	httpd_resp_set_hdr(req, "Location", "/");
	httpd_resp_send(req, NULL, 0);  // Response body can be empty
    return ESP_OK;
}
static const httpd_uri_t pswitchoff = {
	.uri       = "/switchoff",
	.method    = HTTP_GET,
	.handler   = pswitchoff_get_handler,
	.user_ctx  = NULL
};

/* HTTP GET switch on handler */
static esp_err_t pswitchon_get_handler(httpd_req_t *req)
{
	r4slppar = 0;
	r4slpcom = 2;
	r4slpres = 1;
	uint8_t timeout = 35; 	// 100*35 = 3.5 seconds
	while (--timeout && (r4slpres)) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	httpd_resp_set_status(req, "303 See Other");
	httpd_resp_set_hdr(req, "Location", "/");
	httpd_resp_send(req, NULL, 0);  // Response body can be empty
    return ESP_OK;
}
static const httpd_uri_t pswitchon = {
	.uri       = "/switchon",
	.method    = HTTP_GET,
	.handler   = pswitchon_get_handler,
	.user_ctx  = NULL
};

/* HTTP GET heat handler */
static esp_err_t pheaton_get_handler(httpd_req_t *req)
{
	char bsend[4000];
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
	strcat(bsend," for Redmond Kettle</h1>");
	strcat(bsend,"<div class='menubar'><a class='menu' href='.'>&#8962;<span class='showmenulabel'>Main</span></a>");
	strcat(bsend,"<a class='menu' href='switchoff'>&#10006;<span class='showmenulabel'>SwitchOff</span></a>");
	strcat(bsend,"<a class='menu' href='switchon'>&#9832;<span class='showmenulabel'>SwitchOn</span></a>");
	strcat(bsend,"<a class='menu active' href='heaton'>&#9832;<span class='showmenulabel'>HeatOn</span></a>");
	strcat(bsend,"<a class='menu' href='bheaton'>&#9832;<span class='showmenulabel'>Boil&HeatOn</span></a>");
	strcat(bsend,"<a class='menu' href='lighton'>&#127752;<span class='showmenulabel'>NightLightOn</span></a>");
	strcat(bsend,"<a class='menu' href='setting'>&#128295;<span class='showmenulabel'>Setting</span></a>");
	strcat(bsend,"<a class='menu' href='restart'>&#128295;<span class='showmenulabel'>Reboot</span></a>");
	strcat(bsend,"<a class='menu' href='update'>&#128295;<span class='showmenulabel'>Load firmware</span></a></div>");
	strcat(bsend,"</header><table class='normal'><h3>Set Heat Temperature</h3><br />");
	strcat(bsend,"<body><form method=\"POST\" action=\"/heatonok\">");
	strcat(bsend,"<input name=\"heattemp\" type=\"number\" value=\"0\" min=\"0\" max=\"90\" size=\"2\"> 0-90&deg;C. If 0 heat off</br></br>");
	strcat(bsend,"<h3> Store value then press Ok. </h3><br/>");
	strcat(bsend,"<input type=SUBMIT value=\"Ok\">");
	strcat(bsend,"</form><form method=\"POST\" action=\"/setignore\">");
	strcat(bsend,"<input type=SUBMIT value=\"Cancel\"></form></body></html>");
        httpd_resp_sendstr(req, bsend);
    return ESP_OK;
}
static const httpd_uri_t pheaton = {
	.uri       = "/heaton",
	.method    = HTTP_GET,
	.handler   = pheaton_get_handler,
	.user_ctx  = NULL
};

static esp_err_t pheatonok_get_handler(httpd_req_t *req)
{
	char buf1[256] = {0};
	char buf2[16] = {0};
	char buf3[16] = {0};
	int  ret;
	ret = httpd_req_recv(req,buf1,255);
	if ( ret > 0 ) {
	strcpy(buf2,"heattemp");
	parsuri(buf1,buf3,buf2,255,4);
	uint8_t temp = atoi(buf3);
	r4slppar = temp;
	r4slpcom = 3;
	}
	r4slpres = 1;
	uint8_t timeout = 35; 	// 100*35 = 3.5 seconds
	while (--timeout && (r4slpres)) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	httpd_resp_set_status(req, "303 See Other");
	httpd_resp_set_hdr(req, "Location", "/");
	httpd_resp_send(req, NULL, 0);  // Response body can be empty
    return ESP_OK;
}
static const httpd_uri_t pheatonok = {
	.uri       = "/heatonok",
	.method    = HTTP_POST,
	.handler   = pheatonok_get_handler,
	.user_ctx  = NULL
};

static esp_err_t pbheatonok_get_handler(httpd_req_t *req)
{
	char buf1[256] = {0};
	char buf2[16] = {0};
	char buf3[16] = {0};
	int  ret;
	ret = httpd_req_recv(req,buf1,255);
	if ( ret > 0 ) {
	strcpy(buf2,"heattemp");
	parsuri(buf1,buf3,buf2,255,4);
	uint8_t temp = atoi(buf3);
	r4slppar = temp;
	r4slpcom = 4;
	}
	r4slpres = 1;
	uint8_t timeout = 35; 	// 100*35 = 3.5 seconds
	while (--timeout && (r4slpres)) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	httpd_resp_set_status(req, "303 See Other");
	httpd_resp_set_hdr(req, "Location", "/");
	httpd_resp_send(req, NULL, 0);  // Response body can be empty
    return ESP_OK;
}
static const httpd_uri_t pbheatonok = {
	.uri       = "/bheatonok",
	.method    = HTTP_POST,
	.handler   = pbheatonok_get_handler,
	.user_ctx  = NULL
};



/* HTTP GET boil&heat handler */
static esp_err_t pbheaton_get_handler(httpd_req_t *req)
{
	char bsend[4000];
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
	strcat(bsend," for Redmond Kettle</h1>");
	strcat(bsend,"<div class='menubar'><a class='menu' href='.'>&#8962;<span class='showmenulabel'>Main</span></a>");
	strcat(bsend,"<a class='menu' href='switchoff'>&#10006;<span class='showmenulabel'>SwitchOff</span></a>");
	strcat(bsend,"<a class='menu' href='switchon'>&#9832;<span class='showmenulabel'>SwitchOn</span></a>");
	strcat(bsend,"<a class='menu' href='heaton'>&#9832;<span class='showmenulabel'>HeatOn</span></a>");
	strcat(bsend,"<a class='menu active' href='bheaton'>&#9832;<span class='showmenulabel'>Boil&HeatOn</span></a>");
	strcat(bsend,"<a class='menu' href='lighton'>&#127752;<span class='showmenulabel'>NightLightOn</span></a>");
	strcat(bsend,"<a class='menu' href='setting'>&#128295;<span class='showmenulabel'>Setting</span></a>");
	strcat(bsend,"<a class='menu' href='restart'>&#128295;<span class='showmenulabel'>Reboot</span></a>");
	strcat(bsend,"<a class='menu' href='update'>&#128295;<span class='showmenulabel'>Load firmware</span></a></div>");
	strcat(bsend,"</header><table class='normal'><h3>Set Heat Temperature</h3><br />");
	strcat(bsend,"<body><form method=\"POST\" action=\"/bheatonok\">");
	strcat(bsend,"<input name=\"heattemp\" type=\"number\" value=\"0\" min=\"0\" max=\"90\" size=\"2\"> 0-90&deg;C. If 0 boil only</br></br>");
	strcat(bsend,"<h3> Store value then press Ok. </h3><br/>");
	strcat(bsend,"<input type=SUBMIT value=\"Ok\">");
	strcat(bsend,"</form><form method=\"POST\" action=\"/setignore\">");
	strcat(bsend,"<input type=SUBMIT value=\"Cancel\"></form></body></html>");
        httpd_resp_sendstr(req, bsend);
    return ESP_OK;
}
static const httpd_uri_t pbheaton = {
	.uri       = "/bheaton",
	.method    = HTTP_GET,
	.handler   = pbheaton_get_handler,
	.user_ctx  = NULL
};

/* HTTP GET night light on handler */
static esp_err_t plighton_get_handler(httpd_req_t *req)
{
	char bsend[4000];
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
	strcat(bsend," for Redmond Kettle</h1>");
	strcat(bsend,"<div class='menubar'><a class='menu' href='.'>&#8962;<span class='showmenulabel'>Main</span></a>");
	strcat(bsend,"<a class='menu' href='switchoff'>&#10006;<span class='showmenulabel'>SwitchOff</span></a>");
	strcat(bsend,"<a class='menu' href='switchon'>&#9832;<span class='showmenulabel'>SwitchOn</span></a>");
	strcat(bsend,"<a class='menu' href='heaton'>&#9832;<span class='showmenulabel'>HeatOn</span></a>");
	strcat(bsend,"<a class='menu' href='bheaton'>&#9832;<span class='showmenulabel'>Boil&HeatOn</span></a>");
	strcat(bsend,"<a class='menu active' href='lighton'>&#127752;<span class='showmenulabel'>NightLightOn</span></a>");
	strcat(bsend,"<a class='menu' href='setting'>&#128295;<span class='showmenulabel'>Setting</span></a>");
	strcat(bsend,"<a class='menu' href='restart'>&#128295;<span class='showmenulabel'>Reboot</span></a>");
	strcat(bsend,"<a class='menu' href='update'>&#128295;<span class='showmenulabel'>Load firmware</span></a></div>");
	strcat(bsend,"</header><table class='normal'><h3>Set NightLight Colours</h3><br/><body>");
	strcat(bsend,"<form method=\"POST\" action=\"/lightonok\"><input name=\"rlight\" type=\"number\" value=\"");
	itoa(RgbR,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"255\" size=\"3\"> NightLight Red</br><input name=\"glight\" type=\"number\" value=\"");
	itoa(RgbG,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"255\" size=\"3\"> NightLight Green</br><input name=\"blight\" type=\"number\" value=\"");
	itoa(RgbB,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"255\" size=\"3\"> NightLight Blue</br></br>");
	strcat(bsend,"<h3> Store values then press Ok. </h3><br/>");
	strcat(bsend,"<input type=SUBMIT value=\"Ok\"></form><form method=\"POST\" action=\"/setignore\">");
	strcat(bsend,"<input type=SUBMIT value=\"Cancel\"></form></body></html>");
        httpd_resp_sendstr(req, bsend);
    return ESP_OK;
}
static const httpd_uri_t plighton = {
	.uri       = "/lighton",
	.method    = HTTP_GET,
	.handler   = plighton_get_handler,
	.user_ctx  = NULL
};




static esp_err_t plightonok_get_handler(httpd_req_t *req)
{
	char buf1[256] = {0};
	char buf2[16] = {0};
	char buf3[16] = {0};
	int  ret;
	ret = httpd_req_recv(req,buf1,255);
	if ( ret > 0 ) {
//ESP_LOGI(AP_TAG, "Buf: '%s'", buf1);
/*

in buf1 after httpd_req_recv string like below
swfid=wifiname&swfpsw=wifipassword&smqsrv=192.168.1.10&smqid=esp&
smqpsw=esp&devnam=&rlight=255&glight=255&blight=255&chk2=2

*/
	strcpy(buf2,"rlight");
	parsuri(buf1,buf3,buf2,255,4);
	RgbR = atoi(buf3);
	strcpy(buf2,"glight");
	parsuri(buf1,buf3,buf2,255,4);
	RgbG = atoi(buf3);
	strcpy(buf2,"blight");
	parsuri(buf1,buf3,buf2,255,4);
	RgbB = atoi(buf3);
	}
//
	r4slppar = 0;
	r4slpcom = 5;
	r4slpres = 1;
	uint8_t timeout = 35; 	// 100*35 = 3.5 seconds
	while (--timeout && (r4slpres)) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	httpd_resp_set_status(req, "303 See Other");
	httpd_resp_set_hdr(req, "Location", "/");
	httpd_resp_send(req, NULL, 0);  // Response body can be empty
    return ESP_OK;
}
static const httpd_uri_t plightonok = {
	.uri       = "/lightonok",
	.method    = HTTP_POST,
	.handler   = plightonok_get_handler,
	.user_ctx  = NULL
};

/* HTTP GET setting handler */
static esp_err_t psetting_get_handler(httpd_req_t *req)
{
	char bsend[4500];
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
	strcat(bsend," for Redmond Kettle</h1>");
	strcat(bsend,"<div class='menubar'><a class='menu' href='.'>&#8962;<span class='showmenulabel'>Main</span></a>");
	strcat(bsend,"<a class='menu' href='switchoff'>&#10006;<span class='showmenulabel'>SwitchOff</span></a>");
	strcat(bsend,"<a class='menu' href='switchon'>&#9832;<span class='showmenulabel'>SwitchOn</span></a>");
	strcat(bsend,"<a class='menu' href='heaton'>&#9832;<span class='showmenulabel'>HeatOn</span></a>");
	strcat(bsend,"<a class='menu' href='bheaton'>&#9832;<span class='showmenulabel'>Boil&HeatOn</span></a>");
	strcat(bsend,"<a class='menu' href='lighton'>&#127752;<span class='showmenulabel'>NightLightOn</span></a>");
	strcat(bsend,"<a class='menu active' href='setting'>&#128295;<span class='showmenulabel'>Setting</span></a>");
	strcat(bsend,"<a class='menu' href='restart'>&#128295;<span class='showmenulabel'>Reboot</span></a>");
	strcat(bsend,"<a class='menu' href='update'>&#128295;<span class='showmenulabel'>Load firmware</span></a></div>");
	strcat(bsend,"</header><table class='normal'><h3>Wifi Setting</h3><br/><body>");
	strcat(bsend,"<form method=\"POST\" action=\"/setsave\"><input name=\"swfid\" value=\"");
	if (WIFI_SSID[0]) strcat(bsend,WIFI_SSID);
	strcat(bsend,"\" size=\"15\"> WIFI SSID</br><input name=\"swfpsw\" value=\"");
	if (WIFI_PASSWORD[0]) strcat(bsend,WIFI_PASSWORD);
	strcat(bsend,"\"size=\"31\"> WIFI Password</br><h3>MQTT Setting</h3><br/><input name=\"smqsrv\" value=\"");
	if (MQTT_SERVER[0]) strcat(bsend,MQTT_SERVER);
	strcat(bsend,"\"size=\"15\"> MQTT Server</br><input name=\"smqid\" value=\"");
	if (MQTT_USER[0]) strcat(bsend,MQTT_USER);
	strcat(bsend,"\"size=\"15\"> MQTT Login</br><input name=\"smqpsw\" value=\"");
	if (MQTT_PASSWORD[0]) strcat(bsend,MQTT_PASSWORD);
	strcat(bsend,"\"size=\"15\"> MQTT Password</br><h3>Device Setting</h3><br/><input name=\"sreqnam\" value=\"");
	if (REQ_NAME[0]) strcat(bsend,REQ_NAME);
	strcat(bsend,"\" size=\"15\"> Device search name</br>");
	strcat(bsend,"<input name=\"rlight\" type=\"number\" value=\"");
	itoa(RgbR,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"255\" size=\"3\"> NightLight Red</br><input name=\"glight\" type=\"number\" value=\"");
	itoa(RgbG,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"255\" size=\"3\"> NightLight Green</br><input name=\"blight\" type=\"number\" value=\"");
	itoa(RgbB,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"255\" size=\"3\"> NightLight Blue</br><input type=\"checkbox\" name=\"chk1\" value=\"1\"");
	if (DEV_CHK1) strcat(bsend,"checked");
	strcat(bsend," > Check for G200S, G211S, G240S</br><input type=\"checkbox\" name=\"chk2\" value=\"2\"");
	if (DEV_CHK2) strcat(bsend,"checked");
	strcat(bsend," > Check for G211S, G240S</br><h3>System Setting</h3><br/><input name=\"r4snum\" type=\"number\" value=\"");
	itoa(R4SNUM,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"0\" max=\"255\" size=\"3\"> r4sGate Number</br><input name=\"timzon\" type=\"number\" value=\"");
	uint8_t TmZn = TimeZone;
	if (TmZn >127) {
	TmZn = ~TmZn;
	TmZn++;
	strcat(bsend,"-");
	}
	itoa(TmZn,buff,10);
	strcat(bsend,buff);
	strcat(bsend,"\" min=\"-12\" max=\"12\" size=\"3\"> GMT Timezone</br><h3>Store setting then press SAVE. ESP32 r4sGate will restart</h3><br/>");
	strcat(bsend,"<input type=SUBMIT value=\"Save settings\"></form><form method=\"POST\" action=\"/setignore\">");
	strcat(bsend,"<input type=SUBMIT value=\"Cancel\"></form></body></html>");
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
	char buf1[512] = {0};
	char buf2[16] = {0};
	char buf3[16] = {0};
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
	ret = httpd_req_recv(req,buf1,255);
	if ( ret > 0 ) {
//ESP_LOGI(AP_TAG, "Buf: '%s'", buf1);
/*

in buf1 after httpd_req_recv string like below
swfid=wifiname&swfpsw=wifipassword&smqsrv=192.168.1.10&smqid=esp&
smqpsw=esp&devnam=&rlight=255&glight=255&blight=255&chk2=2

*/
	strcpy(buf2,"swfid");
	parsuri(buf1,WIFI_SSID,buf2,255,33);
	strcpy(buf2,"swfpsw");
	parsuri(buf1,WIFI_PASSWORD,buf2,255,65);
	strcpy(buf2,"smqsrv");
	parsuri(buf1,MQTT_SERVER,buf2,255,16);
	strcpy(buf2,"smqid");
	parsuri(buf1,MQTT_USER,buf2,255,16);
	strcpy(buf2,"smqpsw");
	parsuri(buf1,MQTT_PASSWORD,buf2,255,16);
	strcpy(buf2,"sreqnam");
	parsuri(buf1,REQ_NAME,buf2,255,16);
	strcpy(buf2,"rlight");
	parsuri(buf1,buf3,buf2,255,4);
	RgbR = atoi(buf3);
	strcpy(buf2,"glight");
	parsuri(buf1,buf3,buf2,255,4);
	RgbG = atoi(buf3);
	strcpy(buf2,"blight");
	parsuri(buf1,buf3,buf2,255,4);
	RgbB = atoi(buf3);
	buf3[0] = 0;
	strcpy(buf2,"chk1");
	parsuri(buf1,buf3,buf2,255,2);
	DEV_CHK1 = 0;
	if (buf3[0] == 0x31) DEV_CHK1 = 1;
	strcpy(buf2,"chk2");
	parsuri(buf1,buf3,buf2,255,2);
	DEV_CHK2 = 0;
	if (buf3[0] == 0x32) DEV_CHK2 = 1; 
	strcpy(buf2,"timzon");
	parsuri(buf1,buf3,buf2,255,4);
	TimeZone = atoi(buf3);
	strcpy(buf2,"r4snum");
	parsuri(buf1,buf3,buf2,255,4);
	R4SNUM = atoi(buf3);
// write nvs
	nvs_handle_t my_handle;
	ret = nvs_open("storage", NVS_READWRITE, &my_handle);
	if (ret == ESP_OK) {

	nvs_set_u8(my_handle,  "rlight", RgbR);
	nvs_set_u8(my_handle,  "glight", RgbG);
	nvs_set_u8(my_handle,  "blight", RgbB);
	nvs_set_u8(my_handle,  "chk1", DEV_CHK1);
	nvs_set_u8(my_handle,  "chk2", DEV_CHK2);
	nvs_set_u8(my_handle,  "timzon", TimeZone);
	nvs_set_u8(my_handle,  "r4snum", R4SNUM);
	nvs_set_str(my_handle, "swfid", WIFI_SSID);
	nvs_set_str(my_handle, "swfpsw", WIFI_PASSWORD);
	nvs_set_str(my_handle, "smqsrv", MQTT_SERVER);
	nvs_set_str(my_handle, "smqid", MQTT_USER);
	nvs_set_str(my_handle, "smqpsw", MQTT_PASSWORD);
	nvs_set_str(my_handle, "sreqnm", REQ_NAME);
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

//check "WebKitForm" string in received data
	otabufoffs = parsoff(otabuf,"WebKitForm", otabufsize);
	if (!otabufoffs) {
	ESP_LOGE(AP_TAG, "WebKitForm not found");
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
	ESP_LOGI(AP_TAG, "Total Write binary data length: %d", binary_file_length);
	
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
	config.stack_size = 12288;
	// Start the httpd server
	ESP_LOGI(AP_TAG, "Starting server on port: '%d'", config.server_port);
	ESP_LOGI(AP_TAG, "Max URI handlers: '%d'", config.max_uri_handlers);
	ESP_LOGI(AP_TAG, "Max Open Sessions: '%d'", config.max_open_sockets);
	ESP_LOGI(AP_TAG, "Max Header Length: '%d'", HTTPD_MAX_REQ_HDR_LEN);
	ESP_LOGI(AP_TAG, "Max URI Length: '%d'", HTTPD_MAX_URI_LEN);
	ESP_LOGI(AP_TAG, "Max Stack Size: '%d'", config.stack_size);

	if (httpd_start(&server, &config) == ESP_OK) {
// Set URI handlers
	ESP_LOGI(AP_TAG, "Registering URI handlers");
	httpd_register_uri_handler(server, &pmain);
	httpd_register_uri_handler(server, &pswitchoff);
	httpd_register_uri_handler(server, &pswitchon);
	httpd_register_uri_handler(server, &pheaton);
	httpd_register_uri_handler(server, &pheatonok);
	httpd_register_uri_handler(server, &pbheaton);
	httpd_register_uri_handler(server, &pbheatonok);
	httpd_register_uri_handler(server, &plighton);
	httpd_register_uri_handler(server, &plightonok);
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
	ESP_LOGI(AP_TAG,"Wifi disconnected. Restarting now...");
	fflush(stdout);
	esp_restart();
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
#ifdef USE_TFT
	tft_conn = tftinit();
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
	FND_NAME[0] = 0;
	REQ_NAME[0] = 0;
	DEV_NAME[0] = 0;
	MQTT_USER[0] = 0;
	MQTT_PASSWORD[0] = 0;
	MQTT_SERVER[0] = 0;
	WIFI_SSID[0] = 0;
	WIFI_PASSWORD[0] = 0;
	tBLEAddr[0] = 0;
// read nvs
	nvs_handle_t my_handle;
	ret = nvs_open("storage", NVS_READWRITE, &my_handle);
	if (ret == ESP_OK) {
	nvs_get_u8(my_handle, "rlight", &RgbR);
	nvs_get_u8(my_handle, "glight", &RgbG);
	nvs_get_u8(my_handle, "blight", &RgbB);
	PRgbR = ~RgbR;
	PRgbG = ~RgbG;
	PRgbB = ~RgbB;
	nvs_get_u8(my_handle, "chk1", &DEV_CHK1);
	nvs_get_u8(my_handle, "chk2", &DEV_CHK2);
	nvs_get_u8(my_handle, "timzon", &TimeZone);
	nvs_get_u8(my_handle, "r4snum", &R4SNUM);
	size_t nvsize = 32;
	nvs_get_str(my_handle,"swfid", WIFI_SSID,&nvsize);
	nvsize = 64;
	nvs_get_str(my_handle,"swfpsw", WIFI_PASSWORD,&nvsize);
	nvsize = 16;
	nvs_get_str(my_handle,"smqsrv", MQTT_SERVER,&nvsize);
	nvsize = 16;
	nvs_get_str(my_handle,"smqid", MQTT_USER,&nvsize);
	nvsize = 16;
	nvs_get_str(my_handle,"smqpsw", MQTT_PASSWORD,&nvsize);
	nvsize = 16;
	nvs_get_str(my_handle,"sreqnm", REQ_NAME,&nvsize);
// Close nvs
	nvs_close(my_handle);
	}
	if (!WIFI_SSID[0]) {
	strcpy(WIFI_SSID, "r4s");
	strcpy(WIFI_PASSWORD, "12345678");
	}
// fill basic parameters
	tBLEAddr[0] = 0;
	char tzbuff[8];
	strcpy(MQTT_BASE_TOPIC, "r4s");
	if (R4SNUM)  {
	itoa(R4SNUM,tzbuff,10);
	strcat(MQTT_BASE_TOPIC, tzbuff);
	}
//init bt
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
	esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(BLE_INPUT_BUFFSIZE);
	if (local_mtu_ret){
	ESP_LOGI(AP_TAG,"Set local  MTU failed, error code = %x\n", local_mtu_ret);
	}

//Initialize Wifi
	ESP_LOGI(AP_TAG,"Connecting to Wifi: '%s' with password '%s'", WIFI_SSID, WIFI_PASSWORD);
	wifi_init_sta();
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

//Initialize http server
	ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));
/* Start the server for the first time */
	server = start_webserver();
// mark as valid boot for prevent rollback after ota
	esp_ota_mark_app_valid_cancel_rollback();	
	ESP_LOGI(AP_TAG,"Init end free heap: %d\n", esp_get_free_heap_size());
	int floop = 1;
	int64_t t_before_us = 0;

//r4s state nonitoring and command execution loop
	while (floop) {
	if (r4slpcom == 1) { //off
	r4slpres = 1;
	m171sHeat(0);
	bprevHtemp = 255;
	bprevState = 255;
	m171sOff();
	r4slpcom =0;
	t_last_us = ~t_last_us;
	}
	if (r4slpcom == 2) { //boil
	r4slpres = 1;
	m171sHeat(0);
	bprevHtemp = 255;
	bprevState = 255;
	m171sBoil();
	r4slpcom =0;
	t_last_us = ~t_last_us;
	}
	if (r4slpcom == 3) { //heat
	r4slpres = 1;
	m171sOff();
	bprevHtemp = 255;
	bprevState = 255;
	m171sHeat(r4slppar);
	r4slpcom =0;
	t_last_us = ~t_last_us;
	}
	if (r4slpcom == 4) { //boil&heat
	r4slpres = 1;
	m171sOff();
	if (r4slppar == 0) m171sBoil();
	else m171sBoilAndHeat(r4slppar);
	bprevHtemp = 255;
	bprevState = 255;
	r4slpcom =0;
	t_last_us = ~t_last_us;
	}
	if (r4slpcom == 5) { //nightlight
	r4slpres = 1;
	if ((DEV_CHK1) || (DEV_CHK2)) {
	nl_settings[3]=RgbR;
	nl_settings[4]=RgbG;
	nl_settings[5]=RgbB;
	nl_settings[8]=RgbR;
	nl_settings[9]=RgbG;
	nl_settings[10]=RgbB;
	nl_settings[13]=RgbR;
	nl_settings[14]=RgbG;
	nl_settings[15]=RgbB;    
	m171s_NLOn();
	}
	r4slpcom =0;
	t_last_us = ~t_last_us;
	}
        vTaskDelay(200 / portTICK_PERIOD_MS);
	t_before_us = esp_timer_get_time();
// every 4s get kettle state
	if (t_before_us - t_last_us > 4000000) {
        MqState();
	r4slpres = 0;
#ifdef USE_TFT
	if (tft_conn) tfblestate();
#endif
	t_last_us = t_before_us;	
	}
// every 1s display time and date
	if ((tft_conn) && (t_before_us - t_clock_us > 1000000)) {
#ifdef USE_TFT
	tftclock();
#endif
	t_clock_us = t_before_us;	
	}

}
}
