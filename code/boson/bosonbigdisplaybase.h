/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2002 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef BOSONBIGDISPLAYBASE_H
#define BOSONBIGDISPLAYBASE_H

#include "defines.h"

#ifndef NO_OPENGL
#include <qgl.h>
#else
#include <qcanvas.h>
#define GLfloat float 
#define GLdouble double 

#endif // !NO_OPENGL

class BosonCanvas;
class BosonCursor;
class BoSelection;
class Player;
class Unit;
class UnitProperties;

class KGameChat;
class KGameIO;
class QCanvas;

class BoAction
{
public:
	BoAction() 
	{
		mX = 0.0;
		mY = 0.0;
		mZ = 0.0;
		mForceAttack = false;
	}

	~BoAction()
	{
	}

	void setCanvasPos(const QPoint& pos) 
	{
		mCanvasPos = pos;
	}

	const QPoint& canvasPos() const
	{
		return mCanvasPos;
	}

	void setWorldPos(GLfloat x, GLfloat y, GLfloat z)
	{
		mX = x;
		mY = y;
		mZ = z;
	}

	void worldPos(GLfloat* x, GLfloat* y, GLfloat* z)
	{
		*x = mX;
		*y = mY;
		*z = mZ;
	}

	void setForceAttack(bool f)
	{
		mForceAttack = f;
	}

	bool forceAttack() const
	{
		return mForceAttack;
	}

private:
	QPoint mCanvasPos;
	GLfloat mX;
	GLfloat mY;
	GLfloat mZ;

	bool mForceAttack;
};

// moc is stupid
#ifndef NO_OPENGL
class MyHack : public QGLWidget
{
public:
 MyHack(QWidget* p, const char* name) : QGLWidget(p, name){}
};
#else
class MyHack : public QCanvasView
{
public:
 MyHack(QWidget* p, const char* name) : QCanvasView(p, name){}
};
#endif

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonBigDisplayBase : public MyHack
{
	Q_OBJECT
public:
	BosonBigDisplayBase(BosonCanvas* canvas, QWidget* parent);
	virtual ~BosonBigDisplayBase();

	void setLocalPlayer(Player* p);

	BosonCanvas* canvas() const { return mCanvas; }
	BosonCanvas* boCanvas() const { return mCanvas; } // since QScrollView returns QCanvas*

	/**
	 * Emit @ref signalMakeActive to inform @ref BosonWidget and @ref
	 * BoDisplayManager that this display should become the active display.
	 *
	 * Use this if you want to change the active status of the display!
	 **/
	void makeActive();

	/**
	 * Called by @ref BoDisplayManager (once it can handle GL displays)
	 *
	 * Do NOT call directly!
	 **/
	void setActive(bool a);

	BosonCursor* cursor() const { return mCursor; }
	void setCursor(BosonCursor* c) { mCursor = c; }
	BoSelection* selection() const { return mSelection; }

	/**
	 * Final cleanups. Mainly clear the selection.
	 **/
	void quitGame();

	virtual void unitAction(int ) {}

	void setKGameChat(KGameChat* chat);
	void addChatMessage(const QString& message);

	void setUpdateInterval(unsigned int ms);

	void setZoomFactor(float factor);

	/**
	 * Scroll by a certain distance.
	 *
	 * Note that these are pixel values, so depending on the current zoom
	 * factor this may be a long or a short distance in world-coordinates
	 **/
#ifndef NO_OPENGL
	void scrollBy(int x, int y);
#else
	virtual void scrollBy(int x, int y);
#endif


#ifndef NO_OPENGL
	// we should probably make these 2 methods protected. i cant imagine any
	// useful public use
	bool mapCoordinates(const QPoint& pos, GLdouble* posX, GLdouble* posY, GLdouble* posZ) const;
	bool mapDistance(int windowDistanceX, int windowDistanceY, GLdouble* dx, GLdouble* dy) const;
	void worldToCanvas(GLfloat x, GLfloat y, GLfloat z, QPoint* pos) const;

	/**
	 * A hack for an internal #if. Called by the display manager only,
	 * when the (animated) cursor has changed its frame. 
	 *
	 * We should cann updateGL() directly instead.
	 **/
	void updateGLCursor();
  
	double fps() const;
#endif

public slots:
	void slotCenterHomeBase();
	void slotResetViewProperties();

	/**
	 * @param pos the <em>cell</em>-coordinates of the centered position
	 **/
	void slotReCenterDisplay(const QPoint& pos);

	/**
	 * In case the unit has been destroyed make sure that it's removed from
	 * the local selection. 
	 *
	 * Currently this does not do anything else, but we might add some
	 * functionality in the future
	 **/
	void slotUnitChanged(Unit* unit);

signals:
	void signalMakeActive(BosonBigDisplayBase*);

	void signalTopLeftCell(int x, int y);

	void signalSizeChanged(int w, int h); //TODO: use cell values
	
protected slots:
	void slotMouseEvent(KGameIO* , QDataStream& stream, QMouseEvent* e, bool *eatevent);
	void slotCursorEdgeTimeout();

protected:
#ifndef NO_OPENGL
	virtual void initializeGL();
	virtual void resizeGL(int w, int h);
	virtual void paintGL();

	/**
	 * Called by @ref paintGL only to render text on the screen
	 **/
	void renderText();
#endif

	virtual void enterEvent(QEvent*);
	virtual void leaveEvent(QEvent*);
	virtual bool eventFilter(QObject* o, QEvent* e);

	virtual void updateCursor() = 0;

	virtual void actionClicked(const BoAction& action, QDataStream& stream, bool* send) = 0;

#ifndef NO_OPENGL
	void generateMapDisplayList();

	void setCameraPos(GLfloat x, GLfloat y, GLfloat z);
	GLfloat cameraX() const;
	GLfloat cameraY() const;
	GLfloat cameraZ() const;

	bool checkError() const;

	void calcFPS();
#endif

	bool selectAll(const UnitProperties* prop);

	/**
	 * Select units in the curren selection rect
	 **/
	void selectArea();

	/**
	 * Start the selection at x,y,z. Either select the unit at this position
	 * or start to draw the selection rect.
	 **/
	void startSelection(GLdouble x, GLdouble y, GLdouble z);

	/**
	 * Move the selection rect. @ref selectionStart is still the start point
	 * but @ref selectionEnd is now x,y,z
	 **/
	void moveSelectionRect(GLfloat x, GLfloat y, GLfloat z);

	/**
	 * Remove a currently drawn selection rect.
	 **/
	void removeSelectionRect();

	void selectionStart(GLfloat* x, GLfloat* y, GLfloat* z) const;
	void selectionEnd(GLfloat* x, GLfloat* y, GLfloat* z) const;

	/**
	 * @return The canvas coordinates of the selection rect
	 **/
	QRect selectionRectCanvas() const;


	Player* localPlayer() const;

	void addMouseIO(Player* p);

	virtual bool actionLocked() const = 0;

private:
	void init();

private:
	class BosonBigDisplayBasePrivate;
	BosonBigDisplayBasePrivate* d;

	BosonCanvas* mCanvas;
	BosonCursor* mCursor;
	BoSelection* mSelection;
};

#endif
