/**
 * @file Application.cpp
 * @brief Main application initialization and lifecycle management implementation
 */

#include "Application.h"
#include "AppGlobals.h"
#include "WiFiManager.h"
#include "UIGating.h"
#include "StatusPublisher.h"
#include "TaskManager.h"
#include "BlynkHandlers.h"
#include "BlynkGlobal.h"
#include "Config.h"
#include "Logging.h"
#include <WiFi.h>
#include <time.h>

Application gApp;

void Application::initialize() {
#ifdef SIMULATION_MODE
    logInfo("Arrancando (SIMULATION MODE)");
#else
    logInfo("Arrancando (REAL MODE)");
#endif

    initializeHardware();
    initializeWiFi();
    initializeComponents();
    initializeBlynk();
    initializeTasks();

    logInfo("Setup completo.");
}

void Application::initializeHardware() {
    Serial.begin(115200);
}

void Application::initializeWiFi() {
    gWiFiMgr.begin();
    mWiFiConnected = gWiFiMgr.connect();
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
}

void Application::initializeComponents() {
    initGlobals();
    initUIGating();
    
    gComm.begin(HW_RX_PIN_DEFAULT, HW_TX_PIN_DEFAULT, HW_EN_RX_PIN_DEFAULT);
    gController.begin(&gComm);
    gController.poll();
    gScheduler.begin();
    gBlynk.begin(&gController, &gScheduler);
    
    setupBlynkCallbacks();
}

void Application::initializeBlynk() {
    if (mWiFiConnected) {
        BlynkWrapper::begin(BLYNK_AUTH_TOKEN, gWiFiMgr.getSsid().c_str(), gWiFiMgr.getPassword().c_str());
        
        if (BlynkWrapper::connected()) {
            gBlynk.attachBlynkHooks(
                [](uint8_t pin, int val) { BlynkWrapper::virtualWrite(pin, val); },
                [](uint8_t pin, const char* prop, const char* value) { BlynkWrapper::setProperty(pin, prop, value); },
                [](uint8_t pin, const String& txt) { BlynkWrapper::virtualWrite(pin, txt); }
            );
            
            gBlynk.enableOnOff(gController.isOn());
            gBlynk.enablePowerSlider(gController.getPowerLevel());
            gBlynk.enableSchedulerApply();
            gBlynk.pushSchedulerSummary(gScheduler.buildSummary());
        } else {
            logInfo("Blynk no conectado.");
        }
    } else {
        logInfo("Sin WiFi: Blynk omitido.");
    }
}

void Application::initializeTasks() {
    gTerminal.begin(&Serial, &gComm, &gController, &gScheduler);
    createAllTasks();
    gTimer.setInterval(2000L, []() { gStatusPublisher.timerPush(); });
}

void Application::run() {
    if (WiFi.status() == WL_CONNECTED) {
        BlynkWrapper::run();
    }
    gTimer.run();
}
