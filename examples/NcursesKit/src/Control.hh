#ifndef _VIRTUAL_CAN_TARGET_SRC_CONTROL_HH_
#define _VIRTUAL_CAN_TARGET_SRC_CONTROL_HH_

#include <string>

#include <FocusReceiver.hh>
#include <InputReceiver.hh>
#include <Widget.hh>


class Control : public Widget, public FocusReceiver, public InputReceiver
{
public:
    Control(std::string const& name, ScreenPosition position, Dimensions dimensions, bool border);

    virtual ~Control();

    virtual void gainFocus() override;

    inline bool haveFocus() const { return m_haveFocus; }

    virtual void loseFocus() override;

    inline std::string const& name() const { return m_name; }

protected:

    virtual void setColors() override;
    
private:
    std::string m_name;
    bool m_haveFocus;
};

#endif /* !_VIRTUAL_CAN_TARGET_SRC_CONTROL_HH_ */

// vim: set ts=4 sw=4 expandtab:
