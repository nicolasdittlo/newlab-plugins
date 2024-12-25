#include "Axis.h"

#include "FreqAxis.h"

// From 0Hz to sampleRate/2 Hz
#define NUM_AXIS_DATA_FULL 12
const char *labelsFull[NUM_AXIS_DATA_FULL] =
{
    "", "", "100Hz", "500Hz", "1KHz", "2KHz", "5KHz",
    "10KHz", "20KHz", "40KHz", "80KHz", ""
};
const float freqsFull[NUM_AXIS_DATA_FULL] =
{
    25.0, 50.0, 100.0, 500.0, 1000.0, 2000.0, 5000.0,
    10000.0, 20000.0, 40000.0, 80000.0, 176400.0
};

// Use very close lines (to see well log scale), and add just some labels
// From 0Hz to sampleRate/2 Hz
#define NUM_AXIS_DATA_FULL2 39 //48
const char *labelsFull2[NUM_AXIS_DATA_FULL2] =
{
    //"", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "",
    "100Hz", "", "", "", "500Hz", "", "", "", "",
    "1KHz", "2KHz", "", "", "5KHz", "", "", "", "",
    "10KHz", "20KHz", "", "40KHz", "", "", "", "80KHz", "",
    "100Khz", "200KHz", ""
};
const float freqsFull2[NUM_AXIS_DATA_FULL2] =
{
    10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0, 90.0,
    100.0, 200.0, 300.0, 400.0, 500.0, 600.0, 700.0, 800.0, 900.0,
    1000.0, 2000.0, 3000.0, 4000.0, 5000.0, 6000.0, 7000.0, 8000.0, 9000.0,
    10000.0, 20000.0, 30000.0, 40000.0, 50000.0, 60000.0, 70000.0, 80000.0, 90000.0,
    100000.0, 200000.0, 300000.0
};

// Log scale
//

// From 1Hz to 100Hz
#define NUM_AXIS_DATA_LOG100 13
const char *labelsLog100[NUM_AXIS_DATA_LOG100] =
{
    "", "1Hz", "2Hz", "4Hz", "6Hz", "8Hz", "10Hz",
    "20Hz", "40Hz", "60Hz", "80Hz", "100Hz", ""
};
const float freqsLog100[NUM_AXIS_DATA_LOG100] =
{
    0.0, 1.0, 2.0, 4.0, 6.0, 8.0, 10.0, 20.0, 40.0,
    60.0, 80.0, 100.0, 120.0
};

// From 1Hz to 50Hz
#define NUM_AXIS_DATA_LOG50 12
const char *labelsLog50[NUM_AXIS_DATA_LOG50] =
{
    "", "1Hz", "2Hz", "4Hz", "6Hz", "8Hz", "10Hz",
    "20Hz", "30Hz", "40Hz", "50Hz", ""
};
const float freqsLog50[NUM_AXIS_DATA_LOG50] =
{
    0.0, 1.0, 2.0, 4.0, 6.0, 8.0, 10.0, 20.0, 30.0,
    40.0, 50.0, 60.0
};

// From 1Hz to 25Hz
#define NUM_AXIS_DATA_LOG25 11
const char *labelsLog25[NUM_AXIS_DATA_LOG25] =
{
    "", "1Hz", "2Hz", "4Hz", "6Hz", "8Hz", "10Hz",
    "15Hz", "20Hz", "25Hz", ""
};
const float freqsLog25[NUM_AXIS_DATA_LOG25] =
{
    0.0, 1.0, 2.0, 4.0, 6.0, 8.0, 10.0, 15.0,
    20.0, 25.0, 30.0
};

// Linear scale
//

// From 1Hz to 1000Hz
#define NUM_AXIS_DATA_LIN1000 12
const char *labelsLin1000[NUM_AXIS_DATA_LIN1000] =
{
    "", "100Hz", "200Hz", "300Hz", "400Hz", "500Hz", "600Hz",
    "700Hz", "800Hz", "900Hz", "1KHz", ""
};
const float freqsLin1000[NUM_AXIS_DATA_LIN1000] =
{
    0.0, 100.0, 200.0, 300.0, 400.0, 500.0, 600.0, 700.0, 800.0,
    900.0, 1000.0, 1100.0
};

// From 1Hz to 500Hz
#define NUM_AXIS_DATA_LIN500 12
const char *labelsLin500[NUM_AXIS_DATA_LIN500] =
{
    "", "50Hz", "100Hz", "150Hz", "200Hz", "250Hz", "300Hz",
    "350Hz", "400Hz", "450Hz", "500Hz", ""
};
const float freqsLin500[NUM_AXIS_DATA_LIN500] =
{
    0.0, 50.0, 100.0, 150.0, 200.0, 250.0, 300.0, 350.0, 400.0,
    450.0, 500.0, 550.0
};

