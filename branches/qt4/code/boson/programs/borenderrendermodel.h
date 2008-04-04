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
#ifndef BORENDERRENDERMODEL_H
#define BORENDERRENDERMODEL_H

#include <qobject.h>
#include <qstringlist.h>

#include "bo3dtools.h"

class BoCamera;
class BoLight;
class BosonModel;
class BoMesh;


class BoRenderRenderModelPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoRenderRenderModel : public QObject
{
	Q_OBJECT
public:
	BoRenderRenderModel(QObject* parent = 0);
	~BoRenderRenderModel();

	void render();

	void resetModel();

	BoCamera* camera() const { return mCamera; }
	BoLight* light() const { return mLight; }
	BosonModel* model() const { return mModel; }

	void setModel(BosonModel*);

	void setTurretMeshes(const QStringList& meshes);
	const QStringList& turretMeshes() const
	{
		return mTurretMeshes;
	}
	void setTurretMeshesEnabled(bool e)
	{
		mTurretMeshesEnabled = e;
	}
	void setTurretInitialZRotation(float r)
	{
		mTurretInitialZRotation = r;
	}
	void setTurretTimerRotation(bool timer);

	void setSelectedMesh(int m);
	int selectedMesh() const
	{
		return mSelectedMesh;
	}

	bool haveModel() const
	{
		if (mModel && mCurrentFrame >= 0) {
			return true;
		}
		return false;
	}

	/**
	 * @return The mesh index that is at position @p pos (in Qt window
	 * coordinates)
	 **/
	int pickObject(const QPoint& pos, float fovY, float near, float far);
	BoMesh* meshWithIndex(int index) const;

	void updateCamera(const BoVector3Float& cameraPos, const BoQuaternion& q);
	void updateCamera(const BoVector3Float& cameraPos, const BoMatrix& rotationMatrix);
	void updateCamera(const BoVector3Float& cameraPos, const BoVector3Float& lookAt, const BoVector3Float& up);

signals:
	void signalMaxFramesChanged(float);
	void signalMaxLODChanged(float);
	void signalResetModel();

	void signalFrameChanged(float);
	void signalLODChanged(float);
	void signalCameraChanged();

	void signalTurretRotation(float rotation);


public slots:
	void slotSetTurretRotationAngle(float rot);
	void slotSetModelRotationZ(float rot);
	void slotSetModelRotationX(float rot);
	void slotSetModelRotationY(float rot);

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

	void slotHideSelectedMesh();
	void slotHideUnSelectedMeshes();
	void slotUnHideAllMeshes();

protected:
	void renderModel(int mode = -1);
	void renderMeshSelection();

	/**
	 * Use -1 to select nothing.
	 **/
	void selectMesh(int mesh);

	bool isSelected(unsigned int mesh) const;

	/**
	 * Hide @p mesh in all frames
	 **/
	void hideMesh(unsigned int mesh, bool hide = true);

protected slots:
	void slotTurretTimeout();

private:
	BoRenderRenderModelPrivate* d;

	BosonModel* mModel;
	int mCurrentFrame;
	int mCurrentLOD;
	int mSelectedMesh;
	QStringList mTurretMeshes;
	bool mTurretMeshesEnabled;
	float mTurretInitialZRotation;
	float mTurretRotation;
	bool mTurretTimerRotation;
	BoMatrix mTurretMatrix;

	float mModelRotationZ;
	float mModelRotationX;
	float mModelRotationY;

	bool mPlacementPreview;
	bool mDisallowPlacement;
	bool mWireFrame;

	BoCamera* mCamera;
	BoLight* mLight;
};

#endif

