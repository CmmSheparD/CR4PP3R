#include "tile.hh"

namespace map::tile {

Tile::Tile(TileType type) : type_(type) {}

Tile::Tile(const Tile &other)
{
    type_ = other.type_;
}

Tile::~Tile() {}

TileType Tile::getType()
{
    return type_;
}

}   // namespace tiles