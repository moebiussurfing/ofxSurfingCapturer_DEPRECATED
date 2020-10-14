#pragma once

#include "ofMain.h"

// NOTES: 
// this class is currently kind of deprecated... 
// Now I am getting much better performance using the ofxSurfing_CaptureWindowStills.h class !
// -> it saves just raw snapshots (without compression) for every frame. 
// video is created after capture process with an ffmpeg script.
//
// TODO: must finish macOS mode... 
// it's only tested on Windows.
// BUG: (maybe) when enabled antialias 16 or RGBF32 recording goes grey... ??
// another alternative repo:
// https://github.com/tyhenry/ofxFFmpeg

//-

//windows ffmpeg screen recorder
#ifdef TARGET_WIN32
#define USE_FFMPEG_RECORDER//TODO: disable
#ifdef USE_FFMPEG_RECORDER
#include "ofxFFmpegRecorder.h"
#include "ofxFastFboReader.h"
#endif
#endif

//macOS screen recorder
#ifdef TARGET_OSX
#define USE_MAC_RECORDER
#ifdef USE_MAC_RECORDER
#include "ofxMacScreenRecorder.h"
#endif
#endif

#include "ofxSurfingHelpers2.h"

class CaptureWindow
{
public:
	CaptureWindow() {
		cap_w = 1920;
		cap_h = 1080;
		cap_Bitrate = 10000;
		cap_Framerate = 30;

		//string _font = "assets/fonts/telegrama_render.otf";
		string _font = "assets/fonts/overpass-mono-bold.otf";
		font.load(_font, 8);
	};

	~CaptureWindow() {};

	//-

private:
	//macOS screen recorder
#ifdef USE_MAC_RECORDER
	ofxMacScreenRecorder recorder;
	ofxMacScreenRecorderSetting recorderSetting;
#endif

private:
	//windows ffmpeg screen recorder
#ifdef USE_FFMPEG_RECORDER
	ofxFFmpegRecorder cap_Recorder;
	ofFbo cap_Fbo;
	ofPixels cap_Pix;

	ofxFastFboReader cap_Reader;
	ofFbo::Settings cap_Fbo_Settings;

	bool bRecPrepared = false;
	int cap_w, cap_h;
	int cap_Bitrate;
	int cap_Framerate;
#endif

	//TEST: BUG: antialias
	//ofTexture cap_Tex;
	ofFbo blitFbo;

	std::string textInfo;
	std::string _pathFolder;
	ofTrueTypeFont font;
public:
	//--------------------------------------------------------------
	void init() {///must be called after bitrate, framerate and size w/h are setted

		//setup and resolution
		cap_Recorder.setup(true, false, glm::vec2(cap_w, cap_h), cap_Framerate);

		//quality
		cap_Recorder.setBitRate(cap_Bitrate);

		//-

		//extra options
		cap_Recorder.setOverWrite(true);
		cap_Recorder.setFFmpegPathToAddonsPath();
		//cap_Recorder.addAdditionalOutputArgument("-f rawvideo");
		//cap_Recorder.setVideoCodec("libx264");
		//cap_Reader.setAsync(true);

		//-

		//build help info
		textInfo = "";
		textInfo += "HELP KEYS"; textInfo += "\n";
		textInfo += "F8 : Mount Recorder"; textInfo += "\n";
		textInfo += "F9 : Start Recording"; textInfo += "\n";
		textInfo += "F10: Take Snapshot"; textInfo += "\n";
		textInfo += "i  : Set optimal Instagram size"; textInfo += "\n";
		textInfo += "I  : Get a frame from video file"; //textInfo += "\n";
	}

