#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetCircleResolution(300);

	//capturer.setup();// default: captures, PNG
	capturer.setup("captures/", OF_IMAGE_FORMAT_TIFF);// customize
}

//--------------------------------------------------------------
void ofApp::update() {
	capturer.begin();
	{
		ofClear(0, 255);

		float val = (ofGetFrameNum() % 120) / 120.f;
		float radius = ofMap(val, 0, 1, 20, 100);
		float hue = ofMap(val, 0, 1, 50, 200);
		float y = ofMap(val, 0, 1, 400, 200);

		ofFill();
		ofColor c;
		c.setBrightness(200);
		c.setSaturation(128);
		c.setHue(hue);
		ofSetColor(c);
		float x = 200 * ofNoise(val) + ofGetWidth() / 2.0f;

		ofDrawCircle(x, y, radius);
	}
	capturer.end();
}

//--------------------------------------------------------------
void ofApp::draw() {

	capturer.draw();
	
	capturer.drawInfo();
}

//--------------------------------------------------------------
void ofApp::keyPressed(ofKeyEventArgs &eventArgs) {

	capturer.keyPressed(eventArgs);
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {
	capturer.windowResized(w, h);
}