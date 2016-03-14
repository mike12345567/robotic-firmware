#include "Motor.h"
#include "RoboticFirmware.h"
#include "Calibration.h"
#include <math.h>

#define MINIMUM_TURN_SPEED_ADD 10
#define WHEEL_DIAMETER_MM 65
#define NO_LOAD_MOTOR_RPM 140
#define MINIMUM_FRIC_CALIB 0.8
#define SMALL_TURN_LOW_SPEED 1.15
#define LOW_TURN_SPEED 220
#define SMALL_TURN_ANGLE 90
#define CLEANUP_TIME_TURN_MS 500

Motor::Motor(unsigned int directionPin, unsigned int brakePin,
             unsigned int speedPin, bool reversed, MotorPosition position) {
  this->position = position;
  this->stopTimer = new RobotTimer(false);
  this->directionPin = directionPin;
  this->brakePin = brakePin;
  this->speedPin = speedPin;
  this->currentState = STOPPED;
  this->calibration = new Calibration(this->position);

  pinMode(this->directionPin, OUTPUT);
  pinMode(this->brakePin, OUTPUT);
  pinMode(this->speedPin, OUTPUT);
}

void Motor::process() {
  if (this->stateChange) {
#ifndef NO_MOVEMENT
#ifdef WHEEL_CASTER
    if (currentState == CLEANUP) {
      casterCleanup();
    } else {
#endif
      pinOut(currentState, turning);
#ifdef WHEEL_CASTER
    }
#endif
#endif
    stateChange = false;
  }

  if (currentState != STOPPED && stopTimer->isComplete()) {
#ifdef WHEEL_CASTER
    if (currentState == CLEANUP && !cleanOtherDirection) {
      cleanOtherDirection = true;
      casterCleanup();
    } else {
#endif
      setState(STOPPED);
      stopTimer->stop();
      if (parent != NULL && callback != NULL) {
        (parent->*callback)();
      }
#ifdef WHEEL_CASTER
    }
#endif
  }
}

void Motor::pinOut(MotorState state, bool turning) {
  unsigned int trueFwdSpeed = this->speed;
  unsigned int trueBackSpeed = this->speed;
  if (!turning) {
    trueFwdSpeed += calibration->getFwdSpeedCalibration();
    trueBackSpeed += calibration->getBackSpeedCalibration();
  } else {
    if (trueFwdSpeed < minimumTurnSpeed) {
      trueFwdSpeed = minimumTurnSpeed;
    }
    if (trueBackSpeed < minimumTurnSpeed) {
      trueBackSpeed = minimumTurnSpeed;
    }
  }

  bool direction = calibration->getMotorDirection() == DIRECTION_FORWARD;
  switch (state) {
    case STOPPED:
      digitalWrite(this->brakePin, HIGH);
      digitalWrite(this->directionPin, LOW);
      analogWrite(this->speedPin, 0);
      break;
    case FORWARD:
      digitalWrite(this->brakePin, LOW);
      digitalWrite(this->directionPin, direction ? LOW : HIGH);
      analogWrite(this->speedPin, trueFwdSpeed);
      break;
    case BACKWARD:
      digitalWrite(this->brakePin, LOW);
      digitalWrite(this->directionPin, direction ? HIGH : LOW);
      analogWrite(this->speedPin, trueBackSpeed);
      break;
  }
}

void Motor::setSpeed(unsigned int speed) {
  this->speed = speed;
  minimumTurnSpeed = speed + MINIMUM_TURN_SPEED_ADD;
}

#ifdef WHEEL_CASTER
void Motor::casterCleanup() {
  unsigned int durationMs = CLEANUP_TIME_TURN_MS;

  pinOut((position == MOTOR_POS_RIGHT && !cleanOtherDirection) ||
         (position == MOTOR_POS_LEFT && cleanOtherDirection) ?
             FORWARD : STOPPED, true);

  if (durationMs != 0) {
    stopTimer->setDuration(durationMs, MILLISECONDS);
    stopTimer->start();
  }
}
#endif

void Motor::setStateCallback(RobotController* parent, CallbackType callbackFunc) {
  this->parent = parent;
  this->callback = callbackFunc;
}

void Motor::setState(MotorState state) {
  if (state == STOPPED) {
    stopTimer->stop();
  }
#ifdef WHEEL_CASTER
  cleanOtherDirection = false;
#endif
  currentState = state;
  Serial.println(stateToString());
  stateChange = true;
}

MotorState Motor::getState() {
  return currentState;
}

void Motor::moveForDistance(unsigned int distance, DistanceUnit unit) {
  double timeMs = 0;
  unsigned int realSpeed = this->speed;
  if (turning && realSpeed < minimumTurnSpeed) {
    realSpeed = minimumTurnSpeed;
  }
  unsigned int frictionCal = 1 + (double)(this->calibration->getFrictionCalibration() / 100);
  if (unit != DEGREES) {
    double calibration = frictionCal * ((double) MINIMUM_SPEED / realSpeed);
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
    double speedFactor = calculateSurfaceSpeed(minimumTurnSpeed) / calculateSurfaceSpeed(realSpeed);
    speedFactor *= partOfCircle;
    timeMs = (double) (calibration->getTurnCalibration() * speedFactor);
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
#ifdef WHEEL_CASTER
    case CLEANUP:
      return "CLEANUP";
#endif
  }
  return "UNKNOWN";
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

  Serial.print("\tMotor fwd calibration -> ");
  Serial.println(calibration->getFwdSpeedCalibration());

  Serial.print("\tMotor back calibration -> ");
  Serial.println(calibration->getBackSpeedCalibration());

  Serial.print("\tTurn calibration -> ");
  Serial.println(calibration->getTurnCalibration());

  Serial.print("\tFriction calibration -> ");
  Serial.println(calibration->getFrictionCalibration());

  Serial.print("\tCurrently turning -> ");
  Serial.println(turning ? "YES" : "NO");

  if (lastTravelTime) {
    Serial.print("\tMotor travel time (seconds) -> ");
    Serial.println((double) lastTravelTime / SECONDS);
  }
}

