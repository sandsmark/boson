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
#ifndef BOSONCURSOR_H
#define BOSONCURSOR_H

#include "defines.h"
#include <qobject.h>
#include <GL/gl.h>

class QPoint;
class QWidget;
class QCursor;
class BosonCanvas;
class BosonTextureArray;

/**
 * Note: the docs are partially obsolete. I updated some parts, but not all.
 *
 * This is the cursor class for boson. There are two different types of cursors.
 * First of all the default x/qt-cursors. They are used by every KDE/QT program.
 * You can use it by calling @ref QWidget::setCursor for example.
 *
 * Then there are animated and colored cursors. X does not support colored
 * cursors, but only black and white. So we render a simple textured rectangle
 * using OpenGL instead instead of real cursor and just set the cursor to 
 * @ref Qt::BlankCursor. This way enables us to use full colored, transparent
 * and even animated cursors of any size. This is what BosonCursor is for.
 *
 * In theory you only have to create an instance of BosonCursor and then call
 * @ref insertMode, @ref setCursor and @ref setWidgetCursor.
 *
 * You can call @ref setCursor to change the cursor - e.g. you want to have a
 * different cursor for selecting units than for attacking units. You should also
 * call @ref setWidgetCursor after @ref setCursor.
 *
 * @sect Files
 * You need to provide pixmaps for the sprite-cursor and you can provide pixmaps
 * for the X/QT cursor. Please note that the X/QT cursors need to be black/white!
 *
 * The files should reside in your apps data dir, i.e. $KDEDIR/apps/game_name/
 * the sprites-cursors expect a separate subdirectory, while the X/QT pixmaps
 * expect just a file of the same name. The directory name/file name is specified
 * to BosonCursor using @ref insertMode.
 *
 * On my system this looks like this for
 * the mode "move":
 * <pre>
 * /opt/kde3/share/apps/boson/cursors/move/
 * /opt/kde3/share/apps/boson/cursors/move.png
 * </pre>
 * The sprites cursors now expect an additional file in its subdir:
 * index.cursor . This file has one group (BosonCursor) and defines the hotspots
 * and the filenames as well as the number of files that belong to your cursor.
 * Look at example index.cursor files in the cursors dir of boson for the usage.
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
	 * Change the current cursor. You probably want to call setWidgetCursor(this)
	 * from your widget, too in order to get the OpenGL cursor (if it
	 * is used)
	 *
	 * You MUST reimplement this in derived classes! Also call the original
	 * implementation!
	 * @param mode A previously added mode - see @ref insertMode or -1 to
	 * hide the cursor
	 **/
	virtual void setCursor(int mode);
	
	/**
	 * Change the @ref QCursor cursor of the specified widget. E.g. in case
	 * an OpenGL cursor is used this will hide the usual cursor for this
	 * widget (so that not two different cursors are shown)
	 **/
	virtual void setWidgetCursor(QWidget* w) = 0;

	/**
	 * @return The current @ref QCursor according to @ref cursorMode.
	 * If everything else fails you might want to use this cursor.
	 **/
	virtual QCursor cursor() const;

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
	 * opengl-cursors. See class documentation for more information.
	 **/
	virtual bool insertMode(int mode, QString baseDir, QString cursor) = 0;

	/**
	 * Render the cursor on the screen with x,y as the central point. Does
	 * nothing for non-OpenGL cursors.
	 **/
	virtual void renderCursor(GLfloat , GLfloat ) {}

	/**
	 * @return All cursor themes that can be found, i.e. that have a
	 * "index.cursor" file. I.e. there must be a fine
	 * cursors/theme_name/index.cursor, so that cursors/theme_name is a
	 * cursor theme.
	 **/
	static QStringList availableThemes();
	static QString defaultTheme();

private:
	int mMode;
};

class BosonKDECursor : public BosonCursor
{
	Q_OBJECT
public:
	BosonKDECursor();
	virtual ~BosonKDECursor();
	
	virtual void setCursor(int mode);
	virtual void setWidgetCursor(QWidget* w);
	virtual QCursor cursor() const;
	virtual bool insertMode(int mode, QString baseDir, QString cursor);
};


class BosonOpenGLCursorData
{
public:
	BosonOpenGLCursorData();
	~BosonOpenGLCursorData();

	void setHotspot(unsigned int x, unsigned int y)
	{
		mHotspotX = x;
		mHotspotY = y;
	}

	/**
	 * Compute a display lists for every image in @p images. The list will
	 * display a rectangular cursor (you'll want to render it with alpha
	 * blending enabled).
	 *
	 * Note that the hotspot must already be applied when you call this.
	 * @param images Use these images as textures. Note that a @ref
	 * BosonTextureArray object will get generated for this.
	 **/
	bool computeDisplayLists(QValueList<QImage> images);

	GLuint* mDisplayLists;
	GLuint mDisplayListCount;
	bool mAnimated;
	unsigned int mAnimationSpeed;
	int mRotateDegree;

private:
	BosonTextureArray* mTextureArray;
	unsigned int mHotspotX;
	unsigned int mHotspotY;
};

class BosonOpenGLCursor : public BosonCursor
{
	Q_OBJECT
public:
	BosonOpenGLCursor();
	virtual ~BosonOpenGLCursor();
	
	virtual void setCursor(int mode);
	virtual void setWidgetCursor(QWidget* w);

	virtual bool insertMode(int mode, QString baseDir, QString cursor);

	bool insertMode(int mode, BosonOpenGLCursorData* data);
	void setCurrentTextureArray(BosonTextureArray* array);
	void setCurrentData(BosonOpenGLCursorData* data);

	virtual void renderCursor(GLfloat x, GLfloat y);

protected slots:
	void slotAdvance();

protected:
	BosonOpenGLCursorData* loadSpriteCursor(QString baseDir, QString cursor);

private:
	class BosonOpenGLCursorPrivate;
	BosonOpenGLCursorPrivate* d;

	BosonOpenGLCursorData* mCurrentData;
	GLuint mCurrentDisplayList;
};


#endif

