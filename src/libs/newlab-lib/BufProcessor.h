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

#ifndef BUF_PROCESSOR_H
#define BUF_PROCESSOR_H

#include <vector>
using namespace std;

#include "OverlapAdd.h"

class BufProcessor : public OverlapAddProcessor
{
public:
    BufProcessor();
    
    virtual ~BufProcessor();
    
    void processFFT(vector<complex<float> > *ioBuffer) override;

    void getMagnsBuffer(vector<float> *magns);
        
protected:
    vector<float> _magnsBuffer;
};

#endif
