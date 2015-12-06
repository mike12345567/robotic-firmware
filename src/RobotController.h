#ifndef ROBOTCONTROLLER_H_
#define ROBOTCONTROLLER_H_

#include "application.h"
#include "Motor.h"

enum RobotState {
  ROBOT_TURNING_LEFT,
  ROBOT_TURNING_RIGHT,
  ROBOT_FORWARD,
  ROBOT_STOPPED,
  ROBOT_BACKWARD,
  ROBOT_NO_STATE,
};

enum RobotNotifyReason {
  ROBOT_REASON_STOP,
  ROBOT_REASON_CONTINUING,
  ROBOT_REASON_FAIL
};

class RobotController {
  private:
    unsigned int speed = 0;
    RobotState state = ROBOT_NO_STATE;
    RobotState stateBeforeStop = ROBOT_NO_STATE;
    bool movingForDistance = false;

    Motor* motorRight = NULL;
    Motor* motorLeft = NULL;

    void changeTurningState(bool turning);
    const char* robotStateToString(RobotState state);
    void notify(RobotNotifyReason reason);

  public:
    RobotController();
    void changeState(RobotState newState);
    void motorStateChange();
    void process();

    void setRobotSpeed(unsigned int speed);
    void motorsSetDistance(char *arg, DistanceUnit unit);
    void calibrateTurning(unsigned int turnTimeMs);
    void calibrateSpeed(unsigned int extraSpeedRight, unsigned int extraSpeedLeft);

    Calibration* getCalibration(bool left);
    unsigned int getSpeed();

    void outputSerial();
};

#endif
