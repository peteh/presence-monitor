#include <Arduino.h>
#include <SoftwareSerial.h>
#include <esplog.h>

#include "Sensor.h"
// motion detection sensitivity
#define TH1 400
// presence detection sensitivity
#define TH2 400

Sensor mySensor(D2, D1);

void setup()
{
  Serial.begin(115200);
  mySensor.begin();
  delay(2000);
  Serial.println("Boot Complete");
}

bool g_motion = false;
bool g_occupancy = false;

unsigned long lastCommandSend = millis();

void loop()
{
  if (millis() - lastCommandSend > 10000)
  {
    Serial.println("Sending Command");
    // mySerial.printf("th1=%d\n", TH1);
    // mySerial.printf("th2=%d\n", TH2);
    // mySerial.printf("getall\n");
    mySensor.setMotionThreshold(200);
    
    mySensor.setOccupancyThreshold(200);
    while(!mySensor.loop());
    mySensor.sendRequest();
    while(!mySensor.loop());
    lastCommandSend = millis();
    Serial.println("Finished Command");
  }
  mySensor.loop();
  bool occupancy = mySensor.hasOccupancy();
  bool motion = mySensor.hasMotion();
  if (g_occupancy != occupancy)
  {
    g_occupancy = occupancy;
    log_info("Occupancy: %d", occupancy);
  }
  if (g_motion != motion)
  {
    g_motion = motion;
    log_info("Motion: %d", motion);
  }
}