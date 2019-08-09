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
      saveEEPROM();
      currentMode = (byte)inputBuffer.substring(3).toInt();
      loadingFlag = true;
      FastLED.clear();
      delay(1);
      sendCurrent();
      FastLED.setBrightness(modes[currentMode].brightness);
    }

    else if (inputBuffer.startsWith("BRI"))
    {
      modes[currentMode].brightness = constrain(inputBuffer.substring(3).toInt(), 1, 255);
      FastLED.setBrightness(modes[currentMode].brightness);
      settChanged = true;
      eepromTimer = millis();
      sendCurrent();
    }

    else if (inputBuffer.startsWith("SPD"))
    {
      modes[currentMode].speed = inputBuffer.substring(3).toInt();
      loadingFlag = true;
      settChanged = true;
      eepromTimer = millis();
      sendCurrent();
    }

    else if (inputBuffer.startsWith("SCA"))
    {
      modes[currentMode].scale = inputBuffer.substring(3).toInt();
      loadingFlag = true;
      settChanged = true;
      eepromTimer = millis();
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
      byte alarmNum = (char)inputBuffer[7] - '0';
      alarmNum -= 1;
      if (inputBuffer.indexOf("ON") != -1)
      {
        alarm[alarmNum].state = true;
        sendAlarms();
      }
      else if (inputBuffer.indexOf("OFF") != -1)
      {
        alarm[alarmNum].state = false;
        sendAlarms();
      }
      else
      {
        int32_t almTime = inputBuffer.substring(8).toInt();
        alarm[alarmNum].time = almTime;
        byte hour = floor(almTime / 60);
        byte minute = almTime - hour * 60;
        sendAlarms();
      }
      saveAlarm(alarmNum);
    }

    else if (inputBuffer.startsWith("ALM_GET"))
    {
      sendAlarms();
    }

    else if (inputBuffer.startsWith("DAWN")) 
    {
      dawnMode = inputBuffer.substring(4).toInt() - 1;
      saveDawnMmode();
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
  inputBuffer += String(modes[currentMode].brightness);
  inputBuffer += " ";
  inputBuffer += String(modes[currentMode].speed);
  inputBuffer += " ";
  inputBuffer += String(modes[currentMode].scale);
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
  #ifdef USE_NTP
  inputBuffer += timeClient.getFormattedTime();
  #else
  inputBuffer += String(millis());
  #endif

  #ifdef GENERAL_DEBUG
  Serial.println(inputBuffer);
  #endif
}

void sendAlarms()
{
  inputBuffer = "ALMS ";
  for (byte i = 0; i < 7; i++) {
    inputBuffer += String(alarm[i].state);
    inputBuffer += " ";
  }
  for (byte i = 0; i < 7; i++) {
    inputBuffer += String(alarm[i].time);
    inputBuffer += " ";
  }
  inputBuffer += (dawnMode + 1);
}
