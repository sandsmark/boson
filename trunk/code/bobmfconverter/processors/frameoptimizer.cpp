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


#include "frameoptimizer.h"

#include "debug.h"
#include "lod.h"
#include "frame.h"
#include "bo3dtools.h"

#include <qvaluelist.h>
#include <qvaluevector.h>


FrameOptimizer::FrameOptimizer() : Processor()
{
  mKeepFrames = 0;
  setName("FrameOptimizer");
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
  QValueVector<Frame*> validFrames;
  Frame* lastFrame = lod()->frame(0);
  validFrames.append(lastFrame);

  if(mKeepFrames > 0)
  {
    mKeepFrames = QMIN(mKeepFrames, (int)lod()->frameCount());
    for(int i = 1; i < mKeepFrames; i++)
    {
      validFrames.append(lod()->frame(i));
    }
    boDebug() << k_funcinfo << "Keeping only " << mKeepFrames << " first frames (of " << lod()->frameCount() << ")" << endl;
    lod()->removeAllFramesBut(validFrames);
    return true;
  }

  for(unsigned int i = 1; i < lod()->frameCount(); i++)
  {
    Frame* f = lod()->frame(i);
    // Check if lastFrame and f are equal
    bool equal = false;
    // FIXME: this needs meshes to be in same order in the frame
    if(f->nodeCount() == lastFrame->nodeCount())
    {
      // FIXME: this needs meshes to be in same order in the frame
      equal = true;
      for(unsigned int j = 0; j < f->nodeCount(); j++)
      {
        if(f->mesh(j) != lastFrame->mesh(j))
        {
          equal = false;
          break;
        }
        else if(!f->matrix(j)->isEqual(*lastFrame->matrix(j)))
        {
          equal = false;
          break;
        }
      }
    }

    if(!equal)
    {
      validFrames.append(f);
      lastFrame = f;
    }
  }

  if(validFrames.count() != lod()->frameCount())
  {
    // Replace frames list
    // Just for debug
    int oldcount = lod()->frameCount();
    int removedcount = lod()->frameCount() - validFrames.count();
    lod()->removeAllFramesBut(validFrames);
    boDebug() << k_funcinfo << "Removed " << removedcount << " duplicate frames of " <<
        oldcount << endl;
  }

  return true;
}

/*
 * vim: et sw=2
 */
