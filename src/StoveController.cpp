#include "StoveController.h"

StoveController::StoveController():
  _comm(nullptr),
  _stateMutex(nullptr),
  _isOn(false),
  _currentState(STOVE_OFF),
  _onStartMillis(0),
  _lastStateChangeMillis(0),
  _ambientTemp(0.0f),
  _physicalPower(1),
  _powerAdjustInProgress(false),
  _autoShutdownEnabled(false),
  _autoShutdownMinutes(0),
  _autoShutdownDeadlineMs(0),
  _shutdownInProgress(false)
{}

void StoveController::begin(IStoveComm* comm){
  _comm=comm;
  _stateMutex=xSemaphoreCreateMutex();
  logInfo("StoveController ready.");
}

void StoveController::poll(){
  if (!_comm) return;
  if (_powerAdjustInProgress || _shutdownInProgress) return;
  uint8_t buf[8];
  int len=_comm->readRAM(RAM_ADDR_STATE, buf);
  StoveRunState newState=decodeState(buf,len);

  xSemaphoreTake(_stateMutex, portMAX_DELAY);
  if (newState!=_currentState){
    _currentState=newState;
    _lastStateChangeMillis=millis();
    if (_currentState==STOVE_OFF){
      _isOn=false;
      _onStartMillis=0;
      _shutdownInProgress=false;
      _autoShutdownEnabled=false;
      _autoShutdownMinutes=0;
      _autoShutdownDeadlineMs=0;
    } else if (!_isOn && _currentState!=STOVE_UNDEFINED && _currentState!=STOVE_OFF){
      _isOn=true;
      _onStartMillis=millis();
    }
  }
  xSemaphoreGive(_stateMutex);

  syncPhysicalPower();
  updateAmbientTemp();
  evaluateAutoShutdown();
}

StoveRunState StoveController::decodeState(const uint8_t* data, int len){
  if (len<=0) return STOVE_UNDEFINED;
  if (len==1){
    if (data[0]==STOVE_STATE_OFF_BYTE) return STOVE_OFF;
    return STOVE_UNDEFINED;
  }
  uint8_t s=data[len-1];
  switch(s){
    case 0x00: return STOVE_OFF;
    case 0x01: return STOVE_STARTING;
    case 0x02: return STOVE_LOADING_PELLET;
    case 0x03: return STOVE_FIRE_PRESENT;
    case 0x04: return STOVE_WORKING;
    case 0x06: return STOVE_FINAL_CLEAN;
    default:   return STOVE_UNDEFINED;
  }
}

void StoveController::startStove(){
  if (_comm) _comm->writeRAM(RAM_ADDR_STATE, 0x01);
  logInfo("Start command sent.");
}

bool StoveController::requestShutdown(){
  StoveStatus snap=getStatusSnapshot();
  if (!snap.canShutdown){
    logInfo("Shutdown denied (safety).");
    if (snap.msRemainingToAllowShutdown>0){
      if (scheduleEarliestSafeShutdown(snap.msRemainingToAllowShutdown)){
        logf("Auto-shutdown scheduled in %lu ms (≈%u min) for safe shutdown.",
             (unsigned long)snap.msRemainingToAllowShutdown,
             (unsigned)((snap.msRemainingToAllowShutdown + 59999UL)/60000UL));
      }
    }
    return false;
  }
  _shutdownInProgress=true;
  for (int i=0;i<REPEAT_TIMES_FOR_POWER_OFF;i++){
    _comm->writeRAM(RAM_ADDR_COMMAND, COMMAND_SHUTDOWN_STEP);
    vTaskDelay(pdMS_TO_TICKS(MS_FOR_POWER_OFF));
  }
  _shutdownInProgress=false;
  return true;
}

bool StoveController::scheduleEarliestSafeShutdown(uint32_t remainingMs){
  if (!_isOn) return false;
  if (remainingMs==0){
    return false;
  }
  uint32_t now=millis();
  uint32_t newDeadline = now + remainingMs;

  if (_autoShutdownEnabled){
    if (newDeadline >= _autoShutdownDeadlineMs){
      return true;
    }
  }
  _autoShutdownEnabled=true;
  _autoShutdownDeadlineMs=newDeadline;
  _autoShutdownMinutes=(remainingMs + 59999UL)/60000UL;
  return true;
}

void StoveController::setPowerLevel(uint8_t level){
  if (level<1) level=1;
  if (level>5) level=5;
  applyTargetPower(level);
}

uint8_t StoveController::getPowerLevel() const{ return _physicalPower; }
bool StoveController::isPowerAdjustInProgress() const{ return _powerAdjustInProgress; }

