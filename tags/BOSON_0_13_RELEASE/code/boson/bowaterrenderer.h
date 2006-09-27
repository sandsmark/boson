/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Rivo Laks (rivolaks@hot.ee)

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

#ifndef BOWATERRENDERER_H
#define BOWATERRENDERER_H


#include "bo3dtools.h"

#include <qptrlist.h>


class BosonMap;
class BoTexture;
class BoLight;
class PlayerIO;
class BoShader;

class QRect;
class QDomElement;

class BoLake;


/**
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BoLakeGL
{
  public:
    /**
     * Constructs a lake with specified level
     **/
    BoLakeGL(BoLake* l, const BosonMap*);

    ~BoLakeGL();

    BoLake* lake;

    // Center point of the lake, for in-frustum checks
    BoVector3Float center;
    // 2d radius of the lake, for in-frustum checks
    float radius;

    // direction and speed of the waves
    BoVector2Float waveVector;
    BoMatrix textureMatrix;

    float alphaMultiplier;
    float alphaBase;

    /**
     * Represents one chunk of lake.
     * Every lake is divided into one or more chunk. When rendering, ever chunk
     *  tested for visibility and then rendered if necessary.
     **/
    class WaterChunk
    {
      public:
        WaterChunk()
        {
          dirty = true;
          lastdetail = -1.0f;
          vertices = 0;
          colors = 0;
          indices = 0;
          indices_count = 0;
          vbo_vertex = 0;
          vbo_color = 0;
          vbo_index = 0;
        }
        ~WaterChunk()
        {
          delete[] vertices;
          delete[] colors;
          delete[] indices;
        }

        // Dirty flag
        bool dirty;
        // Coordinates, also in corners and inclusive
        int minx, miny;
        int maxx, maxy;
        // Center of the chunk
        BoVector3Float center;
        // Min/max ground height beneath the chunk
        float mingroundheight, maxgroundheight;
        // Number of valid corners, i.e. those that have to be rendered (are
        //  really water). Note that this also includes shore corners.
        int corners;
        // Last detail level that was used to render the chunk. If current
        //  detail level is different from this, caches will be reallocated.
        float lastdetail;
        // Caches
        BoVector3Float* vertices;
        BoVector4Float* colors;
        unsigned int* indices;
        int indices_count;
        // VBO ids
        unsigned int vbo_vertex;
        unsigned int vbo_color;
        unsigned int vbo_index;
    };

    QPtrList<WaterChunk> chunks;

  protected:
    void init(BoLake* l, const BosonMap*);
};


#define boWaterRenderer BoWaterRenderer::waterRenderer()

/**
 * @short Water Renderer
 *
 * Takes care of rendering water
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BoWaterRenderer
{
  public:
    BoWaterRenderer();
    ~BoWaterRenderer();

    static void initStatic();
    static void deleteStatic();
    static BoWaterRenderer* waterRenderer();

    void initOpenGL();
    // Loads necessary textures (envmap, bumpmaps). Call this when config has
    //  changed.
    void loadNecessaryTextures();

    void setMap(const BosonMap* map);
    float waterAlphaAt(BoLakeGL* lake, float x, float y);

    float time() const;

    void update(float elapsed);
    void modelviewMatrixChanged(const BoMatrix& modelview);
    void cellExploredChanged(int x1, int x2, int y1, int y2);

    void render();

    QString currentRenderStatisticsData() const;

    void setViewFrustum(const BoFrustum* f)  { mViewFrustum = f; setDirty(true); }
    void setSun(BoLight* sun)  { mSun = sun; setDirty(true); }
    void setCameraPos(const BoVector3Float& pos)  { mCameraPos = pos; setDirty(true); }
    void setLocalPlayerIO(PlayerIO* playerIO)  { mLocalPlayerIO = playerIO; }

    void reloadConfiguration();

    bool supportsShaders() const;
    bool supportsReflections() const;
    bool supportsTranslucency() const;
    bool supportsBumpmapping() const;

  protected:
    float sphereInFrustum(const BoVector3Float& pos, float radius) const;

    void renderLake(BoLakeGL* lake);
    void renderChunk(BoLakeGL* lake, BoLakeGL::WaterChunk* chunk, float chunk_detail);

    void setDirty(bool d);


    /**
     * Internal class, used to store rendering info about a chunk.
     * This is mostly used to make communications between different methods
     *  easier (so they don't have to return zillion variables).
     **/
    class RenderInfo
    {
      public:
        BoLakeGL* lake;
        BoLakeGL::WaterChunk* chunk;
        float detail;
        // How many "border cells" a chunk has. Those are used only for
        //  calculating per-cell stuff which will be smoothed later (normals
        //  and tangentspace light- and half-vectors).
        // They should be 0 or 1 (no border or border).
        int border_left, border_right, border_top, border_bottom;
        float cellminx, cellminy, cellmaxx, cellmaxy;
        // Chunks size in cells, including border
        int chunkcellw, chunkcellh;
        // Size in corners, _not_ including border
        int chunkcornerw, chunkcornerh;
        int texrepeat;
        BoVector3Float lightvector;
        // Pointers to whereever data is stored - either vbos or plain arrays.
        BoVector3Float* vertices_p;
        BoVector3Float* normals_p;
        BoVector4Float* colors_p;
        // Same, but for indices
        unsigned int* indices_p;
        // Rendering tehniques stuff
        float constalpha;
        bool singleQuad;
    };

    // renderChunk() helper methods:
    // Reallocates various (e.g. normal) arrays in the chunk.
    void reallocateArrays(RenderInfo* info);
    // Calculates per-corner stuff for chunk
    void calculatePerCornerStuff(RenderInfo* info);
    // Inits whatever data buffers are used (vbos/arrays), so that you can use
    //  pointers in RenderInfo to store data. Note that this may also clear the
    //  buffers.
    void initDataBuffersForStorage(RenderInfo* info);
    // Uninits data buffers.
    void uninitDataBuffersForStorage(RenderInfo* info);

    // Calculates indices array for chunk.
    void calculateIndices(RenderInfo* info);

    // Sets up env map texture (for reflection) for given texture unit.
    void setupEnvMapTexture(int unit);
    // Sets up bumpmap texture for given texture unit.
    void setupBumpMapTexture(int unit);
    // Sets up diffuse (i.e. usual) texture for given texture unit.
    void setupDiffuseTexture(int unit);

    // Inits OpenGL rendering environment (e.g. textures).
    void initRenderEnvironment();

  private:
    static BoWaterRenderer* mRenderer;


    const BosonMap* mMap;
    QPtrList<BoLakeGL> mLakes;

    float mTime;
    bool mDirty;
    bool mOpenGLInited;
    const BoFrustum* mViewFrustum;
    BoLight* mSun;
    BoVector3Float mCameraPos;
    PlayerIO* mLocalPlayerIO;
    bool mRenderEnvironmentSetUp;
    BoShader* mShader;

    bool mSupports_texlod;
    bool mSupports_env_combine;
    bool mSupports_env_dot3;
    bool mSupports_vbo;
    bool mSupports_blendcolor;
    bool mSupports_blendcolor_ext;
    bool mSupports_shaders;

    bool mEnableReflections;
    bool mEnableBumpmapping;
    bool mEnableAnimBumpmaps;
    bool mEnableTranslucency;
    bool mEnableVBO;
    bool mEnableSpecular;
    bool mEnableShader;

    float mReflectionSharpness;
    float mReflectionStrength;
    float mWaterAmbientColor;
    float mWaterDiffuseColor;
    float mWaterSpecularColor;
    float mWaterShininess;

    BoTexture* mWaterTex;
    BoTexture* mWaterBump;
    BoTexture* mEnvMap;
    QPtrList<BoTexture> mWaterAnimBump;
    float mWaterAnimBumpCurrent;
    BoMatrix mModelview;
    BoMatrix mInverseModelview;

    int mRenderedLakes;
    int mRenderedChunks;
    int mRenderedQuads;
};

#endif // BOWATERRENDERER_H

