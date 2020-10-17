#pragma once

#include "ofMain.h"

//#include "ofxSurfingConstants.h"

#ifdef USE_IM_GUI
#include "ofxSurfing_ImGui.h"
#endif

#ifdef USE_ofxGui
#include "ofxSurfing_ofxGui.h"
#endif
//
//#define USE_JSON		// uncomment to use default xml instead json for ofParameterGroup de/serializers
//#define USE_XML		// uncomment to use default xml instead json for ofParameterGroup de/serializers
//
//---------

namespace ofxSurfingHelpers2 {
	//using namespace ofxSurfingHelpers2;

	//-

	////--------------------------------------------------------------
	//// circular progress bar
	////
	//// example:
	////float val = ofMap(ofGetFrameNum() % 200, 0, 200, 0.f, 1.f, true);
	////ofxSurfingHelpers::drawCircleProg(val);
	////--------------------------------------------------------------
	//inline void drawCircleProg(float val) {
	//	ofPushMatrix();
	//	ofPushStyle();
	//	ofSetLineWidth(5);

	//	ofRotateXDeg(-90);

	//	float radius = 50;
	//	float ratio = 0.7;

	//	//float val = 0.5;
	//	//float progress = val / 100;

	//	ofPoint point1(150, 120);

	//	ofNoFill();

	//	ofDrawRectangle(point1.x, point1.y, 100, 100);
	//	//int startAngle = -90 * 16;
	//	//int spanAngle = val * 360 * 16;		
	//	int startAngle = -90;
	//	int spanAngle = -90 + val * 360;
	//	//int spanAngle = progress * 360 * 16;

	//	//p.drawArc(rectangle, startAngle, spanAngle);
	//	//void ofPolyline_::arc(float x, float y, float radiusX, float radiusY, float angleBegin, float angleEnd, int circleResolution=20)
	//	//polyline1.arc(point1, 100, 100, 0, 360);

	//	ofPolyline polyline1;
	//	polyline1.lineTo(point1.x, point1.y + radius * ratio);
	//	polyline1.lineTo(point1.x, point1.y + radius);
	//	polyline1.arc(point1, radius, radius, startAngle, spanAngle);
	//	polyline1.arc(point1, radius*ratio, radius*ratio, startAngle, spanAngle);
	//	//polyline1.lineTo(0, point1.y + radius * ratio);
	//	//polyline1.lineTo(0, point1.y + radius);
	//	ofSetColor(ofColor::blue);
	//	polyline1.draw();

	//	string str = "prog" + ofToString(val * 100);
	//	//string str = "prog" + ofToString(progress * 100);
	//	ofDrawBitmapStringHighlight(str, point1.x, point1.y + 100);

	//	ofPopStyle();
	//	ofPopMatrix();
	//}

	//---

	// xml
#ifndef USE_JSON
	//--------------------------------------------------------------
	inline bool loadGroup(ofParameterGroup &g, string path)
	{
		ofLogVerbose(__FUNCTION__) << g.getName() << " to " << path;
		ofLogVerbose(__FUNCTION__) << "parameters: \n" << g.toString();

		ofXml settings;
		bool b = settings.load(path);

		if (b) ofLogVerbose(__FUNCTION__) << "Loading: " << g.getName() << " at " << path;
		else ofLogError(__FUNCTION__) << "Error loading: " << g.getName() << " at " << path;

		ofDeserialize(settings, g);

		return b;
	}

	//--------------------------------------------------------------
	inline bool saveGroup(ofParameterGroup &g, string path)
	{
		ofLogVerbose(__FUNCTION__) << g.getName() << " to " << path;
		ofLogVerbose(__FUNCTION__) << "parameters: \n" << g.toString();

		//CheckFolder(path);

		ofXml settings;
		ofSerialize(settings, g);
		bool b = settings.save(path);

		if (b) ofLogVerbose(__FUNCTION__) << "Save: " << g.getName() << " at " << path;
		else ofLogError(__FUNCTION__) << "Error saving: " << g.getName() << " at " << path;
		return b;
	}
#endif

	//----

#ifdef USE_JSON
	//--------------------------------------------------------------
	inline bool loadGroup(ofParameterGroup &g, string path, bool debug = true)
	{
		if (debug) {
			ofLogNotice(__FUNCTION__) << g.getName() << " to " << path;
			ofLogNotice(__FUNCTION__) << "parameters: \n" << g.toString();
		}
		else
		{
			ofLogVerbose(__FUNCTION__) << g.getName() << " to " << path;
			ofLogVerbose(__FUNCTION__) << "parameters: \n" << g.toString();
		}

		ofJson settings;
		settings = ofLoadJson(path);
		ofDeserialize(settings, g);
		bool b = !settings.is_null();//TODO:
		//bool b = true;//TODO:

		//bool b = settings.is_null;
		//if (b) ofLogVerbose(__FUNCTION__) << "Load: " << g.getName() << " at " << path;
		//else ofLogError(__FUNCTION__) << "Error loading: " << g.getName() << " at " << path;

		return b;
	}

	//--------------------------------------------------------------
	inline bool saveGroup(ofParameterGroup &g, string path, bool debug = true)
	{
		if (debug) {
			ofLogNotice(__FUNCTION__) << g.getName() << " to " << path;
			ofLogNotice(__FUNCTION__) << "parameters: \n" << g.toString();
		}
		else
		{
			ofLogVerbose(__FUNCTION__) << g.getName() << " to " << path;
			ofLogVerbose(__FUNCTION__) << "parameters: \n" << g.toString();
		}

		//ofxSurfingHelpers::CheckFolder(path);

		ofJson settings;
		ofSerialize(settings, g);
		bool b = ofSavePrettyJson(path, settings);

		if (b) ofLogVerbose(__FUNCTION__) << "Save: " << g.getName() << " at " << path;
		else ofLogError(__FUNCTION__) << "Error saving: " << g.getName() << " at " << path;

		return b;
	}
#endif

	//--------------------------------------------------------------
	// check if a folder path exist and creates one if not
	// why? many times when you try to save a file, this is not possible and do not happens bc the container folder do not exist
	//--------------------------------------------------------------
	inline void CheckFolder(string _path)
	{
		//ofLogNotice(__FUNCTION__) << _path;

		//// workaround to avoid error when folders are folder/subfolder
		//auto _fullPath = ofSplitString(_path, "/");
		//for (int i = 0; i < _fullPath.size(); i++) {
		//	ofLogNotice(__FUNCTION__) << ofToString(i) << " " << _fullPath[i];
		//}

		ofDirectory dataDirectory(ofToDataPath(_path, true));// /bin/data/

		// check if folder path exist
		if (!dataDirectory.isDirectory())
		{
			ofLogError(__FUNCTION__) << "FOLDER NOT FOUND! TRYING TO CREATE...";

			// try to create folder
			//bool b = dataDirectory.createDirectory(ofToDataPath(_path, true));
			bool b = dataDirectory.createDirectory(ofToDataPath(_path, true), false, true);
			// added enable recursive to allow create nested subfolders if required

			// debug if creation has been succeded
			if (b) ofLogNotice(__FUNCTION__) << "CREATED '" << _path << "' SUCCESSFULLY!";
			else ofLogError(__FUNCTION__) << "UNABLE TO CREATE '" << _path << "' FOLDER!";
		}
		else
		{
			ofLogNotice(__FUNCTION__) << _path << " OK!";// nothing to do
		}
	}


	///* RETURN FILE EXTENSION */
	//inline string getFileName(string _str, bool _trimExt = false) {
	//	string name = "";
	//	int slashPos = _str.rfind('/');

	//	if (slashPos != string::npos) {
	//		name = _str.substr(slashPos + 1);

	//		if (_trimExt)
	//			name = getFilePathTrimExtension(name);
	//	}

	//	return name;
	//}

	//inline string getFileFolderPath(string _str) {
	//	string name = "";
	//	int slashPos = _str.rfind('/');

	//	if (slashPos != string::npos) {
	//		name = _str.substr(0, slashPos);
	//	}

	//	return name;
	//}

	//inline string getFileExtension(string _str) {
	//	string ext = "";
	//	int extPos = _str.rfind('.');

	//	if (extPos != string::npos) {
	//		ext = _str.substr(extPos + 1);
	//	}

	//	return ext;
	//}

	//inline string getFilePathTrimExtension(string _str) {
	//	string ext = "";
	//	int extPos = _str.rfind('.');

	//	if (extPos != string::npos) {
	//		ext = _str.substr(0, extPos);
	//	}

	//	return ext;
	//}

	//inline int getNumWords(std::string str)
	//{
	//	int word_count(0);
	//	std::stringstream ss(str);
	//	std::string word;
	//	while (ss >> word) ++word_count;
	//	return word_count;
	//}


	//----

	//--------------------------------------------------------------
	// draws a box with text
	//--------------------------------------------------------------
	inline void drawTextBoxed(ofTrueTypeFont &font, string text, int x, int y, ofColor font0_Color = 255, ofColor colorBackground = ofColor{ 0, 200 }, bool useShadow = false, ofColor colorShadow = 128)
	{
		int _pad = 50;
		float _round = 5;

		ofPushStyle();
		//float fontSize = font.getSize();

		if (!font.isLoaded()) {
			ofDrawBitmapStringHighlight(text, x, y);
		}
		else {
			// bbox
			ofSetColor(colorBackground);
			ofFill();

			ofRectangle _r(font.getStringBoundingBox(text, x, y));
			_r.setWidth(_r.getWidth() + _pad);
			_r.setHeight(_r.getHeight() + _pad);
			_r.setX(_r.getPosition().x - _pad / 2.);
			_r.setY(_r.getPosition().y - _pad / 2.);

			ofDrawRectRounded(_r, _round);
			//ofDrawRectangle(_r);

			// text shadow
			if (useShadow) {
				ofSetColor(colorShadow);
				font.drawString(text, x + 1, y + 1);
			}

			// text
			ofSetColor(font0_Color);
			font.drawString(text, x, y);
		}

		ofPopStyle();
	}
	//--------------------------------------------------------------
	// get box width
	//--------------------------------------------------------------
	inline float getWidthBBtextBoxed(ofTrueTypeFont &font, string text) {
		return (font.getStringBoundingBox(text, 0, 0)).getWidth();
	}
	inline float getHeightBBtextBoxed(ofTrueTypeFont &font, string text) {
		return (font.getStringBoundingBox(text, 0, 0)).getHeight();
	}

};// ofxSurfingHelpers2