#ifndef PINMAPPING_H_
#define PINMAPPING_H_

#include "application.h"

class PinMapping {
  private:
  public:
    const static unsigned int brakePinR = A4;
    const static unsigned int motorPinR = D3;
    const static unsigned int speedPinR = WKP;
    const static unsigned int brakePinL = A5;
    const static unsigned int motorPinL = D4;
    const static unsigned int speedPinL = D2;
    const static unsigned int triggerPin = D1;
    const static unsigned int echoPin = D0;
};

#endif
