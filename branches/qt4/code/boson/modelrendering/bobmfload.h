/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Rivo Laks (rivolaks@hot.ee)

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
#ifndef BOBMFLOAD_H
#define BOBMFLOAD_H

#include <bogl.h>

#include <qstring.h>
#include <q3valuevector.h>
//Added by qt3to4:
#include <Q3CString>

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


    /**
     * @return Filename of the cached model for given model- and configfile
     *  pair, or null QString if such model cannot be found
     **/
    static QString cachedModelFilename(const QString& modelfile, const QString& configfile);
    /**
     * Converts given model to bmf format, using given config file.
     * @return Filename of the bmf model or null QString if the conversion failed
     **/
    static QString convertModel(const QString& modelfile, const QString& configfile);


  protected:
    bool loadInfo(QDataStream& stream);
    bool loadTextures(QDataStream& stream);
    bool loadMaterials(QDataStream& stream);
    bool loadLODs(QDataStream& stream);
    bool loadLOD(QDataStream& stream, int lod);
    bool loadMeshes(QDataStream& stream, int lod);
    bool loadFrames(QDataStream& stream, int lod);
    bool loadArrays(QDataStream& stream);


    /**
    * @return The directory that contains the .3ds file. Usually the unit
    * directory
    **/
    const QString& baseDirectory() const;

    static Q3CString calculateHash(const QString& modelfile, const QString& configfile);
    static quint32 getVersion(const QString& modelfile);

    /**
     * Converts array from little-endian to big-endian.
     * @param elements Number of elements (e.g. floats) in the array
     * @param elementsize Size of a single element in bytes
     **/
    static void convertToBigEndian(char* array, unsigned int elements, unsigned int elementsize);

  private:
    void init();

  private:
    QString mFile;
    BosonModel* mModel;
    Q3ValueVector<QString> mTextureNames;
    bool* mTextureTransparent;
};


#endif //BOBMFLOAD_H

