/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOVERYFASTGROUNDRENDERER_H
#define BOVERYFASTGROUNDRENDERER_H

#include "bogroundrendererbase.h"
#include <bogl.h>

class BosonMap;
class BosonGroundTheme;
class BosonGroundThemeData;

/**
 * @short Ground renderer that is optimized for software rendering
 *
 * This groundrenderer should not be used in a normal environment, it is
 * supposed to be used in debugging primarily. All ground textures have been
 * replaced by normal colors and the fog of war texture has been disabled.
 * Additionally the ground LOD is more aggressive.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoVeryFastGroundRenderer : public BoGroundRendererBase
{
	Q_OBJECT
public:
	BoVeryFastGroundRenderer();
	virtual ~BoVeryFastGroundRenderer();

	virtual bool initGroundRenderer();

protected:
	void updateMapCache(const BosonMap*);
	void updateGroundThemeCache(const BosonGroundThemeData*);

	virtual void renderVisibleCells(int* cells, unsigned int cellsCount, const BosonMap* map, RenderFlags flags);
	virtual void renderVisibleCellsStart(const BosonMap* map);
	virtual void renderVisibleCellsStop(const BosonMap* map);

private:
	const BosonMap* mCurrentMap;
	unsigned char* mCellTextures;
	const BosonGroundThemeData* mCurrentThemeData;
	GLubyte* mThemeColors;
};

#endif

