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
 * UPDATE (02/03/09): the documentation is kind of obsolete. We use derived
 * classes now
 * 
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

	BosonCursor();
	virtual ~BosonCursor();

	int cursorMode() const { return mMode; }

	/**
	 * Change the current cursor. You probably want to call setWidget Cursor(this)
	 * from your widget, too in order to get the @ref QCanvas cursor (if it
	 * is used)
	 *
	 * You MUST reimplement this in derived classes! Also call the original
	 * implementation!
	 * @param mode A previously added mode - see @ref insertMode or -1 to
	 * hide the cursor
	 **/
	virtual void setCursor(int mode);
	
	/**
	 * Change the @ref QCursor cursor of the specified widget. This function
	 * uses @ref QWidget::setCursor if the feature is compiled in. Note that
	 * this should do nothing if you don't use @ref BosonNormalCursor
	 **/
	virtual void setWidgetCursor(QWidget* w) = 0;

	/**
	 * @return The current @ref QCursor according to @ref cursorMode. This
	 * does only something useful for @ref BosonNormalCursor but if
	 * everything else fails you might want to use this cursor.
	 **/
	virtual QCursor cursor() const;

	/**
	 * Move the @ref QCanvasSprite cursor. Note that this currently does
	 * only something useful if you use @ref BosonSpriteCursor.
	 *
	 * Reimplement this if it makes sense in your cursor class.
	 * @param x x-coordinate on the @ref QCanvas
	 * @param y y-coordinate on the @ref QCanvas
	 **/
	virtual void move(double x, double y);

	/**
	 * @return @ref QCursor::pos by default
	 **/
	virtual QPoint pos() const;

	/**
	 * Insert a mode to the internal dictionary. You can load your
	 * cursors using this function. You probably want to use an enum in your
	 * game for the mode id, like "Attack" and "Move" and so on.
	 *
	 * The two QString parameters specify the location of the files to be
	 * loaded. 
	 *
	 * You must implement this in your cursor class.
	 * @param mode The ID of the inserted mode 
	 * @param baseDir specifies the directory where the file are to be
	 * searched. 
	 * @param cursor specifies the cursor itself - this is the directoy for
	 * sprite-cursors or the filename for normal-cursors. See class
	 * documentation for more information.
	 **/
	virtual void insertMode(int mode, QString baseDir, QString cursor) = 0;

	/**
	 * Hides the cursor. Does nothing if the cursor is no @ref
	 * BosonSpriteCursor.
	 *
	 * Note that the current cursor settings does not change, e.g. if you
	 * use setCursor(-1) then your cursor does not display anything and will
	 * always be hidden (until you call @ref setCursor again), while this
	 * simply hides it.
	 *
	 * You may use it in your @ref QWidget::leaveEvent implementation.
	 * Note that this isn't useful for @ref BosonNormalCursor as there is no
	 * need for implementing @ref QWidget::leaveEvent or something like
	 * this. Nevertheless if you want to use this you probably need to store
	 * the @ref QWidget pointer in your @ref setWidgetCursor implementation
	 **/
	virtual void hideCursor() {}

	/**
	 * Show the cursor again.
	 *
	 * You may use it in your @ref QWidget::enterEvent implementation.
	 * See also @ref hideCursor
	 **/
	virtual void showCursor() {}


// Note: these do not belong here!
	virtual void paintCursor(QPainter* , const QPoint& ) {}
	virtual void removeOldCursor() {}
	inline virtual QRect oldCursor() const;


private:
	int mMode;
};

class BosonNormalCursor : public BosonCursor
{
public:
	BosonNormalCursor();
	virtual ~BosonNormalCursor();
	
	virtual void setCursor(int mode);
	virtual void setWidgetCursor(QWidget* w);
	void insertMode(int mode, QCursor* cursor);
	virtual QCursor cursor() const;

	virtual void insertMode(int mode, QString baseDir, QString cursor);

protected:
	QCursor* loadQCursor(QString baseDir, QString cursor);

private:
	class BosonNormalCursorPrivate;
	BosonNormalCursorPrivate* d;
};

class BosonKDECursor : public BosonCursor
{
public:
	BosonKDECursor();
	virtual ~BosonKDECursor();
	
	virtual void setCursor(int mode);
	virtual void setWidgetCursor(QWidget* w);
	virtual QCursor cursor() const;
	virtual void insertMode(int mode, QString baseDir, QString cursor);
};


class BosonSpriteCursor : public BosonCursor
{
	Q_OBJECT
public:
	BosonSpriteCursor();
	virtual ~BosonSpriteCursor();
	
	virtual void setCursor(int mode);
	virtual void setWidgetCursor(QWidget* w);
	QCanvasSprite* cursorSprite() const;

	/**
	 * Set the canvas for the sprite cursor. 
	 * @param mode The initial mode. See @ref insertMode
	 * @param z The z-coordinate of the sprite. Should be as high as
	 * possible.
	 **/
	void setCanvas(QCanvas* canvas, int mode, int z = 100000);

	virtual void move(double x, double y);

	void insertMode(int mode, QCanvasPixmapArray* pixmaps);

	virtual void hideCursor();

	virtual void showCursor();

	virtual void insertMode(int mode, QString baseDir, QString cursor);

	/**
	 * @return The position of the cursor on the canvas
	 **/
	virtual QPoint pos() const;

protected slots:
	void slotAdvance();

protected:
	QCanvasPixmapArray* loadSpriteCursor(QString baseDir, QString cursor);

private:
	class BosonSpriteCursorPrivate;
	BosonSpriteCursorPrivate* d;
};

class BosonExperimentalCursor : public BosonCursor
{
	Q_OBJECT
public:
	BosonExperimentalCursor();
	virtual ~BosonExperimentalCursor();

	virtual void setCursor(int mode);
	virtual void setWidgetCursor(QWidget* w);

	virtual void move(double x, double y);
	void insertMode(int mode, QCanvasPixmapArray* pixmaps);

	virtual void hideCursor();

	virtual void showCursor();

	virtual void insertMode(int mode, QString baseDir, QString cursor);


	virtual void removeOldCursor();
	virtual void paintCursor(QPainter* p, const QPoint& origin);
	inline virtual QRect oldCursor() const;


protected slots:
	void slotAdvance();

protected:
	QCanvasPixmapArray* loadCursor(QString baseDir, QString cursor);

private:
	void init();

private:
	class BosonExperimentalCursorPrivate;
	BosonExperimentalCursorPrivate* d;
};

#endif
