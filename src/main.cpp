/**
 * @file main.cpp
 * @brief Entry point for Micronova pellet stove control system
 */

#include <Arduino.h>
#include "Application.h"

void setup() {
    gApp.initialize();
}

void loop() {
    gApp.run();
}