#include "Game.h"

Game::Game(Input& input) : input(input), world({}) {}

void Game::simulateGame() {
	handleStartCommands();
	handleStepsCommands();
	handleAssertCommands();
}

void Game::handleStartCommands() {
	for (const auto& command : input.start) {
		if (command->name == Command::RESOURCE) {
			handleResourceCommand(command);
		}
		else if (command->name == Command::RESOURCES) {
			handleResourcesCommand(command);
		}
		else if (command->name == Command::PEOPLE) {
			handlePeopleCommand(command);
		}
		else if (command->name == Command::BUILD || command->name == Command::MANUFACTURE) {
			handleBuildCommand(command);
		}
		else if (command->name == Command::MAKE_EMPTY) {
			handleMakeEmptyCommand(command);
		}
	}
}

void Game::handleResourceCommand(const shared_ptr<Command>& command) {
	int x = stoi(command->arguments[2]), y = stoi(command->arguments[3]);
	auto& cell = world.getCell(x, y);
	for (auto& component : cell)
	{
		if (component.first != "Road") {
			auto& resourceTypes = Configuration::config()["ResourceTypes"];
			int i;
			for (i = 0; i < resourceTypes.size(); i++)
				if (resourceTypes[i] == command->arguments[1])
					break;
			if (component.first != "People") {
				int capacity = int(Configuration::config()["Capacities"][component.first][i]);
				component.second->resources[i] = min(stoi(command->arguments[0]), capacity);
			}
			else
				component.second->resources[i] = stoi(command->arguments[0]);
			x = -1;
			break;
		}
	}
	if (x != -1)
	{
		Tile& tile = world.getTile(x, y);
		tile.resourceCount = stoi(command->arguments[0]);
	}
}

void Game::handleResourcesCommand(const shared_ptr<Command>& command) {
	auto& cell = world.getCell(stoi(command->arguments[4]), stoi(command->arguments[5]));
	for (auto& component : cell)
	{
		if (component.first != "Road") {
			for (int i = 0; i < 4; i++)
				if (component.first != "People") {
					int capacity = int(Configuration::config()["Capacities"][component.first][i]);
					component.second->resources[i] = min(stoi(command->arguments[i]), capacity);
				}
				else
					component.second->resources[i] = stoi(command->arguments[i]);
			break;
		}
	}
}

void Game::handlePeopleCommand(const shared_ptr<Command>& command) {
	auto& cell = world.getCell(stoi(command->arguments[1]), stoi(command->arguments[2]));
	cell.insert({ Command::PEOPLE, make_shared<Component>(Command::PEOPLE) });
}

void Game::handleBuildCommand(const shared_ptr<Command>& command) {
	string category = command->arguments[0];
	shared_ptr<Component> component = make_shared<Component>(category);
	int beginX = stoi(command->arguments[1]), beginY = stoi(command->arguments[2]);
	for (size_t i = 0; i < component->size(); i++) {
		for (size_t j = 0; j < component->size(); j++) {
			world.getCell(beginX + i, beginY + j).insert({ category ,component });
		}
	}
	component->isComplete = true;
	componentsCount[category]++;
}

void Game::handleMakeEmptyCommand(const shared_ptr<Command>& command) {
	auto& cell = world.getCell(stoi(command->arguments[0]), stoi(command->arguments[1]));
	for (auto& entry : cell) {
		for (auto& resource : entry.second->resources) {
			resource = 0;
		}
	}
}

void Game::handleStepsCommands() {

	for (const auto& command : input.steps) {
		if (command->name == Command::SELECT) {
			handleSelectCommand(stoi(command->arguments[0]), stoi(command->arguments[1]));
		}
		else if (command->name == Command::PEOPLE) {
			handlePeopleCommandDuringStep(command);
		}
		else if (command->name == Command::WORK) {
			handleWorkCommand(command);
		}
		else if (command->name == Command::RAIN) {
			rainCount += stoi(command->arguments[0]);
		}
		else if (command->name == Command::BUILD) {
			handleBuildCommandDuringStep(command);
		}
		else if (command->name == Command::MANUFACTURE) {
			handleManufactureCommand(command);
		}
		else if (command->name == Command::TAKE_RESOURCES) {
			handleTakeResourcesCommand(command);
		}
		else if (command->name == Command::MOVE) {
			handleMoveCommand(command);
		}
		else if (command->name == Command::WAIT) {
			handleWaitCommand(command);
		}
	}
}

void Game::handleSelectCommand(int x, int y) {
	selectedCell = { x,y };
}

void Game::handlePeopleCommandDuringStep(const shared_ptr<Command>& command) {
	int countPeople = stoi(command->arguments[0]);
	auto& cell = world.getCell(stoi(command->arguments[1]), stoi(command->arguments[2]));
	auto& resourceTypes = Configuration::config()["ResourceTypes"];
	int peopleIndex = resourceTypes.find("People") - resourceTypes.begin() - 1;
	for (auto& entry : cell) {
		auto capacities = Configuration::config()["Capacities"];
		auto it = capacities.find(entry.first);
		if (it != capacities.end()) {
			entry.second->resources[peopleIndex] = min(int(it.value()[peopleIndex]), countPeople);
			countPeople = 0;
			break;
		}
	}
	if (countPeople && cell.find(Command::PEOPLE) == cell.end()) {
		cell.insert({ Command::PEOPLE, make_shared<Component>(Command::PEOPLE) });
	}
}

