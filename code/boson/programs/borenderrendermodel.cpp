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

#include "borenderrendermodel.h"
#include "borenderrendermodel.moc"

#include "defines.h"
#include "bosonconfig.h"
#include "bodebug.h"
#include "bocamera.h"
#include "modelrendering/bosonmodel.h"
#include "modelrendering/bomesh.h"
#include "bolight.h"
#include "modelrendering/bomeshrenderermanager.h"
#include "bomaterial.h"
#include <bogl.h>

#include <qvaluevector.h>
#include <qtimer.h>

#include <math.h>
#include <stdlib.h>

static void startTransparentFrameRendering()
{
 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "OpenGL error before method" << endl;
 }
 glPushAttrib(GL_ALL_ATTRIB_BITS);
 glEnable(GL_DEPTH_TEST);
 glEnable(GL_BLEND);
 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 glEnable(GL_ALPHA_TEST);
 glAlphaFunc(GL_GEQUAL, 0.2);
 glDisable(GL_CULL_FACE);
 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "OpenGL error" << endl;
 }
}

static void stopTransparentFrameRendering()
{
 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "OpenGL error before method" << endl;
 }
 glPopAttrib();
 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "OpenGL error" << endl;
 }
}


class BoRenderRenderModelPrivate
{
public:
	BoRenderRenderModelPrivate()
	{
	}
};

BoRenderRenderModel::BoRenderRenderModel(QObject* parent)
	: QObject(parent)
{
 d = new BoRenderRenderModelPrivate();
 mModel = 0;
 mCurrentFrame = 0;
 mCurrentLOD = 0;
 mSelectedMesh = -1;
 mTurretMeshesEnabled = false;
 mTurretInitialZRotation = 0.0f;
 mTurretRotation = 0.0f;
 mTurretTimerRotation = true;
 mCamera = new BoCamera;
 mLight = 0;

 QTimer* turretTimer = new QTimer();
 connect(turretTimer, SIGNAL(timeout()),
		this, SLOT(slotTurretTimeout()));
 turretTimer->start(50);

 mPlacementPreview = false;
 mDisallowPlacement = false;
 mWireFrame = false;



 if (mLight) {
	BoLightManager::manager()->deleteLight(mLight->id());
 }
 mLight = BoLightManager::manager()->createLight();
 if (!mLight->isActive()) {
	boWarning() << k_funcinfo << "light is inactive" << endl;
 }
 BoVector4Float lightDif(1.0f, 1.0f, 1.0f, 1.0f);
 BoVector4Float lightAmb(0.5f, 0.5f, 0.5f, 1.0f);
 BoVector3Float lightPos(-6000.0, 3000.0, 10000.0);
 mLight->setAmbient(lightAmb);
 mLight->setDiffuse(lightDif);
 mLight->setSpecular(lightDif);
 mLight->setDirectional(true);
 mLight->setEnabled(true);
}

BoRenderRenderModel::~BoRenderRenderModel()
{
 resetModel();
 BoMeshRendererManager::manager()->unsetCurrentRenderer();
 if (mLight) {
	BoLightManager::manager()->deleteLight(mLight->id());
 }
 delete mCamera;
 delete d;
}

void BoRenderRenderModel::render()
{
 BO_CHECK_NULL_RET(camera());
 BO_CHECK_NULL_RET(light());
 if (!haveModel()) {
	return;
 }

 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "OpenGL error before method" << endl;
 }

 glPushAttrib(GL_ALL_ATTRIB_BITS);
 glPushMatrix();

 glColor3ub(255, 255, 255);
 if (boConfig->boolValue("UseLight")) {
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_NORMALIZE);
 }

 glLoadIdentity();
 camera()->applyCameraToScene();

 renderModel();
 renderMeshSelection();

 glPopMatrix();
 glPopAttrib();

 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "OpenGL error at end of method" << endl;
 }
}


