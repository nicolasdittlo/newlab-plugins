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

#include <algorithm>
using namespace std;

#include "Scale.h"
#include "AWeighting.h"
#include "PartialFilterMarchand.h"
#include "PartialFilterAMFM.h"
#include "PeakDetectorNL.h"
#include "PeakDetectorBillauer.h"
#include "QIFFT.h"
#include "PhasesUnwrapper.h"
#include "Utils.h"
#include "Defines.h"
#include "PartialTracker.h"

#define MIN_AMP_DB -120.0

#define DISCARD_FLAT_PARTIAL 1
#define DISCARD_FLAT_PARTIAL_COEFF 25000.0

#define GLUE_BARBS              1
#define GLUE_BARBS_AMP_RATIO    10.0

// Filter

// Do we filter ?
#define FILTER_PARTIALS 1

// NOTE: See: https://www.dsprelated.com/freebooks/sasp/Spectral_Modeling_Synthesis.html
//
// and: https://www.dsprelated.com/freebooks/sasp/PARSHL_Program.html#app:parshlapp
//
// and: https://ccrma.stanford.edu/~jos/parshl/

// Get the precision when interpolating peak magns, but also for phases
#define INTERPOLATE_PHASES 1

// Better mel filtering of phase if they are unwrapped!
#define MEL_UNWRAP_PHASES 1

#define DENORM_PARTIAL_INDICES 1

#define USE_QIFFT 1

// It is better with log then just with dB!
#define USE_QIFFT_YLOG 1

#define USE_A_WEIGHTING 1

PartialTracker::PartialTracker(int bufferSize, float sampleRate)
{
    _bufferSize = bufferSize;
    _sampleRate = sampleRate;
    
    _threshold = -60.0;
    
    _maxDetectFreq = -1.0;
    
    // Scale
    _scale = new Scale();
    
    //
    
    _xScale = Scale::LINEAR;
    _yScale = Scale::DB;

#if USE_QIFFT_YLOG
    _yScale2 = Scale::LOG_NO_NORM;
#endif
    
    _xScaleInv = Scale::LINEAR;

    _yScaleInv = Scale::DB_INV;
#if USE_QIFFT_YLOG
    _yScaleInv2 = Scale::LOG_NO_NORM_INV;
#endif
    
    _timeSmoothCoeff = 0.5;
    
    // Optim
    computeAWeights(bufferSize/2, sampleRate);

#if USE_NL_PEAK_DETECTOR
    _peakDetector = new PeakDetectorBL();
#endif
#if USE_BILLAUER_PEAK_DETECTOR
    float maxDelta = (USE_QIFFT_YLOG == 0) ? 1.0 : -MIN_AMP_DB/4;
    _peakDetector = new PeakDetectorBillauer(maxDelta);
#endif

#if USE_PARTIAL_FILTER_MARCHAND
    _partialFilter = new PartialFilterMarchand(bufferSize, sampleRate);
#endif
#if USE_PARTIAL_FILTER_AMFM
    _partialFilter = new PartialFilterAMFM(bufferSize, sampleRate);
#endif
}

PartialTracker::~PartialTracker()
{
    delete _scale;
    delete _peakDetector;
    delete _partialFilter;
}

void
PartialTracker::reset()
{
    _result.clear();
    
    _currentMagns.resize(0);
    _currentPhases.resize(0);
    
    _prevMagns.resize(0);

    if (_partialFilter != NULL)
        _partialFilter->reset(_bufferSize, _sampleRate);
}

void
PartialTracker::reset(int bufferSize, float sampleRate)
{
    _bufferSize = bufferSize;
    _sampleRate = sampleRate;
    
    reset();

    computeAWeights(bufferSize/2, sampleRate);
}

float
PartialTracker::getMinAmpDB()
{
    return MIN_AMP_DB;
}

void
PartialTracker::setThreshold(float threshold)
{
    _threshold = threshold;

    _peakDetector->setThreshold(threshold);
}

void
PartialTracker::setThreshold2(float threshold2)
{
    _peakDetector->setThreshold2(threshold2);
}

