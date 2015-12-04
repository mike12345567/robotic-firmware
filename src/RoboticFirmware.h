#ifndef ROBOTIC_FIRMWARE_H
#define ROBOTIC_FIRMWARE_H

#include "application.h"
#include "Motor.h"
#include "RobotTimer.h"
#include <vector>

class StorageController;
class RobotController;

void serialOutput();
DistanceUnit getDistanceUnitFromArg(char *arg);
int makeMove(String param);
const char* robotStateToString();
void addRobotTimer(RobotTimer *timer);
void publishComplete();
RobotController* getRobotController();
StorageController* getStorageController();

#endif
