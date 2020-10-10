ofxSurfingCapturer
=============================

# Overview
**ofxSurfingCapturer** is an **openFrameworks** addon to do *fast video capturing* but storing **still frames** to join to video with *FFmpeg*. It helps on all the capture workflow. 

# Two alternatives
Includes **two** different **independent** classes:  

- 1. **ofxSurfing_CaptureWindowStills.h**  
The recommended option: It's much faster bc captures still frames.  
Based on: **ofxTextureRecorder**.  

- 2. **ofxSurfing_CaptureWindowFFMPEG.h**  
Less recommended option: It encodes to video live, so it's slower in some machines.  
Based on: **ofxFFmpegRecorder** and **ofxFastFboReader**.

## Screenshots

### 1. example-BasicStills:
Uses ofxSurfing_CaptureWindowStills.h  

### HOW TO / WORKFLOW 
1. **F8** : Mounts/prepare the capturer...  
![image](/readme_images/Capture1.PNG?raw=true "image") 

2. **F9** : START Recording!
![image](/readme_images/Capture2.PNG?raw=true "image")  

3. **F9** : STOP Recording.  

4. **F11** : Run the FFmpeg script to join all the still frames (.tif -> .mp4).  

5. Your videoplayer will auto-start opening the new created video!  
![image](/readme_images/Capture3.PNG?raw=true "image")

6. *Ctrl+Alt+BackSpace*: Remove all still files.  

### Data path and "ffmpeg.exe":
![image](/readme_images/Capture5.PNG?raw=true "image")
![image](/readme_images/Capture6.PNG?raw=true "image")

### 2. example-BasicVideo: realtime encoding
Uses ofxSurfing_CaptureWindowFFMPEG.h  
(with realtime capture and FFmpeg video encoding)  
![image](/readme_images/Capture4.PNG?raw=true "image")

## Features
- **Faster** than other alternatives that capture vide (not "raw" still frames).
- **User controls** to handle all the capture workflow:  
1. **Mount** (F8)  
2. **Record** (F9)  
3. Take **PNG Snapshot** (F10)  
4. **Clear** all stills (Ctrl+Alt+BackSpace)
5. **Auto-call batch FFmpeg** *stills_to_video* compression after capture (F11),  
auto-opens video with your video player.

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

	///----- draw your scene here -----///

	capturer.end();

	capturer.draw();
	capturer.drawInfo();
}
```

## Dependencies
1. **ofxSurfing_CaptureWindowStills**:  
https://github.com/moebiussurfing/ofxTextureRecorder  
forked from **arturoc/ofxTextureRecorder**

2. **ofxSurfing_CaptureWindowFFMPEG**:  
https://github.com/gallagher-tech/ofxFFmpegRecorder.git  
https://github.com/NickHardeman/ofxFastFboReader.git  

## Notes
- Includes some **FFmpeg** scripts, links and a Windows compiled *ffmpeg.exe*.
- To batch-join stills (.tif) to video (.mp4) requires your own **ffmpeg.exe** binary.
- TODO: Should improve data path...
- TODO: Check I window resize don't breaks Fbo capturer size... 

## Tested systems
- **Windows10** / **VS2017** / **OF 0.11**

## Author
Addon by **@moebiusSurfing**  
*(ManuMolina). 2020.*  

Thanks to the coders of the above original addons:  
**NickHardeman** and **arturoc**.  

## License
*MIT License.*