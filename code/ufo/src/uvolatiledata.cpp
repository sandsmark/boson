/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/uvolatiledata.cpp
    begin             : Thu Feb 26 2004
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

#include "ufo/uvolatiledata.hpp"

#include "ufo/udisplay.hpp"
#include "ufo/utoolkit.hpp"

using namespace ufo;

UFO_IMPLEMENT_ABSTRACT_CLASS(UVolatileData, UObject)


UVolatileData::UVolatileData(UDisplay * display)
	: m_display(display)
	, m_refreshTime(0)
{
	if (display == NULL && UDisplay::getDefault()) {
		m_display = UDisplay::getDefault();
	}
	if (m_display) {
		m_display->addVolatileData(this);
	}
}

UVolatileData::~UVolatileData() {
	if (m_display) {
		m_display->removeVolatileData(this);
	}
}

bool
UVolatileData::needsRefresh() const {
	return false;//(m_contextGroup->getLastRefreshTime() > m_refreshTime);
}


void
UVolatileData::updateRefreshTime() {
	m_refreshTime = UToolkit::getToolkit()->getTicks();
}


void
UVolatileData::setDisplay(UDisplay * display) {
	m_display = display;
}
