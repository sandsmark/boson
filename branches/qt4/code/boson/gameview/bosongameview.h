/*
    This file is part of the Boson game
    Copyright (C) 1999-2000 Thomas Capricelli (capricel@email.enst.fr)
    Copyright (C) 2001-2006 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001-2006 Rivo Laks (rivolaks@hot.ee)

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
#ifndef BOSONGAMEVIEW_H
#define BOSONGAMEVIEW_H

#include "../boufo/boufo.h"
#include "../bo3dtools.h"

class BoGameCamera;
class BoAutoGameCamera;
class Unit;
class BosonCanvas;
class BosonCursor;
class BoSelection;
class Player;
class PlayerIO;
class BosonItem;
class BosonScript;
class BoLight;
class BoFontInfo;
class BoItemList;
class BosonGameViewInputBase;
class BoSpecificAction;
class BoUfoActionCollection;
class BosonGameFPSCounter;
class BosonViewData;
class KGameIO;
class QDomElement;
class Boson;
class BosonLocalPlayerInput;
class BosonUfoCanvasWidget;



/**
 * Small class that takes care of scrolling when the cursor is at the edge of
 * the window
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoCursorEdgeScrolling : public QObject
{
	Q_OBJECT
public:
	BoCursorEdgeScrolling(QObject* parent);
	~BoCursorEdgeScrolling();

	void setCamera(BoGameCamera* camera) { mCamera = camera; }
	BoGameCamera* camera() const { return mCamera; }
	void setMatrices(const BoGLMatrices* matrices) { mMatrices = matrices; }
	const BoGLMatrices* matrices() const { return mMatrices; }

	void quitGame();

protected:
	virtual bool eventFilter(QObject* o, QEvent* e);

protected slots:
	void slotCursorEdgeTimeout();

private:
	BoGameCamera* mCamera;
	const BoGLMatrices* mMatrices;
	QTimer* mCursorEdgeTimer;
	int mCursorEdgeCounter;
};


class SelectionRect : public QObject
{
	Q_OBJECT
public:
	SelectionRect();
	void setMatrices(const BoGLMatrices* m)
	{
		mMatrices = m;
	}

	void widgetRect(QRect* rect) const;

	void setStartWidgetPos(const QPoint& pos);
	void setEndWidgetPos(const QPoint& pos);
	const QPoint& startPos() const
	{
		return mStartPos;
	}

	bool isVisible() const
	{
		return mVisible;
	}
	void setVisible(bool v);

	void quitGame();

	/**
	 * @return A list of items that are currently in the selection rect
	 **/
	BoItemList* items(const BosonUfoCanvasWidget* canvasWidget) const;

signals:
	void signalVisible(bool);
	void signalChanged(const QRect&);

private:
	const BoGLMatrices* mMatrices;
	QPoint mStartPos;
	QPoint mEndPos;
	bool mVisible;
};


/**
 * @author Andreas Beckermann <b_mann@gmx.de
 **/
class BoMouseEvent
{
public:
	BoMouseEvent()
	{
		mX = 0.0;
		mY = 0.0;
		mZ = 0.0;
		mControlButton = false;
		mShiftButton = false;
		mAltButton = false;
		mUnitAtEventPos = 0;
	}

	~BoMouseEvent()
	{
	}

	void setGameViewWidgetPos(const QPoint& pos)
	{
		mWidgetPos = pos;
	}
	const QPoint& gameViewWidgetPos() const
	{
		return mWidgetPos;
	}
	void setGroundCanvasVector(const BoVector3Fixed& pos)
	{
		mGroundCanvasVector = pos;
		mGroundCanvasPos = QPoint((int)pos.x(), (int)pos.y());
	}
	void setUnitAtEventPos(Unit* unit)
	{
		mUnitAtEventPos = unit;
	}

	const QPoint& groundCanvasPos() const
	{
		return mGroundCanvasPos;
	}
	const BoVector3Fixed& groundCanvasVector() const
	{
		return mGroundCanvasVector;
	}
	Unit* unitAtEventPos() const
	{
		return mUnitAtEventPos;
	}

	void setGroundWorldPos(GLfloat x, GLfloat y, GLfloat z)
	{
		mX = x;
		mY = y;
		mZ = z;
	}

	void groundWorldPos(GLfloat* x, GLfloat* y, GLfloat* z) const
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
	QPoint mGroundCanvasPos;
	BoVector3Fixed mGroundCanvasVector;
	GLfloat mX;
	GLfloat mY;
	GLfloat mZ;

	bool mControlButton;
	bool mShiftButton;
	bool mAltButton;

	Unit* mUnitAtEventPos;
};


/**
 * @short The current mouse position
 *
 * This class should always contain the current mouse position in the formats
 * required by Boson. The position is set using @ref set, which should be called
 * whenever a mouse move event occurs.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoCursorPos
{
public:
	void set(const QPoint& gameView, const QPoint& rootPane, const BoVector3Fixed& canvas)
	{
		mCursorGameViewPos = gameView;
		mCursorBoUfoRootPanePos = rootPane;
		mCursorCanvasVector = canvas;
	}

	/**
	 * @return The cursor position relative to the game view, i.e. the main
	 * widget of Boson. This is in X11-like coordinates, that means (0,0)
	 * describes the top-left corner of the widget
	 **/
	const QPoint& gameViewPos() const
	{
		return mCursorGameViewPos;
	}

	/**
	 * @return Like @ref gameViewPos, but as a pointer. This pointer is
	 * guaranteed to always contain the current @ref gameViewPos, as long as
	 * this object exists.
	 **/
	const QPoint* gameViewPosPointer() const
	{
		return &mCursorGameViewPos;
	}

	/**
	 * @return The current cursor position relative to the libufo root pane,
	 * that is to @ref BoUfoManager::rootPane. These can be useful to be
	 * used within @ref BoUfoWidget::mapFromRoot widget other than the game
	 * view. Note that libufo uses X11-like coordinates, i.e. (0,0) is the
	 * top-left corner.
	 **/
	const QPoint& rootPanePos() const
	{
		return mCursorBoUfoRootPanePos;
	}
	/**
	 * @return Like @ref rootPanePos, but as a pointer that is guaranteed to
	 * always contain the current @ref rootPanePos as long as this object
	 * exists.
	 **/
	const QPoint* rootPanePosPointer() const
	{
		return &mCursorBoUfoRootPanePos;
	}

	/**
	 * @return The current cursor position in 3d canvas-coordinates
	 **/
	const BoVector3Fixed& canvasVector() const
	{
		return mCursorCanvasVector;
	}

	/**
	 * @return Like @ref rootPanePos, but as a pointer that is guaranteed to
	 * always contain the current @ref rootPanePos as long as this object
	 * exists.
	 **/
	const BoVector3Fixed* canvasVectorPointer() const
	{
		return &mCursorCanvasVector;
	}

private:
	QPoint mCursorGameViewPos;
	QPoint mCursorBoUfoRootPanePos;
	BoVector3Fixed mCursorCanvasVector;
};


/**
 * This class emulates a state machine with the events "button press", "button
 * release", and "mouse move", and the guards "ALT pressed", "Shift pressed",
 * and "CTRL pressed". I.e. the guards are the currently pressed modifiers.
 *
 * The state diagram would look something like the following:
 * <pre>
 * Start: "button press": enter state "Pressed"
 * Start: "button release": stay in "Start" (error)
 * Start: "mouse move": stay in "Start"
 *
 * Pressed: "button press": stay in "Pressed" (error)
 * Pressed: "button release": execute action depending on the guards and enter "Start"
 * Pressed: "mouse move" if camera modifier is pressed: execute camera action
 *                                                      and enter "PressedCamera"
 * Pressed: "mouse move" otherwise: execute mouse move action and enter
 *                                  "PressedMoved"
 *
 * PressedCamera: "button press": Stay in "PressedCamera" (error)
 * PressedCamera: "button release": Enter "Start"
 * PressedCamera: "mouse move" if camera modifier is pressed: execute mouse move
 *                              action and stay in "PressedCamera"
 * PressedCamera: "mouse move" otherwise: execute mouse move action and enter
 *                                        "PressedMoved"
 *
 * PressedMoved: "button press": Stay in "PressedMoved" (error)
 * PressedMoved: "button release": Execute action depending on the guards and
 *                                 enter "Start"
 * PressedMoved: "mouse move" if camera modifier is pressed: Execute camera
 *                                 action and enter "PressedCamera"
 * PressedMoved: "mouse move" otherwise: Execute mouse move action and stay in
 *                                       "PressedMoved"
 * </pre>
 * (the diagram looks much less complex in an actual diagram, instead of the
 * textual representation)
 **/
