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

#ifndef MESH_H
#define MESH_H


#include "bo3dtools.h"

#include <qstring.h>
#include <qvaluevector.h>

class Material;


class Face;

class Vertex
{
  public:
    Vertex()  { faces.reserve(3); id = -1; smoothgroup = 0; }
    Vertex(int _id)  { faces.reserve(3); id = _id; smoothgroup = 0; }

    bool isDuplicate(Vertex* v) const
    {
      // Check pos, tex /*and normal*/
      if((v->pos == pos) /*&& (v->normal == normal)*/ && (v->tex == tex))
      {
        // All are equal
        // For vertices to be duplicates, all their faces must share a
        //  smoothing group, too (otherwise the normals will be different)
        return (smoothgroup & v->smoothgroup);
      }
      else
      {
        return false;
      }
    }

    BoVector3Float pos;
    BoVector3Float normal;
    BoVector2Float tex;
    int id;
    // All smoothing groups of faces AND'ed together (i.e. common smoothing
    //  group which all faces, that have this vertex, have)
    unsigned int smoothgroup;
    QValueVector<Face*> faces;
};

class Face
{
  public:
    Face()  { mNumVertices = 0; mVertices = 0; }
    ~Face()  { delete mVertices; }

    /**
     * Array containing all vertices of this face
     **/
    Vertex** vertices() const  { return mVertices; }

    Vertex* vertex(unsigned int i) const  { return mVertices[i]; }
    void setVertex(unsigned int i, Vertex* v)  { mVertices[i] = v; }

    /**
     * Number of vertices in this face
     **/
    unsigned int vertexCount() const { return mNumVertices; }
    void setVertexCount(int n)
    {
      mNumVertices = n;
      delete[] mVertices;
      mVertices = new Vertex*[mNumVertices];
    }
    bool hasVertex(Vertex* v) const
    {
      for(unsigned int i = 0; i < vertexCount(); i++)
      {
        if(vertex(i) == v)
        {
          return true;
        }
      }
      return false;
    }

    /**
     * Normal of this face (calculated from the vertices)
     **/
    BoVector3Float normal;
    /**
     * Smoothing group of the vertex.
     * All adjacent faces that belong to same smoothing group are smoothed.
     **/
    unsigned int smoothgroup;


  private:
    unsigned int mNumVertices;
    Vertex** mVertices;
    QValueVector<Face*> mNeighbors;
};


class Mesh
{
  public:
    Mesh();
    Mesh(Mesh* m);
    ~Mesh();

    int id() const  { return mId; }
    void setId(int id)  { mId = id; }

    Vertex* vertex(unsigned int i) const  { return mVertices[i]; }
    Face* face(unsigned int i) const  { return mFaces[i]; }

    Vertex** vertices() const  { return mVertices; }
    Face** faces() const  { return mFaces; }

    unsigned int vertexCount() const  { return mVertexCount; }
    unsigned int faceCount() const  { return mFaceCount; }

    void allocateVertices(int n);
    void allocateFaces(int n);

    void replaceVertexList(Vertex** vertices, int count);
    void replaceFaceList(Face** faces, int count);


    void smoothAllFaces();
    void loadingCompleted();

    void calculateFaceNormals();
    void calculateVertexNormals();


    Material* material() const  { return mMaterial; }
    void setMaterial(Material* mat)  { mMaterial = mat; }
    bool isTeamColor() const  { return mIsTeamColor; }
    void setIsTeamColor(bool is)  { mIsTeamColor = is; }


    const QString& name() const  { return mName; }
    void setName(const QString& n)  { mName = n; }

    unsigned int baseNode() const  { return mBaseNode; }
    void setBaseNode(unsigned int node)  { mBaseNode = node; }

    /**
     * Creates vertex and index arrays necessary for rendering.
     **/
    void createArrays();

    float* vertexArray() const  { return mVertexArray; }
    unsigned int vertexArrayElements() const  { return mVertexArrayElements; }


  private:
    int mId;

    Vertex** mVertices;
    Face** mFaces;

    unsigned int mVertexCount;
    unsigned int mFaceCount;

    Material* mMaterial;
    bool mIsTeamColor;
    unsigned int mBaseNode;

    // Interleaved vertex array for rendering
    float* mVertexArray;
    unsigned int mVertexArrayElements;

    QString mName;
};


#endif // MESH_H
