/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#ifndef BOMATERIAL_H
#define BOMATERIAL_H

#include "bo3dtools.h"
#include <qstring.h>

/**
 * Note: .3ds seem to support different materials for every face. But in boson
 * we allow only a single material per mesh (performance reasons).
 *
 * @short Material settings for a @ref BoMesh object
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoMaterial
{
public:
	BoMaterial();
	~BoMaterial();

	static void activate(BoMaterial*);
	static void deactivate() { activate(0); }
	void activate() { activate(this); }

	/**
	 * Set a name for the material. This is used for debugging purposes
	 * only.
	 **/
	void setName(const QString& name) { mName = name; }
	const QString& name() const { return mName; }

	// AB: we don't use all of these, but provide them for future use.
	void setAmbient(const BoVector4& v) { mAmbient = v; }
	void setDiffuse(const BoVector4& v) { mDiffuse = v; }
	void setSpecular(const BoVector4& v) { mSpecular = v; }
	void setShininess(float s) { mShininess = s; }
	void setShinStrength(float s) { mShinStrength = s; }
	void setBlur(float b) { mBlur = b; }
	void setTransparency(float t) { mTransparency = t; }
	void setFallOff(float f) { mFallOff = f; }
	void setAdditive(bool a) { mAdditive = a; }
	void setUseFallOff(bool v) { mUseFallOff = v; }
	void setSelfIllum(bool v) { mSelfIllum = v; }
	void setShading(int s) { mShading = s; }
	void setSoften(bool v) { mSoften = v; }
	void setFaceMap(bool v) { mFaceMap = v; }
	void setTwoSided(bool v) { mTwoSided = v; }
	void setMapDecal(bool v) { mMapDecal = v; }
	void setUseWire(bool v) { mUseWire = v; }
	void setUseWireAbs(bool v) { mUseWireAbs = v; }
	void setWireSize(float s) { mWireSize = s; }

	/**
	 * Set the (file-)name of the primary texture map. In boson we use this
	 * texture map only, I (AB) have no idea what the others are for
	 * (unfortunately lib3ds is not documented).
	 *
	 * In lib3ds (see lib3ds/material.h) this texture map is referred to as
	 * "texture1_map". Also we don't use the _mask maps.
	 *
	 * Note that you won't find all the other values, such as texture
	 * offsets and rotations in this class (not yet?). They are calculated
	 * directly by the .3ds file loader (see @ref Bo3DSLoad) and integrated
	 * into the actual texel positions.
	 *
	 * @param name The name of the texture, in plain format. I.e. don't use
	 * @ref BosonModel::cleanTextureName on this.
	 **/
	void setTextureName(const QString& name) { mTextureName = name; }

	/**
	 * @return The texture (file-)name, as set by @ref setTextureName. Use
	 * @ref BosonModel::cleanTextureName to get the actual filename.
	 **/
	const QString& textureName() const { return mTextureName; }

	void setTextureObject(unsigned int t) { mTextureObject = t; }
	unsigned int textureObject() const { return mTextureObject; }

private:
	static BoMaterial* mCurrentMaterial;

private:
	QString mName;

	// all values from a .3ds material
	// note that we don't use them all!
	// see lib3ds/material.h for a listing (no, no further info, as lib3ds
	// isn't documented)
	BoVector4 mAmbient;
	BoVector4 mDiffuse;
	BoVector4 mSpecular;
	float mShininess;
	float mShinStrength;
	bool mUseBlur;
	float mBlur;
	float mTransparency;
	float mFallOff;
	bool mAdditive;
	bool mUseFallOff;
	bool mSelfIllum;
	int mShading;
	bool mSoften;
	bool mFaceMap;
	bool mTwoSided;
	bool mMapDecal;
	bool mUseWire;
	bool mUseWireAbs;
	float mWireSize;

	// atm we store the first texture map only ("texture1_map" from
	// lib3ds/material.h). the others are unused by us and I (AB) have no
	// idea (yet) what they are for. maybe they will get added one day.
	QString mTextureName;
	unsigned int mTextureObject;

};

#endif

