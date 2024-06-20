#include <sstream>

#include "VerticalSlider.hh"

// Assuming: minimum value = 0; maximum value = 100
// All these could get grouped into a style/config object later
namespace
{
    constexpr int MIN_VAL = 0;
    constexpr int MAX_VAL = 100;
    constexpr short HIGHLIGHT_COLOR_PAIR = 3;
    constexpr std::string::size_type MIN_WIDTH = 5u;
    constexpr int BORDER_HEIGHT = 1;
    constexpr int BORDER_WIDTH = 1;
    constexpr int TITLE_HEIGHT = 1;
    constexpr int VALUE_HEIGHT = 1;
    constexpr int STD_STEP = 1;
    constexpr int BIG_STEP = 5;
}

VerticalSlider::VerticalSlider(std::string const& name, ScreenPosition position, int barHeight):
    Control(
        name,
        position,
        Dimensions(
            barHeight + (BORDER_HEIGHT * 2) + TITLE_HEIGHT + VALUE_HEIGHT,
            std::max(
                name.size() + (BORDER_WIDTH * 2),
                MIN_WIDTH
            )
        ),
        true
    ),
    m_barHeight(barHeight),
    m_value(MIN_VAL)
{
    this->refresh();
}


VerticalSlider::~VerticalSlider()
{}


void
VerticalSlider::handleInput(int input)
{
    if (KEY_UP == input)
    {
        m_value = std::min(m_value + STD_STEP, MAX_VAL);
    }
    else if (KEY_DOWN == input)
    {
        m_value = std::max(m_value - STD_STEP, MIN_VAL);
    }
    else if (KEY_NPAGE == input)
    {
        m_value = std::max(m_value - BIG_STEP, MIN_VAL);
    }
    else if (KEY_PPAGE == input)
    {
        m_value = std::min(m_value + BIG_STEP, MAX_VAL);
    }
    this->refresh();
}


void
VerticalSlider::refresh()
{
    constexpr int BAR_START_OFFSET = (BORDER_HEIGHT + TITLE_HEIGHT);

    attr_t prevAttrs;
    short prevColor;

    this->setColors();
    wattr_get(this->window(), &prevAttrs, &prevColor, nullptr);

    int barFillStart = (m_value * m_barHeight) / MAX_VAL;
    barFillStart = std::min(barFillStart, m_barHeight);
    std::string barSliceContent(this->dimensions().width() - (BORDER_WIDTH * 2), ' ');
    mvwprintw(this->window(), BORDER_HEIGHT, BORDER_WIDTH, this->name().c_str());
    if (this->haveFocus())
    {
        wattron(this->window(), COLOR_PAIR(HIGHLIGHT_COLOR_PAIR));
    }
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
    }

    wattr_set(this->window(), prevAttrs, prevColor, nullptr);

    std::stringstream valFormat;
    valFormat << "%" << (this->dimensions().width() - (BORDER_WIDTH * 2)) << "d";
    mvwprintw(this->window(), BAR_START_OFFSET + m_barHeight, BORDER_WIDTH, valFormat.str().c_str(), m_value);

    Control::refresh();
}

// vim: set ts=4 sw=4 expandtab:
