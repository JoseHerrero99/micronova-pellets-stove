/**
 * @file Config.h
 * @brief Global configuration parameters for Micronova pellet stove control system
 * 
 * This file contains all configuration constants including:
 * - WiFi and Blynk credentials
 * - Virtual pin assignments
 * - Hardware UART configuration
 * - Protocol addresses and commands
 * - Timing parameters
 * - Safety thresholds
 * - FreeRTOS task configuration
 */

#pragma once

#include <Arduino.h>
#include <Blynk/BlynkHandlers.h>

// ============================================================================
// BLYNK CONFIGURATION
// ============================================================================
/** @brief Blynk template ID for this device */
#define BLYNK_TEMPLATE_ID   "Template ID here"

/** @brief Blynk template name */
#define BLYNK_TEMPLATE_NAME "Template Name here"

/** @brief Blynk authentication token */
#define BLYNK_AUTH_TOKEN    "Auth Token here"

// ============================================================================
// WIFI CREDENTIALS
// ============================================================================

/** @brief WiFi network SSID */
static const char* WIFI_SSID = "YourNetworkSSID";

/** @brief WiFi network password */
static const char* WIFI_PASS = "YourNetworkPassword";

// ============================================================================
// BLYNK VIRTUAL PIN ASSIGNMENTS
// ============================================================================

// Control Pins
#define VPIN_STOVE_POWER_SWITCH    V4   ///< Main on/off switch
#define VPIN_POWER_LEVEL_WRITE     V3   ///< Power level write control
#define VPIN_POWER_LEVEL_READ      V2   ///< Power level read feedback
#define VPIN_SET_TIMER_MIN         V6   ///< Timer input (minutes)
#define VPIN_AUTO_SHUTDOWN_REMAIN  V20  ///< Auto-shutdown remaining time display

// Status Display Pins
#define VPIN_STOVE_STATE_NUM       V0   ///< Stove state numeric value
#define VPIN_STOVE_STATE_STRING    V7   ///< Stove state text description
#define VPIN_AMBIENT_TEMP          V1   ///< Ambient temperature reading
#define VPIN_TIME_TO_SAFE_OFF      V8   ///< Time remaining until safe shutdown allowed
#define VPIN_SAFETY_MIN_TIME       V9   ///< Minimum safe on-time display

// Scheduler Pins
#define VPIN_SCHED_GLOBAL_ENABLE   V10  ///< Global scheduler enable/disable switch
#define VPIN_SCHED_INDEX           V11  ///< Scheduler entry index selector
#define VPIN_SCHED_ACTIVE          V12  ///< Current entry active checkbox
#define VPIN_SCHED_DAY             V13  ///< Day of week selector (1-7)
#define VPIN_SCHED_HOUR            V14  ///< Hour selector (0-23)
#define VPIN_SCHED_MINUTE          V15  ///< Minute selector (0-59)
#define VPIN_SCHED_POWER           V16  ///< Target power level (1-5)
#define VPIN_SCHED_APPLY           V17  ///< Apply button for scheduler changes
#define VPIN_SCHED_REFRESH         V19  ///< Refresh scheduler display
#define VPIN_SCHED_SUMMARY         V18  ///< Scheduler summary text display

// ============================================================================
// HARDWARE UART CONFIGURATION
// ============================================================================

/** @brief RX pin for stove serial communication */
#define HW_RX_PIN_DEFAULT    33

/** @brief TX pin for stove serial communication */
#define HW_TX_PIN_DEFAULT    32

/** @brief Enable RX pin (RS485 transceiver control) */
#define HW_EN_RX_PIN_DEFAULT 27

/** @brief Serial baud rate for stove communication */
#define STOVE_SERIAL_BAUD    1200

/** @brief Serial configuration: 8 data bits, no parity, 2 stop bits */
#define STOVE_SERIAL_CONFIG  SERIAL_8N2

// ============================================================================
// MICRONOVA PROTOCOL - MEMORY ACCESS OFFSETS
// ============================================================================

/** @brief Command offset for reading RAM addresses */
#define STOVE_OFFSET_RAM_READ 0x00

