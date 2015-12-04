#include "Calibration.h"

Calibration::Calibration() {

}

void Calibration::calibrateSpeed(unsigned int extraSpeed) {
  this->speedCalibration = extraSpeed;
}

void Calibration::calibrateTurning(unsigned int turnTimeMs) {
  this->turnCalibration = turnTimeMs;
}

unsigned int Calibration::getTurnCalibration() {
  return this->turnCalibration;
}

unsigned int Calibration::getSpeedCalibration() {
  return this->speedCalibration;
}
