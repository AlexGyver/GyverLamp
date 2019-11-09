/*
  Скетч к проекту "Многофункциональный RGB светильник"
  Страница проекта (схемы, описания): https://alexgyver.ru/GyverLamp/
  Исходники на GitHub: https://github.com/AlexGyver/GyverLamp/
  Нравится, как написан код? Поддержи автора! https://alexgyver.ru/support_alex/
  Автор: AlexGyver, AlexGyver Technologies, 2019
  https://AlexGyver.ru/
*/

/*
  Версия 1.4:
  - Исправлен баг при смене режимов
  - Исправлены тормоза в режиме точки доступа
  --- 08.07.2019
  - Исправлены параметры и процесс подключения к WiFi сети (таймаут 7 секунд) и развёртываия WiFi точки доступа (параметры имени/пароля)
  - Добавлено "#define USE_NTP" - позволяет запретить обращаться в интернет
  - Добавлено "#define ESP_USE_BUTTON - позволяет собирать лампу без физической кнопки, иначе яркость эффектов самопроизвольно растёт до максимальной
  - Переработаны параметры IP адресов, STA_STATIC_IP теперь пустой по умолчанию - избавляет от путаницы с IP адресами из неправильных диапазонов
  - Добавлено "#define GENERAL_DEBUG" - выводит в Serial/Telnet некоторые отладочные сообщения
  - Добавлено "#define WIFIMAN_DEBUG (true)" - выводит в Serial/Telnet отладочные сообщения библиотеки WiFiManager
  - Добавлена таблица с тест кейсами
  - Форматирование кода, комментарии
  --- 11.07.2019
  - Исправлена ошибка невыключения матрицы после срабатывания будильника, если до будильника матрица была выключенной
  - Дополнена таблица с тест кейсами
  --- 14.07.2019
  - Исправлена ошибка выключения будильника, если перед его срабатыванием был активен эффект "матрица" (или другой эффект, где задействовано мало светодиодов)
  - Добавлено управление по воздуху:
  -- работает только в режиме WiFi клиента
  -- работает при подключенной кнопке (потому что режим прошивки активируется кнопкой)
  --- 16.07.2019
  - Исправлено регулярное подвисание матрицы на 1-2 секунды при отсутствии подключения к интернету (но при успешном подключении к WiFi)
  --- 28.07.2019
  - Доработано взаимодействие с android приложением (отправка состояния после каждой операции)
  --- 01.08.2019
  - Возврат к стандартной библиотеке GyverButton (изменениё из неё перенесено в button.ino
  - Добавлены 2 эффекта: Светлячки со шлейфом и Белый свет
  - При запросе обновления по воздуху (2 четверных касания к кнопке) лампа переключается в режим "Матрица" для визуального подтверждения готовности к прошивке
  - В android приложение добавлена функция сканирования сети и добавления ламп с помощью multicast пакетов, доработка прошивки под это
  --- 03.08.2019
  - Исправлены ошибки взаимодействия android приложения с лампой, в вывод команды CURR добавлено текущее время (или millis(), если время не синхронизировано)
  --- 10.08.2019
  - Добавлена точная настройка яркости, скорости и масштаба эффектов
  - Добавлено взаимодействие с android приложением по управлению будильниками
  --- 14.08.2019
  - Добавлена функция таймера отключения
  --- 26.08.2019
  - Добавлен режим автоматического переключения избранных эффектов
  - Реорганизован код, исправлены ошибки
  --- 28.08.2019
  - Добавлен вызов режима обновления модуля esp из android приложения
  --- 30.08.2019
  - Эффект "Светлячки со шлейфами" переименован в "Угасающие пиксели"
  - Добавлены 5 новых эффекта: "Радуга диагональная", "Метель", "Звездопад", "Светлячки со шлейфами" (новый) и "Блуждающий кубик"
  - Исправлены ошибки
  --- 04.09.2019
  - Большая часть определений (констант) перенесена в файл Constants.h
  - Большая оптимизация использования памяти
  - Исправлена ошибка невключения эффекта "Белый свет" приложением и кнопкой
  - Исправлена ошибка неправильного выбора интервала в режиме Избранное в android приложении
  --- 16.09.2019
  - Добавлено сохранение состояния (вкл/выкл) лампы в EEPROM память
  - Добавлен новый эффект белого света (с горизонтальной полосой)
  - Реорганизован код, исправлены ошибки
  --- 20.09.2019
  - Добавлена возможность сохранять состояние (вкл/выкл) режима "Избранное"; не сбрасывается выключением матрицы, не сбрасывается перезапуском модуля esp
  - Убрана очистка параметров WiFi при старте с зажатой кнопкой; регулируется директивой ESP_RESET_ON_START, которая определена как false по умолчанию
  --- 24.09.2019
  - Добавлены изменения из прошивка от Alex Gyver v1.5: бегущая строка с IP адресом лампы по пятикратному клику на кнопку
  --- 29.09.2019
  - Добавлена опция вывода отладочных сообщений по пртоколу telnet вместо serial для удалённой отладки
  - Исправлена ошибка регулировки яркости кнопкой
  --- 05.10.2019
  - Добавлено управление по протоколу MQTT
  - Исправлена ошибка выключения будильника кнопкой
  - Добавлена задержка в 1 секунду сразу после старта, в течение которой нужно нажать кнопку, чтобы очистить сохранённые параметры WiFi (если ESP_RESET_ON_START == true)
  --- 12.10.2019
  - Добавлена возможность сменить рабочий режим лампы (ESP_MODE) без необходимости перепрошивки; вызывается по семикратному клику по кнопке при включенной матрице; сохраняется в EEPROM
  - Изменён алгоритм работы будильника:
  -  * обновление его оттенка/яркости происходит 1 раз в 3 секунды вместо 1 раза в минуту
  -  * диоды разбиты на 6 групп, первой из которых назначается новый оттенок/яркость 1 раз в 3 секунды, вторая "отстаёт" на 1 шаг, третья - на 2 шага и т.д. (для большей плавности)
  - Добавлена визуальная сигнализация о некоторых важных действиях/состояниях лампы:
  -  * при запуске в режиме WiFi клиента и ещё не настроенных параметрах WiFi сети (когда их нужно ввести)                                                     - 1 вспышка жёлтым
  -  * если лампа стартовала в режиме WiFi клиента с ненастроенными параметрами WiFi сети, и они не были введены за отведённый таймаут (перед перезагрузкой)   - 1 вспышка красным
  -  * при переходе лампы в режим обновления по воздуху (OTA) по двум четырёхкратным кликам по кнопке или по кнопке OTA из android приложения                  - 2 вспышки жёлтым
  -  * если лампа была переведена в режим OTA, но не дождалась прошивки за отведённый таймаут (перед перезагрузкой)                                            - 2 вспышки красным
  -  * при переключении рабочего режима лампы WiFi точка доступа/WiFi клиент семикратным кликом по кнопке (перед перезагрузкой)                                - 3 вспышки красным
  -  * при запросе вывода времени бегущей строкой, если время не синхронизировано                                                                              - 4 вспышки красным
  - Уменьшен таймаут подключения к WiFi сети до 6 секунд; вызвано увеличившейся продолжительностью работы функции setup(), она в сумме должна быть меньше 8 секунд
  - Оптимизирован код
  --- 14.10.2019
  - Если при первом старте в режиме WiFi клиента запрашиваемые имя и пароль WiFi сети не введены за отведённый таймаут (5 минут), лампа перезагрузится в режиме точки доступа
  - Добавлен вывод времени бегущей строкой:
  -  * по запросу - шестикратному клику - текущее время белым цветом;
  -  * периодически - определяется константой PRINT_TIME в Constants.h - от раза в час (красным цветом) до раза в минуту (синим цветом) с яркостью текущего эффекта как при включенной, так и при выключенной матрице
  --- 19.10.2019
  - Добавлены "ночные часы" (от NIGHT_HOURS_START до NIGHT_HOURS_STOP включительно) и "дневные часы" (всё остальное время), для которых доступна регулировка яркости для вывода времени бегущей строкой - NIGHT_HOURS_BRIGHTNESS и DAY_HOURS_BRIGHTNESS
  --- 20.10.2019
  - Добавлена блокировка кнопки на лампе из android приложения; сохраняется в EEPROM память
  --- 24.10.2019
  - Добавлен вывод сигнала (HIGH/LOW - настраивается константой MOSFET_LEVEL) синхронно с включением матрицы на пин MOSFET транзистора (настраивается константой MOSFET_PIN)
  - Добавлен вывод сигнала (HIGH/LOW - настраивается константой ALARM_LEVEL) на пин будильника (настраивается константой ALARM_PIN); сигнал подаётся в течение одной минуты, начиная со времени, на которое заведён будильник
  --- 02.11.2019
  - Добавлен переход на летнее/зимнее время (изменены настройки часового пояса, см. Constants.h); добавлена библиотека Timezone
  - Добавлен эффект Белый огонь
  - Исправлена ошибка сброса сигнала на пине ALARM_PIN при отключении будильника вручную
  - Добавлена сигнализация (4 вспышки красным) при запросе вывода времени шестикратным кликом, если время не синхронизировано
  --- 04.11.2019
  - Исправлена ошибка невключения MOSFET'а матрицы при срабатывании "рассвета"
  - Исправлена ошибка невключения MOSFET'а матрицы при выводе времени и IP адреса
  --- 08.11.2019
  - Исправлены ошибки назначения статического IP адреса
  - Добавлен набросок WiFiManager Captive Portal для ввода пользовательских параметров и настроек
*/

