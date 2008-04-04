/*
    This file is part of the Boson game
    Copyright (C) 2006 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOCOMMANDFRAMETESTERMAIN_H
#define BOCOMMANDFRAMETESTERMAIN_H

#include <boufo/boufowidget.h>
#include "../bosonufoglwidget.h"

class Player;

class BoCommandFrameTesterMainPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoCommandFrameTesterMain : public BoUfoWidget
{
	Q_OBJECT
public:
	BoCommandFrameTesterMain();
	~BoCommandFrameTesterMain();

	bool init();
	bool start();

protected slots:
	void slotAdvance(unsigned int advanceCallsCount, bool advanceFlag);
	void slotAddIOs(Player* p, int* ioMask, bool* failure);

private:
	BoCommandFrameTesterMainPrivate* d;
};

class BoCommandFrameTesterGLWidget : public BosonUfoGLWidget
{
	Q_OBJECT
public:
	BoCommandFrameTesterGLWidget();
	~BoCommandFrameTesterGLWidget();

	bool init();

protected:
	virtual void initializeGL();
	virtual void paintGL();

private:
	BoCommandFrameTesterMain* mTester;
};

#endif

