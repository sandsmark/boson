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

#include <qgl.h>

class BosonCanvas;
class BosonCursor;
class BoSelection;
class Player;
class Unit;
class UnitProperties;
class Camera;

class KGameChat;
class KGameIO;

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

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonBigDisplayBase : public QGLWidget
{
	Q_OBJECT
public:
	BosonBigDisplayBase(BosonCanvas* canvas, QWidget* parent);
	virtual ~BosonBigDisplayBase();

	void setLocalPlayer(Player* p);

	BosonCanvas* canvas() const { return mCanvas; }

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

	/**
	 * See @ref BosonBigDisplay::unitAction
	 **/
	virtual void unitAction(int) {}

	/**
	 * See @ref EditorBigDisplay::placeUnit
	 **/
	virtual void placeUnit(unsigned long int, Player*) {}

	/**
	 * See @ref EditorBigDisplay::placeCell
	 **/
	virtual void placeCell(int) {}

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
	void scrollBy(int x, int y);//AB: kind of obsolete, since we don't support QCanvas anymore


	// we should probably make these 4 methods protected. i cant imagine any
	// useful public use
	bool mapCoordinates(const QPoint& pos, GLdouble* posX, GLdouble* posY, GLdouble* posZ) const;
	bool mapCoordinatesToCell(const QPoint& pos, QPoint* cell);
	bool mapDistance(int windowDistanceX, int windowDistanceY, GLdouble* dx, GLdouble* dy) const;
	void worldToCanvas(GLfloat x, GLfloat y, GLfloat z, QPoint* pos) const;

	double fps() const;

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

	void signalChangeViewport(const QPoint& topLeft, const QPoint& topRight, const QPoint& bottomLeft, const QPoint& bottomRight);

protected slots:
	void slotMouseEvent(KGameIO* , QDataStream& stream, QMouseEvent* e, bool *eatevent);
	void slotCursorEdgeTimeout();

protected:
	virtual void initializeGL();
	virtual void resizeGL(int w, int h);
	virtual void paintGL();

	/**
	 * Called by @ref paintGL only to render text on the screen
	 **/
	void renderText();

	/**
	 * Called by @ref paintGL only to render the cells on the screen
	 **/
	void renderCells();

	virtual void enterEvent(QEvent*);
	virtual void leaveEvent(QEvent*);
	virtual bool eventFilter(QObject* o, QEvent* e);

	virtual void updateCursor() = 0;
	virtual void actionClicked(const BoAction& action, QDataStream& stream, bool* send) = 0;

	void generateCellList();

	/**
	 * Set a viewport. Basically the same as glViewport, but you should use
	 * this instead the standard OpenGL function. The viewport values are
	 * cached in boson, so that we can easily use it in @ref mapCoordinates.
	 *
	 * Note that the current matrix is not changed before glViewport is
	 * calles, so you need to ensure that it is called at the correct
	 * time/place.
	 **/
	void setViewport(int x, int y, GLsizei w, GLsizei h);

	/**
	 * Extract the frustum from both, modelview and projection matrices.
	 * Credits for this function go to Mark Morley - see
	 * http://www.markmorley.com/opengl/frustumculling.html
	 *
	 * We use pretty much of his code examples here so let me quote from the
	 * article: "[...] Unless otherwise noted, you may use any and all code
	 * examples provided herein in any way you want. [...]"
	 **/
	void extractFrustum();

	/**
	 * See @ref extractFrustum for more information about this stuff.
	 *
	 * We use a bounding spere so that we can easily rotate it.
	 * @return 0 if the object is not in the frustum (i.e. is not visible)
	 * otherwise the distance from the near plane. We might use this for the
	 * level of detail.
	 **/
	float sphereInFrustum(float x, float y, float z, float radius) const;

	void setCamera(const Camera& c);
	GLfloat cameraX() const;
	GLfloat cameraY() const;
	GLfloat cameraZ() const;

	bool checkError() const;

	void calcFPS();

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
