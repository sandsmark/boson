/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann (b_mann@gmx.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

// note the copyright above: this is LGPL!

#include "uboboxlayout.h"

#include <ufo/ufo.hpp>

using namespace ufo;

#define DO_DEBUG 0

UBoBoxLayout::UBoBoxLayout(bool horizontal)
	: ULayoutManager()
{
 mHorizontal = horizontal;
 mHGap = 2;
 mVGap = 2;
}

UDimension UBoBoxLayout::getPreferredLayoutSize(const UWidget* parent) const
{
 UDimension ps;
 for (unsigned int i = 0; i < parent->getWidgetCount(); i++) {
	const UWidget* w = parent->getWidget(i);
	if (!w->isVisible()) {
		continue;
	}
	const UDimension& s = w->getPreferredSize();
	if (mHorizontal) {
		ps.w += s.w;
		ps.h = std::max(ps.h, s.h);
		if (i < parent->getWidgetCount() - 1) {
			ps.w += mHGap;
		}
	} else {
		ps.w = std::max(ps.w, s.w);
		ps.h += s.h;
		if (i < parent->getWidgetCount() - 1) {
			ps.h += mVGap;
		}
	}
 }

 // AB: from UBoxLayout
 const UInsets& insets = parent->getInsets();
 ps.w += insets.left + insets.right + mHGap;
 ps.h += insets.top + insets.bottom + 2 * mVGap;
 return ps;
}

UDimension UBoBoxLayout::getMinimumLayoutSize(const UWidget* parent) const
{
 UDimension ms;
 for (unsigned int i = 0; i < parent->getWidgetCount(); i++) {
	const UWidget* w = parent->getWidget(i);
	if (!w->isVisible()) {
		continue;
	}
	const UDimension& s = w->getMinimumSize();
	if (mHorizontal) {
		ms.w += s.w;
		ms.h = std::max(ms.h, s.h);
		if (i < parent->getWidgetCount() - 1) {
			ms.w += mHGap;
		}
	} else {
		ms.w = std::max(ms.w, s.w);
		ms.h += s.h;
		if (i < parent->getWidgetCount() - 1) {
			ms.h += mVGap;
		}
	}
 }

 // AB: from UBoxLayout
 const UInsets& insets = parent->getInsets();
 ms.w += insets.left + insets.right + mHGap;
 ms.h += insets.top + insets.bottom + 2 * mVGap;
 return ms;
}

void UBoBoxLayout::layoutContainer(const UWidget* parent)
{
 if (parent->getWidgetCount() == 0) {
	return;
 }
 URectangle rootBounds = parent->getBounds();
 URectangle bounds = parent->getInnerBounds();
 UDimension ps = getPreferredLayoutSize(parent);
#if DO_DEBUG
 printf("layoutContainer() on widget with rootBounds=(%d,%d,%d,%d)\n", rootBounds.x, rootBounds.y, rootBounds.w, rootBounds.h);
 printf("innerBounds=(%d,%d,%d,%d)\n", bounds.x, bounds.y, bounds.w, bounds.h);
#endif

 if (parent->getParent()) {
	// a URootPane has a ULayeredPane ("desktopePane"). that widget contains
	// the content pane.
	//
	// the content pane is the first widget that we actually use directly.
	if (dynamic_cast<const ULayeredPane*>(parent->getParent())) {
#if DO_DEBUG
		printf(" CONTENT PANE!!\n");
#endif
	}
 }

#if DO_DEBUG
 printf(" %d widgets\n", parent->getWidgetCount());
#endif

 bool useMin = false; // whether to use minimumSize instead of preferredSize
 int minW; // absolute minimum a widget will ever get (may even be less than the minimumSize a widget wants to have)
 if (mHorizontal) {
	minW = (bounds.w - mHGap * (parent->getWidgetCount() - 1)) / parent->getWidgetCount();
#if DO_DEBUG
	printf(" horizontal layout\n");
#endif
 } else {
	minW = bounds.w;
#if DO_DEBUG
	printf(" vertical layout\n");
#endif
 }
 if (ps.w > bounds.w) {
	useMin = true;
#if DO_DEBUG
	printf(" useMin (preferred width too wide: want %d)\n", ps.w);
#endif
 } else if (ps.h > bounds.h) {
	useMin = true;
#if DO_DEBUG
	printf(" useMin (preferred height too high: want %d)\n", ps.h);
#endif
 }

 int totalWidth = 0;
 int totalHeight = 0;
 std::vector<int> widths(parent->getWidgetCount());
 std::vector<int> heights(parent->getWidgetCount());
 for (unsigned int i = 0; i < parent->getWidgetCount(); i++) {
	widths[i] = 0;
	heights[i] = 0;
 }
 for (unsigned int i = 0; i < parent->getWidgetCount(); i++) {
	UWidget* widget = parent->getWidget(i);
	if (!widget->isVisible()) {
		continue;
	}
	UDimension s;
#if 0
	if (useMin) {
		s = widget->getMinimumSize();
	} else {
		s = widget->getPreferredSize();
	}
#else
	UDimension ms = widget->getMinimumSize();
	UDimension ps = widget->getPreferredSize();
#if DO_DEBUG
	printf(" minimum size for %d is (%d,%d)\n", i, ms.w, ms.h);
	printf(" preferred size for %d is (%d,%d)\n", i, ps.w, ps.h);
#endif
	if (useMin) {
		s = ms;
	} else {
		s = ps;
	}
#endif
#if DO_DEBUG
	printf(" size for %d is (%d,%d)\n", i, s.w, s.h);
#endif
	int w = s.w;
	if (w > bounds.w) {
		w = bounds.w;
	}
	int h = widget->getHeightForWidth(w);
	if (h == 0) {
		h = s.h;
	}
#if DO_DEBUG
	printf("height for width %d of widget %d: %d\n", w, i, h);
#endif
	if (mHorizontal) {
		totalWidth += w;
		totalHeight = std::max(totalHeight, h);
		widths[i] = w;
		heights[i] = std::max(bounds.h, h);
	} else {
		totalWidth = std::max(totalWidth, w);
		totalHeight += h;
		widths[i] = std::max(bounds.w, w);
		heights[i] = h;
	}
 }
 if (mHorizontal) {
	totalWidth += (mHGap * (parent->getWidgetCount() - 1));
	totalHeight = bounds.h;
 } else {
	totalWidth = bounds.w;
	totalHeight += (mVGap * (parent->getWidgetCount() - 1));
 }
#if DO_DEBUG
 printf("totalWidth=%d, bounds.w=%d\n", totalWidth, bounds.w);
#endif
 if (totalWidth > bounds.w) {
	// there are still widgets exceeding our bounds.
	// we force them to be width=minW
#if DO_DEBUG
	printf(" widgets are too wide - want %d. we force them to be at %d\n", totalWidth, minW);
	printf(" totalHeight=%d\n", totalHeight);
#endif
	totalHeight = 0;
	for (unsigned int i = 0; i < parent->getWidgetCount(); i++) {
		UWidget* widget = parent->getWidget(i);
		if (!widget->isVisible()) {
			continue;
		}
		int h = widget->getHeightForWidth(minW);
#if DO_DEBUG
		printf("height for width %d of widget %d: %d\n", minW, i, h);
#endif
		widths[i] = minW;
		heights[i] = h;
		totalHeight += h;
	}
	totalWidth = bounds.w;
	if (mHorizontal) {
		totalHeight = bounds.h;
	} else {
		totalHeight += (mVGap * (parent->getWidgetCount() - 1));
	}
#if DO_DEBUG
	printf(" totalHeight now=%d\n", totalHeight);
#endif
 }

 if (totalHeight > bounds.h) {
	int minH;
	if (mHorizontal) {
		minH = bounds.h;
	} else {
		minH = (bounds.h - mVGap * (parent->getWidgetCount() - 1)) / parent->getWidgetCount();
	}
#if DO_DEBUG
	printf(" widgets too high (%d). force them to be at %d each\n", totalHeight, minH);
#endif
	for (unsigned int i = 0; i < parent->getWidgetCount(); i++) {
		UWidget* widget = parent->getWidget(i);
		if (!widget->isVisible()) {
			continue;
		}
		heights[i] = minH;
	}
	totalHeight = bounds.h;
 }

 int x = bounds.x;
 int y = bounds.y;
 for (unsigned int i = 0; i < parent->getWidgetCount(); i++) {
	UWidget* w = parent->getWidget(i);
	if (!w->isVisible()) {
		continue;
	}
#if DO_DEBUG
	printf(" placing widget %d at ", i);
	printf("%d,%d,%d,%d", x, y, widths[i], heights[i]);
	if (dynamic_cast<UCompound*>(w)) {
		UCompound* c = dynamic_cast<UCompound*>(w);
		printf(" (is a UCompound - text=%s)", c->getText().c_str());
	} else if (dynamic_cast<UCheckBox*>(w)) {
		printf(" (is a UCheckBox)");
	} else if (dynamic_cast<ULineEdit*>(w)) {
		ULineEdit* l = dynamic_cast<ULineEdit*>(w);
		printf(" (is a ULineEdit - text=%s)", l->getText().c_str());
	}
	printf("\n");
#endif
	w->setBounds(x, y, widths[i], heights[i]);
	if (mHorizontal) {
		x += widths[i];
		x += mHGap;
	} else {
		y += heights[i];
		y += mVGap;
	}
 }
#if DO_DEBUG
printf("layoutContainer() on %d,%d,%d,%d done\n", bounds.x, bounds.y, bounds.w, bounds.h);
#endif
}

int UBoBoxLayout::getLayoutHeightForWidth(const UWidget* parent, int width) const
{
 int height = 0;
 bool used = false;
 for (unsigned int i = 0; i < parent->getWidgetCount(); i++) {
	const UWidget* w = parent->getWidget(i);
	if (!w->isVisible()) {
		continue;
	}
	int h = w->getHeightForWidth(width);
	if (h <= 0) {
		// AB: preferred size? minimum size? something else?
		const UDimension& s = w->getMinimumSize();
		h = s.h;
	} else {
		used = true;
	}
	if (mHorizontal) {
		height = std::max(height, h);
	} else {
		height += h;
		if (i < parent->getWidgetCount() - 1) {
			height += mVGap;
		}
	}
 }
 const UInsets& insets = parent->getInsets();
 height += insets.top + insets.bottom + 2 * mVGap;

 if (!used) {
	// height of all widgets in this layout are independant of the width
	height = 0;
 }
 return height;
}
