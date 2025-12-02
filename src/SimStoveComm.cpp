#include "SimStoveComm.h"

SimStoveComm::SimStoveComm():
  _lastStateChangeMs(0),
  _startMs(0),
  _state(SIM_OFF),
  _power(1),
  _ambientBase(20.0f),
  _inShutdown(false),
  _failureMode(false),
  _tStarting(10000),
  _tLoading(10000),
  _tFire(15000)
{}

void SimStoveComm::begin(int rxPin,int txPin,int enPin){
  (void)rxPin; (void)txPin; (void)enPin;
  logInfo("[SIM] Initialized.");
}

void SimStoveComm::simulateLoop(){
  if (_failureMode){
    _state=SIM_UNDEFINED;
    return;
  }
  if (_state==SIM_OFF || _state==SIM_UNDEFINED) return;
  if (_state==SIM_CLEANING){
    if (millis()-_lastStateChangeMs>=10000){
      _state=SIM_OFF;
      _inShutdown=false;
      _lastStateChangeMs=millis();
    }
    return;
  }
  advance();
}

void SimStoveComm::advance(){
  uint32_t elapsed=millis()-_startMs;
  if (_state==SIM_STARTING && elapsed>=_tStarting){
    _state=SIM_LOADING; _lastStateChangeMs=millis();
  } else if (_state==SIM_LOADING && elapsed>=(_tStarting+_tLoading)){
    _state=SIM_FIRE; _lastStateChangeMs=millis();
  } else if (_state==SIM_FIRE && elapsed>=(_tStarting+_tLoading+_tFire)){
    _state=SIM_WORKING; _lastStateChangeMs=millis();
  }
}

int SimStoveComm::readRAM(uint8_t address, uint8_t* buffer){
  if (_failureMode){
    buffer[0]=0xFF; buffer[1]=0xFF; return 2;
  }
  if (address==RAM_ADDR_STATE){
    if (_state==SIM_OFF){
      buffer[0]=STOVE_STATE_OFF_BYTE;
      return 1;
    } else {
      buffer[0]=(uint8_t)random(0,255);
      buffer[1]=(uint8_t)_state;
      return 2;
    }
  } else if (address==RAM_ADDR_AMBIENT_TEMP){
    float amb=ambientTempCalc();
    buffer[0]=(uint8_t)constrain((int)(amb*2.0f),0,255);
    return 1;
  } else if (address==RAM_ADDR_POWER_FEEDBACK){
    buffer[0]=(uint8_t)random(0,255);
    buffer[1]=_power;
    return 2;
  }
  buffer[0]=0x11; buffer[1]=0x22;
  return 2;
}

int SimStoveComm::readEEPROM(uint8_t address, uint8_t* buffer){
  buffer[0]=address;
  buffer[1]=0xEE;
  return 2;
}

void SimStoveComm::writeRAM(uint8_t address, uint8_t data){
  if (_failureMode) return;
  if (address==RAM_ADDR_STATE && data==0x01){
    if (_state==SIM_OFF){
      _state=SIM_STARTING; _startMs=millis(); _lastStateChangeMs=millis();
      logInfo("[SIM] Start accepted.");
    }
  } else if (address==RAM_ADDR_COMMAND){
    if (data==COMMAND_POWER_PLUS && _power<5) _power++;
    else if (data==COMMAND_POWER_MINUS && _power>1) _power--;
    else if (data==COMMAND_SHUTDOWN_STEP){
      if (!_inShutdown){
        _inShutdown=true;
        _state=SIM_CLEANING;
        _lastStateChangeMs=millis();
        logInfo("[SIM] Shutdown sequence (Cleaning).");
      }
    }
  }
}

void SimStoveComm::writeEEPROM(uint8_t address,uint8_t data){
  (void)address; (void)data;
}

void SimStoveComm::forceState(uint8_t st){
  switch(st){
    case 0: _state=SIM_OFF; break;
    case 1: _state=SIM_STARTING; _startMs=millis(); break;
    case 2: _state=SIM_LOADING; break;
    case 3: _state=SIM_FIRE; break;
    case 4: _state=SIM_WORKING; break;
    case 6: _state=SIM_CLEANING; break;
    default:_state=SIM_UNDEFINED; break;
  }
  _lastStateChangeMs=millis();
  logf("[SIM] Forzado estado=%u", st);
}

void SimStoveComm::forcePower(uint8_t p){
  if (p<1) p=1; if (p>5) p=5;
  _power=p;
  logf("[SIM] Forzado potencia=%u", p);
}

void SimStoveComm::forceTempBase(int t){
  _ambientBase=(float)t;
  logf("[SIM] Base temp=%d", t);
}

void SimStoveComm::enableFailureMode(bool en){
  _failureMode=en;
  logf("[SIM] Failure mode=%d", en?1:0);
}

float SimStoveComm::ambientTempCalc() const{
  float phase=(millis()%60000)/60000.0f;
  float delta=sinf(phase*3.14159f)*2.0f;
  return _ambientBase+delta;
}