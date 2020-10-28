#include <iostream>

#include "view.hh"

namespace view {

Console::Console()
{}

Console::~Console()
{
    endwin();
}

void Console::initDisplay()
{
    //cout << "Initializing display" << endl;
    map::Map &map = map::Map::getMap();
    map_width_ = map.getWidth();
    map_height_ = map.getHeight();
    //cout << map.getTile(0, 0);
    
    for (size_t y = 0; y < map_height_; ++y) {
        map_.push_back(std::vector<char>());
        //cout << "Creating row #" << y << endl;
        for (size_t x = 0; x < map_width_; ++x) {
            //cout << "Pushing character #" << x << endl;
            map_[y].push_back('\0');
            map::tile::TileType type = map.getTile(x, y)->getType();
            switch (type) {
            case map::tile::kFloor:
                map_[y][x] = '.';
                break;
            case map::tile::kWall:
                map_[y][x] = '#';
                break;
            case map::tile::kEntrance:
                map_[y][x] = 'I';
                break;
            case map::tile::kExit:
                map_[y][x] = 'O';
                break;
            default:
                map_[y][x] = '?';
                break;
            }
        }
    }
    //cout << "Map dumped to display" << endl;
    initscr();
}

void Console::update()
{
    clear();
    for (size_t y = 0; y < map_height_; ++y) {
        for (size_t x = 0; x < map_width_; ++x) {
            addch(map_[y][x]);
        }
        addch('\n');
    }
    refresh();
}

}   // namespace view