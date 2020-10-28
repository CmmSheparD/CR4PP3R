#include <cstdlib>

#include <iostream>

#include "map/map.hh"
#include "view/view.hh"

using namespace std;


int main()
{
    srand(time(0));
    cout << "Initializing map" << endl;
    map::Map::initMap(150, 35);
    cout << "Map initialized" << endl;
    map::Map &map = map::Map::getMap();
    cout << "Creating console" << endl;
    view::Console *console = new view::Console();
    console->initDisplay();
    console->update();
    getch();
    delete console;
    cout << "Console deleted" << endl;
    map.getTile(10, 10);
    cout << endl;
    return 0;
}