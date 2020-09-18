# ESP32 Ready4Sky (R4S) Gateway for Redmond Kettle and Multicooker

### ENG<br>
This version of ESP32 r4sGate allows you to connect BLE-compatible Redmond kettles, such as RK-M173S or RK-M240S, as well as the RMC-M800S multicooker to a smart home system using the MQTT protocol. Now 3 BLE connections are supported, support for RMC-M800S has been added (Special thanks to vring0 (https://github.com/vring0) for describing its protocol). For a long time already there is an excellent similar project for esp32 on github (https://github.com/olehs/r4sGate, forum https://mjdm.ru/forum/viewtopic.php?f=8&t=5501), written by olehs for the arduino environment. Unfortunately, the BLE library used in arduino periodically loses connection with the device. That is why I rewrote the program again but in the espressif esp-idf environment. The first version of the program with the same capabilities (08/27/2020), supporting only a kettle and one BLE connection, works faster and more stable keeps the connection with the kettle, and there is more free memory. I hope that the new version will work just as well. To start the gateway, you need to program the ESP32. The file fr4sGate.bin in build folder is already assembled binary for esp32 with 4Mb memory is flashed with a single file from the address 0x0000 to a clean esp32. You can also use three standard files for flashing instead: bootloader.bin (addr 0x1000), partitions.bin (addr 0x8000) and r4sGate.bin (addr 0x10000). The r4sGate.bin file can also be used to update the firmware via the web interface. Then you need to create a guest Wi-Fi network in the router with ssid "r4s" and password "12345678", wait until esp32 connects to it, enter the esp32 IP address in a web browser and set other parameters in the Setting tab. Then the guest network is no longer needed. Esp32 will try to connect to the "r4s" network only if the main network is unavailable, for example, if the password is incorrect. Then you need to enter the name of the Redmond device and bind it to the gateway. To bind, you need to press and hold the "+" button on the kettle or "timer" on the multicooker until the device enters the binding mode and after a while connects to the gateway. It is possible to connect several gateways to one MQTT server. To do this, you need to set its own r4sGate Number in each gateway. The gateway with number 0 will write to the topic r4s/devaddr/..., the gateway with number 1 will write r4s1/devaddr/..., etc. You just need to take into account that the authorization request for binding depends on the gateway number and on the connection number in the gateway. This allows you to bind 2 identical kettles or multicookers to 2 different gateways or 2 different connections within the same gateway. My gateway is connected to Iobroker. My MQTT broker settings are below in picture 1.
The retain flag is cleared so that the broker does not remember, but reads the state of devices when connected. In Home Assistant, the retain flag set in it and/or Mqtt broker can lead to spontaneous switching on and off of the device (https://mjdm.ru/forum/viewtopic.php?f=8&t=5501&sid=de6b1e2b43f25c8d9ae9af5673ee9417&start=140#p121604). Also, the flag of publishing when subscribing is set, which allows not to enter all topics manually. Sometimes, when publishing a large number of subscriptions at once, iobroker for some reason makes some of them write-protected :-), I have such a glitch. I have to remove them and restart the Mqtt adapter so that they appear again.
<br>
 Mqtt topics for kettle (see image 2 below):<br>
 r4s/devaddr/cmd/state <-- 0/off/false - switch off, 1/on/true - boil, 2...100 - boil&heat;<br>
 r4s/devaddr/cmd/heat_temp <-- 0 - switch off, 1...100 heat;<br>
 r4s/devaddr/cmd/nightlight  <-- 0/off/false - switch off, 1/on/true - switch on;<br>
 r4s/devaddr/cmd/nightlight_red <- 0..255 Red nightlight level;<br>
 r4s/devaddr/cmd/nightlight_green <- 0..255 Green nightlight level;<br>
 r4s/devaddr/cmd/nightlight_blue <- 0..255 Blue nightlight level;<br>
 r4s/devaddr/rsp/ - current state, temperature, rssi etc.;<br>
The level values are stored in the gateway and transmitted to the kettle when the backlight is turned on.<br><br>
 Mqtt topics for multicooker (see image 3 below): <br>
r4s / devaddr / cmd / state <- 0 / off / false - switch off, 1 / on / true - program start or heating if program not set; <br>
r4s / devaddr / cmd / prog <- program number 1-12, 0 - switch off; <br>
 Programs: <br>
 1 - Rice / Рис Крупы, 2 - Slow Cooking / Томление, 3 - Pilaf / Плов, 4 - Frying / Жарка;<br>
 5 - Stewing / Тушение, 6 - Pasta / Паста, 7 - Milk Porridge / Молочная каша, 8 - Soup / Суп;<br>
 9 - Yogurt / Йогурт, 10 - Baking / Выпечка, 11 - Steam / Пар, 12 - Hot / Варка Бобовые;<br>
r4s/devaddr/cmd/mode <- mode: 1 - vegetables, 2 - fish, 3 - meat for programs 4,5,11 <br>
r4s/devaddr/cmd/temp <- temperature;<br>
r4s/devaddr/cmd/set_hour <- program running time, hours;<br>
r4s/devaddr/cmd/set_min <- program running time, minutes;<br>
r4s/devaddr/cmd/delay_hour <- program running time plus a delay before the program start, hours;<br>
r4s/devaddr/cmd/delay_min <- program running time plus a delay before the program start, minutes;<br>
r4s/devaddr/cmd/warm <- heating after the end of the program; <br>
The delay_hour and delay_min parameters are stored in the gateway and transmitted when setting the mode or heating, and therefore are set after setting the program and before setting the mode or auto-heating. When a program is selected, the temperature and program runtime are set by default, after setting mode they are adjusted again. After setting the program and mode, you can adjust the time and temperature if necessary. Multicook mode is not supported yet, I don't see the point. When writing zero to prog, a switch off command is sent to the multicooker, which is useful for resetting the program.<br>
The devices can also be controlled via the web interface. Examples of the main page and the settings page are below in pictures 4 and 5.<br>

#### Screen support<br>
You can, of course, after programming esp32 and setting it up, connect it to a power source and hide it in the kitchen. But 100 KB of free RAM allowed expanding the capabilities of the firmware, in particular, adding screen support. And I already had working esp32s with a 3.2" 320x240 screen on an ili9341 chip (https://www.aliexpress.com/item/32911859963.html?spm=a2g0s.9042311.0.0.274233edzZnjSp) with firmware from the site https://wifi-iot.com/. For the screen I used only the necessary procedures from https://github.com/Bodmer/TFT_eSPI, adapted not very well, but as it is for esp-iot. Pins for connecting the screen (available in the tft.c file): MOSI-23, MISO-25, CLK-19, CS-16, DC-17, RST-18, BACKLIGHT-21. TOUCH CS is not used yet, but it is connected to pin 33, which is always high. It turned out something like a watch. The current time, day and month, as well as temperature and humidity, boiler outlet temperature, temperature and humidity outside. Everything is taken from Mqtt. Sample screen on page 6. The possibility of displaying images in jpeg format 320x176 is also provided. The size of the picture will be about 20 kB. To do this, you need to specify the url of the image. My camera has url like this: http://192.168.1.7/auto.jpg?usr=admin&pwd=admin. The picture is loaded into a 32768 byte buffer in RAM. The update time can be set in the settings. Sample screen on page 7.<br><br>

### RUS<br>
 Эта версия ESP32 r4sGate позволяет подключать BLE-совместимые чайники Redmond, такие как RK-M173S или RK-M240S, а также мультиварку RMC-M800S к системе "умный дом" по протоколу MQTT. Теперь поддерживаются 3 BLE соединения, добавлена поддержка RMC-M800S (Отдельное спасибо vring0 (https://github.com/vring0) за описание ее протокола). Давно уже есть на гитхабе отличный подобный проект для esp32 (https://github.com/olehs/r4sGate, форум https://mjdm.ru/forum/viewtopic.php?f=8&t=5501), написанный olehs для среды ардуино. К сожалению используемая в ардуино библиотека BLE периодически теряет соединение с устройством. Вот почему переписал программу заново, но уже в espressif esp-idf среде. Первая версия программы с теми же возможностями (27.08.2020), поддерживающая только чайник и одно BLE соединение, работает шустрее и стабильнее держит соединение с чайником, а свободной памяти стало больше. Надеюсь, что и новая версия будет работать не хуже. Для запуска шлюза нужно запрограммировать ESP32. Файл fr4sGate.bin в папке build это уже собранный бинарник для esp32 с памятью 4 Мбайт и прошивается одним файлом с адреса 0x0000 на чистую esp32. Вместо него также можно использовать три стандартных файла для перепрошивки: bootloader.bin (адрес 0x1000),  partitions.bin (адрес 0x8000) и r4sGate.bin (адрес 0x10000). Файл r4sGate.bin можно также использовать для обновления прошивки через web интерфейс. Затем нужно создать гостевую сеть Wi-Fi в роутере с ssid <r4s> и паролем "12345678", подождать, пока esp32 не подключится к нему, ввести esp32 IP-адрес в веб-браузере и во вкладке Setting установить остальные параметры. После чего гостевая сеть больше не нужна. Esp32 будет пытаться подключиться к сети "r4s" только при недоступности основной сети, например, при неправильном пароле. Затем нужно ввести имя Redmond устройства и привязать его к шлюзу. Для привязки нужно нажать и удерживать кнопку "+" на чайнике или "таймер" на мультиварке  до тех пор, пока устройство не войдет в режим привязки и через некоторое время соединится со шлюзом. Предусмотрена возможность подключения к одному MQTT серверу нескольких шлюзов. Для этого нужно в каждом шлюзе установить свой r4sGate Number. Шлюз с номером 0 будет писать в топик r4s/devaddr/..., шлюз с номером 1 - r4s1/devaddr/... и т.д. Нужно только учесть, что запрос на авторизацию при привязке зависит от номера шлюза и от номера соединения в шлюзе. Это позволяет привязать 2 одинаковых чайника или мультиварки к 2 разным шлюзам или к 2 разным соединениям в пределах одного шлюза. У меня шлюз подключен к Iobroker. Мои настройки MQTT брокера ниже на картинке 1.<br><br>
 ![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/mymqtt.jpg)
 Image 1. My Mqtt setting.<br><br>
 Снят флаг retain, чтобы брокер не запоминал, а считывал состояние устройств при соединении. В Home Assistant  установленный в нем и/или Mqtt брокере флаг retain может приводить к самопроизвольному включению и выключению устройства (https://mjdm.ru/forum/viewtopic.php?f=8&t=5501&sid=de6b1e2b43f25c8d9ae9af5673ee9417&start=140#p121604). Также установлен флаг публикации при подписке, что позволяет не вводить все топики вручную. Иногда при публикации сразу большого числа подписок iobroker почему-то делает некоторые из них с защитой от записи :-), есть у меня такой глюк. Приходится их удалять и перезапускать Mqtt адаптер, чтобы они появились опять.<br>
Mqtt топики для чайника (см. картинку 2 ниже):<br>
r4s/devaddr/cmd/state <-- 0/off/false - выключение, 1/on/true - кипячение, 2...100 - кипячение и подогрев;<br>
r4s/devaddr/cmd/heat_temp <-- 0 - выключение, 1...100 подогрев;<br>
r4s/devaddr/cmd/nightlight <-- 0/off/false - выключение ночника, 1/on/true - включение ночника;<br>
r4s/devaddr/cmd/nightlight_red <- 0..255 Уровень красного в ночнике;<br>
r4s/devaddr/cmd/nightlight_green <- 0..255 Уровень зеленого в ночнике;<br>
r4s/devaddr/cmd/nightlight_blue <- 0..255 Уровень синего в ночнике;<br>
r4s/devaddr/rsp/ - текущее состояние, температура, rssi и т.д.;<br>
Значения уровней запоминаются в шлюзе и передаются на чайник при включении подсветки.<br><br>
![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/mymqtt1.jpg)
 Image 2. My Mqtt kettle topics.<br><br> 
Mqtt топики для мультиварки (см. картинку 3 ниже):<br>
r4s/devaddr/cmd/state <-- 0/off/false - выключение, 1/on/true - старт программы или подогрев, если программа не установлена;<br> 
r4s/devaddr/cmd/prog <-- номер программы 1-12, 0 - выключение;<br>
 Программы:<br>
 1 - Rice / Рис Крупы, 2 - Slow Cooking / Томление, 3 - Pilaf / Плов, 4 - Frying / Жарка;<br>
 5 - Stewing / Тушение, 6 - Pasta / Паста, 7 - Milk Porridge / Молочная каша, 8 - Soup / Суп;<br>
 9 - Yogurt / Йогурт, 10 - Baking / Выпечка, 11 - Steam / Пар, 12 - Hot / Варка Бобовые;<br>
r4s/devaddr/cmd/mode <-- режим: 1 - овощи, 2 - рыба, 3 - мясо для программ 4,5,11<br>
r4s/devaddr/cmd/temp <-- температура;<br>
r4s/devaddr/cmd/set_hour <-- время работы программы, часы;<br>
r4s/devaddr/cmd/set_min <-- время работы программы, минуты;<br>
r4s/devaddr/cmd/delay_hour <-- время работы программы плюс задержка до старта программы, часы;<br>
r4s/devaddr/cmd/delay_min <-- время работы программы плюс задержка до старта программы, минуты;<br>
r4s/devaddr/cmd/warm <-- подогрев после завершения программы;<br>
 Параметры delay_hour и delay_min запоминаются в шлюзе и передаются при установке режима или подогрева, а потому устанавливаются после установки программы и перед установкой режима или автоподогрева. При выборе программы устанавливаются температура и время работы программы по умолчанию, после установки mode еще раз корректируются. После установки программы и режима можно при необходимости скорректировать время и температуру. Режим мультиповар пока не поддерживается, я не вижу смысла. При записи нуля в prog на мультиварку посылается команда выключения, что полезно для сброса программы.<br><br>
![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/mymqtt2.jpg)
 Image 3. My Mqtt multicooker topics.<br><br> 
Устройствами можно управлять также и по веб интерфейсу. Примеры главной страницы и страницы настроек ниже на картинках 4 и 5.

**Web interface main page / Веб интерфейс, главная страничка**:
![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/myweb.jpg) 
Image 4. Main web page.<br><br>
**Web interface setting page /Веб интерфейс, страничка настроек**:
![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/myweb1.jpg) 
Image 5. Setting web page.<br><br>
 
#### Поддержка экрана<br>
 Можно, конечно, после программирования esp32 и настройки подсоединить ее к источнику питания и спрятать на кухне. Но 100 кБайт свободной  оперативной памяти позволяли расширить возможности прошивки, в частности, добавить поддержку экрана. И у меня уже были работающие esp32 с экраном 3.2" 320x240 на чипе ili9341 (https://www.aliexpress.com/item/32911859963.html?spm=a2g0s.9042311.0.0.274233edzZnjSp) с прошивкой с сайта https://wifi-iot.com/. Для экрана использовал только необходимые процедуры из https://github.com/Bodmer/TFT_eSPI, адаптированные не совсем хорошо, но как есть для esp-iot. Пины для поключения экрана (есть в файле tft.c): MOSI-23, MISO-25, CLK-19, CS-16, DC-17, RST-18, BACKLIGHT-21. TOUCH CS пока не используется, но подключен к 33 пину, на котором постоянно 1. Получилось что-то похожее на часы. На экран выводится текущее время, день и месяц, а также температура в и влажность, температура на выходе котла, температура и влажность на улице. Все берется с Mqtt. Пример экрана на страничке 6.<br><br>
![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/mytft.jpg)
Image 6. Main screen example 1.<br><br>

 Предусмотрена возможность вывода на экран и картинки в формате jpeg 320x176. Размер картинки будет около 20 кБайт. Для этого нужно указать url картинки. У моей камеры url такой: http://192.168.1.7/auto.jpg?usr=admin&pwd=admin. Картинка грузится в буфер размером 32768 байт в оперативной памяти. Время обновления можно установить в настройках. Пример экрана на страничке 7. <br><br>
 ![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/mytft1.jpg)
Image 7. Main screen example 2.<br><br>

# Rebuild notes
Use Espressif IoT Development Framework (https://docs.espressif.com/projects/esp-idf/en/latest/esp32/) to build binary files.<br>
