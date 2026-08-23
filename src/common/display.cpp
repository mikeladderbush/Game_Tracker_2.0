#include <Adafruit_Protomatter.h>
#include <cstdint>
#include "display.h"

// Order is [R1, G1, B1, R2, G2, B2] per Adafruit_Protomatter. R and B were
// swapped here (confirmed: a near-pure red team color rendered as blue) -
// this corrects it in software by relabeling which physical pin is which
// channel, rather than re-wiring the panel. Affects every color on both
// NBA and NFL screens, not just one sport - they share this same matrix.
static uint8_t rgbPins[]  = {40, 41, 42, 37, 39, 38};
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