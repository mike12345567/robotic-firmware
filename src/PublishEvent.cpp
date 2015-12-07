#include "PublishEvent.h"
#include "RobotController.h"
#include "application.h"
#include "Calibration.h"
#include "RoboticFirmware.h"
#include "libraries/Base64.h"

#include<stdarg.h>

#define MAX_PUBLISH_BYTES 63
#define DEFAULT_PUBLISH_TTL 60
#define INTERVAL_TIMER_MS 5000

static char packedBytes[MAX_PUBLISH_BYTES];
static char publishArray[MAX_PUBLISH_BYTES];

RobotTimer* PublishEvent::intervalTimer = NULL;

void PublishEvent::Setup() {
  PublishEvent::intervalTimer = new RobotTimer(true);
  PublishEvent::intervalTimer->setDuration(INTERVAL_TIMER_MS, MILLISECONDS);
  PublishEvent::intervalTimer->start();
}

void PublishEvent::PublishComplete() {
  Particle.publish("complete");
}

void PublishEvent::PublishStopped() {
  Particle.publish("stopped");
}

void PublishEvent::PublishCalibration() {
  Calibration *leftCal = getRobotController()->getCalibration(true);
  Calibration *rightCal = getRobotController()->getCalibration(false);
  unsigned int speed = getRobotController()->getSpeed();
  unsigned int turnCalibration = leftCal->getTurnCalibration();
  unsigned int leftSpeedCal = leftCal->getFwdSpeedCalibration();
  unsigned int rightSpeedCal = rightCal->getFwdSpeedCalibration();
  unsigned int frictionCal = leftCal->getFrictionCalibration();

  unsigned int byteCount = PublishEvent::PackBytes(4, speed, rightSpeedCal,
      leftSpeedCal, turnCalibration, frictionCal);
  PublishEvent::Publish("calibrationValues", byteCount);
}

void PublishEvent::PublishUltrasonic() {
  unsigned int distance = getFrontUltrasonicSensor()->getDistanceCm();

  unsigned int byteCount = PublishEvent::PackBytes(4, distance);
  PublishEvent::Publish("distanceCm", byteCount);
}

void PublishEvent::Process() {
  if (PublishEvent::intervalTimer->isComplete()) {
    PublishEvent::PublishUltrasonic();
  }
}

unsigned int PublishEvent::PackBytes(int numberInts, ...) {
  memset(&packedBytes, 0, sizeof(packedBytes));
  unsigned int byteCount = 0;

  va_list ap;
  va_start(ap, numberInts);
  for(int i = 1; i <= numberInts+1; i++) {
    unsigned int integer = va_arg(ap, int);
    /* values no larger than 16 bits unsigned */
    packedBytes[byteCount++] = integer & 0xFF;
    packedBytes[byteCount++] = (integer >> 8) & 0xFF;

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
