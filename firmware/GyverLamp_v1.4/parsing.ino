void parseUDP()
{
  int32_t packetSize = Udp.parsePacket();

  if (packetSize)
  {
    int32_t n = Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    packetBuffer[n] = 0;
    inputBuffer = packetBuffer;

    #ifdef GENERAL_DEBUG
    Serial.print("Inbound UDP packet: ");
    Serial.println(inputBuffer);
    #endif

    if (inputBuffer.startsWith("DEB"))
    {
      inputBuffer =
        #ifdef USE_NTP
        "OK " + timeClient.getFormattedTime();
        #else
        "OK --:--";
        #endif
    }

    else if (inputBuffer.startsWith("GET"))
    {
      sendCurrent();
    }

    else if (inputBuffer.startsWith("EFF"))
    {
      EepromManager::SaveModesSettings(&currentMode, modes);
      currentMode = (byte)inputBuffer.substring(3).toInt();
      loadingFlag = true;
      FastLED.clear();
      delay(1);
      sendCurrent();
      FastLED.setBrightness(modes[currentMode].Brightness);
    }

    else if (inputBuffer.startsWith("BRI"))
    {
      modes[currentMode].Brightness = constrain(inputBuffer.substring(3).toInt(), 1, 255);
      FastLED.setBrightness(modes[currentMode].Brightness);
      settChanged = true;
      eepromTimeout = millis();
      sendCurrent();
    }

    else if (inputBuffer.startsWith("SPD"))
    {
      modes[currentMode].Speed = inputBuffer.substring(3).toInt();
      loadingFlag = true;
      settChanged = true;
      eepromTimeout = millis();
      sendCurrent();
    }

    else if (inputBuffer.startsWith("SCA"))
    {
      modes[currentMode].Scale = inputBuffer.substring(3).toInt();
      loadingFlag = true;
      settChanged = true;
      eepromTimeout = millis();
      sendCurrent();
    }

    else if (inputBuffer.startsWith("P_ON"))
    {
      ONflag = true;
      changePower();
      sendCurrent();
    }

    else if (inputBuffer.startsWith("P_OFF"))
    {
      ONflag = false;
      changePower();
      sendCurrent();
    }

    else if (inputBuffer.startsWith("ALM_SET"))
    {
      uint8_t alarmNum = (char)inputBuffer[7] - '0';
      alarmNum -= 1;
      if (inputBuffer.indexOf("ON") != -1)
      {
        alarms[alarmNum].State = true;
        sendAlarms();
      }
      else if (inputBuffer.indexOf("OFF") != -1)
      {
        alarms[alarmNum].State = false;
        sendAlarms();
      }
      else
      {
        int32_t alarmTime = inputBuffer.substring(8).toInt();
        alarms[alarmNum].Time = alarmTime;
        uint8_t hour = floor(alarmTime / 60);
        uint8_t minute = alarmTime - hour * 60;
        sendAlarms();
      }
      EepromManager::SaveAlarmsSettings(&alarmNum, alarms);
    }

    else if (inputBuffer.startsWith("ALM_GET"))
    {
      sendAlarms();
    }

    else if (inputBuffer.startsWith("DAWN")) 
    {
      dawnMode = inputBuffer.substring(4).toInt() - 1;
      EepromManager::SaveDawnMode(&dawnMode);
      sendAlarms();
    }

    else if (inputBuffer.startsWith("DISCOVER"))            // обнаружение приложением модуля esp в локальной сети
    {
      if (ESP_MODE == 1)                                    // работает только в режиме WiFi клиента
      {
        inputBuffer = "IP";
        inputBuffer += " ";
        inputBuffer += String(WiFi.localIP()[0]) + "." +
                       String(WiFi.localIP()[1]) + "." +
                       String(WiFi.localIP()[2]) + "." +
                       String(WiFi.localIP()[3]);
        inputBuffer += ":";
        inputBuffer += String(ESP_UDP_PORT);
      }
    }

    else if (inputBuffer.startsWith("TMR_GET"))
    {
      sendTimer();
    }

    else if (inputBuffer.startsWith("TMR_SET"))
    {
      TimerManager::TimerRunning = inputBuffer.substring(8, 9).toInt();
      TimerManager::TimerOption = inputBuffer.substring(10, 11).toInt();
      TimerManager::TimeToFire = millis() + (uint64_t)(inputBuffer.substring(12).toInt() * 1000);
      TimerManager::TimerHasFired = false;
      sendTimer();
    }

    else if (inputBuffer.startsWith("FAV_GET"))
    {
      sendFavorites();
    }

    else if (inputBuffer.startsWith("FAV_SET"))
    {
      FavoritesManager::ConfigureFavorites(inputBuffer.c_str());
      //FavoritesManager::SetStatus(inputBuffer);
      sendFavorites();
      settChanged = true;
      eepromTimeout = millis();
    }

    else if (inputBuffer.startsWith("OTA"))
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
      inputBuffer = "";
    }

    if (inputBuffer.length() <= 0)
    {
      return;
    }

    if (Udp.remoteIP() == WiFi.localIP())                   // не реагировать на свои же пакеты
    {
      return;
    }

    char reply[inputBuffer.length() + 1];
    inputBuffer.toCharArray(reply, inputBuffer.length() + 1);
    inputBuffer.remove(0);                                  // очистка буфера, читобы не он не интерпретировался, как следующий udp пакет
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(reply);
    Udp.endPacket();

    #ifdef GENERAL_DEBUG
    Serial.print("Outbound UDP packet: ");
    Serial.println(reply);
    Serial.println();
    #endif
  }
}

