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
#ifndef __BOSONCURSOR_H__
#define __BOSONCURSOR_H__

#include <qobject.h>

class QCanvas;
class QCanvasPixmapArray;
class QCanvasSprite;
class QPoint;
class QWidget;
class QCursor;

/**
 * This is the cursor class for boson. There are two different types of cursors.
 * First of all the default x/qt-cursors. They are used by every KDE/QT program.
 * You can use it by calling @ref QWidget::setCursor for example.
 *
 * Then there are animated and colored cursors. X does not support colored
 * cursors, but only black and white. For this we use a @ref QCanvasSprite
 * instead of real cursor and just set the cursor to @ref Qt::BlankCursor. This
 * way enables us to use full colored, transparent and even animated cursors of
 * any size. This is what BosonCursor is for.
 *
 * Please note that we do <em>not</em> support changing from x/qt to sprites or
 * vice versa at runtime anymore. You should define NO_PIXMAP_CURSOR in order to
 * get the default X/QT cursors - maybe you want to add a configure option for
 * the user here. X/QT cursors are faster and use less memory but they are
 * quite... boring. Especially for games. Sprite-Cursors are the default for
 * BosonCursor.
 *
 * In theory you only have to create an instance of BosonCursor and then call
 * @ref insertMode, @ref setCanvas, @ref setCursor and @ref move. Here an example:
 * <pre>
 * mCursor = new BosonCanvas;
 * mCursor->insertMode(CursorMove, mPixmapArrayMove, QCursor(Qt::BlankCursor));
 * mCursor->setCanvas(CursorMove, this);
 * // mCursor->setCursor(CursorMove); // you can leave this out as you specified before
* [...]
* MyCanvasView::contentsMouseMoveEvent(QMouseEvent* e)
* {
* mCursor->move(e->pos.x(), pos.y());
* canvas()->update();
* }
* </pre>
* That's all you have to do.
*
* A special note about @ref move: you most probably want to call
* QCanvas::update() after this. Otherwise you experience heavy flickering!
*
* You can call @ref setCursor to change the cursor - e.g. you want to have a
* different cursor for selecting units than for attacking units. You should also
* call @ref setWidgetCursor after @ref setCursor. You don't need this though -
* but you enable your users to use a normal @ref QCursor instead of sprites.
*
* @section Files
* You need to provide pixmaps for the sprite-cursor and you can provide pixmaps
* for the X/QT cursor. Please note that the X/QT cursors need to be black/white!
*
* The files should reside in your apps data dir, i.e. $KDEDIR/apps/game_name/
* the sprites-cursors expect a separate subdirectory, while the X/QT pixmaps
* expect just a file of the same name. The directory name/file name is specified
* to BosonCursor using @ref insertMode but you can also directly provide the
* @ref QCanvasPixmapArray or @ref QCursor. On my system this looks like this for
* the mode "move":
* <pre>
* /opt/kde3/share/apps/boson/cursors/move/
* /opt/kde3/share/apps/boson/cursors/move.png
* </pre>
* The sprites cursors now expect an additional file in its subdir:
* index.desktop. This file has one group (BosonCursor) and defines the hotspots
* and the filenames as well as the number of files that belong to your cursor.
* Look at example index.desktop files in the cursors dir of boson for the usage.
* @author Andreas Beckermann <b_mann@gmx.de>
* @short Cursor class for animated cursors
**/
class BosonCursor : public QObject
{
	Q_OBJECT
public:
	enum CursorMode {
		Attack = 0,
		Move = 1,
		Default = 2,
		
		Hide // this MUST be the last entry!
	};
	BosonCursor();
	~BosonCursor();

	int cursorMode() const;

	/**
	 * Change the current cursor. You probably want to call setWidget Cursor(this)
	 * from your widget, too in order to get the @ref QCanvas cursor (if it
	 * is used)
	 * @param mode A previously added mode - see @ref insertMode or -1 to
	 * hide the cursor
	 **/
	void setCursor(int mode);
	
	/**
	 * Change the @ref QCursor cursor of the specified widget. This function
	 * uses @ref QWidget::setCursor if the feature is compiled in. Note that
	 * boson uses currently a #define to decide whether to use it or not.
	 **/
	void setWidgetCursor(QWidget* w);

	/**
	 * @return The current @ref QCursor according to @ref cursorMode
	 **/
	QCursor cursor() const;
	QCanvasSprite* cursorSprite() const;

	/**
	 * Set the canvas for the pixmap cursor. There are currently two
	 * different cursors, on @ref QCursor (see @ref cursor) and one @ref
	 * QCanvasSprite. The latter would be <em>far</em> nicer but also is
	 * slower and maybe it is not working at all. You need to call setCanvas
	 * for this experimental cursor.
	 * @param mode The initial mode. See @ref insertMode
	 **/
	void setCanvas(QCanvas* canvas, int mode, int z = 100000);

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
	void loadCursors(const QString& dir);

	/**
	 * Insert an mode to the internal dictionary. You can load your
	 * cursors using this function. You probably want to use an enum in your
	 * game for the mode id, like "Attack" and "Move" and so on.
	 *
	 * The two QString parameters specify the location of the files to be
	 * loaded. 
	 * @param mode The ID of the inserted mode 
	 * @param baseDir specifies the directory where the file are to be
	 * searched. 
	 * @param cursor specifies the cursor itself - this is the directoy for
	 * sprite-cursors or the filename for normal-cursors. See class
	 * documentation for more information.
	 **/
	void insertMode(int mode, QString baseDir, QString cursor);

	/**
	 * Like the above function, but doesn't load the pixmaps but take
	 * already allocated ones.
	 *
	 * Please note that these must be created using new and must no be
	 * deleted! This class takes care of deletion.
	 **/
	void insertMode(int mode, QCanvasPixmapArray* pixmaps, QCursor* cursor);

	/**
	 * Hides the cursor. Equivalent to calling cursorSprite()->hide().
	 *
	 * Note that the current cursor settings does not change, e.g. if you
	 * use setCursor(-1) then your cursor does not display anything and will
	 * always be hidden (until you call @ref setCursor again), while this
	 * simply hides it.
	 *
	 * You may use it in your @ref QWidget::leaveEvent implementation.
	 **/
	void hideCursor();

	/**
	 * Show the cursor again. Equivalent to calling cursorSprite()->show().
	 *
	 * You may use it in your @ref QWidget::enterEvent implementation.
	 * See also @ref hideCursor
	 **/
	void showCursor();


protected:
	QCursor* loadQCursor(QString baseDir, QString cursor);
	QCanvasPixmapArray* loadSpriteCursor(QString baseDir, QString cursor);

protected slots:
	void slotAdvance();

private:
	class BosonCursorPrivate;
	BosonCursorPrivate* d;
};

#endif
