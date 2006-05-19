/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/upalette.cpp
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

#include <ufo/util/upalette.hpp>

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UPalette, UObject)


UPalette::UPalette()
{}

UPalette::UPalette(
		const UColor & background, const UColor & foreground,
		const UColor & base, const UColor & text,
		const UColor & light, const UColor & midLight,
		const UColor & dark, const UColor & mid,
		const UColor & highlight, const UColor & highlightedText,
		const UColor & button, const UColor & buttonText,
		const UColor & link, const UColor & linkVisited)
{
	m_colors[Background] = background;
	m_colors[Foreground] = foreground;
	m_colors[Base] = base;
	m_colors[Text] = text;
	m_colors[Light] = light;
	m_colors[MidLight] = midLight;
	m_colors[Dark] = dark;
	m_colors[Mid] = mid;
	m_colors[Highlight] = highlight;
	m_colors[HighlightedText] = highlightedText;
	m_colors[Button] = button;
	m_colors[ButtonText] = buttonText;
	m_colors[Link] = link;
	m_colors[LinkVisited] = linkVisited;
}

UPalette::UPalette(
		const UColor & background, const UColor & foreground,
		const UColor & base, const UColor & text,
		const UColor & highlight, const UColor & highlightedText
	)
{
	m_colors[Background] = background;
	m_colors[Foreground] = foreground;
	m_colors[Base] = base;
	m_colors[Text] = text;
	m_colors[Light] = background.brighter().brighter();
	m_colors[MidLight] = background.brighter();
	m_colors[Dark] = background.darker().darker();
	m_colors[Mid] = background.darker();
	m_colors[Highlight] = highlight;
	m_colors[HighlightedText] = highlightedText;
	m_colors[Button] = background;
	m_colors[ButtonText] = foreground;
	m_colors[Link] = UColor::blue;
	m_colors[LinkVisited] = UColor::magenta;
}

UPalette::~UPalette()
{}

void
UPalette::transcribe(const UPalette & pal) {
	UPalette defColors;
	for (int i = 0; i < MaxColorRoles; ++i) {
		if (pal.m_colors[ColorRole(i)] != defColors.m_colors[ColorRole(i)]) {
			m_colors[ColorRole(i)] = pal.m_colors[ColorRole(i)];
		}
	}
}

void
UPalette::update(const UPalette & pal) {
	UPalette defColors;
	for (int i = 0; i < MaxColorRoles; ++i) {
		if (m_colors[ColorRole(i)] == defColors.m_colors[ColorRole(i)]) {
			m_colors[ColorRole(i)] = pal.m_colors[ColorRole(i)];
		}
	}
}

bool
UPalette::operator==(const UPalette & pal) const {
	UPalette defColors;
	for (int i = 0; i < MaxColorRoles; ++i) {
		if (m_colors[ColorRole(i)] != defColors.m_colors[ColorRole(i)]) {
			return false;
		}
	}
	return true;
}

std::ostream &
UPalette::paramString(std::ostream & os) const {
	return os
	<< "bg = " << (m_colors)[Background]
	<< "; fg = " << (m_colors)[Foreground]
	<< "; base = " << (m_colors)[Base]
	<< "; text = " << (m_colors)[Text]
	<< "; light = " << (m_colors)[Light]
	<< "; dark = " << (m_colors)[Dark]
	<< "; highlight = " << (m_colors)[Highlight]
	<< "; highlightedText = " << (m_colors)[HighlightedText];
}
