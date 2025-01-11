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

#include <BLUtilsMath.h>
#include <BLUtilsPhases.h>

#include <BLDebug.h>

#include "QIFFT.h"

#define USE_MAP_TO_PI 0 // 1

void
QIFFT::FindPeak(const WDL_TypedBuf<BL_FLOAT> &magns,
                const WDL_TypedBuf<BL_FLOAT> &phases,
                int bufferSize,
                int peakBin, Peak *result)
{
    // Here, magns are in log scale
    
    // Eps used for derivative
#define DERIV_EPS 1e-5 //1e-3 //1e-10
    
    // Default value
    result->mBinIdx = peakBin;
    // Begin with rough computation
    result->mFreq = ((BL_FLOAT)peakBin)/(bufferSize*0.5);
    result->mAmp = magns.Get()[peakBin];
    result->mPhase = phases.Get()[peakBin];
    result->mAlpha0 = 0.0;
    result->mBeta0 = 0.0;
    
    if ((peakBin - 1 < 0) || (peakBin + 1 >= magns.GetSize()))
        return;

    // Bin 1 is the first usable bin (bin 0 is the fft DC)
    // If we have a peak at bin1, we can not make parabola interploation
    if (peakBin <= 1)
        return;
    
    // Magns are in DB
    BL_FLOAT alpha = magns.Get()[peakBin - 1];
    BL_FLOAT beta = magns.Get()[peakBin];
    BL_FLOAT gamma = magns.Get()[peakBin + 1];

    // Will avoid wrong negative result in case of not true peaks
    if ((beta < alpha) || (beta < gamma))
        return;
    
   // Get parabola equation coeffs a and b
    // (we already have c)
    BL_FLOAT a;
    BL_FLOAT b;
    BL_FLOAT c;
    GetParabolaCoeffs(alpha, beta, gamma, &a, &b, &c);

    // We have the true bin!
    result->mBinIdx = peakBin + c;

    // Frequency
    result->mFreq = result->mBinIdx/(bufferSize*0.5);
        
    // We have the true amp!
    result->mAmp = beta - 0.25*(alpha - gamma)*c;

    //DBG_DumpParabola(peakBin, alpha, beta, gamma, c, magns);
                     
    // Phases
    BL_FLOAT alphaP = phases.Get()[peakBin - 1];
    BL_FLOAT betaP = phases.Get()[peakBin];
    BL_FLOAT gammaP = phases.Get()[peakBin + 1];

#if USE_MAP_TO_PI
    // Seems necessary to have good parabola
    alphaP = BLUtilsPhases::MapToPi(alphaP);
    betaP = BLUtilsPhases::MapToPi(betaP);
    gammaP = BLUtilsPhases::MapToPi(gammaP);
#endif
    
    // Get parabola equation coeffs for phases
    BL_FLOAT aP;
    BL_FLOAT bP;
    BL_FLOAT cP;
    GetParabolaCoeffsGen(alphaP, betaP, gammaP, &aP, &bP, &cP);

    //DBG_DumpParabolaGen(peakBin, aP, bP, cP, phases);
    
    // We have the true phase!
    // (should we unwrap phases before ?)
    //result->mPhase = betaP - 0.25*(alphaP - gammaP)*c;
    BL_FLOAT peakPhase = ParabolaFuncGen(c, aP, bP, cP);
    result->mPhase = peakPhase;
    
    // For alpha0 and beta0,
    // See: https://ccrma.stanford.edu/files/papers/stanm118.pdf

    // Magnitudes
    //
    
    // Magnitude derivative at mBinIdx
    BL_FLOAT a0 = ParabolaFunc(c - DERIV_EPS, a, b, c);
    BL_FLOAT a1 = ParabolaFunc(c + DERIV_EPS, a, b, c);
    BL_FLOAT up = (a1 - a0)/(2.0*DERIV_EPS);
    
    // Second derivative of magnitudes
    //

    // Derivative at n - 1
    BL_FLOAT a00 = ParabolaFunc(c - 2.0*DERIV_EPS, a, b, c);
    BL_FLOAT a10 = ParabolaFunc(c, a, b, c);
    BL_FLOAT up0 = (a10 - a00)/(2.0*DERIV_EPS);

    // Derivative at n + 1
    BL_FLOAT a01 = ParabolaFunc(c, a, b, c);
    BL_FLOAT a11 = ParabolaFunc(c + 2.0*DERIV_EPS, a, b, c);
    BL_FLOAT up1 = (a11 - a01)/(2.0*DERIV_EPS);

    // Second derivative
    BL_FLOAT upp = (up1 - up0)/(2.0*DERIV_EPS);

    // Phases
    //
    
    // Phases derivative at mBinIdx
    BL_FLOAT p0 = ParabolaFuncGen(c - DERIV_EPS, aP, bP, cP);
    BL_FLOAT p1 = ParabolaFuncGen(c + DERIV_EPS, aP, bP, cP);
    BL_FLOAT vp = (p1 - p0)/(2.0*DERIV_EPS);

    // Second derivative of phases
    //

    // Derivative at n - 1
    BL_FLOAT p00 = ParabolaFuncGen(c - 2.0*DERIV_EPS, aP, bP, cP);
    BL_FLOAT p10 = ParabolaFuncGen(c, aP, bP, cP);
    BL_FLOAT vp0 = (p10 - p00)/(2.0*DERIV_EPS);
    
    // Derivative at n + 1
    BL_FLOAT p01 = ParabolaFuncGen(c, aP, bP, cP);
    BL_FLOAT p11 = ParabolaFuncGen(c + 2.0*DERIV_EPS, aP, bP, cP);
    BL_FLOAT vp1 = (p11 - p01)/(2.0*DERIV_EPS);
    
    // Second derivative
    BL_FLOAT vpp = (vp1 - vp0)/(2.0*DERIV_EPS);

    // Then finally compute alpha0 and beta0
    //
    BL_FLOAT denom1 = (2.0*(upp*upp + vpp*vpp));
    if (std::fabs(denom1) < BL_EPS)
        return;
    
    BL_FLOAT p = -upp/denom1;

    // TEST
    //BL_FLOAT sigma = sqrt(1.0/M_E);
    //p = 1.0/(2.0*sigma*sigma);
        
#if FIND_PEAK_COMPAT
    // Adjust with a coeff, to be similar to the original paper
    BL_FLOAT N = magns.GetSize()*2;
    p *= (2.0*M_PI/N)*(2.0*M_PI/N);
#endif
    
    // Origin
    //BL_FLOAT alpha0 = -2.0*p*vp;
    
    // GOOD!
    // #bluelab: Must do a -M_PI, otherwise alpha0 will always be positive 
    BL_FLOAT alpha0 = -2.0*p*(vp - M_PI);

#if FIND_PEAK_COMPAT
    // Adjust with a coeff, to be similar to the original paper
    alpha0 *= ((BL_FLOAT)N)/M_PI;
#endif
    
    BL_FLOAT beta0 = 0.0;
    if (std::fabs(upp) > BL_EPS)
        beta0 = p*vpp/upp;
    
    // Result
    result->mAlpha0 = alpha0;
    result->mBeta0 = beta0;
}

// NOTE: this is BAD
// Error in alpha0 values, compared to QIFFT::FindPeak(), which is correct
// Don't know why... (maybe numerical precision...)
//
// If try to fix later, see the appendix of this paper:
// https://ccrma.stanford.edu/files/papers/stanm118.pdf
void
QIFFT::FindPeak2(const WDL_TypedBuf<BL_FLOAT> &magns,
                 const WDL_TypedBuf<BL_FLOAT> &phases,
                 int peakBin, Peak *result)
{
    // Default value
    result->mBinIdx = peakBin;
    result->mFreq = 0.0;
    result->mAmp = magns.Get()[peakBin];
    result->mPhase = phases.Get()[peakBin];
    result->mAlpha0 = 0.0;
    result->mBeta0 = 0.0;
        
    if ((peakBin - 1 < 0) || (peakBin + 1 >= magns.GetSize()))
        return;

    int k0 = peakBin;

    // Amps
    BL_FLOAT um1 = magns.Get()[k0 - 1];
    BL_FLOAT u0 = magns.Get()[k0];
    BL_FLOAT u1 = magns.Get()[k0 + 1];

    // Phases
    BL_FLOAT vm1 = phases.Get()[k0 - 1];
    BL_FLOAT v0 = phases.Get()[k0];
    BL_FLOAT v1 = phases.Get()[k0 + 1];
    
    if ((um1 > u0) || (u1 > u0))
    {
        // Error, peakBin was not the index of a peak
        return;
    }
    
    BL_FLOAT a =  (u1 - 2.0*u0 + um1)*0.5;
    BL_FLOAT b = (u1 - um1)*0.5;
    BL_FLOAT c = u0;
    
    BL_FLOAT d =  (v1 - 2.0*v0 + vm1)*0.5;
    BL_FLOAT e = (v1 - vm1)*0.5;
    BL_FLOAT f = v0;

    // Fft size
    BL_FLOAT N = magns.GetSize()*2;

    // Mistake in the article Appendix ?
    // ok for vibrato
    //BL_FLOAT p = -((M_PI/N)*(M_PI/N))*(d/(a*a + d*d)); // Origin paper
    
    // This gives good result for sine sweep,
    //
    // NOTE: p is exactly the same as in QIFFT::FindPeak() (which is correct)
    BL_FLOAT p = -((M_PI/N)*(M_PI/N))*(a/(a*a + d*d)); // #bluelab fix
    
    BL_FLOAT delta0 = -b/(2.0*a);

    BL_FLOAT omega0 = (2.0*M_PI/N)*(k0 + delta0);
    BL_FLOAT lambda0 = a*(delta0*delta0) + b*delta0 + c;
    BL_FLOAT phy0 = d*(delta0*delta0) + e*delta0 + f;
    // BAD!
    // NOTE: alpha0 is false, different from QIFFT::FindPeak() (which is correct)
    BL_FLOAT alpha0 = -(N/M_PI)*p*(2.0*d*delta0 + e);
    // NOTE: beta0 exactly the same as in QIFFT::FindPeak() (which is correct)
    BL_FLOAT beta0 = p*d/a;
    
    result->mBinIdx = peakBin + delta0;
    result->mFreq = omega0;
    result->mAmp = lambda0;
    result->mPhase = phy0;
    result->mAlpha0 = alpha0;
    result->mBeta0 = beta0;
}

void
QIFFT::GetParabolaCoeffs(BL_FLOAT alpha, BL_FLOAT beta, BL_FLOAT gamma,
                         BL_FLOAT *a, BL_FLOAT *b, BL_FLOAT *c)
{
    // Parabola equation: y(x) = a*(x - c)^2 + b
    // c: center
    // a: concavity
    // b: offset

    // Center
    BL_FLOAT denom0 = (alpha - 2.0*beta + gamma);
    if (std::fabs(denom0) < BL_EPS)
        return;
    
    *c = 0.5*((alpha - gamma)/denom0);
    
    // Use http://mural.maynoothuniversity.ie/4523/1/thesis.pdf
    // To make equations and find a and b
    *b = -(alpha*(*c)*(*c) - beta*(*c +1.0)*(*c + 1.0))/(2.0*(*c) + 1.0);
    *a = (alpha - *b)/((*c + 1.0)*(*c + 1.0));
}

BL_FLOAT
QIFFT::ParabolaFunc(BL_FLOAT x, BL_FLOAT a, BL_FLOAT b, BL_FLOAT c)
{
    // Parabola equation: y(x) = a*(x - c)^2 + b
    // c: center
    // a: concavity
    // b: offset
    BL_FLOAT v = a*(x - c)*(x - c) + b;

    return v;
}

// Parabola equation: y(x) = a*x^2 + b*x + c
// Generalized (no maximum constraint)
void
QIFFT::GetParabolaCoeffsGen(BL_FLOAT alpha, BL_FLOAT beta, BL_FLOAT gamma,
                            BL_FLOAT *a, BL_FLOAT *b, BL_FLOAT *c)
{
    *a = 0.5*(alpha + gamma - 2.0*beta);
    *b = gamma - 0.5*(alpha + gamma - 2.0*beta) - beta;
    *c = beta;
}

// Parabola equation: y(x) = a*x^2 + b*x + c
// Generalized (no maximum constraint)
BL_FLOAT
QIFFT::ParabolaFuncGen(BL_FLOAT x, BL_FLOAT a, BL_FLOAT b, BL_FLOAT c)
{
    BL_FLOAT v = a*x*x + b*x + c;

    return v;
}

void
QIFFT::DBG_DumpParabola(int peakBin,
                        BL_FLOAT alpha, BL_FLOAT beta, BL_FLOAT gamma,
                        BL_FLOAT c,
                        const WDL_TypedBuf<BL_FLOAT> &magns)
{
    // Parabola equation: y(x) = a*(x - c)^2 + b
    // c: center
    // a: concavity
    // b: offset
    
#define NUM_VALUES 100

    WDL_TypedBuf<BL_FLOAT> values;
    values.Resize(NUM_VALUES);
    
    BL_FLOAT start = peakBin - 2.0;
    BL_FLOAT end = peakBin + 2.0;

    // Use http://mural.maynoothuniversity.ie/4523/1/thesis.pdf
    // To make equations and find a and b
    BL_FLOAT b = -(alpha*c*c - beta*(c +1.0)*(c + 1.0))/(2.0*c + 1.0);
    BL_FLOAT a = (alpha - b)/((c + 1.0)*(c + 1.0));
    
    for (int i = 0; i < NUM_VALUES; i++)
    {
        BL_FLOAT x = start + i*(end - start)/NUM_VALUES;

        BL_FLOAT v = a*(x - (peakBin + c))*(x - (peakBin + c)) + b;
        
        values.Get()[i] = v;
    }
        
    // Mark the indices
    //
    int leftIdx = peakBin - 1;
    int leftIdx0 = (((BL_FLOAT)(leftIdx - start))/(end - start))*NUM_VALUES;
    values.Get()[leftIdx0] = magns.Get()[leftIdx]; //0.0;

    int centerIdx = peakBin;
    int centerIdx0 = (((BL_FLOAT)(centerIdx - start))/(end - start))*NUM_VALUES;
    values.Get()[centerIdx0] = magns.Get()[centerIdx]; //0.0;

    int rightIdx = peakBin + 1;
    int rightIdx0 = (((BL_FLOAT)(rightIdx - start))/(end - start))*NUM_VALUES;
    values.Get()[rightIdx0] = magns.Get()[rightIdx]; //0.0;
    
    //BLDebug::DumpData("parabola.txt", values);
}

void
QIFFT::DBG_DumpParabolaGen(int peakBin,
                           BL_FLOAT a, BL_FLOAT b, BL_FLOAT c,
                           const WDL_TypedBuf<BL_FLOAT> &phases)
{
    // Parabola equation: y(x) = a*x^2 + b*x + c
    
#define NUM_VALUES 100
#define INTERVAL 2.0
    
    WDL_TypedBuf<BL_FLOAT> values;
    values.Resize(NUM_VALUES);
    
    BL_FLOAT start = peakBin - INTERVAL;
    BL_FLOAT end = peakBin + INTERVAL;

    for (int i = 0; i < NUM_VALUES; i++)
    {
        BL_FLOAT x = 2.0*INTERVAL*((BL_FLOAT)i)/NUM_VALUES - INTERVAL;

        BL_FLOAT v = ParabolaFuncGen(x, a, b, c);
        
        values.Get()[i] = v;
    }
        
    // Mark the indices
    //
    int leftIdx = peakBin - 1;
    int leftIdx0 = (((BL_FLOAT)(leftIdx - start))/(end - start))*NUM_VALUES;
    values.Get()[leftIdx0] = phases.Get()[leftIdx]; //0.0;

    int centerIdx = peakBin;
    int centerIdx0 = (((BL_FLOAT)(centerIdx - start))/(end - start))*NUM_VALUES;
    values.Get()[centerIdx0] = phases.Get()[centerIdx]; //0.0;

    int rightIdx = peakBin + 1;
    int rightIdx0 = (((BL_FLOAT)(rightIdx - start))/(end - start))*NUM_VALUES;
    values.Get()[rightIdx0] = phases.Get()[rightIdx]; //0.0;

#if USE_MAP_TO_PI
    BLUtilsPhases::MapToPi(values.Get()[leftIdx0]);
    BLUtilsPhases::MapToPi(values.Get()[centerIdx0]);
    BLUtilsPhases::MapToPi(values.Get()[rightIdx0]);
#endif
    
    //BLDebug::DumpData("parabola-gen.txt", values);
}
