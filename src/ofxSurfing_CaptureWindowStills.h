#pragma once

#include "ofMain.h"

//	TODO: 
// + add custom section to gui too
// + check/allow change window capture size without breaking the capturer. not it's posible before call setup()
// + allow gif exporter: https://github.com/pierrep/ofxGifImage
// + allow instagram-ready export.
// + draw rectangle draggable border, to select a section manually
// * customizable frame rate. now hardcoded to 60 fps
//
// FFmpef NOTES:
// nice settings handling and binaries: we can copy things from here
// https://github.com/tyhenry/ofxFFmpeg/blob/master/src/ofxFFmpeg.h
// https://www.reddit.com/r/ffmpeg/comments/8t54bm/converting_yuvj420p_to_yuv420p_issues_with_black/


#include "ofxTextureRecorder.h"
#include "ofxSurfingHelpers2.h"
#include "ofxGui.h"

#define MODE_DEFAULT_THREADS	// uncomment to allow default cores. recommended. makes ignore below MODE_LESS_THREADS 
#define MODE_LESS_THREADS 4	// TEST: reduce threads to improve performance on my "tiny" Intel i5 6500
//#define MODE_LESS_THREADS 12	// default from example

//#define MODE_MORE_MINIMAL // disable some display text

#define ANTIALIAS_NUM_SAMPLES 16 // only used on depth mode

#define TEST_CONSOLE_KILL_PROCESS
// NOTE:
// sometimes it hangs after encoding... trying to avoid
// in general, after finish an encoding, your video player will be opened with your video. in my case VLC
// then it's recommended to close the videoplayer before start a new capture.
// in some way, to release the thread or something realated (?)

// platforms
#ifdef TARGET_OSX// tested without AMD GPU
#endif
#ifdef TARGET_WIN32
#endif
//#ifdef TARGET_LINUX// not tested. I don't have linux sorry.
//#endif

class CaptureWindow : public ofBaseApp, public ofThread
{

public:
	// public to integrate into your ofApp gui
	ofParameterGroup params{ "Capturer" };
	ofParameter<bool> bEnable{ "Enable", false };
	ofParameter<bool> bRecord{ "Record", false };
private:
	ofParameterGroup params_extra{ "Extra" };
	ofParameter<bool> bShowInfo{ "Show info", false };
	ofParameter<bool> bShowMinimal{ "Minimal", true };
	ofParameter<bool> bRefresh{ "Refresh Section", false };
	ofParameter<bool> bAutoencode{ "Autoencode", false };
	
	ofParameterGroup params_CustomSection{ "Custom Section" };
	ofParameter<bool> isSectionCustomized{ "Customized", false };
	//ofParameter<ofRectangle> rectSection;
	ofParameter<glm::vec2> rectSectionPos{ "Pos", glm::vec2(0),glm::vec2(0), glm::vec2(1920, 1200) };
	ofParameter<glm::vec2> rectSectionShape{ "shape", glm::vec2(1920,1080),glm::vec2(0), glm::vec2(1920, 1200) };

	//bool isSectionCustomized = false;

	ofParameter<std::string> timer_str{ "Time","00:00" };

	std::string path_Settings = "WindowCapturer.xml";

	bool bDisableCallbacks = false;

	//--------------------------------------------------------------
	void Changed_params(ofAbstractParameter &e) {
		if (!bDisableCallbacks)
		{
			std::string name = e.getName();
			if (name != timer_str.getName())
				ofLogNotice(__FUNCTION__) << name << " : " << e;

			if (name == bShowMinimal.getName())
			{
				buildHepKeysInfo();
			}

			else if (name == bRecord.getName())
			{
				if (bEnable.get())
				{
					if (bRecord.get())
					{
						// do start
						if (!bIsMounted) bIsMounted = true;// force mount
						startCapturer();
					}
					else
					{
						// do stop
						stopCapturer();
					}
				}
				else {// lock off
					bRecord = false;
				}
			}

			else if (name == bEnable.getName())
			{
				if (bEnable) bIsMounted = true;
				else bIsMounted = false;
			}

			// TODO:
			// custom section
			else if (name == bRefresh.getName() && bRefresh)
			{
				bRefresh = false;

				if (isSectionCustomized) {
					cap_w = rectSection.getWidth();
					cap_h = rectSection.getHeight();
				}
				else {
					cap_w = ofGetWidth();
					cap_h = ofGetHeight();
				}
				buildAllocateFbo();
				//buildRecorder();
			}
			else if (name == rectSectionPos.getName() || name == rectSectionShape.getName() )
			{
				rectSection.set(rectSectionPos.get().x, rectSectionPos.get().y, rectSectionShape.get().x, rectSectionShape.get().y);
			}
		}
	}

private:
	ofxTextureRecorder recorder;

private:
	ofFbo cap_Fbo;
	ofFbo::Settings cap_Fbo_Settings;
	int cap_w, cap_h;

