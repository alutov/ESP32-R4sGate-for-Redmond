# ESP32 Ready4Sky (R4S) gateway for Redmond+ devices
> **[A script based on a PHP server on a local network with Internet access to display the weather on the gateway screen.](https://github.com/artt652/Weather-for-ESP32-R4S-gate).**<br>
>**In versions starting from 2022.06.03, the device topics in gateway number 0 have been changed from “r4s/#” to “r4s0/#”. In new versions, topics in "r4s/#" are used to track tags by multiple gateways. When upgrading from older versions, you need to enable BLE Monitor in the settings (the gateway deletes the contents of "r4s/#" only when the monitor is turned on), selecting Passive, Active or Auto, check the Hass Discovery and Delete Mqtt Topics items and save the settings. After the reboot, the gateway will delete unnecessary topics and create them again. Then, if necessary, BLE Monitor can be disabled. Then fix the devices in automations, scripts, etc.**

#### Current version is 2024.02.05 для [ESP32](https://github.com/alutov/ESP32-R4sGate-for-Redmond/raw/master/build/r4sGate.bin) и [ESP32C3](https://github.com/alutov/ESP32-R4sGate-for-Redmond/raw/master/C3/build/r4sGate.bin).
* 2024.02.05. At startup, the gateway scans all WIFI APs with the required parameters and selects the AP with the best level. Relevant for mesh systems.
* 2024.02.04. Added selection of WIFI 802.11b/g/n mode in settings.
* 2024.02.02. Corrections by Delonghi. In Auto mode, the image buffer size is selected separately for each URL.
* 2024.01.30. Added the ability to automatically increase the buffer size for loading images within 10-65 kilobytes (Adjust option). The gateway operates more stable at wifi levels below -73dBm.
* 2024.01.25. Corrected Delonghi statistics and availability of BLE devices/tags in Home Assistant. Currently Delonghi in HA:
![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/ecam650.jpg)  
* 2024.01.22. Added mute option as an alternative to beep. The availability of devices in Home Assistant has been fixed. If the gateway is unavailable, all devices connected to it become unavailable. If devices disappear from the BLE monitor, the data from them becomes inaccessible. Added statistics to Delonghi coffee machines.
* 2024.01.14. The project was built using esp-idf version 5.2-beta2. Added Russian program names to the multicooker menu. Added support for the Redmond SkyHeat RCH-4560S heater. Added support for LYWSDCGQ thermometers to BLE Gateway Monitor. You can display up to 4 images in turn on the gateway screen using 4 urls.

## 1. Opportunities
&emsp; The ESP32 r4sGate gateway in a minimal configuration (only ESP32 or ESP32C3 with a 3.3v power supply) allows you to connect BLE-compatible Redmond devices, Xiaomi MiKettle kettles and some other devices to the smart home system (Home Assistant, OpenHab, ioBroker, MajorDoMo, etc. ) via MQTT protocol. Initially, the project was only for Redmond, now other devices are being added. Hence the plus in the name of the project.
<details>
<summary>Why MQTT...</summary>

* This is in fact a standard protocol for smart home systems. Another thing is how it is implemented. From the built-in MQTT server in the ioBroker system, where everything that happens in MQTT is immediately displayed in the admin interface, to the external broker in Home Assistant, where sometimes you have to use third-party utilities to configure it. In the latter, however, MQTT Discovery greatly simplifies the integration of devices into the system.
* In MQTT, devices can exchange data with each other, and not just with the smart home server. The gateway can be configured to display the readings of the main sensors in the house on the screen. In response to the argument that some devices can uncontrollably rewrite the data of others, I note that normal brokers have a shared access system (ACL). Although I don’t have an answer to the question, why even introduce devices into the system that are not trusted.
* In MQTT, devices can exchange information without knowing anything about each other except the topic where they should meet. This is used to determine the maximum signal strength from a tag/beacon among multiple gateways, and the gateway receiving the strongest signal.
* MQTT support is included in the esp-idf development environment and does not require third-party libraries. After unsuccessful attempts to finalize a very good [olehs project](https://github.com/olehs/r4sGate) on arduino, I became a supporter of pure esp-idf.
</details>     

#### List of supported devices:

**Electric kettles:**
* Redmond SkyKettle **RK-M170S**
* Redmond SkyKettle **RK-M173S** / RK-M171S / RTP-M810S
* Redmond SkyKettle **RK-G200S** / RK-G204S / RK-G210S / RK-G211S / RK-G212S / RK-G214S / RK-M216S / RK-M139S
* Redmond SkyKettle **RK-G240S** / RK-G204S / RK-G210S / RK-G211S / RK-G212S / RK-G214S / RK-M216S / RK-M139S
* Xiaomi MiKettle **YM-K1501(Int)** - ProductId 275
* Xiaomi MiKettle **YM-K1501(HK)** - ProductId 131
* Xiaomi MiKettle **V-SK152(Int)** - ProductId 1116
<details>
<summary>Read more about Xiaomi MiKettle...</summary>
&emsp;Xiaomi MiKettle can only be controlled from the **keep warm** mode. In this mode, the kettle maintains a minimum temperature of 40°C set by the gateway with a hysteresis of approximately 4°C, that is, at 36°C the heating is turned on, and at 44°C it is turned off. You can turn boiling on and off (state = ON/OFF), set the heating temperature (heat_temp = 40...95). You can switch the kettle to Idle mode (heat_temp = 0). The last command is executed with a delay. After executing the command, further control of the kettle is unavailable. In contrast to turning it off with the **warm** sensor on the kettle, when you turn it off and on again, the kettle returns to the **keep warm** mode. Perhaps this is a feature of a specific version of MCU 6.2.1.9. For now I left it like that and turned on the kettle through the Redmond outlet. If you turn it off and on again, the kettle goes into heating mode. The gateway installs all the necessary parameters of the kettle itself, and the native application is useful for updating the firmware. The heating time is set to 12 hours (720 minutes), after 256 minutes the gateway resets the counter by briefly turning the boiling on and off. And still control is limited. The main problem is that when you turn on boiling with the **boil** sensor on the kettle, the **keep warm** mode is turned off and you can return it only with the **warm** button on the kettle. For the same reason, I postponed work on Mikettle Pro for now.</details>

**Multicookers**

* Redmond SkyCooker **RMC-M224S**
* Redmond SkyCooker **RMC-M800S**
* Redmond SkyCooker **RMC-M903S**
* Redmond SkyCooker **RMC-M92S**
* Redmond SkyCooker **RMC-M961S**
     
**Coffee makers**

* Redmond SkyCoffee **RCM-M1519S**

**Coffee machines**

* Delonghi **ECAM650.75** (Possibly other models 😉 Primadonna Elite series)

**Sockets**

* Redmond SkyPort **RSP-103S** / RSP-100S

**Electric convectors**

* Redmond SkyHeat **RCH-7001S** / RCH-7002S / RCH-7003S
* Redmond SkyHeat **RCH-4560S**

**Climate stations**

* Redmond SkyClimate **RSC-51S**

**Humidifiers**

* Redmond SkyDew **RHF-3310S**

**Sensors**

* Redmond SkySmoke **RSS-61S** - smoke sensor
* Redmond SkyOpen **RSO-31**  - door open sensor
* Hilink **HLK-LD2410B** - motion and presence sensor

**Irrigation controllers**
     
* Galcon **GL9001A** / Green Apple GATB010-03

**Curtain/blind drivers**
     
* **AM43 blinds** A-OK and similar)

&emsp;The gateway supports 5 simultaneous BLE connections. Device management is also possible from the gateway web interface. The web interface is [simply protected with a password from Raerten](https://github.com/alutov/ESP32-R4sGate-for-Redmond/pull/67). To do this, the string in the form login:password must be encrypted in Base 64 and then entered into the Basic Auth field in the settings. The password string is output to the log when the gateway starts.<br/>
&emsp;Home Assistant Mqtt Discovery is supported. To enable it, you need to check **Hass Discovery** in the settings. It is possible to delete all data created by the gateway in Mqtt and devices in Home Assistant. To do this, select the **Delete Mqtt topics** option in the **Setting** tab and then click **Save setting** . After rebooting the gateway, only devices connected to the gateway will be recreated. Recommended when connecting the gateway for the first time and reconfiguring by removing devices.Поддерживается Home Assistant Mqtt Discovery.<br/> 

<details>
<summary>A simple option for integrating a kettle from Home Assistant into a Yandex smart home is to use the climate entity.</summary>

&emsp; And call it the word **teapot**. All commands of the thermostat device will be available. For example, the command **turn on the kettle** (auto mode, turning on boiling or boiling followed by heating, if heating was turned on before), **turn off the kettle** (turns everything off), **set the kettle temperature to 40°** (if not 0° and not 100° turns on heating, mode heat, if 0° - turns it off, if the temperature is 100°, boiling is turned on, auto mode) or **turn on the heating** (turns on heating at the last set temperature). And finally, the command **turn on cooling** - turns on the backlight, cool mode. Not very pretty, but that's how it is. You can ask what’s wrong with the kettle - the kettle’s thermostat is turned off and what the temperature of the kettle is - it will tell the current temperature.<br>
</details>     

<details>
<summary>It supports calculating the amount of water in the kettle when heating in the range of 65-85°C and more than 3°C from the moment the kettle is turned on.</summary>

&emsp;No modifications to the kettle are required. Calculated based on the energy expended and the temperature difference. The calculated value is reset when the kettle is removed from the stand. The option only works in dummies with statistics. The efficiency of the kettle was initially assumed to be 80%. The accuracy is so-so, I get about ~0.2 liters. To improve accuracy, a mode for adjusting the efficiency value is provided. To do this, you need to pour 1 liter of water into the kettle and select **Boil 1l on** in the web interface . When the mode works, you need to enter the settings mode. The new value will be displayed immediately after the device type. You can write a new value to nvram using the Save setting command. It seems to me that it is unrealistic to obtain greater accuracy, since the efficiency of the kettle changes over time, for example, with the appearance of scale, and, what’s worse, the energy expended is not measured, but is simply calculated by the kettle’s processor based on the rated power of the heater and its operating time. The deviation of the supply voltage during operation from the value during calibration introduces a noticeable error; the dependence there is quadratic. When I boil 1.7 liters of water with a RK-M216S kettle at a voltage at the entrance to the house of 200-204V, the result is 1.8 liters, at a voltage of 210-214V it comes out to 1.6 liters. During calibration there was obviously something in between.<br>
</details>     

&emsp;The BLE Gateway Monitor can be used to monitor up to 24 BLE beacon devices with a static MAC address. The presence/absence of a tag (beacon) and rssi is displayed. Supported are BLE beacons of the Home Assistant application on smartphones (binding by uuid), LYWSD02 watch with thermometer, LYWSD03MMC Xiaomi Mijia 2 thermometers with original firmware, firmware from atc1441in [custom mode}(https://github.com/atc1441/ATC_MiThermometer) and firmware from [pvvx in custom mode](https://github.com/pvvx/ATC_MiThermometer), Xiaomi Mi Scale, Qingping Air Monitor Lite (CGDN1), Elehant counters, as well as Samsung Smart Tag.<br>
&emsp;There are 10 I/O ports, 5 of them can be used to control external devices (Out mode) and read their status (In mode). Three ports can be configured as buttons for turning on/off connected BLE devices (Sw mode, in which case the state of the buttons in mqtt is not displayed), the fourth port can be configured as a button for updating the image from the camera. When configured in input mode, pullup is enabled if possible (pin number less than 34). Another 2 ports are used by the I2C bus, and each of the 3 remaining ports can be used as a pulse width modulated (PWM) output, or as an input for connecting either one DS18B20 sensor with direct power supply, or one DHT22/AM2302 sensor (7 and port 8). Reading procedures are simplified, the checksum is not read or verified, and data is rounded to one decimal place. If the gateway is equipped with an audio emitter, then by connecting the PWM output to it (in m5stack basic this is gpio 25), you can output an audio signal. By changing the duty cycle of the pulses, you can adjust the volume. The frequency is fixed and equal to 3.136 kHz. The I2C bus supports sensors SHT3x/SHT4X(addresses 0x44, 0x45), AHT20(0x38), HTU21(0x40), BMP280/BME280/680/688(0x76, 0x77, 688 not yet tested), SGP30(0x58), SGP4x(0x59 ), SCD4x(0x62), as well as RTC DS3231(0x68) and battery controller IP5306(0x75). It is possible to save the SGP30 calibration data in NVRAM and restore it when the gateway starts. To do this, check the AQ base item in the settings. The [Sensirion library](https://github.com/Sensirion/gas-index-algorithm) is used to calculate VOC in SGP4x. The clock is used to store the date and time from the NTP server, the temperature sensor is output to Mqtt. The IP5306 controller is installed in m5stack and ttgo-t4 (SCL 22, SDA 21), allows you to determine the battery level in 25% increments and its mode (Discarging / Charging / Charged). When on battery power, the screen brightness decreases by 16 times. The gateway also supports the AXP192 power controller and PCF8563 RTC, allowing it to run on the M5Stack Tough, and also supports the ADC of the HX711 load cells. The measurement result from the HX711 can be displayed either in kilograms or as a percentage, depending on the calibration. HX711 is polled at intervals of 4 seconds, other sensors at intervals of 12 seconds. The gateway allows hot plugging of all sensors. Sensors 18B20 and DHT22 appear in Mqtt and Home Assistant immediately after the gateway starts, even if they are not connected, and I2C sensors as they are detected on the bus within 2 polling cycles (24 seconds).
<details>
     <summary>There is also support for IR transmitter (IR Tx, port 6).</summary>

&emsp;Supported protocols are **NEC** (8 and 16 bit address) **RC5** (to work in RC5ext mode you need to invert 6 bits of the command), **RC6** , **Samsung** , Sony **SIRC** (12, 15 and 20 bits), **Panasonic**. You can control it both from the Home Assistant interface and individual topics of address, command and protocol, or by directly writing to the **r4sx/ir6code** topic (where **x** is the gateway number) a string of 8 hex characters 0-9,af, for example, **090a1c3d** , where **09** is protocol(01-nec, 02-necx16, 03-rc5, 04-rc6, 05-samsung, 06-sircx12, 07-sircx15, 08-sircx20, 09-panasonic), **0a1c** - address , **3d** command.
What was tested (the power-on command was of interest):<br>
**NEC:** pioneer vsx-830, power: addr 165, cmd 28, code 0100a51c<br>
**NECx16:** lg dvd dks-2000h, power: addr 11565, cmd 48, code 022d2d30<br> 
**RC6:** philips 40pfs6609, power: addr 0, cmd 12, code 0400000c<br>
**SAMSUNG:** ue32n5300, power: addr 7, cmd 2, code 05000702<br>
**SIRCx12:** sony cmt-sx7, power: addr:16, cmd: 21, code 06001015<br>
**SIRCx20:** sony ubp-x800 power: addr 7258, cmd 21, code 081c5a15<br>
**PANASONIC:** sa-pm20 power: addr 2588, cmd 61, code 090a1c3d<br>
**PANASONIC:** dmp-ub900 power: addr 2816, cmd 61, code 090b003d<br>

I checked everything on the Atom lite, it has IR LED on 12 gpio. RC5 and SIRCx15 have not yet been tested.<br>
</details>

&emsp;In order to expand the capabilities of the gateway, it is possible to connect a 320 * 240 TFT screen on ili9341, ili9342 and ST7789 chips. The screen displays the current time, date, as well as temperature, voltage and current in the house (not a problem when powered by a generator), status (blue - off, red - on) and temperature at the boiler outlet, temperature and humidity outside. Everything comes from Mqtt. Next to the date, the status of BLE devices is displayed in color, 1 ... 5 - from the first to the fifth. Gray - not connected or not defined, blue - off, red - on, yellow - heating, white - program installed. More detailed status and some parameters of connected BLE devices are displayed in turn in the bottom line. It is possible to periodically or on request display a picture on the screen in jpeg format, for example, from a camera. Pictures with a horizontal resolution higher than 320 are displayed at a scale of 1:2. The buffer size for loading images can be changed within 20-65 kilobytes. Screen brightness can be changed by Mqtt. You can also display the weather in text form from the site wttr.in or just text by writing it in the Mqtt topic r4sx/jpg_url. The result was something similar to a clock with a thermometer. It is enough to look at the screen to make sure that everything is in order in the house or on the street today.<br>

## 2. Accessories

![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/myparts.jpg)
Picture 1. Components for assembling the gateway.

&emsp; If the goal is to launch the gateway at minimal cost, you will have to buy spare parts and then assemble the gateway from them. I used an [ESP32 WROOM ESP-32 4 MB with a built-in antenna (bottom left) or an ESP32 WROOM ESP-32U 4 MB with an external one (to the right of the first)](https://aliexpress.ru/item/32961594602.html?item_id=32961594602&sku_id=66888778667&spm=a2g2w.productlist.0.0.4f835c61Fvd1gD) . Issue price $2.5. Then I soldered the microcircuit onto [an adapter board ($0.3)](https://www.aliexpress.com/item/32763489487.html?spm=a2g2w.productlist.i0.2.48d33c75KbnjpB&sku_id=62208988599) and then onto a breadboard. ESP32C3 is also suitable, I have ESP32C3-12F. Due to hardware limitations of this chip, the gateway uses port 8 only as a pulse width modulation (PWM) output. There is approximately 28 kilobytes more free RAM. And even with the screen connected, the ESP32C3-12F still has 6 free gpio. Power supply for 3.3 Hi-Link ($2-$4). [I bought them for $1.65](https://aliexpress.ru/item/32953853140.html?spm=a2g39.orderlist.0.0.32964aa6PePEbg&_ga=2.238912000.104655408.1636114275-428746708.1615828563&_gac=1.87036010.1634012869.Cj0KCQjwwY-LBhD6ARIsACvT72Na1GBQp7leEJDlxPCd0jTye8sF-GiknWzlo4hKElMNbtmI4DYpB_8aAktOEALw_wcB). **You can avoid soldering** if you use [esp32-wroom-devkit (bottom center, $14)](ttps://aliexpress.ru/item/4000127837743.html?sku_id=10000000372418546&spm=a2g0s.9042311.0.0.274233edNcajyj). True, this board is very redundant for the project; you can get a [simpler one for $3.54](https://aliexpress.ru/item/32928267626.html?item_id=32928267626&sku_id=12000016847177755&spm=a2g2w.productlist.0.0.430c65c8Kf9vOT). In it, esp32 comes along with a board on which there are also converters from 5V to 3.3V, USB-RS232 and a standard mini-USB connector. Through it you can power the esp32 using a five-volt charger from a smartphone, and program it directly from the computer without any adapters. And on the right in the photo is a 3.2" [320 * 240 TFT screen ($18)](https://aliexpress.ru/item/32911859963.html?spm=a2g0s.9042311.0.0.274233edzZnjSp) , which I used in the gateway. You can also use compatible ready-made devices both with a screen ( **TTGO T-Watcher BTC Ticker** , **M5Stack BASIC** , **M5Stack Tough** ) and without ( **m5atom lite**).

## 3. Gateway setup

&emsp;To run the gateway you need to program the ESP32. You can use the [flash_download_tools program](https://www.espressif.com/en/support/download/other-tools). The file [**fr4sGate.bin**](https://github.com/alutov/ESP32-R4sGate-for-Redmond/raw/master/build/fr4sGate.bin) in the build folder is an already assembled binary for esp32 @160MHz with 4 MB memory, DIO bootloader and is flashed with one file from address 0x0000 in **DIO** mode . If the DIO bootloader does not start, you can use the [**fqr4sGate.bin**](https://github.com/alutov/ESP32-R4sGate-for-Redmond/raw/master/build/fqr4sGate.bin) file with the QIO bootloader and program it in **QIO mode**. As I understand it, most esp32 can be programmed in any mode, but there were cases that the gate only worked when it was flashed with the fqr4sGate.bin file in QIO mode . The [**r4sGate.bin**](https://github.com/alutov/ESP32-R4sGate-for-Redmond/raw/master/build/r4sGate.bin) file is used to update the firmware via the web interface. ESP32C3 programming files in folder C3.<br>
&emsp;Then you need to create an access point on your smartphone with ssid **r4s** and password **12345678** , wait until the esp32 connects to it, find the connected r4s0Gate device and its IP in the access point parameters, enter this address in the web browser and then set it in the web in the Setting tab other parameters. After which the access point is no longer needed. Esp32 will only attempt to connect to the r4s network if the main network is unavailable, for example if the password is incorrect. If it cannot connect to the guest network, the esp32 reboots. The option with a guest network, in contrast to the generally accepted practice of launching an access point on esp32, is, in my opinion, more convenient, since if for some reason the Wi-Fi router fails (and it can be dedicated only for iot devices), the rest of the Wi-Fi does not get clogged by floating esp32.<br>
&emsp;Next, you need to enter the name or MAC address of the Redmond device and bind it to the gateway. The search for devices starts only when there is at least one identified but not connected device, or the BLE monitor is active. If the name of the device is not known exactly (and Redmonds do not always light up via BLE as a one-to-one model), then to start scanning you need to enter any name in the **Name** field in the settings, and then replace it with the one found during scanning and select the closest device type in the settings (field **TYPE** , for example, for teapots from RK-G(M)200S to RK-G(M)240S the protocol is the same, you can select both RK-G200S and RK-G240S). It should be taken into account that not all devices transmit the name during passive scanning (for example, Xiaomi Mikettle, AM43 Blinds). In any case, it is better to enter the MAC address in the name field, either with or without colons. You can find and copy the address **BLE Last found name/address** on the main page or on the BLE monitor page. Next, to bind, you need to press and hold the button **+** or **power* on a kettle or a **timer** on a multicooker until the device enters the binding mode and after a while connects to the gateway. AM43 blinds also require entering a PIN code (Passkey) for connection.<br>
&emsp;It is possible to connect several gateways to one MQTT server. To do this, you need to install your own r4sGate Number in each gateway. Gateway number 0 will write to topic r4s0/devaddr/..., gateway number 1 - r4s1/devaddr/..., etc. You just need to take into account that the authorization request when binding depends on the gateway number and the connection number in the gateway. This allows you to link 2 identical kettles or multicookers to 2 different gateways or to 2 different connections within the same gateway. If there are two gateways with the same parameters working nearby, connected to different smart home systems (for example, a neighbor behind the wall), to exclude the possibility of connecting a device to the neighbor’s gateway, you can use the option to authorize devices using the gateway’s MAC address by selecting **Use MAC in BLE in the settings Authentication**. Then reset all bindings on the devices and then bind them to the gateway again.<br>
&emsp;To connect to the Mqtt broker, you need to enter its address and port, as well as login and password. If the gateway works with Home Assistant paired with a mosquitto broker, you should use the **Hass Discovery** option. Before using it, I recommend deleting all topics with r4s in the Mqtt broker, for which select **Delete Mqtt topics** in the settings.<br>

#### Web interface

Devices can also be managed in the gateway web interface. Examples of the main page and settings page are below in pictures 6 and 7.

![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/myweb.jpg) 
Picture 6. Home page.

![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/myweb1.jpg) 
Picture 7. Settings page.
 
## 4. BLE Monitor
&emsp;The monitor allows you to track tags (beacons) with static MAC addresses. The presence/absence of a tag and rssi is displayed.<br>
**Additionally supported:**
* BLE beacons of the Home Assistant application on smartphones (binding by uuid, timeout for me is 60 seconds)
* Xiaomi LYWSD03MMC thermometers with original firmware, firmware from [atc1441 in custom mode](https://github.com/atc1441/ATC_MiThermometer) and firmware [from pvvx in custom and Mija mode](https://github.com/pvvx/ATC_MiThermometer). Keys for LYWSD03MMC original version can be obtained from the [Xiaomi cloud](https://github.com/PiotrMachowski/Xiaomi-cloud-tokens-extractor?tab=readme-ov-file). (400 seconds)
* Xiaomi Thermometers LYWSDCGQ (400 seconds)
* Xiaomi LYWSD02 thermometer watch (400 seconds)
* Xiaomi Mi Scale (400 seconds)
* Qingping Air Monitor Lite(CGDN1) (400 seconds)
* Elehant water and gas meters (400 seconds)
* Samsung Smart Tag (120 seconds).
     
&emsp;To activate the monitor, in the **Setting** tab , set the **BLE Monitoring** option to **Active** or **Passive** for active or passive scanning and click **Save setting**. An active scanner provides more information, but uses more power on the devices being scanned. Passive mode is recommended for tag scanning. It should be taken into account that in **Auto** mode , when searching for devices before connecting, the scanner always works in active mode, and then switches to passive mode.<br> 
&emsp;After installing the option, the **BLE Monitor** tab will appear in the menu , by opening which you can see the found devices. The **Gap** field displays the time interval between the last two received packets, and the **Last** field displays the time since the last packet arrived. The default timeout (if the **Timeout** field is empty) is 300 seconds, after which the device is considered lost and its data is deleted from the table. This line can later be overwritten with data from another device. **To output data to Mqtt, you need to enter a non-zero value in the Timeout field and confirm the entry by clicking Ok**. All values ​​will be saved in non-volatile memory, and Mqtt Discovery will transfer everything to Home Assistant. Although scanning is ongoing, when setting the **Timeout** value , you need to take into account that maintaining connections also requires time, during which packets may be missed. Unnecessary data in Mqtt and Home Assistant can be deleted by selecting the **Delete Mqtt topics** option in the **Setting** tab when the BLE monitor is turned on and clicking **Save setting**.<br> 
&emsp;Samsung Smart Tags that are not linked to a SmartThings account are not suitable for tracking, as they turn off after a few minutes. The advertising message associated with the Smart Tag account contains a static UUID 0xFD5A, a dynamic MAC address and an encrypted identifier, due to the presence of the RND byte in it, it changes along with the MAC address. The remaining fields (status, advertising message counter, region, battery status) are not unique. The advertising message also contains a digital signature. Standard BLE trackers, as far as I know, are able to recognize the presence of these tags by UUID, but are not able to uniquely identify each tag if there is more than one. The gateway uses verification of the digital signature of the advertising message to identify these tags (beacons), which requires entering a key. After entering, the key is checked and, if everything is normal, you need to enter the timeout value. Only after entering the timeout and pressing **Ok**, the key and timeout are stored in NVS (Non-volatile storage - non-volatile memory).<br>
<details>
<summary>How to get the key...</summary>

&emsp;The Signing Key is an ASCII string of 64 characters (that's 32 bytes in hexadecimal, 16 bytes of the AES128CBC encryption key, and 16 bytes of the initial vector). Valid from the moment you link the Smart Tag to your SmartThings account until you unlink the account or return the device to factory settings. Generated by both the Smart Tag and the SmartThings server, it is not transmitted over the communication channel. The first 16 bytes are used by the gateway as a device identifier in the smart home system. To generate a key, you need to record the Bluetooth HCI log at the time you link the Smart Tag to your SmartThings account. If the tag is already linked, it must be removed from the account before recording the log. To link to your account, you need a Galaxy device with Android version no lower than 8.0. I used Galaxy S7. First of all, you need to enable developer mode. Open **Settings > Phone information > Software information** and press the build number 8 (I think) times. You may need to enter your phone PIN code. The **Developer Options** menu should appear in Settings . Go to the menu and turn on **Bluetooth Tracking Log**. Next, just in case, I turned Bluetooth off and on, then rebooted the phone. Go to the SmartThings application and add the Smart Tag device . Then again go to **Settings > Developer Options** and select **Create error report > Interactive report**. After some time, you will receive a notification about the created report. Next, you need to save it on your Windows computer. I chose to save it in the telegram application in the **Favorites** folder , and then saved the archive in telegram on my computer. Then you need to extract the files btsnoop_hci.log and btsnoop_hci.log.last from the archive (folder FS/data/log/bt/ ) . One of these files should contain a binding log. Next, load the archive with the [stsk console utility](https://github.com/alutov/ESP32-R4sGate-for-Redmond/raw/master/utils/stsk.zip) into the folder with these files and extract the program from the archive. Just in case, I’ll note that there are no viruses in the archive, and the size of the program is 28160 bytes. Open the Windows command line, go to the folder with the files and type **stsk btsnoop_hci.log** and **stsk btsnoop_hci.log.last**. The program will display the found Smart Tags and generate keys for them. The last key will be copied to the windows clipboard:<br>

![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/stsk.jpg) 
Pcture 8. Program stsk.

  The program does not process data packets broken into parts; accordingly, if it encounters such a packet, it does not find anything in the log or generates an error. I've encountered this only once so far on s21. Perhaps this is just a failure, because wireshark also did not quite correctly restore the failed packet. And I have not yet found an algorithm for identifying and assembling such data packets. In this case, you need to repeat the entire procedure again.<br>
</details>

![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/blemon1.jpg) 
Picture 9. BLE Monitor page.

 Added determination of the best signal from a tag (beacon) among several gateways. The algorithm works like this. Each gateway monitors the **r4s/DevId/rssi topic**. If he has a signal from a tag (beacon) with a high level, he writes his level to this topic, as well as his number to the **r4s/DevId/gtnum topic**. After which the gateway periodically, once every 6 seconds, maintains its signal level in the topic, that is, it becomes the leader. The remaining gateways check the level and whether there is an update. If any gateway detects no level update for more than 30 seconds, or its level is greater, it becomes the master. The best level and gateway number can be seen in the second RSSI line on the BLE monitor page. The entities of each device also have the best level and gateway number:<br>
![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/blemon2.jpg)  
 Picture 10. Tag entities in Home Assistant.
 
## 5. Screen support

&emsp;IIn the first version of the gateway, there was a reserve of both operational (~100 kByte) and programmable (~400 kByte) free memory, which made it possible to expand the capabilities of the firmware, in particular, to add screen support. In addition, I already had an assembled esp32 with a [3.2" 320x240 screen on an ili9341 chip](https://www.aliexpress.com/item/32911859963.html?spm=a2g0s.9042311.0.0.274233edzZnjSp) , working with firmware from the wifi-iot website . It is also possible to use ready-made devices on ili9341, ili9342 or ST7789 chips for the gateway. In the gateway I used only the necessary procedures from [Bodmer](https://github.com/Bodmer/TFT_eSPI), not very well adapted, but as it is for esp-iot. Pins for connecting the default screen: MOSI-23, MISO-25, CLK-19, CS-16, DC-17, RST-18, LED-21 . Pins can be reassigned in the settings. If PWR, RST, LED are set to 0, then the gateway will not use these pins. There is also an option to rotate the screen 180°, as well as the ability to adjust the display brightness using Mqtt, using the **r4s/screen** topic. The program checks presence of a screen on the SPI bus at startup. It is possible to display images on the screen in jpeg format. To do this, you need to specify the url of the image. My camera has the following url: **http://192.168.1.7/auto.jpg?usr=admin&pwd=admin**. The image is loaded into a 20-65 kilobyte buffer in RAM.The update time and buffer size can be set in the settings. It is possible to download images via https. Certificate verification is disabled. It is possible to control the parameters for loading up to 4 images via Mqtt, using the topics **r4sx/jpg_url1...r4sx/jpg_url4** and **r4sx/jpg_time**. To clear the url of an image, you need to enter the **#** symbol in the corresponding topic. If these topics are not registered in Mqtt, and also after saving the settings, these parameters are copied from the settings in Mqtt. Setting the refresh interval to zero returns the cat to the screen. The length of the link buffer is currently 384 bytes. Added loading and display of weather in text form from the website wttr.in. In principle, this can be done by any site that provides text and allows formatting. If the link does not contain the lines **http://** or **https://** , then the gateway considers this message to be plain text and displays it on the screen. Available in 2 fonts and 10 color options. Control characters: \ \ or \n - line feed, \F - 26 pixel font and line feed, \f - 16 pixel font and line feed, \0 ... \9 - colors. The Cyrillic alphabet is supported, I checked it, however, only from mosquitto. It actually supports Unicode, like other brokers, I don’t know. An example of displaying an image on the screen in picture 11.<br>

![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/mytft3.jpg)
Picture 11. Image.<br><br>

&emsp;Picture 12 shows an example of displaying Mqtt weather from the website wttr.in:
<td>https://wttr.in/Донецк?format=\F\6+%25l%20\\\4Темп:+\0%25t(%25f)\\\4Давл:\0+%25P\\\4Влажн:\0+%25h\\\6+%25c+%25w+UV:+%25u\f\4Восход:\0+%25D+\4Закат:\0+%25d</td><br> 

![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/mytft10.jpg)
Picture 12. Weather.<br><br>

&emsp;Picture 13 shows an example of displaying a Mqtt string (the degree symbol can be displayed on the screen using a back apostrophe):
<td>\F\0` English \1color \2text\3 example\n\4Русский \5цветной \6текст\n\7text1 \8text2 \9text3\f\0` English \1color \2text\3 example\n\4Русский \5цветной \6текст\n\7text1 \8text2 \9text3</td><br>    
  
![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/mytft9.jpg)
Picture 13. Text.<br><br>

&emsp;It is worth noting that the TFT board itself affects the distribution of both WiFi and BLE. And even if the esp32 antenna peeks out from under the screen, the sensitivity of such a sandwich is noticeably less than the usual esp32. I recommend using the esp32 version with an external antenna with the screen. In my gateway with a screen, replacing the esp32 with a version with a connector and installing an external antenna gave an increase in WIFI and BLE levels by about 15-20dBm.<br>
&emsp;If the screen is not needed, then after programming and setting up the esp32, you need to connect it to a power source and hide it somewhere in the kitchen.<br>

## 6. Compatible devices
&emsp;If you want to launch a gateway as quickly as possible, without soldering, and even with a decent case, you should take a closer look at compatible devices. They just need to be reprogrammed. Only the devices I have tested are listed below. The [flash_download_tools program](https://www.espressif.com/en/support/download/other-tools) was used for flashing the firmware .

#### [TTGO T-Watcher](http://www.lilygo.cn/prod_view.aspx?TypeId=50033&Id=1160) (LILYGO® TTGO T4 in case).<br>
![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/mytft4.jpg)
<br>Picture 14. TTGO T-Watcher.<br><br>
&emsp;I checked the functionality of the gateway on TTGO T4 version 1.3. It is flashed via the built-in USB connector; before flashing the device, you need to connect pins 6 and 7 (gpio0 and gnd) in the bottom row of the connector (picture 15). It is possible to flash the firmware without installing jumpers, depending on the program. Screen settings for version 1.3: 12-MISO, 23-MOSI, 18-CLK, 27-CS, 32-DC, 5-RES, 4-LED, 0-PWR, and for version 1.2: 12-MISO, 23-MOSI , 18-CLK, 27-CS, 26-DC, 5-RES, 4-LED, 0-PWR. In version 1.2 there is no control to turn the screen on and off. Buttons from top to bottom 38-Port1, 37-Port2, 39-Port3. I2C bus: SCL-22, SDA-21.<br>
![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/mytft5.jpg)
Picture 15. Connect pins 6 and 7 of the connector before flashing the TTGO T-Watcher firmware.<br><br>

#### [M5Stack BASIC Kit](https://docs.m5stack.com/en/core/basic)<br>
![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/mytft6.jpg)
<br>Picture 16. M5Stack BASIC Kit.<br><br>
&emsp;As I understand it, old versions of M5Stack Basic came with a screen based on ili9341, and the old version of the gateway also worked on these versions. Screen settings for this version: 19-MISO, 23-MOSI, 18-CLK, 14-CS, 27-DC, 33-RES, 32-LED, 0-PWR. New versions already come with a screen based on ili9342. Starting with version 2021.10.29, screen support for ili9342 has been added. I tested the functionality of the gateway on the new version of the M5Stack BASIC Kit. It is flashed via the built-in USB connector; before flashing the device, you need to connect the last contact in the top row and 4 in the bottom row (gnd and gpio0) of the connector (picture 17). It is possible to flash the firmware without installing jumpers, depending on the program. Screen settings for the new version: 23-MISO, 23-MOSI, 18-CLK, 14-CS, 27-DC, 33-RES, 32-LED, 0-PWR. Buttons from left to right 39-Port1, 38-Port2, 37-Port3. I2C bus: SCL-22, SDA-21.<br>
Settings for[**M5Stack Tough**](https://docs.m5stack.com/en/core/tough) : 23-MISO, 23-MOSI, 18-CLK, 5-CS, 15-DC, 44-RES, 47-LED, 46-PWR. I2C bus: SCL-22, SDA-21. Without I2C the screen will not start.<br>
![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/mytft7.jpg)
<br>Картинка 17. Соединить последний контакт в верхнем ряду и 4 в нижнем ряду (gnd и gpio0) перед прошивкой M5Stack BASIC Kit.<br><br>

#### [ATOM-LITE-ESP32-DEVELOPMENT-KIT](https://docs.m5stack.com/en/core/atom_lite)<br>
![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/mytft8.jpg)
<br>Картинка 18. ATOM-LITE-ESP32-DEVELOPMENT-KIT.<br><br>
     Прошивается атом по usb без установки перемычек. Кнопку использовал для включения-выключения одного из устройств (39-Port1), светодиод пока в прошивке не задействован. IR LED на 12 gpio можно использовать для дистанционного управления. Устройство компактное (24 * 24 * 10 mm), devkit esp32 по размерам больше.
     
     
## 7. Сборка проекта и лицензия
&emsp; Для сборки бинарных файлов использовал [Espressif IoT Development Framework.](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/). Добавлена лицензия MIT. Добавлена конфигурация для сборки в среде PlatformIO, спасибо [bvp](https://github.com/bvp),  его сообщение [здесь](https://github.com/alutov/ESP32-R4sGate-for-Redmond/pull/89).
<details>
<summary>Подробнее...</summary>

## PlatformIO
Это платформа для сборки прошивок для микроконтроллеров. Управляет инструментарием сборки, и зависимостями проекта. Всё нужное скачает сама. Со списком поддерживаемых платформ можно ознакомиться [тут](https://registry.platformio.org/search?t=platform), а фреймворков - [тут](https://registry.platformio.org/search?t=tool&q=keyword%3Aframework).

platformio.ini - файл конфигурации для PlatformIO
Собрать так - `pio run -t build` или просто `pio run`
Загрузить прошивку - `pio run -t upload`
Потребуется только поправить `upload_port` и `monitor_port`.
Для Win32 значение будет вида `COM4` (поставить свой номер порта, на котором находится прошивальщик).
Для Linux - будет `/dev/ttyUSB0` (так же поставить свой номер порта, на котором находится прошивальщик).
Для macOS - как в прилагаемом примере.

## Clang-format
В файле описываются правила форматирования кода, согласно которым код приводится к нужному стилю. Необходим установленный `clang-format`.     
</details>

<br>
