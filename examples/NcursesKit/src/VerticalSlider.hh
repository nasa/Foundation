#ifndef _VIRTUAL_CAN_TARGET_SRC_VERTICAL_SLIDER_HH_
#define _VIRTUAL_CAN_TARGET_SRC_VERTICAL_SLIDER_HH_

#include <Control.hh>


class VerticalSlider : public Control
{
public:
    VerticalSlider(std::string const& name, ScreenPosition position, int barHeight);

    virtual ~VerticalSlider();

    virtual void handleInput(int input) override;

    inline void setValue(int value) { m_value = value; this->refresh(); }

    virtual void refresh() override;

    inline int value() const { return m_value; }

private:
    int m_barHeight;
    int m_value;
};

#endif /* !_VIRTUAL_CAN_TARGET_SRC_VERTICAL_SLIDER_HH_ */

// vim: set ts=4 sw=4 expandtab:
