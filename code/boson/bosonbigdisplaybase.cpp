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
#include "bosonmusic.h"
#include "selectbox.h"
#include "visual/bosonchat.h"
#include "bosonprofiling.h"

#include <kgame/kgameio.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <kdebug.h>

#include <qimage.h>
#include <qtimer.h>
#include <qcursor.h>

#include <sys/time.h>
#include <iostream.h>


#ifdef NO_OPENGL
#include <qwmatrix.h>

#else

#include "bosontexturearray.h"
#include "bosonglfont.h"

// both must be > 0.0:
#define NEAR 1.0 // FIXME: should be > 1.0
#define FAR 100.0


float textureUpperLeft[2] = { 0.0, 0.0 };
float textureLowerLeft[2] = { 0.0, 1.0 };
float textureLowerRight[2] = { 1.0, 1.0 };
float textureUpperRight[2] = { 1.0, 0.0 };
float textureCoordPointer[] = { 0.0, 0.0,
                                0.0, 1.0,
                                1.0, 1.0,
                                1.0, 0.0};
GLubyte unitIndices[] = { 0, 1, 2, 3 };
#endif // !NO_OPENGL

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
	int yy() const
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

#ifndef NO_OPENGL
		mFramecount = 0;
		mFps = 0;
		mFpsTime = 0;
		mDefaultFont = 0;
#endif
	}

	Player* mLocalPlayer;
	QTimer mCursorEdgeTimer;
	int mCursorEdgeCounter;

	BosonChat* mChat;

#ifndef NO_OPENGL
	// maybe we should use GLint here, as glViewport does
	float mPosX;
	float mPosY;
	float mPosZ;
	float mZoomFactor; //TODO

	float mCenterDiffX;
	float mCenterDiffY;

	int mW; // width ... we should use glGetIntegerv(GL_VIEWPORT, view); and then view[foo] instead.
	int mH; // height ... see above

	GLfloat mFovY; // see gluPerspective
	GLfloat mAspect; // see gluPerspective

	GLuint mMapDisplayList;
	BosonGLFont* mDefaultFont;// AB: maybe we should support several fonts

	long long int mFpsTime;
	double mFps;
	int mFramecount;

	bool mEvenFlag; // this is used for a nice trick to avoid clearing the depth 
	                // buffer - see http://www.mesa3d.org/brianp/sig97/perfopt.htm


	bool mInitialized;
#endif // !NO_OPENGL

	SelectionRect mSelectionRect;
	MouseMoveDiff mMouseMoveDiff;


	QTimer mUpdateTimer;
	int mUpdateInterval;
	
};

BosonBigDisplayBase::BosonBigDisplayBase(BosonCanvas* c, QWidget* parent)
		: MyHack(parent, "bigdisplay")
{
 kdDebug() << k_funcinfo << endl;
 mCanvas = c;
 init();
}

BosonBigDisplayBase::~BosonBigDisplayBase()
{
 quitGame();
 delete mSelection;
 delete d->mChat;
// delete d->mUnitTips;
 delete d->mDefaultFont;
 delete d;
}

void BosonBigDisplayBase::init()
{
 d = new BosonBigDisplayBasePrivate;
 mCursor = 0;
 d->mCursorEdgeCounter = 0;
 d->mUpdateInterval = 0;
 d->mInitialized = false;

 mSelection = new BoSelection(this);
 d->mChat = new BosonChat(this);

#ifndef NO_OPENGL
 slotResetViewProperties();
 d->mW = 0;
 d->mH = 0;

 d->mMapDisplayList = 0;

 if (!isValid()) {
	kdError() << k_funcinfo << "No OpenGL support present on this system??" << endl;
	return;
 }
 
 // and another hack..
// setMinimumSize(QSize(400,400));

 glInit();
 generateMapDisplayList();
 connect(&d->mUpdateTimer, SIGNAL(timeout()), this, SLOT(updateGL()));
#else
 setCanvas(c);
 setVScrollBarMode(AlwaysOff);
 setHScrollBarMode(AlwaysOff);
 connect(this, SIGNAL(contentsMoving(int, int)),
		this, SLOT(slotContentsMoving(int, int)));
 disconnect(this, SIGNAL(contentsMoving(int,int)), this, SLOT(cMoving(int,int)));

 connect(&d->mUpdateTimer, SIGNAL(timeout()), this, SLOT(update()));

#endif // !NO_OPENGL

 connect(&d->mCursorEdgeTimer, SIGNAL(timeout()), 
		this, SLOT(slotCursorEdgeTimeout()));

 //TODO: sprite tooltips

 setUpdateInterval(boConfig->updateInterval());
}

#ifndef NO_OPENGL
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
 qglClearColor(Qt::black);
 glShadeModel(GL_FLAT); // GL_SMOOTH is default - but esp. in software rendering way slower. in hardware it *may* be equal (concerning speed) to GL_FLAT
 glDisable(GL_DITHER); // we don't need this, I guess (and its enabled by default)

 glEnableClientState(GL_VERTEX_ARRAY);
 glEnableClientState(GL_TEXTURE_COORD_ARRAY);


 if (checkError()) {
	kdError() << k_funcinfo << endl;
 }
 struct timeval time;
 gettimeofday(&time, 0);
 d->mFpsTime = time.tv_sec * 1000000 + time.tv_usec;

 // this needs to be done in initializeGL():
 d->mDefaultFont = new BosonGLFont(QString::fromLatin1("fixed"));
}

void BosonBigDisplayBase::resizeGL(int w, int h)
{
 d->mW = w;
 d->mH = h;

 kdDebug() << k_funcinfo << w << " " << h << endl;
 glViewport(0, 0, (GLsizei)w, (GLsizei)h);
 glMatrixMode(GL_PROJECTION);
 glLoadIdentity();

 GLfloat fovY = d->mFovY * d->mZoomFactor;
 d->mAspect = (float)w / (float)h;
 gluPerspective(fovY, d->mAspect, NEAR, FAR);

 glMatrixMode(GL_MODELVIEW);


 glClearDepth(1.0);
 glClear(GL_DEPTH_BUFFER_BIT);
 glDepthRange(0.0, 0.5);
 glDepthFunc(GL_LESS);
 d->mEvenFlag = true;


 emit signalSizeChanged(w, h);
 if (checkError()) {
	kdError() << k_funcinfo << endl;
 }
}

