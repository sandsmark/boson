/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/layouts/uborderlayout.cpp
    begin             : Sat Jul 28 2001
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

#include "ufo/layouts/uborderlayout.hpp"

#include <algorithm>

#include "ufo/widgets/uwidget.hpp"
#include "ufo/util/ugeom.hpp"

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UBorderLayout, ULayoutManager)
UFO_IMPLEMENT_ABSTRACT_CLASS(ULayoutManager, UObject)

static void
fixSize(UDimension* size, const UWidget* w, int maxWidth) {
	if (size->w > maxWidth) {
		size->w = maxWidth;
		int h = w->getHeightForWidth(maxWidth);
		if (h > 0) {
			size->h = h;
		}
	}
}

const UString * UBorderLayout::Center = new UString("center");
const UString * UBorderLayout::North = new UString("north");
const UString * UBorderLayout::South = new UString("south");
const UString * UBorderLayout::East = new UString("east");
const UString * UBorderLayout::West = new UString("west");

UBorderLayout::UBorderLayout(int hgap, int vgap)
	: m_hgap(hgap)
	, m_vgap(vgap) {}

UBorderLayout::~UBorderLayout() {}

UDimension
UBorderLayout::getMinimumLayoutSize(const UWidget * parent) const {
	UDimension ret;
	UWidget * w = NULL;

	if ( (w = getChildWidgetAt(parent, East)) ) {
		const UDimension & wd = w->getMinimumSize();
		ret.w = wd.w + m_hgap;
		ret.h = wd.h;
	}
	if ( (w = getChildWidgetAt(parent, West)) ) {
		const UDimension & wd = w->getMinimumSize();
		ret.w += wd.w + m_hgap;
		ret.h = std::max(wd.h, ret.h);
	}
	if ( (w = getChildWidgetAt(parent, Center)) ) {
		const UDimension & wd = w->getMinimumSize();
		ret.w += wd.w;
		ret.h = std::max(wd.h, ret.h);
	}
	if ( (w = getChildWidgetAt(parent, North)) ) {
		const UDimension & wd = w->getMinimumSize();
		ret.w = std::max(wd.w, ret.w);
		ret.h += wd.h + m_vgap;
	}
	if ( (w = getChildWidgetAt(parent, South)) ) {
		const UDimension & wd = w->getMinimumSize();
		ret.w = std::max(wd.w, ret.w);
		ret.h += wd.h + m_vgap;
	}

	const UInsets & in = parent->getInsets();

	ret.w += in.left + in.right;
	ret.h += in.top + in.bottom;

	return ret;
}

UDimension
UBorderLayout::getPreferredLayoutSize(const UWidget * parent) const {
	UDimension ret;

	UWidget * w = NULL;

	// compute first width
	// because NORTH is upper EAST/WEST
	// and SOUTH is below EAST/WEST
	// --------
	// |  N   |
	// --------
	// |E|C |W|
	// --------
	// |  S   |
	// --------

	if ( (w = getChildWidgetAt(parent, East)) ) {
		const UDimension & wd = w->getPreferredSize();
		ret.w = wd.w + m_hgap;
		ret.h = wd.h;
	}
	if ( (w = getChildWidgetAt(parent, West)) ) {
		const UDimension & wd = w->getPreferredSize();
		ret.w += wd.w + m_hgap;
		ret.h = std::max(wd.h, ret.h);
	}
	if ( (w = getChildWidgetAt(parent, Center)) ) {
		const UDimension & wd = w->getPreferredSize();
		ret.w += wd.w;
		ret.h = std::max(wd.h, ret.h);
	}
	if ( (w = getChildWidgetAt(parent, North)) ) {
		const UDimension & wd = w->getPreferredSize();
		ret.w = std::max(wd.w, ret.w);
		ret.h += wd.h + m_vgap;
	}
	if ( (w = getChildWidgetAt(parent, South)) ) {
		const UDimension & wd = w->getPreferredSize();
		ret.w = std::max(wd.w, ret.w);
		ret.h += wd.h + m_vgap;
	}

	const UInsets & in = parent->getInsets();

	ret.w += in.left + in.right;
	ret.h += in.top + in.bottom;

	return ret;
}

void
UBorderLayout::layoutContainer(const UWidget * parent) {
	URectangle bounds = parent->getInnerBounds();

	int top = bounds.y;
	int left = bounds.x;
	int width = bounds.w;
	int height = bounds.h;

	UWidget * w = NULL;

	if ( (w = getChildWidgetAt(parent, North)) ) {
		UDimension wd = w->getPreferredSize();
		fixSize(&wd, w, width);
		w->setBounds(left, top, width, wd.h);
		top += wd.h + m_vgap;
		height -= wd.h + m_vgap;
	}
	if ( (w = getChildWidgetAt(parent, South)) ) {
		UDimension wd = w->getPreferredSize();
		fixSize(&wd, w, width);
		w->setBounds(left, top + height - wd.h, width, wd.h);
		height -= wd.h + m_vgap;
	}
	if ( (w = getChildWidgetAt(parent, West)) ) {
		UDimension wd = w->getPreferredSize();
		fixSize(&wd, w, width);
		w->setBounds(left, top, wd.w, height);
		left += wd.w + m_hgap;
		width -= wd.w + m_hgap;
	}
	if ( (w = getChildWidgetAt(parent, East)) ) {
		UDimension wd = w->getPreferredSize();
		fixSize(&wd, w, width);
		w->setBounds(left + width - wd.w, top, wd.w, height - top);
		width -= wd.w + m_hgap;
	}
	if ( (w = getChildWidgetAt(parent, Center)) ) {
		w->setBounds(left, top, width, height);
	}
}

UWidget *
UBorderLayout::getChildWidgetAt(const UWidget * parent, const UString * position) const {
	UWidget * ret = NULL;
	// special case for centered Widget
	UWidget * center = NULL;
	bool wantCenter = position->equals(Center);

	int nWidgets = parent->getWidgetCount();

	for (int i = 0;i < nWidgets;i++) {
		UWidget * w = parent->getWidget(i);

		UString * pos = dynamic_cast<UString *>(w->get("layout"));
		if (position->equals(pos)) {
			ret = w;
			break;
		} else if (!center && wantCenter) {
			if (!pos || !pos->equals(East) && !pos->equals(West) &&
					!pos->equals(South) && !pos->equals(North)) {
				// make the first unknown widget as CENTER widget
				center = w;
			}
		}
	}
	if (! ret) {
		ret = center;
	}

	return ret;
}