	//--------------------------------------------------------------
	void setup(std::string path = "captures/") {///call with the path folder if you want to customize
		_pathFolder = path;
		ofxSurfingHelpers2::CheckFolder(_pathFolder);

		//windows ffmpeg screen recorder
#ifdef USE_FFMPEG_RECORDER
		cap_w = ofGetWidth();
		cap_h = ofGetHeight();

		//TEST: BUG: antialias
		//cap_Pix.allocate(cap_w, cap_h, GL_RGB);
		//cap_Pix.clear();

		//fbo
		{
			cap_Fbo_Settings.internalformat = GL_RGB;//GL_RGB32F;

			cap_Fbo_Settings.width = cap_w;
			cap_Fbo_Settings.height = cap_h;

			cap_Fbo_Settings.numSamples = 16; // BUG: on ofxFastFboReader requires an aux blitFbo...
			cap_Fbo_Settings.useDepth = true;
			//cap_Fbo_Settings.useStencil = true;
			//cap_Fbo_Settings.depthStencilAsTexture = true;
			//cap_Fbo_Settings.maxFilter

			cap_Fbo.allocate(cap_Fbo_Settings);
		}

		//TEST: BUG: depth/antialias
		blitFbo.allocate(cap_Fbo.getWidth(), cap_Fbo.getHeight(), GL_RGB);//GL_RGB32F);

		/*
		//-
		//
		//instagram settings
		//
		//https://www.bing.com/search?q=best+ffmpeg+settings+for+instagram+video&cvid=6165323b777d42699a0de21423b93032&FORM=ANNTA1&PC=U531
		//https://www.oberlo.com/blog/best-instagram-video-format
		//https://blog.snappa.com/instagram-video-format/
		//https://www.youtube.com/watch?v=ZjA2aYyC_0I
		//
		//The best Instagram video dimensions: 864 pixels(width) by 1080 pixels(height)
		//aspect ratio of 4:5.
		//H.264 codec
		//AAC audio
		//3500 kbps bitrate
		//Frame rate of 30 fps(frames per second)
		//Video can be a maximum of 60 seconds
		//Maximum video width is 1080 px(pixels) wide
		//Videos should be 920 pixels tall
		//
		Output #0, avi, to 'F:\openFrameworks\addons\ofxSphereCam2\ofxSphereCam2-example\bin\data\captures/2020-08-12-01-31-00-756.avi':
		 Metadata:
		ISFT            : Lavf57.67.100
		Stream #0:0: Video: mpeg4 (FMP4 / 0x34504D46), yuv420p, 864x1061, q=2-31, 28000 kb/s, 30 fps, 30 tbn, 30 tbc
		Metadata:
		  encoder         : Lavc57.86.103 mpeg4
		Side data:
		  cpb: bitrate max/min/avg: 0/0/28000000 buffer size: 0 vbv_delay: -1
		frame=  676 fps= 30 q=2.0 Lsize=     870kB time=00:00:22.53 bitrate= 316.4kbits/s speed=   1x
		video:848kB audio:0kB subtitle:0kB other streams:0kB global headers:0kB muxing overhead: 2.600246%
		//-
		*/

		//--

		// some fffmpeg presets

		// TODO: may add some ffmpeg gpu commands to make it faster...

		//// 1. instagram
		//cap_Bitrate = 28000;
		//cap_Framerate = 30;

		// 2. hq
		cap_Bitrate = 80000;
		cap_Framerate = 60;

		//-

		init();
#endif
		//-

		//macOS screen recorder
#ifdef USE_MAC_RECORDER
		recorderSetting.codecType = ofxMacScreenRecorder::CodecType::ProRes4444;
		if (!recorder.setup(recorderSetting))
			ofExit(-1);
		recorder.registerFinishWritingCallback([this](const std::string &path)
			{
				ofLogNotice() << "success recording. save to: " << path;
			});
		recorder.registerStartWritingCallback([this]
			{
			});
#endif
	}

	//--------------------------------------------------------------
	void begin() {///call before draw the scene to record

		//windows ffmpeg screen recorder
#ifdef USE_FFMPEG_RECORDER
		if (bRecPrepared)
		{
			cap_Fbo.begin();

			//ofBackground(0);
			//ofSetColor(255);
			//ofClear(0, 255);
		}
#endif
	}

	//--------------------------------------------------------------
	void end() {///call after draw the scene to record

		//windows ffmpeg screen recorder
#ifdef USE_FFMPEG_RECORDER
		if (bRecPrepared) {
			//ofClearAlpha();
			cap_Fbo.end();

			if (cap_Recorder.isRecording())
			{
				////A. faster
				////ofxFastFboReader can be used to speed this up:
				//cap_Reader.readToPixels(cap_Fbo, cap_Pix, OF_IMAGE_COLOR);
				//if (cap_Pix.getWidth() > 0 && cap_Pix.getHeight() > 0) {
				//	cap_Recorder.addFrame(cap_Pix);
				//}

				////B. standard
				//cap_Fbo.readToPixels(cap_Pix);
				//if (cap_Pix.getWidth() > 0 && cap_Pix.getHeight() > 0) {
				//	cap_Recorder.addFrame(cap_Pix);
				//}

				//-

				//TEST: BUG: antialias
				//cap_Tex.allocate(cap_Pix);

				//TEST: BUG: antialias
				//C. blitting test
				blitFbo.begin();
				ofClear(0, 255);
				cap_Fbo.draw(0, 0, cap_w, cap_h);
				blitFbo.end();

				cap_Reader.readToPixels(blitFbo, cap_Pix, OF_IMAGE_COLOR);

				if (cap_Pix.getWidth() > 0 && cap_Pix.getHeight() > 0)
				{
					cap_Recorder.addFrame(cap_Pix);
				}
			}

			//-

			cap_Fbo.draw(0, 0);//drawing is required outside fbo
		}
#endif
	}

	//--------------------------------------------------------------
	void draw() {///draw the gui info if desired

		//draw red circle and info when recording
		ofPushStyle();

		//windows ffmpeg screen recorder
#ifdef USE_FFMPEG_RECORDER

		//TODO: must improve performance using less draw calls...
		string str = "";

		if (bRecPrepared || cap_Recorder.isRecording()) {

			//cap info
			str += "FFMPEG SETTINGS\n";
			str += "SIZE " + ofToString(cap_w) + "x" + ofToString(cap_h);
			str += "\n";
			str += "BITRATE " + ofToString(cap_Bitrate);
			str += "\n";
			str += "FRAMERATE " + ofToString(cap_Framerate);
			str += "\n\n";

			//fps
			str += "FPS " + ofToString(ofGetFrameRate(), 0);
			str += "\n";

			//refresh window size
			if (cap_Recorder.isRecording())
			{
				str += "RECORD DURATION: " + ofToString(cap_Recorder.getRecordedDuration(), 1);
				str += "\n";
				str += "KEY F9: STOP";
				str += "\n";
			}
			else if (bRecPrepared)
			{
				str += "RECORD MOUNTED. READY...";
				str += "\n";
				str += "KEY F9: START";
				str += "\n";
				str += "KEY F8: UNMOUNT";
				str += "\n";
			}
			str += "KEY F7: REFRESH WINDOW SIZE";
			str += "\n";
		}

		{
			str += "\n" + textInfo;

			//-

			// draw
			int x = 40;
			float h = ofxSurfingHelpers2::getHeightBBtextBoxed(font, str);//TODO: ? makes a bad offset..
			int y = ofGetHeight() - h - 90;

			ofxSurfingHelpers2::drawTextBoxed(font, str, x, y);

			////TEST: BUG: antialias
			//if (cap_Recorder.isRecording())
			//{
			//	cap_Tex.draw(100, 100, 800, 600);
			//}

			//-

			//red rec circle
			x += 190;
			y += 10;
			float radius = 12;
			if (cap_Recorder.isRecording())
			{
				ofFill();
				ofSetColor(ofColor::red);
				ofDrawCircle(ofPoint(x + radius, y), radius);
				ofNoFill();
				ofSetLineWidth(2.f);
				ofSetColor(ofColor::black);
				ofDrawCircle(ofPoint(x + radius, y), radius);
			}
			else if (bRecPrepared)
			{
				if (ofGetFrameNum() % 60 < 20) {
					ofFill();
					ofSetColor(ofColor::red);
					ofDrawCircle(ofPoint(x + radius, y), radius);
				}
				ofNoFill();
				ofSetLineWidth(2.f);
				ofSetColor(ofColor::black);
				ofDrawCircle(ofPoint(x + radius, y), radius);
			}
		}
#endif

		ofPopStyle();
	}

	////--------------------------------------------------------------
	//void drawHelp() {
	//	// help info
	//	ofDrawBitmapStringHighlight(textInfo, 20, 50);
	//}

