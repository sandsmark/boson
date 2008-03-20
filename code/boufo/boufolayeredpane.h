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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

// note the copyright above: this is LGPL!
#ifndef BOUFOLAYEREDPANE_H
#define BOUFOLAYEREDPANE_H

#include "boufowidget.h"

// A layered pane is comparable to a widget stack, but there can be multiple
// widgets visible at any time. The bottom widgets are drawn first, the top
// widgets are drawn later, so that on the screen they are actually "on top" of
// the other widgets.
class BoUfoLayeredPane : public BoUfoWidget
{
	Q_OBJECT
public:
	BoUfoLayeredPane();
	BoUfoLayeredPane(ufo::ULayeredPane*, bool provideLayout = true);
	~BoUfoLayeredPane();

	void addLayer(BoUfoWidget* w, int layer = 0, int position = -1);
	void setLayer(BoUfoWidget* w, int layer, int position = -1);
};


#endif
