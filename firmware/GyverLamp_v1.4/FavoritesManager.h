#pragma once
#include <EEPROM.h>
#include "EepromManager.h"

#define DEFAULT_FAVORITES_INTERVAL           (300U)         // значение по умолчанию для интервала переключения избпранных эффектов в секундах
#define DEFAULT_FAVORITES_DISPERSION         (0U)           // значение по умолчанию для разброса интервала переключения избпранных эффектов в секундах


class FavoritesManager
{
  public:
    static bool FavoritesRunning;                           // флаг "работает режим автоматической смены избранных эффектов"
    static uint16_t Interval;                               // статический интервал (время между сменами эффектов)
    static uint16_t Dispersion;                             // дополнительный динамический (случайный) интервал (время между сменами эффектов)
    static uint8_t FavoriteModes[MODE_AMOUNT];              // массив, каждый элемент которого соответствует флагу "эффект №... добавлен в избранные"

    static void SetStatus(char* statusText)                 // помещает в statusText состояние режима работы избранных эффектов
    {
      char buff[6];
      statusText = "FAV ";

      statusText = strcat(statusText, FavoritesRunning ? "1" : "0");
      statusText = strcat(statusText, " ");

      itoa(Interval, buff, 10);
      statusText = strcat(statusText, buff);
      statusText = strcat(statusText, " ");
      buff[0] = '\0';

      itoa(Dispersion, buff, 10);
      statusText = strcat(statusText, buff);
      statusText = strcat(statusText, " ");
      buff[0] = '\0';

      for (uint8_t i = 0; i < MODE_AMOUNT; i++)
      {
        itoa((uint8_t)FavoriteModes[i], buff, 10);
        statusText = strcat(statusText, buff);
        if (i < MODE_AMOUNT - 1) statusText = strcat(statusText, " ");
        buff[0] = '\0';
      }

      statusText = strcat(statusText, "\0");
    }

    static void ConfigureFavorites(const char* statusText)  // принимает statusText, парсит его и инициализирует свойства класса значениями из statusText'а
    {
      FavoritesRunning = getFavoritesRunning(statusText);
      if (!FavoritesRunning)
      {
        nextModeAt = 0;
      }
      Interval = getInterval(statusText);
      Dispersion = getDispersion(statusText);
      for (uint8_t i = 0; i < MODE_AMOUNT; i++)
      {
        FavoriteModes[i] = getModeOnOff(statusText, i);
      }
    }

    static bool HandleFavorites(                            // функция, обрабатывающая циклическое переключение избранных эффектов; возвращает true, если эффект был переключен
      bool* ONflag,
      int8_t* currentMode,
      bool* loadingFlag
      #ifdef USE_NTP
      , bool* dawnFlag
      #endif
    )
    {
      if (!FavoritesRunning ||
          !*ONflag                                          // лампа не переключается на следующий эффект при выключенной матрице
          #ifdef USE_NTP
          || *dawnFlag                                      // лампа не переключается на следующий эффект при включенном будильнике
          #endif
      )
      {
        return false;
      }

      if (nextModeAt == 0)                                  // лампа не переключается на следующий эффект сразу после включения режима избранных эффектов
      {
        nextModeAt = getNexTime();
        return false;
      }

      if (millis() >= nextModeAt)
      {
        *currentMode = getNextFavoriteMode(currentMode);
        *loadingFlag = true;
        nextModeAt = getNexTime();

        #ifdef GENERAL_DEBUG
        Serial.printf("Переключение на следующий избранный режим: %d\n\n", (*currentMode));
        #endif

        return true;
      }

      return false;
    }

    static void ReadFavoritesFromEeprom()
    {
      Interval = EepromManager::ReadUint16(EEPROM_FAVORITES_START_ADDRESS);
      Dispersion = EepromManager::ReadUint16(EEPROM_FAVORITES_START_ADDRESS + 2);

      for (uint8_t i = 0; i < MODE_AMOUNT; i++)
      {
        FavoriteModes[i] = EEPROM.read(EEPROM_FAVORITES_START_ADDRESS + i + 4);
        FavoriteModes[i] = FavoriteModes[i] > 0 ? 1 : 0;
      }
    }

    static void SaveFavoritesToEeprom()
    {
      // FavoritesRunning в EEPROM не сохраняем для экономии ресурса памяти
      EepromManager::WriteUint16(EEPROM_FAVORITES_START_ADDRESS, Interval);
      EepromManager::WriteUint16(EEPROM_FAVORITES_START_ADDRESS + 2, Dispersion);

      for (uint8_t i = 0; i < MODE_AMOUNT; i++)
      {
        EEPROM.put(EEPROM_FAVORITES_START_ADDRESS + i + 4, FavoriteModes[i] > 0 ? 1 : 0);
      }

      EEPROM.commit();
    }

