/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/widgets/uitem.cpp
    begin             : Fri May 30 2003
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

#include "ufo/widgets/uitem.hpp"

#include "ufo/widgets/uwidget.hpp"

#include "ufo/uicon.hpp"
#include "ufo/ugraphics.hpp"

#include "ufo/font/ufont.hpp"
#include "ufo/font/ufontmetrics.hpp"

#include "ufo/ui/ustylehints.hpp"

using namespace ufo;


UFO_IMPLEMENT_ABSTRACT_CLASS(UItem, UObject)
UFO_IMPLEMENT_DYNAMIC_CLASS(UStringItem, UItem)

//
// class UStringItem
//

UStringItem::UStringItem()
	: m_icon(NULL)
	, m_text("")
{}
UStringItem::UStringItem(const std::string & str)
	: m_icon(NULL)
	, m_text(str)
{}
UStringItem::UStringItem(UIcon * i)
	: m_icon(i)
	, m_text("")
{}
UStringItem::UStringItem(const std::string & str, UIcon * icon)
	: m_icon(icon)
	, m_text(str)
{}

void
UStringItem::paintItem(
		UGraphics * g,
		const URectangle & rect,
		const UStyleHints * hints,
		uint32_t state,
		const UWidget * parent) {
	UColor bg;
	if (state & WidgetSelected) {
		bg = hints->palette.highlight();
	} else {
		bg = hints->palette.base();
		if (hints->opacity != 1.0f) {
			bg.getFloat()[3] = hints->opacity;
		}
	}
	g->setColor(bg);
	int height = g->getStringSize(m_text).getHeight();
	if (m_icon) {
		std::max(m_icon->getIconSize().h, height);
	}
	g->fillRect(rect);

	if (state & WidgetSelected) {
		g->setColor(hints->palette.highlightedText());
	} else {
		g->setColor(hints->palette.text());
	}
	int x = rect.x;
	if (m_icon) {
		m_icon->paintIcon(g, x, rect.y, hints, state);
		x += m_icon->getIconSize().w + 4;
	}
	g->drawString(m_text, x, rect.y);
}

UDimension
UStringItem::getItemSize(
		const UDimension & maxSize,
		const UStyleHints * hints,
		const UWidget * parent) const {
	UDimension ret;
	const UFontMetrics * metrics = hints->font.getFontMetrics();
	ret.w = metrics->getStringWidth(m_text);
	ret.h = metrics->getHeight();
	if (m_icon) {
		ret.w += m_icon->getIconSize().w + 4;
		ret.h = std::max(m_icon->getIconSize().h, ret.h);
	}
	return ret;
	//return ULabel::getSize();
}

std::string
UStringItem::itemToString() const {
	return m_text;
}

void
UStringItem::install(UWidget * parent)
{}

void
UStringItem::uninstall(UWidget * parent)
{}

std::ostream &
UStringItem::paramString(std::ostream & os) const {
	return os << "\"" << itemToString() << "\"";
}
