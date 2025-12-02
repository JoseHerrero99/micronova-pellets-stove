#include "Terminal.h"
#ifdef SIMULATION_MODE
  #include "SimStoveComm.h"
#endif
#include <WiFi.h>

// Extern WiFi vars / funcs
extern String gWifiSsid;
extern String gWifiPass;
extern bool wifiConnect();
extern void wifiSave();
extern void wifiErase();

static const uint32_t PROMPT_MIN_INTERVAL_MS = 120;

void Terminal::printPrompt(){
  _serial->print("\r\n> ");
  _lastPromptMs=millis();
  _cursorPos=_line.length();
}

void Terminal::fullRefresh(){
  _serial->print("\r");
  _serial->print("\033[K");
  _serial->print("> ");
  _serial->print(_line);
  int tail=_line.length()-_cursorPos;
  if (tail>0) _serial->printf("\033[%dD", tail);
}

void Terminal::begin(HardwareSerial* serial,
                     IStoveComm* comm,
                     StoveController* controller,
                     Scheduler* scheduler){
  _serial    = serial;
  _comm      = comm;
  _controller= controller;
  _scheduler = scheduler;
  _line.reserve(128);
  _serial->print("\r\n[Terminal] Ready. Type 'help'.");
  printPrompt();
  _lastKeypressMs=millis();
}

void Terminal::process(){
  if (!_serial) return;
  while (_serial->available()){
    char c=(char)_serial->read();

    if (_inEscape){
      if (_escLen < (int)sizeof(_escBuf)-1){
        _escBuf[_escLen++]=c;
        _escBuf[_escLen]=0;
      }
      if (_escBuf[0]=='[' || _escBuf[0]=='O'){
        if (_escLen==2){
          char code=_escBuf[1];
          if (code=='A'){ historyPrev(); _inEscape=false; _escLen=0; continue; }
          if (code=='B'){ historyNext(); _inEscape=false; _escLen=0; continue; }
          if (code=='C'){ moveCursorRight(); _inEscape=false; _escLen=0; continue; }
          if (code=='D'){ moveCursorLeft();  _inEscape=false; _escLen=0; continue; }
          if (code=='H'){ moveCursorHome();  _inEscape=false; _escLen=0; continue; }
          if (code=='F'){ moveCursorEnd();   _inEscape=false; _escLen=0; continue; }
        }
        if (_escLen==4 && _escBuf[1]=='3' && _escBuf[2]=='~'){
          deleteCharAtCursor();
          _inEscape=false; _escLen=0; continue;
        }
      }
      if (_escLen>=6){
        _inEscape=false; _escLen=0;
      }
      continue;
    }

    if (c=='\x1B'){
      _inEscape=true;
      _escLen=0;
      continue;
    }

    if (c=='\r'){
      if (_serial->available()){
        char nxt=(char)_serial->peek();
        if (nxt=='\n') _serial->read();
      }
      c='\n';
    }

    if (c=='\n'){
      if (_line.length()>0){
        String line=_line;
        _line="";
        _cursorPos=0;
        line.trim();
        if (line.length()){
          _serial->print("\r\n");
          handleLine(line);
          storeHistory(line);
          markCommandProcessed();
        }
        printPrompt();
      } else {
        if (millis()-_lastPromptMs > PROMPT_MIN_INTERVAL_MS) printPrompt();
      }
      _historyIndex=-1;
      continue;
    }

    if (c==0x08 || c==0x7F){
      if (c==0x7F && _cursorPos < _line.length()){
        deleteCharAtCursor();
      } else {
        backspaceChar();
      }
      continue;
    }

    if (c>=32 && c!=127){
      insertChar(c);
      continue;
    }
  }
}

bool Terminal::isUserTyping() const{
  if (_quietMode) return true;
  if (_line.length()==0) return false;
  return (millis()-_lastKeypressMs) < 2500;
}

void Terminal::markCommandProcessed(){
  _justProcessed=true;
  _lastKeypressMs=millis();
}

void Terminal::setQuietMode(bool q){
  _quietMode=q;
  _serial->print(q ? "\r\n[Terminal] Quiet ON." : "\r\n[Terminal] Quiet OFF.");
  printPrompt();
}

bool Terminal::isQuietMode() const{ return _quietMode; }

