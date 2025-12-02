/**
 * @file AppGlobals.h
 * @brief Global application instances and command queue
 * 
 * This module provides centralized access to all major application components
 * including stove communication, controller, scheduler, Blynk interface, and
 * terminal. It also defines the command structure used for inter-task communication
 * via FreeRTOS queues.
 */

#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "Config.h"
#include "IStoveComm.h"

#ifdef SIMULATION_MODE
  #include "SimStoveComm.h"
#else
  #include "StoveComm.h"
#endif

#include "StoveController.h"
#include "Scheduler.h"
#include "BlynkInterface.h"
#include "Terminal.h"

/**
 * @struct Command
 * @brief Command structure for task communication via FreeRTOS queue
 * 
 * Commands are sent between tasks to perform stove operations such as
 * starting, stopping, power adjustment, timer setting, and schedule updates.
 */
struct Command {
  /** @brief Command type enumeration */
  enum Type { START, SHUTDOWN, SET_POWER, SET_TIMER, SCHED_APPLY } type;
  
  uint8_t power;        ///< Power level (1-5) for SET_POWER command
  uint32_t minutes;     ///< Timer duration in minutes for SET_TIMER command
  size_t schedIndex;    ///< Schedule entry index for SCHED_APPLY command
  bool schedActive;     ///< Schedule active flag for SCHED_APPLY command
  uint8_t schedDay;     ///< Day of week (1-7) for SCHED_APPLY command
  uint8_t schedHour;    ///< Hour (0-23) for SCHED_APPLY command
  uint8_t schedMinute;  ///< Minute (0-59) for SCHED_APPLY command
  uint8_t schedPower;   ///< Target power level for SCHED_APPLY command
};

// ============================================================================
// GLOBAL INSTANCES
// ============================================================================

#ifdef SIMULATION_MODE
/** @brief Simulated stove communication instance (simulation mode only) */
extern SimStoveComm gComm;
#else
/** @brief Real stove communication instance via UART */
extern StoveComm gComm;
#endif

/** @brief Main stove controller instance */
extern StoveController gController;

/** @brief Schedule manager for automated stove operations */
extern Scheduler gScheduler;

/** @brief Blynk IoT interface for remote control */
extern BlynkInterface gBlynk;

#include <Blynk.h>

/** @brief Serial terminal interface for debugging and configuration */
extern Terminal gTerminal;

/** @brief Blynk timer for periodic status updates */
extern BlynkTimer gTimer;

/** @brief FreeRTOS queue handle for inter-task command passing */
extern QueueHandle_t gCommandQueue;

/**
 * @brief Initialize global instances and command queue
 * 
 * Creates the FreeRTOS command queue and initializes any global state.
 * Must be called during application setup before using global instances.
 */
void initGlobals();
