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

#ifndef UNUSEDDATAREMOVER_H
#define UNUSEDDATAREMOVER_H


#include "processor.h"

class Model;
class LOD;
class Mesh;
class Material;
class Texture;


class UnusedDataRemover : public Processor
{
  public:
    UnusedDataRemover();
    virtual ~UnusedDataRemover();

    virtual bool process();

    void setProcessMaterialsAndTextures(bool p)  { mProcessAll = p; }
    bool processMaterialsAndTextures() const  { return mProcessAll; }


  protected:
    bool processMeshes();
    bool processMaterials();
    bool processTextures();
    bool isMeshValid(Mesh* m);
    bool isMaterialValid(Material* m);
    bool isTextureValid(Texture* m);


  private:
    bool mProcessAll;
};


#endif //UNUSEDDATAREMOVER_H
