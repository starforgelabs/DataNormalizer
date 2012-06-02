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
 *
 */

#include "DataNormalizer.h"

DataNormalizer::DataNormalizer(int foo)
{
  _StatusCode = F_Uninitialized;
}

DataNormalizer::DataNormalizer(const byte aNumberOfSensors, const byte* aSensorsToUse, 
                               const byte aVectorSize, const int** aCalibrationVectors, const int* aNormalizedVector)
{
  Init(aNumberOfSensors, aSensorsToUse, aVectorSize, aCalibrationVectors, aNormalizedVector);
}

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

void DataNormalizer::Init(const byte aNumberOfSensors, const byte* aSensorsToUse, 
                          const byte aVectorSize, const int** aCalibrationVectors, const int* aNormalizedVector)
{
  //
  // Validate incoming data.
  //
  if(aNumberOfSensors < 0 || aNumberOfSensors > MAX_NUM_ANALOGUE_INPUTS)
  {
    _StatusCode = F_BadNumberOfSensors;
    return;
  }

  if(aSensorsToUse == NULL)
  {
    _StatusCode = F_NoSensorList;
    return;
  }

  for(int i=0; i<aNumberOfSensors; i++)
    if(aSensorsToUse[i] < 0 || aSensorsToUse[i] >= MAX_NUM_ANALOGUE_INPUTS)
    {
      _StatusCode = F_BadPinNumber;
      return;
    }

  if(aVectorSize < 2)
  {
    _StatusCode = F_BadVectorSize;
    return;
  }

  for(int i=0; i<aVectorSize; i++)
    if(aCalibrationVectors[i] == NULL)
    {
      _StatusCode = F_MissingCalibrationVector;
      return;
    }

  if(aNormalizedVector == NULL)
  {
    _StatusCode = F_MissingNormalizedVector;
    return;
  }

  // 
  // Copy validated data to their storage locations.
  //
  _SensorCount  = aNumberOfSensors;
  _VectorSize = aVectorSize;

  for(int i=0; i<_SensorCount; i++)
    _Pins[i] = aSensorsToUse[i];

  for(int i=0; i<_SensorCount; i++)
    _CalibrationVectors[i] = aCalibrationVectors[i];

  _NormalizedVector = aNormalizedVector; 

  _StatusCode = S_OK;
}

bool DataNormalizer::Normalize()
{
  if (_StatusCode != S_OK) 
    return false;

  for(int i=0; i<_SensorCount; i++)
    Normalized[i] = Compensate(Readings[i], _CalibrationVectors[i], &_SegmentBases[i]);

  return true;

}

bool DataNormalizer::Read()
{
  if (_StatusCode != S_OK) 
    return false;

  for(int i=0; i<_SensorCount; i++)
    Readings[i] = analogRead(_Pins[i]);

  return true;
}

bool DataNormalizer::ReadAndNormalize()
{
  if(!Read())
    return false;

  return Normalize();
}

DataNormalizer::ErrorCodes DataNormalizer::StatusCode()
{
  return _StatusCode;
}

