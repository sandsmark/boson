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

	BoCamera& operator=(const BoCamera& c);

	static int minCameraZ();
	static int maxCameraZ();
	static int maxCameraRadius();

	void setLookAt(const BoVector3& pos);

	const BoVector3& lookAt() const
	{
		return mLookAt;
	}

	void changeZ(GLfloat diff);
	void changeRadius(GLfloat diff);
	void changeRotation(GLfloat diff);
	void moveLookAtBy(GLfloat x, GLfloat y, GLfloat z);

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
	void setRotation(GLfloat r)
	{
		mRotation = r;
	}
	void setRadius(GLfloat r)
	{
		mRadius = r;
	}
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

private:
	void init();
	static void initStatic();

private:
	BoVector3 mLookAt;
	GLfloat mPosZ;

	GLfloat mRotation;
	GLfloat mRadius;

	// AB: why float?
	GLfloat mMapWidth;
	GLfloat mMapHeight;
};

#endif
