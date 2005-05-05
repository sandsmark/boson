/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bowater.h"

#include "bosonmap.h"
#include "bodebug.h"
#include "info/boinfo.h"
#include "botexture.h"
#include "bo3dtools.h"
#include "bolight.h"
#include "bosonconfig.h"
#include "playerio.h"
#include "boshader.h"
#include "bosonprofiling.h"

#include <qrect.h>
#include <qpoint.h>
#include <qptrlist.h>
#include <qvaluelist.h>
#include <qstringlist.h>
#include <qdom.h>
#include <qdir.h>

#include <kglobal.h>
#include <kstandarddirs.h>

#include <math.h>

#include <stdio.h>
#include <sys/time.h>

#define CHUNK_SIZE 10



/*****  BoLake  *****/

BoLake::BoLake(BoWaterManager* man, float _level)
{
  init(man, _level);
}

BoLake::~BoLake()
{
  chunks.clear();
  delete[] corners;
}

void BoLake::init(BoWaterManager* _manager, float _level)
{
  manager = _manager;
  level = _level;
  minx = 0; miny = 0;
  maxx = 0; maxy = 0;
  loadedminx = 0; loadedminy = 0; loadedmaxx = 0; loadedmaxy = 0;
  originx = -1; originy = -1;
  cornercount = 0;
  corners = 0;
  waveVector.set(0.866, 0.5);
  textureMatrix.rotate(30, 0, 0, 1);
  alphaMultiplier = 0.8;
  alphaBase = 0.0;
  chunks.setAutoDelete(true);
}

void BoLake::findWater(int x, int y, const QRect& searcharea)
{
  const int xoffsets[] = {  0,  1,  1,  1,  0, -1, -1, -1};
  const int yoffsets[] = { -1, -1,  0,  1,  1,  1,  0, -1};

  originx = x;
  originy = y;
  loadedminx = searcharea.left();
  loadedminy = searcharea.top();
  loadedmaxx = searcharea.right();
  loadedmaxy = searcharea.bottom();

  minx = 10000; miny = 10000;
  maxx = -1; maxy = -1;
  cornercount = 0;

  QValueList<QPoint> open;
  QValueList<QPoint> closed;
  QPoint n, n2;

  open.push_back(QPoint(x, y));
  manager->setUnderwater(x, y, true);

  while(!open.empty())
  {
    n = open.back();
    open.pop_back();
    closed.push_back(n);
    cornercount++;

    minx = QMIN(minx, n.x());
    maxx = QMAX(maxx, n.x());
    miny = QMIN(miny, n.y());
    maxy = QMAX(maxy, n.y());

    for(int i = 0; i < 8; i++)
    {
      n2 = n;
      n2 += QPoint(xoffsets[i], yoffsets[i]);

      if(!searcharea.contains(n2))
      {
        continue;
      }

      if(manager->groundHeight(n2.x(), n2.y()) >= level)
      {
        continue;
      }

      if(!manager->underwater(n2.x(), n2.y()))
      {
        // This cell is free. Take it
        manager->setUnderwater(n2.x(), n2.y(), true);
        open.push_back(n2);
      }
    }
  }
  // We need 1-cell border for shores
  minx = QMAX(minx - 1, searcharea.left());
  maxx = QMIN(maxx + 1, searcharea.right());
  miny = QMAX(miny - 1, searcharea.top());
  maxy = QMIN(maxy + 1, searcharea.bottom());
  // Find center point of the lake
  center.set((minx + maxx) / 2.0f, -(miny + maxy) / 2.0f, level);
  // Find radius
  float radiusx = (maxx - minx) / 2.0f;
  float radiusy = (maxy - miny) / 2.0f;
  radius = sqrt(radiusx * radiusx + radiusy * radiusy);

  // Mark all corners that we have
  int size = (maxx - minx + 1) * (maxy - miny + 1);
  corners = new bool[size];
  // TODO: can we use memset() here?
  for(int i = 0; i < size; i++)
  {
    corners[i] = false;
  }
  // Go through every corners and add this _and it's neighbors_. We need
  //  neighbors for shores.
  for(QValueList<QPoint>::iterator it = closed.begin(); it != closed.end(); it++)
  {
    QPoint p = *it;
    for(int i = 0; i < 8; i++)
    {
      QPoint p2 = p + QPoint(xoffsets[i], yoffsets[i]);
      if(p2.x() < minx || p2.x() > maxx || p2.y() < miny || p2.y() > maxy)
      {
        //boDebug() << k_funcinfo << "corners (" << p2.x() << "; " << p2.y() << ") out of bounds!" << endl;
        continue;
      }
      corners[(p2.y() - miny) * (maxx - minx + 1) + (p2.x() - minx)] = true;
    }
    corners[(p.y() - miny) * (maxx - minx + 1) + (p.x() - minx)] = true;
  }

  // Divide lake into chunks
  for(int cy = miny; cy < maxy; cy += CHUNK_SIZE)
  {
    for(int cx = minx; cx < maxx; cx += CHUNK_SIZE)
    {
      WaterChunk* chunk = new WaterChunk;
      chunk->minx = cx;
      chunk->miny = cy;
      chunk->maxx = QMIN(maxx, cx + CHUNK_SIZE);
      chunk->maxy = QMIN(maxy, cy + CHUNK_SIZE);
      chunk->center = BoVector3Float((chunk->minx + chunk->maxx) / 2.0f, -(chunk->miny + chunk->maxy) / 2.0f, level);
      chunk->lastdetail = -1.0f;
//      boDebug() << "        " << k_funcinfo << "Create chunk, coords: ("
//          << chunk->minx << "; " << chunk->miny << ")-(" << chunk->maxx << "; " << chunk->maxy << ")" << endl;
      chunks.append(chunk);
    }
  }

  // Find min/max ground heights, real sizes and number of corners for chunks
  QPtrList<WaterChunk> invalidchunks;
  for(QPtrListIterator<WaterChunk> it(chunks); it.current(); ++it)
  {
    WaterChunk* chunk = it.current();
    chunk->mingroundheight = 1000000;
    chunk->maxgroundheight = -1000000;
    // "Real size" of the chunk, i.e. not including non-water corners.
    int cminx = 1000000;
    int cminy = 1000000;
    int cmaxx = -1000000;
    int cmaxy = -1000000;
    chunk->corners = 0;
    for(int y = chunk->miny; y <= chunk->maxy; y++)
    {
      for(int x = chunk->minx; x <= chunk->maxx; x++)
      {
        if(hasCorner(x, y))
        {
          float h = manager->groundHeightAt(x, y);
          chunk->mingroundheight = QMIN(chunk->mingroundheight, h);
          chunk->maxgroundheight = QMAX(chunk->maxgroundheight, h);

          cminx = QMIN(cminx, x);
          cminy = QMIN(cminy, y);
          cmaxx = QMAX(cmaxx, x);
          cmaxy = QMAX(cmaxy, y);
          chunk->corners++;
        }
      }
    }
    chunk->minx = cminx;
    chunk->miny = cminy;
    chunk->maxx = cmaxx;
    chunk->maxy = cmaxy;
    if(chunk->corners == 0)
    {
      // Chunk has no valid corners. Mark it for removal.
      invalidchunks.append(chunk);
    }
    else if(chunk->corners < 4)
    {
      // Less than 4 corners isn't valid either because you need at least 4
      //  corners to render a cell.
      boDebug() << k_funcinfo << "Removing chunk with " << chunk->corners << " corners" << endl;
      invalidchunks.append(chunk);
    }
    //boDebug() << "        " << k_funcinfo << "Chunk (" << chunk->minx << "; " << chunk->miny <<
    //    ")-(" << chunk->maxx << "; " << chunk->maxy << ") has " << chunk->corners << " corners" << endl;
  }
  // Delete chunks with no valid corners.
  while(invalidchunks.count() > 0)
  {
    WaterChunk* chunk = invalidchunks.first();
    invalidchunks.removeRef(chunk);
    chunks.removeRef(chunk);
  }

//  boDebug() << k_funcinfo << "findWater(" << x << ", " << y << ", (" << searcharea.left() <<
//      "; " << searcharea.top() << ")-(" << searcharea.right() << "; " << searcharea.bottom() <<
//      ")) found " << cornercount << " corners on " << size << "-corner area (" << chunks.count() <<
//      " chunks)" << endl;
  /*printf("Lake layout (%d, %d)-(%d, %d):\n", minx, miny, maxx, maxy);
  for(int myy = miny; myy <= maxy; myy++)
  {
    printf("        ");
    for(int myx = minx; myx <= maxx; myx++)
    {
      printf("%c", hasCorner(myx, myy) ? 'X' : '.');
    }
    printf("\n");
  }*/
}

