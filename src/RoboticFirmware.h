#ifndef ROBOTIC_FIRMWARE_H
#define ROBOTIC_FIRMWARE_H

#include "application.h"
#include "UltrasonicSensor.h"
#include "Gyroscope.h"
#include "Motor.h"
#include "RobotTimer.h"
#include <vector>

/* forward declare some classes to avoid cyclic dependency */
class StorageController;
class RobotController;
class EventController;

void serialOutput();
DistanceUnit getDistanceUnitFromArg(char *arg);
int makeMove(String param);
const char* robotStateToString();
void addRobotTimer(RobotTimer *timer);
void publishComplete();
RobotController* getRobotController();
UltrasonicSensor* getFrontUltrasonicSensor();
StorageController* getStorageController();
EventController* getEventController();
Gyroscope* getGyroscope();
void turnOffLed(bool state);

#endif
