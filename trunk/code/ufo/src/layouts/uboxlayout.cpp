/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
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

static std::vector<URectangle>
getBounds(int axis, int hgap, int vgap, const UWidget* container, const URectangle & bounds);

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
UBoxLayout::getPreferredLayoutSize(const UWidget * container,
		const UDimension & maxSize) const {
	URectangle bounds = container->getInnerBounds();
	// AB: x, y don't matter in this method
	bounds.w = maxSize.w;
	bounds.h = maxSize.h;
	std::vector<URectangle> childBounds = getBounds(m_axis, m_hgap, m_vgap, container, bounds);
	UDimension ret(0, 0);


	if (container->getWidgetCount() >= 1) {
		const URectangle& b = childBounds[container->getWidgetCount() - 1];
		if (m_axis == XAxis) {
			ret = UDimension(b.x + b.w, maxSize.h);
		} else {
			ret = UDimension(maxSize.w, b.y + b.h);
		}
	}

#if 1
	// AB: the above seems not to work correctly (e.g. for menus).
	// note that with the following code we'll exceed maxSize!
	if (m_axis == XAxis) {
		ret.h = 0;
	} else {
		ret.w = 0;
	}
	for (unsigned int i = 0; i < container->getWidgetCount(); i++) {
		if (m_axis == XAxis) {
			ret.h = std::max(ret.h, childBounds[i].h);
		} else {
			ret.w = std::max(ret.w, childBounds[i].w);
		}
	}
#endif


	// Add (border and other) insets
	ret += container->getInsets();

	return ret;
}

std::vector<URectangle>
getBounds(int axis, int hgap, int vgap, const UWidget * container, const URectangle & bounds) {
	std::vector<URectangle> ret(container->getWidgetCount());

	int x = bounds.x;
	int y = bounds.y;
	UDimension dim(bounds.w, bounds.h);

	for (unsigned int i = 0 ; i < container->getWidgetCount() ; i++) {
		const UWidget * w = container->getWidget(i);

		if (w->isVisible()) {
			if (axis == UBoxLayout::XAxis) {
				if (x > bounds.x) {
					x += hgap;
					if (dim.w >= hgap) {
						dim.w -= hgap;
					} else {
						dim.w = 0;
					}
				}
			} else {
				if (y > bounds.y) {
					y += vgap;
					if (dim.h >= vgap) {
						dim.h -= vgap;
					} else {
						dim.h = 0;
					}
				}
			}

			UDimension d = w->getPreferredSize(dim);
			d.clamp(dim);

			ret[i].x = x;
			ret[i].y = y;
			if (axis == UBoxLayout::XAxis) {
				ret[i].w = d.w;
				ret[i].h = d.h;

				x += d.w;
				dim.w -= d.w;
			} else {
				ret[i].w = d.w;
				ret[i].h = d.h;

				y += d.h;
				dim.h -= d.h;
			}
		} else {
			ret[i].x = x;
			ret[i].y = y;
			ret[i].w = 0;
			ret[i].h = 0;
		}
	}
	return ret;
}

void
UBoxLayout::layoutContainer(const UWidget * container) {
	const URectangle & bounds = container->getInnerBounds();
	std::vector<URectangle> childBounds = getBounds(m_axis, m_hgap, m_vgap, container, bounds);

	for (unsigned int i = 0 ; i < container->getWidgetCount() ; i++) {
		UWidget * w = container->getWidget(i);

		if (w->isVisible()) {
			const URectangle & b = childBounds[i];
			if (m_axis == XAxis) {
				w->setBounds(b.x, b.y, b.w, bounds.h);
			} else {
				w->setBounds(b.x, b.y, bounds.w, b.h);
			}
		}
	}
}

