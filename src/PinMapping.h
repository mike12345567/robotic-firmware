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
    const static unsigned int i2cSclPin = D1; //_SCL = D1;
    const static unsigned int i2cSdaPin = D0; //_SCA = D0;
    const static unsigned int triggerPin = D6;
    const static unsigned int echoPin = D5;
};

#endif
