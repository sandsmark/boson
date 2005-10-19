/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Rivo Laks (rivolaks@hot.ee)

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

#include "../bomemory/bodummymemory.h"
#include "bo3dtools.h"
#include "bomesh.h"
#include "bomaterial.h"
#include "bosonmodel.h"
#include "../bobmfconverter/bmf.h"
#include "bodebug.h"

#include <qfile.h>
#include <qdatastream.h>
#include <qapplication.h>

#include <kmdcodec.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kprocess.h>


#define MIN_SUPPORTED_VERSION BMF_MAKE_VERSION_CODE(0, 1, 0)


BoBMFLoad::BoBMFLoad(const QString& file, BosonModel* model)
{
  init();
  mFile = file;
  mModel = model;
}

void BoBMFLoad::init()
{
  mTextureTransparent = 0;
}

BoBMFLoad::~BoBMFLoad()
{
  boDebug(100) << k_funcinfo << endl;
  delete[] mTextureTransparent;
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
  if(versioncode < MIN_SUPPORTED_VERSION)
  {
    boError(100) << k_funcinfo << "Unsupported BMF version 0x" <<
        QString::number(versioncode, 16) << endl;
    boError(100) << k_funcinfo << "Last supported version is 0x" <<
        QString::number(MIN_SUPPORTED_VERSION, 16) << endl;
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
  // Arrays
  if(!loadArrays(stream))
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

  // It also has the radius of the model
  stream >> magic;
  if(magic != BMF_MAGIC_MODEL_RADIUS)
  {
    boError(100) << k_funcinfo << "Loading failed (no radius found in info section)!" << endl;
    return false;
  }
  float radius;
  stream >> radius;
  mModel->setBoundingSphereRadius(radius);

  // ... and the bounding box of the model
  stream >> magic;
  if(magic != BMF_MAGIC_MODEL_BBOX)
  {
    boError(100) << k_funcinfo << "Loading failed (no bbox found in info section)!" << endl;
    return false;
  }
  BoVector3Float min, max;
  stream >> min >> max;
  mModel->setBoundingBox(min, max);

  // Check end magic
  stream >> magic;
  if(magic != BMF_MAGIC_MODEL_INFO_END)
  {
    boError(100) << k_funcinfo << "Loading failed (info section end magic not found)!" << endl;
    return false;
  }

  return true;
}

bool BoBMFLoad::loadArrays(QDataStream& stream)
{
  Q_UINT32 magic;
  // Load vertex and index arrays
  stream >> magic;
  if(magic != BMF_MAGIC_ARRAYS)
  {
    boError(100) << k_funcinfo << "Loading failed (no arrays section found)!" << endl;
    return false;
  }

  Q_UINT32 points, indices, indextype;
  stream >> points;
  stream >> indices;
  stream >> indextype;

  // Load points (vertices)
  mModel->allocatePointArray(points);
  stream.readRawBytes((char*)mModel->pointArray(), points * 8 * sizeof(float));
  // Load indices
  mModel->allocateIndexArray(indices, indextype);
  if(indextype == BMF_DATATYPE_UNSIGNED_SHORT)
  {
    stream.readRawBytes((char*)mModel->indexArray(), indices * sizeof(Q_UINT16));
  }
  else
  {
    stream.readRawBytes((char*)mModel->indexArray(), indices * sizeof(Q_UINT32));
  }

  // The data is little-endian, so if the system uses big-endian, it has to
  //  be converted.
  int wordsize;
  bool bigendian;
  qSysInfo(&wordsize, &bigendian);
  if(bigendian)
  {
    convertToBigEndian((char*)mModel->pointArray(), points * 8, sizeof(float));
    if(indextype == BMF_DATATYPE_UNSIGNED_SHORT)
    {
      convertToBigEndian((char*)mModel->indexArray(), indices, sizeof(Q_UINT16));
    }
    else
    {
      convertToBigEndian((char*)mModel->indexArray(), indices, sizeof(Q_UINT32));
    }
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
  Q_UINT8 hastransparency;
  stream >> count;
  mTextureNames.clear();
  mTextureNames.reserve(count);
  mTextureTransparent = new bool[count];
  for(unsigned int i = 0; i < count; i++)
  {
    stream >> name;
    stream >> hastransparency;
    mTextureNames.append(name);
    mTextureTransparent[i] = (bool)(hastransparency);
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
      mat->setIsTransparent(mTextureTransparent[textureid]);
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
  float dist;
  stream >> dist;
  mModel->setLodDistance(lod, dist);

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
  bool hastransparentmeshes = false;
  for(unsigned int i = 0; i < count; i++)
  {
    BoMesh* mesh = l->mesh(i);

    stream >> magic;
    if(magic != BMF_MAGIC_MESH_INFO)
    {
      boError(100) << k_funcinfo << "Loading failed (no info section for mesh " <<
          i << " in lod " << lod << ")!" << endl;
      return false;
    }
    char* name;
    stream >> name;
    mesh->setName(name);
    delete[] name;
    BoVector3Float min, max;
    stream >> min >> max;
    mesh->setBoundingBox(min, max);

    stream >> magic;
    if(magic != BMF_MAGIC_MESH_DATA)
    {
      boError(100) << k_funcinfo << "Loading failed (no data section for mesh " <<
          i << " in lod " << lod << ")!" << endl;
      return false;
    }

    Q_UINT8 useindices;
    Q_UINT32 rendermode;
    Q_UINT32 pointoffset;
    Q_UINT32 pointcount;
    stream >> useindices;
    stream >> rendermode;
    stream >> pointoffset;
    stream >> pointcount;

    mesh->setUseIndices(useindices);
    mesh->setRenderMode(rendermode);
    mesh->setPointOffset(pointoffset);
    mesh->setPointCount(pointcount);

    if(useindices)
    {
      Q_UINT32 indexoffset;
      Q_UINT32 indexcount;
      stream >> indexoffset;
      stream >> indexcount;

      mesh->setIndexCount(indexcount);
      if(mModel->indexArrayType() == BMF_DATATYPE_UNSIGNED_SHORT)
      {
        mesh->setIndices(mModel->indexArray() + 2*indexoffset);
      }
      else
      {
        mesh->setIndices(mModel->indexArray() + 4*indexoffset);
      }
    }

    // Load misc info
    stream >> magic;
    if(magic != BMF_MAGIC_MESH_MISC)
    {
      boError(100) << k_funcinfo << "Loading failed (no misc section for mesh " <<
          i << " in lod " << lod << ")!" << endl;
      return false;
    }
    Q_INT32 materialid;
    stream >> materialid;
    if(materialid >= 0)
    {
      mesh->setMaterial(mModel->material(materialid));
      if(mesh->material()->isTransparent())
      {
        hastransparentmeshes = true;
      }
    }
    Q_UINT8 isteamcolor;
    stream >> isteamcolor;
    mesh->setIsTeamColor((bool)(isteamcolor));
  }

  l->setHasTransparentMeshes(hastransparentmeshes);

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


QString BoBMFLoad::cachedModelFilename(const QString& modelfile, const QString& configfile)
{
  QCString hash = calculateHash(modelfile, configfile);
  QString cachedmodel = KGlobal::dirs()->findResource("data", QString("%1/model-%2.bmf").arg("boson/modelcache").arg(hash));

  if(!cachedmodel.isEmpty())
  {
    Q_UINT32 versioncode = getVersion(cachedmodel);
    if(versioncode >= MIN_SUPPORTED_VERSION)
    {
      // File exists and is up-to-date
      return cachedmodel;
    }
    else
    {
      // Cached model is too old
      // TODO: maybe delete the obsolete model?
      return QString::null;
    }
  }
  return QString::null;
}

QString BoBMFLoad::convertModel(const QString& modelfile, const QString& configfile)
{
  QCString hash = calculateHash(modelfile, configfile);

  // Get the path where the cached model can be saved
  QString cachedmodel = KGlobal::dirs()->saveLocation("data", "boson/modelcache/");
  if(cachedmodel.isEmpty())
  {
    boError() << k_funcinfo << "Failed to get save location for cached model" << endl;
    return QString::null;
  }
  cachedmodel += QString("model-%1.bmf").arg(hash);
  // Find path to bobmfconverter binary
  QString converter = KGlobal::dirs()->findResource("exe", "bobmfconverter");
  if(converter.isEmpty())
  {
    converter = KGlobal::dirs()->findExe("bobmfconverter");
    if(converter.isEmpty())
    {
      boError() << k_funcinfo << "Couldn't find bobmfconverter!" << endl;
      return QString::null;
    }
  }
  // Create KProcess object
  KProcess proc;
  proc << converter;
  // Add default cmdline args
  proc << "-lods" << "5" <<  "-keepframes" <<  "-texnametolower" <<  "-useboth" << "-dontloadtex";
  proc << "-o" << cachedmodel;
  if(!configfile.isEmpty())
  {
    proc << "-c" << configfile;
  }
  proc << modelfile;
  proc << "-comment" << QString("Automatically converted from file '%1'").arg(modelfile);

  // FIXME: KProcess:Block ain't pretty here...
  if(!proc.start(KProcess::Block))
  {
    boError() << k_funcinfo << "Error while trying to convert the model" << endl;
    return QString::null;
  }

  cachedmodel = KGlobal::dirs()->findResource("data", QString("%1/model-%2.bmf").arg("boson/modelcache").arg(hash));
  if(cachedmodel.isEmpty())
  {
    boError() << k_funcinfo << "bobmfconverter did not write file " << cachedmodel << endl;
    return QString::null;
  }

  return cachedmodel;
}

QCString BoBMFLoad::calculateHash(const QString& modelfilename, const QString& configfilename)
{
  QFile modelfile(modelfilename);
  if(!modelfile.open(IO_ReadOnly))
  {
    boError() << k_funcinfo << "could not open model file " << modelfilename << endl;
    return QCString();
  }

  KMD5 md5(modelfile.readAll());
  QFile configfile(configfilename);
  if(configfile.open(IO_ReadOnly))
  {
    md5.update(configfile.readAll());
  }
  return md5.hexDigest();
}

Q_UINT32 BoBMFLoad::getVersion(const QString& modelfile)
{
  QFile f(modelfile);
  if(!f.open(IO_ReadOnly))
  {
    boError() << k_funcinfo << "can't open " << modelfile << endl;;
    return 0;
  }

  QDataStream stream(&f);

  char header[BMF_FILE_ID_LEN];
  stream.readRawBytes(header, BMF_FILE_ID_LEN);
  if(strncmp(header, BMF_FILE_ID, BMF_FILE_ID_LEN) != 0)
  {
    boError(100) << k_funcinfo << "This file doesn't seem to be in BMF format (invalid header)!" << endl;
    return 0;
  }

  Q_UINT32 versioncode;
  stream >> versioncode;

  f.close();
  return versioncode;
}

void BoBMFLoad::convertToBigEndian(char* array, unsigned int elements, unsigned int elementsize)
{
  char* x = new char[elementsize];
  for(unsigned int i = 0; i < elements * elementsize; i += elementsize)
  {
    //x[0] = p[i+0]; x[1] = p[i+1]; x[2] = p[i+2]; x[3] = p[i+3];
    //p[i+0] = x[3]; p[i+1] = x[2]; p[i+2] = x[1]; p[i+3] = x[0];
    for(unsigned int j = 0; j < elementsize; j++)
    {
      x[j] = array[i+j];
    }
    for(unsigned int j = 0; j < elementsize; j++)
    {
      array[i+j] = x[elementsize-1-j];
    }
  }

  delete x;
}