bool BoLake::hasAnyCorner(float x1, float y1, float x2, float y2)
{
  if(hasCorner(QMAX((int)x1, minx), QMAX((int)y1, miny)))
  {
    return true;
  }

  for(int celly = QMAX((int)y1, miny); celly <= QMIN((int)y2, maxy); celly++)
  {
    for(int cellx = QMAX((int)x1, minx); cellx <= QMIN((int)x2, maxx); cellx++)
    {
      if(hasCorner(cellx, celly))
      {
        return true;
      }
    }
  }
  return false;
}



/***** BoWaterManager  *****/

BoWaterManager* BoWaterManager::mManager = 0;

BoWaterManager* BoWaterManager::waterManager()
{
  if(!mManager)
  {
#warning TODO: use static deleter
    mManager = new BoWaterManager();
  }
  return mManager;
}

BoWaterManager::BoWaterManager()
{
  if(mManager)
  {
    boError() << k_funcinfo << "You shouldn't have more than 1 BoWaterManager object!" << endl;
  }
  mMap = 0;
  mUnderwater = 0;
  mCellPassable = 0;
  mCellVisible = 0;
  mTime = 0;
  mLakes.setAutoDelete(true);
  mDirty = true;
  mOpenGLInited = false;
  mViewFrustum = 0;
  mSun = 0;
  mWaterTex = 0;
  mWaterBump = 0;
  mEnvMap = 0;
  mWaterAnimBump.setAutoDelete(true);
  mWaterAnimBumpCurrent = 0.0f;
  mShader = 0;
}

BoWaterManager::~BoWaterManager()
{
  mLakes.clear();
  delete[] mUnderwater;
  delete[] mCellPassable;
  delete[] mCellVisible;
  delete mWaterTex;
  delete mWaterBump;
  delete mEnvMap;
  delete mShader;
  mWaterAnimBump.clear();
}

bool BoWaterManager::loadFromXML(const QDomElement& root)
{
  // Init some data structures
  delete[] mUnderwater;
  mUnderwater = new bool[mWidth * mHeight];
  for (int y = 0; y < mHeight; y++)
  {
    for (int x = 0; x < mWidth; x++)
    {
      mUnderwater[y * mWidth + x] = false;
    }
  }

  bool ret = true;
  QDomNodeList list = root.elementsByTagName(QString::fromLatin1("Lake"));

  for (unsigned int i = 0; i < list.count(); i++)
  {
    QDomElement lake = list.item(i).toElement();
    bool ok = false;

    // Load min/max coordinates
    int minx, miny, maxx, maxy;
    minx = lake.attribute("MinX").toInt(&ok);
    if(!ok)
    {
      boError() << k_funcinfo << "Error loading MinX attribute ('" << root.attribute("MinX") << "')" << endl;
      ret = false;
      continue;
    }
    miny = lake.attribute("MinY").toInt(&ok);
    if(!ok)
    {
      boError() << k_funcinfo << "Error loading MinY attribute ('" << root.attribute("MinY") << "')" << endl;
      ret = false;
      continue;
    }
    maxx = lake.attribute("MaxX").toInt(&ok);
    if(!ok)
    {
      boError() << k_funcinfo << "Error loading MaxX attribute ('" << root.attribute("MaxX") << "')" << endl;
      ret = false;
      continue;
    }
    maxy = lake.attribute("MaxY").toInt(&ok);
    if(!ok)
    {
      boError() << k_funcinfo << "Error loading MaxY attribute ('" << root.attribute("MaxY") << "')" << endl;
      ret = false;
      continue;
    }

    // Load origin point
    int originx, originy;
    originx = lake.attribute("OriginX").toInt(&ok);
    if(!ok)
    {
      boError() << k_funcinfo << "Error loading OriginX attribute ('" << root.attribute("OriginX") << "')" << endl;
      ret = false;
      continue;
    }
    originy = lake.attribute("OriginY").toInt(&ok);
    if(!ok)
    {
      boError() << k_funcinfo << "Error loading OriginY attribute ('" << root.attribute("OriginY") << "')" << endl;
      ret = false;
      continue;
    }
    if(underwater(originx, originy))
    {
      boWarning() << k_funcinfo << "Origin point (" << originx << "; " << originy << ") already under water!" << endl;
    }

    // Load level
    float level;
    level = lake.attribute("Level").toFloat(&ok);
    if(!ok)
    {
      boError() << k_funcinfo << "Error loading Level attribute ('" << root.attribute("Level") << "')" << endl;
      ret = false;
      continue;
    }

    BoLake* bolake = new BoLake(this, level);
    bolake->findWater(originx, originy, QRect(QPoint(minx, miny), QPoint(maxx, maxy)));
    mLakes.append(bolake);
  }

  initCellMaps();

  return ret;
}

bool BoWaterManager::saveToXML(QDomElement& root)
{
  QDomDocument doc = root.ownerDocument();
  QPtrListIterator<BoLake> it(mLakes);
  for(; it.current(); ++it)
  {
    BoLake* lake = it.current();
    QDomElement l = doc.createElement("Lake");
    root.appendChild(l);

    l.setAttribute("MinX", lake->loadedminx);
    l.setAttribute("MinY", lake->loadedminy);
    l.setAttribute("MaxX", lake->loadedmaxx);
    l.setAttribute("MaxY", lake->loadedmaxy);
    l.setAttribute("OriginX", lake->originx);
    l.setAttribute("OriginY", lake->originy);
    l.setAttribute("Level", lake->level);
  }

  return true;
}

void BoWaterManager::initCellMaps()
{
  // Size of map in cells
  mCellWidth = mWidth - 1;
  mCellHeight = mHeight - 1;
  delete[] mCellPassable;
  delete[] mCellVisible;
  mCellPassable = new bool[mCellWidth * mCellHeight];
  mCellVisible = new bool[mCellWidth * mCellHeight];

  // Set both arrays to true first
  for (int y = 0; y < mCellHeight; y++)
  {
    for (int x = 0; x < mCellWidth; x++)
    {
      mCellPassable[y * mCellWidth + x] = true;
      mCellVisible[y * mCellWidth + x] = true;
    }
  }

  // Some parameters
  const float max_passable_water_depth = 0.2;
  const float min_solid_water_depth = 2.0f;
  // Go through all lakes
  QPtrListIterator<BoLake> it(mLakes);
  for(; it.current(); ++it)
  {
    BoLake* lake = it.current();
    for (int y = lake->miny; y < lake->maxy; y++)
    {
      for (int x = lake->minx; x < lake->maxx; x++)
      {
        // We calculate params per-cell, so we need to look at all 4 corners
        float minwaterdepth = 1000000.0f;  // Used for visibility check
        float avgwaterdepth = 0.0f;  // Used for passability check
        int corners = 0;

        if(lake->hasCorner(x, y))
        {
          float waterdepth = lake->level - groundHeight(x, y);
          minwaterdepth = QMIN(minwaterdepth, waterdepth);
          avgwaterdepth += waterdepth;
          corners++;
        }
        if(lake->hasCorner(x + 1, y))
        {
          float waterdepth = lake->level - groundHeight(x + 1, y);
          minwaterdepth = QMIN(minwaterdepth, waterdepth);
          avgwaterdepth += waterdepth;
          corners++;
        }
        if(lake->hasCorner(x, y + 1))
        {
          float waterdepth = lake->level - groundHeight(x, y + 1);
          minwaterdepth = QMIN(minwaterdepth, waterdepth);
          avgwaterdepth += waterdepth;
          corners++;
        }
        if(lake->hasCorner(x + 1, y + 1))
        {
          float waterdepth = lake->level - groundHeight(x + 1, y + 1);
          minwaterdepth = QMIN(minwaterdepth, waterdepth);
          avgwaterdepth += waterdepth;
          corners++;
        }

        if(corners)
        {
          avgwaterdepth /= corners;
          if(avgwaterdepth > max_passable_water_depth)
          {
            mCellPassable[y * mCellWidth + x] = false;
          }
          if(corners == 4 && minwaterdepth > min_solid_water_depth)
          {
            mCellVisible[y * mCellWidth + x] = false;
          }
        }
      }
    }
  }
}

void BoWaterManager::initOpenGL()
{
  if(mOpenGLInited)
  {
    boDebug() << k_funcinfo << "OpenGL already inited, returning" << endl;
    return;
  }

  boDebug() << k_funcinfo << "Checking for OpenGL extensions..." << endl;
  QStringList extensions = BoInfo::boInfo()->openGLExtensions();

  // TODO: some of these are part of the core in later OpenGL versions.

  mSupports_texlod = extensions.contains("GL_EXT_texture_lod_bias");
  if(!mSupports_texlod)
  {
    boDebug() << k_funcinfo << "GL_EXT_texture_lod_bias not supported!" << endl;
  }

  mSupports_env_combine = extensions.contains("GL_ARB_texture_env_combine");
  if(!mSupports_env_combine)
  {
    boDebug() << k_funcinfo << "GL_ARB_texture_env_combine not supported!" << endl;
  }

  mSupports_env_dot3 = extensions.contains("GL_ARB_texture_env_dot3");
  if(!mSupports_env_dot3)
  {
    boDebug() << k_funcinfo << "GL_ARB_texture_env_dot3 not supported!" << endl;
  }

  mSupports_blendcolor = extensions.contains("GL_ARB_imaging");
  if(!mSupports_blendcolor)
  {
    mSupports_blendcolor_ext = extensions.contains("GL_EXT_blend_color");
    if(!mSupports_blendcolor_ext)
    {
      boDebug() << k_funcinfo << "GL_ARB_imaging and GL_EXT_blend_color not supported!" << endl;
    }
  }

  mSupports_vbo = extensions.contains("GL_ARB_vertex_buffer_object");
  if(!mSupports_vbo)
  {
    boDebug() << k_funcinfo << "GL_ARB_vertex_buffer_object not supported!" << endl;
  }

  mSupports_shaders = extensions.contains("GL_ARB_shader_objects") &&
      extensions.contains("GL_ARB_fragment_shader") && (boTextureManager->textureUnits() >= 3);
  if(!mSupports_shaders)
  {
    boDebug() << k_funcinfo << "Shaders not supported!" << endl;
  }

  boDebug() << k_funcinfo << "Extensions checking done" << endl;


  // Config settings.
  // TODO: move those to BoLake so that every lake can have independent
  //  settings.
  mReflectionSharpness = 1.5f;
  mReflectionStrength = 0.25f;
  mWaterAmbientColor = 0.8f;
  mWaterDiffuseColor = 0.8f;
  mWaterSpecularColor = 1.0f;
  mWaterShininess = 32.0f;

  // Load config.
  mEnableReflections = boConfig->boolValue("WaterReflections");
  mEnableBumpmapping = boConfig->boolValue("WaterBumpmapping");
  mEnableAnimBumpmaps = boConfig->boolValue("WaterAnimatedBumpmaps");
  mEnableTranslucency = boConfig->boolValue("WaterTranslucency");
  mEnableShader = boConfig->boolValue("WaterShaders");
  // TODO: settings for: VBO, specular


  // Check if loaded config is actually supported
  // Note that we can't use supports*() methods here because opengl stuff isn't
  //  fully inited yet.
  if(mEnableReflections && !((boTextureManager->textureUnits() > 1) && boTextureManager->supportsTextureCube() && mSupports_env_combine))
  {
    boWarning() << k_funcinfo << "Reflections are enabled, but not supported. Disabling." << endl;
    mEnableReflections = false;
    boConfig->setBoolValue("WaterReflections", false);
  }
  if(mEnableBumpmapping && !((boTextureManager->textureUnits() > 1) && mSupports_env_combine && mSupports_env_dot3 && (mSupports_blendcolor || mSupports_blendcolor_ext)))
  {
    boWarning() << k_funcinfo << "Bumpmapping is enabled, but not supported. Disabling." << endl;
    mEnableBumpmapping = false;
    boConfig->setBoolValue("WaterBumpmapping", false);
  }
  if(mEnableTranslucency && !((boTextureManager->textureUnits() > 1) && mSupports_env_combine))
  {
    boWarning() << k_funcinfo << "Translucency is enabled, but not supported. Disabling." << endl;
    mEnableTranslucency = false;
    boConfig->setBoolValue("WaterTranslucency", false);
  }
  if(mEnableShader && !mSupports_shaders)
  {
    boWarning() << k_funcinfo << "Shaders are enabled, but not supported. Disabling." << endl;
    mEnableShader = false;
    boConfig->setBoolValue("WaterShaders", false);
  }

  mEnableVBO = false;
  mEnableSpecular = true;


  setDirty(true);
  mOpenGLInited = true;
}

void BoWaterManager::reloadConfiguration()
{
  bool configDirty = false;
  bool texturesHaveChanged = false;

  if(boConfig->boolValue("WaterReflections") != mEnableReflections)
  {
    mEnableReflections = boConfig->boolValue("WaterReflections");
    // No need to set configDirty to true
    texturesHaveChanged = true;
  }
  if(boConfig->boolValue("WaterAnimatedBumpmaps") != mEnableAnimBumpmaps)
  {
    mEnableAnimBumpmaps = boConfig->boolValue("WaterAnimatedBumpmaps");
    // No need to set configDirty to true
    texturesHaveChanged = true;
  }
  if(boConfig->boolValue("WaterBumpmapping") != mEnableBumpmapping)
  {
    mEnableBumpmapping = boConfig->boolValue("WaterBumpmapping");
    configDirty = true;
    texturesHaveChanged = true;
  }
  if(boConfig->boolValue("WaterTranslucency") != mEnableTranslucency)
  {
    mEnableTranslucency = boConfig->boolValue("WaterTranslucency");
    configDirty = true;
  }
  if(boConfig->boolValue("WaterShaders") != mEnableShader)
  {
    mEnableShader = boConfig->boolValue("WaterShaders");
    configDirty = true;
    texturesHaveChanged = true;
  }

  // TODO: check if config is valid? (if everything's supported)

  if(configDirty)
  {
    setDirty(true);
    // We need to delete all data buffers in all chunks and set chunks' last
    //  detail level to -1.0, so that _needed_ data buffers will be reallocated
    //  next time we render them.
    QPtrListIterator<BoLake> it(mLakes);
    for(; it.current(); ++it)
    {
      QPtrListIterator<BoLake::WaterChunk> cit(it.current()->chunks);
      for(; cit.current(); ++cit)
      {
        BoLake::WaterChunk* chunk = cit.current();
        delete[] chunk->vertices;
        delete[] chunk->colors;
        delete[] chunk->indices;
        chunk->vertices = 0;
        chunk->colors = 0;
        chunk->indices = 0;

        chunk->lastdetail = -1.0f;
      }
    }
  }
  if(texturesHaveChanged)
  {
    loadNecessaryTextures();
  }
}

