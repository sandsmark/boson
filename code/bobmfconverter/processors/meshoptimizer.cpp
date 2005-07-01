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


#include "meshoptimizer.h"

#include "debug.h"
#include "lod.h"
#include "frame.h"
#include "mesh.h"

#include <qvaluelist.h>
#include <qvaluevector.h>


MeshOptimizer::MeshOptimizer(Model* m, LOD* l) : Processor(m, l)
{
}

MeshOptimizer::~MeshOptimizer()
{
}

bool MeshOptimizer::process()
{
  if(lod() == 0)
  {
    boError() << k_funcinfo << "NULL LOD!" << endl;
    return false;
  }

  QValueList<Mesh*> meshes;
  QValueVector<Mesh*> validmeshes;
  // Put all meshes of the lod to meshes list
  for(unsigned int i = 0; i < lod()->meshCount(); i++)
  {
    meshes.append(lod()->mesh(i));
  }

  while(meshes.count() > 0)
  {
    QValueList<Mesh*> equal;
    // Move first mesh from meshes list to equal
    equal.append(meshes.first());
    meshes.pop_front();
    // Find all meshes that can be merged with this mesh
    findEqualMeshes(&equal, &meshes);
    // Merge equal meshes
    Mesh* merged = mergeMeshes(&equal);
    if(!merged)
    {
      return false;
    }
    validmeshes.append(merged);
  }

  if(validmeshes.count() != lod()->meshCount())
  {
    int oldcount = lod()->meshCount();
    lod()->setMeshes(validmeshes);
    boDebug() << k_funcinfo << "Merged " << oldcount << " meshes into " << validmeshes.count() << endl;
  }

  return true;
}

void MeshOptimizer::findEqualMeshes(QValueList<Mesh*>* equal, QValueList<Mesh*>* rest)
{
  // For two meshes two be equal, they need to have:
  //  * same material
  //  * same isTeamColor() status
  //  * same matrices in all the frames
  //  * only one node (TODO: in some cases (where all nodes also have "equal"
  //    matrices in all frames), we still can merge such meshes)
  //  * has one node _in same frames_ : TODO!!!
  if(rest->count() == 0)
  {
    return;
  }
  // Check if mesh has multiple nodes
  Mesh* mesh = equal->first();
  if(hasMultipleNodes(mesh))
  {
    return;
  }

  int seen = 0;
  int shouldsee = rest->count();
  for(QValueList<Mesh*>::Iterator it = rest->begin(); it != rest->end(); ++it)
  {
    seen++;
    Mesh* test = *it;
    // Check if mesh and test are equal
    if(mesh->material() != test->material())
    {
      continue;
    }
    else if(mesh->isTeamColor() != test->isTeamColor())
    {
      continue;
    }
    else if(hasMultipleNodes(test))
    {
      continue;
    }
    else if(!areInSameFrames(mesh, test))
    {
      continue;
    }
    else
    {
      equal->append(*it);
      it = rest->remove(it);
      it--;
    }
  }
  if(seen != shouldsee)
  {
    boError() << "seen != shouldsee : " << seen << " != " << shouldsee << endl;
    return;
  }
}

