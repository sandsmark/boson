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
#include "bosonitemrenderer.h"

#include "../../bomemory/bodummymemory.h"
#include "../bosonmodel.h"
#include "../bomesh.h"
#include "../bo3dtools.h"
#include "../bosonconfig.h"
#include "../boson.h"
#include "../unit.h"
#include "../bosonweapon.h"
#include "../rtti.h"
#include "bosonitem.h"
#include "bodebug.h"


BosonItemRenderer::BosonItemRenderer(BosonItem* item)
{
 mItem = item;

 // 0.866 == sqrt(3*0.5) i.e. lenght of vector whose all components are 0.5
 mBoundingSphereRadius = 0.866f;

 mAnimationMode = -1; // invalid - causes update once animate() is called
}


BosonItemRenderer::~BosonItemRenderer()
{
}

void BosonItemRenderer::startItemRendering()
{
 if (!boConfig->boolValue("ForceDisableModelLoading")) {
	BosonModel::startModelRendering();
 } else {
	glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT);
	glDisable(GL_TEXTURE_2D);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_LIGHTING);
 }
}

void BosonItemRenderer::stopItemRendering()
{
 if (!boConfig->boolValue("ForceDisableModelLoading")) {
	BosonModel::stopModelRendering();
 } else {
	glPopAttrib();
 }
}

const QColor* BosonItemRenderer::teamColor() const
{
 return mItem->teamColor();
}

void BosonItemRenderer::renderItem(unsigned int lod, bool transparentmeshes, RenderFlags)
{
 Q_UNUSED(lod);

 if (transparentmeshes) {
	return;
 }

 // this code renders an item without using a model. this is for debugging only
 // (by not loading the models we can reduce startup time greatly)
 BO_CHECK_NULL_RET(mItem);
 float w = ((float)mItem->width());
 float h = ((float)mItem->width());
 float depth = mItem->depth();

 // make the box a little bit smaller, looks nicer
 w -= 0.1f;
 h -= 0.1f;

 glColor3ub(255, 0, 0);
 glTranslatef(-w/2, -h/2, 0.0f);
 glBegin(GL_QUADS);
	// bottom
	glVertex3f(0.0f, h, 0.0f);
	glVertex3f(w, h, 0.0f);
	glVertex3f(w, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);

	// top
	glVertex3f(0.0f, h, depth);
	glVertex3f(w, h, depth);
	glVertex3f(w, 0.0f, depth);
	glVertex3f(0.0f, 0.0f, depth);

	// left
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, depth);
	glVertex3f(0.0f, h, depth);
	glVertex3f(0.0f, h, 0.0f);

	// right
	glVertex3f(w, 0.0f, 0.0f);
	glVertex3f(w, 0.0f, depth);
	glVertex3f(w, h, depth);
	glVertex3f(w, h, 0.0f);

	// front
	glVertex3f(0.0f, 0.0f, depth);
	glVertex3f(w, 0.0f, depth);
	glVertex3f(w, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);

	// back
	glVertex3f(0.0f, h, depth);
	glVertex3f(w, h, depth);
	glVertex3f(w, h, 0.0f);
	glVertex3f(0.0f, h, 0.0f);
 glEnd();
 glTranslatef(w/2, h/2, 0.0f);
}

float BosonItemRenderer::itemInFrustum(const BoFrustum& frustum) const
{
 if (!mItem) {
	BO_NULL_ERROR(mItem);
	return false;
 }
 // FIXME: can't we use BoVector3 and it's conversion methods here?
 bofixed x = (mItem->x() + mItem->width() / 2);
 bofixed y = -((mItem->y() + mItem->height() / 2));
 bofixed z = mItem->z(); // this is already in the correct format!
 BoVector3Fixed pos(x, y, z);
 return (float)frustum.sphereInFrustum(pos, boundingSphereRadius());
}


BosonItemModelRenderer::BosonItemModelRenderer(BosonItem* item)
	: BosonItemRenderer(item)
{
 mModel = 0;
 mCurrentAnimation = 0;
 mCurrentFrame = 0.0f;
}

BosonItemModelRenderer::~BosonItemModelRenderer()
{
 for (unsigned int i = 0; i < mItemMatrixObjects.count(); i++) {
	delete mItemMatrixObjects[i];
 }
 mItemMatrixObjects.clear();
 mItemMatrices.clear();
}

bool BosonItemModelRenderer::setModel(BosonModel* model)
{
 mModel = model;
 if (!mModel) {
	boError() << k_funcinfo << "NULL model - we will crash!" << endl;
	return false;
 }

 setBoundingSphereRadius(model->boundingSphereRadius());

 BoLOD* lod = mModel->lod(0);
 if (!lod) {
	BO_NULL_ERROR(lod);
	return false;
 }
 mItemMatrices.resize(lod->frameCount());
 for (unsigned int i = 0; i < mItemMatrices.count(); i++) {
	BoFrame* frame = lod->frame(i);
	if (!frame) {
		BO_NULL_ERROR(frame);
		return false;
	}
	mItemMatrices[i].resize(frame->nodeCount());
	for (unsigned int j = 0; j < mItemMatrices[i].count(); j++) {
		(mItemMatrices[i])[j] = 0;
	}
 }

 if (::RTTI::isUnit(item()->rtti())) {
	Unit* u = (Unit*)item();
	int id = 0;
	while (u->weapon(id)) {
		BosonWeapon* w = u->weapon(id);
		id++;

		BosonWeaponTurret* t = w->turret();
		if (!t) {
			continue;
		}
		BoMatrix* matrix = new BoMatrix();
		mItemMatrixObjects.append(matrix);
		for (unsigned int i = 0; i < mItemMatrices.count(); i++) {
			BoFrame* frame = lod->frame(i);
			if (!frame) {
				BO_NULL_ERROR(frame);
				return false;
			}
			for (unsigned int j = 0; j < mItemMatrices[i].count(); j++) {
				BoMesh* mesh = frame->mesh(j);
				if (!mesh) {
					BO_NULL_ERROR(mesh);
					return false;
				}
				if (t->isMeshPartOfTurret(mesh->name())) {
					(mItemMatrices[i])[j] = matrix;
				}
			}
		}
	}
 }

 return true;
}

void BosonItemModelRenderer::setAnimationMode(int mode)
{
 BO_CHECK_NULL_RET(item());
 mAnimationMode = mode;
 if (!model()) {
	boError() << k_funcinfo << "in item id=" << item()->id() << " rtti=" << item()->rtti() << endl;
	BO_NULL_ERROR(model());
	return;
 }
 BosonAnimation* anim = model()->animation(mode);
 if (!anim) {
	if (mCurrentAnimation) {
		return;
	}
	anim = model()->animation(0);
	if (!anim) {
		boError() << k_funcinfo << "NULL default animation mode!" << endl;
		return;
	}
 }
 mCurrentAnimation = anim;
 mCurrentFrame = mCurrentAnimation->start();
}

void BosonItemModelRenderer::animate()
{
 BO_CHECK_NULL_RET(item());
 if (boGame) {
	if (boGame->gameMode()) {
		if (animationMode() != item()->animationMode()) {
			setAnimationMode(item()->animationMode());
			return;
		}
	} else {
		if (animationMode() != UnitAnimationIdle) {
			setAnimationMode(UnitAnimationIdle);
			return;
		}
	}
 }
 if (!mCurrentAnimation) {
	return;
 }
 if (mCurrentAnimation->speed() == 0) {
	return;
 }

 mCurrentFrame += mCurrentAnimation->speed();
 if (mCurrentFrame > mCurrentAnimation->end()) {
	if (mCurrentAnimation->loop()) {
		mCurrentFrame -= mCurrentAnimation->range();
	} else {
		mCurrentFrame = mCurrentAnimation->end();
	}
 }
}

void BosonItemModelRenderer::renderItem(unsigned int lod, bool transparentmeshes, RenderFlags flags)
{
 BO_CHECK_NULL_RET(mModel);
 BoLOD* l = mModel->lod(lod);
 BO_CHECK_NULL_RET(l);
 BoFrame* frame = l->frame((unsigned int)mCurrentFrame);
 BO_CHECK_NULL_RET(frame);
 if ((unsigned int)mCurrentFrame >= mItemMatrices.count()) {
	boError() << k_funcinfo << "frame " << (unsigned int)mCurrentFrame << " is out of range for item matrices" << endl;
	return;
 }
 frame->renderFrame(mItemMatrices[(unsigned int)mCurrentFrame], teamColor(), transparentmeshes, flags);
}

unsigned int BosonItemModelRenderer::lodCount() const
{
 if (!mModel) {
	return 1;
 }
 return mModel->lodCount();
}

unsigned int BosonItemModelRenderer::preferredLod(float dist) const
{
 if (!mModel) {
	return 0;
 }
 return mModel->preferredLod(dist);
}

