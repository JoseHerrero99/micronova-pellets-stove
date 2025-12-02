/**
 * @file SimStoveComm.h
 * @brief Simulated stove communication for testing and development
 * 
 * Provides a software simulation of a Micronova pellet stove controller
 * for testing without physical hardware. Simulates realistic state transitions,
 * temperature changes, and power adjustments.
 */

#pragma once

#include <Arduino.h>
#include "IStoveComm.h"
#include "Config.h"
#include "Logging.h"

// ============================================================================
// ENUMERATIONS
// ============================================================================

/**
 * @enum SimInternalState
 * @brief Internal simulation states matching real stove behavior
 * 
 * These states simulate the actual operating modes of a Micronova stove
 * during startup, operation, and shutdown sequences.
 */
enum SimInternalState : uint8_t {
  SIM_OFF = 0,        ///< Stove is completely off
  SIM_STARTING = 1,   ///< Starting sequence initiated
  SIM_LOADING = 2,    ///< Loading pellets into burn chamber
  SIM_FIRE = 3,       ///< Fire ignition in progress
  SIM_WORKING = 4,    ///< Normal working mode
  SIM_CLEANING = 6,   ///< Final cleaning cycle
  SIM_UNDEFINED = 255 ///< Unknown/error state
};

// ============================================================================
// SIMULATION CLASS
// ============================================================================

/**
 * @class SimStoveComm
 * @brief Simulated stove communication implementation
 * 
 * Implements IStoveComm interface with software simulation of stove behavior.
 * Useful for development, testing, and demonstrations without physical hardware.
 * 
 * Features:
 * - Realistic state transition timing
 * - Temperature simulation based on power level
 * - Configurable failure mode for testing error handling
 * - Manual override capabilities for testing edge cases
 */
class SimStoveComm : public IStoveComm {
public:
  // ========================================================================
  // Construction
  // ========================================================================
  
  /**
   * @brief Constructor - initializes simulation to OFF state
   */
  SimStoveComm();
  
  // ========================================================================
  // IStoveComm Interface Implementation
  // ========================================================================
  
  /**
   * @brief Initialize simulated communication (no-op for simulation)
   * @param rxPin Ignored in simulation
   * @param txPin Ignored in simulation
   * @param enPin Ignored in simulation
   */
  void begin(int rxPin, int txPin, int enPin) override;
  
  /**
   * @brief Read simulated RAM value
   * @param address RAM address to read
   * @param buffer Pointer to receive the byte value
   * @return 1 on success, 0 on failure (simulated)
   */
  int readRAM(uint8_t address, uint8_t* buffer) override;
  
  /**
   * @brief Read simulated EEPROM value
   * @param address EEPROM address to read
   * @param buffer Pointer to receive the byte value
   * @return 1 on success, 0 on failure (simulated)
   */
  int readEEPROM(uint8_t address, uint8_t* buffer) override;
  
  /**
   * @brief Write to simulated RAM
   * @param address RAM address to write
   * @param data Byte value to write
   * 
   * Processes commands sent to RAM_ADDR_COMMAND for state control.
   */
  void writeRAM(uint8_t address, uint8_t data) override;
  
  /**
   * @brief Write to simulated EEPROM (no-op in simulation)
   * @param address EEPROM address
   * @param data Byte value
   */
  void writeEEPROM(uint8_t address, uint8_t data) override;
  
  /**
   * @brief Check if RX is enabled (always true in simulation)
   * @return true
   */
  bool isRXEnabled() override { return true; }
  
  // ========================================================================
  // Simulation Control Methods
  // ========================================================================
  
  /**
   * @brief Update simulation state (call periodically)
   * 
   * Advances the simulation through state transitions based on timing.
   * Should be called regularly (e.g., every 100-500ms) for realistic behavior.
   */
  void simulateLoop();
  
  /**
   * @brief Force simulation to a specific state (testing only)
   * @param st State byte to set (0-255)
   * 
   * Bypasses normal state transitions for testing edge cases.
   */
  void forceState(uint8_t st);
  
  /**
   * @brief Force power level (testing only)
   * @param p Power level (1-5)
   * 
   * Directly sets power level without normal command processing.
   */
  void forcePower(uint8_t p);
  
  /**
   * @brief Force base temperature (testing only)
   * @param t Temperature in degrees Celsius
   * 
   * Sets the base ambient temperature around which variations occur.
   */
  void forceTempBase(int t);
  
  /**
   * @brief Enable/disable failure simulation mode
   * @param en true to enable random failures, false for normal operation
   * 
   * When enabled, simulates communication errors and unexpected states
   * for testing error handling robustness.
   */
  void enableFailureMode(bool en);

private:
  // ========================================================================
  // Internal State Variables
  // ========================================================================
  
  uint32_t _lastStateChangeMs;  ///< Time of last state transition
  uint32_t _startMs;            ///< Time when stove was started
  SimInternalState _state;      ///< Current simulation state
  uint8_t _power;               ///< Current power level (1-5)
  float _ambientBase;           ///< Base ambient temperature
  bool _inShutdown;             ///< Shutdown sequence active flag
  bool _failureMode;            ///< Failure simulation mode flag
  
  // State transition durations (milliseconds)
  uint32_t _tStarting;  ///< Duration of STARTING state
  uint32_t _tLoading;   ///< Duration of LOADING state
  uint32_t _tFire;      ///< Duration of FIRE state
  
  // ========================================================================
  // Internal Helper Methods
  // ========================================================================
  
  /**
   * @brief Advance simulation through state transitions
   * 
   * Called by simulateLoop() to progress through startup/shutdown sequences.
   */
  void advance();
  
  /**
   * @brief Calculate simulated ambient temperature
   * @return Temperature in degrees Celsius
   * 
   * Returns temperature based on state, power level, and time running.
   */
  float ambientTempCalc() const;
};