#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetFrameRate(60);

	ofSetCircleResolution(300);
	halfx = ofGetWidth() / 2;
	halfy = ofGetHeight() / 2;
	pos = glm::vec2(halfx, halfy);

	// default detination is bin/data/captures
	// default image format is TIFF
	capturer.setup(); // customizable using setup("destination\\", OF_IMAGE_FORMAT_PNG);

	capturer.setActive(true);
	//capturer.setVisibleInfo(true);
}

//--------------------------------------------------------------
void ofApp::update() {
	float w = 100;
	val = (ofGetFrameNum() % 120) / 120.f;// value from 0 to 1 in 2 secconds at 60 fps
	if ((ofGetFrameNum() % 60) == 0)//randomize 
	{
		pos = glm::vec2(ofRandom(halfx - w, halfx + w), ofRandom(halfy - w, halfy + w));
		size = ofRandom(1,10);
	}
}

//--------------------------------------------------------------
void ofApp::draw() {

	capturer.begin();
	{
		// draw your scene here //

		ofClear(val * 32, 255);
		ofPushStyle();
		float radius = size * ofMap(val, 0, 1, 20, 100);
		float hue = ofMap(val, 0, 1, 50, 200);
		float y = ofMap(val, 0, 1, -200, 200);
		ofFill();
		ofColor c;
		c.setBrightness(200);
		c.setSaturation(128);
		c.setHue(hue);
		ofSetColor(c);
		float x = 200 * ofNoise(val);
		ofDrawCircle(pos.x + x, pos.y + y, radius);
		ofPopStyle();
	}
	capturer.end();

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