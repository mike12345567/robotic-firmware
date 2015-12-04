#include "RobotController.h"
#include "RoboticFirmware.h"

#include "PinMapping.h"
#include "PublishEvent.h"
#include "StorageController.h"

#include <cstring>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MOTOR_SPEED 128
#define MOTOR_R_CALIBRATION 10
#define MOTOR_L_CALIBRATION 0
#define MOTOR_CALIBRATION_MAX 64
#define DEFAULT_TURNING_CALIBRATION 2000
#define MAX_TURNING_TIME_MS 25000

RobotController::RobotController() {
  motorRight = new Motor(PinMapping::motorPinR, PinMapping::brakePinR,
                         PinMapping::speedPinR, false);
  motorRight->setSpeed(MOTOR_SPEED);
  motorRight->setStateCallback(this, &RobotController::motorStateChange);
  motorLeft = new Motor(PinMapping::motorPinL, PinMapping::brakePinL,
                        PinMapping::speedPinL, true);
  motorLeft->setSpeed(MOTOR_SPEED);

  unsigned int calibrationR = getStorageController()->readUnsignedInt(STORAGE_TYPE_CAL_RIGHT);
#ifdef MOTOR_R_CALIBRATION
  if (calibrationR == 0xFFFF || calibrationR > MOTOR_CALIBRATION_MAX) {
    getStorageController()->writeUnsignedInt(STORAGE_TYPE_CAL_RIGHT, MOTOR_R_CALIBRATION);
    calibrationR = MOTOR_R_CALIBRATION;
  }
#endif
  unsigned int calibrationL = getStorageController()->readUnsignedInt(STORAGE_TYPE_CAL_LEFT);
#ifdef MOTOR_L_CALIBRATION
  if (calibrationL == 0xFFFF || calibrationL > MOTOR_CALIBRATION_MAX) {
    getStorageController()->writeUnsignedInt(STORAGE_TYPE_CAL_LEFT, MOTOR_L_CALIBRATION);
    calibrationL = MOTOR_L_CALIBRATION;
  }
#endif
  unsigned int calibrationT = getStorageController()->readUnsignedInt(STORAGE_TYPE_CAL_TURN);
#ifdef DEFAULT_TURNING_CALIBRATION
  if (calibrationT == 0 || calibrationT > MAX_TURNING_TIME_MS) {
    getStorageController()->writeUnsignedInt(STORAGE_TYPE_CAL_TURN, DEFAULT_TURNING_CALIBRATION);
    calibrationT = DEFAULT_TURNING_CALIBRATION;
  }
#endif

  motorRight->calibration->calibrateSpeed(calibrationR);
  motorLeft->calibration->calibrateSpeed(calibrationL);
  motorRight->calibration->calibrateTurning(calibrationT);
  motorLeft->calibration->calibrateTurning(calibrationT);

  speed = MOTOR_SPEED;
}

void RobotController::process() {
  motorRight->process();
  motorLeft->process();
}

void RobotController::motorsSetDistance(char *arg, DistanceUnit unit) {
  unsigned int distance = strtoul(arg, NULL, 10);
  motorRight->moveForDistance(distance, unit);
  motorLeft->moveForDistance(distance, unit);
  movingForDistance = true;
}

void RobotController::changeState(RobotState newState) {
  Serial.print("CHANGING STATE -> ");
  Serial.println(robotStateToString());

  if (newState == ROBOT_TURNING_LEFT || newState == ROBOT_TURNING_RIGHT) {
    changeTurningState(true);
  } else {
    changeTurningState(false);
  }

  switch (newState) {
    case ROBOT_STOPPED:
      motorRight->setState(STOPPED);
      motorLeft->setState(STOPPED);
      break;
    case ROBOT_FORWARD:
      motorRight->setState(FORWARD);
      motorLeft->setState(FORWARD);
      break;
    case ROBOT_TURNING_LEFT:
      motorRight->setState(FORWARD);
      motorLeft->setState(STOPPED);
      break;
    case ROBOT_TURNING_RIGHT:
      motorRight->setState(STOPPED);
      motorLeft->setState(FORWARD);
      break;
    case ROBOT_BACKWARD:
      motorRight->setState(BACKWARD);
      motorLeft->setState(BACKWARD);
      break;
  }

  state = newState;
}

void RobotController::setRobotSpeed(unsigned int speed) {
  motorRight->setSpeed(speed);
  motorLeft->setSpeed(speed);
  this->speed = speed;
}

void RobotController::calibrateTurning(unsigned int turnTimeMs) {
  turnTimeMs = turnTimeMs > MAX_TURNING_TIME_MS ? MAX_TURNING_TIME_MS : turnTimeMs;
  motorRight->calibration->calibrateTurning(turnTimeMs);
  motorLeft->calibration->calibrateTurning(turnTimeMs);
  getStorageController()->writeUnsignedInt(STORAGE_TYPE_CAL_TURN, turnTimeMs);
}

void RobotController::calibrateSpeed(unsigned int extraSpeedRight,
                                     unsigned int extraSpeedLeft) {
  extraSpeedRight = extraSpeedRight > MOTOR_CALIBRATION_MAX ?
      MOTOR_CALIBRATION_MAX : extraSpeedRight;
  extraSpeedLeft = extraSpeedLeft > MOTOR_CALIBRATION_MAX ?
      MOTOR_CALIBRATION_MAX : extraSpeedLeft;
  motorRight->calibration->calibrateSpeed(extraSpeedRight);
  getStorageController()->writeUnsignedInt(STORAGE_TYPE_CAL_RIGHT, extraSpeedRight);
  motorLeft->calibration->calibrateSpeed(extraSpeedLeft);
  getStorageController()->writeUnsignedInt(STORAGE_TYPE_CAL_LEFT, extraSpeedLeft);
}

Calibration* RobotController::getCalibration(bool left) {
  return left ? motorLeft->calibration : motorRight->calibration;
}

unsigned int RobotController::getSpeed() {
  return speed;
}

void RobotController::changeTurningState(bool turning) {
  motorRight->setTurning(turning);
  motorLeft->setTurning(turning);
}

void RobotController::motorStateChange() {
  if (movingForDistance && motorLeft->getState() == STOPPED &&
      motorRight->getState() == STOPPED) {
    movingForDistance = false;
    PublishEvent::PublishComplete();
    changeState(ROBOT_STOPPED);
  }
}

const char* RobotController::robotStateToString() {
  switch (state) {
    case ROBOT_STOPPED:
      return "ROBOT_STOPPED";
    case ROBOT_FORWARD:
      return "ROBOT_FORWARD";
    case ROBOT_TURNING_LEFT:
      return "ROBOT_TURNING_LEFT";
    case ROBOT_TURNING_RIGHT:
      return "ROBOT_TURNING_RIGHT";
    case ROBOT_BACKWARD:
      return "ROBOT_BACKWARD";
  }

  return "UNKNOWN";
}

void RobotController::outputSerial() {
  Serial.println("ROBOT STATE");
  Serial.print("\tCurrent -> ");
  Serial.println(robotStateToString());

  Serial.println("MOTOR A");
  motorRight->outputSerial();
  Serial.println("MOTOR B");
  motorLeft->outputSerial();
}

