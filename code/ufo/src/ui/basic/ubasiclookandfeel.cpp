/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ui/basic/ubasiclookandfeel.cpp
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

#include "ufo/ui/basic/ubasiclookandfeel.hpp"


#include "ufo/ui/basic/ubasicbuttonui.hpp"
#include "ufo/ui/basic/ubasiccheckboxui.hpp"
#include "ufo/ui/basic/ubasiccheckboxmenuitemui.hpp"
#include "ufo/ui/basic/ubasicradiobuttonui.hpp"
#include "ufo/ui/basic/ubasiccomboboxui.hpp"
#include "ufo/ui/basic/ubasicinternalframeui.hpp"
#include "ufo/ui/basic/ubasiclabelui.hpp"
#include "ufo/ui/basic/ubasicmenubarui.hpp"
#include "ufo/ui/basic/ubasicmenuitemui.hpp"
#include "ufo/ui/basic/ubasicseparatorui.hpp"
//#include "ufo/ui/basic/ubasicmenuui.hpp"
#include "ufo/ui/basic/ubasicpopupmenuui.hpp"
//#include "ufo/ui/basic/ubasictextfieldui.hpp"
//#include "ufo/ui/basic/ubasictextpaneui.hpp"
#include "ufo/ui/basic/ubasiclistboxui.hpp"
#include "ufo/ui/basic/ubasicscrollbarui.hpp"
#include "ufo/ui/basic/ubasicsliderui.hpp"
#include "ufo/ui/uwidgetui.hpp"

#include "ufo/ui/basic/ubasictexteditui.hpp"

// theme defaults
/*
#include "ufo/borders/uborder.hpp"
#include "ufo/borders/ubevelborder.hpp"
#include "ufo/borders/ulineborder.hpp"
#include "ufo/borders/uemptyborder.hpp"
*/
#include "ufo/util/ucolor.hpp"
#include "ufo/util/upalette.hpp"
#include "ufo/util/uinteger.hpp"

#include "ufo/font/ufont.hpp"
#include "ufo/font/ufontrenderer.hpp"

//#include "ufo/ui/basic/ubasicborderfactory.hpp"
#include "ufo/ui/ustyle.hpp"
#include "ufo/gl/ugl_style.hpp"

#include "ufo/uicon.hpp"
#include "ufo/image/uimageicon.hpp"

#include "ufo/utoolkit.hpp"


using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UBasicLookAndFeel, UObject)


//UStyle * UBasicLookAndFeel::sm_style = NULL;

UBasicLookAndFeel::UBasicLookAndFeel()
	: m_controlPalette(NULL)
	, m_windowPalette(NULL)
	, m_controlFont(NULL)
	, m_systemFont(NULL)
	, m_titleFont(NULL)
	, m_userFont(NULL)
	, m_uiMap()
	, m_themeMap()
	, m_style(NULL)
	, m_themeContainer()
{
/*
	m_activeForeground = new UColor(1.0f, 1.0f, 1.0f);
	m_inactiveForeground= new UColor(0.0f, 0.0f, 0.0f);

	m_activeBackground = new UColor(0.04f, 0.37f, 0.53f);
	m_inactiveBackground = new UColor(0.86f, 0.86f, 0.86f);

	m_activeBlend = new UColor(0.04f, 0.37f, 0.53f);
	m_inactiveBlend = new UColor(0.86f, 0.86f, 0.86f);

	m_activeTitleBtnBg = new UColor(0.75f, 0.75f, 0.75f);
	m_inactiveTitleBtnBg = new UColor(0.86f, 0.86f, 0.86f);

	m_alternateBackground = new UColor(0.933f, 0.96f, 1.0f);

	m_foreground = new UColor(0.0f, 0.0f, 0.0f);
	m_background = new UColor(0.86f, 0.86f, 0.86f);

	m_buttonForeground = new UColor(0.0f, 0.0f, 0.0f);
	m_buttonBackground = new UColor(0.86f, 0.86f, 0.86f);

	//m_selectForeground = new UColor(1.0f, 1.0f, 1.0f);
	//m_selectBackground = new UColor(0.04f, 0.37f, 0.53f);
	m_selectForeground = new UColor(0.0f, 0.0f, 0.0f);
	m_selectBackground = new UColor(0.05f, 0.50f, 0.70f);

	m_windowForeground = new UColor(0.0f, 0.0f, 0.0f);
	m_windowBackground = new UColor(1.0f, 1.0f, 1.0f);

	m_white = (UColor*) UColor::white->clone();
	m_black = (UColor*) UColor::black->clone();
*/
}

UBasicLookAndFeel::~UBasicLookAndFeel() {}

