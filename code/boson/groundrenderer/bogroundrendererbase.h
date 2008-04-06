/*
    This file is part of the Boson game
    Copyright (C) 2003-2005 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOGROUNDRENDERERBASE_H
#define BOGROUNDRENDERERBASE_H

#include "../bogroundrenderer.h"
#include "../gameengine/bogroundquadtreenode.h"

#include <q3ptrdict.h>

class Cell;
class PlayerIO;
class QString;
class BosonMap;
class BoMatrix;
class BoPlane;
class bofixed;
template<class T> class BoVector3;
typedef BoVector3<bofixed> BoVector3Fixed;
typedef BoVector3<float> BoVector3Float;

class QRect;
class CellListBuilder;
class BoGroundRendererCellListLOD;
class BoColorMap;
class BoColorMapRenderer;
class BosonGroundThemeData;

class BoGroundRendererQuadTreeNode : public BoGroundQuadTreeNode
{
public:
	BoGroundRendererQuadTreeNode(int l, int t, int r, int b, int depth)
		:
		BoGroundQuadTreeNode(l, t, r, b, depth),
		mRoughnessMultiplier(100.0f)
	{
		mRoughness = 0.0f;
		mTextureRoughnessTotal = 0.0f;
		mRougnessPlusTextureRougnessTotalMulMultiplier = 0.0f;
	}
	static BoGroundRendererQuadTreeNode* createTree(unsigned int width, unsigned int height);

	virtual BoQuadTreeNode* createNode(int l, int t, int r, int b, int depth) const;

	virtual void cellTextureChanged(const BosonMap* map, int x1, int y1, int x2, int y2);
	virtual void cellHeightChanged(const BosonMap* map, int x1, int y1, int x2, int y2);

	/**
	 * The "roughness" is a that indicates how many height differences and
	 * how many texture differences in the node exist and how large they
	 * are.
	 *
	 * This value should be used to choose an LOD level. A higher value
	 * means a more detailed version should be shown, a lower version means
	 * that less details should be shown.
	 **/
	float roughnessValue(float dist) const
	{
		return mRougnessPlusTextureRougnessTotalMulMultiplier / dist;
	}


protected:
	void setRoughness(float r, float t)
	{
		mRoughness = r;
		mTextureRoughnessTotal = t;

		mRougnessPlusTextureRougnessTotalMulMultiplier = (mRoughness + mTextureRoughnessTotal) * mRoughnessMultiplier;
	}

	void calculateRoughness(const BosonMap* map);

private:
	const float mRoughnessMultiplier;
	float mRoughness;
	float mTextureRoughnessTotal;

	// cache for (mRougness + mTextureRougnessTotal) * roughnessMultiplier
	float mRougnessPlusTextureRougnessTotalMulMultiplier;
};


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
		mLastMap = 0;
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
	void cellChanged(int x1, int y1, int x2, int y2);
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
	const BosonMap* mLastMap;
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

	virtual bool initGroundRenderer();

	/**
	 * Generate a list of cells that are (or may) be visible at the moment.
	 * @param map The map that contains the @ref Cell pointers. Use 0 to
	 * delete the current list of cells.
	 **/
	virtual void generateCellList(const BosonMap* map);

	virtual void cellFogChanged(int x1, int y1, int x2, int y2);
	virtual void cellExploredChanged(int x1, int y1, int x2, int y2);
	virtual void cellHeightChanged(int x1, int y1, int x2, int y2);
	virtual void cellTextureChanged(int x1, int y1, int x2, int y2);

	virtual QString debugStringForPoint(const BoVector3Fixed& pos) const;

	/**
	 * Set an LOD object. This object is used to decide whether (and when)
	 * LOD should be used.
	 *
	 * The object is deleted on destruction of this object.
	 **/
	void setLODObject(BoGroundRendererCellListLOD* lod);

	/**
	 * @return The current @ref BosonGroundThemeData that belongs to the
	 * @ref BosonMap::groundTheme of the current map. See also @ref
	 * BosonViewData::groundThemeData.
	 *
	 * Note that you need to call @ref updateMapCache to make sure that this
	 * is valid. That method should be called whenever the current map (and
	 * therefore also the current groundtheme) might change.
	 **/
	BosonGroundThemeData* currentGroundThemeData() const
	{
		return mCurrentGroundThemeData;
	}

	static void getRoughnessInRect(const BosonMap* map, float* roughness, float* textureRoughnessTotal, int x1, int y1, int x2, int y2);
protected:
	virtual void renderVisibleCellsStart(const BosonMap* map);
	virtual void renderVisibleCellsStop(const BosonMap* map);

	/**
	 * Called regulary when the map might have changed. Call this before you
	 * access data that depends on the current map (and reimplement this to
	 * update such data).
	 *
	 *
	 * This method must be a noop when the given map is the same as the
	 * current map.
	 **/
	virtual void updateMapCache(const BosonMap* map);

	BoColorMapRenderer* getUpdatedColorMapRenderer(BoColorMap*);

	/**
	 * @return TRUE if at least one cell in the given rect is visble.
	 * Otherwise FALSE.
	 **/
	bool isCellInRectVisible(int x1, int y1, int x2, int y2) const;


protected:
	float* mHeightMap2;
	float* mVertexArray;
	unsigned char* mColorArray;
	bool* mUsedTextures;
	bool mUsedTexturesDirty;

private:
	CellListBuilder* mCellListBuilder;
	const BosonMap* mCurrentMap;
	BosonGroundThemeData* mCurrentGroundThemeData;

	FogTexture* mFogTexture;
	Q3PtrDict<BoColorMapRenderer> mColorMapRenderers;
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
	virtual ~BoGroundRendererCellListLOD()
	{
	}

	/**
	 * Called by the cell list builder. No need to call yourself.
	 **/
	void setViewFrustum(const BoFrustum* f)
	{
		mViewFrustum = f;
	}
	const BoFrustum* viewFrustum() const
	{
		return mViewFrustum;
	}

	float distanceFromPlane(const BoPlane& plane, const BoGroundRendererQuadTreeNode* node, const BosonMap* map) const;

	/**
	 * @return TRUE if the @p node is supposed to be displayed as a single
	 * quad. This is either the case if the node contains exactly one cell
	 * only, or if the distance from the player is high enough for this
	 * level of detail.
	 **/
	virtual bool doLOD(const BosonMap* map, const BoGroundRendererQuadTreeNode* node) const;

protected:
	const BoFrustum* mViewFrustum;
};


#endif

