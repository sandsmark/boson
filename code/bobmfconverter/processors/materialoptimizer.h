/*
    This file is part of the Boson game
    Copyright (C) 2005 The Boson Team (boson-devel@lists.sourceforge.net)

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

#ifndef MATERIALOPTIMIZER_H
#define MATERIALOPTIMIZER_H


#include "processor.h"

class Model;
class LOD;
class Material;


class MaterialOptimizer : public Processor
{
  public:
    MaterialOptimizer(Model* m, LOD* l);
    virtual ~MaterialOptimizer();

    virtual bool process();


  protected:
    float materialDifference(Material* m1, Material* m2);
    void mergeMaterials(Material* merge, Material* valid);
};


#endif //MATERIALOPTIMIZER_H
