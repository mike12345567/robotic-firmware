#ifndef MOTOR_H_
#define MOTOR_H_

#include "application.h"
#include "RobotTimer.h"

enum MotorState {
  FORWARD,
  BACKWARD,
  STOPPED
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

    unsigned int lastTravelTime = 0;
    bool reversed;
    bool stateChange = false;
    MotorState currentState;

    double calculateSurfaceSpeed();
    const char* stateToString();

  public:
    Motor(unsigned int directionPin, unsigned int brakePin, unsigned int speedPin, bool reversed);

    void setSpeed(unsigned int speed);
    void calibrate(unsigned int extraSpeed);
    void process();
    void setState(MotorState state);
    void moveForDistance(unsigned int distance, DistanceUnit unit);
    void outputSerial();
};

#endif
