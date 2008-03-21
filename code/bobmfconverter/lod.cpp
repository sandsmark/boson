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

#include "lod.h"

#include "frame.h"
#include "mesh.h"
#include "debug.h"

#include <qstring.h>
#include <qvaluelist.h>


LOD::LOD()
{
  mDist = 0;
}

LOD::LOD(LOD* base)
{
  for(unsigned int i = 0; i < base->meshCount(); i++)
  {
    mMeshes.append(new Mesh(base->mesh(i)));
  }
  for(unsigned int i = 0; i < base->frameCount(); i++)
  {
    Frame* f = new Frame(base->frame(i));
    for(unsigned int j = 0; j < f->nodeCount(); j++)
    {
      f->setMesh(j, mesh(base->frame(i)->mesh(j)->id()));
    }
    mFrames.append(f);
  }
}

LOD::~LOD()
{
  for(unsigned int i = 0; i < mFrames.count(); i++)
  {
    delete mFrames[i];
  }
  mFrames.clear();
  for(unsigned int i = 0; i < mMeshes.count(); i++)
  {
    delete mMeshes[i];
  }
  mMeshes.clear();
}

unsigned int LOD::addMesh(Mesh* m)
{
  m->setId(mMeshes.count());
  mMeshes.append(m);
  return mMeshes.count() - 1;
}

unsigned int LOD::createFrame()
{
  mFrames.append(new Frame);
  return mFrames.count() - 1;
}

void LOD::removeAllFramesBut(const QValueVector<Frame*>& frames)
{
  if(frames.count() == 0)
  {
    boError() << k_funcinfo << "must keep at least one frame" << endl;
    return;
  }
  for(QValueVector<Frame*>::const_iterator it = frames.begin(); it != frames.end(); ++it)
  {
    if(qFind(mFrames.begin(), mFrames.end(), *it) == mFrames.end())
    {
      boError() << k_funcinfo << "request contained unknown frame pointer. cannot handle request." << endl;
      return;
    }
  }

  for(QValueVector<Frame*>::iterator it = mFrames.begin(); it != mFrames.end(); ++it)
  {
    if(qFind(frames.begin(), frames.end(), *it) == frames.end())
    {
      delete *it;
    }
  }
  mFrames = frames;
}

void LOD::removeAllMeshesBut(const QValueVector<Mesh*>& meshes)
{
  if(meshes.count() == 0)
  {
    boError() << k_funcinfo << "must keep at least one mesh" << endl;
    return;
  }
  for(QValueVector<Mesh*>::const_iterator it = meshes.begin(); it != meshes.end(); ++it)
  {
    if(qFind(mMeshes.begin(), mMeshes.end(), *it) == mMeshes.end())
    {
      boError() << k_funcinfo << "request contained unknown mesh pointer. cannot handle request." << endl;
      return;
    }
  }

  QValueList<Mesh*> deleteMeshes;
  for(QValueVector<Mesh*>::iterator it = mMeshes.begin(); it != mMeshes.end(); ++it)
  {
    if(qFind(meshes.begin(), meshes.end(), *it) == meshes.end())
    {
      if(!deleteMeshes.contains(*it))
      {
        deleteMeshes.append(*it);
      }
    }
  }
  mMeshes = meshes;

  for(QValueList<Mesh*>::iterator it = deleteMeshes.begin(); it != deleteMeshes.end(); ++it)
  {
    removeReferencesToMesh(*it);
    delete *it;
  }
}

void LOD::removeReferencesToMesh(Mesh* mesh)
{
  BO_CHECK_NULL_RET(mesh);
  for(unsigned int i = 0; i < frameCount(); i++)
  {
    Frame* f = frame(i);
    BO_CHECK_NULL_RET(f);
    f->removeMesh(mesh);
  }

  // AB: atm we require that a mesh is removed from this LOD object _before_
  //     removeReferencesToMesh() is called.
  //     -> we complain about this, if it was forgotten. we do NOT remove the
  //        mesh from mMeshes on our own!
  if (qFind(mMeshes.begin(), mMeshes.end(), mesh) != mMeshes.end())
  {
    boError() << k_funcinfo << "mMeshes still contains mesh " << mesh << ". cannot remove this reference" << endl;
  }
}

QString LOD::shortStats() const
{
  unsigned int totalvertexcount = 0;
  unsigned int totalfacecount = 0;
  for(unsigned int i = 0; i < meshCount(); i++)
  {
    totalvertexcount += mesh(i)->vertexCount();
    totalfacecount += mesh(i)->faceCount();
  }

  return QString("%1 meshes, containing %2 vertices in %3 faces")
      .arg(meshCount()).arg(totalvertexcount).arg(totalfacecount);
}

/*
 * vim: et sw=2
 */