// Ссылка для менеджера плат:
// https://arduino.esp8266.com/stable/package_esp8266com_index.json


#include "pgmspace.h"
#include "Constants.h"
#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include "CaptivePortalManager.h"
#include <WiFiUdp.h>
#include <EEPROM.h>
#include "Types.h"
#include "timerMinim.h"
#ifdef ESP_USE_BUTTON
#include <GyverButton.h>
#endif
#include "fonts.h"
#ifdef USE_NTP
#include <NTPClient.h>
#include <Timezone.h>
#endif
#include <TimeLib.h>
#ifdef OTA
#include "OtaManager.h"
#endif
#if USE_MQTT
#include "MqttManager.h"
#endif
#include "TimerManager.h"
#include "FavoritesManager.h"
#include "EepromManager.h"


// --- ИНИЦИАЛИЗАЦИЯ ОБЪЕКТОВ ----------
CRGB leds[NUM_LEDS];
WiFiManager wifiManager;
WiFiServer wifiServer(ESP_HTTP_PORT);
WiFiUDP Udp;

#ifdef USE_NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, 0, NTP_INTERVAL); // объект, запрашивающий время с ntp сервера; в нём смещение часового пояса не используется (перенесено в объект localTimeZone); здесь всегда должно быть время UTC
  #ifdef SUMMER_WINTER_TIME
  TimeChangeRule summerTime = { SUMMER_TIMEZONE_NAME, SUMMER_WEEK_NUM, SUMMER_WEEKDAY, SUMMER_MONTH, SUMMER_HOUR, SUMMER_OFFSET };
  TimeChangeRule winterTime = { WINTER_TIMEZONE_NAME, WINTER_WEEK_NUM, WINTER_WEEKDAY, WINTER_MONTH, WINTER_HOUR, WINTER_OFFSET };
  Timezone localTimeZone(summerTime, winterTime);
  #else
  TimeChangeRule localTime = { LOCAL_TIMEZONE_NAME, LOCAL_WEEK_NUM, LOCAL_WEEKDAY, LOCAL_MONTH, LOCAL_HOUR, LOCAL_OFFSET };
  Timezone localTimeZone(localTime);
  #endif
#endif

timerMinim timeTimer(3000);
bool ntpServerAddressResolved = false;
bool timeSynched = false;
uint32_t lastTimePrinted = 0U;

#ifdef ESP_USE_BUTTON
GButton touch(BTN_PIN, LOW_PULL, NORM_OPEN);
#endif

#ifdef OTA
OtaManager otaManager(&showWarning);
OtaPhase OtaManager::OtaFlag = OtaPhase::None;
#endif

#if USE_MQTT
AsyncMqttClient* mqttClient = NULL;
AsyncMqttClient* MqttManager::mqttClient = NULL;
char* MqttManager::mqttServer = NULL;
char* MqttManager::mqttUser = NULL;
char* MqttManager::mqttPassword = NULL;
char* MqttManager::clientId = NULL;
char* MqttManager::lampInputBuffer = NULL;
char* MqttManager::topicInput = NULL;
char* MqttManager::topicOutput = NULL;
bool MqttManager::needToPublish = false;
char MqttManager::mqttBuffer[] = {};
uint32_t MqttManager::mqttLastConnectingAttempt = 0;
SendCurrentDelegate MqttManager::sendCurrentDelegate = NULL;
#endif

// --- ИНИЦИАЛИЗАЦИЯ ПЕРЕМЕННЫХ -------
uint16_t localPort = ESP_UDP_PORT;
char packetBuffer[MAX_UDP_BUFFER_SIZE];                     // buffer to hold incoming packet
char inputBuffer[MAX_UDP_BUFFER_SIZE];
static const uint8_t maxDim = max(WIDTH, HEIGHT);

ModeType modes[MODE_AMOUNT];
AlarmType alarms[7];

static const uint8_t dawnOffsets[] PROGMEM = {5, 10, 15, 20, 25, 30, 40, 50, 60};   // опции для выпадающего списка параметра "время перед 'рассветом'" (будильник); синхронизировано с android приложением
uint8_t dawnMode;
bool dawnFlag = false;
uint32_t thisTime;
bool manualOff = false;

int8_t currentMode = 0;
bool loadingFlag = true;
bool ONflag = false;
uint32_t eepromTimeout;
bool settChanged = false;
bool buttonEnabled = true;

unsigned char matrixValue[8][16];

bool TimerManager::TimerRunning = false;
bool TimerManager::TimerHasFired = false;
uint8_t TimerManager::TimerOption = 1U;
uint64_t TimerManager::TimeToFire = 0ULL;

uint8_t FavoritesManager::FavoritesRunning = 0;
uint16_t FavoritesManager::Interval = DEFAULT_FAVORITES_INTERVAL;
uint16_t FavoritesManager::Dispersion = DEFAULT_FAVORITES_DISPERSION;
uint8_t FavoritesManager::UseSavedFavoritesRunning = 0;
uint8_t FavoritesManager::FavoriteModes[MODE_AMOUNT] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint32_t FavoritesManager::nextModeAt = 0UL;

bool CaptivePortalManager::captivePortalCalled = false;


