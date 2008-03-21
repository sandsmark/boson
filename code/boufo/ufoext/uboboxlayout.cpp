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

#include "uboboxlayout.h"

#include <ufo/util/udimension.hpp>
#include <ufo/util/uinteger.hpp>
#include <ufo/widgets/uwidget.hpp>

#include <algorithm>

using namespace ufo;

static std::vector<URectangle>
getBounds(int orientation, int hgap, int vgap, const UWidget* container, const URectangle & bounds, bool useAvailableSize = true);

UFO_IMPLEMENT_DYNAMIC_CLASS(UBoBoxLayout, ULayoutManager)

UBoBoxLayout::UBoBoxLayout()
	: m_orientation(NoOrientation)
	, m_hgap(2)
	, m_vgap(2)
{}

UBoBoxLayout::UBoBoxLayout(int orientation)
	: m_orientation(orientation)
	, m_hgap(2)
	, m_vgap(2)
{}

UBoBoxLayout::UBoBoxLayout(int hgap, int vgap)
	: m_orientation(NoOrientation)
	, m_hgap(hgap)
	, m_vgap(vgap)
{}

UBoBoxLayout::UBoBoxLayout(int orientation, int hgap, int vgap)
	: m_orientation(orientation)
	, m_hgap(hgap)
	, m_vgap(vgap)
{}

UBoBoxLayout::~UBoBoxLayout() {}

UDimension
UBoBoxLayout::getPreferredLayoutSize(const UWidget * container,
		const UDimension & maxSize) const {
	int orientation = m_orientation;
	if (orientation == NoOrientation) {
		orientation = container->getOrientation();
	}
	URectangle bounds = container->getInnerBounds();
	// AB: x, y don't matter in this method
	bounds.w = maxSize.w;
	bounds.h = maxSize.h;
	std::vector<URectangle> childBounds = getBounds(orientation, m_hgap, m_vgap, container, bounds, false);
	UDimension ret(0, 0);


	for (unsigned int i = 0; i < container->getWidgetCount(); i++) {
		const URectangle& b = childBounds[i];
		ret.w = std::max(b.x + b.w, ret.w);
		ret.h = std::max(b.y + b.h, ret.h);
	}
	ret.clamp(maxSize);

#if 0
	// AB: the above seems not to work correctly (e.g. for menus).
	// note that with the following code we'll exceed maxSize!
	if (orientation == Horizontal) {
		ret.h = 0;
	} else {
		ret.w = 0;
	}
	for (unsigned int i = 0; i < container->getWidgetCount(); i++) {
		if (orientation == Horizontal) {
			ret.h = std::max(ret.h, childBounds[i].h);
		} else {
			ret.w = std::max(ret.w, childBounds[i].w);
		}
	}
#endif



	return ret;
}

std::vector<URectangle>
getBounds(int orientation, int hgap, int vgap, const UWidget * container, const URectangle & bounds, bool useAvailableSize) {
	std::vector<URectangle> ret(container->getWidgetCount());

	int x = bounds.x;
	int y = bounds.y;
	UDimension dim(bounds.w, bounds.h);

	for (unsigned int i = 0 ; i < container->getWidgetCount() ; i++) {
		const UWidget * w = container->getWidget(i);

		if (w->isVisible()) {
			if (orientation == Horizontal) {
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
			ret[i].w = d.w;
			ret[i].h = d.h;
			if (orientation == Horizontal) {
				x += d.w;
				dim.w -= d.w;
			} else {
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

	// When useAvailableSize is true, we use the whole size in bounds, by
	// assigning additional space to the widgets. However this is not always
	// intended (getPreferredSize() must not use that).
	if (!useAvailableSize) {
		return ret;
	}

	unsigned int stretchCount = 0;
	unsigned int visibleWidgets = 0;
	for (unsigned int i = 0 ; i < container->getWidgetCount() ; i++) {
		const UWidget * w = container->getWidget(i);

		if (w->isVisible()) {
			visibleWidgets++;
			ufo::UObject* o = w->get("stretch");
			if (o) {
				int stretch = UInteger::toInt(o->toString());
				if (stretch > 0) {
					stretchCount += stretch;
				}
			}
		}
	}

	// the visible widgets share the remaining space
	// TODO:
	// 1. maintain the aspect? probably it makes little sense if a widget
	//    with w=10 receives the same additional width as a widget with
	//    w=1000.
	//    testing will reveal whether aspect maintaining looks better or
	//    not.
	// 2. we totally ignore any maximum size currently
	int addW = 0;
	if (visibleWidgets > 0) {
		if (stretchCount > 0) {
			addW = dim.w / stretchCount;
		} else {
			addW = dim.w / visibleWidgets;
		}
	}
	int addH = 0;
	if (visibleWidgets > 0) {
		if (stretchCount > 0) {
			addH = dim.h / stretchCount;
		} else {
			addH = dim.h / visibleWidgets;
		}
	}
	int fixX = 0;
	int fixY = 0;
	for (unsigned int i = 0 ; i < container->getWidgetCount() ; i++) {
		const UWidget * w = container->getWidget(i);

		ret[i].x += fixX;
		ret[i].y += fixY;

		if (w->isVisible()) {
			int stretchFactor = 0;
			if (stretchCount > 0) {
				ufo::UObject* o = w->get("stretch");
				if (o) {
					stretchFactor = UInteger::toInt(o->toString());
					if (stretchFactor < 0) {
						stretchFactor = 0;
					}
				}
			} else {
				stretchFactor = 1;
			}

			if (orientation == Horizontal) {
				int add = addW;
				add *= stretchFactor;

				fixX += add;
				ret[i].w += add;
			} else {
				int add = addH;
				add *= stretchFactor;

				fixY += add;
				ret[i].h += add;
			}
		}
	}
	dim.w = 0;
	dim.h = 0;

	return ret;
}

void
UBoBoxLayout::layoutContainer(const UWidget * container) {
	int orientation = m_orientation;
	if (orientation == NoOrientation) {
		orientation = container->getOrientation();
	}
	const URectangle & bounds = container->getInnerBounds();
	std::vector<URectangle> childBounds = getBounds(orientation, m_hgap, m_vgap, container, bounds);

	for (unsigned int i = 0 ; i < container->getWidgetCount() ; i++) {
		UWidget * w = container->getWidget(i);

		if (w->isVisible()) {
			const URectangle & b = childBounds[i];
			if (orientation == Horizontal) {
				w->setBounds(b.x, b.y, b.w, bounds.h);
			} else {
				w->setBounds(b.x, b.y, bounds.w, b.h);
			}
		}
	}
}

int
UBoBoxLayout::getTotalFlex(const UWidget * container) {
	return 0;
}
