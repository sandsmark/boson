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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA *
 ***************************************************************************/

#include "ufo/layouts/uboxlayout.hpp"

#include "ufo/util/udimension.hpp"
#include "ufo/util/uinteger.hpp"
#include "ufo/widgets/uwidget.hpp"

#include <algorithm>

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UBoxLayout, ULayoutManager)

UBoxLayout::UBoxLayout()
	: m_orientation(NoOrientation)
	, m_hgap(2)
	, m_vgap(2)
{}

UBoxLayout::UBoxLayout(int orientation)
	: m_orientation(orientation)
	, m_hgap(2)
	, m_vgap(2)
{}

UBoxLayout::UBoxLayout(int hgap, int vgap)
	: m_orientation(NoOrientation)
	, m_hgap(hgap)
	, m_vgap(vgap)
{}

UBoxLayout::UBoxLayout(int orientation, int hgap, int vgap)
	: m_orientation(orientation)
	, m_hgap(hgap)
	, m_vgap(vgap)
{}

UBoxLayout::~UBoxLayout() {}

UDimension
UBoxLayout::getPreferredLayoutSize(const UWidget * container,
		const UDimension & maxSize) const {
	UDimension ret;
	int orientation = m_orientation;
	if (orientation == NoOrientation) {
		orientation = container->getOrientation();
	}

	for (unsigned int i = 0; i < container->getWidgetCount(); i++) {
		UWidget * w = container->getWidget(i);

		if (w->isVisible() || !w->testState(WidgetForceInvisible)) {
			// FIXME: Resize maxSize to respect already "consumed" space
			const UDimension & size = w->getPreferredSize(maxSize);

			if (orientation == Horizontal) {
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
	if (orientation == Horizontal) {
		ret.w -= m_hgap;
	} else {
		ret.h -= m_vgap;
	}

	return ret;
}
/*
void
UBoxLayout::layoutContainer(const UWidget * container) {
	URectangle bounds = container->getInnerBounds();

	int x = bounds.x;
	int y = bounds.y;

	for (unsigned int i = 0 ; i < container->getWidgetCount() ; i++) {
		UWidget * w = container->getWidget(i);

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
*/

int
getFlex(const UWidget * widget)  {
	if (UInteger * flex = dynamic_cast<UInteger*>(widget->get("layout"))) {
		return *flex;
	}
	return 0;
}

void
UBoxLayout::layoutContainer(const UWidget * container) {
	UDimension prefSize;
	if (container->getParent()) {
		prefSize = getPreferredLayoutSize(container, container->getParent()->getSize());
	} else if (!container->getSize().isEmpty()) {
		prefSize = container->getSize();
	} else {
		prefSize = UDimension::maxDimension;
	}
	UDimension size = container->getSize();
	int totalFlex = getTotalFlex(container);
	float ratio = (totalFlex != 0) ? 1.0f / totalFlex : 0.0f;

	int orientation = (m_orientation != NoOrientation) ?
		m_orientation : container->getOrientation();
	if (orientation == NoOrientation) {
		orientation = Horizontal;
	}

	int addSize = 0;
	if (orientation == Vertical) {
		addSize = size.h - prefSize.h;
	} else {
		addSize = size.w - prefSize.w;
	}
	// FIXME: should we shrink?
	if (addSize < 0) {
		addSize = 0;
	}

	URectangle bounds = container->getInnerBounds();

	int x = bounds.x;
	int y = bounds.y;
	int width = 0;
	int height = 0;
	int alignx = container->getHorizontalAlignment();
	int aligny = container->getVerticalAlignment();

	if (totalFlex == 0 && alignx == AlignRight) {
		x += addSize;
	} else if (totalFlex == 0 && alignx == AlignCenter) {
		x += int(addSize / 2.0f);
	}

	for (unsigned int i = 0 ; i < container->getWidgetCount() ; i++) {
		UWidget * w = container->getWidget(i);

		if (w->isVisible()) {
			const UDimension & d = w->getPreferredSize();

			if (orientation == Horizontal) {
				width = d.w;
				if (addSize) {
					width += int(getFlex(w) * ratio * addSize);
				}

				if (aligny == AlignStretch) {
					height = bounds.h;
					y = bounds.y;
				} else if (aligny == AlignTop) {
					height = d.h;
					y = bounds.y;
				} else if (aligny == AlignBottom) {
					height = d.h;
					y = bounds.h - d.h;
				} else { //if (aligny == AlignCenter) {
					height = d.h;
					y = int(bounds.h / 2.0f - d.h / 2.0f);
				}
				w->setBounds(x, y, width, height);

				x += width;
				x += m_hgap;
			} else { // orientation == Vertical
				height = d.h;
				if (addSize) {
					height += int(getFlex(w) * ratio * addSize);
				}
				if (alignx == AlignStretch) {
					width = bounds.w;
					x = bounds.x;
				} else if (alignx == AlignTop) {
					width = d.w;
					x = bounds.x;
				} else if (alignx == AlignBottom) {
					width = d.w;
					x = bounds.w - d.w;
				} else { //if (alignx == AlignCenter) {
					width = d.w;
					x = int(bounds.w / 2.0f - d.w / 2.0f);
				}
				w->setBounds(x, y, width, height);

				y += height;
				y += m_vgap;
			}
		}
	}
}

int
UBoxLayout::getTotalFlex(const UWidget * container) {
	int ret = 0;
	for (unsigned int i = 0; i  < container->getWidgetCount(); ++i) {
		ret += getFlex(container->getWidget(i));
	}
	return ret;
}
