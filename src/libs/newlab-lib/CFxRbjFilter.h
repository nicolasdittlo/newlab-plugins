// See: https://forum.cockos.com/showthread.php?t=121016
//  Created by Christian Schragen on 07.10.13.
//
//  License: open source

#ifndef CFX_RBJ_FILTER_H
#define CFX_RBJ_FILTER_H

#include <math.h>

// See: https://forum.cockos.com/showthread.php?t=121016
// " If you put two filters back-to-back with a Q=0.707, you should get a 4th order filter that sums flat."
//
// Note: no phase inversions for 4th order
// See: https://en.wikipedia.org/wiki/Linkwitz%E2%80%93Riley_filter

// See also (for usage): https://www.kvraudio.com/forum/viewtopic.php?t=286084

#define FILTER_TYPE_LOWPASS         0
#define FILTER_TYPE_HIPASS          1
#define FILTER_TYPE_BANDPASS_CSG    2
#define FILTER_TYPE_BANDPASS_CZPG   3
#define FILTER_TYPE_NOTCH           4
#define FILTER_TYPE_ALLPASS         5
#define FILTER_TYPE_PEAKING         6
#define FILTER_TYPE_LOWSHELF        7
#define FILTER_TYPE_HISHELF         8

class CFxRbjFilter2X;
class CFxRbjFilter
{
public:
	
	CFxRbjFilter()
	{
		// reset filter coeffs
		b0a0=b1a0=b2a0=a1a0=a2a0=0.0;
		
		// reset in/out history
		ou1=ou2=in1=in2=0.0f;	
	};

    void reset()
    {
        // reset in/out history
        ou1=ou2=in1=in2=0.0f;
    }
    
	float filter(float in0)
	{
		// filter
		float yn = b0a0*in0 + b1a0*in1 + b2a0*in2 - a1a0*ou1 - a2a0*ou2;
        
		// push in/out buffers
		in2=in1;
		in1=in0;
		ou2=ou1;
		ou1=yn;
        
		// return output
		return yn;
	};

    void filter(float *ioSamples, int nSamples)
    {
        for (int i = 0; i  < nSamples; i++)
        {
            float in0 = ioSamples[i];
        
            // filter
            float yn = b0a0*in0 + b1a0*in1 + b2a0*in2 - a1a0*ou1 - a2a0*ou2;
        
            // push in/out buffers
            in2=in1;
            in1=in0;
            ou2=ou1;
            ou1=yn;
        
            ioSamples[i] = yn;
        }
    };
    
	void calc_filter_coeffs(int const type,float const frequency,float const sample_rate,float const q,float const db_gain,bool q_is_bandwidth)
	{
		// temp pi
		float const temp_pi=3.1415926535897932384626433832795;
		
		// temp coef vars
		float alpha,a0,a1,a2,b0,b1,b2;

		// peaking, lowshelf and hishelf
		if(type>=6)
		{
			float const A		=	std::pow((float)10.0, (float)((db_gain/40.0)));
			float const omega	=	2.0*temp_pi*frequency/sample_rate;
			float const tsin	=	std::sin(omega);
			float const tcos	=	std::cos(omega);
			
			if(q_is_bandwidth)
			alpha=tsin*std::sinh((float)(std::log((float)2.0)/2.0*q*omega/tsin));
			else
			alpha=tsin/(2.0*q);

			float const beta	=	std::sqrt(A)/q;
			
			// peaking
			if(type==6)
			{
				b0=float(1.0+alpha*A);
				b1=float(-2.0*tcos);
				b2=float(1.0-alpha*A);
				a0=float(1.0+alpha/A);
				a1=float(-2.0*tcos);
				a2=float(1.0-alpha/A);
			}
			
			// lowshelf
			if(type==7)
			{
				b0=float(A*((A+1.0)-(A-1.0)*tcos+beta*tsin));
				b1=float(2.0*A*((A-1.0)-(A+1.0)*tcos));
				b2=float(A*((A+1.0)-(A-1.0)*tcos-beta*tsin));
				a0=float((A+1.0)+(A-1.0)*tcos+beta*tsin);
				a1=float(-2.0*((A-1.0)+(A+1.0)*tcos));
				a2=float((A+1.0)+(A-1.0)*tcos-beta*tsin);
			}

			// hishelf
			if(type==8)
			{
				b0=float(A*((A+1.0)+(A-1.0)*tcos+beta*tsin));
				b1=float(-2.0*A*((A-1.0)+(A+1.0)*tcos));
				b2=float(A*((A+1.0)+(A-1.0)*tcos-beta*tsin));
				a0=float((A+1.0)-(A-1.0)*tcos+beta*tsin);
				a1=float(2.0*((A-1.0)-(A+1.0)*tcos));
				a2=float((A+1.0)-(A-1.0)*tcos-beta*tsin);
			}
		}
		else
		{
			// other filters
			float const omega	=	2.0*temp_pi*frequency/sample_rate;
			float const tsin	=	std::sin(omega);
			float const tcos	=	std::cos(omega);

			if(q_is_bandwidth)
			alpha=tsin*std::sinh((float)(std::log((float)2.0)/2.0*q*omega/tsin));
			else
			alpha=tsin/(2.0*q);

			// lowpass
			if(type==0)
			{
				b0=(1.0-tcos)/2.0;
				b1=1.0-tcos;
				b2=(1.0-tcos)/2.0;
				a0=1.0+alpha;
				a1=-2.0*tcos;
				a2=1.0-alpha;
			}

			// hipass
			if(type==1)
			{
				b0=(1.0+tcos)/2.0;
				b1=-(1.0+tcos);
				b2=(1.0+tcos)/2.0;
				a0=1.0+ alpha;
				a1=-2.0*tcos;
				a2=1.0-alpha;
			}
            
			// bandpass csg
			if(type==2)
			{
				b0=tsin/2.0;
				b1=0.0;
			    b2=-tsin/2;
				a0=1.0+alpha;
				a1=-2.0*tcos;
				a2=1.0-alpha;
			}

			// bandpass czpg
			if(type==3)
			{
				b0=alpha;
				b1=0.0;
				b2=-alpha;
				a0=1.0+alpha;
				a1=-2.0*tcos;
				a2=1.0-alpha;
			}

			// notch
			if(type==4)
			{
				b0=1.0;
				b1=-2.0*tcos;
				b2=1.0;
				a0=1.0+alpha;
				a1=-2.0*tcos;
				a2=1.0-alpha;
			}

			// allpass
			if(type==5)
			{
				b0=1.0-alpha;
				b1=-2.0*tcos;
				b2=1.0+alpha;
				a0=1.0+alpha;
				a1=-2.0*tcos;
				a2=1.0-alpha;
			}
		}
        
		// set filter coeffs
		b0a0=float(b0/a0);
		b1a0=float(b1/a0);
		b2a0=float(b2/a0);
		a1a0=float(a1/a0);
		a2a0=float(a2/a0);
	};

private:
    friend class CFxRbjFilter2X;
    
	// filter coeffs
	float b0a0,b1a0,b2a0,a1a0,a2a0;

	// in/out history
	float ou1,ou2,in1,in2;
};

// Chain 2 filters intelligently
class CFxRbjFilter2X
{
public:
    CFxRbjFilter2X()
    {
        for (int k = 0; k < 2; k++)
        {
            b0a0[k] = 0.0f;
            b1a0[k] = 0.0f;
            b2a0[k] = 0.0f;
            a1a0[k] = 0.0f;
            a2a0[k] = 0.0f;
            
            ou1[k] = 0.0f;
            ou2[k] = 0.0f;
            in1[k] = 0.0f;
            in2[k] = 0.0f;
        }
    }
    
    void reset()
    {
        for (int k = 0; k < 2; k++)
        {
            ou1[k] = 0.0f;
            ou2[k] = 0.0f;
            in1[k] = 0.0f;
            in2[k] = 0.0f;
        }
    }
    
    float filter(float in0)
    {
        // 1
        
        // Filter
        float yn = b0a0[0]*in0 + b1a0[0]*in1[0] +
                         b2a0[0]*in2[0] - a1a0[0]*ou1[0] -
                         a2a0[0]*ou2[0];
        
        // push in/out buffers
        in2[0] = in1[0];
        in1[0] = in0;
        ou2[0] = ou1[0];
        ou1[0] = yn;
        
        // Result
        in0 = yn;
        
        // 2
        
        // Filter
        yn = b0a0[1]*in0 + b1a0[1]*in1[1] +
        b2a0[1]*in2[1] - a1a0[1]*ou1[1] -
        a2a0[1]*ou2[1];
        
        // push in/out buffers
        in2[1] = in1[1];
        in1[1] = in0;
        ou2[1] = ou1[1];
        ou1[1] = yn;
        
        return yn;
    };
    
    void filter(float *ioSamples, int nSamples)
    {
        for (int i = 0; i < nSamples; i++)
        {
            float in0 = ioSamples[i];
        
            // 1
        
            // Filter
            float yn = b0a0[0]*in0 + b1a0[0]*in1[0] + b2a0[0]*in2[0]
                - a1a0[0]*ou1[0] - a2a0[0]*ou2[0];
        
            // push in/out buffers
            in2[0] = in1[0];
            in1[0] = in0;
            ou2[0] = ou1[0];
            ou1[0] = yn;
        
            // Result
            in0 = yn;
        
            // 2
            
            // Filter
            yn = b0a0[1]*in0 + b1a0[1]*in1[1] + b2a0[1]*in2[1]
                             - a1a0[1]*ou1[1] - a2a0[1]*ou2[1];
        
            // push in/out buffers
            in2[1] = in1[1];
            in1[1] = in0;
            ou2[1] = ou1[1];
            ou1[1] = yn;
        
            ioSamples[i] = yn;
        }
    };

    void
    calc_filter_coeffs(int const type,float const frequency,
                       float const sample_rate,
                       float const q,
                       float const db_gain,
                       bool q_is_bandwidth)
    {
        _filter.calc_filter_coeffs(type, frequency,
                                   sample_rate,
                                   q,
                                   db_gain,
                                   q_is_bandwidth);
     
        set_coeffs();
    }
    
protected:
    void
    set_coeffs()
    {
        for (int k = 0; k < 2; k++)
        {
            b0a0[k] = _filter.b0a0;
            b1a0[k] = _filter.b1a0;
            b2a0[k] = _filter.b2a0;
            a1a0[k] = _filter.a1a0;
            a2a0[k] = _filter.a2a0;
        }
    }
    
    //
    CFxRbjFilter _filter;
    
    // filter coeffs
    float b0a0[2];
    float b1a0[2];
    float b2a0[2];
    float a1a0[2];
    float a2a0[2];
    
    // in/out history
    float ou1[2];
    float ou2[2];
    float in1[2];
    float in2[2];
};

#endif
