/*
    This file is part of the Boson game
    Copyright (C) 2004 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bobmfload.h"

#include "bo3dtools.h"
#include "bomesh.h"
#include "bomaterial.h"
#include "bosonmodel.h"
#include "../bobmfconverter/bmf.h"
#include "bodebug.h"

#include <qfile.h>
#include <qdatastream.h>


BoBMFLoad::BoBMFLoad(const QString& file, BosonModel* model)
{
  init();
  mFile = file;
  mModel = model;
}

void BoBMFLoad::init()
{
}

BoBMFLoad::~BoBMFLoad()
{
  boDebug(100) << k_funcinfo << endl;
}

QString BoBMFLoad::file() const
{
  return mFile;
}

bool BoBMFLoad::loadModel()
{
  boDebug(100) << k_funcinfo << file() << endl;
  if(mFile.isEmpty())
  {
    boError(100) << k_funcinfo << "No file has been specified for loading" << endl;
    return false;
  }
  if(!mModel)
  {
    BO_NULL_ERROR(mModel);
    return false;
  }

  QFile f(file());
  if(!f.open(IO_ReadOnly))
  {
    boError() << k_funcinfo << "can't open " << file() << endl;;
    return false;
  }

  QDataStream stream(&f);

  char header[BMF_FILE_ID_LEN];
  stream.readRawBytes(header, BMF_FILE_ID_LEN);
  if(strncmp(header, BMF_FILE_ID, BMF_FILE_ID_LEN) != 0)
  {
    boError(100) << k_funcinfo << "This file doesn't seem to be in BMF format (invalid header)!" << endl;
    return false;
  }

  Q_UINT32 versioncode;
  stream >> versioncode;
  if(versioncode < BMF_MAKE_VERSION_CODE(0, 0, 2))
  {
    boError(100) << k_funcinfo << "Unsupported BMF version 0x" <<
        QString::number(versioncode, 16) << endl;
    boError(100) << k_funcinfo << "Last supported version is 0x" <<
        QString::number(BMF_MAKE_VERSION_CODE(0, 0, 2), 16) << endl;
    boError(100) << k_funcinfo << "Current version is 0x" <<
        QString::number(BMF_VERSION_CODE, 16) << endl;
    return false;
  }

  Q_UINT32 magic;
  stream >> magic;
  if(magic != BMF_MAGIC_MODEL)
  {
    boError(100) << k_funcinfo << "Loading failed (invalid model magic)!" << endl;
    return false;
  }


  // Load the model
  // Info section
  if(!loadInfo(stream))
  {
    return false;
  }
  // Textures
  if(!loadTextures(stream))
  {
    return false;
  }
  // Materials
  if(!loadMaterials(stream))
  {
    return false;
  }
  // LODs
  if(!loadLODs(stream))
  {
    return false;
  }


  stream >> magic;
  if(magic != BMF_MAGIC_MODEL_END)
  {
    boError(100) << k_funcinfo << "Loading failed (invalid model end magic)!" << endl;
    return false;
  }

  // Verify that loading was successful
  stream >> magic;
  if(magic != BMF_MAGIC_END)
  {
    boError(100) << k_funcinfo << "Loading failed (invalid end magic)!" << endl;
    return false;
  }

  return true;
}

bool BoBMFLoad::loadInfo(QDataStream& stream)
{
  Q_UINT32 magic;
  stream >> magic;
  if(magic != BMF_MAGIC_MODEL_INFO)
  {
    boError(100) << k_funcinfo << "Loading failed (no info section)!" << endl;
    return false;
  }
  // Info section has name, comment and author, each of which consists of a
  //  magic and a string.
  char* str;
  // Name
  stream >> magic;
  stream >> str;
  delete[] str;
  // Comment
  stream >> magic;
  stream >> str;
  delete[] str;
  // Author
  stream >> magic;
  stream >> str;
  delete[] str;

  // It also has number of points in the model and radius of the model
  stream >> magic;
  if(magic != BMF_MAGIC_MODEL_POINTS)
  {
    boError(100) << k_funcinfo << "Loading failed (no point count found in info section)!" << endl;
    return false;
  }
  Q_UINT32 points;
  stream >> points;
  mModel->allocatePointArray(points);
  // Get pointer to the point array in the model and reset offset counter
  mPointArray = mModel->pointArray();
  mPointArrayOffset = 0;

  stream >> magic;
  if(magic != BMF_MAGIC_MODEL_RADIUS)
  {
    boError(100) << k_funcinfo << "Loading failed (no radius found in info section)!" << endl;
    return false;
  }
  float radius;
  stream >> radius;
  mModel->setBoundingSphereRadius(radius);

  boDebug(100) << "    " << k_funcinfo << "Model: points: " << points << "; radius: " << radius << endl;

  // Check end magic
  stream >> magic;
  if(magic != BMF_MAGIC_MODEL_INFO_END)
  {
    boError(100) << k_funcinfo << "Loading failed (info section end magic not found)!" << endl;
    return false;
  }

  return true;
}

bool BoBMFLoad::loadTextures(QDataStream& stream)
{
  Q_UINT32 magic;
  stream >> magic;
  if(magic != BMF_MAGIC_TEXTURES)
  {
    boError(100) << k_funcinfo << "Loading failed (no textures section)!" << endl;
    return false;
  }

  Q_UINT32 count;
  char* name;
  stream >> count;
  mTextureNames.clear();
  mTextureNames.reserve(count);
  for(unsigned int i = 0; i < count; i++)
  {
    stream >> name;
    mTextureNames.append(name);
    delete[] name;
  }

  return true;
}

bool BoBMFLoad::loadMaterials(QDataStream& stream)
{
  Q_UINT32 magic;
  stream >> magic;
  if(magic != BMF_MAGIC_MATERIALS)
  {
    boError(100) << k_funcinfo << "Loading failed (no materials section)!" << endl;
    return false;
  }

  if(mModel->materialCount() != 0)
  {
    boError() << k_funcinfo << "Materials already loaded???" << endl;
    return false;
  }

  Q_UINT32 count;
  stream >> count;
  // Allocate necessary amount of materials
  mModel->allocateMaterials(count);

  for(unsigned int i = 0; i < count; i++)
  {
    // Create new material
    BoMaterial* mat = mModel->material(i);
    // Load name
    char* name;
    stream >> name;
    mat->setName(name);
    delete[] name;
    // Load colors
    BoVector4Float color;
    stream >> color;
    mat->setAmbient(color);
    stream >> color;
    mat->setDiffuse(color);
    stream >> color;
    mat->setSpecular(color);
    stream >> color;
    //TODO: mat->setEmissive(color);
    // Shininess
    float shininess;
    stream >> shininess;
    mat->setShininess(shininess);
    // Texture
    Q_INT32 textureid;
    stream >> textureid;
    if(textureid >= 0)
    {
      mat->setTextureName(mTextureNames[textureid]);
    }
  }

  return true;
}


bool BoBMFLoad::loadLODs(QDataStream& stream)
{
  Q_UINT32 magic;
  stream >> magic;
  if(magic != BMF_MAGIC_LODS)
  {
    boError(100) << k_funcinfo << "Loading failed (no LODs section)!" << endl;
    return false;
  }

  Q_UINT32 count;
  stream >> count;
  // Allocate lods...
  mModel->allocateLODs(count);
  // ...and load them
  for(unsigned int i = 0; i < count; i++)
  {
    if(!loadLOD(stream, i))
    {
      return false;
    }
  }

  return true;
}

bool BoBMFLoad::loadLOD(QDataStream& stream, int lod)
{
  Q_UINT32 magic;
  stream >> magic;
  if(magic != BMF_MAGIC_LOD)
  {
    boError(100) << k_funcinfo << "Loading failed (no LODs section)!" << endl;
    return false;
  }

  if(!loadMeshes(stream, lod))
  {
    return false;
  }

  if(!loadFrames(stream, lod))
  {
    return false;
  }

  return true;
}

bool BoBMFLoad::loadMeshes(QDataStream& stream, int lod)
{
  Q_UINT32 magic;
  stream >> magic;
  if(magic != BMF_MAGIC_MESHES)
  {
    boError(100) << k_funcinfo << "Loading failed (no meshes section for lod " <<
        lod << ")!" << endl;
    return false;
  }

  BoLOD* l = mModel->lod(lod);

  Q_UINT32 count;
  stream >> count;
  l->allocateMeshes(count);
  for(unsigned int i = 0; i < count; i++)
  {
    stream >> magic;
    if(magic != BMF_MAGIC_MESH_VERTICES)
    {
      boError(100) << k_funcinfo << "Loading failed (no vertices section for mesh " <<
          i << " in lod " << lod << ")!" << endl;
      return false;
    }
    Q_UINT32 vertexcount;
    stream >> vertexcount;

    BoMesh* mesh = l->mesh(i);
    mesh->setPointOffset(mPointArrayOffset);
    mesh->setPointCount(vertexcount);
    mesh->setRenderMode(GL_TRIANGLES);

    // Load the points
    // Read the whole block of floats into memory at once - should be faster
    //  this way.
    //stream.readRawBytes((char*)(mPointArray + mPointArrayOffset * BoMesh::pointSize()),
    //    sizeof(float) * BoMesh::pointSize() * vertexcount);
    for(unsigned int j = 0; j < vertexcount; j++)
    {
      // Every vertex has 8 floats
      for(unsigned int k = 0; k < 8; k++)
      {
        stream >> mPointArray[(mPointArrayOffset + j) * 8 + k];
      }
    }

    mPointArrayOffset += vertexcount;

    // Load misc info
    stream >> magic;
    if(magic != BMF_MAGIC_MESH_MISC)
    {
      boError(100) << k_funcinfo << "Loading failed (no misc section for mesh " <<
          i << " in lod " << lod << ")!" << endl;
      return false;
    }
    char* name;
    stream >> name;
    // TODO: set name
    mesh->setName(name);
    delete[] name;
    Q_INT32 materialid;
    stream >> materialid;
    if(materialid >= 0)
    {
      mesh->setMaterial(mModel->material(materialid));
    }
    Q_UINT8 isteamcolor;
    stream >> isteamcolor;
    mesh->setIsTeamColor((bool)(isteamcolor));
  }

  return true;
}

bool BoBMFLoad::loadFrames(QDataStream& stream, int lod)
{
  Q_UINT32 magic;
  stream >> magic;
  if(magic != BMF_MAGIC_FRAMES)
  {
    boError(100) << k_funcinfo << "Loading failed (no frames section for lod " <<
        lod << ")!" << endl;
    return false;
  }

  BoLOD* l = mModel->lod(lod);

  Q_UINT32 count;
  stream >> count;
  l->allocateFrames(count);
  for(unsigned int i = 0; i < count; i++)
  {
    BoFrame* frame = l->frame(i);
    if(!frame)
    {
      BO_NULL_ERROR(frame);
      return false;
    }

    Q_UINT32 nodecount;
    stream >> nodecount;
    frame->allocNodes(nodecount);

    for(unsigned int j = 0; j < nodecount; j++)
    {
      Q_INT32 meshid;
      stream >> meshid;
      float matrix[16];
      for(unsigned int k = 0; k < 16; k++)
      {
        stream >> matrix[k];
      }
      frame->setMesh(j, l->mesh(meshid));
      frame->matrix(j)->loadMatrix(matrix);
    }
  }

  return true;
}