void Terminal::storeHistory(const String& cmd){
  if (cmd.isEmpty()) return;
  if (_historyCount>0 && _history[_historyCount-1]==cmd) return;
  if (_historyCount < HISTORY_SIZE){
    _history[_historyCount++]=cmd;
  } else {
    for(int i=1;i<HISTORY_SIZE;i++) _history[i-1]=_history[i];
    _history[HISTORY_SIZE-1]=cmd;
  }
}

void Terminal::historyPrev(){
  if (_historyCount==0) return;
  if (_historyIndex<0) _historyIndex=_historyCount-1;
  else if (_historyIndex>0) _historyIndex--;
  replaceCurrentLine(_history[_historyIndex]);
}

void Terminal::historyNext(){
  if (_historyCount==0) return;
  if (_historyIndex<0) return;
  if (_historyIndex < _historyCount-1){
    _historyIndex++;
    replaceCurrentLine(_history[_historyIndex]);
  } else {
    _historyIndex=-1;
    replaceCurrentLine("");
  }
}

void Terminal::replaceCurrentLine(const String& txt){
  _line=txt;
  _cursorPos=_line.length();
  fullRefresh();
}

void Terminal::insertChar(char c){
  if (_line.length()>=512) return;
  if (_cursorPos==_line.length()){
    _line += c;
    _serial->write(c);
  } else {
    String tail=_line.substring(_cursorPos);
    _line=_line.substring(0,_cursorPos)+c+tail;
    _serial->write(c);
    _serial->print(tail);
    if (tail.length()>0)
      _serial->printf("\033[%dD", tail.length());
  }
  _cursorPos++;
  _lastKeypressMs=millis();
}

void Terminal::backspaceChar(){
  if (_cursorPos==0 || _line.length()==0) return;
  String tail=_line.substring(_cursorPos);
  _line=_line.substring(0,_cursorPos-1)+tail;
  _cursorPos--;
  _serial->print("\033[1D");
  _serial->print(tail);
  _serial->print(" ");
  int back=tail.length()+1;
  if (back>0) _serial->printf("\033[%dD", back);
  _lastKeypressMs=millis();
}

void Terminal::deleteCharAtCursor(){
  if (_cursorPos >= _line.length()) return;
  String tail=_line.substring(_cursorPos+1);
  _line=_line.substring(0,_cursorPos)+tail;
  _serial->print(tail);
  _serial->print(" ");
  int back=tail.length()+1;
  if (back>0) _serial->printf("\033[%dD", back);
  _lastKeypressMs=millis();
}

void Terminal::moveCursorLeft(){
  if (_cursorPos>0){
    _cursorPos--;
    _serial->print("\033[1D");
  }
}

void Terminal::moveCursorRight(){
  if (_cursorPos < _line.length()){
    _cursorPos++;
    _serial->print("\033[1C");
  }
}

void Terminal::moveCursorHome(){
  if (_cursorPos>0){
    _serial->printf("\033[%dD", _cursorPos);
    _cursorPos=0;
  }
}

void Terminal::moveCursorEnd(){
  int diff=_line.length()-_cursorPos;
  if (diff>0){
    _serial->printf("\033[%dC", diff);
    _cursorPos=_line.length();
  }
}

void Terminal::parseArgsQuoted(const String& raw, std::vector<String>& out){
  out.clear();
  bool inQuote=false;
  String cur;
  for (size_t i=0;i<raw.length();++i){
    char c=raw[i];
    if (c=='\"'){ inQuote=!inQuote; continue; }
    if (isspace((unsigned char)c) && !inQuote){
      if (cur.length()){ out.push_back(cur); cur=""; }
    } else cur+=c;
  }
  if (cur.length()) out.push_back(cur);
}

