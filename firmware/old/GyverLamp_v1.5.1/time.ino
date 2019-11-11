byte minuteCounter = 0;
void timeTick() {
  if (ESP_MODE == 1) {
    if (timeTimer.isReady()) {  // каждую секунду
      secs++;
      if (secs == 60) {
        secs = 0;
        mins++;
        minuteCounter++;
      }
      if (mins == 60) {
        mins = 0;
        hrs++;
        if (hrs == 24) {
          hrs = 0;
          days++;
          if (days > 6) days = 0;
        }
      }

      if (minuteCounter > 30 && WiFi.status() == WL_CONNECTED) {    // синхронизация каждые 30 минут
        minuteCounter = 0;
        if (timeClient.update()) {
          hrs = timeClient.getHours();
          mins = timeClient.getMinutes();
          secs = timeClient.getSeconds();
          days = timeClient.getDay();
        }
      }

      if (secs % 3 == 0) checkDawn();   // каждые 3 секунды проверяем рассвет
    }
  }
}

void checkDawn() {
  byte thisDay = days;
  if (thisDay == 0) thisDay = 7;  // воскресенье это 0
  thisDay--;
  thisTime = hrs * 60 + mins;

  // проверка рассвета
  if (alarm[thisDay].state &&                                       // день будильника
      thisTime >= (alarm[thisDay].time - dawnOffsets[dawnMode]) &&  // позже начала
      thisTime < (alarm[thisDay].time + DAWN_TIMEOUT) ) {                      // раньше конца + минута
    if (!manualOff) {
      // величина рассвета 0-255
      int dawnPosition = 255 * ((float)(thisTime - (alarm[thisDay].time - dawnOffsets[dawnMode])) / dawnOffsets[dawnMode]);
      dawnPosition = constrain(dawnPosition, 0, 255);
      CHSV dawnColor = CHSV(map(dawnPosition, 0, 255, 10, 35),
                            map(dawnPosition, 0, 255, 255, 170),
                            map(dawnPosition, 0, 255, 10, DAWN_BRIGHT));
      fill_solid(leds, NUM_LEDS, dawnColor);
      FastLED.setBrightness(255);
      FastLED.show();
      dawnFlag = true;
    }
  } else {
    if (dawnFlag) {
      dawnFlag = false;
      manualOff = false;
      FastLED.setBrightness(modes[currentMode].brightness);
    }
  }
}
