/**
 * @file Scheduler.h
 * @brief Weekly schedule management for automatic stove control
 * 
 * Manages up to MAX_SCHEDULE_ENTRIES timed events that can automatically
 * start the stove and set power levels based on day of week and time.
 * Thread-safe for use with FreeRTOS tasks.
 */

#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "Config.h"

// ============================================================================
// ENUMERATIONS
// ============================================================================

/**
 * @enum Weekday
 * @brief Days of the week enumeration
 * 
 * Values align with common calendar standards (Monday = 1, Sunday = 7).
 */
enum Weekday : uint8_t {
  WD_MON = 1,  ///< Monday
  WD_TUE = 2,  ///< Tuesday
  WD_WED = 3,  ///< Wednesday
  WD_THU = 4,  ///< Thursday
  WD_FRI = 5,  ///< Friday
  WD_SAT = 6,  ///< Saturday
  WD_SUN = 7   ///< Sunday
};

// ============================================================================
// DATA STRUCTURES
// ============================================================================

/**
 * @struct ScheduleEntry
 * @brief Single schedule entry configuration
 * 
 * Represents a timed event that triggers stove power control.
 */
struct ScheduleEntry {
  bool active;          ///< Whether this entry is enabled
  Weekday day;          ///< Day of week when this entry triggers
  uint8_t hour;         ///< Hour of day (0-23)
  uint8_t minute;       ///< Minute of hour (0-59)
  uint8_t targetPower;  ///< Target power level (1-5)
};

// ============================================================================
// SCHEDULER CLASS
// ============================================================================

/**
 * @class Scheduler
 * @brief Weekly schedule manager with thread-safe operations
 * 
 * Manages a collection of schedule entries that can trigger stove operations
 * at specific times. Supports global enable/disable and per-entry activation.
 * All operations are protected by a FreeRTOS mutex for thread safety.
 */
class Scheduler {
public:
  // ========================================================================
  // Initialization
  // ========================================================================
  
  /**
   * @brief Initialize the scheduler
   * 
   * Creates the mutex for thread safety and initializes all entries to inactive.
   * Must be called before using any other methods.
   */
  void begin();
  
  // ========================================================================
  // Entry Management
  // ========================================================================
  
  /**
   * @brief Update a schedule entry
   * @param idx Entry index (0 to MAX_SCHEDULE_ENTRIES-1)
   * @param active Whether the entry is enabled
   * @param day Day of week (1-7, see Weekday enum)
   * @param hour Hour of day (0-23)
   * @param minute Minute of hour (0-59)
   * @param power Target power level (1-5)
   * @return true if update successful, false if index out of range
   * 
   * Thread-safe method to modify a schedule entry.
   */
  bool updateEntry(int idx, bool active, uint8_t day, uint8_t hour, 
                   uint8_t minute, uint8_t power);
  
  /**
   * @brief Retrieve a schedule entry
   * @param idx Entry index (0 to MAX_SCHEDULE_ENTRIES-1)
   * @return Copy of the schedule entry
   * 
   * Thread-safe method to read a schedule entry.
   * If index is out of range, returns a default-initialized entry.
   */
  ScheduleEntry getEntry(size_t idx);
  
  // ========================================================================
  // Global Control
  // ========================================================================
  
  /**
   * @brief Enable or disable the entire scheduler
   * @param en true to enable scheduling, false to disable
   * 
   * When disabled, evaluate() will not trigger any stove actions.
   */
  void setGlobalEnabled(bool en);
  
  /**
   * @brief Check if scheduler is globally enabled
   * @return true if enabled, false if disabled
   */
  bool isGlobalEnabled() const;
  
  // ========================================================================
  // Schedule Evaluation
  // ========================================================================
  
  /**
   * @brief Evaluate schedule and trigger actions if needed
   * @param day Current day of week (1-7)
   * @param hour Current hour (0-23)
   * @param minute Current minute (0-59)
   * @param stoveOn Current stove power state
   * @param startAndPower Callback function to start stove and set power
   * 
   * Checks all active entries for matches with current time.
   * If a match is found and conditions are met, calls startAndPower callback.
   * 
   * This method should be called periodically (e.g., every minute).
   * Thread-safe operation.
   */
  void evaluate(uint8_t day, uint8_t hour, uint8_t minute, bool stoveOn, 
                void(*startAndPower)(uint8_t));
  
  // ========================================================================
  // Reporting
  // ========================================================================
  
  /**
   * @brief Build a text summary of all schedule entries
   * @return Multi-line string with all active entries formatted for display
   * 
   * Format: "[Index] Day HH:MM -> Power X (Active/Inactive)"
   * Empty entries are omitted from the summary.
   * Thread-safe operation.
   */
  String buildSummary();

private:
  // ========================================================================
  // Internal State
  // ========================================================================
  
  ScheduleEntry _entries[MAX_SCHEDULE_ENTRIES];  ///< Array of schedule entries
  bool _globalEnabled;                           ///< Global scheduler enable flag
  SemaphoreHandle_t _mutex;                      ///< Mutex for thread-safe access
};