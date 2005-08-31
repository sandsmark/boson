/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 Andreas Beckermann (b_mann@gmx.de)

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
#include "bosonufoglwidget.h"

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
class BoFrame;
class BoLight;
class BoLightCameraWidget;
class BoLightCameraWidget1;
class BoMaterialWidget;
class KCmdLineArgs;
class QCheckBox;
class BoFontInfo;
class BoUfoManager;
class BoUfoLabel;
class BoUfoAction;
class BosonViewData;
template<class T> class BoVector3;
typedef BoVector3<float> BoVector3Float;

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
class ModelPreview : public BosonUfoGLWidget
{
	Q_OBJECT
public:
	ModelPreview(const QPtrList<SpeciesTheme>&, QWidget*);
	~ModelPreview();

	virtual void initializeGL();
	virtual void paintGL();
	virtual void resizeGL(int, int);

	void setFont(const BoFontInfo& font);

	void load(SpeciesTheme* s, const UnitProperties* prop);
	void loadObjectModel(SpeciesTheme* s, const QString& file);
	void resetModel();

	BoCamera* camera() const { return mCamera; }
	BoLight* light() const { return mLight; }
	BosonModel* model() const { return mModel; }

	void changeUnit(const QString& speciesIdentifier, const QString& unit);
	void changeUnit(const QString& speciesIdentifier, unsigned long int unitType);
	void changeObject(const QString& speciesIdentifier, const QString& file);

	void changeUnit(SpeciesTheme* s, const UnitProperties* prop);
	void changeObject(SpeciesTheme* s, const QString& file);

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
	void signalFrameChanged(float);
	void signalLODChanged(int);
	void signalLODChanged(float);

	void signalMaxFramesChanged(int);
	void signalMaxFramesChanged(float);
	void signalMaxLODChanged(int);
	void signalMaxLODChanged(float);

	void signalMeshSelected(int);


	void signalChangeBackgroundColor();
	void signalShowLightWidget();
	void signalShowMaterialsWidget();
	void signalShowChangeFont();
	void signalDebugModels();
	void signalDebugMemory();
	void signalDebugSpecies();
	void signalShowGLStates();
	void signalReloadModelTextures();
	void signalReloadMeshRenderer();

	void signalUnitChanged(SpeciesTheme*, int);
	void signalObjectChanged(SpeciesTheme*, int);

public slots:
	void slotResetView();
	void slotFovYChanged(float f)
	{
		if (f >= MIN_FOVY && f <= MAX_FOVY) {
			mFovY = f;
			resizeGL(width(), height());
		}
	}
	void slotFrameChanged(float f)
	{
		slotFrameChanged((int)f);
	}
	void slotFrameChanged(int f);
	void slotLODChanged(float l)
	{
		slotLODChanged((int)l);
	}
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
	void slotEnableLight(bool);
	void slotEnableMaterials(bool);
	void slotShowVertexPoints(bool);
	void slotChangeVertexPointSize();
	void slotChangeGridUnitSize();

	void slotUnitChanged(int);
	void slotObjectChanged(int);

	void slotSetVertexPointSize(int);
	void slotSetGridUnitSize(float);

	void slotChangeBackgroundColor();
	void slotShowLightWidget();
	void slotDebugModels();
	void slotDebugMemory();
	void slotDebugSpecies();
	void slotShowMaterialsWidget();
	void slotShowGLStates();
	void slotReloadModelTextures();
	void slotReloadMeshRenderer();
	void slotShowChangeFont();

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

	void updateCamera(const BoVector3Float& cameraPos, const BoQuaternion& q);
	void updateCamera(const BoVector3Float& cameraPos, const BoMatrix& rotationMatrix);
	void updateCamera(const BoVector3Float& cameraPos, const BoVector3Float& lookAt, const BoVector3Float& up);

	void updateMeshUnderMouseLabel();

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

	void uncheckAllBut(BoUfoAction*); // BAH!

	SpeciesTheme* findTheme(const QString& theme) const;

private:
	void initUfoGUI();
	void initUfoAction();

private:
	friend class RenderMain; // we need to emit signals from outside, in order to save lots of forwarding code
	QTimer* mUpdateTimer;
	BosonModel* mModel;
	int mCurrentFrame;
	int mCurrentLOD;
	int mMeshUnderMouse;
	int mSelectedMesh;
	BoUfoLabel* mMeshUnderMouseLabel;
	BoUfoLabel* mSelectedMeshLabel;
	BosonViewData* mViewData;

	BosonGLFont* mDefaultFont;

	float mFovY; // we allow real zooming here!

	bool mPlacementPreview;
	bool mDisallowPlacement;
	bool mWireFrame;
	bool mRenderAxis;
	bool mRenderGrid;

	BoMouseMoveDiff* mMouseMoveDiff;

	BoCamera* mCamera;
	BoLight* mLight;

	QPtrList<SpeciesTheme> mSpecies;
	QPtrDict<SpeciesTheme> mAction2Species;

	// TODO: use a BoUfo widget. don't use a separate window.
	BoLightCameraWidget1* mLightWidget;
	BoMaterialWidget* mMaterialWidget;
};

/**
 * This widget is here mainly for historic reasons. It just contains a @ref
 * Modelpreview widget. Most interesting things are done there.
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

private:
	ModelPreview* mPreview;
	QPtrList<SpeciesTheme> mSpecies;
	BoDebugDCOPIface* mIface;
};



#endif

