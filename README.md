# ESP32 Ready4Sky (R4S) шлюз для устройств Redmond+
![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/mytft2.gif)

#### Открыта группа в телеграме "ESP32 Ready4Sky (R4S)".
&emsp; Текущая версия 2021.07.11. После кипячения чайник Mikettle переводится в режим поддержания температуры, что позволяет повторно включать кипячение без снятия с подставки. Добавлен чайник M170S. После обновления нужно проверить и при необходимости поправить тип чайника. При обновлении закрываются все BLE cоединения. Изменил параметры скана и добавил строку состояния BLE. При соединении показывает 2 этапа: open - установление соединения, auth - чтение характеристик устройства и авторизация. Немного поправил wifi, теперь пытается восстановить соединение 10 раз до рестарта. Уменьшил до 256 минут время сброса счетчика warming Mikettle. Вроде бы нашел в коде [причину зависания BLE при сканировании](https://github.com/espressif/esp-idf/issues/6688). Зависаний BLE больше не замечал. Мониторинг состояния BLE сделан  опцией "BLE monitoring" и по умолчанию выключен, сейчас я им не пользуюсь. Если он включен, поведение прошивки аналогично старой версии, то есть рестарт при потере соединения более минуты и ошибке скана. Добавлена поддержка мультиварки RMC-M903S и частично духового шкафа RO-5707S, за исключением режимов блокировки и автоподогрева. 
В RMC-M800S добавлена программа мультиповар. В режиме RMC-M903S можно попробовать и другие мультиварки. Отключена проверка состояния BLE  соединений во время обновления. На экране вместо влажности в помещении выводится напряжение и ток из [ZMAi-90](https://github.com/alutov/zmai-90_tywe3s_mqtt_gateway).<br>

### ENG<br>
[Use Google to translate into English.](https://translate.google.com/translate?hl=ru&sl=ru&tl=en&u=https%3A%2F%2Fgithub.com%2Falutov%2FESP32-R4sGate-for-Redmond%2Fblob%2Fmaster%2FREADME.md)<br>
### RUS<br>
&emsp; ESP32 r4sGate позволяет подключать BLE-совместимые устройства Redmond и чайники Xiaomi MiKettle к системе "умный дом" по протоколу MQTT. Поддерживаются 3 BLE соединения. Управление Xiaomi MiKettle возможно только из режима "keep warm". В этом режиме чайник поддерживает заданную шлюзом минимальную температуру 40&deg;C с гистерезисом примерно 4&deg;C, то есть при 36&deg;C подогрев включается, а при 44&deg;C отключается. Доступно включение и выключение кипячения (state = ON/OFF), установка температуры подогрева (heat_temp = 40...95). Можно перевести чайник в режим Idle (heat_temp = 0). Последняя команда выполняется с задержкой. После выполнения команды дальнейшее управление чайником недоступно. В отличие от выключения сенсором "warm" на чайнике при дальнейшем выключении и включении чайник возвращается в режим "keep warm". Возможно, это особенность конкретной версии MCU 6.2.1.9. Пока оставил так и включил чайник через редмондовскую розетку. Если ее выключить и опять включить, чайник переходит в режим подогрева. Осталось найти возможность рестарта контроллера QN9020 чайника по BLE и будет полноценное включение. 😉 Все необходимые параметры чайника шлюз устанавливает самостоятельно, а родное приложение пригодится для обновления прошивки. Время подогрева установлено на 12 часов (720 минут), после 256 минут шлюз сбрасывает счетчик кратковременно включая и выключая кипячение.<br>
[Список поддерживаемых устройств текущей версии:](https://github.com/alutov/ESP32-R4sGate-for-Redmond/issues/12)<br>
**Электрочайник**<br>
Redmond SkyKettle M170S<br>
Redmond SkyKettle M171S<br>
Redmond SkyKettle M173S / RTP-M810S<br>
Redmond SkyKettle G200S<br>
Redmond SkyKettle G210S<br>
Redmond SkyKettle G211S<br>
Redmond SkyKettle G212S<br>
Redmond SkyKettle G216S<br>
Redmond SkyKettle G240S<br>
Xiaomi MiKettle YM-K1501(Int) - ProductId 275<br>
Xiaomi MiKettle YM-K1501(HK) - ProductId 131<br>
Xiaomi MiKettle V-SK152(Int) - ProductId 1116<br>

**Мультиварка**<br>
Redmond SkyCooker RMC-M800S<br>
Redmond SkyCooker RMC-M903S<br>
**Кофеварка**<br>
Redmond SkyCoffee M1519S<br>
**Розетка**<br>
Redmond SkyPort 103S<br>
**Конвектор электрический**<br>
Redmond SkyHeat 4529S<br><br>
&emsp; Для запуска шлюза нужно запрограммировать ESP32. Файл fr4sGate.bin в папке build это уже собранный бинарник для esp32 @160MHz с памятью 4 Мбайт, DIO загрузчиком и прошивается одним файлом с адреса 0x0000 на чистую esp32. Вместо него также можно использовать три стандартных файла для перепрошивки: bootloader.bin (адрес 0x1000),  partitions.bin (адрес 0x8000) и r4sGate.bin (адрес 0x10000). Файл r4sGate.bin можно также использовать для обновления прошивки через web интерфейс. Если же DIO загрузчик не стартует, можно использовать файл fqr4sGate.bin с загрузчиком QIO.<br>
&emsp; Затем нужно создать гостевую сеть Wi-Fi в роутере с ssid "r4s" и паролем "12345678", подождать, пока esp32 не подключится к нему, ввести esp32 IP-адрес в веб-браузере и во вкладке Setting установить остальные параметры. После чего гостевая сеть больше не нужна. Esp32 будет пытаться подключиться к сети "r4s" только при недоступности основной сети, например, при неправильном пароле. Если не удается подключиться и к гостевой сети, esp32 перезагружается. Вариант с гостевой сетью в отличие от общепринятой практики запуска точки доступа на esp32, как мне кажется, удобнее, так как позволяет настраивать все с компьютера не тыкая пальцами в смартфон, который при отсутствии инета так и норовит соскочить с esp32. Но, главное, в случае падения по каким-то причинам Wi-Fi роутера (а он может быть выделенным только для iot устройств) остальной Wi-Fi не засоряется дружно "вплывшими" esp32.<br>
&emsp; Далее нужно ввести имя Redmond устройства и привязать его к шлюзу. Поиск устройств запускается только тогда, когда есть хотя бы одно определенное, но не соединенное устройство. Если имя устройства точно не известно (а редмонды не всегда светятся по BLE как модель один в один), то для начала сканирования нужно ввести в поле "Name" в настройках любое имя, а потом заменить его найденным (строка "BLE last found device name") при сканировании и выбрать ближайший тип устройства. Далее для привязки нужно нажать и удерживать кнопку "+" на чайнике или "таймер" на мультиварке  до тех пор, пока устройство не войдет в режим привязки и через некоторое время соединится со шлюзом.<br> 
&emsp; Предусмотрена возможность подключения к одному MQTT серверу нескольких шлюзов. Для этого нужно в каждом шлюзе установить свой r4sGate Number. Шлюз с номером 0 будет писать в топик r4s/devaddr/..., шлюз с номером 1 - r4s1/devaddr/... и т.д. Нужно только учесть, что запрос на авторизацию при привязке зависит от номера шлюза и от номера соединения в шлюзе. Это позволяет привязать 2 одинаковых чайника или мультиварки к 2 разным шлюзам или к 2 разным соединениям в пределах одного шлюза. У меня шлюз подключен к ioBroker. Мои настройки MQTT брокера ниже на картинке 1.<br><br>
![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/mymqtt.jpg)
Картинка 1. Мои Mqtt настройки.<br><br>
&emsp; Снят флаг retain, чтобы брокер не запоминал, а считывал состояние устройств при соединении. В Home Assistant  установленный в нем и/или Mqtt брокере флаг retain [может приводить к самопроизвольному включению и выключению устройства](https://mjdm.ru/forum/viewtopic.php?f=8&t=5501&sid=de6b1e2b43f25c8d9ae9af5673ee9417&start=140#p121604). Для исключения ошибок и упрощения конфигурации Home Assistant начиная с версии 2020.10.23 можно использовать опцию шлюза Hass Discovery. Перед ее использованием рекомендую удалить в Mqtt брокере все топики с r4s. Также установлен флаг публикации при подписке, что позволяет не вводить все топики вручную. Иногда при публикации сразу большого числа подписок ioBroker почему-то делает некоторые из них с защитой от записи :-), есть у меня такой глюк. Приходится их удалять и перезапускать Mqtt адаптер, чтобы они появились опять.<br>
&emsp; Mqtt топики для чайника (см. картинку 2 ниже):<br>
r4s/devaddr/cmd/state <-- 0/off/false - выключение, 1/on/true - кипячение, 2...100 - кипячение и подогрев;<br>
r4s/devaddr/cmd/heat_temp <-- 0...39 - выключение, 40...90 подогрев, > 90 - кипячение;<br>
r4s/devaddr/cmd/nightlight <-- 0/off/false - выключение ночника, 1/on/true - включение ночника;<br>
r4s/devaddr/cmd/nightlight_red <- 0..255 Уровень красного в ночнике;<br>
r4s/devaddr/cmd/nightlight_green <- 0..255 Уровень зеленого в ночнике;<br>
r4s/devaddr/cmd/nightlight_blue <- 0..255 Уровень синего в ночнике;<br>
r4s/devaddr/rsp/ - текущее состояние, температура, rssi и т.д.;<br>
Значения уровней запоминаются в шлюзе и передаются на чайник при включении подсветки.<br><br>
![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/mymqtt1.jpg)
Картинка 2. Мои Mqtt топики для чайника.<br><br> 
&emsp; Mqtt топики для мультиварки (см. картинку 3 ниже):<br>
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
&emsp; Параметры delay_hour и delay_min запоминаются в шлюзе и передаются при установке программы, режима или подогрева, а потому устанавливаются перед установкой программы, режима или автоподогрева. При выборе программы устанавливаются температура и время работы программы по умолчанию, после установки mode еще раз корректируются. После установки программы и режима можно при необходимости скорректировать время и температуру. Этот порядок установки параметров обусловлен тем, что по Mqtt нельзя сразу установить одной командой все параметры, если они находятся в разных топиках. При установке же через web все параметры ставятся одной командой. И значения температуры и времени по умолчанию для каждой программы и режима устанавливаются через web только если перед этим они были  равны 0. Программа мультиповар пока не поддерживается, я не вижу смысла. При записи нуля в prog на мультиварку посылается команда выключения, что полезно для сброса программы.<br><br>
![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/mymqtt2.jpg)
 Картинка 3. Мои Mqtt топики для мультиварки.<br><br> 
&emsp; Mqtt топики для кофеварки:<br>
r4s/devaddr/cmd/state <-- 0/off/false - выключение, 1/on/true - включение;<br>
r4s/devaddr/cmd/delay_hour <-- время отложенного старта, часы;<br>
r4s/devaddr/cmd/delay_min <-- время отложенного старта, минуты;<br>
r4s/devaddr/cmd/delay <-- запуск отложенного старта, 0/off/false - выключение, 1/on/true - включение;<br>
r4s/devaddr/cmd/lock <-- блокировка, 0/off/false - выключение, 1/on/true - включение;<br>
r4s/devaddr/cmd/strength <-- крепость, 0/off/false - выключение, 1/on/true - включение;<br>
r4s/devaddr/rsp/ - текущее состояние, rssi и т.д.;<br>
Значения времени отложенного старта запоминаются в шлюзе и передаются на кофеварку при включении этого режима.<br><br>
&emsp; Mqtt топики для розетки:<br>
r4s/devaddr/cmd/state <-- 0/off/false - выключение, 1/on/true - включение;<br>
r4s/devaddr/cmd/lock <-- блокировка, 0/off/false - выключение, 1/on/true - включение;<br>
r4s/devaddr/rsp/ - текущее состояние, rssi и т.д.;<br><br>
&emsp; Начиная с версии 2020.10.27 появилась возможность использовать совмещенные топики для команд и ответов. Опция включается в настройках. Мне это пригодилось при интеграции устройств в Google Home с помощью драйвера iot iobroker-а. Как я понял, этот драйвер не принимает раздельные топики команд/ответов. Кроме того, так как Google Home понимает "true / false" вместо "ON / OFF", то нужно в настройках драйвера iot "Conversation to Google Home = function (value) {}" ввести строку вида "switch (value) {case "ON": return true ; break; default: return false;} ". Если же автозамена недоступна, то начиная с версии 2020.11.07 можно использовать опцию ""true / false" Response". Опция не работает совместно с Hass Discovery, там она не нужна.<br>
&emsp; Устройствами можно управлять также и в веб интерфейсе шлюза. Примеры главной страницы и страницы настроек ниже на картинках 4 и 5.

**Веб интерфейс, главная страничка**:
![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/myweb.jpg) 
Картинка 4. Главная страничка.<br><br>
**Веб интерфейс, страничка настроек**:
![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/myweb1.jpg) 
Картинка 5. Страничка настроек.<br><br>
 
#### Поддержка экрана<br>
&emsp; В первой версии шлюза оставался запас как оперативной (~100кБайт), так и программируемой (~400кБайт) свободной памяти, что позволяло расширить возможности прошивки, в частности, добавить поддержку экрана. К тому же у меня уже была собранная esp32 с экраном [3.2" 320x240 на чипе ili9341](https://www.aliexpress.com/item/32911859963.html?spm=a2g0s.9042311.0.0.274233edzZnjSp), работающая с прошивкой с сайта [wifi-iot](https://wifi-iot.com/). Возможно и использование для шлюза уже готовых устройств на чипе ili9341, например [TFT BTC Ticker](https://aliexpress.ru/item/1005001358148811.html?spm=a2g0o.productlist.0.0.451e6dbe9z6WOU&algo_pvid=775d439e-9930-454b-91cf-e1f5c1404bf2&algo_expid=775d439e-9930-454b-91cf-e1f5c1404bf2-4&btsid=0b8b036316061501449102110e230e&ws_ab_test=searchweb0_0,searchweb201602_,searchweb201603_) или [m5 stack basic](https://m5stack.com/products/basic-core-iot-development-kit). В шлюзе я использовал только необходимые процедуры из [Bodmer](https://github.com/Bodmer/TFT_eSPI), адаптированные не совсем хорошо, но как есть для esp-iot. Пины для поключения экрана по умолчанию: MOSI-23, MISO-25, CLK-19, CS-16, DC-17, RST-18, BACKLIGHT-21. TOUCH CS пока не используется, но подключен к 33 пину, на котором постоянно высокий уровень. Пины можно переназначить в настройках. Есть также опция поворота экрана на 180&deg;, а также возможность включения и выключения дисплея по Mqtt, иcпользуя топик r4s/screen. Программа проверяет наличие экрана на шине SPI при старте. Получилось что-то похожее на часы. На экран выводится текущее время, день и месяц, а также температура в и влажность в доме, состояние (синий- выкл., красный - вкл.) и температура на выходе котла, температура и влажность на улице. Все берется с Mqtt. Также отображается состояние и некоторые параметры подключенных Ble устройств. Пример экрана на картинке 6.<br><br>
![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/mytft.jpg)
Картинка 6. Экран шлюза 1.<br><br>
&emsp; Предусмотрена возможность вывода на экран и картинки в формате jpeg 320x176. Размер картинки будет около 20 кБайт. Для этого нужно указать url картинки. У моей камеры url такой: http://192.168.1.7/auto.jpg?usr=admin&pwd=admin. Картинка грузится в буфер размером 32768 байт в оперативной памяти. Время обновления можно установить в настройках. Пример экрана на картинке 7. <br><br>
 ![PROJECT_PHOTO](https://github.com/alutov/ESP32-R4sGate-for-Redmond/blob/master/jpg/mytft3.jpg)
Картинка 7. Экран шлюза 2.<br><br>

&emsp; Стоит отметить, что сама TFT плата влияет на распространение как WiFi, так и BLE. И даже если антенна esp32 выглядывает из-под экрана, чувствительнось такого "бутерброда" заметно меньше обычной esp32. Рекомендую использовать с экраном вариант esp32 с внешней антенной. У меня в шлюзе с экраном замена esp32 на вариант с разъемом и установка внешней антенны дала прирост уровней WIFI и BLE примерно на 15-20dBm.<br>
&emsp; Если же экран не нужен, то нужно после программирования и настройки  esp32 подсоединить ее к источнику питания и спрятать где-нибудь на кухне.<br>
# Сборка проекта
&emsp; Для сборки бинарных файлов использован [Espressif IoT Development Framework.](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)<br>
