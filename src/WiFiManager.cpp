/**
 * @file WiFiManager.cpp
 * @brief WiFi connection and credential management implementation
 */

#include "WiFiManager.h"
#include "Config.h"
#include "Logging.h"

WiFiManager gWiFiMgr;

void WiFiManager::begin() {
    loadCredentials();
}

void WiFiManager::loadCredentials() {
    if (!mPrefs.begin("wificfg", false)) {
        mSsid = WIFI_SSID;
        mPassword = WIFI_PASS;
        return;
    }
    
    String s = mPrefs.getString("ssid", "");
    String p = mPrefs.getString("pass", "");
    
    if (s.length() == 0) {
        mSsid = WIFI_SSID;
        mPassword = WIFI_PASS;
    } else {
        mSsid = s;
        mPassword = p;
    }
}

void WiFiManager::saveCredentials(const String& ssid, const String& pass) {
    mSsid = ssid;
    mPassword = pass;
    mPrefs.begin("wificfg", false);
    mPrefs.putString("ssid", mSsid);
    mPrefs.putString("pass", mPassword);
    logInfo("[WiFi] Credenciales guardadas en NVS.");
}

void WiFiManager::eraseCredentials() {
    mPrefs.begin("wificfg", false);
    mPrefs.clear();
    logInfo("[WiFi] Credenciales borradas de NVS.");
}

bool WiFiManager::connect() {
    logf("Conectando WiFi SSID='%s' ...", mSsid.c_str());
    WiFi.mode(WIFI_STA);
    WiFi.begin(mSsid.c_str(), mPassword.c_str());
    
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
        logInfo("WiFi OK");
        return true;
    }
    
    logInfo("WiFi FAIL");
    return false;
}
