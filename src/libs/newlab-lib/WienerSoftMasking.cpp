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

#include "Defines.h"
#include "Utils.h"
#include "Window.h"
#include "WienerSoftMasking.h"

// Optimization
#define USE_FAKE_MASK1 1

WienerSoftMasking::HistoryLine::HistoryLine()
{
    _size = 0;
}

WienerSoftMasking::HistoryLine::HistoryLine(const HistoryLine &other)
{
    _size = other._size;

    resize(_size);
    
    _sum = other._sum;
    _masked0Square = other._masked0Square;
    _masked1Square = other._masked1Square;
}

WienerSoftMasking::HistoryLine::~HistoryLine() {}

void
WienerSoftMasking::HistoryLine::resize(int size)
{
    _size = size;
    
    _sum.resize(_size);
    _masked0Square.resize(_size);
    _masked1Square.resize(_size);
}

int
WienerSoftMasking::HistoryLine::getSize()
{
    return _size;
}

WienerSoftMasking::WienerSoftMasking(int bufferSize, int overlap, int historySize)
{
    _bufferSize = bufferSize;
    _overlap = overlap;
    
    _historySize = historySize;
    
    _processingEnabled = true;
}

WienerSoftMasking::~WienerSoftMasking() {}

void
WienerSoftMasking::reset(int bufferSize, int overlap)
{
    _bufferSize = bufferSize;
    _overlap = overlap;
    
    reset();
}

void
WienerSoftMasking::reset()
{    
    _history.resize(0);
}

void
WienerSoftMasking::setHistorySize(int size)
{
    _historySize = size;
    
    reset();
}

int
WienerSoftMasking::getHistorySize()
{
    return _historySize;
}

void
WienerSoftMasking::setProcessingEnabled(bool flag)
{
    _processingEnabled = flag;
}

bool
WienerSoftMasking::isProcessingEnabled()
{
    return _processingEnabled;
}

int
WienerSoftMasking::getLatency()
{
    // In history, we push_back() and pop_front()
    // and we take the index historySize/2
    //

    // The index where we get the data (from the end)
    // (this covers the case of odd and even history size)
    // Index 0 has 0 latency, since we have just added the current data to it.
    int revIndex = (_historySize - 1) - _historySize/2;
    int latency = revIndex*(((_bufferSize - 1)*2)/_overlap);
    
    return latency;
}

// Process over time
//
// Algo:

// s = input * HM
// n = input * (1.0 - HM)
// SM = s2(s)/(s2(s) + s2(n))
// output = input * SM
//
void
WienerSoftMasking::processCentered(vector<complex<float> > *ioSum,
                                   const vector<float> &mask,
                                   vector<complex<float> > *ioMaskedResult0,
                                   vector<complex<float> > *ioMaskedResult1)
{

    HistoryLine &newHistoLine = _tmpHistoryLine;
    newHistoLine.resize(ioSum->size());

    newHistoLine._sum = *ioSum;

    // Optim: compute square history only if enabled
    // Otherwise, fill with zeros
    if (_processingEnabled)
    {
        // masked0 = sum*mask
        newHistoLine._masked0Square = *ioSum;
        Utils::multBuffers(&newHistoLine._masked0Square, mask);

        // maskd1 = sum - masked0
        // same as: masked1 = sum*(1 - mask)
        newHistoLine._masked1Square = *ioSum;
        Utils::substractBuffers(&newHistoLine._masked1Square,
                                newHistoLine._masked0Square);
        
        // See: https://hal.inria.fr/hal-01881425/document
        // |x|^2
        // NOTE: square abs => complex conjugate
        
        // Compute squares (using complex conjugate)
        Utils::computeSquareConjugate(&newHistoLine._masked0Square);
        Utils::computeSquareConjugate(&newHistoLine._masked1Square);
    }
    else // Not enabled, fill history with zeros
    {
        newHistoLine._masked0Square.resize(ioSum->size());
        Utils::fillZero(&newHistoLine._masked0Square);
        
        newHistoLine._masked1Square.resize(ioSum->size());
        Utils::fillZero(&newHistoLine._masked1Square);
    }
    
    // Manage the history
    if (_history.empty())
    {
        // Fill the whole history with the first line
        _history.resize(_historySize);
        _history.clear(newHistoLine);
    }
    else
    {
        _history.freeze();
        _history.push_pop(newHistoLine);
    }
    
    if (_processingEnabled)
    {
        vector<complex<float> > &sigma2Mask0 = _tmpBuf0;
        vector<complex<float> > &sigma2Mask1 = _tmpBuf1;
        computeSigma2(&sigma2Mask0, &sigma2Mask1);

        complex<float> *s0Data = sigma2Mask0.data();
        complex<float> *s1Data = sigma2Mask1.data();
        
        // Create the mask
        if (_history.empty()) // Just in case
            return;

        vector<complex<float> > &softMask0 = _tmpBuf4;
        softMask0.resize(_history[0].getSize());

        int softMask0Size = softMask0.size();
        complex<float> *softMask0Data = softMask0.data();
        
        // Compute soft mask 0
        complex<float> csum;
        complex<float> maskVal;
        for (int i = 0; i < softMask0Size; i++)
        {
            const complex<float> &s0 = s0Data[i];
            const complex<float> &s1 = s1Data[i];

            // Compute s0 + s1
            csum = s0;
            csum += s1;

            maskVal = complex<float>(0.0, 0.0);

            if ((std::fabs(csum.real()) > NL_EPS) ||
                (std::fabs(csum.imag()) > NL_EPS))
                maskVal = s0 / csum;
            
            float maskMagn = abs(maskVal);

            // Limit to 1
            if (maskMagn >  1.0)
            {
                float maskMagnInv = 1.0/maskMagn;
                maskVal *= maskMagnInv;
            }
            
            softMask0Data[i] = maskVal;
        }

        // Result when enabled
        
        // Apply mask 0
        *ioMaskedResult0 = _history[_history.size()/2]._sum;
        Utils::multBuffers(ioMaskedResult0, softMask0);

        // Mask 1
        if (ioMaskedResult1 != NULL)
        {
#if USE_FAKE_MASK1
            // Simple difference
            *ioMaskedResult1 = _history[_history.size()/2]._sum;;
            Utils::substractBuffers(ioMaskedResult1, *ioMaskedResult0);
#else
            // Use real mask for second mask
            
            vector<complex<float> > &softMask1 = _tmpBuf5;
            softMask1.resize(_history[0].size());

            int softMask1Size = softMask1.size();
            complex<float> *softMask1Data = softMask1.data();
        
            // Compute soft mask 1
            complex<float> csum;
            complex<float> maskVal;
            for (int i = 0; i < softMask1Size; i++)
            {
                const complex<float> &s0 = s0Data[i];
                const complex<float> &s1 = s1Data[i];
                
                // Compute s0 + s1
                csum = s0;
                csum += s1;
        
                if ((fabs(csum.real()) > NL_EPS) ||
                    (fabs(csum.imag()) > NL_EPS))
                    maskVal = s1 / sum;

                BL_float maskMagn = abs(maskVal);

                // Limit to 1
                if (maskMagn >  1.0)
                {
                    float maskMagnInv = 1.0/maskMagn;
                    maskVal *= maskMagnInv;
                }
                    
                softMask1Data[i] = maskVal;
            }

            // Apply mask 1
            *ioMaskedResult1 = _history[_history.size()/2]._sum;
            Utils::MultBuffers(ioMaskedResult1, softMask1);
#endif
        }
    }
    
    // Update the data from the history even if processing enabled is false,
    
    // Compute the centered values
    if (!_history.empty())
        // Shifted input data
        *ioSum = _history[_history.size()/2]._sum;
}

// Variance is equal to sigma^2
void
WienerSoftMasking::computeSigma2(vector<complex<float> > *outSigma2Mask0,
                                 vector<complex<float> > *outSigma2Mask1)
{    
    if (_history.empty())
        return;
    
    outSigma2Mask0->resize(_history[0].getSize());
    outSigma2Mask1->resize(_history[0].getSize());
    
    // Result sum 0
    vector<complex<float> > &currentSum0 = _tmpBuf2;
    currentSum0.resize(_history[0].getSize());
    Utils::fillZero(&currentSum0);

    complex<float> *currentSum0Data = currentSum0.data();
    
    // Result sum 1
    vector<complex<float> > &currentSum1 = _tmpBuf3;
    currentSum1.resize(_history[0].getSize());
    Utils::fillZero(&currentSum1);

    complex<float> *currentSum1Data = currentSum1.data();
    
    // Window
    if (_window.size() != _history.size())
    {
        _window.resize(_history.size());
        Window::makeWindowHann(&_window);
    }
    
    float sumProba = Utils::computeSum(_window);
    float sumProbaInv = 0.0;
    if (sumProba > NL_EPS)
        sumProbaInv = 1.0/sumProba;
    
    for (int j = 0; j < _history.size(); j++)
    {
        const HistoryLine &line = _history[j];
        
        const vector<complex<float> > &line0 = line._masked0Square;
        int line0Size = line0.size();
        const complex<float> *line0Data = line0.data();
        
        const vector<complex<float> > &line1 = line._masked1Square;
        const complex<float> *line1Data = line1.data();
        
        float p = _window[j];
        
        for (int i = 0; i < line0Size; i++)
        {
            complex<float> &expect0 = currentSum0Data[i];
            const complex<float> &val0 = line0Data[i];
            expect0 += p * val0;

            complex<float> &expect1 = currentSum1Data[i];
            const complex<float> &val1 = line1Data[i];
            expect1 += p * val1;
        }
    }

    // Divide by sum probas
    if (sumProba > NL_EPS)
    {
        Utils::multValue(&currentSum0, sumProbaInv);
        Utils::multValue(&currentSum1, sumProbaInv);
    }
    
    // Result
    *outSigma2Mask0 = currentSum0;
    *outSigma2Mask1 = currentSum1;
}
