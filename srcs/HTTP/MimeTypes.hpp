#pragma once
#include <string>
#include <unordered_map>

// Function to get MIME type string directly from extension
inline std::string getMimeTypeFromExtension(const std::string& extension) {
	static const std::unordered_map<std::string, std::string> mimeTypes = {
		{".html", "text/html"},
		{".htm", "text/html"},
		{".jpg", "image/jpeg"},
		{".jpeg", "image/jpeg"},
		{".png", "image/png"},
		{".gif", "image/gif"},
		{".ico", "image/x-icon"},
		{".css", "text/css"},
		{".svg", "image/svg+xml"},
		{".js", "application/javascript"},
		{".json", "application/json"},
		{".xml", "application/xml"},
		{".txt", "text/plain"},
		{".avi", "video/x-msvideo"},
		{".bmp", "image/bmp"},
		{".doc", "application/msword"},
		{".gz", "application/x-gzip"},
		{".mp3", "audio/mp3"},
		{".pdf", "application/pdf"},
		// {"", "application/octet-stream"} // Default MIME type
		{"", "text/plain"}
	};

	// Convert extension to lower case
	std::string lowerExt = extension;
	std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), ::tolower);

	// Look up MIME type based on extension
	auto it = mimeTypes.find(lowerExt);
	if (it != mimeTypes.end()) {
		return it->second;
	} else {
		return "application/octet-stream"; // Default MIME type if not found
	}
}