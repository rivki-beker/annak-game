#pragma once
#include <map>
#include <string>
#include "json.hpp"
#include <fstream>
#include <stdexcept>

using json = nlohmann::json;
using namespace std;
class Configuration
{
private:
	static json _config;
public:
	static const json& config() { return _config; }
	static void loadConfiguration(const string& config_file);
};