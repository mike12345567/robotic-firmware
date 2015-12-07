#include "Gyroscope.h"

Gyroscope::Gyroscope() {
  Wire.begin();

  impl.initialize();
  connectionSuccess = impl.testConnection();
}

void Gyroscope::process() {
  /* read raw accel/gyro measurements from device */
  impl.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
}

void Gyroscope::outputSerial() {
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


