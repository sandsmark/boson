/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/widgets/urootpane.hpp
    begin             : Fri May 18 2001
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

#ifndef UROOTPANE_HPP
#define UROOTPANE_HPP

#include "uwidget.hpp"

namespace ufo {

class UMenuBar;
class UInternalFrame;
class ULayeredPane;
class UStyleManager;
class UDockWidget;
class UDesktopPane;

/** @short The top level widget.
  * @ingroup widgets
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT URootPane : public UWidget {
	UFO_DECLARE_DYNAMIC_CLASS(URootPane)
	friend class URootLayout;
public:
	URootPane();

	/** Sets the new menubar.
	  */
	virtual void setMenuBar(UMenuBar * menuBar);
	/** @return The menubar
	  */
	virtual UMenuBar * getMenuBar();

	/** The content pane is the container for all user created widgets. */
	virtual UWidget * createContentPane() const;
	/** Sets the content pane.
	  * @see createContentPane
	  */
	virtual void setContentPane(UWidget * contentPane);
	/** @return the content pane.
	  * @see createContentPane
	  */
	virtual UWidget * getContentPane() const;

	virtual ULayeredPane * createLayeredPane() const;
	virtual void setLayeredPane(ULayeredPane * layeredPane);
	virtual ULayeredPane * getLayeredPane() const;

	/** Adds a frame to root pane. The new frame is at top.
	  * The frames are stored in a vector. If a new frame is added or
	  * the state of a frame ( on top; * behind another frame ) is changed,
	  * the vector will be reordered.
	  */
	virtual void addFrame(UInternalFrame * frame);
	/** removes a frame to root pane.
	  * @return the removed internal frame or NULL
	  */
	virtual UInternalFrame * removeFrame(UInternalFrame * frame);

	/** Moves the given internal frame to first position in children vector
	  */
	virtual void moveToFront(UInternalFrame * frame);
	/** moves the given internal frame to last position in children vector
	  */
	virtual void moveToBack(UInternalFrame * frame);

	virtual void addDockWidget(UDockWidget * w, DockWidgetArea area);
	virtual void removeDockWidget(UDockWidget * w);

	virtual void setModalWidget(UWidget * w);
	virtual UWidget * getModalWidget() const;

public: // Overrides UWidget
	virtual URootPane * getRootPane(bool topmost = false);
	virtual void addedToHierarchy();
	virtual UWidget * getVisibleWidgetAt(const UPoint & p) const;

private:  // Private attributes
	/** the menubar which hosts the menus */
	UMenuBar * m_menuBar;
	/** the panel that contains all user widgets */
	UWidget * m_contentPane;
	/** the panel that contains all user frames */
	UDesktopPane * m_desktopPane;
	UWidget * m_modalWidget;
};

} // namespace ufo

#endif // UROOTPANE_HPP
