#ifndef _VIRTUAL_CAN_TARGET_SRC_SCREEN_POSITION_HH_
#define _VIRTUAL_CAN_TARGET_SRC_SCREEN_POSITION_HH_

#include <utility>


struct ScreenPosition : public std::pair< int, int >
{
    ScreenPosition() = default;

    ScreenPosition(int row, int col): std::pair< int, int >(row, col) {}

    ScreenPosition(ScreenPosition const& other) = default;

    ScreenPosition(ScreenPosition&& other) = default;

    inline int row() const { return this->first; }

    inline int col() const { return this->second; }

    inline void setRow(int row) { this->first = row; }

    inline void setCol(int col) { this->second = col; }
};

#endif /* !_VIRTUAL_CAN_TARGET_SRC_SCREEN_POSITION_HH_ */

// vim: set ts=4 sw=4 expandtab:
