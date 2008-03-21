/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/widgets/ulabel.cpp
    begin             : Wed May 23 2001
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA *
 ***************************************************************************/

#include "ufo/widgets/ulabel.hpp"

#include "ufo/uicon.hpp"

#include "ufo/ui/ustyle.hpp"
#include "ufo/ui/ustylehints.hpp"
// for shortcut events
#include "ufo/events/ushortcutevent.hpp"
#include "ufo/widgets/ubutton.hpp"

using namespace ufo;


UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(ULabel, UCompound)

ULabel::ULabel()
	: UCompound()
	, m_buddy(NULL)
{
	setCssType("label");
}

ULabel::ULabel(UIcon * icon)
	: UCompound(icon)
	, m_buddy(NULL)
{
	setCssType("label");
}

ULabel::ULabel(const std::string & text, UIcon * icon)
	: UCompound(text, icon)
	, m_buddy(NULL)
{
	setCssType("label");
}

ULabel::ULabel(const std::string & text, UWidget * buddy)
	: UCompound(text)
	, m_buddy(buddy)
{
	setCssType("label");
}

ULabel::ULabel(const std::string & text, UIcon * icon, UWidget * buddy)
	: UCompound(text, icon)
	, m_buddy(buddy)
{
	setCssType("label");
}

void
ULabel::setBuddy(UWidget * buddy) {
	m_buddy = buddy;
}

UWidget *
ULabel::getBuddy() const {
	return m_buddy;
}

UDimension
ULabel::getContentsSize(const UDimension & maxSize) const {
	UDimension ret(getStyle()->getCompoundPreferredSize(
		getStyleHints(),
		getText(),
		getIcon())
	);

	if (ret.isValid()) {
		ret.clamp(maxSize);
		return ret;
	}
	return UDimension::invalid;
}

void
ULabel::processShortcutEvent(UShortcutEvent * e) {
	if (isVisible() && isEnabled() && m_buddy) {
		m_buddy->requestFocus();
		if (!e->isAmbiguous()) {
			if (UButton * button = dynamic_cast<UButton*>(m_buddy)) {
				button->doClick();
			}
		}
		e->consume();
	}
	UWidget::processShortcutEvent(e);
}
