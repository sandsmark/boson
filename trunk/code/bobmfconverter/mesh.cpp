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


#include "mesh.h"

#include "debug.h"


Mesh::Mesh()
{
  mId = -1;
  mVertices = 0;
  mFaces = 0;
  mVertexCount = 0;
  mFaceCount = 0;
  mMaterial = 0;
  mIsTeamColor = false;
  mBaseNode = 0;
  mVertexArray = 0;
  mVertexArrayElements = 0;
}

Mesh::Mesh(Mesh* m)
{
  mId = -1;
  mVertices = 0;
  mFaces = 0;
  mVertexCount = 0;
  mFaceCount = 0;

  allocateVertices(m->vertexCount());
  allocateFaces(m->faceCount());

  // Copy vertices
  for(unsigned int i = 0; i < vertexCount(); i++)
  {
    Vertex* v1 = vertex(i);
    Vertex* v2 = m->vertex(i);
    v1->pos = v2->pos;
    v1->normal = v2->normal;
    v1->tex = v2->tex;
  }
  // Copy faces
  for(unsigned int i = 0; i < faceCount(); i++)
  {
    Face* f1 = face(i);
    Face* f2 = m->face(i);
    f1->normal = f2->normal;
    f1->smoothgroup = f2->smoothgroup;
    f1->setVertexCount(f2->vertexCount());
    // TODO: somehow copy vertices
    f1->setVertex(0, vertex(f2->vertex(0)->id));
    f1->setVertex(1, vertex(f2->vertex(1)->id));
    f1->setVertex(2, vertex(f2->vertex(2)->id));
  }

  mMaterial = m->material();

  mIsTeamColor = m->isTeamColor();
  mBaseNode = m->baseNode();
  mVertexArray = 0;
  mVertexArrayElements = 0;
  mName = m->name();
}

Mesh::~Mesh()
{
  for(unsigned int i = 0; i < mVertexCount; i++)
  {
    delete mVertices[i];
  }
  delete[] mVertices;
  for(unsigned int i = 0; i < mFaceCount; i++)
  {
    delete mFaces[i];
  }
  delete[] mFaces;

  delete[] mVertexArray;
}

void Mesh::allocateVertices(int n)
{
  delete[] mVertices;

  mVertices = new Vertex*[n];
  mVertexCount = n;

  for(unsigned int i = 0; i < mVertexCount; i++)
  {
    mVertices[i] = new Vertex(i);
  }
}

void Mesh::replaceVertexList(Vertex** vertices, int count)
{
  delete[] mVertices;

  mVertices = vertices;
  mVertexCount = count;
}

void Mesh::allocateFaces(int n)
{
  delete[] mFaces;

  mFaces = new Face*[n];
  mFaceCount = n;

  for(unsigned int i = 0; i < mFaceCount; i++)
  {
    mFaces[i] = new Face;
  }
}

void Mesh::replaceFaceList(Face** faces, int count)
{
  delete[] mFaces;

  mFaces = faces;
  mFaceCount = count;
}

