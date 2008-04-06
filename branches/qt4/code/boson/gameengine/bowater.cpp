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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "bowater.h"

#include "../../bomemory/bodummymemory.h"
#include "bosonmap.h"
#include "cell.h"
#include "bodebug.h"
#include "../bo3dtools.h"

#include <qrect.h>
#include <qpoint.h>
#include <q3ptrlist.h>
#include <q3valuelist.h>
#include <qdom.h>

#include <math.h>

#include <stdio.h>


/*****  BoWaterData  *****/

/**
 * Holds map-specifies water data such as lakes
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BoWaterData
{
  public:
    BoWaterData();
    ~BoWaterData();

    bool isValidCorner(int x, int y) const
    {
      if(x < 0 || y < 0)
      {
        return false;
      }
      if (x >= width || y >= height)
      {
        return false;
      }
      return true;
    }


    Q3PtrList<BoLake> lakes;

    BosonMap* map;
    // Size of map in _corners_ _not_ in cells!
    int width, height;
    // Size in cells
    int cellWidth, cellHeight;

    bool* underwater;
};

BoWaterData::BoWaterData()
{
  map = 0;
  underwater = 0;
  lakes.setAutoDelete(true);

  width = 0;
  height = 0;
  cellWidth = 0;
  cellHeight = 0;
}

BoWaterData::~BoWaterData()
{
  lakes.setAutoDelete(true);
  lakes.clear();
  delete[] underwater;
}



/*****  BoLake  *****/

BoLake::BoLake(float _level)
{
  init(_level);
}

BoLake::~BoLake()
{
  delete[] corners;
}

void BoLake::init(float _level)
{
  level = _level;
  minx = 0; miny = 0;
  maxx = 0; maxy = 0;
  loadedminx = 0; loadedminy = 0; loadedmaxx = 0; loadedmaxy = 0;
  originx = -1; originy = -1;
  cornercount = 0;
  corners = 0;
}

void BoLake::findWater(BoWaterManager* manager, int x, int y, const QRect& searcharea)
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

  Q3ValueList<QPoint> open;
  Q3ValueList<QPoint> closed;
  QPoint n, n2;

  open.push_back(QPoint(x, y));
  manager->setUnderwater(x, y, true);

  while(!open.empty())
  {
    n = open.back();
    open.pop_back();
    closed.push_back(n);
    cornercount++;

    minx = qMin(minx, n.x());
    maxx = qMax(maxx, n.x());
    miny = qMin(miny, n.y());
    maxy = qMax(maxy, n.y());

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
  minx = qMax(minx - 1, searcharea.left());
  maxx = qMin(maxx + 1, searcharea.right());
  miny = qMax(miny - 1, searcharea.top());
  maxy = qMin(maxy + 1, searcharea.bottom());

  // Mark all corners that we have
  int size = (maxx - minx + 1) * (maxy - miny + 1);
  if (size < 0 || size > 4194304)
  {
    boError() << k_funcinfo << "invalid size: " << size << endl;
    return;
  }
  corners = new bool[size];
  // TODO: can we use memset() here?
  for(int i = 0; i < size; i++)
  {
    corners[i] = false;
  }
  // Go through every corners and add this _and it's neighbors_. We need
  //  neighbors for shores.
  for(Q3ValueList<QPoint>::iterator it = closed.begin(); it != closed.end(); it++)
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
}

bool BoLake::hasAnyCorner(float x1, float y1, float x2, float y2)
{
  if(hasCorner(qMax((int)x1, minx), qMax((int)y1, miny)))
  {
    return true;
  }

  for(int celly = qMax((int)y1, miny); celly <= qMin((int)y2, maxy); celly++)
  {
    for(int cellx = qMax((int)x1, minx); cellx <= qMin((int)x2, maxx); cellx++)
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

BoWaterManager::BoWaterManager()
{
  mData = 0;
}

BoWaterManager::~BoWaterManager()
{
  delete mData;
}

bool BoWaterManager::loadFromXML(const QDomElement& root)
{
  if(!mData)
  {
    BO_NULL_ERROR(mData);
    return false;
  }
  if(mData->width < 0 || mData->height < 0 || mData->width * mData->height > 4194304)
  {
    boError() << k_funcinfo << "invalid data dimensions" << endl;
    return false;
  }
  // Init some data structures
  delete[] mData->underwater;
  mData->underwater = new bool[mData->width * mData->height];
  for (int y = 0; y < mData->height; y++)
  {
    for (int x = 0; x < mData->width; x++)
    {
      mData->underwater[y * mData->width + x] = false;
    }
  }

  bool ret = true;
  QDomNodeList list = root.elementsByTagName(QString::fromLatin1("Lake"));

  for (int i = 0; i < list.count(); i++)
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

    BoLake* bolake = new BoLake(level);
    bolake->findWater(this, originx, originy, QRect(QPoint(minx, miny), QPoint(maxx, maxy)));
    mData->lakes.append(bolake);
  }

  initCellMaps();

  return ret;
}

bool BoWaterManager::saveToXML(QDomElement& root)
{
  if(!mData)
  {
    // This is valid - happens when creating a new map
    return true;
  }

  QDomDocument doc = root.ownerDocument();
  Q3PtrListIterator<BoLake> it(mData->lakes);
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
  BO_CHECK_NULL_RET(mData);
  // Size of map in cells
  mData->cellWidth = mData->map->width();
  mData->cellHeight = mData->map->height();

  // Set both arrays to true first
  for (int y = 0; y < mData->cellHeight; y++)
  {
    for (int x = 0; x < mData->cellWidth; x++)
    {
      Cell* c = mData->map->cell(x, y);
      if(!c)
      {
        BO_NULL_ERROR(c);
        continue;
      }
      c->setIsWater(false);
    }
  }

  // Go through all lakes
  Q3PtrListIterator<BoLake> it(mData->lakes);
  for(; it.current(); ++it)
  {
    BoLake* lake = it.current();
    for (int y = lake->miny; y < lake->maxy; y++)
    {
      for (int x = lake->minx; x < lake->maxx; x++)
      {
        bool iswater = false;

        if((lake->hasCorner(x, y)) || (lake->hasCorner(x + 1, y)) ||
            (lake->hasCorner(x, y + 1)) || (lake->hasCorner(x + 1, y + 1)))
        {
          iswater = true;
        }

        if(iswater)
        {
          Cell* c = mData->map->cell(x, y);
          if(!c)
          {
            BO_NULL_ERROR(c);
            continue;
          }
          c->setIsWater(true);
        }
      }
    }
  }
}

void BoWaterManager::setMap(BosonMap* map)
{
  delete mData;
  mData = new BoWaterData;

  mData->map = map;
  mData->width = map->width() + 1;
  mData->height = map->height() + 1;
}

float BoWaterManager::groundHeight(int x, int y) const
{
  if(!mData)
  {
    BO_NULL_ERROR(mData);
    return 0.0f;
  }
  if(!mData->map)
  {
    BO_NULL_ERROR(mData->map);
    return 0.0f;
  }
  if(!mData->isValidCorner((int)x, (int)y))
  {
    return 0.0f;
  }
  return mData->map->heightAtCorner(x, y);
}

float BoWaterManager::groundHeightAt(float x, float y) const
{
  if(!mData)
  {
    BO_NULL_ERROR(mData);
    return 0.0f;
  }
  if(!mData->map)
  {
    BO_NULL_ERROR(mData->map);
    return 0.0f;
  }
  return groundHeight((int)x, (int)y);
}

float BoWaterManager::waterDepthAtCorner(int x, int y)
{
  if(!mData)
  {
    BO_NULL_ERROR(mData);
    return 0.0f;
  }
  if(!mData->isValidCorner(x, y))
  {
    return 0.0f;
  }
  if(!underwater(x, y))
  {
    return 0.0f;
  }

  Q3PtrListIterator<BoLake> it(mData->lakes);
  for(; it.current(); ++it)
  {
    BoLake* lake = it.current();
    if(x >= lake->minx && x <= lake->maxx && y >= lake->miny && y <= lake->maxy)
    {
      if(lake->hasCorner(x, y))
      {
        return qMax(0.0f, lake->level - groundHeight(x, y));
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
  if(!mData)
  {
    BO_NULL_ERROR(mData);
    return false;
  }
  if(!mData->isValidCorner(x, y))
  {
    return false;
  }
  return mData->underwater[y * mData->width + x];
}

void BoWaterManager::setUnderwater(int x, int y, bool underwater)
{
  BO_CHECK_NULL_RET(mData);
  if(!mData->isValidCorner(x, y))
  {
    boError() << k_funcinfo << "not a valid corner " << x << " " << y << endl;
    return;
  }
  mData->underwater[y * mData->width + x] = underwater;
}

const Q3PtrList<BoLake>* BoWaterManager::lakes() const
{
  if(!mData)
  {
    BO_NULL_ERROR(mData);
    return 0;
  }
  return &mData->lakes;
}


/*
 * vim: et sw=2
 */
