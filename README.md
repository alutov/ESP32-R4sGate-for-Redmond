# ESP32 Ready4Sky (R4S) gateway for Redmond+ devices
> **[A script based on a PHP server on a local network with Internet access to display the weather on the gateway screen.](https://github.com/artt652/Weather-for-ESP32-R4S-gate).**<br>
>**In versions starting from 2022.06.03, the device topics in gateway number 0 have been changed from “r4s/#” to “r4s0/#”. In new versions, topics in "r4s/#" are used to track tags by multiple gateways. When upgrading from older versions, you need to enable BLE Monitor in the settings (the gateway deletes the contents of "r4s/#" only when the monitor is turned on), selecting Passive, Active or Auto, check the Hass Discovery and Delete Mqtt Topics items and save the settings. After the reboot, the gateway will delete unnecessary topics and create them again. Then, if necessary, BLE Monitor can be disabled. Then fix the devices in automations, scripts, etc.**

#### Current version is 2024.02.05 для [ESP32](https://github.com/alutov/ESP32-R4sGate-for-Redmond/raw/master/build/r4sGate.bin) и [ESP32C3](https://github.com/alutov/ESP32-R4sGate-for-Redmond/raw/master/C3/build/r4sGate.bin).
* 2024.02.05. At startup, the gateway scans all WIFI APs with the required parameters and selects the AP with the best level. Relevant for mesh systems.
* 2024.02.04. Added selection of WIFI 802.11b/g/n mode in settings.
* 2024.02.02. Corrections by Delonghi. In Auto mode, the image buffer size is selected separately for each URL.
* 2024.01.30. Added the ability to automatically increase the buffer size for loading images within 10-65 kilobytes (Adjust option). The gateway operates more stable at wifi levels below -73dBm.
* 2024.01.25. Corrected Delonghi statistics and availability of BLE devices/tags in Home Assistant.
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
&emsp;Поддерживается Home Assistant Mqtt Discovery. Для включения нужно отметить **Hass Discovery** в настройках. Предусмотрена возможность удаления всех созданных шлюзом данных в Mqtt и устройств в Home Assistant. Для этого нужно выбрать во вкладке **Setting** опцию **Delete Mqtt topics** и затем нажать **Save setting**. После перезагрузки шлюза будут заново созданы только подключенные к шлюзу устройства. Рекомендуется при первом подключении шлюза и реконфигурации с удалением устройств.<br> 

<details>
<summary>Простой вариант интеграции чайника из Home Assistant в умный дом Яндекса - использовать сущность климата.</summary>

&emsp; И назвать его словом **чайник**. Доступны будут все команды устройства термостат. Например, команда **включи чайник** (режим auto, включение кипячение или кипячение с последующим подогревом, если до этого был включен подогрев), **выключи чайник** (выключает все), **установи температуру чайника 40&deg;** (если не 0&deg;  и не 100&deg; включает подогрев, режим heat,  если 0&deg; - выключает, если же температура 100&deg;, включается кипячение, режим auto) либо **включи обогрев** (включает подогрев с последней установленной температурой). И, наконец, команда **включи охлаждение** - включает подсветку, режим cool. Не очень красиво, но как есть. Можно спросить, **что там с чайником** - **термостат чайник выключен** и **какая температура чайника** - скажет текущую температуру. <br>
</details>     

<details>
<summary>Поддерживается вычисление количества воды в чайнике при нагреве в интервале 65-85&deg;C и более 3&deg;C с момента включения чайника.</summary>

&emsp;Не требуется никаких доработок чайника. Вычисляется на основе затраченной энергии и разности температур. Вычисленное значение сбрасывается при снятии чайника с подставки. Опция работает только в чайниках со статистикой. КПД чайника изначально принят 80%. Точность так себе, у меня выходит где-то ~0.2 литра. Для повышения точности предусмотрен режим корректировки значения КПД. Для этого нужно залить в чайник 1л воды и выбрать в web-интерфейсе **Boil 1l on**. Когда режим отработает, нужно зайти в режим настроек. Новое значение будет выведено сразу за типом устройства. Записать новое значение в nvram можно командой Save setting. Как мне думается, получить большую точность нереально, так как КПД чайника со временем меняется, например, с появлением накипи, и, что хуже, затраченная энергия не измеряется, а просто вычисляется процессором чайника исходя из номинальной мощности нагревателя и времени его работы. Отклонение питающего напряжения при работе от значения при калибровке вносит заметную погрешность, зависимость там квадратичная. У меня при кипячении чайником RK-M216S 1.7 литра воды при напряжении на входе в дом 200-204V в итоге вычисляется 1.8 литра, при напряжении 210-214V выходит 1.6 литра. При калибровке очевидно было что-то среднее.<br>
</details>     

&emsp;BLE монитор шлюза можно использовать для отслеживания до 24 BLE устройств меток/(маяков) со статическим MAC адресом. Выводится наличие/отсутствие метки(маяка) и rssi. Поддерживаются BLE маяки приложения Home Assistant на смартфонах (привязка по uuid), LYWSD02 часы с термометром, LYWSD03MMC термометры Xiaomi Mijia 2 с оригинальной прошивкой, [прошивкой от atc1441 в режиме custom](https://github.com/atc1441/ATC_MiThermometer) и [прошивкой от pvvx в режиме custom](https://github.com/pvvx/ATC_MiThermometer), Xiaomi Mi Scale, Qingping Air Monitor Lite(CGDN1), счетчики Elehant, а также Samsung Smart Tag.<br>
&emsp;Предусмотрено 10 портов ввода-вывода, 5 из них можно использовать для управления внешними устройствами(режим Out) и чтения их состояния(режим In). Три порта можно настроить как кнопки для включения - выключения подключенных BLE устройств(режим Sw, при этом состояние кнопок в mqtt не выводится), четвертый порт - как кнопку обновления картинки с камеры. При конфигурации в режиме входа включается pullup, если это возможно (номер пина меньше 34). Еще 2 порта используются шиной I2C, а каждый из 3-х оставшихся портов можно использовать как выход с широтно - импульсной модуляцией (PWM), или же как вход для подключения или одного датчика DS18B20 с прямым питанием, или одного датчика DHT22/AM2302 (7 и 8 порт). Процедуры чтения упрощены, контрольная сумма не читается и не проверяется, данные округляются до одного знака после запятой. Если шлюз оборудован звуковым излучателем, то, подключив к нему выход PWM (в m5stack basic это gpio 25), можно организовать вывод звукового сигнала. Изменяя скважность импульсов, можно регулировать громкость. Частота фиксирована и равна 3.136 kHz. Шина I2C поддерживает датчики SHT3x/SHT4X(адреса 0x44, 0x45), AHT20(0x38), HTU21(0x40), BMP280/BME280/680/688(0x76, 0x77, 688 пока не проверен), SGP30(0x58), SGP4x(0x59), SCD4x(0x62), а также RTC DS3231(0x68) и контроллер батареи IP5306(0x75). Предусмотрено сохранение данных калибровки SGP30 в NVRAM и восстановление их при старте шлюза. Для этого нужно отметить пункт AQ base в настройках. Для вычисления VOC в SGP4x используется [библиотека Sensirion](https://github.com/Sensirion/gas-index-algorithm). Часы используются для хранения даты и времени с NTP сервера, датчик температуры выводится в Mqtt. Контроллер IP5306 установлен в m5stack и ttgo-t4 (SCL 22, SDA 21), позволяет определять уровень батареи с шагом 25% и ее режим (Discarging / Charging / Charged). При питании от батареи яркость экрана уменьшается в 16 раз. Шлюз поддерживает также контроллер питания AXP192 и RTC PCF8563, что позволяет ему работать на M5Stack Tough, а также поддерживает ADC тензодатчиков HX711. Результат измерения с HX711 можно выводить как в килограммах, так и в процентах, в зависимости от калибровки. HX711 опрашивается с интервалом 4 секунды, остальные датчики с интервалом 12 секунд. Шлюз допускает горячее подключение всех сенсоров. Датчики 18B20 и DHT22 появляются в Mqtt и Home Assistant сразу после старта шлюза, даже если они не подключены, а I2C сенсоры по мере обнаружения их на шине в течение 2-х циклов опроса (24 секунды).    
<details>
     <summary>Есть также поддержка IR передатчика (IR Tx, 6 порт).</summary>

&emsp;Поддерживаются протоколы **NEC** (8 и 16 битный адрес) **RC5** (для работы в режиме RC5ext нужно инвертировать 6 бит команды), **RC6**, **Samsung**, Sony **SIRC** (12, 15 и 20 бит), **Panasonic**. Управлять можно как из интерфейса Home Assistant и отдельных топиков адреса, команды и протокола, так и прямой записью в топик **r4sx/ir6code** (где **x** - номер шлюза) строки из 8 hex символов 0-9,a-f, например, **090a1c3d**, где **09** - протокол(01-nec, 02-necx16, 03-rc5, 04-rc6, 05-samsung, 06-sircx12, 07-sircx15, 08-sircx20, 09-panasonic), **0a1c** - адрес, **3d** -команда.<br>
Что проверено (интересовала команда включения питания):<br>
**NEC:** pioneer vsx-830, power: addr 165, cmd 28, code 0100a51c<br>
**NECx16:** lg dvd dks-2000h, power: addr 11565, cmd 48, code 022d2d30<br> 
**RC6:** philips 40pfs6609, power: addr 0, cmd 12, code 0400000c<br>
**SAMSUNG:** ue32n5300, power: addr 7, cmd 2, code 05000702<br>
**SIRCx12:** sony cmt-sx7, power: addr:16, cmd: 21, code 06001015<br>
**SIRCx20:** sony ubp-x800 power: addr 7258, cmd 21, code 081c5a15<br>
**PANASONIC:** sa-pm20 power: addr 2588, cmd 61, code 090a1c3d<br>
**PANASONIC:** dmp-ub900 power: addr 2816, cmd 61, code 090b003d<br>
Проверял все на Atom lite, в нем есть IR LED на 12 gpio. Пока не проверены RC5 и SIRCx15.<br>
</details>

&emsp;С целью расширения возможностей шлюза возможно подключение TFT экрана 320 * 240 на чипах ili9341, ili9342 и ST7789. На экран выводится текущее время, дата, а также температура, напряжение и ток  в доме (не помешает при питании от генератора), состояние (синий- выкл., красный - вкл.) и температура на выходе котла, температура и влажность на улице. Все берется с Mqtt. Рядом с датой цветом выводится состояние BLE устройств, 1 ... 5 - с первого по пятое. Серый - не на связи или не определено, синий - выключено, красный - включено, желтый - подогрев, белый - установлена программа. Подробнее состояние и некоторые параметры подключенных BLE устройств по очереди отображаются в нижней строке. Есть возможность периодического или по запросу кнопкой вывода на экран картинки в формате jpeg, например, с камеры.  Картинки разрешением по горизонтали выше 320 выводятся в масштабе 1:2. Размер буфера для загрузки картинки можно менять в пределах 20-65 килобайт. Яркость экрана можно изменять по Mqtt. Можно также на экран выводить погоду в текстовом вида с сайта wttr.in или просто текст, записывая его в Mqtt топик r4sx/jpg_url. Получилось что-то похожее на часы с термометром. Достаточно глянуть на экран, чтобы убедиться, все ли в порядке в доме, что там сегодня на улице.<br>

## 2. Комплектующие

![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/myparts.jpg)
Картинка 1. Комплектующие для сборки шлюза.

&emsp; Если цель запустить шлюз с минимальными затратами, придется покупать запчасти, затем собирать из них шлюз. Я использовал [ESP32 WROOM ESP-32 4 Мб с встроенной антенной (слева внизу) или ESP32 WROOM ESP-32U 4 Мб с внешней (правее первой)](https://aliexpress.ru/item/32961594602.html?item_id=32961594602&sku_id=66888778667&spm=a2g2w.productlist.0.0.4f835c61Fvd1gD). Цена вопроса $2.5. Потом паял микросхему на [адаптер-плату($0.3)](https://www.aliexpress.com/item/32763489487.html?spm=a2g2w.productlist.i0.2.48d33c75KbnjpB&sku_id=62208988599) и далее на макетную плату. Подойдет также ESP32C3, у меня это ESP32C3-12F. Из-за аппаратных ограничений этой микросхемы шлюз использует порт8 только как выход с широтно - импульсной модуляцией (PWM). Cвободной оперативной памяти больше примерно на 28 килобайт. И даже при подключенном экране у ESP32C3-12F остается еще 6 свободных gpio. Источник питания на 3.3 Hi-Link($2-$4). [Я их брал по цене $1.65](https://aliexpress.ru/item/32953853140.html?spm=a2g39.orderlist.0.0.32964aa6PePEbg&_ga=2.238912000.104655408.1636114275-428746708.1615828563&_gac=1.87036010.1634012869.Cj0KCQjwwY-LBhD6ARIsACvT72Na1GBQp7leEJDlxPCd0jTye8sF-GiknWzlo4hKElMNbtmI4DYpB_8aAktOEALw_wcB). **Можно обойтись без пайки**, если использовать [esp32-wroom-devkit(внизу в центре, $14)](https://aliexpress.ru/item/4000127837743.html?sku_id=10000000372418546&spm=a2g0s.9042311.0.0.274233edNcajyj). Правда, эта плата сильно избыточна для проекта, [можно взять попроще за $3.54](https://aliexpress.ru/item/32928267626.html?item_id=32928267626&sku_id=12000016847177755&spm=a2g2w.productlist.0.0.430c65c8Kf9vOT). В нем esp32 идет вместе с платой, на которой есть еще преобразователи с 5в на 3.3в, USB-RS232 и стандартный разъем мини-USB. Через него можно питать esp32, используя пятивольтовое зарядное устройство от смартфона, и программировать прямо с компьютера без всяких переходников. И справа на фото [3.2" 320 * 240 TFT экран($18)](https://aliexpress.ru/item/32911859963.html?spm=a2g0s.9042311.0.0.274233edzZnjSp), который я использовал в шлюзе. Можно использовать и совместимые готовые устройства как с экраном (**TTGO T-Watcher BTC Ticker**, **M5Stack BASIC**, **M5Stack Tough**), так и без (**m5atom lite**).

## 3. Настройка шлюза

&emsp; Для запуска шлюза нужно [запрограммировать ESP32](https://www.youtube.com/watch?v=Vy-YTSdwy7s). Для прошивки можно использовать программу [flash_download_tools](https://www.espressif.com/en/support/download/other-tools). Файл [**fr4sGate.bin**](https://github.com/alutov/ESP32-R4sGate-for-Redmond/raw/master/build/fr4sGate.bin) в папке build это уже собранный бинарник для esp32 @160MHz с памятью 4 Мбайт, DIO загрузчиком и прошивается одним файлом с адреса **0x0000** в режиме **DIO**.  Если же DIO загрузчик не стартует, можно использовать файл [**fqr4sGate.bin**](https://github.com/alutov/ESP32-R4sGate-for-Redmond/raw/master/build/fqr4sGate.bin) с загрузчиком QIO и программировать его в режиме **QIO**. Как я понял, большинство esp32 можно программировать в любом режиме, но были случаи, что шлюз работал только при прошивке его файлом **fqr4sGate.bin** в режиме **QIO**. Файл [**r4sGate.bin**](https://github.com/alutov/ESP32-R4sGate-for-Redmond/raw/master/build/r4sGate.bin) используется для обновления прошивки через web интерфейс. Файлы для программирования ESP32C3 в папке C3.<br>
&emsp; Затем нужно создать точку доступа на смартфоне с ssid **r4s** и паролем **12345678**, подождать, пока esp32 не подключится к нему, найти в параметрах точки доступа подключенное устройство **r4s0Gate** и его IP, ввести этот адрес в веб-браузере и далее в web во вкладке Setting установить остальные параметры. После чего точка доступа больше не нужна. Esp32 будет пытаться подключиться к сети **r4s** только при недоступности основной сети, например, при неправильном пароле. Если не удается подключиться и к гостевой сети, esp32 перезагружается. Вариант с гостевой сетью в отличие от общепринятой практики запуска точки доступа на esp32, как мне кажется, удобнее, так как в случае падения по каким-то причинам Wi-Fi роутера (а он может быть выделенным только для iot устройств) остальной Wi-Fi не засоряется дружно вплывшими esp32.<br>
&emsp; Далее нужно ввести имя или  MAC адрес Redmond устройства и привязать его к шлюзу. Поиск устройств запускается только тогда, когда есть хотя бы одно определенное, но не соединенное устройство, либо активен BLE монитор. Если имя устройства точно не известно (а редмонды не всегда светятся по BLE как модель один в один), то для начала сканирования нужно ввести в поле **Name** в настройках любое имя, а потом заменить его найденным при сканировании и выбрать в настройках ближайший тип устройства (поле **TYPE**, например, для чайников от RK-G(M)200S до RK-G(M)240S  протокол один и тот же, можно выбрать как RK-G200S, так и RK-G240S). Нужно учитывать, что не все устройства передают имя при пассивном сканировании (например, Xiaomi Mikettle, AM43 Blinds). В любом случае лучше вводить в поле имени MAC адрес, можно как с двоеточиями, так и без. Найти и скопировать адрес можно **BLE Last found name/address** на главной страничке или на странице BLE monitor. Далее для привязки нужно нажать и удерживать кнопку **+** на чайнике или **таймер** на мультиварке  до тех пор, пока устройство не войдет в режим привязки и через некоторое время соединится со шлюзом. Жалюзи AM43 требуют еще и ввода пин кода (Passkey) для соединения.<br> 
&emsp; Предусмотрена возможность подключения к одному MQTT серверу нескольких шлюзов. Для этого нужно в каждом шлюзе установить свой r4sGate Number. Шлюз с номером 0 будет писать в топик r4s0/devaddr/..., шлюз с номером 1 - r4s1/devaddr/... и т.д. Нужно только учесть, что запрос на авторизацию при привязке зависит от номера шлюза и от номера соединения в шлюзе. Это позволяет привязать 2 одинаковых чайника или мультиварки к 2 разным шлюзам или к 2 разным соединениям в пределах одного шлюза. Если же рядом работают два шлюза с одинаковыми параметрами, подключенные к разным системам умного дома (например, сосед за стенкой), для исключения возможности подключения устройства к шлюзу соседа можно использовать опцию авторизации устройств с использованием MAC адреса шлюза, выбрав в настройках **Use MAC in BLE Authentication**. После чего сбросить на устройствах все привязки и затем привязать их к шлюзу заново.<br>&emsp; Для подключения к Mqtt брокеру нужно ввести его адрес и порт, а также логин и пароль. Если шлюз работает с Home Assistant в паре с mosquitto брокером, стоит использовать опцию Hass Discovery. Перед ее использованием рекомендую удалить в Mqtt брокере все топики с r4s, для чего выбрать в настройках **Delete Mqtt topics**. Если же система не поддерживает Mqtt Discovery, придется разбираться с Mqtt.  
<details>
<summary>Подробнее по Mqtt...</summary>
     У меня шлюз подключен к ioBroker. Мои настройки MQTT брокера ниже на картинке 3.<br>

![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/mymqtt.jpg)
Картинка 3. Мои Mqtt настройки.<br><br>
     
&emsp; Снят флаг retain, чтобы брокер не запоминал, а считывал состояние устройств при соединении. В Home Assistant  установленный в нем и/или Mqtt брокере флаг retain [может приводить к самопроизвольному включению и выключению устройства](https://mjdm.ru/forum/viewtopic.php?f=8&t=5501&sid=de6b1e2b43f25c8d9ae9af5673ee9417&start=140#p121604).  Также установлен флаг публикации при подписке, что позволяет не вводить все топики вручную. Иногда при публикации сразу большого числа подписок ioBroker почему-то делает некоторые из них с защитой от записи :-), есть у меня такой глюк. Приходится их удалять и перезапускать Mqtt адаптер, чтобы они появились опять.<br>

#### Mqtt топики для чайника (см. картинку 4 ниже):

`r4s0/devaddr/cmd/boil` <-- `0/off/false` - выключение кипячения, `1/on/true` - включение кипячения. Если перед этим был включен подогрев, включается кипячение с последующим подогревом;<br>
`r4s0/devaddr/cmd/heat` <-- `0/off/false` - выключение подогрева, `1/on/true` - включение подогрева с последней запомненной шлюзом температурой. При старте шлюза температура берется из поля Heat в настройках;<br>
`r4s0/devaddr/cmd/heat_temp` <-- `30...90` - включение подогрева, `> 97` - выключение, `< 30` - выключение, если подогрев был включен, последняя температура если был выключен;<br>
`r4s0/devaddr/cmd/boiltime` <-- `-5...5` - время кипения чайника;<br>
`r4s0/devaddr/cmd/nightlight` <-- `0/off/false` - выключение ночника, `1/on/true` - включение ночника;<br>
`r4s0/devaddr/cmd/nightlight_red` <-- `0..255` Уровень красного в ночнике;<br>
`r4s0/devaddr/cmd/nightlight_green` <-- `0..255` Уровень зеленого в ночнике;<br>
`r4s0/devaddr/cmd/nightlight_blue` <-- `0..255` Уровень синего в ночнике;<br>
`r4s0/devaddr/rsp/` - текущее состояние, температура, rssi и т.д.;<br>

> Значения уровней запоминаются в шлюзе и передаются на чайник при включении подсветки.
<br>

![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/mymqtt1.jpg)
Картинка 4. Мои Mqtt топики для чайника.

#### Mqtt топики для мультиварки (см. картинку 5 ниже):

`r4s0/devaddr/cmd/state` <-- `0/off/false` - выключение, `1/on/true` - старт программы или подогрев, если программа не установлена;<br> 
`r4s0/devaddr/cmd/prog` <-- номер программы 1-12, 0 - выключение;<br>
     Программы:<br>
     1 - Rice / Рис Крупы, 2 - Slow Cooking / Томление, 3 - Pilaf / Плов, 4 - Frying / Жарка;<br>
     5 - Stewing / Тушение, 6 - Pasta / Паста, 7 - Milk Porridge / Молочная каша, 8 - Soup / Суп;<br>
     9 - Yogurt / Йогурт, 10 - Baking / Выпечка, 11 - Steam / Пар, 12 - Hot / Варка Бобовые;<br>
`r4s0/devaddr/cmd/mode` <-- режим: 1 - овощи, 2 - рыба, 3 - мясо для программ 4,5,11<br>
`r4s0/devaddr/cmd/temp` <-- температура;<br>
`r4s0/devaddr/cmd/set_hour` <-- время работы программы, часы;<br>
`r4s0/devaddr/cmd/set_min` <-- время работы программы, минуты;<br>
`r4s0/devaddr/cmd/delay_hour` <-- время работы программы плюс задержка до старта программы, часы;<br>
`r4s0/devaddr/cmd/delay_min` <-- время работы программы плюс задержка до старта программы, минуты;<br>
`r4s0/devaddr/cmd/warm` <-- подогрев после завершения программы;<br>

> Параметры `delay_hour` и `delay_min` запоминаются в шлюзе и передаются при установке программы, режима или подогрева, а потому устанавливаются перед установкой программы, режима или автоподогрева. При выборе программы устанавливаются температура и время работы программы по умолчанию, после установки mode еще раз корректируются. После установки программы и режима можно при необходимости скорректировать время и температуру. Этот порядок установки параметров обусловлен тем, что по Mqtt нельзя сразу установить одной командой все параметры, если они находятся в разных топиках. При установке же через web все параметры ставятся одной командой. И значения температуры и времени по умолчанию для каждой программы и режима устанавливаются через web только если перед этим они были  равны 0. Программа мультиповар пока не поддерживается, я не вижу смысла. При записи нуля в prog на мультиварку посылается команда выключения, что полезно для сброса программы.
<br>

![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/mymqtt2.jpg)
 Картинка 5. Мои Mqtt топики для мультиварки.

#### Mqtt топики для кофеварки:

`r4s0/devaddr/cmd/state` <-- `0/off/false` - выключение, `1/on/true` - включение;<br>
`r4s0/devaddr/cmd/delay_hour` <-- время отложенного старта, часы;<br>
`r4s0/devaddr/cmd/delay_min` <-- время отложенного старта, минуты;<br>
`r4s0/devaddr/cmd/delay` <-- запуск отложенного старта, `0/off/false` - выключение, `1/on/true` - включение;<br>
`r4s0/devaddr/cmd/lock` <-- блокировка, `0/off/false` - выключение, `1/on/true` - включение;<br>
`r4s0/devaddr/cmd/strength` <-- крепость, `0/off/false` - выключение, `1/on/true` - включение;<br>
`r4s0/devaddr/rsp/` - текущее состояние, rssi и т.д.;<br>

> Значения времени отложенного старта запоминаются в шлюзе и передаются на кофеварку при включении этого режима.

#### Mqtt топики для розетки:

`r4s0/devaddr/cmd/state` <-- `0/off/false` - выключение, `1/on/true` - включение;<br>
`r4s0/devaddr/cmd/lock` <-- блокировка, `0/off/false` - выключение, `1/on/true` - включение;<br>
`r4s0/devaddr/rsp/` - текущее состояние, rssi и т.д.;

&emsp; Начиная с версии 2020.10.27 появилась возможность использовать совмещенные топики для команд и ответов. Опция включается в настройках. Мне это пригодилось при интеграции устройств в Google Home с помощью драйвера iot iobroker-а. Как я понял, этот драйвер не принимает раздельные топики команд/ответов. Кроме того, так как Google Home понимает `true / false` вместо `ON / OFF`, то нужно в настройках драйвера iot `Conversation to Google Home = function (value) {}` ввести строку вида `switch (value) {case **ON**: return true ; break; default: return false;}`. Если же автозамена недоступна, то начиная с версии 2020.11.07 можно использовать опцию `**true / false** Response`. Опция не работает совместно с Hass Discovery, там она не нужна.<br>
</details>

#### Веб интерфейс

Устройствами можно управлять также и в веб интерфейсе шлюза. Примеры главной страницы и страницы настроек ниже на картинках 6 и 7.

![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/myweb.jpg) 
Картинка 6. Главная страничка.

![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/myweb1.jpg) 
Картинка 7. Страничка настроек.
 
## 4. BLE Monitor
&emsp;Монитор позволяет отслеживать метки(маяки) со статическими MAC адресами. Выводится наличие/отсутствие метки(маяка) и rssi.<br>
**Дополнительно поддерживаются:**
* BLE маяки приложения Home Assistant на смартфонах (привязка по uuid, тайм-аут у меня 60 секунд)
* Термометры Xiaomi LYWSD03MMC с оригинальной прошивкой, [прошивкой от atc1441 в режиме custom](https://github.com/atc1441/ATC_MiThermometer) и [прошивкой от pvvx в режиме custom и Mija](https://github.com/pvvx/ATC_MiThermometer). Ключи для LYWSD03MMC оригинальной версии можно взять из [облака Xiaomi](https://github.com/PiotrMachowski/Xiaomi-cloud-tokens-extractor?tab=readme-ov-file). (400 секунд)
* Термометры Xiaomi LYWSDCGQ (400 секунд)
* Часы с термометром Xiaomi LYWSD02 (400 секунд) 
* Весы Xiaomi Mi Scale  (400 секунд)
* Qingping Air Monitor Lite(CGDN1) (400 секунд)
* Счетчики воды и газа Elehant  (400 секунд)
* Samsung Smart Tag (120 секунд).
     
&emsp;Для активации монитора нужно во вкладке **Setting** установить в опции **BLE Monitoring** значения **Active** или **Passive** для активного или пассивного сканирования и нажать **Save setting**. Активный сканер дает больше информации, но расходует больше энергии на сканируемых устройствах. Для сканирования меток рекомендуется пассивный режим. Нужно учитывать, что в режиме **Auto** при поиске устройств перед соединением сканер всегда работает в активном режиме, а потом переходит в пассивный режим.<br> 
&emsp; После установки опции в меню появится вкладка **BLE Monitor**, открыв которую, можно увидеть найденные устройства. В поле **Gap** выводится временной интервал между двумя последними пришедшими пакетами, в поле **Last** время с момента прихода последнего пакета. Тайм-аут по умолчанию (если в поле **Timeout** пусто) 300 секунд, после чего устройство считается потерянным, а его данные удаляются из таблицы. В дальнейшем эта строка может быть перезаписана данными с другого устройства. **Для вывода данных в Mqtt в поле Timeout нужно ввести ненулевое значение и подтвердить ввод, нажав Ok**. Все значения сохранятся в энергонезависимой памяти, а Mqtt Discovery передаст все в Home Assistant. Хотя сканирование идет постоянно, но при установке значения **Timeout** нужно учитывать, что для поддержания соединений тоже нужно время, в течение которого возможны пропуски пакетов. Лишние данные в Mqtt и Home Assistant можно удалить, выбрав при включенном BLE мониторе во вкладке **Setting** опцию **Delete Mqtt topics** и нажав **Save setting**.<br>
&emsp;Samsung Smart Tag, не привязанные к аккаунту SmartThings, для отслеживания не пригодны, так как через несколько минут отключаются. Рекламное сообщение привязанного к аккаунту Smart Tag содержит статический UUID 0xFD5A, динамический MAC адрес и шифрованный идентификатор, из-за наличия в нем RND байт меняющийся вместе с MAC адресом. Остальные поля (статус, счетчик рекламных сообщений, регион, состояние батареи) не уникальны. Рекламное сообщение содержит также цифровую подпись. Стандартные BLE трекеры, насколько мне известно, способны опознать наличие этих меток по UUID, но не способны однозначно идентифицировать каждую метку, если их больше одной. Шлюз использует проверку цифровой подписи рекламного сообщения для идентификации этих меток(маяков), что требует ввода ключа. После ввода ключ проверяется и, если все нормально, нужно ввести значение тайм-аута. Только после ввода тайм-аута и нажатия **Ok** ключ и тайм-аут запоминаются в NVS (Non-volatile storage - энергонезависимая память).
<details>
<summary>Как получить ключ...</summary>

&emsp; Ключ (Signing Key) представляет собой ASCII строку из 64 символов (это 32 байта в шестнадцатеричном виде, 16 байт ключа шифрования AES128CBC и 16 байт начального вектора).  Действует от момента привязки Smart Tag к аккаунту SmartThings и до момента отвязки от аккаунта или возврата устройства к заводским установкам. Генерируется и меткой Smart Tag,  и сервером SmartThings, по каналу связи не передается. Первые 16 байт используются шлюзом как идентификатор устройства в системе умного дома. Для генерации ключа нужно записать лог Bluetooth HCI в момент привязки Smart Tag к аккаунту SmartThings. Если метка уже привязана, перед записью лога ее нужно удалить из аккаунта. Для привязки к аккаунту нужно устройство Galaxy c версией android не ниже 8.0. Я использовал Galaxy S7. Прежде всего нужно включить режим разработчика. Открываем **Настройки > Сведения о телефоне > Сведения о ПО** и 8 (кажется) раз нажимаем номер сборки. Может потребоваться ввести пин код телефона. В настройках должно появиться меню **Параметры разработчика**. Заходим в меню и включаем **Журнал отслеживания Bluetooth**. Далее я на всякий случай выключал и включал Bluetooth, затем перегружал телефон. Заходим в приложение **SmartThings** и добавляем устройство **Smart Tag**. Затем опять идем в **Настройки > Параметры разработчика** и выбираем **Создать отчет об ошибках > Интерактивный отчет**. Через некоторое время придет уведомление о созданном отчете. Далее его нужно сохранить на компьютере с windows. Я выбирал сохранить в приложении telegram в папке **Избранное**, а затем уже в telegram на компьютере сохранил архив. Затем нужно извлечь из архива (папка **FS/data/log/bt/**) файлы **btsnoop_hci.log** и **btsnoop_hci.log.last**. В одном из этих файлов должен быть лог привязки. Далее в папку с этими файлами загружаем архив с [**консольной утилитой stsk**](https://github.com/alutov/ESP32-R4sGate-for-Redmond/raw/master/utils/stsk.zip) и достаем программу из архива. На всякий случай отмечу, что вирусов в архиве нет, а размер программы 28160 байт. Открываем командную строку windows, заходим в папку с файлами и набираем **stsk btsnoop_hci.log** и **stsk btsnoop_hci.log.last**. Программа выведет найденные Smart Tag и сгенерирует ключи для них. Последний ключ будет скопирован в буфер обмена windows:<br>
     
![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/stsk.jpg) 
Картинка 8. Программа stsk.

&emsp;  Программа не обрабатывает разбитые на части пакеты данных, соответственно, если встречает такой пакет, то не находит в логе ничего или выдает ошибку. Столкнулся с этим пока только один раз на s21. Возможно это просто сбой, т.к wireshark тоже не совсем корректно восстановил сбойный пакет. И пока не нашел алгоритма опознания и сборки таких пакетов данных. В этом случае нужно повторить всю процедуру еще раз.<br> 
</details>


![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/blemon1.jpg) 
Картинка 9. Страничка BLE Monitor.
     
&emsp;Добавлено определение лучшего сигнала от метки(маяка) среди нескольких шлюзов. Алгоритм работает так. Каждый шлюз мониторит топик **r4s/DevId/rssi**. Если у него сигнал от метки(маяка)  с большим уровнем, он прописывает свой уровень в этот топик, а также свой номер в топик **r4s/DevId/gtnum**. После чего шлюз периодически, раз в 6 секунд сохраняет свой уровень сигнала в топике, то есть становится ведущим. Остальные шлюзы проверяют уровень и есть ли его обновление. Если какой-то шлюз обнаруживает отсутствие обновления уровня более 30 секунд, или же его уровень больше,  он становится ведущим. Лучший уровень и номер шлюза можно увидеть во второй строчке RSSI на странице BLE монитора. В сущностях каждого устройства тоже есть лучший уровень и номер шлюза:<br>
![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/blemon2.jpg)      
Картинка 10. Сущности метки(маяка) в Home Assistant.

## 5. Поддержка экрана

&emsp;В первой версии шлюза оставался запас как оперативной (~100кБайт), так и программируемой (~400кБайт) свободной памяти, что позволяло расширить возможности прошивки, в частности, добавить поддержку экрана. К тому же у меня уже была собранная esp32 с экраном [3.2" 320x240 на чипе ili9341](https://www.aliexpress.com/item/32911859963.html?spm=a2g0s.9042311.0.0.274233edzZnjSp), работающая с прошивкой с сайта [wifi-iot](https://wifi-iot.com/). Возможно и использование для шлюза уже готовых устройств на чипах ili9341, ili9342 или ST7789. В шлюзе я использовал только необходимые процедуры из [Bodmer](https://github.com/Bodmer/TFT_eSPI), адаптированные не совсем хорошо, но как есть для esp-iot. Пины для поключения экрана по умолчанию: MOSI-23, MISO-25, CLK-19, CS-16, DC-17, RST-18, LED-21. Пины можно переназначить в настройках. Если PWR, RST, LED установить 0, то шлюз эти пины использовать не будет. Есть также опция поворота экрана на 180&deg;, а также возможность регулировки яркости дисплея по Mqtt, иcпользуя топик **r4s/screen**. Программа проверяет наличие экрана на шине SPI при старте. Предусмотрена возможность вывода на экран и картинки в формате jpeg. Для этого нужно указать url картинки. У моей камеры url такой: **http://192.168.1.7/auto.jpg?usr=admin&pwd=admin**. Картинка грузится в буфер размером 20-65 килобайт в оперативной памяти. Время обновления и размер буфера можно установить в настройках. Есть возможность загрузки картинки по https. Проверка сертификата отключена. Есть возможность управления параметрами загрузки до 4 картинок по Mqtt, используя топики **r4sx/jpg_url1...r4sx/jpg_url4** и **r4sx/jpg_time**. Для очистки url картинки в соответствующий топик нужно ввести символ **#**. Если в Mqtt эти топики не прописаны, а также после сохранения настроек, эти параметры копируются из настроек в Mqtt. Установка нулевого интервала обновления возвращает кота на экран. Длина буфера ссылки пока 384 байта. Добавлены загрузка и отображение в текстовом виде погоды с сайта wttr.in. В принципе, это может любой сайт, отдающий текст и допускающий форматирование. Если ссылка не содержит строки **http://** или **https://**, то шлюз считает это сообщение простым текстом и отображает на экране. Доступно 2 шрифта и 10 вариантов цветов. Управляющие символы: \ \ или \n - перевод строки, \F  - шрифт 26 пикселей и перевод строки, \f - шрифт 16 пикселей и перевод строки, \0 ... \9 - цвета. Поддерживается кириллица, проверял, правда, только из mosquitto. Он поддерживает юникод по факту, как другие брокеры, не знаю. Пример вывода изображения на экран на картинке 11.
<br>

![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/mytft3.jpg)
Картинка 11. Изображение.<br><br>

&emsp;На картинке 12 пример вывода на экран Mqtt погоды с сайта wttr.in:
<td>https://wttr.in/Донецк?format=\F\6+%25l%20\\\4Темп:+\0%25t(%25f)\\\4Давл:\0+%25P\\\4Влажн:\0+%25h\\\6+%25c+%25w+UV:+%25u\f\4Восход:\0+%25D+\4Закат:\0+%25d</td><br> 

![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/mytft10.jpg)
Картинка 12. Погода.<br><br>

&emsp;На картинке 13 пример вывода на экран Mqtt строки (символ градуса можно вывести на экран используя обратный апостроф):
<td>\F\0` English \1color \2text\3 example\n\4Русский \5цветной \6текст\n\7text1 \8text2 \9text3\f\0` English \1color \2text\3 example\n\4Русский \5цветной \6текст\n\7text1 \8text2 \9text3</td><br>    
  
![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/mytft9.jpg)
Картинка 13. Текст.<br><br>

&emsp; Стоит отметить, что сама TFT плата влияет на распространение как WiFi, так и BLE. И даже если антенна esp32 выглядывает из-под экрана, чувствительнось такого бутерброда заметно меньше обычной esp32. Рекомендую использовать с экраном вариант esp32 с внешней антенной. У меня в шлюзе с экраном замена esp32 на вариант с разъемом и установка внешней антенны дала прирост уровней WIFI и BLE примерно на 15-20dBm.<br>
&emsp; Если же экран не нужен, то нужно после программирования и настройки  esp32 подсоединить ее к источнику питания и спрятать где-нибудь на кухне.<br>

## 6. Совместимые устройства
Если хочется запустить шлюз максимально быстро, без пайки, да еще и с приличным корпусом, стоит присмотреться к совместимым устройствам. Их нужно только перепрограммировать. Ниже перечислены только проверенные мной устройства. Для прошивки использовалась программа [flash_download_tools](https://www.espressif.com/en/support/download/other-tools).

#### [TTGO T-Watcher](http://www.lilygo.cn/prod_view.aspx?TypeId=50033&Id=1160) (LILYGO® TTGO T4 в корпусе).<br>
![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/mytft4.jpg)
<br>Картинка 14. TTGO T-Watcher.<br><br>
Я проверял работоспособность шлюза на TTGO T4 версии 1.3. Прошивается он через встроенный USB разъем, перед прошивкой устройства нужно соединить контакты 6 и 7 (gpio0 и gnd) в нижнем ряду разъема (картинка 15). Возможна прошивка и без установки перемычек, зависит от программы. Настройки экрана для версии 1.3: 12-MISO, 23-MOSI, 18-CLK, 27-CS, 32-DC, 5-RES, 4-LED, 0-PWR, и для версии 1.2: 12-MISO, 23-MOSI, 18-CLK, 27-CS, 26-DC, 5-RES, 4-LED, 0-PWR. В версии 1.2 нет управления включением и выключением экрана. Кнопки сверху вниз 38-Port1, 37-Port2, 39-Port3. Шина I2C: SCL-22, SDA-21.<br>
![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/mytft5.jpg)
<br>Картинка 15. Соединить 6 и 7 пины разъема перед прошивкой TTGO T-Watcher.<br><br>

#### [M5Stack BASIC Kit](https://docs.m5stack.com/en/core/basic)<br>
![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/mytft6.jpg)
<br>Картинка 16. M5Stack BASIC Kit.<br><br>
Как я понял, старые версии M5Stack Basic шли с экраном на ili9341, и на этих версиях [работала и старая версия шлюза](https://github.com/alutov/ESP32-R4sGate-for-Redmond/issues/16). 
Настройки экрана для этой версии: 19-MISO, 23-MOSI, 18-CLK, 14-CS, 27-DC, 33-RES, 32-LED, 0-PWR. Новые версии уже идут с экраном на ili9342. Начиная с версии 2021.10.29 добавлена поддержка экрана на ili9342. Я проверял работоспособность шлюза на новой версии M5Stack BASIC Kit. Прошивается он через встроенный USB разъем, перед прошивкой устройства нужно соединить последний контакт в верхнем ряду и 4 в нижнем ряду (gnd и gpio0) разъема (картинка 17). Возможна прошивка и без установки перемычек, зависит от программы. Настройки экрана для новой версии: 23-MISO, 23-MOSI, 18-CLK, 14-CS, 27-DC, 33-RES, 32-LED, 0-PWR. Кнопки слева направо 39-Port1, 38-Port2, 37-Port3. Шина I2C: SCL-22, SDA-21.<br>
Настройки для [**M5Stack Tough**](https://docs.m5stack.com/en/core/tough): 23-MISO, 23-MOSI, 18-CLK, 5-CS, 15-DC, 44-RES, 47-LED, 46-PWR. Шина I2C: SCL-22, SDA-21. Без I2C экран не запустится.<br>  
     
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
