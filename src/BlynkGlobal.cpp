/**
 * @file BlynkGlobal.cpp
 * @brief Blynk wrapper functions and event handlers implementation
 */

#include "BlynkGlobal.h"
#include "AppGlobals.h"
#include "UIGating.h"
#include "Config.h"
#include <BlynkSimpleEsp32.h>

// =================== Blynk Event Handlers ===================
BLYNK_CONNECTED() {
    Serial.println("[BLYNK] Connected, synchronizing.");
    Blynk.syncVirtual(VPIN_STOVE_POWER_SWITCH,
                      VPIN_POWER_LEVEL_WRITE,
                      VPIN_SET_TIMER_MIN,
                      VPIN_SCHED_GLOBAL_ENABLE,
                      VPIN_SCHED_INDEX);
}

BLYNK_WRITE(VPIN_STOVE_POWER_SWITCH) {
    gBlynk.handleOnOff(param.asInt());
}

BLYNK_WRITE(VPIN_POWER_LEVEL_WRITE) {
    gBlynk.handleSetPower((uint8_t)param.asInt());
}

BLYNK_WRITE(VPIN_SET_TIMER_MIN) {
    gBlynk.handleSetTimer((uint32_t)param.asInt());
}

BLYNK_WRITE(VPIN_SCHED_GLOBAL_ENABLE) {
    gBlynk.handleSchedulerEnable(param.asInt());
}

BLYNK_WRITE(VPIN_SCHED_INDEX) {
    size_t idx = (size_t)param.asInt();
    gBlynk.updateSchedIndex(idx);
    ScheduleEntry e = gScheduler.getEntry(idx >= MAX_SCHEDULE_ENTRIES ? MAX_SCHEDULE_ENTRIES - 1 : idx);
    gBlynk.updateSchedActive(e.active);
    gBlynk.updateSchedDay((uint8_t)e.day);
    gBlynk.updateSchedHour(e.hour);
    gBlynk.updateSchedMinute(e.minute);
    gBlynk.updateSchedPower(e.targetPower);
    gBlynk.reflectPendingSchedulerFields();
}

BLYNK_WRITE(VPIN_SCHED_ACTIVE) {
    gBlynk.updateSchedActive(param.asInt() == 1);
}

BLYNK_WRITE(VPIN_SCHED_DAY) {
    gBlynk.updateSchedDay((uint8_t)param.asInt());
}

BLYNK_WRITE(VPIN_SCHED_HOUR) {
    gBlynk.updateSchedHour((uint8_t)param.asInt());
}

BLYNK_WRITE(VPIN_SCHED_MINUTE) {
    gBlynk.updateSchedMinute((uint8_t)param.asInt());
}

BLYNK_WRITE(VPIN_SCHED_POWER) {
    gBlynk.updateSchedPower((uint8_t)param.asInt());
}

BLYNK_WRITE(VPIN_SCHED_APPLY) {
    if (param.asInt() == 1) {
        gBlynk.handleSchedulerApply();
    }
}

BLYNK_WRITE(VPIN_SCHED_REFRESH) {
    if (param.asInt() == 1) {
        gBlynk.pushSchedulerSummary(gScheduler.buildSummary());
    }
}

// =================== Wrapper Functions ===================
namespace BlynkWrapper {
    void virtualWrite(uint8_t pin, int value) {
        Blynk.virtualWrite(pin, value);
    }
    
    void virtualWrite(uint8_t pin, const String& value) {
        Blynk.virtualWrite(pin, value);
    }
    
    void setProperty(uint8_t pin, const char* property, const char* value) {
        Blynk.setProperty(pin, property, value);
    }
    
    void syncVirtual(uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, uint8_t pin5) {
        Blynk.syncVirtual(pin1, pin2, pin3, pin4, pin5);
    }
    
    void begin(const char* auth, const char* ssid, const char* pass) {
        Blynk.begin(auth, ssid, pass);
    }
    
    bool connected() {
        return Blynk.connected();
    }
    
    void run() {
        Blynk.run();
    }
}
