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
	return layoutContainerInternal(parent, false, parent->getWidth());
}

UDimension
UFlowLayout::getPreferredLayoutSize(const UWidget * parent) const {
	return layoutContainerInternal(parent, false, parent->getWidth());
}

void
UFlowLayout::layoutContainer(const UWidget * parent) {
	layoutContainerInternal(parent, parent, parent->getWidth());
}

UDimension
UFlowLayout::layoutContainerInternal(const UWidget * parent, bool doLayout, int maxwidth) const {
	UDimension ret;
	const UInsets & insets = parent->getInsets();
	// the max width of the Container
	maxwidth = maxwidth - (insets.left + insets.right);

	// first, all widgets are laid out top-left.
	// if another align is desired, relayout a complete row to the given
	// layout
	bool realign = ((m_horizontalAlignment != AlignLeft) |
	               (m_verticalAlignment != AlignTop)) &&
	               doLayout;

	// the max height of a row
	int rowHeight = 0;

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
			if (!doLayout) {
				// make sure that no w->set*() method is called
				w = 0;
			}
			// size is always the preferred size
			if (doLayout) {
				w->setSize(d.w, d.h);
			}

			if ((x == left) || ((x + d.w + m_hgap) <= maxwidth)) {
				if (x > left) {
					// add the horizontal gap not before
					// the first widget of a row
					x += m_hgap;
				}

				if (doLayout) {
					w->setLocation(x, y);
				}

				// compute x value of the next widget
				x += d.w;
				// compute height of the row
				rowHeight = std::max(rowHeight, d.h);
			} else {
				if (realign) {
					moveWidgets(parent,
						left, y, maxwidth - x, rowHeight,
						rowStart, i
					);
				}
				ret.w = std::max(ret.w, x);
				// reset x value to left most
				x = left;
				// set y value to y of the next row
				y += m_vgap + rowHeight;

				if (doLayout) {
					w->setLocation(x, y);
				}

				x += d.w;
				// set rowHeight to new row height (height of current widget)
				rowHeight = d.h;

				rowStart = i;
			}
		}
	}
	if (realign) {
		moveWidgets(parent,
			left, y, maxwidth - x, rowHeight,
			rowStart, parent->getWidgetCount()
		);
	}
	ret.w = std::max(ret.w, x);
	ret.h = y + rowHeight;
	return ret;
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
