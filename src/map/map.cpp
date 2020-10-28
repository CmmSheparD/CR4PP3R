#include "map.hh"

#include <cstdlib>

namespace map {

std::shared_ptr<Map> Map::instance_ = nullptr;

Map::Map(size_t width, size_t height) : width_(width), height_(height)
{
    LayoutGenerator lg(height, width);
    lg.createLayout();
    grid_ = std::shared_ptr<std::shared_ptr<std::shared_ptr<tile::Tile>[]>[]>();
    grid_.swap(lg.getLayout());
}

Map::Map(const Map &other)
{
    operator=(other);
}

Map::Map(Map &&other)
{
    width_ = other.width_;
    height_ = other.height_;
    grid_.swap(other.grid_);
}

Map &Map::operator=(const Map &other)
{
    if (this != &other) {
        width_ = other.width_;
        height_ = other.height_;
        grid_.reset(new std::shared_ptr<std::shared_ptr<tile::Tile>[]>[height_]);
        // Copy construct every single tile
        for (size_t y = 0; y < height_; ++y) {
            grid_[y] = std::shared_ptr<std::shared_ptr<tile::Tile>[]>(
                new std::shared_ptr<tile::Tile>[width_]);
            for (size_t x = 0; x < width_; ++x) {
                grid_[y][x] = std::shared_ptr<tile::Tile>(
                    new tile::Tile(*other.grid_[y][x].get()));
            }
        }
    }
    return *this;
}

Map::~Map()
{}

bool Map::initMap(size_t width, size_t height)
{
    // Return false if the map has already been created
    // or requested map size is too small.
    if (instance_ || width < 1 || height < 1) return false;
    instance_ = std::shared_ptr<Map>(new Map(width, height));
    return true;
}

bool Map::destroyMap()
{
    if (!instance_) return false;
    instance_ = nullptr;
    return true;
}

// FIXME: throw an exception, when trying to get non-initialized map
Map &Map::getMap()
{
    return *instance_;
}

size_t Map::getHeight()
{
    return height_;
}

size_t Map::getWidth()
{
    return width_;
}

// FIXME: throw an exception, when x or y is out of range
std::shared_ptr<tile::Tile> Map::getTile(size_t x, size_t y)
{
    if (x >= width_ || y >= height_) return nullptr;
    return grid_[y][x];
}

Map::MapIterator Map::begin()
{
    return MapIterator(0, 0);
}

Map::MapIterator Map::end()
{
    // As the last valid element is [w-1][h-1], [w][h-1] is next after
    // the last and shall be returned.
    return MapIterator(width_, height_ - 1);
}


Map::MapIterator::MapIterator(size_t x, size_t y) : x_(x), y_(y) {}

Map::MapIterator::~MapIterator() {}

bool Map::MapIterator::operator==(const MapIterator &other)
{
    if (x_ == other.x_ && y_ == other.y_) return true;
    return false;
}

bool Map::MapIterator::operator!=(const MapIterator &other)
{
    return !(*this == other);
}

std::shared_ptr<tile::Tile> &Map::MapIterator::operator*()
{
    return Map::getMap().grid_[y_][x_];
}

std::shared_ptr<tile::Tile> Map::MapIterator::operator->()
{
    return Map::getMap().getTile(x_, y_);
}

Map::MapIterator &Map::MapIterator::operator++()
{
    if (x_ >= Map::getMap().getWidth() - 1) {
        y_++;
        x_ = 0;
    } else {
        x_++;
    }
    return *this;
}

Map::MapIterator Map::MapIterator::operator++(int)
{
    Map::MapIterator tmp(*this);
    operator++();
    return tmp;
}


Map::LayoutGenerator::LayoutGenerator(size_t height, size_t width)
    : height_(height), width_(width)
{}

/*
 * Splits a pool of available space into a table of smaller rectangles.
 * Then puts a room into every rectanlge. Every dimension of every room must
 * be at least a tile longer then half of dimension of corresponding area.
 * Then it randomly generates passages between adjacent rooms.
 */
void Map::LayoutGenerator::createLayout()
{
    grid_ = std::shared_ptr<std::shared_ptr<std::shared_ptr<tile::Tile>[]>[]>(
        new std::shared_ptr<std::shared_ptr<tile::Tile>[]>[height_]);
    for (size_t y = 0; y < height_; ++y) {
        grid_[y] = std::shared_ptr<std::shared_ptr<tile::Tile>[]>(
            new std::shared_ptr<tile::Tile>[width_]);
        for (size_t x = 0; x < width_; ++x) {
            grid_[y][x] = std::shared_ptr<tile::Tile>(new tile::Tile);
        }
    }
    genRectsTable();
    bindRooms();
    makeCoherent();
    dumpLayout();
}

std::shared_ptr<std::shared_ptr<std::shared_ptr<tile::Tile>[]>[]>
&Map::LayoutGenerator::getLayout()
{
    return grid_;
}

/*
 * Generates a table of rectangles based on calculated partitioning
 * of the map's tile grid.
 */
void Map::LayoutGenerator::genRectsTable()
{
    // Leave one tile for border
    // Inner width and height
    int innerw = width_ - 2;
    int innerh = height_ - 2;
    // Width and height of every rectangle
    int rectw = 6;
    int recth = 5;
    // Amount of rectangles that fit on every dimension
    int horq;
    int verq;
    // Amount of free space to place between rectangles
    // Must be at leeat xxxpad >= xxxq - 1
    int horpad;
    int verpad;
    while (true) {
        horq = innerw / rectw;
        horpad = innerw % rectw;
        if (horpad >= horq - 1) break;
        --horq;
        horpad += rectw;
        if (horpad >= horq - 1) break;
        ++rectw;
    }
    while (true) {
        verq = innerh / recth;
        verpad = innerh % recth;
        if (verpad >= verq - 1) break;
        --verq;
        verpad += recth;
        if (verpad >= verq - 1) break;
        ++recth;
    }
    for (int y = 0; y < verq; ++y) {
        table_.push_back(std::vector<RectTabField_>());
        for (int x = 0; x < horq; ++x) {
            table_[y].push_back({recth*y + verpad/(verq - 1)*y + 1,
                                rectw*x + horpad/(horq - 1)*x + 1,
                                recth, rectw, false, false, false,
                                false, false});
        }
    }
}

void Map::LayoutGenerator::bindRooms(int y, int x)
{
    int tabh = table_.size();
    int tabw = table_[0].size();
    bool root = false;
    if (y == -1 || x == -1) {
        y = rand() % tabh;
        x = rand() % tabw;
        root = true;
    }
    table_[y][x].visited = true;
    // Generates number between 1 and 15, that consists of 4 bits.
    // Every bit is interpreted as bool value. If it is true, the room has
    // corresponding passage, leading up, right, down or left.
    char d = rand() % 15 + 1;
    if (d & 1 && y > 0 && !table_[y][x].top) {
        table_[y][x].top = true;
        table_[y - 1][x].bottom = true;
        if (!table_[y - 1][x].visited) bindRooms(y - 1, x);
    }
    if (d & 2 && x < tabw - 1 && !table_[y][x].right) {
        table_[y][x].right = true;
        table_[y][x + 1].left = true;
        if (!table_[y][x + 1].visited) bindRooms(y, x + 1);
    }
    if (d & 4 && y < tabh - 1 && !table_[y][x].bottom) {
        table_[y][x].bottom = true;
        table_[y + 1][x].top = true;
        if (!table_[y + 1][x].visited) bindRooms(y + 1, x);
    }
    if (d & 8 && x > 0 && !table_[y][x].left) {
        table_[y][x].left = true;
        table_[y][x - 1].right = true;
        if (!table_[y][x - 1].visited) bindRooms(y, x - 1);
    }
    // As control flow returns to the root of recursive function calls,
    // the table must be check for the rooms, that have been left behind
    // by the algorythm. Now check every room in the table for unvisited
    // ones and call the function for them.
    for (y = 0; root && y < tabh; ++y) {
        for (x = 0; x < tabw; ++x) {
            if (!table_[y][x].visited) bindRooms(y, x);
        }
    }
    for (y = 0; root && y < tabh; ++y) {
        for (x = 0; x < tabw; ++x) {
            table_[y][x].visited = false;
        }
    }
}

void Map::LayoutGenerator::makeCoherent()
{
    size_t distance = traverseRooms();
    size_t tabh = table_.size();
    size_t tabw = table_[0].size();
    size_t totald = tabh * tabw;
    // While traverse algorythm don't travel past every room
    // find an unvisited room that has a visited adjacent room
    // connect them
    // try traversing again starting at this room.
    for (size_t y = 0; distance != totald && y < table_.size(); ++y) {
        for (size_t x = 0; distance != totald && x < table_[1].size(); ++x) {
            if (!table_[y][x].visited) {
                if (y > 0 && table_[y - 1][x].visited) {
                    table_[y][x].top = true;
                    table_[y - 1][x].bottom = true;
                    distance += traverseRooms(y, x);
                } else if (x < tabw - 1 && table_[y][x + 1].visited) {
                    table_[y][x].right = true;
                    table_[y][x + 1].left = true;
                    distance += traverseRooms(y, x);
                } else if (y < tabh - 1 && table_[y + 1][x].visited) {
                    table_[y][x].bottom = true;
                    table_[y + 1][x].top = true;
                    distance += traverseRooms(y, x);
                } else if (x > 0 && table_[y][x - 1].visited) {
                    table_[y][x].left = true;
                    table_[y][x - 1].right = true;
                    distance += traverseRooms(y, x);
                }
            }
        }
    }
    for (size_t y = 0; y < tabh; ++y) {
        for (size_t x = 0; x < tabw; ++x) {
            table_[y][x].visited = false;
        }
    }
}

void Map::LayoutGenerator::dumpLayout()
{
    int tabh = table_.size();
    int tabw = table_[0].size();
    for (int y = 0; y < tabh; ++y) {
        for (int x = 0; x < tabw; ++x) {
            if (!table_[y][x].visited) {
                dumpRoom(table_[y][x]);
            }
            if (x < tabw - 1 && table_[y][x].right) {
                if (!table_[y][x + 1].visited) dumpRoom(table_[y][x + 1]);
                dumpPassage(table_[y][x], table_[y][x + 1]);
            }
            if (y < tabh - 1 && table_[y][x].bottom
                    && !table_[y + 1][x].visited) {
                if (!table_[y + 1][x].visited) dumpRoom(table_[y + 1][x]);
                dumpPassage(table_[y][x], table_[y + 1][x]);
            }
        }
    }
}

int Map::LayoutGenerator::traverseRooms(int y, int x)
{
    int distance = 1;
    table_[y][x].visited = true;
    if (table_[y][x].top && !table_[y - 1][x].visited) {
        distance += traverseRooms(y - 1, x);
    }
    if (table_[y][x].right && !table_[y][x + 1].visited) {
        distance += traverseRooms(y, x + 1);
    }
    if (table_[y][x].bottom && !table_[y + 1][x].visited) {
        distance += traverseRooms(y + 1, x);
    }
    if (table_[y][x].left && !table_[y][x - 1].visited) {
        distance += traverseRooms(y, x - 1);
    }
    return distance;
}

void Map::LayoutGenerator::dumpRoom(RectTabField_ &room)
{
    //int minh = room.h / 2 + 1;
    //int minw = room.w / 2 + 1;
    // Actual height and width of room
    int height = (room.h / 2 + 1) + rand() % (room.h - (room.h / 2 + 1));
    int width = (room.w / 2 + 1) + rand() % (room.w - (room.w / 2 + 1));
    // Inner coordinates of room in the rectangle
    int iy = rand() % (room.h - height + 1);
    int ix = rand() % (room.w - width + 1);
    // Replace rectangle data with room data
    room.y = room.y + iy;
    room.x = room.x + ix;
    room.h = height;
    room.w = width;
    // Replace every tile within the room with floor tile
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            grid_[room.y + y][room.x + x].reset(new tile::Tile(tile::kFloor));
        }
    }
    room.visited = true;
}