void
PartialTracker::setData(const vector<float> &magns,
                        const vector<float> &phases)
{
    _currentMagns = magns;
    _currentPhases = phases;

    // Time smooth
    // Removes the noise and make more neat peaks
    Utils::smooth(&_currentMagns, &_prevMagns, _timeSmoothCoeff);
    
    preProcess(&_currentMagns, &_currentPhases);
}

void
PartialTracker::getPreProcessedMagns(vector<float> *magns)
{
    *magns = _currentMagns;
}

void
PartialTracker::detectPartials()
{
    vector<float> &magns0 = _tmpBuf0;
    magns0 = _currentMagns;
    
    vector<Partial> &partials = _tmpPartials0;
    partials.resize(0);
    detectPartials(magns0, _currentPhases, &partials);

#if USE_NL_PEAK_DETECTOR
    // For first partial detection
    postProcessPartials(magns0, &partials);
#endif
    
    _result = partials;
}

void
PartialTracker::filterPartials()
{    
#if FILTER_PARTIALS
    
#if USE_PARTIAL_FILTER_AMFM
    // Adjust the scale
    for (int i = 0; i < _result.size(); i++)
    {
        Partial &p = _result[i];

        float amp =
            _scale->applyScale(_yScaleInv, p._amp,
                               (float)MIN_AMP_DB, (float)0.0);
        
        float ampNorm =
            _scale->applyScale(_yScale2, amp,
                               (float)MIN_AMP_DB, (float)0.0);
        p._amp = ampNorm;
    }
#endif
    
    _partialFilter->filterPartials(&_result);

#if USE_PARTIAL_FILTER_AMFM
    // Adjust the scale
    for (int i = 0; i < _result.size(); i++)
    {
        Partial &p = _result[i];
        float ampNorm =
            _scale->applyScale(_yScaleInv2, p._amp,
                               (float)MIN_AMP_DB, (float)0.0);
        float ampDbNorm =
            _scale->applyScale(_yScale, ampNorm,
                               (float)MIN_AMP_DB, (float)0.0);
        p._amp = ampDbNorm;
    }
#endif
    
#endif
}

void
PartialTracker::removeRealDeadPartials(vector<Partial> *partials)
{
    vector<Partial> &result = _tmpPartials5;
    result.resize(0);
    
    for (int i = 0; i < partials->size(); i++)
    {
        const Partial &p = (*partials)[i];
        if (p._wasAlive)
            result.push_back(p);
    }
    
    *partials = result;
}

void
PartialTracker::getPartials(vector<Partial> *partials)
{
    *partials = _result;
    
    // For sending good result to SASFrame
    removeRealDeadPartials(partials);
}

void
PartialTracker::getRawPartials(vector<Partial> *partials)
{
    *partials = _result;
}

void
PartialTracker::clearResult()
{
    _result.clear();
}

void
PartialTracker::setMaxDetectFreq(float maxFreq)
{
    _maxDetectFreq = maxFreq;
}

void
PartialTracker::detectPartials(const vector<float> &magns,
                               const vector<float> &phases,
                               vector<Partial> *outPartials)
{
    int maxIndex = magns.size() - 1;
    if (_maxDetectFreq > 0.0)
        maxIndex = _maxDetectFreq*_bufferSize*0.5;
    if (maxIndex > magns.size() - 1)
        maxIndex = magns.size() - 1;
    
    vector<PeakDetector::Peak> peaks;

#if !USE_QIFFT_YLOG
    _peakDetector->detectPeaks(magns, &peaks,
                               DETECT_PARTIALS_START_INDEX, maxIndex);
    
    computePartials(peaks, magns, phases, outPartials);
#else
    _peakDetector->detectPeaks(_logMagns, &peaks,
                               DETECT_PARTIALS_START_INDEX, maxIndex);

    // Log
    computePartials(peaks, _logMagns, phases, outPartials);

    // Adjust the scale
    //
    // We keep alpha0 in log scale
    for (int i = 0; i < outPartials->size(); i++)
    {
        Partial &p = (*outPartials)[i];
        float ampNorm =
            _scale->applyScale(_yScaleInv2, p._amp,
                               (float)MIN_AMP_DB, (float)0.0);
        float ampDbNorm =
            _scale->applyScale(_yScale, ampNorm,
                               (float)MIN_AMP_DB, (float)0.0);
        p._amp = ampDbNorm;
    }
#endif
}

