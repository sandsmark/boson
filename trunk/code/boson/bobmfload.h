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
#ifndef BOBMFLOAD_H
#define BOBMFLOAD_H

#include <bogl.h>

#include <qstring.h>
#include <qvaluevector.h>

class KSimpleConfig;
class QString;
class QStringList;
class BoMesh;
class BoFrame;
class BosonModel;
template<class T> class BoVector3;
typedef BoVector3<float> BoVector3Float;

/**
 * @author Rivo Laks <rivolaks@hot.ee>
 */
class BoBMFLoad
{
  public:
    BoBMFLoad(const QString& file, BosonModel* model);
    ~BoBMFLoad();

    bool loadModel();

    /**
    * @return The absolute filename to the .3ds file of this model.
    **/
    QString file() const;


  protected:
    bool loadInfo(QDataStream& stream);
    bool loadTextures(QDataStream& stream);
    bool loadMaterials(QDataStream& stream);
    bool loadLODs(QDataStream& stream);
    bool loadLOD(QDataStream& stream, int lod);
    bool loadMeshes(QDataStream& stream, int lod);
    bool loadFrames(QDataStream& stream, int lod);


    /**
    * @return The directory that contains the .3ds file. Usually the unit
    * directory
    **/
    const QString& baseDirectory() const;

  private:
    void init();

  private:
    QString mFile;
    BosonModel* mModel;
    QValueVector<QString> mTextureNames;
    float* mPointArray;
    unsigned int mPointArrayOffset;
};


#endif //BOBMFLOAD_H

