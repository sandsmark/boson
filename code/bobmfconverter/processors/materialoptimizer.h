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

#ifndef MATERIALOPTIMIZER_H
#define MATERIALOPTIMIZER_H


#include "processor.h"

class Model;
class LOD;
class Material;


class MaterialOptimizer : public Processor
{
  public:
    MaterialOptimizer();
    virtual ~MaterialOptimizer();

    virtual bool process();
    void setResetMaterials(bool r)  { mReset = r; }


  protected:
    float materialDifference(Material* m1, Material* m2);
    void mergeMaterials(Material* merge, Material* valid);
    void resetMaterials();

    bool mReset;
};


#endif //MATERIALOPTIMIZER_H
