#include "RobotController.h"
#include "RoboticFirmware.h"

#include "PinMapping.h"
#include "PublishEvent.h"
#include "StorageController.h"
#include "Calibration.h"

#include <cstring>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

RobotController::RobotController() {
  motorRight = new Motor(PinMapping::motorPinR, PinMapping::brakePinR,
                         PinMapping::speedPinR, false, MOTOR_POS_RIGHT);
  motorRight->setSpeed(DEFAULT_MOTOR_SPEED);
  motorRight->setStateCallback(this, &RobotController::motorStateChange);
  motorLeft = new Motor(PinMapping::motorPinL, PinMapping::brakePinL,
                        PinMapping::speedPinL, true, MOTOR_POS_LEFT);
  motorLeft->setSpeed(DEFAULT_MOTOR_SPEED);
  motorLeft->setStateCallback(this, &RobotController::motorStateChange);

  speed = DEFAULT_MOTOR_SPEED;
}

void RobotController::process() {
  motorRight->process();
  motorLeft->process();

  unsigned int distanceCm = getFrontUltrasonicSensor()->getDistanceCm();
}

void RobotController::dangerClose(UltrasonicPosition position, unsigned int distance) {
  if (position == US_POSITION_FRONT &&
      this->state != ROBOT_STOPPED &&
      this->state != ROBOT_BACKWARD &&
      this->state != ROBOT_CLEANUP) {
    PublishEvent::QueueEvent(PUBLISH_EVENT_STOP);
    changeState(ROBOT_STOPPED);
  }
}

void RobotController::tiltOccurred(unsigned int x, unsigned int y) {
  if (!failed) {
  // In future this should kill the robot
    PublishEvent::QueueEvent(PUBLISH_EVENT_FAIL);
    failed = true;
  }
  if (this->state != ROBOT_STOPPED) {
    changeState(ROBOT_STOPPED);
  }
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

  if (cleaningCaster) {
    Serial.println("FAILED -> CLEANING CASTER");
    return;
  }

  if (newState == ROBOT_TURNING_LEFT || newState == ROBOT_TURNING_RIGHT) {
    changeTurningState(true);
  } else {
    changeTurningState(false);
  }

  switch (newState) {
    case ROBOT_STOPPED:
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
      if (movingForDistance) {
        movingForDistance = false;
      }
      break;
    case ROBOT_FORWARD:
      setMotorStates(FORWARD);
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
      setMotorStates(BACKWARD);
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
  motorRight->calibration->calibrateTurning(turnTimeMs);
  motorLeft->calibration->calibrateTurning(turnTimeMs);
}

void RobotController::calibrateSpeed(unsigned int extraSpeedRight,
                                     unsigned int extraSpeedLeft) {
  motorRight->calibration->calibrateSpeed(extraSpeedRight, extraSpeedLeft);
  motorLeft->calibration->calibrateSpeed(extraSpeedLeft, extraSpeedRight);
}

void RobotController::calibrateFriction(unsigned int friction) {
  motorRight->calibration->calibrateFriction(friction);
  motorLeft->calibration->calibrateFriction(friction);
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
  if (motorLeft->getState() == STOPPED &&
      motorRight->getState() == STOPPED) {
    if (movingForDistance) {
      movingForDistance = false;
      PublishEvent::QueueEvent(PUBLISH_EVENT_COMPLETE);
    }
    if (cleaningCaster) {
      cleaningCaster = false;
    }
    changeState(ROBOT_STOPPED);
  }
}

bool RobotController::cleaningUpCaster() {
  return cleaningCaster;
}

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
    case ROBOT_CLEANUP:
      return "ROBOT_CLEANUP";
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

