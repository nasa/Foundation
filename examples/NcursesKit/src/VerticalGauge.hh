#ifndef _VIRTUALCANTARGET_SRC_VERTICAL_GAUGE_HH_
#define _VIRTUALCANTARGET_SRC_VERTICAL_GAUGE_HH_

#include <string>
#include <Widget.hh>


class VerticalGauge : public Widget
{
public:
    VerticalGauge(std::string const& name, ScreenPosition position, int barHeight);

    virtual ~VerticalGauge();

    inline std::string const& name() const { return m_name; }
    
    inline void setValue(int value) { m_value = value; this->refresh(); }

    virtual void refresh() override;

    inline int value() const { return m_value; }

private:
    int m_barHeight;
    std::string m_name;
    int m_value;
};

#endif /* !_VIRTUALCANTARGET_SRC_VERTICAL_GAUGE_HH_ */

// vim: set ts=4 sw=4 expandtab:
