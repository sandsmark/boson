/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ux/uxtoolkit
    begin             : Mon Jul 26 2004
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

#ifndef UXTOOLKIT_HPP
#define UXTOOLKIT_HPP

#include "../uabstracttoolkit.hpp"

namespace ufo {

/** @short An implementation of a UFO toolkit. Currently for posix and Win32
  * @ingroup native
  *
  * Works with UXContexts and UXFrames.
  *
  * @see UXContext
  * @see UXFrame
  * @author Johannes Schmidt
  */
class UFO_EXPORT UXToolkit : public UAbstractToolkit {
	UFO_DECLARE_DYNAMIC_CLASS(UXToolkit)
public:
	UXToolkit();
	UXToolkit(UProperties * prop);
	virtual ~UXToolkit();

public: // Implements UToolkit
	/** If the given context is a UXContext object, tries to use ux methods
	  * to make it the current context.
	  * Otherwise just marks it the current UFO context without doing
	  * anything to the underlying OpenGL context.
	  */
	void makeContextCurrent(UContext * contextA);
	/** Returns the last context which was made the current context
	  * by makeContextCurrent.
	  * @see makeContextCurrent
	  */
	UContext * getCurrentContext() const;


	UDimension getScreenSize() const;
	UInsets getScreenInsets() const;
	int getScreenDepth() const;

	/** Wait a specified number of milliseconds before returning.
	  * delay will wait at least the specified time, but possible longer due
	  * to OS scheduling.
	  * Implemented for posix and Win32.
	  *
	  * Note: Count on a delay granularity of at least 10 ms. Some platforms have
	  * shorter clock ticks but this is the most common.
	  * @param millis the minimum delay in milliseconds
	  */
	void sleep(uint32_t millis);
	/** Returns the amount of milli seconds since creating this toolkit.
	  */
	uint32_t getTicks() const;

private: // Private methods
	void startTicks();

private: // Private attributes
	UContext * m_context;
};

} // namespace ufo

#endif // UXTOOLKIT_HPP
