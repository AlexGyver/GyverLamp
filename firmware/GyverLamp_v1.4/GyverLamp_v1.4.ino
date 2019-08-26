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
  - Добавлено "#define GENERAL_DEBUG" - выводит в Serial некоторые отладочные сообщения
  - Добавлено "#define WIFIMAN_DEBUG (true)" - выводит в Serial отладочные сообщения библиотеки WiFiManager
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
*/

// Ссылка для менеджера плат:
// http://arduino.esp8266.com/stable/package_esp8266com_index.json


// ============= НАСТРОЙКИ =============
// --- ВРЕМЯ ---------------------------
#define USE_NTP                                             // закомментировать или удалить эту строку, если нужно, чтобы устройство не лезло в интернет
#define GMT              (3)                                // часовой пояс (москва 3)
#define NTP_ADDRESS      ("ntp2.colocall.net")              // сервер времени
#define NTP_INTERVAL     (30UL * 60UL * 1000UL)             // интервал синхронизации времени (30 минут)

// --- РАССВЕТ -------------------------
#define DAWN_BRIGHT      (200U)                             // максимальная яркость рассвета (0-255)
#define DAWN_TIMEOUT     (1U)                               // сколько рассвет светит после времени будильника, минут

// --- МАТРИЦА -------------------------
#define BRIGHTNESS       (40U)                              // стандартная маскимальная яркость (0-255)
#define CURRENT_LIMIT    (2000U)                            // лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит

#define WIDTH            (16U)                              // ширина матрицы
#define HEIGHT           (16U)                              // высота матрицы

#define COLOR_ORDER      (GRB)                              // порядок цветов на ленте. Если цвет отображается некорректно - меняйте. Начать можно с RGB

#define MATRIX_TYPE      (0U)                               // тип матрицы: 0 - зигзаг, 1 - параллельная
#define CONNECTION_ANGLE (0U)                               // угол подключения: 0 - левый нижний, 1 - левый верхний, 2 - правый верхний, 3 - правый нижний
#define STRIP_DIRECTION  (0U)                               // направление ленты из угла: 0 - вправо, 1 - вверх, 2 - влево, 3 - вниз
                                                            // при неправильной настройке матрицы вы получите предупреждение "Wrong matrix parameters! Set to default"
                                                            // шпаргалка по настройке матрицы здесь! https://alexgyver.ru/matrix_guide/

// --- ESP -----------------------------
#define ESP_MODE         (1U)                               // 0U - WiFi точка доступа, 1U - клиент WiFi (подключение к роутеру)
#define ESP_USE_BUTTON                                      // если строка не закомментирована, должна быть подключена кнопка (иначе ESP может регистрировать "фантомные" нажатия и некорректно устанавливать яркость)
#define ESP_HTTP_PORT    (80U)                              // номер порта, который будет использоваться во время первой утановки имени WiFi сети (и пароля), к которой потом будет подключаться лампа в режиме WiFi клиента (лучше не менять)
#define ESP_UDP_PORT     (8888U)                            // номер порта, который будет "слушать" UDP сервер во время работы лампы как в режиме WiFi точки доступа, так и в режиме WiFi клиента (лучше не менять)
#define ESP_CONN_TIMEOUT (7U)                               // время в секундах (ДОЛЖНО БЫТЬ МЕНЬШЕ 8, иначе сработает WDT), которое ESP будет пытаться подключиться к WiFi сети, после его истечения автоматически развернёт WiFi точку доступа
#define ESP_CONF_TIMEOUT (300U)                             // время в секундах, которое ESP будет ждать ввода SSID и пароля WiFi сети роутера в конфигурационном режиме, после его истечения ESP перезагружается
#define GENERAL_DEBUG                                       // если строка не закомментирована, будут выводиться отладочные сообщения
#define WIFIMAN_DEBUG    (true)                             // вывод отладочных сообщений при подключении к WiFi сети: true - выводятся, false - не выводятся; настройка не зависит от GENERAL_DEBUG
#define OTA                                                 // если строка не закомментирована, модуль будет ждать два последдовательных запроса пользователя на прошивку по воздуху (см. документацию в "шапке")
#ifdef OTA
#define ESP_OTA_PORT     (8266U)                            // номер порта, который будет "прослушиваться" в ожидании команды прошивки по воздуху
#endif

// --- ESP (WiFi клиент) ---------------
uint8_t STA_STATIC_IP[] = {};                               // статический IP адрес: {} - IP адрес определяется роутером; {192, 168, 1, 66} - IP адрес задан явно (если DHCP на роутере не решит иначе); должен быть из того же диапазона адресов, что разадёт роутер
                                                            // SSID WiFi сети и пароль будут запрошены WiFi Manager'ом в режиме WiFi точки доступа, нет способа захардкодить их в прошивке

