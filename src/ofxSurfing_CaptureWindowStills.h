#pragma once

#include "ofMain.h"

// TODO: 
// + allow change window capture size without breaking the capturer

///----
///
#define INCLUDE_RECORDER
///
///----

#include "ofxTextureRecorder.h"
#include "ofxSurfingHelpers.h"

class CaptureWindow : public ofBaseApp, public ofThread
{

public:
	std::string pathRoot;
	void setPathRoot(std::string path) {
		pathRoot = path;
	}

	//--------------------------------------------------------------
	void doBuildFFmpeg() {
		ofLogWarning(__FUNCTION__) << " to: " << _pathFolderStills;

		// we are running the systems commands
		// in a sperate thread so that it does
		// not block the drawing
		startThread();
	}

public:
	CaptureWindow()
	{
		cap_w = 1920;
		cap_h = 1080;

		_pathFolderCaptures = "captures\\";
		_pathFolderStills = _pathFolderCaptures + "Stills\\";
		_pathFolderSnapshots = _pathFolderCaptures + "Snapshots\\";
	};

	~CaptureWindow() {
		// stop the thread on exit
		waitForThread(true);
	};

private:
	ofxTextureRecorder recorder;

	ofFbo cap_Fbo;
	ofFbo::Settings cap_Fbo_Settings;
	int cap_w, cap_h;

	string _pathFolderCaptures;
	string _pathFolderStills;
	string _pathFolderSnapshots;

private:
	bool bRecPrepared = false;
	bool bRecording;
	bool bShowInfo = true;

public:
	ofParameter<bool> bActive{ "Window Capturer", false };
	//--------------------------------------------------------------
	void setActive(bool b) {
		bActive = b;
	}

	//--------------------------------------------------------------
	void setToggleActive() {
		bActive = !bActive;
	}

public:
	//--------------------------------------------------------------
	void setVisibleInfo(bool b) {
		bShowInfo = b;
	}
	//--------------------------------------------------------------
	void setToggleVisibleInfo() {
		bShowInfo = !bShowInfo;
	}

private:
	//--------------------------------------------------------------
	float getRecordedDuration() {
		return (ofGetElapsedTimeMillis() - timeStart) / 1000.f;
	}

private:
	uint32_t timeStart;
	std::string info;

	ofImageFormat stillFormat;
	//enum stillsImageFormat {
	//	OF_IMAGE_FORMAT_BMP = 0,
	//	OF_IMAGE_FORMAT_JPEG = 2,
	//	OF_IMAGE_FORMAT_PNG = 13,
	//	OF_IMAGE_FORMAT_TIFF = 18,
	//	OF_IMAGE_FORMAT_RAW = 34
	//};

public:
	//--------------------------------------------------------------
	//call with the path folder if you want to customize
	void setup(std::string path = "captures\\", ofImageFormat format = OF_IMAGE_FORMAT_TIFF) {
		ofLogWarning(__FUNCTION__) << "path: " << path << " ofImageFormat: " << format;

		_pathFolderCaptures = path; // "captures\\"
		_pathFolderStills = _pathFolderCaptures + "Stills\\";
		_pathFolderSnapshots = _pathFolderCaptures + "Snapshots\\";

		ofxSurfingHelpers::CheckFolder(_pathFolderCaptures);
		ofxSurfingHelpers::CheckFolder(_pathFolderStills);
		ofxSurfingHelpers::CheckFolder(_pathFolderSnapshots);

		buildInfo();

		cap_w = ofGetWidth();
		cap_h = ofGetHeight();
		cap_Fbo.allocate(cap_w, cap_h, GL_RGB);

		stillFormat = format;

		buildRecorder();

		buildAllocateFbo();
	}

public:
	//--------------------------------------------------------------
	void buildAllocateFbo() {//cap_w and cap_h must be updated
		ofLogWarning(__FUNCTION__) << cap_w << ", " << cap_h;

		cap_Fbo_Settings.internalformat = GL_RGB;
		cap_Fbo_Settings.width = cap_w;
		cap_Fbo_Settings.height = cap_h;
		cap_Fbo.allocate(cap_Fbo_Settings);
	}
	//--------------------------------------------------------------
	void buildRecorder() {
		ofLogWarning(__FUNCTION__);

		//recorder.stop();// TODO: trying to allo resize..

		ofxTextureRecorder::Settings settings(cap_Fbo.getTexture());
		settings.imageFormat = stillFormat;

		//settings.imageFormat = OF_IMAGE_FORMAT_TIFF;
		//settings.imageFormat = OF_IMAGE_FORMAT_PNG;
		//settings.imageFormat = OF_IMAGE_FORMAT_JPEG;
		//settings.imageFormat = OF_IMAGE_FORMAT_RAW;

		settings.numThreads = 12;
		settings.maxMemoryUsage = 9000000000;

		recorder.setPath(_pathFolderStills);
		recorder.setup(settings);
	}

