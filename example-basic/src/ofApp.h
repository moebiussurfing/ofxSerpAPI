#pragma once


/*

https://serpapi.com/
https://serpapi.com/youtube-search-api

*/


#include "ofMain.h"

#include "ofxSerpAPI.h"

#include "ofxSurfingImGui.h"

#ifdef USE_EDITOR
#include "SurfingTextEditor.h"
#endif

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void drawUI();
	void doClear();
	void doParse();

	void onSearchHTTPResult(const ofJson& result, ofxSerpAPI::ErrorCode errorCode);

	ofxSerpAPI searchAPI;
	ofJson jResponse;
	string sResponse;
	void doSearchHTTP(const std::string& query, const std::string& engine);

	ofxSurfingGui ui;

#ifdef USE_EDITOR
	SurfingTextEditor e;
#endif

	ofParameter<string> sEngine = { "Engine","youtube" };
	ofParameter<string> sQuery = { "Query","autechre" };

	void AddToLog(string s, ofLogLevel l = OF_LOG_NOTICE);

	struct dataVideo {
		string name;
		string url;
		string thumb;
		ofImage img;
		ofTexture tex;
		GLuint imgID;
		GLuint texID;
	};
	vector<dataVideo> videos;
};
