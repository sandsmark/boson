/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/widgets/upopupmenu.hpp
    begin             : Wed May 30 2001
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

#ifndef UPOPUPMENU_HPP
#define UPOPUPMENU_HPP

#include "uwidget.hpp"

namespace ufo {

class UMenuItem;
class UPopup;

/** @short A popup used for popup menus.
  * @ingroup widgets
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UPopupMenu : public UWidget {
	UFO_DECLARE_DYNAMIC_CLASS(UPopupMenu)
	UFO_UI_CLASS(UPopupMenuUI)
public:
	UPopupMenu();
	UPopupMenu(UWidget * invoker);

public: // hides | overrides UWidget
	/** use show, at least the first time
	  * @see show
	  */
	virtual void setVisible(bool v);

public: // Public methods

	/** appends a separator at the end */
	void addSeparator();

	/** Sets the popup location relative to invoker widget.
	  * The popup location is always in coordinates of the invoker
	  * widget.
	  */
	void setPopupLocation(const UPoint & pos);
	UPoint getPopupLocation() const;

	/** Sets the invoker widget which opens this popupmenu */
	virtual void setInvoker(UWidget * invoker);
	virtual UWidget * getInvoker() const;

public: // Public signals
	typedef USignal1<UPopupMenu*> MenuSignal;
	typedef USignal2<UPopupMenu*, UMenuItem*> MenuItemSignal;

	/** This popup menu is about to be opened */
	MenuSignal & sigMenuAboutToOpen();
	/** This popup menu is about to close */
	MenuSignal & sigMenuAboutToClose();
	/** A menu item is selected */
	MenuItemSignal & sigMenuItemHighlight();

protected: // Protected methods
	/** Closes this popup menu.
	  * FIXME: Should this be within the UI class?
	  */
	//void popupCloseSlot(UEvent * e);
	void popupCloseSlot(UPopup * popup);

private:
	UWidget * m_invoker;
	UPoint m_popupLocation;

	/** context listener for closing this popup menu on mouse press events
	  * FIXME: Should this be within the UI class?
	  */
	USlot1<UPopup*> m_closeSlot;
	class UPopup * m_popup;
private: // Private signals
	MenuSignal m_sigMenuAboutToOpen;
	MenuSignal m_sigMenuAboutToClose;
	MenuItemSignal m_sigMenuItemHighlight;
};

//
// inline implementation
//

inline UPopupMenu::MenuSignal &
UPopupMenu::sigMenuAboutToOpen() {
	return m_sigMenuAboutToOpen;
}

inline UPopupMenu::MenuSignal &
UPopupMenu::sigMenuAboutToClose() {
	return m_sigMenuAboutToClose;
}

inline UPopupMenu::MenuItemSignal &
UPopupMenu::sigMenuItemHighlight() {
	return m_sigMenuItemHighlight;
}

inline void
UPopupMenu::setPopupLocation(const UPoint & pos) {
	m_popupLocation = pos;
}

inline UPoint
UPopupMenu::getPopupLocation() const {
	return m_popupLocation;
}


} // namespace ufo

#endif // UPOPUPMENU_HPP
