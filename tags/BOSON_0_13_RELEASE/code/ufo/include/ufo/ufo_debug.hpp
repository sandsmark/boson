/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ufo_debug.hpp
    begin             : Sun Feb 23 2003
    $Id$
 ***************************************************************************/

/***************************************************************************
 *  This library is free software; you can redistribute it and/or          *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#ifndef UFO_DEBUG_HPP
#define UFO_DEBUG_HPP


//
// debug and error functions
//

//
// macro magic
//

#ifndef UFO_ASSERT
#ifdef UFO_CHECK_ASSERT
#define UFO_ASSERT(x)  ((x) ? (void)0 : uWarning() << "ASSERT: " << #x << " in " << UFO_LINE_INFO;
#else
#define UFO_ASSERT(x)
#endif // UFO_CHECK_ASSERT
#endif // UFO_ASSERT


// line info 
#define UFO_LINE_INFO "[" << __FILE__ << ":" << __LINE__ << "] "


#include <iostream>
#include <vector>

#include "usharedptr.hpp"

namespace ufo {

/** The UDebugStream is a text stream for outputting error messages.
  * It has to be initialized with a stream buffer, like std::cerr.rdbuf() or a
  * string stream buffer.
  * This class typically isn't used directly but by
  * @ref uDebug() @ref uWarning(), @ref uError() or @ref uFatal
  */
class UDebugStream {
public: // typedefs
	//typedef std::ostream::__ios_type ios_type__;
	typedef std::ostream ostream_type;
	USharedPtr<ostream_type> m_stream;
public:
	UDebugStream(std::streambuf * buf) : m_stream(new ostream_type(buf)) {}

#ifdef UFO_DEBUG
	UDebugStream &
	operator<<(ostream_type & (*pf)(ostream_type &)) {
			*m_stream << (pf);
		return *this;
	}
#else
	UDebugStream &
	operator<<(ostream_type & (* /* pf */)(ostream_type &)) {
		return *this;
	}
#endif

	template<typename T>
	UDebugStream &
	operator<<(const T & t) {
#ifdef UFO_DEBUG
			*m_stream << (t);
#endif
		return *this;
	}

	/**
	 * Prints the string @p format which can contain
	 * printf-style formatted values.
	 * @param format the printf-style format
	 * @return this stream
	 */
	UFO_EXPORT UDebugStream &
	printf(const char *format, ...);
};

/** The UPrintStream is a text stream for outputting error messages.
  * In difference to UDebugStream, this error messages are always printed,
  * not only in debug mode.
  */
class UPrintStream {
public: // typedefs
	typedef std::ostream ostream_type;
	USharedPtr<ostream_type> m_stream;
public:
	UPrintStream(std::streambuf * buf) : m_stream(new ostream_type(buf)) {}

	UPrintStream &
	operator<<(ostream_type & (*pf)(ostream_type &)) {
		*m_stream << (pf);
		return *this;
	}

	template<typename T>
	UPrintStream &
	operator<<(const T & t) {
		*m_stream << (t);
		return *this;
	}

	UFO_EXPORT UPrintStream &
	printf(const char *format, ...);
};

//
// returns debug streams
//

/** Returns a debug stream which shouldn't appear in production code. */
extern UFO_EXPORT UDebugStream uDebug();

//
// debug streams which appear also in production code
//
extern UFO_EXPORT UPrintStream uError();
extern UFO_EXPORT UPrintStream uWarning();
extern UFO_EXPORT UPrintStream uFatal();

//extern UFO_EXPORT std::vector<std::string> uStackTrace(int maxLevels);
//extern UFO_EXPORT std::vector<std::string> uStackTrace();

// backwards compatibility
extern UFO_EXPORT std::string uBacktrace(int levels);
extern UFO_EXPORT std::string uBacktrace();

//
// FIXME !
//

/** Initializes and redirects ufo debug streams.
  * This init method may be omitted.
  */
extern UFO_EXPORT void initUFODebug(std::streambuf * debug, std::streambuf * warning,
	std::streambuf * error, std::streambuf * fatal);


} // namespace ufo


#endif // UFO_DEBUG_HPP
