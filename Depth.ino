// Example application

#include <ESP8266RevK.h>
#include <Wire.h>
#include <VL53L0X.h>

#define app_settings  \
  n(timeinitial,60);   \
  n(timewake,60);   \
  n(timesleep,60); \
  n(sda,0); \
  n(scl,2); \
  n(budget,50); \
  s(url); \
  n(address,0x29); \

#define s(n) const char*n=NULL;
#define n(n,d) unsigned int n=d;
  app_settings
#undef n
#undef s

  VL53L0X sensor;
  boolean sensorok = false;

  // Set the OTA host as you require, the rest is set based on filename and date/time
  ESP8266RevK revk(__FILE__, __DATE__ " " __TIME__); //, "ota.revk.uk", "IoT", "insecure", "mqtt.revk.uk");

  const char* app_setting(const char *tag, const byte *value, size_t len)
  { // Called for settings retrieved from EEPROM, return PSTR for tag if setting is OK
#define s(n) do{const char *t=PSTR(#n);if(!strcmp_P(tag,t)){n=(char*)value;return t;}}while(0)
#define n(n,d) do{const char *t=PSTR(#n);if(!strcmp_P(tag,t)){n=value?atoi((char*)value):d;return t;}}while(0)
    app_settings
#undef n
#undef s
    return NULL; // Failed
  }

  boolean app_command(const char*tag, const byte *message, size_t len)
  { // Called for incoming MQTT messages, return true if message is OK
    return false; // Failed
  }

  void setup()
  { // Your set up here as usual
    debug("Start Depth checker");
    debugf("Start wire %d/%d", sda, scl);
    Wire.begin(sda, scl);
    debugf("Check address %02X", address);
    Wire.beginTransmission((byte)address);
    if (Wire.endTransmission())
    {
      debug("VL53L0X failed");
      return;
    }
    sensorok = true;
    debug("Sensor init");
    sensor.init();
    debug("Sensor setup");
    if (budget >= 50)
    { // long range
      sensor.setSignalRateLimit(0.1);
      sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
      sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
    }
    sensor.setMeasurementTimingBudget(budget * 1000);
    sensor.startContinuous();
    sensor.setTimeout(10); // continuous

    debug("VL53L0X OK");
  }

  void loop()
  {
    revk.loop();
    long now = millis();
    static unsigned long bedtime = timeinitial * 1000;
    if ((int)(bedtime - now) < 0)
    {
      revk.info(F("sleep"), F("%d"), timesleep);
      revk.sleep(timesleep);
      bedtime = now + timewake * 1000;
    }
    if (!sensorok)
      return;
    unsigned int range = sensor.readRangeContinuousMillimeters();
    revk.event(F("range"), F("%d"), range);
    if (url)
    { // report via URL
      // TODO
    }
    delay(1000);
  }