void BoRenderRenderModel::setSelectedMesh(int m)
{
 if (m < 0) {
	mSelectedMesh = -1;
	return;
 }
 if (!haveModel()) {
	mSelectedMesh = -1;
	return;
 }
 BoLOD* lod = mModel->lod(mCurrentLOD);
 BoFrame* f = 0;
 if (lod) {
	f = lod->frame(mCurrentFrame);
 }
 if (!f) {
	mSelectedMesh = -1;
	return;
 }
 if ((unsigned int)m >= f->nodeCount()) {
	mSelectedMesh = -1;
	return;
 }
 mSelectedMesh = m;
}

void BoRenderRenderModel::setTurretMeshes(const QStringList& meshes)
{
 mTurretMeshes = meshes;
}

void BoRenderRenderModel::renderModel(int mode)
{
 if (!haveModel()) {
	return;
 }
 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "OpenGL error before method" << endl;
 }


 if (mWireFrame) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
 } else {
	glEnable(GL_TEXTURE_2D);
 }
 glEnable(GL_DEPTH_TEST);

 // AB: if these are enabled we can't use triangle strips by any reason.
 // AB: we don't use triangle strips atm (and there are ways so that we still
 // can use backface culling). but i leave this out, as borender rendering speed
 // isn't critical and it may be easier to debug certain things.
// glEnable(GL_CULL_FACE);
// glCullFace(GL_BACK);

 if (mModel && mCurrentLOD >= 0) {
	BoLOD* lod = mModel->lod(mCurrentLOD);
	BoFrame* f = lod->frame(mCurrentFrame);
	if (f) {
		if (mPlacementPreview) {
			glEnable(GL_BLEND);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // default anyway - redundant call
			GLubyte r, g, b, a;
			a = PLACEMENTPREVIEW_ALPHA;
			r = 255;
			if (mDisallowPlacement) {
				g = PLACEMENTPREVIEW_DISALLOW_COLOR;
				b = PLACEMENTPREVIEW_DISALLOW_COLOR;
			} else {
				g = 255;
				b = 255;
			}
			glColor4ub(r, g, b, a);
		}

		if (mode == GL_SELECT) {
			glDisable(GL_BLEND);
			glDisable(GL_TEXTURE_2D);
		}
		BosonModel::startModelRendering();
			glEnable(GL_DEPTH_TEST);
			glDisable(GL_BLEND);
			glDisable(GL_ALPHA_TEST);
		mModel->prepareRendering();

		mTurretMatrix = BoMatrix();
		mTurretMatrix.rotate(mTurretInitialZRotation, 0.0f, 0.0f, -1.0f);
		mTurretMatrix.rotate(90.0f, 1.0f, 0.0f, 0.0f);

		BoVector3Float v(0.0f, 1.0f, 0.0f);
		{
			{
				BoMatrix rot;
				rot.rotate(mModelRotationZ, 0.0f, 0.0f, 1.0f);
				rot.multiply(&mTurretMatrix);
				mTurretMatrix = rot;
			}

			BoMatrix rot;
			rot.rotate(mTurretRotation, 0.0, 0.0, 1.0);
			BoVector3Float v2 = v;
			rot.transform(&v, &v2);
		}
		BoMatrix lookAt;
		lookAt.setLookAtRotation(BoVector3Float(0, 0, 0), v, BoVector3Float(0, 0, 1));

		mTurretMatrix.multiply(&lookAt);

		QValueVector<const BoMatrix*> itemMatrices(f->nodeCount());
		for (unsigned int i = 0; i < f->nodeCount(); i++) {
			BoMesh* mesh = f->mesh(i);
			if (!mTurretMeshesEnabled || !mTurretMeshes.contains(mesh->name())) {
				continue;
			}
			itemMatrices[i] = &mTurretMatrix;
		}


		glPushMatrix();

		// AB: these rotations emulate the unit rotation in the game
		glRotatef(-mModelRotationZ, 0.0f, 0.0f, 1.0f);
		glRotatef(mModelRotationX, 1.0f, 0.0f, 0.0f);
		glRotatef(mModelRotationY, 0.0f, 1.0f, 0.0f);

		f->renderFrame(itemMatrices, 0, false, Default, mode);

		startTransparentFrameRendering();
		if (mode == GL_SELECT) {
			glDisable(GL_BLEND);
			glDisable(GL_TEXTURE_2D);
		}
		f->renderFrame(itemMatrices, 0, true, Default, mode);
		stopTransparentFrameRendering();

		glPopMatrix();

		BosonModel::stopModelRendering();
		if (mPlacementPreview) {
			// AB: do not reset the actual color - if it will get
			// used it will be set again anyway.
			glColor4ub(255, 255, 255, 255);
		}
	} else {
		boError() << k_funcinfo << "NULL frame" << endl;
	}
 }
 BoMaterial::deactivate();

 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "OpenGL error at end of method" << endl;
 }
}

void BoRenderRenderModel::renderMeshSelection()
{
 if (!haveModel()) {
	return;
 }
 if (mSelectedMesh < 0) {
	return;
 }
 if (mCurrentLOD < 0 || mCurrentFrame < 0) {
	return;
 }
 if ((unsigned int)mCurrentLOD >= mModel->lodCount()) {
	return;
 }

 BoLOD* lod = mModel->lod(mCurrentLOD);
 if ((unsigned int)mCurrentFrame >= lod->frameCount()) {
	return;
 }

 BoFrame* f = lod->frame(mCurrentFrame);
 if ((unsigned int)mSelectedMesh >= f->nodeCount()) {
	return;
 }
 BoMesh* mesh = f->mesh(mSelectedMesh);
 if (!mesh) {
	return;
 }
 BoMatrix* matrix = f->matrix(mSelectedMesh);
 if (!matrix) {
	return;
 }

 glPushAttrib(GL_POLYGON_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT);
 glPushMatrix();
 glMultMatrixf(matrix->data());
 glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
 glColor3ub(0, 255, 0);
 glDisable(GL_TEXTURE_2D);
 glDisable(GL_LIGHTING);
#warning FIXME!!!
 //mesh->renderBoundingObject();
 glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

 if (boConfig->boolValue("ShowVertexPoints")) {
	glColor3ub(0, 255, 0);
	float size = (float)boConfig->uintValue("VertexPointSize", 3);
	glPointSize(size);
	glDisable(GL_DEPTH_TEST);
	mesh->renderVertexPoints(mModel);
 }

 glPopMatrix();
 glPopAttrib();
 glColor3ub(255, 255, 255);
}

void BoRenderRenderModel::setModel(BosonModel* model)
{
 boDebug() << k_funcinfo << endl;
 resetModel();
 mModel = model;
 if (mModel) {
	unsigned int lodcount = mModel->lodCount();
	emit signalMaxLODChanged((int)(lodcount - 1));
	emit signalMaxLODChanged((float)(lodcount - 1));
	if ((unsigned int)mCurrentLOD >= lodcount) {
		mCurrentLOD = lodcount - 1;
	}
	unsigned int framecount = mModel->lod(mCurrentLOD)->frameCount();
	emit signalMaxFramesChanged((float)(framecount - 1));
 }
}

void BoRenderRenderModel::resetModel()
{
 mModel = 0;
 mCurrentFrame = 0;
 mCurrentLOD = 0;
 mSelectedMesh = -1;

 emit signalResetModel();
}

