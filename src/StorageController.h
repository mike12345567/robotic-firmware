#ifndef STORAGECONTROLLER_H_
#define STORAGECONTROLLER_H_

enum StorageSize {
  CALIBRATION = 4,
};

enum StorageType {
  STORAGE_TYPE_CAL_RIGHT_FWD = 0,
  STORAGE_TYPE_CAL_LEFT_FWD = STORAGE_TYPE_CAL_RIGHT_FWD + CALIBRATION,
  STORAGE_TYPE_CAL_TURN = STORAGE_TYPE_CAL_LEFT_FWD + CALIBRATION,
  STORAGE_TYPE_CAL_FRICTION = STORAGE_TYPE_CAL_TURN + CALIBRATION,
  STORAGE_TYPE_CAL_LEFT_BACK = STORAGE_TYPE_CAL_FRICTION + CALIBRATION,
  STORAGE_TYPE_CAL_RIGHT_BACK = STORAGE_TYPE_CAL_LEFT_BACK + CALIBRATION,
};

struct IntegerConversion {
  union {
    unsigned int number;
    char bytes[4];
  };
};

class StorageController {
  private:
    unsigned int getStorageSize(StorageType type);

  public:
    StorageController();
    void writeUnsignedInt(StorageType type, unsigned int integer);
    unsigned int readUnsignedInt(StorageType type);
};

#endif
