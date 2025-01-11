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

#ifndef QIFFT_H
#define QIFFT_H

// Empirical alpha0 additional coeff
#define EMPIR_ALPHA0_COEFF 1.422865
    
// Empirical beta0 additional coeff
#define EMPIR_BETA0_COEFF 0.0030370

// See: https://ccrma.stanford.edu/files/papers/stanm118.pdf
// And (p32): http://mural.maynoothuniversity.ie/4523/1/thesis.pdf
// And: https://ccrma.stanford.edu/~jos/parshl/Peak_Detection_Steps_3.html#sec:peakdet
//
// NOTE: for alpha0 and beta0, the fft must have used a window
// (Gaussian or Hann or other)
//
// NOTE: this will work for amplitude only if either
// - We use a Gaussian window instead of Hann
// - Or we use a fft zero padding factor of x2
//
class QIFFT
{
 public:
    struct Peak
    {
        // True peak idx, in floating point format
        float _binIdx;

        float _freq;
        
        // Amp for true peak, in dB
        float _amp;

        // Phase for true peak
        float _phase;

        // Amp derivative over time
        float _alpha0;

        // Freq derivative over time
        float _beta0;
    };

    // NOTE: FindPeak() and FindPeak2() give the same results,
    // modulo some scaling coefficient
    
    // Magns should be in dB!
    
    // Custom method
    static void findPeak(const vector<float> &magns,
                         const vector<float> &phases,
                         int bufferSize,
                         int peakBin, Peak *result);
    
 protected:
    // Parabola equation: y(x) = a*(x - c)^2 + b
    // Specific to peak tracking
    static void getParabolaCoeffs(float alpha, float beta, float gamma,
                                  float *a, float *b, float *c);

    static float parabolaFunc(float x, float a, float b, float c);

    // Parabola equation: y(x) = a*x^2 + b*x + c
    // Generalized (no maximum constraint)
    static void getParabolaCoeffsGen(float alpha, float beta, float gamma,
                                     float *a, float *b, float *c);

    static float parabolaFuncGen(float x, float a, float b, float c);
};

#endif
