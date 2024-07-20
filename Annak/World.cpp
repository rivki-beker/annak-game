#include "World.h"

World::World(const vector<vector<string>>& worldData) {
    if (!worldData.empty()) {
        for (const auto& row : worldData) {
            vector<Tile> mapRow;
            for (const auto& value : row) {
                mapRow.emplace_back(stoi(value));
            }
            worldMap.emplace_back(mapRow);
        }
    }
}

Tile& World::getTile(int x, int y) {
    x = (x - 1) / Tile::size;
    y = (y - 1) / Tile::size;

    if (x < 0 || x >= worldMap[0].size() || y < 0 || y >= worldMap.size()) {
        return *new Tile(1);
    }

    return worldMap[y][x];
}

map<string, shared_ptr<Component>>& World:: getCell(int x, int y) {
    int xCell = (x - 1) % Tile::size;
    int yCell = (y - 1) % Tile::size;

    if (xCell < 0 || xCell >= Tile::size || yCell < 0 || yCell >= Tile::size) {
        return *new map<string, shared_ptr<Component>>();
    }

    return getTile(x, y).grid[yCell][xCell];
}

pair<int, int> World::beginCell(pair<int, int> cell, string category) {
    map<string, shared_ptr<Component>>* minCell=nullptr;
    if (cell.first > 1 && cell.second > 1) {
        minCell = &getCell(cell.first - 1, cell.second - 1);
        while (minCell->find(category) != minCell->end())
        {
            cell.first--;
            cell.second--;
            if (cell.first == 1 || cell.second == 1)
                break;
            minCell = &getCell(cell.first - 1, cell.second - 1);
        }
        minCell = nullptr;
    }
    if (cell.first > 1) {
        minCell = &getCell(cell.first - 1, cell.second);
        while (minCell->find(category) != minCell->end())
        {
            cell.first--;
            if (cell.first == 1)
                break;
            minCell = &getCell(cell.first - 1, cell.second);
        }
    }
    if (!minCell && cell.second > 1) {
        minCell = &getCell(cell.first, cell.second - 1);
        while (minCell->find(category) != minCell->end())
        {
            cell.second--;
            if (cell.second == 1)
                break;
            minCell = &getCell(cell.first, cell.second - 1);
        }
    }
    return cell;
}

bool World::canBuild(int x, int y, const string& category) {
    int categorySize = Configuration::config()["Sizes"][category][0];
    int roadSize = Configuration::config()["Sizes"]["Road"][0];

    auto checkDirection = [&](int startX, int startY, int dx, int dy) -> bool {
        int roadCount = 0;
        try {
            for (int i = 0; i < categorySize; ++i) {
                auto& cell = getCell(startX + i * dx, startY + i * dy);
                for (const auto& component : cell) {
                    if (component.first == "Road") {
                        roadCount++;
                        break;
                    }
                }
                if (roadCount == roadSize) {
                    return true;
                }
            }
        }
        catch (...) {}
        return false;
    };

    return checkDirection(x - 1, y, 0, 1) || checkDirection(x + 1, y, 0, 1) ||
        checkDirection(x, y - 1, 1, 0) || checkDirection(x, y + 1, 1, 0);
}

void World::increaseResource(string category) {

    for (auto& vec : worldMap)
    {
        for (Tile& tile : vec)
        {
            if (tile.category() == category) {
                tile.resourceCount++;
            }
        }
    }
}