bool MeshOptimizer::hasMultipleNodes(Mesh* m)
{
  for(unsigned int i = 0; i < lod()->frameCount(); i++)
  {
    bool onenodefound = false;
    Frame* f = lod()->frame(i);
    for(unsigned int j = 0; j < f->nodeCount(); j++)
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

bool MeshOptimizer::areInSameFrames(Mesh* m1, Mesh* m2)
{
  for(unsigned int i = 0; i < lod()->frameCount(); i++)
  {
    Frame* f = lod()->frame(i);
    bool hasm1 = false;
    bool hasm2 = false;
    for(unsigned int j = 0; j < f->nodeCount(); j++)
    {
      if(f->mesh(j) == m1)
      {
        hasm1 = true;
      }
      else if(f->mesh(j) == m2)
      {
        hasm2 = true;
      }
      if(hasm1 && hasm2)
      {
        break;
      }
    }
    if(hasm1 != hasm2)
    {
      return false;
    }
  }
  return true;
}

Mesh* MeshOptimizer::mergeMeshes(QValueList<Mesh*>* equal)
{
  // Base mesh, all other meshes will be added to this
  Mesh* base = equal->first();
  if(equal->count() == 1)
  {
    return base;
  }

  QValueList<Mesh*>::Iterator it = equal->begin();
  // Pass 1: count faces/vertices in all meshes
  unsigned int totalvertices = 0;
  unsigned int totalfaces = 0;
  for(; it != equal->end(); ++it)
  {
    totalvertices += (*it)->vertexCount();
    totalfaces += (*it)->faceCount();
  }

  // Pass 2: add all vertices and faces of other meshes to base mesh
  // Allocate arrays for meshes and vertices
  Vertex** vertices = new Vertex*[totalvertices];
  Face** faces = new Face*[totalfaces];
  unsigned int vertexi = 0;
  unsigned int facei = 0;
  // We store pointers to deleted meshes here so that we can later remove nodes
  //  (in frames) that have such meshes.
  QValueList<Mesh*> deletedmeshes;

  it = equal->begin();
  for(; it != equal->end(); ++it)
  {
    Mesh* other = *it;
    // Note that we don't create/delete actual faces and vertices, but just
    //  copy around pointers.
    for(unsigned int i = 0; i < other->vertexCount(); i++)
    {
      other->vertex(i)->id = vertexi;
      vertices[vertexi++] = other->vertex(i);
    }
    for(unsigned int i = 0; i < other->faceCount(); i++)
    {
      faces[facei++] = other->face(i);
    }
    if(other != base)
    {
      // Set vertex and face arrays of the mesh to 0 to avoid deleting
      //  vertex/face objects when the mesh is deleted
      other->replaceVertexList(0, 0);
      other->replaceFaceList(0, 0);
      // Remove the mesh from all frames/nodes
      // TODO!!!!!!!!!!!!!!!!!!!!!!11111
      // Delete the mesh
      deletedmeshes.append(other);
      delete other;
    }
  }
  if(totalvertices != vertexi)
  {
    boError() << "totalvertices != vertexi : " << totalvertices << " != " << vertexi << endl;
    return 0;
  }
  if(totalfaces != facei)
  {
    boError() << "totalfaces != facei : " << totalfaces << " != " << facei << endl;
    return 0;
  }

  // Replace face/vertex list in base mesh
  base->replaceVertexList(vertices, totalvertices);
  base->replaceFaceList(faces, totalfaces);

  // Remove nodes with deleted meshes
  boDebug() << k_funcinfo << "Removing unused nodes for " << deletedmeshes.count()
      << " deleted meshes..." << endl;
  for(unsigned int i = 0; i < lod()->frameCount(); i++)
  {
    Frame* f = lod()->frame(i);
    QValueVector<Mesh*> newmeshes;
    QValueVector<BoMatrix*> newmatrices;
    for(unsigned int j = 0; j < f->nodeCount(); j++)
    {
      if(deletedmeshes.contains(f->mesh(j)))
      {
        // Delete the matrix too
        delete f->matrix(j);
      }
      else
      {
        newmeshes.append(f->mesh(j));
        newmatrices.append(f->matrix(j));
      }
    }
    // Replace mesh/matrix lists if necessary
    if(newmeshes.count() != f->nodeCount())
    {
      // Create temporary Mesh*/BoMatrix* arrays
      Mesh** meshes = new Mesh*[newmeshes.count()];
      BoMatrix** matrices = new BoMatrix*[newmeshes.count()];
      for(unsigned int j = 0; j < newmeshes.count(); j++)
      {
        meshes[j] = newmeshes[j];
        matrices[j] = newmatrices[j];
      }
      // Replace arrays in Frame
      f->replaceNodes(matrices, meshes, newmeshes.count());
    }
  }

  boDebug() << k_funcinfo << "Merged " << totalvertices << " vertices and " <<
      totalfaces << " faces from " << equal->count() << " meshes" << endl;

  return base;
}

