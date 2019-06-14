boolean brightDirection;

void buttonTick() {
  touch.tick();
  if (touch.isSingle()) {
    if (dawnFlag) {
      manualOff = true;
      dawnFlag = false;
      loadingFlag = true;
      FastLED.setBrightness(modes[currentMode].brightness);
      changePower();
    } else {
      if (ONflag) {
        ONflag = false;
        changePower();
      } else {
        ONflag = true;
        changePower();
      }
    }
  }

  if (ONflag && touch.isDouble()) {
    if (++currentMode >= MODE_AMOUNT) currentMode = 0;
    FastLED.setBrightness(modes[currentMode].brightness);
    loadingFlag = true;
    settChanged = true;
    eepromTimer = millis();
  }

  if (ONflag && touch.isHolded()) {
    brightDirection = !brightDirection;
  }
  if (ONflag && touch.isStep()) {
    if (brightDirection) {
      if (modes[currentMode].brightness < 250) modes[currentMode].brightness += 5;
      else modes[currentMode].brightness = 255;
    } else {
      if (modes[currentMode].brightness >= 5) modes[currentMode].brightness -= 5;
      else modes[currentMode].brightness = 0;
    }
    FastLED.setBrightness(modes[currentMode].brightness);
    settChanged = true;
    eepromTimer = millis();
  }
}
