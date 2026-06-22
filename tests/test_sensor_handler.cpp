#include <Arduino.h>

#define private public
#include <SensorHandler.h>
#undef private

#include <cassert>
#include <cmath>
#include <cstring>

extern float dimeterOfBackTire;
extern int currnetGear;
extern bool gearWarning;
extern float tempEngine;
extern float tempOut;
extern float freq1;
extern float freq2;
extern bool oilPreasureLow;

int main() {
  resetFakeArduino();
  SensorHandler handler(9, 115200UL);
  assert(Serial.baudRate == 115200UL);

  currnetGear = 4;
  gearWarning = true;
  tempEngine = 250.0F;
  tempOut = 125.0F;
  freq1 = 100.0F;
  freq2 = 20.0F;
  oilPreasureLow = false;
  assert(handler.GetCurentGear() == 4);
  assert(handler.GetGearWarning());
  assert(std::fabs(handler.GetEngineTemperature() - 25.0F) < 0.001F);
  assert(std::fabs(handler.GetOutTemperature() - 12.5F) < 0.001F);
  assert(std::fabs(handler.GetRPMs() - 6000.0F) < 0.001F);
  const float expectedSpeed = dimeterOfBackTire * (freq2 * 15.0F / 0.34375F) * 0.001885F;
  assert(std::fabs(handler.GetSpeed() - expectedSpeed) < 0.001F);
  assert(!handler.GetOilPreasure());

  char rpmId[] = "Q";
  char gearId[] = "A";
  char wrongId[] = "X";
  handler.DeserializeFreq1Data(rpmId, 12.0F, 12.0F);
  assert(freq1 == 12.0F);
  handler.DeserializeFreq1Data(rpmId, 1.0F, 2.0F);
  handler.DeserializeFreq1Data(wrongId, 3.0F, 3.0F);
  assert(freq1 == 12.0F);
  handler.DeserializeFreq2Data(rpmId, 8.0F, 8.0F);
  assert(freq2 == 8.0F);
  handler.DeserializeFreq2Data(rpmId, 1.0F, 2.0F);
  handler.DeserializeFreq2Data(wrongId, 3.0F, 3.0F);
  assert(freq2 == 8.0F);

  char one[] = "1";
  char zero[] = "0";
  char three[] = "3";
  char four[] = "4";
  handler.DeserializeGearData(gearId, three, one, three, one);
  assert(currnetGear == 3);
  assert(gearWarning);
  handler.DeserializeGearData(gearId, three, zero, four, one);
  assert(currnetGear == 3);
  assert(gearWarning);
  handler.DeserializeGearData(wrongId, four, zero, four, zero);
  assert(currnetGear == 3);

  char voltage25[] = "250";
  char voltage30[] = "300";
  handler.DeserializeEngineTempData(gearId, voltage25, voltage25);
  assert(tempEngine == 250.0F);
  handler.DeserializeEngineTempData(gearId, voltage25, voltage30);
  handler.DeserializeEngineTempData(wrongId, voltage30, voltage30);
  assert(tempEngine == 250.0F);
  handler.DeserializeOutTempData(gearId, voltage30, voltage30);
  assert(tempOut == 300.0F);
  handler.DeserializeOutTempData(gearId, voltage25, voltage30);
  handler.DeserializeOutTempData(wrongId, voltage25, voltage25);
  assert(tempOut == 300.0F);

  handler.DeserializeOilPreasure(gearId, one, one);
  assert(oilPreasureLow);
  handler.DeserializeOilPreasure(gearId, zero, one);
  assert(oilPreasureLow);
  handler.DeserializeOilPreasure(wrongId, zero, zero);
  assert(oilPreasureLow);

  const float original = 42.25F;
  char encoded[sizeof(original)];
  std::memcpy(encoded, &original, sizeof(original));
  for (char& item : encoded) {
    item = static_cast<char>(~static_cast<byte>(item));
  }
  assert(std::fabs(handler.ByteArrayToFloat(encoded) - original) < 0.001F);

  char message[] = "A:5:0";
  assert(std::strcmp(handler.SubStr(message, 1), "A") == 0);
  assert(std::strcmp(handler.SubStr(message, 2), "5") == 0);
  assert(handler.SubStr(message, 5) == nullptr);

  handler.RefreshAllConverterVariables();
  Serial.pushInput({'a', 'b'});
  handler.SerialFlush();
  assert(Serial.available() == 0);
}
