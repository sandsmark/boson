/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ucollectable.cpp
    begin             : Sat Jan 26 2002
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

#include "ufo/ucollectable.hpp"

#include <iostream>

namespace ufo {

bool UCollectable::m_dynamicAlloc = false;

UCollectable::UCollectable() : m_refCount(0), m_isDynamic(0) {
	// evil
	// auto set to dynamic if created dynamically
	if (m_dynamicAlloc) {
		m_isDynamic = true;
		m_dynamicAlloc = false;
	}
}

UCollectable::~UCollectable() {
}

// FIXME
// hm, evil
void *
UCollectable::operator new(std::size_t size) {
	m_dynamicAlloc = true;
	return ::operator new(size);
}

} // namespace ufo
