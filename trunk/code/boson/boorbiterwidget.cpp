/*
    This file is part of the Boson game
    Copyright (C) 2003 Andreas Beckermann (b_mann@gmx.de)

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

#include "boorbiterwidget.h"
#include "boorbiterwidget.moc"

#include "../bomemory/bodummymemory.h"
#include "bodebug.h"
#include "bo3dtools.h"
#include "bocamera.h"
#include "bomousemovediff.h"
#include "defines.h"
#include <bogl.h>

#include <math.h>

static void paintBox(float size)
{
 glBegin(GL_QUADS);
	glVertex3f(-size, -size, -size);
	glVertex3f(size, -size, -size);
	glVertex3f(size, size, -size);
	glVertex3f(-size, size, -size);

	glVertex3f(-size, -size, size);
	glVertex3f(size, -size, size);
	glVertex3f(size, size, size);
	glVertex3f(-size, size, size);

	glVertex3f(-size, -size, -size);
	glVertex3f(-size, -size, size);
	glVertex3f(-size, size, size);
	glVertex3f(-size, size, -size);

	glVertex3f(size, -size, -size);
	glVertex3f(size, -size, size);
	glVertex3f(size, size, size);
	glVertex3f(size, size, -size);

	glVertex3f(-size, -size, -size);
	glVertex3f(-size, -size, size);
	glVertex3f(size, -size, size);
	glVertex3f(size, -size, -size);

	glVertex3f(-size, size, -size);
	glVertex3f(-size, size, size);
	glVertex3f(size, size, size);
	glVertex3f(size, size, -size);
 glEnd();
}


class BoOrbiterWidgetPrivate
{
public:
	BoOrbiterWidgetPrivate()
	{
	}
};


BoOrbiterWidget::BoOrbiterWidget(QWidget* parent)
	: BosonGLWidget(parent)
{
 d = new BoOrbiterWidgetPrivate;
 connect(this, SIGNAL(signalChanged(BoCamera*)), this, SLOT(slotUpdateGL()));
 mMouseMoveDiff = new BoMouseMoveDiff();
 mCamera = 0;
}

BoOrbiterWidget::~BoOrbiterWidget()
{
 delete mMouseMoveDiff;
 delete d;
}

// set the camera position. the camera defines the position of the object
// orbiting around the origin.
// that object may be the actual camera (then the camera pointer is equal to the
// one used in the application) or e.g. the position of the light or similar
void BoOrbiterWidget::setCamera(BoCamera* camera)
{
 mCamera = camera;
 paintGL();
}

void BoOrbiterWidget::initializeGL()
{
 if (isInitialized()) {
	return;
 }
 BO_CHECK_NULL_RET(context());
 static bool recursive = false;
 if (recursive) {
	return;
 }
 recursive = true;

 glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
 glShadeModel(GL_FLAT);
 glDisable(GL_DITHER);
 glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
 glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST);
 glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);
 glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);




 recursive = false;
}

void BoOrbiterWidget::resizeGL(int w, int h)
{
 boDebug() << k_funcinfo << w << " " << h << endl;
 glViewport(0, 0, (GLsizei)w, (GLsizei)h);
 glMatrixMode(GL_PROJECTION);
 glLoadIdentity();

 float aspect = (float)w / (float)h;
 gluPerspective(60.0f, aspect, BO_GL_NEAR_PLANE, BO_GL_FAR_PLANE);
 glMatrixMode(GL_MODELVIEW);
 glLoadIdentity();

 glClearDepth(1.0f);
 glClear(GL_DEPTH_BUFFER_BIT);
}

void BoOrbiterWidget::paintGL()
{
 if (!isInitialized()) {
	initGL();
	return;
 }
 BO_CHECK_NULL_RET(camera());
 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 glEnable(GL_DEPTH_TEST);
 BoMatrix M;
 BoVector3Float cameraPos(camera()->cameraPos());
 camera()->rotationMatrix().invert(&M);

 glPushMatrix();
 glTranslatef(0.0f, 0.0f, -3.0f);

 // TODO: use the cameraPos properly. probably move the center object (not the
 // camera/orbiter object).

 paintCenterObject();
 const float cameraDist = 1.0f; // distance of camera object from center

 glMultMatrixf(M.data());

 paintOrbiterRotation(cameraDist);
 glTranslatef(0.0f, 0.0f, cameraDist);
 paintOrbiterObject();

 glColor3ub(0, 0, 0);

 glPopMatrix();
}

void BoOrbiterWidget::paintCenterObject()
{
 // TODO: paint a nice object :)
 glColor3ub(255, 255, 255);
 const float size = 0.25f;
 glPushMatrix();
 paintBox(size);
 glPopMatrix();
}

void BoOrbiterWidget::paintOrbiterObject()
{
 // TODO: paint a nice object :)
 glColor3ub(0, 255, 0);
 const float size = 0.25f;
 glPushMatrix();
 paintBox(size);
 glPopMatrix();
}

void BoOrbiterWidget::paintOrbiterRotation(float radius)
{
 glColor3ub(255, 255, 255);

 glColor3ub(0, 0, 255);
 glBegin(GL_LINE_STRIP);
 for (double angle = 0.0; angle <= (2.0 * M_PI); angle += 0.01) {
	glVertex3f(0.0f, radius * (float)sin(angle), radius * (float)cos(angle));
 }
 glEnd();

 glColor3ub(255, 0, 0);
 glBegin(GL_LINE_STRIP);
 for (double angle = 0.0; angle <= (2.0 * M_PI); angle += 0.01) {
	glVertex3f(radius * (float)sin(angle), 0.0f, radius * (float)cos(angle));
 }
 glEnd();
}

void BoOrbiterWidget::mousePressEvent(QMouseEvent* e)
{
 BO_CHECK_NULL_RET(camera());
 switch (e->button()) {
	case QMouseEvent::LeftButton:
	boDebug() << k_funcinfo << "left button" << endl;
		mMouseMoveDiff->moveEvent(e->pos());
		break;
	case QMouseEvent::RightButton:
	boDebug() << k_funcinfo << "right button" << endl;
		mMouseMoveDiff->moveEvent(e->pos());
		break;
	default:
		break;
 }
}

void BoOrbiterWidget::mouseMoveEvent(QMouseEvent* e)
{
 BO_CHECK_NULL_RET(camera());
 mMouseMoveDiff->moveEvent(e);
 // TODO: a nicer UI would be fine. currently we recognize simple horizontal
 // mouse movements only - it would be nice if the user could actually move the
 // orbiter object on the lines.
 int dx = mMouseMoveDiff->dx();
 int dy = mMouseMoveDiff->dy();
 if (e->state() & LeftButton) {
	BoQuaternion q = camera()->quaternion();
	BoVector3Float cameraPos;
	q.transform(&cameraPos, &camera()->cameraPos());

	BoQuaternion q2;
	q2.setRotation((float)dx, 0.0f, 0.0f);

	q2.multiply(q);

	// sometimes the quat is (due to rounding errors) not normalized. fix
	// that.
	q2.normalize();

	BoQuaternion inv = q2.conjugate(); // equal to inverse(), but faster
	inv.transform(&cameraPos, &cameraPos);
	updateOrbiterPosition(cameraPos, q2);
 } else if (e->state() & RightButton) {
	BoQuaternion q = camera()->quaternion();
	BoVector3Float cameraPos;
	q.transform(&cameraPos, &camera()->cameraPos());

	BoQuaternion q2;
	q2.setRotation(0.0f, (float)dx, 0.0f);

	q2.multiply(q);

	// sometimes the quat is (due to rounding errors) not normalized. fix
	// that.
	q2.normalize();

	BoQuaternion inv = q2.conjugate(); // equal to inverse(), but faster
	inv.transform(&cameraPos, &cameraPos);
	updateOrbiterPosition(cameraPos, q2);
 }
}

void BoOrbiterWidget::updateOrbiterPosition(const BoVector3Float& cameraPos, const BoQuaternion& q)
{
 BoVector3Float lookAt, up;
 q.matrix().toGluLookAt(&lookAt, &up, cameraPos);
 updateOrbiterPosition(cameraPos, lookAt, up);
}

void BoOrbiterWidget::updateOrbiterPosition(const BoVector3Float& cameraPos, const BoVector3Float& lookAt, const BoVector3Float& up)
{
 BO_CHECK_NULL_RET(camera());
 camera()->setGluLookAt(cameraPos, lookAt, up);
 emit signalChanged(camera());
}