void sendCurrent()
{
  inputBuffer = "CURR";
  inputBuffer += " ";
  inputBuffer += String(currentMode);
  inputBuffer += " ";
  inputBuffer += String(modes[currentMode].Brightness);
  inputBuffer += " ";
  inputBuffer += String(modes[currentMode].Speed);
  inputBuffer += " ";
  inputBuffer += String(modes[currentMode].Scale);
  inputBuffer += " ";
  inputBuffer += String(ONflag);
  inputBuffer += " ";
  inputBuffer += String(ESP_MODE);
  inputBuffer += " ";
  #ifdef USE_NTP
  inputBuffer += "1";
  #else
  inputBuffer += "0";
  #endif
  inputBuffer += " ";
  inputBuffer += String((uint8_t)TimerManager::TimerRunning);
  inputBuffer += " ";
  #ifdef USE_NTP
  inputBuffer += timeClient.getFormattedTime();
  #else
  inputBuffer += String(millis());
  #endif
}

void sendAlarms()
{
  inputBuffer = "ALMS ";
  for (byte i = 0; i < 7; i++)
  {
    inputBuffer += String(alarms[i].State);
    inputBuffer += " ";
  }
  for (byte i = 0; i < 7; i++)
  {
    inputBuffer += String(alarms[i].Time);
    inputBuffer += " ";
  }
  inputBuffer += (dawnMode + 1);
}

void sendTimer()
{
  inputBuffer = "TMR";
  inputBuffer += " ";
  inputBuffer += String((uint8_t)TimerManager::TimerRunning);
  inputBuffer += " ";
  inputBuffer += String(TimerManager::TimerOption);
  inputBuffer += " ";
  inputBuffer += String(TimerManager::TimerRunning ? (uint16_t)floor((TimerManager::TimeToFire - millis()) / 1000) : 0);
}

void sendFavorites()
{
  inputBuffer = "FAV";
  inputBuffer += " ";
  inputBuffer += String((uint8_t)FavoritesManager::FavoritesRunning);
  inputBuffer += " ";
  inputBuffer += String((uint16_t)FavoritesManager::Interval);
  inputBuffer += " ";
  inputBuffer += String((uint16_t)FavoritesManager::Dispersion);
  for (uint8_t i = 0; i < MODE_AMOUNT; i++)
  {
    inputBuffer += " ";
    inputBuffer += String((uint8_t)FavoritesManager::FavoriteModes[i]);
  }
}
