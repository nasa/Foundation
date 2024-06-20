#include <sstream>

#include "ToggleButton.hh"


namespace
{
    constexpr int BORDER_HEIGHT = 1;
    constexpr int BORDER_WIDTH = 1;
    constexpr int TITLE_HEIGHT = 1;
    const std::string ON_TEXT("ON");
    const std::string OFF_TEXT("OFF");
    constexpr int VALUE_WIDTH = 5;
    constexpr int VALUE_HEIGHT = 3;
    constexpr int HIGHLIGHT_COLOR_PAIR = 3;

    std::string centerText(std::string const& content, std::string::size_type totalWidth)
    {
        std::string::size_type leadingSpace = (totalWidth - content.size()) / 2;
        std::string::size_type trailingSpace = totalWidth - content.size() - leadingSpace;

        std::stringstream result;
        result << std::string(leadingSpace, ' ') << content << std::string(trailingSpace, ' ');

        return result.str();
    }
}

ToggleButton::ToggleButton(std::string const& name, ScreenPosition position):
    Control(name, position, Dimensions((BORDER_HEIGHT * 2) + TITLE_HEIGHT + VALUE_HEIGHT, (BORDER_WIDTH * 2) + std::max((VALUE_WIDTH * 2), static_cast< int >(name.size()))), true),
    m_state(false)
{
    this->refresh();
}


ToggleButton::~ToggleButton()
{}


void
ToggleButton::handleInput(int input)
{
    if (input == ' ')
    {
        this->setState(!this->m_state);
    }
}


void
ToggleButton::refresh()
{
    constexpr int VALUE_OFFSET = BORDER_HEIGHT + TITLE_HEIGHT;
    std::string valuePad(static_cast< std::string::size_type >(VALUE_WIDTH), ' ');
    bool textPrinted = false;
    attr_t prevAttr;
    short prevColor;

    this->setColors();

    std::string::size_type labelAreaSize = this->dimensions().width() - (BORDER_WIDTH * 2);
    mvwprintw(this->window(), BORDER_HEIGHT, BORDER_WIDTH, centerText(this->name(), labelAreaSize).c_str());

    wattr_get(this->window(), &prevAttr, &prevColor, nullptr);

    // OFF indicator
    if (!m_state)
    {
        wattron(this->window(), A_REVERSE);
        if (this->haveFocus())
        {
            wattron(this->window(), COLOR_PAIR(HIGHLIGHT_COLOR_PAIR));
        }
    }
    for (int slice = 0; slice < VALUE_HEIGHT; slice++)
    {
        if (!textPrinted && (slice >= (VALUE_HEIGHT / 2)))
        {
            mvwprintw(this->window(), VALUE_OFFSET + slice, BORDER_WIDTH, centerText(OFF_TEXT, VALUE_WIDTH).c_str());
            textPrinted = true;
        }
        else
        {
            mvwprintw(this->window(), VALUE_OFFSET + slice, BORDER_WIDTH, valuePad.c_str());
        }
    }
    
    wattr_set(this->window(), prevAttr, prevColor, nullptr);

    // ON indicator
    textPrinted = false;
    if (m_state)
    {
        wattron(this->window(), A_REVERSE);
        if (this->haveFocus())
        {
            wattron(this->window(), COLOR_PAIR(HIGHLIGHT_COLOR_PAIR));
        }
    }
    for (int slice = 0; slice < VALUE_HEIGHT; slice++)
    {
        if (!textPrinted && (slice >= (VALUE_HEIGHT / 2)))
        {
            mvwprintw(this->window(), VALUE_OFFSET + slice, BORDER_WIDTH + VALUE_WIDTH, centerText(ON_TEXT, VALUE_WIDTH).c_str());
            textPrinted = true;
        }
        else
        {
            mvwprintw(this->window(), VALUE_OFFSET + slice, BORDER_WIDTH + VALUE_WIDTH, valuePad.c_str());
        }
    }
    
    wattr_set(this->window(), prevAttr, prevColor, nullptr);

    Control::refresh();
}

// vim: set ts=4 sw=4 expandtab:
