// работа с бегущим текстом

// --- НАСТРОЙКИ -----------------------
#define TEXT_DIRECTION        (1U)                          // 1 - по горизонтали, 0 - по вертикали
#define MIRR_V                (0U)                          // отразить текст по вертикали (0 / 1)
#define MIRR_H                (0U)                          // отразить текст по горизонтали (0 / 1)

#define TEXT_HEIGHT           (0U)                          // высота, на которой бежит текст (от низа матрицы)
#define LET_WIDTH             (5U)                          // ширина буквы шрифта
#define LET_HEIGHT            (8U)                          // высота буквы шрифта
#define SPACE                 (1U)                          // пробел
#define LETTER_COLOR          (CRGB::White)                 // цвет букв по умолчанию


// --- ДЛЯ РАЗРАБОТЧИКОВ ---------------

int16_t offset = WIDTH;
uint32_t scrollTimer = 0LL;


bool fillString(const char* text, CRGB letterColor)
{
  if (!text || !strlen(text))
  {
    return true;
  }

  if (loadingFlag)
  {
    offset = WIDTH;                                         // перемотка в правый край
    loadingFlag = false;
  }

  if (millis() - scrollTimer >= 100)
  {
    scrollTimer = millis();
    FastLED.clear();
    uint8_t i = 0, j = 0;
    while (text[i] != '\0')
    {
      if ((uint8_t)text[i] > 191)                           // работаем с русскими буквами
      {
        i++;
      }
      else
      {
        drawLetter(text[i], offset + j * (LET_WIDTH + SPACE), letterColor);
        i++;
        j++;
      }
    }

    offset--;
    if (offset < (int16_t)(-j * (LET_WIDTH + SPACE)))       // строка убежала
    {
      offset = WIDTH + 3;
      return true;
    }
    FastLED.show();    
  }

  return false;
}


void printTime(uint32_t thisTime, bool onDemand, bool ONflag) // периодический вывод времени бегущей строкой; onDemand - по требованию, вывод текущего времени; иначе - вывод времени по расписанию
{
  #if defined(USE_NTP) && defined(PRINT_TIME)               // вывод, только если используется синхронизация времени и если заказан его вывод бегущей строкой

  if (espMode != 1U || !ntpServerAddressResolved || !timeSynched)     // вывод только в режиме WiFi клиента и только, если имя сервера времени разрезолвлено
  {
    showWarning(CRGB::Red, 4000U, 500U);                    // мигание красным цветом 4 секунды - смена рабочего режима лампы, перезагрузка
    return;
  }

  CRGB letterColor = CRGB::Black;
  bool needToPrint = false;
  
  #if (PRINT_TIME >= 1U)                                    // вывод только каждый час (красным цветом)
  if (thisTime % 60U == 0U)
  {
    needToPrint = true;
    letterColor = CRGB::Red;
  }
  #endif

  #if (PRINT_TIME == 2U)                                    // вывод каждый час (красным цветом) + каждые 30 минут (синим цветом)
  if (thisTime % 60U != 0U && thisTime % 30U == 0U)
  {
    needToPrint = true;
    letterColor = CRGB::Blue;
  }
  #endif

  #if (PRINT_TIME == 3U)                                    // вывод каждый час (красным цветом) + каждые 15 минут (синим цветом)
  if (thisTime % 60U != 0U && thisTime % 15U == 0U)
  {
    needToPrint = true;
    letterColor = CRGB::Blue;
  }
  #endif

  #if (PRINT_TIME == 4U)                                    // вывод каждый час (красным цветом) + каждые 10 минут (синим цветом)
  if (thisTime % 60U != 0U && thisTime % 10U == 0U)
  {
    needToPrint = true;
    letterColor = CRGB::Blue;
  }
  #endif

  #if (PRINT_TIME == 5U)                                    // вывод каждый час (красным цветом) + каждые 5 минут (синим цветом)
  if (thisTime % 60U != 0U && thisTime % 5U == 0U)
  {
    needToPrint = true;
    letterColor = CRGB::Blue;
  }
  #endif

  #if (PRINT_TIME == 6U)                                    // вывод каждый час (красным цветом) + каждую минуту (синим цветом)
  if (thisTime % 60U != 0U)
  {
    needToPrint = true;
    letterColor = CRGB::Blue;
  }
  #endif

  if (onDemand)
  {
    letterColor = CRGB::White;
  }

  if ((needToPrint && thisTime != lastTimePrinted) || onDemand)
  {
    lastTimePrinted = thisTime;
    char stringTime[10U];                                   // буффер для выводимого текста, его длина должна быть НЕ МЕНЬШЕ, чем длина текста + 1
    sprintf_P(stringTime, PSTR("-> %u:%02u"), (uint8_t)((thisTime - thisTime % 60U) / 60U), (uint8_t)(thisTime % 60U));
    loadingFlag = true;
    FastLED.setBrightness(getBrightnessForPrintTime(thisTime, ONflag));
    delay(1);

    #if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)        // установка сигнала в пин, управляющий MOSFET транзистором, матрица должна быть включена на время вывода текста
    digitalWrite(MOSFET_PIN, MOSFET_LEVEL);
    #endif

    while (!fillString(stringTime, letterColor)) { delay(1); ESP.wdtFeed(); }

    #if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)        // установка сигнала в пин, управляющий MOSFET транзистором, соответственно состоянию вкл/выкл матрицы или будильника
    digitalWrite(MOSFET_PIN, ONflag || (dawnFlag && !manualOff) ? MOSFET_LEVEL : !MOSFET_LEVEL);
    #endif

    loadingFlag = true;
  }

  #endif
}


