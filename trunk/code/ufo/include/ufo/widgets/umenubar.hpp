/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/widgets/umenubar.hpp
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#ifndef UMENUBAR_HPP
#define UMENUBAR_HPP

#include "uwidget.hpp"

// we need this for proper getUI() overriding
//#include "../ui/umenubarui.hpp"

namespace ufo {

class UMenu;
class UMouseEvent;
class URootPane;

/** The menu bar contains different umenu objects
  * @author Johannes Schmidt
  */

class UFO_EXPORT UMenuBar : public UWidget {
	UFO_DECLARE_DYNAMIC_CLASS(UMenuBar)
	UFO_UI_CLASS(UMenuBarUI)
public:
	UMenuBar();
/*
public: // hides | overrides UWidget
	virtual void setUI(UMenuBarUI * ui);
	virtual UWidgetUI * getUI() const;
	virtual void updateUI();
*/
protected: // overrides UWidget
	/** adds a menu */
	virtual void addImpl(UWidget * w, UObject * constraints, int index);

public: // Public methods
	/** closes all popup menus opened by a menu of this menu bar */
	virtual void closePopups();

	/** Don´t call this function directly. It is internally used by UMenu */
	virtual void setVisibleMenu(UMenu * menu);
	/** Don ´t call this function directly. It is internally used by UMenu */
	virtual UMenu * getVisibleMenu();

protected: // Protected slots
	void menuPopup(UMouseEvent * e);

private:  // Private attributes
	UMenu * m_visMenu;
};

} // namespace ufo

#endif // UMENUBAR_HPP
