/**
 * @file BlynkGlobal.h
 * @brief Blynk wrapper functions and centralized Blynk integration
 * 
 * This module provides a wrapper interface to the Blynk library to avoid
 * multiple inclusion issues. BlynkSimpleEsp32.h is included only in the
 * corresponding .cpp file, and all Blynk event handlers (BLYNK_CONNECTED,
 * BLYNK_WRITE) are also defined there to ensure a single Blynk instance.
 * 
 * Other modules should use BlynkWrapper:: functions instead of directly
 * accessing the Blynk object to maintain proper encapsulation.
 */

#pragma once

#include <Arduino.h>

/**
 * @namespace BlynkWrapper
 * @brief Wrapper functions for Blynk IoT platform integration
 * 
 * Provides a clean interface to Blynk functionality without exposing
 * the underlying Blynk library includes to multiple compilation units.
 */
namespace BlynkWrapper {
    /**
     * @brief Write an integer value to a virtual pin
     * @param pin Virtual pin number
     * @param value Integer value to write
     */
    void virtualWrite(uint8_t pin, int value);
    
    /**
     * @brief Write a string value to a virtual pin
     * @param pin Virtual pin number
     * @param value String value to write
     */
    void virtualWrite(uint8_t pin, const String& value);
    
    /**
     * @brief Set a property of a virtual pin widget
     * @param pin Virtual pin number
     * @param property Property name (e.g., "color", "label", "isDisabled")
     * @param value Property value
     */
    void setProperty(uint8_t pin, const char* property, const char* value);
    
    /**
     * @brief Synchronize multiple virtual pins with the server
     * @param pin1 First virtual pin to sync
     * @param pin2 Second virtual pin to sync
     * @param pin3 Third virtual pin to sync
     * @param pin4 Fourth virtual pin to sync
     * @param pin5 Fifth virtual pin to sync
     */
    void syncVirtual(uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, uint8_t pin5);
    
    /**
     * @brief Initialize Blynk connection
     * @param auth Blynk authentication token
     * @param ssid WiFi SSID
     * @param pass WiFi password
     */
    void begin(const char* auth, const char* ssid, const char* pass);
    
    /**
     * @brief Check if Blynk is connected to the server
     * @return true if connected, false otherwise
     */
    bool connected();
    
    /**
     * @brief Process Blynk events (must be called in loop)
     */
    void run();
}
