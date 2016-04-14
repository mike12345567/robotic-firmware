#include "Gyroscope.h"
#include "RoboticFirmware.h"
#include "RobotController.h"

#include <algorithm>

#define FAILED_TILT_LEVEL_X_Y 15000
#define FAILED_TILT_LEVEL_Z 1000
#define MAXIMUM_READINGS 30
int awd = 0;

#define TURNING_GYRO_THRESHOLD 2000

Gyroscope::Gyroscope() {
  Wire.begin();

  impl.initialize();
  connectionSuccess = impl.testConnection();
}

void Gyroscope::process() {
  /* read raw accel/gyro measurements from device */
  GyroscopeReading reading;
  impl.getMotion6(&reading.accelX, &reading.accelY, &reading.accelZ,
                  &reading.gyroX, &reading.gyroY, &reading.gyroZ);
  readings.push_back(reading);
  if (readings.size() > MAXIMUM_READINGS) {
    readings.pop_front();
  }

  if (isReady()) {
    GyroscopeReading* testReading = getCurrentReadings();
    int16_t ax = testReading->accelX;
    int16_t ay = testReading->accelY;
    int16_t az = testReading->accelZ;

    if (ax < FAILED_TILT_LEVEL_X_Y*-1 || ax > FAILED_TILT_LEVEL_X_Y ||
        ay < FAILED_TILT_LEVEL_X_Y*-1 || ay > FAILED_TILT_LEVEL_X_Y ||
        az < FAILED_TILT_LEVEL_Z) {
      getRobotController()->tiltOccurred(ax, ay);
    }
  }
}

bool Gyroscope::isTurning() {
  if (!isReady()) return false;
  GyroscopeReading* reading = getCurrentReadings();
  return reading->gyroZ >= TURNING_GYRO_THRESHOLD || reading->gyroZ <= TURNING_GYRO_THRESHOLD*1;
}

/* NOTE: VALUES MAY CHANGE EACH CYCLE */
GyroscopeReading* Gyroscope::getCurrentReadings() {
  if (isReady()) {
    std::deque<GyroscopeReading> temp(readings);
    std::nth_element(temp.begin(), temp.begin() + temp.size() / 2, temp.end(), compareGyroReading);
    return &temp.at(temp.size()/2);
  } else {
    return NULL;
  }
}

boolean Gyroscope::isReady() {
  return readings.size() == MAXIMUM_READINGS;
}

void Gyroscope::outputSerial() {
  Serial.println(readings.size());

  if (!isReady()) return;
  GyroscopeReading* reading = getCurrentReadings();

  int16_t ax = reading->accelX;
  int16_t ay = reading->accelY;
  int16_t az = reading->accelZ;
  int16_t gx = reading->gyroX;
  int16_t gy = reading->gyroY;
  int16_t gz = reading->gyroZ;
  Serial.println("GYRO");
  Serial.print("Gyro connection successful -> ");
  Serial.println(connectionSuccess ? "YES" : "NO");
  Serial.print("a/g:\t");
  Serial.print(ax); Serial.print("\t");
  Serial.print(ay); Serial.print("\t");
  Serial.print(az); Serial.print("\t");
  Serial.print(gx); Serial.print("\t");
  Serial.print(gy); Serial.print("\t");
  Serial.println(gz);
}

bool compareGyroReading(GyroscopeReading a, GyroscopeReading b) {
  int16_t ax = a.accelX - b.accelX;
  int16_t ay = a.accelY - b.accelY;
  int16_t az = a.accelZ - b.accelZ;
  int16_t gx = a.gyroX - b.gyroX;
  int16_t gy = a.gyroY - b.gyroY;
  int16_t gz = a.gyroZ - b.gyroZ;
  return (ax + ay + az + gx + gy + gz) > 0;
}


