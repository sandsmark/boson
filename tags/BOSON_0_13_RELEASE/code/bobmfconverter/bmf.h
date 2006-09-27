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

#ifndef BMF_H
#define BMF_H


/**
 * BMF (Boson Model Format) file magics.
 *
 * BMF file consists of hierarchy of chunks. Each chunk has a start and
 *  possibly end magic code associated with it.
 **/


// FILE_ID is the string at the beginning of BMF file.
// BMF file can be recognized by it.
#define BMF_FILE_ID                    "BMF "
#define BMF_FILE_ID_LEN                4


// Version of the BMF format
#define BMF_VERSION_MAJOR              0
#define BMF_VERSION_MINOR              1
#define BMF_VERSION_RELEASE            1

#define BMF_MAKE_VERSION_CODE(a, b, c)  ( ((a) << 16) | ((b) << 8) | (c) )
#define BMF_VERSION_CODE \
    BMF_MAKE_VERSION_CODE(BMF_VERSION_MAJOR, BMF_VERSION_MINOR, BMF_VERSION_RELEASE)


// Magics for different chunks
#define BMF_MAGIC_MODEL                0x100000
#define BMF_MAGIC_MODEL_END            0x100001
#define BMF_MAGIC_TEXTURES             0x100010
#define BMF_MAGIC_MATERIALS            0x100020
#define BMF_MAGIC_MODEL_INFO           0x100030
#define BMF_MAGIC_MODEL_INFO_END       0x100031
#define BMF_MAGIC_MODEL_NAME           0x100032
#define BMF_MAGIC_MODEL_COMMENT        0x100033
#define BMF_MAGIC_MODEL_AUTHOR         0x100034
#define BMF_MAGIC_MODEL_RADIUS         0x100036
#define BMF_MAGIC_MODEL_BBOX           0x100037
#define BMF_MAGIC_ARRAYS               0x100050
#define BMF_MAGIC_LODS                 0x100090
#define BMF_MAGIC_LOD                  0x100100
#define BMF_MAGIC_MESHES               0x100110
#define BMF_MAGIC_MESH_INFO            0x100111
#define BMF_MAGIC_MESH_DATA            0x100114
#define BMF_MAGIC_MESH_MISC            0x100118
#define BMF_MAGIC_FRAMES               0x100120
#define BMF_MAGIC_END                  0x109999


// Same as corresponding OpenGL constants
#define BMF_RENDERMODE_TRIANGLES       0x0004
#define BMF_RENDERMODE_TRIANGLE_STRIP  0x0005
#define BMF_RENDERMODE_TRIANGLE_FAN    0x0006

// 8-, 16- and 32-bit unsigned integers
#define BMF_DATATYPE_UNSIGNED_BYTE     0x1401
#define BMF_DATATYPE_UNSIGNED_SHORT    0x1403
#define BMF_DATATYPE_UNSIGNED_INT      0x1405


#endif //BMF_H
