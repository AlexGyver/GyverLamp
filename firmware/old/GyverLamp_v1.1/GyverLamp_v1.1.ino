/*
  Скетч к проекту "Многофункциональный RGB светильник"
  Страница проекта (схемы, описания): https://alexgyver.ru/GyverLamp/
  Исходники на GitHub: https://github.com/AlexGyver/GyverLamp/
  Нравится, как написан код? Поддержи автора! https://alexgyver.ru/support_alex/
  Автор: AlexGyver, AlexGyver Technologies, 2019
  https://AlexGyver.ru/
*/

// Ссылка для менеджера плат:
// http://arduino.esp8266.com/stable/package_esp8266com_index.json

// ============= НАСТРОЙКИ =============
// -------- ВРЕМЯ -------
#define GMT 3              // смещение (москва 3)
#define NTP_ADDRESS  "europe.pool.ntp.org"    // сервер времени

// -------- РАССВЕТ -------
#define DAWN_BRIGHT 200       // макс. яркость рассвета
#define DAWN_TIMEOUT 1        // сколько рассвет светит после времени будильника, минут

// ---------- МАТРИЦА ---------
#define BRIGHTNESS 40         // стандартная маскимальная яркость (0-255)
#define CURRENT_LIMIT 2000    // лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит

#define WIDTH 16              // ширина матрицы
#define HEIGHT 16             // высота матрицы

#define COLOR_ORDER GRB       // порядок цветов на ленте. Если цвет отображается некорректно - меняйте. Начать можно с RGB

#define MATRIX_TYPE 0         // тип матрицы: 0 - зигзаг, 1 - параллельная
#define CONNECTION_ANGLE 0    // угол подключения: 0 - левый нижний, 1 - левый верхний, 2 - правый верхний, 3 - правый нижний
#define STRIP_DIRECTION 0     // направление ленты из угла: 0 - вправо, 1 - вверх, 2 - влево, 3 - вниз
// при неправильной настрйоке матрицы вы получите предупреждение "Wrong matrix parameters! Set to default"
// шпаргалка по настройке матрицы здесь! https://alexgyver.ru/matrix_guide/

// --------- ESP --------
#define ESP_MODE 1
// 0 - точка доступа (192.168.4.1 или другой)
// 1 - локальный (192.168.1.232 или другой)

// -------- Менеджер WiFi ---------
#define AC_SSID "AutoConnectAP"
#define AC_PASS "12345678"

// -------------- AP ---------------
#define AP_SSID "GyverLamp"
#define AP_PASS "12345678"
#define AP_PORT 8888

// ============= ДЛЯ РАЗРАБОТЧИКОВ =============
#define LED_PIN 2             // пин ленты
#define BTN_PIN 4
#define MODE_AMOUNT 14

#define NUM_LEDS WIDTH * HEIGHT
#define SEGMENTS 1            // диодов в одном "пикселе" (для создания матрицы из кусков ленты)
// ---------------- БИБЛИОТЕКИ -----------------
#define FASTLED_INTERRUPT_RETRY_COUNT 0
#define FASTLED_ALLOW_INTERRUPTS 0
#define FASTLED_ESP8266_RAW_PIN_ORDER
#define NTP_INTERVAL 60 * 1000    // обновление (1 минута)

#include "timerMinim.h"
#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include <NTPClient.h>
#include <GyverButton.h>

// ------------------- ТИПЫ --------------------
CRGB leds[NUM_LEDS];
WiFiServer server(80);
WiFiUDP Udp;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, GMT * 3600, NTP_INTERVAL);
timerMinim timeTimer(3000);
GButton touch(BTN_PIN, LOW_PULL, NORM_OPEN);

// ----------------- ПЕРЕМЕННЫЕ ------------------
const char* autoConnectSSID = AC_SSID;
const char* autoConnectPass = AC_PASS;
const char AP_NameChar[] = AP_SSID;
const char WiFiPassword[] = AP_PASS;
unsigned int localPort = AP_PORT;
char packetBuffer[UDP_TX_PACKET_MAX_SIZE + 1]; //buffer to hold incoming packet
String inputBuffer;
static const byte maxDim = max(WIDTH, HEIGHT);
struct {
  byte brightness = 50;
  byte speed = 30;
  byte scale = 40;
} modes[15];

struct {
  boolean state = false;
  int time = 0;
} alarm[7];

