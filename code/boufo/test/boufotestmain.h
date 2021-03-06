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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef BOUFOTESTMAIN_H
#define BOUFOTESTMAIN_H

#include "bodebugdcopiface.h"

#include <qgl.h> // AB: _Q_GLWidget
#include <qmainwindow.h>
#include <qdom.h>
#include <qmap.h>

class BoUfoWidget;
class BoUfoManager;
class BoUfoFontInfo;


/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoUfoTest : public QGLWidget
{
	Q_OBJECT
public:
	BoUfoTest(QWidget* parent = 0, const char* name = 0);
	~BoUfoTest();

protected:
	virtual void initializeGL();
	virtual void paintGL();
	virtual void resizeGL(int, int);

	virtual bool eventFilter(QObject* o, QEvent* e);
	virtual void mouseMoveEvent(QMouseEvent*);
	virtual void mousePressEvent(QMouseEvent*);
	virtual void mouseReleaseEvent(QMouseEvent*);
	virtual void wheelEvent(QWheelEvent*);
	virtual void keyPressEvent(QKeyEvent* e);
	virtual void keyReleaseEvent(QKeyEvent* e);

	void initUfoActions();

protected slots:
	void mySlotUpdateGL();

	void slotChangeFont(const BoUfoFontInfo&);

private:
	BoUfoManager* mUfoManager;
	BoUfoWidget* mContentWidget;
};


#endif

