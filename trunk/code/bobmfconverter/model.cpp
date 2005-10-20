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

#include "model.h"

#include "lod.h"
#include "material.h"
#include "loader.h"
#include "texture.h"
#include "saver.h"
#include "mesh.h"
#include "frame.h"
#include "debug.h"
#include "bmf.h"


Model::Model()
{
  mRadius = 0.0f;

  mVertexArray = 0;
  mVertexArraySize = 0;
  mIndexArray = 0;
  mIndexArraySize = 0;
  mIndexArrayType = 0;
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

bool Model::loadTextures()
{
  bool ret = true;
  QDictIterator<Texture> it(mTextures);
  while(it.current())
  {
    ret &= it.current()->load();
    ++it;
  }
  return ret;
}

void Model::prepareForSaving(unsigned int baseframe)
{
  removeEmptyMeshes();
  updateBoundingBox(baseframe);
  updateIds();
  updateRadius(baseframe);
  createArrays();
}

void Model::createArrays()
{
  // Count the number of indices and vertices we need to allocate
  mVertexArraySize = 0;
  mIndexArraySize = 0;
  unsigned int maxIndex = 0;
  for(unsigned int i = 0; i < lodCount(); i++)
  {
    LOD* l = lod(i);
    for(unsigned int j = 0; j < l->meshCount(); j++)
    {
      Mesh* m = l->mesh(j);
      if(m->useIndices())
      {
        mVertexArraySize += m->vertexCount();
        mIndexArraySize += m->faceCount() * 3;
        maxIndex = mVertexArraySize - 1;
      }
      else
      {
        mVertexArraySize += m->faceCount() * 3;
      }
    }
  }

  // Create the arrays
  mVertexArray = new float[mVertexArraySize * (3+3+2)];
  if(maxIndex <= 65535)
  {
    mIndexArrayType = BMF_DATATYPE_UNSIGNED_SHORT;
    mIndexArray = (unsigned char*)new Q_UINT16[mIndexArraySize];
  }
  else
  {
    mIndexArrayType = BMF_DATATYPE_UNSIGNED_INT;
    mIndexArray = (unsigned char*)new Q_UINT32[mIndexArraySize];
  }

  // Copy data into the arrays
  unsigned int vertexoffset = 0;
  unsigned int indexoffset = 0;
  for(unsigned int i = 0; i < lodCount(); i++)
  {
    LOD* l = lod(i);
    for(unsigned int j = 0; j < l->meshCount(); j++)
    {
      l->mesh(j)->createArrays(mVertexArray, mIndexArray, &vertexoffset, &indexoffset, mIndexArrayType);
    }
  }

  boDebug() << k_funcinfo << vertexoffset << " of " << mVertexArraySize << " vertices copied" << endl;
  boDebug() << k_funcinfo << indexoffset << " of " << mIndexArraySize << " indices copied" << endl;
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
      Mesh* m = l->mesh(j);
      m->setId(j);
      // Vertices
      for(unsigned int k = 0; k < m->vertexCount(); k++)
      {
        m->vertex(k)->id = k;
      }
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

void Model::removeEmptyMeshes()
{
  for(unsigned int i = 0; i < lodCount(); i++)
  {
    LOD* l = lod(i);
    QValueVector<Mesh*> meshes;
    for(unsigned int j = 0; j < l->meshCount(); j++)
    {
      Mesh* m = l->mesh(j);
      if(m->vertexCount() == 0 || m->faceCount() == 0)
      {
        // Remove this invalid mesh
        delete m;
        continue;
      }
      meshes.append(m);
    }
    l->setMeshes(meshes);
  }
}

void Model::updateBoundingBox(unsigned int baseframe)
{
  mMinCoord.set( 1000000000,  1000000000,  1000000000);
  mMaxCoord.set(-1000000000, -1000000000, -1000000000);
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
      mMinCoord.setX(QMIN(mMinCoord.x(), pos.x()));
      mMinCoord.setY(QMIN(mMinCoord.y(), pos.y()));
      mMinCoord.setZ(QMIN(mMinCoord.z(), pos.z()));
      mMaxCoord.setX(QMAX(mMaxCoord.x(), pos.x()));
      mMaxCoord.setY(QMAX(mMaxCoord.y(), pos.y()));
      mMaxCoord.setZ(QMAX(mMaxCoord.z(), pos.z()));
    }
  }

  // Update bounding boxes of meshes
  for(unsigned int i = 0; i < lodCount(); i++)
  {
    LOD* l = lod(i);
    for(unsigned int j = 0; j < l->meshCount(); j++)
    {
      l->mesh(j)->updateBoundingBox();
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

  setLodDistances();
}

void Model::setLodDistances(float multiplier)
{
  // Calculate initial distances for lods
  float dist = 0.0f;
  for(unsigned int i = 0; i < lodCount(); i++)
  {
    lod(i)->setDistance(dist * multiplier);
    // dists will be 0, 10, 25, 45, 70, ...
    dist = dist + (10 * (0.5 + i * 0.5));
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

