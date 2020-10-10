#pragma once

#include "ofMain.h"

#include "ofxSurfing_CaptureWindowStills.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		void keyPressed(ofKeyEventArgs &eventArgs);
		void windowResized(int w, int h);
		
		CaptureWindow capturer;
};
