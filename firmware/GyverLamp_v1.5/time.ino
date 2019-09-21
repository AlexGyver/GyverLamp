void timeTick() {
  if (ESP_MODE == 1) {    
    if (timeTimer.isReady()) {
      timeClient.update();
      byte thisDay = timeClient.getDay();
      if (thisDay == 0) thisDay = 7;  // воскресенье это 0
      thisDay--;
      thisTime = timeClient.getHours() * 60 + timeClient.getMinutes();

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
  }
}
