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

	// title to debug that threading is working
	const int d = 120;
	if (ofGetFrameNum() % d < (d / 0.5)) {
		ofSetWindowTitle("ofxSerpAPI");
	}
	else ofSetWindowTitle("         ");

	if (ofGetFrameNum() == 2) {
		ui.notifier.setDuration(1000);
		ui.notifier.setMini();
	}

	//TODO: 
	// Make callback
	// Curl
	if (searchAPI.isCurlDone()) {
		jResponse = searchAPI.jResponse;
		sResponse = searchAPI.sResponse;

		string s = "onSearchCurl Result: " + sResponse;
		ofLogNotice() << s;
		ui.AddToLog(s, OF_LOG_WARNING);
	}
}

void ofApp::draw() {
	//sResponse = searchAPI.sResponse;
	//ofDrawBitmapString(sResponse, 20, 20);

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
		ui.AddMinimizerToggle();
		ui.AddLogToggle();
		ui.AddNotifierToggle();
		ui.AddAutoResizeToggle();
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
			searchAPI.doCurl(sQuery.get(), sEngine.get());
		}

		if (!jResponse.empty())
		{
			ui.AddSpacing();
			ui.AddLabelBig("FILTER");
			if (ImGui::Button("doParse")) {
				doParse();
			}
		}

		ui.AddSpacingBigSeparated();

		ui.AddLabel(sResponse);

		ui.EndWindow();
	}

	if (videos.size() > 0)
	{
		IMGUI_SUGAR__WINDOWS_CONSTRAINTS_DEFAULT;
		if (ui.BeginWindow("VIDEOS"))
		{
			for (size_t i = 0; i < videos.size(); i++)
			{
				string s = videos[i].name;
				s += "\n";
				s += videos[i].url;

				float w, h;
				w = 200;
				h = ui.getWidgetsHeightUnit() * 4;
				ImVec2 sz{ w, h };

				if (videos[i].tex.isAllocated())
				{
					if (ImGui::ImageButton(ofxImGuiSurfing::GetImTextureID2(videos[i].texID), sz))
					{
						ofLaunchBrowser(videos[i].url);
					}

					ui.PushFontStyle(OFX_IM_FONT_BIG);
					ui.AddTooltipBlink(s);
					ui.PopFontStyle();
				}
			}

			ui.EndWindow();
		}
	}

	ui.End();
}

void ofApp::doClear()
{
	sResponse = "";
	AddToLog(__FUNCTION__, OF_LOG_WARNING);
	ui.ClearLog();
	videos.clear();
}

// Send query
void ofApp::doSearchHTTP(const std::string& query, const std::string& engine)
{
	searchAPI.search(query, engine, [&](const ofJson& result, ofxSerpAPI::ErrorCode errorCode) {

		onSearchHTTPResult(result, errorCode);
		sResponse = result.dump(4);

		string ss = "doSearchHTTP Result: " + sResponse;
		ofLogNotice() << ss;
		ui.AddToLog(ss, OF_LOG_WARNING);

		videos.clear();
		});
}

// Callback response
void ofApp::onSearchHTTPResult(const ofJson& result, ofxSerpAPI::ErrorCode errorCode) {
	if (errorCode == ofxSerpAPI::Success) {
		this->jResponse = result;

		sResponse = result.dump(4);
		string s = "onSearchHTTPResult Result: " + sResponse;
		ofLogNotice() << s;
		ui.AddToLog(s, OF_LOG_WARNING);

#ifdef USE_EDITOR
		e.setText(sResponse);
#endif
	}
	else {
		string s = "Error: " + searchAPI.getErrorMessage(errorCode);
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
		videos.clear();

		ofJson videoResults = data["video_results"];
		for (const auto& video : videoResults)
		{
			std::string videoTitle = video["title"];
			std::string videoLink = video["link"];
			std::string channelName = video["channel"]["name"];
			std::string channelLink = video["channel"]["link"];
			std::string thumbnailLink = video["thumbnail"]["static"];

			stringstream ss;
			ss << "VIDEO #" << i++ << endl;
			ss << "Title: \t" << videoTitle << std::endl;
			ss << "Link:  \t" << videoLink << std::endl;
			ss << "Thumb:  \t" << thumbnailLink << std::endl;
			//ss << "Channel Name: " << channelName << std::endl;
			//ss << "Channel Link: " << channelLink << std::endl;
			//ss << endl;

			//--

			dataVideo v;
			v.name = videoTitle;
			v.url = videoLink;
			v.thumb = thumbnailLink;
			ofHttpResponse r = ofLoadURL(thumbnailLink);
			if (r.status == 200) {
				v.texID = ui.getGuiPtr()->loadTexture(v.tex, v.thumb);

			}
			videos.emplace_back(v);

			//--

			AddToLog(ss.str());
		}
	}
}

void ofApp::AddToLog(string s, ofLogLevel l) {
	ui.AddToLogAndNotifier(s, l);
}