bool
PartialTracker::gluePartialBarbs(const vector<float> &magns,
                                 vector<Partial> *partials)
{
    vector<Partial> &result = _tmpPartials6;
    result.resize(0);
    bool glued = false;
    
    sort(partials->begin(), partials->end(), Partial::freqLess);
    
    int idx = 0;
    while(idx < partials->size())
    {
        Partial currentPartial = (*partials)[idx];
        
        vector<Partial> &twinPartials = _tmpPartials7;
        twinPartials.resize(0);
        
        twinPartials.push_back(currentPartial);
        
        for (int j = idx + 1; j < partials->size(); j++)
        {
            const Partial &otherPartial = (*partials)[j];
            
            if (otherPartial._leftIndex == currentPartial._rightIndex)
                // This is a twin partial...
            {
                float promCur = computePeakProminence(magns,
                                                      currentPartial._peakIndex,
                                                      currentPartial._leftIndex,
                                                      currentPartial._rightIndex);
                
                float promOther = computePeakProminence(magns,
                                                        otherPartial._peakIndex,
                                                        otherPartial._leftIndex,
                                                        otherPartial._rightIndex);
                
                // Default ratio value
                // If it keeps this value, this is ok, this will be glued
                float ratio = 0.0;
                if (promOther > NL_EPS)
                {
                    ratio = promCur/promOther;
                    if ((ratio > GLUE_BARBS_AMP_RATIO) ||
                        (ratio < 1.0/GLUE_BARBS_AMP_RATIO))
                        // ... with a big amp ratio
                    {
                        // Check that the barb is "in the middle" of a side
                        // of the main partial (in height)
                        bool inTheMiddle = false;
                        bool onTheSide = false;
                        if (promCur < promOther)
                        {
                            float hf =
                                computePeakHigherFoot(magns,
                                                      currentPartial._leftIndex,
                                                      currentPartial._rightIndex);

                            
                            float lf =
                                computePeakLowerFoot(magns,
                                                     otherPartial._leftIndex,
                                                     otherPartial._rightIndex);
                            
                            if ((hf > lf) && (hf < otherPartial._amp))
                                inTheMiddle = true;
                            
                            // Check that the barb is on the right side
                            float otherLeftFoot =
                                magns.data()[otherPartial._leftIndex];
                            float otherRightFoot =
                                magns.data()[otherPartial._rightIndex];
                            if (otherLeftFoot > otherRightFoot)
                                onTheSide = true;
                            
                        }
                        else
                        {
                            float hf =
                                computePeakHigherFoot(magns,
                                                      otherPartial._leftIndex,
                                                      otherPartial._rightIndex);
                            
                            
                            float lf =
                                computePeakLowerFoot(magns,
                                                     currentPartial._leftIndex,
                                                     currentPartial._rightIndex);
                            
                            if ((hf > lf) && (hf < currentPartial._amp))
                                inTheMiddle = true;
                            
                            // Check that the barb is on the right side
                            float curLeftFoot = magns.data()[currentPartial._leftIndex];
                            float curRightFoot = magns.data()[currentPartial._rightIndex];
                            if (curLeftFoot < curRightFoot)
                                onTheSide = true;
                        }
                            
                        if (inTheMiddle && onTheSide)
                            // This tween partial is a barb ! 
                            twinPartials.push_back(otherPartial);
                    }
                }
            }
        }
        
        // Glue ?
        //
        
        if (twinPartials.size() > 1)
        {
            glued = true;
            
            // Compute glued partial
            int leftIndex = twinPartials[0]._leftIndex;
            int rightIndex = twinPartials[twinPartials.size() - 1]._rightIndex;
            
            float peakIndex = computePeakIndexAvg(magns, leftIndex, rightIndex);
            
            // For peak amp, take max amp
            float maxAmp = -NL_INF;
            for (int k = 0; k < twinPartials.size(); k++)
            {
                float amp = twinPartials[k]._amp;
                if (amp > maxAmp)
                    maxAmp = amp;
            }
            
            Partial res;
            res._leftIndex = leftIndex;
            res._rightIndex = rightIndex;
            
            // Artificial peak
            res._peakIndex = peakIndex;
            
            float peakFreq = peakIndex/(_bufferSize*0.5);
            res._freq = peakFreq;
            res._amp = maxAmp;
            
            // Kalman
            res._kf.initEstimate(res._freq);
            //res.mPredictedFreq = res.mFreq;
            
            // Do not set mPhase for now
            
            result.push_back(res);
        }
        else
            // Not twin, simply add the partial
        {
            result.push_back(twinPartials[0]);
        }
        
        // 1 or more
        idx += twinPartials.size();
    }
    
    *partials = result;
    
    return glued;
}