void Terminal::handleLine(String line){
  int sp=line.indexOf(' ');
  String cmd=(sp<0)? line : line.substring(0,sp);
  String rest=(sp<0)? "" : line.substring(sp+1);
  cmd.toLowerCase(); rest.trim();

  if (cmd=="help") cmdHelp();
  else if (cmd=="status") cmdStatus();
  else if (cmd=="ram") cmdRam(rest);
  else if (cmd=="eeprom") cmdEE(rest);
  else if (cmd=="on") cmdOn();
  else if (cmd=="off") cmdOff();
  else if (cmd=="power") cmdPower(rest);
  else if (cmd=="timer") cmdTimer(rest);
  else if (cmd=="auto" && rest=="off") cmdAutoOff();
  else if (cmd=="sched"){
    if (rest.startsWith("list")) cmdSchedList();
    else if (rest.startsWith("summary")) cmdSchedSummary();
    else if (rest.startsWith("set")){
      String sub=rest.substring(3); sub.trim();
      cmdSchedSet(sub);
    } else _serial->print("\r\nUsage: sched list | sched summary | sched set <idx> <active> <day> <hour> <min> <power>");
  }
  else if (cmd=="clear") cmdClear();
  else if (cmd=="temp") cmdTemp();
  else if (cmd=="quiet") cmdQuiet(rest);
  else if (cmd=="wifi") cmdWifi(rest);
  else if (cmd=="reboot"){
    _serial->print("\r\nReinicio...");
    delay(150);
    ESP.restart();
  }
#ifdef SIMULATION_MODE
  else if (cmd=="simstate") cmdSimState(rest);
  else if (cmd=="simpower") cmdSimPower(rest);
  else if (cmd=="simtemp")  cmdSimTemp(rest);
  else if (cmd=="simfail")  cmdSimFail();
  else if (cmd=="simrecover") cmdSimRecover();
#endif
  else _serial->print("\r\nUnknown. Type 'help'.");
}

void Terminal::cmdHelp(){
  _serial->print("\r\nCommands:");
  _serial->print("\r\n  help");
  _serial->print("\r\n  status");
  _serial->print("\r\n  ram <addr>");
  _serial->print("\r\n  eeprom <addr>");
  _serial->print("\r\n  on / off");
  _serial->print("\r\n  power <1..5>");
  _serial->print("\r\n  timer <min> | timer status | timer cancel");
  _serial->print("\r\n  auto off");
  _serial->print("\r\n  temp");
  _serial->print("\r\n  sched list | sched summary | sched set i act day hour min power");
  _serial->print("\r\n  wifi show | set \"SSID con espacios\" \"PASS opcional\" | reconnect | save | erase");
  _serial->print("\r\n  reboot");
  _serial->print("\r\n  quiet <on|off>");
#ifdef SIMULATION_MODE
  _serial->print("\r\n  simstate/simpower/simtemp/simfail/simrecover");
#endif
}

void Terminal::cmdStatus(){
  StoveStatus s=_controller->getStatusSnapshot();
  _serial->print("\r\n---- Status ----");
  _serial->printf("\r\nState=%u", s.state);
  _serial->printf("\r\nAmbient=%.2f C", s.ambientTemp);
  _serial->printf("\r\nPower=%u", s.powerLevel);
  _serial->printf("\r\nmsSinceOn=%lu", (unsigned long)s.msSinceOn);
  _serial->printf("\r\nCanShutdown=%s", s.canShutdown?"YES":"NO");
  _serial->printf("\r\nRemainToAllow(ms)=%lu", (unsigned long)s.msRemainingToAllowShutdown);
  timerShowStatus();
}

void Terminal::cmdRam(const String& arg){
  if (arg.isEmpty()){ _serial->print("\r\nUsage: ram <addr>"); return; }
  uint8_t addr=(uint8_t)strtol(arg.c_str(), nullptr, 0);
  uint8_t buf[64]; int len=_comm->readRAM(addr, buf);
  _serial->printf("\r\nRAM 0x%02X len=%d", addr, len);
  for(int i=0;i<len;i++) _serial->printf("\r\n [%d]=0x%02X", i, buf[i]);
}

void Terminal::cmdEE(const String& arg){
  if (arg.isEmpty()){ _serial->print("\r\nUsage: eeprom <addr>"); return; }
  uint8_t addr=(uint8_t)strtol(arg.c_str(), nullptr, 0);
  uint8_t buf[16]; int len=_comm->readEEPROM(addr, buf);
  _serial->printf("\r\nEEPROM 0x%02X len=%d", addr, len);
  for(int i=0;i<len;i++) _serial->printf("\r\n [%d]=0x%02X", i, buf[i]);
}

