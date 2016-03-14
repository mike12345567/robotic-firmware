#include "RoboticFirmware.h"
#include "spark_wiring_timer.h"
#include "PinMapping.h"
#include "RobotController.h"
#include "PublishEvent.h"
#include "StorageController.h"
#include "UltrasonicSensor.h"
#include "Gyroscope.h"

#include <cstring>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <deque>

#define SERIAL_DELAY_MS 2000

std::vector<RobotTimer*> robotTimers;
RobotController* robotController = NULL;
StorageController* storageController = NULL;
UltrasonicSensor* ultrasonicSensor = NULL;
Gyroscope* gyroscope = NULL;

STARTUP(WiFi.selectAntenna(ANT_AUTO));
SYSTEM_MODE(AUTOMATIC);
SYSTEM_THREAD(ENABLED);

Timer serialTimer(SERIAL_DELAY_MS, serialOutput);

unsigned int nextStateChangeMs = 0;
unsigned int lastStateChangeMs = 0;

void setup() {
  Serial.begin(115200);
  storageController = new StorageController();
  robotController = new RobotController();
  ultrasonicSensor = new UltrasonicSensor(US_POSITION_FRONT);
  gyroscope = new Gyroscope();

  PublishEvent::Setup();
  Particle.function("makeMove", makeMove);
  serialTimer.start();
}

void loop() {
  auto iterator = robotTimers.begin();
  ultrasonicSensor->process();
  gyroscope->process();
  robotController->process();

  while (iterator != robotTimers.end()) {
    RobotTimer* timer = *iterator;
    timer->process();
    iterator++;
  }

  PublishEvent::Process();
}

/* TODO: Change this to something more meaningful */
int makeMove(String param) {
  char *pointer = NULL;
  char *cstring = new char[param.length() + 1];
  char **args = NULL;
  // always one argument
  int argCount = 1;
  int argIndex = 0;

  Serial.print("Event Received! -> ");
  Serial.println(param);

  // get our c string out for usage later
  strcpy(cstring, param.c_str());

  // count args
  pointer = strchr(cstring,',');
  while (pointer != NULL) {
    pointer = strchr(pointer+1,',');
    argCount++;
  }

  args = (char **) malloc(sizeof(char*) * argCount);

  if (args != NULL) {
    pointer = strtok(cstring, ",");
    while (pointer != NULL) {
      args[argIndex++] = pointer;
      pointer = strtok(NULL, ",");
    }

    // if values per motor then left comes first, right second in args
    bool moving = false;
    if (strcmp("stop", args[0]) == 0 && argCount == 1) {
      robotController->changeState(ROBOT_STOPPED);
    } else if (strcmp("forward", args[0]) == 0 && argCount >= 1) {
      robotController->changeState(ROBOT_FORWARD);
      moving = true;
    } else if (strcmp("turnLeft", args[0]) == 0 && argCount >= 1) {
      robotController->changeState(ROBOT_TURNING_LEFT);
      moving = true;
    } else if (strcmp("turnRight", args[0]) == 0 && argCount >= 1) {
      robotController->changeState(ROBOT_TURNING_RIGHT);
      moving = true;
    } else if (strcmp("backward", args[0]) == 0 && argCount >= 1) {
      robotController->changeState(ROBOT_BACKWARD);
      moving = true;
    } else if (strcmp("setSpeed", args[0]) == 0 && argCount == 2) {
      unsigned int speed = strtoul(args[1], NULL, 10);
      if (speed != UINTMAX_MAX) {
        robotController->setRobotSpeed(speed);
      }
    } else if (strcmp("calibrateTurning", args[0]) == 0 && argCount == 2) {
      unsigned int turnLengthMs = strtoul(args[1], NULL, 10);
      if (turnLengthMs != UINTMAX_MAX) {
        robotController->calibrateTurning(turnLengthMs);
      }
    } else if (strcmp("calibrateSpeed", args[0]) == 0 && argCount == 3) {
      unsigned int leftCal = strtoul(args[1], NULL, 10);
      unsigned int rightCal = strtoul(args[2], NULL, 10);
      if (leftCal != UINTMAX_MAX && rightCal != UINTMAX_MAX) {
        robotController->calibrateSpeed(leftCal, rightCal);
      }
    } else if (strcmp("sendCalibration", args[0]) == 0 && argCount == 1) {
      PublishEvent::QueueEvent(PUBLISH_EVENT_CALIBRATION);
      PublishEvent::QueueEvent(PUBLISH_EVENT_HAS_FAILED);
    } else if (strcmp("calibrateFriction", args[0]) == 0 && argCount == 2) {
      unsigned int friction = strtoul(args[1], NULL, 10);
      if (friction != UINTMAX_MAX) {
        robotController->calibrateFriction(friction);
      }
    } else if (strcmp("resetFailed", args[0]) == 0 && argCount == 1) {
      robotController->resetFailed();
    } else if (strcmp("calibrateDirection", args[0]) == 0 && argCount == 3) {
      unsigned int leftDir = strtoul(args[1], NULL, 10);
      unsigned int rightDir = strtoul(args[2], NULL, 10);
      if (rightDir != UINTMAX_MAX && leftDir != UINTMAX_MAX) {
        robotController->calibrateDirection(leftDir, rightDir);
      }
    }

    if (moving && argCount == 3) {
      Serial.println(args[1]);
      Serial.println(args[2]);
      robotController->motorsSetDistance(args[1], getDistanceUnitFromArg(args[2]));
    }
    free(args);
  }

  delete cstring;
  return 0;
}

void addRobotTimer(RobotTimer *timer) {
  robotTimers.push_back(timer);
}

RobotController* getRobotController() {
  return robotController;
}

StorageController* getStorageController() {
  return storageController;
}

UltrasonicSensor* getFrontUltrasonicSensor() {
  return ultrasonicSensor;
}

Gyroscope* getGyroscope() {
  return gyroscope;
}

DistanceUnit getDistanceUnitFromArg(char *arg) {
  if (strcmp("mm", arg) == 0) {
    return MM;
  } else if (strcmp("cm", arg) == 0) {
    return CM;
  } else if (strcmp("m", arg) == 0) {
    return M;
  } else if (strcmp("degrees", arg) == 0) {
    return DEGREES;
  }
  return (DistanceUnit) -2;
}

void serialOutput() {
  // Clear screen & home
  Serial.write(27);
  Serial.print("[2J");
  Serial.write(27);
  Serial.print("[H");

  Serial.println("Robotic Firmware - 2015 - Michael Drury\n");

  robotController->outputSerial();
  Serial.println("\n");
  ultrasonicSensor->outputSerial();
  gyroscope->outputSerial();
}
