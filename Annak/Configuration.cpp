#include "Configuration.h"
#include "Tile.h"

json Configuration::_config;

void Configuration::loadConfiguration(const string& config_file) {
    std::ifstream ifs(config_file);
    if (!ifs.is_open())
        throw runtime_error("Failed to open configuration file: " + config_file);

    ifs >> _config;

    Tile::size= _config["Sizes"]["Tile"][0];
}