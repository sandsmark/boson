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

#include "bosonbigdisplaybase.h"
#include "bosonbigdisplaybase.moc"

#include "defines.h"
#include "bosonwidget.h"
#include "bosontiles.h"
#include "bosoncanvas.h"
#include "bosonmap.h"
#include "cell.h"
#include "boitemlist.h"
#include "rtti.h"
#include "unit.h"
#include "unitproperties.h"
#include "speciestheme.h"
#include "player.h"
#include "bosoncursor.h"
#include "boselection.h"
#include "bosonconfig.h"
#include "selectbox.h"
#include "bosonglchat.h"
#include "bosonprofiling.h"
#include "bosonparticlesystem.h"
#include "boson.h"
#include "bodebug.h"
#include "items/bosonshot.h"
#include "unitplugins.h"
#include "bosonmodel.h"
#include "bo3dtools.h"
#include "bosonbigdisplayinputbase.h"
#include "bogltooltip.h"
#include "info/boinfo.h"

#include <kgame/kgameio.h>

#include <klocale.h>
#include <kmessagebox.h>

#include <qtimer.h>
#include <qcursor.h>
#include <qpointarray.h>

#if HAVE_SYS_TIME_H
#include <sys/time.h>
#else
// won't compile anymore!
#warning You dont have sys/time.h - please report this problem to boson-devel@lists.sourceforge.net and provide us with information about your system!
#endif
//#include <iostream.h>
#include <math.h>


#include "bosontexturearray.h"
#include "bosonglfont.h"

// both must be > 0.0:
#define NEAR 1.0f // FIXME: should be > 1.0
#define FAR 100.0f

// Camera limits
#define CAMERA_MIN_Z NEAR + 3
#define CAMERA_MAX_Z FAR - 50
#define CAMERA_MAX_RADIUS 80

#define CLEAR_DEPTH 1.0f

#warning move to class !
static float lightPos[] = {-6000.0, 3000.0, 10000.0, 1.0};

#include <GL/glu.h>

float textureUpperLeft[2] = { 0.0, 1.0 };
float textureLowerLeft[2] = { 0.0, 0.0 };
float textureLowerRight[2] = { 1.0, 0.0 };
float textureUpperRight[2] = { 1.0, 1.0 };

// Maybe camera class should be put to it's own file
class Camera
{
public:
	Camera()
	{
		init();
	}
	// AB: IMHO its a bad idea to place the map width/height into camera
	// code
	Camera(GLfloat mapWidth, GLfloat mapHeight)
	{
		init();
		mMapWidth = mapWidth;
		mMapHeight = mapHeight;
	}
	Camera(const Camera& c)
	{
		*this = c;
	}
	Camera& operator=(const Camera& c)
	{
		mLookAt = c.mLookAt;
		mPosZ = c.mPosZ;
		mRotation = c.mRotation;
		mRadius = c.mRadius;
		mMapWidth = c.mMapWidth;
		mMapHeight = c.mMapHeight;
		return *this;
	}
	void init()
	{
		mLookAt.set(0, 0, 0);
		mPosZ = 8.0;
		mRotation = 0.0;
		mRadius = 5.0;
		mMapWidth = 0.0;
		mMapHeight = 0.0;
	}

	void setLookAt(const BoVector3& pos)
	{
		mLookAt = pos;
		checkPosition();
	}
	const BoVector3& lookAt()
	{
		return mLookAt;
	}
	void changeZ(GLfloat diff)
	{
		float newz = mPosZ + diff;
		if (newz < CAMERA_MIN_Z) {
			newz = CAMERA_MIN_Z;
		} else if (newz > CAMERA_MAX_Z) {
			newz = CAMERA_MAX_Z;
		}
		float factor = newz / mPosZ;
		mPosZ = newz;
		mRadius = mRadius * factor;
	}
	void changeRadius(GLfloat diff)
	{
		float radius = mRadius + mPosZ / CAMERA_MAX_RADIUS * diff;  // How much radius is changed depends on z position
		if (radius < 0.0) {
			radius = 0.0;
		} else if (radius > mPosZ) {
			radius = mPosZ;
		}
		mRadius = radius;
	}
	void changeRotation(GLfloat diff)
	{
		float rotation = mRotation + diff;
		if (rotation < 0.0) {
			rotation += 360.0;
		} else if (rotation > 360.0) {
			rotation -= 360.0;
		}
		mRotation = rotation;
	}
	void moveLookAtBy(GLfloat x, GLfloat y, GLfloat z)
	{
		mLookAt.add(BoVector3(x, y, z));
		checkPosition();
	}
	void setMapSize(GLfloat w, GLfloat h)
	{
		mMapWidth = w;
		mMapHeight = h;
	}

	void loadFromXML(const QDomElement& root)
	{
		bool ok;
		float lookatx, lookaty, lookatz;
		lookatx = root.attribute("LookAtX").toFloat(&ok);
		if (!ok) {
			boError() << k_funcinfo << "Invalid value for LookAtX tag" << endl;
			lookatx = 0;
		}
		lookaty = root.attribute("LookAtY").toFloat(&ok);
		if (!ok) {
			boError() << k_funcinfo << "Invalid value for LookAtY tag" << endl;
			lookaty = 0;
		}
		lookatz = root.attribute("LookAtZ").toFloat(&ok);
		if (!ok) {
			boError() << k_funcinfo << "Invalid value for LookAtZ tag" << endl;
			mPosZ = 0;
		}
		mPosZ = root.attribute("PosZ").toFloat(&ok);
		if (!ok) {
			boError() << k_funcinfo << "Invalid value for PosZ tag" << endl;
			mPosZ = 0;
		}
		mRotation = root.attribute("Rotation").toFloat(&ok);
		if (!ok) {
			boError() << k_funcinfo << "Invalid value for Rotation tag" << endl;
			mRotation = 0;
		}
		mRadius = root.attribute("Radius").toFloat(&ok);
		if (!ok) {
			boError() << k_funcinfo << "Invalid value for Radius tag" << endl;
			mRadius = 0;
		}
		boDebug(260) << k_funcinfo << "Setting lookat to (" << lookatx << ", " << lookaty << ", " << lookatz << ")" << endl;
		mLookAt.set(lookatx, lookaty, lookatz);
		boDebug(260) << k_funcinfo << "lookat is now (" << mLookAt.x() << ", " << mLookAt.y() << ", " << mLookAt.z() << ")" << endl;
	}
	void saveAsXML(QDomElement& root)
	{
		root.setAttribute("LookAtX", mLookAt.x());
		root.setAttribute("LookAtY", mLookAt.y());
		root.setAttribute("LookAtZ", mLookAt.z());
		root.setAttribute("PosZ", mPosZ);
		root.setAttribute("Rotation", mRotation);
		root.setAttribute("Radius", mRadius);
	}

	void setZ(GLfloat z) { mPosZ = z; }
	void setRotation(GLfloat r) { mRotation = r; }
	void setRadius(GLfloat r) { mRadius = r; }
	GLfloat z() const { return mPosZ; }
	GLfloat rotation() const { return mRotation; }
	GLfloat radius() const { return mRadius; }

protected:
	void checkPosition()
	{
		if (!mMapWidth || !mMapHeight) {
			return;
		}
		if (mLookAt.x() < 0.0) {
			mLookAt.setX(0.0);
		} else if (mLookAt.x() > mMapWidth) {
			mLookAt.setX(mMapWidth);
		}
		if (mLookAt.y() > 0.0) {
			mLookAt.setY(0.0);
		} else if (mLookAt.y() < -mMapHeight) {
			mLookAt.setY(-mMapHeight);
		}
	}

private:
	BoVector3 mLookAt;
	GLfloat mPosZ;

	GLfloat mRotation;
	GLfloat mRadius;

	// AB: why float?
	GLfloat mMapWidth;
	GLfloat mMapHeight;
};

class SelectionRect
{
public:
	SelectionRect()
	{
		mVisible = false;
	}
	
	void widgetRect(QRect* rect) const
	{
		QRect r(mStartPos, mEndPos);
		*rect = r.normalize();
	}

	void setStartWidgetPos(const QPoint& pos)
	{
		mStartPos = pos;
		setEndWidgetPos(mStartPos);
	}
	
	void setEndWidgetPos(const QPoint& pos)
	{
		mEndPos = pos;
	}
	const QPoint& startPos() const
	{
		return mStartPos;
	}

	bool isVisible() const
	{
		return mVisible;
	}
	void setVisible(bool v)
	{
		mVisible = v;
	}
private:
	QPoint mStartPos;
	QPoint mEndPos;
	bool mVisible;
};

class MouseMoveDiff
{
public:
	enum Mode {
		ModeNone = 0,
		ModeRMBMove = 1,
		ModeRotate = 2,
		ModeZoom = 3
	} MouseMoveMode;
	
	MouseMoveDiff()
	{
		mMode = ModeNone;
		mX = 0;
		mY = 0;
		mOldX = 0;
		mOldY = 0;
	}

	inline void moveToPos(const QPoint& pos)
	{
		moveToPos(pos.x(), pos.y());
	}
	inline void moveToPos(int x, int y)
	{
		mOldX = mX;
		mOldY = mY;
		mX = x;
		mY = y;
	}

	void startRMBMove()
	{
		mMode = ModeRMBMove;
	}
	void startRotate()
	{
		mMode = ModeRotate;
	}
	void startZoom()
	{
		mMode = ModeZoom;
	}
	void stop()
	{
		mMode = ModeNone;
	}
	int x() const
	{
		return mX;
	}
	int y() const
	{
		return mY;
	}
	int oldX() const
	{
		return mOldX;
	}
	int oldY() const
	{
		return mOldY;
	}
	int dx() const
	{
		return (mX - mOldX);
	}
	int dy() const
	{
		return (mY - mOldY);
	}
	
	bool isStopped() const
	{
		return mode() == ModeNone;
	}
	void setMode(Mode mode)
	{
		mMode = mode;
	}
	Mode mode() const
	{
		return mMode;
	}

private:
	Mode mMode;
	int mX;
	int mY;
	int mOldX;
	int mOldY;
};


class BosonBigDisplayBase::BosonBigDisplayBasePrivate
{
public:
	BosonBigDisplayBasePrivate()
	{
		mLocalPlayer = 0;
		mChat = 0;
		mMouseIO = 0;

		mFrameCount = 0;
		mFps = 0;
		mFpsTime = 0;
		mDefaultFont = 0;

		mPlacementPreviewProperties = 0;
		mPlacementPreviewModel = 0;
		mPlacementPreviewCanPlace = false;

		mInput = 0;
		mToolTips = 0;

		mRenderCells = 0;
		mRenderCellsSize = 0;
		mRenderCellsCount = 0;
	}

	Player* mLocalPlayer;
	QTimer mCursorEdgeTimer;
	int mCursorEdgeCounter;
	KGameMouseIO* mMouseIO;

	BosonGLChat* mChat;

	Camera mCamera;

	GLint mViewport[4]; // x,y,w,h of the viewport. see setViewport
	BoMatrix mProjectionMatrix;
	BoMatrix mModelviewMatrix;
	GLdouble mFrustumMatrix[6][4];


	GLfloat mFovY; // see gluPerspective
	GLfloat mAspect; // see gluPerspective

	BosonGLFont* mDefaultFont;// AB: maybe we should support several fonts

	//AB: we should use a float* here which can be used as vertex array. we
	//should store x,y,z and texture x,y there. for this we need to have
	//cells in a single texture!
	//--> then we could also use GL_QUAD_STRIP
	Cell** mRenderCells;
	int mRenderCellsSize; // max. number of cells in the array
	int mRenderCellsCount; // actual number of cells in the array

	long long int mFpsTime;
	double mFps;
	unsigned int mFrameCount;

	SelectionRect mSelectionRect;
	MouseMoveDiff mMouseMoveDiff;

	QTimer mUpdateTimer;
	int mUpdateInterval;

	bool mIsQuit;

	QPoint mCanvasPos; // obsolete
	BoVector3 mCanvasVector;

	BoVector3 mCameraPos;

	BoParticleList mParticleList;
	bool mParticlesDirty;

	const UnitProperties* mPlacementPreviewProperties;
	BosonModel* mPlacementPreviewModel;
	bool mPlacementPreviewCanPlace;
	QPoint mPlacementCanvasPos;
	GLuint mCellPlacementTexture;

	BosonBigDisplayInputBase* mInput;
	BoGLToolTip* mToolTips;

	bool mDebugMapCoordinates;
	bool mDebugShowCellGrid;
	bool mDebugMatrices;
	bool mDebugItemWorks;
	bool mDebugCamera;
	bool mDebugRenderCounts;
	float mDebugMapCoordinatesX;
	float mDebugMapCoordinatesY;
	float mDebugMapCoordinatesZ;

	unsigned int mRenderedItems;  // units rendered when paintGL was last called
	unsigned int mRenderedCells;  // same, but for cells
};

BosonBigDisplayBase::BosonBigDisplayBase(BosonCanvas* c, QWidget* parent)
		: BosonGLWidget(parent)
{
 boDebug() << k_funcinfo << endl;
 mCanvas = c;
 init();
}

BosonBigDisplayBase::~BosonBigDisplayBase()
{
 boDebug() << k_funcinfo << endl;
 quitGame();
 delete mSelection;
 delete d->mChat;
// delete d->mUnitTips;
 delete d->mDefaultFont;
 delete[] d->mRenderCells;
 delete d->mToolTips;
 delete d;
 boDebug() << k_funcinfo << "done" << endl;
}

void BosonBigDisplayBase::init()
{
 d = new BosonBigDisplayBasePrivate;
 mCursor = 0;
 d->mCursorEdgeCounter = 0;
 d->mUpdateInterval = 0;
 d->mIsQuit = false;
 d->mParticlesDirty = true;
 d->mCellPlacementTexture = 0;
 d->mDebugMapCoordinates = false;
 d->mDebugShowCellGrid = false;
 d->mDebugMatrices = false;
 d->mDebugItemWorks = false;
 d->mDebugRenderCounts = false;
 d->mDebugCamera = false;
 d->mDebugMapCoordinatesX = 0.0f;
 d->mDebugMapCoordinatesY = 0.0f;
 d->mDebugMapCoordinatesZ = 0.0f;

 mSelection = new BoSelection(this);
 d->mChat = new BosonGLChat(this);
 d->mToolTips = new BoGLToolTip(this);

 for (int i = 0; i < 4; i++) {
	d->mViewport[i] = 0;
 }
 for (int i = 0; i < 6; i++) {
	for (int j = 0; j < 4; j++) {
		d->mFrustumMatrix[i][j] = 0.0;
	}
 }

 setUpdatesEnabled(false);
 slotResetViewProperties();

 if (!isValid()) {
	boError() << k_funcinfo << "No OpenGL support present on this system??" << endl;
	return;
 }

 // and another hack..
// setMinimumSize(QSize(400,400));

// initGL();// AB: initializeGL() is virtual
 connect(&d->mUpdateTimer, SIGNAL(timeout()), this, SLOT(slotUpdateGL()));

 connect(&d->mCursorEdgeTimer, SIGNAL(timeout()), 
		this, SLOT(slotCursorEdgeTimeout()));

 connect(boGame, SIGNAL(signalAdvance(unsigned int, bool)),
		this, SLOT(slotAdvance(unsigned int, bool)));
 connect(canvas(), SIGNAL(signalRemovedItem(BosonItem*)),
		this, SLOT(slotRemovedItemFromCanvas(BosonItem*)));

 //TODO: sprite tooltips

 setUpdateInterval(boConfig->updateInterval());
}


void BosonBigDisplayBase::initializeGL()
{
 if (isInitialized()) {
	// already called initializeGL()
	return;
 }
 if (!context()) {
	boError() << k_funcinfo << "NULL context" << endl;
	return;
 }
 // AB: we need at least GLU 1.3 for gluCheckExtension() !
 // TODO: find out if we might be able to run boson with older versions - if yes
 // use the code from http://www.mesa3d.org/brianp/sig97/exten.htm#Compile to
 // check for extensions
 // TODO: check for extensions on configure/compile time, too!
 /*
 const* string = (char*)glGetString(GL_EXTENSIONS);
 if (!gluCheckExtension("GL_EXT_texture_object", string)) { // introduced in OpenGL 1.1 as standard feature
	KMessageBox::sorry(this, i18n("Your OpenGL implementation seems not to support texture objects - don't even try to run boson without them!"));
	kapp->exit(1);
	return;
 }
 */
 glClearColor(0.0, 0.0, 0.0, 0.0);

#warning make configurable!
 // AB: GL_FLAT should be available for software rendering for example!
 glShadeModel(GL_SMOOTH); // GL_SMOOTH is default - but esp. in software rendering way slower. in hardware it *may* be equal (concerning speed) to GL_FLAT
 glDisable(GL_DITHER); // we don't need this (and its enabled by default)

 // for anti-aliased lines (currently unused):
 glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
 // for anti-aliased points (currently unused):
 glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST);
 // for anti-aliased polygons (currently unused):
 glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);

 glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);

#if 0

#warning FIXME: is extenstion there?
 // AB: if everything is inside the view volume we can use this to skip the
 // volume clipping tests. should be faster
 // FIXME: do we actually render stuff thats inside the view volume only?
 glHint(GL_CLIP_VOLUME_CLIPPING_HINT_EXT, GL_FASTEST);

#endif

 // AB: GL_MODULATE is the default and the models don't use it. unortunately the
 // light code seems to have problems with GL_REPLACE. GL_REPLACE would be
 // faster
// glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);


 float lightAmb[] = {0.8, 0.8, 0.8, 1.0};
 float lightDif[] = {1.0, 1.0, 1.0, 1.0};

 glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
 glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDif);
 glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

 // light makes things slower!
 glEnable(GL_LIGHT0);

 if (checkError()) {
	boError() << k_funcinfo << endl;
 }

 struct timeval time;
 gettimeofday(&time, 0);
 d->mFpsTime = time.tv_sec * 1000000 + time.tv_usec;

 if (!directRendering()) {
	// baad.
	boWarning() << k_funcinfo << "direct rendering has NOT been enabled!" << endl;
 }

 // this needs to be done in initializeGL():
 d->mDefaultFont = new BosonGLFont(QString::fromLatin1("fixed"));

 boDebug() << k_funcinfo << "starting timer" << endl;
 // start rendering (will also start the timer if necessary)
 QTimer::singleShot(d->mUpdateInterval, this, SLOT(slotUpdateGL()));

 // update system information (we initializeGL() must have been called before
 // this makes sense)
 BoInfo::boInfo()->update(this);
}

void BosonBigDisplayBase::resizeGL(int w, int h)
{
 boDebug() << k_funcinfo << w << " " << h << endl;
 setViewport(0, 0, (GLsizei)w, (GLsizei)h);
 glMatrixMode(GL_PROJECTION);
 glLoadIdentity();

 // IMO we don't need zooming. changing posY should be equally .. and even
 // better. no distortion
 GLfloat fovY = d->mFovY; // * d->mCamera.zoomFactor();
 d->mAspect = (float)w / (float)h;
 gluPerspective(fovY, d->mAspect, NEAR, FAR);

 // cache the composed projection matrix. we'll need it very often in
 // mapCoordinates()
 d->mProjectionMatrix.loadMatrix(GL_PROJECTION_MATRIX);
 extractFrustum(); // projection matrix changed
 generateCellList();
 glMatrixMode(GL_MODELVIEW);


 glClearDepth(CLEAR_DEPTH);
 glClear(GL_DEPTH_BUFFER_BIT);


 // update the minimap
 cameraChanged();

 if (checkError()) {
	boError() << k_funcinfo << endl;
 }
}

void BosonBigDisplayBase::paintGL()
{
 if (!isInitialized()) {
	initGL();
	return;
 }
 if (d->mIsQuit) {
	// we will get deleted soon
	return;
 }
 if (boGame->gameStatus() == KGame::Init) {
	return;
 }
 BO_CHECK_NULL_RET(localPlayer());
 BO_CHECK_NULL_RET(displayInput());

 d->mFrameCount++;
 calcFPS();
 if (boGame->delayedMessageCount() >= 10 && d->mFrameCount != 0) {
	// d->mFrameCount is set to 0 every second. so if we have >= 10 delayed
	// messages we'll render one frame per second.
	// now we need to reset the framecount, so that we won't display several
	// hundred fps although there is only a single frame rendered :)
	// We incremented by one above, so we decrement here.
	if (d->mFrameCount > 1) {
		d->mFrameCount--;
	}
	return;
 }
 boProfiling->render(true);
 d->mUpdateTimer.stop();
//boDebug() << k_funcinfo << endl;
 // AB: this is the most time-critical function! we need to enhance performance
 // whenever possible. look at
 // http://www.mesa3d.org/brianp/sig97/perfopt.htm
 // for perfomance optimizations

 // TODO: performance: make textures resident
 // maybe use priorities to achieve this
 // TODO: performance: maybe enable depth-buffer/depth-testing after the cells
 // have been drawn - so useless cell-drawings can be discarded. remember to
 // disable it before drawing the background (i.e. cells) !
 // TODO: performance: from the URL above:
 // Transparency may be implemented with stippling instead of blending 
 // If you need simple transparent objects consider using polygon stippling
 // instead of alpha blending. The later is typically faster and may actually
 // look better in some situations. [L,S] 
 // --> what is this "stippling" and can we use it?

 boProfiling->renderClear(true);
 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 boProfiling->renderClear(false);

 glColor3ub(255, 255, 255);

 // note: we don't call gluLookAt() here because of performance. instead we just
 // push the matrix here and pop it at the end of paintGL() again. gluLookAt()
 // is called only whenever cameraChanged() is called.
 glPushMatrix();

 if (checkError()) {
	boError() << k_funcinfo << "before unit rendering" << endl;
 }

 glEnable(GL_TEXTURE_2D);
 glEnable(GL_DEPTH_TEST);
 if (boConfig->useLight()) {
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);
 }

 boProfiling->renderUnits(true);
 BoItemList* allItems = mCanvas->allItems();
 BoItemList::Iterator it = allItems->begin();
 d->mRenderedItems = 0;

 // AB: these are problematic for triangle strips! they need to be in a special
 // format to make culling work!
 glEnable(GL_CULL_FACE);
 glCullFace(GL_BACK);

 glEnableClientState(GL_VERTEX_ARRAY);
 glEnableClientState(GL_TEXTURE_COORD_ARRAY);
 for (; it != allItems->end(); ++it) {
	BosonItem* item = *it;

	// FIXME: can't we use BoVector3 and it's conversion methods here?
	GLfloat x = (item->x() + item->width() / 2) * BO_GL_CELL_SIZE / BO_TILE_SIZE;
	GLfloat y = -((item->y() + item->height() / 2) * BO_GL_CELL_SIZE / BO_TILE_SIZE);
	GLfloat z = item->z(); // this is already in the correct format!

	// TODO: performance: we can improve this greatly:
	// simply group the items to bigger sphere or boxes. every box is of
	// size of (maybe) 10.0*10.0. We maintain a list of items for *every*
	// box. we can simply test if the box is in the frustum and if so we
	// test every item of that list. if not we can skip every item of that
	// box.
	// Especially in bigger games with big maps and several hundred units
	// this would be a great speedup.
	//
	// but note: do *not* use these lists for anything except OpenGL! we
	// mustn't use them for e.g. pathfinding. the bounding spheres of the
	// units depend e.g. on the .3ds files and are therefore UI only. If we
	// depend on it in pathfinding we'd soon have a broken network (think
	// about differnt CPUs with different rounding values for floating
	// point calculations)
	// UPDATE: we could instead use the "sectors" that we are planning to
	// use for collision detection and pathfinding also for the frustum
	// tests (they wouldn't do floating point calculations)
	if (!sphereInFrustum(x, y, z, item->boundingSphereRadius())) {
		// the unit is not visible, currently. no need to draw anything.
		continue;
	}

	// AB: note units are rendered in the *center* point of their
	// width/height.
	// but concerning z-position they are rendered from bottom to top!

	const QPtrVector<Cell>* cells = item->cells();
	bool visible = false;
	for (unsigned int i = 0; i < cells->count(); i++) {
		Cell* c = cells->at(i);
		if (!c) {
			boError() << k_funcinfo << i << " is no valid cell!" << endl;
			continue;
		}
		if (!localPlayer()->isFogged(c->x(), c->y())) {
			visible = true;
			// ugly but faster than placing this into the loop
			// condition
			break;
		}
	}
	if (!visible) {
		continue;
	}

	glTranslatef(x, y, z);
	glPushMatrix();
	glRotatef(-(item->rotation()), 0.0, 0.0, 1.0);
	glRotatef(item->xRotation(), 1.0, 0.0, 0.0);
	glRotatef(item->yRotation(), 0.0, 1.0, 0.0);

	// FIXME: performance: we could create a displaylist that contains the selectbox and simply change item->displayList()
	// when the item is selected/unselected
	// Units will be tinted accordingly to how much health they have left
	if (RTTI::isUnit(item->rtti())) {
		if (((Unit*)item)->isDestroyed()) {
			glColor3f(0.4f, 0.4f, 0.4f);
		} else {
			float f = ((Unit*)item)->health() / (float)((Unit*)item)->unitProperties()->health() * 0.3;
			glColor3f(0.7f + f, 0.7f + f, 0.7f + f);
		}
	} else {
		glColor3ub(255, 255, 255);
	}
	item->renderItem();
	glColor3ub(255, 255, 255);
	glPopMatrix();

	if (item->isSelected()) {
		// FIXME: performance: create a display lists in the SelectBox which also contains the scale!
		// FIXME: should selection boxes be drawn with lighting disabled?
		GLfloat w = ((float)item->width()) * BO_GL_CELL_SIZE / BO_TILE_SIZE;
		GLfloat h = ((float)item->height()) * BO_GL_CELL_SIZE / BO_TILE_SIZE;
		GLfloat depth = item->glDepthMultiplier();
		glPushMatrix();
		if (w != 1.0 || h != 1.0 || depth != 1.0) {
			glScalef(w, h, depth);
		}
		if (boConfig->alignSelectionBoxes()) {
			glRotatef(camera()->rotation(), 0.0, 0.0, 1.0);
		}
		glCallList(item->selectBox()->displayList());
		glPopMatrix();
	}

	glTranslatef(-x, -y, -z);
	d->mRenderedItems++;
 }
 glDisableClientState(GL_VERTEX_ARRAY);
 glDisableClientState(GL_TEXTURE_COORD_ARRAY);
 glDisable(GL_CULL_FACE);
 boProfiling->renderUnits(false, d->mRenderedItems);

 if (checkError()) {
	boError() << k_funcinfo << "after unit rendering" << endl;
 }

 boProfiling->renderCells(true);
 glEnable(GL_DEPTH_TEST);
 renderCells();
 boProfiling->renderCells(false);

 if (checkError()) {
	boError() << k_funcinfo << "cells rendered" << endl;
 }

 // Facility-placing preview code
 if (displayInput()->actionLocked() && displayInput()->actionType() == ActionBuild &&
		((d->mPlacementPreviewModel != 0 && d->mPlacementPreviewProperties) ||
		d->mCellPlacementTexture != 0)) {
	// AB: GL_MODULATE is currently default. if we every change it to
	// GL_REPLACE we should change it here:
//	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	GLubyte color;
	if (d->mPlacementPreviewCanPlace) {
		color = 255;
	} else {
		color = PLACEMENTPREVIEW_DISALLOW_COLOR;
	}
	glEnable(GL_BLEND);
	glColor4ub(255, color, color, PLACEMENTPREVIEW_ALPHA);

#warning FIXME: z value!
	bool modelPreview = false;
	bool cellPreview = false;
	if (d->mPlacementPreviewModel && d->mPlacementPreviewModel->frame(0)) {
		modelPreview = true;
	}
	if (d->mCellPlacementTexture != 0) {
		cellPreview = true;
	}
	const float z = 0.1;
	QPoint pos(d->mPlacementCanvasPos);
	int w = 0;
	int h = 0;
	if (modelPreview) {
		w = d->mPlacementPreviewProperties->unitWidth();
		h = d->mPlacementPreviewProperties->unitHeight();
	}
	float x = ((pos.x() + w / 2)) * BO_GL_CELL_SIZE;
	float y = ((pos.y() + h / 2)) * BO_GL_CELL_SIZE;
	if (pos.x() >= 0) {
		x = x - pos.x() % BO_TILE_SIZE;
	} else {
		x = x - (BO_TILE_SIZE + pos.x() % BO_TILE_SIZE);
	}
	if (pos.y() >= 0) {
		y = y - pos.y() % BO_TILE_SIZE;
	} else {
		y = y - (BO_TILE_SIZE + pos.y() % BO_TILE_SIZE);
	}
	x /= BO_TILE_SIZE;
	y /= BO_TILE_SIZE;
	glTranslatef(x, -y, z);
	if (modelPreview) {
		BoFrame* f = d->mPlacementPreviewModel->frame(0);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		d->mPlacementPreviewModel->enablePointer();
		f->renderFrame(&localPlayer()->teamColor());
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	} else if (cellPreview) {
		glBindTexture(GL_TEXTURE_2D, d->mCellPlacementTexture);
		glBegin(GL_QUADS);
			glTexCoord2fv(textureUpperLeft);
			glVertex3f(0.0f, 0.0f, 0.0f);

			glTexCoord2fv(textureLowerLeft);
			glVertex3f(0.0f, - BO_GL_CELL_SIZE, 0.0f);

			glTexCoord2fv(textureLowerRight);
			glVertex3f(BO_GL_CELL_SIZE, -BO_GL_CELL_SIZE, 0.0f);

			glTexCoord2fv(textureUpperRight);
			glVertex3f(BO_GL_CELL_SIZE, 0.0f, 0.0f);
		glEnd();
	}
	glTranslatef(-x, y, -z);
	glColor4ub(255, 255, 255, 255);
	glDisable(GL_BLEND);
	// AB: see above. if GL_REPLACES ever becomes default we have to set it
	// here again.
//	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
 }


 if (checkError()) {
	boError() << k_funcinfo << "preview rendered" << endl;
 }

 // Render particle systems
 boProfiling->renderParticles(true);
 renderParticles();
 boProfiling->renderParticles(false);

 if (checkError()) {
	boError() << k_funcinfo << "when particles rendered" << endl;
 }


 glDisable(GL_DEPTH_TEST);
 glDisable(GL_LIGHTING);
 glDisable(GL_NORMALIZE);

 boProfiling->renderText(true); // AB: actually this is text and cursor and selectionrect

 // cursor and text are drawn in a 2D-matrix, so that we can use window
 // coordinates
 glMatrixMode(GL_PROJECTION);
 glPushMatrix();
 glLoadIdentity();
 gluOrtho2D(0.0, (GLfloat)d->mViewport[2], 0.0, (GLfloat)d->mViewport[3]); // the same as the viewport
 glMatrixMode(GL_MODELVIEW);
 glPushMatrix();
 glLoadIdentity();

 // alpha blending is used for both, cursor and text
 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

 if (cursor()) {
	// FIXME: use cursorCanvasPos()
	QPoint pos = mapFromGlobal(QCursor::pos());
	GLfloat x = (GLfloat)pos.x();
	GLfloat y = (GLfloat)d->mViewport[3] - (GLfloat)pos.y();
	cursor()->renderCursor(x, y);
 }
 glDisable(GL_TEXTURE_2D);
 if (checkError()) {
	boError() << k_funcinfo << "GL error when cursor rendered" << endl;
 }
 renderText();

 if (d->mSelectionRect.isVisible()) {
	glPushMatrix();

	glColor3ub(255, 0, 0); // FIXME hardcoded
	
	QRect rect;
	d->mSelectionRect.widgetRect(&rect);

	int x = rect.left();
	int w = rect.width();
	int y = d->mViewport[3] - rect.top();
	int h = rect.height();

	glBegin(GL_LINE_LOOP);
		glVertex3f(x, y, 0.0f);
		glVertex3f(x + w, y, 0.0f);
		glVertex3f(x + w, y - h, 0.0f);
		glVertex3f(x, y - h, 0.0f);
	glEnd();

	glColor3ub(255, 255, 255);
	glPopMatrix();
 }

 // now restore the old 3D-matrix
 glMatrixMode(GL_PROJECTION);
 glPopMatrix();
 glMatrixMode(GL_MODELVIEW);
 glPopMatrix();
 boProfiling->renderText(false);

 if (checkError()) {
	boError() << k_funcinfo << "selection rect rendered" << endl;
 }

 glPopMatrix();

 if (d->mUpdateInterval) {
	d->mUpdateTimer.start(d->mUpdateInterval);
 }

 bool showProfilingMessage = boProfiling->renderEntries() < MAX_PROFILING_ENTRIES;
 boProfiling->render(false);
 if ( showProfilingMessage && boProfiling->renderEntries() >= MAX_PROFILING_ENTRIES) {
	boGame->slotAddChatSystemMessage(i18n("%1 frames have been recorded by boProfiling. You can make profiling snapshots using CTRL+P").arg(boProfiling->renderEntries()));
 }
}

void BosonBigDisplayBase::renderText()
{
 glListBase(d->mDefaultFont->displayList()); // AB: this is a redundant call, since we don't change it somewhere in paintGL(). but we might support different fonts one day and so we need it anyway.
 const int border = 5;
 const int alphaborder = 2;

// first the resource display
 // AB: we can avoid these calls to i18n() here! e.g. cache it somewhere and
 // update every 5 seconds or so (maybe less)
 // remember that painGL() is very speed sensitive!
 QString minerals = i18n("Minerals: %1").arg(localPlayer()->minerals());
 QString oil = i18n("Oil:      %1").arg(localPlayer()->oil());
 int w = QMAX(d->mDefaultFont->width(minerals), d->mDefaultFont->width(oil));
 int x = d->mViewport[2] - w - border;
 int y = d->mViewport[3] - d->mDefaultFont->height() - border;

 // Alpha-blended rectangle
 glEnable(GL_BLEND);
 glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
 glRecti(x - alphaborder, y + d->mDefaultFont->height() + alphaborder,
		x + w + alphaborder,
		y - d->mDefaultFont->height() - alphaborder); // y is at bottom of first line, subtract fontheight and we're at the bottom of 2nd line
 glColor3ub(255, 255, 255);
 glRasterPos2i(x, y);
 glCallLists(minerals.length(), GL_UNSIGNED_BYTE, (GLubyte*)minerals.latin1());
 y -= d->mDefaultFont->height();
 glRasterPos2i(x, y);
 glCallLists(oil.length(), GL_UNSIGNED_BYTE, (GLubyte*)oil.latin1());

 if (d->mDebugMapCoordinates) {
	QString s = QString::fromLatin1("World: (%1,%2,%2) Canvas: (%4,%5) Window: %6,%7").
			arg((double)d->mDebugMapCoordinatesX, 6, 'f', 3).
			arg((double)d->mDebugMapCoordinatesY, 6, 'f', 3).
			arg((double)d->mDebugMapCoordinatesZ, 6, 'f', 3).
			arg(d->mCanvasPos.x(), 4, 10).
			arg(d->mCanvasPos.y(), 4, 10).
			arg(mapFromGlobal(QCursor::pos()).x(), 4, 10).
			arg(mapFromGlobal(QCursor::pos()).y(), 4, 10);
	w = d->mDefaultFont->width(s);
	x = d->mViewport[2] - border - w;
	y -= d->mDefaultFont->height();
	y -= alphaborder * 2;

	glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
	glRecti(x - alphaborder, y + d->mDefaultFont->height() + alphaborder,
			x + w + alphaborder,
			y - alphaborder);
	glDisable(GL_BLEND);

	glColor3ub(255, 255, 255);
	glRasterPos2i(x, y);
	glCallLists(s.length(), GL_UNSIGNED_BYTE, (GLubyte*)s.latin1());
 }
 if (d->mDebugMatrices) {
	int x = border;
	int y = d->mViewport[3] - border;
	BoMatrix model(d->mModelviewMatrix);
	BoMatrix proj(d->mProjectionMatrix);
	renderMatrix(x, y, &model, i18n("Modelview matrix:"));
	y -= (5 * d->mDefaultFont->height() + 20);
	renderMatrix(x, y, &proj, i18n("Projection matrix:"));
	y -= (5 * d->mDefaultFont->height() + 20);

	proj.multiply(model.data());
	renderMatrix(x, y, &proj, i18n("Projection * Modelview:"));
	y -= (5 * d->mDefaultFont->height() + 20);

	BoMatrix inverse;
	// AB: we could calculate this for mapCoordinates whenever camera
	// changes!
	proj.invert(&inverse); // invert (proj*model)
	renderMatrix(x, y, &inverse, i18n("(Projection * Modelview)^(-1):"));
	y -= (5 * d->mDefaultFont->height() + 20);

	BoMatrix identity(inverse);
	identity.multiply(proj.data());
	renderMatrix(x, y, &identity, i18n("Should be close to identity:"));
	y -= (5 * d->mDefaultFont->height() + 20);



	// some kind of d->mDebugMapCoordinates... but we do our own
	// calculations instead of glUnProject.
	QPoint widgetPos = mapFromGlobal(QCursor::pos());
	GLint realy = d->mViewport[3] - (GLint)widgetPos.y() - 1;
	GLfloat depth = 0.0f;
	glReadPixels(widgetPos.x(), realy, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);

	BoVector4 v;
	v.setX( (GLfloat)((widgetPos.x() - d->mViewport[0]) * 2) / d->mViewport[2] - 1.0f );
	v.setY( (GLfloat)((realy - d->mViewport[1]) * 2) / d->mViewport[3] - 1.0f );
	v.setZ(2 * depth - 1.0f);
	v.setW(1.0f);
	BoVector4 result;
	inverse.transform(&result, &v);

	// it is a column vector, but we display as a row (so ^T)
	QString text = i18n("(Projection * Modelview)^(-1) * (%1 , %2 , %3 , %3)^T:").
			arg(v[0], 6, 'f', 3).
			arg(v[1], 6, 'f', 3).
			arg(v[2], 6, 'f', 3).
			arg(v[3], 6, 'f', 3);
	QString resultText = i18n("(%1 , %2 , %3 , %3)^T").
			arg(result[0], 6, 'f', 3).
			arg(result[1], 6, 'f', 3).
			arg(result[2], 6, 'f', 3).
			arg(result[3], 6, 'f', 3);
	if (result[3] == 0.0f) {
		boError() << k_funcinfo << endl;
		return;
	}
	QString realCoords = i18n("x = %1  ;  y = %2  ;  z = %3").
			arg(result[0] / result[3]).
			arg(result[1] / result[3]).
			arg(result[2] / result[3]);
	glEnable(GL_BLEND);
	glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
	glRecti(x, y + d->mDefaultFont->height(),
			x + d->mDefaultFont->width(text), y);
	glRecti(x, y,
			x + d->mDefaultFont->width(resultText), y - d->mDefaultFont->height());
	glRecti(x, y - d->mDefaultFont->height(),
			x + d->mDefaultFont->width(realCoords), y - 2 * d->mDefaultFont->height());
	glColor3ub(255, 255, 255);
	glDisable(GL_BLEND);
	glRasterPos2i(x, y);
	glCallLists(text.length(), GL_UNSIGNED_BYTE, (GLubyte*)text.latin1());
	y -= d->mDefaultFont->height();
	glRasterPos2i(x, y);
	glCallLists(resultText.length(), GL_UNSIGNED_BYTE, (GLubyte*)resultText.latin1());
	y -= d->mDefaultFont->height();
	glRasterPos2i(x, y);
	glCallLists(realCoords.length(), GL_UNSIGNED_BYTE, (GLubyte*)realCoords.latin1());
 }
 x = border;
 y = d->mViewport[3] - border;
 if (d->mDebugItemWorks) {
	QMap<int, int> workCounts = *canvas()->workCounts();
	renderString(x, y, i18n("Item work statistics:"));
	y -= d->mDefaultFont->height();
	renderString(x, y, i18n("Total items: %1").arg(canvas()->allItemsCount()));
	y -= d->mDefaultFont->height();
	renderString(x, y, i18n("-1 (items): %1").arg(workCounts[-1]));
	y -= d->mDefaultFont->height();
	renderString(x, y, i18n("Doing nothing:     %1").arg(workCounts[(int)UnitBase::WorkNone]));
	y -= d->mDefaultFont->height();
	renderString(x, y, i18n("Moving or turning: %1").arg(workCounts[(int)UnitBase::WorkMove] +
			workCounts[(int)UnitBase::WorkTurn]));
	y -= d->mDefaultFont->height();
	renderString(x, y, i18n("Attacking:         %1").arg(workCounts[(int)UnitBase::WorkAttack]));
	y -= d->mDefaultFont->height();
	renderString(x, y, i18n("Other:             %1").arg(workCounts[(int)UnitBase::WorkConstructed] +
			workCounts[(int)UnitBase::WorkDestroyed] + workCounts[(int)UnitBase::WorkFollow] +
			workCounts[(int)UnitBase::WorkPlugin]));
	y -= d->mDefaultFont->height();
	y -= d->mDefaultFont->height();
 }
 if (d->mDebugCamera) {
	renderString(x, y, i18n("Camera:"));
	y -= d->mDefaultFont->height();
	const BoVector3 lookAt = camera()->lookAt();
	renderString(x, y, i18n("LookAt: (%1; %2; %3)").arg(lookAt.x()).arg(lookAt.y()).arg(lookAt.z()));
	y -= d->mDefaultFont->height();
	renderString(x, y, i18n("Radius: %1").arg(camera()->radius()));
	y -= d->mDefaultFont->height();
	renderString(x, y, i18n("Height: %1").arg(camera()->z()));
	y -= d->mDefaultFont->height();
	renderString(x, y, i18n("Rotation: %1").arg(camera()->rotation()));
	y -= d->mDefaultFont->height();
	y -= d->mDefaultFont->height();
 }
 if (d->mDebugRenderCounts) {
	renderString(x, y, i18n("Items rendered: %1").arg(d->mRenderedItems));
	y -= d->mDefaultFont->height();
	renderString(x, y, i18n("Cells rendered: %1").arg(d->mRenderedCells));
	y -= d->mDefaultFont->height();
 }

// now the chat messages
 d->mChat->renderMessages(border, border, d->mViewport[2] - 2 * border, d->mViewport[3] - 2 * border, d->mDefaultFont);

// display a paused label if game is paused
 if (boGame->gamePaused()) {
	QString pause = i18n("The game is paused");
	w = d->mDefaultFont->width(pause);
	glRasterPos2i(d->mViewport[2] / 2 - w / 2, d->mViewport[3] / 2);
	glCallLists(pause.length(), GL_UNSIGNED_BYTE, (GLubyte*)pause.latin1());
 }
 if (d->mToolTips->showTip()) {
	QPoint pos = mapFromGlobal(QCursor::pos());
	d->mToolTips->renderToolTip(pos.x(), pos.y(), d->mViewport, d->mDefaultFont);
 }

 glColor3ub(255, 255, 255);
 glDisable(GL_BLEND);
}

