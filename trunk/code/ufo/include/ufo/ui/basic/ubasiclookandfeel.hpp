/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ui/basic/ubasiclookandfeel.hpp
    begin             : Mon Jul 22 2002
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

#ifndef UBASICLOOKANDFEEL_HPP
#define UBASICLOOKANDFEEL_HPP

#include "../ulookandfeel.hpp"

namespace ufo {

class UBorder;
class UPalette;
class UFont;
class UIcon;
class UStyle;

class UFontPlugin;
class UPluginBase;

/** The basic look and feel class.
  *
  * This class is not part of the official UFO API and
  * may be changed without warning.
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UBasicLookAndFeel : public ULookAndFeel {
	UFO_DECLARE_DYNAMIC_CLASS(UBasicLookAndFeel)
public:
	UBasicLookAndFeel();
	~UBasicLookAndFeel();

public: // Implements ULookAndFeel
	virtual UUIMap getDefaults();

	virtual UThemeMap getThemeDefaults();

	virtual UStyle * getStyle();

	/** Reload possibly lost data during recreation of GLX context.
	  */
	virtual void refresh();

	virtual std::string getName();

public: // plugin methods
	static UPluginBase * createPlugin();
	static void destroyPlugin(UPluginBase * plugin);

protected: // Protected methods
	void ensureMaps();

	virtual void initUIMap(UUIMap & uiMap);
	virtual void initThemeMap(UThemeMap & themeMap);

protected: // May be overriden for custom themes
	virtual UStyle * createStyle();
	//
	// palettes
	//
	/** Create a color palette for passive widgets like labels etc. */
	virtual UPalette * createPassivePalette();
	/** Creates the color palette for active controls 
	  * (buttons, combo boxes etc.) 
	  */
	virtual UPalette * createControlPalette();
	/** Creates the color palette for text widgets 
	  */
	virtual UPalette * createInputPalette();
	/** Creates the color palette for input controls (text widgets etc.) */
	//virtual UPalette * createInputPalette();
	/** Creates the color palette for menus */
	virtual UPalette * createMenuPalette();
	/** Creates the color palette for windows (internal frames etc.) */
	virtual UPalette * createWindowPalette();

	//
	// fonts
	//
	virtual UFont * createControlFont();
	virtual UFont * createSystemFont();
	virtual UFont * createTitleFont();
	virtual UFont * createUserFont();

	//
	// icons
	//
	virtual UIcon * createInternalFrameDefaultIcon();
	virtual UIcon * createInternalFrameMaximizeIcon();
	virtual UIcon * createInternalFrameMinimizeIcon();
	virtual UIcon * createInternalFrameCloseIcon();

	virtual UIcon * createCheckBoxDefaultIcon();
	virtual UIcon * createCheckBoxSelectedIcon();
	virtual UIcon * createCheckBoxInactiveIcon();

	virtual UIcon * createRadioButtonDefaultIcon();
	virtual UIcon * createRadioButtonSelectedIcon();
	virtual UIcon * createRadioButtonInactiveIcon();


private: // Protected attributes
	// some common theme defaults
	UPalette * m_passivePalette;
	UPalette * m_controlPalette;
	UPalette * m_inputPalette;
	UPalette * m_menuPalette;
	UPalette * m_windowPalette;
	//
	// fonts

	UFont * m_controlFont; // fonts for controls like buttons, checkboxes
	UFont * m_systemFont; // system texts (warnings, blah, ..)
	UFont * m_titleFont; // title bars
	UFont * m_userFont; // user input widget (e.g.text widgets)

private: //
	UUIMap m_uiMap;
	UThemeMap m_themeMap;
	UStyle * m_style;
	/** Garbage collecting container for theme objects. */
	UObject m_themeContainer;

};

} // namespace ufo

#endif // UBASICLOOKANDFEEL_HPP
