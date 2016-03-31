#ifndef PUBLISHEVENT_H_
#define PUBLISHEVENT_H_

#include "application.h"
#include "RobotTimer.h"
#include "MapHack.h"
#include "NetworkController.h"
#include <deque>

enum EventNames {
  PUBLISH_EVENT_COMPLETE = 1,
  PUBLISH_EVENT_STOP = 2,
  PUBLISH_EVENT_FAIL = 3,
  PUBLISH_EVENT_CALIBRATION = 4,
  PUBLISH_EVENT_ULTRASONIC = 5,
  PUBLISH_EVENT_GYROSCOPE = 6,
  PUBLISH_EVENT_HAS_FAILED = 7,
  PUBLISH_EVENT_LOCAL_IP = 8
};

typedef std::deque<EventNames> EventQueue;

class EventController {
  private:
    String id;
    RobotTimer* intervalTimer = NULL;
    RobotTimer* queueEmptyTimer = NULL;
    EventQueue events;
    NetworkController* networkController = NULL;

    unsigned int packBytes(bool sign, int numberInts, ...);

    void publishComplete();
    void publishStopped();
    void publishFailed();
    void publishHasFailed();
    void publishCalibration();
    void publishUltrasonic();
    void publishGyroscope();
    void publishLocalIP();

    void publishIntervalBased();
    void publishFromQueue();
    void publish(const char *url, unsigned int byteCount);
    void publishString(const char *url, char *string);
    const char* getURL(EventNames name);
  public:
    EventController();

    NetworkController* getNetworkController();

    void queueEvent(EventNames event);
    void process();
    const char* getID();
};

#endif
