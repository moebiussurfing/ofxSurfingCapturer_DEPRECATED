	#pragma once

#include "ofMain.h"

#include "ofxSurfing_CaptureWindowStills.h"

#include "ofxGui.h"

class ofApp : public ofBaseApp {

public:
	void setup();
	void update();
	void draw();
	void keyPressed(ofKeyEventArgs &eventArgs);
	void windowResized(int w, int h);

	// scene
	glm::vec2 pos;
	int halfx, halfy;
	float size;
	float val;

	CaptureWindow capturer;
	
	ofxPanel gui;
};
