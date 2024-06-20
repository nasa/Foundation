#ifndef _VIRTUAL_CAN_TARGET_SRC_WIDGET_HH_
#define _VIRTUAL_CAN_TARGET_SRC_WIDGET_HH_

#include <ncurses.h>

#include <Dimensions.hh>
#include <ScreenPosition.hh>


class Widget
{
public:
    Widget(ScreenPosition position, Dimensions dimensions, bool border);

    virtual ~Widget();

    inline Dimensions const& dimensions() const { return m_dimensions; }

    virtual void refresh();
    
protected:
    inline WINDOW* window() { return m_window; }

    virtual void setColors() {}
    
private:
    bool m_border;
    Dimensions m_dimensions;
    ScreenPosition m_position;
    WINDOW *m_window;
};

#endif /* !_VIRTUAL_CAN_TARGET_SRC_WIDGET_HH_ */

// vim: set ts=4 sw=4 expandtab:
