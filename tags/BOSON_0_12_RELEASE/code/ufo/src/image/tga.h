/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/image/tga.h
    begin             : Thu May 9 2002
    $Id$
 ***************************************************************************/

/***************************************************************************
 *  This library is free software; you can redistribute it and/or          *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#ifndef TGA_H
#define TGA_H


/** TGA plugin for TGA 1.0 files.
  * @author Johannes Schmidt
  */

#include "ufo/image/uimageio.hpp"

// Original header found in the TGA 1.0 spec
struct TGAHeader
{
	unsigned char	idLength;		// length of ID string
	unsigned char	mapType;		// color map type, if 1 then there is a palette. if 0, then there is no palette.
	unsigned char	imageType;		// image type code

	unsigned short	mapStart;		// starting index of map
	unsigned short	mapLength;		// size of map in elements (next field is the element size.
	unsigned char	mapDepth;		// width of map in bits 8, 15, 16, 24, 32

	unsigned short	xOrigin;		// x-origin of image
	unsigned short	yOrigin;		// y-origin of image
	unsigned short	imageWidth;		// width of image
	unsigned short	imageHeight;	// height of image
	unsigned char	pixelDepth;		// bits per pixel
	unsigned char	imageDesc;		// image descriptor
	//1.) bits 0-3 contain the number of attribute bits per pixel.
	//             (16, and 32bit only) alpha channel, overlay or interrupt bits...
	//2.) bits 4,5 image origin location (0,0) of image.
	//              0,0 is origin at lower corner...
	//3.) bits 6,7 unused must be set to 0
};


#define RLEBUFSIZ	512			// size of largest possible RLE packet

unsigned char * tgaLoader(
	ufo::UImageIO * imageIOA,
	ufo::UImageIO::IStream &streamA,
	std::string * commentA,
	int * widthA,
	int * heightA,
	int * componentsA
);


bool tgaSaver(
	ufo::UImageIO * imageIOA,
	ufo::UImageIO::OStream & streamA
);


#endif