void Mesh::createArrays()
{
  /*mVertexArrayElements = vertexCount();

  mVertexArray = new float[mVertexArrayElements * (3+3+2)];

  // Copy vertices to the array
  for(int i = 0; i < mVertexArrayElements; i++)
  {
    Vertex* v = vertex(i);
    // Position
    mVertexArray[(i * 8) + 0] = v->pos.x();
    mVertexArray[(i * 8) + 0] = v->pos.y();
    mVertexArray[(i * 8) + 0] = v->pos.z();
    // Normal
    mVertexArray[(i * 8) + 0] = v->normal.x();
    mVertexArray[(i * 8) + 0] = v->normal.y();
    mVertexArray[(i * 8) + 0] = v->normal.z();
    // Texcoord
    mVertexArray[(i * 8) + 0] = v->tex.x();
    mVertexArray[(i * 8) + 0] = v->tex.y();
  }*/

  mVertexArrayElements = faceCount() * 3;

  mVertexArray = new float[mVertexArrayElements * (3+3+2)];

  // Copy vertices to the array
  for(unsigned int i = 0; i < faceCount(); i++)
  {
    Face* f = face(i);
    for(unsigned int j = 0; j < 3; j++)
    {
      unsigned int pos = i*3 + j;
      Vertex* v = f->vertex(j);
      // Position
      mVertexArray[(pos * 8) + 0] = v->pos.x();
      mVertexArray[(pos * 8) + 1] = v->pos.y();
      mVertexArray[(pos * 8) + 2] = v->pos.z();
      // Normal
      mVertexArray[(pos * 8) + 3] = v->normal.x();
      mVertexArray[(pos * 8) + 4] = v->normal.y();
      mVertexArray[(pos * 8) + 5] = v->normal.z();
      // Texcoord
      mVertexArray[(pos * 8) + 6] = v->tex.x();
      mVertexArray[(pos * 8) + 7] = v->tex.y();
    }
  }
}

void Mesh::calculateFaceNormals()
{
  for(unsigned int i = 0; i < faceCount(); i++)
  {
    Face* f = face(i);
    BoVector3Float a = f->vertex(0)->pos;
    BoVector3Float b = f->vertex(1)->pos;
    BoVector3Float c = f->vertex(2)->pos;

    f->normal = BoVector3Float::crossProduct(c - b, a - b);
    f->normal.normalize();
  }
}

void Mesh::calculateVertexNormals()
{
  // Go through all vertices
  for(unsigned int i = 0; i < vertexCount(); i++)
  {
    Vertex* v = vertex(i);
    // Normal of this vertex will be average of normals of all faces that have
    //  this vertex
    BoVector3Float normal;
    for(unsigned int j = 0; j < v->faces.count(); j++)
    {
      normal += v->faces[j]->normal;
    }
    normal.normalize();
    v->normal = normal;
  }

#if 0
  // Go through all faces
  for(int i = 0; i < faceCount(); i++)
  {
    Face* f = face(i);
    if(f->smoothgroup)
    {
      // Face is in some smoothing group. Use smooth shading
      // For each vertex in the face...
      for (int j = 0; j < 3; j++)
      {
        // ... normal of this vertex will be average of normals of all faces
        //  that have this vertex
        BoVector3Float normal;
        for(int k = 0; k < faceCount(); k++)
        {
          Face* f2 = face(k);
          if(f->smoothgroup & f2->smoothgroup)
          {
            // Two faces have same smooth groups
            if(f2->hasVertex(f->vertex(j)))
            {
              // And it has our point
              normal += f2->normal;
            }
          }
        }
        normal.normalize();
        f->vertex(j)->normal = normal;
      }
    }
    else
    {
      // Face is not in any smoothing group. Use flat shading
      // TODO: duplicate vertices when a vertex is in two faces which have
      //  different smoothing groups
      f->vertex(0)->normal = f->normal;
      f->vertex(1)->normal = f->normal;
      f->vertex(2)->normal = f->normal;
    }
  }
#endif
}

void Mesh::smoothAllFaces()
{
  // Go through all faces
  for(unsigned int i = 0; i < faceCount(); i++)
  {
    face(i)->smoothgroup = 1;
  }
}

void Mesh::loadingCompleted()
{
  // Go through all faces
  for(unsigned int i = 0; i < faceCount(); i++)
  {
    Face* f = face(i);
    // And through all vertices of that face
    for(unsigned int j = 0; j < f->vertexCount(); j++)
    {
      Vertex* v = f->vertex(j);
      // Update vertex's smoothgroup
      if(v->faces.count() == 0)
      {
        v->smoothgroup = f->smoothgroup;
      }
      else
      {
        v->smoothgroup &= f->smoothgroup;
      }
      // Add the face to vertex's list of faces
      v->faces.append(f);
    }
  }
}

