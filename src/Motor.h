#ifndef MOTOR_H_
#define MOTOR_H_

#include "application.h"
#include "RobotTimer.h"
#include "Calibration.h"

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

class RobotController;

class Motor {
  private:
    RobotTimer* stopTimer;
    unsigned int speed = 0;
    unsigned int directionPin;
    unsigned int brakePin;
    unsigned int speedPin;

    unsigned int lastTravelTime = 0;
    bool turning = false;
    bool reversed;
    bool stateChange = false;
    bool movingForDistance = false;
    MotorState currentState;

    typedef void (RobotController::*CallbackType)(void);
    RobotController* parent = NULL;
    CallbackType callback = NULL;

    double calculateSurfaceSpeed(unsigned int speed);
    const char* stateToString();

  public:
    Calibration *calibration = NULL;

    Motor(unsigned int directionPin, unsigned int brakePin, unsigned int speedPin, bool reversed);
    void process();

    void setStateCallback(RobotController* parent, CallbackType callbackFunc);
    void setSpeed(unsigned int speed);
    void setTurning(bool turning);
    void setState(MotorState state);

    MotorState getState();
    void moveForDistance(unsigned int distance, DistanceUnit unit);
    void outputSerial();
};

#endif