void Terminal::cmdOn(){ _serial->print("\r\nStart request."); _controller->startStove(); }
void Terminal::cmdOff(){
  if (!_controller->requestShutdown()) _serial->print("\r\nShutdown refused (safety).");
  else _serial->print("\r\nShutdown sequence initiated.");
}

void Terminal::cmdPower(const String& arg){
  if (arg.isEmpty()){ _serial->print("\r\nUsage: power <1..5>"); return; }
  uint8_t p=(uint8_t)arg.toInt();
  _controller->setPowerLevel(p);
  _serial->printf("\r\nPower target=%u", p);
}

void Terminal::cmdTimer(const String& rest){
  if (rest.isEmpty()){
    _serial->print("\r\nUsage: timer <min> | timer status | timer cancel");
    return;
  }
  if (rest.startsWith("status")){ timerShowStatus(); return; }
  if (rest.startsWith("cancel")){ timerCancel(); return; }
  if (rest.startsWith("start")){
    _serial->print("\r\n'timer start' no se usa. Primero 'on' y luego 'timer <min>'.");
    return;
  }
  uint32_t m=(uint32_t)rest.toInt();
  if (m==0){ _serial->print("\r\nMinutes must be > 0"); return; }
  if (!_controller->isOn()){
    _serial->print("\r\nEstufa OFF: no se puede configurar auto-shutdown.");
    return;
  }
  uint32_t eff=_controller->setAutoShutdown(m);
  if (eff>0) _serial->printf("\r\nAuto-shutdown set: %u min",(unsigned)eff);
  else _serial->print("\r\nNo se pudo establecer auto-shutdown.");
}

void Terminal::timerShowStatus(){
  if (!_controller->isOn()){
    _serial->print("\r\n[Timer] Estufa OFF.");
    return;
  }
  bool enabled=_controller->isAutoShutdownEnabled();
  uint32_t remainMs=_controller->getAutoShutdownRemainingMs();
  if (!enabled){
    _serial->print("\r\n[Timer] No activo.");
    return;
  }
  uint32_t sec=remainMs/1000UL;
  uint32_t min=(sec+59)/60;
  _serial->printf("\r\n[Timer] Restan ~%u min (%u s).",(unsigned)min,(unsigned)sec);
}

void Terminal::timerCancel(){
  if (!_controller->isAutoShutdownEnabled()){
    _serial->print("\r\n[Timer] No activo.");
    return;
  }
  _controller->disableAutoShutdown();
  _serial->print("\r\n[Timer] Cancelado.");
}

void Terminal::cmdAutoOff(){ _controller->disableAutoShutdown(); _serial->print("\r\nAuto-shutdown desactivado."); }

void Terminal::cmdSchedList(){
  _serial->print("\r\n---- Scheduler ----\r\n");
  String sum=_scheduler->buildSummary();
  _serial->print(sum);
}

void Terminal::cmdSchedSummary(){
  _serial->print("\r\n---- Summary ----\r\n");
  String sum=_scheduler->buildSummary();
  _serial->print(sum);
}

bool Terminal::parseSixInts(const String& rest,int out[6]){
  int filled=0; String tmp=rest;
  while(tmp.length() && filled<6){
    int sp=tmp.indexOf(' ');
    String tok=(sp<0)?tmp:tmp.substring(0,sp);
    tok.trim();
    if (tok.length()) out[filled++]=tok.toInt();
    if (sp<0) break;
    tmp=tmp.substring(sp+1); tmp.trim();
  }
  return filled==6;
}

void Terminal::cmdSchedSet(const String& rest){
  int vals[6];
  if(!parseSixInts(rest, vals)){
    _serial->print("\r\nUsage: sched set <idx> <active> <day> <hour> <minute> <power>");
    return;
  }
  if (vals[2]==0){
    _serial->print("\r\n[Info] day=0 interpretado como Domingo (7).");
    vals[2]=7;
  }
  if(!_scheduler->updateEntry(vals[0], vals[1]!=0,(uint8_t)vals[2],(uint8_t)vals[3],(uint8_t)vals[4],(uint8_t)vals[5])){
    _serial->print("\r\nUpdate failed (rangos inválidos).");
    return;
  }
  _serial->print("\r\nSchedule updated.");
}

