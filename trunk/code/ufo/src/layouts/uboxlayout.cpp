/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/layouts/upopuplayout.cpp
    begin             : Thu May 31 2001
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

#include "ufo/layouts/uboxlayout.hpp"

#include "ufo/util/udimension.hpp"
#include "ufo/widgets/uwidget.hpp"

#include <algorithm>

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UBoxLayout, ULayoutManager)

UBoxLayout::UBoxLayout() : m_axis(XAxis), m_hgap(2), m_vgap(2) {}

UBoxLayout::UBoxLayout(int axis) : m_axis(axis), m_hgap(2), m_vgap(2) {}

UBoxLayout::UBoxLayout(int hgap, int vgap)
	: m_axis(XAxis)
	, m_hgap(hgap)
	, m_vgap(vgap)
{}

UBoxLayout::UBoxLayout(int axis, int hgap, int vgap)
	: m_axis(axis)
	, m_hgap(hgap)
	, m_vgap(vgap)
{}

UBoxLayout::~UBoxLayout() {}

UDimension
UBoxLayout::getMinimumLayoutSize(const UWidget * parent) const {
	UDimension ret;
	const UInsets & insets = parent->getInsets();

	for (unsigned int i = 0; i < parent->getWidgetCount(); i++ ) {
		UWidget * w = parent->getWidget(i);

		if (w->isVisible()) {
			const UDimension & size = w->getMinimumSize();

			if (m_axis == XAxis ) {
				ret.h = std::max(size.h, ret.h);
				ret.w += size.w;

				ret.w += m_hgap;
			} else {
				ret.w = std::max(size.w, ret.w);
				ret.h += size.h;

				ret.h += m_vgap;
			}
		}
	}
	ret.w += insets.left + insets.right - m_hgap;
	ret.h += insets.top + insets.bottom + 2 * m_vgap;

	return ret;
}

UDimension
UBoxLayout::getPreferredLayoutSize(const UWidget * parent) const {
	UDimension ret;
	const UInsets & insets = parent->getInsets();

	for (unsigned int i = 0; i < parent->getWidgetCount(); i++) {
		UWidget * w = parent->getWidget(i);

		if (w->isVisible()) {
			const UDimension & size = w->getPreferredSize();

			if (m_axis == XAxis) {
				ret.h = std::max(size.h, ret.h);
				ret.w += size.w;

				ret.w += m_hgap;
			} else {
				ret.w = std::max(size.w, ret.w);
				ret.h += size.h;

				ret.h += m_vgap;
			}
		}
	}
	if (m_axis == XAxis)
		ret.w -= m_hgap;
	else
		ret.h -= m_vgap;

	ret.w += insets.left + insets.right;
	ret.h += insets.top + insets.bottom;

	return ret;
}

void
UBoxLayout::layoutContainer(const UWidget * parent) {
	URectangle bounds = parent->getInnerBounds();

	int x = bounds.x;
	int y = bounds.y;

	for (unsigned int i = 0 ; i < parent->getWidgetCount() ; i++) {
		UWidget * w = parent->getWidget(i);

		if (w->isVisible()) {
			const UDimension & d = w->getPreferredSize();

			if (m_axis == XAxis) {
				if (x > bounds.x) {
					x += m_hgap;
				}

				w->setBounds(x, y, d.w, bounds.h);

				x += d.w;
			} else {
				if (y > bounds.y) {
					y += m_vgap;
				}

				w->setBounds(x, y, bounds.w, d.h);

				y += d.h;
			}
		}
	}
}
