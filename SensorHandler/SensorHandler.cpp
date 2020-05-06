/*
  SensorHandler.cpp - Library for requesting information from all sensors and storage it to local variables.
  Created by Nemanja Kljaic, May, 6th 2020.
*/

#include <Arduino.h>
#include <SensorHandler.h>

#define gearSensorID 'A'
#define gearSensorAll 'A'

#define rpmSensorID 'Q'
#define rpmSensorAll 'Q'

#define aditionalWAit 180

float dimeterOfBackTire = 80.0 //cm

int boundRate = 115200;
int DEREControlPin = 8; //RS-485 Controling Pin
int currnetGear = 0;
bool gearWarning = false;
char* separator = ":";

float tempEngine = 0.0;
float tempOut = 0.0;
float freq1 = 0.0;
float freq2 = 0.0;

bool oilPreasureLow = true;

String output = "";
//Variables for converter
char *sensorId = "";
float fValue1 = 0.0;
float fValue2 = 0.0;
///
///REGION: Constructors.
///
SensorHandler::SensorHandler(int _DEREControlPin, int _boundRate);
{
	DEREControlPin = _DEREControlPin;
	boundRate = _boundRate;
	
	Serial.begin(boundRate);
}

///
///REGION: Public functions.
///
void SensorHandler::RefreshVariables()
{
	RefreshAllConverterVariables();
	RefreshAllFromFreqSensor();
	RefreshAllFromGearSensor();
}

int SensorHandler::GetCurentGear()
{
	return currnetGear;
}

bool SensorHandler::GetGearWarning()
{
	return gearWarning;
}
 
float SensorHandler::GetOutTemperature()
{
	return GetCelsiusFromVoltege(tempOut);
}

float SensorHandler::GetEngineTemperature()
{
	return GetCelsiusFromVoltege(tempEngine);
}

float SensorHandler::GetRPMs()
{
	return freq1 * 60;
}

float SensorHandler::GetSpeed()
{
	//Multiple by 60 to convert Hz to RPMs and 
	//devide by 4 becase of 4 magnets on driving shaft (60/4 = 15).
	float rpmsOfDrivingShaft = freq2 * 15.0; 
	
	//Driving shaft should make 32 turns so tire will make 11
	float rpmsOfBackTire = rpmsOfDrivingShaft / 0.34375;
	
	return dimeterOfBackTire * rpmsOfBackTire * 0.001885;
}

bool SensorHandler::GetOilPreasure()
{
	return oilPreasureLow;
}

///
///REGION: Private functions.
///
void SensorHandler::RefreshAllConverterVariables()
{
  sensorId = "";
  fValue1 = 0.0;
  fValue2 = 0.0;
}

void SensorHandler::RefreshAllFromGearSensor()
{
  //A:0:0:0:0:20.44:20.44:20.44:20.44:1:1::
  //A:5:0:5:0:22.81:22.81:23.00:23.00:1:1::
  digitalWrite(DEREControlPin, HIGH); //DE/RE=HIGH Transmit enable
  delay(1);
  Serial.print(gearSensorID);
  Serial.print(gearSensorAll);
  Serial.flush();

  while (!(UCSR0A & (1 << UDRE0)))  // Wait for empty transmit buffer
    UCSR0A |= 1 << TXC0;  // mark transmission not complete
  while (!(UCSR0A & (1 << TXC0)));   // Wait for the transmission to complete
  digitalWrite(DEREControlPin, LOW); //DE/RE=LOW Receive Enabled

  int massageLength = 45;

  delay((massageLength * 8 * 1000) / BoundRate + aditionalWAit);

  byte receivedBytes[massageLength];
  int i = 0;
  while (Serial.available() > 0 )
  {
    receivedBytes[i] = Serial.read();
    i++;
  }
  //TODO: Receive float as byte, not as string
  //A:5:0:5:0:21.81:21.81:21.75:21.75:1:1::x⸮eZXv

  char *sensorId = SubStr(receivedBytes, 1);
  char *gear1 = SubStr(receivedBytes, 2);
  char *gear2 = SubStr(receivedBytes, 4);
  char *gearW1 = SubStr(receivedBytes, 3);
  char *gearW2 = SubStr(receivedBytes, 5);
  char *engineTemp1 = SubStr(receivedBytes, 6);
  char *engineTemp2 = SubStr(receivedBytes, 7);
  char *outTemp1 = SubStr(receivedBytes, 8);
  char *outTemp2 = SubStr(receivedBytes, 9);
  char *oilPreasure1 = SubStr(receivedBytes, 10);
  char *oilPreasure2 = SubStr(receivedBytes, 11);

  DeserializeGearData(sensorId, gear1, gearW1, gear2, gearW2);
  DeserializeEngineTempData(sensorId, engineTemp1, engineTemp2);
  DeserializeOutTempData(sensorId, outTemp1, outTemp2);
  DeserializeOilPreasure(sensorId, oilPreasure1, oilPreasure2);

  SerialFlush();
}

void SensorHandler::RefreshAllFromFreqSensor() //TODO: Add error correct if receive ZERO once.
{
  //QÿQ:"#ã¼:"#ã¼:ÿÿÿÿ:ÿÿÿÿ::
  digitalWrite(DEREControlPin, HIGH); //DE/RE=HIGH Transmit enable
  delay(1);
  Serial.print(rpmSensorID);
  Serial.print(rpmSensorAll);
  Serial.flush();

  while (!(UCSR0A & (1 << UDRE0)))  // Wait for empty transmit buffer
    UCSR0A |= 1 << TXC0;  // mark transmission not complete
  while (!(UCSR0A & (1 << TXC0)));   // Wait for the transmission to complete
  digitalWrite(DEREControlPin, LOW); //DE/RE=LOW Receive Enabled

  int massageLength = 19;

  delay(((massageLength * 8 * 1000) / BoundRate) + aditionalWAit);

  byte receivedBytes[massageLength];
  int i = 0;
  while (Serial.available() > 0 )
  {
    receivedBytes[i] = Serial.read();
    i++;
  }
  sensorId = SubStr(receivedBytes, 1);
  fValue1 = ByteArrayToFloat(SubStr(receivedBytes, 2 ));
  fValue2 = ByteArrayToFloat(SubStr(receivedBytes, 3 ));

  DeserializeFreq1Data(sensorId, fValue1,  fValue2);

  fValue1 = ByteArrayToFloat(SubStr(receivedBytes, 4 ));
  fValue2 = ByteArrayToFloat(SubStr(receivedBytes, 5 ));
  DeserializeFreq2Data(sensorId, fValue1, fValue2);

  SerialFlush();
}

void SensorHandler::DeserializeFreq1Data(char* id, float value1, float value2)
{
  if (*id == rpmSensorID)
  {
    if (value1 == value2)
    {
      freq1 = float(value1);
    }
  }
}

void SensorHandler::DeserializeFreq2Data(char* id, float value1, float value2)
{
  if (*id == rpmSensorID)
  {
    if (value1 == value2)
    {
      freq2 = float(value1);
    }
  }
}

void SensorHandler::DeserializeOutTempData(char* id, char* value1, char* value2)
{
  if (*id == gearSensorID)
  {
    float fValue1 = atof(value1);
    float fValue2 = atof(value2);
    if (fValue1 == fValue2)
    {
      tempOut = float(fValue1);
    }
  }
}

void SensorHandler::DeserializeEngineTempData(char* id, char* value1, char* value2)
{
  if (*id == gearSensorID)
  {
    float fValue1 = atof(value1);
    float fValue2 = atof(value2);
    if (fValue1 == fValue2)
    {
      tempEngine = fValue1;
    }
  }
}

void SensorHandler::DeserializeGearData(char* id, char* valueGear1, char* valueWarning1, char* valueGear2, char* valueWarning2)
{
  if (*id == gearSensorID)
  {
    int intGear1 = *valueGear1 - '0';
    int intGear2 = *valueGear2 - '0';

    if (intGear1 == intGear2)
    {
      currnetGear = intGear1;
    }

    bool bWarning1;
    if (*valueWarning1 == '1')
    {
      bWarning1 = true;
    }
    else
    {
      bWarning1 = false;
    }
    bool bWarning2;
    if (*valueWarning1 == '1')
    {
      bWarning2 = true;
    }
    else
    {
      bWarning2 = false;
    }
    if (bWarning1 == bWarning2)
    {
      gearwarning = bWarning1;
    }
  }
}

void SensorHandler::DeserializeOilPreasure(char* id, char* value1, char* value2)
{
  if (*id != gearSensorID)
  {
    return;
  }
  bool bValue1;
  if (*value1 == '1')
  {
    bValue1 = true;
  }
  else
  {
    bValue1 = false;
  }
  bool bValue2;
  if (*value2 == '1')
  {
    bValue2 = true;
  }
  else
  {
    bValue2 = false;
  }
  if (bValue1 == bValue2)
  {
    oilPreasureLow = bValue1;
  }

}


char* SensorHandler::SubStr (char* input_string, int segment_number)
{
  char *act, *sub, *ptr;
  static char copy[100];
  int i;

  strcpy(copy, input_string);
  for (i = 1, act = copy; i <= segment_number; i++, act = NULL)
  {
    sub = strtok_r(act, separator, &ptr);
    if (sub == NULL) break;
  }
  return sub;
}

float SensorHandler::ByteArrayToFloat(byte* pointerToArray)
{
  union u_tag {
    byte b[4];
    float fval;
  } u;

  u.b[0] = ~pointerToArray[0];
  u.b[1] = ~pointerToArray[1];
  u.b[2] = ~pointerToArray[2];
  u.b[3] = ~pointerToArray[3];

  return u.fval;
}

void SensorHandler::SerialFlush()
{
  while (Serial.available() > 0) {
    char t = Serial.read();
  }
}

float SensorHandler::GetCelsiusFromVoltege(float voltage)
{
	return voltage/10; //TODO: Add real function here.
}
