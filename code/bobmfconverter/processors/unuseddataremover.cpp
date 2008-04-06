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


#include "unuseddataremover.h"

#include "debug.h"
#include "model.h"
#include "lod.h"
#include "frame.h"
#include "material.h"
#include "mesh.h"
#include "texture.h"
#include "bo3dtools.h"

#include <q3valuevector.h>
#include <q3dict.h>


UnusedDataRemover::UnusedDataRemover() : Processor()
{
  mProcessAll = true;
  setName("UnusedDataRemover");
}

UnusedDataRemover::~UnusedDataRemover()
{
}

bool UnusedDataRemover::process()
{
  if(lod() == 0)
  {
    boError() << k_funcinfo << "NULL LOD!" << endl;
    return false;
  }

  if(!processMeshes())
  {
    return false;
  }
  if(processMaterialsAndTextures())
  {
    if(!processMaterials())
    {
      return false;
    }
    if(!processTextures())
    {
      return false;
    }
  }

  return true;
}

bool UnusedDataRemover::processMeshes()
{
  // Go through all meshes and see if they're used in any frame/node

  Q3ValueVector<Mesh*> validMeshes;
  unsigned int removedCount = 0;
  for(unsigned int i = 0; i < lod()->meshCount(); i++)
  {
    Mesh* m = lod()->mesh(i);
    if(isMeshValid(m))
    {
      validMeshes.append(m);
    }
    else
    {
      removedCount++;
    }
  }

  if(removedCount == 0)
  {
    return true;
  }

  unsigned int oldCount = lod()->meshCount();
  lod()->removeAllMeshesBut(validMeshes);

  boDebug() << k_funcinfo << "Removed " << removedCount << " meshes of " << oldCount << endl;
  return true;
}

bool UnusedDataRemover::processMaterials()
{
  // Go through all materials and see if they're used in any mesh
  Q3ValueVector<Material*> validmaterials;
  unsigned int removedcount = 0;
  for(unsigned int i = 0; i < model()->materialCount(); i++)
  {
    if(isMaterialValid(model()->material(i)))
    {
      validmaterials.append(model()->material(i));
    }
    else
    {
      delete model()->material(i);
      removedcount++;
    }
  }

  if(removedcount == 0)
  {
    return true;
  }

  unsigned int oldcount = model()->materialCount();
  model()->setMaterials(validmaterials);

  boDebug() << k_funcinfo << "Removed " << removedcount << " materials of " << oldcount << endl;
  return true;
}

bool UnusedDataRemover::processTextures()
{
  // Go through all textures and see if they're used in any material
  Q3Dict<Texture> validtextures;
  unsigned int removedcount = 0;
  Q3DictIterator<Texture> it(*model()->texturesDict());
  while(it.current())
  {
    if(it.current()->filename().isEmpty())
    {
      // Empty filenames are NOT valid.
      // Remove texture from all materials
      for(unsigned int i = 0; i < model()->materialCount(); i++)
      {
        if(model()->material(i)->texture() == it.current())
        {
          model()->material(i)->setTexture(0);
        }
      }
      delete it.current();
      removedcount++;
      boDebug() << "    " << "Removed texture with empty filename" << endl;
    }
    else if(isTextureValid(it.current()))
    {
      validtextures.insert(it.current()->filename(), it.current());
    }
    else
    {
      delete it.current();
      removedcount++;
    }
    ++it;
  }

  if(removedcount == 0)
  {
    return true;
  }

  unsigned int oldcount = model()->texturesDict()->count();
  model()->setTextures(validtextures);

  boDebug() << k_funcinfo << "Removed " << removedcount << " textures of " << oldcount << endl;
  return true;
}

bool UnusedDataRemover::isMeshValid(Mesh* m)
{
  Frame* f = lod()->frame(baseFrame());
  for(unsigned int j = 0; j < f->nodeCount(); j++)
  {
    if(f->mesh(j) == m)
    {
      return true;
    }
  }

  return false;
}

bool UnusedDataRemover::isMaterialValid(Material* m)
{
  for(unsigned int i = 0; i < model()->lodCount(); i++)
  {
    LOD* l = model()->lod(i);
    for(unsigned int j = 0; j < l->meshCount(); j++)
    {
      if(l->mesh(j)->material() == m)
      {
        return true;
      }
    }
  }

  return false;
}

bool UnusedDataRemover::isTextureValid(Texture* t)
{
  for(unsigned int i = 0; i < model()->materialCount(); i++)
  {
    if(model()->material(i)->texture() == t)
    {
      return true;
    }
  }

  return false;
}

/*
 * vim: et sw=2
 */
