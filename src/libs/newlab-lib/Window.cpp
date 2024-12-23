#include <math.h>

#include "Window.h"

#ifndef M_PI
#define M_PI 3.141592654
#endif

void
Window::makeWindowHann(vector<float> *win)
{
    // Hann
    for (int i = 0; i < win->size(); i++)
        (*win)[i] = 0.5 * (1.0 - cos(2.0 * M_PI *
                                     ((double)i) / (win->size() - 1)));
}
