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
#ifndef BORENDERMAIN_H
#define BORENDERMAIN_H

#include <qgl.h>
#include <qptrlist.h>
#include <qptrdict.h>
#include <qintdict.h>
#include <qvaluelist.h>

#include <kmainwindow.h>
#include <knuminput.h>

#define MIN_FOVY 0.0
#define MAX_FOVY 360.0
#define MAX_ROTATE 360.0
#define MIN_ROTATE -360.0
#define MIN_CAMERA_X -100.0
#define MAX_CAMERA_X 100.0
#define MIN_CAMERA_Y MIN_CAMERA_X
#define MAX_CAMERA_Y MAX_CAMERA_X
#define MIN_CAMERA_Z 1.0
#define MAX_CAMERA_Z 100.0



class SpeciesTheme;
class UnitProperties;
class BosonModel;
class KMyFloatNumInput : public KDoubleNumInput
{
	Q_OBJECT
public:
	KMyFloatNumInput(QWidget* parent) : KDoubleNumInput(parent)
	{
		connect(this, SIGNAL(valueChanged(double)), this, SLOT(slotValueChanged(double)));
	}
	~KMyFloatNumInput()
	{}

signals:
	void valueChanged(float);

private slots:
	void slotValueChanged(double v) { emit valueChanged((float)v); }
};

class ModelPreview : public QGLWidget
{
	Q_OBJECT
public:
	ModelPreview(QWidget*);
	~ModelPreview();

	virtual void initializeGL();
	virtual void paintGL();
	virtual void resizeGL(int, int);

	void load(SpeciesTheme* s, const UnitProperties* prop);
	void resetModel();

signals:
	void signalFovYChanged(float);
	void signalRotateXChanged(float);
	void signalRotateYChanged(float);
	void signalRotateZChanged(float);
	void signalCameraXChanged(float);
	void signalCameraYChanged(float);
	void signalCameraZChanged(float);

public slots:
	void slotResetView();
	void slotFovYChanged(float f)
	{
		if (f >= MIN_FOVY && f <= MAX_FOVY) {
			mFovY = f;
			resizeGL(width(), height());
		}
	}
	void slotRotateXChanged(float r)
	{
		if (r >= MIN_ROTATE && r <= MAX_ROTATE) {
			mRotateX = r;
		}
	}
	void slotRotateYChanged(float r)
	{
		if (r >= MIN_ROTATE && r <= MAX_ROTATE) {
			mRotateY = r;
		}
	}
	void slotRotateZChanged(float r)
	{
		if (r >= MIN_ROTATE && r <= MAX_ROTATE) {
			mRotateZ = r;
		}
	}
	void slotCameraXChanged(float c)
	{
		if (c >= MIN_CAMERA_X && c <= MAX_CAMERA_X) {
			mCameraX = c;
		}
	}
	void slotCameraYChanged(float c)
	{
		if (c >= MIN_CAMERA_Y && c <= MAX_CAMERA_Y) {
			mCameraY = c;
		}
	}
	void slotCameraZChanged(float c)
	{
		if (c >= MIN_CAMERA_Z && c <= MAX_CAMERA_Z) {
			mCameraZ = c;
		}
	}

protected:
	virtual void mouseMoveEvent(QMouseEvent*);
	virtual void mousePressEvent(QMouseEvent*);
	virtual void mouseReleaseEvent(QMouseEvent*);

private:
	QTimer* mUpdateTimer;
	BosonModel* mModel;
	int mCurrentFrame;

	float mFovY; // we allow real zooming here!
	float mCameraX, mCameraY, mCameraZ;
	float mRotateX, mRotateY, mRotateZ;

	// mouse move:
	int* mMouseDiffX;
	int* mMouseDiffY;
};

class PreviewConfig : public QWidget
{
	Q_OBJECT
public:
	PreviewConfig(QWidget* parent);
	~PreviewConfig();

signals:
	void signalFovYChanged(float);
	void signalRotateXChanged(float);
	void signalRotateYChanged(float);
	void signalRotateZChanged(float);
	void signalCameraXChanged(float);
	void signalCameraYChanged(float);
	void signalCameraZChanged(float);
	void signalResetDefaults();

protected slots:
	void slotFovYChanged(float f) { mFovY->setValue(f); }
	void slotRotateXChanged(float r) { mRotateX->setValue(r); }
	void slotRotateYChanged(float r) { mRotateY->setValue(r); }
	void slotRotateZChanged(float r) { mRotateZ->setValue(r); }
	void slotCameraXChanged(float c) { mCameraX->setValue(c); }
	void slotCameraYChanged(float c) { mCameraY->setValue(c); }
	void slotCameraZChanged(float c) { mCameraZ->setValue(c); }

private:
	KMyFloatNumInput* mFovY;
	KMyFloatNumInput* mRotateX;
	KMyFloatNumInput* mRotateY;
	KMyFloatNumInput* mRotateZ;
	KMyFloatNumInput* mCameraX;
	KMyFloatNumInput* mCameraY;
	KMyFloatNumInput* mCameraZ;
};

class RenderMain : public KMainWindow
{
 	Q_OBJECT
public:
	RenderMain();
	~RenderMain();

protected:
	void initKAction();

	QValueList<unsigned long int> allUnits(SpeciesTheme*) const;

protected:
	void connectBoth(QObject* o1, QObject* o2, const char* signal, const char* slot);

protected slots:
	void slotUnitChanged(int);

private:
	PreviewConfig* mConfig;
	ModelPreview* mPreview;
	QPtrList<SpeciesTheme> mSpecies;
	QPtrDict<SpeciesTheme> mPopup2Species;
};

#endif

