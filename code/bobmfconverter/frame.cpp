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


#include "frame.h"

#include "bo3dtools.h"
#include "debug.h"


Frame::Frame()
{
  mId = -1;
  mMeshes = 0;
  mMatrices = 0;
  mNodeCount = 0;
}

Frame::Frame(Frame* f)
{
  mId = -1;
  mMeshes = 0;
  mMatrices = 0;
  mNodeCount = 0;
  allocateNodes(f->nodeCount());
  for(unsigned int i = 0; i < mNodeCount; i++)
  {
    matrix(i)->loadMatrix(*f->matrix(i));
  }
}

Frame::~Frame()
{
  delete mMeshes;
  for(unsigned int i = 0; i < mNodeCount; i++)
  {
    delete mMatrices[i];
  }
  delete mMatrices;
}

void Frame::allocateNodes(unsigned int i)
{
  delete mMeshes;
  for(unsigned int j = 0; j < mNodeCount; j++)
  {
    delete mMatrices[j];
  }
  delete mMatrices;

  mMeshes = new Mesh*[i];
  mMatrices = new BoMatrix*[i];
  mNodeCount = i;

  for(unsigned int j = 0; j < mNodeCount; j++)
  {
    mMeshes[j] = 0;
    mMatrices[j] = new BoMatrix;
  }
}

void Frame::removeMesh(Mesh* mesh)
{
  unsigned int removeCount = 0;
  for(unsigned int i = 0; i < mNodeCount; i++)
  {
    if(mMeshes[i] == mesh)
    {
      removeCount++;
    }
  }
  unsigned int newNodeCount = mNodeCount - removeCount;
  if(newNodeCount == 0)
  {
    boWarning() << k_funcinfo << "removing all remaining nodes" << endl;
    delete[] mMeshes;
    delete[] mMatrices;
    return;
  }

  Mesh** meshes = new Mesh*[newNodeCount];
  BoMatrix** matrices = new BoMatrix*[newNodeCount];
  int index = 0;
  for(unsigned int i = 0; i < mNodeCount; i++)
  {
    if(mMeshes[i] == mesh)
    {
      mMeshes[i] = 0;
      delete mMatrices[i];
      mMatrices[i] = 0;
      continue;
    }
    meshes[index] = mMeshes[i];
    matrices[index] = mMatrices[i];
    index++;
  }
  mNodeCount = newNodeCount;
  delete[] mMeshes;
  delete[] mMatrices;
  mMeshes = meshes;
  mMatrices = matrices;
}

/*
 * vim: et sw=2
 */
