/**
 * @file BlynkHandlers.h
 * @brief Blynk event handlers and callback setup
 * 
 * This module configures the callback functions that connect Blynk UI events
 * to application logic. It sets up handlers for user interactions with virtual
 * pins (buttons, sliders, inputs) on the Blynk mobile app.
 * 
 * Note: BLYNK_CONNECTED() and BLYNK_WRITE() event handlers are defined in
 * BlynkGlobal.cpp to ensure single inclusion of BlynkSimpleEsp32.h.
 */

#pragma once

#include <Arduino.h>

/**
 * @brief Setup Blynk interface callbacks
 * 
 * Configures lambda functions that respond to user actions from the Blynk app:
 * - On/Off switch callback (start/shutdown stove)
 * - Power level slider callback (adjust power 1-5)
 * - Timer input callback (set auto-shutdown timer)
 * - Scheduler enable/disable callback
 * - Scheduler entry update callback
 * 
 * These callbacks queue commands to the command processing task for
 * thread-safe execution.
 */
void setupBlynkCallbacks();

/**
 * @brief Setup Blynk event handlers (deprecated)
 * 
 * @deprecated This function is no longer used as event handlers are
 * defined directly in BlynkGlobal.cpp using BLYNK_CONNECTED() and
 * BLYNK_WRITE() macros.
 */
void setupBlynkEventHandlers();
