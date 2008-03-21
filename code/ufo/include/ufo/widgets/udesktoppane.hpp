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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA *
 ***************************************************************************/

#ifndef UDESKTOPPANE_HPP
#define UDESKTOPPANE_HPP

#include "ulayeredpane.hpp"

namespace ufo {

class UInternalFrame;
class UDockWidget;

/** @short This class provides basic desktop features like frame decorations,
  *  minimizing and maximizing.
  * @ingroup internal
  *
  * Warning: Experimental; not all features are implemented.
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UDesktopPane : public ULayeredPane {
	UFO_DECLARE_CLASS(UDesktopPane)
public:
	UDesktopPane();
	virtual ~UDesktopPane();

public:
	/** Adds the widget to this desktop pane and installs frame decorations.
	  */
	virtual void addFrame(UInternalFrame * frame);
	/** Removes this frame from this desktop pane (if it is a child) and
	  * uninstalls frame decorations.
	  */
	virtual bool removeFrame(UInternalFrame * frame);

	/** Resizes the frame to cover the whole desktop.
	  */
	virtual void maximize(UInternalFrame * frame);
	virtual bool isMaximized(UInternalFrame * frame);
	/** Iconifies the frame. */
	virtual void minimize(UInternalFrame * frame);
	virtual bool isMinimized(UInternalFrame * frame);
	/** Restores a previously maximized or minimized frame. */
	virtual void restore(UInternalFrame * frame);
	/** Closes the given frame. This basically removes the widget from
	  * the desktop pane.
	  */
	virtual void close(UInternalFrame * frame);

	virtual void raise(UInternalFrame * frame);
	virtual void lower(UInternalFrame * frame);
	virtual bool isActive(UInternalFrame * frame);

	/** */
	virtual void setTitle(UInternalFrame * frame, const std::string & title);
	virtual std::string getTitle(UInternalFrame * frame);

	virtual void addDockWidget(UDockWidget * w, DockWidgetArea area);
	virtual void removeDockWidget(UDockWidget * w);

	virtual void dragDockWidget(UDockWidget * w, const UPoint & pos);
	virtual void dropDockWidget(UDockWidget * w, const UPoint & pos);

	virtual UInsets getContentsInsets() const;

protected: // Protected methods
	virtual void eventListener(UEvent * e);
	void moveDockWidget(UDockWidget * w, DockWidgetArea newArea);
protected: // Overrides UWidget
	virtual void validateSelf();
	virtual void processWidgetEvent(UWidgetEvent * e);
	virtual UDimension getContentsSize(const UDimension & maxSize) const;
private:
	UWidget * m_topDock;
	UWidget * m_leftDock;
	UWidget * m_bottomDock;
	UWidget * m_rightDock;
};

} // namespace ufo

#endif // UDESKTOPPANE_HPP
