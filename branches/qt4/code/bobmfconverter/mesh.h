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

#ifndef MESH_H
#define MESH_H


#include "bo3dtools.h"

#include <qstring.h>
#include <q3valuevector.h>

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
    Q3ValueVector<Face*> faces;
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

    Vertex* vertex(unsigned int i) const;
    void setVertex(unsigned int i, Vertex* v);

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
    Q3ValueVector<Face*> mNeighbors;
};


class Mesh
{
  public:
    Mesh();
    Mesh(Mesh* m);
    ~Mesh();

    int id() const  { return mId; }
    void setId(int id)  { mId = id; }

    Vertex* vertex(unsigned int i) const;
    Face* face(unsigned int i) const;

    Vertex** vertices() const  { return mVertices; }
    Face** faces() const  { return mFaces; }

    unsigned int vertexCount() const  { return mVertexCount; }
    unsigned int faceCount() const  { return mFaceCount; }

    void allocateVertices(int n);
    void allocateFaces(int n);

    void replaceVertexList(Vertex** vertices, unsigned int count);
    void replaceFaceList(Face** faces, unsigned int count);


    void smoothAllFaces();
    void loadingCompleted();

    void calculateFaceNormals();
    void calculateVertexNormals();

    void updateBoundingBox();

    const BoVector3Float& minCoord() const  { return mMinCoord; }
    const BoVector3Float& maxCoord() const  { return mMaxCoord; }


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
    void createArrays(float* vertices, unsigned char* indices,
        unsigned int* vertexoffset, unsigned int* indexoffset, unsigned int indextype);

    unsigned int vertexArrayOffset()  { return mVertexArrayOffset; }
    unsigned int vertexArraySize() const  { return mVertexArraySize; }

    unsigned int indexArrayOffset()  { return mIndexArrayOffset; }
    unsigned int indexArraySize()  { return mIndexArraySize; }

    bool useIndices() const  { return mUseIndices; }
    void setUseIndices(bool use)  { mUseIndices = use; }

    unsigned int renderMode() const  { return mRenderMode; }
    void setRenderMode(unsigned int mode)  { mRenderMode = mode; }


  private:
    // (Internal) id of the mesh
    int mId;

    // Vertices and faces
    Vertex** mVertices;
    Face** mFaces;
    unsigned int mVertexCount;
    unsigned int mFaceCount;

    // BBox
    BoVector3Float mMinCoord;
    BoVector3Float mMaxCoord;

    Material* mMaterial;
    bool mIsTeamColor;
    unsigned int mBaseNode;
    unsigned int mRenderMode;
    QString mName;

    // Interleaved vertex array for rendering
    unsigned int mVertexArraySize;
    unsigned int mVertexArrayOffset;
    // Indices array
    unsigned int mIndexArraySize;
    unsigned int mIndexArrayOffset;
    // Whether to use indices
    bool mUseIndices;
};


/*
 * vim: et sw=2
 */
#endif // MESH_H
