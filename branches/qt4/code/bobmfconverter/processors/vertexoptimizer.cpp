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


#include "vertexoptimizer.h"

#include "debug.h"
#include "lod.h"
#include "mesh.h"

#include <q3valuelist.h>
#include <q3ptrvector.h>
//Added by qt3to4:
#include <Q3PtrCollection>


class VertexOptimizer::VertexPtrVector : public Q3PtrVector<Vertex>
{
  public:
    VertexPtrVector(int size) : Q3PtrVector<Vertex>(size)  {}


  protected:
    virtual int compareItems(Q3PtrCollection::Item d1, Q3PtrCollection::Item d2)
    {
      return compareItems((Vertex*)d1, (Vertex*)d2);
    }

    virtual int compareItems(Vertex* v1, Vertex* v2)
    {
      // Compare pos
      float posxdiff = v2->pos.x() - v1->pos.x();
      if(posxdiff != 0)
      {
        return ((posxdiff > 0) ? 1 : -1);
      }
      float posydiff = v2->pos.y() - v1->pos.y();
      if(posydiff != 0)
      {
        return ((posydiff > 0) ? 1 : -1);
      }
      float poszdiff = v2->pos.z() - v1->pos.z();
      if(poszdiff != 0)
      {
        return ((poszdiff > 0) ? 1 : -1);
      }

      // Compare normal
      float normalxdiff = v2->normal.x() - v1->normal.x();
      if(normalxdiff != 0)
      {
        return ((normalxdiff > 0) ? 1 : -1);
      }
      float normalydiff = v2->normal.y() - v1->normal.y();
      if(normalydiff != 0)
      {
        return ((normalydiff > 0) ? 1 : -1);
      }
      float normalzdiff = v2->normal.z() - v1->normal.z();
      if(normalzdiff != 0)
      {
        return ((normalzdiff > 0) ? 1 : -1);
      }

      // Compare tex
      float texxdiff = v2->tex.x() - v1->tex.x();
      if(texxdiff != 0)
      {
        return ((texxdiff > 0) ? 1 : -1);
      }
      float texydiff = v2->tex.y() - v1->tex.y();
      if(texydiff != 0)
      {
        return ((texydiff > 0) ? 1 : -1);
      }

      return 0;
    }
};


VertexOptimizer::VertexOptimizer() : Processor()
{
  setName("VertexOptimizer");
}

VertexOptimizer::~VertexOptimizer()
{
}

bool VertexOptimizer::process()
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

