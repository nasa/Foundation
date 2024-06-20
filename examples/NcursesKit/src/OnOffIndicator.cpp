#include "OnOffIndicator.hh"


namespace
{
    constexpr int BORDER_WIDTH = 1;
    constexpr int BORDER_HEIGHT = 1;
    constexpr int LABEL_HEIGHT = 1;
    constexpr std::string::size_type MIN_WIDTH = (BORDER_WIDTH * 2u);
    constexpr int INDICATOR_ON_COLOR_PAIR = 3;
    constexpr int INDICATOR_OFF_COLOR_PAIR = 1;
}

OnOffIndicator::OnOffIndicator(std::string const& label, ScreenPosition position):
    Widget(position, Dimensions(LABEL_HEIGHT + (BORDER_HEIGHT * 2), std::max(label.size() + MIN_WIDTH, MIN_WIDTH)), true),
    m_label(label),
    m_state(false)
{
    this->refresh();
}


OnOffIndicator::~OnOffIndicator()
{}


void
OnOffIndicator::refresh()
{
    this->setColors();
    mvwprintw(this->window(), BORDER_HEIGHT, BORDER_WIDTH, m_label.c_str());

    Widget::refresh();
}


void
OnOffIndicator::setColors()
{
    if (m_state)
    {
        wattron(this->window(), COLOR_PAIR(INDICATOR_ON_COLOR_PAIR));
    }
    else
    {
        wattron(this->window(), COLOR_PAIR(INDICATOR_OFF_COLOR_PAIR));
    }

    Widget::setColors();
}

// vim: set ts=4 sw=4 expandtab:
