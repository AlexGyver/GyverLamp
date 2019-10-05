#if defined(GENERAL_DEBUG) && GENERAL_DEBUG_TELNET

void handleTelnetClient()
{
  if (telnetServer.hasClient())
  {
    if (!telnet || !telnet.connected())
    {
      if (telnet)
      {
        telnet.stop();                                      // клиент отключился
        telnetGreetingShown = false;
      }
      telnet = telnetServer.available();                    // готов к подключению нового клиента
    }
    else
    {
      telnetServer.available().stop();                      // один клиент уже подключен, блокируем подключение нового
      telnetGreetingShown = false;
    }
  }

  if (telnet && telnet.connected() && telnet.available())
  {
    if (!telnetGreetingShown)
    {
      telnet.println("Подключение к устройтву по протоколу telnet установлено\n-------");
      telnetGreetingShown = true;
    }
  }
}

#endif
