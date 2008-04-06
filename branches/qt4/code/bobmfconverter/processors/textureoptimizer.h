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

#ifndef TEXTUREOPTIMIZER_H
#define TEXTUREOPTIMIZER_H


#include "processor.h"

#include <qstring.h>
//Added by qt3to4:
#include <Q3ValueList>

class Model;
class LOD;
class Texture;
class Mesh;
class QImage;
template<class T> class BoVector2;
typedef BoVector2<float> BoVector2Float;
template<class T> class BoVector3;
typedef BoVector3<float> BoVector3Float;
template<class T> class Q3ValueList;


class TextureOptimizer : public Processor
{
  public:
    TextureOptimizer();
    virtual ~TextureOptimizer();

    virtual bool process();

    void setCombinedTexSize(unsigned int size)  { mTexSize = size; }
    unsigned int combinedTexSize() const  { return mTexSize; }

    void setCombinedTexFilename(const QString& name)  { mTexFilename = name; }
    const QString& combinedTexFilename() const  { return mTexFilename; }
    void setCombinedTexPath(const QString& name)  { mTexPath = name; }
    const QString& combinedTexPath() const  { return mTexPath; }


  protected:
    class TextureInfo;

    bool normalizeTexCoords(Mesh* m);
    void calculateTotalUsedArea(Texture* t);
    Texture* combineAllTextures();

    float calculateArea(const BoVector3Float& v0, const BoVector3Float& v1, const BoVector3Float& v2);
    float calculateArea(const BoVector2Float& v0, const BoVector2Float& v1, const BoVector2Float& v2);

    void copyTextureToCombinedTexture(TextureOptimizer::TextureInfo* src, QImage* dst, int dstx, int dsty, int w, int h, int combinedsize);
    void modifyTexCoordsForCombinedTexture(TextureOptimizer::TextureInfo* tinfo);
    void replaceTextureInMaterials(Texture* replace, Texture* with);


  private:
    unsigned int mTexSize;
    QString mTexFilename;
    QString mTexPath;
};


#endif //TEXTUREOPTIMIZER_H
