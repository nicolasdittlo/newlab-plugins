#ifndef AMP_AXIS_H
#define AMP_AXIS_H

class Axis;
class AmpAxis
{
public:
    enum Density
    {
        DENSITY_20DB = 0,
        DENSITY_10DB,
    };
    
    AmpAxis(bool displayLines = true, Density density = DENSITY_20DB);
    
    virtual ~AmpAxis();
    
    void init(Axis *axis,
              float minDB, float maxDB,
              int graphWidth);
    
    void reset(float minDB, float maxDB);
    
protected:
    void update();
    
    void updateDensity20dB();
    void updateDensity10dB();
    
    Axis2 *_axis;
    
    float _minDB;
    float _maxDB;
    
    bool _displayLines;
    
    Density _density;
};

#endif
