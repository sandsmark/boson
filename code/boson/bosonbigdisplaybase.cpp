/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "defines.h"
#include "player.h"
#include "bosoncursor.h"
#include "boselection.h"
#include "bosonconfig.h"
#include "bosonmusic.h"

#include <kgame/kgameio.h>

#include <kdebug.h>

#include <qimage.h>
#include <qtimer.h>
#include <qcursor.h>

#include <sys/time.h>


#ifdef NO_OPENGL
#include <qwmatrix.h>

#else

#include "bosontexturearray.h"

// both must be > 0.0:
#define NEAR 1.0
#define FAR 100.0


#define PLIB 0
#define LIB3DS 0

#if PLIB
#include <plib/ssg.h>
#elif LIB3DS
#include <lib3ds/file.h>
#include <lib3ds/camera.h> // we probably don't need this!
#include <lib3ds/node.h>
#include <lib3ds/matrix.h>
#include <lib3ds/mesh.h>
#include <lib3ds/vector.h>
Lib3dsFile* file;

void renderNode(Lib3dsNode* node)
{
 {
	Lib3dsNode* p;
	for (p = node->childs; p; p = p->next) {
		renderNode(p);
	}
 }
 if (node->type == LIB3DS_OBJECT_NODE) {
	if (strcmp(node->name, "$$$DUMMY") == 0) {
		return;
	}
	if (!node->user.d) {
		Lib3dsMesh* mesh = lib3ds_file_mesh_by_name(file, node->name); // FIXME: what is this?
		if (!mesh) {
			return;
		}
	glColor3f(1.0,0.0,0.0);
		node->user.d = glGenLists(1);
		glNewList(node->user.d, GL_COMPILE);

		unsigned int p;
		Lib3dsMatrix invMeshMatrix;
		lib3ds_matrix_copy(invMeshMatrix, mesh->matrix);
		lib3ds_matrix_inv(invMeshMatrix);

		for (p = 0; p < mesh->faces; ++p) {
			Lib3dsFace &f = mesh->faceL[p];
			//...


			{
				//..
				int i;
				Lib3dsVector v[3];
				for (i = 0; i < 3; i++) {
					lib3ds_vector_transform(v[i], invMeshMatrix, mesh->pointL[f.points[i]].pos);
				}
	glColor3f(1.0,0.0,0.0);
				glBegin(GL_TRIANGLES);
//					glNormal3fv(f.normal);
//					kdDebug() << v[0][0] << "," << v[0][1] << ","<<v[0][2] << endl;
					glVertex3fv(v[0]);
					glVertex3fv(v[1]);
					glVertex3fv(v[2]);
				glEnd();
	glColor3f(1.0, 1.0, 1.0);
			}
		}
		glEndList();
	}
	if (node->user.d) {
	glColor3f(1.0,0.0,0.0);
		glPushMatrix();
		Lib3dsObjectData* d = &node->data.object;
		glMultMatrixf(&node->matrix[0][0]);
		glTranslatef(-d->pivot[0], -d->pivot[1], -d->pivot[2]);
		glCallList(node->user.d);
		glPopMatrix();
	glColor3f(1.0, 1.0, 1.0);
	}
 }
}
#endif // LIB3DS

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
#ifndef NO_OPENGL
#if ((PLIB) || (LIB3DS))
		m3ds = 0;
#endif
#endif

		
		mLocalPlayer = 0;
//		mChat = 0;

#ifndef NO_OPENGL
		mFramecount = 0;
		mFps = 0;
		mFpsTime = 0;
#endif
	}

	Player* mLocalPlayer;
	QTimer mCursorEdgeTimer;
	int mCursorEdgeCounter;

//	KGameCanvasChat* mChat; //TODO: write some kind of KGameGLChat

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

	long long int mFpsTime;
	double mFps;
	int mFramecount;
#endif // !NO_OPENGL

	SelectionRect mSelectionRect;
	MouseMoveDiff mMouseMoveDiff;


	QTimer mUpdateTimer;

