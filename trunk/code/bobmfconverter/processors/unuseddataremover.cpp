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


#include "unuseddataremover.h"

#include "debug.h"
#include "model.h"
#include "lod.h"
#include "frame.h"
#include "material.h"
#include "mesh.h"
#include "texture.h"
#include "bo3dtools.h"

#include <qvaluevector.h>
#include <qdict.h>


UnusedDataRemover::UnusedDataRemover(Model* m, LOD* l) : Processor(m, l)
{
  mProcessAll = true;
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

  QValueVector<Mesh*> validmeshes;
  unsigned int removedcount = 0;
  for(unsigned int i = 0; i < lod()->meshCount(); i++)
  {
    Mesh* m = lod()->mesh(i);
    if(isMeshValid(m))
    {
      validmeshes.append(m);
    }
    else
    {
      // Make sure the mesh isn't in any nodes in any frames
      removeMeshFromFrames(m);
      // Delete the mesh
      delete m;
      removedcount++;
    }
  }

  if(removedcount == 0)
  {
    return true;
  }

  unsigned int oldcount = lod()->meshCount();
  lod()->setMeshes(validmeshes);

  boDebug() << k_funcinfo << "Removed " << removedcount << " meshes of " << oldcount << endl;
  return true;
}

bool UnusedDataRemover::processMaterials()
{
  // Go through all materials and see if they're used in any mesh
  QValueVector<Material*> validmaterials;
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
  QDict<Texture> validtextures;
  unsigned int removedcount = 0;
  QDictIterator<Texture> it(*model()->texturesDict());
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

void UnusedDataRemover::removeMeshFromFrames(Mesh* m)
{
  unsigned int replaces = 0;
  for(unsigned int i = 0; i < lod()->frameCount(); i++)
  {
    if(i == baseFrame())
    {
      continue;
    }
    Frame* f = lod()->frame(i);
    QValueVector<Mesh*> newmeshes;
    QValueVector<BoMatrix*> newmatrices;
    bool havetoreplace = false;
    for(unsigned int j = 0; j < f->nodeCount(); j++)
    {
      if(f->mesh(j) == m)
      {
        havetoreplace = true;
        replaces++;
      }
      else
      {
        newmeshes.append(f->mesh(j));
        newmatrices.append(f->matrix(j));
      }
    }
    if(havetoreplace)
    {
      // Create temporary Mesh*/BoMatrix* arrays
      Mesh** meshes = new Mesh*[newmeshes.count()];
      BoMatrix** matrices = new BoMatrix*[newmeshes.count()];
      for(unsigned int j = 0; j < newmeshes.count(); j++)
      {
        meshes[j] = newmeshes[j];
        matrices[j] = newmatrices[j];
      }
      // Replace arrays in the frame
      f->replaceNodes(matrices, meshes, newmeshes.count());
    }
  }

  if(replaces)
  {
    boWarning() << k_funcinfo << "Removed " << replaces << " nodes containing mesh '" <<
        m->name() << "'" << endl;
  }
}

