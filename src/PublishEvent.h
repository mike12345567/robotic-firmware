#ifndef PUBLISHEVENT_H_
#define PUBLISHEVENT_H_

#include "application.h"
#include "RobotTimer.h"

class PublishEvent {
  private:
    static RobotTimer* intervalTimer;

    static unsigned int PackBytes(int numberInts, ...);

  public:
    static void Setup();
    static void PublishComplete();
    static void PublishStopped();
    static void PublishCalibration();
    static void PublishUltrasonic();
    static void Process();
    static void Publish(const char *url, unsigned int byteCount);
};

#endif
