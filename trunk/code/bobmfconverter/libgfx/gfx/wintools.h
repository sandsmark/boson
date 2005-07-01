#ifndef GFXWINTOOLS_INCLUDED // -*- C++ -*-
#define GFXWINTOOLS_INCLUDED
#if !defined(__GNUC__)
#  pragma once
#endif

/************************************************************************

  Support code for handling various tasks under Win32

  $Id$

 ************************************************************************/

#include <windows.h>

extern HGLRC create_glcontext(HDC dc);
extern int set_pixel_format(HDC dc);

// GFXWINTOOLS_INCLUDED
#endif