void Game::handleWorkCommand(const shared_ptr<Command>& command) {
	auto& cell = world.getCell(selectedCell.first, selectedCell.second);
	shared_ptr<Component> people = cell[Command::PEOPLE];
	cell.erase(Command::PEOPLE);
	pair<int, int> workingTail = { stoi(command->arguments[0]), stoi(command->arguments[1]) };
	world.getCell(workingTail.first, workingTail.second).insert({ Command::PEOPLE, people });
	workTime.insert({ workingTail, 0 });
}

void Game::handleBuildCommandDuringStep(const shared_ptr<Command>& command) {
	string category = command->arguments[0];
	int beginX = stoi(command->arguments[1]), beginY = stoi(command->arguments[2]);
	if (world.canBuild(beginX, beginY, category)) {
		shared_ptr<Component> component = make_shared<Component>(category);
		for (size_t i = 0; i < component->size(); i++) {
			for (size_t j = 0; j < component->size(); j++) {
				world.getCell(beginX + i, beginY + j).insert({ category, component });
			}
		}
		buildTime.insert({ component, 0 });
		componentsCount[category]++;
	}
}

void Game::handleManufactureCommand(const shared_ptr<Command>& command) {
	string category = command->arguments[0];
	shared_ptr<Component> component = make_shared<Component>(category);
	int beginX = stoi(command->arguments[1]), beginY = stoi(command->arguments[2]);
	auto& cell = world.getCell(beginX, beginY);
	auto it = cell.find("City");
	if (it == cell.end())
		it = cell.find("Village");
	if (it == cell.end())
		return;
	auto& cost = Configuration::config()["Costs"][category];
	for (int i = 0; i < cost.size(); i++) {
		if (it->second->resources[i] < int(cost[i]))
			return;
	}
	for (int i = 0; i < cost.size(); i++)
		it->second->resources[i] -= int(cost[i]);
	for (size_t i = 0; i < component->size(); i++) {
		for (size_t j = 0; j < component->size(); j++) {
			world.getCell(beginX + i, beginY + j).insert({ category ,component });
		}
	}
	component->isComplete = true;
	componentsCount[category]++;
}

void Game::handleTakeResourcesCommand(const shared_ptr<Command>& command) {
	auto& cell = world.getCell(selectedCell.first, selectedCell.second);
	shared_ptr<Component> component;
	for (auto& comp : cell)
		if (comp.first == "Truck" || comp.first == "Car" || comp.first == "Helicopter")
			component = comp.second;
	auto& fromCell = world.getCell(stoi(command->arguments[0]), stoi(command->arguments[1]));
	vector<int>* resources = nullptr;
	for (auto& entry : fromCell)
		if (entry.first != "Road" && entry.first != "Poeple")
		{
			resources = &entry.second->resources;
			break;
		}
	//if (!resources)------------------------------------------
	takeResourcesTime.insert({ {component,resources},0 });
}

void Game::handleMoveCommand(const shared_ptr<Command>& command) {
	string category;
	auto& cell = world.getCell(selectedCell.first, selectedCell.second);
	for (auto& entry : cell)
	{
		if (entry.first == "Road" || entry.first == "City" || entry.first == "Village")
			continue;
		else {
			category = entry.first;
			break;
		}
	}
	pair<int, int> fromCell = world.beginCell(selectedCell, category);
	pair<int, int> destCell = { stoi(command->arguments[0]),stoi(command->arguments[1]) };
	int length = abs(destCell.first - fromCell.first) + abs(destCell.second - fromCell.second);
	int time = length / float(Configuration::config()["Speeds"][category]);
	moveTime.insert({ {category,{fromCell,destCell}},time });
}

