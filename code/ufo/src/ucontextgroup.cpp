/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ucontextgroup.cpp
    begin             : Thu Feb 19 2004
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

#include "ufo/ucontextgroup.hpp"

#include "ufo/ucontext.hpp"
#include "ufo/utoolkit.hpp"
#include "ufo/uvolatiledata.hpp"

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UContextGroup, UObject)


UContextGroup::UContextGroup()
	: m_contexts()
	, m_volatileData()
	, m_refreshTime(0)
{}

UContextGroup::~UContextGroup() {
	// unreference volatile data
	for(std::vector<UVolatileData*>::iterator next_iter, iter = m_volatileData.begin();
			iter != m_volatileData.end();
			++iter) {
		(*iter)->unreference();
	}
}

std::vector<UContext*>
UContextGroup::getContexts() {
	return m_contexts;
}

bool
UContextGroup::belongsToGroup(UContext * context) {
	return (std::find(m_contexts.begin(), m_contexts.end(), context) !=
		m_contexts.end());
}

void
UContextGroup::refresh() {
	m_refreshTime = UToolkit::getToolkit()->getTicks();

	for(std::vector<UVolatileData*>::iterator next_iter, iter = m_volatileData.begin();
			iter != m_volatileData.end();
			iter = next_iter) {
		next_iter = iter;
		++next_iter;
		if ((*iter)->getReferenceCount() == 1) {
			// this object is ready for deletion
			(*iter)->unreference();
			m_volatileData.erase(iter);
		} else {
			(*iter)->refresh();
		}
	}

	for(std::vector<UContext*>::iterator citer = m_contexts.begin();
			citer != m_contexts.end();
			++citer) {
		(*citer)->refresh();
	}
}

void
UContextGroup::addVolatileData(UVolatileData * vdata) {
	if (std::find(m_volatileData.begin(), m_volatileData.end(), vdata) ==
			m_volatileData.end()) {
		m_volatileData.push_back(vdata);
		// references the volatile data
		vdata->reference();
	}
}

void
UContextGroup::removeVolatileData(UVolatileData * vdata) {
	std::vector<UVolatileData*>::iterator iter =
		std::find(m_volatileData.begin(), m_volatileData.end(), vdata);

	if (iter != m_volatileData.end()) {
		m_volatileData.erase(iter);
		vdata->unreference();
	}
}

void
UContextGroup::addContext(UContext * context) {
	if (std::find(m_contexts.begin(), m_contexts.end(), context) ==
			m_contexts.end()) {
		m_contexts.push_back(context);
	}
}

void
UContextGroup::removeContext(UContext * context) {
	m_contexts.erase(
		std::find(m_contexts.begin(), m_contexts.end(), context)
	);
}
