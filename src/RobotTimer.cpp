#include "RobotTimer.h"
#include "RoboticFirmware.h"

RobotTimer::RobotTimer() {
  addRobotTimer(this);
}

void RobotTimer::start() {
  if (lengthMs != 0) {
    started = true;
    startTimeMs = millis();
    complete = false;
  }
}

void RobotTimer::stop() {
  started = false;
  startTimeMs = 0;
  complete = false;
}

void RobotTimer::process() {
  if (!started) {
    return;
  }

  if ((lengthMs + startTimeMs) < millis()) {
    complete = true;
    startTimeMs = 0;
  }
}

void RobotTimer::setDuration(unsigned int length, TimeUnit unit) {
  this->stop();
  this->lengthMs = length * unit;
}

bool RobotTimer::isComplete() {
  if (!started) {
    return false;
  }

  bool returnComplete = complete;
  if (complete) {
    complete = false;
  }

  return returnComplete;
}