// From 1Hz to 250Hz
#define NUM_AXIS_DATA_LIN250 15
const char *labelsLin250[NUM_AXIS_DATA_LIN250] =
{
    "", "20Hz", "40Hz", "60Hz", "80Hz", "100Hz", "120Hz",
    "140Hz", "160Hz", "180Hz", "200Hz", "220Hz", "240Hz", "260Hz", ""
};
const float freqsLin250[NUM_AXIS_DATA_LIN250] =
{
    0.0, 20.0, 40.0, 60.0, 80.0, 100.0, 120.0, 140.0, 160.0,
    180.0, 200.0, 220.0, 240.0, 260.0, 280.0
};

// From 1Hz to 100Hz
#define NUM_AXIS_DATA_LIN100 12
const char *labelsLin100[NUM_AXIS_DATA_LIN100] =
{
    "", "10Hz", "20Hz", "30Hz", "40Hz", "50Hz", "60Hz",
    "70Hz", "80Hz", "90Hz", "100Hz", ""
};
const float freqsLin100[NUM_AXIS_DATA_LIN100] =
{
    0.0, 10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0,
    90.0, 100.0, 110.0
};

// From 1Hz to 50Hz
#define NUM_AXIS_DATA_LIN50 12
const char *labelsLin50[NUM_AXIS_DATA_LIN50] =
{
    "", "5Hz", "10Hz", "15Hz", "20Hz", "25Hz", "30Hz",
    "35Hz", "40Hz", "45Hz", "50Hz", ""
};
const float freqsLin50[NUM_AXIS_DATA_LIN50] =
{
    0.0, 5.0, 10.0, 15.0, 20.0, 25.0, 30.0, 35.0, 40.0,
    45.0, 50.0, 55.0
};

// From 1Hz to 25Hz
#define NUM_AXIS_DATA_LIN25 15
const char *labelsLin25[NUM_AXIS_DATA_LIN25] =
{
    "", "2Hz", "4Hz", "6Hz", "8Hz", "10Hz", "12Hz",
    "14Hz", "16Hz", "18Hz", "20Hz", "22Hz", "24Hz", "26Hz", ""
};

const float freqsLin25[NUM_AXIS_DATA_LIN25] =
{
    0.0, 2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0,
    18.0, 20.0, 22.0, 24.0, 26.0, 28.0
};

// From 1Hz to 12Hz
#define NUM_AXIS_DATA_LIN12 14
const char *labelsLin12[NUM_AXIS_DATA_LIN12] =
{
    "", "1Hz", "2Hz", "3Hz", "4Hz", "5Hz", "6Hz",
    "7Hz", "8Hz", "9Hz", "10Hz", "11Hz", "12Hz", ""
};

const float freqsLin12[NUM_AXIS_DATA_LIN12] =
{
    0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0,
    9.0, 10.0, 11.0, 12.0, 13.0
};

// From 1Hz to 6Hz
#define NUM_AXIS_DATA_LIN6 14
const char *labelsLin6[NUM_AXIS_DATA_LIN6] =
{
    "", "0.5Hz", "1.0Hz", "1.5Hz", "2.0Hz", "2.5Hz",
    "3.0Hz", "3.5Hz", "4.0Hz", "4.5Hz", "5.0Hz", "5.5Hz", "6.0Hz", ""
};

const float freqsLin6[NUM_AXIS_DATA_LIN6] =
{
    0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0,
    4.5, 5.0, 5.5, 6.0, 6.5
};

FreqAxis::FreqAxis(bool displayLines,
                   Scale::Type scale)
{
    _scale = scale;
    
    _axis = NULL;
    
    _bufferSize = 2048;
    _sampleRate = 44100.0;

    _maxFreq = -1.0;
    
    _displayLines = displayLines;
}

FreqAxis::~FreqAxis() {}

void
FreqAxis::init(Axis *axis,
               bool horizontal,
               int bufferSize, float sampleRate,
               int graphWidth)
{
    _axis = axis;
    
    _bufferSize = bufferSize;
    _sampleRate = sampleRate;
    
    int axisColor[4] = { 48, 48, 48, 255 };
    int axisLabelColor[4] = { 255, 255, 255, 255 };
    float lineWidth = 1.25;
    
    if (!_displayLines)
        axisColor[3] = 0;
    
    if (horizontal)
    {        
        _axis->initHAxis(_scale,
                         0.0, sampleRate*0.5,
                         axisColor, axisLabelColor,
                         lineWidth,
                         0.0);
    }
    else
    {        
        _axis->initVAxis(mScale,
                         0.0, sampleRate*0.5,
                         axisColor, axisLabelColor,
                         lineWidth,
                         1.0 - 40.0/graphWidth, 0.0);
    }
    
    update();
}

void
FreqAxis::reset(int bufferSize, float sampleRate)
{
    _bufferSize = bufferSize;
    _sampleRate = sampleRate;
    
    if (_axis != NULL)
        _axis->setMinMaxValues(0.0, sampleRate*0.5);
    
    update();
}

