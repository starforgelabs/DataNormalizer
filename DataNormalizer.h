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
#include <BaseAnalogRead.h>

//
// SUMMARY
//
// This class reads one or more analogue data pins and converts 
// the raw numbers to normalized values.
//
// PURPOSE
//
// It attempts to address the problem of using multiple sensors
// so that they'll return the same value for a given physical 
// quantity. 
//
// USE
//
// Because real-world values are frequently non-linear, multiple 
// calibration measurements can be made to break up the curve into 
// linear segments. 
//
// This class requires n-tuples of calibration data which consist
// of the raw values captured by the Arduino, and the corresponding
// normalized value for the event. 
//
// The tuples must be sorted by ascending raw values.
// The normalized value may be in ascending or descending order.
//
// The raw value is matched to the correct segment, and a linear 
// interpolation is performed to get the normalized value.
// 
// EXAMPLE
//
// In this example, four light sensors are read, and their values
// reported as f/stop values.
//
// The "Pins" array contains the Arduino pins to use. This also defines the 
// meaning for the Readings and Normalized class memers. 
// * Pin 5 will be sensor index 0.
// * Pin 4 will be sensor index 1.
// * Pin 3 will be sensor index 2.
// * Pin 2 will be sensor index 3.
//
// The Aperture array contains the f/stop values obtained during
// calibration. They are multiplied by ten (150=15.0, 124=12.4, etc.).
//
// Data0, Data1, etc. represent the raw sensor values coming off of the Arduino 
// that correspond to the f/stop values. 
//
// The array of vectors CalibrationVectors is required for initializing the class.
//
// const byte SENSOR_COUNT = 4;
// byte Pins[SENSOR_COUNT] = {5, 4, 3, 2};
//
// const byte VECTOR_SIZE = 16;
// int Aperture[VECTOR_SIZE] = {150, 124, 114, 106, 98, 88,  76,  64,  59,  55,  49,  44,  39,  32,  13,  -9 };
// int Data0[VECTOR_SIZE]    = {  5,   9,  16,  24, 30, 47,  88, 127, 161, 180, 213, 284, 376, 499, 713, 959 };
// int Data1[VECTOR_SIZE]    = {  7,  18,  27,  39, 47, 73, 141, 196, 228, 256, 309, 379, 483, 616, 803, 981 };
// int Data2[VECTOR_SIZE]    = {  5,  16,  24,  33, 43, 66, 132, 177, 220, 253, 289, 385, 465, 600, 813, 980 };
// int Data3[VECTOR_SIZE]    = {  7,  14,  23,  32, 42, 65, 123, 168, 213, 241, 274, 371, 450, 575, 789, 970 };
//
// int CalibrationVectors[SENSOR_COUNT] = {Data0, Data1, Data2, Data3};
// 
// DataNormalizer Sensors(SENSOR_COUNT, Pins, VECTOR_SIZE, CalibrationVectors, Aperture);
//


const int SEGMENT_INDEX_LOW         = -1;
const int SEGMENT_INDEX_HIGH        = -2;

// The maximum number of analogue inputs on the Adruino Uno.
const int MAX_NUM_ANALOGUE_INPUTS   =  6;

class DataNormalizer 
{
  public:
    // The following are status codes returned by StatusCode();
    // S = success
    // F = failure
    enum ErrorCodes 
    {
      S_OK, 
      F_Uninitialized,
      F_BadNumberOfSensors,
      F_NoSensorList,
      F_BadPinNumber,
      F_BadVectorSize,
      F_MissingCalibrationVector,
      F_MissingNormalizedVector
    };

  public:
    //
    // aNumberOfSensors    - the number of sensors this object will track
    // aSensorsToUse       - a list of Arduino analogue pin numbers to use
    //                     - This will determine the meaning of the indices
    //                       in the Readings and Normalized arrays. 
    //                       This will allow the user to work with the concept
    //                       of "sensor 0", "sensor 1", "sensor 2", etc. without
    //                       needing to track the actual pin numbers through 
    //                       the code, and allow the user to iterate through the 
    //                       arrays with a for loop.
    // aVectorSize         - The number of elements in the calibration vectors 
    //                       and the normalized vector. 
    // aCalibrationVectors - An array of vectors that contain calibration data 
    //                       for each of the sensors.
    // aNormalizedVector   - A vector of values for the normalized portion of 
    //                       the calibration data.
    //
    DataNormalizer(const byte aNumberOfSensors, const byte* aSensorsToUse, 
                   const byte aVectorSize, const int** aCalibrationVectors, const int* aNormalizedVector);
    ~DataNormalizer();

    //
    // Contains the latest readings from the sensors. 
    //
    // Note that the indices run from 0..SensorCount()-1, and correspond to 
    // the pin numbers used to initialize the class.
    //
    // In practice they shouldn't be modified, but for diagnostic purposes
    // one may populate this array then call Calibrate(). 
    //
    int Readings[MAX_NUM_ANALOGUE_INPUTS];

    //
    // Contains the normalized sensor readings.
    //
    // Note that the indices run from 0..SensorCount()-1, and correspond to 
    // the pin numbers used to initialize the class.
    //
    // These should not be modified.
    //
    int Normalized[MAX_NUM_ANALOGUE_INPUTS];

    //
    // Gets the index number of a pin number.
    //
    // It will return a non-negative index number upon success, 
    // or -1 upon failure. 
    //
    byte IndexOf(byte aPinNumber);

    //
    // Perform the normalization on all the sensor readings.
    //
    // Returns a boolean indicating success.
    //
    bool Normalize();

    //
    // Populate the Readings array with values from the analog pins.
    //
    // Returns a boolean indicating success.
    //
    bool Read();

    //
    // As advertized; calls Read() and Normalize() if there are no errors.
    //
    // Returns a boolean indicating success.
    //
    bool ReadAndNormalize();

    byte SensorCount();

    bool setInputs(BaseAnalogRead* aInputs[]);

    // Return the status of the object per the status codes above.
    ErrorCodes StatusCode();

  private:
    // Perform compensation.
    int Compensate(int aValue, const int* aVector, int* aIndex);

    // Find the correct segment to use for interpolation.
    char FindPosition(int aValue, const int* aVector);

    // Ensure that the data presented to the constructor makes sense,
    // and set the status code appropriately.
    bool Init(const byte aNumberOfSensors, const byte* aSensorsToUse, 
              const byte aVectorSize, const int** aCalibrationVectors, const int* aNormalizedVector);

    bool InitInputs();

    BaseAnalogRead* _Inputs[MAX_NUM_ANALOGUE_INPUTS];

    bool _IOwnInputs;
    
    byte _Pins[MAX_NUM_ANALOGUE_INPUTS];

    // This is the number of sensors.
    byte _SensorCount;
    
    // This is the number of elements in the calibration vectors.
    byte _VectorSize;

    // This is the vector of normalized values.
    const int* _NormalizedVector;

    // This is the array that contains _SensorCount calibration row vectors.
    const int* _CalibrationVectors[MAX_NUM_ANALOGUE_INPUTS];

    // Last error code.
    ErrorCodes _StatusCode;

    // Contains the lower index of the segment that the readings falls in.
    // SEGMENT_INDEX_LOW means that the reading falls below the lowest segment range.
    // SEGMENT_INDEX_HIGH means that the reading falls above the highest segment range.
    int _SegmentBases[MAX_NUM_ANALOGUE_INPUTS];

};

#endif // DATA_NORMALIZER_H

