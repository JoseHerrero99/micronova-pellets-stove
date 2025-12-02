/**
 * @file TaskManager.cpp
 * @brief FreeRTOS task management and creation implementation
 */

#include "TaskManager.h"
#include "AppGlobals.h"
#include "UIGating.h"
#include "Config.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <time.h>

void taskTerminal(void* param) {
    while (true) {
        gTerminal.process();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void taskComm(void* param) {
    Command cmd;
    while (true) {
        if (xQueueReceive(gCommandQueue, &cmd, portMAX_DELAY) == pdTRUE) {
            switch (cmd.type) {
                case Command::START:
                    gController.startStove();
                    break;
                    
                case Command::SHUTDOWN: {
                    bool ok = gController.requestShutdown();
                    if (!ok) {
                        uiForceSwitchOn = true;
                    }
                    break;
                }
                
                case Command::SET_POWER:
                    gController.setPowerLevel(cmd.power);
                    break;
                    
                case Command::SET_TIMER:
                    if (cmd.minutes == 0) {
                        gController.disableAutoShutdown();
                    } else {
                        if (gController.isOn()) {
                            gController.setAutoShutdown(cmd.minutes);
                        }
                    }
                    break;
                    
                case Command::SCHED_APPLY:
                    gScheduler.updateEntry(cmd.schedIndex, cmd.schedActive, cmd.schedDay,
                                          cmd.schedHour, cmd.schedMinute, cmd.schedPower);
                    break;
            }
        }
    }
}

void taskPoll(void* param) {
    while (true) {
#ifdef SIMULATION_MODE
        gComm.simulateLoop();
#endif
        gController.poll();
        vTaskDelay(pdMS_TO_TICKS(gController.isOn() ? POLL_INTERVAL_ON_MS : POLL_INTERVAL_OFF_MS));
    }
}

void taskScheduler(void* param) {
    uint32_t lastMinute = 0;
    while (true) {
        uint32_t cur = millis() / 60000UL;
        if (cur != lastMinute) {
            lastMinute = cur;
            time_t now = time(nullptr);
            struct tm* info = localtime(&now);
            uint8_t day = (info->tm_wday == 0) ? 7 : (uint8_t)info->tm_wday;
            uint8_t hour = info->tm_hour;
            uint8_t minute = info->tm_min;
            
            gScheduler.evaluate(day, hour, minute, gController.isOn(),
                [](uint8_t targetPower) {
                    if (gCommandQueue) {
                        Command st{Command::START, 0, 0, 0, false, 0, 0, 0, 0};
                        xQueueSend(gCommandQueue, &st, portMAX_DELAY);
                        Command pw{Command::SET_POWER, targetPower, 0, 0, false, 0, 0, 0, 0};
                        xQueueSend(gCommandQueue, &pw, portMAX_DELAY);
                    }
                }
            );
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void createAllTasks() {
    xTaskCreatePinnedToCore(taskTerminal, "TaskTerminal", TASK_STACK_CTRL, nullptr, TASK_PRIO_CTRL, nullptr, 1);
    xTaskCreatePinnedToCore(taskComm, "TaskComm", TASK_STACK_COMM, nullptr, TASK_PRIO_COMM, nullptr, 1);
    xTaskCreatePinnedToCore(taskPoll, "TaskPoll", TASK_STACK_POLL, nullptr, TASK_PRIO_POLL, nullptr, 1);
    xTaskCreatePinnedToCore(taskScheduler, "TaskScheduler", TASK_STACK_SCHED, nullptr, TASK_PRIO_SCHED, nullptr, 1);
}
