void saveEEPROM() {
  EEPROM.put(3 * currentMode, modes[currentMode]);  
  EEPROM.commit();
}

void eepromTick() {
  if (settChanged && millis() - eepromTimer > 30000) {
    settChanged = false;
    eepromTimer = millis();
    saveEEPROM();
    if (EEPROM.read(101) != currentMode) EEPROM.write(101, currentMode);
    EEPROM.commit();
  }
}

void saveAlarm(byte almNumber) {
  EEPROM.write(5 * almNumber + 50, alarm[almNumber].state);   // рассвет
  eeWriteInt(5 * almNumber + 50 + 1, alarm[almNumber].time);
  EEPROM.commit();
}

void saveDawnMmode() {
  EEPROM.write(100, dawnMode);   // рассвет
  EEPROM.commit();
}