int BoRenderRenderModel::pickObject(const QPoint& cursor, float fovY, float near, float far)
{
 if (!haveModel()) {
	return -1;
 }
 BoLOD* lod = mModel->lod(mCurrentLOD);
 BoFrame* f = lod->frame(mCurrentFrame);
 if (!f) {
	return -1;
 }
 const int bufferSize = 256;
 unsigned int buffer[bufferSize];
 int viewport[4];

 glGetIntegerv(GL_VIEWPORT, viewport);

 glSelectBuffer(bufferSize, buffer);


 glRenderMode(GL_SELECT);
 glInitNames();
 glPushName(0);

 glMatrixMode(GL_PROJECTION);
 glPushMatrix();
 glLoadIdentity();
 gluPickMatrix((GLdouble)cursor.x(), (GLdouble)(viewport[3] - cursor.y()), 1.0, 10, viewport);
 gluPerspective(fovY, (float)viewport[2] / (float)viewport[3], near, far);
 glMatrixMode(GL_MODELVIEW);
 glPushMatrix();
 glLoadIdentity();

 camera()->applyCameraToScene();
 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 glColor3f(1.0f, 1.0f, 1.0f);


 renderModel(GL_SELECT);


 glMatrixMode(GL_PROJECTION);
 glPopMatrix();
 glMatrixMode(GL_MODELVIEW);
 glPopMatrix();
 int hits = glRenderMode(GL_RENDER);
 int pos = 0;
 unsigned int closestZ = 0xFFFFFFFF;
 int mesh = -1;
 for (int i = 0; i < hits; i++) {
	unsigned int names = buffer[pos];
	pos++;
	unsigned int minZ = buffer[pos]; // note: depth is [0;1] and multiplied by 2^32-1
	pos++;
//	unsigned int maxZ = buffer[pos];
	pos++;
	if (names != 1) {
		boWarning() << k_funcinfo << "more than 1 name - not supported!" << endl;
		for (unsigned int j = 0; j < names; j++) {
			pos++;
		}
		continue;
	}
	unsigned int name = buffer[pos];
	BoMesh* m = 0;
	if (name < f->nodeCount()) {
		m = f->mesh(name);
	}
	if (m) {
//		boDebug() << k_funcinfo << m->name() << endl;
		if (minZ < closestZ) {
			mesh = name;
			closestZ = minZ;
		}
	} else {
		boWarning() << k_funcinfo << "no such mesh: " << name << endl;
	}
	pos++;
 }


 if (mesh < 0) {
	return mesh;
 }
 if ((unsigned int)mesh >= f->nodeCount()) {
	boError() << k_funcinfo << "invalid result " << mesh << endl;
	return -1;
 }
 return mesh;
}

BoMesh* BoRenderRenderModel::meshWithIndex(int index) const
{
 if (!haveModel()) {
	return 0;
 }
 BoLOD* l = mModel->lod(mCurrentLOD);
 if (!l) {
	return 0;
 }
 BoFrame* f = l->frame(mCurrentFrame);
 if (!f) {
	return 0;
 }
 if (index < 0) {
	return 0;
 }
 if ((unsigned int)index >= f->nodeCount()) {
	return 0;
 }
 return f->mesh(index);
}

void BoRenderRenderModel::slotFrameChanged(int f)
{
 if (f != 0) {
	if (!mModel || f < 0) {
		emit signalFrameChanged(0.0f);
		return;
	}
	int frames = mModel->lod(mCurrentLOD)->frameCount();
	if (f >= frames) {
		emit signalFrameChanged((float)(frames - 1));
		return;
	}
  }
 mCurrentFrame = f;
}

void BoRenderRenderModel::slotLODChanged(int l)
{
 if (l != 0) {
	if (!mModel || l < 0) {
		emit signalLODChanged((float)0.0f);
		return;
	}
	if ((unsigned int)l >= mModel->lodCount()) {
		emit signalLODChanged((float)(mModel->lodCount() - 1));
		return;
	}
  }
 mCurrentLOD = l;
}

void BoRenderRenderModel::updateCamera(const BoVector3Float& cameraPos, const BoQuaternion& q)
{
 updateCamera(cameraPos, q.matrix());
}

void BoRenderRenderModel::updateCamera(const BoVector3Float& cameraPos, const BoMatrix& rotationMatrix)
{
 BoVector3Float lookAt;
 BoVector3Float up;
 rotationMatrix.toGluLookAt(&lookAt, &up, cameraPos);
 updateCamera(cameraPos, lookAt, up);
}

void BoRenderRenderModel::updateCamera(const BoVector3Float& cameraPos, const BoVector3Float& lookAt, const BoVector3Float& up)
{
 BO_CHECK_NULL_RET(camera());
 camera()->setGluLookAt(cameraPos, lookAt, up);
 emit signalCameraChanged();
}

void BoRenderRenderModel::hideMesh(unsigned int mesh, bool hide)
{
#warning FIXME!!! Do we even need this?
#if 0
 if (!haveModel()) {
	return;
 }
 BoLOD* l = mModel->lod(mCurrentLOD);
 for (unsigned int i = 0; i < lod->frameCount(); i++) {
	BoFrame* f = lod->frame(i);
	if (mesh >= f->nodeCount()) {
		boWarning() << k_funcinfo << "mesh does not exist: " << mesh << " meshes in frame: " << f->nodeCount() << endl;
		return;
	}
	if (!f) {
		boWarning() << k_funcinfo << "NULL frame " << i << endl;
		continue;
	}
	f->setHidden(mesh, hide);
 }
 if (isSelected(mesh)) {
	selectMesh(-1);
 }
#endif
}

bool BoRenderRenderModel::isSelected(unsigned int mesh) const
{
 // AB: one day we will use a list of selected meshes
 if (mSelectedMesh < 0) {
	return false;
 }
 if ((unsigned int)mSelectedMesh == mesh) {
	return true;
 }
 return false;
}

void BoRenderRenderModel::slotHideSelectedMesh()
{
 if (!haveModel()) {
	return;
 }
 if (mSelectedMesh < 0) {
	return;
 }
 hideMesh((unsigned int)mSelectedMesh);
}

void BoRenderRenderModel::slotHideUnSelectedMeshes()
{
 if (!haveModel()) {
	return;
 }
 if (mSelectedMesh < 0) {
	return;
 }
 if (mCurrentFrame < 0) {
	return;
 }
#if 0
 BoFrame* f = frame(mCurrentFrame);
 BO_CHECK_NULL_RET(f);
 for (unsigned int i = 0; i < f->meshCount(); i++) {
	if (i == (unsigned int)mSelectedMesh) {
		continue;
	}
	hideMesh(i);
 }
#endif
}

void BoRenderRenderModel::slotUnHideAllMeshes()
{
 boDebug() << k_funcinfo << endl;
 if (!haveModel()) {
	return;
 }
 if (mCurrentFrame < 0) {
	return;
 }
#if 0
 BoFrame* f = frame(mCurrentFrame);
 BO_CHECK_NULL_RET(f);
 for (unsigned int i = 0; i < f->meshCount(); i++) {
	hideMesh(i, false);
 }
#endif
}

void BoRenderRenderModel::slotTurretTimeout()
{
 if (!mTurretTimerRotation) {
	return;
 }
 slotSetTurretRotationAngle(mTurretRotation + 5.0f);
 if (mTurretRotation > 360.0f) {
	slotSetTurretRotationAngle(0.0f);
 }
}

void BoRenderRenderModel::slotSetTurretRotationAngle(float rot)
{
 mTurretRotation = rot;
 emit signalTurretRotation(mTurretRotation);
}

void BoRenderRenderModel::slotSetModelRotationZ(float rot)
{
 mModelRotationZ = rot;
}

void BoRenderRenderModel::slotSetModelRotationX(float rot)
{
 mModelRotationX = rot;
}

void BoRenderRenderModel::slotSetModelRotationY(float rot)
{
 mModelRotationY = rot;
}

void BoRenderRenderModel::setTurretTimerRotation(bool timer)
{
 mTurretTimerRotation = timer;
}

