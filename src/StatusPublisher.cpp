/**
 * @file StatusPublisher.cpp
 * @brief Blynk status publishing and UI state management implementation
 */

#include "StatusPublisher.h"
#include "AppGlobals.h"
#include "UIGating.h"
#include "BlynkGlobal.h"
#include "Config.h"

StatusPublisher gStatusPublisher;

void StatusPublisher::publishIfChanged(const StoveStatus& s) {
    uint32_t now = millis();

    if (s.state != mSnapshot.state && (now - mSnapshot.lastStatusPublishMs) >= STATUS_MIN_PUBLISH_INTERVAL_MS) {
        BlynkWrapper::virtualWrite(VPIN_STOVE_STATE_NUM, (int)s.state);
        String stateStr;
        switch(s.state) {
            case STOVE_OFF: stateStr = "Off"; break;
            case STOVE_STARTING: stateStr = "Starting"; break;
            case STOVE_LOADING_PELLET: stateStr = "Loading"; break;
            case STOVE_FIRE_PRESENT: stateStr = "Fire"; break;
            case STOVE_WORKING: stateStr = "Working"; break;
            case STOVE_FINAL_CLEAN: stateStr = "Cleaning"; break;
            default: stateStr = "Undefined"; break;
        }
        BlynkWrapper::virtualWrite(VPIN_STOVE_STATE_STRING, stateStr);
        mSnapshot.state = s.state;
        mSnapshot.lastStatusPublishMs = now;
    }

    if (s.powerLevel != mSnapshot.power && (now - mSnapshot.lastStatusPublishMs) >= STATUS_MIN_PUBLISH_INTERVAL_MS) {
        BlynkWrapper::virtualWrite(VPIN_POWER_LEVEL_READ, s.powerLevel);
        mSnapshot.power = s.powerLevel;
        mSnapshot.lastStatusPublishMs = now;
    }

    bool tempChanged = isnan(mSnapshot.ambientTemp) || (fabs(s.ambientTemp - mSnapshot.ambientTemp) >= TEMP_CHANGE_THRESHOLD);
    if (tempChanged && (now - mSnapshot.lastTempPublishMs) >= TEMP_MIN_PUBLISH_INTERVAL_MS) {
        BlynkWrapper::virtualWrite(VPIN_AMBIENT_TEMP, s.ambientTemp);
        mSnapshot.ambientTemp = s.ambientTemp;
        mSnapshot.lastTempPublishMs = now;
    }

    uint32_t remainMs = gController.getAutoShutdownRemainingMs();
    int remainMin = remainMs ? (int)((remainMs + 59999UL) / 60000UL) : 0;
    if (remainMin != mSnapshot.lastRemainMin) {
        BlynkWrapper::virtualWrite(VPIN_AUTO_SHUTDOWN_REMAIN, remainMin);
        mSnapshot.lastRemainMin = remainMin;
    }
}

void StatusPublisher::pushStatus() {
    if (gTerminal.isUserTyping()) return;
    
    StoveStatus s = gController.getStatusSnapshot();
    publishIfChanged(s);

    uint32_t now = millis();
    if (!uiGate.onOffLocked)
        gBlynk.enableOnOff(gController.isOn());

    if (uiGate.reqOnOffDisable) { gBlynk.disableOnOff(); uiGate.reqOnOffDisable = false; }
    if (uiGate.reqPowerDisable) { gBlynk.disablePowerSlider(); uiGate.reqPowerDisable = false; }
    if (uiGate.reqTimerDisable) { gBlynk.disableTimerInput(); uiGate.reqTimerDisable = false; }
    if (uiGate.reqSchedDisable) { gBlynk.disableSchedulerApply(); uiGate.reqSchedDisable = false; }

    if (uiGate.onOffLocked) {
        bool stoveOn = gController.isOn();
        if (stoveOn) {
            if (s.canShutdown || (now - uiGate.onOffLockStart > STOVE_START_CONFIRM_TIMEOUT_MS)) {
                uiGate.onOffLocked = false;
                gBlynk.enableOnOff(true);
            }
        } else {
            if (s.state == STOVE_OFF || (now - uiGate.onOffLockStart > STOVE_SHUTDOWN_CONFIRM_TIMEOUT_MS)) {
                uiGate.onOffLocked = false;
                gBlynk.enableOnOff(false);
            }
        }
        if (uiGate.onOffLocked && (now - uiGate.onOffLockStart > UI_REENABLE_FAILSAFE_MS)) {
            uiGate.onOffLocked = false;
            gBlynk.enableOnOff(gController.isOn());
        }
    }

    if (uiGate.powerLocked) {
        if (!gController.isPowerAdjustInProgress()) {
            uiGate.powerLocked = false;
            gBlynk.enablePowerSlider(gController.getPowerLevel());
        } else if (now - uiGate.powerLockStart > POWER_ADJUST_TIMEOUT_MS) {
            uiGate.powerLocked = false;
            gBlynk.enablePowerSlider(gController.getPowerLevel());
        }
        if (uiGate.powerLocked && (now - uiGate.powerLockStart > UI_REENABLE_FAILSAFE_MS)) {
            uiGate.powerLocked = false;
            gBlynk.enablePowerSlider(gController.getPowerLevel());
        }
    }

    if (uiGate.timerLocked) {
        if (now - uiGate.timerLockStart > 1500) {
            uiGate.timerLocked = false;
            BlynkWrapper::setProperty(VPIN_SET_TIMER_MIN, "isDisabled", "false");
        }
        if (uiGate.timerLocked && (now - uiGate.timerLockStart > UI_REENABLE_FAILSAFE_MS)) {
            uiGate.timerLocked = false;
            BlynkWrapper::setProperty(VPIN_SET_TIMER_MIN, "isDisabled", "false");
        }
    }

    if (uiGate.schedLocked) {
        if (now - uiGate.schedLockStart > 1000) {
            uiGate.schedLocked = false;
            gBlynk.enableSchedulerApply();
        }
        if (uiGate.schedLocked && (now - uiGate.schedLockStart > UI_REENABLE_FAILSAFE_MS)) {
            uiGate.schedLocked = false;
            gBlynk.enableSchedulerApply();
        }
    }

    if (uiForceSwitchOn) {
        gBlynk.enableOnOff(true);
        uiForceSwitchOn = false;
    }
}

void StatusPublisher::timerPush() {
    pushStatus();
}
