/**
 * @file TaskManager.h
 * @brief FreeRTOS task management and creation
 */

#pragma once

#include <Arduino.h>

void createAllTasks();
void taskTerminal(void* param);
void taskComm(void* param);
void taskPoll(void* param);
void taskScheduler(void* param);
