ofxAddonTemplateCustomApp
=============================

# Overview
**ofxAddonTemplateCustomApp** is an **openFrameworks** addon template for MYSELF.

## Screenshot
![image](/readme_images/Capture1.PNG?raw=true "image")

## Features
- Customize gui.
- Basic parameters and app modes.
- Store/Recall settings.
- 


## Usage
 
### ofApp.h
```.cpp
#include "ofxAddonTemplateCustomApp.h"
ofxAddonTemplateCustomApp myAddon;
```

### ofApp.cpp
```.cpp
ofApp::setup(){
	ofxAddonTemplateCustomApp.setup();
}

ofApp::update(){
	ofxAddonTemplateCustomApp.update();
}

ofApp::draw(){
	ofxAddonTemplateCustomApp.draw();
	ofxAddonTemplateCustomApp.drawGui();
}
```

## Dependencies
- 

## Tested systems
- **Windows10** / **VS2017** / **OF 0.11**

### TODO/IDEAS
* 

### Notes
*

## Author
Addon by **@moebiusSurfing**  
*(ManuMolina). 2020.*

## License
*MIT License.*