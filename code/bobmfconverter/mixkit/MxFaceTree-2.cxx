/************************************************************************

  Part 2 of the MxFaceTree package -- Distance Queries

  The code in this file implements the methods for performing distance
  queries on face trees.  Since this code is moderately large, and not
  always necessary, it's here in its very own file.

  Copyright (C) 1998 Michael Garland.  See "COPYING.txt" for details.
  
  $Id$

 ************************************************************************/

#include "stdmix.h"
#include "MxFaceTree.h"
#include "MxHeap.h"
#include "MxGeom3D.h"

void MxFaceProbe::clear()
{
    id = MXID_NIL;
    dist = HUGE;

    upper_bound = HUGE;
    lower_bound = 0;

    visited = leaf = internal = 0;
}


