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


#include "frameoptimizer.h"

#include "debug.h"
#include "lod.h"
#include "frame.h"
#include "bo3dtools.h"

#include <qvaluelist.h>
#include <qvaluevector.h>


FrameOptimizer::FrameOptimizer(Model* m, LOD* l) : Processor(m, l)
{
  mRemoveAll = false;
}

FrameOptimizer::~FrameOptimizer()
{
}

bool FrameOptimizer::process()
{
  if(lod() == 0)
  {
    boError() << k_funcinfo << "NULL LOD!" << endl;
    return false;
  }

  if(lod()->frameCount() <= 1)
  {
    // Nothing to do
    return true;
  }

  // Go through all frames and remove identical ones
  // We only go through them linearly, because different animations may include
  //  equal frames.
  QValueVector<Frame*> validframes;
  Frame* lastframe = lod()->frame(0);
  validframes.append(lastframe);

  if(mRemoveAll)
  {
    // Delete other frames
    for(unsigned int i = 1; i < lod()->frameCount(); i++)
    {
      delete lod()->frame(i);
    }

    lod()->setFrames(validframes);
    boDebug() << k_funcinfo << "Removed all frames (but the first one)" << endl;
    return true;
  }

  for(unsigned int i = 1; i < lod()->frameCount(); i++)
  {
    Frame* f = lod()->frame(i);
    // Check if lastframe and f are equal
    bool equal = false;
    // FIXME: this needs meshes to be in same order in the frame
    if(f->nodeCount() == lastframe->nodeCount())
    {
      // FIXME: this needs meshes to be in same order in the frame
      equal = true;
      for(unsigned int j = 0; j < f->nodeCount(); j++)
      {
        if(f->mesh(j) != lastframe->mesh(j))
        {
          equal = false;
          break;
        }
        else if(!f->matrix(j)->isEqual(*lastframe->matrix(j)))
        {
          equal = false;
          break;
        }
      }
    }

    if(equal)
    {
      delete f;
    }
    else
    {
      validframes.append(f);
      lastframe = f;
    }
  }

  if(validframes.count() != lod()->frameCount())
  {
    // Replace frames list
    // Just for debug
    int oldcount = lod()->frameCount();
    int removedcount = lod()->frameCount() - validframes.count();
    lod()->setFrames(validframes);
    boDebug() << k_funcinfo << "Removed " << removedcount << " duplicate frames of " <<
        oldcount << endl;
  }

  return true;
}