bool
PartialTracker::discardFlatPartial(const vector<float> &magns,
                                   int peakIndex, int leftIndex, int rightIndex)
{
    float amp = magns.data()[peakIndex];
    
    float binDiff = rightIndex - leftIndex;
    
    float coeff = binDiff/amp;
    
    bool result = (coeff > DISCARD_FLAT_PARTIAL_COEFF);
    
    return result;
}

void
PartialTracker::discardFlatPartials(const vector<float> &magns,
                                    vector<Partial> *partials)
{
    vector<Partial> &result = _tmpPartials8;
    result.resize(0);
    
    for (int i = 0; i < partials->size(); i++)
    {
        const Partial &partial = (*partials)[i];
            
        bool discard = discardFlatPartial(magns,
                                          partial._peakIndex,
                                          partial._leftIndex,
                                          partial._rightIndex);
        
        if (!discard)
            result.push_back(partial);
    }
    
    *partials = result;
}

void
PartialTracker::suppressZeroFreqPartials(vector<Partial> *partials)
{
    vector<Partial> &result = _tmpPartials9;
    result.resize(0);
    
    for (int i = 0; i < partials->size(); i++)
    {
        const Partial &partial = (*partials)[i];
        
        float peakFreq = partial._freq;
        
        // Zero frequency
        bool discard = false;
        if (peakFreq < NL_EPS)
            discard = true;
        
        if (!discard)
            result.push_back(partial);
    }
    
    *partials = result;
}

void
PartialTracker::thresholdPartialsPeakHeight(const vector<float> &magns,
                                            vector<Partial> *partials)
{
    vector<Partial> &result = _tmpPartials10;
    result.resize(0);
    
    for (int i = 0; i < partials->size(); i++)
    {
        const Partial &partial = (*partials)[i];
        
        float height = computePeakHeight(magns,
                                         partial._peakIndex,
                                         partial._leftIndex,
                                         partial._rightIndex);
        
        // Just in case
        if (height < 0.0)
            height = 0.0;
        
        // Threshold
        
        int binNum = partial._freq*_bufferSize*0.5;
        float thrsNorm = getThreshold(binNum);
        
        if (height >= thrsNorm)
            result.push_back(partial);
    }
    
    *partials = result;
}

// Prominence
float
PartialTracker::computePeakProminence(const vector<float> &magns,
                                      int peakIndex, int leftIndex, int rightIndex)
{
    // Compute prominence
    //
    // See: https://www.mathworks.com/help/signal/ref/findpeaks.html
    //
    float maxFootAmp = magns.data()[leftIndex];
    if (magns.data()[rightIndex] > maxFootAmp)
        maxFootAmp = magns.data()[rightIndex];
    
    float peakAmp = magns.data()[peakIndex];
    
    float prominence = peakAmp - maxFootAmp;
    
    return prominence;
}

