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

#ifndef FRAME_H
#define FRAME_H


class Mesh;
class BoMatrix;


class Frame
{
  public:
    Frame();
    Frame(Frame* f);
    ~Frame();

    int id() const  { return mId; }
    void setId(int id)  { mId = id; }

    BoMatrix* matrix(unsigned int i) const  { return mMatrices[i]; }
    void setMatrix(unsigned int i, BoMatrix* m)  { mMatrices[i] = m; }

    Mesh* mesh(unsigned int i) const  { return mMeshes[i]; }
    void setMesh(unsigned int i, Mesh* m)  { mMeshes[i] = m; }

    void allocateNodes(unsigned int i);
    unsigned int nodeCount() const  { return mNodeCount; }
    void replaceNodes(BoMatrix** matrices, Mesh** meshes, unsigned int count);


  private:
    int mId;
    BoMatrix** mMatrices;
    Mesh** mMeshes;
    unsigned int mNodeCount;
};

#endif //FRAME_H
