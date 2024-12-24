#include "Scale.h"

#include "Curve.h"

Curve::Curve(int numValues)
{
    mDescription = NULL;

    mScale  = new Scale();
    
    mYScale = Scale::LINEAR;
    mMinY = 0.0;
    mMaxY = 1.0;
    
    SET_COLOR_FROM_INT(mColor, 0, 0, 0, 255);
    mAlpha = 1.0;
    
    mXScale = Scale::LINEAR;
    mMinX = 0.0;
    mMaxX = 1.0;
    
    mPointStyle = false;
    mPointsAsLinesPolar = false;
    mPointsAsLines = false;
    
    // Set line width to 1.5 by default,
    // because with 1, it is to thin and badly displayed sometimes.
    //mLineWidth = 1.0;
    mLineWidth = 1.5;
    
    mPointSize = 1.0;
    
    //mBevelFlag = false;
    mBevelFlag = true;
    
    mPointOverlay = false;
    
    mCurveFill = false;
    mFillAlpha = FILL_CURVE_ALPHA;
    mFillAlphaUp = 0.0;
    mFillAlphaUpFlag = false;
    
    SET_COLOR_FROM_INT(mFillColor, 0, 0, 0, 255);
    mFillColorSet = false;
    
    mSingleValueH = false;
    mSingleValueV = false;
    
    mNumValues = numValues;
    mXValues.Resize(mNumValues);
    mYValues.Resize(mNumValues);

    mUseLegacyLock = false;
        
    ClearValues();
    
    FillAllYValues(0.0);
    
    mPushCounter = 0;
    mTmpPushValue = 0.0;
    
    mLimitToBounds = false;
    
    mOptimSameColor = false;
    
    mWeightMultAlpha = true;
    mUseWeightTargetColor = false;
    
    mYScaleFactor = 1.0;
    mAutoAdjustFactor = 1.0;
    
    mViewSize[0] = 256;
    mViewSize[1] = 256;

    mNeedRedraw = true;
}

Curve::~Curve()
{
    if (mDescription != NULL)
        delete []mDescription;

    delete mScale;
}

void
Curve::SetViewSize(BL_FLOAT width, BL_FLOAT height)
{
    mViewSize[0] = width;
    mViewSize[1] = height;

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetYScaleFactor(BL_FLOAT factor)
{
    mYScaleFactor = factor;

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetAutoAdjustFactor(BL_FLOAT factor)
{
    mAutoAdjustFactor = factor;

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetBounds(const BL_GUI_FLOAT bounds[4])
{
    for (int i = 0; i < 4; i++)
    {
        mBounds[i] = bounds[i];
    }

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetDescription(const char *description, int descrColor[4])
{
#define DESCRIPTION_MAX_SIZE 256
    
    if (mDescription != NULL)
        delete []mDescription;
    
    mDescription = new char[DESCRIPTION_MAX_SIZE];
    memset(mDescription, 0, DESCRIPTION_MAX_SIZE*sizeof(char));
    
    sprintf(mDescription, "%s", description);
    
    for (int i = 0; i < 4; i++)
        mDescrColor[i] = descrColor[i];

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::ResetNumValues(int numValues)
{
    mNumValues = numValues;
    
    mXValues.Resize(mNumValues);
    mYValues.Resize(mNumValues);
    
    BLUtils::FillAllZero(&mXValues);
    BLUtils::FillAllZero(&mYValues);

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::ClearValues()
{
    BLUtils::FillAllValue(&mXValues, (BL_GUI_FLOAT)CURVE_VALUE_UNDEFINED);
    BLUtils::FillAllValue(&mYValues, (BL_GUI_FLOAT)CURVE_VALUE_UNDEFINED);

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetYScale(Scale::Type scale, BL_GUI_FLOAT minY, BL_GUI_FLOAT maxY)
{
    mYScale = scale;
    mMinY = minY;
    mMaxY = maxY;

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::GetYScale(Scale::Type *scale, BL_GUI_FLOAT *minY, BL_GUI_FLOAT *maxY)
{
    *scale = mYScale;
    *minY = mMinY;
    *maxY = mMaxY;
}

void
Curve::FillAllYValues(BL_GUI_FLOAT val)
{
    for (int i = 0; i < mYValues.GetSize(); i++)
    {
        mYValues.Get()[i] = val;
    }

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::FillAllXValues(BL_GUI_FLOAT minX, BL_GUI_FLOAT maxX)
{
    for (int i = 0; i < mXValues.GetSize(); i++)
    {
        BL_GUI_FLOAT t = ((BL_GUI_FLOAT)i)/(mXValues.GetSize() - 1);
        
        BL_GUI_FLOAT x = minX + t*(maxX - minX);
        
        mXValues.Get()[i] = x;
    }

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetYValues(const WDL_TypedBuf<BL_GUI_FLOAT> *yValues,
                        BL_GUI_FLOAT minX, BL_GUI_FLOAT maxX)
{
    if (mYValues.GetSize() != yValues->GetSize())
        return;
    
    mYValues = *yValues;
    
    //mXValues.Resize(0);
    mXValues.Resize(mYValues.GetSize());
    for (int i = 0; i < mYValues.GetSize(); i++)
    {
        BL_GUI_FLOAT t = ((BL_GUI_FLOAT)i)/(mYValues.GetSize() - 1);
        
        BL_GUI_FLOAT x = minX + t*(maxX - minX);
    
        //mXValues.Add(x);
        mXValues.Get()[i] = x;
    }

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetValue(BL_GUI_FLOAT t, BL_GUI_FLOAT x, BL_GUI_FLOAT y)
{
    int pos = t*(mNumValues - 1);
    
    if ((pos  < 0) || (pos >= mNumValues))
        return;
    
    mXValues.Get()[pos] = x;
    mYValues.Get()[pos] = y;

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::PushValue(BL_GUI_FLOAT x, BL_GUI_FLOAT y)
{
    if (mDoSmooth)
        // Smooth Y
    {
        mParamSmoother.SetNewValue(y);
        mParamSmoother.Update();
        y = mParamSmoother.GetCurrentValue();
    }
    
    mXValues.Add(x);
    mYValues.Add(y);
    
    if (mYValues.GetSize() > mNumValues)
    {
        WDL_TypedBuf<BL_GUI_FLOAT> &newXValues = mTmpBuf0;
        newXValues.Resize(mNumValues + 1);
        
        WDL_TypedBuf<BL_GUI_FLOAT> &newYValues = mTmpBuf1;
        newYValues.Resize(mNumValues + 1);
        
        for (int i = 1; i < mNumValues + 1; i++)
        {
            BL_GUI_FLOAT x2 = mXValues.Get()[i];
            BL_GUI_FLOAT y2 = mYValues.Get()[i];
            
            //newXValues.Add(x);
            //newYValues.Add(y);

            newXValues.Get()[i] = x2;
            newYValues.Get()[i] = y2;
        }
        
        mXValues = newXValues;
        mYValues = newYValues;
    }

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::NormalizeXValues(BL_GUI_FLOAT maxXValue)
{
    for (int i = 0; i < mXValues.GetSize(); i++)
    {
        BL_GUI_FLOAT t = ((BL_GUI_FLOAT)i)/(mNumValues - 1);
        
        mXValues.Get()[i] = t*maxXValue;
    }

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetXScale(Scale::Type scale, BL_GUI_FLOAT minX, BL_GUI_FLOAT maxX)
{
    mXScale = scale;
    mMinX = minX;
    mMaxX = maxX;

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetPointValues(const WDL_TypedBuf<BL_GUI_FLOAT> &xValues,
                            const WDL_TypedBuf<BL_GUI_FLOAT> &yValues)
{
    mXValues = xValues;
    mYValues = yValues;

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetLimitToBounds(bool flag)
{
    mLimitToBounds = flag;

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetWeightTargetColor(int color[4])
{
    mUseWeightTargetColor = true;
    
    SET_COLOR_FROM_INT(mWeightTargetColor,
                       color[0], color[1], color[2], color[3]);

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetColor(int r, int g, int b)
{
    SET_COLOR_FROM_INT(mColor, r, g, b, 255);

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetAlpha(BL_GUI_FLOAT alpha)
{
    mAlpha = alpha;

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetLineWidth(BL_GUI_FLOAT lineWidth)
{
    mLineWidth = lineWidth;

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetBevel(bool bevelFlag)
{
    mBevelFlag = bevelFlag;

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetSmooth(bool flag)
{
    mDoSmooth = flag;

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetFill(bool flag, BL_GUI_FLOAT originY)
{
    mCurveFill = flag;
    mCurveFillOriginY = originY;

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetFillColor(int r, int g, int b)
{
    SET_COLOR_FROM_INT(mFillColor, r, g, b, 255);
    mFillColorSet = true;

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetFillAlpha(BL_GUI_FLOAT alpha)
{
    mFillAlpha = alpha;

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetFillAlphaUp(BL_GUI_FLOAT alpha)
{
    mFillAlphaUpFlag = true;
    mFillAlphaUp = alpha;

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetPointSize(BL_GUI_FLOAT pointSize)
{
    mPointSize = pointSize;

    mNeedRedraw = true;
    
    NotifyGraph();}

void
Curve::SetPointOverlay(bool flag)
{
    mPointOverlay = flag;

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetWeightMultAlpha(bool flag)
{
    mWeightMultAlpha = flag;

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetPointStyle(bool pointFlag,
                           bool pointsAsLinesPolar,
                           bool pointsAsLines)
{
    mPointStyle = pointFlag;
    mPointsAsLinesPolar = pointsAsLinesPolar;
    mPointsAsLines = pointsAsLines;

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetValuesPoint(const WDL_TypedBuf<BL_GUI_FLOAT> &xValues,
                              const WDL_TypedBuf<BL_GUI_FLOAT> &yValues)
{
    if (mUseLegacyLock)
    {
        SetValuesPointLF(xValues, yValues);
            
        return;
    }
    
    Command &cmd = mTmpBuf9;
    cmd.mType = Command::SET_VALUES_POINT;
    cmd.mXValues = xValues;
    cmd.mYValues = yValues;

#if !OPTIM_LOCK_FREE
    mLockFreeQueues[0].push(cmd);
#else
    if (mLockFreeQueues[0].empty())
        mLockFreeQueues[0].push(cmd);
    else
    {
        mLockFreeQueues[0].set(0, cmd);
    }
#endif
}

void
Curve::SetValuesPointLF(const WDL_TypedBuf<BL_GUI_FLOAT> &xValues,
                              const WDL_TypedBuf<BL_GUI_FLOAT> &yValues)
{
    mPointStyle = true;
    
    ClearValues();
    
    int width = mViewSize[0];
    int height = mViewSize[1];
    
    for (int i = 0; i < xValues.GetSize(); i++)
    {
        BL_GUI_FLOAT x = xValues.Get()[i];
        
        if (i >= yValues.GetSize())
            // Avoids a crash
            continue;
        
        BL_GUI_FLOAT y = yValues.Get()[i];
        
#if !OPTIM_CONVERT_XY
        x = ConvertX(x, width);
        y = ConvertY(y, height);
#endif
        
        mXValues.Get()[i] = x;
        mYValues.Get()[i] = y;
    }
    
#if OPTIM_CONVERT_XY
    ConvertX(&mXValues, width);
    ConvertY(&mYValues, height);
#endif

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetValuesPointEx(const WDL_TypedBuf<BL_GUI_FLOAT> &xValues,
                              const WDL_TypedBuf<BL_GUI_FLOAT> &yValues,
                              bool singleScale, bool scaleX, bool centerFlag)
{
    if (mUseLegacyLock)
    {
        SetValuesPointExLF(xValues, yValues, singleScale, scaleX, centerFlag);
    
        return;
    }
    
    Command &cmd = mTmpBuf9;
    cmd.mType = Command::SET_VALUES_POINT_EX;
    cmd.mXValues = xValues;
    cmd.mYValues = yValues;
 
    cmd.mSingleScale = singleScale;
    cmd.mScaleX = scaleX;
    cmd.mCenterFlag = centerFlag;

#if !OPTIM_LOCK_FREE
    mLockFreeQueues[0].push(cmd);
#else
    if (mLockFreeQueues[0].empty())
        mLockFreeQueues[0].push(cmd);
    else
        mLockFreeQueues[0].set(0, cmd);
#endif
}

void
Curve::SetValuesPointExLF(const WDL_TypedBuf<BL_GUI_FLOAT> &xValues,
                                const WDL_TypedBuf<BL_GUI_FLOAT> &yValues,
                                bool singleScale, bool scaleX, bool centerFlag)
{
    mPointStyle = true;
    
    ClearValues();
    
    int width = mViewSize[0];
    int height = mViewSize[1];
    
    for (int i = 0; i < xValues.GetSize(); i++)
    {
        BL_GUI_FLOAT x = xValues.Get()[i];
        
        if (i >= yValues.GetSize())
            // Avoids a crash
            continue;
        
        BL_GUI_FLOAT y = yValues.Get()[i];
        
#if !OPTIM_CONVERT_XY
        if (!singleScale)
        {
            x = ConvertX(x, width);
            y = ConvertY(y, height);
        }
        else
        {
            if (scaleX)
            {
                x = ConvertX(x, width);
                y = ConvertY(y, width);
                
                // TODO: test this
                if (centerFlag)
                    x -= (width - height)*0.5;
            }
            else
            {
                x = ConvertX(x, height);
                y = ConvertY(y, height);
                
                if (centerFlag)
                    x += (width - height)*0.5;
            }
        }
#endif
        
        mXValues.Get()[i] = x;
        mYValues.Get()[i] = y;
    }
    
#if OPTIM_CONVERT_XY
    if (!singleScale)
    {
        ConvertX(&mXValues, width);
        ConvertY(&mYValues, height);
    }
    else
    {
        if (scaleX)
        {
            ConvertX(&mXValues, width);
            ConvertY(&mYValues, width);
            
            // TODO: test this
            if (centerFlag)
            {
                int offset = (width - height)*0.5;
                for (int i = 0; i < mXValues.GetSize(); i++)
                {
                    mXValues.Get()[i] -= offset;
                }
            }
        }
        else
        {
            ConvertX(&mXValues, height);
            ConvertY(&mYValues, height);
            
            if (centerFlag)
            {
                int offset = (width - height)*0.5;
                for (int i = 0; i < mXValues.GetSize(); i++)
                {
                    mXValues.Get()[i] += offset;
                }
            }
        }
    }
#endif

    mNeedRedraw = true;
    
    NotifyGraph();
}


void
Curve::SetValuesPointWeight(const WDL_TypedBuf<BL_GUI_FLOAT> &xValues,
                                  const WDL_TypedBuf<BL_GUI_FLOAT> &yValues,
                                  const WDL_TypedBuf<BL_GUI_FLOAT> &weights)
{
    mPointStyle = true;
    
    ClearValues();
    
    int width = mViewSize[0];
    int height = mViewSize[1];
    
    for (int i = 0; i < xValues.GetSize(); i++)
    {
        BL_GUI_FLOAT x = xValues.Get()[i];
        BL_GUI_FLOAT y = yValues.Get()[i];
        
#if !OPTIM_CONVERT_XY
        x = ConvertX(x, width);
        y = ConvertY(y, height);
#endif
        
        mXValues.Get()[i] = x;
        mYValues.Get()[i] = y;
    }
    
#if OPTIM_CONVERT_XY
    ConvertX(&mXValues, width);
    ConvertY(&mYValues, height);
#endif
    
    mWeights = weights;

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetColorWeight(const WDL_TypedBuf<BL_GUI_FLOAT> &colorWeights)
{
    mWeights = colorWeights;

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetSingleValueH(bool flag)
{
    mSingleValueH = flag;

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetSingleValueV(bool flag)
{
    mSingleValueV = flag;

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetOptimSameColor(bool flag)
{
    mOptimSameColor = flag;

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::Reset(BL_GUI_FLOAT val)
{
    FillAllXValues(mMinX, mMaxX);
    FillAllYValues(val);

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::FillAllValues(BL_GUI_FLOAT val)
{
    FillAllXValues(mMinX, mMaxX);
    FillAllYValues(val);

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetValues(const WDL_TypedBuf<BL_GUI_FLOAT> *values)
{
    SetYValues(values, mMinX, mMaxX);

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetValues2(const WDL_TypedBuf<BL_GUI_FLOAT> *values)
{
    for (int i = 0; i < values->GetSize(); i++)
    {
        BL_GUI_FLOAT t = ((BL_GUI_FLOAT)i)/(values->GetSize() - 1);
        BL_GUI_FLOAT y = values->Get()[i];
        
        SetValue(t, y);
    }

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetValues3(const WDL_TypedBuf<BL_GUI_FLOAT> *values)
{
    ClearValues();
    
    if (values->GetSize() == 0)
        return;
    
    for (int i = 0; i < values->GetSize(); i++)
    {
        BL_GUI_FLOAT t = ((BL_GUI_FLOAT)i)/(values->GetSize() - 1);
        BL_GUI_FLOAT y = values->Get()[i];
        
        SetValue(t, y);
    }

    mNeedRedraw = true;
    
    NotifyGraph();
}

// Avoid having undefined values
// values (y) must be in amp units
void
Curve::SetValues4(const WDL_TypedBuf<BL_GUI_FLOAT> &values,
                        bool applyYScale)
{
    //if (mYValues.GetSize() != values.GetSize())
    //    return;
    
    // Normalize, then adapt to the graph
    int width = mViewSize[0];
    int height = mViewSize[1];
    
    mXValues.Resize(values.GetSize());
    int numXValues = mXValues.GetSize();
    BL_GUI_FLOAT *xValuesData = mXValues.Get();
    
    BL_GUI_FLOAT t = 0.0;
    BL_GUI_FLOAT tincr = 0.0;
    if (mXValues.GetSize() > 1)
        tincr = 1.0/(mXValues.GetSize() - 1);
    for (int i = 0; i < numXValues; i++)
    {
        //BL_GUI_FLOAT t = ((BL_GUI_FLOAT)i)/(mXValues.GetSize() - 1);
        
        BL_GUI_FLOAT x = mScale->ApplyScale(mXScale, t, mMinX, mMaxX);
        
        // Scale for the interface
        x = x * width;
        
        //mXValues.Get()[i] = x;
        xValuesData[i] = x;

        t += tincr;
    }
    
    mYValues.Resize(values.GetSize());
    int numYValues = mYValues.GetSize();
    BL_GUI_FLOAT *yValuesData = mYValues.Get();
    
    BL_GUI_FLOAT *valuesData = values.Get();
    for (int i = 0; i < numYValues; i++)
    {
        BL_GUI_FLOAT y = valuesData[i];

        if (applyYScale)
            y = ConvertY(y, height);
        else
            y = ConvertYNoScale(y, height);
        
        // For Ghost and mini view
        if (mLimitToBounds)
        {
            if (y < (1.0 - mBounds[3])*height)
                y = (1.0 - mBounds[3])*height;
            
            if (y > (1.0 - mBounds[1])*height)
                y = (1.0 - mBounds[1])*height;
        }
        
        // No need, this is done in ConvertY()
        //y = y * height;
        
        //mYValues.Get()[i] = y;
        yValuesData[i] = y;
    }

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetValues5(const WDL_TypedBuf<BL_GUI_FLOAT> &values,
                        bool applyXScale, bool applyYScale)
{
    if (mUseLegacyLock)
    {
        SetValues5LF(values, applyXScale, applyYScale);
            
        return;
    }
    
    Command &cmd = mTmpBuf9;
    cmd.mType = Command::SET_VALUES5;
    cmd.mValues = values;
    cmd.mApplyXScale = applyXScale;
    cmd.mApplyYScale = applyYScale;

#if !OPTIM_LOCK_FREE
    mLockFreeQueues[0].push(cmd);
#else
    if (mLockFreeQueues[0].empty())
        mLockFreeQueues[0].push(cmd);
    else
        mLockFreeQueues[0].set(0, cmd);
#endif
}

// Avoid having undefined values
// values (y) must be in amp units
//
// And unroll some code
void
Curve::SetValues5LF(const WDL_TypedBuf<BL_GUI_FLOAT> &values,
                          bool applyXScale, bool applyYScale)
{
    // Normalize, then adapt to the graph
    int width = mViewSize[0];
    int height = mViewSize[1];

    // X
    mXValues.Resize(values.GetSize());
    int numXValues = mXValues.GetSize();
    BL_GUI_FLOAT *xValuesData = mXValues.Get();

    BL_GUI_FLOAT t = 0.0;
    BL_GUI_FLOAT tincr = 0.0;
    if (mXValues.GetSize() > 1)
        tincr = 1.0/(mXValues.GetSize() - 1);
    for (int i = 0; i < numXValues; i++)
    {
        xValuesData[i] = t;
        t += tincr;
    }

    if (applyXScale)
        mScale->ApplyScaleForEach(mXScale, &mXValues, mMinX, mMaxX);

    BLUtils::MultValues(&mXValues, (BL_GUI_FLOAT)width);
    
    // Y
    mYValues = values;
    int numYValues = mYValues.GetSize();
    BL_GUI_FLOAT *yValuesData = mYValues.Get();
    
    if (applyYScale)
    {
        if (mYScale != Scale::LINEAR)
            mScale->ApplyScaleForEach(mYScale, &mYValues, mMinY, mMaxY);
        else
            mScale->ApplyScaleForEach(Scale::NORMALIZED, &mYValues, mMinY, mMaxY);
    }
    
    BL_FLOAT factor = mAutoAdjustFactor * mYScaleFactor * height;
    BLUtils::MultValues(&mYValues, factor);

    // For Ghost and mini view
    if (mLimitToBounds)
    {
        for (int i = 0; i < numYValues; i++)
        {
            BL_GUI_FLOAT y = yValuesData[i];
        
            if (y < (1.0 - mBounds[3])*height)
                y = (1.0 - mBounds[3])*height;
            
            if (y > (1.0 - mBounds[1])*height)
                y = (1.0 - mBounds[1])*height;

            yValuesData[i] = y;
        }
    }

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetValuesDecimateSimple(const WDL_TypedBuf<BL_GUI_FLOAT> *values)
{
    int width = mViewSize[0];
    
    BL_GUI_FLOAT prevX = -1.0;
    
    for (int i = 0; i < values->GetSize(); i++)
    {
        BL_GUI_FLOAT t = ((BL_GUI_FLOAT)i)/(values->GetSize() - 1);
        
        BL_GUI_FLOAT x = t*width;
        BL_GUI_FLOAT y = values->Get()[i];
        
        if (x - prevX < 1.0)
            continue;
        
        prevX = x;
        
        SetValue(t, y);
    }
    
    // Avoid last value at 0 !
    // (would make a traversing line in the display)
    BL_GUI_FLOAT lastValue = values->Get()[values->GetSize() - 1];
    SetValue(1.0, lastValue);

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetValuesDecimate(const WDL_TypedBuf<BL_GUI_FLOAT> *values,
                               bool isWaveSignal)
{
    int width = mViewSize[0];
    
    BL_GUI_FLOAT prevX = -1.0;
    BL_GUI_FLOAT maxY = -1.0;
    
    if (isWaveSignal)
        maxY = (mMinY + mMaxY)/2.0;
    
    BL_GUI_FLOAT thrs = 1.0/CURVE_PIXEL_DENSITY;
    for (int i = 0; i < values->GetSize(); i++)
    {
        BL_GUI_FLOAT t = ((BL_GUI_FLOAT)i)/(values->GetSize() - 1);
        
        BL_GUI_FLOAT x = t*width;
        BL_GUI_FLOAT y = values->Get()[i];
        
        // Keep the maximum
        // (we prefer keeping the maxima, and not discard them)
        if (!isWaveSignal)
        {
            if (std::fabs(y) > maxY)
                maxY = y;
        }
        else
        {
            if (std::fabs(y) > std::fabs(maxY))
                maxY = y;
        }
        
        if (x - prevX < thrs)
            continue;
        
        prevX = x;
        
        SetValue(t, maxY);
        
        maxY = -1.0;
        
        if (isWaveSignal)
            maxY = (mMinY + mMaxY)/2.0;
    }

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetValuesDecimate2(const WDL_TypedBuf<BL_GUI_FLOAT> *values,
                                BL_FLOAT decFactor,
                                bool isWaveSignal)
{
    ClearValues();
    
    if (values->GetSize() == 0)
        return;
    
    // Decimate
    WDL_TypedBuf<BL_GUI_FLOAT> &decimValues = mTmpBuf3;
    if (isWaveSignal)
        BLUtilsDecim::DecimateSamples(&decimValues, *values, decFactor);
    else
        BLUtilsDecim::DecimateValues(&decimValues, *values, decFactor);
    
    for (int i = 0; i < decimValues.GetSize(); i++)
    {
        BL_GUI_FLOAT t = ((BL_GUI_FLOAT)i)/(decimValues.GetSize() - 1);
        
        BL_GUI_FLOAT y = decimValues.Get()[i];
        
        SetValue(t, y);
    }

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetValuesDecimate3(const WDL_TypedBuf<BL_GUI_FLOAT> *values,
                                BL_GUI_FLOAT decFactor,
                                bool isWaveSignal)
{
    ClearValues();
    
    if (values->GetSize() == 0)
        return;
    
    // Decimate
    WDL_TypedBuf<BL_GUI_FLOAT> &decimValues = mTmpBuf4;
    if (isWaveSignal)
        BLUtilsDecim::DecimateSamples2(&decimValues, *values, decFactor);
    else
        BLUtilsDecim::DecimateValues(&decimValues, *values, decFactor);
    
    for (int i = 0; i < decimValues.GetSize(); i++)
    {
        BL_GUI_FLOAT t = ((BL_GUI_FLOAT)i)/(decimValues.GetSize() - 1);
        
        BL_GUI_FLOAT y = decimValues.Get()[i];
        
        SetValue(t, y);
    }

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetValuesXDbDecimate(const WDL_TypedBuf<BL_GUI_FLOAT> *values,
                                  int nativeBufferSize, BL_GUI_FLOAT sampleRate,
                                  BL_GUI_FLOAT decimFactor)
{
    int bufferSize = BLUtilsPlug::PlugComputeBufferSize(nativeBufferSize, sampleRate);
    BL_GUI_FLOAT hzPerBin = sampleRate/bufferSize;
    
    BL_GUI_FLOAT minHzValue;
    BL_GUI_FLOAT maxHzValue;
    BLUtils::GetMinMaxFreqAxisValues(&minHzValue, &maxHzValue,
                                     bufferSize, sampleRate);
    
    WDL_TypedBuf<BL_GUI_FLOAT> &logSignal = mTmpBuf5;
    BLUtils::FreqsToDbNorm(&logSignal, *values, hzPerBin,
                           minHzValue, maxHzValue);
    
    WDL_TypedBuf<BL_GUI_FLOAT> &decimSignal = mTmpBuf6;
    BLUtilsDecim::DecimateValues(&decimSignal, logSignal, decimFactor);
    
    for (int i = 0; i < decimSignal.GetSize(); i++)
    {
        BL_GUI_FLOAT t = ((BL_GUI_FLOAT)i) / (decimSignal.GetSize() - 1);
        
        SetValue(t, decimSignal.Get()[i]);
    }

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetValuesXDbDecimateDb(const WDL_TypedBuf<BL_GUI_FLOAT> *values,
                                    int nativeBufferSize, BL_GUI_FLOAT sampleRate,
                                    BL_GUI_FLOAT decimFactor, BL_GUI_FLOAT minValueDb)
{
    int bufferSize = BLUtilsPlug::PlugComputeBufferSize(nativeBufferSize, sampleRate);
    BL_GUI_FLOAT hzPerBin = sampleRate/bufferSize;
    
    BL_GUI_FLOAT minHzValue;
    BL_GUI_FLOAT maxHzValue;
    BLUtils::GetMinMaxFreqAxisValues(&minHzValue, &maxHzValue,
                                     bufferSize, sampleRate);
    
    WDL_TypedBuf<BL_GUI_FLOAT> &logSignal = mTmpBuf7;
    BLUtils::FreqsToDbNorm(&logSignal, *values, hzPerBin,
                           minHzValue, maxHzValue);
    
    WDL_TypedBuf<BL_GUI_FLOAT> &decimSignal = mTmpBuf8;
    BLUtilsDecim::DecimateValuesDb(&decimSignal, logSignal, decimFactor, minValueDb);
    
    for (int i = 0; i < decimSignal.GetSize(); i++)
    {
        BL_GUI_FLOAT t = ((BL_GUI_FLOAT)i) / (decimSignal.GetSize() - 1);
        
        SetValue(t, decimSignal.Get()[i]);
    }

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetValue(BL_GUI_FLOAT t, BL_GUI_FLOAT val)
{
    // Normalize, then adapt to the graph
    int width = mViewSize[0];
    int height = mViewSize[1];
    
    BL_GUI_FLOAT x = t;
    
    if (x < CURVE_VALUE_UNDEFINED) // for float
    {
        x = mScale->ApplyScale(mXScale, x, mMinX, mMaxX);
            
        // X should be already normalize in input
        
        // Scale for the interface
        x = x * width;
    }
    
    BL_GUI_FLOAT y = ConvertY(val, height);
    
    // For Ghost and mini view
    if (mLimitToBounds)
    {
        if (y < (1.0 - mBounds[3])*height)
            y = (1.0 - mBounds[3])*height;
        
        if (y > (1.0 - mBounds[1])*height)
            y = (1.0 - mBounds[1])*height;
    }
    
    SetValue(t, x, y);

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetSingleValueH(BL_GUI_FLOAT val)
{
    SetValue(0.0, val);

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetSingleValueV(BL_GUI_FLOAT val)
{
    SetValue(0.0, val);

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::PushValue(BL_GUI_FLOAT val)
{
    int height = mViewSize[1];
    
    val = ConvertY(val, height);
    
    BL_GUI_FLOAT dummyX = 1.0;
    PushValue(dummyX, val);
    
    BL_GUI_FLOAT maxXValue = mViewSize[0];
    NormalizeXValues(maxXValue);

    mNeedRedraw = true;
    
    NotifyGraph();
}

BL_GUI_FLOAT
Curve::ConvertX(BL_GUI_FLOAT val, BL_GUI_FLOAT width)
{
    BL_GUI_FLOAT x = val;
    if (x < CURVE_VALUE_UNDEFINED)
    {
        if (mXScale != Scale::LINEAR)
            x = mScale->ApplyScale(mXScale, x, mMinX, mMaxX);
        else
            x = (x - mMinX)/(mMaxX - mMinX);
        
        x = x * width;
    }
    
    return x;
}

BL_GUI_FLOAT
Curve::ConvertY(BL_GUI_FLOAT val, BL_GUI_FLOAT height)
{
    BL_GUI_FLOAT y = val;
    if (y < CURVE_VALUE_UNDEFINED)
    {
        if (mYScale != Scale::LINEAR)
            y = mScale->ApplyScale(mYScale, y, mMinY, mMaxY);
        else
            y = (y - mMinY)/(mMaxY - mMinY);
        
        y = y * mAutoAdjustFactor * mYScaleFactor * height;
    }
    
    return y;
}

BL_GUI_FLOAT
Curve::ConvertYNoScale(BL_GUI_FLOAT val, BL_GUI_FLOAT height)
{
    BL_GUI_FLOAT y = val;
    if (y < CURVE_VALUE_UNDEFINED)
    {
        y = y * mAutoAdjustFactor * mYScaleFactor * height;
    }
    
    return y;
}

// Optimized versions
void
Curve::ConvertX(WDL_TypedBuf<BL_GUI_FLOAT> *vals, BL_GUI_FLOAT width)
{
    BL_GUI_FLOAT xCoeff = width/(mMaxX - mMinX);
    
    for (int i = 0; i < vals->GetSize(); i++)
    {
        BL_GUI_FLOAT x = vals->Get()[i];
        if (x < CURVE_VALUE_UNDEFINED)
        {
            if (mXScale == Scale::DB)
            {
                if (x > 0.0)
                {
                    if (std::fabs(x) < BL_EPS)
                        x = mMinX;
                    else
                        x = BLUtils::AmpToDB(x);
                    
                    x = (x - mMinX)*xCoeff;
                }
            }
            else if ((mXScale == Scale::LOG) ||
                     (mXScale == Scale::LOG10) ||
                     (mXScale == Scale::LOG_FACTOR) ||
                     (mXScale == Scale::MEL))
            {
                // Not optimized
                x = mScale->ApplyScale(mXScale, x, mMinX, mMaxX);
                
            }
            else
            {
                x = (x - mMinX)*xCoeff;
            }
        }
        
        vals->Get()[i] = x;
    }

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::ConvertY(WDL_TypedBuf<BL_GUI_FLOAT> *vals,
                      BL_GUI_FLOAT height)
{
    BL_GUI_FLOAT yCoeff =
        mAutoAdjustFactor * mYScaleFactor*height/(mMaxY - mMinY);
    
    for (int i = 0; i < vals->GetSize(); i++)
    {
        BL_GUI_FLOAT y = vals->Get()[i];
        if (y < CURVE_VALUE_UNDEFINED)
        {
            if (mYScale == Scale::DB)
            {
                if (y > 0.0)
                {
                    if (std::fabs(y) < BL_EPS)
                        y = mMinY;
                    else
                        y = BLUtils::AmpToDB(y);
                    
                    y = (y - mMinY)*yCoeff;
                }
            }
            else if ((mYScale == Scale::LOG) ||
                     (mYScale == Scale::LOG10) ||
                     (mYScale == Scale::LOG_FACTOR) ||
                     (mYScale == Scale::MEL))
            {
                // Not optimized
                y = mScale->ApplyScale(mYScale, y, mMinY, mMaxY);
            }
            else
            {
                y = (y - mMinY)*yCoeff;
            }
        }
        
        vals->Get()[i] = y;
    }

    mNeedRedraw = true;
    
    NotifyGraph();
}

void
Curve::SetGraph(GraphControl12 *graph)
{
    mGraph = graph;

    mNeedRedraw = true;
}

void
Curve::NotifyGraph()
{
    if (mGraph != NULL)
        // Notify the graph
        mGraph->SetDataChanged();
}

void
Curve::PushData()
{
    // Must dirty this CustomDrawer if some data is pushed
    // Because GraphControl12::IsDirty() is called before PullAllData(),
    // which is called in GraphControl12::Draw()
    if (!mLockFreeQueues[0].empty())
        mNeedRedraw = true;

#if !OPTIM_LOCK_FREE
    mLockFreeQueues[1].push(mLockFreeQueues[0]);
#else
    if (mLockFreeQueues[1].empty())
        mLockFreeQueues[1].push(mLockFreeQueues[0]);
    else
        mLockFreeQueues[1].set(0, mLockFreeQueues[0]);
#endif
    
    mLockFreeQueues[0].clear();
}

void
Curve::PullData()
{
#if !OPTIM_LOCK_FREE
    mLockFreeQueues[2].push(mLockFreeQueues[1]);
#else
    if (mLockFreeQueues[2].empty())
        mLockFreeQueues[2].push(mLockFreeQueues[1]);
    else
        mLockFreeQueues[1].set(0, mLockFreeQueues[1]);
#endif
    
    mLockFreeQueues[1].clear();
}

void
Curve::ApplyData()
{    
    for (int i = 0; i < mLockFreeQueues[2].size(); i++)
    {
        Command &cmd = mTmpBuf10;
        mLockFreeQueues[2].get(i, cmd);

        if (cmd.mType == Command::SET_VALUES5)
        {
            SetValues5LF(cmd.mValues, cmd.mApplyXScale, cmd.mApplyYScale);
        }
        else if (cmd.mType == Command::SET_VALUES_POINT)
        {
            SetValuesPointLF(cmd.mXValues, cmd.mYValues);
        }
        else if (cmd.mType == Command::SET_VALUES_POINT_EX)
        {
            SetValuesPointExLF(cmd.mXValues, cmd.mYValues,
                               cmd.mSingleScale, cmd.mScaleX, cmd.mCenterFlag);
        }
    }

    mLockFreeQueues[2].clear();
}

bool
Curve::NeedRedraw()
{
    return mNeedRedraw;
}

void
Curve::DrawDone()
{
    mNeedRedraw = false;
}
