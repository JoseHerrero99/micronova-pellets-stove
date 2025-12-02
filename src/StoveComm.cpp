#include "StoveComm.h"

StoveComm::StoveComm(): _rxPin(-1), _txPin(-1), _enPin(-1), _serial(&Serial2), _serialMutex(nullptr) {}

void StoveComm::begin(int rxPin, int txPin, int enPin){
  _rxPin=rxPin; _txPin=txPin; _enPin=enPin;
  _serial->end();
  _serial->begin(STOVE_SERIAL_BAUD, STOVE_SERIAL_CONFIG, _rxPin, _txPin);
  pinMode(_enPin, OUTPUT);
  disableRx();
  _serialMutex = xSemaphoreCreateMutex();
  logInfo("StoveComm REAL initialized.");
}

bool StoveComm::isRXEnabled(){return rx;};
void StoveComm::enableRx(){ digitalWrite(_enPin, LOW); }
void StoveComm::disableRx(){ digitalWrite(_enPin, HIGH); }

int StoveComm::readFromStove(uint8_t cmdBase, uint8_t addr, uint8_t* buffer){
  if (!_serialMutex) return 0;
  if (xSemaphoreTake(_serialMutex, pdMS_TO_TICKS(300)) != pdTRUE) return 0;

  Serial.printf("[readFromStove] Sending cmdBase=0x%02X, addr=0x%02X\n", cmdBase, addr);
  _serial->write(cmdBase);
  _serial->flush();
  _serial->write(addr);
  _serial->flush();

  enableRx();
  delay(120);

  int n = 0;
  while (_serial->available()){
    uint8_t b = _serial->read();
    if (n < 64) {
      buffer[n++] = b;
      Serial.printf("[readFromStove] buffer[%d] = 0x%02X\n", n-1, b);
    } else {
      Serial.printf("[readFromStove] extra byte discarded: 0x%02X\n", b);
    }
  }

  disableRx();
  xSemaphoreGive(_serialMutex);

  Serial.printf("[readFromStove] Total bytes read: %d\n", n);
  return n;
}

int StoveComm::readRAM(uint8_t address, uint8_t *buffer) {
  return readFromStove(STOVE_OFFSET_RAM_READ, address, buffer);
}
int StoveComm::readEEPROM(uint8_t address, uint8_t *buffer) {
  return readFromStove(STOVE_OFFSET_EEPROM_READ, address, buffer);
}

void StoveComm::writeToStove(uint8_t location, uint8_t command, uint8_t data){
  if (!_serialMutex) return;
  if (xSemaphoreTake(_serialMutex, pdMS_TO_TICKS(300))!=pdTRUE) return;
  uint8_t chk = calculate_checksum( location, command, data );
  uint8_t data_to_write[4] = {
    location,
    command,
    data,
    chk
  };
    
  for ( int i = 0; i < 4; i++ ){
    _serial->write( data_to_write[i] );
    delay(1);
  }
  xSemaphoreGive(_serialMutex);
}

byte StoveComm::calculate_checksum( uint8_t dest, uint8_t addr, uint8_t val ){
  uint8_t checksum = 0;
  checksum = dest+addr+val;
  if ( checksum >= 256 ){
    checksum = checksum - 256;
  }
  return (uint8_t)checksum;
}

void StoveComm::writeRAM(uint8_t address, uint8_t data) {
  writeToStove(STOVE_OFFSET_RAM_WRITE, address, data);
}
void StoveComm::writeEEPROM(uint8_t address, uint8_t data) {
  writeToStove(STOVE_OFFSET_EEPROM_WRITE, address, data);
}