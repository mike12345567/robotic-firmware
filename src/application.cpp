#include "RoboticFirmware.h"
#include "spark_wiring_timer.h"
#include "PinMapping.h"

#include <cstring>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define SERIAL_DELAY_MS 1000
#define FORWARD_TIME_MS 1500
#define TURNING_TIME_MS 600
#define MOTOR_SPEED 128
#define SPEED_STR_LEN 3
#define MOTOR_R_CALIBRATION 10
#define BRAKES_ON 0

std::vector<RobotTimer*> robotTimers;

STARTUP(WiFi.selectAntenna(ANT_AUTO));
SYSTEM_MODE(AUTOMATIC);
SYSTEM_THREAD(ENABLED);

Timer serialTimer(SERIAL_DELAY_MS, serialOutput);

Motor* motorRight;
Motor* motorLeft;
RobotState state = ROBOT_FORWARD;

unsigned int nextStateChangeMs = 0;
unsigned int lastStateChangeMs = 0;

void setup() {
  Serial.begin(115200);
  motorRight = new Motor(motorPinR, brakePinR, speedPinR, false);
  motorRight->setSpeed(MOTOR_SPEED);
  motorLeft = new Motor(motorPinL, brakePinL, speedPinL, true);
  motorLeft->setSpeed(MOTOR_SPEED);

  motorRight->calibrate(MOTOR_R_CALIBRATION);
  Particle.function("makeMove", makeMove);
  serialTimer.start();
}

void loop() {
  motorRight->process();
  motorLeft->process();

  auto iterator = robotTimers.begin();

  while (iterator != robotTimers.end()) {
    RobotTimer* timer = *iterator;
    timer->process();
    iterator++;
  }
}

int makeMove(String param) {
  char *pointer = NULL;
  char *cstring = new char[param.length() + 1];
  char **args = NULL;
  // always one argument
  int argCount = 1;
  int argIndex = 0;

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

    bool moving = false;
    if (strcmp("stop", args[0]) == 0 && argCount == 1) {
      changeState(ROBOT_STOPPED);
    } else if (strcmp("forward", args[0]) == 0 && argCount >= 1) {
      changeState(ROBOT_FORWARD);
      moving = true;
    } else if (strcmp("turnLeft", args[0]) == 0 && argCount >= 1) {
      changeState(ROBOT_TURNING_LEFT);
      moving = true;
    } else if (strcmp("turnRight", args[0]) == 0 && argCount >= 1) {
      changeState(ROBOT_TURNING_RIGHT);
      moving = true;
    } else if (strcmp("backward", args[0]) == 0 && argCount >= 1) {
      changeState(ROBOT_BACKWARD);
      moving = true;
    } else if (strcmp("setSpeed", args[0]) == 0 && argCount == 2) {
      unsigned int speed = strtoul(args[1], NULL, 10);
      if (speed != UINTMAX_MAX) {
        motorRight->setSpeed(speed);
        motorLeft->setSpeed(speed);
      }
    }

    if (moving && argCount == 3) {
      Serial.println(args[1]);
      Serial.println(args[2]);
      motorsSetDistance(args[1], getDistanceUnitFromArg(args[2]));
    }
    free(args);
  }

  delete cstring;
  return 0;
}

void addRobotTimer(RobotTimer *timer) {
  robotTimers.push_back(timer);
}

void motorsSetDistance(char *arg, DistanceUnit unit) {
  unsigned int distance = strtoul(arg, NULL, 10);
  motorRight->moveForDistance(distance, unit);
  motorLeft->moveForDistance(distance, unit);
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

const char* robotStateToString() {
  switch (state) {
    case ROBOT_STOPPED:
      return "ROBOT_STOPPED";
    case ROBOT_FORWARD:
      return "ROBOT_FORWARD";
    case ROBOT_TURNING_LEFT:
      return "ROBOT_TURNING_LEFT";
    case ROBOT_TURNING_RIGHT:
      return "ROBOT_TURNING_RIGHT";
    case ROBOT_BACKWARD:
      return "ROBOT_BACKWARD";
  }

  return "UNKNOWN";
}

void changeState(RobotState newState) {
  Serial.print("CHANGING STATE -> ");
  Serial.println(robotStateToString());
  switch (newState) {
    case ROBOT_STOPPED:
      motorRight->setState(STOPPED);
      motorLeft->setState(STOPPED);
      break;
    case ROBOT_FORWARD:
      motorRight->setState(FORWARD);
      motorLeft->setState(FORWARD);
      break;
    case ROBOT_TURNING_LEFT:
      motorRight->setState(FORWARD);
      motorLeft->setState(STOPPED);
      break;
    case ROBOT_TURNING_RIGHT:
      motorRight->setState(STOPPED);
      motorLeft->setState(FORWARD);
      break;
    case ROBOT_BACKWARD:
      motorRight->setState(BACKWARD);
      motorLeft->setState(BACKWARD);
      break;
  }

  state = newState;
}

void serialOutput() {
  // Clear screen & home
  Serial.write(27);
  Serial.print("[2J");
  Serial.write(27);
  Serial.print("[H");

  Serial.println("Robotic Firmware - 2015 - Michael Drury\n");

  Serial.println("ROBOT STATE");
  Serial.print("\tCurrent -> ");
  Serial.println(robotStateToString());

  Serial.println("MOTOR A");
  motorRight->outputSerial();
  Serial.println("MOTOR B");
  motorLeft->outputSerial();
}
