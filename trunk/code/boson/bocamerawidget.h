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

#ifndef BOCAMERAWIDGET_H
#define BOCAMERAWIDGET_H

#include <qwidget.h>

class BoCamera;
class BoGameCamera;
class BoCameraConfigWidgetBase;
class BoVector3;

class BoCameraWidgetPrivate;

/**
 * The camera widget lets the user configure the @ref BoCamera widget in
 * different ways. Every way is defined by a @ref BoCameraConfigWidgetBase
 * derived class, some of which may be relevant to the game camera only (such as
 * @ref BoGameCameraWidget).
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoCameraWidget : public QWidget
{
	Q_OBJECT
public:
	BoCameraWidget(QWidget* parent, const char* name = 0);
	~BoCameraWidget();

	void setCamera(BoCamera* camera);

public slots:
	void slotUpdateFromCamera();

protected:
	void addConfigWidget(const QString& name, BoCameraConfigWidgetBase* widget);


private:
	BoCameraWidgetPrivate* d;
};

/**
 * Base class for all widgets that allow manipulating the camera.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoCameraConfigWidgetBase : public QWidget
{
	Q_OBJECT
public:
	BoCameraConfigWidgetBase(QWidget* parent, const char* name = 0);
	~BoCameraConfigWidgetBase()
	{
	}
	void setCamera(BoCamera* camera)
	{
		mCamera = camera;
	}

	bool updatesBlocked() const
	{
		return mUpdatesBlocked;
	}

	/**
	 * @return The @ref BoCamera::CameraType value that is required by this
	 * widget. Simply use @ref BoCamera::Camera if you have no special
	 * requirements.
	 **/
	virtual int needCameraType() const = 0;

	BoCamera* camera() const
	{
		return mCamera;
	}

	virtual void updateFromCamera() = 0;

protected:
	/**
	 * If @p block is TRUE @ref updateFromCamera won't be called until you
	 * call this function with @p block = FALSE again.
	 *
	 * You should use this when you emit @ref signalCameraChanged from your
	 * implementation, unless you now exactly that no rounding errors will
	 * influence you (you will never know that).
	 **/
	void blockUpdates(bool block)
	{
		mUpdatesBlocked = block;
	}

	void emitSignalCameraChanged()
	{
		blockUpdates(true);
		emit signalCameraChanged();

		// we want to update the matrix widget even if signals are
		// blocked.
		updateMatrixWidget();
		blockUpdates(false);
	}

	/**
	 * Update the matrix widget, if this widget has one. The matrix widget
	 * is even updated if updates are blocked.
	 **/
	virtual void updateMatrixWidget() {}

signals:
	void signalCameraChanged();

private:
	BoCamera* mCamera;
	bool mUpdatesBlocked;
};

class BoGameCameraWidgetPrivate;
/**
 * This is the camera configuration as it is used in boson by default.
 * it depends on lookAt vector, radius and rotation only.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoGameCameraWidget : public BoCameraConfigWidgetBase
{
	Q_OBJECT
public:
	BoGameCameraWidget(QWidget* parent, const char* name = 0);
	~BoGameCameraWidget();

	virtual int needCameraType() const;
	virtual void updateFromCamera();

	BoGameCamera* gameCamera() const
	{
		return (BoGameCamera*)camera();
	}

protected slots:
	void slotLookAtChanged();
	void slotRotationChanged();
	void slotRadiusChanged();

	void slotToggleGameRestrictions();

private:
	BoGameCameraWidgetPrivate* d;
};

class BoGLUCameraWidgetPrivate;
/**
 * Direct manipulation of gluLookAt() through the three vectors lookAt,
 * cameraPos and up.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoGLUCameraWidget : public BoCameraConfigWidgetBase
{
	Q_OBJECT
public:
	BoGLUCameraWidget(QWidget* parent, const char* name = 0);
	~BoGLUCameraWidget();

	virtual int needCameraType() const;

	virtual void updateFromCamera();

protected slots:
	void slotLookAtChanged();
	void slotCameraPosChanged();
	void slotUpChanged();

private:
	BoGLUCameraWidgetPrivate* d;
};

class BoPlainCameraWidgetPrivate;
/**
 * This widget consists of glTranslate() and glRotate() calls only.
 *
 * The widget allows to modify the rotation about the x-,y-,z-axis and the
 * camera position. It will then generate values for gluLookAt(), so that it
 * will behave as if the following code would have been executed
 * <pre>
 * glRotatef(rotationX, 1.0, 0.0, 0.0);
 * glRotatef(rotationY, 0.0, 1.0, 0.0);
 * glRotatef(rotationZ, 0.0, 0.0, 1.0);
 * glTranslatef(-cameraPosX, -cameraPosY, -cameraPosZ);
 * </pre>
 * (remember that -cameraPosX comes from the fact that we are moving the
 * coordinate system, not the camera, in OpenGL. it is the same call that
 * gluLookAt() uses)
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoPlainCameraWidget : public BoCameraConfigWidgetBase
{
	Q_OBJECT
public:
	BoPlainCameraWidget(QWidget* parent, const char* name = 0);
	~BoPlainCameraWidget();

	virtual int needCameraType() const;

	/**
	 * Warning: this is a complex method! It depends on a lot of maths!
	 * There is most probably an easier way of doing this - one that is even
	 * better and safer (concerning invalid values). But I considered this a
	 * good opportunity to get some practice in OpenGL maths.
	 *
	 * This method takes the cameraPos, lookAt and up vectors from @ref
	 * BoCamera and tries to convert it into angles for glRotate(). Every
	 * glRotate() call is about one of the x-,y-,z-axis.
	 **/
	virtual void updateFromCamera();

protected slots:
	void slotCameraChanged();
	void slotTranslateFirstChanged();
	void slotShowMatricesChanged(bool show);

protected:
	virtual void updateMatrixWidget();

	bool translateFirst() const;

private:
	BoPlainCameraWidgetPrivate* d;
};

#endif

