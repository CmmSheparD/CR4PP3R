/*
 * tile.hh - map tile classes
 * Defines tile class for the map:
 * - Tile - a class that stores info about a map tile.
 */
#ifndef MAP_TILES_HH
#define MAP_TILES_HH

namespace map {
namespace tile {

enum TileType {kWall, kFloor, kExit, kEntrance};

class Tile {
public:
    Tile(TileType type = kWall);
    Tile(const Tile &other);
    ~Tile();

    TileType getType();
    bool isPassable();
protected:
    TileType type_;
};

}   // namespace tile
}   // namespace map

#endif // MAP_TILES_HH