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

void UBoGridLayout::layoutContainer(const ufo::UWidget* container)
{
 for (unsigned int i = 0 ; i < container->getWidgetCount() ; i++) {
	 ufo::UWidget* w = container->getWidget(i);
	if (w->isVisible()) {
		w->setBounds(0, 0, container->getWidth(), container->getHeight());
	}
 }
}

ufo::UDimension UBoGridLayout::getPreferredLayoutSize(const ufo::UWidget* container, const ufo::UDimension& maxSize) const
{
 return maxSize;
}



