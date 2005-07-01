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
#ifndef BOFULLSCREEN_H
#define BOFULLSCREEN_H

class QStringList;
class QWidget;

class BoFullScreenPrivate;
class BoFullScreen
{
public:
	~BoFullScreen();

	static void initStatic();
	void initModes();

	/**
	 * Leave fullscreen mode. This will call @ref enterOriginalMode and
	 * will additionally make the application a windowed application again.
	 **/
	static void leaveFullScreen();

	/**
	 * @return @ref availableModeList
	 **/
	static QStringList availableModes();

	/**
	 * See @ref enterModeInList
	 * @param index Index as provided by @ref availableModes
	 **/
	static bool enterMode(int index);

	/**
	 * Enter the mode that was set when boson was started.
	 * @return TRUE
	 **/
	static bool enterOriginalMode();

	/**
	 * Resize the @p widget to @p width and @p height. This method assumes
	 * that @p width and @p height exactly describe the size of the current
	 * resolution. The resized window will not have a menubar or a border,
	 * i.e. it is treated as a fullscreen window (AB: probably this will
	 * cause trouble with other window managers!)
	 *
	 * This method does <em>not</em> change the resolution! See also @ref
	 * enterMode.
	 *
	 * Usually you don't need to call this, as it is called internally.
	 **/
	static void resizeToFullScreen(QWidget* widget, int width, int height);

protected:
	/**
	 * @return A list of modes that we can switch to. If we cannot switch
	 * the resolution an empty list is returned (not even the
	 * default/current mode is returned!)
	 **/
	QStringList availableModeList();

	/**
	 * Change the resolution mode to @p index. @p index must be the index
	 * of a mode in @ref availableModeList or -1 (meaning to go into usual
	 * @ref QWidget::showFullScreen emulation mode)
	 * @return TRUE on success, FALSE on failure.
	 **/
	bool enterModeInList(int index);

private:
	BoFullScreen();

private:
	BoFullScreenPrivate* d;
	static BoFullScreen* mBoFullScreen;
};

#endif

