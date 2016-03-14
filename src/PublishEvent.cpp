#include "PublishEvent.h"
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

static char packedBytes[MAX_PUBLISH_BYTES];
static char publishArray[MAX_PUBLISH_BYTES];

RobotTimer* PublishEvent::intervalTimer = NULL;
RobotTimer* PublishEvent::queueEmptyTimer = NULL;
publishQueue PublishEvent::events;

void PublishEvent::Setup() {
  PublishEvent::intervalTimer = new RobotTimer(true);
  PublishEvent::intervalTimer->setDuration(INTERVAL_TIMER_MS, MILLISECONDS);
  PublishEvent::intervalTimer->start();

  PublishEvent::queueEmptyTimer = new RobotTimer(true);
  PublishEvent::queueEmptyTimer->setDuration(QUEUE_EMPTY_RATE_MS, MILLISECONDS);
  PublishEvent::queueEmptyTimer->start();
}

void PublishEvent::QueueEvent(PublishEvents event) {
  for (auto iterator = PublishEvent::events.begin();
       iterator != PublishEvent::events.end(); iterator++) {
    if (*iterator == event)
      return;
  }
  PublishEvent::events.push_back(event);
}

void PublishEvent::PublishComplete() {
  Particle.publish("complete");
}

void PublishEvent::PublishStopped() {
  Particle.publish("stopped");
}

void PublishEvent::PublishFailed() {
  Particle.publish("failed");
}

void PublishEvent::PublishHasFailed() {
  if (getRobotController()->hasFailed()) {
    Particle.publish("hasFailed");
  }
}

void PublishEvent::PublishCalibration() {
  Calibration *leftCal = getRobotController()->getCalibration(true);
  Calibration *rightCal = getRobotController()->getCalibration(false);
  unsigned int speed = getRobotController()->getSpeed();
  unsigned int turnCalibration = leftCal->getTurnCalibration();
  unsigned int leftSpeedCal = leftCal->getFwdSpeedCalibration();
  unsigned int rightSpeedCal = rightCal->getFwdSpeedCalibration();
  unsigned int frictionCal = leftCal->getFrictionCalibration();
  unsigned int leftDirection = leftCal->getMotorDirection();
  unsigned int rightDirection = rightCal->getMotorDirection();

  unsigned int byteCount = PublishEvent::PackBytes(false, 7,
      speed, rightSpeedCal, leftSpeedCal, turnCalibration,
      frictionCal, leftDirection, rightDirection);
  PublishEvent::Publish("calibrationValues", byteCount);
}

void PublishEvent::PublishUltrasonic() {
  unsigned int distance = getFrontUltrasonicSensor()->getDistanceCm();

  unsigned int byteCount = PublishEvent::PackBytes(false, 4, distance);
  PublishEvent::Publish("distanceCm", byteCount);
}

void PublishEvent::PublishGyroscope() {
  GyroscopeReadings* readings = getGyroscope()->getCurrentReadings();

  unsigned int byteCount = PublishEvent::PackBytes(true, 6,
      readings->accelX, readings->accelY, readings->accelZ,
      readings->gyroX, readings->gyroY, readings->gyroZ);
  PublishEvent::Publish("gyroscopeReadings", byteCount);
}

void PublishEvent::PublishIntervalBased() {
  PublishEvent::QueueEvent(PUBLISH_EVENT_ULTRASONIC);
  PublishEvent::QueueEvent(PUBLISH_EVENT_GYROSCOPE);
}

void PublishEvent::Process() {
  if (PublishEvent::intervalTimer->isComplete()) {
    PublishEvent::PublishIntervalBased();
  }

  if (PublishEvent::queueEmptyTimer->isComplete()) {
    PublishEvent::PublishFromQueue();
  }
}

void PublishEvent::PublishFromQueue() {
  if (events.empty()) {
    return;
  }
  PublishEvents event = events.front();
  events.pop_front();
  Serial.println("EVENT OCCURRED!");

  switch (event) {
    case PUBLISH_EVENT_COMPLETE:
      PublishEvent::PublishComplete();
      break;
    case PUBLISH_EVENT_STOP:
      PublishEvent::PublishStopped();
      break;
    case PUBLISH_EVENT_FAIL:
      PublishEvent::PublishFailed();
      break;
    case PUBLISH_EVENT_GYROSCOPE:
      PublishEvent::PublishGyroscope();
      break;
    case PUBLISH_EVENT_ULTRASONIC:
      PublishEvent::PublishUltrasonic();
      break;
    case PUBLISH_EVENT_CALIBRATION:
      PublishEvent::PublishCalibration();
      break;
    case PUBLISH_EVENT_HAS_FAILED:
      PublishEvent::PublishHasFailed();
      break;
  }
}

unsigned int PublishEvent::PackBytes(bool sign, int numberInts, ...) {
  memset(&packedBytes, 0, sizeof(packedBytes));
  unsigned int byteCount = 0;

  va_list ap;
  va_start(ap, numberInts);
  for(int i = 2; i <= numberInts+2; i++) {
    if (!sign) {
      unsigned int integer = va_arg(ap, unsigned int);
      /* values no larger than 16 bits unsigned */
      packedBytes[byteCount++] = integer & 0xFF;
      packedBytes[byteCount++] = (integer >> 8) & 0xFF;
    } else {
      int integer = va_arg(ap, int);
      /* values no larger than 16 bits signed */
      packedBytes[byteCount++] = integer & 0xFF;
      packedBytes[byteCount++] = (integer >> 8) & 0xFF;
    }
  }
  va_end(ap);

  return byteCount;
}

void PublishEvent::Publish(const char *url, unsigned int byteCount) {
  memset(&publishArray, 0, byteCount+1);

  base64_enc_len(byteCount);
  base64_encode(publishArray, packedBytes, byteCount);

  Particle.publish(url, publishArray, DEFAULT_PUBLISH_TTL, PRIVATE);
}
