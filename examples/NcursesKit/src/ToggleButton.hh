#ifndef _VIRTUAL_CAN_TARGET_SRC_TOGGLE_BUTTON_HH_
#define _VIRTUAL_CAN_TARGET_SRC_TOGGLE_BUTTON_HH_

#include <Control.hh>


class ToggleButton : public Control
{
public:
    ToggleButton(std::string const& name, ScreenPosition position);

    virtual ~ToggleButton();

    virtual void handleInput(int input) override;

    virtual void refresh() override;

    inline void setState(bool state) { m_state = state; this->refresh(); }
    
    inline bool state() const { return m_state; }

private:
    bool m_state;
};

#endif /* !_VIRTUAL_CAN_TARGET_SRC_TOGGLE_BUTTON_HH_ */

// vim: set ts=4 sw=4 expandtab:
