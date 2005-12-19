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

#ifndef MODEL_H
#define MODEL_H


#include "bo3dtools.h"

#include <qstring.h>
#include <qvaluevector.h>
#include <qdict.h>

class LOD;
class Material;
class Texture;


class Model
{
  public:
    Model();

    bool load(const QString& file);

    bool save(const QString& file);

    /**
     * Prepare model for saving.
     * You should call this before calling @ref save().
     *
     * This does some things such as creating arrays in meshes and assigning
     *  ids to textures and materials.
     * Note that if you do changes to the model after calling this method, then
     *  those changes may not be saved.
     **/
    void prepareForSaving(unsigned int baseframe);

    void updateIds();
    void createArrays();


    void calculateFaceNormals();
    void calculateVertexNormals();

    void smoothAllFaces();
    void loadingCompleted();


    /**
     * Creates base (full-detail) LOD
     **/
    void createBaseLOD();

    void createLODs(unsigned int num);

    void setLodDistances(float multiplier = 1.0f);

    LOD* lod(unsigned int i) const  { return mLODs[i]; }
    LOD* baseLOD() const  { return lod(0); }
    unsigned int lodCount() const  { return mLODs.count(); }

    Material* material(unsigned int i) const  { return mMaterials[i]; }
    unsigned int addMaterial(Material* m);
    unsigned int materialCount() const  { return mMaterials.count(); }
    void setMaterials(const QValueVector<Material*>& materials);

    void updateBoundingBox(unsigned int baseframe);
    void removeEmptyMeshes();

    const BoVector3Float& minCoord() const  { return mMinCoord; }
    const BoVector3Float& maxCoord() const  { return mMaxCoord; }

    /**
     * Returns texture with given filename from textures' dict.
     * If texture with such filename doesn't exist, it will be created.
     **/
    Texture* getTexture(const QString& filename);
    void addTexture(Texture* t);
    QDict<Texture>* texturesDict()  { return &mTextures; }
    void setTextures(const QDict<Texture>& textures);
    bool loadTextures();


    const QString& name() const  { return mName; }
    const QString& comment() const  { return mComment; }
    const QString& author() const  { return mAuthor; }
    void setName(const QString& name)  { mName = name; }
    void setComment(const QString& comment)  { mComment = comment; }
    void setAuthor(const QString& author)  { mAuthor = author; }

    float radius() const  { return mRadius; }
    void updateRadius(unsigned int baseframe);

    /**
     * @return Pointer to the vertex array
     **/
    float* vertexArray() const  { return mVertexArray; }
    /**
     * @return Number of vertices in the vertex array.
     * Note that every vertex consists of 8 floats, so the size of the array in
     * bytes is  vertexArraySize() * 8 * sizeof(float)
     **/
    unsigned int vertexArraySize() const  { return mVertexArraySize; }

    /**
     * @return Pointer to the index array
     **/
    unsigned char* indexArray() const  { return mIndexArray; }
    /**
     * @return Number of indices in the index array.
     **/
    unsigned int indexArraySize()  { return mIndexArraySize; }
    unsigned int indexArrayType()  { return mIndexArrayType; }

    /**
     * Check whether the loaded model is valid (e.g. no NULL pointers where
     * there shouldn't be any.
     * @return TRUE if the model seems to be valid, FALSE if an error was found.
     **/
    bool checkLoadedModel() const;

  private:
    QValueVector<LOD*> mLODs;
    QValueVector<Material*> mMaterials;
    QDict<Texture> mTextures;

    QString mName;
    QString mComment;
    QString mAuthor;

    // BBox
    BoVector3Float mMinCoord;
    BoVector3Float mMaxCoord;

    float mRadius;

    // Interleaved vertex array for rendering
    float* mVertexArray;
    unsigned int mVertexArraySize;

    // Indices array
    unsigned char* mIndexArray;
    unsigned int mIndexArraySize;
    unsigned int mIndexArrayType;

};

#endif //MODEL_H
