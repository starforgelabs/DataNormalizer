/*
 *  DataNormalizer.cpp
 *  Sun Tracker
 *
 *  Created by 治永夢守 on 12/05/27.
 *  Copyright 2012 James Knowles. All rights reserved.
 *
 * This work is licensed under a Creative Commons 
 * Attribution-ShareAlike 3.0 Unported License.
 *
 * https://creativecommons.org/licenses/by-sa/3.0/
 *
 * This code is strictly "as is". Use at your own risk. 
 *
 *
 */

#include "DataNormalizer.h"

//
// Normalize the data for a particular reading.
//
// aValue - The reading.
// aVector - The vector of readings to use.
// aIndex - Where to cache the index for aVector.
//
int DataNormalizer::Compensate(int aValue, const int* aVector, int* aIndex)
{
  *aIndex = FindPosition(aValue, aVector);

  if(*aIndex < 0)
  {
    *aIndex = SEGMENT_INDEX_LOW;
    return _NormalizedVector[0];
  }
  else if(*aIndex >= _VectorSize)
  {
    *aIndex = SEGMENT_INDEX_HIGH;
    return _NormalizedVector[_VectorSize];
  }
  
  return map(aValue, aVector[*aIndex], aVector[*aIndex+1], _NormalizedVector[*aIndex], _NormalizedVector[*aIndex+1]);
}

//
// Finds which segment the raw value lies in.
//
// < 0              - Below the segment values. 
// 0.._VectorSize-1 - the segment index
// >= _VectorSize   - the value lies above the segment values.
//
char DataNormalizer::FindPosition(int aValue, const int* aVector)
{
  byte i;
  for(i=0; i<=_VectorSize; i++)
    if(aValue <= aVector[i])
      return i-1;
  
  return _VectorSize;
}

byte DataNormalizer::IndexOf(byte aPinNumber)
{
  if(_StatusCode != S_OK) return -1;

  for(int i=0; i<_SensorCount; i++)
    if(_Inputs[i]->PinNumber() == aPinNumber)
      return i;

  return -1;
}

bool DataNormalizer::configure(const byte aNumberOfSensors, BaseAnalogRead* aSensorReaders[], 
                               const byte aVectorSize, const int* aCalibrationVectors[], const int aNormalizedVector[])
{
	//
	// Validate incoming data.
	//
	if(aNumberOfSensors < 0 || aNumberOfSensors > MAX_NUM_ANALOGUE_INPUTS)
	{
		_StatusCode = F_BadNumberOfSensors;
		return false;
	}
	
	if(aSensorReaders == NULL)
	{
		_StatusCode = F_NoSensorList;
		return false;
	}
	
	for(int i=0; i<aNumberOfSensors; i++)
		if(aSensorReaders[i] == NULL)
		{
			_StatusCode = F_BadPinNumber;
			return false;
		}
	
	if(aVectorSize < 2)
	{
		_StatusCode = F_BadVectorSize;
		return false;
	}
	
	for(int i=0; i<aNumberOfSensors; i++)
		if(aCalibrationVectors[i] == NULL)
		{
			_StatusCode = F_MissingCalibrationVector;
			return false;
		}
	
	if(aNormalizedVector == NULL)
	{
		_StatusCode = F_MissingNormalizedVector;
		return false;
	}
	
	// 
	// Copy validated data to their storage locations.
	//
	_SensorCount  = aNumberOfSensors;
	_VectorSize = aVectorSize;
	
	for(int i=0; i<aNumberOfSensors; i++)
		_Inputs[i] = aSensorReaders[i];
	
	for(int i=0; i<_SensorCount; i++)
		_CalibrationVectors[i] = aCalibrationVectors[i];
	
	_NormalizedVector = aNormalizedVector; 
	
	_StatusCode = S_OK;
	return true;
}

bool DataNormalizer::Normalize()
{
  if (_StatusCode != S_OK) 
    return false;

  for(int i=0; i<_SensorCount; i++)
    Normalized[i] = Compensate(Values[i], _CalibrationVectors[i], &_SegmentBases[i]);

  return true;

}

bool DataNormalizer::Read()
{
  if (_StatusCode != S_OK) 
    return false;

  for(int i=0; i<_SensorCount; i++)
    Values[i] = _Inputs[i]->Read();

  return true;
}

bool DataNormalizer::ReadAndNormalize()
{
  if(!Read())
    return false;

  return Normalize();
}


