/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA *
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
	, m_horizontalAlignment(AlignNone)
	, m_verticalAlignment(AlignNone)
{
}

UFlowLayout::UFlowLayout(int hgap, int vgap)
	: m_hgap(hgap)
	, m_vgap(vgap)
	, m_horizontalAlignment(AlignNone)
	, m_verticalAlignment(AlignNone)
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
UFlowLayout::getPreferredLayoutSize(const UWidget * container,
		const UDimension & maxSize) const {
	UDimension ret;
	UInsets insets = container->getInsets();

	int maxWidth = maxSize.w;

	int rowWidth = 0;
	int rowHeight = 0;

	for (unsigned int i = 0; i < container->getWidgetCount(); ++i) {
		UWidget * w = container->getWidget(i);

		if (w->isVisible()) {
			// FIXME: Resize maxSize to respect already "consumed" space
			const UDimension & size = w->getPreferredSize(maxSize);
			if (/*(rowHeight == 0) || */((rowWidth + size.w + m_hgap) <= maxWidth)) {
				rowWidth += size.w + m_hgap;
				rowHeight = std::max(size.h, rowHeight);
			} else {
				// line swapping
				ret.h += rowHeight + m_vgap;
				ret.w = std::max(ret.w, rowWidth);

				// new row gets the size of the current widget
				rowHeight = size.h;
				rowWidth = size.w;
			}
		}
	}
	ret.h += rowHeight;
	ret.w = std::max(ret.w, rowWidth);
	if (ret.w > maxWidth) {
		ret.w = maxWidth;
	}

	ret.w += insets.getHorizontal();
	ret.h += insets.getVertical();
	return ret;
}

void
UFlowLayout::layoutContainer(const UWidget * container) {
	const UInsets & insets = container->getInsets();
	// the max width of the Container
	int maxwidth = container->getWidth() - (insets.left + insets.right);

	// determine alignment
	int halign = m_horizontalAlignment;
	int valign = m_verticalAlignment;
	if (halign == AlignNone) {
		halign = container->getHorizontalAlignment();
	}
	if (valign == AlignNone) {
		valign = container->getVerticalAlignment();
	}
	// first, all widgets are laid out top-left.
	// if another align is desired, relayout a complete row to the given
	// layout
	bool realign = (halign != AlignLeft) |
	               (valign != AlignTop);

	// the max height of a row
	int height = 0;

	// the left most x value
	int left = insets.left;

	// the current position for a widget
	int x = left;
	int y = insets.top;

	// the index at which the current row starts
	int rowStart = 0;

	for (unsigned int i = 0; i < container->getWidgetCount(); i++) {
		UWidget * w = container->getWidget(i);

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
					moveWidgets(container,
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
		moveWidgets(container,
			left, y, maxwidth - x, height,
			rowStart, container->getWidgetCount()
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
UFlowLayout::moveWidgets(const UWidget * container,
		int x, int y, int width, int height,
		int rowStart, int rowEnd) const {
	// determine alignment rules
	int halign = m_horizontalAlignment;
	int valign = m_verticalAlignment;
	if (halign == AlignNone) {
		halign = container->getHorizontalAlignment();
	}
	if (valign == AlignNone) {
		valign = container->getVerticalAlignment();
	}
	// move the left most possible position of a widget
	if (halign == AlignCenter) {
		x += width / 2;
	} else if (halign == AlignRight) {
		x += width;
	}

	for (int i = rowStart; i < rowEnd; i++) {
		UWidget * w = container->getWidget(i);
		if (w->isVisible()) {
			if (valign == AlignCenter) {
				int widget_height = w->getHeight();
				w->setLocation(x, y + ( height / 2 - widget_height / 2 ));
			} else if (valign == AlignBottom) {
				int widget_height = w->getHeight();
				w->setLocation(x, y + ( height - widget_height ));
			} else { // AlignTop
				w->setLocation(x, y);
			}

			x += w->getWidth() + m_hgap;
		}
	}
}
