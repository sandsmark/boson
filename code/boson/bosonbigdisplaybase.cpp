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

#include <kgame/kgameio.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kapplication.h>

#include <qimage.h>
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
#define NEAR 1.0 // FIXME: should be > 1.0
#define FAR 100.0


float textureUpperLeft[2] = { 0.0, 1.0 };
float textureLowerLeft[2] = { 0.0, 0.0 };
float textureLowerRight[2] = { 1.0, 0.0 };
float textureUpperRight[2] = { 1.0, 1.0 };

class Camera
{
public:
	Camera()
	{
		mPosX = 0.0;
		mPosY = 0.0;
		mPosZ = 10.0;
		mCenterDiffX = 0.0;
		mCenterDiffY = 0.0;
		mZoomFactor = 1.0;
	}
	Camera(const Camera& c)
	{
		*this = c;
	}
	Camera& operator=(const Camera& c)
	{
		mPosX = c.mPosX;
		mPosY = c.mPosY;
		mPosZ = c.mPosZ;
		mCenterDiffX = c.mCenterDiffX;
		mCenterDiffY = c.mCenterDiffY;
		mZoomFactor= c.mZoomFactor;
		return *this;
	}
	
	void setPos(GLfloat x, GLfloat y, GLfloat z)
	{
		mPosX = x;
		mPosY = y;
		mPosZ = QMAX(NEAR + 1.0, QMIN(FAR - 15.0, z));
	}
	void setX(GLfloat x) { setPos(x, mPosY, mPosZ); }
	void setY(GLfloat y) { setPos(mPosX, y, mPosZ); }
	void setZ(GLfloat z) { setPos(mPosX, mPosY, z); }
	GLfloat x() const { return mPosX; }
	GLfloat y() const { return mPosY; }
	GLfloat z() const { return mPosZ; }
	GLfloat centerX() const { return x() + mCenterDiffX; }
	GLfloat centerY() const { return y() + mCenterDiffY; }

	void setZoomFactor(GLfloat f) { mZoomFactor = f; }
	GLfloat zoomFactor() const { return mZoomFactor; }

	void increaseCenterDiffXBy(GLfloat d) { mCenterDiffX += d; }
	void increaseCenterDiffYBy(GLfloat d) { mCenterDiffY += d; }
private:
	GLfloat mPosX;
	GLfloat mPosY;
	GLfloat mPosZ;
	GLfloat mZoomFactor; // we probably won't use this. mPosZ has the same effect and doesn't cause a "fish eye" perspective

	GLfloat mCenterDiffX;
	GLfloat mCenterDiffY;
};
//int a1, a2;

class SelectionRect
{
public:
	SelectionRect()
	{
		mStartX = mEndX = 0.0;
		mStartY = mEndY = 0.0;
		mStartZ = mEndZ = 0.0;
		mVisible = false;
	}
	
	void start(GLfloat* x, GLfloat* y, GLfloat* z) const
	{
		*x = mStartX;
		*y = mStartY;
		*z = mStartZ;
	}
	void end(GLfloat* x, GLfloat* y, GLfloat* z) const
	{
		*x = mEndX;
		*y = mEndY;
		*z = mEndZ;
	}
	void setStart(GLfloat x, GLfloat y, GLfloat z)
	{
		mStartX = x;
		mStartY = y;
		mStartZ = z;
		setEnd(x, y, z);
	}
	
	void setEnd(GLfloat x, GLfloat y, GLfloat z)
	{
		mEndX = x;
		mEndY = y;
		mEndZ = z;
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
	GLfloat mStartX;
	GLfloat mStartY;
	GLfloat mStartZ;
	GLfloat mEndX;
	GLfloat mEndY;
	GLfloat mEndZ;
	bool mVisible;
};

class MouseMoveDiff
{
public:
	enum Mode {
		ModeNone = 0,
		ModeRMBMove = 1
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
		// we need this mode setting for rmb moving cause the right mouse button click might be an action as well
		mMode = ModeRMBMove;
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
	
	bool isRMBMove() const
	{
		return mode() == ModeRMBMove;
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

		mFramecount = 0;
		mFps = 0;
		mFpsTime = 0;
		mDefaultFont = 0;
	}

	Player* mLocalPlayer;
	QTimer mCursorEdgeTimer;
	int mCursorEdgeCounter;
	KGameMouseIO* mMouseIO;

	BosonGLChat* mChat;

	Camera mCamera;

	GLint mViewport[4]; // x,y,w,h of the viewport. see setViewport
	GLdouble mProjectionMatrix[16];
	GLdouble mModelviewMatrix[16];
	GLdouble mFrustumMatrix[6][4];


	GLfloat mFovY; // see gluPerspective
	GLfloat mAspect; // see gluPerspective

	BosonGLFont* mDefaultFont;// AB: maybe we should support several fonts
	QPtrList<Cell> mRenderCells;

	long long int mFpsTime;
	double mFps;
	int mFramecount;

	bool mEvenFlag; // this is used for a nice trick to avoid clearing the depth 
	                // buffer - see http://www.mesa3d.org/brianp/sig97/perfopt.htm


	bool mInitialized;

	SelectionRect mSelectionRect;
	MouseMoveDiff mMouseMoveDiff;

