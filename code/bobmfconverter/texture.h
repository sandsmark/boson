/*
    This file is part of the Boson game
    Copyright (C) 2005 Rivo Laks (rivolaks@hot.ee)

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

#ifndef TEXTURE_H
#define TEXTURE_H


#include <qstring.h>
#include <qstringlist.h>

class QImage;


class Texture
{
  public:
    Texture(const QString& filename);

    int id() const  { return mId; }
    void setId(int id)  { mId = id; }

    const QString& filename()  { return mFilename; }
    void setFilename(const QString& name)  { mFilename = name; }

    float totalUsedTexArea() const  { return mTotalUsedTexArea; }
    void setTotalUsedTexArea(float u)  { mTotalUsedTexArea = u; }
    float totalUsedFaceArea() const  { return mTotalUsedFaceArea; }
    void setTotalUsedFaceArea(float u)  { mTotalUsedFaceArea = u; }

    float usedAreaMinX() const  { return mUsedMinX; }
    float usedAreaMinY() const  { return mUsedMinY; }
    float usedAreaMaxX() const  { return mUsedMaxX; }
    float usedAreaMaxY() const  { return mUsedMaxY; }
    void setUsedArea(float minx, float miny, float maxx, float maxy)
    {
      mUsedMinX = minx;  mUsedMinY = miny;  mUsedMaxX = maxx;  mUsedMaxY = maxy;
    }

    int width() const;
    int height() const;

    bool load();

    QImage* image() const  { return mImage; }

    bool hasTransparency()  { return mHasTransparency; }


    static void addTexturePath(const QString& path);
    static const QStringList& texturePaths();


  protected:
    bool checkTexturePath(const QString& dir, QString& filepath);


  private:
    int mId;

    float mTotalUsedTexArea;
    float mTotalUsedFaceArea;
    float mUsedMinX;
    float mUsedMinY;
    float mUsedMaxX;
    float mUsedMaxY;

    QString mFilename;

    QImage* mImage;
    bool mHasTransparency;


    static QStringList mTexturePaths;
};

#endif //TEXTURE_H
