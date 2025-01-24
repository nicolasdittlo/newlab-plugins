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

#ifndef SMOOTH_AVG_HISTOGRAM_DB_H
#define SMOOTH_AVG_HISTOGRAM_DB_H

#include <vector>
using namespace std;

// Normalize Y to dB internally
class SmoothAvgHistogramDB
{
public:
    SmoothAvgHistogramDB(int size, float smoothCoeff,
                         float defaultValue, float minDB, float maxDB);
    
    virtual ~SmoothAvgHistogramDB();
    
    void addValues(const vector<float> &values);
    
    int getNumValues();

    // Get values scaled back to amp
    void getValues(vector<float> *values);

    // Get internal values which are in DB
    void getValuesDB(vector<float> *values);
    
    // Force the internal values to be the new values,
    // (withtout smoothing)
    void setValues(const vector<float> *values,
                   bool convertToDB = false);
    
    void reset(float smoothCoeff = -1.0);
    
protected:
    vector<float> _data;

    // If smooth is 0, then we get instantaneous value.
    float _smoothCoeff;
    
    float _defaultValue;
    
    float _minDB;
    float _maxDB;

 private:
    // Tmp buffers
    vector<float> _tmpBuf0;
};

#endif
