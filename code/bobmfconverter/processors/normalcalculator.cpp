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


#include "normalcalculator.h"

#include "debug.h"
#include "model.h"
#include "lod.h"
#include "mesh.h"
//Added by qt3to4:
#include <Q3ValueList>


NormalCalculator::NormalCalculator(float threshold) : Processor()
{
  setName("NormalCalculator");
  mThreshold = threshold;
}

NormalCalculator::~NormalCalculator()
{
}

bool NormalCalculator::process()
{
  if(lod() == 0)
  {
    boError() << k_funcinfo << "NULL LOD!" << endl;
    return false;
  }

  for(unsigned int i = 0; i < lod()->meshCount(); i++)
  {
    if(!processMesh(lod()->mesh(i)))
    {
      return false;
    }
  }

  return true;
}

bool NormalCalculator::processMesh(Mesh* mesh)
{
  mesh->calculateFaceNormals();

  // Create new, non-shared vertices
  int vertexcount = mesh->faceCount() * 3;
  Vertex** vertices = new Vertex*[vertexcount];
  for(unsigned int i = 0; i < mesh->faceCount(); i++)
  {
    Face* f = mesh->face(i);
    for(unsigned int j = 0; j < 3; j++)
    {
      Vertex* v = new Vertex(i*3 + j);
      v->pos = f->vertex(j)->pos;
      v->normal = f->normal;
      v->tex = f->vertex(j)->tex;

      v->faces.append(f);
      f->setVertex(j, v);

      vertices[i*3 + j] = v;
    }
  }

  // Delete old vertices
  for(unsigned int i = 0; i < mesh->vertexCount(); i++)
  {
    delete mesh->vertex(i);
  }

  // Replace vertices
  mesh->replaceVertexList(vertices, vertexcount);

  // Find smooth areas
  bool* processed = new bool[vertexcount];
  for(int i = 0; i < vertexcount; i++)
  {
    processed[i] = false;
  }

  for(int i = 0; i < vertexcount; i++)
  {
    // Find all vertices with same position
    Q3ValueList<Vertex*> candidates;
    processed[i] = true;

    for(int j = i; j < vertexcount; j++)
    {
      if(processed[j])
      {
        continue;
      }
      if(vertices[i]->pos.isEqual(vertices[j]->pos))
      {
        candidates.append(vertices[j]);
      }
    }

    if(candidates.count() > 0)
    {
      Q3ValueList<Vertex*> merge;  // Vertices that will actually be merged
      merge.append(vertices[i]);
      BoVector3Float n = vertices[i]->normal;
      for(Q3ValueList<Vertex*>::Iterator it = candidates.begin(); it != candidates.end(); ++it)
      {
        // Vertices in two faces are merged if the dot product of normals of
        //  these faces exceeds the threshold
        if(BoVector3<float>::dotProduct(vertices[i]->normal, (*it)->normal) >= mThreshold)
        {
          n += (*it)->normal;
          merge.append(*it);
        }
      }

      n.normalize();
      for(Q3ValueList<Vertex*>::Iterator it = merge.begin(); it != merge.end(); ++it)
      {
        (*it)->normal = n;
        processed[(*it)->id] = true;
      }
      //boDebug() << k_funcinfo << "Merged " << merge.count() << " of " << candidates.count()+1 << " candidates" << endl;
    }
  }

  return true;
}

