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

#ifndef BOCAMERA_H
#define BOCAMERA_H

#include "bo3dtools.h"

#include <GL/gl.h>

class QDomElement;

/**
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BoCamera
{
public:
	BoCamera();
	// AB: IMHO its a bad idea to place the map width/height into camera
	// code
	BoCamera(GLfloat mapWidth, GLfloat mapHeight);

	BoCamera(const BoCamera& c)
	{
		*this = c;
	}

	/**
	 * Apply the camera to the scene by doing the necessary OpenGL
	 * transformation on the modelview matrix.
	 *
	 * This will first load the identity matrix, so any previous changes are
	 * lost. use glPushMatrix()/glPopMatrix() if you need your old settings
	 * back at a later point.
	 **/
	void applyCameraToScene();

	BoCamera& operator=(const BoCamera& c);

	static float minCameraZ();
	static float maxCameraZ();
	static float maxCameraRadius();

	/**
	 * Set the gluLookAt() paremeters directly. Note that when you use
	 * this @ref radius and @ref rotation will remain undefined.
	 **/
	void setGluLookAt(const BoVector3& lookAt, const BoVector3& cameraPos, const BoVector3& up);

	/**
	 * @param pos The point to look at, as used in gluLookAt().
	 **/
	void setLookAt(const BoVector3& pos);

	/**
	 * @return The point we are looking at. This is the lookAt vector, as it
	 * can get used by gluLookAt().
	 **/
	const BoVector3& lookAt() const
	{
		return mLookAt;
	}
	/**
	 * @return The eye vector (camera position), as it can get used by
	 * gluLookAt().
	 **/
	const BoVector3& cameraPos() const
	{
		return mCameraPos;
	}

	/**
	 * @return The up vector, as it can get used by gluLookAt(). The up
	 * vector is the vector pointing straight "up" from the position of the
	 * camera. it can change when the camera is rotated.
	 **/
	const BoVector3& up() const
	{
		return mUp;
	}

	void changeZ(GLfloat diff);
	void changeRadius(GLfloat diff);
	void changeRotation(GLfloat diff);
	void moveLookAtBy(GLfloat x, GLfloat y, GLfloat z);

	/**
	 * Set limits for the camera. The camera tries not to move beyond the
	 * map size.
	 **/
	void setMapSize(GLfloat w, GLfloat h)
	{
		mMapWidth = w;
		mMapHeight = h;
	}

	void loadFromXML(const QDomElement& root);
	void saveAsXML(QDomElement& root);

	void setZ(GLfloat z)
	{
		mPosZ = z;
	}

	// these will change the up and cameraPos vectors!
	// TODO: document what they actually do
	void setRotation(GLfloat r);
	void setRadius(GLfloat r);


	GLfloat z() const
	{
		return mPosZ;
	}
	GLfloat rotation() const
	{
		return mRotation;
	}
	GLfloat radius() const
	{
		return mRadius;
	}


protected:
	void checkPosition();

	/**
	 * Update the parameters for gluLookAt() (@ref lookAt, @ref cameraPos
	 * and @ref * up) according to the new values from @ref radius and
	 * @ref rotation.
	 *
	 * Note that the @ref lookAt vector isn't changed here, as the kind of
	 * BoCamera's rotation and radius don't influence it.
	 **/
	void updateFromRadiusAndRotation();

private:
	void init();
	static void initStatic();

private:
	BoVector3 mLookAt;
	BoVector3 mUp;

	GLfloat mPosZ;

	GLfloat mRotation;
	GLfloat mRadius;

	BoVector3 mCameraPos;

	// AB: why float?
	GLfloat mMapWidth;
	GLfloat mMapHeight;
};

#endif
