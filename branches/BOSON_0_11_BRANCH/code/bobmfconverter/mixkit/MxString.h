#ifndef MXSTRING_INCLUDED // -*- C++ -*-
#define MXSTRING_INCLUDED
#if !defined(__GNUC__)
#  pragma once
#endif

/************************************************************************

  String class

  Copyright (C) 1998 Michael Garland.  See "COPYING.txt" for details.
  
  $Id$

 ************************************************************************/

#include "MxBlock.h"

class MxString : public MxBlock<char>
{
public:
    MxString(unsigned int n) : MxBlock<char>(n) { }
    MxString(const char *str)
	{ int len=strlen(str)+1; init_block(len); bitcopy(str, len); }

    bool operator==(const char *s) const { return !strcmp(*this, s); }
};

// MXSTRING_INCLUDED
#endif