// --- AP (WiFi точка доступа) ---
#define AP_NAME          ("LedLamp")                        // имя WiFi точки доступа, используется как при запросе SSID и пароля WiFi сети роутера, так и при работе в режиме ESP_MODE = 0
#define AP_PASS          ("31415926")                       // пароль WiFi точки доступа
uint8_t AP_STATIC_IP[] = {192, 168, 4, 1};                  // статический IP точки доступа (лучше не менять)

// ============= ДЛЯ РАЗРАБОТЧИКОВ =====
#define LED_PIN          (2U)                               // пин ленты
#define BTN_PIN          (4U)                               // пин кнопки
#define MODE_AMOUNT      (20U)                              // количество режимов

#define NUM_LEDS         (WIDTH * HEIGHT)
#define SEGMENTS         (1U)                               // диодов в одном "пикселе" (для создания матрицы из кусков ленты)

// --- БИБЛИОТЕКИ ----------------------
#define FASTLED_INTERRUPT_RETRY_COUNT (0U)
#define FASTLED_ALLOW_INTERRUPTS      (0U)
#define FASTLED_ESP8266_RAW_PIN_ORDER

#include "Types.h"
#include "timerMinim.h"
#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#ifdef ESP_USE_BUTTON
#include <GyverButton.h>
#endif
#ifdef USE_NTP
#include <NTPClient.h>
#endif
#ifdef OTA
#include "OtaManager.h"
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
NTPClient timeClient(ntpUDP, NTP_ADDRESS, GMT * 3600, NTP_INTERVAL);
#endif

timerMinim timeTimer(3000);

#ifdef ESP_USE_BUTTON
GButton touch(BTN_PIN, LOW_PULL, NORM_OPEN);
#endif
#ifdef OTA
OtaManager otaManager;
OtaPhase OtaManager::OtaFlag = OtaPhase::None;
#endif

// --- ИНИЦИАЛИЗАЦИЯ ПЕРЕМЕННЫХ -------
uint16_t localPort = ESP_UDP_PORT;
char packetBuffer[UDP_TX_PACKET_MAX_SIZE + 1];              // buffer to hold incoming packet
String inputBuffer;
static const uint8_t maxDim = max(WIDTH, HEIGHT);

ModeType modes[MODE_AMOUNT];
AlarmType alarms[7];

uint8_t dawnOffsets[] = {5, 10, 15, 20, 25, 30, 40, 50, 60};// опции для выпадающего списка параметра "время перед 'рассветом'" (будильник)
uint8_t dawnMode;
bool dawnFlag = false;
long thisTime;
bool manualOff = false;

int8_t currentMode = 0;
bool loadingFlag = true;
bool ONflag = true;
uint32_t eepromTimer;
bool settChanged = false;

// Конфетти, Огонь, Радуга вертикальная, Радуга горизонтальная, Смена цвета,
// Безумие 3D, Облака 3D, Лава 3D, Плазма 3D, Радуга 3D,
// Павлин 3D, Зебра 3D, Лес 3D, Океан 3D,
// Цвет, Снег, Матрица, Светлячки, Светлячки со шлейфом, Белый свет

unsigned char matrixValue[8][16];

bool TimerManager::TimerRunning = false;
bool TimerManager::TimerHasFired = false;
uint8_t TimerManager::TimerOption = 1U;
uint64_t TimerManager::TimeToFire = 0ULL;

bool FavoritesManager::FavoritesRunning = false;
uint16_t FavoritesManager::Interval = DEFAULT_FAVORITES_INTERVAL;
uint16_t FavoritesManager::Dispersion = DEFAULT_FAVORITES_DISPERSION;
uint8_t FavoritesManager::FavoriteModes[MODE_AMOUNT] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint32_t FavoritesManager::nextModeAt = 0UL;