	QTimer mUpdateTimer;
	int mUpdateInterval;

};

BosonBigDisplayBase::BosonBigDisplayBase(BosonCanvas* c, QWidget* parent)
		: QGLWidget(parent, "bigdisplay")
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
 delete d;
 boDebug() << k_funcinfo << "done" << endl;
}

void BosonBigDisplayBase::init()
{
 d = new BosonBigDisplayBasePrivate;
 mCursor = 0;
 d->mCursorEdgeCounter = 0;
 d->mUpdateInterval = 0;
 d->mInitialized = false;

 mSelection = new BoSelection(this);
 d->mChat = new BosonGLChat(this);

 slotResetViewProperties();
 for (int i = 0; i < 4; i++) {
	d->mViewport[i] = 0;
 }
 for (int i = 0; i < 16; i++) {
	d->mProjectionMatrix[i] = 0.0;
	d->mModelviewMatrix[i] = 0.0;
 }
 for (int i = 0; i < 6; i++) {
	for (int j = 0; j < 4; j++) {
		d->mFrustumMatrix[i][j] = 0.0;
	}
 }

 if (!isValid()) {
	boError() << k_funcinfo << "No OpenGL support present on this system??" << endl;
	return;
 }
 
 // and another hack..
// setMinimumSize(QSize(400,400));

 glInit();
 connect(&d->mUpdateTimer, SIGNAL(timeout()), this, SLOT(updateGL()));

 connect(&d->mCursorEdgeTimer, SIGNAL(timeout()), 
		this, SLOT(slotCursorEdgeTimeout()));

 //TODO: sprite tooltips

 setUpdateInterval(boConfig->updateInterval());
}

void BosonBigDisplayBase::initializeGL()
{
 if (d->mInitialized) {
	// already called initializeGL()
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
 glShadeModel(GL_FLAT); // GL_SMOOTH is default - but esp. in software rendering way slower. in hardware it *may* be equal (concerning speed) to GL_FLAT
 glDisable(GL_DITHER); // we don't need this (and its enabled by default)

 // for anti-aliased lines (currently unused):
 glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
 // for anti-aliased points (currently unused):
 glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST);
 // for anti-aliased polygons (currently unused):
 glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);

 glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);

 if (checkError()) {
	boError() << k_funcinfo << endl;
 }

 struct timeval time;
 gettimeofday(&time, 0);
 d->mFpsTime = time.tv_sec * 1000000 + time.tv_usec;

 // this needs to be done in initializeGL():
 d->mDefaultFont = new BosonGLFont(QString::fromLatin1("fixed"));

 // the actual GL initializing should be done now.
 d->mInitialized = true;

 // this is usually done by QT anyway - except if the window was hidden when
 // loading was completed (for example). But we need to initialize at least the
 // depth buffer...
 // AB: this might even fix our resize-problem (see slotHack1() in BosonWidget)
 resizeGL(width(), height());

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
 glGetDoublev(GL_PROJECTION_MATRIX, d->mProjectionMatrix);
 extractFrustum(); // projection matrix changed
 generateCellList();
 glMatrixMode(GL_MODELVIEW);


 glClearDepth(1.0);
 glClear(GL_DEPTH_BUFFER_BIT);
 glDepthRange(0.0, 0.5);
 glDepthFunc(GL_LESS);
 d->mEvenFlag = true;


 // update the minimap
 setCamera(d->mCamera);

 if (checkError()) {
	boError() << k_funcinfo << endl;
 }
}

