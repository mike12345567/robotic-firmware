#include "RobotController.h"
#include "RoboticFirmware.h"

#include "PinMapping.h"
#include "EventController.h"
#include "StorageController.h"
#include "Calibration.h"

#include <cstring>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

RobotController::RobotController() {
  this->speed = getStorageController()->readUnsignedInt(STORAGE_TYPE_CAL_BASE_SPEED);
  motorRight = new Motor(PinMapping::motorPinR, PinMapping::brakePinR,
                         PinMapping::speedPinR, false, MOTOR_POS_RIGHT);
  motorRight->setSpeed(this->speed);
  motorRight->setStateCallback(this, &RobotController::motorStateChange);
  motorLeft = new Motor(PinMapping::motorPinL, PinMapping::brakePinL,
                        PinMapping::speedPinL, true, MOTOR_POS_LEFT);
  motorLeft->setSpeed(this->speed);
  motorLeft->setStateCallback(this, &RobotController::motorStateChange);
}

void RobotController::process() {
  motorRight->process();
  motorLeft->process();
}

void RobotController::dangerClose(UltrasonicPosition position, unsigned int distance) {
  bool notDangerCloseState = this->state != ROBOT_STOPPED && this->state != ROBOT_BACKWARD;
#ifdef WHEEL_CASTER
  notDangerCloseState = notDangerCloseState && this->state != ROBOT_CLEANUP;
#endif
  if (position == US_POSITION_FRONT && notDangerCloseState) {
    getEventController()->queueEvent(PUBLISH_EVENT_STOP);
    changeState(ROBOT_STOPPED);
  }
}

void RobotController::tiltOccurred(unsigned int x, unsigned int y) {
  if (!failed) {
  // In future this should kill the robot
    getEventController()->queueEvent(PUBLISH_EVENT_FAIL);
    failed = true;
  }
  if (this->state != ROBOT_STOPPED) {
    changeState(ROBOT_STOPPED);
  }
}

void RobotController::resetFailed() {
  this->failed = false;
}

void RobotController::motorsSetDistance(char *arg, DistanceUnit unit) {
  unsigned int distance = strtoul(arg, NULL, 10);
  motorRight->moveForDistance(distance, unit);
  motorLeft->moveForDistance(distance, unit);
  movingForDistance = true;
}

void RobotController::setMotorStates(MotorState state) {
  motorRight->setState(state);
  motorLeft->setState(state);
}

void RobotController::changeState(RobotState newState) {
  Serial.print("CHANGING STATE -> ");
  Serial.println(robotStateToString(newState));

#ifdef WHEEL_CASTER
  if (cleaningCaster) {
    Serial.println("FAILED -> CLEANING CASTER");
    return;
  }
#endif

  if (newState == ROBOT_TURNING_LEFT || newState == ROBOT_TURNING_RIGHT) {
    changeTurningState(true);
  } else {
    changeTurningState(false);
  }

  switch (newState) {
    case ROBOT_STOPPED:
#ifdef WHEEL_CASTER
      if (state != ROBOT_FORWARD &&
          state != ROBOT_STOPPED &&
          state != ROBOT_CLEANUP &&
          state != ROBOT_TURNING_RIGHT) {
        setMotorStates(CLEANUP);
        newState = ROBOT_CLEANUP;
        cleaningCaster = true;
      } else {
        setMotorStates(STOPPED);
      }
#else
      setMotorStates(STOPPED);
#endif
      if (movingForDistance) {
        movingForDistance = false;
      }
      break;
    case ROBOT_FORWARD:
      setMotorStates(FORWARD);
      break;
    case ROBOT_TURNING_LEFT:
#ifdef ONE_WHEEL_ROTATION
      motorLeft->setState(STOPPED);
#else
      motorLeft->setState(BACKWARD);
#endif
      motorRight->setState(FORWARD);
      break;
    case ROBOT_TURNING_RIGHT:
#ifdef ONE_WHEEL_ROTATION
      motorRight->setState(STOPPED);
#else
      motorRight->setState(BACKWARD);
#endif
      motorLeft->setState(FORWARD);
      break;
    case ROBOT_BACKWARD:
      setMotorStates(BACKWARD);
      break;
  }

  state = newState;
}

void RobotController::setRobotSpeed(unsigned int speed) {
  if (speed > MAXIMUM_SPEED) {
    this->speed = MAXIMUM_SPEED;
  } else if (speed < MINIMUM_SPEED) {
    this->speed = MINIMUM_SPEED;
  }
  motorRight->setSpeed(speed);
  motorLeft->setSpeed(speed);
  this->speed = speed;
  getStorageController()->writeUnsignedInt(STORAGE_TYPE_CAL_BASE_SPEED, speed);
}

void RobotController::calibrateTurning(unsigned int turnTimeMs) {
  motorRight->calibration->calibrateTurning(turnTimeMs);
  motorLeft->calibration->calibrateTurning(turnTimeMs);
}

void RobotController::calibrateSpeed(unsigned int extraSpeedLeft,
                                     unsigned int extraSpeedRight) {
  motorRight->calibration->calibrateSpeed(extraSpeedRight, extraSpeedLeft);
  motorLeft->calibration->calibrateSpeed(extraSpeedLeft, extraSpeedRight);
}

void RobotController::calibrateFriction(unsigned int friction) {
  motorRight->calibration->calibrateFriction(friction);
  motorLeft->calibration->calibrateFriction(friction);
}

void RobotController::calibrateDirection(unsigned int leftDirection,
                                         unsigned int rightDirection) {
  motorRight->calibration->calibrateDirection(rightDirection);
  motorLeft->calibration->calibrateDirection(leftDirection);
}

Calibration* RobotController::getCalibration(bool left) {
  return left ? motorLeft->calibration : motorRight->calibration;
}

unsigned int RobotController::getSpeed() {
  return speed;
}


bool RobotController::hasFailed() {
  return failed;
}

void RobotController::changeTurningState(bool turning) {
  motorRight->setTurning(turning);
  motorLeft->setTurning(turning);
}

void RobotController::motorStateChange() {
  if (motorLeft->getState() == STOPPED &&
      motorRight->getState() == STOPPED) {
    if (movingForDistance) {
      movingForDistance = false;
      getEventController()->queueEvent(PUBLISH_EVENT_COMPLETE);
    }
#ifdef WHEEL_CASTER
    if (cleaningCaster) {
      cleaningCaster = false;
    }
#endif
    changeState(ROBOT_STOPPED);
  }
}

#ifdef WHEEL_CASTER
bool RobotController::cleaningUpCaster() {
  return cleaningCaster;
}
#endif

const char* RobotController::robotStateToString(RobotState state) {
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
#ifdef WHEEL_CASTER
    case ROBOT_CLEANUP:
      return "ROBOT_CLEANUP";
#endif
    case ROBOT_NO_STATE:
      return "ROBOT_NO_STATE";
  }

  return "UNKNOWN";
}

void RobotController::outputSerial() {
  Serial.println("ROBOT STATE");

  Serial.print("\tCurrent -> ");
  Serial.println(robotStateToString(state));

  Serial.println("MOTOR A");
  motorRight->outputSerial();
  Serial.println("MOTOR B");
  motorLeft->outputSerial();
}

