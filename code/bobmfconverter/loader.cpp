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

#include "loader.h"

#include "loaders/loader-3ds.h"
#include "loaders/loader-ac.h"
#include "loaders/loader-md2.h"

#include "model.h"
#include "debug.h"


Loader::Loader(Model* m, LOD* l, const QString& file)
{
  mModel = m;
  mLOD = l;
  mFilename = file;
}

Loader::~Loader()
{
}

Loader* Loader::createLoader(Model* m, LOD* l, const QString& filename)
{
  if(filename.toLower().endsWith(".3ds"))
  {
    return (Loader*)(new Loader3DS(m, l, filename));
  }
  else if(filename.toLower().endsWith(".ac"))
  {
    return (Loader*)(new LoaderAC(m, l, filename));
  }
  else if(filename.toLower().endsWith(".md2"))
  {
    return (Loader*)(new LoaderMD2(m, l, filename));
  }
  else
  {
    boError() << k_funcinfo << "unrecognized file format of " << filename << endl;
    return 0;
  }
}

/*
 * vim: et sw=2
 */

