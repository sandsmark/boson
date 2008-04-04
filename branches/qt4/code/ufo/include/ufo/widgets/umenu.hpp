/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/widgets/umenu.hpp
    begin             : Tue May 29 2001
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

#ifndef UMENU_HPP
#define UMENU_HPP

#include "umenuitem.hpp"

namespace ufo {

class UPopupMenu;

/** @short A menu item with a popup menu (used for popup menus).
  * @ingroup widgets
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UMenu : public UMenuItem {
	UFO_DECLARE_DYNAMIC_CLASS(UMenu)
	UFO_UI_CLASS(UMenuItemUI)
public:
	UMenu(const std::string & text = "", UIcon * icon = NULL);
	UMenu(UIcon * icon);

protected: // overrides UWidget
	/** adds a sub widget to the popup */
	virtual void addImpl(UWidget * w, UObject * constraints, int index);
	/** invalidate also the popup menu
	  */
	void invalidateSelf();

public: // Public methods

	/** @return true, if the parent is a menu bar */
	bool isTopLevelMenu() const;

	/** appends a separator at the end */
	void addSeparator();

	/** */
	bool isPopupMenuVisible() const;

	/** */
	void setPopupMenuVisible(bool b);

protected:   // Protected methods
	std::ostream & paramString(std::ostream & os) const;

protected:   // Protected attributes
	/** the popup menu that is popped up, when the menu button is pressed */
	UPopupMenu * m_popupMenu;
};


} // namespace ufo

#endif // UMENU_HPP
