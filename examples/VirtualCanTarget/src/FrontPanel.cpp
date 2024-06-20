#include <ncurses.h>
#include <stdexcept>

#include <Identifiers.h>

#include "FrontPanel.h"


namespace
{
    constexpr short HIGHLIGHT_COLOR_PAIR = 3;
    constexpr short IN_FOCUS_COLOR_PAIR = 2;
    constexpr short NOT_IN_FOCUS_COLOR_PAIR = 1;
    constexpr int TAB_KEY = 9;
}

FrontPanel::FrontPanel(CoreKit::Application *const theApp):
    m_controlInFocus(m_navigation.end()),
    m_theApp(theApp)
{
    if (nullptr == theApp)
    {
        throw std::invalid_argument("CoreKit::Application instance must not be null.");
    }
}


void
FrontPanel::initialize(std::shared_ptr< TargetStateRepository > stateRepo)
{
    m_stateRepo = stateRepo;
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

    auto alphaSlider = std::make_shared< VerticalSlider >(" ALPHA ", ScreenPosition(1, 1), 15);
    m_verticalSliders[VctIdentifiers::ALPHA] = alphaSlider;
    auto bravoSlider = std::make_shared< VerticalSlider >(" BRAVO ", ScreenPosition(1, 11), 15);
    m_verticalSliders[VctIdentifiers::BRAVO] = bravoSlider;
    auto charlieSlider = std::make_shared< VerticalSlider >("CHARLIE", ScreenPosition(1, 21), 15);
    m_verticalSliders[VctIdentifiers::CHARLIE] = charlieSlider;

    auto sw0Indicator = std::make_shared< OnOffIndicator >("SW0", ScreenPosition(1, 31));
    m_onOffIndicators[VctIdentifiers::SW0] = sw0Indicator;
    auto sw1Indicator = std::make_shared< OnOffIndicator >("SW1", ScreenPosition(6, 31));
    m_onOffIndicators[VctIdentifiers::SW1] = sw1Indicator;
    auto sw2Indicator = std::make_shared< OnOffIndicator >("SW2", ScreenPosition(11, 31));
    m_onOffIndicators[VctIdentifiers::SW2] = sw2Indicator;

    m_navigation.push_back(alphaSlider);
    m_navigation.push_back(bravoSlider);
    m_navigation.push_back(charlieSlider);
    m_controlInFocus = m_navigation.begin();
    (*m_controlInFocus)->gainFocus();
}


void
FrontPanel::onTargetStateUpdate(TargetStateInformation const& changes)
{
    for (auto const& aChange : changes)
    {
        auto indicatorIter = m_onOffIndicators.find(aChange.first);
        if (indicatorIter != m_onOffIndicators.end())
        {
            (*indicatorIter).second->setState(aChange.second == 0xFF);
        }
    }
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
        (*m_controlInFocus)->handleInput(input);
        for (auto const& aSlider : m_verticalSliders)
        {
            m_stateRepo->set(aSlider.first, aSlider.second->value());
        }
    }
}


void
FrontPanel::terminate()
{
    m_navigation.clear();
    m_onOffIndicators.clear();
    m_verticalSliders.clear();
    endwin();
    m_stateRepo.reset();
}

// vim: set ts=4 sw=4 expandtab:
