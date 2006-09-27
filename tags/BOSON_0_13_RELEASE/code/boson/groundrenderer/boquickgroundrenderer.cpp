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

#include <bogl.h>

#include "boquickgroundrenderer.h"
#include "boquickgroundrenderer.moc"

#include "../../bomemory/bodummymemory.h"
#include "../gameengine/bosonmap.h"
#include "../gameengine/bosongroundtheme.h"
#include "../bosongroundthemedata.h"
#include "../bo3dtools.h"
#include "../gameengine/boson.h"
#include "../botexture.h"
#include "../gameengine/playerio.h"
#include "../bosonconfig.h"
#include "../boshader.h"
#include "../bosonviewdata.h"
#include "bocolormaprenderer.h"
#include "bogroundrendererbase.h"
#include "bodebug.h"

#include <math.h>


#define ROUGHNESS_MULTIPLIER 100.0f


#warning FIXME: use base class
BoQuickGroundRenderer::BoQuickGroundRenderer()
{
  mMap = 0;

  mMapW = mMapH = mMapCW = mMapCH = 0;

  mDrawGrid = false;

  mVBOVertex = 0;
  mVBONormal = 0;
  mVBOTexture = 0;
  mTextureCount = 0;

  mFogTexture = 0;

  mChunks = 0;

  mCellListDirty = true;
}

BoQuickGroundRenderer::~BoQuickGroundRenderer()
{
  // Delete VBOs
  if (glDeleteBuffers)
  {
    glDeleteBuffers(1, &mVBOVertex);
    glDeleteBuffers(1, &mVBONormal);
    glDeleteBuffers(1, &mVBOTexture);
  }

  // Delete fog texture
  delete mFogTexture;

  // Delete chunks
  delete[] mChunks;
}

bool BoQuickGroundRenderer::initGroundRenderer()
{
  if(!BoGroundRenderer::initGroundRenderer())
  {
    return false;
  }

  mFogTexture = new FogTexture();

  return true;
}

bool BoQuickGroundRenderer::usable() const
{
  if(boTextureManager->textureUnits() < 2)
  {
    return false;
  }
  if(!glBindBuffer || !glBufferData)
  {
    return false;
  }
  if(boConfig->boolValue("EnableMesaVertexArraysWorkarounds"))
  {
    return false;
  }

  return true;
}

void BoQuickGroundRenderer::generateCellList(const BosonMap* map)
{
  if(mMap != map)
  {
    initMap(map);
  }

  // First pass
  // Find visible chunks and calculate lod level for them
  float mindist = 1000000.0f;
  float maxdist = -1000000.0f;
  for(unsigned int i = 0; i < mChunkCount; i++)
  {
    TerrainChunk* c = &mChunks[i];
    c->render = false;

    // Don't consider completely unexplored chunks
    if(c->unexplored)
    {
      continue;
    }

    // Check if the chunk is visible
    float dist = viewFrustum()->sphereInFrustum(c->center, c->radius);
    if(dist == 0)
    {
      continue;
    }
    else if(viewFrustum()->boxInFrustum(BoVector3Float(c->minX, -((float)c->minY), c->minZ),
        BoVector3Float(c->maxX, -((float)c->maxY), c->maxZ)) == 0)
    {
      continue;
    }

    // It's in frustum
    c->render = true;
    // Choose lod level for the chunk
    c->useLOD = chooseLOD(c, dist);
    mindist = QMIN(mindist, dist - 2 * c->radius);
    maxdist = QMAX(maxdist, dist);
  }

  // Second pass
  // Calculate min/max coords of the rendered part
  unsigned int visible = 0;
  for(unsigned int i = 0; i < mChunkCount; i++)
  {
    TerrainChunk* c = &mChunks[i];
    if(!c->render)
    {
      continue;
    }
    visible++;

    unsigned int step = (1 << c->useLOD);

    c->minRenderX = c->minX;
    c->minRenderY = c->minY;
    c->maxRenderX = c->maxX;
    c->maxRenderY = c->maxY;

    if(c->neighbors[0] && c->neighbors[0]->render && c->neighbors[0]->useLOD > c->useLOD)
    {
      c->minRenderX += step;
    }
    if(c->neighbors[1] && c->neighbors[1]->render && c->neighbors[1]->useLOD > c->useLOD)
    {
      c->minRenderY += step;
    }
    if(c->neighbors[2] && c->neighbors[2]->render && c->neighbors[2]->useLOD > c->useLOD)
    {
      c->maxRenderX -= step;
    }
    if(c->neighbors[3] && c->neighbors[3]->render && c->neighbors[3]->useLOD > c->useLOD)
    {
      c->maxRenderY -= step;
    }
  }


  // Hack to make BoGroundRenderer happy
#if 0
  int* cells = new int[visible * 4];
  for(unsigned int i = 0; i < visible; i++)
  {
    cells[visible*4 + 0] = cells[visible*4 + 1] = 0;
    cells[visible*4 + 2] = cells[visible*4 + 3] = 1;
  }
  setRenderCells(cells, visible);
  setRenderCellsCount(visible);
#else
  int* cells = new int[4];
  cells[0] = cells[1] = 0;
  cells[2] = cells[3] = 1;
  setRenderCells(cells, 1);
  setRenderCellsCount(1);
#endif

  mCellListDirty = false;

  statistics()->setMinDistance(QMAX(0, mindist));
  statistics()->setMaxDistance(QMAX(0, maxdist));
}

