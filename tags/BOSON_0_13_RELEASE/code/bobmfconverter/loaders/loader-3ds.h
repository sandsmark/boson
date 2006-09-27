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

#ifndef LOADER3DS_H
#define LOADER3DS_H


#include "loader.h"

#include <qptrdict.h>

#include <lib3ds/types.h>

class Mesh;
class QStringList;
class Frame;


class Loader3DS : public Loader
{
  public:
    Loader3DS(Model* m, LOD* l, const QString& file);
    virtual ~Loader3DS();

    bool load();
    void finishLoading();


    /**
    * @return A list of all textures used in this model.
    **/
    static QStringList textures(Lib3dsFile* file);
    QStringList textures() const;


  protected:
    bool loadMaterials(Lib3dsMaterial* firstMaterial);
    void loadMesh(Lib3dsNode* node);
    bool loadFrame(int frame);
    void loadVertices(Mesh* myMesh, Lib3dsMesh* mesh);
    void loadTexels(Mesh* myMesh, Lib3dsMesh* mesh, Lib3dsMaterial* material);
    void loadFaces(Mesh* myMesh, Lib3dsMesh* mesh);
    void loadFrameNode(Frame* frame, int* index, Lib3dsNode* node);

    static QString textureName(const Lib3dsMesh* mesh, Lib3dsFile* file);
    static Lib3dsMaterial* material(const Lib3dsMesh* mesh, Lib3dsFile* file);
    static bool isTeamColor(const Lib3dsMesh* mesh);
    void countNodes(Lib3dsNode* node, int* count);


  private:
    Lib3dsFile* m3ds;
    QPtrDict<Mesh> mMesh2Mesh; // Lib3dsMesh to Mesh
};

#endif //LOADER-3DS_H
