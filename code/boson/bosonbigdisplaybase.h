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

#include "bosonglwidget.h"

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

class KGameChat;
class KGameIO;
class QDomElement;
template<class T> class QPtrList;

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

class BoLineVisualization
{
  public:
    BoLineVisualization()  { color.set(1.0f, 1.0f, 1.0f, 1.0f); timeout = 60; pointsize = 1.0f; }
    QValueList<BoVector3> points;
    BoVector4 color;
    int timeout;
    float pointsize;
};

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonBigDisplayBase : public BosonGLWidget
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

	BosonCursor* cursor() const { return mCursor; }
	void setCursor(BosonCursor* c) { mCursor = c; }
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

	void setKGameChat(KGameChat* chat);
	void addChatMessage(const QString& message);

	void setUpdateInterval(unsigned int ms);

	/**
	 * Scroll by a certain distance.
	 *
	 * Note that these are pixel values, so depending on the current zoom
	 * factor this may be a long or a short distance in world-coordinates
	 **/
	void scrollBy(int x, int y);//AB: kind of obsolete, since we don't support QCanvas anymore

	void rotateLeft(float factor = 5);
	void rotateRight(float factor = 5);
	void zoomIn(float factor = 5);
	void zoomOut(float factor = 5);

	void zoom(float delta);
	void rotate(float delta);


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

	const QPoint& cursorCanvasPos() const; // obsolete!
	const BoVector3& cursorCanvasVector() const;
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
	void changeGroundRenderer(int renderer);

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

	void signalChangeViewport(BosonBigDisplayBase* display, const QPoint& topLeft, const QPoint& topRight, const QPoint& bottomLeft, const QPoint& bottomRight);

	/**
	 * Emitted when the selection for this big display has changed. See also
	 * @ref BoSelection::signalSelectionChanged
	 **/
	void signalSelectionChanged(BoSelection* selection);

protected slots:
	void slotMouseEvent(KGameIO* , QDataStream& stream, QMouseEvent* e, bool *eatevent);
	void slotCursorEdgeTimeout();

	/**
	 * Called when @ref BosonCanvas::signalRemovedItem is emitted. Note that
	 * this usally happens from the @ref BosonItem destructor! So be careful
	 * with calling function of @p item, they might crash the game (as they
	 * dont exist anymore / their data doesnt exist anymore)
	 **/
	void slotRemovedItemFromCanvas(BosonItem* item);

	void slotMouseIODestroyed();

	void slotInitMiniMapFogOfWar();

	void slotAddLineVisualization(const QValueList<BoVector3>& points, const BoVector4& color, float pointSize, int timeout, float zOffset);

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

protected:
	virtual void initializeGL();
	virtual void resizeGL(int w, int h);
	virtual void paintGL();

	virtual void showEvent(QShowEvent* e);

	void createRenderItemList();

	void renderItems();
	/**
	 * @param isFlying If TRUE, the provided z value is always used, which
	 * is expected to be higher than any cells. Otherwise the z value of
	 * every cell is used.
	 **/
	void renderPathLines(QValueList<QPoint>& path, bool isFlying = false, float z = 0.05f);
	void renderCursor();
	void renderSelectionRect();
	void renderPlacementPreview();
	/**
	 * Called by @ref paintGL only to render text on the screen
	 **/
	void renderText();

	/**
	 * Part of @ref renderText.
	 * This just renders the chat messages.
	 **/
	void renderTextChat(int x, int y);

	int renderTextMapCoordinates(int x, int y, int w, int border);
	int renderTextPFData(int x, int y, int w, int border);
	int renderTextOpenGLMatrices(int x, int y);
	int renderTextItemWorkStatistics(int x, int y);
	int renderTextOpenGLCamera(int x, int y);
	int renderTextRenderCounts(int x, int y);
	int renderTextAdvanceCalls(int x, int y);
	void renderTextGamePaused();

	void renderMiniMap();

	/**
	 * Called by @ref paintGL only to render the cells on the screen
	 **/
	void renderCells();

	/**
	 * Called by @ref paintGL only to render the particle effects on the screen
	 **/
	void renderParticles();

	/**
	 * Called by @ref paintGL only to render the fog effects on the screen
	 **/
	void renderFog();

	void renderBulletTrailEffects();

	/**
	 * Called by @ref paintGL only to render the fade effects on the screen
	 **/
	void renderFadeEffects();

	/**
	 * @param y The <em>top</em> of the text to-be rendered. See also @ref
	 * BosonGLFont::renderText
	 * @return The height of the rendered text (see @ref
	 * BosonGLFont::renderText)
	 **/
	int renderMatrix(int x, int y, const BoMatrix* matrix, const QString& text);

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

	void setCamera(const BoGameCamera& c);
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

	void addMouseIO(PlayerIO* playerIO);

	void generateMovieFrame(const QByteArray& data, BoPixmapRenderer* renderer);
private:
	void init();

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
	void slotGetLightPos(int id, BoVector4*);
	void slotGetLightAmbient(int id, BoVector4*);
	void slotGetLightDiffuse(int id, BoVector4*);
	void slotGetLightSpecular(int id, BoVector4*);
	void slotGetLightAttenuation(int id, BoVector3*);
	void slotGetLightEnabled(int id, bool*);
	void slotSetLightPos(int id, const BoVector4&);
	void slotSetLightAmbient(int id, const BoVector4&);
	void slotSetLightDiffuse(int id, const BoVector4&);
	void slotSetLightSpecular(int id, const BoVector4&);
	void slotSetLightAttenuation(int id, const BoVector3&);
	void slotSetLightEnabled(int id, bool);

	/*  Camera  */
	void slotGetCameraPos(BoVector3*);
	void slotGetCameraLookAt(BoVector3*);
	void slotGetCameraUp(BoVector3*);
	void slotGetCameraRotation(float*);
	void slotGetCameraRadius(float*);
	void slotGetCameraZ(float*);
	void slotSetUseCameraLimits(bool);
	void slotSetCameraFreeMovement(bool);

	/*  AutoCamera  */
	void slotSetCameraPos(const BoVector3&);
	void slotSetCameraLookAt(const BoVector3&);
	void slotSetCameraUp(const BoVector3&);
	void slotSetCameraRotation(float);
	void slotSetCameraRadius(float);
	void slotSetCameraZ(float);
	void slotSetCameraMoveMode(int);
	void slotCommitCameraChanges(int);

protected:
	void reconnect(const QObject*, const char*, const QObject*, const char*);

private:
	BosonBigDisplayBase* mDisplay;
};

#endif

