/*
********************************************************************************

    Shared state between the control server and main.cpp's stateTask.

********************************************************************************
*/

#pragma once
#include <Arduino.h>

struct AppInput {
    bool powerOn = false;
    char team[4] = {0};
    bool teamPending = false;
    char sport[8] = "NBA";  // room for "CLOCK\0" (6) plus slack for future modes
    bool sportPending = false;
};
extern AppInput appInput;

void beginControlServer(); 
void pollControlServer();  