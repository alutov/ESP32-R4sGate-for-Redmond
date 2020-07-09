# ESP32 Ready4Sky (R4S) Gateway for Redmond Kettle
ENG
ESP32 r4sGate allows you to connect  BLE compatible Redmond kettles like RK-M173S or RK-M240S to the smart home system using the MQTT protocol.
The file fr4sGate.bin in build folder is already assembled binary for esp32 with 4Mb memory is flashed with a single file from the address 0x0000 to a clean esp32.<br> 
You can also use three standard files for flashing instead: bootloader.bin (addr 0x1000), partitions.bin (addr 0x8000) and r4sGate.bin (addr 0x10000).
Then you need to create a guest wifi network in the router with the ssid "r4s" and password "12345678", wait until esp32 connects to it then enter esp32 
IP address in web browser and set the remaining parameters. After which the guest network is no longer needed. Then you must enter Redmond Kettle name to 
connect to device. Only one BLE connection available now.<br>
Mqtt topics:<br>
 r4s/devaddr/cmd/state <-- 0/off/false - switch off, 1/on/true - boil, 2...100 - boil&heat;<br>
 r4s/devaddr/cmd/heat_temp <-- 0 - switch off, 1...100 heat;<br>
 r4s/devaddr/cmd/nightlight  <-- 0/off/false - switch off, 1/on/true - switch on;<br>
 r4s/devaddr/rsp/ - current state, temperature, rssi etc.;<br>

RUS
ESP32 r4sGate позволяет подключать BLE-совместимые чайники Redmond, такие как RK-M173S или RK-M240S, к системе «умный дом» по с использованием протокола MQTT.
Файл fr4sGate.bin в папке build уже собранный бинарник для  esp32 с памятью 4 Мбайт и прошивается одним файлом с адреса 0x0000 на чистую esp32. <br>
Вместо этого также можно использовать три стандартных файла для перепрошивки: bootloader.bin (адрес 0x1000), partitions.bin (адрес 0x8000) и r4sGate.bin (адрес 0x10000).
Затем нужно создать гостевую сеть Wi-Fi в маршрутизаторе с ssid «r4s» и паролем «12345678», подождать, пока esp32 не подключится к нему, ввести esp32
IP-адрес в веб-браузере и установить остальные параметры. После чего гостевая сеть больше не нужна. Затем вы должны ввести имя Redmond чайника, чтобы
подключиться к устройству. Пока доступно только одно BLE подключение. 

# Rebuild notes
Use Espressif IoT Development Framework(https://docs.espressif.com/projects/esp-idf/en/latest/esp32/) to build binary files.<br>
