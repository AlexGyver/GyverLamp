/*
 * 11.07.2019
 * Класс, который отслеживает действия пользователя по запросу обновления прошивки по воздуху и выполняет эту прошивку.
 * Запрос на обновление - это вызов метода RequestOtaUpdate(), его нужно поместить, например, в обработчик нажатия кнопки, приёма UDP пакета и т.д.
 * Для обновления пользователь должен ДВАЖДЫ запросить обновление в течение заданного промежутка времени (CONFIRMATION_TIMEOUT) во избежание случайного перехода в режим обновления.
 * Режим обновления - это прослушивание специального порта (ESP_OTA_PORT) в ожидании команды обновления прошивки по воздуху (по сети).
 * Режим обновления работает параллельно с основным режимом функционирования, только при ESP_MODE == 1 (WiFi клиент), т.к. требует доступа к ESP по локальной сети и при подключенной кнопке (в данном сетапе, т.к. он вызывается кнопкой).
 * Режим обновления активен в течение заданного промежутка времени (ESP_CONF_TIMEOUT). Потом ESP автоматически перезагружается.
 * Обновление производится из Arduino IDE: меню Инструменты - Порт - <Выбрать обнаруженный СЕТЕВОЙ COM порт из списка> (если он не обнаружен, значит что-то настроено неправильно), затем обычная команда "Загрузка" для прошивки.
 * Для включения опции обновления по воздуху в основном файле должен быть определён идентификатор OTA "#define OTA" и режим "#define ESP_MODE (1U)" (а также в данном проекте должна быть подключена кнопка).
*/

#ifdef OTA

#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>


#define CONFIRMATION_TIMEOUT  (30U)                         // время в сеундах, в течение которого нужно дважды подтвердить старт обновлениЯ по воздуху (иначе сброс в None)

enum OtaPhase                                               // определение стадий процесса обновления по воздуху: нет, получено первое подтверждение, получено второе подтверждение, получено второе подтверждение - в процессе, обновление окончено
{
  None = 0,
  GotFirstConfirm,
  GotSecondConfirm,
  InProgress,
  Done
};

class OtaManager
{
  public:
    static OtaPhase OtaFlag;

    bool RequestOtaUpdate()                                 // пользователь однократно запросил обновление по воздуху; возвращает true, когда переходит в режим обновления - startOtaUpdate()
    {
      if (ESP_MODE != 1)
      {
        #ifdef GENERAL_DEBUG
        Serial.printf("Запрос обновления по воздуху поддерживается только в режиме ESP_MODE = 1\n");
        #endif

        return false;
      }

      if (OtaFlag == OtaPhase::None)
      {
        OtaFlag = OtaPhase::GotFirstConfirm;
        momentOfFirstConfirmation = millis();

        #ifdef GENERAL_DEBUG
        Serial.printf("Получено первое подтверждение обновления по воздуху\nОжидание второго подтверждения\n");
        #endif

        return false;
      }

      if (OtaFlag == OtaPhase::GotFirstConfirm)
      {
        OtaFlag = OtaPhase::GotSecondConfirm;

        #ifdef GENERAL_DEBUG
        Serial.printf("Получено второе подтверждение обновления по воздуху\nСтарт режима обновления\n");
        #endif

        startOtaUpdate();
        return true;
      }

      return false;
    }

    void HandleOtaUpdate()
    {
      if (OtaFlag == OtaPhase::GotFirstConfirm &&
          millis() - momentOfFirstConfirmation >= CONFIRMATION_TIMEOUT * 1000)
      {
        OtaFlag = OtaPhase::None;
        momentOfFirstConfirmation = 0;

        #ifdef GENERAL_DEBUG
        Serial.printf("Таймаут ожидания второго подтверждения превышен\nСброс флага в исходное состояние\n");
        #endif

        return;
      }

      if (OtaFlag == OtaPhase::GotSecondConfirm &&
          millis() - momentOfOtaStart >= ESP_CONF_TIMEOUT * 1000)
      {
        OtaFlag = OtaPhase::None;
        momentOfOtaStart = 0;

        #ifdef GENERAL_DEBUG
        Serial.printf("Таймаут ожидания прошивки по воздуху превышен\nСброс флага в исходное состояние\nПерезагрузка\n");
        delay(500);
        #endif

        #if defined(ESP8266)
        ESP.reset();
        #else
        ESP.restart();
        #endif

        return;
      }

      if (OtaFlag == OtaPhase::InProgress)
      {
        ArduinoOTA.handle();
      }
    }

