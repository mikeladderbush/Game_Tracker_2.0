#include "accelerometer.h"
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

static Adafruit_LIS3DH lis;
static bool available_ = false;

bool initAccelerometer() {
    available_ = lis.begin(0x19);
    if (available_) lis.setRange(LIS3DH_RANGE_4_G);
    return available_;
}

TiltReading readTilt() {
    if (!available_) return TiltReading();
    sensors_event_t event;
    lis.getEvent(&event);
    TiltReading t;
    t.x = event.acceleration.x / 9.8f;
    t.y = event.acceleration.y / 9.8f;
    return t;
}
