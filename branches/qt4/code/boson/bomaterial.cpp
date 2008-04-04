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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "bomaterial.h"

#include "../bomemory/bodummymemory.h"
#include "bosonconfig.h"
#include "botexture.h"
#include <bogl.h>

BoMaterial* BoMaterial::mCurrentMaterial = 0;
BoMaterial BoMaterial::mDefaultMaterial = BoMaterial();

void BoMaterial::init()
{
 // defaults from lib3ds:
// mAmbient = BoVector4Float(0.588235f, 0.588235f, 0.588235f, 0.0f);
// mDiffuse = BoVector4Float(0.588235f, 0.588235f, 0.588235f, 0.0f);
// mSpecular = BoVector4Float(0.898039f, 0.898039f, 0.898039f, 0.0f);
// mShininess = 0.1f;
 mShading = 3;
 mWireSize = 1.0f;

 // defaults from OpenGL:
// mAmbient = BoVector4Float(0.2f, 0.2f, 0.2f, 1.0f);
 // 0.2 for ambient is way too little, you totally depend only on diffuse light
 //  then. So we use 0.8 (same as diffuse)
 mAmbient = BoVector4Float(0.8f, 0.8f, 0.8f, 1.0f);
 mDiffuse = BoVector4Float(0.8f, 0.8f, 0.8f, 1.0f);
 mSpecular = BoVector4Float(0.0f, 0.0f, 0.0f, 1.0f);
 mShininess = 0.0f;

 // FIXME: these had no defaults in lib3ds. check whether the values are fine!
 mShinStrength = 0.0f;
 mUseBlur = false;
 mBlur = 0.0f;
 mTransparency = 0.0f;
 mFallOff = 0.0f;
 mAdditive = false;
 mUseFallOff = false;
 mSelfIllum = false;
 mSoften = false;
 mFaceMap = false;
 mTwoSided = false;
 mMapDecal = false;
 mUseWire = false;
 mUseWireAbs = false;

 mTextureObject = 0;
 mIsTransparent = false;
}

void BoMaterial::activate(BoMaterial* mat)
{
 if (mat == mCurrentMaterial) {
	return;
 }
 if (!mat) {
	activate(&mDefaultMaterial);
	mCurrentMaterial = mat;
	return;
 }

 if (mat->textureObject()) {
	mat->textureObject()->bind();
 } else {
	boTextureManager->unbindTexture();
 }

 if (boConfig->boolValue("UseLight") && boConfig->boolValue("UseMaterials")) { // useMaterials() is about OpenGL materials, not about the rest of BoMaterial (e.g. textures)
	// AB: my OpenGL sample code uses GL_FRONT, so I do as well. I think as back
	// faces are culled anyway we don't need GL_FRONT_AND_BACK.
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat->ambient().data());
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat->diffuse().data());
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat->specular().data());
	glMaterialf(GL_FRONT, GL_SHININESS, mat->shininess());
 }

 mCurrentMaterial = mat;
}

void BoMaterial::setDefaultAlpha(float alpha)
{
 mDefaultMaterial.mDiffuse.setW(alpha);
 mDefaultMaterial.mAmbient.setW(alpha);
 if (!mCurrentMaterial) {
	glMaterialfv(GL_FRONT, GL_AMBIENT, mDefaultMaterial.mAmbient.data());
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mDefaultMaterial.mDiffuse.data());
 }
}

BoMaterial& BoMaterial::operator=(const BoMaterial& mat)
{
#define COPY(x) x = mat.x
 COPY(mName);
 COPY(mAmbient);
 COPY(mDiffuse);
 COPY(mSpecular);
 COPY(mShininess);
 COPY(mShinStrength);
 COPY(mUseBlur);
 COPY(mBlur);
 COPY(mTransparency);
 COPY(mFallOff);
 COPY(mAdditive);
 COPY(mUseFallOff);
 COPY(mSelfIllum);
 COPY(mShading);
 COPY(mSoften);
 COPY(mFaceMap);
 COPY(mTwoSided);
 COPY(mMapDecal);
 COPY(mUseWire);
 COPY(mUseWireAbs);
 COPY(mWireSize);

 COPY(mTextureName);
 COPY(mTextureObject);
#undef COPY
 return *this;
}

