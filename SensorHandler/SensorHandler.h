/*
  SensorHandler.cpp - Library for requesting information from all sensors and storage it to local variables.
  Created by Nemanja Kljaic, May, 6th 2020.
*/
#ifndef SensorHandler_h
#define SensorHandler_h
#include <Arduino.h>

class SensorHandler
{
    //constructor
 public:
    SensorHandler(int _DEREControlPin, int _boundRate);
	//Sends request to all sensors and store their respond to local variables.
    void RefreshVariables();
	//Returns curent gear number (0-5)
	int GetCurentGear();
	//Returns true if rider wants to get higer that 5th gear or lower than 1st.
	bool GetGearWarning();
	//Returns float value for outside temperature in Celsius 
	float GetOutTemperature();
	//Returns float value for engine temperature in Celsius
	float GetEngineTemperature();
	//Returns current value for Engine RPMs.
	float GetRPMs();
	//Returns current speed in Km/h
	float GetSpeed();
	//Returns TRUE if oil preasure is LOW
	bool GetOilPreasure();
	
 private:
	void RefreshAllConverterVariables();
	void RefreshAllFromGearSensor();
	void RefreshAllFromFreqSensor() //TODO: Add error correct if receive ZERO once.
	void DeserializeFreq1Data(char* id, float value1, float value2);
	void DeserializeFreq2Data(char* id, float value1, float value2);
	void DeserializeOilPreasure(char* id,char* value1,char* value2);
	void DeserializeOutTempData(char* id,char* value1,char* value2);
	void DeserializeEngineTempData(char* id,char* value1,char* value2);
	void DeserializeGearData(char* id, char* valueGear1, char* valueWarning1, char* valueGear2, char* valueWarning2);
	void SerialFlush();
	float ByteArrayToFloat(byte* pointerToArray);
	float GetCelsiusFromVoltege(float voltage);
};
#endif





