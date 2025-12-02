/**
 * @file BlynkHandlers.cpp
 * @brief Blynk callback setup implementation
 * 
 * Note: BLYNK_CONNECTED() and BLYNK_WRITE() handlers are defined in BlynkGlobal.cpp
 * to avoid multiple definitions of the Blynk instance.
 */

#include "BlynkHandlers.h"
#include "AppGlobals.h"
#include "UIGating.h"
#include "Config.h"

void setupBlynkCallbacks() {
    gBlynk.setOnOffCallback([](bool turnOn) {
        uiGate.onOffLocked = true;
        uiGate.onOffLockStart = millis();
        uiGate.reqOnOffDisable = true;
        if (gCommandQueue) {
            Command c{turnOn ? Command::START : Command::SHUTDOWN, 0, 0, 0, false, 0, 0, 0, 0};
            xQueueSend(gCommandQueue, &c, portMAX_DELAY);
        }
    });

    gBlynk.setPowerCallback([](uint8_t p) {
        uiGate.powerLocked = true;
        uiGate.powerLockStart = millis();
        uiGate.reqPowerDisable = true;
        if (gCommandQueue) {
            Command c{Command::SET_POWER, p, 0, 0, false, 0, 0, 0, 0};
            xQueueSend(gCommandQueue, &c, portMAX_DELAY);
        }
    });

    gBlynk.setTimerCallback([](uint32_t m) {
        uiGate.timerLocked = true;
        uiGate.timerLockStart = millis();
        uiGate.reqTimerDisable = true;
        if (gCommandQueue) {
            Command c{Command::SET_TIMER, 0, m, 0, false, 0, 0, 0, 0};
            xQueueSend(gCommandQueue, &c, portMAX_DELAY);
        }
    });

    gBlynk.setSchedulerEnableCallback([](bool en) {
        gScheduler.setGlobalEnabled(en);
    });

    gBlynk.setSchedulerApplyCallback([](size_t idx, bool active, uint8_t day, uint8_t hour, uint8_t minute, uint8_t power) {
        uiGate.schedLocked = true;
        uiGate.schedLockStart = millis();
        uiGate.reqSchedDisable = true;
        if (gCommandQueue) {
            Command c;
            c.type = Command::SCHED_APPLY;
            c.schedIndex = idx;
            c.schedActive = active;
            c.schedDay = day;
            c.schedHour = hour;
            c.schedMinute = minute;
            c.schedPower = power;
            c.power = 0;
            c.minutes = 0;
            xQueueSend(gCommandQueue, &c, portMAX_DELAY);
        }
    });
}

void setupBlynkEventHandlers() {
}
