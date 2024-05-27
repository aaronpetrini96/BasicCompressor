/*
  ==============================================================================

    Compressor.h
    Created: 27 May 2024 3:52:20pm
    Author:  Aaron Petrini

  ==============================================================================
*/

#pragma once

#include "CircularBuffer.h"

class Compressor
{
public:
    Compressor();
    float compressSample (float data, float thresh, float ratio, float attack, float release, float knee);
    float interpolatePoints(float* xPoints, float* yPOints, float detectedValue);
    
private:
    CircularBuffer buffer;
    float tav, rms, gain;
};
