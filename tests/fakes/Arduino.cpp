#include "Arduino.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>

uint8_t TCCR2A = 0;
uint8_t TCCR2B = 0;
uint8_t TCNT2 = 0;
uint8_t OCR2A = 0;
uint8_t TIMSK2 = 0;
uint8_t UCSR0A = 0;

FakeSerial Serial;
int fakePinModes[32];
int fakeDigitalValues[32];
int fakeAnalogValues[32];
unsigned long fakeDelayTotal = 0;
bool fakeInterruptsEnabled = true;

String::String() = default;

String::String(int value) : value_(std::to_string(value)) {}

String::String(bool value) : value_(value ? "1" : "0") {}

String::String(float value) : value_(std::to_string(value)) {}

String::String(const char* value) : value_(value) {}

const char* String::c_str() const {
  return value_.c_str();
}

void FakeSerial::begin(unsigned long baud) {
  baudRate = baud;
}

int FakeSerial::available() const {
  return static_cast<int>(input.size());
}

int FakeSerial::read() {
  if (input.empty()) {
    return -1;
  }
  const byte value = input.front();
  input.pop_front();
  return value;
}

size_t FakeSerial::print(char value) {
  output.push_back(static_cast<byte>(value));
  return 1;
}

size_t FakeSerial::print(const char* value) {
  const size_t length = std::strlen(value);
  output.insert(output.end(), value, value + length);
  return length;
}

size_t FakeSerial::print(const String& value) {
  return print(value.c_str());
}

size_t FakeSerial::write(const byte* data, size_t length) {
  output.insert(output.end(), data, data + length);
  return length;
}

void FakeSerial::flush() {
  ++flushCount;
}

void FakeSerial::pushInput(const std::vector<byte>& data) {
  input.insert(input.end(), data.begin(), data.end());
}

void FakeSerial::reset() {
  baudRate = 0;
  flushCount = 0;
  input.clear();
  output.clear();
}

std::string FakeSerial::outputString() const {
  return std::string(output.begin(), output.end());
}

void resetFakeArduino() {
  Serial.reset();
  std::fill_n(fakePinModes, 32, -1);
  std::fill_n(fakeDigitalValues, 32, LOW);
  std::fill_n(fakeAnalogValues, 32, 0);
  fakeDelayTotal = 0;
  fakeInterruptsEnabled = true;
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2 = 0;
  OCR2A = 0;
  TIMSK2 = 0;
  UCSR0A = 0;
}

void pinMode(int pin, int mode) {
  fakePinModes[pin] = mode;
}

void digitalWrite(int pin, int value) {
  fakeDigitalValues[pin] = value;
}

int digitalRead(int pin) {
  return fakeDigitalValues[pin];
}

int analogRead(int pin) {
  return fakeAnalogValues[pin];
}

void delay(unsigned long milliseconds) {
  fakeDelayTotal += milliseconds;
}

void cli() {
  fakeInterruptsEnabled = false;
}

void sei() {
  fakeInterruptsEnabled = true;
}
