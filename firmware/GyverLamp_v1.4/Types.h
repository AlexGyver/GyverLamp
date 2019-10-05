#pragma once


struct AlarmType
{
  bool State = false;
  uint16_t Time = 0;
};

struct ModeType
{
  uint8_t Brightness = 50;
  uint8_t Speed = 30;
  uint8_t Scale = 40;
};

typedef void (*SendCurrentDelegate)(char *outputBuffer);