	// TODO: window resize ??
	////--------------------------------------------------------------
	//void refreshRecorder() {
	//	//recorder = ofxTextureRecorder();
	//	//recorder.settings
	//	//ofxTextureRecorder::Settings
	//	//recorder.
	//}

public:
	//--------------------------------------------------------------
	void begin() {///call before draw the scene to record
		//if (bActive) 
		{
			cap_Fbo.begin();
			ofClear(0, 255);
		}
	}

	//--------------------------------------------------------------
	void end() {///call after draw the scene to record
		//if (bActive) 
		{
			cap_Fbo.end();

			//-

			if (bRecPrepared)
			{
				if (bRecording && ofGetFrameNum() > 0)
				{
					recorder.save(cap_Fbo.getTexture());
				}
			}
		}
	}

	//--------------------------------------------------------------
	void draw() {
		cap_Fbo.draw(0, 0);//drawing is required outside fbo
	}

	//--------------------------------------------------------------
	void drawInfo() {///draw the gui info if desired

		if (bShowInfo && bActive) {
			int x = 40;
			int y = ofGetHeight() - 300;
			string str = "\n\n";

			//--

			if (bRecPrepared || bRecording || isThreadRunning())
			{
				//cap info
				str += "FPS " + ofToString(ofGetFrameRate(), 0) + "   " + ofToString(recorder.getFrame()) + " frames"; str += +"\n";
				str += "WINDOW   " + ofToString(ofGetWidth()) + "x" + ofToString(ofGetHeight()); str += +"\n";
				str += "FBO SIZE " + ofToString(cap_w) + "x" + ofToString(cap_h); str += +"\n";
				str += "RECORDER " + ofToString(recorder.getWidth()) + "x" + ofToString(recorder.getHeight());
				str += +"\n"; str += +"\n";


				if (bRecording)
				{
					str += "F9 : STOP Recording"; str += "\n";
					str += "RECORD DURATION: " + ofToString(getRecordedDuration(), 1); str += +"\n";

					//error
					if (bRecording) {
						if (recorder.getAvgTimeSave() == 0) {
							std::string ss;
							const int p = 30;//blink period in frames
							int fn = ofGetFrameNum() % p;
							if (fn < p / 2) ss = "ERROR RECORDING!";
							else ss = "";
							str += ss + "\n";
						}
					}
				}
				else if (bRecPrepared || isThreadRunning())
				{
					str += "F9 : START Recording"; str += "\n";
					str += "F8 : UnMount Recorder"; str += "\n";

					//animated points..
					const int p = 30;//period in frames
					int fn = ofGetFrameNum() % (p * 4);
					bool b0, b1, b2;
					b0 = (fn > p * 3);
					b1 = (fn > p * 2);
					b2 = (fn > p * 1);
					string sp = "";
					if (b0) sp += ".";
					if (b1) sp += ".";
					if (b2) sp += ".";

					if (isThreadRunning()) { str += "BUILDING STILLS TO VIDEO" + sp; str += "\n"; }
					else {
						str += "> MOUNTED! READY" + sp; str += "\n";
						if (b1 || b2) { str += "> PRESS F9 TO START CAPTURE"; }
						str += "\n";
					}
				}
			}

			//-

			// press F8
			else if (!bRecPrepared && !isThreadRunning() && !bRecording)
			{
				str += "F8 : MOUNT Recorder"; str += "\n";

				//animated points..
				const int p = 30;//period in frames
				int fn = ofGetFrameNum() % (p * 4);
				bool b0, b1, b2;
				b0 = (fn > p * 3);
				b1 = (fn > p * 2);
				b2 = (fn > p * 1);
				string sp = "";
				if (b0) sp += ".";
				if (b1) sp += ".";
				if (b2) sp += ".";

				str += "> PRESS F8" + sp; str += "\n";
			}

			//-

			//draw
			str += info;
			ofDrawBitmapStringHighlight(str, x, y);

			//-

			//red circle

			int yy = y;
			if (!isThreadRunning()) {
				ofPushStyle();
				if (bRecording)
				{
					ofFill();
					ofSetColor(ofColor::red);
					ofDrawCircle(ofPoint(x + 8, yy), 8);
					ofNoFill();
					ofSetLineWidth(2.f);
					ofSetColor(ofColor::black);
					ofDrawCircle(ofPoint(x + 8, yy), 8);
				}
				else if (bRecPrepared)
				{
					if (ofGetFrameNum() % 60 < 20) {
						ofFill();
						ofSetColor(ofColor::red);
						ofDrawCircle(ofPoint(x + 8, yy), 8);
					}
					ofNoFill();
					ofSetLineWidth(2.f);
					ofSetColor(ofColor::black);
					ofDrawCircle(ofPoint(x + 8, yy), 8);
				}
				ofPopStyle();
			}


			//-

			// log
			if (bRecording)
			{
				if (ofGetFrameNum() % 60 == 0) {
					ofLogWarning(__FUNCTION__) << ofGetFrameRate();
					ofLogWarning(__FUNCTION__) << "texture copy: " << recorder.getAvgTimeTextureCopy();
					ofLogWarning(__FUNCTION__) << "gpu download: " << recorder.getAvgTimeGpuDownload();
					ofLogWarning(__FUNCTION__) << "image encoding: " << recorder.getAvgTimeEncode();
					ofLogWarning(__FUNCTION__) << "file save: " << recorder.getAvgTimeSave() << endl;
				}
			}
		}
	}

