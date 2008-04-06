/*
    This file is part of the Boson game
    Copyright (C) 2002-2006 Andreas Beckermann (b_mann@gmx.de)

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef BORENDERMAIN_H
#define BORENDERMAIN_H

#include "bodebugdcopiface.h"
#include "bosonufoglwidget.h"
#include <boufo/boufocustomwidget.h>
#include "bo3dtools.h"

#include <q3ptrlist.h>
#include <q3ptrdict.h>
#include <q3intdict.h>
#include <q3valuelist.h>
#include <qstringlist.h>
//Added by qt3to4:
#include <QWheelEvent>
#include <QMouseEvent>

#include <kmainwindow.h>


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
class BoEditTurretPropertiesDialog;
template<class T> class BoVector3;
typedef BoVector3<float> BoVector3Float;
class BoUfoRadioButton;

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

	const BosonModel* model() const;
	BoCamera* camera() const;
	BoLight* light() const;

	void setFont(const BoFontInfo& font);
	void setModel(BosonModel*);

	void setTurretMeshes(const QStringList& meshes);
	const QStringList& turretMeshes() const;
	void setTurretMeshesEnabled(bool e);
	void setTurretInitialZRotation(float r);
	void setTurretTimerRotation(bool timer);

	void updateCamera(const BoVector3Float& cameraPos, const BoQuaternion& q);
	void updateCamera(const BoVector3Float& cameraPos, const BoMatrix& rotationMatrix);
	void updateCamera(const BoVector3Float& cameraPos, const BoVector3Float& lookAt, const BoVector3Float& up);

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
	void signalTurretRotation(float rotation);


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
	void slotFovYChanged(float f);
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
	void slotPlacementPreviewChanged(bool on);
	void slotDisallowPlacementChanged(bool on);
	void slotWireFrameChanged(bool on);
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
	void slotSetTurretRotationAngle(float rot);
	void slotSetModelRotationZ(float rot);
	void slotSetModelRotationX(float rot);
	void slotSetModelRotationY(float rot);

protected slots:
	void slotMousePressEvent(QMouseEvent*);
	void slotMouseMoveEvent(QMouseEvent*);
	void slotMouseReleaseEvent(QMouseEvent*);
	void slotWheelEvent(QWheelEvent*);

	void slotResetModel();

protected:
	void renderAxii();
	void renderGrid();

	void updateMeshUnderMouse();

	/**
	 * Use -1 to select nothing.
	 **/
	void selectMesh(int mesh);

	/**
	 * Update the text that displays information on what is under the cursor
	 **/
	void updateCursorDisplay(const QPoint& pos);

private:
	void initializeGL();

private:
	friend class RenderMain; // we need to emit signals from outside, in order to save lots of forwarding code
	ModelDisplayPrivate* d;
	int mMeshUnderMouse;
	int mSelectedMesh;
	BosonViewData* mViewData;

	BosonGLFont* mDefaultFont;

	float mFovY; // we allow real zooming here!

	bool mWireFrame;
	bool mRenderAxis;
	bool mRenderGrid;

	BoMouseMoveDiff* mMouseMoveDiff;
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

public slots:
	void slotToggleShowGUI();
	void slotSetGUIVisible(bool);
	void slotSetShowMenubar(bool v);
	void slotApplyShowMenubar();

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
	void slotShowTurretToggled(bool);
	void slotEditTurretProperties();

	void slotApplyTurretProperties(BoEditTurretPropertiesDialog*);
	void slotChangeTurretMode(BoUfoRadioButton* button);



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

