/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "bo3dtools.h"

#include "bosonglwidget.h"

class BosonCanvas;
class BosonCursor;
class BoSelection;
class Player;
class Unit;
class UnitProperties;
class Camera;
class BosonBigDisplayInputBase;
class BoItemList;

class KGameChat;
class KGameIO;
template<class T> class QPtrList;

/**
 * @author Andreas Beckermann <b_mann@gmx.de
 **/
class BoAction
{
public:
	BoAction()
	{
		mX = 0.0;
		mY = 0.0;
		mZ = 0.0;
		mControlButton = false;
		mShiftButton = false;
		mAltButton = false;
	}

	~BoAction()
	{
	}

	void setWidgetPos(const QPoint& pos)
	{
		mWidgetPos = pos;
	}
	const QPoint& widgetPos() const
	{
		return mWidgetPos;
	}
	void setCanvasVector(const BoVector3& pos)
	{
		mCanvasVector = pos;
		mCanvasPos = QPoint((int)pos.x(), (int)pos.y());
	}

	const QPoint& canvasPos() const
	{
		return mCanvasPos;
	}
	const BoVector3& canvasVector() const
	{
		return mCanvasVector;
	}

	void setWorldPos(GLfloat x, GLfloat y, GLfloat z)
	{
		mX = x;
		mY = y;
		mZ = z;
	}

	void worldPos(GLfloat* x, GLfloat* y, GLfloat* z) const
	{
		*x = mX;
		*y = mY;
		*z = mZ;
	}

	void setControlButton(bool b)
	{
		mControlButton = b;
	}
	bool controlButton() const
	{
		return mControlButton;
	}
	void setShiftButton(bool b)
	{
		mShiftButton = b;
	}
	bool shiftButton() const
	{
		return mShiftButton;
	}
	void setAltButton(bool b)
	{
		mAltButton = b;
	}
	bool altButton() const
	{
		return mAltButton;
	}

	bool forceAttack() const
	{
		// TODO: make configurable
		return mControlButton;
	}

private:
	QPoint mWidgetPos;
	QPoint mCanvasPos;
	BoVector3 mCanvasVector;
	GLfloat mX;
	GLfloat mY;
	GLfloat mZ;

	bool mControlButton;
	bool mShiftButton;
	bool mAltButton;
};

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonBigDisplayBase : public BosonGLWidget
{
	Q_OBJECT
public:
	BosonBigDisplayBase(BosonCanvas* canvas, QWidget* parent);
	virtual ~BosonBigDisplayBase();

	void setDisplayInput(BosonBigDisplayInputBase* input);

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
	 * See @ref BosonBigDisplayInputBase::unitAction
	 **/
	void unitAction(int);

	void setKGameChat(KGameChat* chat);
	void addChatMessage(const QString& message);

	void setUpdateInterval(unsigned int ms);

	void setDebugMapCoordinates(bool debug);
	void setDebugShowCellGrid(bool debug);
	void setDebugMatrices(bool debug);
	void setDebugItemWorks(bool debug);

	/**
	 * Scroll by a certain distance.
	 *
	 * Note that these are pixel values, so depending on the current zoom
	 * factor this may be a long or a short distance in world-coordinates
	 **/
	void scrollBy(int x, int y);//AB: kind of obsolete, since we don't support QCanvas anymore


	bool boProject(GLfloat x, GLfloat y, GLfloat z, QPoint* pos) const;
	bool boUnProject(const QPoint& pos, BoVector3* v, float z = -1.0) const;

	// we should probably make these 4 methods protected. i cant imagine any
	// useful public use
	/*
	 * @param useRealDepth If TRUE this function will calculate the real
	 * coordinates at @p pos, if FALSE it will calculate the coordinate at
	 * @p pos with z=0.0. This is useful for e.g. @ref mapDistance, where
	 * different z values could deliver wrong values.
	 **/
	bool mapCoordinates(const QPoint& pos, GLfloat* posX, GLfloat* posY, GLfloat* posZ, bool useRealDepth = true) const;
	bool mapCoordinatesToCell(const QPoint& pos, QPoint* cell);
	bool mapDistance(int windowDistanceX, int windowDistanceY, GLfloat* dx, GLfloat* dy) const;
	void worldToCanvas(GLfloat x, GLfloat y, GLfloat z, QPoint* pos) const;
	void worldToCanvas(GLfloat x, GLfloat y, GLfloat z, BoVector3* pos) const;
	void canvasToWorld(int x, int y, float z, GLfloat* glx, GLfloat* gly, GLfloat* glz) const;

	double fps() const;

	void mapChanged();

	Player* localPlayer() const;

	/**
	 * @param prop The unit that should get placed or NULL if none.
	 * @param canPlace Whether @p prop can be placed at the current cursor
	 * position (current == the moment when @ref BosonBigDisplayInputBase::updatePlacementPreviewData
	 * has been called)
	 **/
	void setPlacementPreviewData(const UnitProperties* prop, bool canPlace);

	/**
	 * Same as above - but this will make a cell placement preview, instead
	 * of a unit placement preview.
	 **/
	void setPlacementCellPreviewData(int groundType, bool canPlace);

	void setToolTipCreator(int type);
	void setToolTipUpdatePeriod(int ms);

	const QPoint& cursorCanvasPos() const; // obsolete!
	const BoVector3& cursorCanvasVector() const;
	BosonBigDisplayInputBase* displayInput() const;

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

	/**
	 * Called when the player clicks (RMB) on the minimap. When there is a
	 * unit selected it should move to that point in game mode. See @ref
	 * BosonBigDisplayInputBase::slotMoveSelection
	 **/
	void slotMoveSelection(int x, int y);

signals:
	void signalMakeActive(BosonBigDisplayBase*);

	void signalChangeViewport(const QPoint& topLeft, const QPoint& topRight, const QPoint& bottomLeft, const QPoint& bottomRight);

protected slots:
	void slotMouseEvent(KGameIO* , QDataStream& stream, QMouseEvent* e, bool *eatevent);
	void slotCursorEdgeTimeout();

	/**
	 * Called by @ref Boson::signalAdvance.
	 *
	 * Note that it is <em>not</em> ensured, that @ref
	 * BosonCanvas::slotAdvance is called first. It might be possible that
	 * this slot gets called before @ref BosonCanvas::slotAdvance but the
	 * other way round might be possible as well.
	 *
	 * Also note that this should <em>not</em> be used for game logic parts
	 * that the network might depend on. Use it for OpenGL or similar
	 * operations (input/output on the local client) only.
	 **/
	void slotAdvance(unsigned int advanceCount, bool advanceFlag);

protected:
	/**
	 * Here the defined action for a wheel event should happen. See
	 * docs/mouse-big_display.txt for a list of actions that are allowed
	 * here.
	 * @param delta See QWheelEvent::delta. This is how much the wheel was
	 * moved.
	 * @param orientation Guess what? Yes! Horizontal or Vertical wheel.
	 * @param action Information about the event (position, modifiers, ...).
	 * See @ref BoAction
	 * @param stream You won't need this here.
	 * @param send You won't need this here.
	 **/
	void mouseEventWheel(float delta, Orientation orientation, const BoAction& action, QDataStream& stream, bool* send);

	/**
	 * @param buttonState See @ref QMouseEvent::state. This tells you which
	 * buttons are currently pressed.
	 * @param action Information about the event (position, modifiers, ...).
	 * See @ref BoAction
	 * @param stream You won't need this here.
	 * @param send You won't need this here.
	 **/
	void mouseEventMove(int buttonState, const BoAction& action, QDataStream& stream, bool* send);

	/**
	 * This is the main event for actual actions. When a player clicks RMB
	 * on an enemy unit and expects his selection to attack that unit, then
	 * this action is started here. No action is allowed in mouse press
	 * events.
	 *
	 * See docs/mouse-big_display.txt for further description and a list of
	 * allowed actions here.
	 * @param button Which button produced this event.
	 * @param action Information about the event (position, modifiers, ...).
	 * See @ref BoAction
	 * @param stream Stream your action here, if it is network (i.e.
	 * game-)relevant. Actions like zooming, rotating (i.e. chaging the
	 * camera) or selecting units can be done immediately, but all that
	 * requires units to do something must be streamed and sent using a
	 * message.
	 * @param send Set this to TRUE in order to actually send the @p stream
	 **/
	void mouseEventRelease(ButtonState button, const BoAction& action, QDataStream& stream, bool* send);

	/**
	 * @param button Which button produced this event.
	 * @param action Information about the event (position, modifiers, ...).
	 * See @ref BoAction
	 * @param stream Stream your action here, if it is network (i.e.
	 * game-)relevant. Actions like zooming, rotating (i.e. chaging the
	 * camera) or selecting units can be done immediately, but all that
	 * requires units to do something must be streamed and sent using a
	 * message.
	 * @param send Set this to TRUE in order to actually send the @p stream
	 **/
	void mouseEventReleaseDouble(ButtonState button, const BoAction& action, QDataStream& stream, bool* send);

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

	/**
	 * Called by @ref paintGL only to render the particles on the screen
	 **/
	void renderParticles();

	void renderMatrix(int x, int y, const BoMatrix* matrix, const QString& text);

	void renderString(int x, int y, const QString& text);

	virtual void enterEvent(QEvent*);
	virtual void leaveEvent(QEvent*);
	virtual bool eventFilter(QObject* o, QEvent* e);

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
	float sphereInFrustum(const BoVector3& pos, float radius) const;
	inline float sphereInFrustum(float x, float y, float z, float radius) const
	{
		BoVector3 pos(x,y,z);
		return sphereInFrustum(pos, radius);
	}

	void setCamera(const Camera& c);
	Camera* camera() const;
	/**
	 * @return Point that the camera is looking at
	 **/
	const BoVector3& cameraLookAtPos() const;
	void cameraChanged();

	bool checkError() const;

	void calcFPS();

	/**
	 * Move the selection rect. @ref selectionStart is still the start point
	 * but @ref selectionEnd is now x,y,z
	 **/
	void moveSelectionRect(const QPoint& widgetPos);

	/**
	 * Calculate the maximum and minimum world coordinates from the
	 * specified rectangles.
	 *
	 * The rect @p rect is in window coordinates (e.g. the selection rect).
	 **/
	void calculateWorldRect(const QRect& rect, float* minX, float* minY, float* maxX, float* maxY) const;

	/**
	 * Remove a currently drawn selection rect and select all units inside
	 * this rect.
	 * @param replace If TRUE the current selection is replaced, otherwise
	 * the selected units are added to the selection.
	 * Usually when the player holds the shift key down while selecting.
	 **/
	void removeSelectionRect(bool replace);

	BoItemList* selectionRectItems();

	void addMouseIO(Player* p);

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