//
// Public methods
//
UStyle *
UBasicLookAndFeel::getStyle() {
	return m_style;
}


//
// implements ULookAndFeel
//

UUIMap
UBasicLookAndFeel::getDefaults() {
	ensureMaps();

	return m_uiMap;
}


UThemeMap
UBasicLookAndFeel::getThemeDefaults() {
	ensureMaps();

	return m_themeMap;
}

void
UBasicLookAndFeel::refresh() {
	// fonts should refresh self

	initThemeMap(m_themeMap);
}


std::string
UBasicLookAndFeel::getName() {
	return "basic";
}


//
// plugin methods
//

class UBasicLAFPlugin : public ULAFPlugin {
	UFO_DECLARE_DYNAMIC_CLASS(UBasicLAFPlugin)
public:
	virtual ULookAndFeel * createLookAndFeel() {
		UBasicLookAndFeel * ret = new UBasicLookAndFeel();

		trackPointer(ret);
		return ret;
	}
};
UFO_IMPLEMENT_DYNAMIC_CLASS(UBasicLAFPlugin, LAFPlugin)


UPluginBase *
UBasicLookAndFeel::createPlugin() {
	return new UBasicLAFPlugin();
}

void
UBasicLookAndFeel::destroyPlugin(UPluginBase * plugin) {
	if (dynamic_cast<UBasicLAFPlugin*>(plugin)) {
		delete (plugin);
	}
}

//
// Protected methods
//

void
UBasicLookAndFeel::ensureMaps() {
	initUIMap(m_uiMap);
	initThemeMap(m_themeMap);

	if (!m_controlPalette) {
	m_passivePalette = createPassivePalette();
	m_controlPalette = createControlPalette();
	m_inputPalette = createInputPalette();
	m_menuPalette = createMenuPalette();
	m_windowPalette = createWindowPalette();

	//trackPointer(m_widgetPalette);
	trackPointer(m_passivePalette);
	trackPointer(m_controlPalette);
	trackPointer(m_inputPalette);
	trackPointer(m_menuPalette);
	trackPointer(m_windowPalette);

	m_controlFont = createControlFont();
	m_systemFont = createSystemFont();
	m_titleFont = createTitleFont();
	m_userFont = createUserFont();

	trackPointer(m_controlFont);
	trackPointer(m_systemFont);
	trackPointer(m_titleFont);
	trackPointer(m_userFont);
	}

	if (!m_style) {
		m_style = createStyle();
	}
}


void
UBasicLookAndFeel::initUIMap(UUIMap & uiMap) {
	uiMap["UButtonUI"] = (UI_HANDLER) & UBasicButtonUI::createUI;
	uiMap["UCheckBoxUI"] = (UI_HANDLER) & UBasicCheckBoxUI::createUI;
	uiMap["UCheckBoxMenuItemUI"] = (UI_HANDLER) & UBasicCheckBoxMenuItemUI::createUI;
	uiMap["URadioButtonUI"] = (UI_HANDLER) & UBasicRadioButtonUI::createUI;
	uiMap["UComboBoxUI"] = (UI_HANDLER) & UBasicComboBoxUI::createUI;
	uiMap["UInternalFrameUI"] = (UI_HANDLER) & UBasicInternalFrameUI::createUI;
	uiMap["ULabelUI"] = (UI_HANDLER) & UBasicLabelUI::createUI;
	uiMap["UMenuBarUI"] = (UI_HANDLER) & UBasicMenuBarUI::createUI;
	uiMap["UMenuItemUI"] = (UI_HANDLER) & UBasicMenuItemUI::createUI;
	uiMap["USeparatorUI"] = (UI_HANDLER) & UBasicSeparatorUI::createUI;
	//uiMap["UMenuUI"] = (UI_HANDLER) & UBasicMenuUI::createUI;
	uiMap["UPopupMenuUI"] = (UI_HANDLER) & UBasicPopupMenuUI::createUI;
	//uiMap["UTextFieldUI"] = (UI_HANDLER) & UBasicTextFieldUI::createUI;
	//uiMap["UTextPaneUI"] = (UI_HANDLER) & UBasicTextPaneUI::createUI;
	uiMap["UTextEditUI"] = (UI_HANDLER) & UBasicTextEditUI::createUI;
	uiMap["UListBoxUI"] = (UI_HANDLER) & UBasicListBoxUI::createUI;
	uiMap["UScrollBarUI"] = (UI_HANDLER) & UBasicScrollBarUI::createUI;
	uiMap["USliderUI"] = (UI_HANDLER) & UBasicSliderUI::createUI;

	uiMap["UWidgetUI"] = (UI_HANDLER) & UWidgetUI::createUI;
}

