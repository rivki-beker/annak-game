#pragma once
#include <vector>
#include "Configuration.h"
using namespace std;
class Component
{
private:
	string _category;
	int _size;
public:
	Component(string category);
	int size() { return _size; }
	vector<int> resources;
	string category(){ return _category; };
	bool isComplete;
};
