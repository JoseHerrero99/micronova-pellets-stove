/**
 * @file StatusPublisher.h
 * @brief Blynk status publishing and UI state management
 */

#pragma once

#include <Arduino.h>
#include "StoveController.h"

#define TEMP_CHANGE_THRESHOLD          3.0f
#define TEMP_MIN_PUBLISH_INTERVAL_MS   10000UL
#define STATUS_MIN_PUBLISH_INTERVAL_MS 300UL

class StatusPublisher {
public:
    void publishIfChanged(const StoveStatus& s);
    void pushStatus();
    void timerPush();

private:
    struct PublishedSnapshot {
        int state = -1;
        int power = -1;
        float ambientTemp = NAN;
        bool canShutdown = false;
        uint32_t msRemainingToAllowShutdown = 0;
        uint32_t lastTempPublishMs = 0;
        uint32_t lastStatusPublishMs = 0;
        int lastRemainMin = -1;
    } mSnapshot;
};

extern StatusPublisher gStatusPublisher;
