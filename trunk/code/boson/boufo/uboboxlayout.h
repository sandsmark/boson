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

#ifndef UBOBOXLAYOUT_H
#define UBOBOXLAYOUT_H

#include <ufo/layouts/ulayoutmanager.hpp>
//#include "ulayoutmanager.hpp"

namespace ufo {

class UBoBoxLayout : public ULayoutManager {
public:
	UBoBoxLayout(bool horizontal);

	virtual UDimension getPreferredLayoutSize(const UWidget* parent) const;
	virtual UDimension getMinimumLayoutSize(const UWidget* parent) const;
	virtual void layoutContainer(const UWidget* parent);

	virtual int getLayoutHeightForWidth(const UWidget* parent, int w) const;

protected:
private:
	bool mHorizontal;
	int mHGap;
	int mVGap;
};

} // namespace ufo

#endif

