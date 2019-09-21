void saveEEPROM() {
  EEPROM.put(3 * currentMode + 40, modes[currentMode]);
  EEPROM.commit();
}

void eepromTick() {
  if (settChanged && millis() - eepromTimer > 30000) {
    settChanged = false;
    eepromTimer = millis();
    saveEEPROM();
    if (EEPROM.read(200) != currentMode) EEPROM.write(200, currentMode);
    EEPROM.commit();
  }
}

void saveAlarm(byte almNumber) {
  EEPROM.write(5 * almNumber, alarm[almNumber].state);   // рассвет
  eeWriteInt(5 * almNumber + 1, alarm[almNumber].time);
  EEPROM.commit();
}

void saveDawnMmode() {
  EEPROM.write(199, dawnMode);   // рассвет
  EEPROM.commit();
}
