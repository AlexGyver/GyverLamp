uint32_t effTimer;

void effectsTick()
{
  if (!dawnFlag)
  {
    if (ONflag && (millis() - effTimer >= ((currentMode < 5 || currentMode > 13) ? modes[currentMode].Speed : 50)))
    {
      effTimer = millis();
      switch (currentMode)
      {
        case EFF_SPARKLES:       sparklesRoutine();           break;
        case EFF_FIRE:           fireRoutine(true);           break;
        case EFF_WHITTE_FIRE:    fireRoutine(false);          break;
        case EFF_RAINBOW_VER:    rainbowVerticalRoutine();    break;
        case EFF_RAINBOW_HOR:    rainbowHorizontalRoutine();  break;
        case EFF_RAINBOW_DIAG:   rainbowDiagonalRoutine();    break;
        case EFF_COLORS:         colorsRoutine();             break;
        case EFF_MADNESS:        madnessNoiseRoutine();       break;
        case EFF_CLOUDS:         cloudsNoiseRoutine();        break;
        case EFF_LAVA:           lavaNoiseRoutine();          break;
        case EFF_PLASMA:         plasmaNoiseRoutine();        break;
        case EFF_RAINBOW:        rainbowNoiseRoutine();       break;
        case EFF_RAINBOW_STRIPE: rainbowStripeNoiseRoutine(); break;
        case EFF_ZEBRA:          zebraNoiseRoutine();         break;
        case EFF_FOREST:         forestNoiseRoutine();        break;
        case EFF_OCEAN:          oceanNoiseRoutine();         break;
        case EFF_COLOR:          colorRoutine();              break;
        case EFF_SNOW:           snowRoutine();               break;
        case EFF_SNOWSTORM:      snowStormRoutine();          break;
        case EFF_STARFALL:       starfallRoutine();           break;
        case EFF_MATRIX:         matrixRoutine();             break;
        case EFF_LIGHTERS:       lightersRoutine();           break;
        case EFF_LIGHTER_TRACES: ballsRoutine();              break;
        case EFF_PAINTBALL:      lightBallsRoutine();         break;
        case EFF_CUBE:           ballRoutine();               break;
        case EFF_WHITE_COLOR:    whiteColorStripeRoutine();   break;
        default:                                              break;
      }
      FastLED.show();
    }
  }
}

void changePower()
{
  if (ONflag)
  {
    effectsTick();
    for (uint8_t i = 0U; i < modes[currentMode].Brightness; i = constrain(i + 8, 0, modes[currentMode].Brightness))
    {
      FastLED.setBrightness(i);
      delay(1);
      FastLED.show();
    }
    FastLED.setBrightness(modes[currentMode].Brightness);
    delay(2);
    FastLED.show();
  }
  else
  {
    effectsTick();
    for (uint8_t i = modes[currentMode].Brightness; i > 0; i = constrain(i - 8, 0, modes[currentMode].Brightness))
    {
      FastLED.setBrightness(i);
      delay(1);
      FastLED.show();
    }
    FastLED.clear();
    delay(2);
    FastLED.show();
  }

  #if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)          // установка сигнала в пин, управляющий MOSFET транзистором, соответственно состоянию вкл/выкл матрицы
  digitalWrite(MOSFET_PIN, ONflag ? MOSFET_LEVEL : !MOSFET_LEVEL);
  #endif
  
  TimerManager::TimerRunning = false;
  TimerManager::TimerHasFired = false;
  TimerManager::TimeToFire = 0ULL;

  if (FavoritesManager::UseSavedFavoritesRunning == 0U)     // если выбрана опция Сохранять состояние (вкл/выкл) "избранного", то ни выключение модуля, ни выключение матрицы не сбрасывают текущее состояние (вкл/выкл) "избранного"
  {
      FavoritesManager::TurnFavoritesOff();
  }

  #if (USE_MQTT)
  if (espMode == 1U)
  {
    MqttManager::needToPublish = true;
  }
  #endif
}
