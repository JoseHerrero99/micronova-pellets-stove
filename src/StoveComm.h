/**
 * @file StoveComm.h
 * @brief Hardware communication layer for Micronova pellet stove controllers
 * 
 * Implements the IStoveComm interface for real hardware communication via UART.
 * Handles the Micronova protocol including checksums, timing, and RS485 control.
 */

#pragma once

#include <Arduino.h>
#include "IStoveComm.h"
#include "Config.h"
#include "Logging.h"
#include <HardwareSerial.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

/**
 * @class StoveComm
 * @brief Hardware UART communication implementation for Micronova protocol
 * 
 * Manages low-level serial communication with Micronova-based pellet stove
 * controllers. Implements proper timing, checksum calculation, and RS485
 * half-duplex control.
 * 
 * Features:
 * - Thread-safe operations via FreeRTOS mutex
 * - Automatic RX/TX switching for RS485
 * - Checksum validation for data integrity
 * - Configurable UART pins and parameters
 */
class StoveComm : public IStoveComm {
public:
  // ========================================================================
  // Construction
  // ========================================================================
  
  /**
   * @brief Constructor - initializes internal state
   */
  StoveComm();
  
  // ========================================================================
  // IStoveComm Interface Implementation
  // ========================================================================
  
  /**
   * @brief Initialize hardware UART communication
   * @param rxPin UART receive pin number
   * @param txPin UART transmit pin number
   * @param enPin RS485 transceiver enable pin (controls TX/RX switching)
   * 
   * Configures HardwareSerial with parameters from Config.h and creates
   * the mutex for thread-safe access.
   */
  void begin(int rxPin, int txPin, int enPin) override;
  
  /**
   * @brief Read a byte from stove RAM
   * @param address RAM address to read (0x00-0xFF)
   * @param buffer Pointer to store the received byte
   * @return 1 on success, 0 on failure or timeout
   * 
   * Sends read command and waits for response with checksum validation.
   */
  int readRAM(uint8_t address, uint8_t* buffer) override;
  
  /**
   * @brief Read a byte from stove EEPROM
   * @param address EEPROM address to read (0x00-0xFF)
   * @param buffer Pointer to store the received byte
   * @return 1 on success, 0 on failure or timeout
   * 
   * Sends read command and waits for response with checksum validation.
   */
  int readEEPROM(uint8_t address, uint8_t* buffer) override;
  
  /**
   * @brief Write a byte to stove RAM
   * @param address RAM address to write (0x00-0xFF)
   * @param data Byte value to write
   * 
   * Sends write command with proper checksum. No response expected.
   */
  void writeRAM(uint8_t address, uint8_t data) override;
  
  /**
   * @brief Write a byte to stove EEPROM
   * @param address EEPROM address to write (0x00-0xFF)
   * @param data Byte value to write
   * 
   * Sends write command with proper checksum. No response expected.
   * Note: EEPROM writes may have wear limitations.
   */
  void writeEEPROM(uint8_t address, uint8_t data) override;
  
  /**
   * @brief Check if RX is currently enabled
   * @return true if receiving is enabled, false if transmitting
   */
  bool isRXEnabled() override;

private:
  // ========================================================================
  // Internal Helper Methods
  // ========================================================================
  
  /**
   * @brief Read data from stove with specific command base
   * @param cmdBase Command offset (RAM_READ or EEPROM_READ)
   * @param addr Address to read
   * @param buffer Pointer to store received byte
   * @return Number of bytes read (1 on success, 0 on failure)
   * 
   * Common implementation for RAM and EEPROM reads.
   */
  int readFromStove(uint8_t cmdBase, uint8_t addr, uint8_t* buffer);
  
  /**
   * @brief Write data to stove
   * @param location Memory type offset (RAM_WRITE or EEPROM_WRITE)
   * @param command Address to write
   * @param data Byte value to write
   * 
   * Constructs and sends write command with checksum.
   */
  void writeToStove(uint8_t location, uint8_t command, uint8_t data);
  
  /**
   * @brief Calculate Micronova protocol checksum
   * @param dest Destination address (usually 0x00)
   * @param addr Memory address
   * @param val Data value
   * @return Calculated checksum byte
   * 
   * Implements the specific checksum algorithm used by Micronova protocol.
   */
  byte calculate_checksum(uint8_t dest, uint8_t addr, uint8_t val);
  
  /**
   * @brief Enable RX mode on RS485 transceiver
   * 
   * Sets enable pin low to allow receiving data.
   */
  void enableRx();
  
  /**
   * @brief Disable RX mode (enable TX) on RS485 transceiver
   * 
   * Sets enable pin high to allow transmitting data.
   */
  void disableRx();

  // ========================================================================
  // Internal State Variables
  // ========================================================================
  
  int _rxPin;                      ///< UART RX pin number
  int _txPin;                      ///< UART TX pin number
  int _enPin;                      ///< RS485 enable pin number
  HardwareSerial* _serial;         ///< Pointer to HardwareSerial instance
  SemaphoreHandle_t _serialMutex;  ///< Mutex for thread-safe serial access
  bool rx;                         ///< Current RX enable state
};