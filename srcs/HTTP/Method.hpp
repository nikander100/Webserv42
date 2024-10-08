#pragma once
#include <string>
#include <unordered_map>

enum class Method {
	GET, POST, DELETE, PUT, HEAD, UNKNOWN
};

inline std::string methodToString(Method method) {
	static const std::unordered_map<Method, std::string> methodMap = {
		{Method::GET, "GET"},
		{Method::POST, "POST"},
		{Method::PUT, "PUT"},
		{Method::HEAD, "HEAD"},
		{Method::DELETE, "DELETE"},
		{Method::UNKNOWN, "UNKNOWN"}
	};

	auto it = methodMap.find(method);
	return it != methodMap.end() ? it->second : "UNKNOWN";
}

inline Method stringToMethod(const std::string &method) {
	static const std::unordered_map<std::string, Method> methodMap = {
		{"GET", Method::GET},
		{"POST", Method::POST},
		{"DELETE", Method::DELETE},
		{"PUT", Method::PUT},
		{"HEAD", Method::HEAD},
		{"UNKNOWN", Method::UNKNOWN}
	};

	auto it = methodMap.find(method);
	return it != methodMap.end() ? it->second : Method::UNKNOWN;
}