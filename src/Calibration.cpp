#include "Calibration.h"
#include "RoboticFirmware.h"

Calibration::Calibration(MotorPosition position) {
  this->position = position;
  this->motorSpeedFwdCal = position == MOTOR_POS_RIGHT ?
      STORAGE_TYPE_CAL_RIGHT_FWD : STORAGE_TYPE_CAL_LEFT_FWD;
  this->motorSpeedBackCal = position == MOTOR_POS_RIGHT ?
      STORAGE_TYPE_CAL_RIGHT_BACK : STORAGE_TYPE_CAL_LEFT_BACK;
  defaultCalibration();
}

void Calibration::defaultCalibration() {
  unsigned int speedFwdCal = getStorageController()->readUnsignedInt(motorSpeedFwdCal);
  unsigned int speedBackCal = getStorageController()->readUnsignedInt(motorSpeedBackCal);
#if defined(MOTOR_R_CALIBRATION) || defined(MOTOR_L_CALIBRATION)
  unsigned testCal = position == MOTOR_POS_RIGHT ?
      MOTOR_R_CALIBRATION : MOTOR_L_CALIBRATION;
  unsigned backTest = position == MOTOR_POS_RIGHT ?
      MOTOR_L_CALIBRATION : MOTOR_R_CALIBRATION;
  if (speedFwdCal == 0xFFFF || speedFwdCal > MOTOR_CALIBRATION_MAX) {
    getStorageController()->writeUnsignedInt(motorSpeedFwdCal, testCal);
    speedFwdCal = testCal;
  }
  if (speedBackCal == 0xFFFF || speedBackCal > MOTOR_CALIBRATION_MAX) {
    getStorageController()->writeUnsignedInt(motorSpeedBackCal, backTest);
    speedBackCal = backTest;
  }
#endif
  this->speedFwdCalibration = speedFwdCal;
  this->speedBackCalibration = speedBackCal;

  unsigned int turnCal = getStorageController()->readUnsignedInt(STORAGE_TYPE_CAL_TURN);
#ifdef DEFAULT_TURNING_CALIBRATION
  if (turnCal == 0 || turnCal > MAX_TURNING_TIME_MS) {
    getStorageController()->writeUnsignedInt(STORAGE_TYPE_CAL_TURN, DEFAULT_TURNING_CALIBRATION);
    turnCal = DEFAULT_TURNING_CALIBRATION;
  }
#endif
  this->turnCalibration = turnCal;

  unsigned int frictionCal = getStorageController()->readUnsignedInt(STORAGE_TYPE_CAL_FRICTION);
#ifdef DEFAULT_FRICTION_CALIBRATION
  if (frictionCal == 0 || frictionCal > MAX_FRICTION_CALIBRATION) {
    getStorageController()->writeUnsignedInt(STORAGE_TYPE_CAL_FRICTION, DEFAULT_FRICTION_CALIBRATION);
    frictionCal = DEFAULT_FRICTION_CALIBRATION;
  }
#endif
  this->frictionCalibration = frictionCal;
}

void Calibration::calibrateSpeed(unsigned int fwdSpeed, unsigned int backSpeed) {
  fwdSpeed = fwdSpeed > MOTOR_CALIBRATION_MAX ?
      MOTOR_CALIBRATION_MAX : fwdSpeed;
  backSpeed = backSpeed > MOTOR_CALIBRATION_MAX ?
      MOTOR_CALIBRATION_MAX : backSpeed;
  this->speedFwdCalibration = fwdSpeed;
  this->speedBackCalibration = backSpeed;
  getStorageController()->writeUnsignedInt(motorSpeedFwdCal, fwdSpeed);
  getStorageController()->writeUnsignedInt(motorSpeedBackCal, backSpeed);
}

void Calibration::calibrateTurning(unsigned int turnTimeMs) {
  turnTimeMs = turnTimeMs > MAX_TURNING_TIME_MS ? MAX_TURNING_TIME_MS : turnTimeMs;
  this->turnCalibration = turnTimeMs;
  if (this->position & LAST_MOTOR) {
    getStorageController()->writeUnsignedInt(STORAGE_TYPE_CAL_TURN, turnCalibration);
  }
}

void Calibration::calibrateFriction(unsigned int frictionCal) {
  frictionCal = frictionCal > MAX_FRICTION_CALIBRATION ? MAX_FRICTION_CALIBRATION : frictionCal;
  this->frictionCalibration = frictionCal;
  if (this->position & LAST_MOTOR) {
    getStorageController()->writeUnsignedInt(STORAGE_TYPE_CAL_FRICTION, frictionCalibration);
  }
}

unsigned int Calibration::getTurnCalibration() {
  return this->turnCalibration;
}

unsigned int Calibration::getFwdSpeedCalibration() {
  return this->speedFwdCalibration;
}

unsigned int Calibration::getBackSpeedCalibration() {
  return this->speedBackCalibration;
}

unsigned int Calibration::getFrictionCalibration() {
  return this->frictionCalibration;
}
