#include "Axis.h"

#include "AmpAxis.h"

AmpAxis::AmpAxis(bool displayLines, Density density)
{
    _axis = NULL;
    
    _minDB = -60.0;
    _maxDB = 0.0;
    
    _displayLines = displayLines;
    
    _density = density;
}

AmpAxis::~AmpAxis() {}

void
AmpAxis::init(Axis *axis, float minDB, float maxDB)
{
    _minDB = minDB;
    _maxDB = maxDB;
    
    _axis = axis;
    
    int axisColor[4] = { 48, 48, 48, 255 };
    int axisLabelColor[4] = { 255, 255, 255, 255 };
    float lineWidth = 1.25;

    float offsetX = 0.00862069;
    
    axis->initVAxis(Scale::LINEAR,
                    minDB, maxDB,
                    axisColor, axisLabelColor,
                    lineWidth,
                    offsetX, 0.0);
    
    update();
}

void
AmpAxis::reset(float minDB, float maxDB)
{
    _minDB = minDB;
    _maxDB = maxDB;
    
    _axis->setMinMaxValues(minDB, maxDB);
    
    update();
}

void
AmpAxis::update()
{
    if (_density == DENSITY_20DB)
        updateDensity20dB();
    else if (_density == DENSITY_10DB)
        updateDensity10dB();
}

void
AmpAxis::updateDensity20dB()
{
    if (_axis == NULL)
        return;
    
#define NUM_AXIS_DATA_20DB 11
    char *AXIS_DATA [NUM_AXIS_DATA_20DB][2];
    for (int i = 0; i < NUM_AXIS_DATA_20DB; i++)
    {
        AXIS_DATA[i][0] = (char *)malloc(255);
        AXIS_DATA[i][1] = (char *)malloc(255);
    }
    
    sprintf(AXIS_DATA[0][1], "-160dB");
    sprintf(AXIS_DATA[1][1], "-140dB");
    sprintf(AXIS_DATA[2][1], "-120dB");
    sprintf(AXIS_DATA[3][1], "-100dB");
    sprintf(AXIS_DATA[4][1], "-80dB");
    sprintf(AXIS_DATA[5][1], "-60dB");
    sprintf(AXIS_DATA[6][1], "-40dB");
    sprintf(AXIS_DATA[7][1], "-20dB");
    sprintf(AXIS_DATA[8][1], "0dB");
    sprintf(AXIS_DATA[9][1], "20dB");
    sprintf(AXIS_DATA[10][1], "40dB"); // For EQHack
    
    float amps[NUM_AXIS_DATA_20DB] =
        { -160.0, -140.0, -120.0, -100.0, -80.0,
          -60.0, -40.0, -20.0, 0.0, 20.0, 40.0 };
    
    for (int i = 0; i < NUM_AXIS_DATA_20DB; i++)
    {
        if ((amps[i] < _minDB) || (amps[i] > _maxDB))
            sprintf(AXIS_DATA[i][1], " ");
        
        sprintf(AXIS_DATA[i][0], "%g", amps[i]);
    }
    
    _axis->setData(AXIS_DATA, NUM_AXIS_DATA_20DB);
    
    for (int i = 0; i < NUM_AXIS_DATA_20DB; i++)
    {
        free(AXIS_DATA[i][0]);
        free(AXIS_DATA[i][1]);
    }
}

void
AmpAxis::updateDensity10dB()
{
    // Just in case
    if (_axis == NULL)
        return;
    
#define NUM_AXIS_DATA_10DB 21
    char *AXIS_DATA [NUM_AXIS_DATA_10DB][2];
    for (int i = 0; i < NUM_AXIS_DATA_10DB; i++)
    {
        AXIS_DATA[i][0] = (char *)malloc(255);
        AXIS_DATA[i][1] = (char *)malloc(255);
    }
    
    sprintf(AXIS_DATA[0][1], "-160dB");
    sprintf(AXIS_DATA[1][1], "-150dB");
    sprintf(AXIS_DATA[2][1], "-140dB");
    sprintf(AXIS_DATA[3][1], "-130dB");
    sprintf(AXIS_DATA[4][1], "-120dB");
    sprintf(AXIS_DATA[5][1], "-110dB");
    sprintf(AXIS_DATA[6][1], "-100dB");
    sprintf(AXIS_DATA[7][1], "-90dB");
    sprintf(AXIS_DATA[8][1], "-80dB");
    sprintf(AXIS_DATA[9][1], "-70dB");
    sprintf(AXIS_DATA[10][1], "-60dB");
    sprintf(AXIS_DATA[11][1], "-50dB");
    sprintf(AXIS_DATA[12][1], "-40dB");
    sprintf(AXIS_DATA[13][1], "-30dB");
    sprintf(AXIS_DATA[14][1], "-20dB");
    sprintf(AXIS_DATA[15][1], "-10dB");
    sprintf(AXIS_DATA[16][1], "0dB");
    sprintf(AXIS_DATA[17][1], "10dB");
    sprintf(AXIS_DATA[18][1], "20dB");
    sprintf(AXIS_DATA[19][1], "30dB");
    sprintf(AXIS_DATA[20][1], "40dB"); // For EQHack
    
    float amps[NUM_AXIS_DATA_10DB] =
        { -160.0, -150.0, -140.0, -130.0, -120.0,
          -110.0, -100.0, -90.0, -80.0, -70.0, -60.0,
          -50.0, -40.0, -30.0, -20.0, -10.0,
          0.0, 10.0, 20.0, 30.0, 40.0 };
    
    for (int i = 0; i < NUM_AXIS_DATA_10DB; i++)
    {
        if ((amps[i] < _minDB) || (amps[i] > _maxDB))
            sprintf(AXIS_DATA[i][1], " ");
        
        sprintf(AXIS_DATA[i][0], "%g", amps[i]);
    }
    
    _axis->setData(AXIS_DATA, NUM_AXIS_DATA_10DB);
    
    for (int i = 0; i < NUM_AXIS_DATA_10DB; i++)
    {
        free(AXIS_DATA[i][0]);
        free(AXIS_DATA[i][1]);
    }
}
