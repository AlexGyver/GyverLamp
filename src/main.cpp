#include <Arduino.h>
#include "Led.h"


void setup()
{
  Led::setup();
  Serial.begin(115200);
}

void loop()
{
  Led::update();
}