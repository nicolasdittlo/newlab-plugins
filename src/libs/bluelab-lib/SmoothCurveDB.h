/* Copyright (C) 2025 Nicolas Dittlo <bluelab.plugins@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this software; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef SMOOTH_CURVE_DB_H
#define SMOOTH_CURVE_DB_H

#include <vector>
using namespace std;

class Curve;
class SmoothAvgHistogramDB;
class SmoothCurveDB
{
 public:
    SmoothCurveDB(Curve *curve,
                  float smoothFactor,
                  int size, float defaultValue,
                  float minDB, float maxDB,
                  float sampleRate);
    
    virtual ~SmoothCurveDB();

    void reset(float sampleRate, float smoothFactor);

    void clearValues();
    
    void setValues(const vector<float> &values);

 protected:
    float _minDB;
    float _maxDB;
    
    SmoothAvgHistogramDB *_histogram;
    
    Curve *_curve;

    float _sampleRate;

 private:
    vector<float> _tmpBuf0;
    vector<float> _tmpBuf1;
    vector<float> _tmpBuf2;
};

#endif