void BoQuickGroundRenderer::renderVisibleCells(int*, unsigned int, const BosonMap* map, RenderFlags flags)
{
  if(map != mMap)
  {
    boError() << k_funcinfo << "Different map expected! Got: " << map << "; expected: " << mMap << endl;
    return;
  }

  if(mCellListDirty)
  {
    generateCellList(map);
  }
  BO_CHECK_NULL_RET(mMap);
  BO_CHECK_NULL_RET(mMap->groundTheme());
  BO_CHECK_NULL_RET(boViewData);
  BosonGroundThemeData* groundThemeData = boViewData->groundThemeData(mMap->groundTheme());
  BO_CHECK_NULL_RET(groundThemeData);

  // Statistics
  unsigned int renderedQuads = 0;

  // Temporary array to hold the indices for tristrips
  unsigned int* indices = new unsigned int[2 * (mChunkSize + 1)];

  bool useShaders = boConfig->boolValue("UseGroundShaders");
  bool depthonly = flags & DepthOnly;


  glPushClientAttrib(GL_ALL_ATTRIB_BITS);

  // Bind VBOs
  glEnableClientState(GL_VERTEX_ARRAY);
  glBindBuffer(GL_ARRAY_BUFFER, mVBOVertex);
  glVertexPointer(3, GL_FLOAT, 0, 0);

  // We don't need normals and texture weights (stored in color) for depth pass
  if(!depthonly)
  {
    glEnableClientState(GL_NORMAL_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, mVBONormal);
    glNormalPointer(GL_FLOAT, 0, 0);

    glEnableClientState(GL_COLOR_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, mVBOTexture);
  }

//  glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_TEXTURE_BIT);
  glPushAttrib(GL_ALL_ATTRIB_BITS);

  // Texture stuff
  // Use OpenGL's automatic texture coordinate generation.
  if(!depthonly)
  {
    boTextureManager->activateTextureUnit(0);
    float texPlaneS[] = { 1.0, 0.0, 0.0, 0.0 };
    float texPlaneT[] = { 0.0, 1.0, 0.0, 0.0 };
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glTexGenfv(GL_S, GL_OBJECT_PLANE, texPlaneS);
    glTexGenfv(GL_T, GL_OBJECT_PLANE, texPlaneT);

    // Set blending function
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }
  glMatrixMode(GL_TEXTURE);

  // Depth test
  glDepthFunc(GL_LEQUAL);

  // Go through all ground types
  unsigned int groundcount = groundThemeData->groundTypeCount();
  for(unsigned int i = 0; i < groundcount; i++)
  {
    // Depth pass needs only a single pass
    if(depthonly && i > 0)
    {
      break;
    }
    // If this is pass 2, enable blending (it's initially disabled)
    if(i == 1)
    {
      glEnable(GL_BLEND);
    }

    // We do lazy initing
    bool inited = false;

    // Render all chunks using this texture
    for(unsigned int j = 0; j < mChunkCount; j++)
    {
      TerrainChunk* c = &mChunks[j];
      if(!c->render)
      {
        continue;
      }
      if(!depthonly && !c->hastexture[i])
      {
        continue;
      }

      // Init if necessary
      if(!depthonly && !inited)
      {
        BosonGroundTypeData* groundData = groundThemeData->groundTypeData(i);
        // Bind current texture
        BoTexture* tex = groundData->currentTexture(boGame->advanceCallsCount());
        tex->bind();
        // Set correct tex planes
        glLoadIdentity();
        glScalef(1.0f / groundData->groundType->textureSize, 1.0f / groundData->groundType->textureSize, 1.0);
        if(useShaders)
        {
          // Bind bump tex
          boTextureManager->activateTextureUnit(2);
          BoTexture* bumptex = groundData->currentBumpTexture(boGame->advanceCallsCount());
          bumptex->bind();
          glLoadIdentity();
          glScalef(1.0f / groundData->groundType->textureSize, 1.0f / groundData->groundType->textureSize, 1.0);
          boTextureManager->activateTextureUnit(0);
          // Shader
          groundData->shader->bind();
          groundData->shader->setUniform("bumpScale", groundData->groundType->bumpScale);
          groundData->shader->setUniform("bumpBias", groundData->groundType->bumpBias);
        }
        // Bind texture weights vbo corresponding to this texture
        glColorPointer(4, GL_UNSIGNED_BYTE, 0, (char*)NULL + mVBOTextureLayerSize*i);
        inited = true;
      }

      // Render!
      renderedQuads += renderChunk(c, indices);
    }
  }

  glLoadIdentity();

  if(!depthonly)
  {
    // Disable shader
    if(useShaders)
    {
      boTextureManager->activateTextureUnit(2);
      glLoadIdentity();
      boTextureManager->disableTexturing();
      boTextureManager->activateTextureUnit(0);
      BoShader::unbind();
    }
    boTextureManager->disableTexturing();

    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_BLEND);
  }

  glMatrixMode(GL_MODELVIEW);
  glColor4ub(255, 255, 255, 255);

  // Render the color map if necessary
#warning FIXME: does NOT belong to quick renderer. belongs to base class.
  if(!depthonly && map->activeColorMap())
  {
    BoColorMapRenderer* renderer = getUpdatedColorMapRenderer(map->activeColorMap());
    if(renderer)
    {
      glDisableClientState(GL_COLOR_ARRAY);
      glPushAttrib(GL_ENABLE_BIT);
      glEnable(GL_BLEND);
      glColor4ub(255, 255, 255, 128);
      glDisable(GL_LIGHTING);
      renderer->start(map);

      for(unsigned int j = 0; j < mChunkCount; j++)
      {
        TerrainChunk* c = &mChunks[j];
        if(!c->render)
        {
          continue;
        }

        renderedQuads += renderChunk(c, indices);
      }

      renderer->stop();
      glPopAttrib();
    }
  }

  // Optionally render the grid
  if(!depthonly && mDrawGrid)
  {
    glDisableClientState(GL_COLOR_ARRAY);
    glDisable(GL_LIGHTING);
    glDisable(GL_NORMALIZE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glColor3ub(127, 127, 127);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    for(unsigned int j = 0; j < mChunkCount; j++)
    {
      TerrainChunk* c = &mChunks[j];
      if(!c->render)
      {
        continue;
      }

      renderedQuads += renderChunk(c, indices);
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  // Disable VBO
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glPopAttrib();
  glPopClientAttrib();

  delete[] indices;

  statistics()->setRenderedQuads(renderedQuads);
}

int BoQuickGroundRenderer::renderChunk(BoQuickGroundRenderer::TerrainChunk* c, unsigned int* indices)
{
  int renderedQuads = 0;

  // Size of a rendered quad (cell) in cells
  unsigned int step = (1 << c->useLOD);

  for(unsigned int y = c->minRenderY; y < c->maxRenderY; y += step)
  {
    unsigned int indexcount = 0;
    unsigned int ystep = QMIN((int)step, (int)c->maxRenderY - (int)y);

    indices[indexcount++] = y * mMapCW + c->minRenderX;
    indices[indexcount++] = (y + ystep) * mMapCW + c->minRenderX;
    for(unsigned int x = c->minRenderX; x < c->maxRenderX; x += step)
    {
      unsigned int xstep = QMIN((int)step, (int)c->maxRenderX - (int)x);
      indices[indexcount++] = y * mMapCW + (x + xstep);
      indices[indexcount++] = (y + ystep) * mMapCW + (x + xstep);
    }

    glDrawElements(GL_TRIANGLE_STRIP, indexcount, GL_UNSIGNED_INT, indices);
    renderedQuads += indexcount - 2;
  }

  // Glue the chunk to neighbors if necessary
  if(c->neighbors[0] && c->neighbors[0]->render && c->neighbors[0]->useLOD > c->useLOD)
  {
    glueToLeft(c, c->neighbors[0]);
  }
  if(c->neighbors[1] && c->neighbors[1]->render && c->neighbors[1]->useLOD > c->useLOD)
  {
    glueToTop(c, c->neighbors[1]);
  }
  if(c->neighbors[2] && c->neighbors[2]->render && c->neighbors[2]->useLOD > c->useLOD)
  {
    glueToRight(c, c->neighbors[2]);
  }
  if(c->neighbors[3] && c->neighbors[3]->render && c->neighbors[3]->useLOD > c->useLOD)
  {
    glueToBottom(c, c->neighbors[3]);
  }

  return renderedQuads;
}

void BoQuickGroundRenderer::glueToLeft(BoQuickGroundRenderer::TerrainChunk* c, BoQuickGroundRenderer::TerrainChunk* n)
{
  int minStep = 1 << c->useLOD;
  int maxStep = 1 << n->useLOD;

  unsigned int tedge = c->minY;
  unsigned int bedge = c->maxY;
  if(c->neighbors[1] && c->neighbors[1]->useLOD > c->useLOD)
  {
    tedge += minStep;
  }
  if(c->neighbors[3] && c->neighbors[3]->useLOD > c->useLOD)
  {
    bedge -= minStep;
  }

  unsigned int j0 = tedge;
  unsigned int j1 = tedge + minStep;

  unsigned int* indices = new unsigned int[mChunkSize*2 * 3];
  unsigned int indexcount = 0;

  // Go through all vertices on the side of the neighbor
  for(unsigned int i = n->minY; i < n->maxY; i += maxStep)
  {
    // Next vertex on neighbor's edge
    unsigned int nexti = QMIN(i + maxStep, c->maxY);

    // Pos of neighbor's current vertex in the vbo
    unsigned int ipos = i * mMapCW + n->maxRenderX;

    // Go through all vertices which lie between neighbor's current and next
    //  vertex.
    for(; j0 < nexti && j0 < bedge; j1 += minStep)
    {
      j1 = QMIN(j1, bedge);
      indices[indexcount++] = ipos;
      indices[indexcount++] = j0 * mMapCW + c->minRenderX;
      indices[indexcount++] = j1 * mMapCW + c->minRenderX;
      j0 = j1;
    }

    // Add last triangle for this pair of vertices
    indices[indexcount++] = ipos;
    indices[indexcount++] = j0 * mMapCW + c->minRenderX;
    indices[indexcount++] = nexti * mMapCW + n->maxRenderX;
  }

  glDrawElements(GL_TRIANGLES, indexcount, GL_UNSIGNED_INT, indices);
  //renderedQuads += indexcount / 3;

  delete[] indices;
}

void BoQuickGroundRenderer::glueToTop(BoQuickGroundRenderer::TerrainChunk* c, BoQuickGroundRenderer::TerrainChunk* n)
{
  int minStep = 1 << c->useLOD;
  int maxStep = 1 << n->useLOD;

  unsigned int ledge = c->minX;
  unsigned int redge = c->maxX;
  if(c->neighbors[0] && c->neighbors[0]->useLOD > c->useLOD)
  {
    ledge += minStep;
  }
  if(c->neighbors[2] && c->neighbors[2]->useLOD > c->useLOD)
  {
    redge -= minStep;
  }

  unsigned int j0 = ledge;
  unsigned int j1 = ledge + minStep;

  unsigned int* indices = new unsigned int[mChunkSize*2 * 3];
  unsigned int indexcount = 0;

  // Go through all vertices on the side of the neighbor
  for(unsigned int i = n->minX; i < n->maxX; i += maxStep)
  {
    // Next vertex on neighbor's edge
    unsigned int nexti = QMIN(i + maxStep, c->maxX);

    // Pos of neighbor's current vertex in the vbo
    unsigned int ipos = n->maxRenderY * mMapCW + i;

    // Go through all vertices which lie between neighbor's current and next
    //  vertex.
    for(; j0 < nexti && j0 < redge; j1 += minStep)
    {
      // Make sure j1 is not over the edge
      j1 = QMIN(j1, redge);
      // Create next triangle
      indices[indexcount++] = ipos;
      indices[indexcount++] = c->minRenderY * mMapCW + j0;
      indices[indexcount++] = c->minRenderY * mMapCW + j1;
      j0 = j1;
    }

    // Add last triangle for this pair of vertices
    indices[indexcount++] = ipos;
    indices[indexcount++] = c->minRenderY * mMapCW + j0;
    indices[indexcount++] = n->maxRenderY * mMapCW + nexti;
  }

  glDrawElements(GL_TRIANGLES, indexcount, GL_UNSIGNED_INT, indices);
  //renderedQuads += indexcount / 3;

  delete[] indices;
}

void BoQuickGroundRenderer::glueToRight(BoQuickGroundRenderer::TerrainChunk* c, BoQuickGroundRenderer::TerrainChunk* n)
{
  int minStep = 1 << c->useLOD;
  int maxStep = 1 << n->useLOD;

  unsigned int tedge = c->minY;
  unsigned int bedge = c->maxY;
  if(c->neighbors[1] && c->neighbors[1]->useLOD > c->useLOD)
  {
    tedge += minStep;
  }
  if(c->neighbors[3] && c->neighbors[3]->useLOD > c->useLOD)
  {
    bedge -= minStep;
  }

  unsigned int j0 = tedge;
  unsigned int j1 = tedge + minStep;

  unsigned int* indices = new unsigned int[mChunkSize*2 * 3];
  unsigned int indexcount = 0;

  // Go through all vertices on the side of the neighbor
  for(unsigned int i = n->minY; i < n->maxY; i += maxStep)
  {
    // Next vertex on neighbor's edge
    unsigned int nexti = QMIN(i + maxStep, c->maxY);

    // Pos of neighbor's current vertex in the vbo
    unsigned int ipos = i * mMapCW + n->minRenderX;

    // Go through all vertices which lie between neighbor's current and next
    //  vertex.
    for(; j0 < nexti && j0 < bedge; j1 += minStep)
    {
      j1 = QMIN(j1, bedge);
      indices[indexcount++] = ipos;
      indices[indexcount++] = j0 * mMapCW + c->maxRenderX;
      indices[indexcount++] = j1 * mMapCW + c->maxRenderX;
      j0 = j1;
    }

    // Add last triangle for this pair of vertices
    indices[indexcount++] = ipos;
    indices[indexcount++] = j0 * mMapCW + c->maxRenderX;
    indices[indexcount++] = nexti * mMapCW + n->minRenderX;
  }

  glDrawElements(GL_TRIANGLES, indexcount, GL_UNSIGNED_INT, indices);
  //renderedQuads += indexcount / 3;

  delete[] indices;
}

void BoQuickGroundRenderer::glueToBottom(BoQuickGroundRenderer::TerrainChunk* c, BoQuickGroundRenderer::TerrainChunk* n)
{
  int minStep = 1 << c->useLOD;
  int maxStep = 1 << n->useLOD;

  unsigned int ledge = c->minX;
  unsigned int redge = c->maxX;
  if(c->neighbors[0] && c->neighbors[0]->useLOD > c->useLOD)
  {
    ledge += minStep;
  }
  if(c->neighbors[2] && c->neighbors[2]->useLOD > c->useLOD)
  {
    redge -= minStep;
  }

  unsigned int j0 = ledge;
  unsigned int j1 = ledge + minStep;

  unsigned int* indices = new unsigned int[mChunkSize*2 * 3];
  unsigned int indexcount = 0;

  // Go through all vertices on the side of the neighbor
  for(unsigned int i = n->minX; i < n->maxX; i += maxStep)
  {
    // Next vertex on neighbor's edge
    unsigned int nexti = QMIN(i + maxStep, c->maxX);

    // Pos of neighbor's current vertex in the vbo
    unsigned int ipos = n->minRenderY * mMapCW + i;

    // Go through all vertices which lie between neighbor's current and next
    //  vertex.
    for(; j0 < nexti && j0 < redge; j1 += minStep)
    {
      j1 = QMIN(j1, redge);
      indices[indexcount++] = c->maxRenderY * mMapCW + j1;
      indices[indexcount++] = c->maxRenderY * mMapCW + j0;
      indices[indexcount++] = ipos;
      j0 = j1;
    }

    // Add last triangle for this pair of vertices
    indices[indexcount++] = ipos;
    indices[indexcount++] = n->minRenderY * mMapCW + nexti;
    indices[indexcount++] = c->maxRenderY * mMapCW + j0;
  }

  glDrawElements(GL_TRIANGLES, indexcount, GL_UNSIGNED_INT, indices);
  //renderedQuads += indexcount / 3;

  delete[] indices;
}

void BoQuickGroundRenderer::initMap(const BosonMap* map)
{
  mMap = map;

  mColorMapRenderers.setAutoDelete(true);
  mColorMapRenderers.clear();

  // Cache map size
  mMapW = map->width();
  mMapH = map->height();
  mMapCW = mMapW + 1;
  mMapCH = mMapH + 1;

  // Create, bind and map VBOs
  // Vertex VBO
  glGenBuffers(1, &mVBOVertex);
  glBindBuffer(GL_ARRAY_BUFFER, mVBOVertex);
  glBufferData(GL_ARRAY_BUFFER, mMapCW * mMapCH * sizeof(BoVector3Float), 0, GL_STATIC_DRAW);
  BoVector3Float* vertices = (BoVector3Float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
  // Normal VBO
  glGenBuffers(1, &mVBONormal);
  glBindBuffer(GL_ARRAY_BUFFER, mVBONormal);
  glBufferData(GL_ARRAY_BUFFER, mMapCW * mMapCH * sizeof(BoVector3Float), 0, GL_STATIC_DRAW);
  BoVector3Float* normals = (BoVector3Float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
  // Texture weights' VBO
  mTextureCount = map->groundTheme()->groundTypeCount();
  // Size of a single texture weights' layer (for a single texture) in bytes
  mVBOTextureLayerSize = mMapCW * mMapCH * 4;
  glGenBuffers(1, &mVBOTexture);
  glBindBuffer(GL_ARRAY_BUFFER, mVBOTexture);
  glBufferData(GL_ARRAY_BUFFER, mTextureCount * mVBOTextureLayerSize * sizeof(unsigned char), 0, GL_STATIC_DRAW);
  unsigned char* weights = (unsigned char*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

  // Get pointers to height and normal maps
  const float* heightmap = map->heightMap();
  const float* normalmap = map->normalMap();
  const unsigned char* texmap = map->texMap();  // Contains weights for _all_ textures

  // Fill the VBOs
  for(unsigned int y = 0; y < mMapCH; y++)
  {
    for(unsigned int x = 0; x < mMapCW; x++)
    {
      unsigned int i = (y * mMapCW) + x;
      int pos = map->cornerArrayPos(x, y);

      vertices[i] = BoVector3Float(x, -((float)y), heightmap[pos]);
      normals[i] = BoVector3Float(normalmap + 3 * pos);
      for(unsigned int t = 0; t < mTextureCount; t++)
      {
        weights[mVBOTextureLayerSize*t + i*4 + 0] = 255;
        weights[mVBOTextureLayerSize*t + i*4 + 1] = 255;
        weights[mVBOTextureLayerSize*t + i*4 + 2] = 255;
        weights[mVBOTextureLayerSize*t + i*4 + 3] = texmap[(t * mMapCW * mMapCH) + pos];
      }
    }
  }

  // Unmap VBOs
  glBindBuffer(GL_ARRAY_BUFFER, mVBOVertex);
  if(!glUnmapBuffer(GL_ARRAY_BUFFER))
  {
    boError() << k_funcinfo << "can't unmap vertices' vbo!" << endl;
  }
  glBindBuffer(GL_ARRAY_BUFFER, mVBONormal);
  if(!glUnmapBuffer(GL_ARRAY_BUFFER))
  {
    boError() << k_funcinfo << "can't unmap vertices' vbo!" << endl;
  }
  glBindBuffer(GL_ARRAY_BUFFER, mVBOTexture);
  if(!glUnmapBuffer(GL_ARRAY_BUFFER))
  {
    boError() << k_funcinfo << "can't unmap texture weights' vbo!" << endl;
  }


  // Divide the terrain into chunks
  mChunkSize = 32;
  mChunkCount = (int)(ceilf(mMapW / (float)mChunkSize) * ceilf(mMapH / (float)mChunkSize));
  mChunks = new TerrainChunk[mChunkCount];


  // Initialize chunks
  unsigned int chunki = 0;
  for(unsigned int chunky = 0; chunky < mMapH; chunky += mChunkSize)
  {
    unsigned int chunkh = QMIN(mChunkSize, mMapH - chunky);
    for(unsigned int chunkx = 0; chunkx < mMapW; chunkx += mChunkSize)
    {
      unsigned int chunkw = QMIN(mChunkSize, mMapW - chunkx);

      TerrainChunk* chunk = &mChunks[chunki++];

      // Calculate and store min/max coordinates of the chunk
      chunk->minX = chunkx;
      chunk->minY = chunky;
      chunk->maxX = chunkx + chunkw;
      chunk->maxY = chunky + chunkh;

      // Init unexplored and hastexture flags
      chunk->unexplored = true;
      chunk->hastexture = new bool[mTextureCount];
      for(unsigned int t = 0; t < mTextureCount; t++)
      {
        chunk->hastexture[t] = false;
      }

      // Init neighbors
      chunk->neighbors[0] = (chunkx >= mChunkSize) ? chunkAt(chunkx - mChunkSize, chunky) : 0;
      chunk->neighbors[1] = (chunky >= mChunkSize) ? chunkAt(chunkx, chunky - mChunkSize) : 0;
      chunk->neighbors[2] = ((chunkx + mChunkSize) < mMapW) ? chunkAt(chunkx + mChunkSize, chunky) : 0;
      chunk->neighbors[3] = ((chunky + mChunkSize) < mMapH) ? chunkAt(chunkx, chunky + mChunkSize) : 0;

      // Pass 1
      // Inits unexplored flag
      for(unsigned int y = chunk->minY; y < chunk->maxY; y++)
      {
        for(unsigned int x = chunk->minX; x < chunk->maxX; x++)
        {
          if(localPlayerIO()->isExplored(x, y))
          {
            chunk->unexplored = false;
            break;
          }
        }
        if(!chunk->unexplored)
        {
          break;
        }
      }

      // Pass 2
      // Finds min/max height of the chunk
      // Inits hastexture flags
      chunk->minZ = chunk->maxZ = heightmap[map->cornerArrayPos(chunk->minX, chunk->minY)];
      for(unsigned int y = chunk->minY; y <= chunk->maxY; y++)
      {
        for(unsigned int x = chunk->minX; x <= chunk->maxX; x++)
        {
          int pos = map->cornerArrayPos(x, y);
          chunk->minZ = QMIN(chunk->minZ, heightmap[pos]);
          chunk->maxZ = QMAX(chunk->maxZ, heightmap[pos]);
          for(unsigned int t = 0; t < mTextureCount; t++)
          {
            if(texmap[(t * mMapCW * mMapCH) + pos] > 0)
            {
              chunk->hastexture[t] = true;
            }
          }
        }
      }

      // Pass 3
      // Finds roughness of the chunk
      BoGroundRendererBase::getRoughnessInRect(map, &chunk->roughness, &chunk->textureRoughnessTotal, chunk->minX, chunk->minY, chunk->maxX, chunk->maxY);

      // Finally find center and radius of the chunk
      float halfdiffX = (chunk->maxX - chunk->minX) / 2.0f;
      float halfdiffY = (chunk->maxY - chunk->minY) / 2.0f;
      float halfdiffZ = (chunk->maxZ - chunk->minZ) / 2.0f;
      chunk->center.setX(chunk->minX + halfdiffX);
      chunk->center.setY(-(chunk->minY + halfdiffY));
      chunk->center.setZ(chunk->minZ + halfdiffZ);
      chunk->radius = sqrt(halfdiffX*halfdiffX + halfdiffY*halfdiffY + halfdiffZ*halfdiffZ);
    }
  }

  glBindBuffer(GL_ARRAY_BUFFER, 0);  // Disable VBO
}

BoQuickGroundRenderer::TerrainChunk* BoQuickGroundRenderer::chunkAt(int x, int y) const
{
  int cx = x / mChunkSize;
  int cy = y / mChunkSize;
  int cw = (int)ceilf(mMap->width() / (float)mChunkSize);
  return &mChunks[cy * cw + cx];
}

unsigned int BoQuickGroundRenderer::chooseLOD(BoQuickGroundRenderer::TerrainChunk* chunk, float dist)
{
  // Good to know:
  // * roughness of an average chunk should be about 2-3
  // * tex roughness of an average chunk should be about 1-1.5
  // So the total roughness of an average chunk might be aruond 4

  dist -= chunk->radius;
  dist = QMAX(dist, 1.0f);

  float e = (chunk->roughness + chunk->textureRoughnessTotal) * ROUGHNESS_MULTIPLIER / dist;

  if(e < 0.5)
  {
    return 5;
  }
  else if(e < 1.25)
  {
    return 4;
  }
  else if(e < 3)
  {
    return 3;
  }
  else if(e < 7)
  {
    return 2;
  }
  else if(e < 16)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

void BoQuickGroundRenderer::cellFogChanged(int x1, int y1, int x2, int y2)
{
  mFogTexture->setLocalPlayerIO(localPlayerIO());
  mFogTexture->cellChanged(x1, y1, x2, y2);
}

void BoQuickGroundRenderer::cellExploredChanged(int x1, int y1, int x2, int y2)
{
  // TODO: don't go over every single cell!
  for(int y = y1; y <= y2; y++)
  {
    for(int x = x1; x <= x2; x++)
    {
      if(localPlayerIO()->isExplored(x, y))
      {
        if(chunkAt(x, y)->unexplored)
        {
          chunkAt(x, y)->unexplored = false;
          // When a chunk becomes visible, the list of visible chunks has to be
          //  rebuilt
          mCellListDirty = true;
        }
      }
    }
  }
  mFogTexture->setLocalPlayerIO(localPlayerIO());
  mFogTexture->cellChanged(x1, y1, x2, y2);
}

void BoQuickGroundRenderer::cellHeightChanged(int x1, int y1, int x2, int y2)
{
  // Normals have to be recalculated for adjacent corners as well
  x1 = QMAX(x1 - 1, 0);
  y1 = QMAX(y1 - 1, 0);
  x2 = QMIN(x2 + 1, (int)mMapW);
  y2 = QMIN(y2 + 1, (int)mMapH);

  const float* heightmap = mMap->heightMap();
  const float* normalmap = mMap->normalMap();

  glBindBuffer(GL_ARRAY_BUFFER, mVBOVertex);
  BoVector3Float* vertices = (BoVector3Float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
  glBindBuffer(GL_ARRAY_BUFFER, mVBONormal);
  BoVector3Float* normals = (BoVector3Float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

  for(int y = y1; y <= y2; y++)
  {
    for(int x = x1; x <= x2; x++)
    {
      unsigned int i = (y * mMapCW) + x;
      int pos = mMap->cornerArrayPos(x, y);
      vertices[i].setZ(heightmap[pos]);
      normals[i] = BoVector3Float(normalmap + 3 * pos);
    }
  }

  // Unmap VBOs
  glBindBuffer(GL_ARRAY_BUFFER, mVBOVertex);
  if(!glUnmapBuffer(GL_ARRAY_BUFFER))
  {
    boError() << k_funcinfo << "can't unmap vertices' vbo!" << endl;
  }
  glBindBuffer(GL_ARRAY_BUFFER, mVBONormal);
  if(!glUnmapBuffer(GL_ARRAY_BUFFER))
  {
    boError() << k_funcinfo << "can't unmap vertices' vbo!" << endl;
  }
}

void BoQuickGroundRenderer::cellTextureChanged(int x1, int y1, int x2, int y2)
{
  const unsigned char* texmap = mMap->texMap();  // Contains weights for _all_ textures

  glBindBuffer(GL_ARRAY_BUFFER, mVBOTexture);
  unsigned char* weights = (unsigned char*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

  for(int y = y1; y <= y2; y++)
  {
    for(int x = x1; x <= x2; x++)
    {
      unsigned int i = (y * mMapCW) + x;
      int pos = mMap->cornerArrayPos(x, y);

      for(unsigned int t = 0; t < mTextureCount; t++)
      {
        weights[mVBOTextureLayerSize*t + i*4 + 0] = 255;
        weights[mVBOTextureLayerSize*t + i*4 + 1] = 255;
        weights[mVBOTextureLayerSize*t + i*4 + 2] = 255;
        weights[mVBOTextureLayerSize*t + i*4 + 3] = texmap[(t * mMapCW * mMapCH) + pos];
      }
    }
  }

  // Unmap VBOs
  glBindBuffer(GL_ARRAY_BUFFER, mVBOTexture);
  if(!glUnmapBuffer(GL_ARRAY_BUFFER))
  {
    boError() << k_funcinfo << "can't unmap texture weights' vbo!" << endl;
  }
}

void BoQuickGroundRenderer::renderVisibleCellsStart(const BosonMap* map)
{
  mDrawGrid = boConfig->boolValue("debug_cell_grid");

  mFogTexture->setLocalPlayerIO(localPlayerIO());
  mFogTexture->start(map);
}

void BoQuickGroundRenderer::renderVisibleCellsStop(const BosonMap* map)
{
  mFogTexture->stop(map);
}

// TODO: use BoGroundRendererBase
#warning FIXME: duplicated code
BoColorMapRenderer* BoQuickGroundRenderer::getUpdatedColorMapRenderer(BoColorMap* map)
{
 BoColorMapRenderer* r = mColorMapRenderers[map];
 if(r)
 {
   r->update();
   return r;
 }
 r = new BoColorMapRenderer(map);
 mColorMapRenderers.insert(map, r);
 return r;
}

/*
 * vim: et sw=2
 */
