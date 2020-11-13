ofxSurfingCapturer
====================

# Overview
**ofxSurfingCapturer** is an **openFrameworks** addon to do *fast video capturing* but storing **still frames** to join to video with *FFmpeg* after capture finished. It helps on all the capture workflow. 

Includes the main class:  

**ofxSurfing_CaptureWindowStills.h**  
It's much faster (in some machines) because captures and compress still frames (.tif) using threading, and it makes the FFmpeg video (.mp4) encoding using system command-line after all: inisde the OF App or using your favourite external video encoder.  
Based on: **ofxTextureRecorder**.  

## Screenshots

### example-BasicStills:
Uses ofxSurfing_CaptureWindowStills.h  

### HOW TO / WORKFLOW 
1. **F8** : Mounts/prepare the capturer...  

2. **F9** : START Recording!

![image](/readme_images/Capture2.PNG?raw=true "image")  

3. **F9** : STOP Recording.  

4. **F11** : RUN the FFmpeg script to join all the still frames (`xxxxx.tif` -> `output.mp4`).  

5. Your videoplayer will auto-start opening the new created video!  

![image](/readme_images/Capture3.PNG?raw=true "image")

6. *Ctrl+Alt+BackSpace*: Remove all still files.  

### Data path structure and "ffmpeg.exe" location:

![image](/readme_images/Capture5.PNG?raw=true "image")
![image](/readme_images/Capture6.PNG?raw=true "image")

## Features
- **Faster** than other live-capture-to-video alternatives (bc "raw" still frames + post-encode after).
- **GPU Hardware accelerated** (or CPU) video encoding. (Nvidia only yet) 
- Selectable image format: *png*, *jpg*, ...etc. *tif* by default.  
- **Key commands** to handle all the workflow:  
1. **Mount** (F8)  
2. **Record** (F9)  
3. **Take PNG Snapshot** (F10)  
4. **Clear** all stills (Ctrl+Alt+BackSpace)
5. **Auto-call batch FFmpeg** *stills_to_video* compression after capture finished (F11),  
auto-opens video with your videoplayer.

## Usage
 
### ofApp.h
```.cpp
#include "ofxSurfing_CaptureWindowStills.h"
CaptureWindow capturer;
```

### ofApp.cpp
```.cpp
ofApp::setup(){
	// disable depth to avoid some fbo problems/bugs when using 2D scenes.
	//capturer.setDephEnabled(false);

	// to capture a section only. call before setup
	//capturer.setCustomizeSection(ofRectangle{ 0, 0, canvasSize.get().x, canvasSize.get().y });
	
	//capturer.setup("Captures\\Captures_Projector_1\\", OF_IMAGE_FORMAT_TIFF);
	capturer.setup();

	// add enabler into your gui
	//gui.add(capturer.params);
}

ofApp::draw(){
	capturer.begin();

	///----- draw your scene here -----///

	capturer.end();
	capturer.draw();

	capturer.drawInfo();
	//gui.draw();
}
```

## Dependencies
- To use **ofxSurfing_CaptureWindowStills**:  
https://github.com/moebiussurfing/ofxTextureRecorder  
forked from **arturoc/ofxTextureRecorder**.  
Already included into addon `/libs`. You don't need to add into *Project Generator*.  

## Notes
- Includes some **FFmpeg** scripts, links and a Windows `ffmpeg.exe` and macOS `ffmpeg` binary.
- Video encoding, batch-join stills (xxxxx.tif) to video (output.mp4) requires `ffmpeg` binary.
- `ofxSurfing_CaptureWindowStills.h` could work on **macOS** and **Linux** too, bc only relays into `ofxTextureRecorder`.
- *TODO*: Check if window resize don't breaks Fbo capturer size...
- FFmpeg encode using `AMD GPU` could work because now you can customize the script on setup.

## Tested systems
- **Windows10** / **VS2017** / **OF +0.11**
- **macOS High Sierra** / **Xcode 10** / **OF +0.11**

## Author
Addon by **@moebiusSurfing**  
*(ManuMolina). 2020.*  

Thanks to the coders of the above original addons:  
**arturoc**.  

## License
*MIT License.*