void
UBasicLookAndFeel::initThemeMap(UThemeMap & themeMap) {
	m_themeContainer.releaseAllPointers();
	// some common objects
	UInteger * four = new UInteger(4);
	UInteger * uiBorder = new UInteger(UIBorder);
	UInteger * noBorder = new UInteger(NoBorder);
	UInteger * lineBorder = new UInteger(LineBorder);
	UInteger * raisedBevelBorder = new UInteger(RaisedBevelBorder);

	UInsetsObject * buttonMargin = new UInsetsObject(2, 4, 2, 4);

	UIcon * checkboxDefaultIcon = createCheckBoxDefaultIcon();
	UIcon * checkBoxSelectedIcon = createCheckBoxSelectedIcon();
	UIcon * checkBoxInactiveIcon = createCheckBoxInactiveIcon();

	UIcon * rbDefaultIcon = createRadioButtonDefaultIcon();
	UIcon * rbSelectedIcon = createRadioButtonSelectedIcon();
	UIcon * rbInactiveIcon = createRadioButtonInactiveIcon();

	UIcon * frameIcon = createInternalFrameDefaultIcon();
	UIcon * frameMaximizeIcon = createInternalFrameMaximizeIcon();
	UIcon * frameMinimizeIcon = createInternalFrameMinimizeIcon();
	UIcon * frameCloseIcon = createInternalFrameCloseIcon();

	// register gc objects
	m_themeContainer.trackPointer(four);
	m_themeContainer.trackPointer(uiBorder);
	m_themeContainer.trackPointer(noBorder);
	m_themeContainer.trackPointer(lineBorder);
	m_themeContainer.trackPointer(raisedBevelBorder);

	m_themeContainer.trackPointer(checkboxDefaultIcon);
	m_themeContainer.trackPointer(checkBoxSelectedIcon);
	m_themeContainer.trackPointer(checkBoxInactiveIcon);

	m_themeContainer.trackPointer(rbDefaultIcon);
	m_themeContainer.trackPointer(rbSelectedIcon);
	m_themeContainer.trackPointer(rbInactiveIcon);

	m_themeContainer.trackPointer(frameIcon);
	m_themeContainer.trackPointer(frameMaximizeIcon);
	m_themeContainer.trackPointer(frameMinimizeIcon);
	m_themeContainer.trackPointer(frameCloseIcon);

	// every widget object should have following properties,
	// which are mapped to the object by the UI classes
	// background
	// foreground
	// font
	// border
	// margin (?)

	//
	// std widgets
	// using theme values
	themeMap["UButton.palette"] = m_controlPalette;
	themeMap["UButton.font"] = m_controlFont;
	themeMap["UButton.border"] = uiBorder;//buttonBorder;
	themeMap["UButton.margin"] = buttonMargin;
	themeMap["UButton.textIconGap"] = four;


	themeMap["UCheckBox.palette"] = m_controlPalette;
	themeMap["UCheckBox.font"] = m_controlFont;
	themeMap["UCheckBox.border"] = noBorder;//NULL;
	themeMap["UCheckBox.textIconGap"] = four;
	// icons
	themeMap["UCheckBox.icon"] = checkboxDefaultIcon;
	themeMap["UCheckBox.selectedIcon"] = checkBoxSelectedIcon;
	themeMap["UCheckBox.inactiveIcon"] = checkBoxInactiveIcon;


	themeMap["URadioButton.palette"] = m_controlPalette;
	themeMap["URadioButton.font"] = m_controlFont;
	themeMap["URadioButton.border"] = noBorder;//NULL;
	themeMap["URadioButton.textIconGap"] = four;
	// icons
	themeMap["URadioButton.icon"] = rbDefaultIcon;
	themeMap["URadioButton.selectedIcon"] = rbSelectedIcon;
	themeMap["URadioButton.inactiveIcon"] = rbInactiveIcon;


	themeMap["UComboBox.palette"] = m_inputPalette;//m_controlPalette;
	themeMap["UComboBox.font"] = m_controlFont;
	themeMap["UComboBox.border"] = lineBorder;//lineBorder;
	//themeMap["UComboBox.selectionBackground"] = m_selectBackground;
	//themeMap["UComboBox.selectionForeground"] = m_selectForeground;


	themeMap["UInternalFrame.palette"] = m_windowPalette;
	themeMap["UInternalFrame.font"] = m_titleFont;
	themeMap["UInternalFrame.border"] = uiBorder;//internalFrameBorder;
/*
	themeMap["UInternalFrame.background"] = m_windowBackground;
	themeMap["UInternalFrame.foreground"] = m_windowForeground;

	themeMap["UInternalFrame.activeTitleBackground"] = m_activeBackground;
	themeMap["UInternalFrame.activeTitleForeground"] = m_activeForeground;
	themeMap["UInternalFrame.activeTitleFont"] = m_titleFont;
	themeMap["UInternalFrame.inactiveTitleBackground"] = m_inactiveBackground;
	themeMap["UInternalFrame.inactiveTitleForeground"] = m_inactiveForeground;
*/
	// icons
	themeMap["UInternalFrame.icon"] = frameIcon;
	themeMap["UInternalFrame.maximizeIcon"] = frameMaximizeIcon;
	themeMap["UInternalFrame.minimizeIcon"] = frameMinimizeIcon;
	themeMap["UInternalFrame.closeIcon"] = frameCloseIcon;


	themeMap["ULabel.palette"] = m_passivePalette;
	/*themeMap["ULabel.background"] = m_background;
	themeMap["ULabel.foreground"] = m_foreground;
	themeMap["ULabel.inactiveForeground"] = m_inactiveForeground;*/
	themeMap["ULabel.font"] = m_controlFont;
	themeMap["ULabel.border"] = noBorder;//NULL;


	themeMap["UMenuBar.palette"] = m_menuPalette;
	themeMap["UMenuBar.font"] = m_controlFont;
	themeMap["UMenuBar.border"] = uiBorder;//menuBarBorder;


	themeMap["UMenuItem.palette"] = m_menuPalette;
	/*
	themeMap["UMenuItem.background"] = m_buttonBackground;
	themeMap["UMenuItem.foreground"] = m_buttonForeground;
	themeMap["UMenuItem.inactiveForeground"] = m_inactiveForeground;
	themeMap["UMenuItem.selectionBackground"] = m_selectBackground;
	themeMap["UMenuItem.selectionForeground"] = m_selectForeground;*/
	themeMap["UMenuItem.font"] = m_controlFont;
	themeMap["UMenuItem.border"] = uiBorder;//noBorder;//marginBorder;

	themeMap["UCheckBoxMenuItem.palette"] = m_controlPalette;
	themeMap["UCheckBoxMenuItem.font"] = m_controlFont;
	themeMap["UCheckBoxMenuItem.border"] = uiBorder;
	themeMap["UCheckBoxMenuItem.textIconGap"] = four;
	// icons
	themeMap["UCheckBoxMenuItem.icon"] = checkboxDefaultIcon;
	themeMap["UCheckBoxMenuItem.selectedIcon"] = checkBoxSelectedIcon;
	themeMap["UCheckBoxMenuItem.inactiveIcon"] = checkBoxInactiveIcon;

/*
	themeMap["UMenu.background"] = m_buttonBackground;
	themeMap["UMenu.foreground"] = m_buttonForeground;
	themeMap["UMenu.font"] = m_controlFont;
	themeMap["UMenu.border"] = menuBorder;
*/

	themeMap["UPopupMenu.palette"] = m_menuPalette;
	/*
	themeMap["UPopupMenu.background"] = m_background;
	themeMap["UPopupMenu.foreground"] = m_foreground;*/
	themeMap["UPopupMenu.font"] = m_controlFont;
	themeMap["UPopupMenu.border"] = raisedBevelBorder;//lineBorder;


	themeMap["USeparator.palette"] = m_menuPalette;
	/*
	themeMap["USeparator.background"] = m_black;
	themeMap["USeparator.foreground"] = m_white;*/
	themeMap["USeparator.font"] = m_controlFont;
	themeMap["USeparator.border"] = noBorder;

/*
	themeMap["UTextField.background"] = m_white;
	themeMap["UTextField.foreground"] = m_black;
	themeMap["UTextField.font"] = m_userFont;
	themeMap["UTextField.border"] = lineBorder;
	themeMap["UTextField.selectionBackground"] = m_selectBackground;
	themeMap["UTextField.selectionForeground"] = m_selectForeground;


	themeMap["UTextPane.background"] = m_white;
	themeMap["UTextPane.foreground"] = m_black;
	themeMap["UTextPane.font"] = m_userFont;
	themeMap["UTextPane.border"] = lineBorder;
	themeMap["UTextPane.selectionBackground"] = m_selectBackground;
	themeMap["UTextPane.selectionForeground"] = m_selectForeground;
*/
	themeMap["UTextEdit.palette"] = m_inputPalette;
	/*
	themeMap["UTextEdit.background"] = m_white;
	themeMap["UTextEdit.foreground"] = m_black;*/
	themeMap["UTextEdit.font"] = m_userFont;
	themeMap["UTextEdit.border"] = lineBorder; // lineBorder
	/*themeMap["UTextEdit.selectionBackground"] = m_selectBackground;
	themeMap["UTextEdit.selectionForeground"] = m_selectForeground;*/


	themeMap["UListBox.palette"] = m_inputPalette;
	/*
	themeMap["UListBox.background"] = m_white;//m_shadow4;
	themeMap["UListBox.foreground"] = m_black;*/
	themeMap["UListBox.font"] = m_controlFont;
	themeMap["UListBox.border"] = lineBorder; // llineBorder;
/*
	themeMap["UListBox.selectionBackground"] = m_selectBackground;
	themeMap["UListBox.selectionForeground"] = m_selectForeground;
*/

	themeMap["UScrollBar.palette"] = m_passivePalette;
	/*
	themeMap["UScrollBar.background"] = m_background;//m_activeBackground;
	themeMap["UScrollBar.foreground"] = m_foreground;//m_activeForeground;*/
	// FIXME
	//themeMap["UScrollBar.track"] = new UColorObject(0.2f, 0.2f, 0.8f);//m_selectBackground;
	themeMap["UScrollBar.font"] = m_controlFont;
	themeMap["UScrollBar.border"] = lineBorder; // llineBorder;


	themeMap["USlider.palette"] = m_passivePalette;
	themeMap["USlider.font"] = m_controlFont;
	themeMap["USlider.border"] = noBorder;


	themeMap["UWidget.palette"] = m_passivePalette;/*
	themeMap["UWidget.background"] = m_background;
	themeMap["UWidget.foreground"] = m_foreground;*/
	themeMap["UWidget.font"] = m_controlFont;
	themeMap["UWidget.border"] = noBorder;//NULL;
}

