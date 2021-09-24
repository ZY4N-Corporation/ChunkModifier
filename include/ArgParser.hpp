#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <functional>
#include <vf3.hpp>

class ArgParser {
private:
	static char prefix;
	std::vector<std::string> arguments;
	 
public:
	ArgParser(int numArgs, char* args[]) : arguments(args + 1, args + numArgs) {};

	static void setPrefix(const char prefix);

	std::string findArg(std::string arg);

	void parseStr(std::string arg, bool mandatory, std::function<void(std::string&)>);
	void parseVec(std::string arg, bool mandatory, std::function<void(vf3&)>);
	void parseFloat(std::string arg, bool mandatory, std::function<void(float)>);
	void parseInt(std::string arg, bool mandatory, std::function<void(int)>);
	void parseBool(std::string arg, bool mandatory, std::function<void(void)>);
	
	void parse(std::string arg, bool mandatory, vf3& v);
	void parse(std::string arg, bool mandatory, float& f);
	void parse(std::string arg, bool mandatory, int& i);
	void parse(std::string arg, bool mandatory, std::string& s);
	void parse(std::string arg, bool mandatory, bool& b);
};