/*
    This file is part of the Boson game
    Copyright (C) 2003-2005 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOFASTGROUNDRENDERER_H
#define BOFASTGROUNDRENDERER_H

#include "bogroundrendererbase.h"

class PlayerIO;
class QString;

class BosonMap;
class BoMatrix;

class QRect;

class BoFastGroundRenderer : public BoGroundRendererBase
{
	Q_OBJECT
public:
	BoFastGroundRenderer();
	virtual ~BoFastGroundRenderer();

	virtual bool initGroundRenderer();

	virtual void renderVisibleCells(int* cells, unsigned int cellsCount, const BosonMap* map, RenderFlags flags);

protected:
	void updateMapCache(const BosonMap*);

private:
	const BosonMap* mCurrentMap;
	unsigned char* mCellTextures;
};

#endif

