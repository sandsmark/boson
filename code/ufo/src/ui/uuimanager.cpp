/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ui/uuimanager.cpp
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

#include "ufo/ui/uuimanager.hpp"

#include "ufo/ui/ustyle.hpp"
#include "ufo/ui/ulookandfeel.hpp"
#include "ufo/ui/uwidgetui.hpp"

#include "ufo/util/ucolor.hpp"
#include "ufo/util/uinteger.hpp"
#include "ufo/font/ufont.hpp"

//#include "ufo/borders/uborder.hpp"

#include "ufo/widgets/uwidget.hpp"

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UUIManager, UObject)

UUIManager::UUIManager()
	: UObject()
	, m_uiDefaults()
	, m_themeDefaults()
	, m_lookAndFeel(NULL)
{}

void
UUIManager::setLookAndFeel(ULookAndFeel * lookAndFeel) {
	if (!lookAndFeel) {
		uWarning() << "UUIManager: Trying to set NULL pointer as look and feel\n";
		return ;
	}
#ifdef DEBUG
	std::cout << "set new UI " << lookAndFeel << std::endl;
#endif
	swapPointers(m_lookAndFeel, lookAndFeel);
	m_lookAndFeel = lookAndFeel;

	// FIXME how do I insert the content of one std::map into another?
	m_uiDefaults.clear();
	m_uiDefaults = lookAndFeel->getDefaults();

	// set theme defaults
	m_themeDefaults.clear();
	m_themeDefaults = lookAndFeel->getThemeDefaults();
}
const
ULookAndFeel * UUIManager::getLookAndFeel() {
	return m_lookAndFeel;
}

UStyle *
UUIManager::getStyle() const {
	if (m_lookAndFeel) {
		return m_lookAndFeel->getStyle();
	}
	return NULL;
}


UWidgetUI *
UUIManager::getUI(UWidget * widget) {
	UI_HANDLER createUI = m_uiDefaults[widget->getUIClassID()];
	if (createUI) {
		return createUI(widget);
	} else {
		// NOTE: throw an exception
		uError() << "UUIManager: Can't find ui delegate for " << widget->toString()
		<< "\nsearching for " << widget->getUIClassID() << " class" << "\n";

		// return NULL;
		// should the application exit here?
		//UApp::getInstance()->shutdown();
		//exit(-2);

#ifdef HAVE_SIGNAL_H
		raise(SIGKILL);
#endif

		return NULL;
	}
}

void
UUIManager::setUI(const std::string & key, UI_HANDLER ui) {
	m_uiDefaults[key] = ui;
}



UObject *
UUIManager::get(const std::string & key) {
	return m_themeDefaults[key];
}

void
UUIManager::put(const std::string & key, UObject * value) {
	m_themeDefaults[key] = value;
}


UPalette
UUIManager::getPalette(const std::string & key) {
	UPalette * pal = dynamic_cast<UPalette*>(m_themeDefaults[key]);
	if (pal) {
		return *pal;
	}
	return UPalette::nullPalette;
}

UColor *
UUIManager::getColor(const std::string & key) {
	return dynamic_cast<UColor*>(m_themeDefaults[key]);
}

UFont *
UUIManager::getFont(const std::string & key) {
	return dynamic_cast<UFont*>(m_themeDefaults[key]);
}

BorderType
UUIManager::getBorder(const std::string & key) {
	UInteger * border = dynamic_cast<UInteger*>(m_themeDefaults[key]);
	if (border) {
		return BorderType(border->toInt());
	}
	return NoBorder;
}


void
UUIManager::refresh() {
	updateRefreshTime();
	//if (m_contextGroup->getLastRefreshTime() >
	//		m_lookAndFeel->getLastRefreshTime()) {
		m_lookAndFeel->refresh();
	//}
	
	// there shouldn't be a need for refreshing in UI classes
	
	// set theme defaults
	m_themeDefaults.clear();
	m_themeDefaults = m_lookAndFeel->getThemeDefaults();
}
