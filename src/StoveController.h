/**
 * @file StoveController.h
 * @brief High-level pellet stove control and state management
 * 
 * Provides the main control logic for Micronova pellet stove operations including
 * power management, state monitoring, safety enforcement, and auto-shutdown features.
 * Acts as the primary interface between user commands and hardware communication.
 */

#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "IStoveComm.h"
#include "Config.h"
#include "Logging.h"

// ============================================================================
// ENUMERATIONS
// ============================================================================

/**
 * @enum StoveRunState
 * @brief Operating states of the pellet stove
 * 
 * Represents the actual physical state of the stove during operation.
 * States are based on Micronova controller state bytes.
 */
enum StoveRunState : uint8_t {
  STOVE_OFF = 0,            ///< Stove is completely off and cold
  STOVE_STARTING = 1,       ///< Ignition sequence in progress
  STOVE_LOADING_PELLET = 2, ///< Loading pellets into burn chamber
  STOVE_FIRE_PRESENT = 3,   ///< Fire detected, warming up
  STOVE_WORKING = 4,        ///< Normal operating mode
  STOVE_FINAL_CLEAN = 6,    ///< Final cleaning cycle before shutdown
  STOVE_UNDEFINED = 255     ///< Unknown or error state
};

// ============================================================================
// DATA STRUCTURES
// ============================================================================

/**
 * @struct StoveStatus
 * @brief Complete snapshot of stove operational status
 * 
 * Contains all relevant state information for monitoring and decision making.
 * Returned by getStatusSnapshot() for thread-safe status queries.
 */
struct StoveStatus {
  StoveRunState state;                    ///< Current operating state
  uint8_t rawStateValue;                  ///< Raw state byte from controller
  uint8_t powerLevel;                     ///< Current power level (1-5)
  float ambientTemp;                      ///< Ambient temperature in °C
  bool canShutdown;                       ///< Whether safe shutdown is allowed
  uint32_t msSinceOn;                     ///< Milliseconds since stove turned on
  uint32_t msRemainingToAllowShutdown;    ///< Milliseconds until safe shutdown allowed
};

// ============================================================================
// CONTROLLER CLASS
// ============================================================================

/**
 * @class StoveController
 * @brief Main controller for pellet stove operations
 * 
 * Manages all aspects of stove control including:
 * - Power on/off with safety enforcement
 * - Power level adjustment with feedback synchronization
 * - Temperature monitoring
 * - Auto-shutdown timer management
 * - State transition tracking
 * 
 * Thread-safe for use with FreeRTOS tasks via mutex protection.
 */
class StoveController {
public:
  // ========================================================================
  // Construction & Initialization
  // ========================================================================
  
  /**
   * @brief Constructor - initializes internal state
   */
  StoveController();
  
  /**
   * @brief Initialize controller with communication interface
   * @param comm Pointer to IStoveComm implementation (hardware or simulation)
   * 
   * Creates mutex for thread safety and prepares for operation.
   */
  void begin(IStoveComm* comm);
  
  // ========================================================================
  // Periodic Operations
  // ========================================================================
  
  /**
   * @brief Poll stove for current status (call periodically)
   * 
   * Reads state, temperature, and power from stove hardware.
   * Updates internal tracking and evaluates auto-shutdown conditions.
   * Should be called at regular intervals (see POLL_INTERVAL_* in Config.h).
   */
  void poll();
  
  // ========================================================================
  // Power Control
  // ========================================================================
  
  /**
   * @brief Start the stove
   * 
   * Initiates the stove startup sequence. The stove will progress through
   * STARTING -> LOADING -> FIRE -> WORKING states automatically.
   * Safe to call even if stove is already on.
   */
  void startStove();
  
  /**
   * @brief Request stove shutdown
   * @return true if shutdown initiated, false if denied due to safety constraints
   * 
   * Attempts to shut down the stove. May be denied if minimum safe on-time
   * has not elapsed (see ENFORCE_MIN_ON_TIME and SAFETY_MIN_ON_TIME_MS).
   * If allowed, sends repeated shutdown commands for reliable operation.
   */
  bool requestShutdown();
  
  /**
   * @brief Set target power level
   * @param level Desired power level (1-5, where 1=lowest, 5=highest)
   * 
   * Adjusts stove power by sending incremental POWER_PLUS/MINUS commands.
   * Power adjustment happens asynchronously during subsequent poll() calls.
   * Only effective when stove is in WORKING state.
   */
  void setPowerLevel(uint8_t level);
  
  /**
   * @brief Get current power level
   * @return Current power level (1-5)
   * 
   * Thread-safe read of current power level.
   */
  uint8_t getPowerLevel() const;
  
  // ========================================================================
  // Status Queries
  // ========================================================================
  
