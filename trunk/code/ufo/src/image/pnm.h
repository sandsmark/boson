/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/image/pnm.h
    begin             : Wed May 8 2002
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

#ifndef PNM_H
#define PNM_H

#include "ufo/image/uimageio.hpp"

// FIXME
// should be in namespace ufo, too
//UFO_BEGIN_NAMESPACE

enum PNMType {
	PBM = 1,
	PGM = 2,
	PPM = 3
};


// custom pnm header
struct PNMHeader
{
	/** image type code */
	unsigned char imageType;

	// width of image
	unsigned int imageWidth;
	// height of image
	unsigned int imageHeight;

	unsigned int maxValue;

	bool isAscii;

	std::string comment;
};

unsigned char * pnmLoader
(
	ufo::UImageIO * imageIOA,
	ufo::UImageIO::IStream &streamA,
	std::string * commentA,
	int * widthA,
	int * heightA,
	int * componentsA
	);


bool pnmSaver
(
	ufo::UImageIO * imageIOA,
	ufo::UImageIO::OStream & streamA
);

//UFO_END_NAMESPACE

#endif // __TGA_H
