#pragma once
#include <vector>
#include "Configuration.h"
#include "Component.h"

using namespace std;
class Tile
{
private:
	string _category;
	int _categoryId;
public:
	vector <vector<map<string, shared_ptr<Component>>>> grid;
	static int size;
	int resourceCount;
	Tile(int categoryId);
	int categoryId() { return _categoryId; };
	const string & category() { return _category;};
};

