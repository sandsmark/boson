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

#ifndef LOD_H
#define LOD_H


#include <q3valuevector.h>

class Mesh;
class Frame;
class QString;


class LOD
{
  public:
    LOD();
    LOD(LOD* base);
    ~LOD();

    unsigned int addMesh(Mesh* m);
    Mesh* mesh(unsigned int i) const  { return mMeshes[i]; }
    unsigned int meshCount() const  { return mMeshes.count(); }
    void removeAllMeshesBut(const Q3ValueVector<Mesh*>& meshes);
    void removeReferencesToMesh(Mesh* mesh);

    unsigned int createFrame();
    Frame* frame(unsigned int i) const  { return mFrames[i]; }
    unsigned int frameCount() const  { return mFrames.count(); }
    void removeAllFramesBut(const Q3ValueVector<Frame*>& frames);

    float distance() const  { return mDist; }
    void setDistance(float dist)  { mDist = dist; }

    QString shortStats() const;


  private:
    Q3ValueVector<Mesh*> mMeshes;
    Q3ValueVector<Frame*> mFrames;
    float mDist;
};

#endif //LOD_H
