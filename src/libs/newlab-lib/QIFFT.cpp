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

#include "QIFFT.h"

void
QIFFT::findPeak(const vector<float> &magns,
                const vector<float> &phases,
                int bufferSize,
                int peakBin, Peak *result)
{
    // Here, magns are in log scale
    
    // Eps used for derivative
#define DERIV_EPS 1e-5 //1e-3 //1e-10
    
    // Default value
    result->_binIdx = peakBin;
    
    // Begin with rough computation
    result->_freq = ((float)peakBin)/(bufferSize*0.5);
    result->_amp = magns.data()[peakBin];
    result->_phase = phases.data()[peakBin];
    result->_alpha0 = 0.0;
    result->_beta0 = 0.0;
    
    if ((peakBin - 1 < 0) || (peakBin + 1 >= magns.size()))
        return;

    // Bin 1 is the first usable bin (bin 0 is the fft DC)
    // If we have a peak at bin1, we can not make parabola interploation
    if (peakBin <= 1)
        return;
    
    // Magns are in DB
    float alpha = magns.data()[peakBin - 1];
    float beta = magns.data()[peakBin];
    float gamma = magns.data()[peakBin + 1];

    // Will avoid wrong negative result in case of not true peaks
    if ((beta < alpha) || (beta < gamma))
        return;
    
   // Get parabola equation coeffs a and b
    // (we already have c)
    float a;
    float b;
    float c;
    getParabolaCoeffs(alpha, beta, gamma, &a, &b, &c);

    // We have the true bin!
    result->_binIdx = peakBin + c;

    // Frequency
    result->_freq = result->_binIdx/(bufferSize*0.5);
        
    // We have the true amp!
    result->_amp = beta - 0.25*(alpha - gamma)*c;

    //DBG_DumpParabola(peakBin, alpha, beta, gamma, c, magns);
                     
    // Phases
    float alphaP = phases.data()[peakBin - 1];
    float betaP = phases.data()[peakBin];
    float gammaP = phases.data()[peakBin + 1];
    
    // Get parabola equation coeffs for phases
    float aP;
    float bP;
    float cP;
    getParabolaCoeffsGen(alphaP, betaP, gammaP, &aP, &bP, &cP);
    
    // We have the true phase!
    // (should we unwrap phases before ?)
    //result->mPhase = betaP - 0.25*(alphaP - gammaP)*c;
    float peakPhase = parabolaFuncGen(c, aP, bP, cP);
    result->_phase = peakPhase;
    
    // For alpha0 and beta0,
    // See: https://ccrma.stanford.edu/files/papers/stanm118.pdf

    // Magnitudes
    //
    
    // Magnitude derivative at mBinIdx
    float a0 = parabolaFunc(c - DERIV_EPS, a, b, c);
    float a1 = parabolaFunc(c + DERIV_EPS, a, b, c);
    float up = (a1 - a0)/(2.0*DERIV_EPS);
    
    // Second derivative of magnitudes
    //

    // Derivative at n - 1
    float a00 = parabolaFunc(c - 2.0*DERIV_EPS, a, b, c);
    float a10 = parabolaFunc(c, a, b, c);
    float up0 = (a10 - a00)/(2.0*DERIV_EPS);

    // Derivative at n + 1
    float a01 = parabolaFunc(c, a, b, c);
    float a11 = parabolaFunc(c + 2.0*DERIV_EPS, a, b, c);
    float up1 = (a11 - a01)/(2.0*DERIV_EPS);

    // Second derivative
    float upp = (up1 - up0)/(2.0*DERIV_EPS);

    // Phases
    //
    
    // Phases derivative at mBinIdx
    float p0 = parabolaFuncGen(c - DERIV_EPS, aP, bP, cP);
    float p1 = parabolaFuncGen(c + DERIV_EPS, aP, bP, cP);
    float vp = (p1 - p0)/(2.0*DERIV_EPS);

    // Second derivative of phases
    //

    // Derivative at n - 1
    float p00 = parabolaFuncGen(c - 2.0*DERIV_EPS, aP, bP, cP);
    float p10 = parabolaFuncGen(c, aP, bP, cP);
    float vp0 = (p10 - p00)/(2.0*DERIV_EPS);
    
    // Derivative at n + 1
    float p01 = parabolaFuncGen(c, aP, bP, cP);
    float p11 = parabolaFuncGen(c + 2.0*DERIV_EPS, aP, bP, cP);
    float vp1 = (p11 - p01)/(2.0*DERIV_EPS);
    
    // Second derivative
    float vpp = (vp1 - vp0)/(2.0*DERIV_EPS);

    // Then finally compute alpha0 and beta0
    //
    float denom1 = (2.0*(upp*upp + vpp*vpp));
    if (fabs(denom1) < NL_EPS)
        return;
    
    float p = -upp/denom1;

    // Must do a -M_PI, otherwise alpha0 will always be positive 
    float alpha0 = -2.0*p*(vp - M_PI);
    
    float beta0 = 0.0;
    if (fabs(upp) > NL_EPS)
        beta0 = p*vpp/upp;
    
    // Result
    result->_alpha0 = alpha0;
    result->_beta0 = beta0;
}

void
QIFFT::getParabolaCoeffs(float alpha, float beta, float gamma,
                         float *a, float *b, float *c)
{
    // Parabola equation: y(x) = a*(x - c)^2 + b
    // c: center
    // a: concavity
    // b: offset

    // Center
    float denom0 = (alpha - 2.0*beta + gamma);
    if (fabs(denom0) < BL_EPS)
        return;
    
    *c = 0.5*((alpha - gamma)/denom0);
    
    // Use http://mural.maynoothuniversity.ie/4523/1/thesis.pdf
    // To make equations and find a and b
    *b = -(alpha*(*c)*(*c) - beta*(*c +1.0)*(*c + 1.0))/(2.0*(*c) + 1.0);
    *a = (alpha - *b)/((*c + 1.0)*(*c + 1.0));
}

float
QIFFT::parabolaFunc(float x, float a, float b, float c)
{
    // Parabola equation: y(x) = a*(x - c)^2 + b
    // c: center
    // a: concavity
    // b: offset
    float v = a*(x - c)*(x - c) + b;

    return v;
}

// Parabola equation: y(x) = a*x^2 + b*x + c
// Generalized (no maximum constraint)
void
QIFFT::getParabolaCoeffsGen(float alpha, float beta, float gamma,
                            float *a, float *b, float *c)
{
    *a = 0.5*(alpha + gamma - 2.0*beta);
    *b = gamma - 0.5*(alpha + gamma - 2.0*beta) - beta;
    *c = beta;
}

// Parabola equation: y(x) = a*x^2 + b*x + c
// Generalized (no maximum constraint)
float
QIFFT::parabolaFuncGen(float x, float a, float b, float c)
{
    float v = a*x*x + b*x + c;

    return v;
}
