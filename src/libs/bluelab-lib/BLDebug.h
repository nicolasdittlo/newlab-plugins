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

#ifndef BL_DEBUG_H
#define BL_DEBUG_H

#include <stdlib.h>

#include <vector>
#include <complex>

using namespace std;

class BLDebug
{
 public:
    static void dumpVector(const char *fileName, const vector<float> &values)
    {
        FILE *file = fopen(fileName, "wb");

        for (int i = 0; i < values.size(); i++)
            fprintf(file, "%g\n", values[i]);
        
        fclose(file);
    }

    static void dumpVector(const char *fileName, const float *values, int numValues)
    {
        FILE *file = fopen(fileName, "wb");

        for (int i = 0; i < numValues; i++)
            fprintf(file, "%g\n", values[i]);
        
        fclose(file);
    }

    static void dumpVector(const char *fileName, const vector<complex<float> > &values)
    {
        FILE *file = fopen(fileName, "wb");

        for (int i = 0; i < values.size(); i++)
            fprintf(file, "%g\n", abs(values[i]));
        
        fclose(file);
    }

};

#endif
