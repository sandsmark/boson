/*
    This file is part of the Boson game
    Copyright (C) 2004 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef BOACLOAD_H
#define BOACLOAD_H

#include <qstring.h>

#include <GL/gl.h>

class KSimpleConfig;
class QString;
class QStringList;
class BoMesh;
class BoFrame;
class BosonModelLoaderData;
class ACObject;
class ACFace;
template<class T> class BoVector3;
typedef BoVector3<float> BoVector3Float;

/**
 * @author Rivo Laks <rivolaks@hot.ee>
 */
class BoACLoad
{
  public:
    BoACLoad(const QString& dir, const QString& file, BosonModelLoaderData* data);
    ~BoACLoad();

    bool loadModel();

    /**
    * @return The absolute filename to the .3ds file of this model.
    **/
    QString file() const;


  protected:
    // These load data from ac3d file into special objects in memory
    bool loadObject(QTextStream& stream, ACObject* obj);
    bool loadFace(QTextStream& stream, ACFace* face);
    bool loadMaterial(const QString& line);

    // This converts ACObject into BoMesh and adds it to frame
    bool convertIntoMesh(BoFrame* f, int* index, ACObject* obj);

    void countObjects(ACObject* obj, int* count);

    void translateObject(ACObject* obj, const BoVector3Float& trans);


    /**
    * @return The directory that contains the .3ds file. Usually the unit
    * directory
    **/
    const QString& baseDirectory() const;

  private:
    void init();

  private:
    QString mDirectory;
    QString mFile;
    BosonModelLoaderData* mData;
};


#endif

