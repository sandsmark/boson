/*
    This file is part of the Boson game
    Copyright (C) 2002-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bodebugdcopiface.h"
#include "bosonglwidget.h"

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


class BoMouseMoveDiff;
class SpeciesTheme;
class UnitProperties;
class BosonModel;
class BosonGLFont;
class BoCamera;
class QCheckBox;

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

class KMyIntNumInput : public KIntNumInput
{
	Q_OBJECT
public:
	KMyIntNumInput(QWidget* parent) : KIntNumInput(parent)
	{
		connect(this, SIGNAL(valueChanged(int)), this, SLOT(slotValueChanged(int)));
	}
	~KMyIntNumInput()
	{}

signals:
	void valueChanged(float);

private slots:
	void slotValueChanged(int v) { emit valueChanged((float)v); }
};

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class ModelPreview : public BosonGLWidget
{
	Q_OBJECT
public:
	ModelPreview(QWidget*);
	~ModelPreview();

	virtual void initializeGL();
	virtual void paintGL();
	virtual void resizeGL(int, int);

	void load(SpeciesTheme* s, const UnitProperties* prop);
	void loadObjectModel(SpeciesTheme* s, const QString& file);
	void resetModel();

	BoCamera* camera() const { return mCamera; }

signals:
	void signalFovYChanged(float);
	void signalRotateXChanged(float);
	void signalRotateYChanged(float);
	void signalRotateZChanged(float);
	void signalCameraXChanged(float);
	void signalCameraYChanged(float);
	void signalCameraZChanged(float);
	void signalFrameChanged(int);
	void signalLODChanged(int);

	void signalMaxFramesChanged(int);

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
	void slotFrameChanged(int f);
	void slotLODChanged(int l);
	void slotPlacementPreviewChanged(bool on)
	{
		mPlacementPreview = on;
	}
	void slotDisallowPlacementChanged(bool on)
	{
		mDisallowPlacement = on;
	}
	void slotWireFrameChanged(bool on)
	{
		mWireFrame = on;
	}

protected:
	virtual bool eventFilter(QObject* o, QEvent* e);
	virtual void mouseMoveEvent(QMouseEvent*);
	virtual void mousePressEvent(QMouseEvent*);
	virtual void mouseReleaseEvent(QMouseEvent*);

	void renderModel(int mode = -1);
	void renderGrid();
	void renderMeshSelection();
	void renderText();

	bool haveModel() const
	{
		if (mModel && mCurrentFrame >= 0) {
			return true;
		}
		return false;
	}

	/**
	 * Update the text that displays information on what is under the cursor
	 **/
	void updateCursorDisplay(const QPoint& pos);
	int pickObject(const QPoint& pos);

private:
	friend class RenderMain; // we need to emit signals from outside, in order to save lots of forwarding code
	QTimer* mUpdateTimer;
	BosonModel* mModel;
	int mCurrentFrame;
	int mCurrentLOD;
	int mMeshUnderMouse;
	int mSelectedMesh;

	BosonGLFont* mDefaultFont;

	float mFovY; // we allow real zooming here!
	float mCameraX, mCameraY, mCameraZ;
	float mRotateX, mRotateY, mRotateZ;

	bool mPlacementPreview;
	bool mDisallowPlacement;
	bool mWireFrame;

	BoMouseMoveDiff* mMouseMoveDiff;

	BoCamera* mCamera;
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
	void signalFrameChanged(int);
	void signalLODChanged(int);
	void signalResetDefaults();
	void signalPlacementPreviewChanged(bool); // display preview placement - if false display normal model
	void signalDisallowPlacementChanged(bool); // only valid of placementpreview is also on. if true display the model that is shown when the unit can't be placed - otherwise the model that is shown if it can be placed.
	void signalWireFrameChanged(bool);

protected slots:
	void slotFovYChanged(float f) { mFovY->setValue(f); }
	void slotRotateXChanged(float r) { mRotateX->setValue((int)r); }
	void slotRotateYChanged(float r) { mRotateY->setValue((int)r); }
	void slotRotateZChanged(float r) { mRotateZ->setValue((int)r); }
	void slotCameraXChanged(float c) { mCameraX->setValue(c); }
	void slotCameraYChanged(float c) { mCameraY->setValue(c); }
	void slotCameraZChanged(float c) { mCameraZ->setValue(c); }
	void slotFrameChanged(int f) { mFrame->setValue(f); }
	void slotLODChanged(int l) { mFrame->setValue(l); }

	void slotMaxFramesChanged(int max) { mFrame->setRange(0, max); }

private:
	KMyFloatNumInput* mFovY;
	KMyFloatNumInput* mRotateX;
	KMyFloatNumInput* mRotateY;
	KMyFloatNumInput* mRotateZ;
	KMyFloatNumInput* mCameraX;
	KMyFloatNumInput* mCameraY;
	KMyFloatNumInput* mCameraZ;
	KIntNumInput* mFrame;
	KIntNumInput* mLOD;
	QCheckBox* mPlacementPreview;
	QCheckBox* mDisallowPlacement;
	QCheckBox* mWireFrame;
};

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class RenderMain : public KMainWindow
{
	Q_OBJECT
public:
	RenderMain();
	~RenderMain();

	void changeUnit(const QString& speciesIdentifier, const QString& unit);
	void changeUnit(const QString& speciesIdentifier, unsigned long int unitType);
	void changeObject(const QString& speciesIdentifier, const QString& file);

	void emitSignalFovY(float f) { emit mPreview->signalFovYChanged(f); }
	void emitSignalRotateX(float r) { emit mPreview->signalRotateXChanged(r); }
	void emitSignalRotateY(float r) { emit mPreview->signalRotateYChanged(r); }
	void emitSignalRotateZ(float r) { emit mPreview->signalRotateZChanged(r); }
	void emitSignalCameraX(float c) { emit mPreview->signalCameraXChanged(c); }
	void emitSignalCameraY(float c) { emit mPreview->signalCameraYChanged(c); }
	void emitSignalCameraZ(float c) { emit mPreview->signalCameraZChanged(c); }
	void emitSignalFrame(int f) { emit mPreview->signalFrameChanged(f); }
	void emitSignalLOD(int l) { emit mPreview->signalLODChanged(l); }

protected:
	void initKAction();

protected:
	/**
	 * Connect both objects to the signal. Both objects need to provide the
	 * signal and the slot (i.e. it must have the same name/params)
	 **/
	void connectBoth(QObject* o1, QObject* o2, const char* signal, const char* slot);

	/**
	 * Display another unit. This will load the unit from the @ref
	 * ModelPreview. See @ref ModelPreview::load
	 **/
	void changeUnit(SpeciesTheme* s, const UnitProperties* prop);

	void changeObject(SpeciesTheme* s, const QString& file);

	SpeciesTheme* findTheme(const QString& theme) const;
	void uncheckAllBut(KAction*); // BAH!

protected slots:
	void slotUnitChanged(int);
	void slotObjectChanged(int);
	void slotDebugModels();
	void slotDebugSpecies();

private:
	PreviewConfig* mConfig;
	ModelPreview* mPreview;
	QPtrList<SpeciesTheme> mSpecies;
	QPtrDict<SpeciesTheme> mAction2Species;
	BoDebugDCOPIface* mIface;
};

#endif

