#pragma once
#include <iostream>
#include <map>
#include <memory>
#include "Input.h"
#include "Tile.h"
#include "World.h"

using namespace std;

class Game {
private:
	Input& input;
	map<string, int> componentsCount;
	pair<int, int> selectedCell = { 0, 0 };
	pair<int, int> selectedCoordinates = { 0, 0 };
	map<shared_ptr<Component>, int> buildTime;
	map<pair<shared_ptr<Component>, vector<int>*>, int> takeResourcesTime;
	map<pair<int, int>, int> workTime;
	map<pair<string, vector<pair<int, int>>>, int>moveTime;
	map<string, int> rainTime = { {"Wood", 0}, {"Wool", 0} };
	int rainCount = 0;

public:
	World world;

	Game(Input& input);

	void simulateGame();

	void handleStartCommands();

	void handleStepsCommands();

	void handleAssertCommands();

	void handleSelectCommand(int x, int y);
private:

	void handleResourceCommand(const shared_ptr<Command>& command);

	void handleResourcesCommand(const shared_ptr<Command>& command);

	void handlePeopleCommand(const shared_ptr<Command>& command);

	void handleBuildCommand(const shared_ptr<Command>& command);

	void handleMakeEmptyCommand(const shared_ptr<Command>& command);

	void handlePeopleCommandDuringStep(const shared_ptr<Command>& command);

	void handleWorkCommand(const shared_ptr<Command>& command);

	void handleBuildCommandDuringStep(const shared_ptr<Command>& command);

	void handleManufactureCommand(const shared_ptr<Command>& command);

	void handleTakeResourcesCommand(const shared_ptr<Command>& command);

	void handleMoveCommand(const shared_ptr<Command>& command);

	void handleWaitCommand(const shared_ptr<Command>& command);

	void handleCountAssert(const string& assert);

	void handleSelectedCategoryAssert(const map<string, shared_ptr<Component>>& selectedCellMap);

	void handleSelectedResourceAssert(const map<string, shared_ptr<Component>>& selectedCellMap);

	void handleSelectedPeopleAssert(const map<string, shared_ptr<Component>>& selectedCellMap);

	void handleSelectedCompleteAssert(const map<string, shared_ptr<Component>>& selectedCellMap);
};