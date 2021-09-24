#include <Logger.hpp>

#include <algorithm>
#include <iostream>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define WINDOWS
#define NOMINMAX
#include <windows.h>
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#define DISABLE_NEWLINE_AUTO_RETURN  0x0008
#endif

uint8_t Logger::logLevel = 3;

bool Logger::logType = true;
bool Logger::logColors = true;
char Logger::prefix = '|';


void Logger::initColors() {
#ifdef WINDOWS
	DWORD l_mode;
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleMode(hStdout, &l_mode);
	SetConsoleMode(hStdout, l_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN);
#endif
}

void Logger::setLogLevel(const int _logLevel) {
	logLevel = (uint8_t)_logLevel;
}

void Logger::setPrefix(const char _prefix) {
	prefix = _prefix;
}

void Logger::showType(bool show) {
	logType = show;
}

void Logger::showColor(bool show) {
	logColors = show;
}

void Logger::parseColors(std::string& text, const uint8_t level) {

	bool defaultColor = true;

	for (int i = 0; i < sizeof(tokens); i++) {

		const bool applyDefaultColor = i == sizeof(tokens) - 1 && defaultColor;

		const int charIndex = applyDefaultColor ? std::string(tokens).find(defaultColors[level]) : i;

		const char token[] = { prefix, tokens[charIndex], '\0' };

		const bool dark = charIndex < sizeof(tokens) / 2;
		const uint8_t len = dark ? 22 : 20;
		char* color = new char[len];
		
#ifdef _MSC_VER
		if (dark) sprintf_s(color, len, "\x001B[0m\x001B[%dm", charIndex + 30);
		else sprintf_s(color, len, "\x001B[0m\x001B[%d;1m", charIndex + 22);
#else
		if (dark) sprintf(color, "\x001B[0m\x001B[%dm", charIndex + 30);
		else sprintf(color, "\x001B[0m\x001B[%d;1m", charIndex + 22);
#endif

		if (applyDefaultColor) {
			text = color + text;
		} else for (size_t j = text.find(token, 0); j != std::string::npos; j = text.find(token, j)) {
			text.replace(j, 2, color);
			j += len - 2;
			defaultColor = false;
		}

		delete[] color;
	}
}

void Logger::println(std::string text, const uint8_t level) {
	if(logColors) parseColors(text, level);
	std::cout << text + "\x001B[0m\n";
}

void Logger::print(std::string text, const uint8_t level) {
	if (logColors) parseColors(text, level);
	std::cout << text + "\x001B[0m";
}

void Logger::debug(const std::string& text) {
	if (logLevel >= 4) {
		println((logType ? "[debug] " : "") + text, 3);
	}
}

void Logger::log(const std::string& text) {
	if (logLevel >= 3) {
		println((logType ? "[log] " : "") + text, 2);
	}
}

void Logger::warn(const std::string& text) {
	if (logLevel >= 2) {
		println((logType ? "[warn] " : "") + text, 1);
	}
}