void BosonBigDisplayBase::paintGL()
{
 if (!d->mInitialized) {
	glInit();
	return;
 }
 if (boGame->gameStatus() == KGame::Init) {
	return;
 }
 if (!localPlayer()) {
	boError() << k_funcinfo << "NULL local player" << endl;
 }
 boProfiling->render(true);
 d->mUpdateTimer.stop();
//boDebug() << k_funcinfo << endl;
 // TODO: use 0,0 as lower left, instead of top left
 // AB: I've dalayed this. since we still use canvas-coordinates it is easier
 // and more logical to use 0,0 as top left.
 // AB: this is the most time-critical function! we need to enhance performance
 // whenever possible. look at
 // http://www.mesa3d.org/brianp/sig97/perfopt.htm
 // for perfomance optimizations

 // TODO: e.g. remember than code between glBegin() and glEnd() must be send to
 // the hardware as soon as possible - avaoid any code between them.

 // TODO: performance: make textures resident (how to do this?)
 // maybe use priorities to achieve this
 // TODO: performance: maybe enable depth-buffer/depth-testing after the cells
 // have been drawn - so useless cell-drawings can be discarded. remember to
 // disable it before drawing the background (i.e. cells) !
 // FIXME: remove the glGetError() calls in releases and so on, since they slow
 // rendering down, like all getGet*'s
 // TODO: performance: from the URL above:
 // Transparency may be implemented with stippling instead of blending 
 // If you need simple transparent objects consider using polygon stippling
 // instead of alpha blending. The later is typically faster and may actually
 // look better in some situations. [L,S] 
 // --> what is this "stippling" and can we use it?
 // TODO: performance: we might want to replace QT's makeCurrent() function - it
 // is virtual and we don't use different overlays anyway
 // AB: performance: don't use functions that take double-precision floating
 // point arguments (i.e. GLfloat) if there are single precisions (GLfloat)
 // available. mesa uses float internally and therefore needs to convert...

 // TODO: performance: we'll probably need the depth buffer soon - there is a
 // nice trick to avoid clearing it. see
 // http://www.mesa3d.org/brianp/sig97/perfopt.htm
 // in 3.5!

 // TODO: performance: sort by z-coordinates. not just the units, but also
 // cells! i.e. render cells *after* units!

 boProfiling->renderClear(true);
 glClear(GL_COLOR_BUFFER_BIT);
 boProfiling->renderClear(false);

 glColor3f(1.0, 1.0, 1.0);

 // the guy who wrote http://www.mesa3d.org/brianp/sig97/perfopt.htm is *really* clever!
 // this trick avoids clearing the depth buffer:
 if (d->mEvenFlag) {
	glDepthFunc(GL_LESS);
	glDepthRange(0.0, 0.5);
 } else {
	glDepthFunc(GL_GREATER);
	glDepthRange(1.0, 0.5);
 }
 d->mEvenFlag = !d->mEvenFlag;
 float elapsed = calcFPS();

 // note: we don't call gluLookAt() here because of performance. instead we just
 // push the matrix here and pop it at the end of paintGL() again. gluLookAt()
 // is called only whenever setCamera() is called.
 glPushMatrix();

 // FIXME: put these next to gluLookAt()
// glRotatef(a1, 0.0, 0.0, 1.0);
// glRotatef(a2, 0.0, 1.0, 0.0);

 if (checkError()) {
	boError() << k_funcinfo << "before unit rendering" << endl;
 }

 glEnable(GL_TEXTURE_2D);
 glEnable(GL_DEPTH_TEST);

 boProfiling->renderUnits(true);
 BoItemList allItems = mCanvas->allBosonItems();
 BoItemList::Iterator it = allItems.begin();
 unsigned int renderedUnits = 0;
 glEnable(GL_CULL_FACE);
 glCullFace(GL_BACK);
 for (; it != allItems.end(); ++it) {
	//FIXME: order by z-coordinates! first those which are
	//closer to surface, then flying units

	BosonItem* item = *it;

	GLfloat x = (item->x() + item->width() / 2) * BO_GL_CELL_SIZE / BO_TILE_SIZE;
	GLfloat y = -((item->y() + item->height() / 2) * BO_GL_CELL_SIZE / BO_TILE_SIZE);
	GLfloat z = item->z() * BO_GL_CELL_SIZE / BO_TILE_SIZE;

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
	// depend on it in pathfinding we'd soon have a brolen network (think
	// about differnt CPUs with different rounding values for floating
	// point calculations)
	if (!sphereInFrustum(x, y, z, item->boundingSphereRadius())) {
		// the unit is not visible, currently. no need to draw anything.
		continue;
	}

	// AB: note units are rendered in the *center* point of their
	// width/height.
	// but concerning z-position they are rendered from bottom to top!

	if (item->displayList() == 0) {
		boWarning() << k_funcinfo << "NULL display list for item rtti=" << item->rtti() << endl;
		continue;
	}

	// FIXME: we have to copy the complete list of cells here, since it is
	// calculated on the fly by cells()! nicer version would be to add
	// BosonPrite::isFogged() and check it there.
	QPointArray cells = item->cells();
	bool visible = false;
	for (unsigned int i = 0; i < cells.count(); i++) {
		if (!localPlayer()->isFogged(cells[i].x(), cells[i].y())) {
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

	// FIXME: performance: we could create a displaylist that contains the selectbox and simply change item->displayList()
	// when the item is selected/unselected
	glCallList(item->displayList());
	glPopMatrix();

	if (item->isSelected()) {
		// FIXME: performance: create a display lists in the SelectBox which also contains the scale!
		GLfloat w = ((float)item->width()) * BO_GL_CELL_SIZE / BO_TILE_SIZE;
		GLfloat h = ((float)item->height()) * BO_GL_CELL_SIZE / BO_TILE_SIZE;
		GLfloat depth = item->glDepthMultiplier();
		glPushMatrix();
		if (w != 1.0 || h != 1.0 || depth != 1.0) {
			glScalef(w, h, depth);
		}
		glCallList(item->selectBox()->displayList());
		glPopMatrix();
	}

	glTranslatef(-x, -y, -z);
	renderedUnits++;
 }
 glDisable(GL_CULL_FACE);
 boProfiling->renderUnits(false, renderedUnits);

 if (checkError()) {
	boError() << k_funcinfo << "after unit rendering" << endl;
 }

 boProfiling->renderCells(true);
 renderCells();
 boProfiling->renderCells(false);

 if (checkError()) {
	boError() << k_funcinfo << "cells rendered" << endl;
 }

 // Render particle systems
 boProfiling->renderParticles(true);
 canvas()->updateParticleSystems(elapsed);
 int count = canvas()->particleSystemsCount();
 if (count > 0) {
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	QPtrListIterator<BosonParticleSystem> it(*(canvas()->particleSystems()));
	BosonParticleSystem* s;
	while ((s = it.current()) != 0) {
		++it;
		s->draw();
	}
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_BLEND);
 }
 boProfiling->renderParticles(false);

 if (checkError()) {
	boError() << k_funcinfo << "when particles rendered" << endl;
 }


 glDisable(GL_DEPTH_TEST);

 boProfiling->renderText(true); // AB: actually this is text and cursor

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

 // now restore the old 3D-matrix
 glMatrixMode(GL_PROJECTION);
 glPopMatrix();
 glMatrixMode(GL_MODELVIEW);
 glPopMatrix();
 boProfiling->renderText(false);



 if (d->mSelectionRect.isVisible()) {
	glPushMatrix();
	GLfloat x, y, w, h;
	GLfloat x1, y1, x2, y2;
	GLfloat z; // currently the z-coordinate of the rect is not used - we set our own below

	glColor3f(1.0, 0.0, 0.0); // FIXME hardcoded
	
	d->mSelectionRect.start(&x1, &y1, &z);
	d->mSelectionRect.end(&x2, &y2, &z);

	x = QMIN(x1, x2);
	y = QMAX(y1, y2);
	w = QABS(x1 - x2);
	h = QABS(y1 - y2);
	z = 0.0;

	glTranslatef(x, y - h, z);
	glBegin(GL_LINE_LOOP);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(0.0, h, 0.0);
		glVertex3f(w, h, 0.0);
		glVertex3f(w, 0.0, 0.0);
	glEnd();

	glColor3f(1.0, 1.0, 1.0);
	glPopMatrix();
 }
 if (checkError()) {
	boError() << k_funcinfo << "selection rect rendered" << endl;
 }

 if (d->mUpdateInterval) {
	d->mUpdateTimer.start(d->mUpdateInterval);
 }

 glPopMatrix();

 boProfiling->render(false);
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
 int w = QMAX(d->mDefaultFont->metrics()->width(minerals), d->mDefaultFont->metrics()->width(oil));
 int x = d->mViewport[2] - w - border;
 int y = d->mViewport[3] - d->mDefaultFont->height() - border;

 // Alpha-blended rectangle
 glEnable(GL_BLEND);
 glColor4f(0.0, 0.0, 0.0, 0.5);
 glRecti(x - alphaborder, d->mViewport[3] - border + alphaborder,
		d->mViewport[2] - border + alphaborder, d->mViewport[3] - (2 * d->mDefaultFont->height()) - border - alphaborder);
 glColor3f(1.0, 1.0, 1.0);
 glRasterPos2i(x, y);
 glCallLists(minerals.length(), GL_UNSIGNED_BYTE, (GLubyte*)minerals.latin1());
 y -= d->mDefaultFont->height();
 glRasterPos2i(x, y);
 glCallLists(oil.length(), GL_UNSIGNED_BYTE, (GLubyte*)oil.latin1());

// now the chat messages
 d->mChat->renderMessages(border, border, d->mDefaultFont);

 glColor3f(1.0, 1.0, 1.0);
 glDisable(GL_BLEND);
}


void BosonBigDisplayBase::renderCells()
{
 BosonTiles* tiles = mCanvas->tileSet();
 if (!tiles) {
	boError() << k_funcinfo << "NULL tiles" << endl;
	return;
 }
 BosonTextureArray* textures = mCanvas->tileSet()->textures();
 if (!textures) {
	makeCurrent();
	tiles->generateTextures();
	textures = tiles->textures();
	if (!textures) {
		boWarning() << k_funcinfo << "NULL textures for cells" << endl;
		return;
	}
 }

 if (d->mRenderCells.count() == 0) {
	// this happens either when we have to generate the list first or if no
	// cell is visible at all. The latter case isn't speed relevant, so we
	// can simply re-generate then.
	generateCellList();
 }

 QPtrListIterator<Cell> cellIt(d->mRenderCells);
 GLuint texture = 0;
 int tile = -1;
 for (; cellIt.current(); ++cellIt) {
	Cell* c = cellIt.current();
	if (localPlayer()->isFogged(c->x(), c->y())) {
		// don't draw anything at all. the cell will just be black,
		// because of the glClear() call.
		continue;
	}
	GLfloat cellXPos = (float)c->x() * BO_GL_CELL_SIZE;
	GLfloat cellYPos = -(float)c->y() * BO_GL_CELL_SIZE;
	if (c->tile() != tile) {
		texture = textures->texture(c->tile());
		tile = c->tile();
		glBindTexture(GL_TEXTURE_2D, texture);

	}
	// FIXME: performance: only a single glBegin(GL_QUADS)!
	glBegin(GL_QUADS);
		glTexCoord2fv(textureUpperLeft); glVertex3f(cellXPos, cellYPos, 0.0);
		glTexCoord2fv(textureLowerLeft); glVertex3f(cellXPos, cellYPos - BO_GL_CELL_SIZE, 0.0);
		glTexCoord2fv(textureLowerRight); glVertex3f(cellXPos + BO_GL_CELL_SIZE, cellYPos - BO_GL_CELL_SIZE, 0.0);
		glTexCoord2fv(textureUpperRight); glVertex3f(cellXPos + BO_GL_CELL_SIZE, cellYPos, 0.0);
	glEnd();
 }
}

void BosonBigDisplayBase::slotMouseEvent(KGameIO* , QDataStream& stream, QMouseEvent* e, bool *eatevent)
{
 GLdouble posX, posY, posZ;
 if (!mapCoordinates(e->pos(), &posX, &posY, &posZ)) {
	boError() << k_funcinfo << "Cannot map coordinates" << endl;
	return;
 }
 QPoint canvasPos;
 worldToCanvas(posX, posY, posZ, &canvasPos);

 switch (e->type()) {
	case QEvent::Wheel:
	{
		// FIXME: use wheel for zooming
		QWheelEvent* wheel = (QWheelEvent*)e;
		int x, y;

		float delta = -wheel->delta() / 120;//120: see QWheelEvent::delta()
		if (wheel->state() & ControlButton) {
			x = width();
			y = height();
		} else {
			x = 20;
			y = 20;
			delta *= QApplication::wheelScrollLines();
		}

		if (wheel->orientation() == Horizontal) {
			x *= (int)delta;
			y = 0;
		} else {
			x = 0;
			y *= (int)delta;
		}
		scrollBy(x, y);
		e->accept();
		break;
	}
	case QEvent::MouseMove:
	{
		d->mMouseMoveDiff.moveToPos(e->pos());
		if (e->state() & AltButton) {
			// The Alt button is the camera modifier in boson.
			// Better don't do important stuff (like unit movement
			// or selections) here, since a single push on Alt gives
			// the focus to the mneu which might be very confusing
			// during a game.
			if (e->state() & LeftButton) {
				Camera camera = d->mCamera;
				camera.setZ(cameraZ() + d->mMouseMoveDiff.dy());
				setCamera(camera);
				boDebug() << "posZ: " << d->mCamera.z() << endl;
			} else if (e->state() & RightButton) {
				Camera camera = d->mCamera;
				camera.increaseCenterDiffXBy(d->mMouseMoveDiff.dx());
				camera.increaseCenterDiffYBy(-d->mMouseMoveDiff.dy());
//				a1 += d->mMouseMoveDiff.dx();
//				a2 += d->mMouseMoveDiff.dy();
				setCamera(camera);
			}
		} else if (e->state() & LeftButton) {
			if (e->state() & ControlButton) {
				// not yet used
			} else {
				// selection rect gets drawn for both - shift
				// down and no modifier pressed.
				d->mSelectionRect.setVisible(true);
				moveSelectionRect(posX, posY, posZ);
			}
		} else if (e->state() & RightButton) {
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
//				static bool a = false;
//				if (a) {
//					a = false;
//					break;
//				}
//				a = true;
//				QPoint pos = mapToGlobal(QPoint(d->mMouseMoveDiff.oldX(), d->mMouseMoveDiff.oldY()));
//				QCursor::setPos(pos);
				d->mMouseMoveDiff.startRMBMove();
				GLdouble dx, dy;
				int moveX = d->mMouseMoveDiff.dx();
				int moveY = d->mMouseMoveDiff.dy();
				mapDistance(moveX, moveY, &dx, &dy);
				Camera camera = d->mCamera;
				camera.setX(cameraX() + dx);
				camera.setY(cameraY() + dy);
				setCamera(camera);
			} else {
				d->mMouseMoveDiff.stop();
			}
		}
		updateCursor();
		e->accept();
		break;
	}
	case QEvent::MouseButtonDblClick:
		makeActive();
		if (e->button() == LeftButton) {
			bool replace = !(e->state() & ShiftButton);
			Unit* unit = ((BosonCanvas*)canvas())->findUnitAt(canvasPos);
			if (unit) {
				if (!selectAll(unit->unitProperties(), replace)) {
					selection()->selectUnit(unit, replace);
				}
			}
		}
		e->accept();
		break;
	case QEvent::MouseButtonPress:
		makeActive();
		if (e->button() == LeftButton) {
			if (actionLocked()) {
				// If action is locked then it means that user clicked on an action
				// button and wants to perform specific action
				// AB: IMHO this fits better in RMB-Pressed.
				// mixing LMB and RMB for actions is very
				// confusing
				bool send = false;
				BoAction action;
				action.setWorldPos(posX, posY, posZ);
				action.setCanvasPos(canvasPos);
				actionClicked(action, stream, &send);
				if (send) {
					*eatevent = true;
				}
			} else {
				d->mSelectionRect.setStart(posX, posY, posZ);
			}
		} else if (e->button() == MidButton) {
			if (boConfig->mmbMove()) {
				int cellX, cellY;
				cellX = (int)(posX / BO_GL_CELL_SIZE);
				cellY = (int)(-posY / BO_GL_CELL_SIZE);
				slotReCenterDisplay(QPoint(cellX, cellY));
				updateCursor();
			}
		} else if (e->button() == RightButton) {
			if (boConfig->rmbMove()) {
				//AB: this might be obsolete..
				d->mMouseMoveDiff.moveToPos(e->pos()); // set position, but do not yet start!
			}
		}
		e->accept();
		break;
	case QEvent::MouseButtonRelease:
		if (e->state() & AltButton) {
			// the camera modifier. no cleanups to be done.
		} else if (e->button() == LeftButton) {
			if (e->state() & ControlButton) {
				// unused
			} else if (e->state() & ShiftButton) {
				removeSelectionRect(false);
			} else {
				removeSelectionRect(true);
			}
		} else if (e->button() == RightButton) {
			if (d->mMouseMoveDiff.isRMBMove()) {
				d->mMouseMoveDiff.stop();
			} else {
				//TODO: port editor to KGameIO - otherwise
				//eatevent and send are useless

			 	// AB: is *eatevent the correct parameter? KGame should
				// *not* send the stream if this is false!
				bool send = false;
				BoAction action;
				action.setCanvasPos(canvasPos);
				action.setWorldPos(posX, posY, posZ);
				if (e->state() & ControlButton) {
					action.setForceAttack(true);
				}
			 	actionClicked(action, stream, &send);
				if (send) {
					*eatevent = true;
				}
			}
		}
		e->accept();
		break;
	default:
		boWarning() << "unexpected mouse event " << e->type() << endl;
		e->ignore();
		return;
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
 //TODO
 // find the command center of the local player
 QPoint pos(0, 0); // note: we use *cell* coordinates!

 slotReCenterDisplay(pos);
}

void BosonBigDisplayBase::slotResetViewProperties()
{
 d->mFovY = 60.0;
 d->mAspect = 1.0;
 setCamera(Camera());
 resizeGL(d->mViewport[2], d->mViewport[3]);
// a1 = 0;
// a2 = 0;
}

void BosonBigDisplayBase::slotReCenterDisplay(const QPoint& pos)
{
//TODO don't center the corners - e.g. 0;0 should be top left, never center 
 Camera camera = d->mCamera;
 camera.setX(((float)pos.x()) * BO_GL_CELL_SIZE);
 camera.setY(-((float)pos.y()) * BO_GL_CELL_SIZE);
 setCamera(camera);
}

void BosonBigDisplayBase::worldToCanvas(GLfloat x, GLfloat y, GLfloat /*z*/, QPoint* pos) const
{
 pos->setX((int)(x / BO_GL_CELL_SIZE * BO_TILE_SIZE));
 pos->setY((int)(-y / BO_GL_CELL_SIZE * BO_TILE_SIZE));
}

bool BosonBigDisplayBase::mapCoordinates(const QPoint& pos, GLdouble* posX, GLdouble* posY, GLdouble* posZ) const
{
 GLint realy = d->mViewport[3] - (GLint)pos.y() - 1;

 // we basically calculate a line here .. nearX/Y/Z is the starting point,
 // farX/Y/Z is the end point. From these points we can calculate a direction.
 // using this direction and the points nearX(Y)/farX(Y) you can build triangles
 // and then find the point that is on z=0.0
 GLdouble nearX, nearY, nearZ;
 GLdouble farX, farY, farZ;
 if (!gluUnProject((GLdouble)pos.x(), (GLdouble)realy, 0.0,
		d->mModelviewMatrix, d->mProjectionMatrix, d->mViewport,
		&nearX, &nearY, &nearZ)) {
	return false;
 }
 if (!gluUnProject((GLdouble)pos.x(), (GLdouble)realy, 1.0,
		d->mModelviewMatrix, d->mProjectionMatrix, d->mViewport,
		&farX, &farY, &farZ)) {
	return false;
 }

 // simple maths .. however it took me pretty much time to do this.. I haven't
 // done this for way too long time!
 GLdouble tanAlphaX = (nearX - farX) / (nearZ - farZ);
 *posX = (GLfloat)(nearX - tanAlphaX * nearZ);

 GLdouble tanAlphaY = (nearY - farY) / (nearZ - farZ);
 *posY = (GLfloat)(nearY - tanAlphaY * nearZ);

 // AB: what should we do with posZ ??

 return true;
}

bool BosonBigDisplayBase::mapDistance(int windx, int windy, GLdouble* dx, GLdouble* dy) const
{
 GLdouble moveZ; // unused
 GLdouble moveX1, moveY1;
 GLdouble moveX2, moveY2;
 if (!mapCoordinates(QPoint(0, 0), &moveX1, &moveY1, &moveZ)) {
	boError() << k_funcinfo << "Cannot map coordinates" << endl;
	return false;
 }
 if (!mapCoordinates(QPoint(windx, windy), &moveX2, &moveY2, &moveZ)) {
	boError() << k_funcinfo << "Cannot map coordinates" << endl;
	return false;
 }
 *dx = moveX2 - moveX1;
 *dy = moveY2 - moveY1;
 return true;
}

bool BosonBigDisplayBase::mapCoordinatesToCell(const QPoint& pos, QPoint* cell)
{
 GLdouble x, y, z;
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
 return QGLWidget::eventFilter(o, e);
}

bool BosonBigDisplayBase::selectAll(const UnitProperties* prop, bool replace)
{
 if (!localPlayer()) {
	boError() << k_funcinfo << "NULL localplayer" << endl;
	return false;
 }
 if (prop->isFacility()) {
	// we don't select all facilities, but only the one that was
	// double-clicked. it makes no sense for facilities
	return false;
 }
 QPtrList<Unit> allUnits = localPlayer()->allUnits();
 QPtrList<Unit> list;
 QPtrListIterator<Unit> it(allUnits);
 while (it.current()) {
	if (it.current()->unitProperties() == prop) {
		list.append(it.current());
	}
	++it;
 }
 selection()->selectUnits(list, replace);
 return true;
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


void BosonBigDisplayBase::selectionStart(GLfloat* x, GLfloat* y, GLfloat* z) const
{
 d->mSelectionRect.start(x, y, z);
}

void BosonBigDisplayBase::selectionEnd(GLfloat* x, GLfloat* y, GLfloat* z) const
{
 d->mSelectionRect.end(x, y, z);
}

void BosonBigDisplayBase::startSelection(GLdouble x, GLdouble y, GLdouble z)
{
 QPoint canvasPos;
 worldToCanvas(x, y, z, &canvasPos);
 /*
 Unit* unit = canvas()->findUnitAt(canvasPos);
 boDebug() << k_funcinfo << x << " " << y << " " << z << endl;
 if (!unit) {*/
	// nothing has been found - its a ground click
	// Here we have to draw the selection rect
	d->mSelectionRect.setStart(x, y, z);
	d->mSelectionRect.setVisible(true);
	return;
	/*
 }

 boDebug() << k_funcinfo << "unit" << endl;
 selection()->selectUnit(unit);

 // cannot be placed into mSelection cause we don't have localPlayer
 // there
 if (localPlayer() == unit->owner()) {
	unit->playSound(SoundOrderSelect);
 }
 */
}

void BosonBigDisplayBase::moveSelectionRect(GLfloat x, GLfloat y, GLfloat z)
{
 if (d->mSelectionRect.isVisible()) {
	d->mSelectionRect.setEnd(x, y, z);
	// would be great here but is a huge performance
	// problem (therefore in releasebutton):
	// selectArea();
 }
}

void BosonBigDisplayBase::removeSelectionRect(bool replace)
{
 if (d->mSelectionRect.isVisible()) {
	// here as there is a performance problem in
	// mousemove:
	selectArea(replace);

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
	d->mSelectionRect.start(&x, &y, &z);
	QPoint canvasPos;
	worldToCanvas(x, y, z, &canvasPos);
	Unit* unit = 0l;
	if(!localPlayer()->isFogged(canvasPos.x() / BO_TILE_SIZE, canvasPos.x() / BO_TILE_SIZE)) {
	unit = canvas()->findUnitAt(canvasPos);
	}
	if (unit) {
		boDebug() << k_funcinfo << "unit" << endl;
		selection()->selectUnit(unit, replace);
		// cannot be placed into mSelection cause we don't have localPlayer
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

void BosonBigDisplayBase::selectArea(bool replace)
{
 if (!d->mSelectionRect.isVisible()) {
	boDebug() << k_funcinfo << "no rect" << endl;
	return;
 }
 if (boConfig->debugMode() == BosonConfig::DebugSelection) {
	BoItemList list;
	QRect r = selectionRectCanvas();
	list = canvas()->bosonCollisions(r);
	BoItemList::Iterator it;
	boDebug() << "Selection count: " << list.count() << endl;
	for (it = list.begin(); it != list.end(); ++it) {
		QString s = QString("Selected: RTTI=%1").arg((*it)->rtti());
		if (RTTI::isUnit((*it)->rtti())) {
			Unit* u = (Unit*)*it;
			s += QString(" Unit ID=%1").arg(u->id());
			if (u->isDestroyed()) {
				s += QString("(destroyed)");
			}
		}
		boDebug() << s << endl;
	}
 }

 QRect r = selectionRectCanvas();
 BoItemList list;
 QPtrList<Unit> unitList;
 Unit* fallBackUnit= 0; // in case no localplayer mobile unit can be found we'll select this instead
 BoItemList::Iterator it;
 list = canvas()->bosonCollisions(r);
 for (it = list.begin(); it != list.end(); ++it) {
	if (!RTTI::isUnit((*it)->rtti())) {
		continue;
	}
	Unit* unit = (Unit*)*it;
	if (unit->isDestroyed()) {
		continue;
	}
	if (unit->unitProperties()->isMobile()) {
		if (unit->owner() == localPlayer()) {
			unitList.append(unit);
		} else {
			fallBackUnit = unit;
		}
	} else {
		fallBackUnit = unit; 
	}
	
 }

 if (unitList.count() > 0) {
	boDebug() << "select " << unitList.count() << " units" << endl;
	selection()->selectUnits(unitList, replace);
 } else if (fallBackUnit) {
	selection()->selectUnit(fallBackUnit, replace);
 } else {
	if (replace) {
		selection()->clear();
	}
 }
}


QRect BosonBigDisplayBase::selectionRectCanvas() const
{
 QPoint start, end;
 GLfloat startx, starty, startz, endx, endy, endz;
 selectionStart(&startx, &starty, &startz);
 selectionEnd(&endx, &endy, &endz);
 worldToCanvas(startx, starty, startz, &start);
 worldToCanvas(endx, endy, endz, &end);
 QRect r(start, end);
 r = r.normalize();
 return r;
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
 GLdouble dx, dy;
 mapDistance(move, move, &dx, &dy);
 GLfloat moveX = (GLfloat)dx;
 GLfloat moveY = (GLfloat)dy;
 if (pos.x() <= sensity && pos.x() > -1) {
	x = -moveX;
 } else if (pos.x() >= w->width() - sensity && pos.x() <= w->width()) {
	x = moveX;
 }
 if (pos.y() <= sensity && pos.y() > -1) {
	y = -moveY;
 } else if (pos.y() >= w->height() - sensity && pos.y() <= w->height()) {
	y = moveY;
 }
 if (!x && !y || !sensity) {
	d->mCursorEdgeTimer.stop();
	d->mCursorEdgeCounter = 0;
 } else {
	if (!d->mCursorEdgeTimer.isActive()) {
		d->mCursorEdgeTimer.start(20);
	}
	d->mCursorEdgeCounter++;
	if (d->mCursorEdgeCounter > 30) {
		Camera camera = d->mCamera;
		camera.setX(cameraX() + x);
		camera.setY(cameraY() + y);
		setCamera(camera);
	}
 }
}


void BosonBigDisplayBase::scrollBy(int dx, int dy)
{
 GLdouble x, y;
 mapDistance(dx, dy, &x, &y);
 Camera camera = d->mCamera;
 camera.setX(cameraX() + x);
 camera.setY(cameraY() + y);
 setCamera(camera);
}

void BosonBigDisplayBase::generateCellList()
{
 // we need to regenerate the cell list whenever the modelview or the projection
 // matrix changes. then the displayed cells have most probably changed.

 // some clever guy made QGLContet::initialized() protected - so we need to
 // implement our own version here :-(
 if (!d->mInitialized) {
	glInit();
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
 d->mRenderCells.clear();
 Cell* allCells = map->cells();
 float radius = sqrt(2 * (BO_GL_CELL_SIZE/2) * (BO_GL_CELL_SIZE/2));
 for (unsigned int i = 0; i < map->width() * map->height(); i++) {
	Cell* c = &allCells[i];
	GLfloat x = (float)c->x() * BO_GL_CELL_SIZE + BO_GL_CELL_SIZE / 2;
	GLfloat y = -((float)c->y() * BO_GL_CELL_SIZE + BO_GL_CELL_SIZE / 2);
	
	if (sphereInFrustum(x, y, 0.0, radius)) {
		d->mRenderCells.append(c);
	}
 }
}

void BosonBigDisplayBase::setCamera(const Camera& camera)
{
 if (!d->mInitialized) {
	glInit();
 }
 makeCurrent();
 d->mCamera = camera;

 glMatrixMode(GL_MODELVIEW); // default matrix mode anyway ; redundant!
 glLoadIdentity();

 float upX, upY, upZ;
 upX = 0.0;
 upY = 1.0;
 upZ = 0.0;
 float centerX, centerY, centerZ;
 centerX = d->mCamera.centerX();
 centerY = d->mCamera.centerY();
 centerZ = -100.0;
// centerZ = d->mPosZ;
// glRotatef(a1, 0.0, 0.0, 1.0);
// glRotatef(a2, 1.0, 0.0, 0.0);
 gluLookAt(cameraX(), cameraY(), cameraZ(), 
		centerX, centerY, centerZ, 
		upX, upY, upZ);
 if (checkError()) {
	boError() << k_funcinfo << "after gluLookAt()" << endl;
 }

 // the gluLookAt() above is the most important call for the modelview matrix.
 // everything else will be discarded by glPushMatrix/glPopMatrix anyway (in
 // paintGL()). So we cache the matrix here, for mapCoordinates()
 glGetDoublev(GL_MODELVIEW_MATRIX, d->mModelviewMatrix);
 extractFrustum(); // modelview matrix changed
 generateCellList();

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

GLfloat BosonBigDisplayBase::cameraX() const
{
 return d->mCamera.x();
}

GLfloat BosonBigDisplayBase::cameraY() const
{
 return d->mCamera.y();
}

GLfloat BosonBigDisplayBase::cameraZ() const
{
 return d->mCamera.z();
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
	boError() << "Error string: " << gluErrorString(e) << endl;
 }
 return ret;
}

void BosonBigDisplayBase::setUpdateInterval(unsigned int ms)
{
 boDebug() << k_funcinfo << ms << endl;
 d->mUpdateInterval = ms;
 QTimer::singleShot(d->mUpdateInterval, this, SLOT(updateGL()));
}

float BosonBigDisplayBase::calcFPS()
{
 float elapsed;
 static long long int now = 0;
 struct timeval time;
 gettimeofday(&time, 0);
 elapsed = (time.tv_sec * 1000000 + time.tv_usec) - now;
 elapsed /= 1000000;
 now = time.tv_sec * 1000000 + time.tv_usec;
 // FPS is updated once per second
 if ((now - d->mFpsTime) >= 1000000) {
	d->mFps = d->mFramecount / ((now - d->mFpsTime) / 1000000.0);
	d->mFpsTime = now;
	d->mFramecount = 0;
//	boDebug() << k_funcinfo << "FPS: " << d->mFps << endl;
 }
 d->mFramecount++;
 return elapsed;
}

double BosonBigDisplayBase::fps() const
{
  return d->mFps;
}

void BosonBigDisplayBase::setZoomFactor(float f)
{
 boDebug() << k_funcinfo << f << endl;
 d->mCamera.setZoomFactor(f); // no need to call setCamera(), since resizeGL() does it
 resizeGL(d->mViewport[2], d->mViewport[3]);
}

void BosonBigDisplayBase::setViewport(int x, int y, GLsizei w, GLsizei h)
{
 if (!d->mInitialized) {
	glInit();
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
 GLdouble clip[16];
 GLdouble t;

 // Combine the two matrices (multiply projection by modelview)
 clip[0] = d->mModelviewMatrix[0] * d->mProjectionMatrix[0] +
		d->mModelviewMatrix[1] * d->mProjectionMatrix[4] +
		d->mModelviewMatrix[2] * d->mProjectionMatrix[8] +
		d->mModelviewMatrix[3] * d->mProjectionMatrix[12];
 clip[1] = d->mModelviewMatrix[0] * d->mProjectionMatrix[1] +
		d->mModelviewMatrix[1] * d->mProjectionMatrix[5] +
		d->mModelviewMatrix[2] * d->mProjectionMatrix[9] +
		d->mModelviewMatrix[3] * d->mProjectionMatrix[13];
 clip[2] = d->mModelviewMatrix[0] * d->mProjectionMatrix[2] +
		d->mModelviewMatrix[1] * d->mProjectionMatrix[6] +
		d->mModelviewMatrix[2] * d->mProjectionMatrix[10] +
		d->mModelviewMatrix[3] * d->mProjectionMatrix[14];
 clip[3] = d->mModelviewMatrix[0] * d->mProjectionMatrix[3] +
		d->mModelviewMatrix[1] * d->mProjectionMatrix[7] +
		d->mModelviewMatrix[2] * d->mProjectionMatrix[11] +
		d->mModelviewMatrix[3] * d->mProjectionMatrix[15];

 clip[4] = d->mModelviewMatrix[4] * d->mProjectionMatrix[0] +
		d->mModelviewMatrix[5] * d->mProjectionMatrix[4] +
		d->mModelviewMatrix[6] * d->mProjectionMatrix[8] +
		d->mModelviewMatrix[7] * d->mProjectionMatrix[12];
 clip[5] = d->mModelviewMatrix[4] * d->mProjectionMatrix[1] +
		d->mModelviewMatrix[5] * d->mProjectionMatrix[5] +
		d->mModelviewMatrix[6] * d->mProjectionMatrix[9] +
		d->mModelviewMatrix[7] * d->mProjectionMatrix[13];
 clip[6] = d->mModelviewMatrix[4] * d->mProjectionMatrix[2] +
		d->mModelviewMatrix[5] * d->mProjectionMatrix[6] +
		d->mModelviewMatrix[6] * d->mProjectionMatrix[10] +
		d->mModelviewMatrix[7] * d->mProjectionMatrix[14];
 clip[7] = d->mModelviewMatrix[4] * d->mProjectionMatrix[3] +
		d->mModelviewMatrix[5] * d->mProjectionMatrix[7] +
		d->mModelviewMatrix[6] * d->mProjectionMatrix[11] +
		d->mModelviewMatrix[7] * d->mProjectionMatrix[15];

 clip[8] = d->mModelviewMatrix[8] * d->mProjectionMatrix[0] +
		d->mModelviewMatrix[9] * d->mProjectionMatrix[4] +
		d->mModelviewMatrix[10] * d->mProjectionMatrix[8] +
		d->mModelviewMatrix[11] * d->mProjectionMatrix[12];
 clip[9] = d->mModelviewMatrix[8] * d->mProjectionMatrix[1] +
		d->mModelviewMatrix[9] * d->mProjectionMatrix[5] +
		d->mModelviewMatrix[10] * d->mProjectionMatrix[9] +
		d->mModelviewMatrix[11] * d->mProjectionMatrix[13];
 clip[10] = d->mModelviewMatrix[8] * d->mProjectionMatrix[2] +
		d->mModelviewMatrix[9] * d->mProjectionMatrix[6] +
		d->mModelviewMatrix[10] * d->mProjectionMatrix[10] +
		d->mModelviewMatrix[11] * d->mProjectionMatrix[14];
 clip[11] = d->mModelviewMatrix[8] * d->mProjectionMatrix[3] +
		d->mModelviewMatrix[9] * d->mProjectionMatrix[7] +
		d->mModelviewMatrix[10] * d->mProjectionMatrix[11] +
		d->mModelviewMatrix[11] * d->mProjectionMatrix[15];

 clip[12] = d->mModelviewMatrix[12] * d->mProjectionMatrix[0] +
		d->mModelviewMatrix[13] * d->mProjectionMatrix[4] +
		d->mModelviewMatrix[14] * d->mProjectionMatrix[8] +
		d->mModelviewMatrix[15] * d->mProjectionMatrix[12];
 clip[13] = d->mModelviewMatrix[12] * d->mProjectionMatrix[1] +
		d->mModelviewMatrix[13] * d->mProjectionMatrix[5] +
		d->mModelviewMatrix[14] * d->mProjectionMatrix[9] +
		d->mModelviewMatrix[15] * d->mProjectionMatrix[13];
 clip[14] = d->mModelviewMatrix[12] * d->mProjectionMatrix[2] +
		d->mModelviewMatrix[13] * d->mProjectionMatrix[6] +
		d->mModelviewMatrix[14] * d->mProjectionMatrix[10] +
		d->mModelviewMatrix[15] * d->mProjectionMatrix[14];
 clip[15] = d->mModelviewMatrix[12] * d->mProjectionMatrix[3] +
		d->mModelviewMatrix[13] * d->mProjectionMatrix[7] +
		d->mModelviewMatrix[14] * d->mProjectionMatrix[11] +
		d->mModelviewMatrix[15] * d->mProjectionMatrix[15];

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

float BosonBigDisplayBase::sphereInFrustum(float x, float y, float z, float radius) const
{
 // FIXME: performance: we might unrull the loop and then make this function
 // inline. We call it pretty often!
 float distance;
 for (int p = 0; p < 6; p++) {
	distance = d->mFrustumMatrix[p][0] * x + d->mFrustumMatrix[p][1] * y +
			d->mFrustumMatrix[p][2] * z + d->mFrustumMatrix[p][3];
	if (distance <= -radius){
		return 0;
	}
 }
 return distance + radius;
}

