#ifndef PUBLISHEVENT_H_
#define PUBLISHEVENT_H_

#include "application.h"

class PublishEvent {
  private:
    static unsigned int PackBytes(int numberInts, ...);
  public:
    static void PublishComplete();
    static void PublishCalibration();
};

#endif