void BosonBigDisplayBase::paintGL()
{
 boProfiling->render(true);
 d->mUpdateTimer.stop();
//kdDebug() << k_funcinfo << endl;
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

 boProfiling->renderClear(true);
 glClear(GL_COLOR_BUFFER_BIT);
 boProfiling->renderClear(false);

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
 calcFPS();
 glMatrixMode(GL_MODELVIEW); // default matrix mode anyway ; redundant!
 glLoadIdentity();

 float upX, upY, upZ;
 upX = 0.0;
 upY = 1.0;
 upZ = 0.0;
 float centerX, centerY, centerZ;
 centerX = cameraX() + d->mCenterDiffX;
 centerY = cameraY() - d->mCenterDiffY;
 centerZ = -100.0;
// centerZ = d->mPosZ;
 // TODO: performance: isn't it possible to skip this by using pushMatrix() and
 // popMatrix() a clever way? - afaics OpenGL needs to calculate the inverse at
 // this point...
 gluLookAt(cameraX(), cameraY(), cameraZ(), 
		centerX, centerY, centerZ, 
		upX, upY, upZ);
 if (checkError()) {
	kdError() << k_funcinfo << "after gluLookAt()" << endl;
 }

 glEnable(GL_TEXTURE_2D);
 // hmm.. I don't want to enable the depth buffer for cell-drawing, but otherwise we seem to have some overlapping on scrolling
// glEnable(GL_DEPTH_TEST); // if we enable this we need to change the z-positions of cells and/or units
 if (!d->mMapDisplayList) {
	generateMapDisplayList();
	if (!d->mMapDisplayList) {
		kdError() << k_funcinfo << "Unable to generate map display list" << endl;
		return;
	}
 }
 boProfiling->renderCells(true);
 glCallList(d->mMapDisplayList);
 boProfiling->renderCells(false);

 if (checkError()) {
	kdError() << k_funcinfo << "cells rendered" << endl;
 }

 glEnable(GL_DEPTH_TEST); // FIXME: this should be the first occurance of glEnable(GL_DEPTH_TEST)!
 glEnable(GL_BLEND); // AB: once we have 3d models for all units we can get rid of this. we need it for the cursor only then.
 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


 boProfiling->renderUnits(true);
 BoItemList allItems = mCanvas->allBosonItems();
 BoItemList::Iterator it = allItems.begin();
 for (; it != allItems.end(); ++it) {
	//FIXME: order by z-coordinates! first those which are
	//closer to surface, then flying units

	BosonSprite* item = *it;

	GLfloat x = item->x() * BO_GL_CELL_SIZE / BO_TILE_SIZE;
	GLfloat y = -(item->y() * BO_GL_CELL_SIZE / BO_TILE_SIZE + ((float)item->height()) * BO_GL_CELL_SIZE / BO_TILE_SIZE);
	GLfloat z = item->z() * BO_GL_CELL_SIZE / BO_TILE_SIZE;

	glTranslatef(x, y, z); // AB: we can use the item->vertexPointer(), too!

	if (item->displayList() == 0) {
		kdWarning() << k_funcinfo << "NULL display list for item rtti=" << item->rtti() << endl;
		continue;
	}
	// FIXME: performance: we could create a displaylist that contains the selectbox and simply change item->displayList()
	// when the item is selected/unselected
	glCallList(item->displayList());

	if (item->isSelected()) {
		// FIXME: performance: create a display lists in the SelectBox which also contains the scale!
		GLfloat w = ((float)item->width()) * BO_GL_CELL_SIZE / BO_TILE_SIZE;
		GLfloat h = ((float)item->height()) * BO_GL_CELL_SIZE / BO_TILE_SIZE;
		glPushMatrix();
		if (w != 1.0 || h != 1.0) {
			glScalef(w, h, 1.0);
		}
		glCallList(item->selectBox()->displayList());
		glPopMatrix();
	}

	glTranslatef(-x, -y, -z); 
 }
 boProfiling->renderUnits(false);

 if (checkError()) {
	kdError() << k_funcinfo << "when units rendered" << endl;
 }
 glDisable(GL_DEPTH_TEST);

 boProfiling->renderText(true); // AB: actually this is text and cursor

// TODO: cursor must always be on top and should not be scaled (i.e. cause of zoom)
// possible solutions: load the identity matrix ; maybe even use an ortho
// projection matrix for the cursor (it seems that we can mix them!)
// AB: it'll be always on top, cause we disabled the depth buffer test above
 if (cursor() && cursor()->isA("BosonSpriteCursor")) {
	// cursor and text are drawn in a 2D-matrix, so that we can use window
	// coordinates
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.0, (GLfloat)d->mW, 0.0, (GLfloat)d->mH); // the same as the viewport
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	 
	BosonSpriteCursor* c = (BosonSpriteCursor*)cursor();
	GLuint tex = c->currentTexture();
	if (tex != 0) {
		glPushMatrix();
		QPoint pos = mapFromGlobal(c->pos());
		GLfloat x;
		GLfloat y;
//		GLdouble z;
		x = (GLfloat)pos.x();
		y = (GLfloat)-pos.y();
//		mapCoordinates(pos, &x, &y, &z);
//		glTranslatef(x, y, 0.0);
//		glRasterPos2i(x, y);
		glRasterPos2i(pos.x(), -pos.y());

		float w = 0.5;
		float h = 0.5;
		glBindTexture(GL_TEXTURE_2D, tex);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0); glVertex3f(0.0, 0.0, 0.0);
			glTexCoord2f(0.0, 1.0); glVertex3f(0.0, h, 0.0);
			glTexCoord2f(1.0, 1.0); glVertex3f(w, h, 0.0);
			glTexCoord2f(1.0, 0.0); glVertex3f(w, 0.0, 0.0);
		glEnd();

		glPopMatrix();
	}

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	if (checkError()) {
		kdError() << k_funcinfo << "cursor rendered" << endl;
	}
	renderText();

	// now restore the old 3D-matrix
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
 }
 boProfiling->renderText(false);

 if (d->mSelectionRect.isVisible()) {
	glPushMatrix();
	GLfloat x, y, w, h;
	GLfloat x1, y1, x2, y2;
	GLfloat z; // currently the z-coordinate of the rect is not used - we set our own below

	qglColor(Qt::red); // FIXME hardcoded
	
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
	kdError() << k_funcinfo << "selection rect rendered" << endl;
 }

 if (d->mUpdateInterval) {
	d->mUpdateTimer.start(d->mUpdateInterval);
 }

 boProfiling->render(false);
}

