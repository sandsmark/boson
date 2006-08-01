#ifndef MXMAT3_INCLUDED // -*- C++ -*-
#define MXMAT3_INCLUDED
#if !defined(__GNUC__)
#  pragma once
#endif

/************************************************************************

  3x3 Matrix class

  Copyright (C) 1998 Michael Garland.  See "COPYING.txt" for details.
  
  $Id$

 ************************************************************************/

#include <gfx/mat3.h>

// AB: removed due to licensing issues with MxMat[3|4]-jacobi.cxx which
// is required for this
/*
extern bool jacobi(const Mat3& m, Vec3& vals, Vec3 vecs[3]);
extern bool jacobi(const Mat3& m, double *vals, double *vecs);
*/

extern bool fast_jacobi(const Mat3& m, Vec3& vals, Vec3 vecs[3]);

// MXMAT3_INCLUDED
#endif
