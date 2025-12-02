/**
 * @file Logging.h
 * @brief Simple logging utilities for debugging and diagnostics
 * 
 * Provides inline logging functions that output to Serial.
 * Designed for lightweight logging during development and troubleshooting.
 */

#pragma once

#include <Arduino.h>

/**
 * @brief Log an informational message
 * @param msg Null-terminated string message to log
 * 
 * Outputs the message to Serial with a newline.
 */
inline void logInfo(const char* msg) { 
  Serial.println(msg); 
}

/**
 * @brief Log a formatted message (printf-style)
 * @param fmt Format string (printf-compatible)
 * @param ... Variable arguments matching the format specifiers
 * 
 * Formats the message using vsnprintf and outputs to Serial.
 * Buffer is limited to 160 characters.
 * 
 * Example:
 * @code
 * logf("Temperature: %d°C, Power: %d", temp, power);
 * @endcode
 */
inline void logf(const char* fmt, ...) {
  char buf[160];
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);
  Serial.println(buf);
}