uint8_t getBrightnessForPrintTime(uint32_t thisTime, bool ONflag)     // определение яркости для вывода времени бегущей строкой в зависимости от ESP_MODE, USE_NTP, успешности синхронизации времени,
                                                                      // текущего времени суток, настроек дневного/ночного времени и того, включена ли сейчас матрица
{
  #if defined(USE_NTP) && defined(PRINT_TIME)

  if (espMode != 1U || !ntpServerAddressResolved || ONflag)
  {
    return modes[currentMode].Brightness;
  }

  if (NIGHT_HOURS_START >= NIGHT_HOURS_STOP)                          // ночное время включает переход через полночь
  {
    if (thisTime >= NIGHT_HOURS_START || thisTime <= NIGHT_HOURS_STOP)// период действия ночного времени
    {
      return (NIGHT_HOURS_BRIGHTNESS >= 0)
        ? NIGHT_HOURS_BRIGHTNESS
        : modes[currentMode].Brightness;
    }
  }
  else                                                                // ночное время не включает переход через полночь
  {
    if (thisTime >= NIGHT_HOURS_START && thisTime <= NIGHT_HOURS_STOP)// период действия ночного времени
    {
      return (NIGHT_HOURS_BRIGHTNESS >= 0)
        ? NIGHT_HOURS_BRIGHTNESS
        : modes[currentMode].Brightness;
    }
  }

  return (DAY_HOURS_BRIGHTNESS >= 0)                                  // дневное время
    ? DAY_HOURS_BRIGHTNESS
    : modes[currentMode].Brightness;

  #endif

  return modes[currentMode].Brightness;
}


void drawLetter(uint8_t letter, int8_t offset, CRGB letterColor)
{
  uint8_t start_pos = 0, finish_pos = LET_WIDTH;

  if (offset < (int8_t)-LET_WIDTH || offset > (int8_t)WIDTH)
  {
    return;
  }
  if (offset < 0)
  {
    start_pos = (uint8_t)-offset;
  }
  if (offset > (int8_t)(WIDTH - LET_WIDTH))
  {
    finish_pos = (uint8_t)(WIDTH - offset);
  }

  for (uint8_t i = start_pos; i < finish_pos; i++)
  {
    uint8_t thisByte;
    if (MIRR_V)
    {
      thisByte = getFont(letter, (uint8_t)(LET_WIDTH - 1 - i));
    }
    else
    {
      thisByte = getFont(letter, i);
    }

    for (uint8_t j = 0; j < LET_HEIGHT; j++)
    {
      bool thisBit = MIRR_H
        ? thisByte & (1 << j)
        : thisByte & (1 << (LET_HEIGHT - 1 - j));

      // рисуем столбец (i - горизонтальная позиция, j - вертикальная)
      if (TEXT_DIRECTION)
      {
        if (thisBit)
        {
          leds[getPixelNumber(offset + i, TEXT_HEIGHT + j)] = letterColor;
        }
        else
        {
          drawPixelXY(offset + i, TEXT_HEIGHT + j, 0x000000);
        }
      }
      else
      {
        if (thisBit)
        {
          leds[getPixelNumber(i, offset + TEXT_HEIGHT + j)] = letterColor;
        }
        else
        {
          drawPixelXY(i, offset + TEXT_HEIGHT + j, 0x000000);
        }
      }
    }
  }
}


// --- СЛУЖЕБНЫЕ ФУНКЦИИ ---------------
uint8_t getFont(uint8_t asciiCode, uint8_t row)             // интерпретатор кода символа в массиве fontHEX (для Arduino IDE 1.8.* и выше)
{
  asciiCode = asciiCode - '0' + 16;                         // перевод код символа из таблицы ASCII в номер согласно нумерации массива

  if (asciiCode <= 90)                                      // печатаемые символы и английские буквы
  {
    return pgm_read_byte(&fontHEX[asciiCode][row]);
  }
  else if (asciiCode >= 112 && asciiCode <= 159)
  {
    return pgm_read_byte(&fontHEX[asciiCode - 17][row]);
  }
  else if (asciiCode >= 96 && asciiCode <= 111)
  {
    return pgm_read_byte(&fontHEX[asciiCode + 47][row]);
  }

  return 0;
}