	ofImageFormat stillFormat;

private:
	ofDirectory dataDirectory;
	int amountStills = 0;

public:
	bool isRecording() {
		return bIsRecording;
	}
	bool isMounted() {
		return bIsMounted;
	}
	void setMounted(bool b) {
		bIsMounted = b;
	}
	bool isEnabled() {
		return bEnable.get();
	}
	//bool isVisible() {
	//	return bShowInfo.get();
	//}
	//void setVisible(bool b) {
	//	bShowInfo = b;
	//}

private:
	bool bIsMounted = false;
	bool bIsRecording;
	//bool bShowInfo = true;
	bool bError = false;

private:
	uint32_t timeStart;
	std::string infoHelpKeys;
	std::string infoFFmpeg;
	std::string info;

public:
	//--------------------------------------------------------------
	void setShowMinimal(bool b) {
		bShowMinimal = b;
	};
	//--------------------------------------------------------------
	void setToggleShowMinimal() {
		bShowMinimal = !bShowMinimal;
	};

	bool isEncoding = false;
	bool isPlayingPLayer = false;

	ofRectangle rectSection;// customize capture section. TODO: hardcoded to 0,0 position yet!

private:
	ofTrueTypeFont font;

private:
	bool bFfmpegLocated = false;
	bool bFfmpegCustomScript = false;

private:
	//bool bDepth3D = false;// disabled by default. must enable before call setup()
	bool bDepth3D = true;// enabled by default. must disable before call setup()

	// BUG solved: when using antialias/depth we get "black screen"
	ofFbo blitFbo;// so we need this aux fbo to solve the bug

private:
	bool bOverwriteOutVideo = true;// we only want the last video. we use same name for all re takes and overwrite.

private:
	std::string filesSrc;
	std::string fileOutPath;
	std::string fileOutName;
	std::string nameDest;
	std::string cmd;
	std::string cmdEncodingArgs;
	std::string ffmpeg;
	std::string ffmpegScript;
	std::string ffmpegNameBinary;
	std::string pathRoot; // default root path is /bin/data/
	std::string pathAppData;
	std::string pathDest;
	std::string pathFolderCaptures;
	std::string pathFolderStills;
	std::string pathFolderSnapshots;

	// platforms
#ifdef TARGET_OSX
	std::string _slash_ = "/";
#endif
#ifdef TARGET_WIN32
	std::string _slash_ = "\\";
#endif

	// TODO:
	int _fpsTar;
	int _fps;
	int _fpsThsh;
	int _fpsDiff;
	bool _bAlert;
	int lostFrames;

	//----

public:
	// helpers to minimize gui group folders when added to a external ofxPanel
	//--------------------------------------------------------------
	void refreshGui(ofxGuiGroup &_group)
	{
		auto &g1 = _group.getGroup(params.getName());
		auto &g2 = g1.getGroup(params_extra.getName());
		g2.minimize();
		auto &g3 = g2.getGroup(params_CustomSection.getName());
		g3.minimize();
	}
	//--------------------------------------------------------------
	void refreshGui(ofxPanel & _gui)
	{
		auto &g1 = _gui.getGroup(params.getName());
		auto &g2 = g1.getGroup(params_extra.getName());
		g2.minimize();
		auto &g3 = g2.getGroup(params_CustomSection.getName());
		g3.minimize();
	}

	//--------------------------------------------------------------
	CaptureWindow()
	{
		cap_w = 1920;
		cap_h = 1080;

		// NOTE:
		//the best way to handle paths to avoid this problems is to use 
		//std::filesystem::path which when compared doens�t do a strict string comparison 
		//and will work for / or \ or double // or even comparing absolute and relative paths if they exist. 
		//you just need to wrap the string for the path like:
		//std::filesystem::path("some/path") == std::filesystem::path("some//path")
		//which should return true
		pathFolderCaptures = "Captures" + _slash_;
		pathFolderStills = pathFolderCaptures + "Stills" + _slash_;
		pathFolderSnapshots = pathFolderCaptures + "Snapshots" + _slash_;
		setPathRoot(ofToDataPath(_slash_, true));

		//std::string _font = "assets/fonts/overpass-mono-bold.otf";
		std::string _font = "assets/fonts/Hack-Bold.ttf";
		bool b = font.load(_font, 8);
		if (!b) font.load(OF_TTF_SERIF, 8);// solve font-file-not-found with OF bundled alternative font

		fileOutName = "output";// default filename will be "output.mp4" or with timestamps if enabled

		params.add(timer_str);
		params.add(bEnable);
		params.add(bRecord);

		params_extra.add(bShowInfo);
		params_extra.add(bShowMinimal);
		params_extra.add(bAutoencode);
		params_CustomSection.add(isSectionCustomized);
		params_CustomSection.add(bRefresh);
		params_CustomSection.add(rectSectionPos);
		params_CustomSection.add(rectSectionShape);
		//params_CustomSection.add(rectSection);
		params_extra.add(params_CustomSection);

		params.add(params_extra);

		timer_str.setSerializable(false);
		bRefresh.setSerializable(false);
		bRecord.setSerializable(false);

		ofAddListener(params.parameterChangedE(), this, &CaptureWindow::Changed_params);

		//rectSection.setName("Section");
		rectSection.set(rectSectionPos.get().x, rectSectionPos.get().y, rectSectionShape.get().x, rectSectionShape.get().y);

		// TODO:
		_fpsTar = 60;
		_fps = (int)ofGetFrameRate();
		_fpsThsh = 2;
		lostFrames = 0;

		ofxSurfingHelpers2::loadGroup(params, path_Settings);
	};

	//--------------------------------------------------------------
	~CaptureWindow() {
		// stop the thread on exit
		waitForThread(true);
		ofxSurfingHelpers2::saveGroup(params, path_Settings);
		ofRemoveListener(params.parameterChangedE(), this, &CaptureWindow::Changed_params);
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
			ofLogNotice(__FUNCTION__) << "path: " << path << " ofImageFormat: " << format;

			// we can select a still format passing one ofImageFormat like this ones:
			// OF_IMAGE_FORMAT_BMP = 0,
			// OF_IMAGE_FORMAT_JPEG = 2,
			// OF_IMAGE_FORMAT_PNG = 13,
			// OF_IMAGE_FORMAT_TIFF = 18,
			// OF_IMAGE_FORMAT_RAW = 34

			pathFolderCaptures = path; // "Captures\\"
			pathFolderStills = pathFolderCaptures + "Stills" + _slash_;
			pathFolderSnapshots = pathFolderCaptures + "Snapshots" + _slash_;

			ofxSurfingHelpers2::CheckFolder(pathFolderCaptures);
			ofxSurfingHelpers2::CheckFolder(pathFolderStills);
			ofxSurfingHelpers2::CheckFolder(pathFolderSnapshots);

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
			ffmpegNameBinary = "ffmpeg.exe";
#endif
#ifdef TARGET_OSX
			ffmpegNameBinary = "ffmpeg";
#endif
            
#ifdef TARGET_WIN32
			_pathFfmpeg = pathRoot + ffmpegNameBinary;
            bFfmpegLocated = file.doesFileExist(_pathFfmpeg, true);//win
#endif
            
            #ifdef TARGET_OSX
            _pathFfmpeg = ffmpegNameBinary;
            bFfmpegLocated = ofFile::doesFileExist(ofToDataPath( _pathFfmpeg, true ), false);//macOS
            #endif
            
            if (bFfmpegLocated) ofLogWarning(__FUNCTION__) << "Located: " + ffmpegNameBinary + " into " << _pathFfmpeg;
			else ofLogError(__FUNCTION__) << "Missing required binary file " + ffmpegNameBinary + " into " << _pathFfmpeg << " !";

			// stills folder
			// let the folder open to list amount files sometimes...
			dataDirectory.open(ofToDataPath(pathFolderStills, true));
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
		//// TODO:
		//cap_Fbo_Settings.numSamples = (int)ANTIALIAS_NUM_SAMPLES;// antialias 

		cap_Fbo.allocate(cap_Fbo_Settings);
		cap_Fbo.begin();
		ofClear(0, 255);
		cap_Fbo.end();

		// TODO:
		// BUG:
		if (bDepth3D)
		{
			blitFbo.allocate(cap_Fbo_Settings);
			blitFbo.begin();
			ofClear(0, 255);
			blitFbo.end();
		}
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

#ifndef MODE_DEFAULT_THREADS 
		settings.numThreads = MODE_LESS_THREADS;
#endif
		//// TODO:
		//settings.maxMemoryUsage = 9000000000;

		// NOTE: about my ofxTextureRecorder fork
		// this setPath is not required on my customized fork of ofxTextureRecorder
		// bc it's already implemented using settings
		//settings.folderPath = "path";

		recorder.setPath(pathFolderStills);
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
		
		rectSectionPos = glm::vec2( rectSection.getX(), rectSection.getY());
		rectSectionShape = glm::vec2( rectSection.getWidth(), rectSection.getHeight());

		cap_w = rectSection.getWidth();
		cap_h = rectSection.getHeight();
		
		buildAllocateFbo();
	}

public:
	//--------------------------------------------------------------
	void begin() {// call before draw the scene to record
		//if (bEnable)
		{
			// updates
			if (bIsRecording)
				timer_str = ofxSurfingHelpers2::calculateTime(getRecordedDuration());

			//-

			cap_Fbo.begin();
			ofClear(0, 255);
		}
	}

	//--------------------------------------------------------------
	void end() {// call after draw the scene to record
		//if (bEnable)
		{
			cap_Fbo.end();

			//-

			if (bEnable && bIsMounted)
			{
				if (bIsRecording && ofGetFrameNum() > 0)
				{
					recorder.save(cap_Fbo.getTexture());
				}
			}
		}
	}

	//--------------------------------------------------------------
	void draw() {// must draw the scene content to show
		if (bEnable)
		{
			// BUG: depth/antialias
			if (bDepth3D)
			{
				blitFbo.begin();
				{
					ofClear(0, 255);
					//ofClear(0);
					cap_Fbo.draw(0, 0, cap_w, cap_h);
				}
				blitFbo.end();
				blitFbo.draw(0, 0);
			}
			else
			{
				cap_Fbo.draw(0, 0, cap_w, cap_h);
			}
		}
		else
		{
			// BUG: depth/antialias
			cap_Fbo.draw(0, 0);// drawing is required outside fbo
		}
	}

