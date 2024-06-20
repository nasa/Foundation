#ifndef _VIRTUAL_CAN_TARGET_SRC_DIMENSIONS_HH_
#define _VIRTUAL_CAN_TARGET_SRC_DIMENSIONS_HH_

#include <utility>


struct Dimensions : public std::pair< int, int >
{
    Dimensions() = default;

    Dimensions(int height, int width): std::pair< int, int >(height, width) {}

    Dimensions(Dimensions const& other) = default;

    Dimensions(Dimensions&& other) = default;

    inline int height() const { return this->first; }

    inline int width() const { return this->second; }

    inline void setHeight(int height) { this->first = height; }

    inline void setWidth(int width) { this->second = width; }
};

#endif /* !_VIRTUAL_CAN_TARGET_SRC_DIMENSIONS_HH_ */

// vim: set ts=4 sw=4 expandtab:
