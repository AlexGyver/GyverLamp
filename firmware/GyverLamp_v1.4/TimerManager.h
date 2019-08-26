#pragma once


class TimerManager
{
  public:
    static bool TimerRunning;                               // флаг "таймер взведён"
    static bool TimerHasFired;                              // флаг "таймер отработал"
    static uint8_t TimerOption;                             // индекс элемента в списке List Picker'а
    static uint64_t TimeToFire;                             // время, в которое должен сработать таймер (millis)

    static void HandleTimer(                                // функция, обрабатывающая срабатывание таймера, гасит матрицу
      bool* ONflag,
      void (*changePower)())
    {
      if (!TimerManager::TimerHasFired &&
           TimerManager::TimerRunning &&
           millis() >= TimerManager::TimeToFire)
      {
        #ifdef GENERAL_DEBUG
        Serial.printf("Выключение по таймеру\n\n");
        #endif

        TimerManager::TimerRunning = false;
        TimerManager::TimerHasFired = true;
        FastLED.clear();
        delay(2);
        FastLED.show();
        *ONflag = !(*ONflag);
        changePower();
      }
    }
};
