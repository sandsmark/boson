/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosonufoglwidget.h"

class BosonCanvas;
class BosonCursor;
class BoSelection;
class Player;
class PlayerIO;
class Unit;
class UnitProperties;
class BoGameCamera;
class BoAutoGameCamera;
class BosonBigDisplayInputBase;
class BoItemList;
class BosonItem;
class BoPixmapRenderer;
class BoLight;
class BoFontInfo;
class BosonScript;
class BoVisibleEffects;
class BosonMap;
class BosonEffect;
class BoSpecificAction;
class BoGLMatrices;

class KGameChat;
class KGameIO;
class QDomElement;
template<class T> class QPtrList;

class BosonCanvasRendererPrivate;
/**
 * @short This class renders everything that is actually "part of the game".
 *
 * The canvas renderer basically renders the canvas. That means it renders the
 * ground and everything on it (items/units, effects, ...), i.e. everything that
 * is "part of the game".
 *
 * It does <em>not</em> render anything that is used to control the game -
 * cmdframe, labels, cursor, ... These things are handled elsewhere.
 *
 * @author Andreas Beckermann <b_mann@gmx.de
 **/
class BosonCanvasRenderer
{
public:
	BosonCanvasRenderer(const BoGLMatrices& gameMatrices);
	~BosonCanvasRenderer();

	void setCamera(BoGameCamera* camera);
	void setLocalPlayerIO(PlayerIO* io);

	void setParticlesDirty(bool dirty);

	void reset();
	void initGL();
	void paintGL(const BosonCanvas* canvas);
	unsigned int renderedItems() const;
	unsigned int renderedCells() const;
	unsigned int renderedParticles() const;
	int textureBindsCells() const;
	int textureBindsItems() const;
	int textureBindsWater() const;
	int textureBindsParticles() const;

	BoGameCamera* camera() const;
	PlayerIO* localPlayerIO() const;
	const GLfloat* viewFrustum() const;

protected:
	void renderGround(const BosonMap*);
	void renderItems(const BoItemList* allCanvasItems);
	void renderSelections(const BoItemList* selectedItems);
	void renderWater();
	void renderFog(BoVisibleEffects&);
	void renderParticles(BoVisibleEffects&);
	void renderBulletTrailEffects(BoVisibleEffects& visible);
	void renderFadeEffects(BoVisibleEffects& visible);
	void createRenderItemList(BoItemList* renderItemList, const BoItemList* allItems);
	void createSelectionsList(BoItemList* selections, const BoItemList* relevantItems);
	void createVisibleEffectsList(BoVisibleEffects*, const QPtrList<BosonEffect>& allEffects, unsigned int mapWidth, unsigned int mapHeight);

	void renderBoundingBox(const BosonItem* item);
	void renderBoundingBox(const BoVector3Float& c1, const BoVector3Float& c2);

private:
	BosonCanvasRendererPrivate* d;
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
	}

	~BoMouseEvent()
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
	void setCanvasVector(const BoVector3Fixed& pos)
	{
		mCanvasVector = pos;
		mCanvasPos = QPoint((int)pos.x(), (int)pos.y());
	}

	const QPoint& canvasPos() const
	{
		return mCanvasPos;
	}
	const BoVector3Fixed& canvasVector() const
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
	BoVector3Fixed mCanvasVector;
	GLfloat mX;
	GLfloat mY;
	GLfloat mZ;

	bool mControlButton;
	bool mShiftButton;
	bool mAltButton;
};

class BoLineVisualization
{
public:
	BoLineVisualization()
	{
		color.set(1.0f, 1.0f, 1.0f, 1.0f);
		timeout = 60;
		pointsize = 1.0f;
	}

	QValueList<BoVector3Fixed> points;
	BoVector4Float color;
	int timeout;
	float pointsize;
};

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


/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonBigDisplayBase : public BosonUfoGLWidget
{
	Q_OBJECT
public:
	BosonBigDisplayBase(QWidget* parent);
	virtual ~BosonBigDisplayBase();

	void setCanvas(BosonCanvas* canvas);

	void setLocalPlayerScript(BosonScript* script);
	void setDisplayInput(BosonBigDisplayInputBase* input);

	void setLocalPlayerIO(PlayerIO* p);

	const BosonCanvas* canvas() const { return mCanvas; }

	void addLineVisualization(BoLineVisualization v);
	void advanceLineVisualization();

	BosonCursor* cursor() const;
	BoSelection* selection() const { return mSelection; }

	void setFont(const BoFontInfo& font);
	QImage screenShot();

	/**
	 * Final cleanups. This should remove basically all game relevant data
	 * from this class, especially pointers to cells or to units.
	 *
	 * I am not yet sure whether the localPlayer should get reset here, too
	 * - probably it should.
	 **/
	void quitGame();

	bool isInputInitialized();
	void setInputInitialized(bool initialized);

	void addChatMessage(const QString& message);

	void setUpdateInterval(unsigned int ms);

	/**
	 * Scroll by a certain distance.
	 *
	 * Note that these are pixel values, so depending on the current zoom
	 * factor this may be a long or a short distance in world-coordinates
	 **/
	void scrollBy(int x, int y);//AB: kind of obsolete, since we don't support QCanvas anymore

	void zoom(float delta);
	void rotate(float delta);


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

	double fps() const;

	PlayerIO* localPlayerIO() const;

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
	void setPlacementCellPreviewData(unsigned int textureCount, unsigned char* alpha, bool canPlace);

	void setToolTipCreator(int type);
	void setToolTipUpdatePeriod(int ms);

	void setParticlesDirty(bool dirty);

	const BoVector3Fixed& cursorCanvasVector() const;
	BosonBigDisplayInputBase* displayInput() const;

	void saveAsXML(QDomElement& root);
	void loadFromXML(const QDomElement& root);

	BoGameCamera* camera() const;
	BoAutoGameCamera* autoCamera() const;
	void advanceCamera();

	BoLight* light(int id) const;
	BoLight* newLight();
	void removeLight(int id);

	void updateOpenGLSettings();

	/**
	 * Grab a frame for a movie. The returned @ref QByteArray contains
	 * everything that is necessary to display one frame. At the moment that
	 * is the whole screenshot, later we may use the positions of the units
	 * only or something similar.
	 **/
	QByteArray grabMovieFrame();

	/**
	 * Generate a movie frame for every @ref QByteArray object in @ref data.
	 * The frames will be placed into @p dir. You should be able to create a
	 * movie from this using something like
	 * <pre>
	 * 'mencoder -mf on:fps=20 -ovc lavc -lavcopts
	 *    vcodec=mpeg4:vbitrate=2000
	 *    -o boson-movie.avi boson-movie-\*.jpg'
	 * </pre>
	 **/
	void generateMovieFrames(const QValueList<QByteArray>& data, const QString& dir);

public slots:
	void slotAction(const BoSpecificAction&);

	/**
	 * @param pos the <em>cell</em>-coordinates of the centered position
	 **/
	void slotReCenterDisplay(const QPoint& pos);
	void slotCenterHomeBase();

	/**
	 * In case the unit has been destroyed make sure that it's removed from
	 * the local selection.
	 *
	 * Currently this does not do anything else, but we might add some
	 * functionality in the future
	 **/
	void slotUnitChanged(Unit* unit);

	void slotFog(int x, int y);
	void slotUnfog(int x, int y);

signals:
	void signalMakeActive(BosonBigDisplayBase*);

	void signalChangeViewport(BosonBigDisplayBase* display, const QPoint& topLeft, const QPoint& topRight, const QPoint& bottomLeft, const QPoint& bottomRight);

	/**
	 * Emitted when the selection for this big display has changed. See also
	 * @ref BoSelection::signalSelectionChanged
	 **/
	void signalSelectionChanged(BoSelection* selection);

	void signalToggleChatVisible();
	void signalToggleStatusbar(bool);
	void signalSaveGame();
	void signalEndGame();
	void signalQuit();
	void signalEditorChangeLocalPlayer(Player*);

protected slots:
	void slotMouseEvent(KGameIO* , QDataStream& stream, QMouseEvent* e, bool *eatevent);

	/**
	 * Called when @ref BosonCanvas::signalRemovedItem is emitted. Note that
	 * this usally happens from the @ref BosonItem destructor! So be careful
	 * with calling function of @p item, they might crash the game (as they
	 * dont exist anymore / their data doesnt exist anymore)
	 **/
	void slotRemovedItemFromCanvas(BosonItem* item);

	void slotMouseIODestroyed();

	void slotInitMiniMapFogOfWar();

	void slotAdvance(unsigned int, bool);

	void slotAddLineVisualization(const QValueList<BoVector3Fixed>& points, const BoVector4Float& color, bofixed pointSize, int timeout, bofixed zOffset);
	void slotPreferencesApply();
	void slotUpdateOpenGLSettings();
	void slotChangeCursor(int, const QString&);

protected:
	/**
	 * Here the defined action for a wheel event should happen. See
	 * docs/mouse-big_display.txt for a list of actions that are allowed
	 * here.
	 * @param delta See QWheelEvent::delta. This is how much the wheel was
	 * moved.
	 * @param orientation Guess what? Yes! Horizontal or Vertical wheel.
	 * @param action Information about the event (position, modifiers, ...).
	 * See @ref BoMouseEvent
	 * @param stream You won't need this here.
	 * @param send You won't need this here.
	 **/
	void mouseEventWheel(float delta, Orientation orientation, const BoMouseEvent& action, QDataStream& stream, bool* send);

	/**
	 * @param buttonState See @ref QMouseEvent::state. This tells you which
	 * buttons are currently pressed.
	 * @param action Information about the event (position, modifiers, ...).
	 * See @ref BoMouseEvent
	 * @param stream You won't need this here.
	 * @param send You won't need this here.
	 **/
	void mouseEventMove(int buttonState, const BoMouseEvent& action, QDataStream& stream, bool* send);

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
	void mouseEventRelease(ButtonState button, const BoMouseEvent& action, QDataStream& stream, bool* send);

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
	void mouseEventReleaseDouble(ButtonState button, const BoMouseEvent& action, QDataStream& stream, bool* send);

protected slots:
	void slotScroll(int);

	void slotResetViewProperties();
	void slotShowLight0Widget();
	void slotEditorDeleteSelectedUnits();
	void slotEditorEditHeight(bool);
	void slotEditorShowPlaceFacilities();
	void slotEditorShowPlaceMobiles();
	void slotEditorShowPlaceGround();

protected:
	virtual void initializeGL();
	virtual void resizeGL(int w, int h);
	virtual void paintGL();

	virtual void showEvent(QShowEvent* e);

	void createRenderItemList();

	void renderUfo();
	/**
	 * @param isFlying If TRUE, the provided z value is always used, which
	 * is expected to be higher than any cells. Otherwise the z value of
	 * every cell is used.
	 **/
	void renderPathLines(QValueList<QPoint>& path, bool isFlying = false, float z = 0.05f);
	void renderCursor();
	void renderPlacementPreview();
	/**
	 * Called by @ref paintGL only to render text on the screen
	 **/
	void renderText();

	virtual void enterEvent(QEvent*);
	virtual void leaveEvent(QEvent*);
	virtual bool eventFilter(QObject* o, QEvent* e);

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
	 * Update the @ref cursorCanvasVector according to the current cursor and
	 * camera settings.
	 *
	 * This should be called whenever the mouse is moved (i.e. when a mouse
	 * move event occurs) and whenever the camera is changed.
	 **/
	void updateCursorCanvasVector();

	/**
	 * See @ref extractFrustum for more information about this stuff.
	 *
	 * We use a bounding spere so that we can easily rotate it.
	 * @return 0 if the object is not in the frustum (i.e. is not visible)
	 * otherwise the distance from the near plane. We might use this for the
	 * level of detail.
	 **/
	float sphereInFrustum(const BoVector3Fixed& pos, float radius) const;
	inline float sphereInFrustum(float x, float y, float z, float radius) const
	{
		BoVector3Fixed pos(x,y,z);
		return sphereInFrustum(pos, radius);
	}

	void setCamera(const BoGameCamera& c);
	void cameraChanged();

	void grabMovieFrameAndSave();

	/**
	 * Move the selection rect. @ref selectionStart is still the start point
	 * but @ref selectionEnd is now x,y,z
	 **/
	void moveSelectionRect(const QPoint& widgetPos);

	/**
	 * Remove a currently drawn selection rect and select all units inside
	 * this rect.
	 * @param replace If TRUE the current selection is replaced, otherwise
	 * the selected units are added to the selection.
	 * Usually when the player holds the shift key down while selecting.
	 **/
	void removeSelectionRect(bool replace);

	BoItemList* selectionRectItems();

	void addMouseIO(PlayerIO* playerIO);

	void generateMovieFrame(const QByteArray& data, BoPixmapRenderer* renderer);
	void resetGameMode();
	void setGameMode(bool);

private:
	void init();
	void initUfoGUI();
	void initUfoActions(bool gameMode);
	void initUfoGameActions();
	void initUfoEditorActions();

private:
	class BosonBigDisplayBasePrivate;
	BosonBigDisplayBasePrivate* d;

	BosonCanvas* mCanvas;
	BosonCursor* mCursor;
	BoSelection* mSelection;
};


/**
 * This class connects to the relevant signals of @ref BosonScriptInterface. All
 * communication between @ref BosonScript and @ref BosonBigDisplayBase happens
 * trough the interface class and this class.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonBigDisplayScriptConnector : public QObject
{
	Q_OBJECT
public:
	BosonBigDisplayScriptConnector(BosonBigDisplayBase* parent);
	~BosonBigDisplayScriptConnector();

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
	void slotGetCameraRadius(float*);
	void slotGetCameraZ(float*);
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
	void slotSetCameraRadius(float);
	void slotSetCameraZ(float);
	void slotSetCameraMoveMode(int);
	void slotSetCameraInterpolationMode(int);
	void slotCommitCameraChanges(int);
	void slotSetAcceptUserInput(bool);

protected:
	void reconnect(const QObject*, const char*, const QObject*, const char*);

private:
	BosonBigDisplayBase* mDisplay;
};

#endif

