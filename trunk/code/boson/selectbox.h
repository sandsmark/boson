/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#ifndef SELECTBOX_H
#define SELECTBOX_H

class BosonItem;
class BosonCanvas;

#include <GL/gl.h>

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

class SelectBox
{
public :
	SelectBox(BosonItem*, BosonCanvas* canvas, bool groupLeader = false);
	~SelectBox();

	/**
	 * Set a factor between 0 and 1 that describes the amount of health of
	 * the item this box belongs to.
	 *
	 * The health bar of the selectbox displays the health according to this
	 * factor.
	 **/
	void setFactor(float factor)
	{
		mFactor = factor;
	}

	float factor() const
	{
		return mFactor;
	}

private:
	float mFactor;
};

#endif

