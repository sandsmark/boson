/* -*- c -*- */
#ifndef INCLUDED_LIB3DS_VERSION_H
#define INCLUDED_LIB3DS_VERSION_H
/*
 * The 3D Studio File Format Library
 * Copyright (C) 1996-2001 by J.E. Hoffmann <je-h@gmx.net>
 * All rights reserved.
 *
 * This program is  free  software;  you can redistribute it and/or modify it
 * under the terms of the  GNU Lesser General Public License  as published by 
 * the  Free Software Foundation;  either version 2.1 of the License,  or (at 
 * your option) any later version.
 *
 * This  program  is  distributed in  the  hope that it will  be useful,  but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or  FITNESS FOR A  PARTICULAR PURPOSE.  See the  GNU Lesser General Public  
 * License for more details.
 *
 * You should  have received  a copy of the GNU Lesser General Public License
 * along with  this program;  if not, write to the  Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#define LIB3DS_VERSION_STRING "1.3.0"
#define LIB3DS_VERSION_MAJOR 1
#define LIB3DS_VERSION_MINOR 3
#define LIB3DS_VERSION_RELEASE 0
#define LIB3DS_MAKE_VERSION( a,b,c ) (((a) << 16) | ((b) << 8) | (c))

#define LIB3DS_VERSION \
  LIB3DS_MAKE_VERSION(LIB3DS_VERSION_MAJOR,LIB3DS_VERSION_MINOR,LIB3DS_VERSION_RELEASE)

#define LIB3DS_IS_VERSION(a,b,c) ( LIB3DS_VERSION >= LIB3DS_MAKE_VERSION(a,b,c) )

#endif

