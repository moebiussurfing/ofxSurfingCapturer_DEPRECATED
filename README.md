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
Less recommended option: It encodes to video live, so it's slower in some machines.  
Based on: **ofxFFmpegRecorder** and **ofxFastFboReader**.

## Screenshots

### example-BasicStills:
Uses ofxSurfing_CaptureWindowStills.h  
![image](/readme_images/Capture1.PNG?raw=true "image")
![image](/readme_images/Capture2.PNG?raw=true "image")
![image](/readme_images/Capture3.PNG?raw=true "image")

### example-BasicVideo: realtime encoding
Uses ofxSurfing_CaptureWindowFFMPEG.h  
![image](/readme_images/Capture4.PNG?raw=true "image")

### Data path and "ffmpeg.exe":
![image](/readme_images/Capture5.PNG?raw=true "image")
![image](/readme_images/Capture6.PNG?raw=true "image")

## Features
- **Faster** than other alternatives that capture video.
- **User controls** to handle all the proccess:  
Mount (F8)  
Record (F9)  
Take snapshot (F10)  
Clear all stills (Ctrl+Alt+BackSpace)
- Command (F11) to **auto-call ffmpeg** batch stills_to_video compression after capture and opens the video player.

## Usage
 
### ofApp.h
```.cpp
#include "ofxSurfing_CaptureWindowStills.h"
CaptureWindow capturer;
```

### ofApp.cpp
```.cpp
ofApp::setup(){
	capturer.setPathRoot("F:\\openFrameworks\\addons\\ofxSurfingCapturer\\example-BasicStills\\bin\\data\\");
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

## Notes
- Includes some **ffmpeg** scripts and links.
- To batch-join stills to video requires **ffmpeg.exe**.
- TODO: Should improve data path...

## Tested systems
- **Windows10** / **VS2017** / **OF 0.11**

## Author
Addon by **@moebiusSurfing**  
*(ManuMolina). 2020.*

## License
*MIT License.*