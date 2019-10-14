#ifdef ESP_USE_BUTTON
bool brightDirection;

static bool startButtonHolding = false;                     // флаг: кнопка удерживается для изменения яркости лампы

void buttonTick()
{
  touch.tick();
  uint8_t clickCount = touch.hasClicks() ? touch.getClicks() : 0U;

  if (clickCount == 1U)
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

    #if (USE_MQTT)
    if (espMode == 1U)
    {
      MqttManager::needToPublish = true;
    }
    #endif
  }

  if (ONflag && clickCount == 2U)
  {
    if (++currentMode >= (int8_t)MODE_AMOUNT) currentMode = 0;
    FastLED.setBrightness(modes[currentMode].Brightness);
    loadingFlag = true;
    settChanged = true;
    eepromTimeout = millis();
    FastLED.clear();
    delay(1);

    #if (USE_MQTT)
    if (espMode == 1U)
    {
      MqttManager::needToPublish = true;
    }
    #endif
  }

  if (ONflag && clickCount == 3U)
  {
    if (--currentMode < 0) currentMode = MODE_AMOUNT - 1;
    FastLED.setBrightness(modes[currentMode].Brightness);
    loadingFlag = true;
    settChanged = true;
    eepromTimeout = millis();
    FastLED.clear();
    delay(1);

    #if (USE_MQTT)
    if (espMode == 1U)
    {
      MqttManager::needToPublish = true;
    }
    #endif
  }

  if (clickCount == 4U)
  {
    #ifdef OTA
    if (otaManager.RequestOtaUpdate())
    {
      ONflag = true;
      currentMode = EFF_MATRIX;                             // принудительное включение режима "Матрица" для индикации перехода в режим обновления по воздуху
      FastLED.clear();
      delay(1);
      changePower();
    }
    #endif
  }

  if (clickCount == 5U)                                     // вывод IP на лампу
  {
    if (espMode == 1U)
    {
      loadingFlag = true;
      while(!fillString(WiFi.localIP().toString().c_str(), CRGB::White)) { delay(1); ESP.wdtFeed(); }
      loadingFlag = true;
    }
  }

  if (clickCount == 6U)                                     // вывод текущего времени бегущей строкой
  {
    printTime(thisTime, true);
  }

  if (ONflag && clickCount == 7U)                           // смена рабочего режима лампы: с WiFi точки доступа на WiFi клиент или наоборот
  {
    espMode = (espMode == 0U) ? 1U : 0U;
    EepromManager::SaveEspMode(&espMode);

    #ifdef GENERAL_DEBUG
    LOG.printf_P(PSTR("Рабочий режим лампы изменён и сохранён в энергонезависимую память\nНовый рабочий режим: ESP_MODE = %d, %s\nРестарт...\n"),
      espMode, espMode == 0U ? F("WiFi точка доступа") : F("WiFi клиент (подключение к роутеру)"));
    delay(1000);
    #endif

    showWarning(CRGB::Red, 3000U, 500U);                    // мигание красным цветом 3 секунды - смена рабочего режима лампы, перезагрузка
    ESP.restart();
  }
    

  if (ONflag && touch.isHolded())
  {
    brightDirection = !brightDirection;
    startButtonHolding = true;
  }

  if (ONflag && touch.isStep())
  {
    uint8_t delta = modes[currentMode].Brightness < 10U     // определение шага изменения яркости: при яркости [1..10] шаг = 1, при [11..16] шаг = 3, при [17..255] шаг = 15
      ? 1U
      : 5U;
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

  #if (USE_MQTT)
  if (espMode == 1U && ONflag && !touch.isHold() && startButtonHolding)         // кнопка отпущена после удерживания, нужно отправить MQTT сообщение об изменении яркости лампы
  {
    MqttManager::needToPublish = true;
    startButtonHolding = false;
  }
  #endif
}
#endif
