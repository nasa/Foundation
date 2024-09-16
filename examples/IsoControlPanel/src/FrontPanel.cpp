#include <ncurses.h>
#include <stdexcept>

#include "FrontPanel.h"


using MyIsoSoftwareBus::EngineeringData;

namespace
{
    const std::string ALPHA_NAME("alpha");
    const std::string BRAVO_NAME("bravo");
    const std::string CHARLIE_NAME("charlie");

    const std::string SW0_NAME("sw0");
    const std::string SW1_NAME("sw1");
    const std::string SW2_NAME("sw2");

    constexpr short HIGHLIGHT_COLOR_PAIR = 3;
    constexpr short IN_FOCUS_COLOR_PAIR = 2;
    constexpr short NOT_IN_FOCUS_COLOR_PAIR = 1;
    constexpr int TAB_KEY = 9;
}

FrontPanel::FrontPanel(CoreKit::Application *const theApp):
    m_theApp(theApp)
{
    m_controlInFocus = m_navigation.end();
    if (nullptr == theApp)
    {
        throw std::invalid_argument("CoreKit::Application instance must not be null.");
    }
}


void
FrontPanel::initialize(std::shared_ptr< CommandDispatch > commandDispatch)
{
    m_commandDispatch = commandDispatch;

    initscr();
    start_color();
    cbreak();
    keypad(stdscr, TRUE);

    init_pair(NOT_IN_FOCUS_COLOR_PAIR, COLOR_CYAN, COLOR_BLACK);
    init_pair(IN_FOCUS_COLOR_PAIR, COLOR_BLACK, COLOR_CYAN);
    init_pair(HIGHLIGHT_COLOR_PAIR, COLOR_MAGENTA, COLOR_CYAN);

    attron(COLOR_PAIR(NOT_IN_FOCUS_COLOR_PAIR));
    mvprintw(LINES - 1, 0, "Press [Q] to exit.");
    attroff(COLOR_PAIR(NOT_IN_FOCUS_COLOR_PAIR));
    refresh();

    auto alphaGauge = std::make_shared< VerticalGauge >(" ALPHA ", ScreenPosition(1, 1), 15);
    m_verticalGauges[ALPHA_NAME] = alphaGauge;
    auto bravoGauge = std::make_shared< VerticalGauge >(" BRAVO ", ScreenPosition(1, 11), 15);
    m_verticalGauges[BRAVO_NAME] = bravoGauge;
    auto charlieGauge = std::make_shared< VerticalGauge >("CHARLIE", ScreenPosition(1, 21), 15);
    m_verticalGauges[CHARLIE_NAME] = charlieGauge;

    auto sw0Button = std::make_shared< ToggleButton >("SW0", ScreenPosition(1, 31));
    m_toggleButtons[sw0Button] = SW0_NAME;
    auto sw1Button = std::make_shared< ToggleButton >("SW1", ScreenPosition(8, 31));
    m_toggleButtons[sw1Button] = SW1_NAME;
    auto sw2Button = std::make_shared< ToggleButton >("SW2", ScreenPosition(15, 31));
    m_toggleButtons[sw2Button] = SW2_NAME;

    m_navigation.push_back(sw0Button);
    m_navigation.push_back(sw1Button);
    m_navigation.push_back(sw2Button);
    m_controlInFocus = m_navigation.begin();
    (*m_controlInFocus)->gainFocus();
}


void
FrontPanel::onEngineeringData(EngineeringData const& engrData)
{
    m_verticalGauges[ALPHA_NAME]->setValue(engrData.alpha());
    m_verticalGauges[BRAVO_NAME]->setValue(engrData.bravo());
    m_verticalGauges[CHARLIE_NAME]->setValue(engrData.charlie());
}


void
FrontPanel::onUserInput(int input)
{
    if (TAB_KEY == input)
    {
        (*m_controlInFocus)->loseFocus();
        m_controlInFocus++;
        if (m_navigation.end() == m_controlInFocus)
        {
            m_controlInFocus = m_navigation.begin();
        }
        (*m_controlInFocus)->gainFocus();
    }
    else if (('q' == input) || ('Q' == input))
    {
        m_theApp->requestTermination();
    }
    else
    {
        ToggleButtonPtr theButton(std::static_pointer_cast< ToggleButton >(*m_controlInFocus));

        theButton->handleInput(input);
        if (m_commandDispatch)
        {
            m_commandDispatch->post(m_toggleButtons[theButton], theButton->state() ? "ON" : "OFF");
        }
    }
}


void
FrontPanel::terminate()
{
    m_navigation.clear();
    m_toggleButtons.clear();
    m_verticalGauges.clear();
    endwin();
}

// vim: set ts=4 sw=4 expandtab:
