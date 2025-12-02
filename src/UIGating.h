/**
 * @file UIGating.h
 * @brief UI state gating and lock management for Blynk interface
 */

#pragma once

#include <Arduino.h>

struct UIGating {
    bool onOffLocked = false;
    uint32_t onOffLockStart = 0;
    bool reqOnOffDisable = false;
    
    bool powerLocked = false;
    uint32_t powerLockStart = 0;
    bool reqPowerDisable = false;
    
    bool timerLocked = false;
    uint32_t timerLockStart = 0;
    bool reqTimerDisable = false;
    
    bool schedLocked = false;
    uint32_t schedLockStart = 0;
    bool reqSchedDisable = false;
};

extern UIGating uiGate;
extern bool uiForceSwitchOn;

void initUIGating();