//
// Protected methods
//

UStyle *
UBasicLookAndFeel::createStyle() {
	return new UGL_Style();
}

UPalette *
UBasicLookAndFeel::createPassivePalette() {
	UColorGroup inactive(
		UColor(0.93f, 0.93f, 0.90f), // base
		UColor(0.0f, 0.0f, 0.0f), // baseFore
		UColor(0.93f, 0.93f, 0.90f), // background
		UColor(0.0f, 0.0f, 0.0f), // foreground
		UColor(0.0f, 0.0f, 0.0f), // text
		UColor(0.71f, 0.76f, 0.83f), // light
		UColor(0.4f, 0.52f, 0.68f), // dark
		UColor(0.5f, 0.62f, 0.78f), // midLight
		UColor(0.5f, 0.62f, 0.78f), // highlight
		UColor(1.0f, 1.0f, 1.0f) // highlightedText
	);
	return new UPalette(inactive, inactive, inactive);
}

UPalette *
UBasicLookAndFeel::createControlPalette() {
	UColorGroup active(
		UColor(0.93f, 0.93f, 0.90f), // base
		UColor(0.0f, 0.0f, 0.0f), // baseFore
		UColor(0.25f, 0.55f, 0.86f), // background
		UColor(0.0f, 0.0f, 0.0f), // foreground
		UColor(0.0f, 0.0f, 0.0f), // text
		UColor(0.7f, 0.82f, 0.98f), // light
		UColor(0.3f, 0.42f, 0.58f), // dark
		UColor(0.5f, 0.62f, 0.78f), // midLight
		UColor(0.5f, 0.62f, 0.78f), // highlight
		UColor(1.0f, 1.0f, 1.0f) // highlightedText
	);
	UColorGroup inactive(
		UColor(0.93f, 0.93f, 0.90f), // base
		UColor(0.0f, 0.0f, 0.0f), // baseFore
		UColor(0.93f, 0.93f, 0.90f), // background
		UColor(0.0f, 0.0f, 0.0f), // foreground
		UColor(0.0f, 0.0f, 0.0f), // text
		UColor(0.71f, 0.76f, 0.83f), // light
		UColor(0.4f, 0.52f, 0.68f), // dark
		UColor(0.5f, 0.62f, 0.78f), // midLight
		UColor(0.5f, 0.62f, 0.78f), // highlight
		UColor(1.0f, 1.0f, 1.0f) // highlightedText
	);
	return new UPalette(active, inactive, inactive);
}

UPalette *
UBasicLookAndFeel::createInputPalette() {
	UColorGroup active(
		UColor(0.93f, 0.93f, 0.90f), // base
		UColor(0.0f, 0.0f, 0.0f), // baseFore
		UColor(1.0f, 1.0f, 1.0f), // background
		UColor(0.0f, 0.0f, 0.0f), // foreground
		UColor(0.0f, 0.0f, 0.0f), // text
		UColor(0.05f, 0.50f, 0.70f), // light
		UColor(0.05f, 0.50f, 0.70f), // dark
		UColor(0.5f, 0.62f, 0.78f), // midLight
		UColor(0.5f, 0.62f, 0.78f), // highlight
		UColor(1.0f, 1.0f, 1.0f) // highlightedText
	);
	UColorGroup inactive(
		UColor(0.93f, 0.93f, 0.90f), // base
		UColor(0.0f, 0.0f, 0.0f), // baseFore
		UColor(1.0f, 1.0f, 1.0f), // background
		UColor(0.0f, 0.0f, 0.0f), // foreground
		UColor(0.0f, 0.0f, 0.0f), // text
		UColor(0.70f, 0.70f, 0.70f), // light
		UColor(0.05f, 0.50f, 0.70f), // dark
		UColor(0.5f, 0.62f, 0.78f), // midLight
		UColor(0.5f, 0.62f, 0.78f), // highlight
		UColor(1.0f, 1.0f, 1.0f) // highlightedText
	);
	UColorGroup disabled(
		UColor(0.93f, 0.93f, 0.90f), // base
		UColor(0.0f, 0.0f, 0.0f), // baseFore
		UColor(0.7f, 0.7f, 0.7f), // background
		UColor(1.0f, 1.0f, 1.0f), // foreground
		UColor(0.0f, 0.0f, 0.0f), // text
		UColor(0.70f, 0.70f, 0.70f), // light
		UColor(0.05f, 0.50f, 0.70f), // dark
		UColor(0.5f, 0.62f, 0.78f), // midLight
		UColor(0.5f, 0.5f, 0.5f), // highlight
		UColor(0.0f, 0.0f, 0.0f) // highlightedText
	);
	return new UPalette(active, disabled, inactive);
}

