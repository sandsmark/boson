/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/widgets/udesktoppane.hpp
    begin             : Mon Dec 27 2004
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

#ifndef UDESKTOPPANE_HPP
#define UDESKTOPPANE_HPP

#include "ulayeredpane.hpp"

namespace ufo {

/** Warning: Experimental; not all features are implemented.
  * @short This class provides basic desktop features like frame decorations,
  *  minimizing and maximizing.
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UDesktopPane : public ULayeredPane {
	UFO_DECLARE_DYNAMIC_CLASS(UDesktopPane)
public:
	UDesktopPane();
	virtual ~UDesktopPane();

public:
	/** Adds the widget to this desktop pane and installs frame decorations.
	  */
	virtual void addFrame(UWidget * w);
	/** Removes this frame from this desktop pane (if it is a child) and
	  * uninstalls frame decorations.
	  */
	virtual UWidget * removeFrame(UWidget * w);

	/** Resizes the frame to cover the whole desktop.
	  */
	virtual void maximize(UWidget * w);
	virtual bool isMaximized(UWidget * w);
	/** Iconifies the frame to cover the whole desktop. */
	virtual void minimize(UWidget * w);
	virtual bool isMinimized(UWidget * w);
	/** Restores a previously maximized or minimized frame. */
	virtual void restore(UWidget * w);
	/** Closes the given frame. This basically removes the widget from
	  * the desktop pane.
	  */
	virtual void close(UWidget * w);

	/** */
	virtual void setTitle(UWidget * w, const std::string & title);
	virtual std::string getTitle(UWidget * w);

protected: // Protected methods
	/** @return The frame widget for the given desktop window or NULL. */
	virtual UWidget * getFrame(UWidget * w) const;
	virtual void eventListener(UEvent * e);
protected: // Overrides UWidget
	virtual void addedToHierarchy();
	virtual void removedFromHierarchy();
};

} // namespace ufo

#endif // UDESKTOPPANE_HPP
