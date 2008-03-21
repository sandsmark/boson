/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Andreas Beckermann (b_mann@gmx.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// note the copyright above: this is LGPL!
#ifndef UBOSONSTYLE_H
#define UBOSONSTYLE_H

#include <ufo/ui/ubasicstyle.hpp>

namespace ufo {

class UBosonStyle : public UBasicStyle {
public:
	UBosonStyle();
	~UBosonStyle();

	virtual void paintComponent(UGraphics * g,
		ComponentElement elem,
		const URectangle & rect,
		const UStyleHints * hints,
		const UWidgetModel * model,
		UWidget * w = NULL);

	void paintBoUfoLabel(
		UGraphics * g,
		const UStyleHints * hints,
		const std::string & text,
		UIcon * icon,
		const URectangle & rect,
		uint32_t widgetState,
		int acceleratorIndex = -1);


};

}; // namespace ufo

#endif
