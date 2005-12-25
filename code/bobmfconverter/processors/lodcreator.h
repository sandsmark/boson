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

#ifndef LODCREATOR_H
#define LODCREATOR_H


#include "processor.h"

class Model;
class Mesh;
class MxStdModel;
class MxQSlim;


class LodCreator : public Processor
{
  public:
    LodCreator(int lodIndex);
    virtual ~LodCreator();

    virtual bool initProcessor(Model* model);
    virtual bool process();

    void setFaceTargetFactor(float factor)  { mTargetFactor = factor; }
    float faceTargetFactor() const  { return mTargetFactor; }

    void setUseError(bool use)  { mUseError = use; }
    bool useError() const  { return mUseError; }

    void setUseBoth(bool use)  { mUseBoth = use; }
    bool useBoth() const  { return mUseBoth; }

    void setMaxError(float error)  { mMaxError = error; }
    float maxError() const  { return mMaxError; }


  protected:
    bool processMesh(Mesh* mesh);

    MxStdModel* loadMeshIntoMxModel(Mesh* mesh);
    MxQSlim* initSlim(MxStdModel* m);
    void cleanupModel(MxStdModel* m);
    bool updateMeshFromMxModel(Mesh* mesh, MxStdModel* model);


  private:
    int mLODIndex;
    MxStdModel* mMxModel;
    float mTargetFactor;
    float mMaxError;
    bool mUseError;
    bool mUseBoth;
};


#endif //LODCREATOR_H
