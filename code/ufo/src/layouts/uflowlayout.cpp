/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/layouts/uflowlayout.cpp
    begin             : Tue May 29 2001
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

#include "ufo/layouts/uflowlayout.hpp"

#include "ufo/util/udimension.hpp"
#include "ufo/widgets/uwidget.hpp"

#include <algorithm>

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UFlowLayout, ULayoutManager)

UFlowLayout::UFlowLayout()
	: m_hgap(4)
	, m_vgap(4)
	, m_horizontalAlignment(AlignLeft)
	, m_verticalAlignment(AlignCenter)
{
}

UFlowLayout::UFlowLayout(int hgap, int vgap)
	: m_hgap(hgap)
	, m_vgap(vgap)
	, m_horizontalAlignment(AlignLeft)
	, m_verticalAlignment(AlignCenter)
{
}

UFlowLayout::UFlowLayout(int hgap, int vgap, Alignment hAlign, Alignment vAlign)
	: m_hgap(hgap)
	, m_vgap(vgap)
	, m_horizontalAlignment(hAlign)
	, m_verticalAlignment(vAlign)
{
}

UFlowLayout::~UFlowLayout() {}

UDimension
UFlowLayout::getMinimumLayoutSize(const UWidget * parent) const {
	UDimension ret;
	const UInsets & insets = parent->getInsets();
	int maxwidth = parent->getWidth() - (insets.left + insets.right);

	int width = 0;
	int height = 0;

	for (unsigned int i = 0; i < parent->getWidgetCount(); i++ ) {
		UWidget * w = parent->getWidget(i);

		if (w->isVisible()) {
			const UDimension & size = w->getMinimumSize();
			if ( (height == 0) || (( width + size.w + m_hgap ) <= maxwidth )) {
				width += size.w + m_hgap;

				height = std::max(size.h, height);
			} else {
				ret.h += height + m_vgap;
				ret.w = std::max(ret.w, width);

				height = 0;
			}
		}
	}
	if (ret.w > maxwidth) {
		ret.w = maxwidth;
	}

	ret.h += height;

	ret.w += insets.left + insets.right;
	ret.h += insets.top + insets.bottom;

	return ret;
}

UDimension
UFlowLayout::getPreferredLayoutSize(const UWidget * parent) const {
	UDimension ret;
	const UInsets & insets = parent->getInsets();
	
/*
int maxwidth = parent->getWidth() - (insets->left + insets->right);
	
	// FIXME !
	// redesign the whole maxwidth thingie
	if (maxwidth == 0) { maxwidth = 65536; }

	int width = 0;
	int height = 0;

	for (unsigned int i = 0; i < parent->getWidgetCount(); i++ ) {
		UWidget * w = parent->getWidget(i);

		if (w->isVisible()) {
			const UDimension * size = w->getPreferredSize();
			if ( (height == 0) || (( width + size->w + m_hgap ) <= maxwidth )) {
				width += size->w + m_hgap;

				height = std::max( size->h, height);
			} else {
				ret->h += height + m_vgap;
				ret->w = maxwidth;//std::max(ret->w, width);

				height = 0;
				width = 0;
			}
		}
	}
	ret->h += height;
	ret->w += width;

	if (ret->w > maxwidth) {
		ret->w = maxwidth;
	}

*/
	for (unsigned int i = 0; i < parent->getWidgetCount(); i++ ) {
		UWidget * w = parent->getWidget(i);

		if (w->isVisible()) {
			const UDimension & size = w->getPreferredSize();
			ret.w += size.w + m_hgap;
			ret.h = std::max(size.h, ret.h);
		}
	}
	if (parent->getWidgetCount() > 1) {
		ret.w -= m_hgap;
	}

	ret.w += insets.left + insets.right;
	ret.h += insets.top + insets.bottom;

	return ret;
}

void
UFlowLayout::layoutContainer(const UWidget * parent) {
	const UInsets & insets = parent->getInsets();
	// the max width of the Container
	int maxwidth = parent->getWidth() - (insets.left + insets.right);

	// first, all widgets are laid out top-left.
	// if another align is desired, relayout a complete row to the given
	// layout
	bool realign = (m_horizontalAlignment != AlignLeft) |
	               (m_verticalAlignment != AlignTop);

	// the max height of a row
	int height = 0;

	// the left most x value
	int left = insets.left;

	// the current position for a widget
	int x = left;
	int y = insets.top;

	// the index at which the current row starts
	int rowStart = 0;

	for (unsigned int i = 0; i < parent->getWidgetCount(); i++) {
		UWidget * w = parent->getWidget(i);

		if (w->isVisible()) {
			const UDimension & d = w->getPreferredSize();
			// size is always the preferred size
			w->setSize(d.w, d.h);

			if ((x == left) || ((x + d.w + m_hgap) <= maxwidth)) {
				if (x > left) {
					// add the horizontal gap not before
					// the first widget of a row
					x += m_hgap;
				}

				w->setLocation(x, y);

				// compute x value of the next widget
				x += d.w;
				// compute height of the row
				height = std::max(height, d.h);
			} else {
				if (realign) {
					moveWidgets(parent,
						left, y, maxwidth - x, height,
						rowStart, i
					);
				}
				// reset x value to left most
				x = left;
				// set y value to y of the next row
				y += m_vgap + height;
				
				
				w->setLocation(x, y);

				x += d.w;
				// set height to new row height (height of current widget)
				height = d.h;

				rowStart = i;
			}
		}
	}
	if (realign) {
		moveWidgets(parent,
			left, y, maxwidth - x, height,
			rowStart, parent->getWidgetCount()
		);
	}
}


void
UFlowLayout::setHorizontalAlignment(Alignment newHAlign) {
	m_horizontalAlignment = newHAlign;
}

Alignment
UFlowLayout::getHorizontalAlignment() {
	return m_horizontalAlignment;
}

void
UFlowLayout::setVerticalAlignment(Alignment newVAlign) {
	m_verticalAlignment = newVAlign;
}

Alignment
UFlowLayout::getVerticalAlignment() {
	return m_verticalAlignment;
}

void
UFlowLayout::moveWidgets(const UWidget * parent,
		int x, int y, int width, int height,
		int rowStart, int rowEnd) const {

	// move the left most possible position of a widget
	if (m_horizontalAlignment == AlignCenter) {
		x += width / 2;
	} else if (m_horizontalAlignment == AlignRight) {
		x += width;
	}

	for (int i = rowStart; i < rowEnd; i++) {
		UWidget * w = parent->getWidget(i);
		if (w->isVisible()) {
			if (m_verticalAlignment == AlignCenter) {
				int widget_height = w->getHeight();
				w->setLocation(x, y + ( height / 2 - widget_height / 2 ));
			} else if (m_verticalAlignment == AlignBottom) {
				int widget_height = w->getHeight();
				w->setLocation(x, y + ( height - widget_height ));
			} else { // AlignTop
				w->setLocation(x, y);
			}

			x += w->getWidth() + m_hgap;
		}
	}
}
