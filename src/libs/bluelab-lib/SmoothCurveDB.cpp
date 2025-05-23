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

#include <math.h>

#include "Defines.h"
#include "SmoothAvgHistogramDB.h"
#include "Curve.h"

#include "SmoothCurveDB.h"

SmoothCurveDB::SmoothCurveDB(Curve *curve,
                             float smoothFactor,
                             int size, float defaultValue,
                             float minDB, float maxDB,
                             float sampleRate)
{
    _minDB = minDB;
    _maxDB = maxDB;
    
    _histogram = new SmoothAvgHistogramDB(size, smoothFactor,
                                          defaultValue, minDB, maxDB);
    
    _curve = curve;

    _sampleRate = sampleRate;
}

SmoothCurveDB::~SmoothCurveDB()
{
    delete _histogram;
}

void
SmoothCurveDB::reset(float sampleRate, float smoothFactor)
{
    // Smooth factor can change when DAW block size changes
    _histogram->reset(smoothFactor);

    _sampleRate = sampleRate;
}

void
SmoothCurveDB::clearValues()
{
    _histogram->reset();
    
    _curve->clearValues();
}

void
SmoothCurveDB::setValues(const vector<float> &values)
{
    // Add the values
    int histoNumValues = _histogram->getNumValues();
    
    vector<float> &values0 = _tmpBuf0;
    values0 = values;

    bool useFilterBank = true;

    // Filter banks
    vector<float> &decimValues = _tmpBuf1;
    
    Scale::FilterBankType type =
        _curve->_scale->typeToFilterBankType(_curve->_xScale);
    _curve->_scale->applyScaleFilterBank(type, &decimValues, values0,
                                         _sampleRate, histoNumValues);
    
    values0 = decimValues;

    vector<float> &avgValues = _tmpBuf2;
    avgValues = values0;

    // Check if we have the same scale
    bool sameScale = false;
    Scale::Type curveScale;
    float curveMinY;
    float curveMaxY;
    _curve->getYScale(&curveScale, &curveMinY, &curveMaxY);

    if ((curveScale == Scale::DB) &&
        (fabs(curveMinY - _minDB) < BL_EPS) &&
        (fabs(curveMaxY - _maxDB) < BL_EPS))
        sameScale = true;
        
    _histogram->addValues(values0);
    
    // Process values and update curve
    if (!sameScale)
        _histogram->getValues(&avgValues);
    else
        _histogram->getValuesDB(&avgValues);
    
    _curve->clearValues();
    
    _curve->setValues(avgValues, !useFilterBank, !sameScale);
}
