/*
    This file is part of the Boson game
    Copyright (C) 2004 The Boson Team (boson-devel@lists.sourceforge.net)

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

#define GL_GLEXT_LEGACY
#define GL_GLEXT_PROTOTYPES
#define GLX_GLXEXT_PROTOTYPES

#define QT_CLEAN_NAMESPACE

#include "bowater.h"

#include "bosonmap.h"
#include "bodebug.h"
#include "info/boinfo.h"
#include "botexture.h"
#include "bo3dtools.h"
#include "bolight.h"
#include "bosonconfig.h"
#include "playerio.h"

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

#ifndef GLAPI
#define GLAPI
#endif
#include <GL/glext.h>
#include <GL/glx.h>

#define CHUNK_SIZE 10


// OpenGL functions stuff
#ifndef GLsizeiptrARB
// This type was added for vbo extension.
typedef int GLsizeiptrARB;
#endif

typedef void (*_bo_glBlendColor) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void (*_bo_glBlendColorEXT) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void (*_bo_glDeleteBuffersARB)(GLsizei, const GLuint*);
typedef void (*_bo_glGenBuffersARB)(GLsizei, GLuint*);
typedef void (*_bo_glBindBufferARB)(GLenum, GLuint);
typedef void (*_bo_glBufferDataARB)(GLenum, GLsizeiptrARB, const GLvoid*, GLenum);
typedef GLvoid* (* _bo_glMapBufferARB) (GLenum target, GLenum access);
typedef GLboolean (* _bo_glUnmapBufferARB) (GLenum target);

static _bo_glBlendColor bo_glBlendColor = 0;
static _bo_glBlendColorEXT bo_glBlendColorEXT = 0;
static _bo_glDeleteBuffersARB bo_glDeleteBuffersARB = 0;
static _bo_glGenBuffersARB bo_glGenBuffersARB = 0;
static _bo_glBindBufferARB bo_glBindBufferARB = 0;
static _bo_glBufferDataARB bo_glBufferDataARB = 0;
static _bo_glMapBufferARB bo_glMapBufferARB = 0;
static _bo_glUnmapBufferARB bo_glUnmapBufferARB = 0;

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
  waveVector.set(0.866, 0.5, 0.3);
  waveSpeed = 1.0;
  waveHeightMin = 0.2;
  waveHeightMax = 2.0;
  //textureMatrix.rotate(30, 0, 0, 1);
  type = Flat;
  alphaMultiplier = 0.8;
  alphaBase = 0.2;
  chunks.setAutoDelete(true);
}

float BoLake::height(float x, float y)
{
  if(type == Flat || !manager->wavesEnabled())
  {
    return level;
  }
  else
  {
    return level +
        sin(((x * waveVector.x()) + (y * waveVector.y())) + waveSpeed * manager->time()) *
        QMAX(waveHeightMin, QMIN(waveHeightMax,
            (manager->groundHeightAt(x, y) - level) * waveVector.z()
        ));
  }
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
      chunk->center = BoVector3((chunk->minx + chunk->maxx) / 2.0f, -(chunk->miny + chunk->maxy) / 2.0f, level);
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
      boWarning() << k_funcinfo << "Removing chunk with " << chunk->corners << " corners" << endl;
      invalidchunks.append(chunk);
    }
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

    // Load surface type
    BoLake::SurfaceType type;
    type = (BoLake::SurfaceType)lake.attribute("SurfaceType").toInt(&ok);
    if(!ok)
    {
      boError() << k_funcinfo << "Error loading SurfaceType attribute ('" << root.attribute("SurfaceType") << "')" << endl;
      ret = false;
      continue;
    }

    BoLake* bolake = new BoLake(this, level);
    bolake->type = type;
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
    l.setAttribute("SurfaceType", (int)lake->type);
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
#ifdef GL_ARB_texture_cube_map
  mSupports_cubemap = extensions.contains("GL_ARB_texture_cube_map");
#else
#warning GL_ARB_texture_cube_map not supported at compile time!
  mSupports_cubemap = false;
#endif
  if(!mSupports_cubemap)
  {
    boDebug() << k_funcinfo << "GL_ARB_texture_cube_map not supported!" << endl;
  }

#ifdef GL_EXT_texture_lod_bias
  mSupports_texlod = extensions.contains("GL_EXT_texture_lod_bias");
#else
#warning GL_EXT_texture_lod_bias not supported at compile time!
  mSupports_texlod = false;
#endif
  if(!mSupports_texlod)
  {
    boDebug() << k_funcinfo << "GL_EXT_texture_lod_bias not supported!" << endl;
  }

#ifdef GL_ARB_texture_env_combine
  mSupports_env_combine = extensions.contains("GL_ARB_texture_env_combine");
#else
#warning GL_ARB_texture_env_combine not supported at compile time!
  mSupports_env_combine = false;
#endif
  if(!mSupports_env_combine)
  {
    boDebug() << k_funcinfo << "GL_ARB_texture_env_combine not supported!" << endl;
  }

#ifdef GL_ARB_texture_env_dot3
  mSupports_env_dot3 = extensions.contains("GL_ARB_texture_env_dot3");
#else
#warning GL_ARB_texture_env_dot3 not supported at compile time!
  mSupports_env_dot3 = false;
#endif
  if(!mSupports_env_dot3)
  {
    boDebug() << k_funcinfo << "GL_ARB_texture_env_dot3 not supported!" << endl;
  }

  // This one is required by Boson anyway...
#ifdef GL_ARB_multitexture
  mSupports_multitexture = extensions.contains("GL_ARB_multitexture");
#else
#warning GL_ARB_multitexture not supported at compile time!
  mSupports_multitexture = false;
#endif
  if(!mSupports_multitexture)
  {
    boDebug() << k_funcinfo << "GL_ARB_multitexture not supported!" << endl;
  }

#ifdef GL_ARB_imaging
  mSupports_blendcolor = extensions.contains("GL_ARB_imaging");
#else
#warning GL_ARB_imaging not supported at compile time!
  mSupports_blendcolor = false;
#endif
  if(!mSupports_blendcolor)
  {
#ifdef GL_EXT_blend_color
    mSupports_blendcolor_ext = extensions.contains("GL_EXT_blend_color");
#else
#warning GL_EXT_blend_color not supported at compile time!
    mSupports_blendcolor_ext = false;
#endif
    if(!mSupports_blendcolor_ext)
    {
      boDebug() << k_funcinfo << "GL_ARB_imaging and GL_EXT_blend_color not supported!" << endl;
    }
    else
    {
      bo_glBlendColorEXT = (_bo_glBlendColorEXT)glXGetProcAddressARB((const GLubyte*)"glBlendColorEXT");
    }
  }
  else
  {
      bo_glBlendColor = (_bo_glBlendColor)glXGetProcAddressARB((const GLubyte*)"glBlendColorEXT");
  }

#ifdef GL_ARB_vertex_buffer_object
  mSupports_vbo = extensions.contains("GL_ARB_vertex_buffer_object");
#else
#warning GL_ARB_vertex_buffer_object not supported at compile time!
  mSupports_vbo = false;
#endif
  if(!mSupports_vbo)
  {
    boDebug() << k_funcinfo << "GL_ARB_vertex_buffer_object not supported!" << endl;
  }
  else
  {
    bo_glDeleteBuffersARB = (_bo_glDeleteBuffersARB)glXGetProcAddressARB((const GLubyte*)"glDeleteBuffersARB");
    bo_glGenBuffersARB = (_bo_glGenBuffersARB)glXGetProcAddressARB((const GLubyte*)"glGenBuffersARB");
    bo_glBindBufferARB = (_bo_glBindBufferARB)glXGetProcAddressARB((const GLubyte*)"glBindBufferARB");
    bo_glBufferDataARB = (_bo_glBufferDataARB)glXGetProcAddressARB((const GLubyte*)"glBufferDataARB");
    bo_glMapBufferARB = (_bo_glMapBufferARB)glXGetProcAddressARB((const GLubyte*)"glMapBufferARB");
    bo_glUnmapBufferARB = (_bo_glUnmapBufferARB)glXGetProcAddressARB((const GLubyte*)"glUnmapBufferARB");
  }

#ifdef GL_EXT_texture3D
  mSupports_texture3d = extensions.contains("GL_EXT_texture3D");
#else
#warning GL_EXT_texture3D not supported at compile time!
  mSupports_texture3d = false;
#endif
  if(!mSupports_texture3d)
  {
    boDebug() << k_funcinfo << "GL_EXT_texture3D not supported!" << endl;
  }

  boDebug() << k_funcinfo << "Extensions checking done" << endl;


  // Config settings.
  // TODO: move those to BoLake so that every lake can have independent
  //  settings.
  mReflectionSharpness = 1.5f;
  mReflectionStrength = 0.25f;
  mWaterAmbientColor = 0.2f;
  mWaterDiffuseColor = 0.6f;
  mWaterSpecularColor = 1.0f;
  mWaterShininess = 32.0f;

  // Load config.
  mEnableReflections = boConfig->waterReflections();
  mEnableBumpmapping = boConfig->waterBumpmapping();
  mEnableAnimBumpmaps = boConfig->waterAnimatedBumpmaps();
  mEnableTranslucency = boConfig->waterTranslucency();
  mEnableWaves = boConfig->waterWaves();
  // TODO: settings for: VBO, specular

  // Check if loaded config is actually supported
  // Note that we can't use supports*() methods here because opengl stuff isn't
  //  fully inited yet.
  if(mEnableReflections && !(mSupports_multitexture && mSupports_env_combine))
  {
    boWarning() << k_funcinfo << "Reflections are enabled, but not supported. Disabling." << endl;
    mEnableReflections = false;
    boConfig->setWaterReflections(false);
  }
  if(mEnableBumpmapping && !(mSupports_multitexture && mSupports_env_combine && mSupports_env_dot3 && (mSupports_blendcolor || mSupports_blendcolor_ext)))
  {
    boWarning() << k_funcinfo << "Bumpmapping is enabled, but not supported. Disabling." << endl;
    mEnableBumpmapping = false;
    boConfig->setWaterBumpmapping(false);
  }
  if(mEnableTranslucency && !(mSupports_multitexture && mSupports_env_combine))
  {
    boWarning() << k_funcinfo << "Translucency is enabled, but not supported. Disabling." << endl;
    mEnableTranslucency = false;
    boConfig->setWaterTranslucency(false);
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

  if(boConfig->waterReflections() != mEnableReflections)
  {
    mEnableReflections = boConfig->waterReflections();
    // No need to set configDirty to true
    texturesHaveChanged = true;
  }
  if(boConfig->waterAnimatedBumpmaps() != mEnableAnimBumpmaps)
  {
    mEnableAnimBumpmaps = boConfig->waterAnimatedBumpmaps();
    // No need to set configDirty to true
    texturesHaveChanged = true;
  }
  if(boConfig->waterBumpmapping() != mEnableBumpmapping)
  {
    mEnableBumpmapping = boConfig->waterBumpmapping();
    configDirty = true;
    texturesHaveChanged = true;
  }
  if(boConfig->waterTranslucency() != mEnableTranslucency)
  {
    mEnableTranslucency = boConfig->waterTranslucency();
    configDirty = true;
  }
  if(boConfig->waterWaves() != mEnableWaves)
  {
    mEnableWaves = boConfig->waterWaves();
    configDirty = true;
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
        delete[] chunk->normals;
        delete[] chunk->tangentlight;
        delete[] chunk->tangentlight4;
        delete[] chunk->halfvector;
        delete[] chunk->colors;
        delete[] chunk->cellnormals;
        delete[] chunk->celltangentlight;
        delete[] chunk->cellhalfvector;
        delete[] chunk->indices;
        chunk->vertices = 0;
        chunk->normals = 0;
        chunk->tangentlight = 0;
        chunk->tangentlight4 = 0;
        chunk->halfvector = 0;
        chunk->colors = 0;
        chunk->cellnormals = 0;
        chunk->celltangentlight = 0;
        chunk->cellhalfvector = 0;
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

bool BoWaterManager::supportsReflections() const
{
  if(!mOpenGLInited)
  {
    return false;
  }

  return mSupports_multitexture && mSupports_env_combine;
}

bool BoWaterManager::supportsTranslucency() const
{
  if(!mOpenGLInited)
  {
    return false;
  }

  return mSupports_multitexture && mSupports_env_combine;
}

bool BoWaterManager::supportsBumpmapping() const
{
  if(!mOpenGLInited)
  {
    return false;
  }

  return mSupports_multitexture && mSupports_env_combine && mSupports_env_dot3 && (mSupports_blendcolor || mSupports_blendcolor_ext);
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
  return QMIN(1.0, ((lake->height(x, y) - groundHeightAt(x, y)) * lake->alphaMultiplier + lake->alphaBase)/* * mWaterDiffuseColor*/);
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
        return QMAX(0.0f, lake->height(x, y) - groundHeight(x, y));
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
  mWaterAnimBumpCurrent += elapsed * 5;
  if(mWaterAnimBump.count() && ((unsigned int)mWaterAnimBumpCurrent >= mWaterAnimBump.count()))
  {
    mWaterAnimBumpCurrent -= mWaterAnimBump.count();
  }
  setDirty(true);
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
  setDirty(true);
}

float BoWaterManager::sphereInFrustum(const BoVector3& pos, float radius) const
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

  /*if(mSupports_cubemap)
  {
    // Render skybox
    glPushAttrib(GL_LIGHTING_BIT | GL_TEXTURE_BIT | GL_ENABLE_BIT | GL_TRANSFORM_BIT);

    glDisable(GL_LIGHTING);

    mEnvMap->enable();
    mEnvMap->bind();
    // We have z-axis pointing upwards, but most cubemaps are made for programs
    //  where y-axis points upwards. So we rotate texture matrix to get correct
    //  texture coords
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadMatrixf(mModelview.data());
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glLoadIdentity();
    //glRotatef(-90, 1, 0, 0);
    //glScalef(1, -1, 1);  // Because y is mirrored

    const float sky_radius = 400.0f;

    glBegin(GL_QUADS);
      // Sky (positive z)
      glTexCoord3f(-1, -1,  1); glVertex3f(-sky_radius, -sky_radius,  sky_radius);
      glTexCoord3f(-1,  1,  1); glVertex3f(-sky_radius,  sky_radius,  sky_radius);
      glTexCoord3f( 1,  1,  1); glVertex3f( sky_radius,  sky_radius,  sky_radius);
      glTexCoord3f( 1, -1,  1); glVertex3f( sky_radius, -sky_radius,  sky_radius);

      // Ground (negative z)
      glTexCoord3f(-1, -1, -1); glVertex3f(-sky_radius, -sky_radius, -sky_radius);
      glTexCoord3f(-1,  1, -1); glVertex3f(-sky_radius,  sky_radius, -sky_radius);
      glTexCoord3f( 1,  1, -1); glVertex3f( sky_radius,  sky_radius, -sky_radius);
      glTexCoord3f( 1, -1, -1); glVertex3f( sky_radius, -sky_radius, -sky_radius);

      // negative x
      glTexCoord3f(-1, -1,  1); glVertex3f(-sky_radius, -sky_radius,  sky_radius);
      glTexCoord3f(-1, -1, -1); glVertex3f(-sky_radius, -sky_radius, -sky_radius);
      glTexCoord3f(-1,  1, -1); glVertex3f(-sky_radius,  sky_radius, -sky_radius);
      glTexCoord3f(-1,  1,  1); glVertex3f(-sky_radius,  sky_radius,  sky_radius);

      // positive x
      glTexCoord3f( 1, -1,  1); glVertex3f( sky_radius, -sky_radius,  sky_radius);
      glTexCoord3f( 1, -1, -1); glVertex3f( sky_radius, -sky_radius, -sky_radius);
      glTexCoord3f( 1,  1, -1); glVertex3f( sky_radius,  sky_radius, -sky_radius);
      glTexCoord3f( 1,  1,  1); glVertex3f( sky_radius,  sky_radius,  sky_radius);

      // negative y
      glTexCoord3f(-1, -1,  1); glVertex3f(-sky_radius, -sky_radius,  sky_radius);
      glTexCoord3f( 1, -1,  1); glVertex3f( sky_radius, -sky_radius,  sky_radius);
      glTexCoord3f( 1, -1, -1); glVertex3f( sky_radius, -sky_radius, -sky_radius);
      glTexCoord3f(-1, -1, -1); glVertex3f(-sky_radius, -sky_radius, -sky_radius);

      // positive y
      glTexCoord3f(-1,  1,  1); glVertex3f(-sky_radius,  sky_radius,  sky_radius);
      glTexCoord3f( 1,  1,  1); glVertex3f( sky_radius,  sky_radius,  sky_radius);
      glTexCoord3f( 1,  1, -1); glVertex3f( sky_radius,  sky_radius, -sky_radius);
      glTexCoord3f(-1,  1, -1); glVertex3f(-sky_radius,  sky_radius, -sky_radius);
    glEnd();

    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    mEnvMap->disable();
    glPopAttrib();
  }*/

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
    if(mEnableReflections && mSupports_multitexture && !mEnableBumpmapping)
    {
      glActiveTextureARB(GL_TEXTURE1_ARB);
#ifdef GL_EXT_texture_lod_bias
      if(mSupports_texlod)
      {
        glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, 0.0f);
      }
#endif
      glPopMatrix();
      mEnvMap->disable();
      glActiveTextureARB(GL_TEXTURE0_ARB);
    }
    glPopAttrib();
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
  BoVector3 lakemin(lake->minx, -lake->miny, lake->level - lake->waveHeightMax);
  BoVector3 lakemax(lake->maxx, -lake->maxy, lake->level + lake->waveHeightMax);
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
    BoVector3 chunkmin(chunk->minx, -chunk->miny, lake->level - lake->waveHeightMax);
    BoVector3 chunkmax(chunk->maxx, -chunk->maxy, lake->level + lake->waveHeightMax);
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

// Some macros for easily accessing data in arrays.
#define NORM(x, y) info->chunk->cellnormals[(y) * info->chunkcellw + (x)]
#define TANGENTLIGHT(x, y) info->chunk->celltangentlight[(y) * info->chunkcellw + (x)]
#define HALFVECTOR(x, y) info->chunk->cellhalfvector[(y) * info->chunkcellw + (x)]

#define ARRAY_CORNER(array, x, y)  array[(y) * info->chunkcornerw + (x)]

void BoWaterManager::renderChunk(BoLake* lake, BoLake::WaterChunk* chunk, float chunkdetail)
{
  // Create new RenderInfo object
  RenderInfo* info = new RenderInfo;
  info->lake = lake;
  info->chunk = chunk;
  info->detail = chunkdetail;
  info->texrepeat = 10;

  // If nothing has been rendered yet, OpenGL stuff (e.g. textures) are
  //  uninited. Init them now.
  if(!mRenderEnvironmentSetUp)
  {
    initRenderEnvironment();
  }

  // Set texture matrices. They make textures move slowly to create an illusion
  //  that water surface is moving.
  BoMatrix texMatrix = lake->textureMatrix;
  texMatrix.translate(0, mTime * 0.04, 0);
  glMatrixMode(GL_TEXTURE);
  glActiveTextureARB(GL_TEXTURE0_ARB);  // Either diffuse or bump
  glPushMatrix();
  glLoadMatrixf(texMatrix.data());
  if(mEnableBumpmapping)
  {
    glActiveTextureARB(GL_TEXTURE1_ARB);  // Diffuse
    glPushMatrix();
    glLoadMatrixf(texMatrix.data());
  }

  // First find out which rendering tehniques to use for the chunk:
  // Check if we'll render flat surface or not.
  info->flat = (lake->type == BoLake::Flat) || !mEnableWaves;
  // Check whether water's alpha in chunk is variable or constant.
  info->constalpha = -1.0f;  // -1.0f means variable alpha
  if(mEnableTranslucency)
  {
    if(info->flat)
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
    else if(lake->type == BoLake::Waves)
    {
      if(lake->level - chunk->maxgroundheight > 0.0f)
      {
        // Whole chunk is underwater. Maybe water is deep enough so that alpha
        //  is 1?
        float minalpha = (lake->level - lake->waveHeightMax - chunk->maxgroundheight) * lake->alphaMultiplier + lake->alphaBase;
        if(minalpha >= 1.0f)
        {
          // Alpha is always at least 1.0f, i.e. constant
          info->constalpha = 1.0f;
        }
      }
    }
  }
  else
  {
    // Translucency isn't used. Set alpha to 1.0f
    info->constalpha = 1.0f;
  }
  // Check if we can use derivations to calculate normals and tangent-space
  //  vectors.
  info->useDerivations = false;
  if(lake->type == BoLake::Waves && mEnableWaves)
  {
    info->useDerivations = true;
  }
  // If we're very lucky, we may be able to render whole chunk as a single quad
  info->singleQuad = false;
  if(info->flat && info->constalpha != -1.0f && !mEnableReflections)
  {
    info->singleQuad = true;
    info->detail = 1000;
    chunkdetail = 1000;
  }


  // If we'll calculate per-cell normals and tangent-space vectors, we need
  //  to set up chunk borders first.
  if(!info->flat && !info->useDerivations)
  {
    calculateChunkBorders(info);
  }

  // Chunk size in corners (depends on detail level) and doesn't include
  //  border.
  // This is for stuff which is calculated per-corner.
  info->chunkcornerw = (int)ceilf((chunk->maxx - chunk->minx) / chunkdetail + 1.0f);
  info->chunkcornerh = (int)ceilf((chunk->maxy - chunk->miny) / chunkdetail + 1.0f);

  // Light vector for bumpmapping. Sun is directional light, so we don't need
  //  to calculate this per-vertex.
  if(mEnableBumpmapping)
  {
    if(!mSun)
    {
      boError() << k_funcinfo << "NULL sun!" << endl;
    }
    info->lightvector = mSun->position3();
    info->lightvector.normalize();
  }


  // Recalculate all the data if necessary
  if(chunk->dirty)
  {
    // Init dat buffers (vbos/arrays)
    initDataBuffersForStorage(info);

    // Pass 1: calculate normals of each cell (unless derivations are used)
    //  Here, we also fill vertex and texcoords vbos if they're used
    if(!info->flat && !info->useDerivations)
    {
      calculatePerCellStuff(info);
    }

    // Pass 2: calculate normal for each corner by taking average of normals of all
    //  cells that have this corner.
    //  Also fill in normal, tangentlight and halfvector vbos.
    calculatePerCornerStuff(info);

    calculateIndices(info);

    uninitDataBuffersForStorage(info);

    chunk->dirty = false;
  }


  // Render all the cells in this chunk
  if(info->constalpha == 1.0f)
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
      BoVector4 diffusecolor = mSun ? mSun->diffuse() : BoVector4(1.0f, 1.0f, 1.0f, 1.0f);
      diffusecolor.scale(mWaterDiffuseColor);
      diffusecolor.setW(1.0f);
      glEnable(GL_BLEND);
#ifdef GL_ARB_imaging
      if(mSupports_blendcolor)
      {
        bo_glBlendColor(diffusecolor.x(), diffusecolor.y(), diffusecolor.z(), diffusecolor.w());
        glBlendFunc(GL_CONSTANT_COLOR, GL_ZERO);
      }
      else
#endif
#ifdef GL_EXT_blend_color
      if(mSupports_blendcolor_ext)
      {
        bo_glBlendColorEXT(diffusecolor.x(), diffusecolor.y(), diffusecolor.z(), diffusecolor.w());
        glBlendFunc(GL_CONSTANT_COLOR_EXT, GL_ZERO);
      }
#endif
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
#ifdef GL_ARB_vertex_buffer_object
  if(mEnableVBO)
  {
    bo_glBindBufferARB(GL_ARRAY_BUFFER_ARB, chunk->vbo_vertex);
    glVertexPointer(3, GL_FLOAT, 0, 0);
  }
  else
#endif
  {
    glVertexPointer(3, GL_FLOAT, 0, chunk->vertices);
  }
  if(mEnableBumpmapping)
  {
    // Tangent-space light vectors are sent as colors
    glEnableClientState(GL_COLOR_ARRAY);
    if(mEnableTranslucency)
    {
      // If we have translucency enabled, water's translucency is sent as
      //  color's alpha
#ifdef GL_ARB_vertex_buffer_object
      if(mEnableVBO)
      {
        bo_glBindBufferARB(GL_ARRAY_BUFFER_ARB, chunk->vbo_tangentlight4);
        glColorPointer(4, GL_FLOAT, 0, 0);
      }
      else
#endif
      {
        glColorPointer(4, GL_FLOAT, 0, chunk->tangentlight4);
      }
    }
    else
    {
      // No translucency
#ifdef GL_ARB_vertex_buffer_object
      if(mEnableVBO)
      {
        bo_glBindBufferARB(GL_ARRAY_BUFFER_ARB, chunk->vbo_tangentlight);
        glColorPointer(3, GL_FLOAT, 0, 0);
      }
      else
#endif
      {
        glColorPointer(3, GL_FLOAT, 0, chunk->tangentlight);
      }
    }
  }
  else
  {
    // For OpenGL lighting (and reflections), we need normals
    glEnableClientState(GL_NORMAL_ARRAY);
#ifdef GL_ARB_vertex_buffer_object
    if(mEnableVBO)
    {
      bo_glBindBufferARB(GL_ARRAY_BUFFER_ARB, chunk->vbo_normal);
      glNormalPointer(GL_FLOAT, 0, 0);
    }
    else
#endif
    {
      glNormalPointer(GL_FLOAT, 0, chunk->normals);
    }
    if(mEnableTranslucency)
    {
      // If translucency is enabled but bumpmapping is not, then we need to
      //  supply alpha.
      glEnableClientState(GL_COLOR_ARRAY);
#ifdef GL_ARB_vertex_buffer_object
      if(mEnableVBO)
      {
        bo_glBindBufferARB(GL_ARRAY_BUFFER_ARB, chunk->vbo_color);
        glColorPointer(4, GL_FLOAT, 0, 0);
      }
      else
#endif
      {
        glColorPointer(4, GL_FLOAT, 0, chunk->colors);
      }
    }
  }

  // Do the drawing
#ifdef GL_ARB_vertex_buffer_object
  if(mEnableVBO)
  {
    bo_glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, chunk->vbo_index);
    glDrawElements(GL_QUADS, chunk->indices_count, GL_UNSIGNED_INT, 0);
  }
  else
#endif
  {
    glDrawElements(GL_QUADS, chunk->indices_count, GL_UNSIGNED_INT, chunk->indices);
  }
  mRenderedQuads += chunk->indices_count / 4;
  glPopClientAttrib();


  if(mEnableBumpmapping)
  {
    // Pass 2: ambient lighting and reflection pass.
    // TODO: combine this with diffuse pass on programmable hardware.

    // Push attribute stack
    glPushAttrib(/*GL_COLOR_BUFFER_BIT | GL_LIGHTING_BIT | GL_TEXTURE_BIT | */GL_ALL_ATTRIB_BITS);

    setupDiffuseTexture(GL_TEXTURE0_ARB);
    // Modulate with primary color
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    if(mEnableReflections)
    {
      glActiveTextureARB(GL_TEXTURE1_ARB);
      setupEnvMapTexture(GL_TEXTURE1_ARB);
    }

    // Amount of ambient light water will receive.
    BoVector4 ambientcolor = mSun ? mSun->ambient() : BoVector4(1.0f, 1.0f, 1.0f, 1.0f);
    ambientcolor.scale(mWaterAmbientColor);
    ambientcolor.setW(1.0f);

    // Amount of reflections water will receive.
    BoVector4 reflectioncolor;
    if(mEnableReflections)
    {
      reflectioncolor.set(mReflectionStrength, mReflectionStrength, mReflectionStrength, 1.0f);
    }

    // We want final color to be:
    //    diffusetex * ambientcolor + reflectiontex * reflectioncolor
    //  (note that both ambient and diffuse lighting use same texture)

    BoVector4 resultcolor = ambientcolor;
    if(mEnableReflections)
    {
      // If reflections are used, then we use:
      //    reflectiontex * reflectioncolor + diffusetex * primarycolor * (1 - reflectioncolor)
      //  (reflectioncolor is used for interpolating)
      //  We need  primarycolor * (1 - reflectioncolor)  to be equal to ambientcolor, so:
      //    primarycolor = ambientcolor / (1 - reflectioncolor)

      // TODO: actually sky will get darker with dim light, compensate for this
      //  (e.g. by multiplying reflection color with light's ambient (diffuse?)
      //  color).
      BoVector4 reflectioncolor(mReflectionStrength, mReflectionStrength, mReflectionStrength, 1.0f);
      glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, reflectioncolor.data());

      resultcolor = ambientcolor;
      resultcolor.scale(1.0f - mReflectionStrength);
      resultcolor.setW(1.0f);
    }
    // Set primary color.  Whole model will be drawn using that color (and no
    //  shading/lighting).
    glColor4fv(resultcolor.data());

    // Use additive blending.
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    // Disable depth buffer writes. This might save some time.
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);

    // FIXME!!! We need to send alpha for ambient lighting as well.

    glPushClientAttrib(GL_ALL_ATTRIB_BITS);
    // Vertices
    glEnableClientState(GL_VERTEX_ARRAY);
#ifdef GL_ARB_vertex_buffer_object
    if(mEnableVBO)
    {
      bo_glBindBufferARB(GL_ARRAY_BUFFER_ARB, chunk->vbo_vertex);
      glVertexPointer(3, GL_FLOAT, 0, 0);
    }
    else
#endif
    {
      glVertexPointer(3, GL_FLOAT, 0, chunk->vertices);
    }
    // We need normals for the automatic texgen
    glEnableClientState(GL_NORMAL_ARRAY);
#ifdef GL_ARB_vertex_buffer_object
    if(mEnableVBO)
    {
      bo_glBindBufferARB(GL_ARRAY_BUFFER_ARB, chunk->vbo_normal);
      glNormalPointer(GL_FLOAT, 0, 0);
    }
    else
#endif
    {
      glNormalPointer(GL_FLOAT, 0, chunk->normals);
    }

    // Do the drawing
#ifdef GL_ARB_vertex_buffer_object
    if(mEnableVBO)
    {
      bo_glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, chunk->vbo_index);
      glDrawElements(GL_QUADS, chunk->indices_count, GL_UNSIGNED_INT, 0);
    }
    else
#endif
    {
      glDrawElements(GL_QUADS, chunk->indices_count, GL_UNSIGNED_INT, chunk->indices);
    }
    mRenderedQuads += chunk->indices_count / 4;
    glPopClientAttrib();

    if(mEnableReflections)
    {
      // Texture matrix was push()ed for envmap. Pop it now
      glMatrixMode(GL_TEXTURE);
      glActiveTextureARB(GL_TEXTURE1_ARB);
      glPopMatrix();
    }
    // Diffuse texture's matrix was also pushed. Pop this one as well.
    glMatrixMode(GL_TEXTURE);
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glPopMatrix();

    // Enable depth writes and pop attributes.
    glDepthMask(GL_TRUE);
    glPopAttrib();
  }

  glMatrixMode(GL_TEXTURE);
  glActiveTextureARB(GL_TEXTURE0_ARB);
  glPopMatrix();

  delete info;
}

void BoWaterManager::calculateChunkBorders(RenderInfo* info)
{
  // First find out size of the border of the chunk. Border is used to
  //  calculate flat normals (we calculate them for cells both in the chunk and
  //  on the border).
  // Reset all borders to 1 first.
  info->border_left = 1, info->border_right = 1, info->border_top = 1, info->border_bottom = 1;
  // FIXME: name
  info->cellminx = info->chunk->minx - info->detail;
  if(info->cellminx < info->lake->minx)
  {
    info->cellminx = info->lake->minx;
    info->border_left = 0;
  }
  info->cellminy = info->chunk->miny - info->detail;
  if(info->cellminy < info->lake->miny)
  {
    info->cellminy = info->lake->miny;
    info->border_top = 0;
  }
  info->cellmaxx = info->chunk->maxx + info->detail;
  if(info->cellmaxx > info->lake->maxx)
  {
    info->cellmaxx = info->lake->maxx;
    info->border_right = 0;
  }
  info->cellmaxy = info->chunk->maxy + info->detail;
  if(info->cellmaxy > info->lake->maxy)
  {
    info->cellmaxy = info->lake->maxy;
    info->border_bottom = 0;
  }

  // Chunk size in cells (depends on detail level). Includes border.
  // Use this only for stuff which is calculated per-cell!!!
  info->chunkcellw = (int)ceilf((info->cellmaxx - info->cellminx) / info->detail);
  info->chunkcellh = (int)ceilf((info->cellmaxy - info->cellminy) / info->detail);
}

void BoWaterManager::calculatePerCellStuff(RenderInfo* info)
{
  int xi, yi;
  xi = 0;  // X Index
  for(float x = info->cellminx; x < info->cellmaxx; x += info->detail, xi++)
  {
    yi = 0;  // Y Index
    for(float y = info->cellminy; y < info->cellmaxy; y += info->detail, yi++)
    {
      if(xi >= info->chunkcellw || yi >= info->chunkcellh)
      {
        boError() << k_funcinfo << "ERROR: invalid index coords: (" << xi << "; " << yi <<
            ")  (chunk size: " << info->chunkcellw << "x" << info->chunkcellh << ")" << endl;
      }
      if(!info->lake->hasAnyCorner(x, y, x + info->detail, y + info->detail))
      {
        NORM(xi, yi) = BoVector3();
        if(mEnableBumpmapping)
        {
          TANGENTLIGHT(xi, yi) = BoVector3();
          HALFVECTOR(xi, yi) = BoVector3();
        }
        continue;
      }
      // Calculate normal for this cell
      BoVector3 a, b, c;
      float x2 = QMIN(x + info->detail, info->cellmaxx);
      float y2 = QMIN(y + info->detail, info->cellmaxy);
      a = BoVector3(x, y, info->lake->height(x, y));
      b = BoVector3(x2, y, info->lake->height(x2, y));
      c = BoVector3(x, y2, info->lake->height(x, y2));
      NORM(xi, yi) = BoVector3::crossProduct(c - b, a - b);
      NORM(xi, yi).normalize();
      if(mEnableBumpmapping)
      {
        // Calculate tangent space light position
        // Calculate tangents of the quad
        /*** Original equation  (for triangle abc):
        BoVector3 side0 = b - a;
        BoVector3 side1 = c - a;
        float deltaT0 = y / texrepeat - y / texrepeat;  // b.texcoord.y - a.texcoord.y
        float deltaT1 = y2 / texrepeat - y / texrepeat;  // c.texcoord.y - a.texcoord.y
        BoVector3 sTangent = deltaT1*side0-deltaT0*side1;
        sTangent.normalize();

        //Calculate t tangent
        float deltaS0 = x2 / texrepeat - x / texrepeat;  // b.texcoord.x - a.texcoord.x
        float deltaS1 = x / texrepeat - x / texrepeat;  // c.texcoord.x - a.texcoord.x
        BoVector3 tTangent = deltaS1*side0-deltaS0*side1;
        tTangent.normalize();*/
        // Simplified & optimized:
        BoVector3 side0 = b - a;
        BoVector3 side1 = a - c;
        BoVector3 sTangent = side0 * ((x2 - x) / info->texrepeat);
        sTangent.normalize();
        BoVector3 tTangent = (side1 * ((y2 - y) / info->texrepeat));
        tTangent.normalize();
        // reverse tangents if necessary
        BoVector3 tangentCross = BoVector3::crossProduct(sTangent, tTangent);
        if(BoVector3::dotProduct(tangentCross, NORM(xi, yi)) < 0.0f)
        {
          sTangent = -sTangent;
          tTangent = -tTangent;
        }
        // Calculate base tangent space light vector (real vector depends on
        //  surface normal and thus differs for each corner)
        BoVector3 tangentSpaceLight;
        tangentSpaceLight.setX(BoVector3::dotProduct(sTangent, info->lightvector));
        tangentSpaceLight.setY(BoVector3::dotProduct(tTangent, info->lightvector));
        tangentSpaceLight.setZ(BoVector3::dotProduct(NORM(xi, yi), info->lightvector));
        tangentSpaceLight.normalize();
        TANGENTLIGHT(xi, yi) = tangentSpaceLight;

        // Calculate H (half-angle vector)
        // H is average of light and view vectors
        BoVector3 viewvector = mCameraPos - a;
        viewvector.normalize();
        BoVector3 halfvector = viewvector + info->lightvector;
        halfvector.normalize();
        BoVector3 tangenthalfvector;
        tangenthalfvector.setX(BoVector3::dotProduct(sTangent, halfvector));
        tangenthalfvector.setY(BoVector3::dotProduct(tTangent, halfvector));
        tangenthalfvector.setZ(BoVector3::dotProduct(NORM(xi, yi), halfvector));
        HALFVECTOR(xi, yi) = tangenthalfvector;
        HALFVECTOR(xi, yi).normalize();
      }
    }
  }
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
      float posz = info->lake->height(posx, posy);

//      if(!info->lake->hasAnyCorner(posx, posy, posx + info->detail, posy + info->detail))
      // hasAnyCorner() does automatic limits checking, so no need to worry about
      //  this here.
      if(!info->lake->hasAnyCorner(posx - info->detail, posy - info->detail, posx + info->detail, posy + info->detail))
      {
        // Reset all variables, just in case...
        // TODO: find out if this is really needed or is it just waste of time
        ARRAY_CORNER(info->vertices_p, x, y) = BoVector3(posx, -posy, 10);  // 10 is random
        ARRAY_CORNER(info->normals_p, x, y) = BoVector3(0, 0, 1);
        if(mEnableBumpmapping)
        {
          if(mEnableTranslucency)
          {
            ARRAY_CORNER(info->tangentlights_p4, x, y) = BoVector4(0, 0, 1, 0);
          }
          else
          {
            ARRAY_CORNER(info->tangentlights_p, x, y) = BoVector3(0, 0, 1);
          }
          ARRAY_CORNER(info->halfvectors_p, x, y) = BoVector3(0, 0, 1);
        }
        else if(mEnableTranslucency)
        {
          ARRAY_CORNER(info->colors_p, x, y) = BoVector4(1, 1, 1, 1);
        }
        continue;
      }

      // Corner's normal
      BoVector3 n;
      // Corner's Tangent-Space Light vector
      BoVector3 tsl;
      // Corner's half-angle vector (also tangent-space)
      BoVector3 halfv;

      if(info->flat)
      {
        // For flat lakes, things are _really_ easy (and fast :-))
        n.set(0, 0, 1);
        if(mEnableBumpmapping)
        {
          // When surface's normal points upwards (is (0; 0; 1)), then
          //  tangent-space is same as world-space.
          tsl = info->lightvector;
          BoVector3 viewvector = mCameraPos - BoVector3(posx, posy, posz);
          viewvector.normalize();
          halfv = viewvector + info->lightvector;
          // halfv will be normalized later
        }
      }
      else if(info->useDerivations)
      {
        // We use derivation to calculate water's normal directly, without having
        //  to first calculate per-cell normals and then averaging.
        float derivation = cos(((posx * info->lake->waveVector.x()) + (posy * info->lake->waveVector.y())) + info->lake->waveSpeed * time()) *
            QMAX(info->lake->waveHeightMin, QMIN(info->lake->waveHeightMax, (groundHeightAt(posx, posy) - info->lake->level) * info->lake->waveVector.z()));
        // Calculate point's normal on 2d plane (xz plane), by rotating it 90 degrees cw or ccw
        BoVector3 planenormal(-derivation, 0, 1);
        // Now rotate 2dnormal around z axis, according to direction of waves, to get real normal
        // Z will remain same
        n.setZ(planenormal.z());
        // Calculate x and y components
        n.setX(planenormal.x() * info->lake->waveVector.x());
        n.setY(planenormal.x() * info->lake->waveVector.y());
        n.normalize();

        // Now calculate bumpmapping stuff
        if(mEnableBumpmapping)
        {
          // Both tangents are perpendicular to normal and their either x- or
          //  y-component is 0.
          // Calculate s-tangent (y=0). x*nx + z*nz = 0
          BoVector3 sTangent;
          if(n.x() == 0)
          {
            // Special case. Tangent's z is also 0, x can be anything, but we
            //  want normalized vector, so let it be 1.
            sTangent.set(1, 0, 0);
          }
          /*else if(n.z() == 0)
          {
            // Another special case, very unlikely.
            sTangent.set(0, 0, 1);
          }*/
          else
          {
            // Let tangent's x be 1 first, then z = -nx / nz
            sTangent.set(1, 0, -n.x() / n.z());
            sTangent.normalize();
          }
          // Calculate t-tangent (x=0). y*ny + z*nz = 0
          BoVector3 tTangent;
          if(n.y() == 0)
          {
            // Special case. Tangent's z is also 0, y can be anything, but we
            //  want normalized vector, so let it be 1.
            tTangent.set(0, 1, 0);
          }
          /*else if(n.z() == 0)
          {
            // Another special case, very unlikely.
            tTangent.set(0, 0, 1);
          }*/
          else
          {
            // Let tangent's y be 1 first, then z = -ny / nz
            tTangent.set(1, 0, -n.y() / n.z());
            tTangent.normalize();
          }

          // Calculate tangent-space light vector.
          tsl.setX(BoVector3::dotProduct(sTangent, info->lightvector));
          tsl.setY(BoVector3::dotProduct(tTangent, info->lightvector));
          tsl.setZ(BoVector3::dotProduct(n, info->lightvector));
          // Will be normalized later

          // Calculate H (half-angle vector).
          // H is average of light and view vectors (vector which is between them).
          BoVector3 viewvector = mCameraPos - BoVector3(posx, posy, posz);
          viewvector.normalize();
          BoVector3 halfvector = viewvector + info->lightvector;
          //halfvector.normalize();
          halfv.setX(BoVector3::dotProduct(sTangent, halfvector));
          halfv.setY(BoVector3::dotProduct(tTangent, halfvector));
          halfv.setZ(BoVector3::dotProduct(n, halfvector));
          // halfv will also be normalized later
        }
      }
      else
      {
        // Don't use derivations.
        // Use average of pre-calculated cell normals to get smooth normal.

        // Coordinates of cell which is bottom-right of current corner.
        int cellx = x + info->border_left;
        int celly = y + info->border_top;
        // upper-left cell
        if (cellx > 0 && celly > 0)
        {
          n += NORM(cellx - 1, celly - 1);
          if(mEnableBumpmapping)
          {
            tsl += TANGENTLIGHT(cellx - 1, celly - 1);
            halfv += HALFVECTOR(cellx - 1, celly - 1);
          }
        }
        // upper-right cell
        if (cellx < (info->chunkcellw - info->border_left) && celly > 0)
        {
          n += NORM(cellx, celly - 1);
          if(mEnableBumpmapping)
          {
            tsl += TANGENTLIGHT(cellx, celly - 1);
            halfv += HALFVECTOR(cellx, celly - 1);
          }
        }
        // lower-right cell
        if (cellx < (info->chunkcellw - info->border_left) && celly < (info->chunkcellh - info->border_top))
        {
          n += NORM(cellx, celly);
          if(mEnableBumpmapping)
          {
            tsl += TANGENTLIGHT(cellx, celly);
            halfv += HALFVECTOR(cellx, celly);
          }
        }
        // lower-left cell
        if (cellx > 0 && celly < (info->chunkcellh - info->border_top))
        {
          n += NORM(cellx - 1, celly);
          if(mEnableBumpmapping)
          {
            tsl += TANGENTLIGHT(cellx - 1, celly);
            halfv += HALFVECTOR(cellx - 1, celly);
          }
        }
        if(n.x() * n.x() + n.y() * n.y() + n.z() * n.z() < 0.05)
        {
          printf("WARNING: vec(%f, %f, %f) has length %f\n", n.x(), n.y(), n.z(), n.length());
          info->chunk->normals[y * info->chunkcornerw + x] = BoVector3(0, 0, 1);
          //continue;
        }
        if(n.z() < 0.1)
        {
          printf("ERROR: z too low: vec(%f, %f, %f)\n", n.x(), n.y(), n.z());
        }
        n.normalize();
      }


      ARRAY_CORNER(info->vertices_p, x, y) = BoVector3(posx, -posy, posz);
      ARRAY_CORNER(info->normals_p, x, y) = n;

      if(mEnableBumpmapping)
      {
        // Tangent-space light vector
        tsl.normalize();
        // Convert it from range [-1; 1] into [0; 1] ...
        tsl.scale(0.5);
        tsl += BoVector3(0.5, 0.5, 0.5);
        BoVector4 tsl4;
        if(mEnableTranslucency)
        {
          tsl4 = BoVector4(tsl.x(), tsl.y(), tsl.z(), waterAlphaAt(info->lake, posx, posy));
          ARRAY_CORNER(info->tangentlights_p4, x, y) = tsl4;
        }
        else
        {
          ARRAY_CORNER(info->tangentlights_p, x, y) = tsl;
        }

        // Half-angle vector
        halfv.normalize();
        // Convert it from range [-1; 1] into [0; 1] ...
        halfv.scale(0.5);
        halfv += BoVector3(0.5, 0.5, 0.5);
        ARRAY_CORNER(info->halfvectors_p, x, y) = halfv;
      }
      else if(mEnableTranslucency)
      {
        ARRAY_CORNER(info->colors_p, x, y) = BoVector4(1, 1, 1, waterAlphaAt(info->lake, posx, posy));
      }
    }
  }
}

void BoWaterManager::initDataBuffersForStorage(RenderInfo* info)
{
  // Corner arrays/vbos first
#ifdef GL_ARB_vertex_buffer_object
  if(mEnableVBO)
  {
    int corners = info->chunkcornerw * info->chunkcornerh;
    // Generate VBOs if it's not yet done
    if(!info->chunk->vbo_vertex)
    {
      bo_glGenBuffersARB(1, &info->chunk->vbo_vertex);
      bo_glGenBuffersARB(1, &info->chunk->vbo_normal);
      bo_glGenBuffersARB(1, &info->chunk->vbo_tangentlight);
      bo_glGenBuffersARB(1, &info->chunk->vbo_tangentlight4);
      bo_glGenBuffersARB(1, &info->chunk->vbo_halfvector);
      bo_glGenBuffersARB(1, &info->chunk->vbo_color);
      bo_glGenBuffersARB(1, &info->chunk->vbo_index);
    }
#define WATER_VBO_MODE GL_DYNAMIC_DRAW_ARB
    bo_glBindBufferARB(GL_ARRAY_BUFFER_ARB, info->chunk->vbo_vertex);
    bo_glBufferDataARB(GL_ARRAY_BUFFER_ARB, corners * sizeof(BoVector3), 0, WATER_VBO_MODE);
    info->vertices_p = (BoVector3*)bo_glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
    bo_glBindBufferARB(GL_ARRAY_BUFFER_ARB, info->chunk->vbo_normal);
    bo_glBufferDataARB(GL_ARRAY_BUFFER_ARB, corners * sizeof(BoVector3), 0, WATER_VBO_MODE);
    info->normals_p = (BoVector3*)bo_glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
    if(mEnableBumpmapping)
    {
      if(mEnableTranslucency)
      {
        bo_glBindBufferARB(GL_ARRAY_BUFFER_ARB, info->chunk->vbo_tangentlight4);
        bo_glBufferDataARB(GL_ARRAY_BUFFER_ARB, corners * sizeof(BoVector4), 0, WATER_VBO_MODE);
        info->tangentlights_p4 = (BoVector4*)bo_glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
      }
      else
      {
        bo_glBindBufferARB(GL_ARRAY_BUFFER_ARB, info->chunk->vbo_tangentlight);
        bo_glBufferDataARB(GL_ARRAY_BUFFER_ARB, corners * sizeof(BoVector3), 0, WATER_VBO_MODE);
        info->tangentlights_p = (BoVector3*)bo_glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
      }
      bo_glBindBufferARB(GL_ARRAY_BUFFER_ARB, info->chunk->vbo_halfvector);
      bo_glBufferDataARB(GL_ARRAY_BUFFER_ARB, corners * sizeof(BoVector3), 0, WATER_VBO_MODE);
      info->halfvectors_p = (BoVector3*)bo_glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
    }
    else if(mEnableTranslucency)
    {
      bo_glBindBufferARB(GL_ARRAY_BUFFER_ARB, info->chunk->vbo_color);
      bo_glBufferDataARB(GL_ARRAY_BUFFER_ARB, corners * sizeof(BoVector4), 0, WATER_VBO_MODE);
      info->colors_p = (BoVector4*)bo_glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
    }
    // Buffer for indices
    bo_glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, info->chunk->vbo_index);
    bo_glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, corners * 4 * sizeof(unsigned int), 0, WATER_VBO_MODE);
    info->indices_p = (unsigned int*)bo_glMapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
  }
  else
#endif // GL_ARB_vertex_buffer_object
  {
    // Use plain arrays. They're stored in water chunks
    if(info->chunk->lastdetail != info->detail)
    {
      // Reallocate arrays
      delete[] info->chunk->vertices;
      delete[] info->chunk->normals;
      info->chunk->vertices = new BoVector3[info->chunkcornerw * info->chunkcornerh];
      info->chunk->normals = new BoVector3[info->chunkcornerw * info->chunkcornerh];
      if(mEnableBumpmapping)
      {
        delete[] info->chunk->tangentlight;
        delete[] info->chunk->tangentlight4;
        delete[] info->chunk->halfvector;
        info->chunk->tangentlight = new BoVector3[info->chunkcornerw * info->chunkcornerh];
        info->chunk->tangentlight4 = new BoVector4[info->chunkcornerw * info->chunkcornerh];
        info->chunk->halfvector = new BoVector3[info->chunkcornerw * info->chunkcornerh];
      }
      else if(mEnableTranslucency)
      {
        delete[] info->chunk->colors;
        info->chunk->colors = new BoVector4[info->chunkcornerw * info->chunkcornerh];
      }
      delete[] info->chunk->indices;
      info->chunk->indices = new unsigned int[(info->chunkcornerw - 1) * (info->chunkcornerh - 1) * 4];
      // Don't reset chunk->lastdetail yet, it will be done ~20 lines below
    }
    // Update pointers in RenderInfo. Note that some of those pointers may be
    //  invalid, but those shouldn't be used anyway.
    info->vertices_p = info->chunk->vertices;
    info->normals_p = info->chunk->normals;
    info->tangentlights_p = info->chunk->tangentlight;
    info->tangentlights_p4 = info->chunk->tangentlight4;
    info->halfvectors_p = info->chunk->halfvector;
    info->colors_p = info->chunk->colors;
    info->indices_p = info->chunk->indices;
  }

  // Now resize cell* arrays if necessary. These are plaint arrays even if vbos
  //  are enabled, beause they aren't used for rendering.
  if(info->chunk->lastdetail != info->detail)
  {
    // Init/resize them only if they are actually needed
    if(!info->flat && !info->useDerivations)
    {
      delete[] info->chunk->cellnormals;
      delete[] info->chunk->celltangentlight;
      delete[] info->chunk->cellhalfvector;
      info->chunk->cellnormals = new BoVector3[info->chunkcellw * info->chunkcellh];
      info->chunk->celltangentlight = new BoVector3[info->chunkcellw * info->chunkcellh];
      info->chunk->cellhalfvector = new BoVector3[info->chunkcellw * info->chunkcellh];
    }
    info->chunk->lastdetail = info->detail;
  }

  // Reset indices count
  info->chunk->indices_count = 0;
}

void BoWaterManager::uninitDataBuffersForStorage(RenderInfo* info)
{
  // If VBOs are used, we need to unmap them
#ifdef GL_ARB_vertex_buffer_object
  if(mEnableVBO)
  {
    // Unmap vbos
    bo_glBindBufferARB(GL_ARRAY_BUFFER_ARB, info->chunk->vbo_vertex);
    if(!bo_glUnmapBufferARB(GL_ARRAY_BUFFER_ARB))
    {
      boError() << k_funcinfo << "can't unmap vertices' vbo!" << endl;
    }
    bo_glBindBufferARB(GL_ARRAY_BUFFER_ARB, info->chunk->vbo_normal);
    if(!bo_glUnmapBufferARB(GL_ARRAY_BUFFER_ARB))
    {
      boError() << k_funcinfo << "can't unmap normals' vbo!" << endl;
    }
    if(mEnableBumpmapping)
    {
      if(mEnableTranslucency)
      {
        bo_glBindBufferARB(GL_ARRAY_BUFFER_ARB, info->chunk->vbo_tangentlight4);
      }
      else
      {
        bo_glBindBufferARB(GL_ARRAY_BUFFER_ARB, info->chunk->vbo_tangentlight);
      }
      if(!bo_glUnmapBufferARB(GL_ARRAY_BUFFER_ARB))
      {
        boError() << k_funcinfo << "can't unmap tangentlights' vbo!" << endl;
      }
      bo_glBindBufferARB(GL_ARRAY_BUFFER_ARB, info->chunk->vbo_halfvector);
      if(!bo_glUnmapBufferARB(GL_ARRAY_BUFFER_ARB))
      {
        boError() << k_funcinfo << "can't unmap halfvectors' vbo!" << endl;
      }
    }
    else if(mEnableTranslucency)
    {
      bo_glBindBufferARB(GL_ARRAY_BUFFER_ARB, info->chunk->vbo_color);
      if(!bo_glUnmapBufferARB(GL_ARRAY_BUFFER_ARB))
      {
        boError() << k_funcinfo << "can't unmap colors' vbo!" << endl;
      }
    }
    bo_glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, info->chunk->vbo_index);
    if(!bo_glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB))
    {
      boError() << k_funcinfo << "can't unmap indices' vbo!" << endl;
    }
    bo_glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);  // Disable VBO
    bo_glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);  // Disable index VBO
  }
#endif // GL_ARB_vertex_buffer_object
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
      if(mEnableReflections && mLocalPlayerIO->isFogged(posx, posy))
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

void BoWaterManager::setupEnvMapTexture(GLenum unit)
{
#ifdef GL_ARB_texture_cube_map
  // Activate given texture unit
  glActiveTextureARB(unit);

  // Bind envmap texture
  mEnvMap->enable();
  mEnvMap->bind();

  // Load texture matrix
  glMatrixMode(GL_TEXTURE);
  glPushMatrix();
  glLoadMatrixf(mInverseModelview.data());

  // Automatically generate texture coords for reflection cubemap
  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_ARB);
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_ARB);
  glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_ARB);
  glEnable(GL_TEXTURE_GEN_S);
  glEnable(GL_TEXTURE_GEN_T);
  glEnable(GL_TEXTURE_GEN_R);

#ifdef GL_EXT_texture_lod_bias
  // Make texture a bit blurred if it's supported
  if(mSupports_texlod)
  {
    glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, mReflectionSharpness);
  }
#endif

#ifdef GL_ARB_texture_env_combine
  // Interpolate between water texture and envmap
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
  glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_INTERPOLATE_ARB);
  glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
  glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PREVIOUS_ARB);
  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);
  glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_ARB, GL_CONSTANT_ARB);
  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_ARB, GL_SRC_COLOR);
  float reflectioncolor[] = { mReflectionStrength, mReflectionStrength, mReflectionStrength, 1.0f };
  glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, reflectioncolor);
#endif // GL_ARB_texture_env_combine
#endif // GL_ARB_texture_cube_map
}

void BoWaterManager::setupBumpMapTexture(GLenum unit)
{
  glActiveTextureARB(unit);

  if(mEnableAnimBumpmaps)
  {
    // TODO: 3d texture support!
#ifdef GL_EXT_texture3D
    /*if(mSupports_texture3d)
    {
      waterbump_anim_3d->enable();
      waterbump_anim_3d->bind();
    }
    else*/
#endif
    {
      mWaterAnimBump.at((int)mWaterAnimBumpCurrent)->enable();
      mWaterAnimBump.at((int)mWaterAnimBumpCurrent)->bind();
    }
  }
  else
  {
    mWaterBump->enable();
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

#ifdef GL_ARB_texture_env_combine
#ifdef GL_ARB_texture_env_dot3
  // Tangent space light vector will be encoded in primary color.
  // Do dot3 between primary color (light vector) and normal. (N.L)
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
  glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_DOT3_RGB_ARB);
  glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PRIMARY_COLOR_ARB);
  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
  glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE);
  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);
  if(mEnableTranslucency)
  {
    // If we use translucency, then surface's alpha is in alpha component of
    //  primary color. Replace surface's alpha with it.
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_REPLACE);
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_PRIMARY_COLOR_ARB);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, GL_SRC_ALPHA);
  }
#endif
#endif
}

void BoWaterManager::setupDiffuseTexture(GLenum unit)
{
  glActiveTextureARB(unit);

  mWaterTex->enable();
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
  // Attributes will be popped at the end of render().
  glPushAttrib(/*GL_LIGHTING_BIT | GL_TEXTURE_BIT | GL_ENABLE_BIT | GL_TRANSFORM_BIT | GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT*/GL_ALL_ATTRIB_BITS);
  if(mEnableReflections && !mEnableBumpmapping)
  {
    // If we have environment mapping but no bumpmapping then we have:
    //  * tex0: water texture
    //  * tex1: environment cubemap

    setupEnvMapTexture(GL_TEXTURE1_ARB);

    setupDiffuseTexture(GL_TEXTURE0_ARB);
  }
  else if(mEnableBumpmapping)
  {
    // If we have bumpmapping then we have:
    //  * tex0: water bumpmap
    //  * tex1: water texture
    // THIS IS ONLY FOR PASS1 !!!

    glDisable(GL_LIGHTING);

    setupBumpMapTexture(GL_TEXTURE0_ARB);

    setupDiffuseTexture(GL_TEXTURE1_ARB);

#ifdef GL_ARB_texture_env_combine
    // Water surface texture will be modulated with the result of previous
    //  texture operation (N.L).
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);
    if(mEnableTranslucency)
    {
      // Use alpha from previous texture operation.
      // TODO: maybe replace alpha initially here (not in texture 0)?
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
      glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_REPLACE);
      glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_PREVIOUS_ARB);
      glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, GL_SRC_ALPHA);
    }
#endif
    glActiveTextureARB(GL_TEXTURE0_ARB);
  }
  else
  {
    // No reflections, no bumpmapping. Boring :-)
    setupDiffuseTexture(GL_TEXTURE0_ARB);
  }

  if(!mEnableBumpmapping)
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
  if(mEnableReflections && !mEnvMap)
  {
    mEnvMap = new BoTexture(path + "sky-%1.jpg",
        BoTexture::FilterLinearMipmapLinear | BoTexture::FormatAuto, BoTexture::TextureCube);
  }

  // Create bumpmaps if necessary
  if(mEnableBumpmapping)
  {
    if(mEnableAnimBumpmaps && (mWaterAnimBump.count() == 0))
    {
      // Load animated water bumpmap textures
      QDir d(path);
      QStringList files = d.entryList("water-animbumpmap-*.png", QDir::Files, QDir::Name);
      for(QStringList::Iterator it = files.begin(); it != files.end(); it++)
      {
        BoTexture* tex = new BoTexture(path + *it, BoTexture::Terrain);
        mWaterAnimBump.append(tex);
      }
      mWaterAnimBumpCurrent = 0.0f;
    }
    else if(!mWaterBump)
    {
      mWaterBump = new BoTexture(path + "water-bumpmap.png", BoTexture::Terrain);
    }
  }
}
