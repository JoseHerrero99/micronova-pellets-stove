/**
 * @file Application.h
 * @brief Main application initialization and lifecycle management
 * 
 * This module encapsulates the entire application lifecycle, handling
 * initialization of all subsystems (hardware, WiFi, Blynk, components,
 * and FreeRTOS tasks) and managing the main execution loop.
 */

#pragma once

#include <Arduino.h>

/**
 * @class Application
 * @brief Main application controller and lifecycle manager
 * 
 * Coordinates the startup sequence and runtime execution of all
 * application subsystems including WiFi, Blynk IoT, stove communication,
 * terminal interface, and FreeRTOS tasks.
 */
class Application {
public:
    /**
     * @brief Initialize all application subsystems
     * 
     * Performs complete system initialization in the correct order:
     * 1. Hardware (Serial)
     * 2. WiFi connection
     * 3. Application components (stove, scheduler, etc.)
     * 4. Blynk IoT platform
     * 5. FreeRTOS tasks
     */
    void initialize();
    
    /**
     * @brief Execute main application loop
     * 
     * Processes Blynk events and timer callbacks. Called repeatedly
     * from Arduino loop() function.
     */
    void run();

private:
    /**
     * @brief Initialize hardware interfaces (Serial port)
     */
    void initializeHardware();
    
    /**
     * @brief Connect to WiFi network and configure NTP time
     */
    void initializeWiFi();
    
    /**
     * @brief Initialize Blynk IoT connection and attach hooks
     */
    void initializeBlynk();
    
    /**
     * @brief Initialize application components (comm, controller, scheduler, etc.)
     */
    void initializeComponents();
    
    /**
     * @brief Create and start all FreeRTOS tasks
     */
    void initializeTasks();
    
    bool mWiFiConnected;  ///< WiFi connection status flag
};

/** @brief Global application instance */
extern Application gApp;
