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

    inline BoMatrix* matrix(unsigned int i) const  { return mMatrices[i]; }
    void setMatrix(unsigned int i, BoMatrix* m);

    inline Mesh* mesh(unsigned int i) const  { return mMeshes[i]; }
    void setMesh(unsigned int i, Mesh* m);

    void allocateNodes(unsigned int i);
    inline unsigned int nodeCount() const  { return mNodeCount; }

    /**
     * Remove the mesh @p mesh from the allocated list of meshes. If @p mesh is
     * not referenced by this frame, then this method is a noop.
     *
     * If @p mesh is referenced by this frame, it is removed and the @ref
     * nodeCount is reduced accordingly (@p mesh may be referenced multiple
     * times!).
     **/
    void removeMesh(Mesh* mesh);

  private:
    int mId;
    BoMatrix** mMatrices;
    Mesh** mMeshes;
    unsigned int mNodeCount;
};

/*
 * vim: et sw=2
 */

#endif //FRAME_H
