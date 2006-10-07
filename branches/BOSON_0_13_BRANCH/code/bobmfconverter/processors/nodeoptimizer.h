/*
    This file is part of the Boson game
    Copyright (C) 2006 Rivo Laks (rivolaks@hot.ee)

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

#ifndef NODEOPTIMIZER_H
#define NODEOPTIMIZER_H


#include "processor.h"

class Mesh;
class BoMatrix;


class NodeOptimizer : public Processor
{
  public:
    NodeOptimizer();
    virtual ~NodeOptimizer();

    virtual bool process();


  protected:
    bool hasMultipleNodes(Mesh* m);
    void breakup(Mesh* mesh);
    Mesh* duplicateMesh(Mesh* original, BoMatrix* transform);


  private:
};


#endif //NODEOPTIMIZER_H