#ifndef NO_OPENGL
#if PLIB
//	ssgEntity* m3ds;
#elif LIB3DS
	Lib3dsFile* m3ds;
#endif
#endif
};

BosonBigDisplayBase::BosonBigDisplayBase(BosonCanvas* c, QWidget* parent)
		: MyHack(parent, "bigdisplay")
{
 kdDebug() << k_funcinfo << endl;
 d = new BosonBigDisplayBasePrivate;
 mCursor = 0;
 d->mCursorEdgeCounter = 0;

#ifndef NO_OPENGL
 d->mPosX = 0.0;
 d->mPosY = 0.0;
 d->mPosZ = 10.0;
 d->mCenterDiffX = 0.0;
 d->mCenterDiffY = 0.0;

 d->mFovY = 60.0;
 d->mAspect = 1.0;

 d->mW = 0;
 d->mH = 0;

 d->mMapDisplayList = 0;

 if (!isValid()) {
	kdError() << k_funcinfo << "No OpenGL support present on this system??" << endl;
	return;
 }
 
 // and another hack..
 setMinimumSize(QSize(400,400));


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

 mSelection = new BoSelection(this);
 mCanvas = c;


 
 connect(&d->mCursorEdgeTimer, SIGNAL(timeout()), 
		this, SLOT(slotCursorEdgeTimeout()));

 //TODO: sprite tooltips

 setUpdateInterval(boConfig->updateInterval());
}

BosonBigDisplayBase::~BosonBigDisplayBase()
{
#ifndef NO_OPENGL
#if LIB3DS
 if (d->m3ds) {
	lib3ds_file_free(d->m3ds);
	d->m3ds = 0;
 }
#endif
#endif
	
 quitGame();
 delete mSelection;
// delete d->mChat;
// delete d->mUnitTips;
 delete d;
}

#ifndef NO_OPENGL
void BosonBigDisplayBase::initializeGL()
{
 glClearColor(0.0, 0.0, 0.0, 0.0);

 qglClearColor(Qt::white);
 glShadeModel(GL_FLAT); // GL_SMOOTH is default - but esp. in software rendering way slower. in hardware it *may* be equal (concerning speed) to GL_FLAT
 glDisable(GL_DITHER); // we don't need this, I guess (and its enabled by default)

 glEnableClientState(GL_VERTEX_ARRAY);
 glEnableClientState(GL_TEXTURE_COORD_ARRAY);


#if PLIB
 d->m3ds = ssgLoad3ds("/home/andi/tank.3ds");
#elif LIB3DS
 d->m3ds = lib3ds_file_load("/home/andi/tank.3ds");
 kdDebug() << k_funcinfo << "current frame: " << d->m3ds->current_frame << endl;
 lib3ds_file_eval(d->m3ds, d->m3ds->current_frame);
 file = d->m3ds;
#endif
 if (checkError()) {
	kdError() << k_funcinfo << endl;
 }
  struct timeval time;
  gettimeofday(&time, 0);
  d->mFpsTime = time.tv_sec * 1000000 + time.tv_usec;
}

void BosonBigDisplayBase::resizeGL(int w, int h)
{
 glViewport(0, 0, (GLsizei)w, (GLsizei)h);
 glMatrixMode(GL_PROJECTION);
 glLoadIdentity();

 d->mW = w;
 d->mH = h;

 d->mAspect = (float)w / (float)h;
 gluPerspective(d->mFovY, d->mAspect, NEAR, FAR);

 glMatrixMode(GL_MODELVIEW);

 emit signalSizeChanged(w, h);
 if (checkError()) {
	kdError() << k_funcinfo << endl;
 }
}