bool BoWaterManager::supportsShaders() const
{
  if(!mOpenGLInited)
  {
    return false;
  }

  return mSupports_shaders;
}

bool BoWaterManager::supportsReflections() const
{
  if(!mOpenGLInited)
  {
    return false;
  }

  return (boTextureManager->textureUnits() > 1) && boTextureManager->supportsTextureCube() && mSupports_env_combine;
}

bool BoWaterManager::supportsTranslucency() const
{
  if(!mOpenGLInited)
  {
    return false;
  }

  return (boTextureManager->textureUnits() > 1) && mSupports_env_combine;
}

bool BoWaterManager::supportsBumpmapping() const
{
  if(!mOpenGLInited)
  {
    return false;
  }

  return (boTextureManager->textureUnits() > 1) && mSupports_env_combine && mSupports_env_dot3 && (mSupports_blendcolor || mSupports_blendcolor_ext);
}

void BoWaterManager::setMap(BosonMap* map)
{
  mMap = map;
  mWidth = mMap->width() + 1;
  mHeight = mMap->height() + 1;
}

float BoWaterManager::groundHeight(int x, int y) const
{
  return mMap->heightAtCorner(x, y);
}

float BoWaterManager::groundHeightAt(float x, float y) const
{
  return groundHeight((int)x, (int)y);
}

float BoWaterManager::waterAlphaAt(BoLake* lake, float x, float y)
{
  return QMIN(1.0, ((lake->level - groundHeightAt(x, y)) * lake->alphaMultiplier + lake->alphaBase)/* * mWaterDiffuseColor*/);
}

float BoWaterManager::waterDepth(int x, int y)
{
  if(!underwater(x, y))
  {
    return 0.0f;
  }

  QPtrListIterator<BoLake> it(mLakes);
  for(; it.current(); ++it)
  {
    BoLake* lake = it.current();
    if(x >= lake->minx && x <= lake->maxx && y >= lake->miny && y <= lake->maxy)
    {
      if(lake->hasCorner(x, y))
      {
        return QMAX(0.0f, lake->level - groundHeight(x, y));
      }
    }
  }

  // This cell is not under water
  boWarning() << k_funcinfo << "Cell (" << x << "; " << y <<
      ") is not underwater, but underwater() returned true!" << endl;
  return 0.0f;
}

bool BoWaterManager::underwater(int x, int y)
{
  return mUnderwater[y * mWidth + x];
}

void BoWaterManager::setUnderwater(int x, int y, bool underwater)
{
  mUnderwater[y * mWidth + x] = underwater;
}

float BoWaterManager::time() const
{
  return 10.6379863536f + mTime;
}

void BoWaterManager::update(float elapsed)
{
  mTime += elapsed;
  mWaterAnimBumpCurrent += elapsed * 15;
  if(mWaterAnimBump.count() && ((unsigned int)mWaterAnimBumpCurrent >= mWaterAnimBump.count()))
  {
    mWaterAnimBumpCurrent -= mWaterAnimBump.count();
  }
}

void BoWaterManager::modelviewMatrixChanged(const BoMatrix& modelview)
{
  mModelview = modelview;
  BoMatrix affine = modelview;
  // We have z-direction upwards, most cubemaps are made for scenes where
  //  y-coordinate points upwards, so we rotate it a little.
  //affine.rotate(90, 1, 0, 0);

  affine.setElement(0, 3, 0.0f);
  affine.setElement(1, 3, 0.0f);
  affine.setElement(2, 3, 0.0f);
  affine.setElement(3, 0, 0.0f);
  affine.setElement(3, 1, 0.0f);
  affine.setElement(3, 2, 0.0f);
  affine.setElement(3, 3, 1.0f);
  if(!affine.invert(&mInverseModelview))
  {
    boError() << k_funcinfo << "Couldn't invert affine matrix!" << endl;
  }
}

float BoWaterManager::sphereInFrustum(const BoVector3Float& pos, float radius) const
{
 return Bo3dTools::sphereInFrustum(mViewFrustum, pos, radius);
}

QString BoWaterManager::currentRenderStatisticsData() const
{
  QString stat = QString("  Lakes rendered: %1\n  Chunks rendered: %2\n  Quads rendered: %3").arg(mRenderedLakes).arg(mRenderedChunks).arg(mRenderedQuads);
  return stat;
}

void BoWaterManager::setDirty(bool d)
{
  if(mDirty == d)
  {
    return;
  }

  mDirty = d;

  if(d == true)
  {
    // Set dirty flags of _all_ chunks to true.
    QPtrListIterator<BoLake> it(mLakes);
    for(; it.current(); ++it)
    {
      QPtrListIterator<BoLake::WaterChunk> cit(it.current()->chunks);
      for(; cit.current(); ++cit)
      {
        cit.current()->dirty = true;
      }
    }
  }
}

void BoWaterManager::render()
{
  if(!mOpenGLInited)
  {
    boWarning() << k_funcinfo << "OpenGL not inited! Initing now..." << endl;
    initOpenGL();
  }

  // Clear statistics
  mRenderedLakes = 0;
  mRenderedChunks = 0;
  mRenderedQuads = 0;


  // Textures (and other OpenGL stuff) will be inited when they will be used
  //  first. This indicates that they're not inited yet.
  mRenderEnvironmentSetUp = false;

  // Render all the lakes (in case they're visible).
  QPtrListIterator<BoLake> it(mLakes);
  while(it.current())
  {
    renderLake(it.current());
    ++it;
  }

  if(mRenderEnvironmentSetUp)
  {
    // Something was drawn and textures (and other OpenGL stuff was inited).
    // Pop attributes.
    if(mEnableShader)
    {
      mShader->unbind();
      // Pop envmap's texture matrix
      boTextureManager->activateTextureUnit(2);
      glPopMatrix();
      boTextureManager->activateTextureUnit(0);
    }
    if(mEnableReflections && !mEnableBumpmapping)
    {
      boTextureManager->activateTextureUnit(1);
      if(mSupports_texlod)
      {
        glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, 0.0f);
      }
      glPopMatrix();
      boTextureManager->activateTextureUnit(0);
    }
    glPopAttrib();
    boTextureManager->invalidateCache();
    glDisable(GL_BLEND);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  }

  Bo3dTools::checkError();

  setDirty(false);
}

void BoWaterManager::renderLake(BoLake* lake)
{
  // Check if lake is in frustum
  if(!sphereInFrustum(lake->center, lake->radius))
  {
    return;
  }
  // Box test is slower than sphere test, but often more accurate
  BoVector3Float lakemin(lake->minx, -lake->miny, lake->level);
  BoVector3Float lakemax(lake->maxx, -lake->maxy, lake->level);
  if(!Bo3dTools::boxInFrustum(mViewFrustum, lakemin, lakemax))
  {
    return;
  }

  // Lake is visible. Go through all it's chunks.
  QPtrListIterator<BoLake::WaterChunk> it(lake->chunks);
  for(; it.current(); ++it)
  {
    BoLake::WaterChunk* chunk = it.current();
    // Check if chunk is visible
    float distance = sphereInFrustum(chunk->center, CHUNK_SIZE / 2.0f * 1.414);  // 1.414 = sqrt(2)
    if(distance == 0.0f)
    {
      continue;
    }
    // Test with more accurate box-in-frustum test
    BoVector3Float chunkmin(chunk->minx, -chunk->miny, lake->level);
    BoVector3Float chunkmax(chunk->maxx, -chunk->maxy, lake->level);
    if(!Bo3dTools::boxInFrustum(mViewFrustum, chunkmin, chunkmax))
    {
      continue;
    }

    // LOD stuff. This isn't working yet.
    float water_detail = 1.0f;
    float chunk_detail = water_detail;
    bool dynamic_lod = false;
    if(dynamic_lod)
    {
      // Modify chunk's water detail
      if(distance > 50)
      {
        chunk_detail = water_detail * QMIN(((distance - 50) / 75 + 1), 4);
      }
    }
    renderChunk(lake, chunk, chunk_detail);
    mRenderedChunks++;
  }
  mRenderedLakes++;
}

// Macro for easily accessing data in arrays.
#define ARRAY_CORNER(array, x, y)  array[(y) * info->chunkcornerw + (x)]

void BoWaterManager::renderChunk(BoLake* lake, BoLake::WaterChunk* chunk, float chunkdetail)
{
  long int tm_initinfo, tm_initenv, tm_texmatrix, tm_miscinit, tm_dirty, tm_renderinit, tm_render, tm_uninit;
  BosonProfilingItem profiler;

  // Create new RenderInfo object
  RenderInfo* info = new RenderInfo;
  info->lake = lake;
  info->chunk = chunk;
  info->detail = chunkdetail;
  info->texrepeat = 10;
  tm_initinfo = profiler.elapsedSinceStart();

  // If nothing has been rendered yet, OpenGL stuff (e.g. textures) are
  //  uninited. Init them now.
  initRenderEnvironment();
  tm_initenv = profiler.elapsedSinceStart();

  // Set texture matrices. They make textures move slowly to create an illusion
  //  that water surface is moving.
  BoMatrix texMatrix = lake->textureMatrix;
  texMatrix.translate(0, mTime * 0.04, 0);
  glMatrixMode(GL_TEXTURE);
  boTextureManager->activateTextureUnit(0);  // Either diffuse or bump
  glPushMatrix();
  glLoadMatrixf(texMatrix.data());
  if(mEnableBumpmapping || mEnableShader)
  {
    boTextureManager->activateTextureUnit(1);  // Diffuse
    glPushMatrix();
    glLoadMatrixf(texMatrix.data());
  }
  tm_texmatrix = profiler.elapsedSinceStart();

  // First find out which rendering tehniques to use for the chunk:
  // Check whether water's alpha in chunk is variable or constant.
  info->constalpha = -1.0f;  // -1.0f means variable alpha
  if(mEnableTranslucency)
  {
    if(chunk->mingroundheight == chunk->maxgroundheight)
    {
      // Ground depth doesn't vary within chunk. Alpha is certainly const.
      info->constalpha = waterAlphaAt(lake, chunk->minx, chunk->miny);
    }
    else if(lake->level - chunk->maxgroundheight > 0.0f)
    {
      // Whole chunk is underwater. Maybe water is deep enough so that alpha
      //  is 1?
      float minalpha = (lake->level - chunk->maxgroundheight) * lake->alphaMultiplier + lake->alphaBase;
      if(minalpha >= 1.0f)
      {
        // Alpha is always at least 1.0f, i.e. constant
        info->constalpha = 1.0f;
      }
    }
  }
  else
  {
    // Translucency isn't used. Set alpha to 1.0f
    info->constalpha = 1.0f;
  }
  // If we're very lucky, we may be able to render whole chunk as a single quad
  info->singleQuad = false;
  if(info->constalpha != -1.0f && !mEnableReflections)
  {
    info->singleQuad = true;
    info->detail = 1000;
    chunkdetail = 1000;
  }


  // Chunk size in corners (depends on detail level) and doesn't include
  //  border.
  // This is for stuff which is calculated per-corner.
  info->chunkcornerw = (int)ceilf((chunk->maxx - chunk->minx) / chunkdetail + 1.0f);
  info->chunkcornerh = (int)ceilf((chunk->maxy - chunk->miny) / chunkdetail + 1.0f);

  if(mEnableBumpmapping && !mEnableShader)
  {
    // Light vector for bumpmapping. Sun is directional light, so we don't need
    //  to calculate this per-vertex.
    if(!mSun)
    {
      boError() << k_funcinfo << "NULL sun!" << endl;
    }
    info->lightvector = mSun->position3();
    info->lightvector.normalize();

    boTextureManager->activateTextureUnit(0);
    float lightv[] = { info->lightvector.x(), info->lightvector.y(), info->lightvector.z(), 1.0f };
    glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, lightv);
  }
  tm_miscinit = profiler.elapsedSinceStart();


  // Recalculate all the data if necessary
  if(chunk->dirty)
  {
    // Init dat buffers (vbos/arrays)
    initDataBuffersForStorage(info);

    // Calculate normal for each corner.
    //  Also fill in normal, tangentlight and halfvector vbos.
    calculatePerCornerStuff(info);

    calculateIndices(info);

    uninitDataBuffersForStorage(info);

    chunk->dirty = false;
  }
  tm_dirty = profiler.elapsedSinceStart();


  // Render all the cells in this chunk

  // Enable/disable blending and other opengl states
  if(mEnableShader)
  {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }
  else if(info->constalpha == 1.0f)
  {
    glDisable(GL_BLEND);
  }
  else if(mEnableBumpmapping)
  {
    if(mEnableTranslucency)
    {
      // FIXME: with this, N.L * diffusetex (diffuse color) will be in range
      //  [0; 1], while it actually needs to be in
      //  [0; mWaterDiffuseColor * light.diffuse()]
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
    {
      // This will clamp result to range [0; mWaterDiffuseColor * light.diffuse()]
      BoVector4Float diffusecolor = mSun ? mSun->diffuse() : BoVector4Float(1.0f, 1.0f, 1.0f, 1.0f);
      diffusecolor.scale(mWaterDiffuseColor);
      diffusecolor.setW(1.0f);
      glEnable(GL_BLEND);
      if(mSupports_blendcolor)
      {
        boglBlendColor(diffusecolor.x(), diffusecolor.y(), diffusecolor.z(), diffusecolor.w());
        glBlendFunc(GL_CONSTANT_COLOR, GL_ZERO);
      }
    }
  }
  else if(mEnableTranslucency)
  {
    // Translucency but no bumpmapping.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }

  // Init arrays/vbos and render
  // Push attributes
  glPushClientAttrib(GL_ALL_ATTRIB_BITS);
  // Vertices are always needed
  glEnableClientState(GL_VERTEX_ARRAY);
  if(mEnableVBO)
  {
    boglBindBuffer(GL_ARRAY_BUFFER, chunk->vbo_vertex);
    glVertexPointer(3, GL_FLOAT, 0, 0);
  }
  else
  {
    glVertexPointer(3, GL_FLOAT, 0, chunk->vertices);
  }
  // For translucency, we supply alpha in the primary color
  if(mEnableTranslucency && !mEnableShader)
  {
    // If translucency is enabled but bumpmapping is not, then we need to
    //  supply alpha.
    glEnableClientState(GL_COLOR_ARRAY);
    if(mEnableVBO)
    {
      boglBindBuffer(GL_ARRAY_BUFFER, chunk->vbo_color);
      glColorPointer(4, GL_FLOAT, 0, 0);
    }
    else
    {
      glColorPointer(4, GL_FLOAT, 0, chunk->colors);
    }
  }

  // We need to supply normal in case we use OpenGL lighting
  if(!mEnableBumpmapping && !mEnableShader)
  {
    glNormal3f(0.0f, 0.0f, 1.0f);
  }
  tm_renderinit = profiler.elapsedSinceStart();

  // Do the drawing
  if(mEnableVBO)
  {
    boglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->vbo_index);
    glDrawElements(GL_QUADS, chunk->indices_count, GL_UNSIGNED_INT, 0);
  }
  else
  {
    glDrawElements(GL_QUADS, chunk->indices_count, GL_UNSIGNED_INT, chunk->indices);
  }
  mRenderedQuads += chunk->indices_count / 4;
  glPopClientAttrib();
  tm_render = profiler.elapsedSinceStart();


  if(mEnableBumpmapping && !mEnableShader)
  {
    // Pass 2: ambient lighting and reflection pass.

    // Push attribute stack
    glPushAttrib(/*GL_COLOR_BUFFER_BIT | GL_LIGHTING_BIT | GL_TEXTURE_BIT | */GL_ALL_ATTRIB_BITS);

    // Lighting calculations

    // Amount of ambient light water will receive.
    BoVector4Float ambientcolor = mSun ? mSun->ambient() : BoVector4Float(1.0f, 1.0f, 1.0f, 1.0f);
    ambientcolor.scale(mWaterAmbientColor);
    ambientcolor.setW(1.0f);

    // Amount of reflections water will receive.
    BoVector4Float reflectioncolor;
    if(mEnableReflections)
    {
      reflectioncolor.set(mReflectionStrength, mReflectionStrength, mReflectionStrength, 1.0f);
    }

    // We want final color to be:
    //    diffusetex * ambientcolor + reflectiontex * reflectioncolor
    //  (note that both ambient and diffuse lighting use same texture)
    BoVector4Float resultcolor = ambientcolor;
    if(mEnableReflections)
    {
      // Setup envmap
      setupEnvMapTexture(1);
      // Use alpha from previous texture
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
      glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
      glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PREVIOUS);
      glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
      // If reflections are used, then we use:
      //    reflectiontex * reflectioncolor + diffusetex * primarycolor * (1 - reflectioncolor)
      //  (reflectioncolor is used for interpolating)
      //  We need  primarycolor * (1 - reflectioncolor)  to be equal to ambientcolor, so:
      //    primarycolor = ambientcolor / (1 - reflectioncolor)

      // TODO: actually sky will get darker with dim light, compensate for this
      //  (e.g. by multiplying reflection color with light's ambient (diffuse?)
      //  color).
      BoVector4Float reflectioncolor(mReflectionStrength, mReflectionStrength, mReflectionStrength, 1.0f);
      glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, reflectioncolor.data());

      resultcolor = ambientcolor;
      resultcolor.scale(1.0f - mReflectionStrength);
      resultcolor.setW(1.0f);
    }
    else
    {
      // Disable TU1
      boTextureManager->activateTextureUnit(1);
      boTextureManager->disableTexturing();
    }

    // Setup diffuse tex
    setupDiffuseTexture(0);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
    // Texture will be modulated by resultcolor.
    glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, resultcolor.data());
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_CONSTANT);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
    if(mEnableTranslucency)
    {
      // Set alpha to the one given in primary color
      glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
      glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
      glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
    }


    // Use additive blending.
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    // Disable depth buffer writes. This might save some time.
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);

    // FIXME!!! We need to send alpha for ambient lighting as well.

    glPushClientAttrib(GL_ALL_ATTRIB_BITS);
    // For translucency, we supply alpha in the primary color
    if(mEnableTranslucency)
    {
      // If translucency is enabled but bumpmapping is not, then we need to
      //  supply alpha.
      glEnableClientState(GL_COLOR_ARRAY);
      if(mEnableVBO)
      {
        boglBindBuffer(GL_ARRAY_BUFFER, chunk->vbo_color);
        glColorPointer(4, GL_FLOAT, 0, 0);
      }
      else
      {
        glColorPointer(4, GL_FLOAT, 0, chunk->colors);
      }
    }
    // Vertices
    glEnableClientState(GL_VERTEX_ARRAY);
    if(mEnableVBO)
    {
      boglBindBuffer(GL_ARRAY_BUFFER, chunk->vbo_vertex);
      glVertexPointer(3, GL_FLOAT, 0, 0);
    }
    else
    {
      glVertexPointer(3, GL_FLOAT, 0, chunk->vertices);
    }
    // We need normal for the automatic texgen
    // As the water is flat, we can supply the normal just once
    glNormal3f(0.0f, 0.0f, 1.0f);

    // Do the drawing
    if(mEnableVBO)
    {
      boglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->vbo_index);
      glDrawElements(GL_QUADS, chunk->indices_count, GL_UNSIGNED_INT, 0);
    }
    else
    {
      glDrawElements(GL_QUADS, chunk->indices_count, GL_UNSIGNED_INT, chunk->indices);
    }
    mRenderedQuads += chunk->indices_count / 4;
    glPopClientAttrib();

    if(mEnableReflections)
    {
      // Texture matrix was push()ed for envmap. Pop it now
      glMatrixMode(GL_TEXTURE);
      boTextureManager->activateTextureUnit(1);
      glPopMatrix();
    }

    // Enable depth writes and pop attributes.
    glDepthMask(GL_TRUE);
    glPopAttrib();
    boTextureManager->invalidateCache();
  }
  if(mEnableBumpmapping || mEnableShader)
  {
    // Diffuse texture's matrix was also pushed. Pop this one as well.
    glMatrixMode(GL_TEXTURE);
    boTextureManager->activateTextureUnit(1);
    glPopMatrix();
  }

  glMatrixMode(GL_TEXTURE);
  boTextureManager->activateTextureUnit(0);
  glPopMatrix();

  delete info;

  tm_uninit = profiler.elapsedSinceStart();

  /*boDebug() << k_funcinfo << "Took " << tm_uninit << "us IN TOTAL" << endl << "   " <<
      " IInfo: " << tm_initinfo <<
      " IEnv: " << tm_initenv - tm_initinfo <<
      " TexMatrix: " << tm_texmatrix - tm_initenv <<
      " IMisc: " << tm_miscinit - tm_texmatrix <<
      " Dirty: " << tm_dirty - tm_miscinit <<
      " RenderI: " << tm_renderinit - tm_dirty <<
      " Render: " << tm_render - tm_renderinit <<
      " Uninit: " << tm_uninit - tm_render << endl;*/
}

void BoWaterManager::calculatePerCornerStuff(RenderInfo* info)
{
  for(int x = 0; x < info->chunkcornerw; x++)
  {
    for(int y = 0; y < info->chunkcornerh; y++)
    {
      // Position of current corner, in cell coordinates
      float posx = QMIN(info->chunk->minx + x * info->detail, info->chunk->maxx);
      float posy = QMIN(info->chunk->miny + y * info->detail, info->chunk->maxy);

//      if(!info->lake->hasAnyCorner(posx, posy, posx + info->detail, posy + info->detail))
      // hasAnyCorner() does automatic limits checking, so no need to worry about
      //  this here.
      if(!info->lake->hasAnyCorner(posx - info->detail, posy - info->detail, posx + info->detail, posy + info->detail))
      {
        // Reset all variables, just in case...
        // TODO: find out if this is really needed or is it just waste of time
        continue;
      }

      ARRAY_CORNER(info->vertices_p, x, y) = BoVector3Float(posx, -posy, info->lake->level);

      if(mEnableTranslucency && !mEnableShader)
      {
        ARRAY_CORNER(info->colors_p, x, y) = BoVector4Float(1, 1, 1, waterAlphaAt(info->lake, posx, posy));
      }
    }
  }
}

void BoWaterManager::initDataBuffersForStorage(RenderInfo* info)
{
  // Corner arrays/vbos first
  if(mEnableVBO)
  {
    int corners = info->chunkcornerw * info->chunkcornerh;
    // Generate VBOs if it's not yet done
    if(!info->chunk->vbo_vertex)
    {
      boglGenBuffers(1, &info->chunk->vbo_vertex);
      boglGenBuffers(1, &info->chunk->vbo_color);
      boglGenBuffers(1, &info->chunk->vbo_index);
    }
#define WATER_VBO_MODE GL_DYNAMIC_DRAW
    boglBindBuffer(GL_ARRAY_BUFFER, info->chunk->vbo_vertex);
    boglBufferData(GL_ARRAY_BUFFER, corners * sizeof(BoVector3Float), 0, WATER_VBO_MODE);
    info->vertices_p = (BoVector3Float*)boglMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    if(!mEnableShader)
    {
      if(mEnableTranslucency)
      {
        boglBindBuffer(GL_ARRAY_BUFFER, info->chunk->vbo_color);
        boglBufferData(GL_ARRAY_BUFFER, corners * sizeof(BoVector4Float), 0, WATER_VBO_MODE);
        info->colors_p = (BoVector4Float*)boglMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
      }
    }
    // Buffer for indices
    boglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, info->chunk->vbo_index);
    boglBufferData(GL_ELEMENT_ARRAY_BUFFER, corners * 4 * sizeof(unsigned int), 0, WATER_VBO_MODE);
    info->indices_p = (unsigned int*)boglMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
  }
  else
  {
    // Use plain arrays. They're stored in water chunks
    if(info->chunk->lastdetail != info->detail)
    {
      // Reallocate arrays
      delete[] info->chunk->vertices;
      info->chunk->vertices = new BoVector3Float[info->chunkcornerw * info->chunkcornerh];
      if(!mEnableShader)
      {
        if(mEnableTranslucency)
        {
          delete[] info->chunk->colors;
          info->chunk->colors = new BoVector4Float[info->chunkcornerw * info->chunkcornerh];
        }
      }
      delete[] info->chunk->indices;
      info->chunk->indices = new unsigned int[(info->chunkcornerw - 1) * (info->chunkcornerh - 1) * 4];
      // Don't reset chunk->lastdetail yet, it will be done ~20 lines below
    }
    // Update pointers in RenderInfo. Note that some of those pointers may be
    //  invalid, but those shouldn't be used anyway.
    info->vertices_p = info->chunk->vertices;
    info->colors_p = info->chunk->colors;
    info->indices_p = info->chunk->indices;
  }

  // Now resize cell* arrays if necessary. These are plaint arrays even if vbos
  //  are enabled, beause they aren't used for rendering.
  if(info->chunk->lastdetail != info->detail)
  {
    info->chunk->lastdetail = info->detail;
  }

  // Reset indices count
  info->chunk->indices_count = 0;
}

void BoWaterManager::uninitDataBuffersForStorage(RenderInfo* info)
{
  // If VBOs are used, we need to unmap them
  if(mEnableVBO)
  {
    // Unmap vbos
    boglBindBuffer(GL_ARRAY_BUFFER, info->chunk->vbo_vertex);
    if(!boglUnmapBuffer(GL_ARRAY_BUFFER))
    {
      boError() << k_funcinfo << "can't unmap vertices' vbo!" << endl;
    }
    if(!mEnableShader)
    {
      if(mEnableTranslucency)
      {
        boglBindBuffer(GL_ARRAY_BUFFER, info->chunk->vbo_color);
        if(!boglUnmapBuffer(GL_ARRAY_BUFFER))
        {
          boError() << k_funcinfo << "can't unmap colors' vbo!" << endl;
        }
      }
    }
    boglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, info->chunk->vbo_index);
    if(!boglUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER))
    {
      boError() << k_funcinfo << "can't unmap indices' vbo!" << endl;
    }
    boglBindBuffer(GL_ARRAY_BUFFER, 0);  // Disable VBO
    boglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);  // Disable index VBO
  }
}

void BoWaterManager::calculateIndices(RenderInfo* info)
{
  int xi, yi;
  xi = 0;  // X Index
  for(float x = info->chunk->minx; x < info->chunk->maxx; x += info->detail, xi++)
  {
    yi = 0;  // Y Index
    for(float y = info->chunk->miny; y < info->chunk->maxy; y += info->detail, yi++)
    {
      if(xi >= (info->chunkcornerw - 1) || yi >= (info->chunkcornerh - 1))
      {
        boError() << k_funcinfo << "ERROR: invalid index coords: (" << xi << "; " << yi <<
            ")  (chunk size: " << info->chunkcornerw << "x" << info->chunkcornerh << " corners)" << endl;
        continue;
      }
      int posx = (int)x;
      int posy = (int)y;
      if(mLocalPlayerIO->isFogged(posx, posy))
      {
        continue;
      }
      int posx2 = QMIN((int)(x + info->detail), info->chunk->maxx);
      int posy2 = QMIN((int)(y + info->detail), info->chunk->maxy);
      // Check if lake has all corners of the cell
      /*if(!info->lake->hasCorner(posx, posy) ||
          !info->lake->hasCorner(posx2, posy) ||
          !info->lake->hasCorner(posx, posy2) ||
          !info->lake->hasCorner(posx2, posy2))
      {
        continue;
      }*/
      // If lake has any corners in this cell, we need to render it.
      if(!info->lake->hasAnyCorner(posx, posy, posx2, posy2))
      {
        continue;
      }

      // If it does, add indices for it
      info->indices_p[info->chunk->indices_count++] = yi * info->chunkcornerw + xi;
      info->indices_p[info->chunk->indices_count++] = yi * info->chunkcornerw + (xi + 1);
      info->indices_p[info->chunk->indices_count++] = (yi + 1) * info->chunkcornerw + (xi + 1);
      info->indices_p[info->chunk->indices_count++] = (yi + 1) * info->chunkcornerw + xi;
    }
  }
}

void BoWaterManager::setupEnvMapTexture(int unit)
{
  // Activate given texture unit
  boTextureManager->activateTextureUnit(unit);

  // Bind envmap texture
  mEnvMap->bind();

  // Load texture matrix
  glMatrixMode(GL_TEXTURE);
  glPushMatrix();
  glLoadMatrixf(mInverseModelview.data());

  // Automatically generate texture coords for reflection cubemap
  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
  glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
  glEnable(GL_TEXTURE_GEN_S);
  glEnable(GL_TEXTURE_GEN_T);
  glEnable(GL_TEXTURE_GEN_R);

  // Make texture a bit blurred if it's supported
  if(mSupports_texlod)
  {
    glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, mReflectionSharpness);
  }

  // Interpolate between water texture and envmap
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
  glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE);
  glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
  glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PREVIOUS);
  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
  glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_CONSTANT);
  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_COLOR);
  float reflectioncolor[] = { mReflectionStrength, mReflectionStrength, mReflectionStrength, 1.0f };
  glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, reflectioncolor);
}

void BoWaterManager::setupBumpMapTexture(int unit)
{
  boTextureManager->activateTextureUnit(unit);

  if(mEnableAnimBumpmaps)
  {
    mWaterAnimBump.at((int)mWaterAnimBumpCurrent)->bind();
  }
  else
  {
    mWaterBump->bind();
  }

  // Use automatic texcoord generation
  const float texPlaneS[] = { 0.1, 0.0, 0.0, 0.0 };
  const float texPlaneT[] = { 0.0, 0.1, 0.0, 0.0 };
  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
  glTexGenfv(GL_S, GL_OBJECT_PLANE, texPlaneS);
  glTexGenfv(GL_T, GL_OBJECT_PLANE, texPlaneT);
  glEnable(GL_TEXTURE_GEN_S);
  glEnable(GL_TEXTURE_GEN_T);

  // Tangent space light vector will be encoded in primary color.
  // Do dot3 between texenv constant color (light vector) and normal. (N.L)
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
  glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_DOT3_RGB);
  glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_CONSTANT);
  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
  glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
  if(mEnableTranslucency)
  {
    // If we use translucency, then surface's alpha is in alpha component of
    //  primary color. Replace surface's alpha with it.
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
  }
}

void BoWaterManager::setupDiffuseTexture(int unit)
{
  boTextureManager->activateTextureUnit(unit);

  mWaterTex->bind();

  // Use OpenGL's automatic texture coordinate generation.
  const float texPlaneS[] = { 0.1, 0.0, 0.0, 0.0 };
  const float texPlaneT[] = { 0.0, 0.1, 0.0, 0.0 };
  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
  glTexGenfv(GL_S, GL_OBJECT_PLANE, texPlaneS);
  glTexGenfv(GL_T, GL_OBJECT_PLANE, texPlaneT);
  glEnable(GL_TEXTURE_GEN_S);
  glEnable(GL_TEXTURE_GEN_T);
}

void BoWaterManager::initRenderEnvironment()
{
  if(mRenderEnvironmentSetUp)
  {
    // Rebind textures, because they may have been disabled.
    if(mEnableShader)
    {
      mShader->bind();

      // Texture 0 is water texture
      boTextureManager->activateTextureUnit(0);
      mWaterTex->bind();

      // Texture 1 is bumpmap
      boTextureManager->activateTextureUnit(1);
      if(mEnableAnimBumpmaps)
      {
        mWaterAnimBump.at((int)mWaterAnimBumpCurrent)->bind();
      }
      else
      {
        mWaterBump->bind();
      }

      // Texture 2 is envmap
      boTextureManager->activateTextureUnit(2);
      mEnvMap->bind();

      boTextureManager->activateTextureUnit(0);
    }
    else if(mEnableReflections && !mEnableBumpmapping)
    {
      boTextureManager->activateTextureUnit(1);
      mEnvMap->bind();

      boTextureManager->activateTextureUnit(0);
      mWaterTex->bind();
    }
    else if(mEnableBumpmapping)
    {
      boTextureManager->activateTextureUnit(0);
      if(mEnableAnimBumpmaps)
      {
        mWaterAnimBump.at((int)mWaterAnimBumpCurrent)->bind();
      }
      else
      {
        mWaterBump->bind();
      }

      boTextureManager->activateTextureUnit(1);
      mWaterTex->bind();

      boTextureManager->activateTextureUnit(0);
    }
    else
    {
      boTextureManager->activateTextureUnit(0);
      mWaterTex->bind();
    }
    return;
  }

  // Attributes will be popped at the end of render().
  glPushAttrib(GL_ALL_ATTRIB_BITS);

  // Setup textures
  if(mEnableShader)
  {
    glDisable(GL_LIGHTING);

    mShader->bind();

    setupDiffuseTexture(0);
    setupBumpMapTexture(1);
    setupEnvMapTexture(2);
  }
  else if(mEnableReflections && !mEnableBumpmapping)
  {
    // If we have environment mapping but no bumpmapping then we have:
    //  * tex0: water texture
    //  * tex1: environment cubemap

    setupDiffuseTexture(0);
    setupEnvMapTexture(1);
  }
  else if(mEnableBumpmapping)
  {
    // If we have bumpmapping then we have:
    //  * tex0: water bumpmap
    //  * tex1: water texture
    // THIS IS ONLY FOR PASS1 !!!

    glDisable(GL_LIGHTING);

    setupBumpMapTexture(0);

    setupDiffuseTexture(1);

    // Water surface texture will be modulated with the result of previous
    //  texture operation (N.L).
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
    if(mEnableTranslucency)
    {
      // Use alpha from previous texture operation.
      // TODO: maybe replace alpha initially here (not in texture 0)?
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
      glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
      glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PREVIOUS);
      glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
    }
  }
  else
  {
    // No reflections, no bumpmapping. Boring :-)
    setupDiffuseTexture(0);
  }

  if(!mEnableBumpmapping && !mEnableShader)
  {
    float ambientcolor[] = { mWaterAmbientColor, mWaterAmbientColor, mWaterAmbientColor, 1.0f };
    float diffusecolor[] = { mWaterDiffuseColor, mWaterDiffuseColor, mWaterDiffuseColor, 1.0f };
    float specularcolor[] = { mWaterSpecularColor, mWaterSpecularColor, mWaterSpecularColor, 1.0f };
    if(!mEnableSpecular)
    {
      specularcolor[0] = specularcolor[1] = specularcolor[2] = 0.0f;
    }
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,  ambientcolor);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,  diffusecolor);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specularcolor);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mWaterShininess);
  }

  // Just in case...
  boTextureManager->activateTextureUnit(0);

  mRenderEnvironmentSetUp = true;
}

void BoWaterManager::loadNecessaryTextures()
{
  if(mLakes.count() == 0)
  {
    // If we don't have any lakes, we don't need textures either.
    return;
  }

  // Find out texture path
  QString path = KGlobal::dirs()->findResourceDir("data", "boson/themes/grounds/earth/water/water.jpg");
  if(path.isNull())
  {
    boError() << k_funcinfo << "No boson/themes/grounds/earth/water/water.jpg file found!" << endl;
    return;
  }
  path += "boson/themes/grounds/earth/water/";
  // Load water (diffuse) texture
  if(!mWaterTex)
  {
    mWaterTex = new BoTexture(path + "water.jpg", BoTexture::Terrain);
  }
  // Load environment map if we need it
  if((mEnableShader || mEnableReflections) && !mEnvMap)
  {
    mEnvMap = new BoTexture(path + "sky-%1.jpg",
        BoTexture::FilterLinearMipmapLinear | BoTexture::FormatAuto, BoTexture::TextureCube);
  }

  // Create bumpmaps if necessary
  if(mEnableShader || mEnableBumpmapping)
  {
    if(mEnableAnimBumpmaps && (mWaterAnimBump.count() == 0))
    {
      // Load animated water bumpmap textures
      QDir d(path);
      QStringList files = d.entryList("water-animbumpmap-*.png", QDir::Files, QDir::Name);
      for(QStringList::Iterator it = files.begin(); it != files.end(); it++)
      {
        BoTexture* tex = new BoTexture(path + *it, BoTexture::NormalMap);
        mWaterAnimBump.append(tex);
      }
      mWaterAnimBumpCurrent = 0.0f;
    }
    else if(!mWaterBump)
    {
      mWaterBump = new BoTexture(path + "water-bumpmap.png", BoTexture::NormalMap);
    }
  }

  if(mEnableShader)
  {
    delete mShader;
    mShader = new BoShader(path + "water.shader");
    if(!mShader->valid())
    {
      boDebug() << k_funcinfo << "Shader loading failed (from file '" << path << "water.shader'), disabling shader" << endl;
      delete mShader;
      mShader = 0;
      mEnableShader = false;
    }
  }
  else if(mShader)
  {
    delete mShader;
    mShader = 0;
  }
}

