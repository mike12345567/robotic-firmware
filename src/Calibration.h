#ifndef CALIBRATION_H_
#define CALIBRATION_H_

#define TIME_MS_FULL_CIRCLE 2050

class Calibration {
  private:
    unsigned int turnCalibration = TIME_MS_FULL_CIRCLE;
    unsigned int speedCalibration = 0;
  public:
    Calibration();
    void calibrateSpeed(unsigned int extraSpeed);
    void calibrateTurning(unsigned int turnTimeMs);

    unsigned int getTurnCalibration();
    unsigned int getSpeedCalibration();
};

#endif
