/*
    This file is part of the Boson game
    Copyright (C) 2006 Rivo Laks (rivolaks@hot.ee)

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


#include "nodeoptimizer.h"

#include "debug.h"
#include "lod.h"
#include "frame.h"
#include "mesh.h"

#include <q3valuelist.h>
#include <q3valuevector.h>


NodeOptimizer::NodeOptimizer() : Processor()
{
  setName("NodeOptimizer");
}

NodeOptimizer::~NodeOptimizer()
{
}

bool NodeOptimizer::process()
{
  if(lod() == 0)
  {
    boError() << k_funcinfo << "NULL LOD!" << endl;
    return false;
  }

  Q3ValueList<Mesh*> meshes;
  Q3ValueVector<Mesh*> validmeshes;
  // Put all meshes of the lod to meshes list
  unsigned int initialMeshCount = lod()->meshCount();
  for(unsigned int i = 0; i < initialMeshCount; i++)
  {
    Mesh* m = lod()->mesh(i);
    if(hasMultipleNodes(m))
    {
      breakup(m);
    }
  }

  for(unsigned int i = 0; i < lod()->meshCount(); i++)
  {
    if(hasMultipleNodes(lod()->mesh(i)))
    {
      boError() << k_funcinfo << "mesh " << i << " still has multiple nodes!!!" << endl;
    }
  }

  if(lod()->meshCount() != initialMeshCount)
  {
    boDebug() << k_funcinfo << "Broke " << initialMeshCount << " meshes into " << lod()->meshCount() << endl;
  }

  return true;
}

bool NodeOptimizer::hasMultipleNodes(Mesh* m)
{
  for(unsigned int i = 0; i < lod()->frameCount(); i++)
  {
    bool onenodefound = false;
    Frame* f = lod()->frame(i);
    const unsigned int nodeCount = f->nodeCount();
    for(unsigned int j = 0; j < nodeCount; j++)
    {
      if(f->mesh(j) == m)
      {
        if(onenodefound)
        {
          // This is the second node
          return true;
        }
        onenodefound = true;
      }
    }
  }
  return false;
}

void NodeOptimizer::breakup(Mesh* mesh)
{
  Q3ValueVector<BoMatrix> mMatrices;
  Q3ValueVector<Mesh*> mMeshes;

  for(unsigned int i = 0; i < lod()->frameCount(); i++)
  {
    bool onenodefound = false;
    Frame* f = lod()->frame(i);
    const unsigned int nodeCount = f->nodeCount();
    for(unsigned int j = 0; j < nodeCount; j++)
    {
      if(f->mesh(j) != mesh)
      {
        continue;
      }
      else if(f->matrix(j)->isIdentity())
      {
        if(onenodefound)
        {
          boError() << k_funcinfo << "Multiple identity-matrix nodes of the same mesh!?" << endl;
        }
        onenodefound = true;
        continue;
      }
      // This node uses same mesh but non-identity matrix
      // Check if we already have a mesh corresponding to such matrix
      int index = -1;
      for(int k = 0; k < mMatrices.count(); k++)
      {
        if(f->matrix(j)->isEqual(mMatrices[k]))
        {
          index = (int)k;
          break;
        }
      }

      if(index == -1)
      {
        // Create new mesh
        Mesh* m = duplicateMesh(mesh, f->matrix(j));
        // Add it to the "cache"
        mMatrices.append(BoMatrix(*f->matrix(j)));
        mMeshes.append(m);
        // Update node and add the mesh to the lod
        f->setMesh(j, m);
        f->matrix(j)->loadIdentity();
        lod()->addMesh(m);
      }
      else
      {
        // Mesh with such transform already exists. Use it
        f->setMesh(j, mMeshes[index]);
        f->matrix(j)->loadIdentity();
      }
    }
  }
}

Mesh* NodeOptimizer::duplicateMesh(Mesh* original, BoMatrix* transform)
{
  // Create copy of the original mesh
  Mesh* m = new Mesh(original);

  // Tranform copy's vertices
  for(unsigned int i = 0; i < m->vertexCount(); i++)
  {
    Vertex* v = m->vertex(i);
    // Transform only the position
    BoVector3Float newpos;
    transform->transform(&newpos, &v->pos);
    v->pos = newpos;
  }

  return m;
}

