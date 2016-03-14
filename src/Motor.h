#ifndef MOTOR_H_
#define MOTOR_H_

#include "application.h"
#include "RobotTimer.h"

#define LAST_MOTOR (1 << 8)
#define DEFAULT_MOTOR_SPEED 160
#define MINIMUM_SPEED 50
#define MAXIMUM_SPEED 255

/* whether the motor is attached reversed or not */
enum MotorDirection {
  DIRECTION_FORWARD  = 1 << 0,
  DIRECTION_BACKWARD = 1 << 1,
  DIRECTION_MAX      = 1 << 2
};

enum MotorState {
  FORWARD,
  BACKWARD,
  STOPPED,
#ifdef WHEEL_CASTER
  CLEANUP,
#endif
};

enum DistanceUnit {
  MM = 0,
  CM = 10,
  M  = 1000,
  DEGREES = -1
};

enum MotorPosition {
  MOTOR_POS_LEFT = 1 << 1,
  MOTOR_POS_RIGHT = (1 << 2) | LAST_MOTOR,
  MOTOR_POS_UNSET = -1,
};

class RobotController;
class Calibration;

class Motor {
  private:
    RobotTimer* stopTimer = NULL;
    MotorPosition position = MOTOR_POS_UNSET;
    unsigned int speed = 0;
    unsigned minimumTurnSpeed = 0;
    unsigned int directionPin;
    unsigned int brakePin;
    unsigned int speedPin;
#ifdef WHEEL_CASTER
    bool cleanOtherDirection = false;
#endif

    unsigned int lastTravelTime = 0;
    bool turning = false;
    bool stateChange = false;
    MotorState currentState;

    typedef void (RobotController::*CallbackType)(void);
    RobotController* parent = NULL;
    CallbackType callback = NULL;

    double calculateSurfaceSpeed(unsigned int speed);
    const char* stateToString();
#ifdef WHEEL_CASTER
    void casterCleanup();
#endif
    void pinOut(MotorState state, bool turning);
  public:
    Calibration *calibration = NULL;

    Motor(unsigned int directionPin, unsigned int brakePin, unsigned int speedPin,
          bool reversed, MotorPosition position);
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
