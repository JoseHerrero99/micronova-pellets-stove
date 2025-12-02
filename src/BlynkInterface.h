/**
 * @file BlynkInterface.h
 * @brief Blynk IoT platform interface for remote pellet stove control
 * 
 * This class manages the bidirectional communication between the Blynk mobile/web
 * application and the stove controller. It handles UI widget states, user interactions,
 * and scheduler configuration through virtual pins.
 */

#pragma once

#include <Arduino.h>
#include "Config.h"
#include "Logging.h"
#include "StoveController.h"
#include "Scheduler.h"

/**
 * @class BlynkInterface
 * @brief Interface layer between Blynk IoT platform and stove control system
 * 
 * Manages all Blynk virtual pin interactions, widget states, and user callbacks.
 * Provides methods to enable/disable UI elements based on stove state and handles
 * scheduler configuration through temporary storage before applying changes.
 */
class BlynkInterface {
public:
  // ========================================================================
  // Initialization
  // ========================================================================
  
  /**
   * @brief Initialize the Blynk interface with controller and scheduler references
   * @param controller Pointer to the StoveController instance
   * @param scheduler Pointer to the Scheduler instance
   */
  void begin(StoveController* controller, Scheduler* scheduler);
  
  /**
   * @brief Attach Blynk output function hooks for widget manipulation
   * @param writeFn Function to write integer values to virtual pins
   * @param propFn Function to set widget properties (e.g., enable/disable)
   * @param textFn Function to send text to virtual pins
   */
  void attachBlynkHooks(
    void(*writeFn)(uint8_t pin, int val),
    void(*propFn)(uint8_t pin, const char* prop, const char* value),
    void(*textFn)(uint8_t pin, const String& txt)
  );

  // ========================================================================
  // UI Widget State Management
  // ========================================================================
  
  /**
   * @brief Enable the on/off button and set its current state
   * @param stoveOn Current stove power state (true = on, false = off)
   */
  void enableOnOff(bool stoveOn);
  
  /**
   * @brief Disable the on/off button (during transitions)
   */
  void disableOnOff();
  
  /**
   * @brief Enable the power level slider
   * @param currentPower Current power level to display
   */
  void enablePowerSlider(uint8_t currentPower);
  
  /**
   * @brief Disable the power level slider
   */
  void disablePowerSlider();
  
  /**
   * @brief Enable the timer input field
   * @param def Default value in minutes
   */
  void enableTimerInput(uint32_t def);
  
  /**
   * @brief Disable the timer input field
   */
  void disableTimerInput();
  
  /**
   * @brief Enable the scheduler apply button
   */
  void enableSchedulerApply();
  
  /**
   * @brief Disable the scheduler apply button
   */
  void disableSchedulerApply();
  
  /**
   * @brief Push scheduler summary text to the Blynk UI
   * @param sum Summary string to display
   */
  void pushSchedulerSummary(const String& sum);

  // ========================================================================
  // User Interaction Callbacks
  // ========================================================================
  
  /**
   * @brief Set callback for on/off button press
   * @param cb Callback function receiving desired state (true = turn on)
   */
  void setOnOffCallback(void(*cb)(bool));
  
  /**
   * @brief Set callback for power level change
   * @param cb Callback function receiving desired power level (1-5)
   */
  void setPowerCallback(void(*cb)(uint8_t));
  
  /**
   * @brief Set callback for timer configuration
   * @param cb Callback function receiving timer duration in minutes
   */
  void setTimerCallback(void(*cb)(uint32_t));
  
  /**
   * @brief Set callback for global scheduler enable/disable
   * @param cb Callback function receiving enable state
   */
  void setSchedulerEnableCallback(void(*cb)(bool));
  
  /**
   * @brief Set callback for applying scheduler entry changes
   * @param cb Callback function receiving (index, active, day, hour, minute, power)
   */
  void setSchedulerApplyCallback(void(*cb)(size_t, bool, uint8_t, uint8_t, uint8_t, uint8_t));

  // ========================================================================
  // Scheduler Temporary Field Updates
  // ========================================================================
  
  /**
   * @brief Update pending scheduler entry index
   * @param idx Entry index (0 to MAX_SCHEDULE_ENTRIES-1)
   */
  void updateSchedIndex(size_t idx);
  
  /**
   * @brief Update pending entry active state
   * @param active Whether the entry should be active
   */
  void updateSchedActive(bool active);
  
  /**
   * @brief Update pending entry day of week
   * @param day Day value (1=Monday, 7=Sunday)
   */
  void updateSchedDay(uint8_t day);
  
  /**
   * @brief Update pending entry hour
   * @param hour Hour value (0-23)
   */
  void updateSchedHour(uint8_t hour);
  
  /**
   * @brief Update pending entry minute
   * @param minute Minute value (0-59)
   */
  void updateSchedMinute(uint8_t minute);
  
  /**
   * @brief Update pending entry target power
   * @param power Power level (1-5)
   */
  void updateSchedPower(uint8_t power);
  
  /**
   * @brief Reflect current pending scheduler fields to Blynk widgets
   */
  void reflectPendingSchedulerFields();

  // ========================================================================
  // Input Handlers (called from Blynk event handlers)
  // ========================================================================
  
  /**
   * @brief Handle on/off button change
   * @param val Button state (1 = turn on, 0 = turn off)
   */
  void handleOnOff(int val);
  
  /**
   * @brief Handle power slider change
   * @param p New power level (1-5)
   */
  void handleSetPower(uint8_t p);
  
  /**
   * @brief Handle timer input change
   * @param minutes Timer duration in minutes
   */
  void handleSetTimer(uint32_t minutes);
  
  /**
   * @brief Handle scheduler global enable switch
   * @param val Enable state (1 = enabled, 0 = disabled)
   */
  void handleSchedulerEnable(int val);
  
  /**
   * @brief Handle scheduler apply button press
   */
  void handleSchedulerApply();

private:
  // ========================================================================
  // Internal State
  // ========================================================================
  
  StoveController* _controller = nullptr;  ///< Reference to stove controller
  Scheduler* _scheduler = nullptr;         ///< Reference to scheduler

  // Blynk output function pointers
  void(*_writeFn)(uint8_t, int) = nullptr;                       ///< Write value to virtual pin
  void(*_propFn)(uint8_t, const char*, const char*) = nullptr;   ///< Set widget property
  void(*_textFn)(uint8_t, const String&) = nullptr;              ///< Send text to virtual pin

  // User action callback pointers
  void(*_onOffCb)(bool) = nullptr;                                              ///< On/off callback
  void(*_powerCb)(uint8_t) = nullptr;                                           ///< Power change callback
  void(*_timerCb)(uint32_t) = nullptr;                                          ///< Timer callback
  void(*_schedEnableCb)(bool) = nullptr;                                        ///< Scheduler enable callback
  void(*_schedApplyCb)(size_t, bool, uint8_t, uint8_t, uint8_t, uint8_t) = nullptr;  ///< Scheduler apply callback

  // Pending scheduler entry fields (temporary storage before applying)
  size_t  _pendingIdx = 0;        ///< Pending entry index
  bool    _pendingActive = false; ///< Pending entry active state
  uint8_t _pendingDay = 1;        ///< Pending entry day (1-7)
  uint8_t _pendingHour = 0;       ///< Pending entry hour (0-23)
  uint8_t _pendingMinute = 0;     ///< Pending entry minute (0-59)
  uint8_t _pendingPower = 1;      ///< Pending entry power level (1-5)
};