void BosonBigDisplayBase::renderText()
{
 glListBase(d->mDefaultFont->displayList()); // AB: this is a redundant call, since we don't change it somewhere in paintGL(). but we might support different fonts one day and so we need it anyway.
 glColor3f(1.0, 1.0, 1.0);
 const int border = 5;

// first the resource display
 // AB: we can avoid these calls to i18n() here! e.g. cache it somewhere and
 // update every 5 seconds or so (maybe less)
 // remember that painGL() is very speed sensitive!
 QString minerals = i18n("Minerals: %1").arg(localPlayer()->minerals());
 QString oil = i18n("Oil:      %1").arg(localPlayer()->oil());
 int w = QMAX(d->mDefaultFont->metrics()->width(minerals), d->mDefaultFont->metrics()->width(oil));
 int x = d->mW - w - border;
 int y = d->mH - d->mDefaultFont->height() - border;
 glRasterPos2i(x, y);
 glCallLists(minerals.length(), GL_UNSIGNED_BYTE, (GLubyte*)minerals.latin1());
 y -= d->mDefaultFont->height();
 glRasterPos2i(x, y);
 glCallLists(oil.length(), GL_UNSIGNED_BYTE, (GLubyte*)oil.latin1());

// now the chat messages
// TODO: line break?
 x = border;
 y = border;
 QStringList list = d->mChat->messages(); 
 QStringList::Iterator it = list.end();
 --it;
 for (; it != list.begin(); --it) {
	glRasterPos2i(x, y);
	glCallLists((*it).length(), GL_UNSIGNED_BYTE, (GLubyte*)(*it).latin1());
	y += d->mDefaultFont->height();
 }
 glRasterPos2i(x, y);
 glCallLists((*it).length(), GL_UNSIGNED_BYTE, (GLubyte*)(*it).latin1()); // list.begin()

 glColor3f(1.0, 1.0, 1.0);
 
}

#endif // !NO_OPENGL


