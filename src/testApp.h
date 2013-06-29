#pragma once

#include "ofMain.h"
#include "ofxEvm.h"
#include "ofxUI.h"

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

private:
    ofxUICanvas *gui;
    ofxEvm evm;
    ofxCv::ObjectFinder finder;
    ofImage sunglasses;
    
    vector<float> colorValues;
    
//--- VIDEO INPUT ---//
#ifdef USE_WEBCAM
    ofVideoGrabber vid;
#else
    ofVideoPlayer vid;
#endif
//---//---//---//---//
    
};
