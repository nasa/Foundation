#ifndef _VIRTUAL_CAN_TARGET_SRC_FRONT_PANEL_H_
#define _VIRTUAL_CAN_TARGET_SRC_FRONT_PANEL_H_

#include <map>
#include <memory>
#include <vector>

#include <CoreKit/CoreKit.h>

#include <MyIsoSoftwareBus.h>

#include <CommandDispatch.h>
#include <VerticalGauge.hh>
#include <ToggleButton.hh>


class FrontPanel
{
public:
    FrontPanel(CoreKit::Application *const theApp);

    FrontPanel(FrontPanel const& other) = delete;

    FrontPanel(FrontPanel&& other) = delete;

    virtual ~FrontPanel()
    { this->terminate(); }

    void initialize(std::shared_ptr< CommandDispatch > commandDispatch);

    void onEngineeringData(MyIsoSoftwareBus::EngineeringData const& engrData);

    void onUserInput(int input);

    void terminate();

    FrontPanel& operator=(FrontPanel const& other) = delete;

    FrontPanel& operator=(FrontPanel&& other) = delete;

private:
    using ControlPtr = std::shared_ptr< Control >;
    using NavigationList = std::vector< ControlPtr >;
    using ToggleButtonPtr = std::shared_ptr< ToggleButton >;
    using ToggleButtonPtrMap = std::map< ToggleButtonPtr, std::string >;
    using VerticalGaugePtr = std::shared_ptr< VerticalGauge >;
    using VerticalGaugePtrMap = std::map< std::string, VerticalGaugePtr >;

    std::shared_ptr< CommandDispatch > m_commandDispatch;
    NavigationList::iterator m_controlInFocus;
    NavigationList m_navigation;
    CoreKit::Application *const m_theApp;
    ToggleButtonPtrMap m_toggleButtons;
    VerticalGaugePtrMap m_verticalGauges;
};

#endif /* !_VIRTUAL_CAN_TARGET_SRC_FRONT_PANEL_H_ */

// vim: set ts=4 sw=4 expandtab:
