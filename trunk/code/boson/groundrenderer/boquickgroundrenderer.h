/*
    This file is part of the Boson game
    Copyright (C) 2005 Rivo Laks <rivolaks@hot.ee>

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

#ifndef BOQUICKGROUNDRENDERER_H
#define BOQUICKGROUNDRENDERER_H

#include "../bogroundrenderer.h"

#include "../bo3dtools.h"

#include <qptrlist.h>
#include <qptrdict.h>


class BosonMap;
class BoColorMap;
class BoColorMapRenderer;
class FogTexture;


/**
 * @short Quick ground renderer.
 *
 * This is the "Quick" ground renderer.
 * It uses VBOs and few other tricks to accelerate the rendering.
 * These "tricks" include:
 * @li Vertices, normals and texture weights are stored in VBOs and thus they
 *  don't have to be calculated and sent to GPU every frame.
 * @li It uses only fog texture to fog the cells, it doesn't "cut out" fogged
 *  cells.
 * @li It divides terrain into chunks and then does culling per-chunk.
 * @li Texture coordinates are not calculated/sent to GPU. Instead, OpenGL's
 *  automatic texture coordinate generation is used.
 *
 * The renderer uses geomipmapping for LOD.
 * LOD choosing algorithm takes chunk's roughness into account, so flat areas
 *  will be optimized more than rough areas.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BoQuickGroundRenderer : public BoGroundRenderer
{
  Q_OBJECT
  public:
    BoQuickGroundRenderer();
    virtual ~BoQuickGroundRenderer();

    virtual bool initGroundRenderer();

    virtual bool usable() const;

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

  protected:
    class TerrainChunk
    {
      public:
        ~TerrainChunk()
        {
          delete[] hastexture;
        }

        // Min/max coordinates of the chunk. Note that max coordinate is not
        //  inclusive, so  width = maxX - minX.
        unsigned int minX;
        unsigned int minY;
        unsigned int maxX;
        unsigned int maxY;

        // Min/max coordinates of the part of the chunk that will be rendered
        //  using triangle strips. The rest will be rendered using trifans, to
        //  glue the chunk to a neighboring chunk which has different lod
        //  level.
        unsigned int minRenderX;
        unsigned int minRenderY;
        unsigned int maxRenderX;
        unsigned int maxRenderY;

        // Whether the chunk should be rendered
        bool render;

        // LOD level to be used for this chunk
        unsigned int useLOD;

        // Pointers to neighbors of this chunk. 0 if neighbor doesn't exist.
        // Neighbors are in order: left, top, right, bottom
        TerrainChunk* neighbors[4];

        // Whether all the cells in this chunk are unexplored
        bool unexplored;
        // Whether a certain texture is used by this chunk
        bool* hastexture;

        // These are used for in-frustum checks
        float minZ;
        float maxZ;
        BoVector3Float center;
        float radius;

        // 'Roughness' of the chunk, used for lod level calculations
        float roughness;
        float textureRoughnessTotal;
    };


    virtual void renderVisibleCellsStart(const BosonMap* map);
    virtual void renderVisibleCellsStop(const BosonMap* map);

    virtual void renderVisibleCells(int* cells, unsigned int cellsCount, const BosonMap* map, RenderFlags flags);

    void initMap(const BosonMap* map);

    TerrainChunk* chunkAt(int x, int y) const;

    unsigned int chooseLOD(TerrainChunk* chunk, float dist);

    // Helper methods to 'glue' a chunk to a neighboring chunk which has higher
    //  lod level (and thus less detail)
    void glueToLeft(TerrainChunk* chunk, TerrainChunk* neighbor);
    void glueToTop(TerrainChunk* chunk, TerrainChunk* neighbor);
    void glueToRight(TerrainChunk* chunk, TerrainChunk* neighbor);
    void glueToBottom(TerrainChunk* chunk, TerrainChunk* neighbor);

    int renderChunk(TerrainChunk* c, unsigned int* indices);

  private:
#warning FIXME: duplicated code
    BoColorMapRenderer* getUpdatedColorMapRenderer(BoColorMap*);

  private:
    const BosonMap* mMap;
    // Map width/height, in both cells and corners
    unsigned int mMapW;
    unsigned int mMapH;
    unsigned int mMapCW;
    unsigned int mMapCH;

    bool mDrawGrid;

    // VBOs for vertices, normals and texture weights
    unsigned int mVBOVertex;
    unsigned int mVBONormal;
    unsigned int mVBOTexture;
    // Size of one texture weights layer, in bytes
    unsigned int mVBOTextureLayerSize;
    // How many textures are there
    unsigned int mTextureCount;

    FogTexture* mFogTexture;

    TerrainChunk* mChunks;
    unsigned int mChunkCount;
    unsigned int mChunkSize;

    bool mCellListDirty;

    QPtrDict<BoColorMapRenderer> mColorMapRenderers;
};

#endif

