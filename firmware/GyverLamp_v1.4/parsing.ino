void parseUDP()
{
  int32_t packetSize = Udp.parsePacket();
  char buff[MAX_UDP_BUFFER_SIZE], *endToken = NULL;

  if (packetSize)
  {
    int16_t n = Udp.read(packetBuffer, MAX_UDP_BUFFER_SIZE);
    packetBuffer[n] = '\0';
    strcpy(inputBuffer, packetBuffer);

    #ifdef GENERAL_DEBUG
    Serial.print(F("Inbound UDP packet: "));
    Serial.println(inputBuffer);
    #endif

    if (!strncmp_P(inputBuffer, PSTR("DEB"), 3))
    {
        #ifdef USE_NTP
        sprintf_P(inputBuffer, PSTR("%s%s"), PSTR("OK "), timeClient.getFormattedTime().c_str());
        #else
        strcpy_P(inputBuffer, PSTR("OK --:--"));
        #endif
    }

    else if (!strncmp_P(inputBuffer, PSTR("GET"), 3))
    {
      sendCurrent();
    }

    else if (!strncmp_P(inputBuffer, PSTR("EFF"), 3))
    {
      EepromManager::SaveModesSettings(&currentMode, modes);
      memcpy(buff, &inputBuffer[3], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 4
      currentMode = (uint8_t)atoi(buff);
      loadingFlag = true;
      FastLED.clear();
      delay(1);
      sendCurrent();
      FastLED.setBrightness(modes[currentMode].Brightness);
    }

    else if (!strncmp_P(inputBuffer, PSTR("BRI"), 3))
    {
      memcpy(buff, &inputBuffer[3], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 4
      modes[currentMode].Brightness = constrain(atoi(buff), 1, 255);
      FastLED.setBrightness(modes[currentMode].Brightness);
      loadingFlag = true;
      settChanged = true;
      eepromTimeout = millis();
      sendCurrent();
    }

    else if (!strncmp_P(inputBuffer, PSTR("SPD"), 3))
    {
      memcpy(buff, &inputBuffer[3], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 4
      modes[currentMode].Speed = atoi(buff);
      loadingFlag = true;
      settChanged = true;
      eepromTimeout = millis();
      sendCurrent();
    }

    else if (!strncmp_P(inputBuffer, PSTR("SCA"), 3))
    {
      memcpy(buff, &inputBuffer[3], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 4
      modes[currentMode].Scale = atoi(buff);
      loadingFlag = true;
      settChanged = true;
      eepromTimeout = millis();
      sendCurrent();
    }

    else if (!strncmp_P(inputBuffer, PSTR("P_ON"), 4))
    {
      ONflag = true;
      loadingFlag = true;
      changePower();
      sendCurrent();
    }

    else if (!strncmp_P(inputBuffer, PSTR("P_OFF"), 5))
    {
      ONflag = false;
      changePower();
      sendCurrent();
    }

    else if (!strncmp_P(inputBuffer, PSTR("ALM_SET"), 7))
    {
      uint8_t alarmNum = (char)inputBuffer[7] - '0';
      alarmNum -= 1;
      if (strstr_P(inputBuffer, PSTR("ON")) - inputBuffer == 9)
      {
        alarms[alarmNum].State = true;
        sendAlarms();
      }
      else if (strstr_P(inputBuffer, PSTR("OFF")) - inputBuffer == 9)
      {
        alarms[alarmNum].State = false;
        sendAlarms();
      }
      else
      {
        memcpy(buff, &inputBuffer[8], strlen(inputBuffer)); // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 9
        alarms[alarmNum].Time = atoi(buff);
        uint8_t hour = floor(alarms[alarmNum].Time / 60);
        uint8_t minute = alarms[alarmNum].Time - hour * 60;
        sendAlarms();
      }
      EepromManager::SaveAlarmsSettings(&alarmNum, alarms);
    }

    else if (!strncmp_P(inputBuffer, PSTR("ALM_GET"), 7))
    {
      sendAlarms();
    }

    else if (!strncmp_P(inputBuffer, PSTR("DAWN"), 4))
    {
      memcpy(buff, &inputBuffer[4], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 5
      dawnMode = atoi(buff) - 1;
      EepromManager::SaveDawnMode(&dawnMode);
      sendAlarms();
    }

    else if (!strncmp_P(inputBuffer, PSTR("DISCOVER"), 8))  // обнаружение приложением модуля esp в локальной сети
    {
      if (ESP_MODE == 1)                                    // работает только в режиме WiFi клиента
      {
        sprintf_P(inputBuffer, PSTR("IP %u.%u.%u.%u:%u"),
          WiFi.localIP()[0],
          WiFi.localIP()[1],
          WiFi.localIP()[2],
          WiFi.localIP()[3],
          ESP_UDP_PORT);
      }
    }

    else if (!strncmp_P(inputBuffer, PSTR("TMR_GET"), 7))
    {
      sendTimer();
    }

    else if (!strncmp_P(inputBuffer, PSTR("TMR_SET"), 7))
    {
      memcpy(buff, &inputBuffer[8], 2);                     // взять подстроку, состоящую из 9 и 10 символов, из строки inputBuffer
      TimerManager::TimerRunning = (bool)atoi(buff);

      memcpy(buff, &inputBuffer[10], 2);                    // взять подстроку, состоящую из 11 и 12 символов, из строки inputBuffer
      TimerManager::TimerOption = (uint8_t)atoi(buff);

      memcpy(buff, &inputBuffer[12], strlen(inputBuffer));  // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 13
      TimerManager::TimeToFire = millis() + strtoull(buff, &endToken, 10) * 1000;

      TimerManager::TimerHasFired = false;
      sendTimer();
    }

    else if (!strncmp_P(inputBuffer, PSTR("FAV_GET"), 7))
    {
        FavoritesManager::SetStatus(inputBuffer);
    }

    else if (!strncmp_P(inputBuffer, PSTR("FAV_SET"), 7))
    {
      FavoritesManager::ConfigureFavorites(inputBuffer);
      FavoritesManager::SetStatus(inputBuffer);
      settChanged = true;
      eepromTimeout = millis();
    }

    else if (!strncmp_P(inputBuffer, PSTR("OTA"), 3))
    {
      #ifdef OTA
      otaManager.RequestOtaUpdate();
      delay(50);
      otaManager.RequestOtaUpdate();
      currentMode = EFF_MATRIX;                             // принудительное включение режима "Матрица" для индикации перехода в режим обновления по воздуху
      FastLED.clear();
      delay(1);
      ONflag = true;
      changePower();
      #endif
    }

    else
    {
      inputBuffer[0] = '\0';
    }

    if (strlen(inputBuffer) <= 0)
    {
      return;
    }

    if (Udp.remoteIP() == WiFi.localIP())                   // не реагировать на свои же пакеты
    {
      return;
    }

    char reply[strlen(inputBuffer) + 1];
    strcpy(reply, inputBuffer);
    inputBuffer[0] = '\0';                                  // очистка буфера, читобы не он не интерпретировался, как следующий udp пакет
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(reply);
    Udp.endPacket();

    #ifdef GENERAL_DEBUG
    Serial.print(F("Outbound UDP packet: "));
    Serial.println(reply);
    Serial.println();
    #endif
  }
}

void sendCurrent()
{
  sprintf_P(inputBuffer, PSTR("CURR %u %u %u %u %u %u"),
    currentMode,
    modes[currentMode].Brightness,
    modes[currentMode].Speed,
    modes[currentMode].Scale,
    ONflag,
    ESP_MODE);
  
  #ifdef USE_NTP
  strcat_P(inputBuffer, PSTR(" 1"));
  #else
  strcat_P(inputBuffer, PSTR(" 0"));
  #endif

  sprintf_P(inputBuffer, PSTR("%s %u"), inputBuffer, (uint8_t)TimerManager::TimerRunning);

  #ifdef USE_NTP
  sprintf_P(inputBuffer, PSTR("%s %s"), inputBuffer, timeClient.getFormattedTime().c_str());
  #else
  sprintf_P(inputBuffer, PSTR("%s %ull"), inputBuffer, millis());
  #endif
}

void sendAlarms()
{
  strcpy_P(inputBuffer, PSTR("ALMS"));

  for (byte i = 0; i < 7; i++)
  {
    sprintf_P(inputBuffer, PSTR("%s %u"), inputBuffer, (uint8_t)alarms[i].State);
  }

  for (byte i = 0; i < 7; i++)
  {
    sprintf_P(inputBuffer, PSTR("%s %u"), inputBuffer, alarms[i].Time);
  }

  sprintf_P(inputBuffer, PSTR("%s %u"), inputBuffer, dawnMode + 1);
}

void sendTimer()
{
  sprintf_P(inputBuffer, PSTR("TMR %u %u %u"),
    TimerManager::TimerRunning,
    TimerManager::TimerOption,
   (TimerManager::TimerRunning ? (uint16_t)floor((TimerManager::TimeToFire - millis()) / 1000) : 0));
}
