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

#ifndef PARTIAL_TRACKER_H
#define PARTIAL_TRACKER_H

#include <deque>
using namespace std;

#include "bl_queue.h"
#include "Scale.h"
#include "KalmanFilter.h"

class PartialTracker
{
public:
    // class Partial
    class Partial
    {
    public:
        enum State
        {
            ALIVE,
            ZOMBIE,
            DEAD
        };
        
        Partial();
        
        Partial(const Partial &other);
        
        virtual ~Partial();
        
        void genNewId();
        
        //
        static bool freqLess(const Partial &p1, const Partial &p2);
        
        static bool ampLess(const Partial &p1, const Partial &p2);
        
        static bool idLess(const Partial &p1, const Partial &p2);
        
        static bool cookieLess(const Partial &p1, const Partial &p2);
        
    public:
        int _peakIndex;
        int _leftIndex;
        int _rightIndex;
        
        // When detecting and filtering, mFreq and mAmp are "scaled and normalized"
        // After processing, we can compute the real frequencies in Hz and amp in dB.
        float _freq;
        union{
            // Inside PartialTracker5
            float _amp;
            
            // After, outside PartialTracker5, if external classes need amp in dB
            // Need to call DenormPartials() then PartialsAmpToAmpDB()
            float _ampDB;
        };
        float _phase;
        
        float _peakHeight;
        
        long _id;
        
        enum State _state;
        
        bool _wasAlive;
        long _zombieAge;
        
        long _age;
        
        // All-purpose field
        float _cookie;
        
        KalmanFilter _kf;
        float _predictedFreq;
        
    protected:
        static unsigned long _currentId;
    };
    
    PartialTracker(int bufferSize, float sampleRate);
    
    virtual ~PartialTracker();
    
    void reset();
    
    void reset(int bufferSize, float sampleRate);

    void setComputeAccurateFreqs(bool flag);
    
    float getMinAmpDB();
    
    void setThreshold(float threshold);
    
    // Magn/phase
    void setData(const vector<float> &magns,
                 const vector<float> &phases);
    
    void getPreProcessedMagns(vector<float> *magns);
    
    void detectPartials();
    void extractNoiseEnvelope();
    void filterPartials();
    
    void getPartials(vector<Partial> *partials);

    // For getting current partials before filtering
    void getPartialsRAW(vector<Partial> *partials);
    
    void clearResult();
    
    void getNoiseEnvelope(vector<float> *noiseEnv);
    void getHarmonicEnvelope(vector<float> *harmoEnv);
    
    // Maximum frequency we try to detect (limit for BL-Infra for example)
    void setMaxDetectFreq(float maxFreq);
    
    // Preprocess time smoth
    void setTimeSmoothCoeff(float coeff);
    
    void setTimeSmoothNoiseCoeff(float coeff);
    
    // For processing result warping for example
    void preProcessDataX(vector<float> *data);

    //
    void preProcessDataY(vector<float> *data);
    
    // For processing result color for example, just before display
    void preProcessDataXY(vector<float> *data);
    
    // Unwrap phases for interpolatin in Mel
    void preProcessUnwrapPhases(vector<float> *magns,
                                vector<float> *phases);
    
    void denormPartials(vector<PartialTracker::Partial> *partials);
    void denormData(vector<float> *data);
    
    void partialsAmpToAmpDB(vector<PartialTracker::Partial> *partials);
    
protected:
    // Pre process
    
    void preProcess(vector<float> *magns, vector<float> *phases);
    
    // Apply time smooth (removes the noise and make more neat peaks)
    void preProcessTimeSmooth(vector<float> *magns);
    
    // Apply A-Weighting, so the peaks at highest frequencies will not be small
    void preProcessAWeighting(vector<float> *magns, bool reverse = false);
    
    float processAWeighting(int binNum, int numBins,
                            float magn, bool reverse);

    
    // Get the partials which are alive
    bool getAlivePartials(vector<Partial> *partials);
    
    void removeRealDeadPartials(vector<Partial> *partials);
    
    // Detection
    
    void detectPartials(const vector<float> &magns,
                        const vector<float> &phases,
                        vector<Partial> *partials);
    
    // Peak frequency computation
    
    float computePeakIndexAvg(const vector<float> &magns,
                              int leftIndex, int rightIndex);
    float computePeakIndexAvgSimple(const vector<float> &magns,
                                    int leftIndex, int rightIndex);
    
    float computePeakIndexParabola(const vector<float> &magns,
                                   int peakIndex);
    
    // Advanced method
    float computePeakIndexHalfProminenceAvg(const vector<float> &magns,
                                            int peakIndex,
                                            int leftIndex, int rightIndex);
    
    // Peak amp
    
    float computePeakAmpInterp(const vector<float> &magns,
                               float peakFreq);
    
    void computePeakMagnPhaseInterp(const vector<float> &magns,
                                    const vector<float> &unwrappedPhases,
                                    float peakFreq,
                                    float *peakAmp, float *peakPhase);
    
    
    // Avoid the partial foot to leak on the left and right
    // with very small amplitudes
    void narrowPartialFoot(const vector<float> &magns,
                           int peakIndex,
                           int *leftIndex, int *rightIndex);
    
