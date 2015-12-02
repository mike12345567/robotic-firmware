#ifndef ROBOTIC_FIRMWARE_H
#define ROBOTIC_FIRMWARE_H

#include "application.h"
#include "Motor.h"
#include "RobotTimer.h"
#include <vector>

enum RobotState {
  ROBOT_TURNING_LEFT,
  ROBOT_TURNING_RIGHT,
  ROBOT_FORWARD,
  ROBOT_STOPPED,
  ROBOT_BACKWARD,
};

void serialOutput();
void changeState(RobotState newState);
void changeTurningState(bool turning);
void motorsSetDistance(char *arg, DistanceUnit unit);
DistanceUnit getDistanceUnitFromArg(char *arg);
void turnComplete();
int makeMove(String param);
const char* robotStateToString();
void addRobotTimer(RobotTimer *timer);
void motorStateChange();
void publishComplete();

#endif