void Game::handleWaitCommand(const shared_ptr<Command>& command) {
	int waitTime = stoi(command->arguments[0]);
	for (auto it = buildTime.begin(); it != buildTime.end();) {
		it->second += waitTime;
		if (it->second >= 1000) {
			it->first->isComplete = true;
			it = buildTime.erase(it);
		}
		else {
			++it;
		}
	}
	for (auto it = workTime.begin(); it != workTime.end();) {
		it->second += waitTime;
		if (it->second >= 1000) {
			world.getTile(it->first.first, it->first.second).resourceCount--;
			it = workTime.erase(it);
		}
		else {
			++it;
		}
	}
	for (auto it = takeResourcesTime.begin(); it != takeResourcesTime.end();) {
		it->second += waitTime;
		if (it->second >= 2000) {
			auto& capacity = Configuration::config()["Capacities"][it->first.first->category()];
			for (int i = 0; i < capacity.size(); i++) {
				int countTake = min(it->first.second->at(i), int(capacity[i])) - it->first.first->resources[i];
				it->first.first->resources[i] += countTake;
				it->first.second->at(i) -= countTake;
			}
			it = takeResourcesTime.erase(it);
		}
		else {
			++it;
		}
	}
	for (auto it = moveTime.begin(); it != moveTime.end();) {
		it->second -= waitTime;
		if (it->second <= 0) {
			auto& fromCell = world.getCell(it->first.second[0].first, it->first.second[0].second);
			auto& component = fromCell[it->first.first];
			for (int i = 0; i < component->size(); i++)
				for (int j = 0; j < component->size(); j++)
				{
					auto& removeCell = world.getCell(it->first.second[0].first + i, it->first.second[0].second + j);
					removeCell.erase(component->category());
					auto& addCell = world.getCell(it->first.second[1].first + i, it->first.second[1].second + j);
					addCell.insert({ component->category(),component });
				}
			selectedCoordinates = it->first.second[1];
			it = moveTime.erase(it);
		}
		else {
			++it;
		}
	}

	waitTime = min(waitTime, rainCount);
	rainCount -= waitTime;

	for (auto& entry : rainTime) {
		entry.second += waitTime;
		int lid = Configuration::config()["Rains"][entry.first];
		if (entry.second >= lid) {
			entry.second -= lid;
			string category = "Field";
			if (entry.first == "Wood") {
				category = "Forest";
			}
			world.increaseResource(category);
		}
	}
}

void Game::handleAssertCommands() {
	auto& selectedCellMap = world.getCell(selectedCell.first, selectedCell.second);

	for (const auto& assert : input.asserts) {
		cout << assert << " ";
		if (assert.size() >= 5 && assert.compare(assert.size() - 5, 5, "Count") == 0) {
			handleCountAssert(assert);
		}
		else if (assert == "SelectedCategory") {
			handleSelectedCategoryAssert(selectedCellMap);
		}
		else if (assert == "SelectedResource") {
			handleSelectedResourceAssert(selectedCellMap);
		}
		else if (assert == "SelectedPeople") {
			handleSelectedPeopleAssert(selectedCellMap);
		}
		else if (assert == "SelectedComplete") {
			handleSelectedCompleteAssert(selectedCellMap);
		}
		else if (assert == "SelectedCar") {
			cout << (selectedCellMap.count("Car") ? 1 : 0) << endl;
		}
		else if (assert == "SelectedTruck") {
			cout << (selectedCellMap.count("Truck") ? 1 : 0) << endl;
		}
		else if (assert == "SelectedCoordinates") {
			cout << (selectedCoordinates.first - 1) / Tile::size << " " << (selectedCoordinates.second - 1) / Tile::size << endl;
		}
	}
}

void Game::handleCountAssert(const string& assert) {
	string category = assert.substr(0, assert.size() - 5);
	cout << componentsCount[category] << endl;
}

void Game::handleSelectedCategoryAssert(const map<string, shared_ptr<Component>>& selectedCellMap) {
	if (!selectedCellMap.size()) {
		cout << world.getTile(selectedCell.first, selectedCell.second).category() << endl;
	}
	else {
		cout << selectedCellMap.begin()->first << endl;
	}
}

void Game::handleSelectedResourceAssert(const map<string, shared_ptr<Component>>& selectedCellMap) {
	if (selectedCellMap.size() > 1 || selectedCellMap.size() == 1 && selectedCellMap.begin()->first != "Road") {
		for (int i = 0; i < 4; i++)
			cout << " " << selectedCellMap.begin()->second->resources[i];
		cout << endl;
	}
	else {
		auto& selectedTile = world.getTile(selectedCell.first, selectedCell.second);
		for (size_t i = 0; i < 4; i++) {
			if (selectedTile.categoryId() - 3 == i) {
				cout << " " << selectedTile.resourceCount;
			}
			else {
				cout << " 0";
			}
		}
		cout << endl;
	}
}

void Game::handleSelectedPeopleAssert(const map<string, shared_ptr<Component>>& selectedCellMap) {
	bool isPrinted = false;
	auto& resourceTypes = Configuration::config()["ResourceTypes"];
	int peopleIndex = resourceTypes.find("People") - resourceTypes.begin() - 1;
	for (auto& entry : selectedCellMap) {
		auto& capacities = Configuration::config()["Capacities"];
		if (capacities.find(entry.first) != capacities.end()) {
			cout << entry.second->resources[peopleIndex] << endl;
			isPrinted = true;
			break;
		}
	}
	if (!isPrinted) {
		cout << (selectedCellMap.find("People") != selectedCellMap.end()) << endl;
	}
}

void Game::handleSelectedCompleteAssert(const map<string, shared_ptr<Component>>& selectedCellMap) {
	bool isComplete = true;
	for (auto& entry : selectedCellMap) {
		if (!entry.second->isComplete) {
			isComplete = false;
			break;
		}
	}
	cout << (isComplete ? "True" : "False") << endl;
}