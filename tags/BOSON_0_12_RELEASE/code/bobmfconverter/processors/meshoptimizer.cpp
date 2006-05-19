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


MeshOptimizer::MeshOptimizer() : Processor()
{
  setName("MeshOptimizer");
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
    lod()->removeAllMeshesBut(validmeshes);
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
    else if(animationsDiffer(mesh, test))
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

bool MeshOptimizer::animationsDiffer(Mesh* m1, Mesh* m2)
{
  BoMatrix* matrix = 0;

  for(unsigned int i = 0; i < lod()->frameCount(); i++)
  {
    Frame* f = lod()->frame(i);
    for(unsigned int j = 0; j < f->nodeCount(); j++)
    {
      if(f->mesh(j) == m1 || f->mesh(j) == m2)
      {
        if(matrix)
        {
          if(!matrix->isEqual(*f->matrix(j)))
          {
            // TODO: probably we could be able to merge in some cases even when
            //  matrices differ, e.g. if both meshes are translated by same
            //  amount
            return true;
          }
        }
        else
        {
          matrix = f->matrix(j);
        }
      }
    }
  }
  return false;
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
  unsigned int totalVertices = 0;
  unsigned int totalFaces = 0;
  for(; it != equal->end(); ++it)
  {
    totalVertices += (*it)->vertexCount();
    totalFaces += (*it)->faceCount();
  }

  // Pass 2: add all vertices and faces of other meshes to base mesh
  // Allocate arrays for meshes and vertices
  Vertex** vertices = new Vertex*[totalVertices];
  Face** faces = new Face*[totalFaces];
  unsigned int vertexi = 0;
  unsigned int facei = 0;

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
    }
  }
  if(totalVertices != vertexi)
  {
    boError() << "totalVertices != vertexi : " << totalVertices << " != " << vertexi << endl;
    return 0;
  }
  if(totalFaces != facei)
  {
    boError() << "totalFaces != facei : " << totalFaces << " != " << facei << endl;
    return 0;
  }

  // Replace face/vertex list in base mesh
  base->replaceVertexList(vertices, totalVertices);
  base->replaceFaceList(faces, totalFaces);

  boDebug() << k_funcinfo << "Merged " << totalVertices << " vertices and " <<
      totalFaces << " faces from " << equal->count() << " meshes" << endl;

  return base;
}

