/*
    This file is part of the Boson game
    Copyright (C) 2003-2005 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef BOGROUNDRENDERERBASE_H
#define BOGROUNDRENDERERBASE_H

#include "../bogroundrenderer.h"

class Cell;
class PlayerIO;
class QString;
class BosonMap;
class BoMatrix;
class bofixed;
template<class T> class BoVector3;
typedef BoVector3<bofixed> BoVector3Fixed;
typedef BoVector3<float> BoVector3Float;

class QRect;
class CellListBuilder;
class BoQuadTreeNode;
class BoGroundRendererCellListLOD;


class FogTexture
{
public:
	FogTexture(bool smoothedges = true)
	{
		mFogTexture = 0;
		mFogTextureData = 0;
		mFogTextureDataW = 0;
		mFogTextureDataH = 0;
		mLastMapWidth = 0;
		mLastMapHeight = 0;
		mFogTextureDirty = false;
		mFogTextureDirtyAreaX1 = 0;
		mFogTextureDirtyAreaX2 = 0;
		mFogTextureDirtyAreaY1 = 0;
		mFogTextureDirtyAreaY2 = 0;
		mSmoothEdges = smoothedges;
	}
	~FogTexture();

	void start(const BosonMap* map);
	void stop(const BosonMap* map);
	void cellChanged(int x, int y);
	void setLocalPlayerIO(PlayerIO* io)
	{
		mLocalPlayerIO = io;
	}
	PlayerIO* localPlayerIO() const
	{
		return mLocalPlayerIO;
	}

protected:
	void initFogTexture(const BosonMap* map);

	/**
	 * Updates fog texture if it's dirty
	 **/
	void updateFogTexture();

private:
	BoTexture* mFogTexture;
	unsigned char* mFogTextureData;
	int mFogTextureDataW;
	int mFogTextureDataH;
	unsigned int mLastMapWidth;
	unsigned int mLastMapHeight;
	bool mFogTextureDirty;
	int mFogTextureDirtyAreaX1;
	int mFogTextureDirtyAreaY1;
	int mFogTextureDirtyAreaX2;
	int mFogTextureDirtyAreaY2;
	bool mSmoothEdges;

	PlayerIO* mLocalPlayerIO;
};


class BoGroundRendererBase : public BoGroundRenderer
{
	Q_OBJECT
public:
	BoGroundRendererBase();
	virtual ~BoGroundRendererBase();

	/**
	 * Generate a list of cells that are (or may) be visible at the moment.
	 * @param map The map that contains the @ref Cell pointers. Use 0 to
	 * delete the current list of cells.
	 **/
	virtual void generateCellList(const BosonMap* map);

	virtual void cellFogChanged(int x, int y);

	virtual QString debugStringForPoint(const BoVector3Fixed& pos) const;

	/**
	 * Set an LOD object. This object is used to decide whether (and when)
	 * LOD should be used.
	 *
	 * The object is deleted on destruction of this object.
	 **/
	void setLODObject(BoGroundRendererCellListLOD* lod);

protected:
	virtual void renderVisibleCellsStart(const BosonMap* map);
	virtual void renderVisibleCellsStop(const BosonMap* map);

protected:
	float* mHeightMap2;

private:
	CellListBuilder* mCellListBuilder;
	const BosonMap* mMap;

	FogTexture* mFogTexture;
};


/**
 * Helper class for building the list of visible cells. An object of this class
 * is queried to find out whether LOD should be applied or not.
 *
 * The default implementation should be sufficient usually.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoGroundRendererCellListLOD
{
public:
	BoGroundRendererCellListLOD()
	{
		mViewFrustum = 0;
	}

	/**
	 * Called by the cell list builder. No need to call yourself.
	 **/
	void setViewFrustum(const float* f)
	{
		mViewFrustum = f;
	}
	const float* viewFrustum() const
	{
		return mViewFrustum;
	}

	float distanceFromPlane(const float* plane, const BoQuadTreeNode* node, const BosonMap* map) const;

	/**
	 * @return TRUE if the @p node is supposed to be displayed as a single
	 * quad. This is either the case if the node contains exactly one cell
	 * only, or if the distance from the player is high enough for this
	 * level of detail.
	 **/
	virtual bool doLOD(const BosonMap* map, const BoQuadTreeNode* node) const;

protected:
	const float* mViewFrustum;
};


#endif

