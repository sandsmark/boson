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

#include "lod.h"

#include "frame.h"
#include "mesh.h"
#include "debug.h"

#include <qstring.h>


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

void LOD::setFrames(const QValueVector<Frame*>& frames)
{
  mFrames = frames;
}

void LOD::setMeshes(const QValueVector<Mesh*>& meshes)
{
  mMeshes = meshes;
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

