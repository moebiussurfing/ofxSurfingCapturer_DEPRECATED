#pragma once

#include "ofMain.h"

// TODO: 
// + check/allow change window capture size without breaking the capturer. not it's posible before call setup()
// + allow gif exporter
// + allow instagram ready export
// + draw rectangle draggable border
//
// nice settings handling and binaries: we can copy things from here
// https://github.com/tyhenry/ofxFFmpeg/blob/master/src/ofxFFmpeg.h

#define ANTIALIAS_NUM_SAMPLES 16

#include "ofxTextureRecorder.h"
#include "ofxSurfingHelpers2.h"

// platforms
#ifdef TARGET_OSX
#endif
#ifdef TARGET_WIN32
#endif
//#ifdef TARGET_LINUX
//#endif

class CaptureWindow : public ofBaseApp, public ofThread
{

public:
	std::stringstream cmd;
	std::stringstream cmdEncodingArgs;
	std::stringstream ffmpeg;
	std::stringstream filesSrc;
	std::stringstream pathDest;
	std::stringstream nameDest;
	std::stringstream fileOut;
	std::stringstream pathAppData;

	//                std::ostringstream cmd;
	//                std::ostringstream cmdEncodingArgs;
	//                std::ostringstream ffmpeg;
	//                std::ostringstream filesSrc;
	//                std::ostringstream pathDest;
	//                std::ostringstream nameDest;
	//                std::ostringstream fileOut;
	//                std::ostringstream pathAppData;

	//--

public:
	ofParameter<bool> bActive{ "Window Capturer", true };// public to integrate into your ofApp gui

private:
	ofxTextureRecorder recorder;

private:
	ofFbo cap_Fbo;
	ofFbo::Settings cap_Fbo_Settings;
	int cap_w, cap_h;

	ofImageFormat stillFormat;

private:
	std::string _pathFolderCaptures;
	std::string _pathFolderStills;
	std::string _pathFolderSnapshots;

	ofDirectory dataDirectory;
	int amountStills = 0;

private:
	bool isMounted = false;
	bool isRecording;
	bool bShowInfo = true;
	bool bError = false;

private:
	uint32_t timeStart;
	std::string infoHelpKeys;
	std::string infoFFmpeg;
	std::string info;
public:
	void setShowMinimal(bool b) {
		bShowMinimal = b;
	};
	void setToggleShowMinimal() {
		bShowMinimal = !bShowMinimal;
	};
private:
	bool bShowMinimal = true;// hide more info when recording to improve performance a little
	bool isEncoding = false;
	bool isPlayingPLayer = false;

	ofRectangle rectSection;// customize capture section. TODO: hardcoded to 0,0 position yet!
	bool isSectionCustomized = false;

private:
	std::string pathRoot;
	ofTrueTypeFont font;

private:
	bool bFfmpegLocated = false;
	bool bFfmpegCustomScript = false;
	std::string ffmpegScript;
	std::string _nameBinary;

private:
	//bool bDepth3D = false;// disabled by default. must enable before call setup()
	bool bDepth3D = true;// enabled by default. must disable before call setup()

	// BUG solved: when using antialias/depth we get "black screen"
	ofFbo blitFbo;// so we need this aux fbo to solve the bug

private:
	bool bOverwriteOutVideo = true;// we only want the last video. we use same name for all re takes and overwrite.

public:
	//--------------------------------------------------------------
	CaptureWindow()
	{
		cap_w = 1920;
		cap_h = 1080;

#ifdef TARGET_WIN32
		_pathFolderCaptures = "Captures\\";
		_pathFolderStills = _pathFolderCaptures + "Stills\\";
		_pathFolderSnapshots = _pathFolderCaptures + "Snapshots\\";
		// default root path is /bin/data/
		setPathRoot(ofToDataPath("\\", true));
#endif

#ifdef TARGET_OSX
		_pathFolderCaptures = "Captures/";
		_pathFolderStills = _pathFolderCaptures + "Stills/";
		_pathFolderSnapshots = _pathFolderCaptures + "Snapshots/";
		// default root path is /bin/data/
		setPathRoot(ofToDataPath("/", true));
#endif

		std::string _font = "assets/fonts/overpass-mono-bold.otf";
		bool b = font.load(_font, 8);
		if (!b) font.load(OF_TTF_SERIF, 8);// solve font file not found OF bundled alternative font
	};