class BoMouseButtonState
{
public:
	BoMouseButtonState();
	virtual ~BoMouseButtonState();

	/**
	 * Should be called when the mouse button that is responsible for this
	 * state is being pressed.
	 *
	 * This enters this state.
	 **/
	void pressButton(const QPoint& widgetPos);

	/**
	 * Should be called when the mouse button that is responsible for this
	 * state is being released.
	 *
	 * This leaves this state.
	 **/
	void releaseButton(const BoMouseEvent& modifiers, bool doubleRelease = false);

	/**
	 * Should be called when the mouse is moved
	 **/
	void mouseMoved(const BoMouseEvent& modifiers);

protected:
	/**
	 * Called when the button is released, i.e. when this state is left.
	 * Now the action that belongs to this object should be executed. Note
	 * that when this method is called, the mouse was not moved since the
	 * button was pressed. See also @ref actionAfterMove.
	 *
	 * For example after a LMB release a unit might get selected or after a
	 * RMB release the selection may be ordered to move or attack.
	 **/
	virtual void action(const BoMouseEvent& modifiers) = 0;

	/**
	 * Like @ref action, but responds to double clicks (i.e. double
	 * button releases).
	 *
	 * By default this calls @ref action
	 **/
	virtual void actionDouble(const BoMouseEvent& modifiers);

	/**
	 * Called when the button is released and the mouse was moved after the
	 * button was pressed. Now a corresponding action should be executed.
	 *
	 * For example after a LMB release the selection rect may get used to
	 * select a group of units.
	 **/
	virtual void actionAfterMove(const BoMouseEvent& modifiers) = 0;

	/**
	 * Called when the mouse is moved while the camera modifier is pressed.
	 * A reimplementation should modify the camera in some way
	 * or do nothing.
	 *
	 * For example LMB move + camera modifier may zoom the camera ; RMB move +
	 * camera modifier may rotate it.
	 **/
	virtual void cameraAction(const BoMouseEvent&) = 0;

	/**
	 * Called when the mouse is mvoed with the camera modifier not being
	 * pressed. A Reimplmenetation should do the normal mouse move action.
	 *
	 * For example LMB move may resize the selection rect and RMB move may move
	 * the camera.
	 **/
	virtual void moveAction(const BoMouseEvent&) = 0;


	/**
	 * @return The position where @ref pressButton was called
	 **/
	const QPoint& startedAtWidgetPos() const { return mStartWidgetPos; }

	/**
	 * @return The position where @ref mouseMoved or @ref pressButton
	 * whichever happened most recently, was called the last time. If no
	 * call to @ref mouseMoved was made, this equals @ref
	 * startedAtWidgetPos.
	 **/
	const QPoint& currentWidgetPos() const { return mCurrentWidgetPos; }

	/**
	 * @return The amount of pixels in x direction that were moved by the
	 * last @ref mouseMoved call. 0 if no such call was made since the last
	 * @ref pressButton call.
	 **/
	int currentWidgetPosDiffX() const { return mCurrentWidgetPosDiffX; }

	/**
	 * @return The amount of pixels in y direction that were moved by the
	 * last @ref mouseMoved call. 0 if no such call was made since the last
	 * @ref pressButton call.
	 **/
	int currentWidgetPosDiffY() const { return mCurrentWidgetPosDiffY; }

private:
	bool mButtonIsReleased;
	bool mIsMove;
	bool mIsCameraAction;

	QPoint mStartWidgetPos;

	QPoint mCurrentWidgetPos;

	int mCurrentWidgetPosDiffX;
	int mCurrentWidgetPosDiffY;
};




class BosonGameViewPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonGameView : public BoUfoCustomWidget
{
	Q_OBJECT
public:
	BosonGameView();
	virtual ~BosonGameView();

	void bosonObjectCreated(Boson* boson);
	void bosonObjectAboutToBeDestroyed(Boson* boson);

	void setGameFPSCounter(BosonGameFPSCounter* counter);
	BosonGameFPSCounter* gameFPSCounter() const;

	void setDisplayInput(BosonGameViewInputBase* input);
	BosonGameViewInputBase* displayInput() const;

	bool isInputInitialized();
	void setInputInitialized(bool initialized);

	void setCanvas(BosonCanvas* canvas);
	const BosonCanvas* canvas() const { return mCanvas; }
	void setLocalPlayerIO(PlayerIO* p);
	PlayerIO* localPlayerIO() const;
	void setLocalPlayerScript(BosonScript* script);

	void createActionCollection(BoUfoActionCollection* parent);
	BoUfoActionCollection* actionCollection() const;


	const QPoint& cursorWidgetPos() const;
	const BoVector3Fixed& cursorCanvasVector() const;
	BoSelection* selection() const { return mSelection; }
	BoGameCamera* camera() const;
	BoAutoGameCamera* autoCamera() const;
	void advanceCamera();

#warning FIXME: use BoUfo method
	void setFont(const BoFontInfo& font);

	void saveAsXML(QDomElement& root);
	void loadFromXML(const QDomElement& root);

	/**
	 * Scroll by a certain distance.
	 *
	 * Note that these are pixel values, so depending on the current zoom
	 * factor this may be a long or a short distance in world-coordinates
	 **/
	void scrollBy(int x, int y);//AB: kind of obsolete, since we don't support QCanvas anymore

	void zoom(float delta);
	void rotate(float delta);

	void quitGame();

	void setToolTipCreator(int type);
	void setToolTipUpdatePeriod(int ms);

	void addChatMessage(const QString& message);

	/**
	 * Called once after starting a game, to initialize the items that are
	 * already in the game.
	 **/
	bool initializeItems();

	BoLight* light(int id) const;
	BoLight* newLight();
	void removeLight(int id);

	void addEffect(unsigned int id, const BoVector3Fixed& pos, bofixed zrot);
	void addEffectToUnit(int unitid, unsigned int effectid, BoVector3Fixed offset, bofixed zrot);
	void advanceEffects(int ticks);

	virtual void paint();
	virtual void paintWidget();

public slots:
	/**
	 * @param pos the <em>cell</em>-coordinates of the centered position
	 **/
	void slotReCenterDisplay(const QPoint& pos);
	void slotCenterHomeBase();
	void slotChangeCursor(int mode, const QString& dir);

	/**
	 * In case the unit has been destroyed make sure that it's removed from
	 * the local selection.
	 *
	 * Currently this does not do anything else, but we might add some
	 * functionality in the future
	 **/
	void slotUnitChanged(Unit* unit);

	void slotExplored(int x, int y);
	void slotUnexplored(int x, int y);
	void slotFog(int x, int y);
	void slotUnfog(int x, int y);

	void slotChangeTexMap(int x, int y);
	void slotChangeHeight(int x, int y);

	void slotAction(const BoSpecificAction&);
	void slotShowMiniMap(bool);

signals:
	/**
	 * Emitted when the selection for this big display has changed. See also
	 * @ref BoSelection::signalSelectionChanged
	 **/
	void signalSelectionChanged(BoSelection* selection);
	void signalCursorCanvasVectorChanged(const BoVector3Fixed&);

	void signalToggleChatVisible();
	void signalToggleStatusbar(bool);
	void signalSaveGame();
	void signalLoadGame();
	void signalQuicksaveGame();
	void signalQuickloadGame();
	void signalEndGame();
	void signalQuit();
	void signalEditorChangeLocalPlayer(Player*);
	void signalEditorHasUndo(const QString&);
	void signalEditorHasRedo(const QString&);

	/**
	 * See @ref BosonCursorCollection::signalSetWidgetCursor
	 **/
	void signalSetWidgetCursor(BosonCursor* c);

protected:
	/**
	 * Update the @ref cursorCanvasVector according to the current cursor and
	 * camera settings.
	 *
	 * This should be called whenever the mouse is moved (i.e. when a mouse
	 * move event occurs) and whenever the camera is changed.
	 *
	 * @param cursorGameViewPos The current position of the cursor in the
	 * gameView. This parameter is supposed to be in a X11-like coordinate
	 * system, i.e. (0,0) specifies the top-left corner of the widget.
	 **/
	void updateCursorCanvasVector(const QPoint& cursorGameViewPos);

	void setCamera(const BoGameCamera& c);
	void cameraChanged();

	/*
	 * @param useRealDepth If TRUE this function will calculate the real
	 * coordinates at @p pos, if FALSE it will calculate the coordinate at
	 * @p pos with z=0.0. This is useful for e.g. @ref mapDistance, where
	 * different z values could deliver wrong values.
	 **/
	bool mapCoordinatesToGround(const QPoint& pos, GLfloat* posX, GLfloat* posY, GLfloat* posZ, bool useRealDepth = true) const;
	bool mapDistance(int windowDistanceX, int windowDistanceY, GLfloat* dx, GLfloat* dy) const;

	/**
	 * Set a viewport. Basically the same as glViewport, but you should use
	 * this instead the standard OpenGL function, so that we can easily keep
	 * track of the viewport.
	 *
	 * Note that the current matrix is not changed before glViewport is
	 * calles, so you need to ensure that it is called at the correct
	 * time/place.
	 **/
	void setViewport(int x, int y, GLsizei w, GLsizei h);

	void resetGameMode();
	void setGameMode(bool);

	/**
	 * Move the selection rect. @ref selectionStart is still the start point
	 * but @ref selectionEnd is now x,y,z
	 **/
	void moveSelectionRect(const QPoint& widgetPos);

	/**
	 * Here the defined action for a wheel event should happen. See
	 * docs/mouse-big_display.txt for a list of actions that are allowed
	 * here.
	 * @param delta See QWheelEvent::delta. This is how much the wheel was
	 * moved.
	 * @param orientation Guess what? Yes! Horizontal or Vertical wheel.
	 * @param action Information about the event (position, modifiers, ...).
	 * See @ref BoMouseEvent
	 **/
	void mouseEventWheel(float delta, Qt::Orientation orientation, const BoMouseEvent& action);

	/**
	 * @param buttonState See @ref QMouseEvent::state. This tells you which
	 * buttons are currently pressed.
	 * @param action Information about the event (position, modifiers, ...).
	 * See @ref BoMouseEvent
	 **/
	void mouseEventMove(int buttonState, const BoMouseEvent& action);

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
	 * See @ref BoMouseEvent
	 * @param stream Stream your action here, if it is network (i.e.
	 * game-)relevant. Actions like zooming, rotating (i.e. chaging the
	 * camera) or selecting units can be done immediately, but all that
	 * requires units to do something must be streamed and sent using a
	 * message.
	 * @param send Set this to TRUE in order to actually send the @p stream
	 **/
	void mouseEventRelease(ButtonState button, const BoMouseEvent& action);

	/**
	 * @param button Which button produced this event.
	 * @param action Information about the event (position, modifiers, ...).
	 * See @ref BoMouseEvent
	 * @param stream Stream your action here, if it is network (i.e.
	 * game-)relevant. Actions like zooming, rotating (i.e. chaging the
	 * camera) or selecting units can be done immediately, but all that
	 * requires units to do something must be streamed and sent using a
	 * message.
	 * @param send Set this to TRUE in order to actually send the @p stream
	 **/
	void mouseEventReleaseDouble(ButtonState button, const BoMouseEvent& action);

	void resetGameViewPlugin();
	void resetGameViewPlugin(bool gameMode);


protected slots:
	void slotPlugLocalPlayerInput();
	void slotAddMenuInput();
	void slotResetViewProperties();

	/**
	 * Called when @ref BosonCanvas::signalRemovedItem is emitted. Note that
	 * this usally happens from the @ref BosonItem destructor! So be careful
	 * with calling function of @p item, they might crash the game (as they
	 * dont exist anymore / their data doesnt exist anymore)
	 **/
	void slotRemovedItemFromCanvas(BosonItem* item);

	void slotAdvance(unsigned int, bool);

	void slotInitMiniMapFogOfWar();

	void slotReloadGameViewPlugin();

	void slotEditorDeleteSelectedUnits();
	void slotEditorEditHeight(bool);
	void slotEditorShowPlaceFacilities();
	void slotEditorShowPlaceMobiles();
	void slotEditorShowPlaceGround();
	void slotEditorUndo();
	void slotEditorRedo();


	void slotShowLight0Widget();

	void slotScroll(int);
	void slotCenterOnSelectionGroup(int);

	void slotWidgetResized();

	void slotMouseEvent(QMouseEvent* e);
	void slotWheelEvent(QWheelEvent* e);

	void slotWidgetShown();
	void slotWidgetHidden();

	void slotGameOver();

private:
	void init();
	void initUfoGUI();

private:
	BosonGameViewPrivate* d;

	BosonCanvas* mCanvas;
	BosonCursor* mCursor;
	BoSelection* mSelection;
};



/**
 * This class connects to the relevant signals of @ref BosonScriptInterface. All
 * communication between @ref BosonScript and @ref BosonGameView happens
 * trough the interface class and this class.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonGameViewScriptConnector : public QObject
{
	Q_OBJECT
public:
	BosonGameViewScriptConnector(BosonGameView* parent);
	~BosonGameViewScriptConnector();

	/**
	 * Make this object resond to the signals of @p script. Note that only
	 * one slot is allowed to connect to one signal in @p script, as the
	 * return values are retrieved through the slots.
	 **/
	void connectToScript(BosonScript* script);


protected slots:
	/*  Light  */
	void slotAddLight(int* id);
	void slotRemoveLight(int id);
	void slotGetLightPos(int id, BoVector4Float*);
	void slotGetLightAmbient(int id, BoVector4Float*);
	void slotGetLightDiffuse(int id, BoVector4Float*);
	void slotGetLightSpecular(int id, BoVector4Float*);
	void slotGetLightAttenuation(int id, BoVector3Float*);
	void slotGetLightEnabled(int id, bool*);
	void slotSetLightPos(int id, const BoVector4Float&);
	void slotSetLightAmbient(int id, const BoVector4Float&);
	void slotSetLightDiffuse(int id, const BoVector4Float&);
	void slotSetLightSpecular(int id, const BoVector4Float&);
	void slotSetLightAttenuation(int id, const BoVector3Float&);
	void slotSetLightEnabled(int id, bool);

	/*  Camera  */
	void slotGetCameraPos(BoVector3Float*);
	void slotGetCameraLookAt(BoVector3Float*);
	void slotGetCameraUp(BoVector3Float*);
	void slotGetCameraRotation(float*);
	void slotGetCameraXRotation(float*);
	void slotGetCameraDistance(float*);
	void slotSetUseCameraLimits(bool);
	void slotSetCameraFreeMovement(bool);

	/*  AutoCamera  */
	void slotSetCameraPos(const BoVector3Float&);
	void slotSetCameraLookAt(const BoVector3Float&);
	void slotSetCameraUp(const BoVector3Float&);
	void slotAddCameraPosPoint(const BoVector3Float&, float);
	void slotAddCameraLookAtPoint(const BoVector3Float&, float);
	void slotAddCameraUpPoint(const BoVector3Float&, float);
	void slotSetCameraRotation(float);
	void slotSetCameraXRotation(float);
	void slotSetCameraDistance(float);
	void slotSetCameraMoveMode(int);
	void slotSetCameraInterpolationMode(int);
	void slotCommitCameraChanges(int);
	void slotSetAcceptUserInput(bool);

	/*  Effects  */
	void slotAddEffect(unsigned int id, const BoVector3Fixed& pos, bofixed zrot);
	void slotAddEffectToUnit(int unitid, unsigned int effectid, BoVector3Fixed offset, bofixed zrot);
	void slotAdvanceEffects(int ticks);
	void slotSetWind(const BoVector3Float& wind);
	void slotGetWind(BoVector3Float* wind);

protected:
	void reconnect(const QObject*, const char*, const QObject*, const char*);

private:
	BosonGameView* mDisplay;
};


#endif

