#include "Scheduler.h"

void Scheduler::begin(){
  _globalEnabled=true;
  _mutex=xSemaphoreCreateMutex();
  for(size_t i=0;i<MAX_SCHEDULE_ENTRIES;i++){
    _entries[i].active=false;
    _entries[i].day=WD_MON;
    _entries[i].hour=0;
    _entries[i].minute=0;
    _entries[i].targetPower=1;
  }
}

bool Scheduler::updateEntry(int idx,bool active,uint8_t day,uint8_t hour,uint8_t minute,uint8_t power){
  if (idx<0 || idx>=(int)MAX_SCHEDULE_ENTRIES) return false;
  if (day<1 || day>7) return false;
  if (hour>23 || minute>59) return false;
  if (power<1) power=1;
  if (power>5) power=5;
  if (!_mutex) return false;
  if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(200))!=pdTRUE) return false;
  _entries[idx].active=active;
  _entries[idx].day=(Weekday)day;
  _entries[idx].hour=hour;
  _entries[idx].minute=minute;
  _entries[idx].targetPower=power;
  xSemaphoreGive(_mutex);
  return true;
}

ScheduleEntry Scheduler::getEntry(size_t idx){
  if (idx>=MAX_SCHEDULE_ENTRIES) idx=MAX_SCHEDULE_ENTRIES-1;
  ScheduleEntry e=_entries[idx];
  if (!_mutex) return e;
  if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(200))!=pdTRUE) return e;
  e=_entries[idx];
  xSemaphoreGive(_mutex);
  return e;
}

void Scheduler::setGlobalEnabled(bool en){ _globalEnabled=en; }
bool Scheduler::isGlobalEnabled() const{ return _globalEnabled; }

void Scheduler::evaluate(uint8_t day,uint8_t hour,uint8_t minute,bool stoveOn,void(*startAndPower)(uint8_t)){
  if (!_globalEnabled) return;
  if (!_mutex) return;
  if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(200))!=pdTRUE) return;
  for(size_t i=0;i<MAX_SCHEDULE_ENTRIES;i++){
    const ScheduleEntry& e=_entries[i];
    if (!e.active) continue;
    if ((uint8_t)e.day!=day) continue;
    if (e.hour==hour && e.minute==minute){
      startAndPower(e.targetPower);
    }
  }
  xSemaphoreGive(_mutex);
}

String Scheduler::buildSummary(){
  String out;
  if (!_mutex) return out;
  if (xSemaphoreTake(_mutex,pdMS_TO_TICKS(200))!=pdTRUE) return out;
  out += String("Global: ") + (_globalEnabled?"ENABLED":"DISABLED") + "\n";
  for(size_t i=0;i<MAX_SCHEDULE_ENTRIES;i++){
    const auto& e=_entries[i];
    out += String("#")+i+" act="+(e.active?"1":"0")+" day="+(uint8_t)e.day+
           " "+(e.hour<10?"0":"")+e.hour+":"+(e.minute<10?"0":"")+e.minute+
           " power="+e.targetPower+"\n";
  }
  xSemaphoreGive(_mutex);
  return out;
}