void Terminal::cmdTemp(){
  uint8_t buf[4]; int len=_comm->readRAM(RAM_ADDR_AMBIENT_TEMP, buf);
  if(len>=1){
    float t=(float)buf[0]/2.0f;
    _serial->printf("\r\nAmbient=%.2f C", t);
  } else _serial->print("\r\nTemp read fail.");
}

void Terminal::cmdClear(){
  _serial->print("\033[2J\033[H");
  _line="";
  _cursorPos=0;
  printPrompt();
}

void Terminal::cmdQuiet(const String& arg){
  if (arg.equalsIgnoreCase("on")) setQuietMode(true);
  else if (arg.equalsIgnoreCase("off")) setQuietMode(false);
  else _serial->print("\r\nUsage: quiet <on|off>");
}

void Terminal::cmdWifi(const String& rest){
  if (rest=="show"){
    _serial->printf("\r\n[WiFi] SSID: %s", gWiFiMgr.getSsid().c_str());
    _serial->printf("\r\n[WiFi] PASS: %s", gWiFiMgr.getPassword().c_str());
    _serial->printf("\r\n[WiFi] Estado: %s", WiFi.status()==WL_CONNECTED?"CONECTADO":"NO CONECTADO");
    _serial->print("\r\nUso: wifi set \"SSID con espacios\" \"PASS opcional\" | wifi reconnect | wifi save | wifi erase");
    return;
  }
  if (rest.startsWith("set ")){
    String args=rest.substring(4);
    std::vector<String> tokens;
    parseArgsQuoted(args, tokens);
    if (tokens.size()<1){
      _serial->print("\r\nUso: wifi set \"SSID\" \"PASS opcional\"");
      return;
    }
    gWiFiMgr.setSsid(tokens[0]);
    if (tokens.size()>1){
      String pass;
      for(size_t i=1;i<tokens.size();++i){
        if (i>1) pass+=" ";
        pass+=tokens[i];
      }
      gWiFiMgr.setPassword(pass);
    }
    _serial->print("\r\n[WiFi] Credenciales en RAM. Usa 'wifi reconnect' o 'wifi save'.");
    return;
  }
  if (rest=="reconnect"){
    _serial->print("\r\n[WiFi] Reconectando...");
    WiFi.disconnect(true);
    if (gWiFiMgr.connect()) _serial->print("\r\n[WiFi] Reconexión OK.");
    else _serial->print("\r\n[WiFi] Reconexión FAIL.");
    return;
  }
  if (rest=="save"){
    gWiFiMgr.saveCredentials(gWiFiMgr.getSsid(), gWiFiMgr.getPassword());
    _serial->print("\r\n[WiFi] Guardado en NVS. (Reboot para ciclo completo).");
    return;
  }
  if (rest=="erase"){
    gWiFiMgr.eraseCredentials();
    _serial->print("\r\n[WiFi] Borrado. Tras reboot volverá a defaults.");
    return;
  }
  _serial->print("\r\nUso: wifi show | set \"SSID\" \"PASS\" | reconnect | save | erase");
}

#ifdef SIMULATION_MODE
void Terminal::cmdSimState(const String& arg){
  if(arg.isEmpty()){ _serial->print("\r\nUsage: simstate <code>"); return; }
  SimStoveComm* sim=(SimStoveComm*)_comm;
  sim->forceState((uint8_t)arg.toInt());
}
void Terminal::cmdSimPower(const String& arg){
  if(arg.isEmpty()){ _serial->print("\r\nUsage: simpower <1..5>"); return; }
  SimStoveComm* sim=(SimStoveComm*)_comm;
  sim->forcePower((uint8_t)arg.toInt());
}
void Terminal::cmdSimTemp(const String& arg){
  if(arg.isEmpty()){ _serial->print("\r\nUsage: simtemp <C>"); return; }
  SimStoveComm* sim=(SimStoveComm*)_comm;
  sim->forceTempBase(arg.toInt());
}
void Terminal::cmdSimFail(){
  SimStoveComm* sim=(SimStoveComm*)_comm;
  sim->enableFailureMode(true);
}
void Terminal::cmdSimRecover(){
  SimStoveComm* sim=(SimStoveComm*)_comm;
  sim->enableFailureMode(false);
}
#endif