bool VertexOptimizer::processMesh(Mesh* mesh)
{
  unsigned int validcount = 0;
  unsigned int removedcount = 0;

  Q3ValueList<unsigned int> removed;

  // Create temporary sorted list of vertices.
  // This speeds up duplicates search process a _lot_
  VertexPtrVector vertices(mesh->vertexCount());
  for(unsigned int i = 0; i < mesh->vertexCount(); i++)
  {
    vertices.insert(i, mesh->vertex(i));
  }
  boDebug() << "    VO::processMesh(): " << "Vertices added to list, sorting list..." << endl;
  vertices.sort();
  boDebug() << "    VO::processMesh(): " << "List sorted, checking for dups..." << endl;

  for(unsigned int i = 0; i < vertices.count(); i++)
  {
    if(removed.contains(i))
    {
      // Vertex has already been removed
      continue;
    }

    // This vertex isn't removed yet
    // Check if it has duplicates
    Vertex* v = vertices[i];
    validcount++;
    /*boDebug() << "  VO::processMesh(): " << "Processing vertex " << i << ": id: " << v->id <<
        "; pos: (" << v->pos.x() << "; " << v->pos.y() << "; " << v->pos.z() <<
        "); normal: (" << v->normal.x() << "; " << v->normal.y() << "; " << v->normal.z() <<
        "); tex: (" << v->tex.x() << "; " << v->tex.y() << ")" << endl;*/
    // Search for duplicates
    // Note that duplicates are always _after each other_ in the list
    while((i < vertices.count() - 1) && v->isDuplicate(vertices[i+1]))
    {
      // Duplicate found
      replaceVertex(mesh, vertices[i+1], v);
      removed.append(i+1);
      removedcount++;
      i++;
      if(i >= (vertices.count() - 1))
      {
        break;
      }
    }
  }

  if((validcount + removedcount) != vertices.count())
  {
    boError() << k_funcinfo << "Counts differ: " << validcount << " + " << removedcount <<
        " != " << vertices.count() << endl;
    return false;
  }
  boDebug() << k_funcinfo << "Vertices removed: " << removedcount << " of " << mesh->vertexCount() << endl;

  if(removedcount == 0)
  {
    return true;
  }

  // Create new vertex list for the mesh
  Vertex** newvertices = new Vertex*[validcount];

  // Copy valid vertices to the new list and update their id, delete other
  //  (invalid) vertices
  int newpos = 0;
  Q3ValueList<unsigned int>::Iterator it = removed.begin();
  for(unsigned int i = 0; i < vertices.count(); i++)
  {
    Vertex* v = vertices[i];
    // Check if removed list contains this vertex id
    bool vertexisremoved = false;
    while((it != removed.end()) && (*it <= i))
    {
      if(*it == i)
      {
        vertexisremoved = true;
      }
      ++it;
    }

    if(vertexisremoved)
    {
      delete v;
    }
    else
    {
      // This vertex is valid. Copy it to the new list and update it's id
      v->id = newpos;
      newvertices[newpos] = v;
      newpos++;
    }
  }

  if(newpos != (int)validcount)
  {
    boError() << k_funcinfo << "Wrong number of vertices was copied! copied: " << newpos <<
        "; should've: " << validcount << endl;
  }

  mesh->replaceVertexList(newvertices, validcount);

  boDebug() << "    VO::processMesh(): " << "Vertex list replaced" << endl;

#if 0
  for(int i = 0; i < mesh->vertexCount(); i++)
  {
    if(removed.contains(i))
    {
      // Vertex has already been removed
      continue;
    }

    // This vertex isn't removed yet
    // Check if it has duplicates
    Vertex* v = mesh->vertex(i);
    /*boDebug() << "  VO::processMesh(): " << "Processing vertex " << i <<
        ": pos: (" << v->pos.x() << "; " << v->pos.y() << "; " << v->pos.z() <<
        "); normal: (" << v->normal.x() << "; " << v->normal.y() << "; " << v->normal.z() <<
        "); tex: (" << v->tex.x() << "; " << v->tex.y() << ")" << endl;*/
    Q3ValueList<int> duplicates;
    for(int j = i + 1; j < mesh->vertexCount(); j++)
    {
      if(*v == *mesh->vertex(j))
      {
        // Vertices at i and j are equal: j will be removed
        //boDebug() << "    VO::processMesh(): " << "Found duplicate vertex " << j << " for " << i << endl;
        duplicates.append(j);
      }
    }
    if(!duplicates.isEmpty())
    {
      // Replace all duplicates with this vertex
      for(Q3ValueList<int>::Iterator it = duplicates.begin(); it != duplicates.end(); ++it)
      {
        replaceVertex(mesh, mesh->vertex(*it), v);
        removed.append(*it);
        removedcount++;
      }
    }
    validcount++;
  }
#endif

  return true;
}

void VertexOptimizer::replaceVertex(Mesh* mesh, Vertex* replace, Vertex* with)
{
  for(int i = 0; i < replace->faces.count(); i++)
  {
    Face* f = replace->faces[i];
    for(unsigned int j = 0; j < f->vertexCount(); j++)
    {
      if(f->vertex(j) == replace)
      {
        f->setVertex(j, with);
        with->faces.append(f);
      }
    }
  }
}
