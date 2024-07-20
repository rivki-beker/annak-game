#include "Game.h"
#include "Screen.h"
#include <iostream>

int main() {
    try {
        string s;
        getline(cin, s);

        Configuration::loadConfiguration("configuration.json");

        Screen screen;

        screen.run();
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}