    static void TurnFavoritesOff()
    {
      FavoritesRunning = false;
      nextModeAt = 0;
    }

  private:
    static uint32_t nextModeAt;                             // ближайшее время переключения на следующий избранный эффект (millis())

    static bool isStatusTextCorrect(const char* statusText) // валидирует statusText (проверяет, правильное ли коичество компонентов он содержит)
    {
      char buff[128];
      strcpy(buff, statusText);

      uint8_t lexCount = 0;
      char* p = strtok(buff, " ");
      while (p != NULL)                                   // пока есть лексемы...
      {
        lexCount++;
        p = strtok(NULL, " ");
      }

      return lexCount == getStatusTextNormalComponentsCount();
    }

    static uint8_t getStatusTextNormalComponentsCount()     // возвращает правильное ли коичество компонентов для statusText в зависимости от определённого формата команды и количества эффектов
    {
      // "FAV 0/1 <цифра> <цифра> <массив цифр 0/1 для каждого режима>" (вкл/выкл, интервал в секундах, разброс в секундах, вкл/выкл каждого эффекта в избранные)
      return
        1 +          // "FAV"
        1 +          // On/Off
        1 +          // интервал
        1 +          // разброс
        MODE_AMOUNT; // 0/1 для каждого эффекта
    }

    static bool getFavoritesRunning(const char* statusText) // возвращает признак вкл/выкл режима избранных эффектов из statusText
    {
      char lexem[2];
      memset(lexem, 0, 2);
      strcpy(lexem, getLexNo(statusText, 1));
      return lexem != NULL
        ? !strcmp(lexem, "1")
        : false;
    }

    static uint16_t getInterval(const char* statusText)     // возвращает интервал (постоянную составляющую) переключения избранных эффектов из statusText
    {
      char lexem[6];
      memset(lexem, 0, 6);
      strcpy(lexem, getLexNo(statusText, 2));
      return lexem != NULL
        ? atoi((const char*)lexem)
        : DEFAULT_FAVORITES_INTERVAL;
    }

    static uint16_t getDispersion(const char* statusText)   // возвращает разброс (случайную составляющую) интервала переключения избранных эффектов из statusText
    {
      char lexem[6];
      memset(lexem, 0, 6);
      strcpy(lexem, getLexNo(statusText, 3));
      return lexem != NULL
        ? atoi((const char*)lexem)
        : DEFAULT_FAVORITES_DISPERSION;
    }

    static bool getModeOnOff(const char* statusText, uint8_t modeId)  // возвращает признак включения указанного эффекта в избранные эффекты
    {
      char lexem[2];
      memset(lexem, 0, 2);
      strcpy(lexem, getLexNo(statusText, modeId + 4));
      return lexem != NULL
        ? !strcmp(lexem, "1")
        : false;
    }

    static char* getLexNo(const char* statusText, uint8_t pos)        // служебная функция, разбивает команду statusText на лексемы ("слова", разделённые пробелами) и возвращает указанную по счёту лексему
    {
      if (!isStatusTextCorrect(statusText))
      {
        return NULL;
      }

      const uint8_t buffSize = 128;
      char buff[buffSize];
      memset(buff, 0, buffSize);
      strcpy(buff, statusText);

      uint8_t lexPos = 0;
      char* p = strtok(buff, " ");
      while (p != NULL)                                   // пока есть лексемы...
      {
        if (lexPos == pos)
        {
          return p;
        }

        p = strtok(NULL, " ");
        lexPos++;
      }

      return NULL;
    }

    static int8_t getNextFavoriteMode(int8_t* currentMode)  // возвращает следующий (случайный) включенный в избранные эффект
    {
      int8_t result = *currentMode;

      for (int8_t tryNo = 0; tryNo <= random(0, MODE_AMOUNT); tryNo++)// случайное количество попыток определения следующего эффекта; без этого будет выбран следующий (избранный) по порядку после текущего
      {
        for (uint8_t i = (result + 1); i <= (result + MODE_AMOUNT); i++)
        {
          if (FavoriteModes[i < MODE_AMOUNT ? i : i - MODE_AMOUNT] > 0)
          {
            result = i < MODE_AMOUNT ? i : i - MODE_AMOUNT;
            break;
          }
        }        
      }

      return result;
    }

    static uint32_t getNexTime()                            // определяет время следующего переключения на следующий избранный эффект
    {
      return millis() + Interval * 1000 + random(0, Dispersion + 1) * 1000;
    }
};
