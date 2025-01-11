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

#ifndef PARTIAL_TRACKER_H
#define PARTIAL_TRACKER_H

#include <vector>
using namespace std;

#include "PeakDetector.h"

#include <Partial.h>

#include "IPlug_include_in_plug_hdr.h"

#include "../../WDL/fft.h"

// Use PeakDetector class, original NewLab implementation
#define USE_NL_PEAK_DETECTOR 0

// Use smart peak detection from http://billauer.co.il/peakdet.html
// See also: https://github.com/xuphys/peakdetect/blob/master/peakdetect.c
#define USE_BILLAUER_PEAK_DETECTOR 1

#define USE_PARTIAL_FILTER_MARCHAND 0
#define USE_PARTIAL_FILTER_AMFM 1

#if USE_NL_PEAK_DETECTOR
#define DETECT_PARTIALS_START_INDEX 2
#else
// Better, in order to not discard low freq peaks with Billauer real prominence
#define DETECT_PARTIALS_START_INDEX 0
#endif

class PartialFilter;
class Scale;
class PartialTracker
{
public:
    PartialTracker(int bufferSize, float sampleRate);
    
    virtual ~PartialTracker();
    
    void reset();
    
    void reset(int bufferSize, float sampleRate);
    
    float getMinAmpDB();
    
    void setThreshold(float threshold);
    void setThreshold2(float threshold2);

    // Magn/phase
    void setData(const vector<float> &magns,
                 const vector<float> &phases);
    
    void getPreProcessedMagns(vector<float> *magns);
    
    void detectPartials();
    void filterPartials();
    
    void getPartials(vector<Partial> *partials);

    // For getting current partials before filtering
    void getRawPartials(vector<Partial> *partials);
    
    void clearResult();
    
    // Maximum frequency we try to detect
    void setMaxDetectFreq(float maxFreq);
    
    // Preprocess time smooth
    void setTimeSmoothCoeff(float coeff);
    
    // For processing result warping for example
    void preProcessDataX(vector<float> *data);

    void preProcessDataY(vector<float> *data);
    
    // For processing result color for example, just before display
    void preProcessDataXY(vector<float> *data);
    
    // Unwrap phases for interpolatin in Mel
    void preProcessUnwrapPhases(vector<float> *magns,
                                vector<float> *phases);
    
    void denormPartials(vector<Partial> *partials);
    void denormData(vector<float> *data);
    
    void partialsAmpToAmpDB(vector<Partial> *partials);

    BL_FLOAT partialScaleToQIFFTScale(float ampDbNorm);
    BL_FLOAT QIFFTScaleToPartialScale(float ampLog);

    void setNeriDelta(float delta);
    
protected:
    // Pre processing
    
    void preProcess(vector<float> *magns,
                    vector<float> *phases);
    
    // Do it in the complex domain (to be compatible with AM/FM parameters)
    void preProcessTimeSmooth(vector<complex<float> > *data);
    
    // Apply A-Weighting, so the peaks at highest frequencies will not be small
    void preProcessAWeighting(vector<float> *magns, bool reverse = false);
    
    float processAWeighting(int binNum, int numBins,
                            float magn, bool reverse);


    void computePartials(const vector<PeakDetector::Peak> &peaks,
                         const vector<float> &magns,
                         const vector<float> &phases,
                         vector<Partial> *partials);

    void removeRealDeadPartials(vector<Partial> *partials);
        
    // Detection
    
    void detectPartials(const vector<float> &magns,
                        const vector<float> &phases,
                        vector<Partial> *partials);
    
    // Peak frequency computation
    
    float computePeakIndexAvg(const vector<float> &magns,
                              int leftIndex, int rightIndex);
    
    float computePeakIndexParabola(const vector<float> &magns,
                                   int peakIndex);
    
    // Peak amp
    
    float computePeakAmpInterp(const vector<float> &magns,
                               float peakFreq);
    
    void computePeakMagnPhaseInterp(const vector<float> &magns,
                                    const vector<float> &unwrappedPhases,
                                    float peakFreq,
                                    float *peakAmp, float *peakPhase);
    
    
    // Glue the barbs to the main partial
    // Return true if some barbs have been glued
    bool gluePartialBarbs(const vector<float> &magns,
                          vector<Partial> *partials);
     
    // Discard partials which are almost flat
    // (compare height of the partial, and width in the middle)
    bool discardFlatPartial(const vector<float> &magns,
                            int peakIndex, int leftIndex, int rightIndex);
    
    void discardFlatPartials(const vector<float> &magns,
                             vector<Partial> *partials);
    
    // Suppress partials with zero frequencies
    void suppressZeroFreqPartials(vector<Partial> *partials);

    float computePeakHeight(const vector<float> &magns,
                            int peakIndex, int leftIndex, int rightIndex);
    void thresholdPartialsPeakHeight(const vector<float> &magns,
                                     vector<Partial> *partials);
    
    // Peaks
    
    float computePeakProminence(const vector<float> &magns,
                                int peakIndex, int leftIndex, int rightIndex);

    float computePeakHigherFoot(const vector<float> &magns,
                                int leftIndex, int rightIndex);

    float computePeakLowerFoot(const vector<float> &magns,
                               int leftIndex, int rightIndex);
    
    // Adaptive threshold, depending on bin num;
    float getThreshold(int binNum);

    // Optim: pre-compute a weights
    void computeAWeights(int numBins, float sampleRate);
        
    int denormBinIndex(int idx);

    void postProcessPartials(const vector<float> &magns,
                             vector<Partial> *partials);
    
    int _bufferSize;
    float _sampleRate;
    
    float _threshold;
    
    vector<float> _currentMagns;
    vector<float> _currentPhases;

    vector<float> _linearMagns;
    vector<float> _logMagns;
    
    vector<Partial> _result;
    
    float _maxDetectFreq;
    
    // For Pre-Process
    float _timeSmoothCoeff;
        
    // Smooth only magns
    WDL_TypedBuf<BL_FLOAT> _prevMagns;
    
    // Scales
    Scale *_scale;
    Scale::Type _xScale;
    Scale::Type _yScale;
    Scale::Type _yScale2; // For log
    
    Scale::Type _xScaleInv;
    Scale::Type _yScaleInv;
    Scale::Type _yScaleInv2; // for log

    // Optim
    vertor<float> _aWeights;

    PeakDetector *_peakDetector;
    PartialFilter *_partialFilter;
    
private:
    // Tmp buffers
    vector<float> _tmpBuf0;
    vector<float> _tmpBuf1;
    vector<float> _tmpBuf2;
    vector<float> _tmpBuf3;
    vector<float> _tmpBuf4;
    vector<float> _tmpBuf5;
    vector<float> _tmpBuf6;
    vector<float> _tmpBuf7;
    vector<float> _tmpBuf8;
    vector<float> _tmpBuf9;
    vector<float> _tmpBuf10;
    vector<complex<float> > _tmpBuf11;
    
    vector<Partial> _tmpPartials0;
    vector<Partial> _tmpPartials1;
    vector<Partial> _tmpPartials2;
    vector<Partial> _tmpPartials3;
    vector<Partial> _tmpPartials4;
    vector<Partial> _tmpPartials5;
    vector<Partial> _tmpPartials6;
    vector<Partial> _tmpPartials7;
    vector<Partial> _tmpPartials8;
    vector<Partial> _tmpPartials9;
    vector<Partial> _tmpPartials10;
    vector<Partial> _tmpPartials11;
};

#endif