void BosonBigDisplayBase::paintGL()
{
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
 // TODO: performance and code cleanness: use glGetError() !!! fixinf such
 // errors will speed up rendering and lead to cleaner code :)
 // remove the glGetError() calls in releases and so on, since they slow
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

 // AB: performance: we don't need to clear the color buffer, since the scene we
 // are drawing covers the entire screen anyway.
 // TODO: make sure that the player can't move the screen beyond the map!
 // TODO: performance: we'll probably need the depth buffer soon - there is a
 // nice trick to avoid clearing it. see
 // http://www.mesa3d.org/brianp/sig97/perfopt.htm
 // in 3.5!
 glClear(GL_COLOR_BUFFER_BIT);
 calcFPS();
 glMatrixMode(GL_MODELVIEW); // default matrix mode anyway ; redundant!
 glLoadIdentity();
 if (checkError()) {
	kdError() << k_funcinfo << "1" << endl;
 }

 float upX, upY, upZ;
 upX = 0.0;
 upY = 1.0;
 upZ = 0.0;
 float centerX, centerY, centerZ;
 centerX = cameraX() + d->mCenterDiffX;
 centerY = cameraY() + d->mCenterDiffY;
 centerZ = -100.0;
// centerZ = d->mPosZ;
 // TODO: performance: isn't it possible to skip this by using pushMatrix() and
 // popMatrix() a clever way? - afaics OpenGL needs to calculate the inverse at
 // this point...
 gluLookAt(cameraX(), cameraY(), cameraZ(), 
		centerX, centerY, centerZ, 
		upX, upY, upZ);
 if (checkError()) {
	kdError() << k_funcinfo << "2" << endl;
 }

 glEnable(GL_TEXTURE_2D);
 if (!d->mMapDisplayList) {
	generateMapDisplayList();
	if (!d->mMapDisplayList) {
		kdError() << k_funcinfo << "Unable to generate map display list" << endl;
		return;
	}
	kdDebug() << k_funcinfo << "Successfully generated map display list" << endl;
 }
 glCallList(d->mMapDisplayList);
 if (checkError()) {
	kdError() << k_funcinfo << "cells rendered" << endl;
 }

#if LIB3DS
 glPushMatrix();
 glTranslatef(1.0,-1.0,0.0);

 // FIXME: the .3ds files are *way* too big!
 // they should be smaller - bigger files with more polygons lead to bad
 // performance
 // FIXME: we need to find a way to have a fixed size. e.g. a unit should have
 // size of about BO_GL_CELL_SIZE. but we can only scale relative :(
 glScalef(0.001,0.001,0.001);
 Lib3dsNode* node;
 for (node = d->m3ds->nodes; node != 0; node = node->next) {
	renderNode(node);
 }
 glPopMatrix();
#endif

 glEnable(GL_BLEND);
 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 //wow.. we have a LOT of space for tuning here!
 BoItemList allItems = mCanvas->allBosonItems();
 BoItemList::Iterator it = allItems.begin();
 for (; it != allItems.end(); ++it) {
	//FIXME: order by z-coordinates! first those which are
	//closer to surface, then flying units
		
	BosonSprite* item = *it;
	GLuint tex = item->currentTexture();
	if (tex == 0) {
		kdWarning() << k_funcinfo << "invalid texture" << endl;
		if (!item->textures()) {
			kdError() << "NULL textures" << endl;
		} else {
			kdDebug() << "frame=" << item->frame() << endl;
		}
		continue;
	}
	//FIXME: for ships and aircrafts boundingRect().width() and .height()
	//returns a wrong value!! (48;48 instead of e.g. 96;96) why??

	glBindTexture(GL_TEXTURE_2D, tex); // which texture to load

	// FIXME: vertex arrays are an opengl extension! add an #ifdef for it!!
	// FIXME: performance: we could combine all units with the same texture
	glTexCoordPointer(2, GL_FLOAT, 0, textureCoordPointer); 
	glVertexPointer(3, GL_FLOAT, 0, item->vertexPointer());
	/*
#warning FIXME REMOVE!!
	if (((Unit*)item)->id() == 37) {
		kdDebug() << "x=" << item->x() << endl;
		kdDebug() << "y=" << item->y() << endl;
		kdDebug()<< "vertices:"<<endl;
		for (int i = 0; i < 3*4; i++) {
			kdDebug() << item->vertexPointer()[i] << endl;
		}
		kdDebug()<< "verticesdone"<<endl;
	}
	*/
			
	glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, unitIndices);
 }
 if (checkError()) {
	kdError() << k_funcinfo << "units rendered" << endl;
 }

