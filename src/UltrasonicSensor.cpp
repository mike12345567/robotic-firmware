#include "UltrasonicSensor.h"
#include "RobotController.h"
#include "application.h"
#include "PinMapping.h"
#include "RoboticFirmware.h"

#define HIGH_PULSE_LENGTH 10
#define QUEUE_MAX_LENGTH 3
#define SPEED_SOUND_MICROSECONDS_CM 29 // 340 m/s (29 us/cm)
#define SPIKE_THRESHOLD 2.0
#define BLOCKED_DISTANCE_CM 15

UltrasonicSensor::UltrasonicSensor(UltrasonicPosition position) {
  this->position = position;
  this->echoPin = PinMapping::echoPin;
  this->triggerPin = PinMapping::triggerPin;

  pinMode(triggerPin, OUTPUT);
  digitalWriteFast(triggerPin, LOW);
  pinMode(echoPin, INPUT);
}

void UltrasonicSensor::process() {
  if (!setupComplete) {
      digitalWriteFast(triggerPin, LOW);
      delay(50);
      setupComplete = true;
  }

  digitalWriteFast(triggerPin, HIGH);
  delayMicroseconds(HIGH_PULSE_LENGTH);
  digitalWriteFast(triggerPin, LOW);

  unsigned int duration = pulseIn(echoPin, HIGH);

  if (rawQueue.size() < QUEUE_MAX_LENGTH) {
    rawQueue.push_back(duration);
  } else {
    rawQueue.push_back(duration);
    rawQueue.pop_front();
  }

  unsigned int distance = getDistanceCm();
  if (distance < BLOCKED_DISTANCE_CM) {
    getRobotController()->dangerClose(this->position, distance);
  }
}

bool UltrasonicSensor::checkValueConsitent(unsigned int value) {
  samplesQueue::iterator iterator;
  for (iterator = rawQueue.begin(); iterator != rawQueue.end(); iterator++) {
    if (value * SPIKE_THRESHOLD < *iterator ||
        value > *iterator * SPIKE_THRESHOLD) {
      return false;
    }
  }
  return true;
}

unsigned int UltrasonicSensor::getRawDistance() {
  samplesQueue::iterator iterator;
  unsigned long long total = 0ULL;

  for (iterator = rawQueue.begin(); iterator != rawQueue.end(); iterator++) {
    unsigned int value = *iterator;
    total += value;
  }

    return total / rawQueue.size();
}

unsigned int UltrasonicSensor::getDistanceCm() {
  unsigned int duration = getRawDistance();

  /* divide this by 2 as it is out and back */
  return duration / SPEED_SOUND_MICROSECONDS_CM / 2;
}

void UltrasonicSensor::outputSerial() {
  Serial.println("ULTRASONIC SENSOR");
  Serial.print("\tCurrent distance (cm) -> ");
  Serial.println(getDistanceCm());
}
