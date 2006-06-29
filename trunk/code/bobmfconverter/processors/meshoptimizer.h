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

#ifndef MESHOPTIMIZER_H
#define MESHOPTIMIZER_H


#include "processor.h"

class Model;
class LOD;
class Mesh;
template<class T> class QValueList;


class MeshOptimizer : public Processor
{
  public:
    MeshOptimizer();
    virtual ~MeshOptimizer();

    virtual bool process();


  protected:
    void findEqualMeshes(QValueList<Mesh*>* equal, QValueList<Mesh*>* rest);
    Mesh* mergeMeshes(QValueList<Mesh*>* equal);
    bool hasMultipleNodes(Mesh* m);
    bool areInSameFrames(Mesh* m1, Mesh* m2);
    bool animationsDiffer(Mesh* m1, Mesh* m2);
//    bool areMeshesEqual(Mesh* m1, Mesh* m2);
    unsigned int computeFramesHash(Mesh* m);


  private:
    bool* mHasMultipleNodes;  // Stores whether a mesh has multiple nodes
    unsigned int* mMeshFrameHashes;  // Stores hash describing which frames a mesh is in
};


#endif //MESHOPTIMIZER_H
