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
 mMapWidth = mapWidth;
 mMapHeight = mapHeight;
}

BoCamera& BoCamera::operator=(const BoCamera& c)
{
 mLookAt = c.mLookAt;
 mPosZ = c.mPosZ;
 mRotation = c.mRotation;
 mRadius = c.mRadius;
 mMapWidth = c.mMapWidth;
 mMapHeight = c.mMapHeight;
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
}

void BoCamera::initStatic()
{
 static bool initialized = false;
 if (initialized) {
	return;
 }
 initialized = true;
 boConfig->addDynamicEntry(new BoConfigIntEntry(boConfig, "CameraMinZ",
		CAMERA_MIN_Z));
 boConfig->addDynamicEntry(new BoConfigIntEntry(boConfig, "CameraMaxZ",
		CAMERA_MAX_Z));
 boConfig->addDynamicEntry(new BoConfigIntEntry(boConfig, "CameraMaxRadius",
		CAMERA_MAX_RADIUS));
}

int BoCamera::minCameraZ()
{
 initStatic();
 return boConfig->intValue("CameraMinZ");
}

int BoCamera::maxCameraZ()
{
 initStatic();
 return boConfig->intValue("CameraMaxZ");
}

int BoCamera::maxCameraRadius()
{
 initStatic();
 return boConfig->intValue("CameraMaxRadius");
}

void BoCamera::setLookAt(const BoVector3& pos)
{
 mLookAt = pos;
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
 mRadius = mRadius * factor;
}

void BoCamera::changeRadius(GLfloat diff)
{
 float radius = mRadius + mPosZ / CAMERA_MAX_RADIUS * diff;  // How much radius is changed depends on z position
 if (radius < 0.0f) {
	radius = 0.0f;
 } else if (radius > mPosZ) {
	radius = mPosZ;
 }
 mRadius = radius;
}

void BoCamera::changeRotation(GLfloat diff)
{
 float rotation = mRotation + diff;
 if (rotation < 0.0f) {
	rotation += 360.0f;
 } else if (rotation > 360.0f) {
	rotation -= 360.0f;
 }
 mRotation = rotation;
}

void BoCamera::moveLookAtBy(GLfloat x, GLfloat y, GLfloat z)
{
 mLookAt.add(BoVector3(x, y, z));
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
 mRotation = root.attribute("Rotation").toFloat(&ok);
 if (!ok) {
	boError() << k_funcinfo << "Invalid value for Rotation tag" << endl;
	mRotation = 0.0f;
 }
 mRadius = root.attribute("Radius").toFloat(&ok);
 if (!ok) {
	boError() << k_funcinfo << "Invalid value for Radius tag" << endl;
	mRadius = 0.0f;
 }
 boDebug(260) << k_funcinfo << "Setting lookat to (" << lookatx << ", " << lookaty << ", " << lookatz << ")" << endl;
 mLookAt.set(lookatx, lookaty, lookatz);
 boDebug(260) << k_funcinfo << "lookat is now (" << mLookAt.x() << ", " << mLookAt.y() << ", " << mLookAt.z() << ")" << endl;
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
 if (mLookAt.x() < 0.0f) {
	mLookAt.setX(0.0f);
 } else if (mLookAt.x() > mMapWidth) {
	mLookAt.setX(mMapWidth);
 }
 if (mLookAt.y() > 0.0f) {
	mLookAt.setY(0.0f);
 } else if (mLookAt.y() < -mMapHeight) {
	mLookAt.setY(-mMapHeight);
 }
}

void BoCamera::applyCameraToScene()
{
 glMatrixMode(GL_MODELVIEW); // default matrix mode anyway ; redundant!
 glLoadIdentity();

 float diffX, diffY;
 float radius = this->radius();
 if (radius <= 0.02f) {
	// If radius is 0, up vector will be wrong so we change it
	radius = 0.02f;
 }
 pointByRotation(&diffX, &diffY, this->rotation(), radius);
 float lookatX, lookatY, lookatZ;  // Point that we look at
 lookatX = lookAt().x();
 lookatY = lookAt().y();
// lookatZ = 0.0f;
 lookatZ = lookAt().z();
 float eyeX, eyeY, eyeZ;  // Position of camera
 eyeX = lookatX + diffX;
 eyeY = lookatY + diffY;
 eyeZ = lookatZ + z();
 mCameraPos.set(eyeX, eyeY, eyeZ);
 float upX, upY, upZ;  // up vector (points straight up in viewport)
 upX = -diffX;
 upY = -diffY;
 upZ = 0.0f;

 gluLookAt(eyeX, eyeY, eyeZ,
		lookatX, lookatY, lookatZ,
		upX, upY, upZ);
}

