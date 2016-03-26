#ifndef ROBOTCONTROLLER_H_
#define ROBOTCONTROLLER_H_

#include "application.h"
#include "Motor.h"
#include "UltrasonicSensor.h"

enum RobotState {
  ROBOT_TURNING_LEFT,
  ROBOT_TURNING_RIGHT,
  ROBOT_FORWARD,
  ROBOT_STOPPED,
  ROBOT_BACKWARD,
#ifdef WHEEL_CASTER
  ROBOT_CLEANUP,
#endif
  ROBOT_NO_STATE,
};

class RobotController {
  private:
    bool failed = false;
    unsigned int speed = 0;
    RobotState state = ROBOT_STOPPED;
    bool movingForDistance = false;
#ifdef WHEEL_CASTER
    bool cleaningCaster = false;
#endif

    Motor* motorRight = NULL;
    Motor* motorLeft = NULL;

    void changeTurningState(bool turning);
    const char* robotStateToString(RobotState state);
    void setMotorStates(MotorState state);
  public:
    RobotController();
    void changeState(RobotState newState);
    void motorStateChange();
    void process();
    bool hasFailed();
    void resetFailed();

    void setRobotSpeed(unsigned int speed);
    void motorsSetDistance(char *arg, DistanceUnit unit);
    void calibrateTurning(unsigned int turnTimeMs);
    void calibrateSpeed(unsigned int extraSpeedLeft, unsigned int extraSpeedRight);
    void calibrateFriction(unsigned int friction);
    void calibrateDirection(unsigned int leftDirection, unsigned int rightDirection);
    void dangerClose(UltrasonicPosition position, unsigned int distanceCm);
    void tiltOccurred(unsigned int x, unsigned int y);
#ifdef WHEEL_CASTER
    bool cleaningUpCaster();
#endif

    Calibration* getCalibration(bool left);
    unsigned int getSpeed();

    void outputSerial();
};

#endif
