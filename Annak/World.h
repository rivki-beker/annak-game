#pragma once
#include <vector>
#include <stdexcept>
#include "Tile.h"
#include "Configuration.h"
#include "Component.h"

using namespace std;

class World
{
private:
    vector<vector<Tile>> worldMap;

public:
    World(const vector<vector<string>>& worldData);

    Tile& getTile(int x, int y);

    map<string, shared_ptr<Component>>& getCell(int x, int y);

    pair<int,int> beginCell(pair<int,int> cell,string category);

    bool canBuild(int x, int y, const string& category);

    void increaseResource(string category);
};
