/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ui/uthemelookandfeel.cpp
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

#include "ufo/ui/uthemelookandfeel.hpp"

#include "ufo/utoolkit.hpp"
#include "ufo/util/ucolor.hpp"
#include "ufo/util/uproperties.hpp"

#include "ufo/image/uimageicon.hpp"

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UThemeLookAndFeel, UObject)

UThemeLookAndFeel::UThemeLookAndFeel() : m_properties(new UProperties()) {
	preinitColors();
	// search within the UFO properties for a valid theme file
	std::string themeFile = UToolkit::getToolkit()->getProperty("theme_config");
	if (themeFile.length()) {
		load(themeFile);
	} else {
		loadSystemDefaults();
	}
}
UThemeLookAndFeel::UThemeLookAndFeel(const std::string & themeFile)
	: m_properties(new UProperties())
{
	preinitColors();
	load(themeFile);
}

//
// Public methods
//

void
UThemeLookAndFeel::setPath(const std::string & path) {
	if (path.length()) {
		m_properties->put("path", path);
	}
}

std::string
UThemeLookAndFeel::getPath() const {
	return m_properties->get("path");
}

void
UThemeLookAndFeel::load(const std::string & themeFile) {
	m_properties->load(themeFile);

	UProperties * cscheme = m_properties->getChild("Color Scheme");
	if (cscheme) {
		// check for light dark mid
		if (cscheme->get("light") == "") {
			cscheme->put("light", cscheme->get("alternateBackground"));
		}
		if (cscheme->get("dark") == "") {
			cscheme->put("dark", cscheme->get("activeBackground"));
		}
		if (cscheme->get("mid") == "") {
			cscheme->put("mid", cscheme->get("inactiveBlend"));
		}
	}
/*
	UProperties * cscheme = m_properties->getChild("Color Scheme");
	//uDebug() << "themefiles " << themeFile << "\n";
	if (!cscheme) {
		// no color scheme available in theme config file
		// using system defaults as fall back
		loadSystemDefaults();
		cscheme = m_properties->getChild("Color Scheme");
	}*/
}

#if defined UFO_GFX_X11
#include <bogl.h>
#include <boglx.h>
#include <X11/Xlib.h>
inline std::string _theme_toString(const char * p) {
	if (p) {
		return std::string(p);
	} else {
		return "";
	}
}
#endif // UFO_GFX_X11

#include "ufo/usysteminfo.hpp"

void
UThemeLookAndFeel::loadSystemDefaults() {
	// it is guaranteed that a color scheme object is created
	//UProperties * cscheme = new UProperties();
	//m_properties->putChild("Color Scheme", cscheme);
	UProperties * cscheme = m_properties->getChild("Color Scheme");
	// system specific stuff
#if defined(UFO_GFX_X11)
	//USystemInfo info;
	//UContext * context = UToolkit::getToolkit()->getCurrentContext();

	//if (!context || !context->getSystemInfo(&info)) {
	//	return;
	//}
	// FIXME !
	// doh, yet again a display variable ...
	Display * display = XOpenDisplay(0);//glXGetCurrentDisplay();////info.display;//XOpenDisplay(0);

	UToolkit * tk = UToolkit::getToolkit();
	std::string prgname = tk->getPrgName();

	std::string foreground = _theme_toString(XGetDefault(display, prgname.c_str(), "foreground"));
	std::string background = _theme_toString(XGetDefault(display, prgname.c_str(), "background"));
	std::string selectForeground =
		_theme_toString(XGetDefault(display, "Text", "selectForeground"));
	std::string selectBackground =
		_theme_toString(XGetDefault(display, "Text", "selectBackground"));
	std::string buttonForeground =
		_theme_toString(XGetDefault(display, "Button", "foreground"));
	std::string buttonBackground =
		_theme_toString(XGetDefault(display, "Button", "background"));
	std::string activeForeground =
		_theme_toString(XGetDefault(display, "Button", "activeForeground"));
	std::string activeBackground =
		_theme_toString(XGetDefault(display, "Button", "activeBackground"));
	std::string alternateBackground =
		_theme_toString(XGetDefault(display, "Listbox", "selectBackground"));
	std::string windowForeground =
		_theme_toString(XGetDefault(display, "Window", "foreground"));
	std::string windowBackground =
		_theme_toString(XGetDefault(display, "Window", "background"));

	// uff, randomly chosen
#define UFO_PUT_VALID(key, value) \
if (value != "") { cscheme->put(#key, value); }
	UFO_PUT_VALID(activeForeground, activeForeground)
	UFO_PUT_VALID(inactiveForeground, foreground)
	UFO_PUT_VALID(activeBackground, activeBackground)
	UFO_PUT_VALID(inactiveBackground, background)
	UFO_PUT_VALID(activeTitleBtnBg, selectBackground)
	UFO_PUT_VALID(inactiveTitleBtnBg, selectBackground)
	UFO_PUT_VALID(activeBlend, selectBackground)
	UFO_PUT_VALID(inactiveBlend, background)
	UFO_PUT_VALID(alternateBackground, alternateBackground)
	UFO_PUT_VALID(foreground, foreground)
	UFO_PUT_VALID(background, background)
	UFO_PUT_VALID(buttonForeground, buttonForeground)
	UFO_PUT_VALID(buttonBackground, buttonBackground)
	UFO_PUT_VALID(selectForeground, selectForeground)
	UFO_PUT_VALID(selectBackground, selectBackground)
	UFO_PUT_VALID(windowForeground, windowForeground)
	UFO_PUT_VALID(windowBackground, windowBackground)

	UFO_PUT_VALID(light, selectBackground)
	UFO_PUT_VALID(mid, buttonBackground)
	UFO_PUT_VALID(dark, selectBackground)

#undef UFO_PUT_VALID

	XCloseDisplay(display);
#elif defined(UFO_GFX_WIN32) // UFO_GFX_X11
#endif
}

//
// Overrides ULookAndFeel
//

UThemeMap
UThemeLookAndFeel::getThemeDefaults() {
	UThemeMap themeMap = UBasicLookAndFeel::getThemeDefaults();

	return themeMap;
}

void
UThemeLookAndFeel::refresh() {
	UBasicLookAndFeel::refresh();
}

std::string
UThemeLookAndFeel::getName() {
	return "theme";
}



//
// plugin methods
//

class UThemeLAFPlugin : public ULAFPlugin {
	UFO_DECLARE_DYNAMIC_CLASS(UThemeLAFPlugin)
public:
	virtual ULookAndFeel * createLookAndFeel() {
		UThemeLookAndFeel * ret = new UThemeLookAndFeel();

		trackPointer(ret);
		return ret;
	}
};
UFO_IMPLEMENT_DYNAMIC_CLASS(UThemeLAFPlugin, LAFPlugin)


UPluginBase *
UThemeLookAndFeel::createPlugin() {
	return new UThemeLAFPlugin();
}

void
UThemeLookAndFeel::destroyPlugin(UPluginBase * plugin) {
	if (dynamic_cast<UThemeLAFPlugin*>(plugin)) {
		delete (plugin);
	}
}

//
// Protected methods
//

UPalette *
UThemeLookAndFeel::createPassivePalette() {
	//return UBasicLookAndFeel::createPassivePalette();
	UProperties * cscheme = m_properties->getChild("Color Scheme");
	UColorGroup active(
		UColor(0.93f, 0.93f, 0.90f), // base
		UColor(0.0f, 0.0f, 0.0f), // baseFore
		UColor(cscheme->get("background")), // background
		UColor(cscheme->get("foreground")), // foreground
		UColor(cscheme->get("text")), // text
		UColor(cscheme->get("light")), // light
		UColor(cscheme->get("dark")), // dark
		UColor(cscheme->get("mid")), // midLight
		UColor(cscheme->get("selectBackground")), // highlight
		UColor(cscheme->get("selectForeground")) // highlightedText
	);
	UColorGroup inactive(
		UColor(0.93f, 0.93f, 0.90f), // base
		UColor(0.0f, 0.0f, 0.0f), // baseFore
		UColor(cscheme->get("background")), // background
		UColor(cscheme->get("foreground")), // foreground
		UColor(cscheme->get("text")), // text
		UColor(cscheme->get("light")), // light
		UColor(cscheme->get("dark")), // dark
		UColor(cscheme->get("mid")), // midLight
		UColor(cscheme->get("alternateBackground")), // highlight
		UColor(cscheme->get("activeForeground")) // highlightedText
	);
	return new UPalette(active, inactive, inactive);
}


UPalette *
UThemeLookAndFeel::createControlPalette() {
	//return UBasicLookAndFeel::createControlPalette();
	UProperties * cscheme = m_properties->getChild("Color Scheme");
	UColorGroup active(
		UColor(0.93f, 0.93f, 0.90f), // base
		UColor(0.0f, 0.0f, 0.0f), // baseFore
		UColor(cscheme->get("activeBackground")), // background
		UColor(cscheme->get("activeForeground")), // foreground
		UColor(cscheme->get("text")), // text
		UColor(cscheme->get("light")), // light
		UColor(cscheme->get("dark")), // dark
		UColor(cscheme->get("mid")), // midLight
		UColor(cscheme->get("selectBackground")), // highlight
		UColor(cscheme->get("selectForeground")) // highlightedText
	);
	UColorGroup inactive(
		UColor(0.93f, 0.93f, 0.90f), // base
		UColor(0.0f, 0.0f, 0.0f), // baseFore
		UColor(cscheme->get("buttonBackground")), // background
		UColor(cscheme->get("buttonForeground")), // foreground
		UColor(cscheme->get("text")), // text
		UColor(cscheme->get("light")), // light
		UColor(cscheme->get("dark")), // dark
		UColor(cscheme->get("mid")), // midLight
		UColor(cscheme->get("alternateBackground")), // highlight
		UColor(cscheme->get("text")) // highlightedText
	);
	return new UPalette(active, inactive, inactive);
}

UPalette *
UThemeLookAndFeel::createInputPalette() {
	//return UBasicLookAndFeel::createInputPalette();
	UProperties * cscheme = m_properties->getChild("Color Scheme");
	UColorGroup active(
		UColor(0.93f, 0.93f, 0.90f), // base
		UColor(0.0f, 0.0f, 0.0f), // baseFore
		UColor(1.0f, 1.0f, 1.0f), // background
		UColor(0.0f, 0.0f, 0.0f), // foreground
		UColor(cscheme->get("text")), // text
		UColor(cscheme->get("light")), // light
		UColor(cscheme->get("dark")), // dark
		UColor(cscheme->get("mid")), // midLight
		UColor(cscheme->get("selectBackground")), // highlight
		UColor(cscheme->get("selectForeground")) // highlightedText
	);
	UColorGroup inactive(
		UColor(0.93f, 0.93f, 0.90f), // base
		UColor(0.0f, 0.0f, 0.0f), // baseFore
		UColor(1.0f, 1.0f, 1.0f), // background
		UColor(0.0f, 0.0f, 0.0f), // foreground
		UColor(cscheme->get("text")), // text
		UColor(cscheme->get("light")), // light
		UColor(cscheme->get("dark")), // dark
		UColor(cscheme->get("mid")), // midLight
		UColor(cscheme->get("alternateBackground")), // highlight
		UColor(cscheme->get("text")) // highlightedText
	);
	return new UPalette(active, inactive, inactive);
}

UPalette *
UThemeLookAndFeel::createMenuPalette() {
	return UBasicLookAndFeel::createMenuPalette();
}

UPalette *
UThemeLookAndFeel::createWindowPalette() {
	return UBasicLookAndFeel::createWindowPalette();
}


UFont *
UThemeLookAndFeel::createControlFont() {
	return UBasicLookAndFeel::createControlFont();
}

UFont *
UThemeLookAndFeel::createSystemFont() {
	return UBasicLookAndFeel::createSystemFont();
}

UFont *
UThemeLookAndFeel::createTitleFont() {
	return UBasicLookAndFeel::createTitleFont();
}

UFont *
UThemeLookAndFeel::createUserFont() {
	return UBasicLookAndFeel::createUserFont();
}


inline UIcon * createLAFIcon(UProperties * prop, const std::string & key) {
	// should never happen
	if (!prop) return NULL;

	UProperties * iconProp = prop->getChild("icons");

	// this may happen
	if (!iconProp) return NULL;

	std::string fileName = iconProp->get(key);
	if (fileName.length()) {
		std::string path = prop->get("path");
		path += '/';
		path.append(fileName);

		// FIXME
		// What about some error checking
		return new UImageIcon(path);
	}
	return NULL;
}

//
// internal frame
//

UIcon *
UThemeLookAndFeel::createInternalFrameDefaultIcon() {
	if (UIcon * ret = createLAFIcon(m_properties, "internalframe_default")) {
		return ret;
	}
	return UBasicLookAndFeel::createInternalFrameDefaultIcon();
}

UIcon *
UThemeLookAndFeel::createInternalFrameMaximizeIcon() {
	if (UIcon * ret = createLAFIcon(m_properties, "internalframe_maximize")) {
		return ret;
	}
	return UBasicLookAndFeel::createInternalFrameMaximizeIcon();
}

UIcon *
UThemeLookAndFeel::createInternalFrameMinimizeIcon() {
	if (UIcon * ret = createLAFIcon(m_properties, "internalframe_minimize")) {
		return ret;
	}
	return UBasicLookAndFeel::createInternalFrameMinimizeIcon();
}

UIcon *
UThemeLookAndFeel::createInternalFrameCloseIcon() {
	if (UIcon * ret = createLAFIcon(m_properties, "internalframe_close")) {
		return ret;
	}
	return UBasicLookAndFeel::createInternalFrameCloseIcon();
}


//
// checkbox
//

UIcon *
UThemeLookAndFeel::createCheckBoxDefaultIcon() {
	if (UIcon * ret = createLAFIcon(m_properties, "checkbox_default")) {
		return ret;
	}
	return UBasicLookAndFeel::createCheckBoxDefaultIcon();
}

UIcon *
UThemeLookAndFeel::createCheckBoxSelectedIcon() {
	if (UIcon * ret = createLAFIcon(m_properties, "checkbox_selected")) {
		return ret;
	}
	return UBasicLookAndFeel::createCheckBoxSelectedIcon();
}

UIcon *
UThemeLookAndFeel::createCheckBoxInactiveIcon() {
	if (UIcon * ret = createLAFIcon(m_properties, "checkbox_inactive")) {
		return ret;
	}
	return UBasicLookAndFeel::createCheckBoxInactiveIcon();
}

//
// radio button
//

UIcon *
UThemeLookAndFeel::createRadioButtonDefaultIcon() {
	if (UIcon * ret = createLAFIcon(m_properties, "radiobutton_default")) {
		return ret;
	}
	return UBasicLookAndFeel::createRadioButtonDefaultIcon();
}

UIcon *
UThemeLookAndFeel::createRadioButtonSelectedIcon() {
	if (UIcon * ret = createLAFIcon(m_properties, "radiobutton_selected")) {
		return ret;
	}
	return UBasicLookAndFeel::createRadioButtonSelectedIcon();
}

UIcon *
UThemeLookAndFeel::createRadioButtonInactiveIcon() {
	if (UIcon * ret = createLAFIcon(m_properties, "radiobutton_inactive")) {
		return ret;
	}
	return UBasicLookAndFeel::createRadioButtonInactiveIcon();
}

void
UThemeLookAndFeel::preinitColors() {
	UProperties * cscheme = new UProperties();
	m_properties->putChild("Color Scheme", cscheme);
/*
	cscheme->put("activeForeground", "255,255,255");
	cscheme->put("inactiveForeground", "221,221,221");
	cscheme->put("activeBackground", "65,142,220");
	cscheme->put("inactiveBackground", "157,170,186");
	cscheme->put("activeTitleBtnBg", "127,158,200");
	cscheme->put("inactiveTitleBtnBg", "167,181,199");
	cscheme->put("activeBlend", "107,145,184");
	cscheme->put("inactiveBlend", "157,170,186");
	cscheme->put("alternateBackground", "107,145,184");//238,238,238");
	cscheme->put("foreground", "0,0,0");
	cscheme->put("background", "239,239,239");
	cscheme->put("buttonForeground", "0,0,0");
	cscheme->put("buttonBackground", "221,223,228");
	cscheme->put("selectForeground", "255,255,255");
	cscheme->put("selectBackground", "103,141,178");
	cscheme->put("windowForeground", "0,0,0");
	cscheme->put("windowBackground", "255,255,255");
*/
	cscheme->put("activeForeground", "0,0,0");
	cscheme->put("inactiveForeground", "0,0,0");
	cscheme->put("activeBackground", "157,170,186");
	cscheme->put("inactiveBackground", "157,170,186");
	cscheme->put("activeTitleBtnBg", "127,158,200");
	cscheme->put("inactiveTitleBtnBg", "167,181,199");
	cscheme->put("activeBlend", "107,145,184");
	cscheme->put("inactiveBlend", "157,170,186");
	cscheme->put("alternateBackground", "107,145,184");//238,238,238");
	cscheme->put("foreground", "0,0,0");
	cscheme->put("background", "239,239,239");
	cscheme->put("buttonForeground", "0,0,0");
	cscheme->put("buttonBackground", "239,239,245");
	cscheme->put("selectForeground", "255,255,255");
	cscheme->put("selectBackground", "103,141,178");
	cscheme->put("windowForeground", "0,0,0");
	cscheme->put("windowBackground", "255,255,255");

	cscheme->put("light", "157,170,186");
	cscheme->put("mid", "103,141,178");
	cscheme->put("dark", "65,142,220");
}
