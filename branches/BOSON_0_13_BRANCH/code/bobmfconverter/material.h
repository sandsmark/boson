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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef MATERIAL_H
#define MATERIAL_H


#include <qstring.h>

#include "bo3dtools.h"

class Texture;


class Material
{
  public:
    Material();

    int id() const  { return mId; }
    void setId(int id)  { mId = id; }

    void setName(const QString& name)  { mName = name; }
    const QString& name() const  { return mName; }

    void setAmbient(const BoVector4Float& amb)  { mAmbient = amb; }
    const BoVector4Float& ambient() const  { return mAmbient; }

    void setDiffuse(const BoVector4Float& dif)  { mDiffuse = dif; }
    const BoVector4Float& diffuse() const  { return mDiffuse; }

    void setSpecular(const BoVector4Float& spec)  { mSpecular = spec; }
    const BoVector4Float& specular() const  { return mSpecular; }

    void setEmissive(const BoVector4Float& em)  { mEmissive = em; }
    const BoVector4Float& emissive() const  { return mEmissive; }

    void setShininess(float s)  { mShininess = s; }
    float shininess() const  { return mShininess; }

    void setTexture(Texture* t)  { mTexture = t; }
    Texture* texture() const  { return mTexture; }

  private:
    int mId;

    QString mName;

    BoVector4Float mAmbient;
    BoVector4Float mDiffuse;
    BoVector4Float mSpecular;
    BoVector4Float mEmissive;
    float mShininess;

    Texture* mTexture;
};

#endif //MATERIAL_H