void BosonBigDisplayBase::slotMouseEvent(KGameIO* , QDataStream& stream, QMouseEvent* e, bool *eatevent)
{
 GLdouble posX, posY, posZ;
#ifndef NO_OPENGL
 if (!mapCoordinates(e->pos(), &posX, &posY, &posZ)) {
	kdError() << k_funcinfo << "Cannot map coordinates" << endl;
	return;
 }
#endif
 QPoint canvasPos; // FIXME we should write a canvas that uses GL-coordinates instead. currently we are maintaining 3 kinds of coordinates - canvas,window,world(GL)
#ifndef NO_OPENGL
 worldToCanvas(posX, posY, posZ, &canvasPos);
#else
 QWMatrix wm = inverseWorldMatrix(); // for zooming
 canvasPos = viewportToContents(e->pos());
 wm.map(canvasPos.x(), canvasPos.y(), &canvasPos.rx(), &canvasPos.ry());
 posX = (GLfloat)canvasPos.x();
 posX = (GLfloat)canvasPos.y();
 posY = 0.0;
#endif

 
 switch (e->type()) {
	case QEvent::Wheel:
	{
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
		d->mMouseMoveDiff.moveToPos(e->pos());
		if (e->state() & AltButton) {
#ifndef NO_OPENGL
			// zooming
			d->mZoomFactor += ((float)d->mMouseMoveDiff.dy()) / 10;
			kdDebug() << "zoom factor: " << d->mZoomFactor << endl;
			resizeGL(d->mW, d->mH);
#endif
		} else if (e->state() & LeftButton) {
#ifndef NO_OPENGL
			if (e->state() & ControlButton) {
				d->mCenterDiffX += d->mMouseMoveDiff.dx();
				d->mCenterDiffY -= d->mMouseMoveDiff.dy();
				kdDebug() << d->mCenterDiffX << " " << d->mCenterDiffY << endl;
			} else if (e->state() & ShiftButton) {
				// move the z-position of the cameraa
				setCameraPos(cameraX(), cameraY(), cameraZ() + d->mMouseMoveDiff.dy());
				kdDebug() << "posZ: " << d->mPosZ << endl;
			} else if (e->state() & AltButton) {
				// we can't use Alt+LMB since KDE already uses it to move the window :(
			} else 
#endif
			{
				d->mSelectionRect.setVisible(true);
				moveSelectionRect(posX, posY, posZ);
			}
		} else if (e->state() & RightButton) {
#ifndef NO_OPENGL
			if (boConfig->rmbMove()) {
				//FIXME: doesn't work, since QCursor::setPos() also generates a MouseMove event
//				QPoint pos = mapToGlobal(QPoint(d->mMouseMoveDiff.oldX(), d->mMouseMoveDiff.oldY()));
//				QCursor::setPos(pos);
				d->mMouseMoveDiff.startRMBMove();
				GLdouble dx, dy;
				int moveX = d->mMouseMoveDiff.dx();
				int moveY = d->mMouseMoveDiff.dy();
				mapDistance(moveX, moveY, &dx, &dy);
				setCameraPos(cameraX() + dx, cameraY() + dy, cameraZ());
			} else {
				d->mMouseMoveDiff.stop();
			}
#else
			//TODO - the canvasview version is broken
#endif
		}
		updateCursor();
		e->accept();
		break;
	case QEvent::MouseButtonDblClick:
		makeActive();
		if (e->button() == LeftButton) {
			Unit* unit = ((BosonCanvas*)canvas())->findUnitAt(canvasPos);
			if (unit) {
				if (!selectAll(unit->unitProperties())) {
					selection()->selectUnit(unit);
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
#ifndef NO_OPENGL
				cellX = (int)(posX / BO_GL_CELL_SIZE);
				cellY = (int)(-posY / BO_GL_CELL_SIZE);
#else
				cellX = canvasPos.x() / BO_TILE_SIZE;
				cellY = canvasPos.y() / BO_TILE_SIZE;
#endif
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
		if (e->button() == LeftButton) {
			if (e->state() & ControlButton) {
				// rotating
			} else if (e->state() & ShiftButton) {
				// z-position
			} else if (e->state() & AltButton) {
				// unused, since Alt+LMB is already used by KDE to move the window :(
			} else {
				removeSelectionRect();
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
		kdWarning() << "unexpected mouse event " << e->type() << endl;
		e->ignore();
		return;
 }
}

void BosonBigDisplayBase::addMouseIO(Player* p)
{
 kdDebug() << k_funcinfo << endl;
 // FIXME: check if player is valid, mouse IO already present, ... see
 // BosonBigDisplayBase
 // another TODO: implement a GL based widget for the editor!
 KGameMouseIO* mouseIO = new KGameMouseIO(this, true);
 connect(mouseIO, SIGNAL(signalMouseEvent(KGameIO*, QDataStream&, QMouseEvent*, bool*)),
		this, SLOT(slotMouseEvent(KGameIO*, QDataStream&, QMouseEvent*, bool*)));
 p->addGameIO(mouseIO);
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
 kdDebug() << k_funcinfo << endl;
 if (d->mLocalPlayer) {
	kdError() << k_funcinfo << "already a local player present!! unset first (TODO)!" << endl;
	return;
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
#ifndef NO_OPENGL
 d->mPosX = 0.0;
 d->mPosY = 0.0;
 d->mPosZ = 10.0;
 d->mCenterDiffX = 0.0;
 d->mCenterDiffY = 0.0;
 d->mZoomFactor = 1.0;

 d->mFovY = 60.0;
 d->mAspect = 1.0;
 setCameraPos(d->mPosX, d->mPosY, d->mPosZ);
 resizeGL(d->mW, d->mH);
#endif
}

void BosonBigDisplayBase::slotReCenterDisplay(const QPoint& pos)
{
#ifndef NO_OPENGL
//TODO don't center the corners - e.g. 0;0 should be top left, never center
 setCameraPos(((float)pos.x()) * BO_GL_CELL_SIZE, -((float)pos.y()) * BO_GL_CELL_SIZE, cameraZ());
#else
 center(pos.x() * BO_TILE_SIZE, pos.y() * BO_TILE_SIZE);
 canvas()->update();
#endif
}

#ifndef NO_OPENGL
void BosonBigDisplayBase::worldToCanvas(GLfloat x, GLfloat y, GLfloat /*z*/, QPoint* pos) const
{
 pos->setX((int)(x / BO_GL_CELL_SIZE * BO_TILE_SIZE));
 pos->setY((int)(-y / BO_GL_CELL_SIZE * BO_TILE_SIZE));
}

bool BosonBigDisplayBase::mapCoordinates(const QPoint& pos, GLdouble* posX, GLdouble* posY, GLdouble* posZ) const
{
 GLint view[4];
 GLdouble proj[16];
 GLdouble model[16];
 GLint realy;
 glGetIntegerv(GL_VIEWPORT, view);
 glGetDoublev(GL_MODELVIEW_MATRIX, model);
 glGetDoublev(GL_PROJECTION_MATRIX, proj);

 realy = view[3] - (GLint)pos.y() - 1;

// kdDebug() << "mouse pos: " << pos.x() << "," << pos.y() << endl;
// kdDebug() << "real mouse pos: " << pos.x() << "," << realy << endl;

 GLdouble winX = (GLdouble)pos.x();
 GLdouble winY = (GLdouble)realy;
 GLdouble winZ;

// winZ = (1.0 / (FAR - NEAR)) * (d->mPosZ - NEAR);
// kdDebug() << "winZ: " << winZ << "; mPosZ: " << d->mPosZ << endl;
 GLdouble tmp;
 if (!gluProject(winX, winY, 0.0,
		model, proj, view,
		&tmp, &tmp, &winZ)) {
	return false;
 }
 if (!gluUnProject(winX, winY, winZ,
		model, proj, view,
		posX, posY, posZ)) {
	return false;
 }

// kdDebug() << "click on: " << *posX << "," << *posY << "," <<*posZ << endl;
 return true;
}

bool BosonBigDisplayBase::mapDistance(int windx, int windy, GLdouble* dx, GLdouble* dy) const
{
 GLdouble moveZ; // unused
 GLdouble moveX1, moveY1;
 GLdouble moveX2, moveY2;
 if (!mapCoordinates(QPoint(0, 0), &moveX1, &moveY1, &moveZ)) {
	kdError() << k_funcinfo << "Cannot map coordinates" << endl;
	return false;
 }
 if (!mapCoordinates(QPoint(windx, windy), &moveX2, &moveY2, &moveZ)) {
	kdError() << k_funcinfo << "Cannot map coordinates" << endl;
	return false;
 }
 *dx = moveX2 - moveX1;
 *dy = moveY2 - moveY1;
 return true;
}

#endif // !NO_OPENGL

void BosonBigDisplayBase::enterEvent(QEvent*)
{
 if (!cursor()) {
	// don't generate an error, since a NULL cursor might be valid for
	// editor mode
	return;
 }
 cursor()->showCursor();
}

void BosonBigDisplayBase::leaveEvent(QEvent*)
{
 if (!cursor()) {
	// don't generate an error, since a NULL cursor might be valid for
	// editor mode
	return;
 }
 cursor()->hideCursor();
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
#ifndef NO_OPENGL
 return QGLWidget::eventFilter(o, e);
#else
 return QCanvasView::eventFilter(o, e);
#endif
}

bool BosonBigDisplayBase::selectAll(const UnitProperties* prop)
{
 if (!localPlayer()) {
	kdError() << k_funcinfo << "NULL localplayer" << endl;
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
 selection()->selectUnits(list);
 return true;
}

void BosonBigDisplayBase::slotUnitChanged(Unit* unit)
{
// FIXME: we might want to place this code into BoSelection directly (cleaner)
 if (!unit) {
	kdError() << k_funcinfo << "NULL unit" << endl;
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
#ifndef NO_OPENGL
 worldToCanvas(x, y, z, &canvasPos);
#endif
 /*
 Unit* unit = canvas()->findUnitAt(canvasPos);
 kdDebug() << k_funcinfo << x << " " << y << " " << z << endl;
 if (!unit) {*/
	// nothing has been found - its a ground click
	// Here we have to draw the selection rect
	d->mSelectionRect.setStart(x, y, z);
	d->mSelectionRect.setVisible(true);
	return;
	/*
 }

 kdDebug() << k_funcinfo << "unit" << endl;
 selection()->selectUnit(unit);

 // cannot be placed into mSelection cause we don't have localPlayer
 // there
 if (localPlayer() == unit->owner()) {
	boMusic->playSound(unit, Unit::SoundOrderSelect);
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

void BosonBigDisplayBase::removeSelectionRect()
{
 if (d->mSelectionRect.isVisible()) {
	// here as there is a performance problem in
	// mousemove:
	selectArea();

	d->mSelectionRect.setVisible(false);
	if (!selection()->isEmpty()) {
		Unit* u = selection()->leader();
		if (u->owner() == localPlayer()) {
			// TODO: do not play sound here
			// instead make virtual and play in derived class
			boMusic->playSound(u, Unit::SoundOrderSelect);
		}
	}
 } else {
	// a simple click on the map
	GLfloat x,y,z;
	d->mSelectionRect.start(&x, &y, &z);
	QPoint canvasPos;
#ifndef NO_OPENGL
	worldToCanvas(x, y, z, &canvasPos);
#endif
	Unit* unit = canvas()->findUnitAt(canvasPos);
	selection()->clear();
	if (unit) {
		kdDebug() << k_funcinfo << "unit" << endl;
		selection()->selectUnit(unit);
		// cannot be placed into mSelection cause we don't have localPlayer
		// there
		if (localPlayer() == unit->owner()) {
			boMusic->playSound(unit, Unit::SoundOrderSelect);
		}
	}
 }
}

void BosonBigDisplayBase::selectArea()
{
 if (!d->mSelectionRect.isVisible()) {
	kdDebug() << k_funcinfo << "no rect" << endl;
	return;
 }
 if (boConfig->debugMode() == BosonConfig::DebugSelection) {
	BoItemList list;
	QRect r = selectionRectCanvas();
	list = canvas()->bosonCollisions(r);
	BoItemList::Iterator it;
	kdDebug() << "Selection count: " << list.count() << endl;
	for (it = list.begin(); it != list.end(); ++it) {
		QString s = QString("Selected: RTTI=%1").arg((*it)->rtti());
		if (RTTI::isUnit((*it)->rtti())) {
			Unit* u = (Unit*)*it;
			s += QString(" Unit ID=%1").arg(u->id());
			if (u->isDestroyed()) {
				s += QString("(destroyed)");
			}
		}
		kdDebug() << s << endl;
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
	kdDebug() << "select " << unitList.count() << " units" << endl;
	selection()->selectUnits(unitList);
 } else if (fallBackUnit) {
	selection()->selectUnit(fallBackUnit);
 } else {
	selection()->clear();
 }
}


QRect BosonBigDisplayBase::selectionRectCanvas() const
{
 QPoint start, end;
 GLfloat startx, starty, startz, endx, endy, endz;
 selectionStart(&startx, &starty, &startz);
 selectionEnd(&endx, &endy, &endz);
#ifndef NO_OPENGL
 worldToCanvas(startx, starty, startz, &start);
 worldToCanvas(endx, endy, endz, &end);
#else
 start = QPoint(startx, starty);
 end = QPoint(endx, endy);
#endif
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
#ifndef NO_OPENGL
 GLdouble dx, dy;
 mapDistance(move, move, &dx, &dy);
 GLfloat moveX = (GLfloat)dx;
 GLfloat moveY = (GLfloat)dy;
#else
 const float moveX = move;
 const float moveY = move;
#endif
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
		#ifndef NO_OPENGL
			setCameraPos(cameraX() + x, cameraY() + y, cameraZ());
		#else
			scrollBy((int)x, (int)y);
		#endif
	}
 }
}


void BosonBigDisplayBase::scrollBy(int dx, int dy)
{
#ifndef NO_OPENGL
 GLdouble x, y;
 mapDistance(dx, dy, &x, &y);
 setCameraPos(cameraX() + x, cameraY() + y, cameraZ());
#else 
 QCanvasView::scrollBy(dx, dy);
#endif
}

#ifndef NO_OPENGL
void BosonBigDisplayBase::generateMapDisplayList()
{
 makeCurrent();
 // some clever guy made QGLContet::initialized() protected - so we need to
 // implement our own version here :-(
 if (!d->mInitialized) {
	glInit();
 }
 BosonMap* map = mCanvas->map();
 if (!map) {
	kdError() << k_funcinfo << "NULL map" << endl;
	return;
 }
 BosonTiles* tiles = mCanvas->tileSet();
 if (!tiles) {
	kdError() << k_funcinfo << "NULL tiles" << endl;
	return;
 }
 if (!mCanvas->tileSet()->textures()) {
	mCanvas->tileSet()->generateTextures();
	if (!mCanvas->tileSet()->textures()) {
		kdWarning() << k_funcinfo << "NULL textures for cells" << endl;
		return;
	}
 }


 GLuint list;
 list = glGenLists(1);
 if (!list) {
	kdError() << k_funcinfo << "Error generating display list" << endl;
	return;
 }
 glNewList(list, GL_COMPILE);
 
 GLfloat cellYPos = -BO_GL_CELL_SIZE;
 GLfloat cellXPos = 0.0;
 for (unsigned int cellY = 0; cellY < map->height(); cellY++) {
	cellXPos = 0.0;
	for (unsigned int cellX = 0; cellX < map->width(); cellX++) {
		Cell* cell = map->cell(cellX, cellY);
		// we don't check for a NULL cell in favor of performance
		GLuint texture = tiles->textures()->texture(cell->tile());
		glBindTexture(GL_TEXTURE_2D, texture); // which texture to load
	
		// FIXME: we can improve performance here!
		// manage different lists for different textures and
		// iterate them separetely
		// then we could render them in a single glBegin(GL_QUADS) call
		// we can't render cells with differnt textures in a single call
		// :(
		glBegin(GL_QUADS);
			glTexCoord2fv(textureUpperLeft); glVertex3f(cellXPos, cellYPos, 0.0);
			glTexCoord2fv(textureLowerLeft); glVertex3f(cellXPos, cellYPos + BO_GL_CELL_SIZE, 0.0);
			glTexCoord2fv(textureLowerRight); glVertex3f(cellXPos + BO_GL_CELL_SIZE, cellYPos + BO_GL_CELL_SIZE, 0.0);
			glTexCoord2fv(textureUpperRight); glVertex3f(cellXPos + BO_GL_CELL_SIZE, cellYPos, 0.0);
		glEnd();

		cellXPos += BO_GL_CELL_SIZE;
	}
	// 0.0,0.0 is bottom-left in opengl, not top-left
	cellYPos -= BO_GL_CELL_SIZE;
 }

 glEndList();
 d->mMapDisplayList = list;
}

void BosonBigDisplayBase::setCameraPos(GLfloat x, GLfloat y, GLfloat z)
{
 // TODO: rotating the screen is not yet supported!
 GLdouble x1, x2;
 GLdouble y1, y2;
 GLdouble tmp;
 GLdouble w, h;
 // we cannot simply map 0,0 and compare it with the mapped d->mW,d->mH since
 // the screen might be rotated... but thats not supported anyway
 mapCoordinates(QPoint(0,0), &x1, &y1, &tmp);
 mapCoordinates(QPoint(d->mW,0), &x2, &tmp, &tmp);
 mapCoordinates(QPoint(0,d->mH), &tmp, &y2, &tmp);
 w = x2-x1;
 h = y2-y1;
 
 d->mPosX = QMAX(w / 2, QMIN(((float)mCanvas->mapWidth()) * BO_GL_CELL_SIZE - w / 2, x));
 d->mPosY = QMIN(h / 2, QMAX((-((float)mCanvas->mapHeight()) * BO_GL_CELL_SIZE) - h / 2, y));
 d->mPosZ = QMAX(NEAR+0.1, QMIN(FAR-0.1, z));

 int cellX = (int)(x1 / BO_GL_CELL_SIZE);
 int cellY = -(int)(y1 / BO_GL_CELL_SIZE);
 emit signalTopLeftCell(cellX, cellY);

 // TODO: if z changed also the size changes - the player can see more on the
 // screen. we should emit signalSizeChanged(). that signal should be renamed
 // (signalViewSize() or something)
}

GLfloat BosonBigDisplayBase::cameraX() const
{
 return d->mPosX;
}

GLfloat BosonBigDisplayBase::cameraY() const
{
 return d->mPosY;
}

GLfloat BosonBigDisplayBase::cameraZ() const
{
 return d->mPosZ;
}

void BosonBigDisplayBase::updateGLCursor()
{
 // FIXME: use updateGL() directly instead. this function is just for testing.
}

bool BosonBigDisplayBase::checkError()
{
 bool ret = true;
 GLenum e = glGetError();
 switch (e) {
	case GL_INVALID_ENUM:
		kdError() << "GL_INVALID_ENUM" << endl;
		break;
	case GL_INVALID_VALUE:
		kdError() << "GL_INVALID_VALUE" << endl;
		break;
	case GL_INVALID_OPERATION:
		kdError() << "GL_INVALID_OPERATION" << endl;
		break;
	case GL_STACK_OVERFLOW:
		kdError() << "GL_STACK_OVERFLOW" << endl;
		break;
	case GL_STACK_UNDERFLOW:
		kdError() << "GL_STACK_UNDERFLOW" << endl;
		break;
	case GL_OUT_OF_MEMORY:
		kdError() << "GL_OUT_OF_MEMORY" << endl;
		break;
	case GL_NO_ERROR:
		ret = false;
		break;
	default:
		kdError() << "Unknown OpenGL Error: " << (int)e << endl;
		break;
 }
 if (e != GL_NO_ERROR) {
	kdError() << "Error string: " << gluErrorString(e) << endl;
 }
 return ret;
}
#endif // !NO_OPENGL

void BosonBigDisplayBase::setUpdateInterval(unsigned int ms)
{
 kdDebug() << k_funcinfo << ms << endl;
 d->mUpdateInterval = ms;
 QTimer::singleShot(d->mUpdateInterval, this, SLOT(updateGL()));
}

#ifndef NO_OPENGL
void BosonBigDisplayBase::calcFPS()
{
 long long int now;
 struct timeval time;
 gettimeofday(&time, 0);
 now = time.tv_sec * 1000000 + time.tv_usec;
 // FPS is updated once per second
 if((now - d->mFpsTime) >= 1000000) {
	d->mFps = d->mFramecount / ((now - d->mFpsTime) / 1000000.0);
	d->mFpsTime = now;
	d->mFramecount = 0;
//	kdDebug() << k_funcinfo << "FPS: " << d->mFps << endl;
 }
 d->mFramecount++;
}

double BosonBigDisplayBase::fps() const
{
  return d->mFps;
}
#endif

void BosonBigDisplayBase::setZoomFactor(float f)
{
#ifndef NO_OPENGL
 kdDebug() << k_funcinfo << f << endl;
 d->mZoomFactor = f;
 resizeGL(d->mW, d->mH);
#else
 QWMatrix w;
 w.scale((double)f, (double)f);
 setWorldMatrix(w);
#endif
}

