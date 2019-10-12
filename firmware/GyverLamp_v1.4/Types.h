#pragma once


struct AlarmType
{
  bool State = false;
  uint16_t Time = 0U;
};

struct ModeType
{
  uint8_t Brightness = 50U;
  uint8_t Speed = 30U;
  uint8_t Scale = 40U;
};

typedef void (*SendCurrentDelegate)(char *outputBuffer);
typedef void (*ShowWarningDelegate)(CRGB color, uint32_t duration, uint16_t blinkHalfPeriod);
