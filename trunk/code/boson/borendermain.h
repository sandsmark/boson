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
#include <boufo/boufocustomwidget.h>

#include <qptrlist.h>
#include <qptrdict.h>
#include <qintdict.h>
#include <qvaluelist.h>

#include <kmainwindow.h>

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

class ModelDisplayPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class ModelDisplay : public BoUfoCustomWidget
{
	Q_OBJECT
public:
	ModelDisplay();
	~ModelDisplay();

	virtual void paintWidget();

	void setFont(const BoFontInfo& font);

	void resetModel();

	BoCamera* camera() const { return mCamera; }
	BoLight* light() const { return mLight; }
	BosonModel* model() const { return mModel; }

	void setModel(BosonModel*);

signals:
	void signalShowSelectedMeshLabel(bool);
	void signalSelectedMeshLabel(const QString&);
	void signalMeshUnderMouseLabel(const QString&);

	void signalRotateXChanged(float);
	void signalRotateYChanged(float);
	void signalRotateZChanged(float);
	void signalCameraXChanged(float);
	void signalCameraYChanged(float);
	void signalCameraZChanged(float);
	void signalCameraChanged();

	void signalFovYChanged(float);
	void signalFrameChanged(float);
	void signalLODChanged(float);

	void signalMaxFramesChanged(float);
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

	void slotSetVertexPointSize(int);

	void slotDebugMemory();

protected slots:
	void slotMousePressEvent(QMouseEvent*);
	void slotMouseMoveEvent(QMouseEvent*);
	void slotMouseReleaseEvent(QMouseEvent*);
	void slotWheelEvent(QWheelEvent*);

protected:
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

	void updateCamera(const BoVector3Float& cameraPos, const BoQuaternion& q);
	void updateCamera(const BoVector3Float& cameraPos, const BoMatrix& rotationMatrix);
	void updateCamera(const BoVector3Float& cameraPos, const BoVector3Float& lookAt, const BoVector3Float& up);

	void updateMeshUnderMouse();

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
	void initializeGL();

private:
	friend class RenderMain; // we need to emit signals from outside, in order to save lots of forwarding code
	ModelDisplayPrivate* d;
	BosonModel* mModel;
	int mCurrentFrame;
	int mCurrentLOD;
	int mMeshUnderMouse;
	int mSelectedMesh;
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
};

class BoRenderGLWidgetPrivate;
class BoRenderGLWidget : public BosonUfoGLWidget
{
	Q_OBJECT
public:
	BoRenderGLWidget(QWidget* parent, bool direct);
	~BoRenderGLWidget();

	void initWidget();

	bool parseCmdLineArgs(KCmdLineArgs*);

	void load(SpeciesTheme* s, const UnitProperties* prop);
	void loadObjectModel(SpeciesTheme* s, const QString& file);

	void changeUnit(const QString& speciesIdentifier, const QString& unit);
	void changeUnit(const QString& speciesIdentifier, unsigned long int unitType);
	void changeObject(const QString& speciesIdentifier, const QString& file);

	void changeUnit(SpeciesTheme* s, const UnitProperties* prop);
	void changeObject(SpeciesTheme* s, const QString& file);


protected:
	virtual void initializeGL();
	virtual void paintGL();

	bool loadCamera(KCmdLineArgs*);
	SpeciesTheme* findTheme(const QString& theme) const;
	void uncheckAllBut(BoUfoAction*); // BAH!

protected slots:
	void slotShowVertexPoints(bool);
	void slotChangeVertexPointSize();
	void slotChangeGridUnitSize();
	void slotChangeBackgroundColor();
	void slotDebugModels();
	void slotDebugSpecies();
	void slotShowMaterialsWidget();
	void slotShowLightWidget();
	void slotSetGridUnitSize(float);
	void slotUnitChanged(int);
	void slotObjectChanged(int);
	void slotReloadModelTextures();
	void slotReloadMeshRenderer();
	void slotShowGLStates();
	void slotShowChangeFont();




private:
	void initUfoGUI();
	void initUfoAction();

private:
	BoRenderGLWidgetPrivate* d;

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

	bool parseCmdLineArgs(KCmdLineArgs*);

private:
	BoRenderGLWidget* mGLWidget;
	QTimer* mUpdateTimer;
	BoDebugDCOPIface* mIface;
};



#endif

