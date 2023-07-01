#pragma once
#include "ofMain.h"

/*

This addon is hardly based on https://github.com/tettou771/ofxDuckDuckGo

*/

//TODO:
//#include <curl/curl.h>

class ofxSerpAPI {
public:
	enum ErrorCode {
		Success,
		BadRequest,
		Unauthorized,
		Forbidden,
		NotFound,
		RequestTimeout,
		InternalServerError,
		ServiceUnavailable,
		UnknownError
	};

	void search(const string& query, const string& engine, function<void(const ofJson&, ErrorCode)> callback);
private:
	void onResponse(ofHttpResponse& response);
	function<void(const ofJson&, ErrorCode)> searchCallback;
	string uriEncode(const string& str);
	ErrorCode parseErrorResponse(const ofHttpResponse& response);

public:
	ofxSerpAPI();
	~ofxSerpAPI();

	static string getErrorMessage(ErrorCode errorCode);

	string sResponse;
	ofJson jResponse;

	// Curl approach
	string sResponseCurl;
	static size_t WriteCallback(char* contents, size_t size, size_t nmemb, std::string* response);
	void doCurl(const std::string& query, const std::string& engine);

	ofParameterGroup params{ "ofxSerpAPI" };
	ofParameter<string> sKey{ "Key", "SET-YOUR-SERP-API-KEY-HERE-OR-IN-JSON-FILE"};
	ofParameter<string> sEngine = { "Engine","youtube" };

	bool bDoneCurl = false;
public:
	bool isCurlDone() {
		if (bDoneCurl)
		{
			bDoneCurl = 0;
			return true;
		}
		return false;
	}
};