byte dawnOffsets[] = {5, 10, 15, 20, 25, 30, 40, 50, 60};
byte dawnMode;
boolean dawnFlag = false;
long thisTime;
boolean manualOff = false;

int8_t currentMode = 0;
boolean loadingFlag = true;
boolean ONflag = true;
uint32_t eepromTimer;
boolean settChanged = false;
// Конфетти, Огонь, Радуга верт., Радуга гориз., Смена цвета,
// Безумие 3D, Облака 3D, Лава 3D, Плазма 3D, Радуга 3D,
// Павлин 3D, Зебра 3D, Лес 3D, Океан 3D,

unsigned char matrixValue[8][16];

void setup() {
  ESP.wdtDisable();
  //ESP.wdtEnable(WDTO_8S);
  delay(1000);
  // ЛЕНТА
  FastLED.addLeds<WS2812B, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS)/*.setCorrection( TypicalLEDStrip )*/;
  FastLED.setBrightness(BRIGHTNESS);
  if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);
  FastLED.clear();
  FastLED.show();
  randomSeed(analogRead(0));    // пинаем генератор случайных чисел
  touch.setStepTimeout(100);
  touch.setClickTimeout(500);

  Serial.begin(115200);

  // WI-FI
  if (ESP_MODE == 0) {    // режим точки доступа
    boolean conn = WiFi.softAP(AP_NameChar, WiFiPassword);
    server.begin();

    IPAddress myIP = WiFi.softAPIP();
    Serial.println(conn);
    Serial.print("Access point Mode");
    Serial.print("AP IP address: ");
    Serial.println(myIP);
  } else {                // подключаемся к роутеру
    Serial.print("WiFi manager");
    WiFiManager wifiManager;
    //wifiManager.resetSettings();
    //wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
    wifiManager.autoConnect(autoConnectSSID, autoConnectPass);
    Serial.print("Connected! IP address: ");
    Serial.println(WiFi.localIP());
  }
  Serial.printf("UDP server on port %d\n", localPort);
  Udp.begin(localPort);

  // EEPROM
  EEPROM.begin(128);
  delay(50);
  if (EEPROM.read(102) != 20) {   // первый запуск
    EEPROM.write(102, 20);
    EEPROM.commit();

    for (byte i = 0; i < MODE_AMOUNT; i++) {
      EEPROM.put(3 * i, modes[i]);
      EEPROM.commit();
    }
    for (byte i = 0; i < 7; i++) {
      EEPROM.write(5 * i + 50, alarm[i].state);   // рассвет
      eeWriteInt(5 * i + 50 + 1, alarm[i].time);
      EEPROM.commit();
    }
    EEPROM.write(100, 0);   // рассвет
    EEPROM.write(101, 0);   // режим
    EEPROM.commit();
  }
  for (byte i = 0; i < MODE_AMOUNT; i++) {
    EEPROM.get(3 * i, modes[i]);
  }
  for (byte i = 0; i < 7; i++) {
    alarm[i].state = EEPROM.read(5 * i + 50);
    alarm[i].time = eeGetInt(5 * i + 50 + 1);
  }
  dawnMode = EEPROM.read(100);
  currentMode = (int8_t)EEPROM.read(101);

  // отправляем настройки
  sendCurrent();
  char reply[inputBuffer.length() + 1];
  inputBuffer.toCharArray(reply, inputBuffer.length() + 1);
  Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  Udp.write(reply);
  Udp.endPacket();

  timeClient.begin();
  memset(matrixValue, 0, sizeof(matrixValue));
}

void loop() {
  parseUDP();
  effectsTick();
  eepromTick();
  timeTick();
  buttonTick();
  ESP.wdtFeed();   // пнуть собаку
}

void eeWriteInt(int pos, int val) {
  byte* p = (byte*) &val;
  EEPROM.write(pos, *p);
  EEPROM.write(pos + 1, *(p + 1));
  EEPROM.write(pos + 2, *(p + 2));
  EEPROM.write(pos + 3, *(p + 3));
  EEPROM.commit();
}

int eeGetInt(int pos) {
  int val;
  byte* p = (byte*) &val;
  *p        = EEPROM.read(pos);
  *(p + 1)  = EEPROM.read(pos + 1);
  *(p + 2)  = EEPROM.read(pos + 2);
  *(p + 3)  = EEPROM.read(pos + 3);
  return val;
}