UPalette *
UBasicLookAndFeel::createMenuPalette() {
	return createControlPalette();
}

UPalette *
UBasicLookAndFeel::createWindowPalette() {
	UColorGroup active(
		UColor(0.93f, 0.93f, 0.90f), // base
		UColor(0.0f, 0.0f, 0.0f), // baseFore
		UColor(0.25f, 0.55f, 0.86f), // background
		//UColor(0.5f, 0.5f, 0.8f), // background
		UColor(1.0f, 1.0f, 1.0f), // foreground
		UColor(0.0f, 0.0f, 0.0f), // text
		UColor(0.05f, 0.50f, 0.70f), // light
		UColor(0.05f, 0.50f, 0.70f), // dark
		UColor(0.5f, 0.62f, 0.78f), // midLight
		UColor(0.5f, 0.62f, 0.78f), // highlight
		UColor(1.0f, 1.0f, 1.0f) // highlightedText
	);
	UColorGroup inactive(
		UColor(0.93f, 0.93f, 0.90f), // base
		UColor(0.0f, 0.0f, 0.0f), // baseFore
		UColor(0.86f, 0.86f, 0.86f), // background
		UColor(0.0f, 0.0f, 0.0f), // foreground
		UColor(0.0f, 0.0f, 0.0f), // text
		UColor(0.70f, 0.70f, 0.70f), // light
		UColor(0.05f, 0.50f, 0.70f), // dark
		UColor(0.5f, 0.62f, 0.78f), // midLight
		UColor(0.5f, 0.62f, 0.78f), // highlight
		UColor(1.0f, 1.0f, 1.0f) // highlightedText
	);
	return new UPalette(active, inactive, inactive);
}

