/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
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


const UPalette UPalette::nullPalette = 
	UPalette(UColorGroup(), UColorGroup(), UColorGroup());

UPalette::UPalette(const UColorGroup & active, const UColorGroup & disabled, 
		const UColorGroup & inactive)
	: m_active(new UColorGroup(active))
	, m_inactive(new UColorGroup(inactive))
	, m_disabled(new UColorGroup(disabled))
{
}

UPalette::~UPalette()
{}

const UColor &
UPalette::getColor(ColorGroupType group, 
		UColorGroup::ColorRole role) const {
	switch (group) {
		case Active: return m_active->getColor(role);
			break;
		case Inactive: return m_inactive->getColor(role);
			break;
		case Disabled: return m_disabled->getColor(role);
			break;
		default:
			return m_active->getColor(role);
	}
}

void
UPalette::setColor(ColorGroupType group, 
		UColorGroup::ColorRole role, const UColor & color) {
	detach();
	switch (group) {
		case Active: m_active->setColor(role, color);
			break;
		case Inactive: m_inactive->setColor(role, color);
			break;
		case Disabled: m_disabled->setColor(role, color);
			break;
		default:
			break;
	}
}

void
UPalette::setColorGroup(ColorGroupType groupType, const UColorGroup & group) {
	detach();
	switch (groupType) {
		case Active:
			m_active = new UColorGroup(group);
			break;
		case Inactive:
			m_inactive = new UColorGroup(group);
			break;
		case Disabled:
			m_disabled = new UColorGroup(group);
			break;
		default:
			break;
	}
}

void
UPalette::setActive(const UColorGroup & active) {
	detach();
	m_active = new UColorGroup(active);
}

void
UPalette::setInactive(const UColorGroup & inactive) {
	detach();
	m_inactive = new UColorGroup(inactive);
}

void
UPalette::setDisabled(const UColorGroup & disabled) {
	detach();
	m_disabled = new UColorGroup(disabled);
}

bool
UPalette::operator==(const UPalette & pal) const {
	return (
		*m_active == pal.getActive() &&
		*m_inactive == pal.getInactive() &&
		*m_disabled == pal.getDisabled()
	);
}

void
UPalette::detach() {
	if (m_active.refCount() > 1 ||
		m_inactive.refCount() > 1 ||
		m_disabled.refCount() > 1
		) {
		m_active = new UColorGroup(m_active);
		m_inactive = new UColorGroup(m_inactive);
		m_disabled = new UColorGroup(m_disabled);
	}
}

std::ostream & 
UPalette::paramString(std::ostream & os) const {
	return os << "active: " << *m_active
	<< "inactive: " << *m_inactive
	<< "disabled: " << *m_disabled;
}
