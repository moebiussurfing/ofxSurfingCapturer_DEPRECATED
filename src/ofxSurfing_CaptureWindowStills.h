#pragma once

#include "ofMain.h"

// TODO: 
// + check/allow change window capture size without breaking the capturer

///----
///
#define USE_3D_DEPTH
#define ANTIALIAS_NUM_SAMPLES 16
///
///----

#include "ofxTextureRecorder.h"
#include "ofxSurfingHelpers2.h"

class CaptureWindow : public ofBaseApp, public ofThread
{
private:
	std::string pathRoot;
	ofTrueTypeFont font;

	bool bFfmpegLocated = false;
	bool bDepth3D = true;

	bool bUseFfmpegNvidiaGPU = true;
public:
	//--------------------------------------------------------------
	void setFfpmegGpu(bool b) {
		bUseFfmpegNvidiaGPU = b;
	}
public:
	//--------------------------------------------------------------
	void setPathRoot(std::string path) {
		// NOTES:
		//Poco::Path dataFolder(ofToDataPath("", true));
		//ofToDataPath()
		//std::filesystem::path("some/path") == std::filesystem::path("some//path")

		pathRoot = path;
	}

	//--------------------------------------------------------------
	void doRunFFmpegCommand() {
		ofLogWarning(__FUNCTION__) << " to: " << _pathFolderStills;

		// we are running the systems commands
		// in a sperate thread so that it does
		// not block the drawing
		startThread();
	}

public:
	//--------------------------------------------------------------
	CaptureWindow()
	{
		cap_w = 1920;
		cap_h = 1080;

		_pathFolderCaptures = "captures\\";
		_pathFolderStills = _pathFolderCaptures + "Stills\\";
		_pathFolderSnapshots = _pathFolderCaptures + "Snapshots\\";

		//string _font = "assets/fonts/telegrama_render.otf";
		string _font = "assets/fonts/overpass-mono-bold.otf";
		font.load(_font, 8);

		// TODO:
		//pathRoot = ofToDataPath("", false);
	};

	//--------------------------------------------------------------
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
	ofParameter<bool> bActive{ "Window Capturer", false };// public to integrate into your ofApp gui
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

public:
	//--------------------------------------------------------------
	//call with the path folder if you want to customize
	void setup(std::string path = "captures\\", ofImageFormat format = OF_IMAGE_FORMAT_TIFF) {
		ofLogWarning(__FUNCTION__) << "path: " << path << " ofImageFormat: " << format;

		// we can select a still format passing one ofImageFormat like this ones:
		//	OF_IMAGE_FORMAT_BMP = 0,
		//	OF_IMAGE_FORMAT_JPEG = 2,
		//	OF_IMAGE_FORMAT_PNG = 13,
		//	OF_IMAGE_FORMAT_TIFF = 18,
		//	OF_IMAGE_FORMAT_RAW = 34

		_pathFolderCaptures = path; // "captures\\"
		_pathFolderStills = _pathFolderCaptures + "Stills\\";
		_pathFolderSnapshots = _pathFolderCaptures + "Snapshots\\";

		ofxSurfingHelpers2::CheckFolder(_pathFolderCaptures);
		ofxSurfingHelpers2::CheckFolder(_pathFolderStills);
		ofxSurfingHelpers2::CheckFolder(_pathFolderSnapshots);

		buildInfo();

		cap_w = ofGetWidth();
		cap_h = ofGetHeight();
		cap_Fbo.allocate(cap_w, cap_h, GL_RGB);

		stillFormat = format;

		buildRecorder();

		buildAllocateFbo();

		// locate ffmpeg .exe to allow ffmpeg script. but not mandatory if you join the stills using another software...
		ofFile file;
		std::string _pathFfmpeg;
		_pathFfmpeg = pathRoot + "ffmpeg.exe";
		bFfmpegLocated = file.doesFileExist(_pathFfmpeg, true);
		if (bFfmpegLocated) ofLogWarning(__FUNCTION__) << "Located: ffmpeg.exe into " << _pathFfmpeg;
		else ofLogError(__FUNCTION__) << "Missing required binary file ffmpeg.exe into " << _pathFfmpeg << " !";
	}

public:
	//--------------------------------------------------------------
	void buildAllocateFbo() {//cap_w and cap_h must be updated
		ofLogWarning(__FUNCTION__) << cap_w << ", " << cap_h;

		cap_Fbo_Settings.internalformat = GL_RGB;
		cap_Fbo_Settings.width = cap_w;
		cap_Fbo_Settings.height = cap_h;

#ifdef USE_3D_DEPTH
		if (bDepth3D) {
			cap_Fbo_Settings.useDepth = true;// required to enable depth test
			cap_Fbo_Settings.numSamples = ANTIALIAS_NUM_SAMPLES;// antialias 
			// BUG: seems like on ofxFastFboReader, requires an aux blitFbo... ??
			// this can be copied from ofxSurfing_CaptureWindowFFMPEG.h
			//cap_Fbo_Settings.useStencil = true;
			//cap_Fbo_Settings.depthStencilAsTexture = true;
			//cap_Fbo_Settings.maxFilter
		}
#endif
		cap_Fbo.allocate(cap_Fbo_Settings);
	}
	//--------------------------------------------------------------
	void buildRecorder() {
		ofLogWarning(__FUNCTION__);

		// TODO: trying to allow resize..
		//recorder.stop();

		ofxTextureRecorder::Settings settings(cap_Fbo.getTexture());

		settings.imageFormat = stillFormat;

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
		cap_Fbo.draw(0, 0);// drawing is required outside fbo
	}

	//--------------------------------------------------------------
	void drawInfo(int x = 40, int y = 0) {// draw the gui info if desired

		if (bShowInfo && bActive) {

			string str = "\n\n\n";

			//--

			// 1. waiting mount: press F8
			if (!bRecPrepared && !isThreadRunning() && !bRecording)
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
			// 2. mounted, recording or running ffmpeg script
			else if (bRecPrepared || bRecording || isThreadRunning())
			{
				// cap info
				str += "FPS " + ofToString(ofGetFrameRate(), 0) + "   " + ofToString(recorder.getFrame()) + " frames\n";
				str += "WINDOW   " + ofToString(ofGetWidth()) + "x" + ofToString(ofGetHeight()); str += +"\n";
				str += "FBO SIZE " + ofToString(cap_w) + "x" + ofToString(cap_h); str += +"\n";
				str += "RECORDER " + ofToString(recorder.getWidth()) + "x" + ofToString(recorder.getHeight());
				str += +"\n\n";

				if (bRecording)
				{
					str += "F9 : STOP Recording\n";
					str += "RECORD DURATION: " + ofToString(getRecordedDuration(), 1); str += +"\n";

					// error
					if (bRecording) {
						if (recorder.getAvgTimeSave() == 0) {
							std::string ss;
							const int p = 30;// blink period in frames
							int fn = ofGetFrameNum() % p;
							if (fn < p / 2) ss = "ERROR RECORDING!";
							else ss = "";
							str += ss + "\n";
						}
					}
				}
				else if (bRecPrepared || isThreadRunning())// mounted or running ffmpeg script
				{
					str += "F9 : START Recording\n";
					str += "F8 : UnMount Recorder\n";
					if ((!bFfmpegLocated) && ofGetFrameNum() % 60 < 20) str += "> ALERT! Missing ffmpeg.exe...\n";
					else str += "\n";

					// animated points..
					const int p = 30;// period in frames
					int fn = ofGetFrameNum() % (p * 4);
					bool b0, b1, b2;
					b0 = (fn > p * 3);
					b1 = (fn > p * 2);
					b2 = (fn > p * 1);
					string sp = "";
					if (b0) sp += ".";
					if (b1) sp += ".";
					if (b2) sp += ".";

					if (isThreadRunning()) { str += "> BUILDING STILLS TO VIDEO" + sp; str += "\n"; }
					else {
						str += "> MOUNTED! READY" + sp; str += "\n";
						if (b1 || b2) { str += "> PRESS F9 TO START CAPTURE"; }
						str += "\n";
						str += "> PRESS F11 TO RUN FFMPEG SCRIPT\n";
					}
				}
			}

			//-

			// draw

			str += info;

			float h = ofxSurfingHelpers2::getHeightBBtextBoxed(font, str);// TODO: ? makes a bad offset..

			y = ofGetHeight() - h - 90;

			ofxSurfingHelpers2::drawTextBoxed(font, str, x, y);
			//ofDrawBitmapStringHighlight(str, x, y);

			//-

			// red circle

			int yy = y + 50;
			x += 210;
			float radius = 10;

			if (!isThreadRunning()) {
				ofPushStyle();
				if (bRecording)
				{
					ofFill();
					ofSetColor(ofColor::red);
					ofDrawCircle(ofPoint(x + radius, yy), radius);
					ofNoFill();
					ofSetLineWidth(2.f);
					ofSetColor(ofColor::black);
					ofDrawCircle(ofPoint(x + radius, yy), radius);
				}
				else if (bRecPrepared)
				{
					if (ofGetFrameNum() % 60 < 20) {
						ofFill();
						ofSetColor(ofColor::red);
						ofDrawCircle(ofPoint(x + radius, yy), radius);
					}
					ofNoFill();
					ofSetLineWidth(2.f);
					ofSetColor(ofColor::black);
					ofDrawCircle(ofPoint(x + radius, yy), radius);
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

				// toggle show info
			case 'h':
				setToggleVisibleInfo();
				break;

				// set Full HD
			case OF_KEY_F5:
				ofSetWindowShape(1920, 1080);
				windowResized(1920, 1080);
				break;

				// set instagram size
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

			// refresh window size to update fbo settings
			case OF_KEY_F7:
				windowResized(ofGetWidth(), ofGetHeight());
				break;

				// mount prepare record
			case OF_KEY_F8:
			{
				bRecPrepared = !bRecPrepared;
				ofLogWarning(__FUNCTION__) << "Mount: " << (bRecPrepared ? "ON" : "OFF");
			}
			break;

			// start recording
			case OF_KEY_F9:
			{
				if (bRecording)// do stop
				{
					ofLogWarning(__FUNCTION__) << "Stop Recording";

					//bRecPrepared = false;
					bRecording = false;
				}
				else// do start
				{
					bRecording = true;
					timeStart = ofGetElapsedTimeMillis();
					ofLogWarning(__FUNCTION__) << "Start Recording into: " << _pathFolderStills;
				}
			}
			break;

			// take screenshot
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

			// join stills to video after capture
			case OF_KEY_F11:
			{
				doRunFFmpegCommand();
			}
			break;

			// remove all captures stills
			case OF_KEY_BACKSPACE: // ctrl + alt + backspace
				if (!mod_COMMAND && !mod_SHIFT && mod_ALT && mod_CONTROL)
				{
					std::string _path = _pathFolderStills;
					ofDirectory dataDirectory(ofToDataPath(_path, true));
					dataDirectory.remove(true);
					ofxSurfingHelpers2::CheckFolder(_path);

				}
				break;
			}
		}
	}

	//--------------------------------------------------------------
	void windowResized(int w, int h) {// must be called to resize the fbo and video resolution
		cap_w = w;
		cap_h = h;
		buildAllocateFbo();

		// TODO: trying to allow resize..
		// this brakes the capturer...
		//buildRecorder();
	}

private:
	//--------------------------------------------------------------
	void buildInfo() {// must be called after bitrate, framerate and size w/h are setted

		//build help info
		info = "\n";
		info += "HELP KEYS"; info += "\n";
		info += "h  : Show Help info"; info += "\n";
		info += "F5 : Set FullHD size"; info += "\n";
		info += "F6 : Set optimal Instagram size"; info += "\n";
		info += "F7 : Refresh Window size"; info += "\n";
		info += "F8 : Mount Recorder"; info += "\n";
		info += "F9 : Start/Stop Recording"; info += "\n";
		info += "F10: Capture Screenshot"; info += "\n";
		info += "F11: Run FFmpeg join video"; info += "\n";
		info += "Ctrl+Alt+BackSpace: Clear Stills ";// info += "\n";
		//info += "path Stills     : "+ _pathFolderStills; info += "\n";
		//info += "path Screenshots: "+ _pathFolderSnapshots; info += "\n";
	}

	// join all stills to a video file
	//--------------------------------------------------------------
	void threadedFunction() {
		cout << "--------------------------------------------------------------" << endl;
		ofLogWarning(__FUNCTION__) << endl;
		cout << (__FUNCTION__) << endl;

		string warninglog = "";
		warninglog += "> WARNING! ofApp.exe must run as Administrator !\n";
		warninglog += "> WARNING! ffmpeg.exe must be located on: " + pathRoot + "/ffmpeg.exe !\n";

		cout << endl << warninglog << endl;

		if (isThreadRunning())
		{
			// build ffmpeg command

			cout << "> Starting join all stills (.tif) to a video file (.mp4)...";

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

			//-

			// used template to join stills:

			// template 1:
			// this  intended to be a lossless preset
			// ffmpeg -r 60 -f image2 -s 1920x1080 -i %05d.tif -c:v libx264 -preset veryslow -qp 0 output.mp4 // lossless

			// template 2:
			// TODO: search for a GPU encoder script...
			// https://developer.nvidia.com/blog/nvidia-ffmpeg-transcoding-guide/

			// command:

			// template 1: (cpu)
			if (!bUseFfmpegNvidiaGPU)
				cmd << ffmpeg << " -r 60 -f image2 -s 1920x1080 -i " << filesSrc << " -c:v libx264 -preset veryslow -qp 0 " << fileOut;

			// template 2: (Nvidia gpu)
			else if (bUseFfmpegNvidiaGPU)
				cmd << ffmpeg << " -r 60 -f image2 -s 1920x1080 -i " << filesSrc << " -c:v h264_nvenc -b:v 5M " << fileOut;
			//cmd << ffmpeg << " -r 60 -f image2 -s 1920x1080 -i " << filesSrc << " -c:v h264_nvenc -qp 0 " << fileOut;
			//cmd << ffmpeg << " -vsync 0 -hwaccel cuvid -c:v h264_cuvid -i " << filesSrc << " -c:a copy -c:v h264_nvenc -b:v 5M " << fileOut;

			//-

			cout << endl << endl;
			cout << "> ffmpeg.exe : " << endl << ffmpeg.str().c_str();
			cout << endl << endl;
			cout << "> Source : " << endl << filesSrc.str().c_str();
			cout << endl << endl;
			cout << "> Out : " << endl << fileOut.str().c_str();
			cout << endl << endl;
			cout << "> Command: " << endl << cmd.str().c_str();
			cout << endl << endl;

			string slog;

			// run
			slog = ofSystem(cmd.str().c_str());
			cout << endl << "> Log: " << endl << slog << endl;

			//-

			// loop repeat if above while
			//ofSleepMillis(5000);
			//cout << "repeat" << endl;

			cout << endl;
			cout << "> Done/Trying video encoding into: " << endl << fileOut.str().c_str();
			cout << endl << endl;
			cout << "--------------------------------------------------------------" << endl << endl;

			//-

			// open video
			slog = ofSystem(fileOut.str().c_str());
			cout << slog << endl;

			// error seems to print this:
			//" is not recognized as an internal or external command,
			//operable program or batch file."

			cout << "> WARNING: if output video is not opened NOW, and getting above an error like:" << endl;
			cout << "\t'is not recognized as an internal or external command, operable program or batch file.'" << endl;
			cout << "> Then probably you need to set your ofApp.exe settings to run as Administrator:" << endl;
			cout << "> Use Windows File Explorer file properties / Change settings for all users / compatibility." << endl;
			cout << "> This is to allow run ffmpeg.exe and access to files from here!" << endl;

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
		}
	}
};