#include "Control.hh"


namespace
{
    constexpr int IN_FOCUS_COLOR_PAIR = 2;
    constexpr int NOT_IN_FOCUS_COLOR_PAIR = 1;
}

Control::Control(std::string const& name, ScreenPosition position, Dimensions dimensions, bool border):
    Widget(position, dimensions, border),
    m_haveFocus(false),
    m_name(name)
{}


Control::~Control()
{}


void
Control::gainFocus()
{
    m_haveFocus = true;
    this->refresh();
}


void
Control::loseFocus()
{
    m_haveFocus = false;
    this->refresh();
}


void
Control::setColors()
{
    if (m_haveFocus)
    {
        wattron(this->window(), COLOR_PAIR(IN_FOCUS_COLOR_PAIR));
    }
    else
    {
        wattron(this->window(), COLOR_PAIR(NOT_IN_FOCUS_COLOR_PAIR));
    }

    Widget::setColors();
}

// vim: set ts=4 sw=4 expandtab:
