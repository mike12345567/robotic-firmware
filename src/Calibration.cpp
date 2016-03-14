#include "Calibration.h"
#include "RoboticFirmware.h"

Calibration::Calibration(MotorPosition position) {
  this->position = position;
  switch (position) {
    case MOTOR_POS_RIGHT:
      motorSpeedFwdCal = STORAGE_TYPE_CAL_RIGHT_FWD;
      motorSpeedBackCal = STORAGE_TYPE_CAL_RIGHT_BACK;
      directionType = STORAGE_TYPE_MOTOR_DIR_RIGHT;
      break;
    case MOTOR_POS_LEFT:
      motorSpeedFwdCal = STORAGE_TYPE_CAL_LEFT_FWD;
      motorSpeedBackCal = STORAGE_TYPE_CAL_LEFT_BACK;
      directionType = STORAGE_TYPE_MOTOR_DIR_LEFT;
      break;
  }

  defaultCalibration();
}

void Calibration::defaultCalibration() {
  StorageController* controller = getStorageController();

  /* SPEED DEFAULTS */
  unsigned int speedFwdCal = controller->readUnsignedInt(motorSpeedFwdCal);
  unsigned int speedBackCal = controller->readUnsignedInt(motorSpeedBackCal);
#if defined(MOTOR_R_CALIBRATION) || defined(MOTOR_L_CALIBRATION)
  unsigned testCal = position == MOTOR_POS_RIGHT ?
      MOTOR_R_CALIBRATION : MOTOR_L_CALIBRATION;
  unsigned backTest = position == MOTOR_POS_RIGHT ?
      MOTOR_L_CALIBRATION : MOTOR_R_CALIBRATION;
  if (speedFwdCal == 0xFFFF || speedFwdCal > MOTOR_CALIBRATION_MAX) {
    controller->writeUnsignedInt(motorSpeedFwdCal, testCal);
    speedFwdCal = testCal;
  }
  if (speedBackCal == 0xFFFF || speedBackCal > MOTOR_CALIBRATION_MAX) {
    controller->writeUnsignedInt(motorSpeedBackCal, backTest);
    speedBackCal = backTest;
  }
#endif
  this->speedFwdCalibration = speedFwdCal;
  this->speedBackCalibration = speedFwdCal;
  /* END SPEED DEFAULTS */

  /* TURNING DEFAULTS */
  unsigned int turnCal = controller->readUnsignedInt(STORAGE_TYPE_CAL_TURN);
#ifdef DEFAULT_TURNING_CALIBRATION
  if (turnCal == 0 || turnCal > MAX_TURNING_TIME_MS) {
    controller->writeUnsignedInt(STORAGE_TYPE_CAL_TURN, DEFAULT_TURNING_CALIBRATION);
    turnCal = DEFAULT_TURNING_CALIBRATION;
  }
#endif
  this->turnCalibration = turnCal;
  /* END TURNING DEFAULTS */

  /* FRICTION DEFAULTS */
  unsigned int frictionCal = controller->readUnsignedInt(STORAGE_TYPE_CAL_FRICTION);
#ifdef DEFAULT_FRICTION_CALIBRATION
  if (frictionCal == 0 || frictionCal > MAX_FRICTION_CALIBRATION) {
    controller->writeUnsignedInt(STORAGE_TYPE_CAL_FRICTION, DEFAULT_FRICTION_CALIBRATION);
    frictionCal = DEFAULT_FRICTION_CALIBRATION;
  }
#endif
  this->frictionCalibration = frictionCal;
  /* END FRICTION DEFAULTS */

  /* MOTOR DIRECTION DEFAULTS */
  MotorDirection direction = (MotorDirection) controller->readUnsignedInt(directionType);
#ifdef DEFAULT_MOTOR_DIRECTION
  if (direction == 0 || direction >= DIRECTION_MAX) {
    controller->writeUnsignedInt(directionType, DEFAULT_MOTOR_DIRECTION);
    direction = DEFAULT_MOTOR_DIRECTION;
  }
#endif
  this->direction = direction;
  /* END MOTOR DIRECTION DEFAULTS */
}

void Calibration::calibrateSpeed(unsigned int fwdSpeed, unsigned int backSpeed) {
  fwdSpeed = fwdSpeed > MOTOR_CALIBRATION_MAX ?
      MOTOR_CALIBRATION_MAX : fwdSpeed;
  backSpeed = backSpeed > MOTOR_CALIBRATION_MAX ?
      MOTOR_CALIBRATION_MAX : backSpeed;
  this->speedFwdCalibration = fwdSpeed;
  this->speedBackCalibration = fwdSpeed;
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

void Calibration::calibrateDirection(unsigned int direction) {
  MotorDirection dir = direction >= DIRECTION_BACKWARD ? DIRECTION_BACKWARD : DIRECTION_FORWARD;
  this->direction = dir;
  getStorageController()->writeUnsignedInt(directionType, (unsigned int) dir);
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

unsigned int Calibration::getMotorDirection() {
  return direction;
}