/** @brief Command offset for reading EEPROM addresses */
#define STOVE_OFFSET_EEPROM_READ 0x20

/** @brief Command offset for writing RAM addresses */
#define STOVE_OFFSET_RAM_WRITE 0x80

/** @brief Command offset for writing EEPROM addresses */
#define STOVE_OFFSET_EEPROM_WRITE 0xA0

// ============================================================================
// MICRONOVA PROTOCOL - RAM ADDRESSES AND COMMANDS
// ============================================================================
//
// IMPORTANT: These addresses are specific to this stove model and may differ
// from other Micronova implementations (e.g., philibertc/micronova_controller).
//
// HOW THIS STOVE WORKS:
// ---------------------
// 1. Commands are sent by writing to RAM address 0x58 (RAM_ADDR_COMMAND)
// 2. Power level feedback is read from RAM address 0xB9 (RAM_ADDR_POWER_FEEDBACK)
// 3. State is read from RAM address 0x21 (RAM_ADDR_STATE)
// 4. Temperature is read from RAM address 0x01 (RAM_ADDR_AMBIENT_TEMP)
//
// TESTED COMMAND VALUES (written to address 0x58):
// -------------------------------------------------
// 0x54 - Power +        (Increase power level)
// 0x50 - Power -        (Decrease power level)
// 0x52 - Temperature +  (Increase set temperature - not used in this implementation)
// 0x58 - Temperature -  (Decrease set temperature - not used in this implementation)
// 0x5A - Power ON/OFF   (Toggle stove on/off state)
//
// DIFFERENCES FROM ORIGINAL PROJECT (philibertc/micronova_controller):
// --------------------------------------------------------------------
// - Original uses RAM address 0x34 for power feedback
// - This stove uses RAM address 0xB9 for power feedback
// - Command address (0x58) remains the same
// - Command codes (0x54, 0x50, 0x5A) are identical
//
// NOTE: If you have a different Micronova stove model, you may need to adjust:
// - RAM_ADDR_POWER_FEEDBACK (try 0x34 if 0xB9 doesn't work)
// - RAM_ADDR_STATE (try different addresses if 0x21 doesn't work)
// - Use the terminal commands "ram <addr>" and "ee <addr>" to discover your values
//
// ============================================================================

/** @brief RAM address containing stove state byte */
#define RAM_ADDR_STATE          0x21

/** @brief RAM address for ambient temperature reading */
#define RAM_ADDR_AMBIENT_TEMP   0x01

/** 
 * @brief RAM address for power level feedback
 * 
 * IMPORTANT: This is 0xB9 for this specific stove model.
 * Other Micronova stoves may use 0x34 (original project) or other addresses.
 * The power level returned is typically 1-5 representing the current flame intensity.
 */
#define RAM_ADDR_POWER_FEEDBACK 0xB9

/** 
 * @brief RAM address for sending commands to the stove
 * 
 * Commands are written to this address to control the stove.
 * This address appears to be standard across Micronova controllers.
 */
#define RAM_ADDR_COMMAND        0x58

// ============================================================================
// CONTROL COMMAND BYTES
// ============================================================================
// These command bytes are written to RAM_ADDR_COMMAND (0x58) to control the stove

/** 
 * @brief Increase power level command
 * 
 * Write 0x54 to address 0x58 to increase power by one level (max 5).
 * The stove will acknowledge by updating RAM_ADDR_POWER_FEEDBACK (0xB9).
 */
#define COMMAND_POWER_PLUS      0x54

/** 
 * @brief Decrease power level command
 * 
 * Write 0x50 to address 0x58 to decrease power by one level (min 1).
 * The stove will acknowledge by updating RAM_ADDR_POWER_FEEDBACK (0xB9).
 */
#define COMMAND_POWER_MINUS     0x50

/** 
 * @brief Shutdown/Power ON-OFF toggle command
 * 
 * Write 0x5A to address 0x58 to toggle stove power state.
 * For reliable shutdown, this command should be sent multiple times
 * (see REPEAT_TIMES_FOR_POWER_OFF).
 */
#define COMMAND_SHUTDOWN_STEP   0x5A