/*
UPalette *
UBasicLookAndFeel::createControlPalette() {
	UColorGroup active(
		UColor(0.0f, 0.0f, 0.0f), // foreground
		UColor(0.86f, 0.86f, 0.90f), // background
		//UColor(0.04f, 0.37f, 0.53f), // background
		UColor(0.40f, 0.40f, 0.60f), // button
		UColor(0.05f, 0.50f, 0.70f), // light
		UColor(0.50f, 0.50f, 0.50f), // mid
		UColor(0.05f, 0.50f, 0.70f), // dark
		UColor(0.0f, 0.0f, 0.0f), // text
		UColor(0.933f, 0.96f, 1.0f) // base
	);
	UColorGroup inactive(
		UColor(0.0f, 0.0f, 0.0f), // foreground
		UColor(0.86f, 0.86f, 0.86f), // background
		UColor(0.50f, 0.50f, 0.70f), // button
		UColor(0.70f, 0.70f, 0.70f), // light
		UColor(0.50f, 0.50f, 0.50f), // mid
		UColor(0.05f, 0.50f, 0.70f), // dark
		UColor(0.0f, 0.0f, 0.0f), // text
		UColor(0.933f, 0.96f, 1.0f) // base
	);
	return new UPalette(active, inactive, inactive);
}

UPalette *
UBasicLookAndFeel::createTextPalette() {
	UColorGroup active(
		UColor(0.0f, 0.0f, 0.0f), // foreground
		UColor(1.0f, 1.0f, 1.0f), // background
		UColor(0.86f, 0.86f, 0.86f), // button
		UColor(0.05f, 0.50f, 0.70f), // light
		UColor(0.50f, 0.50f, 0.50f), // mid
		UColor(0.05f, 0.50f, 0.70f), // dark
		UColor(0.0f, 0.0f, 0.0f), // text
		UColor(0.933f, 0.96f, 1.0f) // base
	);
	UColorGroup inactive(
		UColor(0.0f, 0.0f, 0.0f), // foreground
		UColor(1.0f, 1.0f, 1.0f), // background
		UColor(0.86f, 0.86f, 0.86f), // button
		UColor(0.70f, 0.70f, 0.70f), // light
		UColor(0.50f, 0.50f, 0.50f), // mid
		UColor(0.05f, 0.50f, 0.70f), // dark
		UColor(0.0f, 0.0f, 0.0f), // text
		UColor(0.933f, 0.96f, 1.0f) // base
	);
	UColorGroup disabled(
		UColor(0.5f, 0.5f, 0.5f), // foreground
		UColor(1.0f, 1.0f, 1.0f), // background
		UColor(0.86f, 0.86f, 0.86f), // button
		UColor(0.70f, 0.70f, 0.70f), // light
		UColor(0.50f, 0.50f, 0.50f), // mid
		UColor(0.05f, 0.50f, 0.70f), // dark
		UColor(0.0f, 0.0f, 0.0f), // text
		UColor(0.933f, 0.96f, 1.0f) // base
	);
	return new UPalette(active, disabled, inactive);
}

UPalette *
UBasicLookAndFeel::createMenuPalette() {
	return createControlPalette();
}

UPalette *
UBasicLookAndFeel::createWindowPalette() {
	UColorGroup active(
		UColor(1.0f, 1.0f, 1.0f), // foreground
		UColor(0.6f, 0.6f, 0.86f), // background
		UColor(0.86f, 0.86f, 0.86f), // button
		UColor(0.05f, 0.50f, 0.70f), // light
		UColor(0.50f, 0.50f, 0.50f), // mid
		UColor(0.05f, 0.50f, 0.70f), // dark
		UColor(0.0f, 0.0f, 0.0f), // text
		UColor(0.933f, 0.96f, 1.0f) // base
	);
	UColorGroup inactive(
		UColor(0.0f, 0.0f, 0.0f), // foreground
		UColor(0.86f, 0.86f, 0.86f), // background
		UColor(0.86f, 0.86f, 0.86f), // button
		UColor(0.70f, 0.70f, 0.70f), // light
		UColor(0.50f, 0.50f, 0.50f), // mid
		UColor(0.05f, 0.50f, 0.70f), // dark
		UColor(0.0f, 0.0f, 0.0f), // text
		UColor(0.933f, 0.96f, 1.0f) // base
	);
	return new UPalette(active, inactive, inactive);
}
*/
UFont *
UBasicLookAndFeel::createControlFont() {
	return new UFont(UFontInfo::SansSerif, 14);
}

UFont *
UBasicLookAndFeel::createSystemFont() {
	return createControlFont();
}

UFont *
UBasicLookAndFeel::createTitleFont() {
	return createControlFont();
}

UFont *
UBasicLookAndFeel::createUserFont() {
	return createControlFont();
}

#include "ufo/image/uxbmicon.hpp"
#include "if.xbm"

UIcon *
UBasicLookAndFeel::createInternalFrameDefaultIcon() {
	return new UXBMIcon(if_default_bits, if_default_width, if_default_height);
	//return new UImageIcon("basic/if_default.pnm");
}

UIcon *
UBasicLookAndFeel::createInternalFrameMaximizeIcon() {
	return new UXBMIcon(if_maximize_bits, if_maximize_width, if_maximize_height);
	//return new UImageIcon("basic/if_maximize.pnm");
}

UIcon *
UBasicLookAndFeel::createInternalFrameMinimizeIcon() {
	return new UXBMIcon(if_minimize_bits, if_minimize_width, if_minimize_height);
	//return new UImageIcon("basic/if_minimize.pnm");
}

UIcon *
UBasicLookAndFeel::createInternalFrameCloseIcon() {
	return new UXBMIcon(if_close_bits, if_close_width, if_close_height);
	//return new UImageIcon("basic/if_close.pnm");
}

