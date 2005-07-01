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

#ifndef SAVER_H
#define SAVER_H


#include <qstring.h>

class Model;
class Texture;
class Material;
class LOD;
class Mesh;
class Frame;


class Saver
{
  public:
    Saver(Model* m, const QString& filename);

    bool save();


  private:
    bool saveModel(QDataStream& stream, Model* model);
    bool saveTexture(QDataStream& stream, Texture* tex);
    bool saveMaterial(QDataStream& stream, Material* mat);
    bool saveLOD(QDataStream& stream, LOD* lod);
    bool saveMesh(QDataStream& stream, Mesh* mesh);
    bool saveFrame(QDataStream& stream, Frame* frame);

    QString mFilename;
    Model* mModel;
};

#endif //SAVER_H
