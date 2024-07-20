#include "Tile.h"

int Tile::size;

Tile::Tile(int categoryId)
{
	this->_categoryId = categoryId;

	for (const auto& entry : Configuration::config()["Tiles"].items())
		if(entry.value() == categoryId)
			_category = entry.key();

	auto startingResources = Configuration::config()["StartingResources"];
	const auto& it= startingResources.find(_category);
	if (it != startingResources.end())
		this->resourceCount = it.value();
	for (size_t i = 0; i < size; i++) {
		vector<map<string, shared_ptr<Component>>> cols;
		for (size_t j = 0; j < size; j++) {
			map<string, shared_ptr<Component>> square;
			cols.emplace_back(square);
		}
		grid.emplace_back(cols);
	}
}