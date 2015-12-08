#ifndef GYROSCOPE_H_
#define GYROSCOPE_H_

#include "application.h"
#include "libraries/MPU6050/MPU6050.h"

struct GyroscopeReadings {
  int16_t accelX, accelY, accelZ;
  int16_t gyroX, gyroY, gyroZ;
};

class Gyroscope {
  private:
    // MPU variables:
    MPU6050 impl;
    GyroscopeReadings readings;
    bool connectionSuccess = false;

  public:
    Gyroscope();

    void process();
    void outputSerial();
    GyroscopeReadings* getCurrentReadings();
};

#endif