  /**
   * @brief Get complete status snapshot
   * @return StoveStatus structure with current state information
   * 
   * Thread-safe method to retrieve all status information at once.
   * Useful for UI updates and decision making.
   */
  StoveStatus getStatusSnapshot();
  
  /**
   * @brief Check if stove is currently on
   * @return true if stove is on (any non-OFF state), false if off
   * 
   * Thread-safe power state check.
   */
  bool isOn() const;
  
  /**
   * @brief Check if power adjustment is in progress
   * @return true if currently adjusting power level
   * 
   * Used to prevent concurrent power adjustments.
   */
  bool isPowerAdjustInProgress() const;
  
  // ========================================================================
  // Auto-Shutdown Timer
  // ========================================================================
  
  /**
   * @brief Set automatic shutdown timer
   * @param minutes Number of minutes until auto-shutdown (from now)
   * @return Actual minutes set (may be adjusted for safety), or 0 if rejected
   * 
   * Schedules automatic shutdown after specified time. Timer only counts down
   * while stove is on. If stove is off, timer is disabled.
   * Maximum duration is AUTO_SHUTDOWN_MAX_MIN from Config.h.
   * Minimum duration may be enforced to allow safe shutdown window.
   */
  uint32_t setAutoShutdown(uint32_t minutes);
  
  /**
   * @brief Disable auto-shutdown timer
   * 
   * Cancels any pending auto-shutdown.
   */
  void disableAutoShutdown();
  
  /**
   * @brief Check if auto-shutdown is currently enabled
   * @return true if auto-shutdown is scheduled, false otherwise
   */
  bool isAutoShutdownEnabled() const;
  
  /**
   * @brief Get remaining time until auto-shutdown
   * @return Milliseconds remaining, or 0 if auto-shutdown not enabled
   * 
   * Returns the actual countdown value. Only counts down while stove is on.
   */
  uint32_t getAutoShutdownRemainingMs() const;

private:
  // ========================================================================
  // Internal State Variables
  // ========================================================================
  
  IStoveComm* _comm;                   ///< Communication interface pointer
  SemaphoreHandle_t _stateMutex;       ///< Mutex for thread-safe access
  
  // Power and State Tracking
  bool _isOn;                          ///< Stove on/off state
  StoveRunState _currentState;         ///< Current operating state
  uint32_t _onStartMillis;             ///< Time when stove was turned on
  uint32_t _lastStateChangeMillis;     ///< Time of last state transition
  
  // Sensor Data
  float _ambientTemp;                  ///< Current ambient temperature
  uint8_t _physicalPower;              ///< Physical power level from stove
  
  // Power Adjustment
  bool _powerAdjustInProgress;         ///< Power adjustment active flag
  
  // Auto-Shutdown
  bool _autoShutdownEnabled;           ///< Auto-shutdown enabled flag
  uint32_t _autoShutdownMinutes;       ///< Requested auto-shutdown duration
  uint32_t _autoShutdownDeadlineMs;    ///< Absolute deadline for auto-shutdown
  bool _shutdownInProgress;            ///< Shutdown sequence active flag
  
  // ========================================================================
  // Internal Helper Methods
  // ========================================================================
  
  /**
   * @brief Decode raw state byte into StoveRunState
   * @param data Pointer to state byte buffer
   * @param len Length of data (should be 1)
   * @return Decoded StoveRunState
   */
  StoveRunState decodeState(const uint8_t* data, int len);
  
  /**
   * @brief Update ambient temperature from stove
   * 
   * Reads temperature sensor and updates internal state.
   */
  void updateAmbientTemp();
  
  /**
   * @brief Read ambient temperature from stove
   * @return Temperature in degrees Celsius
   */
  float readAmbientTemperature();
  
  /**
   * @brief Synchronize physical power level with stove
   * 
   * Reads current power level from stove hardware.
   */
  void syncPhysicalPower();
  
  /**
   * @brief Apply target power level adjustments
   * @param target Target power level (1-5)
   * 
   * Sends incremental power commands to reach target level.
   */
  void applyTargetPower(uint8_t target);
  
  /**
   * @brief Update shutdown-related status fields
   * @param s Reference to StoveStatus structure to update
   * 
   * Calculates canShutdown and time remaining fields.
   */
  void internalUpdateShutdown(StoveStatus& s);
  
  /**
   * @brief Evaluate and trigger auto-shutdown if due
   * 
   * Called by poll() to check if auto-shutdown deadline has been reached.
   */
  void evaluateAutoShutdown();
  
  /**
   * @brief Schedule earliest safe shutdown time
   * @param remainingMs Desired remaining time in milliseconds
   * @return true if scheduled successfully
   * 
   * Ensures shutdown is scheduled beyond minimum safe on-time.
   */
  bool scheduleEarliestSafeShutdown(uint32_t remainingMs);
};