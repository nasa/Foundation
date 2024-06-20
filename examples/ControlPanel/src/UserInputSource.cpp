#include <ncurses.h>

#include "UserInputSource.h"


UserInputSource::UserInputSource(std::shared_ptr< UserInputEventDispatch > dispatch):
    m_dispatch(dispatch)
{}


int
UserInputSource::readNext()
{
    int result = 0;

    nodelay(stdscr, TRUE);
    result = getch();
    nodelay(stdscr, FALSE);

    return result;
}

// vim: set ts=4 sw=4 expandtab:
