#pragma once
#include <string>
#include <bitset>

class Logger {
private:
	static uint8_t logLevel;
	static bool logType;
	static bool logColors;
	static char prefix;
	
	static constexpr char tokens[] = "krgybpcwKRGYBPCW";
	static constexpr char defaultColors[] = "RYWK";

	static void parseColors(std::string &text, const uint8_t level);

public:
	static void initColors();

	static void setLogLevel(const int logLevel);
	static void setPrefix(const char prefix);
	static void showType(bool show);
	static void showColor(bool show);

	static void println(std::string text, const uint8_t level);
	static void print(std::string text, const uint8_t level);

	static void debug(const std::string& text);
	static void log(const std::string& text);
	static void warn(const std::string& text);

	static void error(const std::string& text, const char funcName[], int line) {
		if (logLevel >= 1) {
			std::string info = showType ? "[error]" : "";
			info += '[' + std::string(funcName) + ']';
			info += '[' + std::to_string(line) + "] ";
			println(info + text, 0);
		}
	}
	#define error(text) error(text, __FUNCTION__, __LINE__);
};
