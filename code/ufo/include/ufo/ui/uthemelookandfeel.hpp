/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ui/uthemelookandfeel.hpp
    begin             : Tue Feb 11 2003
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

#ifndef UTHEMELOOKANDFEEL_HPP
#define UTHEMELOOKANDFEEL_HPP
 
#include "basic/ubasiclookandfeel.hpp"

namespace ufo {

class UProperties;

class UFontPlugin;
class UPluginBase;

/** A themed look and feel.
  * A config file can be used to initialize colors and icons for this LAF.
  */
class UFO_EXPORT UThemeLookAndFeel : public UBasicLookAndFeel {
	UFO_DECLARE_DYNAMIC_CLASS(UThemeLookAndFeel)

public:
	UThemeLookAndFeel();
	UThemeLookAndFeel(const std::string & themeFile);

public: // Public methods
	virtual void load(const std::string & themeFile);

	/** Loads the default system colors and fonts.
	  * Implemented for X11 and Win32.
	  */
	virtual void loadSystemDefaults();

	/** Sets the path for media data. */
	virtual void setPath(const std::string & path);
	/** Returns the path for media data. */
	virtual std::string getPath() const;

public: // Overrides ULookAndFeel
	UThemeMap getThemeDefaults();
	void refresh();
	std::string getName();

public: // plugin methods
	static UPluginBase * createPlugin();
	static void destroyPlugin(UPluginBase * plugin);

protected: // Overrides UBasicLookAndFeel
	virtual UPalette * createControlPalette();
	virtual UPalette * createWindowPalette();
	/*virtual UPalette * createTextPalette();
	virtual UPalette * createMenuPalette();*/

	virtual UFont * createControlFont();
	virtual UFont * createSystemFont();
	virtual UFont * createTitleFont();
	virtual UFont * createUserFont();
	
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

protected: // Protected methods
	/** Ensures that there are valid color object in the properties. */
	virtual void preinitColors();

private:
	UProperties * m_properties;
	std::string m_themeFile;
};

} // namespace ufo

#endif // UTHEMELOOKANDFEEL_HPP
