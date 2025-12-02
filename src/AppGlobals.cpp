/**
 * @file AppGlobals.cpp
 * @brief Global application instances and command queue implementation
 */

#include "AppGlobals.h"
#include "Logging.h"

#ifdef SIMULATION_MODE
SimStoveComm gComm;
#else
StoveComm gComm;
#endif

StoveController gController;
Scheduler gScheduler;
BlynkInterface gBlynk;
Terminal gTerminal;
BlynkTimer gTimer;
QueueHandle_t gCommandQueue = nullptr;

void initGlobals() {
    gCommandQueue = xQueueCreate(COMMAND_QUEUE_LEN, sizeof(Command));
    if (!gCommandQueue) {
        logInfo("[ERROR] Cola comandos no creada");
    }
}