void setup()
{
  Serial.begin(115200);
  Serial.println();

  ESP.wdtDisable();
  //ESP.wdtEnable(WDTO_8S);

  #ifdef ESP_USE_BUTTON
  touch.setStepTimeout(100);
  touch.setClickTimeout(500);
  buttonTick();
  if (touch.state())                                        // сброс сохранённых SSID и пароля при старте с зажатой кнопкой
  {
    wifiManager.resetSettings();

    #ifdef GENERAL_DEBUG
    Serial.println("Настройки WiFiManager сброшены");
    #endif
  }
  #endif

  // ЛЕНТА
  FastLED.addLeds<WS2812B, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS)/*.setCorrection( TypicalLEDStrip )*/;
  FastLED.setBrightness(BRIGHTNESS);
  if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);
  FastLED.clear();
  FastLED.show();

  // WI-FI
  wifiManager.setDebugOutput(WIFIMAN_DEBUG);                // вывод отладочных сообщений
  //wifiManager.setMinimumSignalQuality();                  // установка минимально приемлемого уровня сигнала WiFi сетей (8% по умолчанию)
  if (ESP_MODE == 0)                                        // режим WiFi точки доступа
  {
    // wifiManager.setConfigPortalBlocking(false);
    WiFi.softAPConfig(                                      // wifiManager.startConfigPortal использовать нельзя, т.к. он блокирует вычислительный процесс внутри себя, а затем перезагружает ESP, т.е. предназначен только для ввода SSID и пароля
      IPAddress(AP_STATIC_IP[0], AP_STATIC_IP[1], AP_STATIC_IP[2], AP_STATIC_IP[3]),        // IP адрес WiFi точки доступа
      IPAddress(AP_STATIC_IP[0], AP_STATIC_IP[1], AP_STATIC_IP[2], 1),                      // первый доступный IP адрес сети
      IPAddress(255, 255, 255, 0));                                                         // маска подсети

    WiFi.softAP(AP_NAME, AP_PASS);

    Serial.println("Режим WiFi точки доступа");
    Serial.print("IP адрес: ");
    Serial.println(WiFi.softAPIP());

    wifiServer.begin();
  }
  else                                                      // режим WiFi клиента (подключаемся к роутеру, если есть сохранённые SSID и пароль, иначе создаём WiFi точку доступа и запрашиваем их)
  {
    Serial.println("Режим WiFi клиента");
    if (WiFi.SSID())
    {
      Serial.print("Подключение WiFi сети: ");
      Serial.println(WiFi.SSID());
    }
    else
    {
      Serial.println("WiFi сеть не определена, запуск WiFi точки доступа для настройки параметров подключения к WiFi сети...");
    }

    if (STA_STATIC_IP)
    {
      wifiManager.setSTAStaticIPConfig(
        IPAddress(STA_STATIC_IP[0], STA_STATIC_IP[1], STA_STATIC_IP[2], STA_STATIC_IP[3]),  // статический IP адрес ESP в режиме WiFi клиента
        IPAddress(STA_STATIC_IP[0], STA_STATIC_IP[1], STA_STATIC_IP[2], 1),                 // первый доступный IP адрес сети (справедливо для 99,99% случаев; для сетей меньше чем на 255 адресов нужно вынести в константы)
        IPAddress(255, 255, 255, 0));                                                       // маска подсети (справедливо для 99,99% случаев; для сетей меньше чем на 255 адресов нужно вынести в константы)
    }

    wifiManager.setConnectTimeout(ESP_CONN_TIMEOUT);        // установка времени ожидания подключения к WiFi сети, затем старт WiFi точки доступа
    wifiManager.setConfigPortalTimeout(ESP_CONF_TIMEOUT);   // установка времени работы WiFi точки доступа, затем перезагрузка; отключить watchdog?
    wifiManager.autoConnect(AP_NAME, AP_PASS);              // пытаемся подключиться к сохранённой ранее WiFi сети; в случае ошибки, будет развёрнута WiFi точка доступа с указанными AP_NAME и паролем на время ESP_CONN_TIMEOUT секунд; http://AP_STATIC_IP:ESP_HTTP_PORT (обычно http://192.168.0.1:80) - страница для ввода SSID и пароля от WiFi сети роутера

    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.printf("Время ожидания ввода SSID и пароля от WiFi сети или подключения к WiFi сети превышено\nПерезагрузка модуля");

      #if defined(ESP8266)
      ESP.reset();
      #else
      ESP.restart();
      #endif
    }

    Serial.print("IP адрес: ");
    Serial.println(WiFi.localIP());
  }

  Serial.printf("Порт UDP сервера: %u\n", localPort);
  Udp.begin(localPort);

  EepromManager::InitEepromSettings(                        // инициализация EEPROM; запись начального состояния настроек, если их там ещё нет; инициализация настроек лампы значениями из EEPROM
    modes, alarms, &dawnMode, &currentMode,
    &(FavoritesManager::ReadFavoritesFromEeprom),
    &(FavoritesManager::SaveFavoritesToEeprom));

  sendCurrent();                                            // отправляем настройки (куда?)
  char reply[inputBuffer.length() + 1];
  inputBuffer.toCharArray(reply, inputBuffer.length() + 1);
  Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  Udp.write(reply);
  Udp.endPacket();

  #ifdef USE_NTP
  timeClient.begin();
  #endif

  memset(matrixValue, 0, sizeof(matrixValue));

  randomSeed(micros());
}


void loop()
{
  parseUDP();
  effectsTick();
  EepromManager::HandleEepromTick(&settChanged, &eepromTimer, &currentMode, modes, &(FavoritesManager::SaveFavoritesToEeprom));
  #ifdef USE_NTP
  timeTick();
  #endif
  #ifdef ESP_USE_BUTTON
  buttonTick();
  #endif
  #ifdef OTA
  otaManager.HandleOtaUpdate();                             // ожидание и обработка команды на обновление прошивки по воздуху
  #endif
  TimerManager::HandleTimer(&ONflag, &changePower);         // обработка событий таймера отключения лампы
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
  ESP.wdtFeed();                                            // пнуть собаку
  yield();                                                  // обработать все "служебные" задачи: wdt, WiFi подключение и т.д. (?)
}