	//--------------------------------------------------------------
	void drawInfo(int x = 30, int yOffest = 0) {// draw the gui info if desired. offset from bottom border

		if (isRecording())
		{
			// log lost frames amount on last recording
			_fps = (int)ofGetFrameRate();
			_fpsThsh = 3;
			_fpsDiff = _fpsTar - _fps;
			_bAlert = (_fpsDiff > _fpsThsh);
			if (_bAlert) {
				lostFrames += _fpsDiff;
			}
		}

		//-

		if (bShowInfo)
		{
			info = "";

			// animated points..
			std::string sp = "";
			const int p = 30;// period in frames
			const int fn = ofGetFrameNum() % (p * 4);
			bool b0, b1, b2;
			b0 = (fn > p * 3);
			b1 = (fn > p * 2);
			b2 = (fn > p * 1);
			if (b0) sp += "...";
			else if (b1) sp += ".. ";
			else if (b2) sp += ".  ";

			//-

			// workaround: to usen when force stop encoding 
			// i don't know how to stop the process without breaking the thread restart...
			if (bError)
			{
				//info += "> ALERT! BROKEN FFmpeg THREAD !\n";
				//info += "> MUST RESTART THE APP" + sp + "\n";

				// blink
				if (ofGetFrameNum() % 120 < 90)
				{
					info += "> ALERT! BROKEN FFmpeg THREAD !\n";
					info += "> MUST RESTART THE APP...\n";
				}
				else {
					info += "                                \n";
					info += "                                \n";
				}
			}
			else
			{
				//--

				if (!bShowMinimal && bIsRecording) {
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

				if (bShowMinimal && bIsRecording)// reduced info when recording to imrpove performance a little
				{
					info += "RECORDING" + sp + "\n";
					info += "FPS " + ofToString(_fps) + (_bAlert ? " !" : "  ") + "\n";// alert when fps performance drops..
					info += "Lost Frames " + ofToString(lostFrames) + "\n";
					info += "DURATION : " + timer_str.get() + "\n";
#ifdef MODE_MORE_MINIMAL
					//info += infoFFmpeg;
					info += "F9 : STOP\n";
					// too much slow also when checking every 2 seconds...
					//info += "Disk Stills " + ofToString(amountStills) + "\n";
					//if (ofGetFrameNum() % 120 == 0) amountStills = dataDirectory.listDir();// refresh amount stills
					info += "Ctrl + m : SET MINIMAL INFO " + ofToString(!bShowMinimal ? "ON" : "OFF") + "\n";
#endif
				}
				else
				{
					// 1. waiting mount: press F8
					if (!bIsMounted && !isEncoding && !bIsRecording)
					{
						info += "> PRESS F8  TO MOUNT CAPTURER" + sp + "\n";
						info += "> PRESS F10 TO TAKE SNAPSHOT\n";
						if (bShowMinimal) info += "> PRESS M TO SET MINIMAL INFO " + ofToString(!bShowMinimal ? "ON" : "OFF") + "\n";
						//if (bShowMinimal) info += "> PRESS Ctrl + m TO SET MINIMAL INFO " + ofToString(!bShowMinimal ? "ON" : "OFF") + "\n";
					}

					// 2. mounted, recording or running ffmpeg script
					else if ((bIsMounted || bIsRecording || isEncoding))
					{
						if (!isEncoding)
						{
							info += "FPS " + ofToString(ofGetFrameRate(), 0) + "          " + ofToString(recorder.getFrame()) + " frames   \n";
							info += "Lost Frames     " + ofToString(lostFrames) + "\n";
							info += "WINDOW          " + ofToString(ofGetWidth()) + "x" + ofToString(ofGetHeight()) + "\n";
							info += "RECORDER        " + ofToString(recorder.getWidth()) + "x" + ofToString(recorder.getHeight()) + "\n";
							info += "FBO SIZE        " + ofToString(cap_w) + "x" + ofToString(cap_h) + "\n";
						}
						if (isSectionCustomized)
						{
							info += "SECTION         " + ofToString(rectSection.getX(), 0) + "," + ofToString(rectSection.getY(), 0);
							info += " " + ofToString(rectSection.getWidth(), 0) + "x" + ofToString(rectSection.getHeight(), 0) + "\n";
						}
						info += "Disk Stills     " + ofToString(amountStills) + "\n"; //info += "\n";

						if (bIsRecording)
						{
							info += "F9  : STOP\n";
							info += "RECORD DURATION: " + timer_str.get() + "\n";
							info += infoFFmpeg;

							// error
							if (bIsRecording) {
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
						else if (bIsMounted || isEncoding)// mounted or running ffmpeg script
						{
							if ((!bFfmpegLocated) && ofGetFrameNum() % 60 < 20) info += "> ALERT! Missing "+ffmpegNameBinary+"...";
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
									info += "                             \n";
									info += "                             \n";
								}
								info += "> PRESS F8  TO UNMOUNT\n";
							}
						}
					}
				}

				//-

				if (!bShowMinimal && (!bIsRecording || !isEncoding))
				{
					info += infoHelpKeys;//info += "\n";
				}
			}

			//-

			// draw text info
			float h = ofxSurfingHelpers2::getHeightBBtextBoxed(font, info);
			float y = ofGetHeight() - h - x + 8 - yOffest;// bad offset
			ofxSurfingHelpers2::drawTextBoxed(font, info, x, y);
			float ww = ofxSurfingHelpers2::getWidthBBtextBoxed(font, info);

			//-

			// red blink circle
			if (!
				(bError || (bShowMinimal && bIsRecording))) // exclude this states
			{
				if (bIsMounted || bIsRecording)
				{
					float radius = 10;
					int yy = y + radius - 10;
					//int xx = x + ww - radius + 25;
					int xx = x + ww - radius + 10;
					//if (bShowMinimal) xx += 5;

					// only while not encoding video
					//if (!isThreadRunning())
					if (!isEncoding)
					{
						ofPushStyle();
						ofColor c1{ ofColor(0,128) };
						ofColor c2{ ofColor(ofColor::red,200) };
						ofSetLineWidth(1.f);
						if (bIsRecording)
						{
							ofFill();
							ofSetColor(c2);
							ofDrawCircle(ofPoint(xx, yy), radius);
							ofNoFill();
							ofSetColor(c1);
							ofDrawCircle(ofPoint(xx, yy), radius);
						}
						else if (bIsMounted)
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

			//--

			// blinking preview custom section rect borders
			if (isSectionCustomized && bIsMounted && !bIsRecording) {
				ofPushStyle();
				int _period = 15;
				bool b = ofGetFrameNum() % _period > _period / 2;
				ofColor c = ofColor::white;
				//ofColor c = ofColor::blue;
				ofSetColor(ofColor(c, (b ? 16 : 8)));
				ofNoFill();
				ofSetLineWidth(1.0f);
				ofDrawRectangle(rectSection);
				ofPopStyle();
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
		ofLogWarning(__FUNCTION__) << " to: " << pathFolderStills;

		isEncoding = true;
		isPlayingPLayer = false;

		// we are running the systems commands
		// in a separated thread so it does
		// not block the drawing!
		startThread();
	}

	//--------------------------------------------------------------
	void setActive(bool b) {
		bEnable = b;
		//bShowInfo = bEnable;
	}

	//--------------------------------------------------------------
	void setToggleActive() {
		bEnable = !bEnable;
		//bShowInfo = bEnable;
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
		if (bEnable)
		{
			const int key = eventArgs.key;

			// modifiers
			bool mod_CONTROL = eventArgs.hasModifier(OF_KEY_CONTROL);
			bool mod_COMMAND = eventArgs.hasModifier(OF_KEY_COMMAND);
			bool mod_ALT = eventArgs.hasModifier(OF_KEY_ALT);
			bool mod_SHIFT = eventArgs.hasModifier(OF_KEY_SHIFT);



			// toggle show minimal
			if (key == 'M')// && mod_CONTROL)
			{
				//if (!mod_SHIFT && !mod_ALT && mod_CONTROL) 
				{
					setToggleShowMinimal();
					ofLogNotice(__FUNCTION__) << "bShowMinimal: " << (bShowMinimal ? "ON" : "OFF");
				}
			}

			//-

			switch (key)
			{
				//	//toggle active
				//case 'a':
				//	setToggleActive();
				//	break;

				// toggle show info
			case 'H':
				setToggleVisibleInfo();
				break;

				//	// toggle show minimal
				//case 'm':
				//{
				//	if (!mod_SHIFT && !mod_ALT && mod_CONTROL) {
				//		setToggleShowMinimal();
				//		ofLogNotice(__FUNCTION__) << "bShowMinimal: " << (bShowMinimal ? "ON" : "OFF");
				//	}
				//}

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
				bIsMounted = !bIsMounted;
				ofLogWarning(__FUNCTION__) << "Mount: " << (bIsMounted ? "ON" : "OFF");
			}
			break;

			// start recording
			case OF_KEY_F9:
			{
				if (bIsRecording)// do stop
				{
					//stopCapturer();
					bRecord = false;
				}
				else if (bIsMounted)// do start
				{
					//startCapturer();
					bRecord = true;
				}
				else ofLogError(__FUNCTION__) << "Must Mount before Start Capture!";
			}
			break;

			// take screenshot 
			// (not using ofxTextureRecorder and multi thread like on sequence stills capture)
			case OF_KEY_F10:
			{
				std::string _fileName = fileOutName + "__" + ofGetTimestampString() + ".png";
				//std::string _fileName = "snapshot_" + ofGetTimestampString() + ".png";
				std::string _pathFilename = ofToDataPath(pathFolderSnapshots + _fileName, true);//bin/data

				ofImage img;
				bool b;

				if (!isSectionCustomized) {
					img.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
				}
				else {
					img.grabScreen(0, 0, cap_w, cap_h);
				}
				b = img.save(_pathFilename);

				if (b) ofLogWarning(__FUNCTION__) << "Saved screenshot successfully: " << _pathFilename;
				else ofLogWarning(__FUNCTION__) << " Error saving screenshot:" << _pathFilename;

				// auto open snapshot
				std::string slog;
				slog = ofSystem(_pathFilename);
				cout << slog << endl;
			}
			break;

			// marge stills to video after capture
			case OF_KEY_F11:
			{
				startEncodeVideo();
			}
			break;

			// remove all captures stills
			case OF_KEY_BACKSPACE: // ctrl + alt + backspace
			{
				if (!mod_COMMAND && !mod_SHIFT && mod_ALT && mod_CONTROL)
				{
					ofLogWarning(__FUNCTION__) << "Remove all stills captures";
					dataDirectory.remove(true);
					ofxSurfingHelpers2::CheckFolder(pathFolderStills);
					amountStills = dataDirectory.listDir();
				}
			}
			break;
			}
		}
	}

	//--------------------------------------------------------------
	void windowResized(int w, int h) {// must be called to resize the fbo and video resolution
		if (!isSectionCustomized)// && bEnable) // we don't want to resize the canvas when  custom section is enabled
		{
			cap_w = w;
			cap_h = h;
			buildAllocateFbo();
		}

		// TODO: trying to allow resize..
		// this breakes the capturer...
		//buildRecorder();
	}

private:
	//--------------------------------------------------------------
	void buildHepKeysInfo() {// must be called after bitrate, framerate and size w/h are setted
		ofLogNotice(__FUNCTION__);

		// build help info
		infoHelpKeys = "\n";
		infoHelpKeys += "HELP KEYS"; infoHelpKeys += "\n";
		infoHelpKeys += "h   : Show Help info"; infoHelpKeys += "\n";
		//infoHelpKeys += "M   : Minimal Info ON/OFF\n";
		infoHelpKeys += "F5  : Set FullHD size"; infoHelpKeys += "\n";
		infoHelpKeys += "F6  : Set optimal Instagram size"; infoHelpKeys += "\n";
		infoHelpKeys += "F7  : Refresh Window size"; infoHelpKeys += "\n";

		if (!bShowMinimal)
		{
			infoHelpKeys += "F8  : Mount Recorder"; infoHelpKeys += "\n";
			infoHelpKeys += "F9  : Start/Stop Recording"; infoHelpKeys += "\n";
		}

		infoHelpKeys += "F10 : Capture Screenshot"; infoHelpKeys += "\n";
		infoHelpKeys += "F11 : Run FFmpeg video Encoder"; infoHelpKeys += "\n";
		infoHelpKeys += "M   : Minimal Info set " + ofToString(!bShowMinimal ? "ON" : "OFF") + "\n";
		//infoHelpKeys += "Ctrl + m : Minimal Info set " + ofToString(!bShowMinimal ? "ON" : "OFF") + "\n";
		infoHelpKeys += "Ctrl + Alt + BackSpace: Clear Stills"; infoHelpKeys += "\n";

		if (!bShowMinimal)
		{
			infoHelpKeys += "\n";
			infoHelpKeys += "PATHS\n";
			infoHelpKeys += "Stills      : " + pathFolderStills; infoHelpKeys += "\n";
			infoHelpKeys += "Screenshots : " + pathFolderSnapshots; infoHelpKeys += "\n";
			infoHelpKeys += pathRoot + "\n";
			//infoHelpKeys += "path root : \n" + pathRoot; infoHelpKeys += "\n";
		}

		//-

		infoFFmpeg = "\n";
		infoFFmpeg += "Texture copy       : " + ofToString(recorder.getAvgTimeTextureCopy()) + "\n";
		infoFFmpeg += "GPU download       : " + ofToString(recorder.getAvgTimeGpuDownload()) + "\n";
		infoFFmpeg += "Image encoding     : " + ofToString(recorder.getAvgTimeEncode()) + "\n";
		infoFFmpeg += "File save (avg ms) : " + ofToString(recorder.getAvgTimeSave()) + "\n";
		infoFFmpeg += "\n";
	}

	// external control
public:
	//--------------------------------------------------------------
	void startCapturer() {
		// TODO: should prompt to confirm by user or add some security locker flag !

		// 1. remove all captures stills
		dataDirectory.remove(true);

		ofxSurfingHelpers2::CheckFolder(pathFolderStills);
		amountStills = dataDirectory.listDir();
		ofLogWarning(__FUNCTION__) << "Remove all stills captures";
		resetFrameCounter();

		// 2. mount
		bIsMounted = true;
		ofLogWarning(__FUNCTION__) << "Mount: " << (bIsMounted ? "ON" : "OFF");

		// 3. start recording
		bIsRecording = true;

		bDisableCallbacks = true;
		if (!bRecord.get()) bRecord = true;
		bDisableCallbacks = false;

		timeStart = ofGetElapsedTimeMillis();
		ofLogWarning(__FUNCTION__) << "Start Recording into: " << pathFolderStills;

		lostFrames = 0;

		ofxSurfingHelpers2::CheckFolder(pathFolderCaptures);
		ofxSurfingHelpers2::CheckFolder(pathFolderStills);
		//ofxSurfingHelpers2::CheckFolder(pathFolderSnapshots);
	}

	//--------------------------------------------------------------
	void stopCapturer() {
		// 1. stop recording
		if (bIsRecording)
		{
			ofLogWarning(__FUNCTION__) << "Stop Recording";

			bIsRecording = false;

			bDisableCallbacks = true;
			if (bRecord.get()) bRecord = false;
			bDisableCallbacks = false;

			amountStills = dataDirectory.listDir();
		}

		// workflow
		if (bAutoencode.get())
		{
			// 2. start encoding
			ofLogWarning(__FUNCTION__) << "AutoStart encoding";
			//doRunFFmpegCommand();
			startEncodeVideo();
			//isMounted = false;
		}
	}

	//--------------------------------------------------------------
	void startEncodeVideo() {
		//!isThreadRunning()
		if (!isEncoding && !bIsRecording && bIsMounted)
		{
			doRunFFmpegCommand();
		}
		else
		{
#ifdef TEST_CONSOLE_KILL_PROCESS
			ofLogWarning(__FUNCTION__) << "Trying to force skip FFmpeg batch encoding: Can't be recording or already encoding";

			// TODO: BUG:
			// when called stop, must restart the app...
			if (isThreadRunning() && isEncoding) stopThread();

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
			someCmd << "taskkill /F /IM ffmpeg.exe";//TODO: force
			cout << someCmd;
			cout << ofSystem(someCmd.str().c_str()) << endl;
#endif
#ifdef TARGET_OSX
			someCmd << "say Hello";
#endif
			cout << "> DONE !" << endl;
#endif
		}

		if (amountStills == 0) {
			ofLogError(__FUNCTION__) << "Missing stills files into " << pathFolderStills << " !";
		}
	}

	//--------------------------------------------------------------
	void resetFrameCounter() {
		recorder.resetFrameCounter();
	}
	//--------------------------------------------------------------
	void setOutputFilename(std::string name) {
		fileOutName = name;
	}
public:
	// this allows to change the destination folder on runtime
	// making easier to sort our captures
	// will create the folder if do not exist
	//--------------------------------------------------------------
	void setPathCaptures(std::string path) {

		// workaound to adapt path formats between different platforms projects
#ifdef TARGET_WIN32
		ofStringReplace(path, "/", "\\");
#endif
#ifdef TARGET_OSX
		ofStringReplace(path, "\\", "/");
#endif

#ifdef TARGET_WIN32
		pathFolderCaptures = path;
		//pathFolderCaptures = path + "\\";
		pathFolderStills = pathFolderCaptures + "Stills\\";
		pathFolderSnapshots = pathFolderCaptures + "Snapshots\\";
#endif
#ifdef TARGET_OSX
		pathFolderCaptures = path;
		//pathFolderCaptures = path + "/";
		pathFolderStills = pathFolderCaptures + "Stills/";
		pathFolderSnapshots = pathFolderCaptures + "Snapshots/";
#endif

		ofxSurfingHelpers2::CheckFolder(pathFolderCaptures);
		ofxSurfingHelpers2::CheckFolder(pathFolderStills);
		ofxSurfingHelpers2::CheckFolder(pathFolderSnapshots);

		buildHepKeysInfo();

		if (!isSectionCustomized) {
			cap_w = ofGetWidth();
			cap_h = ofGetHeight();
			buildAllocateFbo();
		}

		// stills folder
		// let the folder open to list amount files sometimes...
		dataDirectory.open(ofToDataPath(pathFolderStills, true));
		amountStills = dataDirectory.listDir();

		// re set ofxTextureRecorder
		recorder.setPath(pathFolderStills);
	}

private:
	// call ffmpeg command
	// join all stills to a video file
	//--------------------------------------------------------------
	void threadedFunction() {
		if (bEnable) {
			ofLogWarning(__FUNCTION__) << endl;

			cout << "--------------------------------------------------------------" << endl;
			cout << (__FUNCTION__) << endl;

			std::string warninglog = "";
			warninglog += "> WARNING! Sometimes ofApp must run as Administrator !\n";
			warninglog += "> WARNING! ffmpeg binary must be located on: " + pathRoot + ffmpegNameBinary + " !\n";

			cout << endl << warninglog << endl;

			if (isEncoding)
			{
				// build ffmpeg command

				cout << "> Starting join all stills (xxxxx.tif) to a video file (.mp4)...";

				pathAppData = pathRoot;
                
#ifdef TARGET_WIN32
                               ffmpeg = pathAppData + ffmpegNameBinary;
#endif
#ifdef TARGET_OSX
                ffmpeg = ofToDataPath( ffmpegNameBinary, true );
#endif

				// input files
#ifdef TARGET_WIN32
                pathDest = pathAppData + pathFolderCaptures;
                filesSrc = pathAppData + pathFolderStills + "%05d.tif"; // data/stills/%05d.tif
#endif
#ifdef TARGET_OSX
                pathDest = ofToDataPath( pathAppData + pathFolderCaptures, true );
                filesSrc = ofToDataPath( pathAppData + pathFolderStills + "%05d.tif", true ); // data/stills/%05d.tif
#endif


				// output video file
				if (bOverwriteOutVideo) nameDest = fileOutName + ".mp4"; // "output.mp4";
				else nameDest = fileOutName + "__" + ofGetTimestampString() + ".mp4"; // "output_2020-10-11-19-08-01-417.mp4";// timestamped

				fileOutPath = pathDest + nameDest;

				//-

				// used template to join stills:

				// https://trac.ffmpeg.org/wiki/Encode/H.264
				// Constant Rate Factor: CRF scale is 0�51, where 0 is lossless, 23 is the default, and 51 is worst quality possible. 
				// A lower value generally leads to higher quality, and a subjectively sane range is 17�28. 
				// Consider 17 or 18 to be visually lossless or nearly so; it should look the same or nearly the same as the input but it isn't technically lossless. 
				// https://bytescout.com/blog/2016/12/ffmpeg-command-lines-convert-various-video-formats.html

				// Template is selectable by API

				//-

				cmd = "";
				cmdEncodingArgs = "";

				//--

				// A

				// customized script by user:
				if (bFfmpegCustomScript)
				{
					// 1. append exe + source files
					cmd += ffmpeg + " -y -f image2 -i " + filesSrc + " ";

					// 2. apend script
					cmd += ffmpegScript + " ";

					// 3. append file output
					cmd += fileOutPath;

					//-

					cout << endl << endl;
					cout << "> CUSTOM FFmpeg SCRIPT" << endl << endl;
					cout << "> " + ffmpegNameBinary + " : " << endl << ffmpeg;
					cout << endl << endl;
					cout << "> Source: " << endl << filesSrc;
					cout << endl << endl;
					cout << "> FFmpeg CustomScript : " << endl << ffmpegScript;
					cout << endl << endl;
					cout << "> Out : " << endl << fileOutPath;
					cout << endl << endl;
					cout << "> Raw Command: " << endl << cmd;
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

					cmd += ffmpeg + " ";

					cmd += "-y -f image2 ";
					cmd += "-r 60 ";
					cmd += "-i " + filesSrc + " ";

					// we can resize too or mantain the original window size
					//cmd << "-s hd1080 ";
					//cmd << "-s 1920x1080 ";

					// B

					// 2. append encoding settings
					// template 1: (CPU)
					if (!bUseFFmpegGPU) cmdEncodingArgs += "-c:v libx264 -preset veryslow -qp 0 ";

					//-

#ifdef TARGET_WIN32

					// C

					// template 2: (Nvidia GPU)
					else if (bUseFFmpegGPU)
					{
						cmdEncodingArgs += "-c:v h264_nvenc ";// enables GPU hardware accellerated Nvidia encoding. Could check similar arg to AMD..
						cmdEncodingArgs += "-b:v 25M "; // constant bitrate 25000
						//mdEncodingArgs += "-framerate 60 ";// BUG: not applying 60 fps ??
						cmdEncodingArgs += "-crf 20 ";
						//cmdEncodingArgs += "-vsync 0 ";
						//cmdEncodingArgs += "-hwaccel cuvid ";
						//cmdEncodingArgs += "-qp 0 ";
						cmdEncodingArgs += "-preset slow ";	// 10secs = 30MB
						//cmdEncodingArgs += "-preset lossless ";	// 10secs = 150MB
						//cmdEncodingArgs += "-profile high ";
						//cmdEncodingArgs += "-pix_fmt yuv444p ";// 10secs = 300MB. doubles size! raw format but too heavy weight!

						// https://www.reddit.com/r/ffmpeg/comments/8t54bm/converting_yuvj420p_to_yuv420p_issues_with_black/
						// TODO: TEST: trying to get real black color (on the video) instead of dark grey...
						//ffmpeg -framerate 30 -i input_%05d.png -vf format=yuv420p output.mp4
						//cmdEncodingArgs += "-vf \"scale = w = -2:h = 1920 : sws_flags = spline + accurate_rnd : in_range = tv : out_range = tv\" ";
						//cmdEncodingArgs += "-colorspace bt709 -color_trc bt709 -color_primaries bt709 -color_range tv ";
						//cmdEncodingArgs += "-vf format=yuv420p ";
						//cmdEncodingArgs += "-pix_fmt yuv420p ";

					}
#endif
#ifdef TARGET_OSX
					// template 2: (macOS GPU)
					else if (bUseFFmpegGPU)
					{
						// TODO:
						// must search a script for macOS, because in general they have AMD Radeon GPU, not Nvidia
						//// CPU
						//cmdEncodingArgs += "-c:v libx264 -preset veryslow -qp 0 ";

						// GPU Nvidia
						cmdEncodingArgs += "-c:v h264_nvenc ";// enables GPU hardware accellerated Nvidia encoding. Could check similar arg to AMD..
						cmdEncodingArgs += "-b:v 25M "; // constant bitrate 25000
						cmdEncodingArgs += "-crf 20 ";
						//cmdEncodingArgs += "-vsync 0 ";
						//cmdEncodingArgs += "-hwaccel cuvid ";
						//cmdEncodingArgs += "-qp 0 ";
						cmdEncodingArgs += "-preset slow ";	// 10secs = 30MB
						//cmdEncodingArgs += "-preset lossless ";	// 10secs = 150MB
						//cmdEncodingArgs += "-profile high ";
						//cmdEncodingArgs += "-pix_fmt yuv444p ";// 10secs = 300MB. doubles size! raw format but too heavy weight!
					}
#endif
					//--

					// TODO:
					// add customzable fps:
					cmdEncodingArgs += "-r 60 ";// BUG: not applying 60 fps ??

					// append
					cmd += cmdEncodingArgs;

					// 3. append file output
					cmd += fileOutPath;

					//-

					cout << endl << endl;
					cout << "> HARDCODED FFmpeg SCRIPT" << endl << endl;
					cout << "> " + ffmpegNameBinary + " : " << endl << ffmpeg;
					cout << endl << endl;
					cout << "> Source : " << endl << filesSrc;
					cout << endl << endl;
					cout << "> Out : " << endl << fileOutPath;
					cout << endl << endl;
					cout << "> Raw Command: " << endl << cmd;
					cout << endl << endl;
					cout << "> Quality Encoding arguments: " << endl << cmdEncodingArgs;
					cout << endl << endl;
				}

				//-

				// 4. run video encoding

				std::string slog;
				slog = ofSystem(cmd);
				cout << endl << "> Log: " << endl << slog << endl;

				//-

				cout << endl;
				cout << "> DONE/TRYING VIDEO ENCODING INTO: " << endl << fileOutPath;
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
				cout << "> This is to allow run "+ffmpegNameBinary+" and access to files from here!" << endl;

				//-

				// 5. open video file with your system player
				slog = ofSystem(fileOutPath);
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

				// TODO:
				// should check system log to know if failed..
				//bError = true;// workaround. i don't know how to stop the process without breaking the thread restart...
			}
		}
	}

	//--

};
