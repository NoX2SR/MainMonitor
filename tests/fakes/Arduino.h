#ifndef TESTS_FAKES_ARDUINO_H
#define TESTS_FAKES_ARDUINO_H

#include <stddef.h>
#include <stdint.h>

#include <deque>
#include <string>
#include <vector>

typedef uint8_t byte;

#define HIGH 0x1
#define LOW 0x0
#define INPUT 0x0
#define OUTPUT 0x1

#define A0 14
#define A1 15
#define A2 16

#define WGM21 1
#define CS21 1
#define OCIE2A 1
#define UDRE0 5
#define TXC0 6

extern uint8_t TCCR2A;
extern uint8_t TCCR2B;
extern uint8_t TCNT2;
extern uint8_t OCR2A;
extern uint8_t TIMSK2;
extern uint8_t UCSR0A;

#define TIMER2_COMPA_vect timer2_compa_vect
#define ISR(vector) void vector(void)

class String {
 public:
  String();
  String(int value);
  String(bool value);
  String(float value);
  explicit String(const char* value);

  const char* c_str() const;

 private:
  std::string value_;
};

class FakeSerial {
 public:
  void begin(unsigned long baud);
  int available() const;
  int read();
  size_t print(char value);
  size_t print(const char* value);
  size_t print(const String& value);
  size_t write(const byte* data, size_t length);
  void flush();

  void pushInput(const std::vector<byte>& data);
  void reset();
  std::string outputString() const;

  unsigned long baudRate = 0;
  int flushCount = 0;
  std::deque<byte> input;
  std::vector<byte> output;
};

extern FakeSerial Serial;
extern int fakePinModes[32];
extern int fakeDigitalValues[32];
extern int fakeAnalogValues[32];
extern unsigned long fakeDelayTotal;
extern bool fakeInterruptsEnabled;

void resetFakeArduino();
void pinMode(int pin, int mode);
void digitalWrite(int pin, int value);
int digitalRead(int pin);
int analogRead(int pin);
void delay(unsigned long milliseconds);
void cli();
void sei();

#endif