void StoveController::applyTargetPower(uint8_t target){
  if (_shutdownInProgress) return;
  if (_powerAdjustInProgress) return;

  Serial.printf("[applyTargetPower] target=%u, _physicalPower(before)=%u\n", target, _physicalPower);
  syncPhysicalPower();
  Serial.printf("[applyTargetPower] _physicalPower(after sync)=%u\n", _physicalPower);

  if (target == _physicalPower) return;

  _powerAdjustInProgress = true;

  if (_comm){
    if (target > _physicalPower){
      uint8_t steps = (target - _physicalPower) + 1;
      for(uint8_t i = 0; i < steps; i++){
        _comm->writeRAM(RAM_ADDR_COMMAND, COMMAND_POWER_MINUS);
        vTaskDelay(pdMS_TO_TICKS(600));
      }
    } else {
      uint8_t steps = (_physicalPower - target) +1;
      for(uint8_t i = 0; i < steps; i++){
        _comm->writeRAM(RAM_ADDR_COMMAND, COMMAND_POWER_PLUS);
        vTaskDelay(pdMS_TO_TICKS(600));
      }
    }
    vTaskDelay(pdMS_TO_TICKS(4000));
    syncPhysicalPower();
  }

  _powerAdjustInProgress = false;
}


void StoveController::updateAmbientTemp(){
  _ambientTemp=readAmbientTemperature();
}

float StoveController::readAmbientTemperature(){
  uint8_t buf[4];
  int len=_comm ? _comm->readRAM(RAM_ADDR_AMBIENT_TEMP, buf) : 0;
  if (len>=1) return (float)buf[0]/2.0f;
  return _ambientTemp;
}

void StoveController::syncPhysicalPower(){
  uint8_t buf[4];
  int len = _comm ? _comm->readRAM(RAM_ADDR_POWER_FEEDBACK, buf) : 0;

  Serial.printf("[syncPhysicalPower] len=%d, buf=", len);
  for(int i=0; i<len; i++){
    Serial.printf("%02X ", buf[i]);
  }
  Serial.println();

  if (len >= 2){
    uint8_t p = buf[1];
    if (p < 1) p = 1;
    if (p > 5) p = 5;
    _physicalPower = p;
  }
}

StoveStatus StoveController::getStatusSnapshot(){
  StoveStatus s;
  xSemaphoreTake(_stateMutex, portMAX_DELAY);
  s.state=_currentState;
  s.rawStateValue=(uint8_t)_currentState;
  s.powerLevel=_physicalPower;
  s.ambientTemp=_ambientTemp;
  s.msSinceOn=_isOn ? (millis()-_onStartMillis) : 0;
  s.msRemainingToAllowShutdown=0;
  s.canShutdown=false;
  xSemaphoreGive(_stateMutex);
  internalUpdateShutdown(s);
  return s;
}

void StoveController::internalUpdateShutdown(StoveStatus& s){
  if (!_isOn){
    s.canShutdown=false;
    s.msRemainingToAllowShutdown=0;
    return;
  }
  if (!ENFORCE_MIN_ON_TIME){
    s.canShutdown=true;
    return;
  }
  if (s.msSinceOn>=SAFETY_MIN_ON_TIME_MS){
    s.canShutdown=true;
  } else {
    s.canShutdown=false;
    s.msRemainingToAllowShutdown=SAFETY_MIN_ON_TIME_MS - s.msSinceOn;
  }
}

bool StoveController::isOn() const{ return _isOn; }

uint32_t StoveController::setAutoShutdown(uint32_t minutes){
  if (!_isOn) return 0;
  if (minutes==0) return 0;

  uint32_t now=millis();
  uint32_t elapsedMs=now - _onStartMillis;
  uint32_t safetyRemainingMs = (elapsedMs < SAFETY_MIN_ON_TIME_MS) ? (SAFETY_MIN_ON_TIME_MS - elapsedMs) : 0;

  uint32_t requestedMs = minutes*60000UL;

  if (requestedMs < safetyRemainingMs){
    requestedMs = safetyRemainingMs;
    minutes = (requestedMs + 59999UL)/60000UL;
        logf("Auto-shutdown adjusted to remaining safety minimum (%u ms ≈ %u min).",
          (unsigned)safetyRemainingMs,
          (unsigned)minutes);
  }

  if (minutes > AUTO_SHUTDOWN_MAX_MIN){
    minutes = AUTO_SHUTDOWN_MAX_MIN;
    requestedMs = minutes*60000UL;
    logf("Auto-shutdown reduced to maximum %u min.", (unsigned)minutes);
  }

  _autoShutdownEnabled=true;
  _autoShutdownMinutes=minutes;
  _autoShutdownDeadlineMs=now + requestedMs;
    logf("Auto-shutdown scheduled in %u min (deadline %lu).",
      (unsigned)minutes,(unsigned long)_autoShutdownDeadlineMs);
  return minutes;
}

void StoveController::disableAutoShutdown(){
  _autoShutdownEnabled=false;
  _autoShutdownMinutes=0;
  _autoShutdownDeadlineMs=0;
  logInfo("Auto-shutdown cancelled.");
}

bool StoveController::isAutoShutdownEnabled() const{
  return _autoShutdownEnabled;
}

uint32_t StoveController::getAutoShutdownRemainingMs() const{
  if (!_autoShutdownEnabled) return 0;
  uint32_t now=millis();
  if (now>=_autoShutdownDeadlineMs) return 0;
  return _autoShutdownDeadlineMs - now;
}

void StoveController::evaluateAutoShutdown(){
  if (!_autoShutdownEnabled) return;
  if (millis() >= _autoShutdownDeadlineMs){
    logInfo("Auto-shutdown triggered (deadline reached).");
    requestShutdown();
    _autoShutdownEnabled=false;
  }
}