#include "EventController.h"
#include "RobotController.h"
#include "application.h"
#include "Calibration.h"
#include "Gyroscope.h"
#include "RoboticFirmware.h"
#include "libraries/Base64.h"

#include <stdarg.h>

#define MAX_PUBLISH_BYTES 63
#define DEFAULT_PUBLISH_TTL 60
#define INTERVAL_TIMER_MS 5000
#define QUEUE_EMPTY_RATE_MS 300
#define STR_BUFFER 64

static char packedBytes[MAX_PUBLISH_BYTES];
static char publishArray[MAX_PUBLISH_BYTES];
static char localBuffer[MAX_PUBLISH_BYTES];
static char localURLBuffer[STR_BUFFER];

EventController::EventController() {
  networkController = new NetworkController();

  intervalTimer = new RobotTimer(true);
  intervalTimer->setDuration(INTERVAL_TIMER_MS, MILLISECONDS);
  intervalTimer->start();

  queueEmptyTimer = new RobotTimer(true);
  queueEmptyTimer->setDuration(QUEUE_EMPTY_RATE_MS, MILLISECONDS);
  queueEmptyTimer->start();

  /* copy to avoid memory corruption */
  id = Particle.deviceID();
}

NetworkController* EventController::getNetworkController() {
  return networkController;
}

const char* EventController::getURL(EventNames name) {
  switch (name) {
    case PUBLISH_EVENT_COMPLETE:
      return "complete";
    case PUBLISH_EVENT_STOP:
      return "stopped";
    case PUBLISH_EVENT_FAIL:
      return "failed";
    case PUBLISH_EVENT_CALIBRATION:
      return "calibrationValues";
    case PUBLISH_EVENT_ULTRASONIC:
      return "distanceCm";
    case PUBLISH_EVENT_GYROSCOPE:
      return "gyroscopeReadings";
    case PUBLISH_EVENT_HAS_FAILED:
      return "hasFailed";
    case PUBLISH_EVENT_LOCAL_IP:
      return "localIP";
    case PUBLISH_EVENT_MOVE_STATUS:
      return "moveStatus";
  }
  return "unknown";
}

void EventController::queueEvent(EventNames event) {
  for (auto iterator = events.begin();
       iterator != events.end(); iterator++) {
    if (*iterator == event)
      return;
  }
  events.push_back(event);
}

void EventController::publishComplete() {
  Particle.publish(getURL(PUBLISH_EVENT_COMPLETE));
}

void EventController::publishStopped() {
  Particle.publish(getURL(PUBLISH_EVENT_STOP));
}

void EventController::publishFailed() {
  Particle.publish(getURL(PUBLISH_EVENT_FAIL));
}

void EventController::publishHasFailed() {
  if (getRobotController()->hasFailed()) {
    Particle.publish(getURL(PUBLISH_EVENT_HAS_FAILED));
  }
}

void EventController::publishCalibration() {
  Calibration *leftCal = getRobotController()->getCalibration(true);
  Calibration *rightCal = getRobotController()->getCalibration(false);
  unsigned int speed = getRobotController()->getSpeed();
  unsigned int turnCalibration = leftCal->getTurnCalibration();
  unsigned int leftSpeedCal = leftCal->getFwdSpeedCalibration();
  unsigned int rightSpeedCal = rightCal->getFwdSpeedCalibration();
  unsigned int frictionCal = leftCal->getFrictionCalibration();
  unsigned int leftDirection = leftCal->getMotorDirection();
  unsigned int rightDirection = rightCal->getMotorDirection();

  unsigned int byteCount = packBytes(false, 7,
      speed, rightSpeedCal, leftSpeedCal, turnCalibration,
      frictionCal, leftDirection, rightDirection);
  publish(PUBLISH_EVENT_CALIBRATION, byteCount);
  queueEvent(PUBLISH_EVENT_LOCAL_IP);
}

void EventController::publishUltrasonic() {
  unsigned int distance = getFrontUltrasonicSensor()->getDistanceCm();
  /* distance no more than 2 bytes */
  distance &= 0xFFFF;

  unsigned int byteCount = packBytes(false, 1, distance);
  publish(PUBLISH_EVENT_ULTRASONIC, byteCount);
}

void EventController::publishGyroscope() {
  if (!getGyroscope()->isReady()) return;
  GyroscopeReading* readings = getGyroscope()->getCurrentReadings();

  unsigned int byteCount = packBytes(true, 6,
      readings->accelX, readings->accelY, readings->accelZ,
      readings->gyroX, readings->gyroY, readings->gyroZ);
  publish(PUBLISH_EVENT_GYROSCOPE, byteCount);
}

