/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ui/ustylemanager.cpp
    begin             : Mon Feb 28 2005
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

#include "ufo/ui/ustylemanager.hpp"

#include "ufo/ugraphics.hpp"
#include "ufo/font/ufont.hpp"
#include "ufo/font/ufontmetrics.hpp"

#include "ufo/ui/ustylehints.hpp"
#include "ufo/ui/ucss.hpp"
//#include "ufo/gl/ugl_style.hpp"
#include "ufo/ui/ubasicstyle.hpp"
//#include "ufo/gl/ugl_driver.hpp"

#include "ufo/widgets/uwidget.hpp"
#include "ufo/widgets/ulabel.hpp"
#include "ufo/widgets/ubutton.hpp"

#include "ufo/umodel.hpp"

using namespace ufo;

UFO_IMPLEMENT_ABSTRACT_CLASS(UStyle, UObject)

UStyleManager::UStyleManager()
	: m_style(NULL)
	, m_hints()
{
	m_style = new UBasicStyle();
	m_style->reference();

	UFont font(UFontInfo::SansSerif, 14);
	// FIXME: Add a possibilty to create a system palette
	UPalette palette(
		UColor(0.93f, 0.93f, 0.90f), // background
		UColor(0.0f, 0.0f, 0.0f), // foreground
		UColor(1.0f, 1.0f, 1.0f), // base
		UColor(0.0f, 0.0f, 0.0f), // text
		UColor(0.5f, 0.62f, 0.78f), // highlight
		UColor(1.0f, 1.0f, 1.0f) // highlightedText
	);

	// FIXME: mem leak
	UStyleHints * hints = new UStyleHints();
	hints->minimumSize = UDimension();
	hints->maximumSize = UDimension::maxDimension;
	hints->preferredSize = UDimension::invalid;
	hints->border->borderType = NoBorder;
	hints->border->color[0] = UColor(0.73f, 0.73f, 0.70f);
	hints->margin = UInsets();
	//hints->horizontalAlignment = AlignLeft;
	//hints->verticalAlignment = AlignTop;
	hints->hAlignment = AlignStart;
	hints->vAlignment = AlignStart;
	hints->direction = Up;
	hints->orientation = Horizontal;
	hints->font = font;
	hints->palette = palette;
	hints->opacity = 1.0f;
	hints->background = NULL;
	m_hints["default"] = hints;


	UStyleHints * widget = new UStyleHints();
	m_hints["widget"] = widget;

	UStyleHints * buttons = new UStyleHints();
	buttons->border->borderType = StyleBorder;
	buttons->margin = UInsets(2, 3, 2, 3);
	buttons->hAlignment = AlignStart;
	buttons->vAlignment = AlignCenter;
	m_hints["button"] = buttons;

	UStyleHints * menuitem = new UStyleHints();
	menuitem->margin = UInsets(2, 2, 2, 2);
	menuitem->border->borderType = StyleBorder;
	menuitem->hAlignment = AlignLeft;
	m_hints["menuitem"] = menuitem;

	UStyleHints * separator = new UStyleHints();
	m_hints["separator"] = separator;

	UStyleHints * menubar = new UStyleHints();
	menubar->border->borderType = BottomLineBorder;
	menubar->border->color[0] = UColor(0.53f, 0.53f, 0.50f);
	m_hints["menubar"] = menubar;

	UStyleHints * popup = new UStyleHints();
	popup->border->borderType = LineBorder;
	popup->hAlignment = AlignStretch;
	popup->vAlignment = AlignStart;
	m_hints["popupmenu"] = popup;

	UStyleHints * iframe = new UStyleHints();
	iframe->border->borderType = StyleBorder;
	m_hints["internalframe"] = iframe;

	UStyleHints * textedit = new UStyleHints();
	textedit->border->borderType = LineBorder;
	textedit->border->color[0] = UColor(0.0f, 0.0f, 0.0f);

	m_hints["textedit"] = textedit;
	m_hints["listbox"] = textedit->clone();

	UStyleHints * transparent = new UStyleHints();
	transparent->opacity = 0.0f;
	m_hints[".transparent"] = transparent;
}

