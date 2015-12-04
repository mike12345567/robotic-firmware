#include "PublishEvent.h"
#include "RobotController.h"
#include "application.h"
#include "Calibration.h"
#include "RoboticFirmware.h"
#include "libraries/Base64.h"

#include<stdarg.h>

#define MAX_PUBLISH_BYTES 63
#define DEFAULT_PUBLISH_TTL 60

static char packedBytes[MAX_PUBLISH_BYTES];
static char publishArray[MAX_PUBLISH_BYTES];

void PublishEvent::PublishComplete() {
  Particle.publish("complete");
}

void PublishEvent::PublishCalibration() {
  Calibration *leftCal = getRobotController()->getCalibration(true);
  Calibration *rightCal = getRobotController()->getCalibration(false);
  unsigned int speed = getRobotController()->getSpeed();
  unsigned int turnCalibration = leftCal->getTurnCalibration();
  unsigned int leftSpeedCal = leftCal->getSpeedCalibration();
  unsigned int rightSpeedCal = rightCal->getSpeedCalibration();

  unsigned int byteCount = PublishEvent::PackBytes(4, speed, rightSpeedCal,
      leftSpeedCal, turnCalibration);

  memset(&publishArray, 0, byteCount+1);

  int length = base64_enc_len(byteCount);
  base64_encode(publishArray, packedBytes, byteCount);

  Particle.publish("calibrationValues", publishArray, DEFAULT_PUBLISH_TTL, PRIVATE);
}

unsigned int PublishEvent::PackBytes(int numberInts, ...) {
  memset(&packedBytes, 0, sizeof(packedBytes));
  unsigned int byteCount = 0;

  va_list ap;
  va_start(ap, numberInts);
  for(int i = 1; i <= numberInts; i++) {
    unsigned int integer = va_arg(ap, int);
    /* values no larger than 16 bits unsigned */
    packedBytes[byteCount++] = integer & 0xFF;
    packedBytes[byteCount++] = (integer >> 8) & 0xFF;

  }
  va_end(ap);

  return byteCount;
}
