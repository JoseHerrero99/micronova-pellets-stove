/**
 * @file Terminal.h
 * @brief Interactive serial terminal for debugging and manual control
 * 
 * Provides a command-line interface over serial for direct interaction with
 * the stove controller. Supports VT100 terminal features including line editing,
 * command history, and cursor movement.
 */

#pragma once

#include <Arduino.h>
#include <vector>
#include "IStoveComm.h"
#include "StoveController.h"
#include "Scheduler.h"
#include "WiFiManager.h"
#include "Config.h"

/**
 * @class Terminal
 * @brief VT100-compatible serial terminal with command processor
 * 
 * Features:
 * - Line editing with cursor movement
 * - Command history (up/down arrows)
 * - Backspace and delete support
 * - Quiet mode for reduced output
 * - Comprehensive command set for stove control and diagnostics
 * 
 * Commands include:
 * - Status monitoring (status, temp, ram, eeprom)
 * - Control operations (on, off, power, timer)
 * - Scheduler management (sched_list, sched_set)
 * - WiFi configuration (wifi_set)
 * - Simulation controls (when SIMULATION_MODE enabled)
 */
class Terminal {
public:
  // ========================================================================
  // Initialization
  // ========================================================================
  
  /**
   * @brief Initialize terminal with required subsystems
   * @param serial Pointer to HardwareSerial for I/O
   * @param comm Pointer to stove communication interface
   * @param controller Pointer to stove controller
   * @param scheduler Pointer to scheduler instance
   * 
   * Connects terminal to all subsystems for full control access.
   */
  void begin(HardwareSerial* serial,
             IStoveComm* comm,
             StoveController* controller,
             Scheduler* scheduler);
  
  // ========================================================================
  // Main Processing
  // ========================================================================
  
  /**
   * @brief Process incoming serial data (call frequently)
   * 
   * Reads characters from serial, handles escape sequences, processes
   * line editing, and executes commands when Enter is pressed.
   * Non-blocking operation.
   */
  void process();
  
  // ========================================================================
  // State Queries
  // ========================================================================
  
  /**
   * @brief Check if user is actively typing
   * @return true if user has typed recently (within timeout period)
   * 
   * Used to suppress automatic status updates while user is typing.
   */
  bool isUserTyping() const;
  
  /**
   * @brief Mark that a command has been processed
   * 
   * Resets the command processed flag for next command.
   */
  void markCommandProcessed();
  
  /**
   * @brief Set quiet mode
   * @param q true to enable quiet mode (minimal output)
   * 
   * Quiet mode suppresses prompts and automatic status updates.
   */
  void setQuietMode(bool q);
  
  /**
   * @brief Check if quiet mode is enabled
   * @return true if quiet mode is active
   */
  bool isQuietMode() const;

private:
  // ========================================================================
  // Internal State Variables
  // ========================================================================
  
  HardwareSerial*  _serial = nullptr;      ///< Serial port for terminal I/O
  IStoveComm*      _comm = nullptr;        ///< Stove communication interface
  StoveController* _controller = nullptr;  ///< Stove controller instance
  Scheduler*       _scheduler = nullptr;   ///< Scheduler instance
  
  // Line Editing State
  String   _line;                          ///< Current input line buffer
  int      _cursorPos = 0;                 ///< Current cursor position
  uint32_t _lastKeypressMs = 0;            ///< Time of last keypress
  uint32_t _lastPromptMs = 0;              ///< Time of last prompt display
  bool     _quietMode = false;             ///< Quiet mode flag
  bool     _justProcessed = false;         ///< Command just processed flag
  
  // Command History
  static const int HISTORY_SIZE = 16;      ///< Maximum history entries
  String _history[HISTORY_SIZE];           ///< History buffer
  int    _historyCount = 0;                ///< Number of entries in history
  int    _historyIndex = -1;               ///< Current history navigation index
  
  // VT100 Escape Sequence Processing
  bool   _inEscape = false;                ///< Currently processing escape sequence
  char   _escBuf[16];                      ///< Escape sequence buffer
  int    _escLen = 0;                      ///< Length of current escape sequence
  
  // ========================================================================
  // Display Methods
  // ========================================================================
  
  /**
   * @brief Print command prompt
   */
  void printPrompt();
  
  /**
   * @brief Refresh entire line display
   * 
   * Redraws the current input line from scratch.
   */
  void fullRefresh();
  
  // ========================================================================
  // Command Processing
  // ========================================================================
  
  /**
   * @brief Process a complete command line
   * @param line Command string to execute
   */
  void handleLine(String line);
  
  /**
   * @brief Store command in history
   * @param cmd Command string to store
   */
  void storeHistory(const String& cmd);
  
  // ========================================================================
  // History Navigation
  // ========================================================================
  
  /**
   * @brief Navigate to previous command in history
   */
  void historyPrev();
  
  /**
   * @brief Navigate to next command in history
   */
  void historyNext();
  
  /**
   * @brief Replace current line with text
   * @param txt New line content
   */
  void replaceCurrentLine(const String& txt);
  
  // ========================================================================
  // Line Editing Operations
  // ========================================================================
  
  /**
   * @brief Insert character at cursor position
   * @param c Character to insert
   */
  void insertChar(char c);
  
  /**
   * @brief Delete character before cursor (backspace)
   */
  void backspaceChar();
  
  /**
   * @brief Delete character at cursor position
   */
  void deleteCharAtCursor();
  
  /**
   * @brief Move cursor left one position
   */
  void moveCursorLeft();
  
  /**
   * @brief Move cursor right one position
   */
  void moveCursorRight();
  
  /**
   * @brief Move cursor to start of line
   */
  void moveCursorHome();
  
  /**
   * @brief Move cursor to end of line
   */
  void moveCursorEnd();
  
  // ========================================================================
  // Argument Parsing
  // ========================================================================
  
  /**
   * @brief Parse arguments with quoted string support
   * @param raw Raw argument string
   * @param out Vector to receive parsed arguments
   * 
   * Handles quoted strings with spaces correctly.
   */
  void parseArgsQuoted(const String& raw, std::vector<String>& out);
  
  // ========================================================================
  // Command Implementations
  // ========================================================================
  
  void cmdHelp();                      ///< Display help text
  void cmdStatus();                    ///< Show stove status
  void cmdRam(const String& arg);      ///< Read RAM address
  void cmdEE(const String& arg);       ///< Read EEPROM address
  void cmdOn();                        ///< Turn stove on
  void cmdOff();                       ///< Turn stove off
  void cmdPower(const String& arg);    ///< Set power level
  void cmdTimer(const String& rest);   ///< Timer commands
  void timerShowStatus();              ///< Show timer status
  void timerCancel();                  ///< Cancel timer
  void cmdAutoOff();                   ///< Auto-shutdown command
  void cmdSchedList();                 ///< List schedule entries
  void cmdSchedSet(const String& rest);///< Set schedule entry
  void cmdSchedSummary();              ///< Show schedule summary
  void cmdClear();                     ///< Clear screen
  void cmdTemp();                      ///< Show temperature
  void cmdQuiet(const String& arg);    ///< Toggle quiet mode
  void cmdWifi(const String& rest);    ///< WiFi configuration
  
#ifdef SIMULATION_MODE
  // Simulation-specific commands
  void cmdSimState(const String& arg); ///< Force simulation state
  void cmdSimPower(const String& arg); ///< Force simulation power
  void cmdSimTemp(const String& arg);  ///< Force simulation temperature
  void cmdSimFail();                   ///< Enable failure mode
  void cmdSimRecover();                ///< Disable failure mode
#endif
  
  // ========================================================================
  // Parsing Utilities
  // ========================================================================
  
  /**
   * @brief Parse six integer values from string
   * @param rest Input string
   * @param out Array to receive parsed integers
   * @return true if successfully parsed 6 integers
   */
  bool parseSixInts(const String& rest, int out[6]);
};