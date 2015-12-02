#ifndef MOTOR_H_
#define MOTOR_H_

#include "application.h"
#include "RobotTimer.h"

#define TIME_MS_FULL_CIRCLE 2050

enum MotorState {
  FORWARD,
  BACKWARD,
  STOPPED,
};

enum DistanceUnit {
  MM = 0,
  CM = 10,
  M  = 1000,
  DEGREES = -1
};

class Motor {
  private:
    RobotTimer* stopTimer;
    unsigned int speed = 0;
    unsigned int extraSpeed = 0;
    unsigned int directionPin;
    unsigned int brakePin;
    unsigned int speedPin;
    unsigned int turnCalibrationMs = TIME_MS_FULL_CIRCLE;

    unsigned int lastTravelTime = 0;
    bool turning = false;
    bool reversed;
    bool stateChange = false;
    bool movingForDistance = false;
    MotorState currentState;

    double calculateSurfaceSpeed(unsigned int speed);
    const char* stateToString();

  public:
    Motor(unsigned int directionPin, unsigned int brakePin, unsigned int speedPin, bool reversed);

    void setSpeed(unsigned int speed);
    void calibrateStraightLine(unsigned int extraSpeed);
    void calibrateTurning(unsigned int turnTimeMs);
    void process();
    void setTurning(bool turning);
    MotorState getState();
    void setState(MotorState state);
    void moveForDistance(unsigned int distance, DistanceUnit unit);
    void outputSerial();
};

#endif