#include "ufo/gl/ugl_driver.hpp"
class DefaultCheckBoxIcon : public UIcon {
	bool m_selected;
	bool m_inactive;
	int m_width;
	int m_height;
public:
	DefaultCheckBoxIcon(bool selected = false, bool inactive = false,
			int width = 12, int height = 12)
		: m_selected(selected)
		, m_inactive(inactive)
		, m_width(width)
		, m_height(height)
	{}
	void paintIcon(UGraphics * g, UWidget * widget, int x, int y) {
		/*glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_QUADS);
		glVertex2i(x, y);
		glVertex2i(x, y + 12);
		glVertex2i(x + 12, y + 12);
		glVertex2i(x + 12, y);
		glEnd();*/
		if (m_inactive) {
			ugl_driver->glColor3f(0.5f, 0.5f, 0.5f);
		} else {
			ugl_driver->glColor3f(0.0f, 0.0f, 0.0f);
		}
		ugl_driver->glRecti(x, y, x + 12, y + 12);
		ugl_driver->glColor3f(1.0f, 1.0f, 1.0f);
		ugl_driver->glRecti(x + 1, y + 1, x + 10, y + 10);
		if (m_selected) {
			ugl_driver->glColor3f(0.0f, 0.0f, 0.0f);
			ugl_driver->glBegin(GL_LINES);
			ugl_driver->glVertex2i(x + 2, y + 2);
			ugl_driver->glVertex2i(x + 9, y + 9);
			ugl_driver->glVertex2i(x + 8, y + 2);
			ugl_driver->glVertex2i(x + 1, y + 9);
			ugl_driver->glEnd();
		}
	}

	int getIconWidth() const { return 12; }
	int getIconHeight() const { return 12; }
};


UIcon *
UBasicLookAndFeel::createCheckBoxDefaultIcon() {
	return new DefaultCheckBoxIcon();
	//return new UImageIcon("basic/cb_default.pnm");
}

UIcon *
UBasicLookAndFeel::createCheckBoxSelectedIcon() {
	return new DefaultCheckBoxIcon(true);
	//return new UImageIcon("basic/cb_selected.pnm");
}

UIcon *
UBasicLookAndFeel::createCheckBoxInactiveIcon() {
	return new DefaultCheckBoxIcon(false, true);
	//return new UImageIcon("basic/cb_inactive.pnm");
}

class DefaultRadioButtonIcon : public UIcon {
	bool m_selected;
	bool m_inactive;
	int m_width;
	int m_height;
public:
	DefaultRadioButtonIcon(bool selected = false, bool inactive = false,
			int width = 12, int height = 12)
		: m_selected(selected)
		, m_inactive(inactive)
		, m_width(width)
		, m_height(height)
	{}
	void paintIcon(UGraphics * g, UWidget * widget, int x, int y) {
		if (m_inactive) {
			ugl_driver->glColor3f(0.5f, 0.5f, 0.5f);
		} else {
			ugl_driver->glColor3f(0.0f, 0.0f, 0.0f);
		}
		ugl_driver->glRecti(x, y, x + 12, y + 12);
		ugl_driver->glColor3f(1.0f, 1.0f, 1.0f);
		ugl_driver->glRecti(x + 1, y + 1, x + 10, y + 10);
		if (m_selected) {
			ugl_driver->glColor3f(0.0f, 0.0f, 0.0f);
			ugl_driver->glBegin(GL_LINE_LOOP);
			ugl_driver->glVertex2i(x + 4, y + 2);
			ugl_driver->glVertex2i(x + 2, y + 4);
			ugl_driver->glVertex2i(x + 2, y + 6);
			ugl_driver->glVertex2i(x + 4, y + 8);
			ugl_driver->glVertex2i(x + 6, y + 8);
			ugl_driver->glVertex2i(x + 8, y + 6);
			ugl_driver->glVertex2i(x + 8, y + 4);
			ugl_driver->glVertex2i(x + 6, y + 2);
			ugl_driver->glEnd();
		}
	}

	int getIconWidth() const { return 12; }
	int getIconHeight() const { return 12; }
};

UIcon *
UBasicLookAndFeel::createRadioButtonDefaultIcon() {
	return new DefaultRadioButtonIcon();
}

UIcon *
UBasicLookAndFeel::createRadioButtonSelectedIcon() {
	return new DefaultRadioButtonIcon(true);
}

UIcon *
UBasicLookAndFeel::createRadioButtonInactiveIcon() {
	return new DefaultRadioButtonIcon(false, true);
}
