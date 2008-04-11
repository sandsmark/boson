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


#include "saver.h"

#include "debug.h"
#include "model.h"
#include "texture.h"
#include "material.h"
#include "lod.h"
#include "mesh.h"
#include "frame.h"
#include "bmf.h"

#include <qfile.h>
#include <qdatastream.h>
#include <q3dict.h>


Saver::Saver(Model* m, const QString& filename)
{
  mModel = m;
  mFilename = filename;
}

bool Saver::save()
{
  boDebug() << k_funcinfo << endl;
  // Open the file
  QFile f(mFilename);
  if(!f.open(QIODevice::WriteOnly))
  {
    boError() << k_funcinfo << "Can't open output file " << mFilename << endl;
    return false;
  }

  bool ret = true;
  // Open datastream
  QDataStream stream(&f);
  //stream.setPrintableData(true);

  // Write header
  stream.writeRawData(BMF_FILE_ID, BMF_FILE_ID_LEN);
  stream << (quint32)BMF_VERSION_CODE;

  ret = saveModel(stream, mModel);

  stream << (quint32)BMF_MAGIC_END;

  // Close the file
  f.close();
  boDebug() << k_funcinfo << "DONE" << endl;
  return ret;
}

bool Saver::saveModel(QDataStream& stream, Model* model)
{
  stream << (quint32)BMF_MAGIC_MODEL;

  // Info
  stream << (quint32)BMF_MAGIC_MODEL_INFO;
  stream << (quint32)BMF_MAGIC_MODEL_NAME;
  {
    QByteArray tmp = model->name().toLatin1();
    stream << tmp.data();
  }
  stream << (quint32)BMF_MAGIC_MODEL_COMMENT;
  {
    QByteArray tmp = model->comment().toLatin1();
    stream << tmp.data();
  }
  stream << (quint32)BMF_MAGIC_MODEL_AUTHOR;
  {
    QByteArray tmp = model->author().toLatin1();
    stream << tmp.data();
  }
  stream << (quint32)BMF_MAGIC_MODEL_RADIUS;
  stream << model->radius();
  stream << (quint32)BMF_MAGIC_MODEL_BBOX;
  stream << model->minCoord();
  stream << model->maxCoord();
  stream << (quint32)BMF_MAGIC_MODEL_INFO_END;

  // Vertex and index arrays
  stream << (quint32)BMF_MAGIC_ARRAYS;
  stream << (quint32)model->vertexArraySize();
  stream << (quint32)model->indexArraySize();
  stream << (quint32)model->indexArrayType();
  // Arrays will be little-endian-encoded
  stream.setByteOrder(QDataStream::LittleEndian);
  // Vertex array
  for(unsigned int i = 0; i < model->vertexArraySize(); i++)
  {
    for(unsigned int j = 0; j < 8; j++)
    {
      stream << model->vertexArray()[i*8 + j];
    }
  }
  // Index array
  if(model->indexArrayType() == BMF_DATATYPE_UNSIGNED_SHORT)
  {
    quint16* indices = (quint16*)model->indexArray();
    for(unsigned int i = 0; i < model->indexArraySize(); i++)
    {
      stream << indices[i];
    }
  }
  else
  {
    quint32* indices = (quint32*)model->indexArray();
    for(unsigned int i = 0; i < model->indexArraySize(); i++)
    {
      stream << indices[i];
    }
  }
  stream.setByteOrder(QDataStream::BigEndian);

  // Textures
  stream << (quint32)BMF_MAGIC_TEXTURES;
  stream << (quint32)model->texturesDict()->count();
  Q3DictIterator<Texture> it(*model->texturesDict());
  while(it.current())
  {
    if(!saveTexture(stream, it.current()))
    {
      boError() << k_funcinfo << "Couldn't save texture with filename " <<
          it.current()->filename() << endl;
      return false;
    }
    ++it;
  }

  // Materials
  stream << (quint32)BMF_MAGIC_MATERIALS;
  unsigned int materials = model->materialCount();
  stream << (quint32)materials;
  for(unsigned int i = 0; i < materials; i++)
  {
    saveMaterial(stream, model->material(i));
  }

  // LODs
  stream << (quint32)BMF_MAGIC_LODS;
  unsigned int lodcount = model->lodCount();
  stream << (quint32)lodcount;
  for(unsigned int i = 0; i < lodcount; i++)
  {
    saveLOD(stream, model->lod(i));
  }

  // End of the model
  stream << (quint32)BMF_MAGIC_MODEL_END;

  return true;
}

bool Saver::saveTexture(QDataStream& stream, Texture* tex)
{
  QByteArray tmp = tex->filename().toLatin1();
  stream << tmp.data();
  stream << (quint8)tex->hasTransparency();
  return true;
}

bool Saver::saveMaterial(QDataStream& stream, Material* mat)
{
  QByteArray tmp = mat->name().toLatin1();
  stream << tmp.data();

  stream << mat->ambient();
  stream << mat->diffuse();
  stream << mat->specular();
  stream << mat->emissive();

  stream << mat->shininess();

  stream << (qint32)mat->texture()->id();

  return true;
}

bool Saver::saveLOD(QDataStream& stream, LOD* lod)
{
  stream << (quint32)BMF_MAGIC_LOD;
  stream << lod->distance();

  // Meshes
  stream << (quint32)BMF_MAGIC_MESHES;
  unsigned int meshcount = lod->meshCount();
  stream << (quint32)meshcount;
  for(unsigned int i = 0; i < meshcount; i++)
  {
    saveMesh(stream, lod->mesh(i));
  }

  // Frames
  stream << (quint32)BMF_MAGIC_FRAMES;
  unsigned int framecount = lod->frameCount();
  stream << (quint32)framecount;
  for(unsigned int i = 0; i < framecount; i++)
  {
    saveFrame(stream, lod->frame(i));
  }

  return true;
}

bool Saver::saveMesh(QDataStream& stream, Mesh* mesh)
{
  // Info
  stream << (quint32)BMF_MAGIC_MESH_INFO;
  QByteArray tmp = mesh->name().toLatin1();
  stream << tmp.data();
  stream << mesh->minCoord();
  stream << mesh->maxCoord();

  // Format
  stream << (quint32)BMF_MAGIC_MESH_DATA;
  bool useindices = mesh->useIndices();
  stream << (quint8)useindices;
  stream << (quint32)mesh->renderMode();
  stream << (quint32)mesh->vertexArrayOffset();
  stream << (quint32)mesh->vertexArraySize();
  if(useindices)
  {
    stream << (quint32)mesh->indexArrayOffset();
    stream << (quint32)mesh->indexArraySize();
  }

  // Misc info
  stream << (quint32)BMF_MAGIC_MESH_MISC;
  stream << (qint32)mesh->material()->id();
  stream << (quint8)(mesh->isTeamColor() ? 255 : 0);

  return true;
}

bool Saver::saveFrame(QDataStream& stream, Frame* frame)
{
  unsigned int nodecount = frame->nodeCount();
  stream << (quint32)nodecount;
  for(unsigned int i = 0; i < nodecount; i++)
  {
    stream << (qint32)frame->mesh(i)->id();
    BoMatrix* m = frame->matrix(i);
    for(unsigned int i = 0; i < 16; i++)
    {
      stream << m->data()[i];
    }
  }

  return true;
}

/*
 * vim: et sw=2 ts=2
 */
