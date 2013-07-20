#include "testApp.h"

using namespace cv;
using namespace ofxCv;

const int input_width  = 320;
const int input_height = 240;

vector<string> temporal_filter;
EVM_TEMPORAL_FILTER filter = EVM_TEMPORAL_IIR;
//iir params
float alpha_iir = 25;
float lambda_c_iir = 16;
float r1 = 0.4;
float r2 = 0.05;
float chromAttenuation_iir = 0.1;
//ideal params
float alpha_ideal = 150;
float lambda_c_ideal = 6;
float fl = 140.0/60.0;
float fh = 160.0/60.0;
float samplingRate = 30;
float chromAttenuation_ideal = 1;

float graphLow = 0.0;
float graphHigh = 256.0;

float sampleOffsetY = 40;


//--------------------------------------------------------------
void testApp::setup(){
    ofSetVerticalSync(true);
    ofEnableSmoothing();
    ofSetFrameRate(samplingRate);

    
    //setup video source
    videoSetup();
    
    //setup UI control panel
    guiSetup();
    
    cvSetup();
    
    gViewer1.setup(1024);

    plotHeight = 100;
    bufferSize = 512;
	
    fft = ofxFft::create(bufferSize, OF_FFT_WINDOW_HAMMING, OF_FFT_FFTW);
    fullFftBins.resize(fft->getBinSize());
	partialFftBins.resize(fft->getBinSize());
//	audioBins.resize(fft->getBinSize());
    
    fftFont.loadFont("Arial.ttf", 10);

}

//--------------------------------------------------------------
void testApp::update(){
    //update EVM parameters
    evm.setTemporalFilter(filter);
    evm.setParamsIIR(alpha_iir, lambda_c_iir, r1, r2, chromAttenuation_iir);
    evm.setParamsIdeal(alpha_ideal, lambda_c_ideal, fl, fh, samplingRate, chromAttenuation_ideal);
    
    //process the frame
    vid.update();
    if (vid.isFrameNew()) {
        evm.update(toCv(vid));
        
        finder.update(vid);
        
        updateDetection();
    }    
}

//--------------------------------------------------------------
void testApp::draw(){
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	ofBackgroundGradient(ofColor(64), ofColor(0));

    
    float stageWidth = ofGetWidth() - 360;
    float scale = (float) stageWidth / vid.width / 2;
    
    ofPushStyle(); {
        ofPushMatrix(); {
            ofTranslate(360, 0);
            //float scale = (float) ofGetHeight() / vid.height / 2;
            ofScale(scale, scale);
            glDisable(GL_DEPTH_TEST);
            vid.draw(0, 0);
            //ofTranslate(0, ofGetHeight()*0.5/scale);
            ofTranslate( stageWidth * 0.5 / scale, 0);
            evm.draw(0, 0);
            
            // draw sample area
            ofPushMatrix(); {
                ofTranslate(forehead);
                ofRect(0,0,10,10);
            } ofPopMatrix();
                        
        } ofPopMatrix();
        ofPushMatrix(); {
            ofTranslate(360, vid.height*scale);
            if( colorValues.size()>1 )
            {
                float lastVal = colorValues[colorValues.size()-1];
            
                ofDrawBitmapStringHighlight(ofToString(lastVal), 100, 0);
            }
            ofTranslate(0,0);
            
            gViewer1.setRange(graphLow, graphHigh);
            gViewer1.setSize(ofGetWidth()-360, 200);//ofGetHeight()-(vid.height*scale));
            gViewer1.draw(0, 0);
            
        } ofPopMatrix();
        ofPushMatrix(); {
            ofTranslate(360, vid.height * scale + 220);
            plotFullFft(fullFftBins, -plotHeight, plotHeight / 2, ofGetWidth()-360);
            
            ofTranslate(0,150);
            plotPartialFft(partialFftBins, -plotHeight, plotHeight / 2, ofGetWidth()-360, bpmToIndex(30), bpmToIndex(200));
        } ofPopMatrix();
    } ofPopStyle();
    
}

//--------------------------------------------------------------
void testApp::exit()
{
    delete gui;
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    gui->toggleVisible();
}

//--------------------------------------------------------------
void testApp::guiEvent(ofxUIEventArgs &e)
{
    if (e.widget->getName() == "Temporal IIR")
    {
        filter = EVM_TEMPORAL_IIR;
    }
    if (e.widget->getName() == "Temporal Ideal (Unimplemented)") {
        filter = EVM_TEMPORAL_IDEAL;
    }
}


float testApp::indexToBpm( float index ) {
    
    const double Hz = static_cast<double>(index) / (fft->getBinSize()-1) * (samplingRate / 2.0);
    //const double Hz = static_cast<double>(i) * samplingRate / fft->getBinSize();
    
    return 60 * Hz;
}
float testApp::bpmToIndex( float bpm ) {
    //   12 / 3 * 2 = 8
    // 12 = 8 / 2 * 3
    const double hz = bpm / 60;
    return hz / (samplingRate / 2.0) * (fft->getBinSize()-1);
}


void testApp::plotFullFft(vector<float>& buffer, float scale, float offset,float width) {
	ofNoFill();
	int n = buffer.size();
    float sizePerSample = width / n;
	ofRect(0, 0, n * sizePerSample, plotHeight);
    
	glPushMatrix();{
        glTranslatef(0, plotHeight / 2 + offset, 0);
        ofBeginShape();{
            for (int i = 0; i < n; i++) {
                ofVertex(i*sizePerSample, sqrt(buffer[i]) * scale);
            }
        } ofEndShape();
    } glPopMatrix();
    glPushMatrix(); {
        for (int i = 0; i < n; i+=10) {
            ofLine(i*sizePerSample,0,i*sizePerSample,plotHeight);
            fftFont.drawString(ofToString(indexToBpm(i),0), i*sizePerSample, plotHeight+30);
        }
    } glPopMatrix();
}

void testApp::plotPartialFft(vector<float>& buffer, float scale, float offset, float width, int startIndex, int endIndex){
    
    ofNoFill();
	int n = endIndex - startIndex;
    float sizePerSample = width / n;
	ofRect(0, 0, n * sizePerSample, plotHeight);
    
	glPushMatrix();{
        glTranslatef(0, plotHeight / 2 + offset, 0);
        ofBeginShape();{
            for (int i = 0; i < n; i++) {
                ofVertex(i*sizePerSample, sqrt(buffer[startIndex+i]) * scale);
            }
        } ofEndShape();
    } glPopMatrix();
    glPushMatrix(); {
        for (int i = 0; i < n; i+=2) {
            ofLine(i*sizePerSample,0,i*sizePerSample,plotHeight);
            fftFont.drawString(ofToString(indexToBpm(i+startIndex),0), i*sizePerSample, plotHeight+30);
        }
    } glPopMatrix();
}


void testApp::updateDetection() {
    
    float averageColor = calculateColor();
    
    gViewer1.pushData( averageColor );
    colorValues.push_back( averageColor );
    
    
    if( colorValues.size() > bufferSize )
    {
        colorValues.erase(colorValues.begin(),colorValues.begin()+(colorValues.size()-bufferSize));
    }
    
    
    fft->setSignal(colorValues);
    
    
    float* curFft = fft->getAmplitude();
	memcpy(&fullFftBins[0], curFft, sizeof(float) * fft->getBinSize());
	
    partialFftBins = fullFftBins;
    
	//normalize values for full FFT range
    float maxValue = 0;
	for(int i = 0; i < fft->getBinSize(); i++) {
        if(abs(fullFftBins[i]) > maxValue ) {
            maxValue = abs(fullFftBins[i]);
		}
    }
    for ( int i = 0; i < fft->getBinSize(); i++ ) {
		fullFftBins[i] /= maxValue;
	}


    maxValue = 0;
	for(int i = 0; i < fft->getBinSize(); i++) {
        
        const float bpm = indexToBpm(i);
        if(abs(partialFftBins[i]) > maxValue && bpm > 30 && bpm < 200 ) {
			
            maxValue = abs(partialFftBins[i]);
            heartRate = bpm;
		}
    }
    
    for ( int i = 0; i < fft->getBinSize(); i++ ) {
		partialFftBins[i] /= maxValue;
	}
  
    
}

float testApp::calculateColor()
{
  
    if ( finder.size() == 0 ) {
        forehead.set(0,0);
        return ( colorValues.size() > 0 )? *(colorValues.end()) : 0.0;
    }

    ofRectangle object = finder.getObjectSmoothed(0);
    forehead = object.getCenter();
    forehead.y -= sampleOffsetY;
    
    float total = 0;
    ofColor color;

    for(int i = 0; i < 10; i++) {
        for( int j=0; j< 10; j++) {
            //color = evm.frame.getColor(forehead.x+i,forehead.y+j);
            color = evm.getMagnifiedImage()
            total += color.r;
        }
    }
    
    return total / 100.0;
}


//--------------------------------------------------------------
void testApp::cvSetup()
{
    finder.setup("haarcascade_frontalface_alt2.xml");
	finder.setPreset(ObjectFinder::Fast);
	finder.getTracker().setSmoothingRate(.3);
    
    sunglasses.loadImage("sunglasses.png");
	ofEnableAlphaBlending();
}

//--------------------------------------------------------------
void testApp::videoSetup()
{
#ifdef USE_WEBCAM
    vid.initGrabber(input_width,input_height);
    
#else
    vid.loadMovie("wrist");
    if (!vid.isLoaded()) {
        ofLogError();
        ofExit();
    }
    vid.play();
#endif
}

//--------------------------------------------------------------
void testApp::guiSetup()
{
    ofxXmlSettings xml;
    xml.loadFile("settings.xml");
	
	float dim = 20;
	float xInit = OFX_UI_GLOBAL_WIDGET_SPACING;
	float length = 360 - xInit;
	gui = new ofxUICanvas(0, 0, length + xInit * 2, 2560);
	gui->setFont("/System/Library/Fonts/Geneva.dfont");
	ofColor cb(64, 192),
	co(192, 192),
	coh(128, 192),
	cf(240, 255),
	cfh(128, 255),
	cp(96, 192),
	cpo(255, 192);
	gui->setUIColors(cb, co, coh, cf, cfh, cp, cpo);
	
    gui->addLabel("EULERIAN VIDEO MAGNIFICATION", OFX_UI_FONT_LARGE);
    gui->addWidgetDown(new ofxUIFPS(OFX_UI_FONT_MEDIUM));
    
    gui->addSpacer(length, 2);
	gui->addLabel("TEMPORAL FILTER", OFX_UI_FONT_LARGE);
    temporal_filter.push_back("Temporal IIR");
    temporal_filter.push_back("Temporal Ideal (Unimplemented)");
    gui->addRadio("SELECT TEMPORAL FILTER TYPE", temporal_filter, OFX_UI_ORIENTATION_VERTICAL, dim, dim);
    
    gui->addSpacer(length, 2);
    gui->addLabel("IIR FILTER PARAMETER", OFX_UI_FONT_LARGE);
    gui->addSlider("Amplification", 0, 100, &alpha_iir, length, dim);
    gui->addSlider("Cut-off Wavelength", 0, 100, &lambda_c_iir, length, dim);
    gui->addSlider("r1 (Low cut-off?)", 0, 1, &r1, length, dim);
    gui->addSlider("r2 (High cut-off?)", 0, 1, &r2, length, dim);
	gui->addSlider("ChromAttenuation", 0, 1, &chromAttenuation_iir, length, dim);
    
    gui->addSpacer(length, 2);
    gui->addLabel("IDEAL FILTER PARAMETER", OFX_UI_FONT_LARGE);
    gui->addSlider("Amplification", 0, 200, &alpha_ideal, length, dim);
    gui->addSlider("Cut-off Wavelength", 0, 100, &lambda_c_ideal, length, dim);
    gui->addSlider("Low cut-off", 0, 10, &fl, length, dim);
    gui->addSlider("High cut-off", 0, 10, &fh, length, dim);
    gui->addSlider("SamplingRate", 1, 60, &samplingRate, length, dim);
    gui->addSlider("ChromAttenuation", 0, 1, &chromAttenuation_ideal, length, dim);
    
    gui->addSpacer(length, 2);
    gui->addLabel("GRAPH PROPERTIES", OFX_UI_FONT_LARGE);
    gui->addSlider("Low Y",0,256,&graphLow,length,dim);
    gui->addSlider("High Y",0,256,&graphHigh,length,dim);
    
    gui->addSpacer(length, 2);
    gui->addLabel("COLOR SAMPLE AREA", OFX_UI_FONT_LARGE);
    gui->addSlider("y offset",0,100,&sampleOffsetY,length,dim);
    gui->addSpacer(length, 2);
    gui->addLabel("Heart Rate: ", OFX_UI_FONT_LARGE);
    gui->addSlider("heart rate",0,250,&heartRate,length,dim);
    
    ofAddListener(gui->newGUIEvent, this, &testApp::guiEvent);
}
