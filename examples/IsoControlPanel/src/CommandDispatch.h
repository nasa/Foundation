#ifndef _VIRTUAL_CAN_TARGET_SRC_COMMAND_DISPATCH_H_
#define _VIRTUAL_CAN_TARGET_SRC_COMMAND_DISPATCH_H_

#include <functional>
#include <memory>
#include <queue>
#include <vector>

#include <CanBusKit/CanBusKit.h>


class CommandDispatch : public CoreKit::InterruptListener
{
public:
    CommandDispatch() = default;

    CommandDispatch(CommandDispatch const& other) = delete;
    
    CommandDispatch(CommandDispatch&& other) = delete;

    virtual ~CommandDispatch() = default;

    template< typename CommandHandler >
    void addCommandHandler(CommandHandler&& handler)
    { m_handlers.emplace_back(std::forward< CommandHandler >(handler)); }

    void initialize(std::shared_ptr< CoreKit::EventInputSource > cmdEvent);

    virtual void inputAvailableFrom(CoreKit::InputSource *source) override;

    void post(std::string const& target, std::string const& action);

    void terminate();

    CommandDispatch& operator=(CommandDispatch const& other) = delete;

    CommandDispatch& operator=(CommandDispatch&& other) = delete;
    
private:
    std::shared_ptr< CoreKit::EventInputSource > m_cmdEvent;
    std::queue< std::string > m_cmdQueue;
    std::vector< std::function< void (std::string) > > m_handlers;
};

#endif /* !_VIRTUAL_CAN_TARGET_SRC_COMMAND_DISPATCH_H_ */

// vim: set ts=4 sw=4 expandtab:
