/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ux/uxcontext.hpp
    begin             : Tue Jul 27 2004
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

#ifndef UXCONTEXT_HPP
#define UXCONTEXT_HPP

#include "../uabstractcontext.hpp"

namespace ufo {

class UXFrame;

/** @short A platform-indenpendent implementation of a UFO context.
  * @ingroup native
  *
  * May be used in conjunction with the UX modul or
  * using a custom OpenGL context.
  * See also test/sdl.cpp and test/glut.cpp
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UXContext : public UAbstractContext {
	UFO_DECLARE_DYNAMIC_CLASS(UXContext)
	friend class UXFrame;
public:
	/** Creates a pure context and initializes it.
	  * A valid GL context must! exist.
	  *
	  * @param deviceBounds The bounds of the GL context
	  * @param contextBounds The desired bounds of the UFO context
	  */
	UXContext(const URectangle & deviceBounds, const URectangle & contextBounds);

	/** Creates a pure context and initializes it.
	  * A valid GL context must! exist.
	  * This time, GL context and UFO context have the same size.
	  *
	  * @param bounds The bounds of the GL and UFO context.
	  */
	UXContext(const URectangle & bounds);
	virtual ~UXContext();

public: // Public methods
	/** Returns true if any widgets in this context need a repaint. */
	bool needsRepaint();
	/** Pushes all GL and system attributes.
	  * This method does not support nested calls.
	  */
	void pushAttributes();
	/** Restores the GL and system state stored at the last call to
	  * @ref pushAttributes.
	  */
	void popAttributes();
	/** Resets the GL attributes and calls the paint method of the root pane.
	  */
	void repaint();

	/** If this context was created by a UXFrame object, return that.
	  * Otherwise returns NULL.
	  */
	UXFrame * getFrame() const;

public: // Implements UAbstractContext
	virtual void lock();
	virtual void unlock();
private: // Private attributes
	UXFrame * m_frame;
};

} // namespace ufo

#endif // UXCONTEXT_HPP
