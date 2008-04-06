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

#ifndef BOWATER_H
#define BOWATER_H


#include "../bo3dtools.h"

#include <q3ptrlist.h>


class BosonMap;

class QRect;
class QDomElement;

class BoWaterManager;
class BoWaterData;


/**
 * @short Lake class
 *
 * BoLake represents a single lake in Boson.
 * Every lake has certain properties, such as it's water level, min/max
 *  coordinates and cells it consists of.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BoLake
{
  public:
    /**
     * Constructs a lake with specified level
     **/
    BoLake(float _level);

    ~BoLake();

    /**
     * Finds all corners that belong to this lake.
     * It performs a flood-fill search, starting from point (x; y) and takes
     *  all corners whose height is less than lake's level.
     * Search will be kept inside searcharea (inclusive).
     **/
    void findWater(BoWaterManager* manager, int x, int y, const QRect& searcharea);

    /**
     * @return Whether this lake has given corner.
     **/
    bool hasCorner(int x, int y)
    {
      int index = (y - miny) * (maxx - minx + 1) + (x - minx);
      int size = (maxx - minx + 1) * (maxy - miny + 1);
      if(index >= size)
      {
        return false;
      }
      return corners[index];
    }
    /**
     * @return Whether this lake has any cells in given rectangle.
     **/
    bool hasAnyCorner(float x1, float y1, float x2, float y2);

    // In corner coordinates. Note that they are inclusive, i.e. point
    //  (maxx, maxy) is in the lake.
    int minx, miny;
    int maxx, maxy;
    // Those are same as previous ones, except for that they aren't
    //  automatically changed when lake is loaded.
    // This is necessary to make saved xml look exactly like loaded one.
    int loadedminx, loadedminy, loadedmaxx, loadedmaxy;
    // Water level of the lake
    float level;
    // Origin point of lake, used for saving
    int originx, originy;

    // Number of corners that this lake has. 1x1 lake has 4 corners.
    int cornercount;

    bool* corners;


  protected:
    void init(float _level);
};


/**
 * @short Water manager
 *
 * Takes care of loading/saving/rendering/etc of water
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BoWaterManager
{
  public:
    BoWaterManager();
    ~BoWaterManager();

    bool loadFromXML(const QDomElement& root);
    bool saveToXML(QDomElement& root);

    const Q3PtrList<BoLake>* lakes() const;

    void initDefaultWaterLevel(float level);
    void initCellMaps();  // FIXME: name

    void setMap(BosonMap* map);
    float groundHeight(int x, int y) const;
    float groundHeightAt(float x, float y) const;
    float waterDepthAtCorner(int x, int y);

    bool underwater(int x, int y);
    void setUnderwater(int x, int y, bool free);

  private:
    BoWaterData* mData;
};

#endif // BOWATER_H
/*
 * vim: et sw=2
 */

