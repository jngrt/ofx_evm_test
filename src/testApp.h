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
    
    void calculateColor(ofPoint& p);
    void plot(vector<float>& buffer, float scale, float offset);


private:
    ofxUICanvas *gui;
    ofxEvm evm;
    ofxCv::ObjectFinder finder;
    ofImage sunglasses;
    
    vector<float> colorValues;
    ofxGraphViewer gViewer1;
    
    ofxFft* fft;
    
    int plotHeight, bufferSize;
    
    vector<float> drawBins, middleBins, audioBins;

    float heartRate;
    
//--- VIDEO INPUT ---//
#ifdef USE_WEBCAM
    ofVideoGrabber vid;
#else
    ofVideoPlayer vid;
#endif
//---//---//---//---//
    
};
