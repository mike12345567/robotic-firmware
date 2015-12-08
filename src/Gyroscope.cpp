#include "Gyroscope.h"
#include "RoboticFirmware.h"
#include "RobotController.h"

#define FAILED_TILT_LEVEL 5000

Gyroscope::Gyroscope() {
  Wire.begin();
  memset(&readings, 0, sizeof(readings));

  impl.initialize();
  connectionSuccess = impl.testConnection();
}

void Gyroscope::process() {
  /* read raw accel/gyro measurements from device */
  impl.getMotion6(&readings.accelX, &readings.accelY, &readings.accelZ,
                  &readings.gyroX, &readings.gyroY, &readings.gyroZ);

  int16_t ax = readings.accelX;
  int16_t ay = readings.accelY;

  if (readings.accelX < FAILED_TILT_LEVEL*-1 || ax > FAILED_TILT_LEVEL ||
      ay < FAILED_TILT_LEVEL*-1 || ay > FAILED_TILT_LEVEL) {
    getRobotController()->tiltOccurred(ax, ay);
  }
}

/* NOTE: VALUES MAY CHANGE EACH CYCLE */
GyroscopeReadings* Gyroscope::getCurrentReadings() {
  return &readings;
}

void Gyroscope::outputSerial() {
  int16_t ax = readings.accelX;
  int16_t ay = readings.accelY;
  int16_t az = readings.accelZ;
  int16_t gx = readings.gyroX;
  int16_t gy = readings.gyroY;
  int16_t gz = readings.gyroZ;
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