void setup()
{
  Serial.begin(115200);
  Serial.println();
  ESP.wdtEnable(WDTO_8S);


  // ПИНЫ
  #ifdef MOSFET_PIN                                         // инициализация пина, управляющего MOSFET транзистором в состояние "выключен"
  pinMode(MOSFET_PIN, OUTPUT);
  #ifdef MOSFET_LEVEL
  digitalWrite(MOSFET_PIN, !MOSFET_LEVEL);
  #endif
  #endif

  #ifdef ALARM_PIN                                          // инициализация пина, управляющего будильником в состояние "выключен"
  pinMode(ALARM_PIN, OUTPUT);
  #ifdef ALARM_LEVEL
  digitalWrite(ALARM_PIN, !ALARM_LEVEL);
  #endif
  #endif


  // TELNET
  #if defined(GENERAL_DEBUG) && GENERAL_DEBUG_TELNET
  telnetServer.begin();
  for (uint8_t i = 0; i < 100; i++)                         // пауза 10 секунд в отладочном режиме, чтобы успеть подключиться по протоколу telnet до вывода первых сообщений
  {
    handleTelnetClient();
    delay(100);
    ESP.wdtFeed();
  }
  #endif


  // КНОПКА
  #if defined(ESP_USE_BUTTON)
  touch.setStepTimeout(BUTTON_STEP_TIMEOUT);
  touch.setClickTimeout(BUTTON_CLICK_TIMEOUT);
    #if ESP_RESET_ON_START
    delay(1000);                                            // ожидание инициализации модуля кнопки ttp223 (по спецификации 250мс)
    if (digitalRead(BTN_PIN))
    {
      wifiManager.resetSettings();                          // сброс сохранённых SSID и пароля при старте с зажатой кнопкой, если разрешено
      LOG.println(F("Настройки WiFiManager сброшены"));
    }
    buttonEnabled = true;                                   // при сбросе параметров WiFi сразу после старта с зажатой кнопкой, также разблокируется кнопка, если была заблокирована раньше
    EepromManager::SaveButtonEnabled(&buttonEnabled);
    ESP.wdtFeed();
    #endif
  #endif


  // ЛЕНТА/МАТРИЦА
  FastLED.addLeds<WS2812B, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS)/*.setCorrection(TypicalLEDStrip)*/;
  FastLED.setBrightness(BRIGHTNESS);
  if (CURRENT_LIMIT > 0)
  {
    FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);
  }
  FastLED.clear();
  FastLED.show();


  // EEPROM
  EepromManager::InitEepromSettings(                        // инициализация EEPROM; запись начального состояния настроек, если их там ещё нет; инициализация настроек лампы значениями из EEPROM
    modes, alarms, &espMode, &ONflag, &dawnMode, &currentMode, &buttonEnabled,
    &(FavoritesManager::ReadFavoritesFromEeprom),
    &(FavoritesManager::SaveFavoritesToEeprom));
  LOG.printf_P(PSTR("Рабочий режим лампы: ESP_MODE = %d\n"), espMode);


  // WI-FI
  wifiManager.setDebugOutput(WIFIMAN_DEBUG);                // вывод отладочных сообщений
  // wifiManager.setMinimumSignalQuality();                 // установка минимально приемлемого уровня сигнала WiFi сетей (8% по умолчанию)
  CaptivePortalManager *captivePortalManager = new CaptivePortalManager(&wifiManager);
  if (espMode == 0U)                                        // режим WiFi точки доступа
  {
    // wifiManager.setConfigPortalBlocking(false);

    if (sizeof(AP_STATIC_IP))
    {
      LOG.println(F("Используется статический IP адрес WiFi точки доступа"));
      wifiManager.setAPStaticIPConfig(                      // wifiManager.startConfigPortal использовать нельзя, т.к. он блокирует вычислительный процесс внутри себя, а затем перезагружает ESP, т.е. предназначен только для ввода SSID и пароля
        IPAddress(AP_STATIC_IP[0], AP_STATIC_IP[1], AP_STATIC_IP[2], AP_STATIC_IP[3]),      // IP адрес WiFi точки доступа
        IPAddress(AP_STATIC_IP[0], AP_STATIC_IP[1], AP_STATIC_IP[2], 1),                    // первый доступный IP адрес сети
        IPAddress(255, 255, 255, 0));                                                       // маска подсети
    }

    WiFi.softAP(AP_NAME, AP_PASS);

    LOG.println(F("Старт в режиме WiFi точки доступа"));
    LOG.print(F("IP адрес: "));
    LOG.println(WiFi.softAPIP());

    wifiServer.begin();
  }
  else                                                      // режим WiFi клиента (подключаемся к роутеру, если есть сохранённые SSID и пароль, иначе создаём WiFi точку доступа и запрашиваем их)
  {
    LOG.println(F("Старт в режиме WiFi клиента (подключение к роутеру)"));

    if (WiFi.SSID().length())
    {
      LOG.printf_P(PSTR("Подключение к WiFi сети: %s\n"), WiFi.SSID().c_str());

      if (sizeof(STA_STATIC_IP))                            // ВНИМАНИЕ: настраивать статический ip WiFi клиента можно только при уже сохранённых имени и пароле WiFi сети (иначе проявляется несовместимость библиотек WiFiManager и WiFi)
      {
        LOG.print(F("Сконфигурирован статический IP адрес: "));
        LOG.printf_P(PSTR("%u.%u.%u.%u\n"), STA_STATIC_IP[0], STA_STATIC_IP[1], STA_STATIC_IP[2], STA_STATIC_IP[3]);
        wifiManager.setSTAStaticIPConfig(
          IPAddress(STA_STATIC_IP[0], STA_STATIC_IP[1], STA_STATIC_IP[2], STA_STATIC_IP[3]),// статический IP адрес ESP в режиме WiFi клиента
          IPAddress(STA_STATIC_IP[0], STA_STATIC_IP[1], STA_STATIC_IP[2], 1),               // первый доступный IP адрес сети (справедливо для 99,99% случаев; для сетей меньше чем на 255 адресов нужно вынести в константы)
          IPAddress(255, 255, 255, 0));                                                     // маска подсети (справедливо для 99,99% случаев; для сетей меньше чем на 255 адресов нужно вынести в константы)
      }
    }
    else
    {
      LOG.println(F("WiFi сеть не определена, запуск WiFi точки доступа для настройки параметров подключения к WiFi сети..."));
      CaptivePortalManager::captivePortalCalled = true;
      wifiManager.setBreakAfterConfig(true);                // перезагрузка после ввода и сохранения имени и пароля WiFi сети
      showWarning(CRGB::Yellow, 1000U, 500U);               // мигание жёлтым цветом 0,5 секунды (1 раз) - нужно ввести параметры WiFi сети для подключения
    }

    wifiManager.setConnectTimeout(ESP_CONN_TIMEOUT);        // установка времени ожидания подключения к WiFi сети, затем старт WiFi точки доступа
    wifiManager.setConfigPortalTimeout(ESP_CONF_TIMEOUT);   // установка времени работы WiFi точки доступа, затем перезагрузка; отключить watchdog?
    wifiManager.autoConnect(AP_NAME, AP_PASS);              // пытаемся подключиться к сохранённой ранее WiFi сети; в случае ошибки, будет развёрнута WiFi точка доступа с указанными AP_NAME и паролем на время ESP_CONN_TIMEOUT секунд; http://AP_STATIC_IP:ESP_HTTP_PORT (обычно http://192.168.0.1:80) - страница для ввода SSID и пароля от WiFi сети роутера

    delete captivePortalManager;
    captivePortalManager = NULL;

    if (WiFi.status() != WL_CONNECTED)                      // подключение к WiFi не установлено
    {
      if (CaptivePortalManager::captivePortalCalled)        // была показана страница настройки WiFi ...
      {
        if (millis() < (ESP_CONN_TIMEOUT + ESP_CONF_TIMEOUT) * 1000U) // пользователь ввёл некорректное имя WiFi сети и/или пароль или запрошенная WiFi сеть недоступна
        {
          LOG.println(F("Не удалось подключиться к WiFi сети\nУбедитесь в корректности имени WiFi сети и пароля\nРестарт для запроса нового имени WiFi сети и пароля...\n"));
          wifiManager.resetSettings();
        }
        else                                                // пользователь не вводил имя WiFi сети и пароль
        {
          LOG.println(F("Время ожидания ввода SSID и пароля от WiFi сети или подключения к WiFi сети превышено\nЛампа будет перезагружена в режиме WiFi точки доступа!\n"));

          espMode = (espMode == 0U) ? 1U : 0U;
          EepromManager::SaveEspMode(&espMode);

          LOG.printf_P(PSTR("Рабочий режим лампы изменён и сохранён в энергонезависимую память\nНовый рабочий режим: ESP_MODE = %d, %s\nРестарт...\n"),
            espMode, espMode == 0U ? F("WiFi точка доступа") : F("WiFi клиент (подключение к роутеру)"));
        }
      }
      else                                                  // страница настройки WiFi не была показана, не удалось подключиться к ранее сохранённой WiFi сети (перенос в новую WiFi сеть)
      {
        LOG.println(F("Не удалось подключиться к WiFi сети\nВозможно, заданная WiFi сеть больше не доступна\nРестарт для запроса нового имени WiFi сети и пароля...\n"));
        wifiManager.resetSettings();
      }

      showWarning(CRGB::Red, 1000U, 500U);                  // мигание красным цветом 0,5 секунды (1 раз) - ожидание ввода SSID'а и пароля WiFi сети прекращено, перезагрузка
      ESP.restart();
    }

    if (CaptivePortalManager::captivePortalCalled &&        // первое подключение к WiFi сети после настройки параметров WiFi на странице настройки - нужна перезагрузка для применения статического IP
        sizeof(STA_STATIC_IP) &&
        WiFi.localIP() != IPAddress(STA_STATIC_IP[0], STA_STATIC_IP[1], STA_STATIC_IP[2], STA_STATIC_IP[3]))
    {
      LOG.println(F("Рестарт для применения заданного статического IP адреса..."));
      delay(100);
      ESP.restart();
    }

    LOG.print(F("IP адрес: "));
    LOG.println(WiFi.localIP());
  }
  ESP.wdtFeed();

  LOG.printf_P(PSTR("Порт UDP сервера: %u\n"), localPort);
  Udp.begin(localPort);


  // NTP
  #ifdef USE_NTP
  timeClient.begin();
  ESP.wdtFeed();
  #endif


  // MQTT
  #if (USE_MQTT)
  if (espMode == 1U)
  {
    mqttClient = new AsyncMqttClient();
    MqttManager::setupMqtt(mqttClient, inputBuffer, &sendCurrent);    // создание экземпляров объектов для работы с MQTT, их инициализация и подключение к MQTT брокеру
  }
  ESP.wdtFeed();
  #endif


  // ОСТАЛЬНОЕ
  memset(matrixValue, 0, sizeof(matrixValue));
  randomSeed(micros());
  changePower();
  loadingFlag = true;
}


