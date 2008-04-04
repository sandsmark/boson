/*
    This file is part of the Boson game
    Copyright (C) 2006 Rivo Laks (rivolaks@hot.ee)

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

#ifndef DEFAULTMATERIALS_H
#define DEFAULTMATERIALS_H


#include "processor.h"

class Material;
class Texture;


class DefaultMaterials : public Processor
{
  public:
    DefaultMaterials();
    virtual ~DefaultMaterials();

    virtual bool process();

  protected:
    Texture* defaultTexture();
    Material* defaultMaterial();

    Texture* mDefaultTexture;
    Material* mDefaultMaterial;
};


#endif //DEFAULTMATERIALS_H
