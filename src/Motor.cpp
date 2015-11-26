#include "Motor.h"
#include <math.h>

#define MINIMUM_SPEED 80
#define MAXIMUM_SPEED 255
#define WHEEL_DIAMETER_MM 65
#define NO_LOAD_MOTOR_RPM 140
#define FRICTION_CALIBRATION 1.4
#define MINIMUM_FRIC_CALIB 0.8
#define TIME_MS_FULL_CIRCLE 4100

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
    switch (currentState) {
      case STOPPED:
        digitalWrite(this->brakePin, HIGH);
        digitalWrite(this->directionPin, LOW);
        analogWrite(this->speedPin, 0);
        break;
      case FORWARD:
        digitalWrite(this->brakePin, LOW);
        digitalWrite(this->directionPin, reversed ? HIGH : LOW);
        analogWrite(this->speedPin, speed + extraSpeed);
        break;
      case BACKWARD:
        digitalWrite(this->brakePin, LOW);
        digitalWrite(this->directionPin, reversed ? LOW : HIGH);
        analogWrite(this->speedPin, speed + extraSpeed);
        break;
    }
#endif
    stateChange = false;
  }

  if (currentState != STOPPED && stopTimer->isComplete()) {
    setState(STOPPED);
    stopTimer->stop();
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

void Motor::moveForDistance(unsigned int distance, DistanceUnit unit) {
  double timeMs = 0;
  if (unit != DEGREES) {
    double calibration = (double) FRICTION_CALIBRATION * ((double) MINIMUM_SPEED / this->speed);
    if (calibration < MINIMUM_FRIC_CALIB) {
      calibration = MINIMUM_FRIC_CALIB;
    }
    unsigned int distanceMm = distance * unit;
    double surfaceSpeed = calculateSurfaceSpeed();
    timeMs = (double) ((distanceMm / surfaceSpeed) * SECONDS);
    if (calibration) {
      timeMs *= calibration;
    }
  } else {
    double partOfCircle = (double) distance / 360;
    timeMs = (double) TIME_MS_FULL_CIRCLE * partOfCircle * ((double) MINIMUM_SPEED / this->speed);
  }

  this->stopTimer->setDuration(timeMs, MILLISECONDS);
  this->stopTimer->start();
  lastTravelTime = timeMs;
}

// returns speed in mm per second, needs the distance to calculate threshold
double Motor::calculateSurfaceSpeed() {
  double speedFactor = (double) this->speed / MAXIMUM_SPEED;
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

void Motor::calibrate(unsigned int extraSpeed) {
  this->extraSpeed = extraSpeed;
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