void BosonBigDisplayBase::renderCells()
{
 BosonTiles* tiles = mCanvas->tileSet();
 if (!tiles) {
	boError() << k_funcinfo << "NULL tiles" << endl;
	return;
 }
 BosonTextureArray* textures = tiles->textures();
 if (!textures) {
	makeCurrent();
	tiles->generateTextures();
	textures = tiles->textures();
	if (!textures) {
		boWarning() << k_funcinfo << "NULL textures for cells" << endl;
		return;
	}
 }

 if (d->mRenderCellsCount == 0) {
	// this happens either when we have to generate the list first or if no
	// cell is visible at all. The latter case isn't speed relevant, so we
	// can simply re-generate then.
	generateCellList();
 }

 if (!localPlayer()) {
	boError() << k_funcinfo << "NULL local player" << endl;
	return;
 }
 BosonMap* map = mCanvas->map();
 if (!map) {
	boError() << k_funcinfo << "NULL map" << endl;
	return;
 }
 float* heightMap = map->heightMap();
 if (!heightMap) {
	boError() << k_funcinfo << "NULL height map" << endl;
 }


 // AB: we can increase performance even more here. lets replace d->mRenderCells
 // by two array defining the coordinates of cells and the heightmap values.
 // we could use that as vertex array for example.
 GLuint texture = 0;
 int tile = -1;
 int heightMapWidth = map->width() + 1;
 d->mRenderedCells = 0;
// int heightMapHeight = map->height() + 1;
 for (int i = 0; i < d->mRenderCellsCount; i++) {
	Cell* c = d->mRenderCells[i];
	int x = c->x();
	int y = c->y();
	if (localPlayer()->isFogged(x, y)) {
		// don't draw anything at all. the cell will just be black,
		// because of the glClear() call.
		continue;
	}
	GLfloat cellXPos = (float)x * BO_GL_CELL_SIZE;
	GLfloat cellYPos = -(float)y * BO_GL_CELL_SIZE;
	if (c->tile() != tile) {
		texture = textures->texture(c->tile());
		tile = c->tile();
		glBindTexture(GL_TEXTURE_2D, texture);

	}
	// FIXME: performance: only a single glBegin(GL_QUADS)!
//	boDebug() << heightMap[y * heightMapWidth + x] << endl;
	glBegin(GL_QUADS);
		glTexCoord2fv(textureUpperLeft);
		glVertex3f(cellXPos, cellYPos, heightMap[y * heightMapWidth + x]);

		glTexCoord2fv(textureLowerLeft);
		glVertex3f(cellXPos, cellYPos - BO_GL_CELL_SIZE, heightMap[(y+1) * heightMapWidth + x]);

		glTexCoord2fv(textureLowerRight);
		glVertex3f(cellXPos + BO_GL_CELL_SIZE, cellYPos - BO_GL_CELL_SIZE, heightMap[(y+1) * heightMapWidth + (x+1)]);

		glTexCoord2fv(textureUpperRight);
		glVertex3f(cellXPos + BO_GL_CELL_SIZE, cellYPos, heightMap[y * heightMapWidth + (x+1)]);
	glEnd();
	d->mRenderedCells++;
 }
 if (d->mDebugShowCellGrid) {
	glDisable(GL_LIGHTING);
	glDisable(GL_NORMALIZE);
	glDisable(GL_DEPTH_TEST);
	glBindTexture(GL_TEXTURE_2D, 0);
	glColor3ub(255, 255, 255);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_QUADS);
	for (int i = 0; i < d->mRenderCellsCount; i++) {
		Cell* c = d->mRenderCells[i];
		int x = c->x();
		int y = c->y();
		const float dist = 0.0f;
		GLfloat cellXPos = (float)x * BO_GL_CELL_SIZE;
		GLfloat cellYPos = -(float)y * BO_GL_CELL_SIZE;
			glVertex3f(cellXPos, cellYPos, heightMap[y * heightMapWidth + x] + dist);
			glVertex3f(cellXPos, cellYPos - BO_GL_CELL_SIZE, heightMap[(y+1) * heightMapWidth + x] + dist);
			glVertex3f(cellXPos + BO_GL_CELL_SIZE, cellYPos - BO_GL_CELL_SIZE, heightMap[(y+1) * heightMapWidth + (x+1)] + dist);
			glVertex3f(cellXPos + BO_GL_CELL_SIZE, cellYPos, heightMap[y * heightMapWidth + (x+1)] + dist);
	}
	glEnd();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_DEPTH_TEST);
	if (boConfig->useLight()) {
		glEnable(GL_LIGHTING);
		glEnable(GL_NORMALIZE);
	}
 }
}

void BosonBigDisplayBase::renderParticles()
{
 //struct timeval start, end, tmvisiblecheck, tmsort;
 //gettimeofday(&start, 0);

 // Return if there aren't any particle systems
 if (canvas()->particleSystemsCount() == 0) {
	//gettimeofday(&end, 0);
	//boDebug(150) << k_funcinfo << "Returning (no particle systems); time elapsed: " << end.tv_usec - start.tv_usec << " us" << endl;
	return;
 }

 // We sort out non-visible systems ourselves
 QPtrListIterator<BosonParticleSystem> allIt(*(canvas()->particleSystems()));
 QPtrList<BosonParticleSystem> visible;
 BosonParticleSystem* s = 0;
 for (; allIt.current(); ++allIt) {
	s = allIt.current();
	//boDebug(150) << k_funcinfo << "System: " << s << "; radius: " << s->boundingSphereRadius() << endl;
	if (sphereInFrustum(s->position(), s->boundingSphereRadius())) {
#warning FIXME
		// FIXME: this is wrong: parts of particle system may be visible even if it's center point isn't
		if (canvas()->cell(s->x(), s->y())) {
			if (!localPlayer()->isFogged(s->x(), s->y())) {
				visible.append(s);
			}
		} else {
			// AB: we don't complain here, as particle systems are
			// visual appearance only. note that invalid positions
			// are BAAAAAAD !!
		}
	}
 }
 //gettimeofday(&tmvisiblecheck, 0);

 // Return if none of particle systems are visible
 if (visible.count() == 0) {
	//gettimeofday(&end, 0);
	//boDebug(150) << k_funcinfo << "Returning (no visible particle systems); time elapsed: " << end.tv_usec - start.tv_usec << " us" << endl;
	return;
 }

 // Resort list of particles if needed
 // This sorts all particles by distance from camera and may be pretty slow, so
 //  we don't resort the list if there hasn't been any advance() calls and
 //  camera hasn't changed either
 BosonParticle* p;
 //bool wassorted = d->mParticlesDirty;  // only for debug, commented because of compiler warning
 if (d->mParticlesDirty) {
	BosonParticleSystem* s;
	float x, y, z;
	d->mParticleList.clear();
	// Add all particles to the list
	QPtrListIterator<BosonParticleSystem> visibleIt(visible);
	s = 0;
	for (; visibleIt.current(); ++visibleIt) {
		s = visibleIt.current();
		for (int i = 0; i < s->mMaxNum; i++) {
			if (s->mParticles[i].life > 0.0) {
				p = &(s->mParticles[i]);
				// Calculate distance from camera. Note that for performance reasons,
				//  we don't calculate actual distance, but square of it.
				x = p->pos.x() - d->mCameraPos.x();
				y = p->pos.y() - d->mCameraPos.y();
				z = p->pos.z() - d->mCameraPos.z();
				p->distance = (x*x + y*y + z*z);
				// Append to list
				d->mParticleList.append(p);
			}
		}
	}

	// If there's no particles, return
	if(d->mParticleList.count() == 0) {
		//gettimeofday(&end, 0);
		//boDebug(150) << k_funcinfo << "Returning (no visible particles); time elapsed: " << end.tv_usec - start.tv_usec << " us" << endl;
		return;
	}

	// Sort the list
	d->mParticleList.sort();
	d->mParticlesDirty = false;
 }
 //gettimeofday(&tmsort, 0);

 /// Draw particles
 glEnable(GL_DEPTH_TEST);
 glDepthMask(GL_FALSE);
 glEnable(GL_TEXTURE_2D);
 glEnable(GL_BLEND);
 glDisable(GL_LIGHTING); // warning: this functions leaves light at *disabled* !
 glDisable(GL_NORMALIZE);

 // Matrix stuff for aligned particles
 BoVector3 x(d->mModelviewMatrix[0], d->mModelviewMatrix[4], d->mModelviewMatrix[8]);
 BoVector3 y(d->mModelviewMatrix[1], d->mModelviewMatrix[5], d->mModelviewMatrix[9]);

 // Some cache variables
 int blendfunc = -1;
 GLuint texture = 0;
 bool betweenbeginend = false;  // If glBegin has been called, but glEnd() hasn't. Very hackish.
 BoVector3 a, b, c, e;  // Vertex positions. e is used instead of d which clashes with private class

 QPtrListIterator<BosonParticle> i(d->mParticleList);
 //boDebug(150) << k_funcinfo << "Drawing " << i.count() << " particles" << endl;
 while ((p = i.current()) != 0) {
	++i;
	// We change blend function and texture only if it's necessary
	if (blendfunc != p->system->mBlendFunc[1]) {
		// Note that we only check for dest blending function currently, because src
		//  is always same. If this changes in the future, change this as well!
		if (betweenbeginend) {
			glEnd();
			betweenbeginend = false;
		}
		glBlendFunc(p->system->mBlendFunc[0], p->system->mBlendFunc[1]);
		blendfunc = p->system->mBlendFunc[1];
	}
	if (texture != p->tex) {
		if (betweenbeginend) {
			glEnd();
			betweenbeginend = false;
		}
		glBindTexture(GL_TEXTURE_2D, p->tex);
		texture = p->tex;
	}
	if (!betweenbeginend) {
		glBegin(GL_QUADS);
		betweenbeginend = true;
	}

  if (p->system->mAlign) {
		a = p->pos + ((-x + y) * p->size);
		b = p->pos + (( x + y) * p->size);
		c = p->pos + (( x - y) * p->size);
		e = p->pos + ((-x - y) * p->size);
	} else {
		a = p->pos + (BoVector3(-0.5, 0.5, 0.0) * p->size);
		b = p->pos + (BoVector3(0.5, 0.5, 0.0) * p->size);
		c = p->pos + (BoVector3(0.5, -0.5, 0.0) * p->size);
		e = p->pos + (BoVector3(-0.5, -0.5, 0.0) * p->size);
	}

	glColor4fv(p->color.data());  // Is it worth to cache color as well?
	glTexCoord2f(0.0, 1.0);  glVertex3fv(a.data());
	glTexCoord2f(1.0, 1.0);  glVertex3fv(b.data());
	glTexCoord2f(1.0, 0.0);  glVertex3fv(c.data());
	glTexCoord2f(0.0, 0.0);  glVertex3fv(e.data());
 }
 glEnd();

 glColor4ub(255, 255, 255, 255); // Reset color
 glDepthMask(GL_TRUE);
 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 glDisable(GL_BLEND);
 //gettimeofday(&end, 0);
 //boDebug(150) << k_funcinfo << "Returning (all particles drawn); time elapsed: " << end.tv_usec - start.tv_usec << " us" << endl;
 //boDebug(150) << k_funcinfo << "        Visibility check:  " << tmvisiblecheck.tv_usec - start.tv_usec << " us" << endl;
 //boDebug(150) << k_funcinfo << "        Particles sorting: " << tmsort.tv_usec - tmvisiblecheck.tv_usec << " us (wassorted: " << wassorted << ")" << endl;
 //boDebug(150) << k_funcinfo << "        Particles drawing: " << end.tv_usec - tmsort.tv_usec << " us" << endl;
}

void BosonBigDisplayBase::renderMatrix(int x, int y, const BoMatrix* matrix, const QString& text)
{
 y -= d->mDefaultFont->height(); // y is now at the bottom of the first line
 glListBase(d->mDefaultFont->displayList());

 QString lines[4];
 int w = 0;
 for (int i = 0; i < 4; i++) {
	lines[i] = QString("%1   %2   %3   %4").
			arg(matrix->data()[i + 0], 6, 'f', 3).
			arg(matrix->data()[i + 4], 6, 'f', 3).
			arg(matrix->data()[i + 8], 6, 'f', 3).
			arg(matrix->data()[i + 12], 6, 'f', 3);
	w = QMAX(w, d->mDefaultFont->width(lines[i]));
 }
 glEnable(GL_BLEND);
 glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
 glRecti(x, y + d->mDefaultFont->height(),
		x + d->mDefaultFont->width(text),
		y);
 glRecti(x, y,
		x + w,
		y - 4 * d->mDefaultFont->height());

 glColor3ub(255, 255, 255);

 glRasterPos2i(x, y);
 glCallLists(text.length(), GL_UNSIGNED_BYTE, (GLubyte*)text.latin1());
 y -= d->mDefaultFont->height();


 for (int i = 0; i < 4; i++) {
	glRasterPos2i(x, y);
	glCallLists(lines[i].length(), GL_UNSIGNED_BYTE, (GLubyte*)lines[i].latin1());
	y -= d->mDefaultFont->height();
 }
 glDisable(GL_BLEND);
}

void BosonBigDisplayBase::renderString(int x, int y, const QString& text)
{
 y -= d->mDefaultFont->height(); // y is now at the bottom of the first line
 glEnable(GL_BLEND);
 glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
 glRecti(x, y + d->mDefaultFont->height(),
		x + d->mDefaultFont->width(text),
		y);
 glColor3ub(255, 255, 255);
 glRasterPos2i(x, y);
 glCallLists(text.length(), GL_UNSIGNED_BYTE, (GLubyte*)text.latin1());
 glDisable(GL_BLEND);
}



// one day we might support swapping LMB and RMB so let's use defines already to
// make that easier.
#define LEFT_BUTTON LeftButton
#define RIGHT_BUTTON RightButton

void BosonBigDisplayBase::slotMouseEvent(KGameIO* , QDataStream& stream, QMouseEvent* e, bool *eatevent)
{
// AB: maybe we could move this function to the displayInput directly!
 BO_CHECK_NULL_RET(displayInput());
 GLfloat posX = 0.0;
 GLfloat posY = 0.0;
 GLfloat posZ = 0.0;
 if (!mapCoordinates(e->pos(), &posX, &posY, &posZ)) {
	boError() << k_funcinfo << "Cannot map coordinates" << endl;
	return;
 }
// boDebug() << posZ << endl;
 BoVector3 canvasVector;
 worldToCanvas(posX, posY, posZ, &canvasVector);

 BoAction action;
 action.setCanvasVector(canvasVector);
 action.setWorldPos(posX, posY, posZ);
 if (e->type() != QEvent::Wheel) {
	action.setWidgetPos(e->pos());
	action.setControlButton(e->state() & ControlButton);
	action.setShiftButton(e->state() & ShiftButton);
	action.setAltButton(e->state() & AltButton);
 } else {
	QWheelEvent* w = (QWheelEvent*)e;
	action.setWidgetPos(w->pos());
	action.setControlButton(w->state() & ControlButton);
	action.setShiftButton(w->state() & ShiftButton);
	action.setAltButton(w->state() & AltButton);
 }

 // our actions are done on Button*Release*, not Press. That conflicts with
 // DblClick, so we store whether the last Press event was an actual press event
 // or a double click.
 static ButtonState isDoubleClick = NoButton;

 switch (e->type()) {
	case QEvent::Wheel:
	{
		QWheelEvent* wheel = (QWheelEvent*)e;
		bool send = false;
		float delta = -wheel->delta() / 120;//120: see QWheelEvent::delta()
		mouseEventWheel(delta, wheel->orientation(), action, stream, &send);
		if (send) {
			*eatevent = true;
		}
		wheel->accept();
		break;
	}
	case QEvent::MouseMove:
	{
		bool send = false;
		isDoubleClick = NoButton; // when the mouse was pressed twice but the second press is hold down and moved then it isn't a double click anymore.
		mouseEventMove(e->state(), action, stream, &send);
		if (send) {
			*eatevent = true;
		}
		e->accept();
		break;
	}
	case QEvent::MouseButtonDblClick:
	{
		makeActive();
		isDoubleClick = e->button();
		// actual actions will happen on ButtonRelease!
		e->accept();
		break;
	}
	case QEvent::MouseButtonPress:
	{
		makeActive();
		// no action should happen here!
		isDoubleClick = NoButton;
		if (e->button() == LEFT_BUTTON) {
			d->mSelectionRect.setStartWidgetPos(e->pos());
		} else if (e->button() == MidButton) {
			// nothing to be done here
		} else if (e->button() == RIGHT_BUTTON) {
			if (boConfig->rmbMove()) {
				//AB: this might be obsolete..
				d->mMouseMoveDiff.moveToPos(e->pos()); // set position, but do not yet start!
			}
		}
		e->accept();
		break;
	}
	case QEvent::MouseButtonRelease:
	{
		bool send = false;
		if (e->button() == isDoubleClick) {
			mouseEventReleaseDouble(e->button(), action, stream, &send);
		} else {
			mouseEventRelease(e->button(), action, stream, &send);
		}
		if (send) {
			*eatevent = true;
		}
		e->accept();
		break;
	}
	default:
		boWarning() << "unexpected mouse event " << e->type() << endl;
		e->ignore();
		return;
 }
}

void BosonBigDisplayBase::mouseEventWheel(float delta, Orientation orientation, const BoAction& boAction, QDataStream&, bool*)
{
 int action;
 if (boAction.shiftButton()) {
	action = boConfig->mouseWheelShiftAction();
 } else {
	action = boConfig->mouseWheelAction();
 }
 switch (action) {
	case CameraMove:
	{
		int scrollX, scrollY;
		if (boAction.controlButton()) {
			scrollX = width();
			scrollY = height();
		} else {
			scrollX = 20;
			scrollY = 20;
			delta *= QApplication::wheelScrollLines();
		}
		if (orientation == Horizontal) {
			scrollX *= (int)delta;
			scrollY = 0;
		} else {
			scrollX = 0;
			scrollY *= (int)delta;
		}
		scrollBy(scrollX, scrollY);
		break;
	}
	case CameraZoom:
		float z;
		if (boAction.controlButton()) {
			delta *= 3;
		} else {
			delta *= 1; // no effect, btw
		}
		camera()->changeZ(delta);
		z = canvas()->cell((int)(camera()->lookAt().x()), (int)-(camera()->lookAt().y()))->averageHeight();
		if (camera()->z() < z + CAMERA_MIN_Z) {
			camera()->changeZ(z + CAMERA_MIN_Z - camera()->z());
		}
		cameraChanged();
		break;
	case CameraRotate:
		if (boAction.controlButton()) {
			delta *= 30;
		} else {
			delta *= 10;
		}
		camera()->changeRotation(delta);
		cameraChanged();
		break;
	default:
	{
		boWarning() << k_funcinfo << "invalid wheel action: " << action << endl;
		break;
	}
 }
}

void BosonBigDisplayBase::mouseEventMove(int buttonState, const BoAction& action, QDataStream&, bool*)
{
 float posX, posY, posZ;
 action.worldPos(&posX, &posY, &posZ);
 d->mMouseMoveDiff.moveToPos(action.widgetPos());
 if (action.altButton()) {
	// The Alt button is the camera modifier in boson.
	// Better don't do important stuff (like unit movement
	// or selections) here, since a single push on Alt gives
	// the focus to the menu which might be very confusing
	// during a game.
	if (buttonState & LEFT_BUTTON) {
		d->mMouseMoveDiff.startZoom();
		camera()->changeZ(d->mMouseMoveDiff.dy());
		float z = canvas()->cell((int)(camera()->lookAt().x()), (int)-(camera()->lookAt().y()))->averageHeight();
		if (camera()->z() < z + CAMERA_MIN_Z) {
			camera()->changeZ(z + CAMERA_MIN_Z - camera()->z());
		}
		cameraChanged();
	} else if (buttonState & RIGHT_BUTTON) {
		d->mMouseMoveDiff.startRotate();
		camera()->changeRotation(d->mMouseMoveDiff.dx());
		camera()->changeRadius(d->mMouseMoveDiff.dy());
		cameraChanged();
	}
 } else if (buttonState & LEFT_BUTTON) {
	if (!displayInput()->actionLocked()) {
		// selection rect gets drawn.
		// other modifiers are ignored
		d->mSelectionRect.setVisible(true);
		moveSelectionRect(action.widgetPos());
	}
 } else if (buttonState & RIGHT_BUTTON) {
	// RMB+MouseMove does *not* depend on CTRL or Shift. the
	// map is moved in all cases (currently - we have some
	// free buttons here :))
	if (boConfig->rmbMove()) {
		// problem is that QCursor::setPos() also causes
		// a mouse move event. we can use this hack in
		// order to check whether it is a real mouse
		// move event or we caused it here.
		// TODO: use d->mMouseMoveDiff.x()/y() in
		// paintGL() for the cursor, not QCursor::pos()
//		static bool a = false;
//		if (a) {
//			a = false;
//			break;
//		}
//		a = true;moveLookAtBy
//		QPoint pos = mapToGlobal(QPoint(d->mMouseMoveDiff.oldX(), d->mMouseMoveDiff.oldY()));
//		QCursor::setPos(pos);

		// modifiers are ignored.
		d->mMouseMoveDiff.startRMBMove();
		GLfloat dx, dy;
		int moveX = d->mMouseMoveDiff.dx();
		int moveY = d->mMouseMoveDiff.dy();
		mapDistance(moveX, moveY, &dx, &dy);
		// FIXME: we must also change camera's z-coordinate here to ensure that no
		//  cells go through near clip.
		camera()->moveLookAtBy(dx, dy, 0);
		float z = canvas()->cell((int)(camera()->lookAt().x()), (int)-(camera()->lookAt().y()))->averageHeight();
		if (camera()->z() < z + CAMERA_MIN_Z) {
			camera()->changeZ(z + CAMERA_MIN_Z - camera()->z());
		}
		cameraChanged();
	} else {
		d->mMouseMoveDiff.stop();
	}
 } else if (buttonState & MidButton) {
	// currently unused
 }
 QPoint widgetPos = mapFromGlobal(QCursor::pos());
 GLfloat x = 0.0, y = 0.0, z = 0.0;
 mapCoordinates(widgetPos, &x, &y, &z);
 d->mDebugMapCoordinatesX = x;
 d->mDebugMapCoordinatesY = y;
 d->mDebugMapCoordinatesZ = z;
 worldToCanvas(x, y, z, &(d->mCanvasPos));
 worldToCanvas(x, y, z, &(d->mCanvasVector)); // AB: are these already real z coordinates?
 displayInput()->updatePlacementPreviewData();

 // AB: we might want to use a timer here instead - then we would also be able
 // to change the cursor type when units move under the cursor. i don't want to
 // call updateCursor() from BosonCanvas::slotAdvance() as it would get called
 // too often then
 displayInput()->updateCursor();
}

void BosonBigDisplayBase::mouseEventRelease(ButtonState button,const BoAction& action, QDataStream& stream, bool* send)
{
 switch (button) {
	case LEFT_BUTTON:
	{
		if (displayInput()->actionLocked()) {
			// basically the same as a normal RMB
			displayInput()->actionClicked(action, stream, send);
		} else if (action.shiftButton()) {
			BoItemList* items = selectionRectItems();
			displayInput()->unselectArea(items);
			d->mSelectionRect.setVisible(false);
		} else if (action.controlButton()) {
			removeSelectionRect(false);
		} else {
			// select the unit(s) below the cursor/inside the
			// selection rect
			removeSelectionRect(true);
		}
		break;
	}
	case MidButton:
	{
		// we ignore all modifiers here, currently.
		if (boConfig->mmbMove()) {
			float posX, posY, posZ;
			action.worldPos(&posX, &posY, &posZ);
			int cellX, cellY;
			cellX = (int)(posX / BO_GL_CELL_SIZE);
			cellY = (int)(-posY / BO_GL_CELL_SIZE);
			slotReCenterDisplay(QPoint(cellX, cellY));
			displayInput()->updateCursor();
		}
		break;
	}
	case RIGHT_BUTTON:
	{
		if (!d->mMouseMoveDiff.isStopped()) {
			d->mMouseMoveDiff.stop();
		} else if (displayInput()->actionLocked()) {
			displayInput()->unlockAction();
			displayInput()->updateCursor();
		} else {
			displayInput()->actionClicked(action, stream, send);
		}
		break;
	}
	default:
		boError() << k_funcinfo << "invalid mouse button " << button << endl;
		break;
 }
}

void BosonBigDisplayBase::mouseEventReleaseDouble(ButtonState button, const BoAction& action, QDataStream& , bool* )
{
 switch (button) {
	case LEFT_BUTTON:
	{
		// we ignore UnitAction is locked here
		// currently!
		bool replace = !action.controlButton();
		bool onScreenOnly = !action.shiftButton();
		Unit* unit = canvas()->findUnitAt(action.canvasVector());
		if (unit) {
			if (onScreenOnly) {
				boDebug() << "TODO: select only those that are currently on the screen!" << endl;
			}
			if (!displayInput()->selectAll(unit->unitProperties(), replace)) {
				displayInput()->selectSingle(unit, replace);
			}
		}
		break;
	}
	case MidButton:
	case RIGHT_BUTTON:
	{
		// we ignore all other (RMB, MMB) for now. we
		// might use this one day.
		break;
	}
	default:
		boError() << k_funcinfo << "invalid mouse button " << button << endl;
		break;
 }
}

void BosonBigDisplayBase::addMouseIO(Player* p)
{
 boDebug() << k_funcinfo << endl;
 if (!p) {
	boError() << k_funcinfo << "NULL player" << endl;
	return;
 }
 if (d->mMouseIO) {
	boError() << k_funcinfo << "mouse IO already present for this display!" << endl;
	return;
 }
 if (p->hasRtti(KGameIO::MouseIO)) {
	// FIXME: this is only invalid if the IO is for the same big display!
	boWarning() << k_funcinfo << "player already has a mouse IO" << endl;
	return;
 }
 d->mMouseIO = new KGameMouseIO(this, true);
 connect(d->mMouseIO, SIGNAL(signalMouseEvent(KGameIO*, QDataStream&, QMouseEvent*, bool*)),
		this, SLOT(slotMouseEvent(KGameIO*, QDataStream&, QMouseEvent*, bool*)));
 p->addGameIO(d->mMouseIO);
}

void BosonBigDisplayBase::makeActive()
{
 emit signalMakeActive(this);
}

void BosonBigDisplayBase::setActive(bool a)
{
 if (a) {
	qApp->setGlobalMouseTracking(true);
	qApp->installEventFilter(this);
 } else {
	qApp->setGlobalMouseTracking(false);
	qApp->removeEventFilter(this);
 }
 selection()->activate(a);
}

void BosonBigDisplayBase::setLocalPlayer(Player* p) 
{
 boDebug() << k_funcinfo << endl;
 if (d->mLocalPlayer == p) {
	boDebug() << k_funcinfo << "player already set. nothing to do." << endl;
	return;
 }
 if (d->mLocalPlayer) {
	boDebug() << k_funcinfo << "already a local player present! unset..." << endl;
	delete d->mMouseIO;
	d->mMouseIO = 0;
	d->mLocalPlayer = 0;
 }
 d->mLocalPlayer = p;
 if (!p) {
	return;
 }
 addMouseIO(p);
}

Player* BosonBigDisplayBase::localPlayer() const
{
 return d->mLocalPlayer;
}

void BosonBigDisplayBase::slotCenterHomeBase()
{
 boDebug() << k_funcinfo << endl;
 BO_CHECK_NULL_RET(localPlayer());
 //TODO
 // find the command center of the local player
 QPoint pos(0, 0); // note: we use *cell* coordinates!
 Player* p = localPlayer();
 QPtrList<Unit> units = *(p->allUnits());
 QPtrListIterator<Unit> it(units);
 Unit* commandCenter = 0;
 for (; it.current() && !commandCenter; ++it) {
	// now we have a problem. what do we need to check for?
	// checking for Unit::type() isn't nice. maybe we need a
	// UnitProperties::isCommandCenter() or so?
	//
	// so for now we get around this problem by picking the first unit and
	// then exiting the loop.
	commandCenter = it.current();
 }
 if (!commandCenter) {
	commandCenter = units.getFirst();
 }
 if (!commandCenter) {
	boWarning() << k_funcinfo << "cannot find a unit for localplayer" << endl;
	// no units for player
	return;
 }
 pos = QPoint((int)commandCenter->x() / BO_TILE_SIZE, (int)commandCenter->y() / BO_TILE_SIZE);

 slotReCenterDisplay(pos);
}

void BosonBigDisplayBase::slotResetViewProperties()
{
 d->mFovY = 60.0;
 d->mAspect = 1.0;
 setCamera(Camera(mCanvas->mapWidth(), mCanvas->mapHeight()));
 resizeGL(d->mViewport[2], d->mViewport[3]);
}

void BosonBigDisplayBase::slotReCenterDisplay(const QPoint& pos)
{
//TODO don't center the corners - e.g. 0;0 should be top left, never center 
 camera()->setLookAt(BoVector3(((float)pos.x()) * BO_GL_CELL_SIZE, -((float)pos.y()) * BO_GL_CELL_SIZE, 0));
 cameraChanged();
}

void BosonBigDisplayBase::worldToCanvas(GLfloat x, GLfloat y, GLfloat /*z*/, QPoint* pos) const
{
 pos->setX((int)(x / BO_GL_CELL_SIZE * BO_TILE_SIZE));
 pos->setY((int)(-y / BO_GL_CELL_SIZE * BO_TILE_SIZE));
 // AB: z remains as-is
}

void BosonBigDisplayBase::worldToCanvas(GLfloat x, GLfloat y, GLfloat z, BoVector3* pos) const
{
 // we want the rounding errors here (at least for now).
 int intx = (int)(x / BO_GL_CELL_SIZE * BO_TILE_SIZE);
 int inty = (int)(-y / BO_GL_CELL_SIZE * BO_TILE_SIZE);
 pos->set((float)intx, (float)inty, z);
}

void BosonBigDisplayBase::canvasToWorld(int x, int y, float z, GLfloat* glx, GLfloat* gly, GLfloat* glz) const
{
 *glx = (((GLfloat)x) * BO_GL_CELL_SIZE) / BO_TILE_SIZE;
 *gly = (((GLfloat)-y) * BO_GL_CELL_SIZE) / BO_TILE_SIZE;
 *glz = z;
}

bool BosonBigDisplayBase::mapCoordinates(const QPoint& pos, GLfloat* posX, GLfloat* posY, GLfloat* posZ, bool useRealDepth) const
{
 GLint realy = d->mViewport[3] - (GLint)pos.y() - 1;
 // we basically calculate a line here .. nearX/Y/Z is the starting point,
 // farX/Y/Z is the end point. From these points we can calculate a direction.
 // using this direction and the points nearX(Y)/farX(Y) you can build triangles
 // and then find the point that is on z=0.0
 GLdouble nearX, nearY, nearZ;
 GLdouble farX, farY, farZ;
 BoVector3 near, far;
 if (!boUnProject(pos, &near, 0.0f)) {
	return false;
 }
 if (!boUnProject(pos, &far, 1.0f)) {
	return false;
 }
 nearX = near[0];
 nearY = near[1];
 nearZ = near[2];
 farX = far[0];
 farY = far[1];
 farZ = far[2];

 GLdouble zAtPoint = 0.0f;

 // we need to find out which z position is at the point pos. this is important
 // for mapping 2d values (screen coordinates) to 3d (world coordinates)
 GLfloat depth = 0.0;
 glReadPixels(pos.x(), realy, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);

 // AB: 0.0f is reached when we have a point that is outside the actual window!
 if (useRealDepth && depth != CLEAR_DEPTH && depth != 0.0f) {
	// retrieve z
	BoVector3 v;
	if (!boUnProject(pos, &v)) {
		return false;
	}
	zAtPoint = v[2];
 } else {
	// assume we're using z = 0.0f
	zAtPoint = 0.0f;
 }

 // simple maths .. however it took me pretty much time to do this.. I haven't
 // done this for way too long time!
 GLdouble dist = (nearZ - zAtPoint); // distance from nearZ to our actual z. for z=0.0 this is equal to nearZ.
 GLdouble tanAlphaX = (nearX - farX) / (nearZ - farZ);
 *posX = (GLfloat)(nearX - tanAlphaX * dist);

 GLdouble tanAlphaY = (nearY - farY) / (nearZ - farZ);
 *posY = (GLfloat)(nearY - tanAlphaY * dist);

 *posZ = zAtPoint;
 return true;
}

bool BosonBigDisplayBase::mapDistance(int windx, int windy, GLfloat* dx, GLfloat* dy) const
{
 GLfloat moveZ; // unused
 GLfloat moveX1, moveY1;
 GLfloat moveX2, moveY2;
 if (windx >= width()) {
	boError() << k_funcinfo << "windx (" << windx <<") must be < " << width() << endl;
	return false;
 }
 if (windy >= height()) {
	boError() << k_funcinfo << "windy (" << windy <<") must be < " << height() << endl;
	return false;
 }
 if (!mapCoordinates(QPoint(width() / 2 - windx / 2, height() / 2 - windy / 2), &moveX1, &moveY1, &moveZ, false)) {
	boError() << k_funcinfo << "Cannot map coordinates" << endl;
	return false;
 }
 if (!mapCoordinates(QPoint(width() / 2 + windx / 2, height() / 2 + windy / 2), &moveX2, &moveY2, &moveZ, false)) {
	boError() << k_funcinfo << "Cannot map coordinates" << endl;
	return false;
 }
 *dx = moveX2 - moveX1;
 *dy = moveY2 - moveY1;
 return true;
}

bool BosonBigDisplayBase::mapCoordinatesToCell(const QPoint& pos, QPoint* cell)
{
 GLfloat x, y, z;
 if (!mapCoordinates(pos, &x, &y, &z)) {
	return false;
 }
 y *= -1;
 int cellX = (int)(x / BO_GL_CELL_SIZE);
 int cellY = (int)(y / BO_GL_CELL_SIZE);
 cellX = QMAX(0, QMIN((int)mCanvas->mapWidth(), cellX));
 cellY = QMAX(0, QMIN((int)mCanvas->mapHeight(), cellY));
 cell->setX(cellX);
 cell->setY(cellY);
 return true;
}

void BosonBigDisplayBase::enterEvent(QEvent*)
{
 if (!cursor()) {
	// don't generate an error, since a NULL cursor might be valid for
	// editor mode
	return;
 }
// cursor()->showCursor();
}

void BosonBigDisplayBase::leaveEvent(QEvent*)
{
 if (!cursor()) {
	// don't generate an error, since a NULL cursor might be valid for
	// editor mode
	return;
 }
// cursor()->hideCursor();
}

void BosonBigDisplayBase::quitGame()
{
 selection()->clear();
 d->mIsQuit = true; // don't call paintGL() anymore
}

bool BosonBigDisplayBase::eventFilter(QObject* o, QEvent* e)
{
 switch (e->type()) {
	case QEvent::MouseMove:
		if (!d->mCursorEdgeTimer.isActive()) {
			slotCursorEdgeTimeout();
		}
		break;
	default:
		break;
 }
 return BosonGLWidget::eventFilter(o, e);
}

void BosonBigDisplayBase::slotUnitChanged(Unit* unit)
{
// FIXME: we might want to place this code into BoSelection directly (cleaner)
 if (!unit) {
	boError() << k_funcinfo << "NULL unit" << endl;
	return;
 }
 if (unit->isDestroyed()) {
	if (selection()->contains(unit)) {
		selection()->removeUnit(unit);
	}
 }
}


void BosonBigDisplayBase::moveSelectionRect(const QPoint& widgetPos)
{
 if (d->mSelectionRect.isVisible()) {
	d->mSelectionRect.setEndWidgetPos(widgetPos);
	// would be great here but is a huge performance
	// problem (therefore in releasebutton):
	// selectArea();
 }
}

void BosonBigDisplayBase::removeSelectionRect(bool replace)
{
 BO_CHECK_NULL_RET(displayInput());
 if (d->mSelectionRect.isVisible()) {
	// here as there is a performance problem in
	// mousemove:
	BoItemList* items = selectionRectItems();
	displayInput()->selectArea(items, replace);

	d->mSelectionRect.setVisible(false);
	if (!selection()->isEmpty()) {
		Unit* u = selection()->leader();
		if (u->owner() == localPlayer()) {
			// TODO: do not play sound here
			// instead make virtual and play in derived class
			u->playSound(SoundOrderSelect);
		}
	}
 } else {
	// a simple click on the map
	GLfloat x,y,z;
	if (!mapCoordinates(d->mSelectionRect.startPos(), &x, &y, &z)) {
		boError() << k_funcinfo << "Cannot map coordinates" << endl;
		return;
	}
	BoVector3 canvasVector;
	worldToCanvas(x, y, z, &canvasVector);
	Unit* unit = 0l;
	if (!canvas()->onCanvas(canvasVector)) {
		boError() << k_funcinfo << canvasVector.x() << "," << canvasVector.y() << " is not on the canvas!" << endl;
		return;
	}
	// this is not good: isFogged() should get checked *everywhere* where a
	// player tries to select a unit!
	// maybe in selectSingle() or so.
	if (!localPlayer()->isFogged((int)(canvasVector.x() / BO_TILE_SIZE), (int)(canvasVector.y() / BO_TILE_SIZE))) {
		unit = canvas()->collisions()->findUnitAt(canvasVector);
	}
	if (unit) {
		boDebug() << k_funcinfo << "select unit at " << canvasVector.x() << "," << canvasVector.y() << " (canvas)" << endl;
		displayInput()->selectSingle(unit, replace);
		// cannot be placed into selection() cause we don't have localPlayer
		// there
		if (localPlayer() == unit->owner()) {
			unit->playSound(SoundOrderSelect);
		}
	} else {
		if (replace) {
			selection()->clear();
		}
	}
 }
}


BoItemList* BosonBigDisplayBase::selectionRectItems()
{
 if (!canvas()) {
	return new BoItemList();
 }
 const bool debugMe = false;

 QRect widgetRect;
 d->mSelectionRect.widgetRect(&widgetRect);

 GLfloat maxX, maxY;
 GLfloat minX, minY;
 calculateWorldRect(widgetRect, &minX, &minY, &maxX, &maxY);
 maxX /= BO_GL_CELL_SIZE;
 minX /= BO_GL_CELL_SIZE;
 maxY /= -BO_GL_CELL_SIZE;
 minY /= -BO_GL_CELL_SIZE;

 if (debugMe) {
	boDebug() << k_funcinfo << "maxX: " << maxX << " maxY: " << maxY
			<< " minX: " << minX
			<< " minY: " << minY
			<< endl;
 }


 // now the really ugly part starts. we really should improve this.
 // the rect (pos1, pos2) is the smallest possible 2d rect that contains all
 // cells (with 2d rect i mean: parallel to x and y world-axis).
 // but we we need an actual world rect, as the view is most probably rotated
 // and so a lot of cells that are in the parallel rect are not in the actual
 // rect.
 // to achieve this we loop through all cells in the parallel rect and check
 // whether they are in the actual rect.
 // someone with some maths knowledge and/or some time can surely speed this up
 // by using some proper formulas!

 QRect cellRect = QRect(QPoint((int)minX, (int)minY), QPoint((int)maxX, (int)maxY)).normalize();
 QPoint win;
 QValueList<Cell*> cells;
 if (debugMe) {
	boDebug() << k_funcinfo << cellRect.left()
			<< " " << cellRect.right()
			<< " " << cellRect.top()
			<< " " << cellRect.bottom() << endl;
	boDebug() << "widgetrect: " << widgetRect.left()
			<< " " << widgetRect.right()
			<< " " << widgetRect.top()
			<< " " << widgetRect.bottom() << endl;
 }
 for (int x = cellRect.left(); x <= cellRect.right(); x++) {
	for (int y = cellRect.top(); y <= cellRect.bottom(); y++) {
		Cell* c = canvas()->cell(x, y);
		if (!c) {
			boDebug() << "NULL cell at " << x << "," << y << endl;
			continue;
		}
		// AB: we use z==0.0, that is not really nice here. anyway,
		// let's wait until it causes trouble.
		// (read: I don't know which kind of trouble it will cause)
		GLfloat glx, gly, glz;
		// top left corner of cell
		glx = x * BO_GL_CELL_SIZE;
		gly = -y * BO_GL_CELL_SIZE;
		glz = 0.0f;
		boProject(glx, gly, glz, &win);
		if (widgetRect.contains(win)) {
			if (debugMe) {
				boDebug() << "adding cell at " << x << "," << y << endl;
			}
			cells.append(c);
			continue;
		}
		// top right corner of cell
		glx = (x + 1) * BO_GL_CELL_SIZE;
		gly = -y * BO_GL_CELL_SIZE;
		boProject(glx, gly, glz, &win);
		if (widgetRect.contains(win)) {
			if (debugMe) {
				boDebug() << "adding cell at " << x << "," << y << endl;
			}
			cells.append(c);
			continue;
		}
		// bottom left corner of cell
		glx = x * BO_GL_CELL_SIZE;
		gly = -(y + 1) * BO_GL_CELL_SIZE;
		boProject(glx, gly, glz, &win);
		if (widgetRect.contains(win)) {
			if (debugMe) {
				boDebug() << "adding cell at " << x << "," << y << endl;
			}
			cells.append(c);
			continue;
		}
		// bottom right corner of cell
		glx = (x + 1) * BO_GL_CELL_SIZE;
		gly = -(y + 1) * BO_GL_CELL_SIZE;
		boProject(glx, gly, glz, &win);
		if (widgetRect.contains(win)) {
			if (debugMe) {
				boDebug() << "adding cell at " << x << "," << y << endl;
			}
			cells.append(c);
			continue;
		}
		if (debugMe) {
			boDebug() << "not adding cell at " << x << "," << y
					<< " at winx=" << win.x() << " "
					<< " winy=" << win.y() << endl;
		}
	}
 }

 // another ugly part of this function...
 QPtrVector<Cell> cellVector(cells.count());
 QValueList<Cell*>::Iterator it;
 int i = 0;
 for (it = cells.begin(); it != cells.end(); ++it) {
	cellVector.insert(i, *it);
	i++;
 }

 return canvas()->collisions()->collisionsAtCells(&cellVector, 0, true);
}

void BosonBigDisplayBase::setKGameChat(KGameChat* chat)
{
 d->mChat->setChat(chat);
}

void BosonBigDisplayBase::addChatMessage(const QString& message)
{
 d->mChat->addMessage(message);
}

void BosonBigDisplayBase::slotCursorEdgeTimeout()
{
 float x = 0;
 float y = 0;
 const int sensity = boConfig->cursorEdgeSensity();
 QWidget* w = qApp->mainWidget();
 QPoint pos = w->mapFromGlobal(QCursor::pos());

 const int move = 20; // FIXME hardcoded - use BosonConfig instead
 if (pos.x() <= sensity && pos.x() > -1) {
	x = -move;
 } else if (pos.x() >= w->width() - sensity && pos.x() <= w->width()) {
	x = move;
 }
 if (pos.y() <= sensity && pos.y() > -1) {
	y = -move;
 } else if (pos.y() >= w->height() - sensity && pos.y() <= w->height()) {
	y = move;
 }
 if (!x && !y || !sensity) {
	d->mCursorEdgeTimer.stop();
	d->mCursorEdgeCounter = 0;
 } else {
	GLfloat dx, dy;
	mapDistance((int)x, (int)y, &dx, &dy);
	if (!d->mCursorEdgeTimer.isActive()) {
		d->mCursorEdgeTimer.start(20);
	}
	d->mCursorEdgeCounter++;
	if (d->mCursorEdgeCounter > 30) {
		camera()->moveLookAtBy(dx, dy, 0);
		cameraChanged();
	}
 }
}


void BosonBigDisplayBase::scrollBy(int dx, int dy)
{
 GLfloat x, y;
 mapDistance(dx, dy, &x, &y);
 camera()->moveLookAtBy(x, y, 0);
 cameraChanged();
}

void BosonBigDisplayBase::calculateWorldRect(const QRect& rect, float* minX, float* minY, float* maxX, float* maxY) const
{
 const BosonMap* map = mCanvas->map();
 BO_CHECK_NULL_RET(map);
 GLfloat posX, posY;
 GLfloat posZ;
 mapCoordinates(rect.topLeft(), &posX, &posY, &posZ);
 *maxX = *minX = posX;
 *maxY = *minY = -posY;
 mapCoordinates(rect.topRight(), &posX, &posY, &posZ);
 *maxX = QMAX(*maxX, posX);
 *maxY = QMAX(*maxY, -posY);
 *minX = QMIN(*minX, posX);
 *minY = QMIN(*minY, -posY);
 mapCoordinates(rect.bottomLeft(), &posX, &posY, &posZ);
 *maxX = QMAX(*maxX, posX);
 *maxY = QMAX(*maxY, -posY);
 *minX = QMIN(*minX, posX);
 *minY = QMIN(*minY, -posY);
 mapCoordinates(rect.bottomRight(), &posX, &posY, &posZ);
 *maxX = QMAX(*maxX, posX);
 *maxY = QMAX(*maxY, -posY);
 *minX = QMIN(*minX, posX);
 *minY = QMIN(*minY, -posY);

 *maxX = QMAX(0, *maxX);
 *maxY = QMAX(0, *maxY);
 *minX = QMAX(0, *minX);
 *minY = QMAX(0, *minY);
 *maxX = QMIN((map->width() - 1) * BO_GL_CELL_SIZE, *maxX);
 *minX = QMIN((map->width() - 1) * BO_GL_CELL_SIZE, *minX);
 *maxY = QMIN((map->height() - 1) * BO_GL_CELL_SIZE, *maxY);
 *minY = QMIN((map->height() - 1) * BO_GL_CELL_SIZE, *minY);
 *minY *= -1;
 *maxY *= -1;
}

void BosonBigDisplayBase::generateCellList()
{
 // we need to regenerate the cell list whenever the modelview or the projection
 // matrix changes. then the displayed cells have most probably changed.

 if (!isInitialized()) {
	initGL();
	return;
 }
 if (boGame->gameStatus() == KGame::Init) {
	// we construct the display before the map is received
	return;
 }
 BosonMap* map = mCanvas->map();
 if (!map) {
	boError() << k_funcinfo << "NULL map" << endl;
	return;
 }

 // re-generate the list of to-be-rendered cells:
 Cell* allCells = map->cells();
 if (!allCells) {
	boError() << k_funcinfo << "NULL cells!" << endl;
	return;
 }
 float maxX, maxY;
 float minX, minY;
 calculateWorldRect(QRect(QPoint(0, 0), QPoint(width(), height())), &minX, &minY, &maxX, &maxY);
 minY *= -1;
 maxY *= -1;

 // if everything went fine we need to add those cells that are in the
 // ((minX,minY),(maxX,maxY)) rectangle only.

 int cellMinX = (int)(minX / BO_GL_CELL_SIZE); // AB: *no* +1 for min values!
 int cellMaxX = (int)(maxX / BO_GL_CELL_SIZE) + 1; // +1 because of a modulo (very probably at this point)
 int cellMinY = (int)(minY / BO_GL_CELL_SIZE);
 int cellMaxY = (int)(maxY / BO_GL_CELL_SIZE) + 1;

 // finally we ensure that the cell values are valid, too.
 // after these lines we mustn't modify cellM* anymore!
 cellMinX = QMAX(cellMinX, 0);
 cellMinY = QMAX(cellMinY, 0);
 cellMaxX = QMAX(cellMaxX, 0);
 cellMaxY = QMAX(cellMaxY, 0);
 cellMinX = QMIN(cellMinX, (int)map->width() - 1);
 cellMinY = QMIN(cellMinY, (int)map->height() - 1);
 cellMaxX = QMIN(cellMaxX, (int)map->width() - 1);
 cellMaxY = QMIN(cellMaxY, (int)map->height() - 1);

 int size = (cellMaxX - cellMinX + 1) * (cellMaxY - cellMinY + 1);
 size = QMIN((int)(map->width() * map->height()), size);
 if (size > d->mRenderCellsSize) {
	delete[] d->mRenderCells;
	d->mRenderCells = new Cell*[size];
	d->mRenderCellsSize = size;
 }

 // all cells between those min/max values above might be visible. unfortunately
 // we need to add *all* visible cells to our list, but we need to add as *few*
 // as possible.
 // we could improve speed (important for big maps!) here if we would group
 // several cells into a single sphereInFrustum() call for example.
 //
 // note that the current implementation is very fast at default zoom, but if
 // you zoom out (and therefore there are lots of cells visible) it is still too
 // slow.

 int count = 0;
 for (int x = cellMinX; x <= cellMaxX; x++) {
	for (int y = cellMinY; y <= cellMaxY; y++) {
		// WARNING: x,y MUST be valid!!! there is *no* additional check
		// here!
		Cell* c = &allCells[map->cellArrayPos(x, y)];
		
		GLfloat glX = (float)c->x() * BO_GL_CELL_SIZE + BO_GL_CELL_SIZE / 2;
		GLfloat glY = -((float)c->y() * BO_GL_CELL_SIZE + BO_GL_CELL_SIZE / 2);

		if (sphereInFrustum(BoVector3(glX, glY, c->averageHeight()), c->boundingSphereRadius())) {
			// AB: instead of storing the cell here we should store
			// cell coordinates and create a vertex array with that
			d->mRenderCells[count] = c;
			count++;
		}
	}
 }
 d->mRenderCellsCount = count;
}

void BosonBigDisplayBase::setCamera(const Camera& camera)
{
 d->mCamera = camera;
 cameraChanged();
}

void BosonBigDisplayBase::cameraChanged()
{
 if (!isInitialized()) {
	initGL();
 }
 makeCurrent();

 glMatrixMode(GL_MODELVIEW); // default matrix mode anyway ; redundant!
 glLoadIdentity();

 float diffX, diffY;
 float radius = camera()->radius();
 if (radius <= 0.02) {
	// If radius is 0, up vector will be wrong so we change it
	radius = 0.02;
 }
 pointByRotation(&diffX, &diffY, camera()->rotation(), radius);
 float lookatX, lookatY, lookatZ;  // Point that we look at
 lookatX = camera()->lookAt().x();
 lookatY = camera()->lookAt().y();
 lookatZ = 0.0;
 float eyeX, eyeY, eyeZ;  // Position of camera
 eyeX = lookatX + diffX;
 eyeY = lookatY + diffY;
 eyeZ = lookatZ + camera()->z();
 d->mCameraPos.set(eyeX, eyeY, eyeZ);
 float upX, upY, upZ;  // up vector (points straight up in viewport)
 upX = -diffX;
 upY = -diffY;
 upZ = 0.0;

 gluLookAt(eyeX, eyeY, eyeZ,
		lookatX, lookatY, lookatZ,
		upX, upY, upZ);

 // Reposition light
 glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

 if (checkError()) {
	boError() << k_funcinfo << "after gluLookAt()" << endl;
 }

 // the gluLookAt() above is the most important call for the modelview matrix.
 // everything else will be discarded by glPushMatrix/glPopMatrix anyway (in
 // paintGL()). So we cache the matrix here, for mapCoordinates() and some other
 // stuff
 d->mModelviewMatrix.loadMatrix(GL_MODELVIEW_MATRIX);

 extractFrustum(); // modelview matrix changed
 generateCellList();

 d->mParticlesDirty = true;

 QPoint cellTL; // topleft cell
 QPoint cellTR; // topright cell
 QPoint cellBL; // bottomleft cell
 QPoint cellBR; // bottomright cell
 mapCoordinatesToCell(QPoint(0, 0), &cellTL);
 mapCoordinatesToCell(QPoint(0, d->mViewport[3]), &cellBL);
 mapCoordinatesToCell(QPoint(d->mViewport[2], 0), &cellTR);
 mapCoordinatesToCell(QPoint(d->mViewport[2], d->mViewport[3]), &cellBR);
 emit signalChangeViewport(cellTL, cellTR, cellBL, cellBR);
}

Camera* BosonBigDisplayBase::camera() const
{
 return &d->mCamera;
}

const BoVector3& BosonBigDisplayBase::cameraLookAtPos() const
{
 return d->mCamera.lookAt();
}

bool BosonBigDisplayBase::checkError() const
{
 bool ret = true;
 GLenum e = glGetError();
 switch (e) {
	case GL_INVALID_ENUM:
		boError() << "GL_INVALID_ENUM" << endl;
		break;
	case GL_INVALID_VALUE:
		boError() << "GL_INVALID_VALUE" << endl;
		break;
	case GL_INVALID_OPERATION:
		boError() << "GL_INVALID_OPERATION" << endl;
		break;
	case GL_STACK_OVERFLOW:
		boError() << "GL_STACK_OVERFLOW" << endl;
		break;
	case GL_STACK_UNDERFLOW:
		boError() << "GL_STACK_UNDERFLOW" << endl;
		break;
	case GL_OUT_OF_MEMORY:
		boError() << "GL_OUT_OF_MEMORY" << endl;
		break;
	case GL_NO_ERROR:
		ret = false;
		break;
	default:
		boError() << "Unknown OpenGL Error: " << (int)e << endl;
		break;
 }
 if (e != GL_NO_ERROR) {
	boError() << "Error string: " << (char*)gluErrorString(e) << endl;
 }
 return ret;
}

void BosonBigDisplayBase::setUpdateInterval(unsigned int ms)
{
 boDebug() << k_funcinfo << ms << endl;
 d->mUpdateInterval = ms;
 boProfiling->setGLUpdateInterval(ms);
 QTimer::singleShot(d->mUpdateInterval, this, SLOT(slotUpdateGL()));
}

void BosonBigDisplayBase::calcFPS()
{
 static long long int now = 0;
 struct timeval time;
 gettimeofday(&time, 0);
 now = time.tv_sec * 1000000 + time.tv_usec;
 // FPS is updated once per second
 if ((now - d->mFpsTime) >= 1000000) {
	d->mFps = d->mFrameCount / ((now - d->mFpsTime) / 1000000.0);
	d->mFpsTime = now;
	d->mFrameCount = 0;
//	boDebug() << k_funcinfo << "FPS: " << d->mFps << endl;
 }
}

double BosonBigDisplayBase::fps() const
{
  return d->mFps;
}

void BosonBigDisplayBase::setViewport(int x, int y, GLsizei w, GLsizei h)
{
 if (!isInitialized()) {
	initGL();
 }
 makeCurrent();
 glViewport(x, y, w, h);
 // AB: we could use glGetIntegerv(GL_VIEWPORT, d->mViewport); here. But our own
 // version should be the same
 d->mViewport[0] = x;
 d->mViewport[1] = y;
 d->mViewport[2] = w;
 d->mViewport[3] = h;
}

void BosonBigDisplayBase::extractFrustum()
{
 // modelview or projection matrix was changed (and therefore the frustum).
 GLfloat clip[16];
 GLfloat t;

 const GLfloat* model = d->mModelviewMatrix.data();
 const GLfloat* proj = d->mProjectionMatrix.data();

 // Combine the two matrices (multiply projection by modelview)
 clip[0] = model[0] * proj[0] +
		model[1] * proj[4] +
		model[2] * proj[8] +
		model[3] * proj[12];
 clip[1] = model[0] * proj[1] +
		model[1] * proj[5] +
		model[2] * proj[9] +
		model[3] * proj[13];
 clip[2] = model[0] * proj[2] +
		model[1] * proj[6] +
		model[2] * proj[10] +
		model[3] * proj[14];
 clip[3] = model[0] * proj[3] +
		model[1] * proj[7] +
		model[2] * proj[11] +
		model[3] * proj[15];

 clip[4] = model[4] * proj[0] +
		model[5] * proj[4] +
		model[6] * proj[8] +
		model[7] * proj[12];
 clip[5] = model[4] * proj[1] +
		model[5] * proj[5] +
		model[6] * proj[9] +
		model[7] * proj[13];
 clip[6] = model[4] * proj[2] +
		model[5] * proj[6] +
		model[6] * proj[10] +
		model[7] * proj[14];
 clip[7] = model[4] * proj[3] +
		model[5] * proj[7] +
		model[6] * proj[11] +
		model[7] * proj[15];

 clip[8] = model[8] * proj[0] +
		model[9] * proj[4] +
		model[10] * proj[8] +
		model[11] * proj[12];
 clip[9] = model[8] * proj[1] +
		model[9] * proj[5] +
		model[10] * proj[9] +
		model[11] * proj[13];
 clip[10] = model[8] * proj[2] +
		model[9] * proj[6] +
		model[10] * proj[10] +
		model[11] * proj[14];
 clip[11] = model[8] * proj[3] +
		model[9] * proj[7] +
		model[10] * proj[11] +
		model[11] * proj[15];

 clip[12] = model[12] * proj[0] +
		model[13] * proj[4] +
		model[14] * proj[8] +
		model[15] * proj[12];
 clip[13] = model[12] * proj[1] +
		model[13] * proj[5] +
		model[14] * proj[9] +
		model[15] * proj[13];
 clip[14] = model[12] * proj[2] +
		model[13] * proj[6] +
		model[14] * proj[10] +
		model[15] * proj[14];
 clip[15] = model[12] * proj[3] +
		model[13] * proj[7] +
		model[14] * proj[11] +
		model[15] * proj[15];

 // Extract the numbers for the RIGHT plane
 d->mFrustumMatrix[0][0] = clip[3] - clip[0];
 d->mFrustumMatrix[0][1] = clip[7] - clip[4];
 d->mFrustumMatrix[0][2] = clip[11] - clip[8];
 d->mFrustumMatrix[0][3] = clip[15] - clip[12];

 // Normalize the result
 t = sqrt(d->mFrustumMatrix[0][0] * d->mFrustumMatrix[0][0] +
		d->mFrustumMatrix[0][1] * d->mFrustumMatrix[0][1] +
		d->mFrustumMatrix[0][2] * d->mFrustumMatrix[0][2]);
 d->mFrustumMatrix[0][0] /= t;
 d->mFrustumMatrix[0][1] /= t;
 d->mFrustumMatrix[0][2] /= t;
 d->mFrustumMatrix[0][3] /= t;

 // Extract the numbers for the LEFT plane
 d->mFrustumMatrix[1][0] = clip[3] + clip[0];
 d->mFrustumMatrix[1][1] = clip[7] + clip[4];
 d->mFrustumMatrix[1][2] = clip[11] + clip[8];
 d->mFrustumMatrix[1][3] = clip[15] + clip[12];

 // Normalize the result
 t = sqrt(d->mFrustumMatrix[1][0] * d->mFrustumMatrix[1][0] +
		d->mFrustumMatrix[1][1] * d->mFrustumMatrix[1][1] +
		d->mFrustumMatrix[1][2] * d->mFrustumMatrix[1][2]);
 d->mFrustumMatrix[1][0] /= t;
 d->mFrustumMatrix[1][1] /= t;
 d->mFrustumMatrix[1][2] /= t;
 d->mFrustumMatrix[1][3] /= t;

 // Extract the BOTTOM plane
 d->mFrustumMatrix[2][0] = clip[3] + clip[1];
 d->mFrustumMatrix[2][1] = clip[7] + clip[5];
 d->mFrustumMatrix[2][2] = clip[11] + clip[9];
 d->mFrustumMatrix[2][3] = clip[15] + clip[13];

 // Normalize the result
 t = sqrt(d->mFrustumMatrix[2][0] * d->mFrustumMatrix[2][0] +
		d->mFrustumMatrix[2][1] * d->mFrustumMatrix[2][1] +
		d->mFrustumMatrix[2][2] * d->mFrustumMatrix[2][2]);
 d->mFrustumMatrix[2][0] /= t;
 d->mFrustumMatrix[2][1] /= t;
 d->mFrustumMatrix[2][2] /= t;
 d->mFrustumMatrix[2][3] /= t;

 // Extract the TOP plane
 d->mFrustumMatrix[3][0] = clip[3] - clip[1];
 d->mFrustumMatrix[3][1] = clip[7] - clip[5];
 d->mFrustumMatrix[3][2] = clip[11] - clip[9];
 d->mFrustumMatrix[3][3] = clip[15] - clip[13];

 // Normalize the result
 t = sqrt(d->mFrustumMatrix[3][0] * d->mFrustumMatrix[3][0] +
		d->mFrustumMatrix[3][1] * d->mFrustumMatrix[3][1] +
		d->mFrustumMatrix[3][2] * d->mFrustumMatrix[3][2]);
 d->mFrustumMatrix[3][0] /= t;
 d->mFrustumMatrix[3][1] /= t;
 d->mFrustumMatrix[3][2] /= t;
 d->mFrustumMatrix[3][3] /= t;

 // Extract the FAR plane
d->mFrustumMatrix[4][0] = clip[3] - clip[2];
d->mFrustumMatrix[4][1] = clip[7] - clip[6];
d->mFrustumMatrix[4][2] = clip[11] - clip[10];
d->mFrustumMatrix[4][3] = clip[15] - clip[14];

 // Normalize the result
 t = sqrt(d->mFrustumMatrix[4][0] * d->mFrustumMatrix[4][0] +
		d->mFrustumMatrix[4][1] * d->mFrustumMatrix[4][1] +
		d->mFrustumMatrix[4][2] * d->mFrustumMatrix[4][2]);
 d->mFrustumMatrix[4][0] /= t;
 d->mFrustumMatrix[4][1] /= t;
 d->mFrustumMatrix[4][2] /= t;
 d->mFrustumMatrix[4][3] /= t;

 // Extract the NEAR plane
 d->mFrustumMatrix[5][0] = clip[3] + clip[2];
 d->mFrustumMatrix[5][1] = clip[7] + clip[6];
 d->mFrustumMatrix[5][2] = clip[11] + clip[10];
 d->mFrustumMatrix[5][3] = clip[15] + clip[14];

 // Normalize the result
 t = sqrt(d->mFrustumMatrix[5][0] * d->mFrustumMatrix[5][0] +
		d->mFrustumMatrix[5][1] * d->mFrustumMatrix[5][1] +
		d->mFrustumMatrix[5][2] * d->mFrustumMatrix[5][2]);
 d->mFrustumMatrix[5][0] /= t;
 d->mFrustumMatrix[5][1] /= t;
 d->mFrustumMatrix[5][2] /= t;
 d->mFrustumMatrix[5][3] /= t;
}

float BosonBigDisplayBase::sphereInFrustum(const BoVector3& pos, float radius) const
{
 // FIXME: performance: we might unrull the loop and then make this function
 // inline. We call it pretty often!
 float distance;
 for (int p = 0; p < 6; p++) {
	distance = d->mFrustumMatrix[p][0] * pos[0] + d->mFrustumMatrix[p][1] * pos[1] +
			d->mFrustumMatrix[p][2] * pos[2] + d->mFrustumMatrix[p][3];
	if (distance <= -radius){
		return 0;
	}
 }
 return distance + radius;
}

void BosonBigDisplayBase::mapChanged()
{
 camera()->setMapSize(mCanvas->mapWidth(), mCanvas->mapHeight());
}

const QPoint& BosonBigDisplayBase::cursorCanvasPos() const
{
 return d->mCanvasPos;
}

const BoVector3& BosonBigDisplayBase::cursorCanvasVector() const
{
 return d->mCanvasVector;
}

void BosonBigDisplayBase::slotAdvance(unsigned int /*advanceCount*/, bool)
{
 d->mParticlesDirty = true;
}

void BosonBigDisplayBase::setPlacementPreviewData(const UnitProperties* prop, bool canPlace)
{
 d->mCellPlacementTexture = 0;
 if (!prop) {
	d->mPlacementPreviewProperties = 0;
	d->mPlacementPreviewModel = 0;
	d->mPlacementPreviewCanPlace = false;
	return;
 }
 if (!localPlayer()) {
	boError() << k_funcinfo << "NULL local player" << endl;
	setPlacementPreviewData(0, false);
	return;
 }
 if (!localPlayer()->speciesTheme()) {
	boError() << k_funcinfo << "NULL theme" << endl;
	setPlacementPreviewData(0, false);
	return;
 }
 if (d->mPlacementPreviewProperties != prop) {
	BosonModel* m = localPlayer()->speciesTheme()->unitModel(prop->typeId()); // AB: this does a lookup in a list and therefore should be avoided (this method gets called at least whenever the mouse is moved!)
	if (!m) {
		boError() << k_funcinfo << "NULL model for " << prop->typeId() << endl;
		setPlacementPreviewData(0, false);
		return;
	}
	BoFrame* f = m->frame(0);
	if (!f) {
		boError() << k_funcinfo << "NULL frame 0" << endl;
		setPlacementPreviewData(0, false);
		return;
	}
	d->mPlacementPreviewProperties = prop;
	d->mPlacementPreviewModel = m;
 }
 d->mPlacementPreviewCanPlace = canPlace;
 d->mPlacementCanvasPos = cursorCanvasPos();
}

void BosonBigDisplayBase::setPlacementCellPreviewData(int groundType, bool canPlace)
{
 if (!Cell::isValidGround(groundType)) {
	boWarning() << k_funcinfo << "no valid ground " << groundType << endl;
	setPlacementPreviewData(0, false);
	return;
 }
 BosonTiles* tiles = mCanvas->tileSet();
 if (!tiles) {
	boError() << k_funcinfo << "NULL tiles" << endl;
	setPlacementPreviewData(0, false);
	return;
 }
 BosonTextureArray* textures = tiles->textures();
 if (!textures) {
	boError() << k_funcinfo << "no cell textures available" << endl;
	setPlacementPreviewData(0, false);
	return;
 }
 d->mCellPlacementTexture = textures->texture(Cell::tile(groundType, 0));
 d->mPlacementPreviewCanPlace = canPlace;
 d->mPlacementCanvasPos = cursorCanvasPos();
 d->mPlacementPreviewModel = 0;
}

void BosonBigDisplayBase::setDisplayInput(BosonBigDisplayInputBase* input)
{
 if (d->mInput) {
	boWarning() << k_funcinfo << "input non-NULL" << endl;
	delete d->mInput;
 }
 d->mInput = input;
}

void BosonBigDisplayBase::unitAction(int unitType)
{
 BO_CHECK_NULL_RET(displayInput());
 displayInput()->unitAction(unitType);
}

BosonBigDisplayInputBase* BosonBigDisplayBase::displayInput() const
{
 return d->mInput;
}

void BosonBigDisplayBase::slotMoveSelection(int x, int y)
{
 BO_CHECK_NULL_RET(displayInput());
 displayInput()->slotMoveSelection(x, y);
}

void BosonBigDisplayBase::setDebugMapCoordinates(bool debug)
{
 d->mDebugMapCoordinates = debug;
}

void BosonBigDisplayBase::setDebugShowCellGrid(bool debug)
{
 d->mDebugShowCellGrid = debug;
}

void BosonBigDisplayBase::setDebugMatrices(bool debug)
{
 d->mDebugMatrices = debug;
}

void BosonBigDisplayBase::setDebugItemWorks(bool debug)
{
 d->mDebugItemWorks = debug;
}

void BosonBigDisplayBase::setDebugCamera(bool debug)
{
 d->mDebugCamera = debug;
}

void BosonBigDisplayBase::setDebugRenderCounts(bool debug)
{
 d->mDebugRenderCounts = debug;
}

void BosonBigDisplayBase::setToolTipCreator(int type)
{
 d->mToolTips->setToolTipCreator(type);
}

void BosonBigDisplayBase::setToolTipUpdatePeriod(int ms)
{
 d->mToolTips->setUpdatePeriod(ms);
}

bool BosonBigDisplayBase::boProject(GLfloat x, GLfloat y, GLfloat z, QPoint* pos) const
{
 // AB: once again - most credits go to mesa :)
 BoVector4 v;
 v.setX(x);
 v.setY(y);
 v.setZ(z);
 v.setW(1.0f);

 BoVector4 v2;
 d->mModelviewMatrix.transform(&v2, &v);
 d->mProjectionMatrix.transform(&v, &v2);

 if (v[3] == 0.0f) {
	boError() << k_funcinfo << "Can't divide by zero" << endl;
	return false;
 }
 v2.setX(v[0] / v[3]);
 v2.setY(v[1] / v[3]);
 v2.setZ(v[2] / v[3]);

 pos->setX((int)(d->mViewport[0] + (1 + v2[0]) * d->mViewport[2] / 2));
 pos->setY((int)(d->mViewport[1] + (1 + v2[1]) * d->mViewport[3] / 2));

 // return the actual window y
 pos->setY(d->mViewport[3] - pos->y());
 return true;
}

bool BosonBigDisplayBase::boUnProject(const QPoint& pos, BoVector3* ret, float z) const
{
 // AB: most code is from mesa's gluUnProject().
 BoMatrix A(d->mProjectionMatrix);
 BoMatrix B;

 // A = A x Modelview (== Projection x Modelview)
 A.multiply(&d->mModelviewMatrix);

 // B = A^(-1)
 if (!A.invert(&B)) {
	boError() << k_funcinfo << "Could not invert (Projection x Modelview)" << endl;
	return false;
 }

 // AB: we could calculate the inverse whenever camera changes!
 // --> less inverses to be calculated.

 GLfloat depth = 0.0f;
 GLint realy = d->mViewport[3] - (GLint)pos.y() - 1;
 if (z == -1.0f) {
	glReadPixels(pos.x(), realy, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
 } else {
	depth = z;
 }


 BoVector4 v;
 BoVector4 result;
 v.setX( ((GLfloat)((pos.x() - d->mViewport[0]) * 2)) / d->mViewport[2] - 1.0f);
 v.setY( ((GLfloat)((realy - d->mViewport[1]) * 2)) / d->mViewport[3] - 1.0f);
#if 0
 // mesa uses this
 v.setX( (pos.x() - d->mViewport[0]) * 2 / d->mViewport[2] - 1.0f);
 v.setY( (realy - d->mViewport[1]) * 2 / d->mViewport[3] - 1.0f);
#endif
 v.setZ(2 * depth - 1.0f);
 v.setW(1.0f);
 B.transform(&result, &v);

 if (result[3] == 0.0f) {
	boError() << k_funcinfo << "Can't divide by zero" << endl;
	return false;
 }

 ret->set(result[0] / result[3], result[1] / result[3], result[2] / result[3]);

 return true;
}

void BosonBigDisplayBase::slotRemovedItemFromCanvas(BosonItem* item)
{
 // be careful with the item pointer! this is usually called from the BosonItem
 // destructor, its functions might be destroyed already!
 BO_CHECK_NULL_RET(item);
 d->mToolTips->unsetItem(item);
}

void BosonBigDisplayBase::loadFromXML(const QDomElement& root)
{
 boDebug() << k_funcinfo << endl;
 // Load camera
 QDomElement cam = root.namedItem(QString::fromLatin1("Camera")).toElement();
 if (cam.isNull()) {
	boError(260) << k_funcinfo << "no camera" << endl;
	return;
 }
 camera()->loadFromXML(cam);
 cameraChanged();
 // Load selection
 QDomElement sel = root.namedItem(QString::fromLatin1("Selection")).toElement();
 if (sel.isNull()) {
	boError(260) << k_funcinfo << "no selection" << endl;
	return;
 }
 selection()->loadFromXML(sel, true);
}

void BosonBigDisplayBase::saveAsXML(QDomElement& root)
{
 boDebug() << k_funcinfo << endl;
 QDomDocument doc = root.ownerDocument();
 // Save camera
 QDomElement cam = doc.createElement(QString::fromLatin1("Camera"));
 camera()->saveAsXML(cam);
 root.appendChild(cam);
 // Save current selection
 QDomElement sel = doc.createElement(QString::fromLatin1("Selection"));
 selection()->saveAsXML(sel);
 root.appendChild(sel);
}

