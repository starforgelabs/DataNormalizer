//
//  DataNormalizer.h
//  Sun Tracker
//
//  Created by 治永夢守 on 12/05/27.
//  Copyright 2012 James Knowles. All rights reserved.
//
// This work is licensed under a Creative Commons 
// Attribution-ShareAlike 3.0 Unported License.
//
// https://creativecommons.org/licenses/by-sa/3.0/
//
//
//

#ifndef DATA_NORMALIZER_H
#define DATA_NORMALIZER_H

#include "Arduino.h"

// This is part of an Arduino library. 

//
// SUMMARY
//
// This class reads an analogue data pin and converts the raw number 
// to a normalized value.
//
// PURPOSE
//
// It attempts to address the problem of synchronizing multiple sensors
// so that (ideally) for a given external quantity, the same number 
// can be obtained for each of the sensors even though the sensors may
// not return identical raw values themselves.
//
// USE
//
// Because real-world values are frequently non-linear, multiple 
// calibration measurements can be made to break up the curve into 
// linear segments. 
//
// This class requires two tables, representing tuples of 
// (raw value, normalized value). 
// The tuples bust be sorted by ascending raw values.
// The normalized value may be in ascending or descending order.
//
// The raw value is matched to the correct segment, and a linear 
// interpolation is performed on that segment. 
// 
// Normally the same array representing normalized values will be 
// shared among sensors. 
//
// EXAMPLE
//
// In this example, Aperture contains the normalized values, specifically
// an aperture value multiplied by ten (150=15.0, 124=12.4, etc.).
//
// Data1, Data2, etc. represent the raw values coming off of the Arduino 
// analogue pins that correspond to the aperture values.
//
// int Aperture[DATA_COUNT] = {150, 124, 114, 106, 98, 88, 76, 64, 59, 55, 49, 44, 39, 32, 13, -9 };
// int Data1[DATA_COUNT] = { 5, 9, 16, 24, 30, 47, 88, 127, 161, 180, 213, 284, 376, 499, 713, 959 };
// int Data2[DATA_COUNT] = { 7, 18, 27, 39, 47, 73, 141, 196, 228, 256, 309, 379, 483, 616, 803, 981 };
// int Data3[DATA_COUNT] = { 5, 16, 24, 33, 43, 66, 132, 177, 220, 253, 289, 385, 465, 600, 813, 980 };
// int Data4[DATA_COUNT] = { 7, 14, 23, 32, 42, 65, 123, 168, 213, 241, 274, 371, 450, 575, 789, 970 };
// 
// DataNormalizer Sensor1(5, DATA_COUNT, Data1, Aperture);
// DataNormalizer Sensor2(4, DATA_COUNT, Data2, Aperture);
// DataNormalizer Sensor3(3, DATA_COUNT, Data3, Aperture);
// DataNormalizer Sensor4(2, DATA_COUNT, Data4, Aperture);
//


// The following are status codes returned by StatusCode();
// S = success
// F = failure
const byte S_OK                     =  0;
const byte F_NOT_INITIALIZED        =  1;
const byte F_SEGMENTS_NOT_ASCENDING =  2;
const byte F_NOT_ENOUGH_DATA        =  3;

const int SEGMENT_INDEX_LOW         = -1;
const int SEGMENT_INDEX_HIGH        = -2;

class DataNormalizer 
{
  public:
    //
    // aAnalogueInputPinNumber - the analogue pin number to read
    // aArrayElements          - the number of entries in the conversion tables
    // aSegmentArray           - an ascending array of raw values
    // aNormalizedArray        - an array of normalized values
    //
    DataNormalizer(byte aAnalogueInputPinNumber, byte aNumberOfArrayElements, int* aSegmentArray, int* aNormalizedArray);

    //
    // Read the analog pin, store the value in RawValue, perform the 
    // normalization, and store its value in Value.
    //
    bool Read();

    // Return the raw value read from the analogue input pin.
    int RawValue();

    // Instead of reading the current value from the analogue pin and 
    // normalizing, perform the process with a specific value.
    bool setRawValue(int aValue);

    // Return the lower index of the segment that the raw value falls in.
    // SEGMENT_INDEX_LOW means that the raw value falls below the lowest segment range.
    // SEGMENT_INDEX_HIGH means that the raw value falls above the highest segment range.
    int Segment();
    
    // Return the status of the object per the status codes above.
    byte StatusCode();

    // Return the normalized value.
    int Value();
  
  private:
    // Perform compensation.
    int Compensate(int aValue);

    // Find the correct segment to use for interpolation.
    char FindPosition(int aValue);

    // Ensure that the data presented to the constructor makes sense,
    // and set the status code appropriately.
    void Init();
    
    byte _Pin;

    byte _Count;
    int* _Normalized;
    int* _Segments;

    int  _RawValue;
    byte _StatusCode;
    int  _Segment;
    int  _Value;
};

#endif // DATA_NORMALIZER_H

