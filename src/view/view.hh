/*
 * view.hh - general graphics header
 */
#ifndef VIEW_VIEW_HH
#define VIEW_VIEW_HH

#include <ncurses.h>

#include <vector>

#include "../map/map.hh"

namespace view {


class Console {
public:
    Console();
    virtual ~Console();

    virtual void initDisplay();

    virtual void update();
protected:
    size_t map_width_;
    size_t map_height_;
    std::vector<std::vector<char>> map_;
};

}   // namespace view

#endif  // VIEW_VIEW_HH