// Inverse of prominence
float
PartialTracker::computePeakHeight(const vector<float> &magns,
                                  int peakIndex, int leftIndex, int rightIndex)
{
    // Compute height
    //
    // See: https://www.mathworks.com/help/signal/ref/findpeaks.html
    //
    float minFootAmp = magns.data()[leftIndex];
    if (magns.data()[rightIndex] < minFootAmp)
        minFootAmp = magns.data()[rightIndex];
    
    float peakAmp = magns.data()[peakIndex];
    
    float height = peakAmp - minFootAmp;
    
    return height;
}

float
PartialTracker::computePeakHigherFoot(const vector<float> &magns,
                                      int leftIndex, int rightIndex)
{
    float leftVal = magns.data()[leftIndex];
    float rightVal = magns.data()[rightIndex];
    
    if (leftVal > rightVal)
        return leftVal;
    else
        return rightVal;
}

float
PartialTracker::computePeakLowerFoot(const vector<float> &magns,
                                     int leftIndex, int rightIndex)
{
    float leftVal = magns.data()[leftIndex];
    float rightVal = magns.data()[rightIndex];
    
    if (leftVal < rightVal)
        return leftVal;
    else
        return rightVal;
}

// Better than "Simple" => do not make jumps between bins
float
PartialTracker::computePeakIndexAvg(const vector<float> &magns,
                                    int leftIndex, int rightIndex)
{
    // Pow coeff, to select preferably the high amp values
    // With 2.0, makes smoother freq change
    // With 3.0, make just a little smoother than 2.0
#define COEFF 3.0
    
    float sumIndex = 0.0;
    float sumMagns = 0.0;
    
    for (int i = leftIndex; i <= rightIndex; i++)
    {
        float magn = magns.data()[i];
        
        magn = pow(magn, COEFF);
        
        sumIndex += i*magn;
        sumMagns += magn;
    }
    
    if (sumMagns < NL_EPS)
        return 0.0;
    
    float result = sumIndex/sumMagns;
    
    return result;
}

// Parabola peak center detection
// Works well
//
// See: http://eprints.maynoothuniversity.ie/4523/1/thesis.pdf (p32)
//
// and: https://ccrma.stanford.edu/~jos/parshl/Peak_Detection_Steps_3.html#sec:peakdet
//
float
PartialTracker::computePeakIndexParabola(const vector<float> &magns,
                                         int peakIndex)
{
    if ((peakIndex - 1 < 0) || (peakIndex + 1 >= magns.size()))
        return peakIndex;
    
    // magns are in DB
    float alpha = magns.data()[peakIndex - 1];
    float beta = magns.data()[peakIndex];
    float gamma = magns.data()[peakIndex + 1];

    // Will avoid wrong negative result
    if ((beta < alpha) || (beta < gamma))
        return peakIndex;
    
    // Center
    float denom = (alpha - 2.0*beta + gamma);
    if (std::fabs(denom) < NL_EPS)
        return peakIndex;
    
    float c = 0.5*((alpha - gamma)/denom);
    
    float result = peakIndex + c;
    
    return result;
}

// Simple method
float
PartialTracker::computePeakAmpInterp(const vector<float> &magns,
                                     float peakFreq)
{
    float bin = peakFreq*_bufferSize*0.5;
    
    int prevBin = (int)bin;
    int nextBin = (int)bin + 1;
    
    if (nextBin >= magns.size())
    {
        float peakAmp = magns.data()[prevBin];
        
        return peakAmp;
    }
    
    float prevAmp = magns.data()[prevBin];
    float nextAmp = magns.data()[nextBin];
    
    float t = bin - prevBin;
    
    float peakAmp = (1.0 - t)*prevAmp + t*nextAmp;
    
    return peakAmp;
}

void
PartialTracker::computePeakMagnPhaseInterp(const vector<float> &magns,
                                           const vector<float> &uwPhases,
                                           float peakFreq,
                                           float *peakAmp, float *peakPhase)
{
    // Phases are unwrapped here
    
    float bin = peakFreq*_bufferSize*0.5;
    
    int prevBin = (int)bin;
    int nextBin = (int)bin + 1;
    
    if (nextBin >= magns.size())
    {
        *peakAmp = magns.data()[prevBin];
        *peakPhase = uwPhases.data()[prevBin];
        
        return;
    }
    
    // Interpolate
    float t = bin - prevBin;
    
    *peakAmp = (1.0 - t)*magns.data()[prevBin] + t*magns.data()[nextBin];
    *peakPhase = (1.0 - t)*uwPhases.data()[prevBin] + t*uwPhases.data()[nextBin];
}

