#ifndef WIENER_SOFT_MASKING_H
#define WIENER_SOFT_MASKING_H

#include <bl_queue.h>

// Wiener soft masking applied on complexes
//
// See: https://github.com/TUIlmenauAMS/ASP/blob/master/MaskingMethods.py
// and: http://www.jonathanleroux.org/pdf/Erdogan2015ICASSP04.pdf
// and: https://www.researchgate.net/publication/220736985_Degenerate_Unmixing_Estimation_Technique_using_the_Constant_Q_Transform
// and: https://hal.inria.fr/inria-00544949/document
// and: https://hal.inria.fr/hal-01881425/document
class WienerSoftMasking
{
public:
    WienerSoftMasking(int bufferSize, int overlap, int historySize);
    
    virtual ~WienerSoftMasking();

    void reset();
    
    void reset(int bufferSize, int overlap);
    
    void setHistorySize(int size);
    
    int getHistorySize();
    
    void setProcessingEnabled(bool flag);
    bool isProcessingEnabled();

    int getLatency();
    
    // Returns the centered data value in ioSum
    // Returns the centered masked data in ioMaskedResult0
    // Return sum - maskedResult0 in  ioMaskedResult1 if required
    void processCentered(vector<complex<float> > *ioSum,
                         const vector<float> &mask,
                         vector<complex<float> > *ioMaskedResult0,
                         vector<complex<float> > *ioMaskedResult1 = NULL);
               
protected:
    void computeSigma2(vector<complex<float> > *outSigma2Mask0,
                       vector<complex<float> > *outSigma2Mask1);
                       
    
    class HistoryLine
    {
    public:
        HistoryLine();
        HistoryLine(const HistoryLine &other);

        virtual ~HistoryLine();

        void resize(int size);
        int getSize();
        
    public:
        vector<complex<float> > _sum;
        
        vector<complex<float> > _masked0Square;
        vector<complex<float> > _masked1Square;

    protected:
        int _size;
    };

    int _bufferSize;
    int _overlap;
    
    int _historySize;
    
    bl_queue<HistoryLine> _history;
    
    vector<float> _window;
    
    bool _processingEnabled;

private:
    HistoryLine _tmpHistoryLine;
    vector<complex<float> > _tmpBuf0;
    vector<complex<float> > _tmpBuf1;
    vector<complex<float> > _tmpBuf2;
    vector<complex<float> > _tmpBuf3;
    vector<complex<float>> _tmpBuf4;
    vector<complex<float> > _tmpBuf5;
    vector<float> _tmpBuf6;
    vector<float> _tmpBuf7;
};

#endif
