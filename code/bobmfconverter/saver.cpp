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
#include <qdict.h>


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
  if(!f.open(IO_WriteOnly))
  {
    boError() << k_funcinfo << "Can't open output file " << mFilename << endl;
    return false;
  }

  bool ret = true;
  // Open datastream
  QDataStream stream(&f);
  //stream.setPrintableData(true);

  // Write header
  stream.writeRawBytes(BMF_FILE_ID, BMF_FILE_ID_LEN);
  stream << (Q_UINT32)BMF_VERSION_CODE;

  ret = saveModel(stream, mModel);

  stream << (Q_UINT32)BMF_MAGIC_END;

  // Close the file
  f.close();
  boDebug() << k_funcinfo << "DONE" << endl;
  return ret;
}

bool Saver::saveModel(QDataStream& stream, Model* model)
{
  stream << (Q_UINT32)BMF_MAGIC_MODEL;

  // Find number of points (vertices) in the whole model
  unsigned int points = 0;
  for(unsigned int i = 0; i < model->lodCount(); i++)
  {
    LOD* l = model->lod(i);
    for(unsigned int j = 0; j < l->meshCount(); j++)
    {
      points += l->mesh(j)->faceCount()*3;
    }
  }

  // Info
  stream << (Q_UINT32)BMF_MAGIC_MODEL_INFO;
  stream << (Q_UINT32)BMF_MAGIC_MODEL_NAME;
  stream << model->name().latin1();
  stream << (Q_UINT32)BMF_MAGIC_MODEL_COMMENT;
  stream << model->comment().latin1();
  stream << (Q_UINT32)BMF_MAGIC_MODEL_AUTHOR;
  stream << model->author().latin1();
  stream << (Q_UINT32)BMF_MAGIC_MODEL_POINTS;
  stream << (Q_UINT32)points;
  stream << (Q_UINT32)BMF_MAGIC_MODEL_RADIUS;
  stream << model->radius();
  stream << (Q_UINT32)BMF_MAGIC_MODEL_INFO_END;

  // Textures
  stream << (Q_UINT32)BMF_MAGIC_TEXTURES;
  stream << (Q_UINT32)model->texturesDict()->count();
  QDictIterator<Texture> it(*model->texturesDict());
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
  stream << (Q_UINT32)BMF_MAGIC_MATERIALS;
  unsigned int materials = model->materialCount();
  stream << (Q_UINT32)materials;
  for(unsigned int i = 0; i < materials; i++)
  {
    saveMaterial(stream, model->material(i));
  }

  // LODs
  stream << (Q_UINT32)BMF_MAGIC_LODS;
  unsigned int lodcount = model->lodCount();
  stream << (Q_UINT32)lodcount;
  for(unsigned int i = 0; i < lodcount; i++)
  {
    saveLOD(stream, model->lod(i));
  }

  // End of the model
  stream << (Q_UINT32)BMF_MAGIC_MODEL_END;

  return true;
}

bool Saver::saveTexture(QDataStream& stream, Texture* tex)
{
  stream << tex->filename().latin1();
  return true;
}

bool Saver::saveMaterial(QDataStream& stream, Material* mat)
{
  stream << mat->name().latin1();

  stream << mat->ambient();
  stream << mat->diffuse();
  stream << mat->specular();
  stream << mat->emissive();

  stream << mat->shininess();

  stream << (Q_INT32)(mat->texture() ? mat->texture()->id() : -1);

  return true;
}

bool Saver::saveLOD(QDataStream& stream, LOD* lod)
{
  stream << (Q_UINT32)BMF_MAGIC_LOD;

  // Meshes
  stream << (Q_UINT32)BMF_MAGIC_MESHES;
  unsigned int meshcount = lod->meshCount();
  stream << (Q_UINT32)meshcount;
  for(unsigned int i = 0; i < meshcount; i++)
  {
    saveMesh(stream, lod->mesh(i));
  }

  // Frames
  stream << (Q_UINT32)BMF_MAGIC_FRAMES;
  unsigned int framecount = lod->frameCount();
  stream << (Q_UINT32)framecount;
  for(unsigned int i = 0; i < framecount; i++)
  {
    saveFrame(stream, lod->frame(i));
  }

  return true;
}

bool Saver::saveMesh(QDataStream& stream, Mesh* mesh)
{
  // Vertices
  stream << (Q_UINT32)BMF_MAGIC_MESH_VERTICES;
#if 0
  int vertexcount = mesh->vertexArrayElements();
  float* array = mesh->vertexArray();
  stream << vertexcount;
  for(int i = 0; i < vertexcount; i++)
  {
    // Save the vertex
    // Every vertex consists of 8 floats
    for(int j = 0; j < 8; j++)
    {
      stream << array[(i * 8) + j];
    }
  }
#else
  unsigned int facecount = mesh->faceCount();
  stream << (Q_UINT32)facecount*3;
  for(unsigned int i = 0; i < facecount; i++)
  {
    Face* f = mesh->face(i);
    for(unsigned int j = 0; j < 3; j++)
    {
      Vertex* v = f->vertex(j);
      stream << v->pos;
      stream << v->normal;
      stream << v->tex;
    }
  }
#endif

  // Misc info
  stream << (Q_UINT32)BMF_MAGIC_MESH_MISC;
  stream << mesh->name().latin1();
  stream << (Q_INT32)(mesh->material() ? mesh->material()->id() : -1);
  stream << (Q_UINT8)(mesh->isTeamColor() ? 255 : 0);

  return true;
}

bool Saver::saveFrame(QDataStream& stream, Frame* frame)
{
  unsigned int nodecount = frame->nodeCount();
  stream << (Q_UINT32)nodecount;
  for(unsigned int i = 0; i < nodecount; i++)
  {
    stream << (Q_INT32)frame->mesh(i)->id();
    BoMatrix* m = frame->matrix(i);
    for(unsigned int i = 0; i < 16; i++)
    {
      stream << m->data()[i];
    }
  }

  return true;
}

