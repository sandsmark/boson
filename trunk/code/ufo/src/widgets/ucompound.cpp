/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/widgets/ucompound.cpp
    begin             : Fri Mar 7 2003
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

#include "ufo/widgets/ucompound.hpp"

namespace ufo {


UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(UCompound, UWidget)

UCompound::UCompound()
	: m_text("")
	, m_icon(NULL)
	, m_disabledIcon(NULL)
	, m_iconTextGap(4)
{
	trackPointer(m_icon);
	trackPointer(m_disabledIcon);
}

UCompound::UCompound(const std::string & text)
	: m_text(text)
	, m_icon(NULL)
	, m_disabledIcon(NULL)
	, m_iconTextGap(4)
{
	trackPointer(m_icon);
	trackPointer(m_disabledIcon);
}

UCompound::UCompound(UIcon * icon)
	: m_text("")
	, m_icon(icon)
	, m_disabledIcon(icon)
	, m_iconTextGap(4)
{
	trackPointer(m_icon);
	trackPointer(m_disabledIcon);
}

UCompound::UCompound(const std::string & text, UIcon * icon)
	: m_text(text)
	, m_icon(icon)
	, m_disabledIcon(icon)
	, m_iconTextGap(4)
{
	trackPointer(m_icon);
	trackPointer(m_disabledIcon);
}

void
UCompound::setText(const std::string & text) {
	m_text = text;
	invalidate();
	repaint();
}

void
UCompound::setIcon(UIcon * icon) {
	swapPointers(m_icon, icon);
	m_icon = icon;
	repaint();
}

UIcon *
UCompound::getIcon() const {
	if (!isEnabled()) {
		return m_disabledIcon;
	}
	return m_icon;
}

void
UCompound::setDisabledIcon(UIcon * icon) {
	swapPointers(m_disabledIcon, icon);
	m_disabledIcon = icon;
	repaint();
}

void
UCompound::setIconTextGap(int iconTextGap) {
	m_iconTextGap = iconTextGap;
	invalidate();
	repaint();
}

std::ostream &
UCompound::paramString(std::ostream & os) const {
	os << "\"" << m_text << "\"";

	return UWidget::paramString(os);
}

} // namespace ufo
