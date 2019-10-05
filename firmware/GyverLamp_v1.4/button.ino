#ifdef ESP_USE_BUTTON
bool brightDirection;

static bool startButtonHolding = false;                     // флаг: кнопка удерживается для изменения яркости лампы

void buttonTick()
{
  touch.tick();
  uint8_t clickCount = touch.hasClicks() ? touch.getClicks() : 0;

  if (clickCount == 1)
  {
    if (dawnFlag)
    {
      manualOff = true;
      dawnFlag = false;
      FastLED.setBrightness(modes[currentMode].Brightness);
      changePower();
    }
    else
    {
      ONflag = !ONflag;
      changePower();
    }
    settChanged = true;
    eepromTimeout = millis();
    loadingFlag = true;

    #if (USE_MQTT && ESP_MODE == 1)
    MqttManager::needToPublish = true;
    #endif
  }

  if (ONflag && clickCount == 2)
  {
    if (++currentMode >= MODE_AMOUNT) currentMode = 0;
    FastLED.setBrightness(modes[currentMode].Brightness);
    loadingFlag = true;
    settChanged = true;
    eepromTimeout = millis();
    FastLED.clear();
    delay(1);

    #if (USE_MQTT && ESP_MODE == 1)
    MqttManager::needToPublish = true;
    #endif
  }

  if (ONflag && clickCount == 3)
  {
    if (--currentMode < 0) currentMode = MODE_AMOUNT - 1;
    FastLED.setBrightness(modes[currentMode].Brightness);
    loadingFlag = true;
    settChanged = true;
    eepromTimeout = millis();
    FastLED.clear();
    delay(1);

    #if (USE_MQTT && ESP_MODE == 1)
    MqttManager::needToPublish = true;
    #endif
  }

  if (clickCount == 4)
  {
    #ifdef OTA
    if (otaManager.RequestOtaUpdate())
    {
      currentMode = EFF_MATRIX;                             // принудительное включение режима "Матрица" для индикации перехода в режим обновления по воздуху
      FastLED.clear();
      delay(1);
    }
    #endif
  }

  if (ONflag && clickCount == 5)                            // вывод IP на лампу
  {
    if (ESP_MODE == 1U)
    {
      loadingFlag = true;
      while(!fillString(WiFi.localIP().toString().c_str())) delay(1);
      loadingFlag = true;
    }
  }

  if (ONflag && touch.isHolded())
  {
    brightDirection = !brightDirection;
    startButtonHolding = true;
  }

  if (ONflag && touch.isStep())
  {
    uint8_t delta = modes[currentMode].Brightness < 10      // определение шага изменения яркости: при яркости [1..10] шаг = 1, при [11..16] шаг = 3, при [17..255] шаг = 15
      ? 1
      : 5;
    modes[currentMode].Brightness =
      constrain(brightDirection
        ? modes[currentMode].Brightness + delta
        : modes[currentMode].Brightness - delta,
      1, 255);
    FastLED.setBrightness(modes[currentMode].Brightness);
    settChanged = true;
    eepromTimeout = millis();

    #ifdef GENERAL_DEBUG
    LOG.printf_P(PSTR("New brightness value: %d\n"), modes[currentMode].Brightness);
    #endif
  }

  #if (USE_MQTT && ESP_MODE == 1)
  if (ONflag && !touch.isHold() && startButtonHolding)      // кнопка отпущена после удерживания, нужно отправить MQTT сообщение об изменении яркости лампы
  {
    MqttManager::needToPublish = true;
    startButtonHolding = false;
  }
  #endif
}
#endif
