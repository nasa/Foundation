#include "Widget.hh"


Widget::Widget(ScreenPosition position, Dimensions dimensions, bool border):
    m_border(border),
    m_dimensions(dimensions),
    m_position(position),
    m_window(nullptr)
{
    m_window = newwin(dimensions.height(), dimensions.width(), position.row(), position.col());
    this->refresh();
}


Widget::~Widget()
{
    delwin(m_window);
    m_window = nullptr;
}


void
Widget::refresh()
{
    if (m_border)
    {
        box(m_window, 0, 0);
    }
    wrefresh(m_window);
}

// vim: set ts=4 sw=4 expandtab:
