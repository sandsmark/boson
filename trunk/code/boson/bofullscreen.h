/*
    This file is part of the Boson game
    Copyright (C) 2004 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef BOFULLSCREEN_H
#define BOFULLSCREEN_H

class QStringList;

class BoFullScreenPrivate;
class BoFullScreen
{
public:
	~BoFullScreen();

	static void initStatic();
	void initModes();

	/**
	 * @return @ref availableModeList
	 **/
	static QStringList availableModes();

	/**
	 * @param index Index as provided by @ref availableModes
	 **/
	static bool enterMode(int index);

	/**
	 * Enter the mode that was set when boson was started.
	 **/
	static bool enterOriginalMode();

protected:
	QStringList availableModeList();
	bool enterModeInList(int index);

private:
	BoFullScreen();

private:
	BoFullScreenPrivate* d;
	static BoFullScreen* mBoFullScreen;
};

#endif

