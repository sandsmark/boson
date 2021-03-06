/*
    This file is part of the Boson game
    Copyright (C) 2008 Andreas Beckermann (b_mann@gmx.de)

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef CANVASTEST_H
#define CANVASTEST_H

#include <qobject.h>

class BosonPlayField;
class BosonMap;
class BosonCanvas;
class BoEventManager;
class BosonPlayerListManager;
class BosonItem;

class CanvasContainer;

class CanvasTest : public QObject
{
	Q_OBJECT
public:
	CanvasTest(QObject* parent = 0);
	~CanvasTest();

	bool test();

protected:
	bool initTest();
	void cleanupTest();

	bool testCreateNewCanvas();
	bool testSaveLoadCanvas();
	bool testMoveUnits();

	bool checkIfCanvasIsValid(BosonCanvas* canvas);
	bool checkIfCanvasAreEqual(BosonCanvas* canvas1, BosonCanvas* canvas2);

private:
	CanvasContainer* mCanvasContainer;
};

#endif