float
PartialTracker::getThreshold(int binNum)
{
#if USE_NL_PEAK_DETECTOR
    float thrsNorm = -(MIN_AMP_DB - _threshold)/(-MIN_AMP_DB);
#else // For debugging
    const float defaultThrs = -100.0;
    float thrsNorm = -(MIN_AMP_DB - defaultThrs)/(-MIN_AMP_DB);
#endif
    
    return thrsNorm;
}

void
PartialTracker::preProcessDataX(vector<float> *data)
{        
    // Use FilterBank (avoid stairs effect)
    vector<float> &scaledData = _tmpBuf8;
    Scale::FilterBankType type = _scale->typeToFilterBankType(_xScale);
    _scale->applyScaleFilterBank(type, &scaledData, *data,
                                 _sampleRate, data->size());
    *data = scaledData;
}

void
PartialTracker::preProcessDataY(vector<float> *data)
{
    // Y
    _scale->applyScaleForEach(_yScale, data, (float)MIN_AMP_DB, (float)0.0);

#if USE_A_WEIGHTING
    // Better tracking on high frequencies with this!
    preProcessAWeighting(data, true);
#endif
}

void
PartialTracker::preProcessDataXY(vector<float> *data)
{
    // Process Y first
    preProcessDataY(data);
    preProcessDataX(data);
}
    
// Unwrap phase before converting to mel
void
PartialTracker::preProcess(vector<float> *magns,
                           vector<float> *phases)
{
    // Use time smooth on raw magns too
    // (time smoothed, but linearly scaled)
    _linearMagns = *magns;
    preProcessDataY(&_linearMagns); // We want raw data in dB (just keep linear on x)
    
#if USE_QIFFT_YLOG
    _logMagns = *magns;
    _scale->applyScaleForEach(_yScale2, &_logMagns);
#endif
    
    preProcessDataXY(magns);
    
#if MEL_UNWRAP_PHASES
    Utils::unwrapPhases(phases);
#endif

    // Phases

    vector<float> &scaledPhases = _tmpBuf9;
    Scale::FilterBankType type = _scale->typeToFilterBankType(_xScale);
    _scale->applyScaleFilterBank(type, &scaledPhases, *phases,
                                 _sampleRate, phases->size());
    *phases = scaledPhases;
}

void
PartialTracker::setTimeSmoothCoeff(float coeff)
{
    _timeSmoothCoeff = coeff;
}

void
PartialTracker::denormPartials(vector<Partial> *partials)
{
    float hzPerBin =  _sampleRate/_bufferSize;
    
    for (int i = 0; i < partials->size(); i++)
    {
        Partial &partial = (*partials)[i];
        
        // Reverse Mel
        float freq = partial._freq;
        freq = _scale->applyScale(_xScaleInv, freq, (float)0.0,
                                  (float)(_sampleRate*0.5));
        partial._freq = freq;
        
        // Convert to real freqs
        partial._freq *= _sampleRate*0.5;

#if !USE_QIFFT_YLOG

#if USE_A_WEIGHTING
        // Reverse AWeighting
        int binNum = partial._freq/hzPerBin;
        partial.mAmp = processAWeighting(binNum, _bufferSize*0.5,
                                         partial._amp, false);
#endif
        
        // Y
        partial._amp = _scale->applyScale(_yScaleInv, partial._amp,
                                          (float)MIN_AMP_DB, (float)0.0);
#endif
#if USE_QIFFT_YLOG
        // Y
        partial._amp = _scale->applyScale(_yScaleInv, partial._amp,
                                          (float)MIN_AMP_DB, (float)0.0);
#endif

#if DENORM_PARTIAL_INDICES
        partial._leftIndex = denormBinIndex(partial._leftIndex);
        partial._peakIndex = denormBinIndex(partial._peakIndex);
        partial._rightIndex = denormBinIndex(partial._rightIndex);
#endif
    }
}