// Temperature adjustment commands (available but not used in this implementation)
// #define COMMAND_TEMP_PLUS    0x52  ///< Increase set temperature
// #define COMMAND_TEMP_MINUS   0x58  ///< Decrease set temperature

/** @brief Stove state byte value when fully off */
#define STOVE_STATE_OFF_BYTE    0x21

// ============================================================================
// SAFETY AND TIMING PARAMETERS
// ============================================================================

/** @brief Minimum safe on-time before allowing shutdown (10 minutes in milliseconds) */
#define SAFETY_MIN_ON_TIME_MS        (10UL * 60UL * 1000UL)

/** @brief Enable enforcement of minimum on-time (1 = enabled, 0 = disabled) */
#define ENFORCE_MIN_ON_TIME          1

/** @brief Enable auto-shutdown feature (0 = disabled by default) */
#define AUTO_SHUTDOWN_ENABLED        0

/** @brief Default auto-shutdown timer duration (minutes) */
#define AUTO_SHUTDOWN_DEFAULT_MIN    60

/** @brief Maximum auto-shutdown timer duration (minutes) */
#define AUTO_SHUTDOWN_MAX_MIN        480

// ============================================================================
// POLLING INTERVALS
// ============================================================================

/** @brief Polling interval when stove is off (milliseconds) */
#define POLL_INTERVAL_OFF_MS    6000

/** @brief Polling interval when stove is on (milliseconds) */
#define POLL_INTERVAL_ON_MS     6000

// ============================================================================
// POWER ADJUSTMENT PARAMETERS
// ============================================================================

/** @brief Delay between power adjustment steps (milliseconds) */
#define POWER_STEP_DELAY_MS     3000

/** @brief Timeout for power adjustment completion (milliseconds) */
#define POWER_ADJUST_TIMEOUT_MS 8000

// ============================================================================
// SHUTDOWN PROCEDURE PARAMETERS
// ============================================================================

/** @brief Number of shutdown command repetitions for safe power-off */
#define REPEAT_TIMES_FOR_POWER_OFF  22

/** @brief Delay between repeated shutdown commands (milliseconds) */
#define MS_FOR_POWER_OFF            100

// ============================================================================
// SCHEDULER CONFIGURATION
// ============================================================================

/** @brief Maximum number of schedule entries */
#define MAX_SCHEDULE_ENTRIES 8

// ============================================================================
// FREERTOS TASK CONFIGURATION
// ============================================================================

/** @brief Command queue length for inter-task communication */
#define COMMAND_QUEUE_LEN 16

// Task Stack Sizes (bytes)
#define TASK_STACK_COMM     4096  ///< Communication task stack size
#define TASK_STACK_POLL     4096  ///< Polling task stack size
#define TASK_STACK_SCHED    4096  ///< Scheduler task stack size
#define TASK_STACK_CTRL     4096  ///< Controller task stack size

// Task Priorities (higher number = higher priority)
#define TASK_PRIO_COMM      3  ///< Communication task priority
#define TASK_PRIO_POLL      2  ///< Polling task priority
#define TASK_PRIO_SCHED     2  ///< Scheduler task priority
#define TASK_PRIO_CTRL      1  ///< Controller task priority

// ============================================================================
// STATE TRANSITION TIMEOUTS
// ============================================================================

/** @brief Maximum time to wait for stove start confirmation (milliseconds) */
#define STOVE_START_CONFIRM_TIMEOUT_MS    15000

/** @brief Maximum time to wait for stove shutdown confirmation (milliseconds) */
#define STOVE_SHUTDOWN_CONFIRM_TIMEOUT_MS 20000

/** @brief Failsafe timeout for re-enabling UI after state change (milliseconds) */
#define UI_REENABLE_FAILSAFE_MS           30000

// ============================================================================
// SIMULATION MODE
// ============================================================================

/**
 * @def SIMULATION_MODE
 * @brief Enable simulation mode for testing without physical stove
 * 
 * Define this in platformio.ini build flags: -DSIMULATION_MODE
 * When enabled, uses SimStoveComm instead of real hardware communication
 */