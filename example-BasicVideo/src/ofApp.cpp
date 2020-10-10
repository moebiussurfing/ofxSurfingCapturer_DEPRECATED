#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetCircleResolution(300);
	capturer.setup();
}

//--------------------------------------------------------------
void ofApp::draw() {
	capturer.begin();
	{
		ofPushStyle();
		ofClear(16, 255);
		float val = (ofGetFrameNum() % 120) / 120.f;
		float radius = ofMap(val, 0, 1, 70, 200);
		float hue = ofMap(val, 0, 1, 0, 100);
		float y = ofMap(val, 0, 1, 400, 200);
		ofFill();
		ofColor c;
		c.setBrightness(100);
		c.setSaturation(200);
		c.setHue(hue);
		ofSetColor(c);
		float x = 200 * ofNoise(val) + ofGetWidth() / 2.0f;
		ofDrawCircle(x, y, radius);
		ofPopStyle();
	}
	capturer.end();

	capturer.draw();
	capturer.drawHelp();
}

//--------------------------------------------------------------
void ofApp::keyPressed(ofKeyEventArgs &eventArgs) {
	capturer.keyPressed(eventArgs);
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {
	capturer.windowResized(w, h);
}

