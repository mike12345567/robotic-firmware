#ifndef GYROSCOPE_H_
#define GYROSCOPE_H_

#include "application.h"
#include "libraries/MPU6050/MPU6050.h"
#include <deque>

struct GyroscopeReading {
  int16_t accelX, accelY, accelZ;
  int16_t gyroX, gyroY, gyroZ;
};

bool compareGyroReading(GyroscopeReading a, GyroscopeReading b);

class Gyroscope {
  private:
    // MPU variables:
    MPU6050 impl;
    //GyroscopeReadings readings;
    bool connectionSuccess = false;
    std::deque<GyroscopeReading> readings;

  public:
    Gyroscope();

    void process();
    void outputSerial();
    boolean isTurning();
    boolean isReady();
    GyroscopeReading* getCurrentReadings();
};

#endif
