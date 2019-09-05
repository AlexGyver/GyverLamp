#ifdef ESP_USE_BUTTON
bool brightDirection;

void buttonTick()
{
  touch.tick();
  if (touch.isSingle())
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
    loadingFlag = true;
  }

  if (ONflag && touch.isDouble())
  {
    if (++currentMode >= MODE_AMOUNT) currentMode = 0;
    FastLED.setBrightness(modes[currentMode].Brightness);
    loadingFlag = true;
    settChanged = true;
    eepromTimeout = millis();
    FastLED.clear();
    delay(1);
  }

  if (ONflag && touch.isTriple())
  {
    if (--currentMode < 0) currentMode = MODE_AMOUNT - 1;
    FastLED.setBrightness(modes[currentMode].Brightness);
    loadingFlag = true;
    settChanged = true;
    eepromTimeout = millis();
    FastLED.clear();
    delay(1);
  }

  if (ONflag && touch.hasClicks() && touch.getClicks() >= 4)
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

  if (ONflag && touch.isHolded())
  {
    brightDirection = !brightDirection;
  }

  if (ONflag && touch.isStep())
  {
    if (brightDirection)
    {
      if (modes[currentMode].Brightness < 10) modes[currentMode].Brightness += 1;
      else if (modes[currentMode].Brightness < 250) modes[currentMode].Brightness += 5;
      else modes[currentMode].Brightness = 255;
    }
    else
    {
      if (modes[currentMode].Brightness > 15) modes[currentMode].Brightness -= 5;
      else if (modes[currentMode].Brightness > 1) modes[currentMode].Brightness -= 1;
      else modes[currentMode].Brightness = 0;
    }
    FastLED.setBrightness(modes[currentMode].Brightness);
    settChanged = true;
    eepromTimeout = millis();

    #ifdef GENERAL_DEBUG
    Serial.printf_P(PSTR("New brightness value: %d\n"), modes[currentMode].Brightness);
    #endif
  }
}
#endif
