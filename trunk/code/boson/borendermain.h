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
#include <qpushbutton.h>

#include <kmainwindow.h>
#include <knuminput.h>

#define MIN_FOVY 0.0
#define MAX_FOVY 180.0


class BoMouseMoveDiff;
class SpeciesTheme;
class UnitProperties;
class BosonModel;
class BosonGLFont;
class BoCamera;
class BoCameraWidget;
class BoMatrix;
class BoQuaternion;
class BoVector3;
class BoFrame;
class BoLight;
class BoLightCameraWidget;
class BoMaterialWidget;
class KCmdLineArgs;
class QCheckBox;
class BoFontInfo;

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

	void setFont(const BoFontInfo& font);
	const BoFontInfo& fontInfo() const;

	void load(SpeciesTheme* s, const UnitProperties* prop);
	void loadObjectModel(SpeciesTheme* s, const QString& file);
	void resetModel();

	BoCamera* camera() const { return mCamera; }
	BoLight* light() const { return mLight; }
	BosonModel* model() const { return mModel; }

signals:
	void signalFovYChanged(float);

	void signalRotateXChanged(float);
	void signalRotateYChanged(float);
	void signalRotateZChanged(float);
	void signalCameraXChanged(float);
	void signalCameraYChanged(float);
	void signalCameraZChanged(float);
	void signalCameraChanged();

	void signalFrameChanged(int);
	void signalLODChanged(int);

	void signalMaxFramesChanged(int);
	void signalMaxLODChanged(int);

	void signalMeshSelected(int);

public slots:
	void slotResetView();
	void slotFovYChanged(float f)
	{
		if (f >= MIN_FOVY && f <= MAX_FOVY) {
			mFovY = f;
			resizeGL(width(), height());
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
	void slotConstructionChanged(bool on);
	void slotRenderAxisChanged(bool on)
	{
		mRenderAxis = on;
	}
	void slotRenderGridChanged(bool on)
	{
		mRenderGrid = on;
	}

	void slotHideSelectedMesh();
	void slotHideUnSelectedMeshes();
	void slotUnHideAllMeshes();

protected:
	virtual bool eventFilter(QObject* o, QEvent* e);
	virtual void mouseMoveEvent(QMouseEvent*);
	virtual void mousePressEvent(QMouseEvent*);
	virtual void mouseReleaseEvent(QMouseEvent*);
	virtual void wheelEvent(QWheelEvent*);

	void renderAxii();
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

	void setModel(BosonModel*);
	BoFrame* frame(unsigned int f) const;

	void updateCamera(const BoVector3& cameraPos, const BoQuaternion& q);
	void updateCamera(const BoVector3& cameraPos, const BoMatrix& rotationMatrix);
	void updateCamera(const BoVector3& cameraPos, const BoVector3& lookAt, const BoVector3& up);

	/**
	 * Use -1 to select nothing.
	 **/
	void selectMesh(int mesh);

	bool isSelected(unsigned int mesh) const;

	/**
	 * Hide @p mesh in all frames
	 **/
	void hideMesh(unsigned int mesh, bool hide = true);

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

	bool mPlacementPreview;
	bool mDisallowPlacement;
	bool mWireFrame;
	bool mConstruction;
	bool mRenderAxis;
	bool mRenderGrid;

	BoMouseMoveDiff* mMouseMoveDiff;

	BoCamera* mCamera;
	BoLight* mLight;
};

class PreviewConfig : public QWidget
{
	Q_OBJECT
public:
	PreviewConfig(QWidget* parent);
	~PreviewConfig();

	void setCamera(BoCamera* camera);

signals:
	void signalFovYChanged(float);
	void signalFrameChanged(int);
	void signalLODChanged(int);
	void signalResetDefaults();
	void signalPlacementPreviewChanged(bool); // display preview placement - if false display normal model
	void signalDisallowPlacementChanged(bool); // only valid of placementpreview is also on. if true display the model that is shown when the unit can't be placed - otherwise the model that is shown if it can be placed.
	void signalWireFrameChanged(bool);
	void signalConstructionChanged(bool);
	void signalRenderAxisChanged(bool);
	void signalRenderGridChanged(bool);
	void signalHideMesh();
	void signalHideOthers();
	void signalUnHideAll();

public slots:
	void slotCameraChanged();
	void slotMeshSelected(int);

protected slots:
	void slotFovYChanged(float f) { mFovY->setValue(f); }
	void slotFrameChanged(int f) { mFrame->setValue(f); }
	void slotLODChanged(int l) { mFrame->setValue(l); }

	void slotMaxFramesChanged(int max) { mFrame->setRange(0, max); }
	void slotMaxLODChanged(int max) { mLOD->setRange(0, max); }
	void slotEnableLightChanged(bool);
	void slotEnableMaterialsChanged(bool);

private:
	KMyFloatNumInput* mFovY;
	KIntNumInput* mFrame;
	KIntNumInput* mLOD;
	QPushButton* mHideMesh;
	QPushButton* mHideOthers;
	QPushButton* mUnHideAll;
	QCheckBox* mPlacementPreview;
	QCheckBox* mDisallowPlacement;
	QCheckBox* mWireFrame;
	QCheckBox* mConstruction;
	BoCameraWidget* mCameraWidget;
	QCheckBox* mRenderAxis;
	QCheckBox* mRenderGrid;
	QCheckBox* mEnableLight;
	QCheckBox* mEnableMaterials;
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

	bool loadCamera(KCmdLineArgs*);

	void changeUnit(const QString& speciesIdentifier, const QString& unit);
	void changeUnit(const QString& speciesIdentifier, unsigned long int unitType);
	void changeObject(const QString& speciesIdentifier, const QString& file);

	void emitSignalFovY(float f) { emit mPreview->signalFovYChanged(f); }
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
	void slotDebugMemory();
	void slotDebugSpecies();
	void slotVertexPointSize();
	void slotGridUnitSize();
	void slotShowVertexPoints(bool);
	void slotBackgroundColor();
	void slotShowLightWidget();
	void slotShowMaterialsWidget();
	void slotShowGLStates();
	void slotReloadModelTextures();
	void slotReloadMeshRenderer();
	void slotChangeFont();

private:
	PreviewConfig* mConfig;
	ModelPreview* mPreview;
	QPtrList<SpeciesTheme> mSpecies;
	QPtrDict<SpeciesTheme> mAction2Species;
	BoDebugDCOPIface* mIface;

	BoLightCameraWidget* mLightWidget;
	BoMaterialWidget* mMaterialWidget;
};

#endif

