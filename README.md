ofxSurfingCapturer
=============================

# Overview
**ofxSurfingCapturer** is an **openFrameworks** addon to do fast video capturing but storing still frames to join to video with ffmpeg.
Includes two different independent classes:  
**ofxSurfing_CaptureWindowStills.h** -> Recommended option: It's much faster bc captures still frames.  
based on: **ofxTextureRecorder**.  

**ofxSurfing_CaptureWindowFFMPEG.h** -> Less recommended option: It lives encodes to video, so it's slower in some machines.  
based on: **ofxFFmpegRecorder** and **ofxFastFboReader**.

## Screenshots

### example-BasicStills:
Uses ofxSurfing_CaptureWindowStills.h  
![image](/readme_images/Capture1.PNG?raw=true "image")
![image](/readme_images/Capture2.PNG?raw=true "image")
![image](/readme_images/Capture3.PNG?raw=true "image")

### example-BasicVideo:
Uses ofxSurfing_CaptureWindowFFMPEG.h  

## Features
- Faster than other alternatives that capture video.
- User controls to handle all the proccess: mount, record, clear all stills...etc
- Command to auto call ffmpeg compression after capture.

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
- https://github.com/moebiussurfing/ofxTextureRecorder  
forked from arturoc/ofxTextureRecorder

## Tested systems
- **Windows10** / **VS2017** / **OF 0.11**

## TODO/IDEAS
* 

## Notes
*

## Author
Addon by **@moebiusSurfing**  
*(ManuMolina). 2020.*

## License
*MIT License.*