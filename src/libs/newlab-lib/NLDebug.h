#ifndef NL_DEBUG_H
#define NL_DEBUG_H

#include <stdlib.h>

#include <vector>
#include <complex>

using namespace std;

class NLDebug
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
