/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef BOSONPROFILING_H
#define BOSONPROFILING_H

#define boProfiling BosonProfiling::bosonProfiling()

/**
 * Information about the rendering times. Helper class for @ref BosonProfiling.
 *
 * We avoid functions here in order to make it as fast as possible
 **/
class RenderGLTimes
{
public:
	RenderGLTimes()
	{
		mClear = 0;
		mCells = 0;
		mUnits = 0;
		mText = 0;
		mFunction = 0;
	}
	RenderGLTimes(const RenderGLTimes& c)
	{
		*this = c;
	}
	RenderGLTimes& operator=(const RenderGLTimes& c)
	{
		mClear = c.mClear;
		mCells = c.mCells;
		mUnits = c.mUnits;
		mText = c.mText;
		mFunction = c.mFunction;
		return *this;
	}

	// remember to update operator= and default c'tor if you change
	// something!
	long int mClear;
	long int mCells;
	long int mUnits;
	long int mText;
	long int mFunction;

};

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonProfiling
{
public:
	BosonProfiling();
	~BosonProfiling();

	static void initProfiling();
	static BosonProfiling* bosonProfiling() { return mProfiling; }

	void loadUnit();
	void loadUnitDone(int typeId);

	void render(bool start); // always call this first, before any other render*()
	void renderClear(bool start);
	void renderCells(bool start);
	void renderUnits(bool start);
	void renderText(bool start);
	int renderCount() const;
	RenderGLTimes renderTimes(unsigned int i) const;
	void debugRender();

private:
	class BosonProfilingPrivate;
	BosonProfilingPrivate* d;

	static BosonProfiling* mProfiling;
};

#endif
