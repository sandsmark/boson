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
#ifndef __BOSONMUSIC_H__
#define __BOSONMUSIC_H__

//#include <qobject.h>

class QCanvas;
class QPoint;
class QWidget;
class QCursor;

/**
 **/
class BosonCursor
{
//	Q_OBJECT
public:
	enum CursorMode {
		Attack = 0,
		Move = 1,
		Default = 2,
		
		Hide // this MUST be the last entry!
	};
	BosonCursor();
	~BosonCursor();

	CursorMode cursorMode() const;

	/**
	 * Change the current cursor. You probably want to call setCursor(this)
	 * from your widget, too in order to get the @ref QCanvas cursor (if it
	 * is used)
	 **/
	void setCursor(CursorMode mode);
	
	/**
	 * Change the @ref QCursor cursor of the specified widget. This function
	 * uses @ref QWidget::setCursor if the feature is compiled in. Note that
	 * boson uses currently a #define to decide whether to use it or not.
	 **/
	void setCursor(QWidget* w);

	/**
	 * @return The current @ref QCursor according to @ref cursorMode
	 **/
	const QCursor& cursor() const;

	/**
	 * Set the canvas for the pixmap cursor. There are currently two
	 * different cursors, on @ref QCursor (see @ref cursor) and one @ref
	 * QCanvasSprite. The latter would be <em>far</em> nicer but also is
	 * slower and maybe it is not working at all. You need to call setCanvas
	 * for this experimental cursor.
	 **/
	void setCanvas(QCanvas* canvas);

	/**
	 * Move the @ref QCanvasSprite cursor.
	 * @param x x-coordinate on the @ref QCanvas
	 * @param y y-coordinate on the @ref QCanvas
	 **/
	void move(double x, double y);

	/**
	 * Load the cursor pixmaps for both types, the @ref QCanvasSprite
	 * cursors and the @ref QCursor cursors.
	 *
	 * Automatically called by @ref setCanvas
	 **/
	void loadCursors();

protected:

private:
	class BosonCursorPrivate;
	BosonCursorPrivate* d;
};

#endif
