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

class QString;
class BosonProfilingDialog;

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
	// also update operator>>() and operator<<() in the .cpp file
	long int mClear;
	long int mCells;
	long int mUnits;
	long int mText;
	long int mFunction;

};

// note that there are several workarounds in this class to reduce the number of
// #includes as far as possible. i want to be able to place this header to about
// every other class without increasing compile-time.
// all QValueList,QMap,... #includes are in the .cpp file
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonProfiling
{
public:
	enum ProfilingEvent {
		// note that all entries here need to be in order, so that we
		// can iterate them easily in a Profiling dialog.
		ProfilingStart = 0, // must remain the first entry!
		LoadGameData1,
		LoadGameData2,
		LoadGameData3,
		LoadModel,
		LoadModelTextures,
		LoadModelDisplayLists,
		LoadModelDummy,

		ProfilingEnd // must remain the last entry!
	};
	BosonProfiling();
	~BosonProfiling();

	static void initProfiling();
	static BosonProfiling* bosonProfiling() { return mProfiling; }

	void start(ProfilingEvent event);
	void stop(ProfilingEvent event);

	void loadUnit();
	void loadUnitDone(unsigned long int typeId);

	void render(bool start); // always call this first, before any other render*()
	void renderClear(bool start);
	void renderCells(bool start);
	void renderUnits(bool start);
	void renderText(bool start);
	void debugRender();

	bool saveToFile(const QString& fileName);
	bool loadFromFile(const QString& fileName);

private:
	class BosonProfilingPrivate;
	BosonProfilingPrivate* d;
	friend class BosonProfilingDialog;

	static BosonProfiling* mProfiling;
};

#endif
