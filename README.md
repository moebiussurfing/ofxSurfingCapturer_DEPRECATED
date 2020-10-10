ofxSurfingCapturer
=============================

# Overview
**ofxSurfingCapturer** is an **openFrameworks** addon to do fast video capturing but storing still frames to join to video with ffmpeg.  

# Two alternatives
Includes **two** different **independent** classes:  

- **ofxSurfing_CaptureWindowStills.h**  
The recommended option: It's much faster bc captures still frames.  
Based on: **ofxTextureRecorder**.  

- **ofxSurfing_CaptureWindowFFMPEG.h**  
Less recommended option: It lives encodes to video, so it's slower in some machines.  
Based on: **ofxFFmpegRecorder** and **ofxFastFboReader**.

## Screenshots

### example-BasicStills:
Uses ofxSurfing_CaptureWindowStills.h  
![image](/readme_images/Capture1.PNG?raw=true "image")
![image](/readme_images/Capture2.PNG?raw=true "image")
![image](/readme_images/Capture3.PNG?raw=true "image")

### example-BasicVideo:
Uses ofxSurfing_CaptureWindowFFMPEG.h  

## Features
- **Faster** than other alternatives that capture video.
- **User controls** to handle all the proccess:  
mount, record, take snapshot, clear all stills, ...etc.
- Command to **auto-call ffmpeg** video compression after capture.

## Usage
 
### ofApp.h
```.cpp
#include "ofxSurfing_CaptureWindowStills.h"
CaptureWindow capturer;
```

### ofApp.cpp
```.cpp
ofApp::setup(){
	capturer.setup();
	capturer.setActive(true);// make visible
}

ofApp::draw(){
	capturer.begin();
	// draw your scene here //
	capturer.end();

	capturer.draw();
	capturer.drawInfo();
}
```

## Dependencies
**ofxSurfing_CaptureWindowStills**:  
https://github.com/moebiussurfing/ofxTextureRecorder  
forked from **arturoc/ofxTextureRecorder**

**ofxSurfing_CaptureWindowFFMPEG**:  
https://github.com/gallagher-tech/ofxFFmpegRecorder.git  
https://github.com/NickHardeman/ofxFastFboReader.git  

## Tested systems
- **Windows10** / **VS2017** / **OF 0.11**

## Author
Addon by **@moebiusSurfing**  
*(ManuMolina). 2020.*

## License
*MIT License.*