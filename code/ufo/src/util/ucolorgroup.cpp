/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/util/ucolorgroup.cpp
    begin             : Fri Jun 11 2004
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

#include <ufo/util/ucolorgroup.hpp>

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UColorGroup, UObject)

UColorGroup::UColorGroup()
{}

UColorGroup::UColorGroup(const UColor & prim1, const UColor & prim2, const UColor & prim3,
	const UColor & sec1, const UColor & sec2, const UColor & sec3) {

	m_colors[Base] = prim1;
	m_colors[BaseFore] = sec1;
	m_colors[Background] = prim1;
	m_colors[Foreground] = sec1;
	m_colors[Text] = UColor::black;
	m_colors[Light] = prim2;
	m_colors[Dark] = sec2;
	m_colors[MidLight] = prim3;
	m_colors[Highlight] = prim2;
	m_colors[HighlightedText] = UColor::white;
}


UColorGroup::UColorGroup(
	const UColor & base, const UColor & baseFore,
	const UColor & background, const UColor & foreground,
	const UColor & text,
	const UColor & light, const UColor & dark,
	const UColor & midLight, const UColor & highlight,
	const UColor & highlightedText
)
{
	m_colors[Base] = base;
	m_colors[BaseFore] = baseFore;
	m_colors[Background] = background;
	m_colors[Foreground] = foreground;
	m_colors[Text] = text;
	m_colors[Light] = light;
	m_colors[Dark] = dark;
	m_colors[MidLight] = midLight;
	m_colors[Highlight] = highlight;
	m_colors[HighlightedText] = highlightedText;
}

UColorGroup::~UColorGroup() 
{}

bool
UColorGroup::operator==(const UColorGroup & group) const {
	return (
		m_colors[Base] == group.getColor(Base) &&
		m_colors[BaseFore] == group.getColor(BaseFore) &&
		m_colors[Background] == group.getColor(Background) &&
		m_colors[Foreground] == group.getColor(Foreground) &&
		m_colors[Text] == group.getColor(Text) &&
		m_colors[Light] == group.getColor(Light) &&
		m_colors[Dark] == group.getColor(Dark) &&
		m_colors[MidLight] == group.getColor(MidLight) &&
		m_colors[Highlight] == group.getColor(Highlight) &&
		m_colors[HighlightedText] == group.getColor(HighlightedText)
	);
}

std::ostream & 
UColorGroup::paramString(std::ostream & os) const {
	return os 
	<< "base = " << (m_colors)[Base]
	<< "; baseF = " << (m_colors)[BaseFore]
	<< "; bg = " << (m_colors)[Background]
	<< "; fg = " << (m_colors)[Foreground]
	<< "; text = " << (m_colors)[Text]
	<< "; light = " << (m_colors)[Light]
	<< "; dark = " << (m_colors)[Dark]
	<< "; midlight = " << (m_colors)[MidLight]
	<< "; highlight = " << (m_colors)[Highlight]
	<< "; highlightedText = " << (m_colors)[HighlightedText];
}
/*
void
UColorGroup::detach() {
	if (m_colors.refCount() > 1) {
		// FIXME
		// whee! does this work?
		UColor * colors = new UColor[MaxColorRoles];
		m_colors = &colors;
	}
}
*/
