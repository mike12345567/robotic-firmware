#include "Motor.h"
#include "RoboticFirmware.h"
#include <math.h>

#define MINIMUM_SPEED 80
#define MINIMUM_TURN_SPEED 170
#define MAXIMUM_SPEED 255
#define WHEEL_DIAMETER_MM 65
#define NO_LOAD_MOTOR_RPM 140
#define FRICTION_CALIBRATION 1.4
#define MINIMUM_FRIC_CALIB 0.8
#define SMALL_TURN_LOW_SPEED 1.15
#define LOW_TURN_SPEED 220
#define SMALL_TURN_ANGLE 90

Motor::Motor(unsigned int directionPin, unsigned int brakePin,
             unsigned int speedPin, bool reversed) {
  this->stopTimer = new RobotTimer();
  this->directionPin = directionPin;
  this->brakePin = brakePin;
  this->speedPin = speedPin;
  this->reversed = reversed;
  this->currentState = STOPPED;

  pinMode(this->directionPin, OUTPUT);
  pinMode(this->brakePin, OUTPUT);
  pinMode(this->speedPin, OUTPUT);
}

void Motor::process() {
  if (this->stateChange) {
#ifndef NO_MOVEMENT
    unsigned int trueSpeed = this->speed;
    if (!turning) {
      trueSpeed += extraSpeed;
    } else if (trueSpeed < MINIMUM_TURN_SPEED) {
      trueSpeed = MINIMUM_TURN_SPEED;
    }

    switch (currentState) {
      case STOPPED:
        digitalWrite(this->brakePin, HIGH);
        digitalWrite(this->directionPin, LOW);
        analogWrite(this->speedPin, 0);
        break;
      case FORWARD:
        digitalWrite(this->brakePin, LOW);
        digitalWrite(this->directionPin, reversed ? HIGH : LOW);
        analogWrite(this->speedPin, trueSpeed);
        break;
      case BACKWARD:
        digitalWrite(this->brakePin, LOW);
        digitalWrite(this->directionPin, reversed ? LOW : HIGH);
        analogWrite(this->speedPin, trueSpeed);
        break;
    }
#endif
    stateChange = false;
  }

  if (currentState != STOPPED && stopTimer->isComplete()) {
    setState(STOPPED);
    stopTimer->stop();
    motorStateChange();
  }
}

void Motor::setSpeed(unsigned int speed) {
  if (speed > MAXIMUM_SPEED) {
    this->speed = MAXIMUM_SPEED;
  } else if (speed < MINIMUM_SPEED) {
    this->speed = MINIMUM_SPEED;
  } else {
    this->speed = speed;
  }
}

void Motor::setState(MotorState state) {
  this->currentState = state;
  stateChange = true;
}

MotorState Motor::getState() {
  return currentState;
}

void Motor::moveForDistance(unsigned int distance, DistanceUnit unit) {
  double timeMs = 0;
  unsigned int realSpeed = this->speed;
  if (turning && realSpeed < MINIMUM_TURN_SPEED) {
    realSpeed = MINIMUM_TURN_SPEED;
  }
  if (unit != DEGREES) {
    double calibration = (double) FRICTION_CALIBRATION * ((double) MINIMUM_SPEED / realSpeed);
    if (calibration < MINIMUM_FRIC_CALIB) {
      calibration = MINIMUM_FRIC_CALIB;
    }
    unsigned int distanceMm = distance * unit;
    double surfaceSpeed = calculateSurfaceSpeed(realSpeed);
    timeMs = (double) ((distanceMm / surfaceSpeed) * SECONDS);
    if (calibration) {
      timeMs *= calibration;
    }
  } else {
    double partOfCircle = (double) distance / 360;
    double speedFactor = calculateSurfaceSpeed(MINIMUM_TURN_SPEED) / calculateSurfaceSpeed(realSpeed);
    speedFactor *= partOfCircle;
    timeMs = (double) (turnCalibrationMs * speedFactor);
    if (distance <= SMALL_TURN_ANGLE && this->speed <= LOW_TURN_SPEED) {
      timeMs *= SMALL_TURN_LOW_SPEED;
    }
  }

  this->stopTimer->setDuration(timeMs, MILLISECONDS);
  this->stopTimer->start();
  lastTravelTime = timeMs;
}

// returns speed in mm per second, needs the distance to calculate threshold
double Motor::calculateSurfaceSpeed(unsigned int speed) {
  double speedFactor = (double) speed / MAXIMUM_SPEED;
  double currentRpm = NO_LOAD_MOTOR_RPM * speedFactor;
  return (currentRpm * (WHEEL_DIAMETER_MM * M_PI) / 60);
}

const char* Motor::stateToString() {
  switch (currentState) {
    case STOPPED:
      return "STOPPED";
    case FORWARD:
      return "FORWARD";
    case BACKWARD:
      return "BACKWARD";
  }
  return "UNKNOWN";
}

void Motor::calibrateStraightLine(unsigned int extraSpeed) {
  this->extraSpeed = extraSpeed;
}

void Motor::calibrateTurning(unsigned int turnTimeMs) {
  this->turnCalibrationMs = turnTimeMs;
}

void Motor::setTurning(bool turning) {
  this->turning = turning;
  stateChange = true;
}

void Motor::outputSerial() {
  Serial.print("\tMotor state -> ");
  Serial.println(stateToString());

  Serial.print("\tMotor speed -> ");
  Serial.println(speed);

  Serial.print("\tMotor calibration -> ");
  Serial.println(extraSpeed);

  if (lastTravelTime) {
    Serial.print("\tMotor travel time (seconds) -> ");
    Serial.println((double) lastTravelTime / SECONDS);
  }
}

