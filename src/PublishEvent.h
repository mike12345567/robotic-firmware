#ifndef PUBLISHEVENT_H_
#define PUBLISHEVENT_H_

#include "application.h"
#include "RobotTimer.h"
#include "MapHack.h"
#include <deque>

enum PublishEvents {
  PUBLISH_EVENT_COMPLETE = 1,
  PUBLISH_EVENT_STOP = 2,
  PUBLISH_EVENT_FAIL = 3,
  PUBLISH_EVENT_CALIBRATION = 4,
  PUBLISH_EVENT_ULTRASONIC = 5,
  PUBLISH_EVENT_GYROSCOPE = 6,
  PUBLISH_EVENT_HAS_FAILED = 7
};

typedef std::deque<PublishEvents> publishQueue;

class PublishEvent {
  private:
    static RobotTimer* intervalTimer;
    static RobotTimer* queueEmptyTimer;
    static publishQueue events;

    static unsigned int PackBytes(bool sign, int numberInts, ...);
    static void PublishComplete();
    static void PublishStopped();
    static void PublishFailed();
    static void PublishHasFailed();
    static void PublishCalibration();
    static void PublishUltrasonic();
    static void PublishGyroscope();
    static void PublishIntervalBased();
    static void PublishFromQueue();
    static void Publish(const char *url, unsigned int byteCount);
  public:
    static void Setup();
    static void QueueEvent(PublishEvents event);
    static void Process();
};

#endif
