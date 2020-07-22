# ESP32 Ready4Sky (R4S) Gateway for Redmond Kettle
### ENG<br>
ESP32 r4sGate allows you to connect  BLE compatible Redmond kettles like RK-M173S or RK-M240S to the smart home system using the MQTT protocol. For a long time already there is a great similar project for esp32 on the github (https://github.com/olehs/r4sGate), written by olehs for the arduino environment. Unfortunately, as I understand it, the BLE library used in arduino periodically loses connection with the device. And for about two years nothing has changed. That is why I rewrote the program again, but in an espressif esp-idf environment. I have not tested much yet, but the program works faster and there is more free memory. I hope that the BLE connection with the kettle will be more stable.
The file fr4sGate.bin in build folder is already assembled binary for esp32 with 4Mb memory is flashed with a single file from the address 0x0000 to a clean esp32. You can also use three standard files for flashing instead: bootloader.bin (addr 0x1000), partitions.bin (addr 0x8000) and r4sGate.bin (addr 0x10000). The r4sGate.bin file can also be used to update the firmware via the web interface. Then you need to create a guest wifi network in the router with the ssid "r4s" and password "12345678", wait until esp32 connects to it then enter esp32 IP address in web browser and set the remaining parameters. After which the guest network is no longer needed. Esp32 will try to connect to the "r4s" network only if the main network is unavailable, for example, if the password is incorrect. Then you must enter Redmond Kettle name and bind the kettle to the gate (https://mjdm.ru/forum/viewtopic.php?f=8&t=5501). Only one BLE connection available now. It is possible to connect several gateways to one MQTT server. To do this, you need to set different r4sGate Number in each gateway. The gateway with number 0 will write to the topic r4s/devaddr/..., the gateway with number 1 will write r4s1/devaddr/..., etc. It is only necessary to take into account that the authorization request when binding depends on r4sGate Number. This allows you to bind 2 identical kettles to 2 different gateways.<br>
Mqtt topics:<br>
 r4s/devaddr/cmd/state <-- 0/off/false - switch off, 1/on/true - boil, 2...100 - boil&heat;<br>
 r4s/devaddr/cmd/heat_temp <-- 0 - switch off, 1...100 heat;<br>
 r4s/devaddr/cmd/nightlight  <-- 0/off/false - switch off, 1/on/true - switch on;<br>
 r4s/devaddr/cmd/nightlight_red <- 0..255 Red nightlight level;<br>
 r4s/devaddr/cmd/nightlight_green <- 0..255 Green nightlight level;<br>
 r4s/devaddr/cmd/nightlight_blue <- 0..255 Blue nightlight level;<br>
 r4s/devaddr/rsp/ - current state, temperature, rssi etc.;<br>

### RUS<br>
ESP32 r4sGate позволяет подключать BLE-совместимые чайники Redmond, такие как RK-M173S или RK-M240S, к системе «умный дом» по протоколу MQTT.
Давно уже есть  на гитхабе отличный подобный проект  для esp32 (https://github.com/olehs/r4sGate), написанный olehs для среды ардуино. К сожалению,  как я понял,  используемая в ардуино библиотека BLE периодически теряет соединение с устройством. И около двух лет уже ничего не меняется. Вот почему  переписал программу  заново, но уже в espressif esp-idf среде. Пока еще мало тестировал, но программа работает шустрее и свободной памяти стало больше. Надеюсь, и BLE соединение с чайником будет стабильнее. Файл fr4sGate.bin в папке build это уже собранный бинарник для  esp32 с памятью 4 Мбайт и прошивается одним файлом с адреса 0x0000 на чистую esp32. Вместо него также можно использовать три стандартных файла для перепрошивки: bootloader.bin (адрес 0x1000), partitions.bin (адрес 0x8000) и r4sGate.bin (адрес 0x10000). Файл r4sGate.bin можно также использовать для обновления прошивки через web интерфейс. Затем нужно создать гостевую сеть Wi-Fi в роутере с ssid «r4s» и паролем «12345678», подождать, пока esp32 не подключится к нему, ввести esp32 IP-адрес в веб-браузере и установить остальные параметры. После чего гостевая сеть больше не нужна. Esp32 будет пытаться подключиться к сети "r4s" только при недоступности основной сети, например, при неправильном пароле. Затем нужно ввести имя Redmond чайника и привязать чайник к шлюзу (https://mjdm.ru/forum/viewtopic.php?f=8&t=5501). Пока доступно только одно BLE подключение. Предусмотрена возможность подключения к одному MQTT серверу нескольких шлюзов. Для этого нужно в каждом шлюзе установить свой r4sGate Number. Шлюз с номером 0 будет писать в топик r4s/devaddr/..., шлюз с номером 1 - r4s1/devaddr/... и т.д. Нужно только учесть, что запрос на авторизацию при привязке зависит от r4sGate Number. Это позволяет привязать 2 одинаковых чайника к 2 разным шлюзам.<br> 
Mqtt топики:<br>
 r4s/devaddr/cmd/state <-- 0/off/false - выключение, 1/on/true - кипячение, 2...100 - кипячение и подогрев;<br>
 r4s/devaddr/cmd/heat_temp <-- 0 - выключение, 1...100 подогрев;<br>
 r4s/devaddr/cmd/nightlight  <-- 0/off/false - выключение ночника, 1/on/true - включение ночника;<br>
 r4s/devaddr/cmd/nightlight_red <- 0..255 Уровень красного в ночнике;<br>
 r4s/devaddr/cmd/nightlight_green <- 0..255 Уровень зеленого в ночнике;<br>
 r4s/devaddr/cmd/nightlight_blue <- 0..255 Уровень синего в ночнике;<br>
 r4s/devaddr/rsp/ - текущее состояние, температура, rssi и т.д.;<br><br>
Веб интерфейс:
![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/myweb.jpg) 
 <br>Мои настройки Mqtt в Iobroker-е:
 ![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/mymqtt.jpg)
 <br><br> Добавил экран 320x240 на ili9341. Получилось что-то похожее на часы. Температура в помещении, на выходе котла и на улице. Все берется с Mqtt. Пока выглядит так:
 ![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/mytft.jpg)
 <br><br>Если заинтересует, выложу и этот вариант здесь.<br><br>

# Rebuild notes
Use Espressif IoT Development Framework (https://docs.espressif.com/projects/esp-idf/en/latest/esp32/) to build binary files.<br>
