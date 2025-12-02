/**
 * @file IStoveComm.h
 * @brief Interface for stove communication layer abstraction
 * 
 * Defines the abstract interface for communicating with Micronova pellet stoves.
 * This allows for multiple implementations (real hardware, simulation, testing mocks)
 * while maintaining a consistent API for the controller layer.
 */

#pragma once

#include <Arduino.h>

/**
 * @class IStoveComm
 * @brief Abstract interface for stove communication
 * 
 * Provides memory access methods for reading/writing RAM and EEPROM
 * on Micronova-based pellet stove controllers. Implementations handle
 * the low-level protocol details.
 */
class IStoveComm {
public:
  /**
   * @brief Virtual destructor for proper cleanup of derived classes
   */
  virtual ~IStoveComm() {}
  
  /**
   * @brief Initialize communication hardware
   * @param rxPin UART RX pin number
   * @param txPin UART TX pin number
   * @param enPin RS485 transceiver enable pin (for half-duplex control)
   */
  virtual void begin(int rxPin, int txPin, int enPin) = 0;
  
  /**
   * @brief Read a byte from stove RAM
   * @param address RAM address to read (0x00-0xFF)
   * @param buffer Pointer to buffer for storing the read byte
   * @return Number of bytes successfully read (1 on success, 0 on failure)
   */
  virtual int readRAM(uint8_t address, uint8_t* buffer) = 0;
  
  /**
   * @brief Read a byte from stove EEPROM
   * @param address EEPROM address to read (0x00-0xFF)
   * @param buffer Pointer to buffer for storing the read byte
   * @return Number of bytes successfully read (1 on success, 0 on failure)
   */
  virtual int readEEPROM(uint8_t address, uint8_t* buffer) = 0;
  
  /**
   * @brief Write a byte to stove RAM
   * @param address RAM address to write (0x00-0xFF)
   * @param data Byte value to write
   */
  virtual void writeRAM(uint8_t address, uint8_t data) = 0;
  
  /**
   * @brief Write a byte to stove EEPROM
   * @param address EEPROM address to write (0x00-0xFF)
   * @param data Byte value to write
   */
  virtual void writeEEPROM(uint8_t address, uint8_t data) = 0;
  
  /**
   * @brief Check if RX line is currently enabled
   * @return true if receiving is enabled, false otherwise
   */
  virtual bool isRXEnabled() = 0;
};