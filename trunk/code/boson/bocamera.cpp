/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bocamera.h"

#include "defines.h"
#include "bosonconfig.h"
#include "bodebug.h"

#include <qdom.h>

// Camera limits
#define CAMERA_MIN_Z BO_GL_NEAR_PLANE + 3
#define CAMERA_MAX_Z BO_GL_FAR_PLANE - 50
#define CAMERA_MAX_RADIUS 80

#include <GL/glu.h>

BoCamera::BoCamera()
{
 init();
}

BoCamera::BoCamera(GLfloat mapWidth, GLfloat mapHeight)
{
 init();
 setMapSize(mapWidth, mapHeight);
}

BoCamera& BoCamera::operator=(const BoCamera& c)
{
 mPosZ = c.mPosZ;
 mMapWidth = c.mMapWidth;
 mMapHeight = c.mMapHeight;
 setLookAt(c.lookAt());
 setRotation(c.rotation());
 setRadius(c.radius());
 return *this;
}

void BoCamera::init()
{
 initStatic();
 mLookAt.set(0.0f, 0.0f, 0.0f);
 mPosZ = 8.0f;
 mRotation = 0.0f;
 mRadius = 5.0f;
 mMapWidth = 0.0f;
 mMapHeight = 0.0f;
 updateFromRadiusAndRotation();
}

void BoCamera::initStatic()
{
 static bool initialized = false;
 if (initialized) {
	return;
 }
 initialized = true;
 boConfig->addDynamicEntry(new BoConfigDoubleEntry(boConfig, "CameraMinZ",
		CAMERA_MIN_Z));
 boConfig->addDynamicEntry(new BoConfigDoubleEntry(boConfig, "CameraMaxZ",
		CAMERA_MAX_Z));
 boConfig->addDynamicEntry(new BoConfigDoubleEntry(boConfig, "CameraMaxRadius",
		CAMERA_MAX_RADIUS));
}

float BoCamera::minCameraZ()
{
 initStatic();
 return (float)boConfig->doubleValue("CameraMinZ");
}

float BoCamera::maxCameraZ()
{
 initStatic();
 return (float)boConfig->doubleValue("CameraMaxZ");
}

float BoCamera::maxCameraRadius()
{
 initStatic();
 return (float)boConfig->doubleValue("CameraMaxRadius");
}

void BoCamera::setLookAt(const BoVector3& pos)
{
 mLookAt = pos;
 updateFromRadiusAndRotation(); // eye and up vectors will/may change as well
 checkPosition();
}

void BoCamera::changeZ(GLfloat diff)
{
 float newz = mPosZ + diff;
 if (newz < CAMERA_MIN_Z) {
	newz = CAMERA_MIN_Z;
 } else if (newz > CAMERA_MAX_Z) {
	newz = CAMERA_MAX_Z;
 }
 float factor = newz / mPosZ;
 mPosZ = newz;
 setRadius(radius() * factor);
}

void BoCamera::changeRadius(GLfloat diff)
{
 float radius = this->radius() + mPosZ / CAMERA_MAX_RADIUS * diff;  // How much radius is changed depends on z position
 if (radius < 0.0f) {
	radius = 0.0f;
 } else if (radius > mPosZ) {
	radius = mPosZ;
 }
 setRadius(radius);
}

void BoCamera::changeRotation(GLfloat diff)
{
 float rotation = this->rotation() + diff;
 if (rotation < 0.0f) {
	rotation += 360.0f;
 } else if (rotation > 360.0f) {
	rotation -= 360.0f;
 }
 setRotation(rotation);
}

void BoCamera::moveLookAtBy(GLfloat x, GLfloat y, GLfloat z)
{
 BoVector3 v = lookAt();
 v.add(BoVector3(x, y, z));
 setLookAt(v);
 checkPosition();
}

