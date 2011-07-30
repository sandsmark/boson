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
//Added by qt3to4:
#include <QMouseEvent>
#include <QEvent>
#include <QWheelEvent>

#include "bosonqtgameview.h"

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



class SelectionRectBoUfo : public QObject
{
	Q_OBJECT
public:
	SelectionRectBoUfo();
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
	void mouseEventRelease(Qt::ButtonState button, const BoMouseEvent& action);

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
	void mouseEventReleaseDouble(Qt::ButtonState button, const BoMouseEvent& action);

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
class BosonGameViewScriptConnectorBoUfo : public QObject
{
	Q_OBJECT
public:
	BosonGameViewScriptConnectorBoUfo(BosonGameView* parent);
	~BosonGameViewScriptConnectorBoUfo();

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
