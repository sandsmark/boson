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


#include "processor.h"

#include "model.h"
#include "debug.h"


unsigned int Processor::mBaseFrame = 0;


Processor::Processor()
{
  mName = "Unnamed";
  mModel = 0;
  mLOD = 0;
}

Processor::~Processor()
{
}

bool Processor::initProcessor(Model* model)
{
  mModel = model;
  if(!mModel)
  {
    BO_NULL_ERROR(mModel);
    return false;
  }
  setLOD(mModel->baseLOD());
  if(!mLOD)
  {
    BO_NULL_ERROR(mLOD);
    return false;
  }
  return true;
}

void Processor::setLOD(LOD* lod)
{
  mLOD = lod;
}

/*
 * vim: et sw=2
 */
