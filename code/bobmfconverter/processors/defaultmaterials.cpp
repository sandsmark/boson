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


#include "defaultmaterials.h"

#include "debug.h"
#include "lod.h"
#include "model.h"
#include "mesh.h"
#include "material.h"
#include "texture.h"
#include "bo3dtools.h"

#include <qvaluelist.h>
#include <qvaluevector.h>


DefaultMaterials::DefaultMaterials() : Processor()
{
  setName("DefaultMaterials");
  mDefaultTexture = 0;
  mDefaultMaterial = 0;
}

DefaultMaterials::~DefaultMaterials()
{
}

bool DefaultMaterials::process()
{
  if(model() == 0)
  {
    boError() << k_funcinfo << "NULL model!" << endl;
    return false;
  }
  if(lod() == 0)
  {
    boError() << k_funcinfo << "NULL LOD!" << endl;
    return false;
  }

  // First make sure every material has a texture
  for(unsigned int i = 0; i < model()->materialCount(); i++)
  {
    Material* m = model()->material(i);
    if(!m->texture())
    {
      boDebug() << k_funcinfo << "Giving default texture to material " << m->name() << endl;
      m->setTexture(defaultTexture());
    }
  }

  // Then make sure every mesh has a material
  for(unsigned int i = 0; i < lod()->meshCount(); i++)
  {
    Mesh* mesh = lod()->mesh(i);
    if(!mesh->material())
    {
      boDebug() << k_funcinfo << "Giving default material to mesh " << mesh->name() << (mesh->isTeamColor() ? " (tc)" : "-") << endl;
      mesh->setMaterial(defaultMaterial());
    }
  }

  return true;
}

Texture* DefaultMaterials::defaultTexture()
{
  if(!mDefaultTexture)
  {
    mDefaultTexture = model()->getTexture("default.jpg");
  }
  return mDefaultTexture;
}

Material* DefaultMaterials::defaultMaterial()
{
  if(!mDefaultMaterial)
  {
    mDefaultMaterial = new Material;

    mDefaultMaterial->setName("BMFDefault");
    mDefaultMaterial->setTexture(defaultTexture());

    mDefaultMaterial->setAmbient(BoVector4Float(0.8, 0.8, 0.8, 1.0));
    mDefaultMaterial->setDiffuse(BoVector4Float(0.8, 0.8, 0.8, 1.0));
    mDefaultMaterial->setSpecular(BoVector4Float(0.2, 0.2, 0.2, 1.0));
    mDefaultMaterial->setShininess(10);

    model()->addMaterial(mDefaultMaterial);
  }
  return mDefaultMaterial;
}

/*
 * vim: et sw=2
 */
