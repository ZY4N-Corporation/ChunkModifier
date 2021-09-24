#include "ArgParser.hpp"

char ArgParser::prefix = '-';

void ArgParser::setPrefix(const char _prefix) {
	prefix = _prefix;
}

std::string ArgParser::findArg(std::string arg) {
	arg = prefix + arg;
	std::string out;
	for (size_t i = 0; i < arguments.size(); i++) {
		if (arguments[i].find(arg) == 0 && i + sizeof(prefix) < arguments.size()) {
			out = arguments[i + sizeof(prefix)];
			arguments.erase(arguments.begin() + i, arguments.begin() + i + sizeof(prefix));
			break;
		}
	}
	return out;
}


void ArgParser::parse(std::string arg, bool mandatory, vf3& v) {
	if(sscanf(findArg(arg).c_str(), "%f,%f,%f", &v.x, &v.y, &v.z) != 3 &&  mandatory)
		throw std::invalid_argument("Mandatory argument \"" + arg + "\" is missing.");
}

void ArgParser::parse(std::string arg, bool mandatory, float& f) {
	try {
		std::string::size_type sz;
		f = std::stof(findArg(arg), &sz);
	} catch (std::invalid_argument) {
		if (mandatory) throw std::invalid_argument("Mandatory argument \"" + arg + "\" is missing.");
	}
}

void ArgParser::parse(std::string arg, bool mandatory, int& i) {
	try {
		std::string::size_type sz;
		i = std::stoi(findArg(arg), &sz);
	} catch (std::invalid_argument) {
		if (mandatory) throw std::invalid_argument("Mandatory argument \"" + arg + "\" is missing.");
	}
}

void ArgParser::parse(std::string arg, bool mandatory, std::string& s) {
	s = findArg(arg);
	if (mandatory && s.length() <= 0) {
		throw std::invalid_argument("Mandatory argument \"" + arg + "\" is missing.");
	}
}

void ArgParser::parse(std::string arg, bool mandatory, bool& b) {
	std::string value = findArg(arg);
	if (value == "true") {
		b = true;
	} else if (value == "false") {
		b = false;
	} else if (mandatory) {
		throw std::invalid_argument("Mandatory argument \"" + arg + "\" is missing.");
	}
}

void ArgParser::parseStr(std::string arg, bool mandatory, std::function<void(std::string&)> callback) {
	std::string s = findArg(arg);
	if (s.length() > 0) callback(s);
	else if (mandatory) throw std::invalid_argument("Mandatory argument \"" + arg + "\" is missing.");
}

void ArgParser::parseVec(std::string arg, bool mandatory, std::function<void(vf3&)> callback) {
	vf3 v;
	if (sscanf(findArg(arg).c_str(), "%f,%f,%f", &v.x, &v.y, &v.z) == 3) callback(v);
	else if (mandatory) throw std::invalid_argument("Mandatory argument \"" + arg + "\" is missing.");
}

void ArgParser::parseFloat(std::string arg, bool mandatory, std::function<void(float)> callback) {
	try {
		std::string::size_type sz;
		callback(std::stof(findArg(arg), &sz));
	} catch (std::invalid_argument) {
		if (mandatory) throw std::invalid_argument("Mandatory argument \"" + arg + "\" is missing.");
	}
}

void ArgParser::parseInt(std::string arg, bool mandatory, std::function<void(int)> callback) {
	try {
		std::string::size_type sz;
		callback(std::stoi(findArg(arg), &sz));
	} catch (std::invalid_argument) {
		if (mandatory) throw std::invalid_argument("Mandatory argument \"" + arg + "\" is missing.");
	}
}

void ArgParser::parseBool(std::string arg, bool mandatory, std::function<void(void)> callback) {
	std::string value = findArg(arg);
	if (value == "true") callback();
	else if (mandatory) throw std::invalid_argument("Mandatory argument \"" + arg + "\" is missing.");
}
