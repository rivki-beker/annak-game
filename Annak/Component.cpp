#include "Component.h"

Component::Component(string category) :_category(category), resources(5),isComplete(false) {
	if (category == "City" || category == "Village")
		resources[4] = Configuration::config()["StartingResources"][category];
	_size = Configuration::config()["Sizes"][category][0];
}