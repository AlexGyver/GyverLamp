#pragma once

#include <WiFiManager.h>


class CaptivePortalManager
{
  public:
    static bool captivePortalCalled;

    CaptivePortalManager(WiFiManager *wifiManager);
    ~CaptivePortalManager();
    static void captivePortalManagerCallback(WiFiManager *wifiManager);

  private:
    WiFiManager *wifiManager = NULL;
};


CaptivePortalManager::CaptivePortalManager(WiFiManager *wifiManager)
{
  this->wifiManager = wifiManager;

  // добавление пользовательских параметров: id, placeholder, prompt/default value, length[, custom html]
  // ...

  if (wifiManager != NULL)
  {
    wifiManager->setAPCallback(CaptivePortalManager::captivePortalManagerCallback);
  }
 }

CaptivePortalManager::~CaptivePortalManager()
{
  this->wifiManager = NULL;
}

void CaptivePortalManager::captivePortalManagerCallback(WiFiManager *wifiManager)
{
  CaptivePortalManager::captivePortalCalled = true;
  // сохранение введённых значений пользовательских параметров
  // ...
}
