#ifndef CALIBRATION_H_
#define CALIBRATION_H_

#include "Motor.h"
#include "StorageController.h"

#define DEFAULT_FRICTION_CALIBRATION 40
#define MAX_FRICTION_CALIBRATION 500
#define MOTOR_R_CALIBRATION 10
#define MOTOR_L_CALIBRATION 0
#define MAX_TURNING_TIME_MS 25000
#define MOTOR_CALIBRATION_MAX 64
#define DEFAULT_TURNING_CALIBRATION 2000


class Calibration {
  private:
    MotorPosition position;
    StorageType motorSpeedFwdCal;
    StorageType motorSpeedBackCal;

    unsigned int turnCalibration = DEFAULT_TURNING_CALIBRATION;
    unsigned int speedFwdCalibration = 0;
    unsigned int speedBackCalibration = 0;
    unsigned int frictionCalibration = DEFAULT_FRICTION_CALIBRATION;

    void defaultCalibration();

  public:
    Calibration(MotorPosition position);
    void calibrateSpeed(unsigned int fwdSpeed, unsigned int backSpeed);
    void calibrateTurning(unsigned int turnTimeMs);
    void calibrateFriction(unsigned int frictionCal);

    unsigned int getTurnCalibration();
    unsigned int getFwdSpeedCalibration();
    unsigned int getBackSpeedCalibration();
    unsigned int getFrictionCalibration();
};

#endif