    void NarrowPartialFoot(const vector<float> &magns,
                           vector<Partial> *partials);
    
    // Glue the barbs to the main partial
    // Return true if some barbs have been glued
    bool gluePartialBarbs(const vector<float> &magns,
                          vector<Partial> *partials);
    
    // Suppress the "barbs" (tiny partials on a side of a bigger partial)
    void suppressBarbs(vector<Partial> *partials);
    
    // Discard partials which are almost flat
    // (compare height of the partial, and width in the middle
    bool discardFlatPartial(const vector<float> &magns,
                            int peakIndex, int leftIndex, int rightIndex);
    
    void discardFlatPartials(const vector<float> &magns,
                             vector<Partial> *partials);
    
    bool discardInvalidPeaks(const vector<float> &magns,
                             int peakIndex, int leftIndex, int rightIndex);

    
    // Suppress partials with zero frequencies
    void suppressZeroFreqPartials(vector<Partial> *partials);
    
    void thresholdPartialsPeakHeight(vector<Partial> *partials);
    
    void timeSmoothNoise(vector<float> *noise);
    
    // Peaks
    
    // Fixed
    float computePeakProminence(const vector<float> &magns,
                                int peakIndex, int leftIndex, int rightIndex);

    // Inverse of prominence
    float computePeakHeight(const vector<float> &magns,
                            int peakIndex, int leftIndex, int rightIndex);
    
    float computePeakHeightDb(const vector<float> &magns,
                              int peakIndex, int leftIndex, int rightIndex,
                              const Partial &partial);
    
    float computePeakHigherFoot(const vector<float> &magns,
                                int leftIndex, int rightIndex);

    float computePeakLowerFoot(const vector<float> &magns,
                               int leftIndex, int rightIndex);

    // Compute for all peaks
    void computePeaksHeights(const vector<float> &magns,
                             vector<Partial> *partials);

    
    // Filter
    
    void filterPartials(vector<Partial> *result);
    
    void keepOnlyPartials(const vector<Partial> &partials,
                          vector<float> *magns);

    
    // Extract noise envelope
    
    void extractNoiseEnvelopeMax();
    
    void extractNoiseEnvelopeTrack();
    
    void extractNoiseEnvelopeSimple();
    
    
    void processMusicalNoise(vector<float> *noise);

    void thresholdNoiseIsles(vector<float> *noise);

    int findPartialById(const vector<PartialTracker::Partial> &partials, int idx);
    
    // Associate partials
    
    // Simple method, based on frequencies only
    void associatePartials(const vector<PartialTracker::Partial> &prevPartials,
                           vector<PartialTracker::Partial> *currentPartials,
                           vector<PartialTracker::Partial> *remainingPartials);
    
    // See: https://www.dsprelated.com/freebooks/sasp/PARSHL_Program.html#app:parshlapp
    // "Peak Matching (Step 5)"
    // Use fight/winner/loser
    void associatePartialsPARSHL(const vector<PartialTracker::Partial> &prevPartials,
                                 vector<PartialTracker::Partial> *currentPartials,
                                 vector<PartialTracker::Partial> *remainingPartials);

    // Adaptive threshold, depending on bin num;
    float getThreshold(int binNum);
    float getDeltaFreqCoeff(int binNum);

    // Optim: pre-compute a weights
    void computeAWeights(int numBins, float sampleRate);

    int denormBinIndex(int idx);

    void computeAccurateFreqs(vector<Partial> *partials);
    
    
    int _bufferSize;
    float _sampleRate;
    
    float _threshold;
    
    //
    vector<float> _currentMagns;
    vector<float> _currentPhases;

    vector<float> _linearMagns;
    
    deque<vector<Partial> > _partials;
    
    vector<Partial> _result;
    vector<float> _noiseEnvelope;
    vector<float> _harmonicEnvelope;
    
    vector<float> _smoothWinNoise;
    
    // For SmoothNoiseEnvelopeTime()
    vector<float> _prevNoiseEnvelope;
    
    // For ComputeMusicalNoise()
    bl_queue<vector<float> > _prevNoiseMasks;
    
    float _maxDetectFreq;
    
    // For Pre-Process
    float _timeSmoothCoeff;
    // Smooth only magns
    vector<float> _timeSmoothPrevMagns;
    
    // Scales
    Scale *_scale;
    Scale::Type _xScale;
    Scale::Type _yScale;
    
    Scale::Type _xScaleInv;
    Scale::Type _yScaleInv;
    
    // Time smooth noise
    float _timeSmoothNoiseCoeff;
    vector<float> _timeSmoothPrevNoise;

    // Optim
    vector<float> _aWeights;

    bool _computeAccurateFreqs;
    
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
    vector<Partial> _tmpPartials12;
    vector<Partial> _tmpPartials13;
    vector<Partial> _tmpPartials14;
    vector<Partial> _tmpPartials15;
    vector<Partial> _tmpPartials16;
    vector<Partial> _tmpPartials17;
    vector<Partial> _tmpPartials18;
};

#endif