void Map::LayoutGenerator::dumpPassage(RectTabField_ &first,
                                       RectTabField_ &second)
{
    int y, x, h, w;
    // If the passage is being created horizontaly from left to right
    if (abs(first.x - second.x) > first.w && first.x < second.x) {
        x = first.x + first.w;
        w = second.x - x;
        y = first.y >= second.y ? first.y : second.y;
        int height = (first.y + first.h <= second.y + second.h ?
                    first.y + first.h : second.y + second.h) - y;
        // Randomize the height of the passage
        h = rand() % height + 1;
        // Randomly shift the passage verticaly
        y += rand() % (height - h + 1);
    } else {    // The other way - verticaly from top to bottom
        y = first.y + first.h;
        h = second.y - y;
        x = first.x >= second.x ? first.x : second.x;
        int width = (first.x + first.w <= second.x + second.w ?
                    first.x + first.w : second.x + second.w) - x;
        // Randomize the width of the passage
        w = rand() % width + 1;
        // Randonly shift the passage horizontaly
        x += rand() % (width - w + 1);
    }
    for (int _y = 0; _y < h; ++_y) {
        for (int _x = 0; _x < w; ++_x) {
            grid_[_y + y][_x + x].reset(new tile::Tile(tile::kFloor));
        }
    }
}

}   // namespace map