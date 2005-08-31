/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ufo_debug.cpp
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

#include "ufo/ufo_global.hpp"

#include <cstring>
#include <cstdarg>
#include <cstdio> // for vsnprintf

ufo::UDebugStream &
ufo::UDebugStream::printf(const char *format, ...) {
	char buffer[4096];
	va_list arguments;
	va_start(arguments, format);
	vsprintf(buffer, format, arguments);
	va_end(arguments);
	*this << buffer;
	return *this;
}

ufo::UPrintStream &
ufo::UPrintStream::printf(const char *format, ...) {
	char buffer[4096];
	va_list arguments;
	va_start(arguments, format);
	vsprintf(buffer, format, arguments);
	va_end(arguments);
	*this << buffer;
	return *this;
}

static std::streambuf * debugBuffer = NULL;
static std::streambuf * warningBuffer = NULL;
static std::streambuf * errorBuffer = NULL;
static std::streambuf * fatalBuffer = NULL;

void ufo::initUFODebug(std::streambuf * debug, std::streambuf * warning,
		std::streambuf * error, std::streambuf * fatal) {
	debugBuffer = debug;
	warningBuffer = warning;
	errorBuffer = error;
	fatalBuffer = fatal;
}

ufo::UDebugStream
ufo::uDebug() {
	if (debugBuffer) {
		return ufo::UDebugStream(debugBuffer);
	} else {
		return ufo::UDebugStream(std::cerr.rdbuf());
	}
}

ufo::UPrintStream
ufo::uError() {
	if (errorBuffer) {
		ufo::UPrintStream dbg(errorBuffer);
		dbg << " :: ERROR ";
		return dbg;
	} else {
		ufo::UPrintStream dbg(std::cerr.rdbuf());
		dbg << " :: ERROR ";
		return dbg;
	}
}

ufo::UPrintStream
ufo::uWarning() {
	if (warningBuffer) {
		ufo::UPrintStream dbg(warningBuffer);
		dbg << " :: WARNING ";
		return dbg;
	} else {
		ufo::UPrintStream dbg(std::cerr.rdbuf());
		dbg << " :: WARNING ";
		return dbg;
	}
}

ufo::UPrintStream
ufo::uFatal() {
	if (fatalBuffer) {
		ufo::UPrintStream dbg(fatalBuffer);
		dbg << " :: FATAL ERROR ";
		return dbg;
	} else {
		ufo::UPrintStream dbg(std::cerr.rdbuf());
		dbg << " :: FATAL ERROR ";
		return dbg;
	}
}

#ifdef HAVE_EXECINFO_H
#include <execinfo.h>

std::string
ufo::uBacktrace(int levels) {
	UStringStream stream;
	void* trace[256];
	int n = backtrace(trace, 256);
	char** strings = backtrace_symbols(trace, n);

	if ( levels != -1 ) {
		n = std::min(n, levels);
	}
	stream << "[\n";

	for (int i = 0; i < n; ++i) {
		stream << i << ": " << strings[i] << "\n";
	}
	stream << "]\n";
	
	free (strings);
	
	return stream.str();
}


#else // HAVE_BACKTRACE

std::string
ufo::uBacktrace(int levels) {
	return "";
}
#endif // HAVE_BACKTRACE

std::string
ufo::uBacktrace() {
	return uBacktrace(-1);
}
