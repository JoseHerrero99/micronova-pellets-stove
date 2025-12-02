/**
 * @file WiFiManager.h
 * @brief WiFi connection and credential management
 */

#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>

class WiFiManager {
public:
    void begin();
    bool connect();
    void saveCredentials(const String& ssid, const String& pass);
    void eraseCredentials();
    
    String getSsid() const { return mSsid; }
    String getPassword() const { return mPassword; }
    
    void setSsid(const String& ssid) { mSsid = ssid; }
    void setPassword(const String& pass) { mPassword = pass; }

private:
    void loadCredentials();
    
    Preferences mPrefs;
    String mSsid;
    String mPassword;
};

extern WiFiManager gWiFiMgr;
