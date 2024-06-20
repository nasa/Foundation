/**
 * \file CmdLineMultiArg.h
 * \brief Contains the definition of the \c CmdLineMultiArg class.
 * \date 2016-06-19 10:38:45
 * \author Rolando J. Nieves
 */

#ifndef _COREKIT_CMD_LINE_MULTI_ARGS_H_
#define _COREKIT_CMD_LINE_MULTI_ARGS_H_

#include <string>
#include <vector>

namespace CoreKit
{

/**
 * \brief Models a collection of arguments associated with a single command-line flag.
 *
 * The list of arguments is assumed to arrive as a comma-separated amalgam. The
 * class makes no attempt to decode the arguments into a more native form,
 * instead leaving them as character strings.
 */
struct CmdLineMultiArg
{
    /**
     * \brief Constant reference to broken out list of arguments.
     *
     * This reference serves as an accessor to the private field that actually
     * holds the broken out arguments.
     */
    std::vector< std::string > const& argList;

    /**
     * \brief Break out arguments from the single comma-separated string.
     *
     * \param[in] argListString - String with all comma-separated, encoded
     *            arguments.
     */
    explicit CmdLineMultiArg(std::string const& argListString);

private:
    std::vector< std::string > m_argList;
};

} // namespace CoreKit

#endif /* !_COREKIT_CMD_LINE_MULTI_ARGS_H_ */

// vim: set ts=4 sw=4 expandtab:
