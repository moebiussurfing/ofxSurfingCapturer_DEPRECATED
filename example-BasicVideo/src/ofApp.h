#pragma once

#include "ofMain.h"

#include "ofxSurfing_CaptureWindowFFMPEG.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void draw();
		void keyPressed(ofKeyEventArgs &eventArgs);
		void windowResized(int w, int h);

		CaptureWindow capturer;
};