void loop()
{
  parseUDP();
  effectsTick();

  EepromManager::HandleEepromTick(&settChanged, &eepromTimeout, &ONflag, 
    &currentMode, modes, &(FavoritesManager::SaveFavoritesToEeprom));

  #ifdef USE_NTP
  timeTick();
  #endif

  #ifdef ESP_USE_BUTTON
  if (buttonEnabled)
  {
    buttonTick();
  }
  #endif

  #ifdef OTA
  otaManager.HandleOtaUpdate();                             // ожидание и обработка команды на обновление прошивки по воздуху
  #endif

  TimerManager::HandleTimer(&ONflag, &settChanged,          // обработка событий таймера отключения лампы
    &eepromTimeout, &changePower);

  if (FavoritesManager::HandleFavorites(                    // обработка режима избранных эффектов
      &ONflag,
      &currentMode,
      &loadingFlag
      #ifdef USE_NTP
      , &dawnFlag
      #endif
      ))
  {
    FastLED.setBrightness(modes[currentMode].Brightness);
    FastLED.clear();
    delay(1);
  }

  #if USE_MQTT
  if (espMode == 1U && mqttClient && WiFi.isConnected() && !mqttClient->connected())
  {
    MqttManager::mqttConnect();                             // библиотека не умеет восстанавливать соединение в случае потери подключения к MQTT брокеру, нужно управлять этим явно
    MqttManager::needToPublish = true;
  }

  if (MqttManager::needToPublish)
  {
    if (strlen(inputBuffer) > 0)                            // проверка входящего MQTT сообщения; если оно не пустое - выполнение команды из него и формирование MQTT ответа
    {
      processInputBuffer(inputBuffer, MqttManager::mqttBuffer, true);
    }
    
    MqttManager::publishState();
  }
  #endif

  #if defined(GENERAL_DEBUG) && GENERAL_DEBUG_TELNET
  handleTelnetClient();
  #endif

  ESP.wdtFeed();                                            // пнуть собаку
}
