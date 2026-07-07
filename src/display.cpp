#include <Adafruit_Protomatter.h>
#include <cstdint>
#include "display.h"

static uint8_t rgbPins[]  = {42, 41, 40, 38, 39, 37};
static uint8_t addrPins[] = {45, 36, 48, 35, 21};
static uint8_t clockPin = 2;
static uint8_t latchPin = 47;
static uint8_t oePin = 14;

Adafruit_Protomatter matrix(
    64,
    4,
    6,
    rgbPins,
    5,          
    addrPins,
    clockPin,
    latchPin,
    oePin,
    false,
    1           
);

void initDisplay() {
    auto status = matrix.begin();
    if (status != PROTOMATTER_OK) {
        while (1); // or log
    }
    matrix.fillScreen(0);
    matrix.show();
}