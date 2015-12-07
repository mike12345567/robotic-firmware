#ifndef GYROSCOPE_H_
#define GYROSCOPE_H_

#include "application.h"
#include "libraries/MPU6050/MPU6050.h"

class Gyroscope {
  private:
    // MPU variables:
    MPU6050 impl;
    int16_t ax = 0, ay = 0, az = 0;
    int16_t gx = 0, gy = 0, gz = 0;
    bool connectionSuccess = false;

  public:
    Gyroscope();

    void process();
    void outputSerial();
};

#endif