	//--------------------------------------------------------------
	~CaptureWindow() {
		// stop the thread on exit
		waitForThread(true);
	};

public:
	//call with the path folder if you want to customize
#ifdef TARGET_WIN32
	//--------------------------------------------------------------
	void setup(std::string path = "Captures\\", ofImageFormat format = OF_IMAGE_FORMAT_TIFF) {
#endif
#ifdef TARGET_OSX
		//--------------------------------------------------------------
		void setup(std::string path = "Captures/", ofImageFormat format = OF_IMAGE_FORMAT_TIFF) {
#endif
			ofLogWarning(__FUNCTION__) << "path: " << path << " ofImageFormat: " << format;

			// we can select a still format passing one ofImageFormat like this ones:
			// OF_IMAGE_FORMAT_BMP = 0,
			// OF_IMAGE_FORMAT_JPEG = 2,
			// OF_IMAGE_FORMAT_PNG = 13,
			// OF_IMAGE_FORMAT_TIFF = 18,
			// OF_IMAGE_FORMAT_RAW = 34

			_pathFolderCaptures = path; // "Captures\\"
#ifdef TARGET_WIN32
			_pathFolderStills = _pathFolderCaptures + "Stills\\";
			_pathFolderSnapshots = _pathFolderCaptures + "Snapshots\\";
#endif
#ifdef TARGET_OSX
			_pathFolderStills = _pathFolderCaptures + "Stills/";
			_pathFolderSnapshots = _pathFolderCaptures + "Snapshots/";
#endif
			ofxSurfingHelpers2::CheckFolder(_pathFolderCaptures);
			ofxSurfingHelpers2::CheckFolder(_pathFolderStills);
			ofxSurfingHelpers2::CheckFolder(_pathFolderSnapshots);

			buildHepKeysInfo();

			if (!isSectionCustomized) {
				cap_w = ofGetWidth();
				cap_h = ofGetHeight();
				buildAllocateFbo();
			}

			stillFormat = format;// selectable image format
			buildRecorder();// setup ofxTextureRecorder 

			// locate ffmpeg .exe to allow ffmpeg script.
			// but not mandatory if you join the stills using another external software.
			ofFile file;
			std::string _pathFfmpeg;
#ifdef TARGET_WIN32
			_nameBinary = "ffmpeg.exe";
#endif
#ifdef TARGET_OSX
			_nameBinary = "ffmpeg";
#endif
			_pathFfmpeg = pathRoot + _nameBinary;
			bFfmpegLocated = file.doesFileExist(_pathFfmpeg, true);
			if (bFfmpegLocated) ofLogWarning(__FUNCTION__) << "Located: " + _nameBinary + " into " << _pathFfmpeg;
			else ofLogError(__FUNCTION__) << "Missing required binary file " + _nameBinary + " into " << _pathFfmpeg << " !";

			// stills folder
			// let the folder open to list amount files sometimes...
			dataDirectory.open(ofToDataPath(_pathFolderStills, true));
			amountStills = dataDirectory.listDir();
		}

public:
	//--------------------------------------------------------------
	void buildAllocateFbo() {// cap_w and cap_h must be updated before call
		ofLogWarning(__FUNCTION__) << cap_w << ", " << cap_h;

		cap_Fbo_Settings.internalformat = GL_RGB;
		cap_Fbo_Settings.width = cap_w;
		cap_Fbo_Settings.height = cap_h;

		if (bDepth3D)
		{
			cap_Fbo_Settings.useDepth = true;// required to enable depth test
			cap_Fbo_Settings.numSamples = (int)ANTIALIAS_NUM_SAMPLES;// antialias 
			//cap_Fbo_Settings.useStencil = true;
			//cap_Fbo_Settings.depthStencilAsTexture = true;
			//cap_Fbo_Settings.maxFilter
		}

		cap_Fbo.allocate(cap_Fbo_Settings);
		cap_Fbo.begin();
		ofClear(0, 255);
		//ofClear(0);
		cap_Fbo.end();

		blitFbo.allocate(cap_Fbo_Settings);
		blitFbo.begin();
		ofClear(0, 255);
		//ofClear(0);
		blitFbo.end();
	}

	//--------------------------------------------------------------
	void buildRecorder() {
		ofLogWarning(__FUNCTION__);

		// TODO: trying to allow resize.. 
		// but can not be called twice ?? bc stops working..
		// TODO: how to recreate the capturer ??
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
	void setCustomizeSection(ofRectangle r) {
		rectSection = r;
		isSectionCustomized = true;

		cap_w = rectSection.getWidth();
		cap_h = rectSection.getHeight();
		buildAllocateFbo();
	}

public:
	//--------------------------------------------------------------
	void begin() {// call before draw the scene to record
		if (bActive)
		{
			cap_Fbo.begin();
			ofClear(0, 255);
			//ofClear(0);
		}
	}

	//--------------------------------------------------------------
	void end() {// call after draw the scene to record
		if (bActive)
		{
			cap_Fbo.end();

			//-

			if (isMounted)
			{
				if (isRecording && ofGetFrameNum() > 0)
				{
					recorder.save(cap_Fbo.getTexture());
				}
			}
		}
	}

	//--------------------------------------------------------------
	void draw() {// must draw the scene content to show
		if (bActive)
		{
			// BUG: depth/antialias
			blitFbo.begin();
			{
				ofClear(0, 255);
				//ofClear(0);
				cap_Fbo.draw(0, 0, cap_w, cap_h);
			}
			blitFbo.end();
			blitFbo.draw(0, 0);

			// BUG: depth/antialias
			//cap_Fbo.draw(0, 0);// drawing is required outside fbo

			// blinking preview custom section rect borders
			if (isSectionCustomized && isMounted && !isRecording) {
				ofPushStyle();
				int _period = 15;
				bool b = ofGetFrameNum() % _period > _period / 2;
				ofSetColor(ofColor(ofColor::blue, (b ? 255 : 128)));
				ofNoFill();
				ofSetLineWidth(1.0f);
				ofDrawRectangle(rectSection);
				ofPopStyle();
			}
		}
	}

	//--------------------------------------------------------------
	void drawInfo(int x = 30, int y = 0) {// draw the gui info if desired

		if (bShowInfo && bActive) {

			info = "";

			// animated points..
			std::string sp = "";
			const int p = 30;// period in frames
			const int fn = ofGetFrameNum() % (p * 4);
			bool b0, b1, b2;
			b0 = (fn > p * 3);
			b1 = (fn > p * 2);
			b2 = (fn > p * 1);
			if (b0) sp += ".";
			if (b1) sp += ".";
			if (b2) sp += ".";

			//-

			// workaround: to usen when force stop encoding 
			// i don't know how to stop the process without breaking the thread restart...
			if (bError)
			{
				if (ofGetFrameNum() % 120 < 90)
				{
					info += "> ALERT! BROKEN FFmpeg THREAD !\n";
					info += "> MUST RESTART THE APP...\n";
				}
				else info += " \n\n";
			}
			else
			{
				//--

				if (!bShowMinimal && isRecording) {
					if (ofGetFrameNum() % 120 == 0)
					{
						infoFFmpeg = "\n";
						infoFFmpeg += "Texture copy       : " + ofToString(recorder.getAvgTimeTextureCopy()) + "\n";
						infoFFmpeg += "GPU download       : " + ofToString(recorder.getAvgTimeGpuDownload()) + "\n";
						infoFFmpeg += "Image encoding     : " + ofToString(recorder.getAvgTimeEncode()) + "\n";
						infoFFmpeg += "File save (avg ms) : " + ofToString(recorder.getAvgTimeSave()) + "\n";
						infoFFmpeg += "\n";
					}
				}

				if (bShowMinimal && isRecording)// reduced info when recording to imrpove performance a little
				{
					info += "RECORDING...\n";
					int _fps = ofGetFrameRate();
					info += "FPS " + ofToString(_fps) + ((_fps < 59) ? " !" : "") + "\n";
					info += "DURATION : " + calculateTime(getRecordedDuration()) + "\n";
					//info += infoFFmpeg;
					info += "F9 : STOP\n";
					// too much slow
					//info += "Disk Stills " + ofToString(amountStills) + "\n";
					//if (ofGetFrameNum() % 120 == 0) amountStills = dataDirectory.listDir();// refresh amount stills
					info += "M  : Minimal Info " + ofToString(bShowMinimal ? "ON" : "OFF") + "\n";

				}
				else
				{
					// 1. waiting mount: press F8
					if (!isMounted && !isThreadRunning() && !isRecording)
					{
						info += "> PRESS F8 TO MOUNT CAPTURER" + sp + "\n";
						info += "M  : Minimal Info " + ofToString(bShowMinimal ? "ON" : "OFF") + "\n";
					}

					// 2. mounted, recording or running ffmpeg script
					else if ((isMounted || isRecording || isEncoding))
					{
						if (!isEncoding)
						{
							info += "FPS " + ofToString(ofGetFrameRate(), 0) + "          " + ofToString(recorder.getFrame()) + " frames\n";
							info += "WINDOW          " + ofToString(ofGetWidth()) + "x" + ofToString(ofGetHeight()) + "\n";
							info += "RECORDER        " + ofToString(recorder.getWidth()) + "x" + ofToString(recorder.getHeight()) + "\n";
							info += "FBO SIZE        " + ofToString(cap_w) + "x" + ofToString(cap_h) + "\n";
						}
						if (isSectionCustomized)
						{
							info += "SECTION         " + ofToString(rectSection.getX(), 0) + "," + ofToString(rectSection.getY(), 0);
							info += " " + ofToString(rectSection.getWidth(), 0) + "x" + ofToString(rectSection.getHeight(), 0) + "\n";
						}
						info += "Disk Stills     " + ofToString(amountStills) + "\n";
						//info += "\n";

						if (isRecording)
						{
							info += "F9  : STOP\n";
							info += "RECORD DURATION: " + calculateTime(getRecordedDuration()) + "\n";

							info += infoFFmpeg;

							// error
							if (isRecording) {
								if (recorder.getAvgTimeSave() == 0) {
									std::string ss;
									const int p = 30;// blink period in frames
									int fn = ofGetFrameNum() % p;
									if (fn < p / 2) ss = "ERROR RECORDING!";
									else ss = "";
									info += ss + "\n";
								}
							}
						}
						else if (isMounted || isEncoding)// mounted or running ffmpeg script
						{
							if ((!bFfmpegLocated) && ofGetFrameNum() % 60 < 20) info += "> ALERT! Missing FFmpeg.exe...";
							info += "\n";

							if (isThreadRunning()) {

								if (isEncoding)
								{
									info += "> ENCODING VIDEO" + sp + " [" + ofToString(bUseFFmpegGPU ? "GPU" : "CPU") + "]\n";
								}
								else if (isPlayingPLayer) info += "> PLAYING VIDEO\n";
							}
							else {
								info += "  MOUNTED! READY" + sp + "\n";
								if (b1 || b2) {
									info += "> PRESS F9  TO START CAPTURER\n";
									info += "> PRESS F11 TO ENCODE VIDEO\n";
								}
								else {
									info += " \n \n";
								}
								info += "> PRESS F8  TO UNMOUNT\n";
							}
						}
						//else
						//{
						//	info += "\n\n";
						//}
					}
				}

				//-

				if (!bShowMinimal && (!isRecording || !isEncoding))
				{
					info += infoHelpKeys;
					//info += "\n";
					//info += "M  : Minimal Info " + ofToString(bShowMinimal ? "ON" : "OFF") + "\n";
				}
			}

			//-

			// draw text info
			float h = ofxSurfingHelpers2::getHeightBBtextBoxed(font, info);
			y = ofGetHeight() - h - x + 8;// bad offset
			ofxSurfingHelpers2::drawTextBoxed(font, info, x, y);

			//-

			// red blink circle
			if ((!bShowMinimal) && (isMounted || !bError))
			{
				float radius = 15;
				int yy = y + radius;
				int xx = x + 2 * radius + 200;

				if (!isThreadRunning()) {// only while not encoding video
					ofPushStyle();
					ofColor c1{ ofColor(0,128) };
					ofColor c2{ ofColor(ofColor::red,200) };
					ofSetLineWidth(1.f);
					if (isRecording)
					{
						ofFill();
						ofSetColor(c2);
						ofDrawCircle(ofPoint(xx, yy), radius);
						ofNoFill();
						ofSetColor(c1);
						ofDrawCircle(ofPoint(xx, yy), radius);
					}
					else if (isMounted)
					{
						if (ofGetFrameNum() % 60 < 20) {
							ofFill();
							ofSetColor(c2);
							ofDrawCircle(ofPoint(xx, yy), radius);
						}
						ofNoFill();
						ofSetColor(c1);
						ofDrawCircle(ofPoint(xx, yy), radius);
					}
					ofPopStyle();
				}
			}
		}
	}

public:
	//--------------------------------------------------------------
	void setFFmpegScript(std::string sscript) {// if this is setted, the other hardcoded CPU/GPU aren't used at all
		bFfmpegCustomScript = true;
		ffmpegScript = sscript;
	}

public:
	//--------------------------------------------------------------
	void setDephEnabled(bool b) {
		bDepth3D = b;
	}

public:
	//--------------------------------------------------------------
	void setOverwriteVideoOut(bool b) {// default is true. set to false to allow add timestamp to filenames
		bOverwriteOutVideo = b;
	}

private:
	bool bUseFFmpegGPU = true;

public:
	//--------------------------------------------------------------
	void setFfpmegGpu(bool b) {
		bUseFFmpegGPU = b;
	}
public:
	//--------------------------------------------------------------
	void setPathRoot(std::string path) {
		pathRoot = path;
	}

public:
	//--------------------------------------------------------------
	void doRunFFmpegCommand() {
		ofLogWarning(__FUNCTION__) << " to: " << _pathFolderStills;

		isEncoding = true;
		isPlayingPLayer = false;

		// we are running the systems commands
		// in a sperate thread so that it does
		// not block the drawing
		startThread();
	}

	//--------------------------------------------------------------
	void setActive(bool b) {
		bActive = b;
		bShowInfo = bActive;
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

public:
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

				// toggle show minimal
			case 'M':
			{
				setToggleShowMinimal();
				ofLogNotice(__FUNCTION__) << "bShowMinimal: " << (bShowMinimal ? "ON" : "OFF");
			}
			break;

			// set Full HD
			case OF_KEY_F5:
			{
				ofSetWindowShape(1920, 1080);
				windowResized(1920, 1080);
			}
			break;

			// TODO:
			// allow resize on runtime
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
				isMounted = !isMounted;
				ofLogWarning(__FUNCTION__) << "Mount: " << (isMounted ? "ON" : "OFF");
			}
			break;

			// start recording
			case OF_KEY_F9:
			{
				if (isRecording)// do stop
				{
					ofLogWarning(__FUNCTION__) << "Stop Recording";

					//isMounted = false;
					isRecording = false;
					amountStills = dataDirectory.listDir();
				}
				else if (isMounted)// do start
				{
					isRecording = true;
					timeStart = ofGetElapsedTimeMillis();
					ofLogWarning(__FUNCTION__) << "Start Recording into: " << _pathFolderStills;
				}
				else ofLogError(__FUNCTION__) << "Must Mount before Start Capture!";
			}
			break;

			// take screenshot
			case OF_KEY_F10:
			{
				std::string _fileName = "snapshot_" + ofGetTimestampString() + ".png";
				std::string _pathFilename = ofToDataPath(_pathFolderSnapshots + _fileName, true);//bin/data

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
				if (!isThreadRunning() && !isRecording && isMounted)
				{
					doRunFFmpegCommand();
				}
				else
				{
					ofLogWarning(__FUNCTION__) << "Trying to force skip FFmpeg batch encoding: can't be recording or already encoding";

					// TODO: BUG:
					// when called stop, must restart the app...
					if (isThreadRunning()) stopThread();

					// TODO:
					// force stop the thread
					//waitForThread(true);
					isEncoding = false;
					isPlayingPLayer = false;
					cout << "> FORCE STOP ENCODING PROCESS !" << endl;

					// create the command
					//std::ostringstream someCmd;
					std::stringstream someCmd;

					someCmd.clear();
#ifdef TARGET_WIN32
					someCmd << "taskkill /F /IM ffmpeg.exe";
					cout << someCmd;
					cout << ofSystem(someCmd.str().c_str()) << endl;
#endif
#ifdef TARGET_OSX
					someCmd << "say Hello";
#endif
					cout << "> DONE !" << endl;
				}

				if (amountStills == 0) {
					ofLogError(__FUNCTION__) << "Missing stills files into " << _pathFolderStills << " !";
				}
			}
			break;

			// remove all captures stills
			case OF_KEY_BACKSPACE: // ctrl + alt + backspace
			{
				if (!mod_COMMAND && !mod_SHIFT && mod_ALT && mod_CONTROL)
				{
					//std::string _path = _pathFolderStills;
					//dataDirectory.open(ofToDataPath(_path, true));
					dataDirectory.remove(true);
					ofxSurfingHelpers2::CheckFolder(_pathFolderStills);

					amountStills = dataDirectory.listDir();
				}
			}
			break;
			}
		}
	}

	//--------------------------------------------------------------
	void windowResized(int w, int h) {// must be called to resize the fbo and video resolution
		if (!isSectionCustomized && bActive) // we don't want to resize the canvas when  custom section is enabled
		{
			cap_w = w;
			cap_h = h;
			buildAllocateFbo();
		}

		// TODO: trying to allow resize..
		// this brakes the capturer...
		//buildRecorder();
	}

