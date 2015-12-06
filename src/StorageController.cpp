#include "StorageController.h"
#include "application.h"

StorageController::StorageController() {

}

unsigned int StorageController::getStorageSize(StorageType type) {
  switch (type) {
    case STORAGE_TYPE_CAL_LEFT: case STORAGE_TYPE_CAL_RIGHT: case STORAGE_TYPE_CAL_TURN:
      return CALIBRATION;
      break;
  }
  return 0;
}

void StorageController::writeUnsignedInt(StorageType type, unsigned int integer) {
  IntegerConversion conversion;

  if (getStorageSize(type) != sizeof(unsigned int)) {
    return;
  }

  conversion.number = integer;
  for (unsigned int i = 0; i < sizeof(conversion.bytes); i++) {
    EEPROM.write(type + i, conversion.bytes[i]);
  }
}

unsigned int StorageController::readUnsignedInt(StorageType type) {
  IntegerConversion conversion;

  if (getStorageSize(type) != sizeof(unsigned int)) {
    return 0;
  }

  for (unsigned int i = 0; i < sizeof(conversion.bytes); i++) {
    conversion.bytes[i] = EEPROM.read(type + i);
  }
  return conversion.number;
}
