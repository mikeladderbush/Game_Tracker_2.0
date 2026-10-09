/*
********************************************************************************

    The MatrixPortal's onboard LIS3DH (I2C 0x19, not the library default
    0x18). Optional - clock mode's sand effect biases drift with it if
    present, falls back to straight-down gravity if not.

********************************************************************************
*/

#pragma once

bool initAccelerometer();

struct TiltReading {
    float x = 0;
    float y = 0;
};

// {0,0} if initAccelerometer() wasn't called or found nothing.
TiltReading readTilt();
