/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ui/uuimanager.hpp
    begin             : Sat Jul 7 2001
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

#ifndef UUIMANAGER_HPP
#define UUIMANAGER_HPP

#include "../uobject.hpp"
#include "../uvolatiledata.hpp"

#include "uuidefs.hpp"
#include "../util/upalette.hpp"

namespace ufo {

class UWidget;

class UColor;
class UFont;
class UBorder;

class ULookAndFeel;
class UStyle;

/** The ui manager handles the look and feel and themes
  *
  * @see ULookAndFeel
  * @author Johannes Schmidt
  */

class UFO_EXPORT UUIManager : public UVolatileData {
	UFO_DECLARE_DYNAMIC_CLASS(UUIManager)
public:
	UUIManager();
	
	/** changes the look and feel. The look and feel says
	  * which ui classes should be used.
	  * If this look and feel has its own theme,
	  * the method setTheme is also called.
	  *
	  * This function overwrites all UI and theme defaults of this UI manager.
	  * @see ULookAndFeel
	  */
	void setLookAndFeel(ULookAndFeel * lookAndFeel);
	const ULookAndFeel * getLookAndFeel();

	/** Returns the style by the look and feel if supported.
	  * Otherwise this returns NULL.
	  * This method generally shouldn't be called by other classes than
	  * UI classes.
	  */
	UStyle * getStyle() const;

	/** returns a proper ui class for the given widget or NULL
	  */
	UWidgetUI * getUI(UWidget * widget);

	/** sets the ui class for the given type of widgets.
	  *If the ui is not suitable, nothing is changed
	  */
	void setUI(const std::string & key, UI_HANDLER ui);

	/** get a theme property ( like fonts, colors, .. of widgets)
	  */
	UObject * get(const std::string & key);
	/** set a theme property ( like fonts, colors, .. of widgets)
	  */
	void put(const std::string & key, UObject * value);

	/** A specialized version of get. Tries to get a color from theme
	  * property map.
	  */
	UPalette getPalette(const std::string & key);
	UColor * getColor(const std::string & key);
	UFont * getFont(const std::string & key);
	BorderType getBorder(const std::string & key);

public: // Implements UVolatileData
	/** Refreshes all data which could be lost during the destruction and
	  * recreation of a GLX context.
	  */
	virtual void refresh();

private:  // Private attributes
	/** the function pointers to get the right UI classes are stored in this
	  * hash map
	  */
	UUIMap m_uiDefaults;
	UThemeMap m_themeDefaults;

	ULookAndFeel * m_lookAndFeel;
};

} // namespace ufo

#endif // UUIMANAGER_HPP
