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
	bool bFfmpegCustomScript = false;
	std::string ffmpegScript;
public:
	//--------------------------------------------------------------
	void setFFmpegScript(std::string sscript) {// if this is setted, the other hardcoded CPU/GPU aren't used at all
		bFfmpegCustomScript = true;
		ffmpegScript = sscript;
	}

private:
	bool bDepth3D = true;
	// BUG solved: when using antialias/depth we get "black screen"
	ofFbo blitFbo;// we need this aux fbo to solve the bug

private:
	bool bOverwriteOutVideo = true;// we only want the last video. we use same name for all re takes
public:
	//--------------------------------------------------------------
	void setOverwriteVideoOut(bool b) {// default is true. set to false to allow add timestamp to filenames
		bOverwriteOutVideo = b;
	}

private:
	bool bUseFfmpegNvidiaGPU = true;
public:
	//--------------------------------------------------------------
	void setFfpmegGpu(bool b) {
		bUseFfmpegNvidiaGPU = b;
	}
public:
	//--------------------------------------------------------------
	void setPathRoot(std::string path) {
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

		_pathFolderCaptures = "Captures\\";
		_pathFolderStills = _pathFolderCaptures + "Stills\\";
		_pathFolderSnapshots = _pathFolderCaptures + "Snapshots\\";

		//string _font = "assets/fonts/telegrama_render.otf";
		string _font = "assets/fonts/overpass-mono-bold.otf";
		font.load(_font, 8);

		// default root path is /bin/data/
		setPathRoot(ofToDataPath("\\", true));
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
	ofParameter<bool> bActive{ "Window Capturer", true };// public to integrate into your ofApp gui
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

private:
	uint32_t timeStart;
	std::string info;

	ofImageFormat stillFormat;

public:
	//--------------------------------------------------------------
	//call with the path folder if you want to customize
	void setup(std::string path = "Captures\\", ofImageFormat format = OF_IMAGE_FORMAT_TIFF) {
		ofLogWarning(__FUNCTION__) << "path: " << path << " ofImageFormat: " << format;

		// we can select a still format passing one ofImageFormat like this ones:
		// OF_IMAGE_FORMAT_BMP = 0,
		// OF_IMAGE_FORMAT_JPEG = 2,
		// OF_IMAGE_FORMAT_PNG = 13,
		// OF_IMAGE_FORMAT_TIFF = 18,
		// OF_IMAGE_FORMAT_RAW = 34

		_pathFolderCaptures = path; // "Captures\\"
		_pathFolderStills = _pathFolderCaptures + "Stills\\";
		_pathFolderSnapshots = _pathFolderCaptures + "Snapshots\\";

		ofxSurfingHelpers2::CheckFolder(_pathFolderCaptures);
		ofxSurfingHelpers2::CheckFolder(_pathFolderStills);
		ofxSurfingHelpers2::CheckFolder(_pathFolderSnapshots);

		buildInfo();

		cap_w = ofGetWidth();
		cap_h = ofGetHeight();
		cap_Fbo.allocate(cap_w, cap_h, GL_RGB);

		blitFbo.allocate(cap_Fbo.getWidth(), cap_Fbo.getHeight(), GL_RGB);

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
	void buildAllocateFbo() {// cap_w and cap_h must be updated before call
		ofLogWarning(__FUNCTION__) << cap_w << ", " << cap_h;

		cap_Fbo_Settings.internalformat = GL_RGB;
		cap_Fbo_Settings.width = cap_w;
		cap_Fbo_Settings.height = cap_h;

#ifdef USE_3D_DEPTH
		if (bDepth3D) {
			cap_Fbo_Settings.useDepth = true;// required to enable depth test
			cap_Fbo_Settings.numSamples = (int)ANTIALIAS_NUM_SAMPLES;// antialias 
			//cap_Fbo_Settings.useStencil = true;
			//cap_Fbo_Settings.depthStencilAsTexture = true;
			//cap_Fbo_Settings.maxFilter
		}
#endif
		cap_Fbo.allocate(cap_Fbo_Settings);

		blitFbo.allocate(cap_Fbo.getWidth(), cap_Fbo.getHeight());
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
		//cap_Fbo.draw(0, 0);// drawing is required outside fbo

		// TEST: BUG: depth/antialias
		blitFbo.begin();
		ofClear(0, 255);
		cap_Fbo.draw(0, 0, cap_w, cap_h);
		blitFbo.end();
		blitFbo.draw(0, 0);
	}

	//--------------------------------------------------------------
	void drawInfo(int x = 40, int y = 0) {// draw the gui info if desired

		if (bShowInfo && bActive) {

			string str = "\n\n\n";

			//--

			// 1. waiting mount: press F8
			if (!bRecPrepared && !isThreadRunning() && !bRecording)
			{
				str += "F8 : MOUNT Recorder\n";

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

				str += "> PRESS F8" + sp + "\n";
			}
			// 2. mounted, recording or running ffmpeg script
			else if (bRecPrepared || bRecording || isThreadRunning())
			{
				// cap info
				str += "FPS " + ofToString(ofGetFrameRate(), 0) + "   " + ofToString(recorder.getFrame()) + " frames\n";
				str += "WINDOW   " + ofToString(ofGetWidth()) + "x" + ofToString(ofGetHeight()) + "\n";
				str += "FBO SIZE " + ofToString(cap_w) + "x" + ofToString(cap_h) + "\n";
				str += "RECORDER " + ofToString(recorder.getWidth()) + "x" + ofToString(recorder.getHeight()) + "\n";
				str += +"\n";

				if (bRecording)
				{
					str += "F9 : STOP Recording\n";
					str += "RECORD DURATION: " + ofToString(getRecordedDuration(), 1) + "\n";

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
					if ((!bFfmpegLocated) && ofGetFrameNum() % 60 < 20) str += "> ALERT! Missing ffmpeg.exe...";
					str += "\n";

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

					if (isThreadRunning()) {
						str += "> ENCODING VIDEO" + sp;
						str += " [" + ofToString(bUseFfmpegNvidiaGPU ? "GPU" : "CPU") + "]\n";
					}

					else {
						str += "> MOUNTED! READY" + sp + "\n";
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
					ofLogWarning(__FUNCTION__) << "Texture copy   : " << recorder.getAvgTimeTextureCopy();
					ofLogWarning(__FUNCTION__) << "GPU download   : " << recorder.getAvgTimeGpuDownload();
					ofLogWarning(__FUNCTION__) << "Image encoding : " << recorder.getAvgTimeEncode();
					ofLogWarning(__FUNCTION__) << "File save	  : " << recorder.getAvgTimeSave() << endl;
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
				if (!isThreadRunning())
				{
					doRunFFmpegCommand();
				}
				else
				{
					// TODO: BUG:
					// when called stop, must restart the app...
					if (isThreadRunning()) stopThread();
				}
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

	// call ffmpeg command
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

			cout << "> Starting join all stills (xxxxx.tif) to a video file (.mp4)...";

			stringstream cmd;
			stringstream cmdEncodingArgs;
			stringstream ffmpeg;
			stringstream filesSrc;
			stringstream pathDest;
			stringstream nameDest;
			stringstream fileOut;
			stringstream pathAppData;

			pathAppData << pathRoot;

			ffmpeg << pathAppData.str().c_str() << "ffmpeg.exe";

			// input files
			pathDest << pathAppData.str().c_str() << _pathFolderCaptures;
			filesSrc << pathAppData.str().c_str() << _pathFolderStills << "%05d.tif"; // data/stills/%05d.tif

			// output video file
			if (bOverwriteOutVideo) nameDest << "output.mp4"; // "output.mp4";
			else nameDest << "output_" << ofGetTimestampString() << ".mp4"; // "output_2020-10-11-19-08-01-417.mp4";// timestamped
			fileOut << pathDest.str().c_str() << nameDest;

			//-

			// used template to join stills:

			// https://trac.ffmpeg.org/wiki/Encode/H.264
			// Constant Rate Factor: CRF scale is 0–51, where 0 is lossless, 23 is the default, and 51 is worst quality possible. 
			// A lower value generally leads to higher quality, and a subjectively sane range is 17–28. 
			// Consider 17 or 18 to be visually lossless or nearly so; it should look the same or nearly the same as the input but it isn't technically lossless. 
			// https://bytescout.com/blog/2016/12/ffmpeg-command-lines-convert-various-video-formats.html

			// Template is selectable by API

			//-

			// customized script by user:
			if (bFfmpegCustomScript)
			{
				// 1. append exe + source files
				cmd << ffmpeg << " -y -f image2 -i " << filesSrc.str().c_str() << " ";

				// 2. apend script
				cmd << ffmpegScript.c_str() << " ";

				// 3. append file output
				cmd << fileOut.str().c_str();

				//-

				cout << endl << endl;
				cout << "> CUSTOM FFmpeg SCRIPT" << endl << endl;
				cout << "> ffmpeg.exe : " << endl << ffmpeg.str().c_str();
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
				cmd << ffmpeg << " -y -f image2 -i " << filesSrc.str().c_str() << " ";
				cmd << "-r 60 ";// framerate

				// we can resize too or mantain the original window size
				//cmd << "-s hd1080 ";
				//cmd << "-s 1920x1080 ";

				// 2. append encoding settings
				// template 1: (CPU)
				if (!bUseFfmpegNvidiaGPU) cmdEncodingArgs << "-c:v libx264 -preset veryslow -qp 0 ";

				// template 2: (Nvidia GPU)
				else if (bUseFfmpegNvidiaGPU)
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
				// append
				cmd << cmdEncodingArgs;

				// 3. append file output
				cmd << fileOut.str().c_str();

				//-

				cout << endl << endl;
				cout << "> HARDCODED FFmpeg SCRIPT" << endl << endl;
				cout << "> ffmpeg.exe : " << endl << ffmpeg.str().c_str();
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
			cout << "> Done/Trying video encoding into: " << endl << fileOut.str().c_str();
			cout << endl << endl;
			cout << "> WARNING: Close your videoplayer to unblock allow close the app !";
			cout << endl << endl;
			cout << "--------------------------------------------------------------" << endl << endl;

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

			cout << "> WARNING: If output video is not opened NOW, and getting above an error like:" << endl;
			cout << "\t'is not recognized as an internal or external command, operable program or batch file.'" << endl;
			cout << "> Then probably you need to set your ofApp.exe settings to run as Administrator:" << endl;
			cout << "> Use Windows File Explorer file properties / Change settings for all users / compatibility." << endl;
			cout << "> This is to allow run ffmpeg.exe and access to files from here!" << endl;

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
		}
	}
};