void
PartialTracker::denormData(vector<float> *data)
{
    // Use FilterBank internally (avoid stairs effect)
    vector<float> &scaledData = _tmpBuf7;
    Scale::FilterBankType type = _scale->typeToFilterBankType(_xScale);
    _scale->applyScaleFilterBankInv(type, &scaledData, *data,
                                    _sampleRate, data->size());
    *data = scaledData;

#if USE_A_WEIGHTING
    // A-Weighting
    preProcessAWeighting(data, false);
#endif
    
    _scale->applyScaleForEach(_yScaleInv, data, (float)MIN_AMP_DB, (float)0.0);
}

void
PartialTracker::partialsAmpToAmpDB(vector<Partial> *partials)
{
    for (int i = 0; i < partials->size(); i++)
    {
        Partial &partial = (*partials)[i];
        
        partial._ampDB = Utils::ampToDB(partial._amp);
    }
}

float
PartialTracker::partialScaleToQIFFTScale(float ampDbNorm)
{
    float amp =
        _scale->applyScale(_yScaleInv, ampDbNorm,
                           (float)MIN_AMP_DB, (float)0.0);

    float ampLog =
        _scale->applyScale(_yScale2, amp,
                           (float)MIN_AMP_DB, (float)0.0);

    return ampLog;
}

float
PartialTracker::QIFFTScaleToPartialScale(float ampLog)
{
    float amp =
        _scale->applyScale(_yScaleInv2, ampLog,
                           (float)MIN_AMP_DB, (float)0.0);
    
    float ampDbNorm =
        _scale->applyScale(_yScale, amp,
                           (float)MIN_AMP_DB, (float)0.0);
    
    return ampDbNorm;
}

void
PartialTracker::setNeriDelta(float delta)
{
#if USE_PARTIAL_FILTER_AMFM
    ((PartialFilterAMFM *)_partialFilter)->setNeriDelta(delta);
#endif
}

void
PartialTracker::preProcessAWeighting(vector<float> *magns,
                                     bool reverse)
{
    // Input magns are in normalized dB
    
    vector<float> &weights = _tmpBuf6;
    weights = _aWeights;
    
    float hzPerBin = 0.5*_sampleRate/magns->size();
    
    // W-Weighting property: 0dB at 1000Hz!
    float zeroDbFreq = 1000.0;
    int zeroDbBin = zeroDbFreq/hzPerBin;
    
    for (int i = zeroDbBin; i < magns->size(); i++)
    {
        float a = weights.data()[i];
        
        float normDbMagn = magns->data()[i];
        float dbMagn = (1.0 - normDbMagn)*MIN_AMP_DB;
        
        if (reverse)
            dbMagn -= a;
        else
            dbMagn += a;
        
        normDbMagn = 1.0 - dbMagn/MIN_AMP_DB;
        
        if (normDbMagn < 0.0)
            normDbMagn = 0.0;
        if (normDbMagn > 1.0)
            normDbMagn = 1.0;
        
        magns->data()[i] = normDbMagn;
    }
}

float
PartialTracker::processAWeighting(int binNum, int numBins,
                                  float magn, bool reverse)
{
    // Input magn is in normalized dB
    
    float hzPerBin = 0.5*_sampleRate/numBins;
    
    // W-Weighting property: 0dB at 1000Hz!
    float zeroDbFreq = 1000.0;
    int zeroDbBin = zeroDbFreq/hzPerBin;
    
    if (binNum <= zeroDbBin)
        // Do nothing
        return magn;
    
    float a = _aWeights.data()[binNum];
    
    float normDbMagn = magn;
    float dbMagn = (1.0 - normDbMagn)*MIN_AMP_DB;
        
    if (reverse)
        dbMagn -= a;
    else
        dbMagn += a;
        
    normDbMagn = 1.0 - dbMagn/MIN_AMP_DB;
        
    if (normDbMagn < 0.0)
        normDbMagn = 0.0;
    if (normDbMagn > 1.0)
        normDbMagn = 1.0;
        
    return normDbMagn;
}

