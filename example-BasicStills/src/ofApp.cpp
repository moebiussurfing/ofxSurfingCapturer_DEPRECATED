#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetFrameRate(60);

	// scene

	ofSetCircleResolution(300);
	halfx = ofGetWidth() / 2;
	halfy = ofGetHeight() / 2;
	pos = glm::vec2(halfx, halfy);

	//-

	// TODO: testing to simplify...
	// set /rootPath/
	//cout << defaultWorkingDirectory();
	//cout << ofEnableDataPath
	//cout << ofToDataPath
	//capturer.setPathRoot(ofToDataPath());

	//--

	// capturer

	capturer.setPathRoot("F:\\openFrameworks\\addons\\ofxSurfingCapturer\\example-BasicStills\\bin\\data\\");
	// "ffmpeg.exe" -> must be located on /rootPath/
	// look for /Snapshots/ and /Stills/ on there
	// converted output video "output_2020-10-10-03-48-49-172.mp4" will be placed into: /pathRoot/captures/
	
	capturer.setup(); 
	// customizable destination, ffmpeg.exe and IMAGE_FORMAT
	// default destination is bin/data/captures
	// default image format is TIFF

	capturer.setActive(true); 
	// make gui visible. hidden by default
	
	//capturer.setFfpmegGpu(false); 
	// Nvidia GPU HW accelerated is enabled by default. 
	// disable if you have AMD GPU or you prefer slower CPU encoding
	
	//capturer.setOverwriteVideoOut(false); 
	// true by default. overwirtes out.mp4 file. 
	// set to false to allow add timestamp to filenames
}

//--------------------------------------------------------------
void ofApp::update() {
	// scene
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
		//-- draw your scene here --//

		// scene
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

	capturer.draw();// draw scene

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