  private:
    uint64_t momentOfFirstConfirmation = 0;                 // момент времени, когда получено первое подтверждение и с которого начинается отсчёт ожидания второго подтверждения
    uint64_t momentOfOtaStart = 0;                          // момент времени, когда развёрнута WiFi точка доступа для обновления по воздуху

    void startOtaUpdate()
    {
      char espHostName[65];
      sprintf(espHostName, "%s-%u", AP_NAME, ESP.getChipId());
      ArduinoOTA.setPort(ESP_OTA_PORT);
      ArduinoOTA.setHostname(espHostName);
      ArduinoOTA.setPassword(AP_PASS);

      ArduinoOTA.onStart([this]()
      {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
        {
          type = "sketch";
        }
        else // U_SPIFFS
        {
          type = "filesystem";
        }

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()

        #ifdef GENERAL_DEBUG
        Serial.println("Start updating " + type);
        #endif
      });

      ArduinoOTA.onEnd([this]()
      {
        OtaFlag = OtaPhase::Done;

        #ifdef GENERAL_DEBUG
        Serial.printf("Обновление по воздуху выполнено\nПерезапуск");
        delay(500);
        #endif
      });

      ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
      {
        #ifdef GENERAL_DEBUG
        Serial.printf("Ход выполнения: %u%%\r", (progress / (total / 100)));
        #endif
      });

      ArduinoOTA.onError([this](ota_error_t error)
      {
        OtaFlag = OtaPhase::None;

        #ifdef GENERAL_DEBUG
        Serial.printf("Обновление по воздуху завершилось ошибкой [%u]: ", error);
        #endif

        if (error == OTA_AUTH_ERROR)
        {
          #ifdef GENERAL_DEBUG
          Serial.println("Auth Failed");
          #endif
        }
        else if (error == OTA_BEGIN_ERROR)
        {
          #ifdef GENERAL_DEBUG
          Serial.println("Begin Failed");
          #endif
        }
        else if (error == OTA_CONNECT_ERROR)
        {
          #ifdef GENERAL_DEBUG
          Serial.println("Connect Failed");
          #endif
        }
        else if (error == OTA_RECEIVE_ERROR)
        {
          #ifdef GENERAL_DEBUG
          Serial.println("Receive Failed");
          #endif
        }
        else if (error == OTA_END_ERROR)
        {
          #ifdef GENERAL_DEBUG
          Serial.println("End Failed");
          #endif
        }

        #ifdef GENERAL_DEBUG
        Serial.printf("Сброс флага в исходное состояние\nПереход в режим ожидания запроса прошивки по воздуху\n");
        #endif
      });

      ArduinoOTA.setRebootOnSuccess(true);
      ArduinoOTA.begin();
      OtaFlag = OtaPhase::InProgress;
      momentOfFirstConfirmation = 0;
      momentOfOtaStart = 0;

      #ifdef GENERAL_DEBUG
      Serial.printf("Для обновления в Arduino IDE выберите пункт меню Инструменты - Порт - '%s at ", espHostName);
      Serial.print(WiFi.localIP());
      Serial.println("'");
      Serial.printf("Затем нажмите кнопку 'Загрузка' в течение %u секунд и по запросу введите пароль '%s'\n", ESP_CONF_TIMEOUT, AP_PASS);
      Serial.println("Устройство с Arduino IDE должно быть в одной локальной сети с модулем ESP!");
      #endif
    }
};

#endif