private:
	//--------------------------------------------------------------
	void buildHepKeysInfo() {// must be called after bitrate, framerate and size w/h are setted

		// build help info
		infoHelpKeys = "\n";
		infoHelpKeys += "HELP KEYS"; infoHelpKeys += "\n";
		infoHelpKeys += "h   : Show Help info"; infoHelpKeys += "\n";
		infoHelpKeys += "M   : Minimal Info " + ofToString(bShowMinimal ? "ON" : "OFF") + "\n";
		infoHelpKeys += "F5  : Set FullHD size"; infoHelpKeys += "\n";
		infoHelpKeys += "F6  : Set optimal Instagram size"; infoHelpKeys += "\n";
		infoHelpKeys += "F7  : Refresh Window size"; infoHelpKeys += "\n";
		if (!bShowMinimal) {
			infoHelpKeys += "F8  : Mount Recorder"; infoHelpKeys += "\n";
			infoHelpKeys += "F9  : Start/Stop Recording"; infoHelpKeys += "\n";
		}
		infoHelpKeys += "F10 : Capture Screenshot"; infoHelpKeys += "\n";
		infoHelpKeys += "F11 : Run FFmpeg video Encoder"; infoHelpKeys += "\n";
		infoHelpKeys += "M   : Minimal Info\n";

		infoHelpKeys += "Ctrl + Alt + BackSpace: Clear Stills";// info += "\n";
		if (!bShowMinimal) {
			info += "path Stills     : " + _pathFolderStills; info += "\n";
			info += "path Screenshots: " + _pathFolderSnapshots; info += "\n";
		}
		infoFFmpeg = "\n";
		infoFFmpeg += "Texture copy       : " + ofToString(recorder.getAvgTimeTextureCopy()) + "\n";
		infoFFmpeg += "GPU download       : " + ofToString(recorder.getAvgTimeGpuDownload()) + "\n";
		infoFFmpeg += "Image encoding     : " + ofToString(recorder.getAvgTimeEncode()) + "\n";
		infoFFmpeg += "File save (avg ms) : " + ofToString(recorder.getAvgTimeSave()) + "\n";
		infoFFmpeg += "\n";
	}

	// call ffmpeg command
	// join all stills to a video file
	//--------------------------------------------------------------
	void threadedFunction() {
		if (bActive) {
			cout << "--------------------------------------------------------------" << endl;
			ofLogWarning(__FUNCTION__) << endl;
			cout << (__FUNCTION__) << endl;

			std::string warninglog = "";
			warninglog += "> WARNING! ofApp must run as Administrator !\n";
			warninglog += "> WARNING! ffmpeg binary must be located on: " + pathRoot + _nameBinary + " !\n";

			cout << endl << warninglog << endl;

			if (isThreadRunning())
			{
				// build ffmpeg command

				cout << "> Starting join all stills (xxxxx.tif) to a video file (.mp4)...";

				pathAppData << pathRoot;

				ffmpeg << pathAppData.str().c_str() << _nameBinary;

				// input files
				pathDest << pathAppData.str().c_str() << _pathFolderCaptures;
				filesSrc << pathAppData.str().c_str() << _pathFolderStills << "%05d.tif"; // data/stills/%05d.tif

				// output video file
				if (bOverwriteOutVideo) nameDest << "output.mp4"; // "output.mp4";
				else nameDest << "output_" << ofGetTimestampString() << ".mp4"; // "output_2020-10-11-19-08-01-417.mp4";// timestamped

				// macOS error here
				fileOut << pathDest << nameDest;
				//                fileOut << pathDest.str().c_str() << nameDest;
				//                pathDest = pathDest + nameDest;
				//                fileOut = fileOut + pathDest;

								//-

								// used template to join stills:

								// https://trac.ffmpeg.org/wiki/Encode/H.264
								// Constant Rate Factor: CRF scale is 0�51, where 0 is lossless, 23 is the default, and 51 is worst quality possible. 
								// A lower value generally leads to higher quality, and a subjectively sane range is 17�28. 
								// Consider 17 or 18 to be visually lossless or nearly so; it should look the same or nearly the same as the input but it isn't technically lossless. 
								// https://bytescout.com/blog/2016/12/ffmpeg-command-lines-convert-various-video-formats.html

								// Template is selectable by API

								//-

								// customized script by user:
				if (bFfmpegCustomScript)
				{
					// 1. append exe + source files
					//// macOS error here
					cmd << ffmpeg << " -y -f image2 -i " << filesSrc.str().c_str() << " ";

					// 2. apend script
					cmd << ffmpegScript.c_str() << " ";

					// 3. append file output
					cmd << fileOut.str().c_str();

					//-

					cout << endl << endl;
					cout << "> CUSTOM FFmpeg SCRIPT" << endl << endl;
					cout << "> " + _nameBinary + " : " << endl << ffmpeg.str().c_str();
					cout << endl << endl;
					cout << "> Source: " << endl << filesSrc.str().c_str();
					cout << endl << endl;
					cout << "> FFmpeg CustomScript : " << endl << ffmpegScript.c_str();
					cout << endl << endl;
					cout << "> Out : " << endl << fileOut.str().c_str();
					cout << endl << endl;
					cout << "> Raw Command: " << endl << cmd.str().c_str();
					cout << endl << endl;
				}
				else // hardcoded scripts
				{
					// template A: CPU
					// this  intended to be a lossless preset
					// ffmpeg -r 60 -f image2 -s 1920x1080 -i %05d.tif -c:v libx264 -preset veryslow -qp 0 output.mp4 // lossless

					//-

					// template B: GPU
					// https://developer.nvidia.com/blog/nvidia-ffmpeg-transcoding-guide/
					// Command Line for Latency-Tolerant High-Quality Transcoding:
					// "ffmpeg -y -vsync 0 -hwaccel cuda -hwaccel_output_format cuda 
					// -i input.mp4 -c:a copy  
					// -c:v h264_nvenc -preset slow -profile high -b:v 5M 
					// -bufsize 5M -maxrate 10M -qmin 0 -g 250 -bf 3 -b_ref_mode middle -temporal-aq 1 -rc-lookahead 20 -i_qfactor 0.75 -b_qfactor 1.1 
					// output.mp4"
					// -hwaccel cuda > Unrecognized hwaccel : cuda. Supported hwaccels : dxva2 qsv cuvid

					// https://forums.guru3d.com/threads/how-to-encode-video-with-ffmpeg-using-nvenc.411509/
					// If you want lossless encoding use preset = lossless.
					// cq = number controls quality, lower number means better quality. 
					// - rc constqp enables constant quality rate mode which in my opinion is really, really handy and I always use it over fixed bitrate modes.
					// It's really great to see than NVENC supports this mode and on top of that it even supports lossless encoding and yuv444p format. 
					// On top of that NVENC's constant quality rate mode works surprisingly well, quality wise.
					// You can also play with - temporal - aq 1 switch (works for AVC) and -spatial_aq 1 switch (works for HEVC).
					// Add them after - preset % preset%.For AVC you can enable b frames with - b switch.NVIDIA recommended using three b - frames(-b) in one of their pdfs for optimal quality(switch: -b 3).
					// if your source material is lossless RGB and you want the absolutely best quality, use preset=lossless and uncomment SET videofilter=-pix_fmt yuv444p

					//-

					// build the ffmpeg command:

					// 1. prepare source and basic settings: auto overwrite file, fps, size, stills source
				//// macOS error here
					cmd << ffmpeg << " -y -f image2 -i " << filesSrc.str().c_str() << " ";
					cmd << "-r 60 ";// framerate

					// we can resize too or mantain the original window size
					//cmd << "-s hd1080 ";
					//cmd << "-s 1920x1080 ";

					// 2. append encoding settings
					// template 1: (CPU)
					if (!bUseFFmpegGPU) cmdEncodingArgs << "-c:v libx264 -preset veryslow -qp 0 ";

#ifdef TARGET_WIN32
					// template 2: (Nvidia GPU)
					else if (bUseFFmpegGPU)
					{
						cmdEncodingArgs << "-c:v h264_nvenc ";// enables GPU hardware accellerated Nvidia encoding. Could check similar arg to AMD..
						cmdEncodingArgs << "-b:v 25M "; // constant bitrate 25000
						cmdEncodingArgs << "-crf 20 ";
						//cmdEncodingArgs << "-vsync 0 ";
						//cmdEncodingArgs << "-hwaccel cuvid ";
						//cmdEncodingArgs << "-qp 0 ";
						cmdEncodingArgs << "-preset slow ";	// 10secs = 30MB
						//cmdEncodingArgs << "-preset lossless ";	// 10secs = 150MB
						//cmdEncodingArgs << "-profile high ";
						//cmdEncodingArgs << "-pix_fmt yuv444p ";// 10secs = 300MB. doubles size! raw format but too heavy weight!
					}
#endif
#ifdef TARGET_OSX
					// template 2: (macOS GPU)
					else if (bUseFFmpegGPU)
					{
						// TODO:
						// must search a script for macOS, because in general they have AMD Radeon GPU, not Nvidia
						//// CPU
						//cmdEncodingArgs << "-c:v libx264 -preset veryslow -qp 0 ";

						// GPU Nvidia
						cmdEncodingArgs << "-c:v h264_nvenc ";// enables GPU hardware accellerated Nvidia encoding. Could check similar arg to AMD..
						cmdEncodingArgs << "-b:v 25M "; // constant bitrate 25000
						cmdEncodingArgs << "-crf 20 ";
						//cmdEncodingArgs << "-vsync 0 ";
						//cmdEncodingArgs << "-hwaccel cuvid ";
						//cmdEncodingArgs << "-qp 0 ";
						cmdEncodingArgs << "-preset slow ";	// 10secs = 30MB
						//cmdEncodingArgs << "-preset lossless ";	// 10secs = 150MB
						//cmdEncodingArgs << "-profile high ";
						//cmdEncodingArgs << "-pix_fmt yuv444p ";// 10secs = 300MB. doubles size! raw format but too heavy weight!
					}
#endif
					// append
					//// macOS error here
					cmd << cmdEncodingArgs;

					// 3. append file output
					cmd << fileOut.str().c_str();

					//-

					cout << endl << endl;
					cout << "> HARDCODED FFmpeg SCRIPT" << endl << endl;
					cout << "> " + _nameBinary + " : " << endl << ffmpeg.str().c_str();
					cout << endl << endl;
					cout << "> Source : " << endl << filesSrc.str().c_str();
					cout << endl << endl;
					cout << "> Out : " << endl << fileOut.str().c_str();
					cout << endl << endl;
					cout << "> Raw Command: " << endl << cmd.str().c_str();
					cout << endl << endl;
					cout << "> Quality Encoding arguments: " << endl << cmdEncodingArgs.str().c_str();
					cout << endl << endl;
				}

				//-

				// 4. run video encoding

				std::string slog;
				slog = ofSystem(cmd.str().c_str());
				cout << endl << "> Log: " << endl << slog << endl;

				//-

				cout << endl;
				cout << "> DONE/TRYING VIDEO ENCODING INTO: " << endl << fileOut.str().c_str();
				cout << endl << endl;
				cout << "> WARNING: CLOSE YOUR VIDEOPLAYER TO UNBLOCK ALLOW CLOSE THE APP !";
				cout << endl << endl;
				cout << "--------------------------------------------------------------" << endl << endl;

				// TODO: not sure if can collide bc threading and if it's correct (mutex?)
				isEncoding = false;
				isPlayingPLayer = true;

				cout << "> WARNING: If output video is not opened NOW, and getting above an error like:" << endl;
				cout << "\t'is not recognized as an internal or external command, operable program or batch file.'" << endl;
				cout << "> Then probably you need to set your ofApp.exe settings to run as Administrator:" << endl;
				cout << "> Use Windows File Explorer file properties / Change settings for all users / compatibility." << endl;
				cout << "> This is to allow run ffmpeg.exe and access to files from here!" << endl;

				//-

				// 5. open video file with your system player
				slog = ofSystem(fileOut.str().c_str());
				cout << slog << endl;

				//-

				// 6. log 
				// TODO: should check log errors...
				// error seems to print this:
				//" is not recognized as an internal or external command,
				//operable program or batch file."

				//--

				// some system examples

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

				// TODO:
				// stop the thread on exit
				waitForThread(true);
				isEncoding = false;
				isPlayingPLayer = false;
				cout << "> VIDEOPLAYER CLOSED !" << endl;
				cout << "> ENCODING PROCESS / THREAD FINISHED !" << endl;
				bError = true;// workaround. i don't know how to stop the process without breaking the thread restart...
			}
		}
	}

	//-

private:
	// original code copied from: ofxFilikaUtils.h
#define SECS_PER_MIN 60
#define SECS_PER_HOUR 3600
	//--------------------------------------------------------------
	std::string calculateTime(float _time) {

		int seconds;
		int minutes;
		int mins_left;
		int secs_left;

		seconds = (/*gameTimeSeconds - */int(_time));
		minutes = (/*gameTimeSeconds - */int(_time)) / SECS_PER_MIN;
		mins_left = minutes % SECS_PER_MIN;
		secs_left = seconds % SECS_PER_MIN;

		std::string mins;
		std::string secs;

		if (mins_left < 10) {
			mins = "0" + ofToString(mins_left);
		}
		else {
			mins = ofToString(mins_left);
		}

		if (secs_left < 10) {
			secs = "0" + ofToString(secs_left);
		}
		else {
			secs = ofToString(secs_left);
		}

		//cout << ofGetElapsedTimeMillis() / 1000 << endl;
		//cout << "remaining time : " << mins_left << " : " <<  secs_left << endl;
		//cout << "remaining time : " << mins << " : " <<  secs << endl;

		if (mins_left < 0 || secs_left < 0)
			return "00:00";
		else
			return (mins + ":" + secs);
	}
	};