void BoCamera::loadFromXML(const QDomElement& root)
{
 bool ok;
 float lookatx, lookaty, lookatz;
 lookatx = root.attribute("LookAtX").toFloat(&ok);
 if (!ok) {
	boError() << k_funcinfo << "Invalid value for LookAtX tag" << endl;
	lookatx = 0.0f;
 }
 lookaty = root.attribute("LookAtY").toFloat(&ok);
 if (!ok) {
	boError() << k_funcinfo << "Invalid value for LookAtY tag" << endl;
	lookaty = 0.0f;
 }
 lookatz = root.attribute("LookAtZ").toFloat(&ok);
 if (!ok) {
	boError() << k_funcinfo << "Invalid value for LookAtZ tag" << endl;
	mPosZ = 0.0f;
 }
 mPosZ = root.attribute("PosZ").toFloat(&ok);
 if (!ok) {
	boError() << k_funcinfo << "Invalid value for PosZ tag" << endl;
	mPosZ = 0.0f;
 }
 float rotation= root.attribute("Rotation").toFloat(&ok);
 if (!ok) {
	boError() << k_funcinfo << "Invalid value for Rotation tag" << endl;
	rotation = 0.0f;
 }
 setRotation(rotation);
 float radius = root.attribute("Radius").toFloat(&ok);
 if (!ok) {
	boError() << k_funcinfo << "Invalid value for Radius tag" << endl;
	radius = 0.0f;
 }
 setRadius(radius);
 boDebug(260) << k_funcinfo << "Setting lookat to (" << lookatx << ", " << lookaty << ", " << lookatz << ")" << endl;
 BoVector3 newLookAt(lookatx, lookaty, lookatz);
 setLookAt(newLookAt);
 boDebug(260) << k_funcinfo << "lookat is now (" << lookAt().x() << ", " << lookAt().y() << ", " << lookAt().z() << ")" << endl;
 }

void BoCamera::saveAsXML(QDomElement& root)
{
 root.setAttribute("LookAtX", lookAt().x());
 root.setAttribute("LookAtY", lookAt().y());
 root.setAttribute("LookAtZ", lookAt().z());
 root.setAttribute("PosZ", z());
 root.setAttribute("Rotation", rotation());
 root.setAttribute("Radius", radius());
}

void BoCamera::checkPosition()
{
 if (!mMapWidth || !mMapHeight) {
	return;
 }
 if (lookAt().x() < 0.0f) {
	BoVector3 v = lookAt();
	v.setX(0.0f);
	setLookAt(v);
 } else if (lookAt().x() > mMapWidth) {
	BoVector3 v = lookAt();
	v.setX(mMapWidth);
	setLookAt(v);
 }
 if (lookAt().y() > 0.0f) {
	BoVector3 v = lookAt();
	v.setY(0.0f);
	setLookAt(v);
 } else if (lookAt().y() < -mMapHeight) {
	BoVector3 v = lookAt();
	v.setY(-mMapHeight);
	setLookAt(v);
 }
}

void BoCamera::applyCameraToScene()
{
 glMatrixMode(GL_MODELVIEW);
 glLoadIdentity();

 gluLookAt(eye().x(), eye().y(), eye().z(),
		lookAt().x(), lookAt().y(), lookAt().z(),
		up().x(), up().y(), up().z());
}

void BoCamera::updateFromRadiusAndRotation()
{
 float diffX = 0.0f;
 float diffY = 0.0f;
 float radius = this->radius();
 if (radius <= 0.02f) {
	// If radius is 0, up vector will be wrong so we change it
	radius = 0.02f;
 }
 pointByRotation(&diffX, &diffY, this->rotation(), radius);
 float eyeX, eyeY, eyeZ;  // Position of camera
 eyeX = lookAt().x() + diffX;
 eyeY = lookAt().y() + diffY;
 eyeZ = lookAt().z() + z();
 mCameraPos.set(eyeX, eyeY, eyeZ);
 float upX, upY, upZ;  // up vector (points straight up in viewport)
 upX = -diffX;
 upY = -diffY;
 upZ = 0.0f;

 mEye.set(eyeX, eyeY, eyeZ);
 mUp.set(upX, upY, upZ);
}

void BoCamera::setRadius(GLfloat r)
{
 mRadius = r;
 updateFromRadiusAndRotation();
}

void BoCamera::setRotation(GLfloat r)
{
 mRotation = r;
 updateFromRadiusAndRotation();
}

