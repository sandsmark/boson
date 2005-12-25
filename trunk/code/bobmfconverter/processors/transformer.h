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

#ifndef TRANSFORMER_H
#define TRANSFORMER_H


#include "processor.h"

class Model;
class LOD;
class Mesh;
class BoMatrix;
template<class T> class BoVector3;
typedef BoVector3<float> BoVector3Float;


class Transformer : public Processor
{
  public:
    Transformer();
    virtual ~Transformer();

    virtual bool process();

    void setModelSize(float size)  { mModelSize = size; }
    float modelSize() const  { return mModelSize; }

    void setCenterModel(bool center)  { mCenterModel = center; }
    float centerModel() const  { return mCenterModel; }


  protected:
    bool resizeModel();
    bool applyTransformations();
    bool applyTransformations(LOD* lod, Mesh* mesh);

    float transformedBBoxVolume(const BoVector3Float& origmin, const BoVector3Float& origmax, BoMatrix* matrix);


  private:
    float mModelSize;
    bool mCenterModel;
};


#endif //TRANSFORMER_H
