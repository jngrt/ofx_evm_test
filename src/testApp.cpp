#include "testApp.h"
using namespace cv;
using namespace ofxCv;

const int input_width  = 320;
const int input_height = 240;

vector<string> temporal_filter;
EVM_TEMPORAL_FILTER filter = EVM_TEMPORAL_IIR;
//iir params
float alpha_iir = 10;
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

//--------------------------------------------------------------
void testApp::setup(){
    ofSetVerticalSync(true);
    ofEnableSmoothing();

    //setup video source
    videoSetup();
    
    //setup UI control panel
    guiSetup();
    
    cvSetup();
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
    }    
}

//--------------------------------------------------------------
void testApp::draw(){
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	ofBackgroundGradient(ofColor(64), ofColor(0));

    ofPushStyle(); {
        ofPushMatrix(); {
            ofTranslate(360, 0);
            float scale = (float) ofGetHeight() / vid.height / 2;
            ofScale(scale, scale);
            glDisable(GL_DEPTH_TEST);
            vid.draw(0, 0);
            ofTranslate(0, ofGetHeight()*0.5/scale);
            evm.draw(0, 0);
            
            for(int i = 0; i < finder.size(); i++) {
                
                ofRectangle object = finder.getObjectSmoothed(i);
                ofPoint forehead = object.getCenter();
                forehead.y -= 30;
                calculateColor(forehead);
                
                ofPushMatrix(); {
                    ofTranslate(forehead);
                    ofRect(0,0,10,10);
                } ofPopMatrix();
                
                
                /*sunglasses.setAnchorPercent(.5, .5);
                float scaleAmount = .85 * object.width / sunglasses.getWidth();
                ofPushMatrix();{
                    ofTranslate(object.x + object.width / 2., object.y + object.height * .42);
                    ofScale(scaleAmount, scaleAmount);
                    sunglasses.draw(0, 0);
                } ofPopMatrix();
                ofPushMatrix();{
                    ofTranslate(object.getPosition());
                    ofDrawBitmapStringHighlight(ofToString(finder.getLabel(i)), 0, 0);
                    ofLine(ofVec2f(), toOf(finder.getVelocity(i)) * 10);
                } ofPopMatrix();
                 */
            }
            if( colorValues.size()>1 )
            {
                float lastVal = colorValues[colorValues.size()-1];
            
                ofDrawBitmapStringHighlight(ofToString(lastVal), 100, 0);
            }
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

void testApp::calculateColor(ofPoint& p)
{
    float total = 0;
    ofColor color;
    //vector<int> values;
    for(int i = 0; i < 10; i++) {
        for( int j=0; j< 10; j++) {
            color = evm.frame.getColor(p.x+i,p.y+j);
            total += color.r;
            //values.push_back(color.r);
        }
    }
    colorValues.push_back(total/100);
    if( colorValues.size() > 100 )
    {
        colorValues.erase(colorValues.begin(),colorValues.begin()+(colorValues.size()-100));
    }
    
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
    
    ofAddListener(gui->newGUIEvent, this, &testApp::guiEvent);
}
