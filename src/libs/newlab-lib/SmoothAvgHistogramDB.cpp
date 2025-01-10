/* Copyright (C) 2025 Nicolas Dittlo <newlab.plugins@gmail.com>
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

#include <Utils.h>

#include "SmoothAvgHistogramDB.h"


SmoothAvgHistogramDB::SmoothAvgHistogramDB(int size, float smoothCoeff,
                                           float defaultValue,
                                           float minDB, float maxDB)
{
    _data.resize(size);
    
    _smoothCoeff = smoothCoeff;
    
    _minDB = minDB;
    _maxDB = maxDB;

    _defaultValue = (defaultValue - _minDB)/(_maxDB - _minDB);
    
    reset();
}

SmoothAvgHistogramDB::~SmoothAvgHistogramDB() {}

void
SmoothAvgHistogramDB::addValues(const vector<float> &values)
{
    if (values.size() != _data.size())
        return;

    vector<float> &normY = _tmpBuf0;
    normY.resize(values.size());

    Utils::normalizedYTodB(values, _minDB, _maxDB, &normY);
    
    for (int i = 0; i < values.size(); i++)
    {
        float valDB = normY.data()[i];
    
        float newVal =
            (1.0 - _smoothCoeff) * valDB + _smoothCoeff*_data.data()[i];
    
        _data.data()[i] = newVal;
    }
}

int
SmoothAvgHistogramDB::getNumValues()
{
    return _data.size();
}

void
SmoothAvgHistogramDB::getValues(vector<float> *values)
{
    values->resize(_data.size());
    
    for (int i = 0; i < _data.size(); i++)
    {
        float val = _data.data()[i];
        
        val = Utils::normalizedYTodBInv(val, _minDB, _maxDB);
        
        values->data()[i] = val;
    }
}

void
SmoothAvgHistogramDB::getValuesDB(vector<float> *values)
{
    values->resize(_data.size());
    
    for (int i = 0; i < _data.size(); i++)
    {
        float val = _data.data()[i];
        
        values->data()[i] = val;
    }
}

void
SmoothAvgHistogramDB::setValues(const vector<float> *values,
                                bool convertToDB)
{
    if (!convertToDB)
        _data = *values;
    else
        Utils::normalizedYTodB(*values, _minDB, _maxDB, &_data);
}

void
SmoothAvgHistogramDB::reset(float smoothCoeff)
{
    if (smoothCoeff > 0.0)
        _smoothCoeff = smoothCoeff;
            
    for (int i = 0; i < _data.size(); i++)
        _data.data()[i] = _defaultValue;
}
