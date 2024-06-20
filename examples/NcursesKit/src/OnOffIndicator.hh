#ifndef _VIRTUAL_CAN_TARGET_SRC_ON_OFF_INDICATOR_HH_
#define _VIRTUAL_CAN_TARGET_SRC_ON_OFF_INDICATOR_HH_

#include <string>
#include <Widget.hh>


class OnOffIndicator : public Widget
{
public:
    OnOffIndicator(std::string const& label, ScreenPosition position);

    virtual ~OnOffIndicator();

    inline std::string const& label() const { return m_label; }
    
    virtual void refresh() override;

    inline bool state() const { return m_state; }

    inline void setState(bool state) { m_state = state; this->refresh(); }
    
protected:
    virtual void setColors() override;

private:
    std::string m_label;
    bool m_state;
};

#endif /* !_VIRTUAL_CAN_TARGET_SRC_ON_OFF_INDICATOR_HH_ */

// vim: set ts=4 sw=4 expandtab:
