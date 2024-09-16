#include <UserInputSource.h>

#include "UserInputEventDispatch.h"


using CoreKit::InputSource;

void
UserInputEventDispatch::inputAvailableFrom(InputSource *source)
{
    UserInputSource *actual = dynamic_cast< UserInputSource* >(source);
    if (actual != nullptr)
    {
        int userInput = actual->readNext();
        for (auto& aHandler : m_eventHandlers)
        {
            aHandler(userInput);
        }
    }
}

// vim: set ts=4 sw=4 expandtab:
