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
SmoothCurveDB::setValues(const vector<float> &values, bool reset)
{
    // Add the values
    int histoNumValues = _histogram->getNumValues();
    
    vector<float> &values0 = _tmpBuf0;
    values0 = values;

    bool useFilterBank = false;

    // Filter banks
    if (!reset)
    {
        vector<float> &decimValues = _tmpBuf1;

        Scale::FilterBankType type =
            _curve->_scale->typeToFilterBankType(_curve->_xScale);
        _curve->_scale->applyScaleFilterBank(type, &decimValues, values0,
                                             _sampleRate, histoNumValues);
        
        values0 = decimValues;
    }
    
    vector<float> &avgValues = _tmpBuf2;
    avgValues = values0;

    // Check if we have the same scale
    bool sameScale = false;
    Scale::Type curveScale;
    float curveMinY;
    float curveMaxY;
    _curve->getYScale(&curveScale, &curveMinY, &curveMaxY);

    if ((curveScale == Scale::DB) &&
        (fabs(curveMinY - _minDB) < NL_EPS) &&
        (fabs(curveMaxY - _maxDB) < NL_EPS))
        sameScale = true;
        
    if (!reset)
    {
        _histogram->addValues(values0);

        // Process values and update curve
        if (!sameScale)
            _histogram->getValues(&avgValues);
        else
            _histogram->getValuesDB(&avgValues);
    }
    else
        _histogram->setValues(&values0, false);
    
    _curve->clearValues();
    
    _curve->setValues(avgValues, !useFilterBank, !sameScale);
}
