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

#ifndef BOORBITERWIDGET_H
#define BOORBITERWIDGET_H

#include "bosonglwidget.h"

class BoVector3;
class BoQuaternion;
class BoCamera;
class BoMouseMoveDiff;

class BoOrbiterWidgetPrivate;

/**
 * A simple OpenGL orbiter widget. An object at the center of the screen is
 * displayed, with another object orbiting around the center. The user is able
 * to move the orbiting object with the mouse.
 *
 * The main intention for this widget is a camera configuration widget. The
 * orbiting object is the camera and the center object is the center of the
 * scene (e.g. the model in borender). You can use the widget in other places as
 * well, e.g. it could probably get used to define the position of the light -
 * it just needs to be defined by a @ref BoCamera object.
 *
 * Using this widget is very easy - you create it, call @ref setCamera once and
 * then you are done. Nothing else to do for basic uses.
 *
 * There is also a @ref signalChanged signal, which gets emitted whenever the
 * position of the orbiting object changes - as long as your scene updates from
 * the @ref BoCamera object assigned to @ref setCamera regulary, you will not
 * need this signal.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoOrbiterWidget : public BosonGLWidget
{
	Q_OBJECT
public:
	BoOrbiterWidget(QWidget* parent);
	~BoOrbiterWidget();

	void setCamera(BoCamera* camera);

	virtual void paintGL();

signals:
	/**
	 * The position/rotation of @p camera got changed. Of course @p camera
	 * does not have to be an actual camera (could be e.g. position of
	 * light). See also @ref setCamera
	 **/
	void signalChanged(BoCamera* camera);

protected:
	virtual void initializeGL();
	virtual void resizeGL(int width, int height);

	virtual void mousePressEvent(QMouseEvent*);
	virtual void mouseMoveEvent(QMouseEvent*);

	BoCamera* camera() const
	{
		return mCamera;
	}

	void paintCenterObject();
	void paintOrbiterObject();
	void paintOrbiterRotation(float radius); // the lines of the orbit

	void updateOrbiterPosition(const BoVector3& cameraPos, const BoQuaternion& q);
	void updateOrbiterPosition(const BoVector3& cameraPos, const BoVector3& lookAt, const BoVector3& up);

private:
	BoOrbiterWidgetPrivate* d;
	BoCamera* mCamera;
	BoMouseMoveDiff* mMouseMoveDiff;
};

#endif

