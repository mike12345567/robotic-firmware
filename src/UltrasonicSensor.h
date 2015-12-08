#ifndef ULTRASONICSENSOR_H_
#define ULTRASONICSENSOR_H_

#include "MapHack.h"
#include <deque>

enum UltrasonicPosition {
  US_POSITION_FRONT,
};

class UltrasonicSensor {
  private:
    UltrasonicPosition position;
    unsigned int echoPin;
    unsigned int triggerPin;
    bool setupComplete = false;
    typedef std::deque<unsigned int> samplesQueue;

    samplesQueue rawQueue;

    unsigned int getRawDistance();
    bool checkValueConsitent(unsigned int value);
  public:
    UltrasonicSensor(UltrasonicPosition position);
    void process();
    unsigned int getDistanceCm();
    void outputSerial();
};

#endif
