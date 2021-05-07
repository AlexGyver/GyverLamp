#ifndef LED_H
#define LED_H

#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif
#include <FastLED.h>
#include "LedEffect.h"
#include "LedSettings.h"

uint32_t previousMillis = 0;
uint32_t interval = 66;

// uint8_t speed = 255;
// uint8_t scale = 255;

RainbowVertical rotateRainbow;
// RotateRainbow rotateRainbow;
Effect &currentEffect = rotateRainbow;

class Led
{
public:
  static void update()
  {
    uint32_t ms = millis();
    if (ms - previousMillis > interval)
    {
      previousMillis = ms;
      currentEffect.update(ms);
      FastLED.show();
    }
  }

  static void setup()
  {
    FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 100);
    FastLED.setBrightness(BRIGHTNESS);
  }
};
#endif