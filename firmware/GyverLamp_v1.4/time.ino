#ifdef USE_NTP

#define RESOLVE_INTERVAL (5UL * 60UL * 1000UL)                            // интервал проверки подключения к интеренету в миллисекундах (5 минут)
                                                                          // при старте ESP пытается получить точное время от сервера времени в интрнете
                                                                          // эта попытка длится RESOLVE_TIMEOUT
                                                                          // если при этом отсутствует подключение к интернету (но есть WiFi подключение),
                                                                          // модуль будет подвисать на RESOLVE_TIMEOUT каждое срабатывание таймера, т.е., 3 секунды
                                                                          // чтобы избежать этого, будем пытаться узнать состояние подключения 1 раз в RESOLVE_INTERVAL (5 минут)
                                                                          // попытки будут продолжаться до первой успешной синхронизации времени
                                                                          // до этого момента функции будильника работать не будут
                                                                          // интервал последующих синхронизаций времени определяён в NTP_INTERVAL (30 минут)
                                                                          // при ошибках повторной синхронизации времени функции будильника отключаться не будут
#define RESOLVE_TIMEOUT  (1500UL)                                         // таймаут ожидания подключения к интернету в миллисекундах (1,5 секунды)
uint64_t lastResolveTryMoment = 0UL;
bool timeSynched = false;
bool ntpServerAddressResolved = false;
IPAddress ntpServerIp = {0, 0, 0, 0};

void timeTick()
{
  if (ESP_MODE == 1)
  {
    if (timeTimer.isReady())
    {
      if (!timeSynched)
      {
        if (millis() - lastResolveTryMoment >= RESOLVE_INTERVAL || lastResolveTryMoment == 0)
        {
          resolveNtpServerAddress(ntpServerAddressResolved);              // пытаемся получить IP адрес сервера времени (тест интернет подключения) до тех пор, пока время не будет успешно синхронизировано
          lastResolveTryMoment = millis();
          if (!ntpServerAddressResolved)
          {
            #ifdef GENERAL_DEBUG
            Serial.println("Функции будильника отключены до восстановления подключения к интернету");
            #endif
          }
        }
        if (!ntpServerAddressResolved)
        {
          return;                                                         // если нет интернет подключения, отключаем будильник до тех пор, пока оно не будет восстановлено
        }
      }

      timeSynched = timeSynched || timeClient.update();                   // если время хотя бы один раз было синхронизировано, продолжаем
      if (!timeSynched)                                                   // если время не было синхронизиировано ни разу, отключаем будильник до тех пор, пока оно не будет синхронизировано
      {
        return;
      }
      byte thisDay = timeClient.getDay();
      if (thisDay == 0) thisDay = 7;                                      // воскресенье это 0
      thisDay--;
      thisTime = timeClient.getHours() * 60 + timeClient.getMinutes();

      // проверка рассвета
      if (alarm[thisDay].state &&                                         // день будильника
          thisTime >= (alarm[thisDay].time - dawnOffsets[dawnMode]) &&    // позже начала
          thisTime < (alarm[thisDay].time + DAWN_TIMEOUT))                // раньше конца + минута
      {
        if (!manualOff)                                                   // будильник не был выключен вручную (из приложения или кнопкой)
        {
          // величина рассвета 0-255
          int32_t dawnPosition = 255 * ((float)(thisTime - (alarm[thisDay].time - dawnOffsets[dawnMode])) / dawnOffsets[dawnMode]);
          dawnPosition = constrain(dawnPosition, 0, 255);
          CHSV dawnColor = CHSV(map(dawnPosition, 0, 255, 10, 35),
                                map(dawnPosition, 0, 255, 255, 170),
                                map(dawnPosition, 0, 255, 10, DAWN_BRIGHT));
          fill_solid(leds, NUM_LEDS, dawnColor);
          FastLED.setBrightness(255);
          delay(1);
          FastLED.show();
          dawnFlag = true;
        }
      }
      else
      {
        if (dawnFlag)
        {
          dawnFlag = false;
          manualOff = false;
          FastLED.clear();
          delay(2);
          FastLED.show();
          changePower();                                                  // выключение матрицы или установка яркости текущего эффекта в засисимости от того, была ли включена лампа до срабатывания будильника
        }
      }
    }
  }
}

void resolveNtpServerAddress(bool &ntpServerAddressResolved)              // функция проверки подключения к интернету
{
  if (ntpServerAddressResolved)
  {
    return;
  }

  WiFi.hostByName(NTP_ADDRESS, ntpServerIp, RESOLVE_TIMEOUT);
  if (ntpServerIp[0] <= 0)
  {
    #ifdef GENERAL_DEBUG
    if (ntpServerAddressResolved)
    {
      Serial.println("Подключение к интернету отсутствует");
    }
    #endif

    ntpServerAddressResolved = false;
  }
  else
  {
    #ifdef GENERAL_DEBUG
    if (!ntpServerAddressResolved)
    {
      Serial.println("Подключение к интернету установлено");
    }
    #endif

    ntpServerAddressResolved = true;
  }
}
#endif USE_NTP
