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


#include "mesh.h"

#include "debug.h"
#include "bmf.h"

Vertex* Face::vertex(unsigned int i) const
{
  if(i >= mNumVertices)
  {
    boError() << k_funcinfo << "index out of bounds: " << i << endl;
    return 0;
  }
  return mVertices[i];
}

void Face::setVertex(unsigned int i, Vertex* v)
{
  if(i >= mNumVertices)
  {
    boError() << k_funcinfo << "index out of bounds: " << i << endl;
    return;
  }
  mVertices[i] = v;
}


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
  mVertexArraySize = 0;
  mVertexArrayOffset = 0;
  mIndexArraySize = 0;
  mIndexArrayOffset = 0;
  mUseIndices = true;
  mRenderMode = BMF_RENDERMODE_TRIANGLES;
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
  mVertexArraySize = 0;
  mVertexArrayOffset = 0;
  mIndexArraySize = 0;
  mIndexArrayOffset = 0;
  mName = m->name();

  mUseIndices = m->useIndices();
  mRenderMode = m->renderMode();
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
}

Vertex* Mesh::vertex(unsigned int i) const
{
  if(i >= vertexCount())
  {
    boError() << k_funcinfo << "index out of bounds: " << i << endl;
    return 0;
  }
  return mVertices[i];
}

Face* Mesh::face(unsigned int i) const
{
  if(i >= faceCount())
  {
    boError() << k_funcinfo << "index out of bounds: " << i << endl;
    return 0;
  }
  return mFaces[i];
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

void Mesh::replaceVertexList(Vertex** vertices, unsigned int count)
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

void Mesh::replaceFaceList(Face** faces, unsigned int count)
{
  delete[] mFaces;

  mFaces = faces;
  mFaceCount = count;
}

void Mesh::createArrays(float* vertices, unsigned char* indices,
    unsigned int* vertexoffset, unsigned int* indexoffset, unsigned int indextype)
{
  mVertexArrayOffset = *vertexoffset;
  mIndexArrayOffset = *indexoffset;

  float* varray = vertices + (mVertexArrayOffset * 8);

  if(useIndices())
  {
    // Copy vertices to the array
    for(unsigned int i = 0; i < vertexCount(); i++)
    {
      Vertex* v = vertex(i);
      // Position
      varray[(i * 8) + 0] = v->pos.x();
      varray[(i * 8) + 1] = v->pos.y();
      varray[(i * 8) + 2] = v->pos.z();
      // Normal
      varray[(i * 8) + 3] = v->normal.x();
      varray[(i * 8) + 4] = v->normal.y();
      varray[(i * 8) + 5] = v->normal.z();
      // Texcoord
      varray[(i * 8) + 6] = v->tex.x();
      varray[(i * 8) + 7] = v->tex.y();
    }
    mVertexArraySize = vertexCount();
    *vertexoffset += mVertexArraySize;

    // Copy the indices
    for(unsigned int i = 0; i < faceCount(); i++)
    {
      Face* f = face(i);
      for(unsigned int j = 0; j < 3; j++)
      {
        unsigned int pos = i*3 + j;
        Vertex* v = f->vertex(j);
        if(indextype == BMF_DATATYPE_UNSIGNED_SHORT)
        {
          ((quint16*)indices)[mIndexArrayOffset + pos] = (quint16)(mVertexArrayOffset + v->id);
        }
        else
        {
          ((quint32*)indices)[mIndexArrayOffset + pos] = (quint32)(mVertexArrayOffset + v->id);
        }
      }
    }
    mIndexArraySize = faceCount()*3;
    *indexoffset += mIndexArraySize;
  }
  else
  {
    // Copy vertices to the array
    for(unsigned int i = 0; i < faceCount(); i++)
    {
      Face* f = face(i);
      for(unsigned int j = 0; j < 3; j++)
      {
        unsigned int pos = i*3 + j;
        Vertex* v = f->vertex(j);
        // Position
        varray[(pos * 8) + 0] = v->pos.x();
        varray[(pos * 8) + 1] = v->pos.y();
        varray[(pos * 8) + 2] = v->pos.z();
        // Normal
        varray[(pos * 8) + 3] = v->normal.x();
        varray[(pos * 8) + 4] = v->normal.y();
        varray[(pos * 8) + 5] = v->normal.z();
        // Texcoord
        varray[(pos * 8) + 6] = v->tex.x();
        varray[(pos * 8) + 7] = v->tex.y();
      }
    }
    mVertexArraySize = faceCount()*3;
    *vertexoffset += mVertexArraySize;
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
    for(int j = 0; j < v->faces.count(); j++)
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

void Mesh::updateBoundingBox()
{
  mMinCoord = vertex(0)->pos;
  mMaxCoord = vertex(0)->pos;
  // Go through all vertices
  for(unsigned int i = 0; i < vertexCount(); i++)
  {
    Vertex* v = vertex(i);
    mMinCoord.setX(qMin(mMinCoord.x(), v->pos.x()));
    mMinCoord.setY(qMin(mMinCoord.y(), v->pos.y()));
    mMinCoord.setZ(qMin(mMinCoord.z(), v->pos.z()));
    mMaxCoord.setX(qMax(mMaxCoord.x(), v->pos.x()));
    mMaxCoord.setY(qMax(mMaxCoord.y(), v->pos.y()));
    mMaxCoord.setZ(qMax(mMaxCoord.z(), v->pos.z()));
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

/*
 * vim: et sw=2
 */
