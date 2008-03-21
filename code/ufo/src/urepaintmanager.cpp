/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/urepaintmanager.cpp
    begin             : Fri Nov 1 2002
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

#include "ufo/urepaintmanager.hpp"

#include "ufo/ugraphics.hpp"
#include "ufo/widgets/uwidget.hpp"

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(URepaintManager, UObject)

URepaintManager::URepaintManager()
	: m_dirtyRegion(0, 0, 1, 1)
{}

URepaintManager::~URepaintManager() {}


void
URepaintManager::addDirtyRegion(UWidget * widget, int x, int y, int w, int h) {
	URectangle rect;
	if (widget) {
		rect =  URectangle(widget->getRootLocation() + UPoint(x, y), widget->getSize());
	} else {
		rect = URectangle(x, y, w, h);
	};
	if (m_dirtyRegion.isEmpty()) {
		m_dirtyRegion = rect;
	} else {
		m_dirtyRegion = m_dirtyRegion.computeUnion(rect);
	}
}

void
URepaintManager::addDirtyWidget(UWidget * widget) {
	if (widget) {
		if (m_dirtyRegion.isEmpty()) {
			m_dirtyRegion = widget->getRootBounds();
		} else {
			m_dirtyRegion = m_dirtyRegion.computeUnion(widget->getRootBounds());
		}
	}
}

URectangle
URepaintManager::getDirtyRegion() const {
	return m_dirtyRegion;
}
/*
void
URepaintManager::paintDirtyRegions(UGraphics * g) {
	for (ListType::iterator iter = m_dirty.begin();
			iter != m_dirty.end();
			++iter) {
		(*iter)->paint(g);
		m_dirtyCollector.releasePointer(*iter);
	}
	m_dirty.clear();
}
*/

bool
URepaintManager::isDirty() const {
	return !(m_dirtyRegion.isEmpty());
}


void
URepaintManager::clearDirtyRegions() {
	m_dirtyRegion = URectangle(0, 0, 0, 0);
}

void
URepaintManager::clearDirtyRegion(UWidget * widget, int x, int y, int w, int h) {
	// FIXME oh oh, this is very inefficient
	URectangle rect(widget->getRootLocation() + UPoint(x, y), UDimension(w, h));
	if (m_dirtyRegion.computeUnion(rect) == rect) {
		m_dirtyRegion = URectangle(0, 0, 0, 0);
	}
}
