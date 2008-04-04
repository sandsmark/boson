/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOCURSORMAIN_H
#define BOCURSORMAIN_H

#include "bosonglwidget.h"

class BosonCursor;
class BoDebugDCOPIface;

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class CursorPreview : public BosonGLWidget
{
	Q_OBJECT
public:
	CursorPreview(QWidget*);
	~CursorPreview();

	virtual void initializeGL();
	virtual void paintGL();
	virtual void resizeGL(int, int);

	BosonCursor* cursor() { return mCursor; }

public slots:
	void slotChangeCursor(int, const QString& theme);
	void slotChangeCursorType(int);

protected:
	virtual void mouseMoveEvent(QMouseEvent*);

private:
	BosonCursor* mCursor;
	QTimer* mUpdateTimer;
	BoDebugDCOPIface* mIface;
};

#endif
