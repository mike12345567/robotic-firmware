#ifndef ULTRASONICSENSOR_H_
#define ULTRASONICSENSOR_H_

#include "MapHack.h"
#include <deque>

class UltrasonicSensor {
  private:
    unsigned int echoPin;
    unsigned int triggerPin;
    bool setupComplete = false;
    typedef std::deque<unsigned int> samplesQueue;

    samplesQueue rawQueue;

    unsigned int getRawDistance();
  public:
    UltrasonicSensor();
    void process();
    unsigned int getDistanceCm();
    void outputSerial();
};

#endif
