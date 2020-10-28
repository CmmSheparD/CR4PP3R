/*
 * maps.hh - map library 
 */
#ifndef MAP_MAP_HH
#define MAP_MAP_HH

#include <memory>
#include <vector>

#include "tile.hh"

namespace map {

class Map {
public:
    class MapIterator;

    static bool initMap(size_t width, size_t height);
    static bool destroyMap();
    static Map& getMap();

    ~Map();

    size_t getHeight();
    size_t getWidth();
    std::shared_ptr<tile::Tile> getTile(size_t x, size_t y);

    MapIterator begin();
    MapIterator end();
protected:
    class LayoutGenerator;

    Map(size_t width, size_t height);
    Map(const Map &other);
    Map(Map &&other);
    Map &operator=(const Map &other);

    static std::shared_ptr<Map> instance_;
    // A two dimension matrix of pointers to tiles..
    std::shared_ptr<std::shared_ptr<std::shared_ptr<tile::Tile>[]>[]> grid_;
    size_t width_;
    size_t height_;
};

/*
 * Used to iterate through map's grid using shared pointers. Mind that
 * ``operator->`` returns shared pointer to ``Tile`` not to the grid's
 * shared pointer.
 */
class Map::MapIterator {
public:
    MapIterator(size_t x = 0, size_t y = 0);
    ~MapIterator();
    
    bool operator==(const MapIterator &other);
    bool operator!=(const MapIterator &other);

    std::shared_ptr<tile::Tile> &operator*();
    std::shared_ptr<tile::Tile> operator->();

    MapIterator &operator++();
    MapIterator operator++(int);
protected:
    size_t x_;
    size_t y_;
};

/*
 * Generates a layout of map - walls and floor.
 */
class Map::LayoutGenerator {
public:
    LayoutGenerator(size_t height, size_t width);

    void createLayout();
    std::shared_ptr<std::shared_ptr<std::shared_ptr<tile::Tile>[]>[]> &getLayout();
protected:
    struct RectTabField_ {
        int y;
        int x;
        int h;
        int w;
        bool visited;
        bool top, left, bottom, right;
    };

    void genRectsTable();
    void bindRooms(int y = -1, int x = -1);
    // void bindRoomsSparse();
    void makeCoherent();
    void dumpLayout();

    int traverseRooms(int y = 0, int x = 0);
    void dumpRoom(RectTabField_ &room);
    void dumpPassage(RectTabField_ &first, RectTabField_ &second);

    size_t height_;
    size_t width_;
    std::vector<std::vector<RectTabField_>> table_;
    std::shared_ptr<std::shared_ptr<std::shared_ptr<tile::Tile>[]>[]> grid_;
};

}   // namespace map

#endif  // MAP_MAP_HH