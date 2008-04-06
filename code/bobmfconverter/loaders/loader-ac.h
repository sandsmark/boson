/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

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

#ifndef LOADERAC_H
#define LOADERAC_H


#include "loader.h"
#include "bo3dtools.h"

#include <qmap.h>
#include <q3valuelist.h>
//Added by qt3to4:
#include <Q3TextStream>

class Material;
class Mesh;
class Face;
class QStringList;
class Frame;
class Q3TextStream;
class ACObject;
class ACFace;


class LoaderAC : public Loader
{
  public:
    LoaderAC(Model* m, LOD* l, const QString& file);
    virtual ~LoaderAC();

    virtual bool load();


  protected:
    // These load data from ac3d file into special objects in memory
    bool loadObject(Q3TextStream& stream, ACObject* obj);
    bool loadFace(Q3TextStream& stream, ACFace* face);
    bool loadMaterial(const QString& line);

    // This converts ACObject into BoMesh and adds it to frame
    bool convertIntoMesh(Frame* f, int* index, ACObject* obj);
    bool convertPolygonToFaces(ACObject* obj, int face, Mesh* boMesh, Face* boFaces, int* pointIndex);

    void countObjects(ACObject* obj, int* count);

    void translateObject(ACObject* obj, const BoVector3Float& trans);

    Material* requestMaterial(const QString& line, const QString& texture);


  private:
    QMap<QString, Q3ValueList<Material*> > mLine2Materials;
    Q3ValueList<QString> mMaterialLines;
};

/*
 * vim: et sw=4
 */
#endif
