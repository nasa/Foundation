/**
 * \file InputSource.h
 * \brief Definition of the InputSource Class
 * \date 2012-09-24 08:39:57
 * \author Rolando J. Nieves
 */
#if !defined(EA_76480D69_0A8C_42b9_BE14_C8681CB7F96D__INCLUDED_)
#define EA_76480D69_0A8C_42b9_BE14_C8681CB7F96D__INCLUDED_

#include <stdint.h>

#include "InterruptListener.h"

namespace CoreKit
{
	/**
	 * \brief Interface that Defines a \c RunLoop Input Source
	 *
	 * The \c CoreKit::InputSource class defines the interface that \c RunLoop
	 * compatible input sources must implement.\par
	 *
	 * An \c InputSource integrates into a \c RunLoop by providing a file
	 * descriptor that must show activity whenever the \c InputSource requires
	 * attention.
	 *
	 * \author Rolando J. Nieves
	 * \date 20-Aug-2012 2:37:51 PM
	 */
	class InputSource
	{

	public:
		/**
		 * \brief Default Constructor for \c InputSource
		 */
		InputSource();
		/**
		 * \brief Constructor for \c InputSource that Specifies a Relative Priority
		 *
		 * The constructor for the \c InputSource interface that accepts an
		 * integer as input gives callers the ability to apply a relative
		 * priority to this input source.
		 *
		 * \param relativePriority Value between 0 and 255 defining the
		 *                         priority of this InputSource with respect to
		 *                         other input sources. The lower the value the
		 *                         higher the priority of the input source.
		 */
		InputSource(uint8_t relativePriority);
		/**
		 * \brief End of Destructor Chain for \c InputSource Derived Classes
		 */
		virtual ~InputSource();

		/**
		 * \brief Provide the Relative Priority of this Input Source
		 */
		inline uint8_t relativePriority() const { return m_relativePriority; }
		/**
		 * \brief Obtain the File Descriptor Associated with this \c InputSource
		 *
		 * \return File descriptor associated with this input source.
		 */
		virtual int fileDescriptor() const;
		/**
		 * \brief Obtain the \c InterruptListener Associated with this \c InputSource
		 *
		 * \return \c InterruptListener that will receive event callbacks
		 *         associated with this input source.
		 */
		virtual InterruptListener* interruptListener() const;
		/**
		 * \brief Execute the Appropriate \c InterruptListener Callback in Response to Activity
		 */
		virtual void fireCallback();

	private:
		static uint8_t NextDefaultPriority;
		uint8_t m_relativePriority;
	};

}
#endif // !defined(EA_76480D69_0A8C_42b9_BE14_C8681CB7F96D__INCLUDED_)