	////--------------------------------------------------------------
	//void drawHelp(int x = 50, int y = 50) {
	//	// help info
	//	ofDrawBitmapStringHighlight(info, x, y);

	//}

	//--------------------------------------------------------------
	void keyPressed(ofKeyEventArgs &eventArgs) {///to received short keys control commands
		if (bActive)
		{
			const int key = eventArgs.key;

			// modifiers
			bool mod_COMMAND = eventArgs.hasModifier(OF_KEY_COMMAND);
			bool mod_CONTROL = eventArgs.hasModifier(OF_KEY_CONTROL);
			bool mod_ALT = eventArgs.hasModifier(OF_KEY_ALT);
			bool mod_SHIFT = eventArgs.hasModifier(OF_KEY_SHIFT);

			//-

			switch (key)
			{
				//	//toggle active
				//case 'a':
				//	setToggleActive();
				//	break;

				//toggle show info
			case 'h':
				setToggleVisibleInfo();
				break;

				//set full HD
			case OF_KEY_F5:
				ofSetWindowShape(1920, 1080);
				windowResized(1920, 1080);
				break;

				//set instagram size
			case OF_KEY_F6:
			{
				int w, h;
				w = 864;
				h = 1080 + 19;
				ofSetWindowShape(w, h);
				windowResized(w, h);

				cap_w = w;
				cap_h = h;
				buildAllocateFbo();
			}
			break;

			//refresh window size to update fbo settings
			case OF_KEY_F7:
				windowResized(ofGetWidth(), ofGetHeight());
				break;

				//mount prepare record
			case OF_KEY_F8:
			{
				bRecPrepared = !bRecPrepared;
				ofLogWarning(__FUNCTION__) << "Mount: " << (bRecPrepared ? "ON" : "OFF");
			}
			break;

			//start recording
			case OF_KEY_F9:
			{
				if (bRecording)//do stop
				{
					ofLogWarning(__FUNCTION__) << "Stop Recording";

					//bRecPrepared = false;
					bRecording = false;
				}
				else//do start
				{
					bRecording = true;
					timeStart = ofGetElapsedTimeMillis();
					ofLogWarning(__FUNCTION__) << "Start Recording into: " << _pathFolderStills;
				}
			}
			break;

			//take screenshot
			case OF_KEY_F10:
			{
				string _fileName = "snapshot_" + ofGetTimestampString() + ".png";
				string _pathFilename = ofToDataPath(_pathFolderSnapshots + _fileName, true);//bin/data

				ofImage img;
				img.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
				bool b = img.save(_pathFilename);

				if (b) ofLogWarning(__FUNCTION__) << " Saved screenshot successfully: " << _pathFilename;
				else ofLogWarning(__FUNCTION__) << " Error saving screenshot:" << _pathFilename;
			}
			break;

			//join stills to video after capture
			case OF_KEY_F11:
			{
				doBuildFFmpeg();
			}
			break;

			//remove all captures stills
			case OF_KEY_BACKSPACE: // ctrl + alt + backspace
				if (!mod_COMMAND && !mod_SHIFT && mod_ALT && mod_CONTROL)
				{
					std::string _path = _pathFolderStills;
					ofDirectory dataDirectory(ofToDataPath(_path, true));
					dataDirectory.remove(true);
					ofxSurfingHelpers::CheckFolder(_path);

				}
				break;
			}
		}
	}

	//--------------------------------------------------------------
	void windowResized(int w, int h) {///must be called to resize the fbo and video resolution
		cap_w = w;
		cap_h = h;
		buildAllocateFbo();
		//buildRecorder();
	}

private:
	//--------------------------------------------------------------
	void buildInfo() {///must be called after bitrate, framerate and size w/h are setted

		//build help info
		info = "\n";
		info += "HELP KEYS"; info += "\n";
		info += "h  : Show Help info"; info += "\n";
		info += "F5 : Set FullHD size"; info += "\n";
		info += "F6 : Set optimal Instagram size"; info += "\n";
		info += "F7 : Refresh Window size"; info += "\n";
		info += "F8 : Mount Recorder"; info += "\n";
		info += "F9 : Start Recording"; info += "\n";
		info += "F10: Capture Screenshot"; info += "\n";
		info += "F11: Join Stills to video"; info += "\n";
		info += "Ctrl+Alt+BackSpace: Clear Stills ";// info += "\n";
		//info += "path Stills     : "+ _pathFolderStills; info += "\n";
		//info += "path Screenshots: "+ _pathFolderSnapshots; info += "\n";
	}

	// join all stills to a video file
	//--------------------------------------------------------------
	void threadedFunction() {
		ofLogWarning(__FUNCTION__) << " Must run as Adniministrator!" << endl;
		cout << (__FUNCTION__) << " Must run as Adniministrator!" << endl;

		////while (isThreadRunning()) {
		if (isThreadRunning())
		{
			// build ffmpeg command

			cout << "> Starting join all stills to a video file " << endl;

			// template to join stills
			// ffmpeg -r 60 -f image2 -s 1920x1080 -i %05d.tif -c:v libx264 -preset veryslow -qp 0 output.mp4 // lossless

			stringstream cmd;
			stringstream ffmpeg;
			stringstream filesSrc;
			stringstream pathDest;
			stringstream nameDest;
			stringstream fileOut;
			stringstream pathAppData;

			pathAppData << pathRoot;

			ffmpeg << pathAppData.str().c_str() << "ffmpeg.exe";
			//ffmpeg << pathAppData.str().c_str() << _pathFolderCaptures << "ffmpeg.exe";

			pathDest << pathAppData.str().c_str() << _pathFolderCaptures;
			filesSrc << pathAppData.str().c_str() << _pathFolderStills << "%05d.tif"; // data/stills/%05d.tif
			nameDest << "output_" << ofGetTimestampString() << ".mp4"; // "output.mp4";
			fileOut << pathDest.str().c_str() << nameDest;

			// command:
			cmd << ffmpeg << " -r 60 -f image2 -s 1920x1080 -i " << filesSrc << " -c:v libx264 -preset veryslow -qp 0 " << fileOut;

			cout << endl << endl;
			cout << "ffmpeg : " << ffmpeg.str().c_str();
			cout << endl << endl;
			cout << "Source : " << filesSrc.str().c_str();
			cout << endl << endl;
			cout << "Out    : " << fileOut.str().c_str();
			cout << endl << endl;
			cout << "Command: " << cmd.str().c_str();
			cout << endl << endl;

			// run
			cout << ofSystem(cmd.str().c_str()) << endl;

			//-

			// loop repeat if above while
			//ofSleepMillis(5000);
			//cout << "repeat" << endl;

			cout << endl;
			cout << "> Done video encoding into: " << fileOut.str().c_str();
			cout << endl;

			//-

			// open video
			cout << ofSystem(fileOut.str().c_str()) << endl;

			//--

			// some examples

			//cout << ofSystem("cd data\\") << endl;
			//cout << ofSystem("dir") << endl;
			//cout << ofSystem("cd captures") << endl;
			//cout << ofSystem("dir") << endl;

			//stringstream someCmd;
			//someCmd.clear();
			//someCmd << "dir";
			//cout << someCmd << endl;
			//cout << ofSystem(someCmd.str().c_str()) << endl;

			//-

		}
		//else waitForThread(true);
	}
};