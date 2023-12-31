#include "../Commands/i2c.h"

#include "../Commands/Common.h"
#include "../ESPEasyCore/Serial.h"

#include "../Globals/I2Cdev.h"
#include "../Globals/Settings.h"

#include "../../ESPEasy_common.h"

const __FlashStringHelper* Command_i2c_Scanner(struct EventStruct *event, const char *Line)
{
  uint8_t error, address;

  if (Settings.isI2CEnabled()) {
    for (address = 1; address <= 127; address++) {
      Wire.beginTransmission(address);
      error = Wire.endTransmission();

      if (error == 0) {
        serialPrint(F("I2C  : Found 0x"));
        serialPrintln(String(address, HEX));
      } else if (error == 4) {
        serialPrint(F("I2C  : Error at 0x"));
        serialPrintln(String(address, HEX));
      }
    }
  } else {
    serialPrintln(F("I2C  : Not enabled."));
  }
  return return_see_serial(event);
}
