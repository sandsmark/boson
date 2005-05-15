/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "ubogridlayout.h"

#include <ufo/ufo.hpp>

using namespace ufo;


UFO_IMPLEMENT_DYNAMIC_CLASS(UBoGridLayout, ufo::ULayoutManager)

static std::vector<URectangle>
getBounds(const UWidget * container,
		const URectangle & bounds,
		UDimension * fullSize, // returns the desired size of the container
		bool useAvailableSize = false);

std::vector<URectangle>
getBounds(const UWidget * container, const URectangle & bounds, UDimension * fullSize, bool useAvailableSize) {
	std::vector<URectangle> ret(container->getWidgetCount());
	UDimension dim(bounds.w, bounds.h);

	if (container->getWidgetCount() < 1) {
		return ret;
	}

	int columnCount = -1;
	int rowCount = -1;
	{
		ufo::UObject * c = container->get("gridLayoutColumns");
		if (c) {
			columnCount = UInteger::toInt(c->toString());
		}
		ufo::UObject * r = container->get("gridLayoutRows");
		if (r) {
			rowCount = UInteger::toInt(r->toString());
		}
	}

	if (columnCount == 0 || rowCount == 0) {
		// nonsense parameters
		for (unsigned int i = 0; i < container->getWidgetCount(); i++) {
			ret[i] = URectangle(0, 0, 0, 0);
			return ret;
		}
	}

	// if columnCount != -1, we use columnCount to build up the grid.
	// otherwise we use rowCount. 
	// Note that both values != -1 makes little sense (e.g. is invalid when
	// widgetCount > columnCount * rowCount)
	if (columnCount > 0) {
		rowCount = -1;
	}

	columnCount = std::min(columnCount, (int)container->getWidgetCount());
	rowCount = std::min(rowCount, (int)container->getWidgetCount());

	bool sortByColumn = true;
	if (columnCount > 0) {
		rowCount = container->getWidgetCount() / columnCount;
		if (container->getWidgetCount() % columnCount > 0) {
			rowCount++;
		}
		sortByColumn = true;
	} else if (rowCount > 0) {
		columnCount = container->getWidgetCount() / rowCount;
		if (container->getWidgetCount() % rowCount > 0) {
			columnCount++;
		}
		sortByColumn = false;
	} else {
		columnCount = container->getWidgetCount();
		rowCount = 1;
		sortByColumn = true;
	}


	// AB: (cells[column])[row]
	std::vector< std::vector<const UWidget*> > cells(columnCount);
	for (int c = 0; c < columnCount; c++) {
		cells[c].resize(rowCount, 0);
	}
	for (unsigned int i = 0; i < container->getWidgetCount(); i++) {
		const UWidget * w = container->getWidget(i);
		int row = 0;
		int column = 0;
		if (sortByColumn) {
			column = i % columnCount;
			row = i / columnCount;
		} else {
			row = i % rowCount;
			column = i / rowCount;
		}
		(cells[column])[row] = w;
	}


	std::map<const UWidget*, unsigned int> widget2Index;
	for (unsigned int i = 0; i < container->getWidgetCount(); i++) {
		const UWidget * w = container->getWidget(i);
		widget2Index[w] = i;
	}
	int columnX = 0;
	UDimension maxDim = dim;
	for (int column = 0; column < columnCount; column++) {
		UDimension maxColumnDim = maxDim;
		int columnWidth = 0;
		int y = 0;
		for (int row = 0; row < rowCount; row++) {
			const UWidget * w = (cells[column])[row];
			if (!w) {
				continue;
			}
			if (widget2Index.find(w) == widget2Index.end()) {
				std::cerr << "oops - internal error: widget2Index does not know " << (void*)w << std::endl;
				continue;
			}
			unsigned int i = widget2Index[w];
			ret[i].x = columnX;
			ret[i].y = y;
			if (!w->isVisible()) {
				ret[i].w = 0;
				ret[i].h = 0;
				continue;
			}
			UDimension d = w->getPreferredSize(maxColumnDim);
			d.clamp(maxColumnDim);

			ret[i].w = d.w;
			ret[i].h = d.h;

			maxColumnDim.h -= d.h;
			y += d.h;

			if (d.w > columnWidth) {
				columnWidth = d.w;
			}
		}

		// use columnWidth for all cells in the column
		for (int row = 0; row < rowCount; row++) {
			const UWidget * w = (cells[column])[row];
			if (!w) {
				continue;
			}
			if (widget2Index.find(w) == widget2Index.end()) {
				std::cerr << "oops - internal error: widget2Index does not know " << (void*)w << std::endl;
				continue;
			}
			unsigned int i = widget2Index[w];
			if (!w->isVisible()) {
				continue;
			}
			ret[i].w = columnWidth;
		}

		if (columnX + columnWidth > fullSize->w) {
			fullSize->w = columnX + columnWidth;
		}

		maxDim.w -= columnWidth;
		columnX += columnWidth;
	}

	// AB: the columns are already correct, but the rows have different
	// heights in every column, so we need to fix them now.
	int maxHeight = dim.h;
	int rowY = 0;
	for (int row = 0; row < rowCount; row++) {
		int rowHeight = 0;
		for (int column = 0; column < columnCount; column++) {
			const UWidget * w = (cells[column])[row];
			if (!w) {
				continue;
			}
			if (widget2Index.find(w) == widget2Index.end()) {
				std::cerr << "oops - internal error: widget2Index does not know " << (void*)w << std::endl;
				continue;
			}
			unsigned int i = widget2Index[w];
			if (!w->isVisible()) {
				continue;
			}
			if (ret[i].h > rowHeight) {
				// AB: h < dim.h is already ensured, as all
				// widgets are clamped to dim
				rowHeight = ret[i].h;
			}
		}

		// we want more space than we have. sorry but that we cannot
		// allow
		if (rowHeight > maxHeight) {
			rowHeight = maxHeight;
		}

		maxHeight -= rowHeight;
		if (rowY + rowHeight > fullSize->h) {
			fullSize->h = rowY + rowHeight;
		}

		for (int column = 0; column < columnCount; column++) {
			const UWidget * w = (cells[column])[row];
			if (!w) {
				continue;
			}
			if (widget2Index.find(w) == widget2Index.end()) {
				std::cerr << "oops - internal error: widget2Index does not know " << (void*)w << std::endl;
				continue;
			}
			unsigned int i = widget2Index[w];
			ret[i].y = rowY;
			if (!w->isVisible()) {
				continue;
			}
			ret[i].h = rowHeight;


		}
		rowY += rowHeight;
	}




	// AB: sometime we do not want to use all available space (e.g.
	// getPreferredLayoutSize() must not use all space)
	if (!useAvailableSize) {
		return ret;
	}


	return ret;
}

