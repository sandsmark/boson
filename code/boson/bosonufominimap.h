/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann (b_mann@gmx.de)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef BOSONUFOMINIMAP_H
#define BOSONUFOMINIMAP_H

#include "boufo/boufo.h"

class BosonGLMiniMap;

namespace ufo {
	class UMouseEvent;
};

class BosonUfoMiniMapPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonUfoMiniMap : public BoUfoWidget
{
	Q_OBJECT
public:
	BosonUfoMiniMap();
	virtual ~BosonUfoMiniMap();

	void setMiniMap(BosonGLMiniMap*);

protected slots:
	void slotMouseEvent(QMouseEvent*);

protected:
	QPoint widgetToCell(const QPoint& pos);

private:
	BosonUfoMiniMapPrivate* d;
};


#endif