UStyleManager::UStyleManager(UStyle * style,
		std::map<std::string, UStyleHints*> hints)
	: m_style(style)
	, m_hints(hints)
{
}

UStyleManager::~UStyleManager() {
	for (std::map<std::string, UStyleHints*>::iterator iter = m_hints.begin();
			iter != m_hints.end();
			++iter) {
		if ((*iter).second) {
			delete ((*iter).second);
		}
	}
	m_hints.clear();
	if (m_style) {
		m_style->unreference();
	}
}


UStyle *
UStyleManager::getStyle() {
	if (m_style) {
		return m_style;
	}
	/*if (m_parent) {
		return m_parent->getStyle();
	} else */{
		std::cerr << " Fatal ERROR: No style object found.\n";
		return NULL;
	}
}

void
UStyleManager::setStyle(UStyle * style) {
	if (style) {
		style->reference();
		m_style->unreference();
		m_style = style;
	}
}

UFont
UStyleManager::getFont() {
	if (m_hints["default"]) {
		return m_hints["default"]->font;
	}
	return UFont();
}

void
UStyleManager::setFont(const UFont & font) {
	// FIXME: Oops, should we check whether default exists?
	m_hints["default"]->font = font;
}

UPalette
UStyleManager::getPalette() {
	if (m_hints["default"]) {
		return m_hints["default"]->palette;
	}
	return UPalette();
}

void
UStyleManager::setPalette(const UPalette & palette) {
	// FIXME: Oops, should we check whether default exists?
	m_hints["default"]->palette = palette;
}

std::string
concat(const std::string & type,
		const std::string & classId,
		const std::string & name) {
	std::string ret(type);
	if (classId != "") {
		ret += '.';
		ret.append(classId);
	}
	if (name != "") {
		ret += '#';
		ret.append(name);
	}
	return ret;
}

void
UStyleManager::putStyleHints(const std::string & classid, UStyleHints * styleHints) {
	if (styleHints) {
		m_hints[classid] = styleHints->clone();;
	}
}

void
UStyleManager::putStyleHints(
		const std::string & type,
		const std::string & classId,
		const std::string & name,
		UStyleHints * styleHints) {
	if (styleHints) {
		m_hints[concat(type, classId, name)] = styleHints->clone();;
	}
}

UStyleHints *
UStyleManager::getStyleHints(
		const std::string & type,
		const std::string & classId,
		const std::string & name) {
	// FIXME: we get only
	UStyleHints * hints = NULL;
	std::string type_class(concat(type, classId, ""));
	if (m_hints[type_class]) {
		m_hints[type_class]->update(m_hints["default"]);
		return m_hints[type_class];
	} else if (m_hints[type]) {
		hints = m_hints[type];
	}/* else if (m_parent) {
		hints = m_parent->getStyleHints(type, classId, name);
	}*/
	if (!hints) {
		hints = m_hints["default"];
	} else {
		hints->update(m_hints["default"]);
	}
	// have we CSS style?
	if (m_hints[concat("", classId, "")]) {
		hints = hints->clone();
		hints->transcribe(m_hints[concat("", classId, "")]);
		// save a copy
		m_hints[concat(type, classId, "")] = hints;
	}
	return hints;
}

void
UStyleManager::loadStyleSheet(const std::string & fileName) {
	UCss css(fileName);
	std::map<std::string, UStyleHints*> newHints = css.getStyleHints();
	for (std::map<std::string, UStyleHints*>::const_iterator iter = newHints.begin();
			iter != newHints.end();
			++iter) {
		if (m_hints[(*iter).first]) {
			m_hints[(*iter).first]->transcribe((*iter).second);
		} else {
			UStyleHints * hints = ((*iter).second)->clone();
			hints->update(getStyleHints((*iter).first));
			m_hints[(*iter).first] = hints;
		}
	}
}