void EventController::publishLocalIP() {
  IPAddress addr = WiFi.localIP();
  unsigned int byteCount = packBytes(false, 1, addr.raw().ipv4);
  publish(PUBLISH_EVENT_LOCAL_IP, byteCount);
}

void EventController::publishMoveStatus() {
  int isMoving = getRobotController()->isMoving() ? 1 : 0;

  unsigned int byteCount = packBytes(false, 1, isMoving);
  publish(PUBLISH_EVENT_MOVE_STATUS, byteCount);
}

void EventController::publishIntervalBased() {
  queueEvent(PUBLISH_EVENT_ULTRASONIC);
  queueEvent(PUBLISH_EVENT_GYROSCOPE);
}

void EventController::process() {
  if (EventController::intervalTimer->isComplete()) {
    publishIntervalBased();
  }

  if (EventController::queueEmptyTimer->isComplete()) {
    publishFromQueue();
  }

  networkController->process();
}

void EventController::publishFromQueue() {
  if (events.empty()) {
    return;
  }
  EventNames event = events.front();
  events.pop_front();

  switch (event) {
    case PUBLISH_EVENT_COMPLETE:
      publishComplete();
      break;
    case PUBLISH_EVENT_STOP:
      publishStopped();
      break;
    case PUBLISH_EVENT_FAIL:
      publishFailed();
      break;
    case PUBLISH_EVENT_GYROSCOPE:
      publishGyroscope();
      break;
    case PUBLISH_EVENT_ULTRASONIC:
      publishUltrasonic();
      break;
    case PUBLISH_EVENT_CALIBRATION:
      publishCalibration();
      break;
    case PUBLISH_EVENT_HAS_FAILED:
      publishHasFailed();
      break;
    case PUBLISH_EVENT_LOCAL_IP:
      publishLocalIP();
      break;
    case PUBLISH_EVENT_MOVE_STATUS:
      publishMoveStatus();
      break;
  }
}

bool EventController::onlyLocalEndpoint(EventNames name) {
  return (name == PUBLISH_EVENT_MOVE_STATUS);
}

unsigned int EventController::packBytes(bool sign, int numberInts, ...) {
  memset(&packedBytes, 0, sizeof(packedBytes));
  unsigned int byteCount = 0;

  va_list ap;
  va_start(ap, numberInts);
  for(int i = 2; i <= numberInts+2; i++) {
    if (!sign) {
      unsigned int integer = va_arg(ap, unsigned int);
      /* values no larger than 32 bits unsigned */
      packedBytes[byteCount++] = integer & 0xFF;
      packedBytes[byteCount++] = (integer >> 8) & 0xFF;
      if (((integer >> 16) & 0xFF) != 0) {
        packedBytes[byteCount++] = (integer >> 16) & 0xFF;
      }
      if (((integer >> 24) & 0xFF) != 0) {
        packedBytes[byteCount++] = (integer >> 24) & 0xFF;
      }
    } else {
      int integer = va_arg(ap, int);
      /* values no larger than 32 bits signed */
      packedBytes[byteCount++] = integer & 0xFF;
      packedBytes[byteCount++] = (integer >> 8) & 0xFF;
      packedBytes[byteCount++] = (integer >> 16) & 0xFF;
      packedBytes[byteCount++] = (integer >> 24) & 0xFF;
    }
  }
  va_end(ap);

  return byteCount;
}

void EventController::publish(EventNames name, unsigned int byteCount) {
  memset(&publishArray, 0, sizeof(publishArray));
  memset(&localBuffer, 0, sizeof(localBuffer));
  memset(&localURLBuffer, 0, sizeof(localURLBuffer));
  const char *url = getURL(name);

  strcat(localURLBuffer, id.c_str());
  strcat(localURLBuffer, "/");
  strcat(localURLBuffer, url);

  int base64Len = base64_enc_len(byteCount);
  base64_encode(publishArray, packedBytes, byteCount);
  memcpy(localBuffer, publishArray, sizeof(localBuffer));

  if (!onlyLocalEndpoint(name)) {
    Particle.publish(url, publishArray, DEFAULT_PUBLISH_TTL, PRIVATE);
  }
  networkController->sendPackedBytes(localURLBuffer, localBuffer, base64Len);
}

const char* EventController::getID() {
  return id.c_str();
}
