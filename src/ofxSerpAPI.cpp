#include "ofxSerpAPI.h"
ofxSerpAPI::ofxSerpAPI() {
	params.add(sKey);

	string path = params.getName() + ".json";
	ofJson settings;
	settings = ofLoadJson(path);
	ofDeserialize(settings, params);
	ofLogWarning() << "Loaded: " << path;
}

ofxSerpAPI::~ofxSerpAPI() {
	string path = params.getName() + ".json";
	if (!ofDirectory::doesDirectoryExist(ofFilePath::getEnclosingDirectory(path))) {
		ofFilePath::createEnclosingDirectory(path);
		ofLogWarning() << "Created enclosing folder for: " << path;
	}
	ofJson settings;
	ofSerialize(settings, params);
	ofSavePrettyJson(path, settings);
}

void ofxSerpAPI::search(const std::string& query, const std::string& engine, std::function<void(const ofJson&, ErrorCode)> callback) {
	searchCallback = callback;

	// DuckDuckGo
	//https://github.com/tettou771/ofxDuckDuckGo
	//string url = "https://api.duckduckgo.com/?q=" + uriEncode(query) + "&format=json";

	// SERP API
	//https://serpapi.com/search.json?engine=youtube&search_query=autechre&api_key=533fe6a1d8582a75eeb08f5126291b0807b8e9d7610e55de33f01e26310abdd5

	//string sEngine = "youtube"; // " + sEngine + "
	//string sQuery = "autechre"; // " + sQuery + "

	string sEngine = engine;
	string sQuery = query;

	string sUrl = "https://serpapi.com/search.json?engine=" + sEngine + "&search_query=" + sQuery + "&api_key=" + sKey.get();

	ofAddListener(ofURLResponseEvent(), this, &ofxSerpAPI::onResponse);
	ofLoadURLAsync(sUrl, "search_request");
}

void ofxSerpAPI::onResponse(ofHttpResponse& response) {
	if (response.request.name != "search_request") return;
	ofRemoveListener(ofURLResponseEvent(), this, &ofxSerpAPI::onResponse);

	ErrorCode errorCode = parseErrorResponse(response);
	if (errorCode == Success) {
		ofJson jsonData = ofJson::parse(response.data.getText());
		sResult = jsonData.dump(4);
		ofLogNotice("ofxSerpAPI") << "Response JSON: " << sResult;
		if (searchCallback) {
			searchCallback(jsonData, errorCode);
		}
	}
	else {
		ofLogError("ofxSerpAPI") << getErrorMessage(errorCode);
	}
}

string ofxSerpAPI::uriEncode(const string& str) {
	std::ostringstream escaped;
	escaped.fill('0');
	escaped << std::hex;

	for (auto c : str) {
		if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
			escaped << c;
		}
		else {
			escaped << '%' << std::setw(2) << int((unsigned char)c);
		}
	}

	return escaped.str();
}

ofxSerpAPI::ErrorCode ofxSerpAPI::parseErrorResponse(const ofHttpResponse& response) {
	switch (response.status) {
	case 200:
		return Success;
	case 400:
		return BadRequest;
	case 401:
		return Unauthorized;
	case 403:
		return Forbidden;
	case 404:
		return NotFound;
	case 408:
		return RequestTimeout;
	case 500:
		return InternalServerError;
	case 503:
		return ServiceUnavailable;
	default:
		return UnknownError;
	}
}

string ofxSerpAPI::getErrorMessage(ErrorCode errorCode) {
	switch (errorCode) {
	case Success:
		return "Success";
	case BadRequest:
		return "Bad Request";
	case Unauthorized:
		return "Unauthorized";
	case Forbidden:
		return "Forbidden";
	case NotFound:
		return "Not Found";
	case RequestTimeout:
		return "Request Timeout";
	case InternalServerError:
		return "Internal Server Error";
	case ServiceUnavailable:
		return "Service Unavailable";
	case UnknownError:
		return "Unknown error";
	}
	return "";
}



//----

//TODOl USE CURL

size_t ofxSerpAPI::WriteCallback(char* contents, size_t size, size_t nmemb, std::string* response) {
	size_t totalSize = size * nmemb;
	response->append(contents, totalSize);

	ofLogNotice("ofxSerpAPI") << "WriteCallback: " << response;

	//sResult = response;

	return totalSize;
}

void ofxSerpAPI::doCurl(const std::string& query, const std::string& engine) {
	ofLogNotice() << "doCurl: Query: " << query << " Engine:" << engine;

	// Initialize cURL
	curl_global_init(CURL_GLOBAL_ALL);

	// Create a cURL handle
	CURL* curl = curl_easy_init();


	/*
	string sCurl = "curl --get https ://serpapi.com/search \";
		sCurl += " - d engine = "youtube" \"
		sCurl += "-d search_query="star + wars" \"
		sCurl += "-d api_key="533fe6a1d8582a75eeb08f5126291b0807b8e9d7610e55de33f01e26310abdd5"
	*/

	//curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);//disable

	/*
	CURLcode res = curl_easy_perform(curl);

	// Check if the request was successful
	if (res == CURLE_OK) {
		// Print the response
		std::cout << "Response: " << response << std::endl;
	}
	else {
		// Get SSL verification result
		long sslVerifyResult;
		curl_easy_getinfo(curl, CURLINFO_SSL_VERIFYRESULT, &sslVerifyResult);

		// Print the error message
		std::cout << "Error: " << curl_easy_strerror(res) << std::endl;
		std::cout << "SSL Verify Result: " << sslVerifyResult << std::endl;
	}
	*/

	// ssl
	curl_easy_setopt(curl, CURLOPT_CAINFO, ofToDataPath("cacert.pem"));
	//curl_easy_setopt(curl, CURLOPT_CAINFO, ofToDataPath("ssl/cacert.pem"));
	//curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");


	// Set the URL to request
	curl_easy_setopt(curl, CURLOPT_URL, "https://serpapi.com/search");

	// Set the write callback function using a lambda wrapper
	std::string sResponseCurl;
	auto writeCallbackWrapper = [](char* contents, size_t size, size_t nmemb, std::string* resp) -> size_t {
		return ofxSerpAPI::WriteCallback(contents, size, nmemb, resp);
	};
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallbackWrapper);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &sResponseCurl);

	// Set the request method to GET
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

	// Set the request parameters
	//string sEngine = "youtube";
	string params = "engine=" + engine + "&search_query=" + query + "&api_key=" + sKey.get();
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params.c_str());

	// Perform the request
	CURLcode res = curl_easy_perform(curl);

	// Check if the request was successful
	if (res == CURLE_OK) {
		// Print the sResponseCurl
		ofLogNotice("ofxSerpAPI") << "Response: " << sResponseCurl;
	}
	else {
		// Print the error message
		ofLogError("ofxSerpAPI") << curl_easy_strerror(res);
	}

	// Cleanup cURL handle
	curl_easy_cleanup(curl);

	// Cleanup cURL global resources
	curl_global_cleanup();
}


