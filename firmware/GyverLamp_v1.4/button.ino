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
      loadingFlag = true;
      FastLED.setBrightness(modes[currentMode].Brightness);
      changePower();
    }
    else
    {
      ONflag = !ONflag;
      changePower();
    }
  }

  if (ONflag && touch.isDouble())
  {
    if (++currentMode >= MODE_AMOUNT) currentMode = 0;
    FastLED.setBrightness(modes[currentMode].Brightness);
    loadingFlag = true;
    settChanged = true;
    eepromTimer = millis();
    FastLED.clear();
    delay(1);
  }

  if (ONflag && touch.isTriple())
  {
    if (--currentMode < 0) currentMode = MODE_AMOUNT - 1;
    FastLED.setBrightness(modes[currentMode].Brightness);
    loadingFlag = true;
    settChanged = true;
    eepromTimer = millis();
    FastLED.clear();
    delay(1);
  }

  if (ONflag && touch.hasClicks() && touch.getClicks() >= 4)
  {
    #ifdef OTA
    if (otaManager.RequestOtaUpdate())
    {
      currentMode = 16;                                     // принудительное включение режима "Матрица" для индикации перехода в режим обновления по воздуху
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
    eepromTimer = millis();

    #ifdef GENERAL_DEBUG
    Serial.printf("New brightness value: %d\n", modes[currentMode].Brightness);
    #endif
  }
}
#endif
