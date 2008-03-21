/*
    This file is part of the Boson game
    Copyright (C) 1999-2000 Thomas Capricelli (capricel@email.enst.fr)
    Copyright (C) 2001-2002 Andreas Beckermann (b_mann@gmx.de)

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef SELECTBOX_H
#define SELECTBOX_H

class BosonItem;
class BosonCanvas;

#include <bogl.h>

class BoTexture;

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class SelectBoxData
{
public:
	SelectBoxData();
	~SelectBoxData();

	/**
	 * @param factor Should be health/maxHealth of the unit
	 **/
	GLuint list(float factor);

protected:
	void loadBoxes();
	static void drawCube();
	static void drawHealthBar(int frame, int displayListCount);

private:
	BoTexture* mTexture;
	GLuint mDisplayListBase;
	GLsizei mDisplayListCount;
};

#endif

