#ifndef _VIRTUAL_CAN_TARGET_SRC_FRONT_PANEL_H_
#define _VIRTUAL_CAN_TARGET_SRC_FRONT_PANEL_H_

#include <map>
#include <memory>
#include <vector>

#include <VerticalSlider.hh>
#include <OnOffIndicator.hh>

#include <TargetStateRepository.h>
#include <UserInputSource.h>


class FrontPanel
{
public:
    FrontPanel(CoreKit::Application *const theApp);

    FrontPanel(FrontPanel const& other) = delete;

    FrontPanel(FrontPanel&& other) = delete;

    virtual ~FrontPanel()
    { this->terminate(); }

    void initialize(std::shared_ptr< TargetStateRepository > stateRepo);

    void onTargetStateUpdate(TargetStateInformation const& changes);

    void onUserInput(int input);

    void terminate();

    FrontPanel& operator=(FrontPanel const& other) = delete;

    FrontPanel& operator=(FrontPanel&& other) = delete;

private:
    using ControlPtr = std::shared_ptr< Control >;
    using NavigationList = std::vector< ControlPtr >;
    using OnOffIndicatorPtr = std::shared_ptr< OnOffIndicator >;
    using OnOffIndicatorPtrMap = std::map< canid_t, OnOffIndicatorPtr >;
    using VerticalSliderPtr = std::shared_ptr< VerticalSlider >;
    using VerticalSliderPtrMap = std::map< canid_t, VerticalSliderPtr >;

    NavigationList m_navigation;
    NavigationList::const_iterator m_controlInFocus;
    OnOffIndicatorPtrMap m_onOffIndicators;
    CoreKit::Application *const m_theApp;
    std::shared_ptr< TargetStateRepository > m_stateRepo;
    VerticalSliderPtrMap m_verticalSliders;
};

#endif /* !_VIRTUAL_CAN_TARGET_SRC_FRONT_PANEL_H_ */

// vim: set ts=4 sw=4 expandtab:
