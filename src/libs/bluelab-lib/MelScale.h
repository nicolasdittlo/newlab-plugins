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

#ifndef MEL_SCALE_H
#define MEL_SCALE_H

#include <vector>
using namespace std;

class MelScale
{
public:
    MelScale();
    virtual ~MelScale();
    
    static float hzToMel(float freq);
    static float melToHz(float mel);
    
    // Quick transformations, without filtering
    static void hzToMel(vector<float> *resultMagns,
                        const vector<float> &magns,
                        float sampleRate);
    static void melToHz(vector<float> *resultMagns,
                        const vector<float> &magns,
                        float sampleRate);
};

#endif
