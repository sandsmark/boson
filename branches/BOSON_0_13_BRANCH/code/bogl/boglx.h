/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann <b_mann@gmx.de>

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

#ifndef BOGLX_H
#define BOGLX_H

#include <bogl.h>

#include <qevent.h> // to avoid GLX/Qt conflicts
#include <GL/glx.h>

#ifndef GLX_NV_float_buffer
#define GLX_NV_float_buffer 1
#define GLX_FLOAT_COMPONENTS_NV         0x20B0
#endif

#ifndef GLX_SGIX_fbconfig
#define GLX_SGIX_fbconfig 1
typedef struct __GLXFBConfigRec *GLXFBConfigSGIX;
#endif

#ifndef GLX_SGIX_pbuffer
#define GLX_SGIX_pbuffer 1
#endif

#include <boglx_decl_p.h>



#endif // BOGLX_H
