#ifdef ESP_USE_BUTTON
boolean brightDirection;

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
      FastLED.setBrightness(modes[currentMode].brightness);
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
    FastLED.setBrightness(modes[currentMode].brightness);
    loadingFlag = true;
    settChanged = true;
    eepromTimer = millis();
    FastLED.clear();
    delay(1);
  }

  if (ONflag && touch.isTriple())
  {
    if (--currentMode < 0) currentMode = MODE_AMOUNT - 1;
    FastLED.setBrightness(modes[currentMode].brightness);
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
      if (modes[currentMode].brightness < 10) modes[currentMode].brightness += 1;
      else if (modes[currentMode].brightness < 250) modes[currentMode].brightness += 5;
      else modes[currentMode].brightness = 255;
    }
    else
    {
      if (modes[currentMode].brightness > 15) modes[currentMode].brightness -= 5;
      else if (modes[currentMode].brightness > 1) modes[currentMode].brightness -= 1;
      else modes[currentMode].brightness = 0;
    }
    FastLED.setBrightness(modes[currentMode].brightness);
    settChanged = true;
    eepromTimer = millis();

    #ifdef GENERAL_DEBUG
    Serial.printf("New brightness value: %d\n", modes[currentMode].brightness);
    #endif
  }
}
#endif
