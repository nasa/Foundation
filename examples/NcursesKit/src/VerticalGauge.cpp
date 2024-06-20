#include <sstream>

#include "VerticalGauge.hh"


// Assuming: minimum value = 0; maximum value = 100
// All these could get grouped into a style/config object later
namespace
{
    constexpr int MIN_VAL = 0;
    constexpr int MAX_VAL = 100;
    constexpr std::string::size_type MIN_WIDTH = 5u;
    constexpr int BORDER_HEIGHT = 1;
    constexpr int BORDER_WIDTH = 1;
    constexpr int LABEL_HEIGHT = 1;
    constexpr int VALUE_HEIGHT = 1;
    constexpr int STD_STEP = 1;
    constexpr int BIG_STEP = 5;
}

VerticalGauge::VerticalGauge(std::string const& name, ScreenPosition position, int barHeight):
    Widget(
        position,
        Dimensions(
            (barHeight + (BORDER_HEIGHT * 2) + LABEL_HEIGHT + VALUE_HEIGHT),
            std::max(MIN_WIDTH, name.size() + (BORDER_WIDTH * 2))
        ),
        true
    ),
    m_barHeight(barHeight),
    m_name(name),
    m_value(MIN_VAL)
{
    this->refresh();
}


VerticalGauge::~VerticalGauge()
{}


void
VerticalGauge::refresh()
{
    constexpr int BAR_START_OFFSET = (BORDER_HEIGHT + LABEL_HEIGHT);

    this->setColors();

    int barFillStart = (m_value * m_barHeight) / MAX_VAL;
    barFillStart = std::min(barFillStart, m_barHeight);
    std::string barSliceContent(this->dimensions().width() - (BORDER_WIDTH * 2), ' ');
    mvwprintw(this->window(), BORDER_HEIGHT, BORDER_WIDTH, m_name.c_str());
    for (int barSlice = 0; barSlice < m_barHeight; barSlice++)
    {
        if ((m_barHeight - barSlice) <= barFillStart)
        {
            wattron(this->window(), A_REVERSE);
        }
        else
        {
            wattroff(this->window(), A_REVERSE);
        }
        mvwprintw(this->window(), BAR_START_OFFSET + barSlice, BORDER_WIDTH, barSliceContent.c_str());
        wattroff(this->window(), A_REVERSE);
    }

    std::stringstream valFormat;
    valFormat << "%" << (this->dimensions().width() - (BORDER_WIDTH * 2)) << "d";
    mvwprintw(this->window(), BAR_START_OFFSET + m_barHeight, BORDER_WIDTH, valFormat.str().c_str(), m_value);

    Widget::refresh();
}

// vim: set ts=4 sw=4 expandtab:
