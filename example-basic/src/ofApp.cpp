#include "ofApp.h"

//https://serpapi.com/duckduckgo-search-api
//https://help.duckduckgo.com/duckduckgo-help-pages/settings/params/

void ofApp::setup() {
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetWindowTitle("ofxSerpAPI");

	ui.setup();
#ifdef USE_EDITOR
	e.setup("myEditor");
	//e.addKeyword("JSON");
#endif
}

void ofApp::update() {
	if (ofGetFrameNum() == 2) {
		ui.notifier.setDuration(500);
		//ui.notifier.setMini;
	}
}

void ofApp::draw() {
	//sResult = duckDuckGo.sResult;
	//ofDrawBitmapString(sResult, 20, 20);

	drawUI();
}

void ofApp::drawUI() {
	ui.Begin();

#ifdef USE_EDITOR
	IMGUI_SUGAR__WINDOWS_CONSTRAINTS_DEFAULT;
	e.drawImGui();
#endif

	IMGUI_SUGAR__WINDOWS_CONSTRAINTS_DEFAULT;
	if (ui.BeginWindow("ofApp", ImGuiWindowFlags_None))
	{
		ui.AddLogToggle();
		ui.AddNotifierToggle();
		ui.AddMinimizerToggle();
		ui.AddSpacingBigSeparated();

		ui.AddLabelBig("QUERY");
		if (ui.Add(sQuery, OFX_IM_TEXT_INPUT_NAMED)) {
			AddToLog(sQuery.get());
		};

		if (ui.Add(sEngine, OFX_IM_TEXT_INPUT_NAMED)) {
		};
		ui.AddSpacingBig();

		if (ImGui::Button("Clear")) {
			doClear();
		}
		ui.AddSpacingBigSeparated();

		ui.AddLabelBig("SEARCH");
		if (ImGui::Button("doHTTP")) {
			doSearchHTTP(sQuery.get(), sEngine.get());
		}
		ui.SameLine();
		if (ImGui::Button("doCurl")) {
			duckDuckGo.doCurl(sQuery.get(), sEngine.get());
		}
		if (!jResponse.empty()) {
			ui.AddSpacing();
			ui.AddLabelBig("FILTER");
			if (ImGui::Button("doParse")) {
				doParse();
			}
		}

		ui.AddSpacingBigSeparated();

		ui.AddLabel(sResult);

		ui.EndWindow();
	}

	ui.End();
}

void ofApp::doClear()
{
	sResult = "";
	AddToLog(__FUNCTION__, OF_LOG_WARNING);

}

// Send query
void ofApp::doSearchHTTP(const std::string& query, const std::string& engine)
{
	duckDuckGo.search(query, engine, [&](const ofJson& result, ofxSerpAPI::ErrorCode errorCode) {

		onSearchHTTPResult(result, errorCode);
		sResult = result.dump(4);

		string ss = "doSearchHTTP Result: " + sResult;
		ofLogNotice() << ss;
		AddToLog(ss, OF_LOG_WARNING);
		});
}

// Callback response
void ofApp::onSearchHTTPResult(const ofJson& result, ofxSerpAPI::ErrorCode errorCode) {
	if (errorCode == ofxSerpAPI::Success) {
		this->jResponse = result;

		sResult = result.dump(4);
		string s = "onSearchHTTPResult Result: " + sResult;
		ofLogNotice() << s;
		AddToLog(s, OF_LOG_WARNING);

#ifdef USE_EDITOR
		e.setText(sResult);
#endif
	}
	else {
		string s = "Error: " + duckDuckGo.getErrorMessage(errorCode);
		ofLogError() << s;
		AddToLog(s, OF_LOG_ERROR);
	}
	ofLogNotice() << "Result: " << result.dump(4);
}

// Extracting information from the JSON
void ofApp::doParse() {
	ofJson data = jResponse;

	if (data.empty()) {
		std::cout << "The JSON data is empty." << std::endl;
	}
	else {
		int i = 0;
		ofJson videoResults = data["video_results"];
		for (const auto& video : videoResults) {
			std::string videoTitle = video["title"];
			std::string videoLink = video["link"];
			std::string channelName = video["channel"]["name"];
			std::string channelLink = video["channel"]["link"];

			stringstream ss;
			//ss << endl;
			ss << "VIDEO #" << i++ << endl;
			ss << "Title: \t" << videoTitle << std::endl;
			ss << "Link:  \t" << videoLink << std::endl;
			//ss << "Channel Name: " << channelName << std::endl;
			//ss << "Channel Link: " << channelLink << std::endl;

			AddToLog(ss.str());
		}
	}
}

void ofApp::AddToLog(string s, ofLogLevel l) {
	ui.AddToLogAndNotifier(s, l);
}

