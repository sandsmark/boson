/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : uvolatiledata.cpp
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#include "ufo/uvolatiledata.hpp"

#include "ufo/ucontextgroup.hpp"
#include "ufo/utoolkit.hpp"

using namespace ufo;

UFO_IMPLEMENT_ABSTRACT_CLASS(UVolatileData, UObject)


UVolatileData::UVolatileData(UContextGroup * group)
	: m_contextGroup(group)
	, m_refreshTime(0)
{
	if (m_contextGroup == NULL) {
		m_contextGroup =
			UToolkit::getToolkit()->getCurrentContext()->getContextGroup();
	}
}


bool
UVolatileData::needsRefresh() const {
	return (m_contextGroup->getLastRefreshTime() > m_refreshTime);
}


void
UVolatileData::updateRefreshTime() {
	m_refreshTime = UToolkit::getToolkit()->getTicks();
}


void
UVolatileData::setContextGroup(UContextGroup * group) {
	m_contextGroup = group;
}
