/*
 - get color value over scaled rectangle instead of fixed
 - draw a graph
 - detect heartbeat...
 */


#pragma once

#include "ofMain.h"
#include "ofxEvm.h"
#include "ofxUI.h"
#include "ofxGraphViewer.h"
#include "ofxFft.h"

#define USE_WEBCAM

class testApp : public ofBaseApp{

public:
    void setup();
    void update();
    void draw();
    void exit();

    void keyPressed  (int key);    
    void guiEvent(ofxUIEventArgs& e);
    
    void videoSetup();
    void guiSetup();
    void cvSetup();
    
    float calculateColor();
    void plotFullFft(vector<float>& buffer, float scale, float offset, float width);
    void plotPartialFft(vector<float>& buffer, float scale, float offset, float width, int startIndex, int endIndex);
    
    void updateDetection();
    float getAverageFramerate();
    
    float indexToBpm( float index );
    float bpmToIndex( float bpm );
private:
    ofxUICanvas *gui;
    ofxEvm evm;
    ofxCv::ObjectFinder finder;
    ofImage sunglasses;
    
    vector<float> colorValues;
    vector<float> framerates;
    ofxGraphViewer gViewer1;
    
    ofPoint forehead;
    
    ofxFft* fft;
    
    int plotHeight, bufferSize;
    
    vector<float> fullFftBins, partialFftBins; //, audioBins;

    float heartRate;
    
    ofTrueTypeFont fftFont;
    
    

    
//--- VIDEO INPUT ---//
#ifdef USE_WEBCAM
    ofVideoGrabber vid;
#else
    ofVideoPlayer vid;
#endif
//---//---//---//---//
    
};
