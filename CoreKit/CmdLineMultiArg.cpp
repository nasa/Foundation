/**
 * \file CmdLineMultiArg.cpp
 * \brief Contains the implementation of the \c CoreKit::CmdLineMultiArg class.
 * \date 2016-06-19 10:47:11
 * \author Rolando J. Nieves
 */

#include <cstddef>

#include "CmdLineMultiArg.h"

using std::size_t;
using std::string;

namespace CoreKit
{

CmdLineMultiArg::CmdLineMultiArg(string const& argListString)
: argList(m_argList)
{
    size_t startPos = 0u;
    size_t endPos = argListString.find_first_of(',');
    while (endPos != string::npos)
    {
        m_argList.push_back(argListString.substr(startPos, (endPos - startPos)));
        startPos = endPos + 1u;
        endPos = argListString.find_first_of(',', startPos);
    }

    if (startPos < argListString.length())
    {
        m_argList.push_back(argListString.substr(startPos));
    }
}

} // namespace CoreKit

// vim: set ts=4 sw=4 expandtab:
