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
  if(!l)
  {
    boError() << k_funcinfo << "could not create a loader for " << file << endl;
    return false;
  }
  bool ret = l->load();
  if(ret)
  {
    ret = checkLoadedModel();
    if(!ret)
    {
      boError() << k_funcinfo << "loader reported success, but model is not valid" << endl;
      // do NOT return (loader must be deleted)
    }
  }

  delete l;
  if(!ret)
  {
    boWarning() << k_funcinfo << "unable to load file " << file << endl;
  }
  return ret;
}

bool Model::checkLoadedModel() const
{
  LOD* lod = baseLOD();
  if(!lod)
  {
    BO_NULL_ERROR(lod);
    return false;
  }
  for(unsigned int i = 0; i < materialCount(); i++)
  {
    if(!material(i))
    {
      boError(100) << k_funcinfo << "NULL material " << i << endl;
      return false;
    }
  }
  for(unsigned int i = 0; i < lod->meshCount(); i++)
  {
    if(!lod->mesh(i))
    {
      boError(100) << k_funcinfo << "NULL mesh" << i << endl;
      return false;
    }
    Mesh* mesh = lod->mesh(i);
    if(!mesh->vertices())
    {
      BO_NULL_ERROR(mesh->vertices());
      return false;
    }
    if(!mesh->faces())
    {
      BO_NULL_ERROR(mesh->faces());
      return false;
    }
    for(unsigned int j = 0; j < mesh->vertexCount(); j++)
    {
      if(!mesh->vertex(j))
      {
        boError(100) << k_funcinfo << "NULL vertex " << j << " in mesh " << i << endl;
        return false;
      }
    }
    for(unsigned int j = 0; j < mesh->faceCount(); j++)
    {
      if(!mesh->face(j))
      {
        boError(100) << k_funcinfo << "NULL face " << j << " in mesh " << i << endl;
        return false;
      }
    }
  }
  for(unsigned int i = 0; i < lod->frameCount(); i++)
  {
    if(!lod->frame(i))
    {
      boError(100) << k_funcinfo << "NULL frame" << i << endl;
      return false;
    }
    Frame* f = lod->frame(i);
    if(f->nodeCount() == 0)
    {
      boError(100) << k_funcinfo << "no nodes in frame " << i << endl;
      return false;
    }
    for(unsigned int j = 0; j < f->nodeCount(); j++)
    {
      if(!f->mesh(j))
      {
        boError(100) << k_funcinfo << "NULL mesh " << j << " in frame " << i << endl;
        return false;
      }
      if(!f->matrix(j))
      {
        boError(100) << k_funcinfo << "NULL matrix " << j << " in frame " << i << endl;
        return false;
      }
      if(f->matrix(j)->hasNaN())
      {
        boError(100) << k_funcinfo << "matrix " << j << " in frame " << i << " is invalid: has NaN entry" << endl;
        return false;
      }
    }
  }
  return true;
}

bool Model::save(const QString& file)
{
  Saver s(this, file);
  return s.save();
}

void Model::createBaseLOD()
{
  if(lodCount() > 0)
  {
    boError() << k_funcinfo << "base LOD already created" << endl;
    return;
  }
  LOD* l = new LOD;
  mLODs.append(l);
}

unsigned int Model::addMaterial(Material* m)
{
  mMaterials.append(m);
  return mMaterials.count() - 1;
}

void Model::setMaterials(const Q3ValueVector<Material*>& materials)
{
  mMaterials = materials;
}

void Model::setTextures(const Q3Dict<Texture>& textures)
{
  mTextures = textures;
}

Texture* Model::getTexture(const QString& filename)
{
  if(filename.isEmpty())
  {
    return 0;
  }
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
  Q3DictIterator<Texture> it(mTextures);
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
    mIndexArray = (unsigned char*)new quint16[mIndexArraySize];
  }
  else
  {
    mIndexArrayType = BMF_DATATYPE_UNSIGNED_INT;
    mIndexArray = (unsigned char*)new quint32[mIndexArraySize];
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
  Q3DictIterator<Texture> it(mTextures);
  for(unsigned int i = 0; it.current(); ++it, i++)
  {
    it.current()->setId(i);
  }

  // Materials
  for(int i = 0; i < mMaterials.count(); i++)
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
  // TODO: maybe this should be moved to some processor?
  for(unsigned int i = 0; i < lodCount(); i++)
  {
    LOD* l = lod(i);

    Q3ValueVector<Mesh*> meshes;
    Q3ValueVector<Mesh*> removedmeshes;
    for(unsigned int j = 0; j < l->meshCount(); j++)
    {
      Mesh* m = l->mesh(j);
      if(m->vertexCount() == 0 || m->faceCount() == 0)
      {
        removedmeshes.append(m);
        continue;
      }
      meshes.append(m);
    }

    if(removedmeshes.isEmpty())
    {
      // Data didn't change
      continue;
    }

    // Some meshes should be removed.
    // Set new mesh list
    l->removeAllMeshesBut(meshes);
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
      mMinCoord.setX(qMin(mMinCoord.x(), pos.x()));
      mMinCoord.setY(qMin(mMinCoord.y(), pos.y()));
      mMinCoord.setZ(qMin(mMinCoord.z(), pos.z()));
      mMaxCoord.setX(qMax(mMaxCoord.x(), pos.x()));
      mMaxCoord.setY(qMax(mMaxCoord.y(), pos.y()));
      mMaxCoord.setZ(qMax(mMaxCoord.z(), pos.z()));
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
  if(lodCount() < 1)
  {
    boError() << k_funcinfo << "need a base LOD first" << endl;
    return;
  }
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
      maxdist = qMax(maxdist, pos.dotProduct());
    }
  }
  mRadius = sqrt(maxdist);
}

/*
 * vim: et sw=2
 */
