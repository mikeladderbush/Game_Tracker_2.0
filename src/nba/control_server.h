#pragma once
#include <Arduino.h>

struct AppInput {
    bool powerOn = false;
    char team[4] = {0};      
    bool teamPending = false;
};
extern AppInput appInput;

void beginControlServer(); 
void pollControlServer();  