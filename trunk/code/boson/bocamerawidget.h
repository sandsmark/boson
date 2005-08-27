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

#ifndef BOCAMERAWIDGET_H
#define BOCAMERAWIDGET_H

#include <qwidget.h>
#include "boufo/boufo.h"

class BoCamera;
class BoGameCamera;
class BoLight;
class BoLightCamera;
class BoContext;

class BosonGLWidgetLight;
class BoUfoLightCameraWidget;

/**
 * This widget allows configuring all aspects of a light (ambient, diffuse,
 * specular color, ...) and therefore it is not actually a "camera" widget.
 *
 * But nevertheless the main purpose is to configure the position of the light
 * and it therefore is very similar to configuring the camera.
 *
 * This widget does NOT provide a separate camera configuration widget and
 * therefore it is not derived of @ref BoCameraConfigWidgetBase. Instead it
 * <em>uses</em> the other camera widget and therefore it contains a @ref
 * BoUfoCameraWidget object.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoLightCameraWidget1 : public QWidget
{
	Q_OBJECT
public:
	BoLightCameraWidget1(QWidget* parent = 0, bool showGlobalValues = false);
	~BoLightCameraWidget1();

	void setLight(BoLight* light, BoContext* context);

private:
	BosonGLWidgetLight* mWidget;
	BoUfoLightCameraWidget* mLightWidget;
};


class BoUfoCameraConfigWidgetBase;
class BoUfoCameraWidgetPrivate;
/**
 * The camera widget lets the user configure the @ref BoCamera widget in
 * different ways. Every way is defined by a @ref BoUfoCameraConfigWidgetBase
 * derived class, some of which may be relevant to the game camera only (such as
 * @ref BoUfoGameCameraWidget).
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoUfoCameraWidget : public BoUfoWidget
{
	Q_OBJECT
public:
	BoUfoCameraWidget();
	~BoUfoCameraWidget();

	void setCamera(BoCamera* camera);

public slots:
	void slotUpdateFromCamera();

protected:
	void addConfigWidget(const QString& name, BoUfoCameraConfigWidgetBase* widget);


private:
	BoUfoCameraWidgetPrivate* d;
};

/**
 * Base class for all widgets that allow manipulating the camera.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoUfoCameraConfigWidgetBase : public BoUfoWidget
{
	Q_OBJECT
public:
	BoUfoCameraConfigWidgetBase();
	~BoUfoCameraConfigWidgetBase()
	{
	}
	virtual void setCamera(BoCamera* camera)
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


class BoUfoGLUCameraWidgetPrivate;
/**
 * Direct manipulation of gluLookAt() through the three vectors lookAt,
 * cameraPos and up.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoUfoGLUCameraWidget : public BoUfoCameraConfigWidgetBase
{
	Q_OBJECT
public:
	BoUfoGLUCameraWidget();
	~BoUfoGLUCameraWidget();

	virtual int needCameraType() const;

	virtual void updateFromCamera();

protected slots:
	void slotLookAtChanged();
	void slotCameraPosChanged();
	void slotUpChanged();

protected:
	virtual void updateMatrixWidget();

private:
	BoUfoGLUCameraWidgetPrivate* d;
};

class BoUfoPlainCameraWidgetPrivate;
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
class BoUfoPlainCameraWidget : public BoUfoCameraConfigWidgetBase
{
	Q_OBJECT
public:
	BoUfoPlainCameraWidget();
	~BoUfoPlainCameraWidget();

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
	BoUfoPlainCameraWidgetPrivate* d;
};

class BoUfoGameCameraWidgetPrivate;
/**
 * This is the camera configuration as it is used in boson by default.
 * it depends on lookAt vector, radius and rotation only.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoUfoGameCameraWidget : public BoUfoCameraConfigWidgetBase
{
	Q_OBJECT
public:
	BoUfoGameCameraWidget();
	~BoUfoGameCameraWidget();

	virtual int needCameraType() const;
	virtual void updateFromCamera();

	BoGameCamera* gameCamera() const
	{
		return (BoGameCamera*)camera();
	}

protected slots:
	void slotLookAtChanged();
	void slotRotationChanged();
	void slotXRotationChanged();
	void slotDistanceChanged();

	void slotToggleGameRestrictions();

private:
	BoUfoGameCameraWidgetPrivate* d;
};


class BoUfoOrbiterCameraWidgetPrivate;
class BoUfoOrbiterCameraWidget : public BoUfoCameraConfigWidgetBase
{
	Q_OBJECT
public:
	BoUfoOrbiterCameraWidget();
	~BoUfoOrbiterCameraWidget();

	virtual void setCamera(BoCamera* camera);

	virtual int needCameraType() const;

	virtual void updateFromCamera();

protected slots:
	void slotCameraChanged();

protected:
	virtual void updateMatrixWidget();

private:
	BoUfoOrbiterCameraWidgetPrivate* d;
};


class BoUfoLightCameraWidget : public BoUfoWidget
{
	Q_OBJECT
public:
	/**
	 * @param showGlobalValues If TRUE this will allow you to edit the
	 * global light values, i.e. the light model (see man glLightModel). You
	 * should ensure that only one widget is allowed to modify the global
	 * values, to avoid unsynchronized valus.
	 **/
	BoUfoLightCameraWidget(bool showGlobalValues = false);
	~BoUfoLightCameraWidget();

	void setLight(BoLight* light, BoContext* context);

private slots:
	void slotLightChanged();
	void slotLightModelChanged();

private:
	BoLightCamera* mCamera;
	BoUfoCameraWidget* mCameraWidget;
	BoLight* mLight;
	BoContext* mContext;
	bool mBlockLightChanges;
	bool mShowGlobalValues;

	BoUfoCheckBox* mDirectional;
	BoUfoNumInput* mConstantAttenuation;
	BoUfoNumInput* mLinearAttenuation;
	BoUfoNumInput* mQuadraticAttenuation;
	BoUfoNumInput* mAmbientR;
	BoUfoNumInput* mAmbientG;
	BoUfoNumInput* mAmbientB;
	BoUfoNumInput* mAmbientA;
	BoUfoNumInput* mDiffuseR;
	BoUfoNumInput* mDiffuseG;
	BoUfoNumInput* mDiffuseB;
	BoUfoNumInput* mDiffuseA;
	BoUfoNumInput* mSpecularR;
	BoUfoNumInput* mSpecularG;
	BoUfoNumInput* mSpecularB;
	BoUfoNumInput* mSpecularA;

	BoUfoNumInput* mGlobalAmbientR;
	BoUfoNumInput* mGlobalAmbientG;
	BoUfoNumInput* mGlobalAmbientB;
	BoUfoNumInput* mGlobalAmbientA;

};

#endif