// Pre-compute a weights
void
PartialTracker::computeAWeights(int numBins, float sampleRate)
{
    AWeighting::computeAWeights(&_aWeights, numBins, sampleRate);
}

void
PartialTracker::computePartials(const vector<PeakDetector::Peak> &peaks,
                                const vector<float> &magns,
                                const vector<float> &phases,
                                vector<Partial> *partials)
{
    partials->resize(peaks.size());

    vector<float> &phasesUW = _tmpBuf10;
    phasesUW = phases;
    
    PhasesUnwrapper::unwrapPhasesFreq(&phasesUW);
    
    for (int i = 0; i < peaks.size(); i++)
    {
        const PeakDetector::Peak &peak = peaks[i];
        Partial &p = (*partials)[i];

        // Indices
        p._peakIndex = peak._peakIndex;
        p._leftIndex = peak._leftIndex;
        p._rightIndex = peak._rightIndex;

        
#if !USE_QIFFT
        float peakIndexF = computePeakIndexParabola(magns, p._peakIndex);
#else
        QIFFT::Peak qifftPeak;
        QIFFT::findPeak(magns, phasesUW, _bufferSize, peak._peakIndex, &qifftPeak);
        // QIFFT::FindPeak2() is not fixed yet...

        // Apply empirical coeffs, so that next values will match
        // current values + deta.
        //
        // NOTE: this may depend on window type, maybe on overlap too..
        qifftPeak._alpha0 *= EMPIR_ALPHA0_COEFF;
        qifftPeak._beta0 *= EMPIR_BETA0_COEFF;
            
        p._binIdxF = qifftPeak._binIdx;
        p._freq = qifftPeak._freq;
        p._amp = qifftPeak._amp;
        
        p._phase = qifftPeak._phase;
        p._alpha0 = qifftPeak._alpha0;
        p._beta0 = qifftPeak._beta0;

        float peakIndexF = qifftPeak._binIdx;
#endif
        
        p._peakIndex = round(peakIndexF);
        if (p._peakIndex < 0)
            p._peakIndex = 0;
            
        if (p._peakIndex > magns.size() - 1)
            p._peakIndex = magns.size() - 1;

#if !USE_QIFFT
        // Remainder: freq is normalized here
        float peakFreq = peakIndexF/(_bufferSize*0.5);
        p._freq = peakFreq;
#endif
        
        // Kalman
        //
        // Update the estimate with the first value
        p._kf.initEstimate(p._freq);

#if !USE_QIFFT
        
#if !INTERPOLATE_PHASES
        // Magn
        p._amp = computePeakAmpInterp(magns, peakFreq);
        
        // Phase
        p._phase = phases.data()[(int)peakIndexF];
#else
        computePeakMagnPhaseInterp(magns, phases, peakFreq,
                                   &p._amp, &p._phase);
#endif

#endif
    }
}

int
PartialTracker::denormBinIndex(int idx)
{
    float freq = ((float)idx)/(_bufferSize*0.5);
    freq = _scale->applyScale(_xScaleInv, freq, (float)0.0,
                              (float)(_sampleRate*0.5));

    float res = freq*(_bufferSize*0.5);

    int resi = round(res);
    if (resi < 0)
        resi = 0;
    if (resi > _bufferSize*0.5 - 1)
        resi = _bufferSize*0.5 - 1;

    return resi;
}

void
PartialTracker::postProcessPartials(const vector<float> &magns,
                                    vector<Partial> *partials)
{
    suppressZeroFreqPartials(partials);

    // Some operations
#if GLUE_BARBS
    vector<Partial> &prev = _tmpPartials1;
    prev = *partials;
    
    gluePartialBarbs(magns, partials);
#endif

#if DISCARD_FLAT_PARTIAL
    discardFlatPartials(magns, partials);
#endif

    // Threshold
    thresholdPartialsPeakHeight(magns, partials);
}
