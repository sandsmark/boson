#ifndef MXCAMERA_INCLUDED // -*- C++ -*-
#define MXCAMERA_INCLUDED
#if !defined(__GNUC__)
#  pragma once
#endif

/************************************************************************

  MxCamera

  Copyright (C) 1998 Michael Garland.  See "COPYING.txt" for details.
  
  $Id$

 ************************************************************************/

#include "MxGL.h"
#include "MxGeom3D.h"

class MxCamera
{
public:

    double fovy, aspect;
    double znear, zfar;
    Vec3 from, at, up;

    void look_at(const MxBounds&, double ratio=1.333333);

    void apply();
    void apply(GLenum mode);
};



// MXCAMERA_INCLUDED
#endif
