/*
    This file is part of the Boson game
    Copyright (C) 2005 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "model.h"

#include "lod.h"
#include "material.h"
#include "loader.h"
#include "texture.h"
#include "saver.h"
#include "mesh.h"
#include "frame.h"


Model::Model()
{
  mRadius = 0.0f;
}

bool Model::load(const QString& file)
{
  createBaseLOD();

  Loader* l = Loader::createLoader(this, baseLOD(), file);
  bool ret = l->load();

  delete l;
  return ret;
}

bool Model::save(const QString& file)
{
  Saver s(this, file);
  return s.save();
}

void Model::createBaseLOD()
{
  LOD* l = new LOD;
  mLODs.append(l);
}

unsigned int Model::addMaterial(Material* m)
{
  mMaterials.append(m);
  return mMaterials.count() - 1;
}

void Model::setMaterials(const QValueVector<Material*>& materials)
{
  mMaterials = materials;
}

void Model::setTextures(const QDict<Texture>& textures)
{
  mTextures = textures;
}

Texture* Model::getTexture(const QString& filename)
{
  Texture* t = mTextures[filename];
  if(!t)
  {
    t = new Texture(filename);
    mTextures.insert(filename, t);
  }

  return t;
}

void Model::addTexture(Texture* t)
{
  mTextures.insert(t->filename(), t);
}

void Model::prepareForSaving(unsigned int baseframe)
{
  updateIds();
  updateRadius(baseframe);

  // Create arrays for meshes
  /*for(unsigned int i = 0; i < lodCount(); i++)
  {
    LOD* l = lod(i);
    for(unsigned int j = 0; j < l->meshCount(); j++)
    {
      l->mesh(j)->createArrays();
    }
  }*/
}

void Model::updateIds()
{
  // Textures
  QDictIterator<Texture> it(mTextures);
  for(unsigned int i = 0; it.current(); ++it, i++)
  {
    it.current()->setId(i);
  }

  // Materials
  for(unsigned int i = 0; i < mMaterials.count(); i++)
  {
    mMaterials[i]->setId(i);
  }

  // Stuff in LODs
  for(unsigned int i = 0; i < lodCount(); i++)
  {
    LOD* l = lod(i);
    // Meshes
    for(unsigned int j = 0; j < l->meshCount(); j++)
    {
      l->mesh(j)->setId(j);
    }
    // Frames
    for(unsigned int j = 0; j < l->frameCount(); j++)
    {
      l->frame(j)->setId(j);
    }
  }
}

void Model::calculateFaceNormals()
{
  for(unsigned int i = 0; i < lodCount(); i++)
  {
    LOD* l = lod(i);
    for(unsigned int j = 0; j < l->meshCount(); j++)
    {
      l->mesh(j)->calculateFaceNormals();
    }
  }
}

void Model::calculateVertexNormals()
{
  for(unsigned int i = 0; i < lodCount(); i++)
  {
    LOD* l = lod(i);
    for(unsigned int j = 0; j < l->meshCount(); j++)
    {
      l->mesh(j)->calculateVertexNormals();
    }
  }
}

void Model::smoothAllFaces()
{
  for(unsigned int i = 0; i < lodCount(); i++)
  {
    LOD* l = lod(i);
    for(unsigned int j = 0; j < l->meshCount(); j++)
    {
      l->mesh(j)->smoothAllFaces();
    }
  }
}

void Model::loadingCompleted()
{
  for(unsigned int i = 0; i < lodCount(); i++)
  {
    LOD* l = lod(i);
    for(unsigned int j = 0; j < l->meshCount(); j++)
    {
      l->mesh(j)->loadingCompleted();
    }
  }
}

void Model::createLODs(unsigned int num)
{
  updateIds();

  for(unsigned int i = lodCount(); i < num; i++)
  {
    LOD* l = new LOD(baseLOD());
    mLODs.append(l);
  }
}

void Model::updateRadius(unsigned int baseframe)
{
  float maxdist = 0.0f;
  // Calculate the radius of the (bounding sphere of the) model.
  // We only consider the base lod and base frame here.
  Frame* f = baseLOD()->frame(baseframe);
  for(unsigned int i = 0; i < f->nodeCount(); i++)
  {
    Mesh* mesh = f->mesh(i);
    BoMatrix* matrix = f->matrix(i);
    for(unsigned int j = 0; j < mesh->vertexCount(); j++)
    {
      BoVector3Float pos;
      matrix->transform(&pos, &mesh->vertex(j)->pos);
      // Calculate distance to the pos from the origin point.
      // Note that the distance is left squared for performace reasons.
      maxdist = QMAX(maxdist, pos.dotProduct());
    }
  }
  mRadius = sqrt(maxdist);
}

