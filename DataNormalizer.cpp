/*
 *  DataNormalizer.h
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

DataNormalizer::DataNormalizer(byte aAnalogueInputPinNumber, byte aNumberOfArrayElements, int* aSegmentArray, int* aNormalizedArray)
{
  _StatusCode = F_NOT_INITIALIZED;

  _Pin = aAnalogueInputPinNumber;
  _Count = aNumberOfArrayElements-1;
  _Segments = aSegmentArray;
  _Normalized = aNormalizedArray;
  
  Init();
}

int DataNormalizer::Compensate(int aValue)
{
  char lIndex = FindPosition(aValue);

  if(lIndex < 0)
  {
    _Segment = SEGMENT_INDEX_LOW;
    return _Normalized[0];
  }
  else if(lIndex >= _Count)
  {
    _Segment = SEGMENT_INDEX_HIGH;
    return _Normalized[_Count];
  }
  else
    _Segment = lIndex;
  
  return map(aValue, _Segments[lIndex], _Segments[lIndex+1], _Normalized[lIndex], _Normalized[lIndex+1]);
}

//
// Finds which segment the raw value lies in.
//
// < 0         - Below the segment values. 
// 0.._Count-1 - the segment index
// >= _Count   - the value lies above the segment values.
//
char DataNormalizer::FindPosition(int aValue)
{
  byte i;
  for(i=0; i<=_Count; i++)
    if(aValue <= _Segments[i])
      return i-1;
  
  return _Count;
}

void DataNormalizer::Init()
{
  if (_Count < 1) 
  {
    _StatusCode = F_NOT_ENOUGH_DATA;
    return;
  }
  
  for (int i=0 ; i< _Count ; i++) 
    if (_Segments[i] >= _Segments[i+1]) 
    {
      _StatusCode = F_SEGMENTS_NOT_ASCENDING;
      return;
    }

  _StatusCode = S_OK;
}

int DataNormalizer::RawValue()
{
  return _RawValue;
}

bool DataNormalizer::Read()
{
  if (_StatusCode != S_OK) 
    return false;

  _RawValue = analogRead(_Pin);
  _Value = Compensate(_RawValue);
  return true;
}

bool DataNormalizer::setRawValue(int aValue)
{
  if (_StatusCode != S_OK) 
    return false;

  _RawValue = aValue;
  _Value = Compensate(_RawValue);
  return true;
}

byte DataNormalizer::StatusCode()
{
  return _StatusCode;
}

int DataNormalizer::Value()
{
  return _Value;
}

