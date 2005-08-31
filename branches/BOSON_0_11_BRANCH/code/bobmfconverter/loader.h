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

#ifndef LOADER_H
#define LOADER_H


#include <qstring.h>

class Model;
class LOD;


class Loader
{
  public:
    Loader(Model* m, LOD* l, const QString& file);
    virtual ~Loader();

    static Loader* createLoader(Model* m, LOD* l, const QString& filename);

    virtual bool load() = 0;


  protected:
    const QString& filename() const  { return mFilename; }
    Model* model() const  { return mModel; }
    LOD* lod() const  { return mLOD; }


  private:
    Model* mModel;
    LOD* mLOD;
    QString mFilename;
};

#endif //LOADER_H