//TODO: cursor must always be on top!
//possible solutions: load the identity matrix ; maybe even use an ortho
//projection matrix for the cursor (it seems that we can mix them!)
 if (cursor() && cursor()->isA("BosonSpriteCursor")) {
	BosonSpriteCursor* c = (BosonSpriteCursor*)cursor();
	GLuint tex = c->currentTexture();
	if (tex != 0) {
		glPushMatrix();
		QPoint pos = mapFromGlobal(c->pos());
		GLdouble x;
		GLdouble y;
		GLdouble z;
		mapCoordinates(pos, &x, &y, &z);
		glTranslatef(x, y, 0.0);

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
 }
 if (checkError()) {
	kdError() << k_funcinfo << "cursor rendered" << endl;
 }

 glDisable(GL_BLEND);
 glDisable(GL_TEXTURE_2D);

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

	//FIXME: ensure that the rect is *always* on top - even for units with z > 0
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
		GLdouble dx, dy;

		if (e->state() & ControlButton) {
			d->mCenterDiffX += d->mMouseMoveDiff.dx();
			d->mCenterDiffY += d->mMouseMoveDiff.dy();
			kdDebug() << d->mCenterDiffX << " " << d->mCenterDiffY << endl;
		} else if (e->state() & ShiftButton) {
			// "zooming"
			// actually we simply move the z-position of the camera but it basically has the effect of zooming
			setCameraPos(cameraX(), cameraY(), cameraZ() - d->mMouseMoveDiff.dy());
			kdDebug() << "posZ: " << d->mPosZ << endl;
//		} else if (e->state() & AltButton) {
		} else if (e->state() & LeftButton) {
			d->mSelectionRect.setVisible(true);
			moveSelectionRect(posX, posY, posZ);
		} else if (e->state() & RightButton) {
#ifndef NO_OPENGL
			if (boConfig->rmbMove()) {
				d->mMouseMoveDiff.startRMBMove();
				setCameraPos(cameraX() + d->mMouseMoveDiff.dx(), 
						cameraY() + d->mMouseMoveDiff.dy(), cameraZ());
			} else {
				// oops
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
		/*
			if (e->state() & ControlButton) {

			} else if (e->state() & AltButton) {
			} else if (e->state() & ShiftButton) {
			} else {
			*/
				// not modifier key was pressed
				// start selection rect (or a simple selection 
				// if mouse doesn't get moved)
//				startSelection(posX, posY, posZ);
				d->mSelectionRect.setStart(posX, posY, posZ);
//			}
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
			removeSelectionRect();
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

void BosonBigDisplayBase::slotCenterBase()
{
 //TODO
 // find the command center of the local player
 QPoint pos(0, 0); // note: we use *cell* coordinates!
 
 slotReCenterDisplay(pos);
}

void BosonBigDisplayBase::slotReCenterDisplay(const QPoint& pos)
{
#ifdef NO_OPENGL
 center(pos.x() * BO_TILE_SIZE, pos.y() * BO_TILE_SIZE);
 canvas()->update();
#else
//TODO don't center the corners - e.g. 0;0 should be top left, never center
 setCameraPos(((float)pos.x()) * BO_GL_CELL_SIZE, -((float)pos.y()) * BO_GL_CELL_SIZE, cameraZ());
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
 return r;
}

void BosonBigDisplayBase::setKGameChat(KGameChat* chat)
{
// d->mChat->setChat(chat);
}

void BosonBigDisplayBase::addChatMessage(const QString& message)
{
// d->mChat->addMessage(message);
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
	// hmm currently this happens on startup - should get fixed. paintGL()
	// must be called only once those are loaded!
	kdWarning() << k_funcinfo << "NULL textures for cells" << endl;
	return;
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
 d->mUpdateTimer.stop();
 d->mUpdateTimer.start(ms);
}

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
	kdDebug() << "FPS: " << d->mFps << endl;
 }
 d->mFramecount++;
}

double BosonBigDisplayBase::fps() const
{
  return d->mFps;
}