void
FreqAxis::setMaxFreq(float maxFreq)
{
    _maxFreq = maxFreq;

    if (_axis != NULL)
        _axis->setMinMaxValues(0.0, _maxFreq);
    
    update();
}

float
FreqAxis::getMaxFreq() const
{
    return _maxFreq;
}

void
FreqAxis::setScale(Scale::Type scale)
{
    _scale = scale;

    if (_axis != NULL)
        _axis->setScaleType(_scale);
    
    update();
}

void
FreqAxis::getMinMaxFreqAxisValues(float *minHzValue, float *maxHzValue,
                                 int bufferSize, float sampleRate)
{
    float hzPerBin = sampleRate/bufferSize;
    
    *minHzValue = 1*hzPerBin;
    *maxHzValue = (bufferSize/2)*hzPerBin;
}

void
FreqAxis::update()
{
    if (_axis == NULL)
        return;
            
    if (_maxFreq < 0.0)
    {
        float minHzValue;
        float maxHzValue;
        getMinMaxFreqAxisValues(&minHzValue, &maxHzValue,
                                _bufferSize, _sampleRate);
    
        // Avoid a shift
        minHzValue = 0.0;

        UpdateAxis(NUM_AXIS_DATA_FULL2, freqsFull2, labelsFull2,
                   minHzValue, maxHzValue);
    }
    else if ((mScale == Scale::LOG) ||
             (mScale == Scale::LOG10) ||
             (mScale == Scale::LOG_FACTOR) ||
             (mScale == Scale::MEL) ||
             (mScale == Scale::LOW_ZOOM))
    {
        if (mMaxFreq <= 30.0)
            updateAxis(NUM_AXIS_DATA_LOG25,
                       freqsLog25, labelsLog25,
                       0.0, 30.0);
        else if (mMaxFreq <= 60.0)
            updateAxis(NUM_AXIS_DATA_LOG50,
                       freqsLog50, labelsLog50,
                       0.0, 60.0);
        else if (mMaxFreq <= 120.0)
            updateAxis(NUM_AXIS_DATA_LOG100,
                       freqsLog100, labelsLog100,
                       0.0, 120.0);
    }
    else if (_scale == Scale::LINEAR)
    {
        if (maxFreq <= 6.5)
            updateAxis(NUM_AXIS_DATA_LIN6,
                       freqsLin6, labelsLin6,
                       0.0, 6.5);
        else if (_maxFreq <= 13.0)
            updateAxis(NUM_AXIS_DATA_LIN12,
                       freqsLin12, labelsLin12,
                       0.0, 13.0);
        else if (_maxFreq <= 28.0)
            updateAxis(NUM_AXIS_DATA_LIN25,
                       freqsLin25, labelsLin25,
                       0.0, 28.0);
        else if (_maxFreq <= 56.0)
            updateAxis(NUM_AXIS_DATA_LIN50,
                       freqsLin50, labelsLin50,
                       0.0, 56.0);
        else if (_maxFreq <= 112.0)
            updateAxis(NUM_AXIS_DATA_LIN100,
                       freqsLin100, labelsLin100,
                       0.0, 112.0);
        else if (_maxFreq <= 262.0)
            updateAxis(NUM_AXIS_DATA_LIN250,
                       freqsLin250, labelsLin250,
                       0.0, 262.0);
        else if (_maxFreq <= 524.0)
            updateAxis(NUM_AXIS_DATA_LIN500,
                       freqsLin500, labelsLin500,
                       0.0, 524.0);
        else if (_maxFreq <= 1048.0)
            updateAxis(NUM_AXIS_DATA_LIN1000,
                       freqsLin1000, labelsLin1000,
                       0.0, 1048.0);
    }
}

void
FreqAxis::updateAxis(int numAxisData,
                     const float freqs[],
                     const char *labels[],
                     float minHzValue, float maxHzValue)
{    
    char **axisData = (char **)malloc(numAxisData*sizeof(char *)*2);
    
    for (int i = 0; i < numAxisData; i++)
    {
        axisData[i*2] = (char *)malloc(sizeof(char)*255);
        axisData[i*2 + 1] = (char *)malloc(sizeof(char)*255);
    }

    for (int i = 0; i < numAxisData; i++)
    {
        sprintf(axisData[i*2 + 1], "%s", labels[i]);
    }

    // Normalize
    for (int i = 0; i < numAxisData; i++)
    {
        sprintf(axisData[i*2], "%g", freqs[i]);
        
        // We are over the sample rate, make empty label
        if ((freqs[i] < minHzValue) || (freqs[i] > maxHzValue))
            sprintf(axisData[i*2 + 1], "");
    }

    if (_axis != NULL)
        _axis->setData((char *(*)[2])axisData, numAxisData);
    
    for (int i = 0; i < numAxisData; i++)
    {
        free(axisData[i*2]);
        free(axisData[i*2 + 1]);
    }

    free(axisData);
}