	//--------------------------------------------------------------
	void keyPressed(ofKeyEventArgs &eventArgs) {///to received short keys control commands
		const int key = eventArgs.key;
		const int keycode = eventArgs.keycode;
		const int scancode = eventArgs.scancode;
		const uint32_t codepoint = eventArgs.codepoint;

		//-

		switch (key)
		{
			//windows ffmpeg screen recorder
			//case OF_KEY_F11://screenshot pict
			//cap_Recorder.saveThumbnail(0, 0, 2, ("data/captures/cap" + ofGetTimestampString() + ".png"), ofVec2f(0, 0), ofRectangle(0, 0, 500, 400));
			//break;

			//set instagram size
		case 'i':
		{
			int w, h;
			w = 864;
			h = 1080 + 19;
			ofSetWindowShape(w, h);
			windowResized(w, h);

			cap_w = w;
			cap_h = h;

			//--

			//windows ffmpeg screen recorder
			cap_Fbo_Settings.width = cap_w;
			cap_Fbo_Settings.height = cap_h;
			cap_Fbo.allocate(cap_Fbo_Settings);
			blitFbo.allocate(cap_Fbo.getWidth(), cap_Fbo.getHeight());

			//-

			//presets

			//instagram
			cap_Bitrate = 28000;
			cap_Framerate = 30;

			////hq
			//cap_Bitrate = 80000;
			//cap_Framerate = 60;

			//-

			init();
		}
		break;

		//prepare video record
		case OF_KEY_F8:
			bRecPrepared = !bRecPrepared;
			break;

			//start video record
		case OF_KEY_F9:
		{
			if (cap_Recorder.isRecording())//stop
			{
				cap_Recorder.stop();
				ofLogWarning(__FUNCTION__) << "Stop Recording";

				bRecPrepared = false;
			}
			else//start
			{
				//string path = "data/";
				//ofxSurfingHelpers2::CheckFolder(path);
				//std::string fileCap = "data/capture" + ofGetTimestampString() + ".avi";
				//std::string fileCap = path + "capture_01.avi";
				//cap_Recorder.setOutputPath(fileCap);

				//bin/data
				string _path = ofToDataPath(_pathFolder + ofGetTimestampString() + ".avi", true);
				cap_Recorder.setOutputPath(_path);

				//start record
				cap_Recorder.startCustomRecord();
				ofLogWarning(__FUNCTION__) << "Start Recording: " << _path;
			}
		}
		break;

		//take screenshot
		case OF_KEY_F10:
		{
			ofImage img;
			img.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
			//string _pathFolder = "captures/";
			//string _fileName = "snapshot_"+ofToString(snapCounter++, 5, '0')+".png";
			string _fileName = "snapshot_" + ofGetTimestampString() + ".png";
			string _pathFilename = ofToDataPath(_pathFolder + _fileName, true);//bin/data
			bool b = img.save(_pathFilename);
			if (b) cout << __FUNCTION__ << " Saved screenshot successfully: " << _pathFilename << endl;
			else cout << __FUNCTION__ << " Error saving screenshot: " << _pathFilename << endl;
		}
		break;

		//gets a frame from the located video. not a live screenshot!
		case 'I':
		{
			string _path = ofToDataPath(_pathFolder + "Screenshot_" + ofToString(ofGetTimestampString()) + ".png");
			cap_Recorder.saveThumbnail(0, 0, 2, _path, ofVec2f(0, 0), ofRectangle(0, 0, ofGetWidth(), ofGetHeight()));
			ofLogWarning(__FUNCTION__) << "Snapshot: " << _path;
		}
		break;

		case OF_KEY_F7:// refresh window size to update fbo settings
			windowResized(ofGetWidth(), ofGetHeight());
			break;

			//-

			//macOS screen recorder
#ifdef USE_MAC_RECORDER
		case OF_KEY_F11:
			ofLogNotice() << ofGetWindowPositionX() << ", " << ofGetWindowPositionY();
			recorder.setSetting(recorderSetting);
			recorder.start(ofToDataPath("./test"));
			break;
		case OF_KEY_F12:
			recorder.stop();
			break;
#endif
		}
		}

	//windows ffmpeg screen recorder
	//--------------------------------------------------------------
	void windowResized(int w, int h) {///must be called to resize the fbo and video resolution
#ifdef USE_FFMPEG_RECORDER
		cap_w = w;
		cap_h = h;

		cap_Fbo_Settings.width = cap_w;
		cap_Fbo_Settings.height = cap_h;
		cap_Fbo.allocate(cap_Fbo_Settings);
		blitFbo.allocate(cap_Fbo.getWidth(), cap_Fbo.getHeight());

		//-

		//presets

		////instagram
		//cap_Bitrate = 28000;
		//cap_Framerate = 30;

		////hq
		//cap_Bitrate = 80000;
		//cap_Framerate = 60;

		//-

		init();
#endif
	}
	};