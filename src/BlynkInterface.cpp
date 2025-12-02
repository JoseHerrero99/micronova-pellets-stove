#include "BlynkInterface.h"

void BlynkInterface::begin(StoveController* controller, Scheduler* scheduler){
  _controller=controller;
  _scheduler=scheduler;
}

void BlynkInterface::attachBlynkHooks(
  void(*writeFn)(uint8_t pin,int val),
  void(*propFn)(uint8_t pin,const char* prop,const char* value),
  void(*textFn)(uint8_t pin,const String& txt)
){
  _writeFn=writeFn;
  _propFn=propFn;
  _textFn=textFn;
}

void BlynkInterface::enableOnOff(bool stoveOn){
  if (_writeFn) _writeFn(VPIN_STOVE_POWER_SWITCH, stoveOn?1:0);
  if (_propFn)  _propFn(VPIN_STOVE_POWER_SWITCH,"isDisabled","false");
}
void BlynkInterface::disableOnOff(){
  if (_propFn) _propFn(VPIN_STOVE_POWER_SWITCH,"isDisabled","true");
}
void BlynkInterface::enablePowerSlider(uint8_t currentPower){
  if (_writeFn) _writeFn(VPIN_POWER_LEVEL_WRITE,currentPower);
  if (_propFn) _propFn(VPIN_POWER_LEVEL_WRITE,"isDisabled","false");
}
void BlynkInterface::disablePowerSlider(){
  if (_propFn) _propFn(VPIN_POWER_LEVEL_WRITE,"isDisabled","true");
}
void BlynkInterface::enableTimerInput(uint32_t def){
  if (_writeFn) _writeFn(VPIN_SET_TIMER_MIN, def);
  if (_propFn) _propFn(VPIN_SET_TIMER_MIN,"isDisabled","false");
}
void BlynkInterface::disableTimerInput(){
  if (_propFn) _propFn(VPIN_SET_TIMER_MIN,"isDisabled","true");
}
void BlynkInterface::enableSchedulerApply(){
  if (_propFn) _propFn(VPIN_SCHED_APPLY,"isDisabled","false");
}
void BlynkInterface::disableSchedulerApply(){
  if (_propFn) _propFn(VPIN_SCHED_APPLY,"isDisabled","true");
}
void BlynkInterface::pushSchedulerSummary(const String& sum){
  if (_textFn) _textFn(VPIN_SCHED_SUMMARY, sum);
}

void BlynkInterface::setOnOffCallback(void(*cb)(bool)){ _onOffCb=cb; }
void BlynkInterface::setPowerCallback(void(*cb)(uint8_t)){ _powerCb=cb; }
void BlynkInterface::setTimerCallback(void(*cb)(uint32_t)){ _timerCb=cb; }
void BlynkInterface::setSchedulerEnableCallback(void(*cb)(bool)){ _schedEnableCb=cb; }
void BlynkInterface::setSchedulerApplyCallback(void(*cb)(size_t,bool,uint8_t,uint8_t,uint8_t,uint8_t)){ _schedApplyCb=cb; }

void BlynkInterface::updateSchedIndex(size_t idx){ _pendingIdx=idx; }
void BlynkInterface::updateSchedActive(bool active){ _pendingActive=active; }
void BlynkInterface::updateSchedDay(uint8_t day){ _pendingDay=day; }
void BlynkInterface::updateSchedHour(uint8_t hour){ _pendingHour=hour; }
void BlynkInterface::updateSchedMinute(uint8_t minute){ _pendingMinute=minute; }
void BlynkInterface::updateSchedPower(uint8_t power){ _pendingPower=power; }

void BlynkInterface::reflectPendingSchedulerFields(){
  if (_writeFn){
    _writeFn(VPIN_SCHED_ACTIVE, _pendingActive?1:0);
    _writeFn(VPIN_SCHED_DAY, _pendingDay);
    _writeFn(VPIN_SCHED_HOUR, _pendingHour);
    _writeFn(VPIN_SCHED_MINUTE, _pendingMinute);
    _writeFn(VPIN_SCHED_POWER, _pendingPower);
  }
}

void BlynkInterface::handleOnOff(int val){
  if (_onOffCb) _onOffCb(val==1);
}

void BlynkInterface::handleSetPower(uint8_t p){
  if (_powerCb) _powerCb(p);
}

void BlynkInterface::handleSetTimer(uint32_t minutes){
  if (_timerCb) _timerCb(minutes);
}

void BlynkInterface::handleSchedulerEnable(int val){
  if (_schedEnableCb) _schedEnableCb(val==1);
}

void BlynkInterface::handleSchedulerApply(){
  if (_schedApplyCb) _schedApplyCb(_pendingIdx,_pendingActive,_pendingDay,_pendingHour,_pendingMinute,_pendingPower);
}