void UBoGridLayout::layoutContainer(const ufo::UWidget* container)
{
 URectangle bounds = container->getInnerBounds();
 UDimension ret(0, 0); // AB: we don't really need this, but getBounds() requires that it is non-NULL atm
 std::vector<URectangle> childBounds = getBounds(container, bounds, &ret, false);
 if (ret.w > bounds.w) {
	std::cerr << "Ooops: ret.w > bounds.w!!" << std::endl;
 }
 if (ret.h > bounds.h) {
	std::cerr << "Ooops: ret.h > bounds.h!!" << std::endl;
 }


 for (unsigned int i = 0 ; i < container->getWidgetCount() ; i++) {
	UWidget* w = container->getWidget(i);
	if (w->isVisible()) {
		const URectangle & b = childBounds[i];
		w->setBounds(b.x, b.y, b.w, b.h);
	}
 }
}

ufo::UDimension UBoGridLayout::getPreferredLayoutSize(const ufo::UWidget* container, const ufo::UDimension& maxSize) const
{
 URectangle bounds = container->getInnerBounds();
 // AB: x, y don't matter in this method
 bounds.w = maxSize.w;
 bounds.h = maxSize.h;
 UDimension ret(0, 0);
 std::vector<URectangle> childBounds = getBounds(container, bounds, &ret, false);

 ret += container->getInsets();

 return ret;
}



