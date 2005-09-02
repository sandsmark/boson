/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/image/pnm.cpp
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


#include "pnm.h"

#include "ufo/util/ustring.hpp"

#include <cctype>

using namespace ufo;

//
// private load functions for bmp streams
//

int
eatWhitespaces(UImageIO::IStream & streamA) {
	uint8_t tempChar = streamA.peek();
	int ret = 0;


	// ignore leading whitespaces
	while (/*std::*/isspace(tempChar)) {
		streamA.ignore(1);
		tempChar = streamA.peek();
		ret ++;
	}
	return ret;
}

// eats comment lines and empty lines
// returns the comment
std::string
eatComments(UImageIO::IStream & streamA) {
	char tempChar;
	char comment[255];

	eatWhitespaces(streamA);
	tempChar = streamA.peek();

	if (tempChar == '#') {
		// this is a comment
		streamA.getline(comment, 255);
		return comment;
	}
	return "";
}

#define PNM_LOADER_ERROR(s) \
ufo::uWarning() << "couldn´t load pnm image.\n Reason: "<< s << std::endl;


PNMHeader
readPNMHeader(UImageIO::IStream &streamA) {
	PNMHeader header;

	char magic[2];
	
	streamA.read(magic, 2);

	header.imageType = magic[1] - '0';
	header.isAscii = true;
	if (header.imageType >= 4) {
		header.isAscii = false;
		header.imageType -= 3;
	}

	header.comment = eatComments(streamA);
	
#ifdef DEBUG
	std::cout << "  :: pnm magic " << magic[0] << magic[1] << std::endl;
	std::cout << "  :: pnm type " << uint32_t(header.imageType)
		<< "  ascii=" << header.isAscii << std::endl;
	std::cout << "  :: pnm comment " << header.comment << std::endl;
#endif

	streamA >> header.imageWidth;
	// unsigned char does not have ' ' characters
	eatWhitespaces(streamA);

	streamA >> header.imageHeight;
	eatWhitespaces(streamA);


#ifdef DEBUG
	std::cout << "  :: pnm width " << header.imageWidth
		<< "   height " << header.imageHeight << std::endl;
#endif

	if (header.imageWidth <= 0 || header.imageHeight <= 0) {
		PNM_LOADER_ERROR("Unable to read image width and height");
	}

	if (header.imageType != PBM) {
		streamA >> header.maxValue;
		if (header.maxValue <= 0 || header.maxValue > 255) {
			PNM_LOADER_ERROR("unsupported PNM format");
		}
	} else {
		header.maxValue = 255;	/* never scale PBMs */
	}

	/* binary PNM allows just a single character of whitespace after
	   the last parameter, and we've already consumed it */
	eatWhitespaces(streamA);
	
	return header;
}

unsigned char * pnmLoader(
		UImageIO * imageIOA,
		UImageIO::IStream &streamA,
		std::string * commentA,
		int * widthA,
		int * heightA,
		int * componentsA) {

	unsigned char * row;
	unsigned char * buf = NULL;

	unsigned char * data;
	int bpp;
	int bpl;

	PNMHeader header = readPNMHeader(streamA);

	
	*widthA = header.imageWidth;
	*heightA = header.imageHeight;
	*commentA = header.comment;

	if (header.imageType == PGM) {
		// 8-bit gray map
		bpp = 1;
		bpl = header.imageWidth;
	} else if (header.imageType == PBM) {
		// 8-bit gray map
		bpp = 1;
		// one byte for 8 pixel
		bpl = (header.imageWidth + 7) / 8;
		buf = new unsigned char[bpl];
	} else {
		/* 24-bit surface in R,G,B byte order */
		bpp = 3;
		bpl = header.imageWidth * 3;
	}
	data = new unsigned char[header.imageWidth * header.imageHeight * bpp];

	*componentsA = bpp;

	// Read the image into the pixel array
	row = data;
	for (unsigned int y = 0; y < header.imageHeight; y++) {
		if (header.isAscii) {
			eatWhitespaces(streamA);

			if (header.imageType == PBM) {
				for (unsigned int i = 0; i < header.imageWidth * bpp; i++) {
					uint8_t c;
					streamA >> c;

					eatWhitespaces(streamA);

					if (c) {
						row[i] = 255;
					} else {
						row[i] = 0;
					}
				}
			} else {
				for (int i = 0; i < bpl; i++) {
					uint8_t c;
					streamA >> c;

					row[i] = c;
				}
			}
		} else {
			uint8_t * dst = (header.imageType == PBM) ? buf : row;

			streamA.read((char*)dst, bpl);
			
			if (header.imageType == PBM) {
				// expand bitmap to 8bpp
				for (unsigned int i = 0; i < header.imageWidth; i++) {
					int bit = 7 - (i & 7);
					row[i] = (buf[i >> 3] >> bit) & 1;
				}
			}
		}
		if (header.maxValue < 255) {
			// scale up to full dynamic range (slow)
			for (unsigned int i = 0; i < header.imageWidth * bpp; i++) {
				row[i] = row[i] * 255 / header.maxValue;
			}
		}
		row += header.imageWidth * bpp;
	}

	if (header.imageType == PBM) {
		delete[](buf);
	}

	return (data);
}




bool pnmSaver(UImageIO * imageIOA, UImageIO::OStream & streamA) {
	int bpp = imageIOA->getImageComponents();
	std::string comment = imageIOA->getComment();
	int w = imageIOA->getWidth();
	int h = imageIOA->getHeight();
	unsigned char * src = imageIOA->getPixels();

	streamA << "P";
	if (bpp == 4 || bpp == 3) {
		// ignore Alpha
		streamA << "6";
	} else {
		streamA << "5";
	}
	streamA << "\n";
	if (comment != "") {
		streamA << "#" << comment.c_str() << std::endl;
	}
	streamA << UString::toString(w) << " "
	<< UString::toString(h) << "\n"
	<< "255" << "\n";

	unsigned char * data = NULL;
	uint32_t byteCount = 0;
	if (bpp == 4) {
		byteCount = w * h * 3;
		// remove alpha channel
		data = new unsigned char[byteCount];
		for (unsigned int i = 0, j = 0; i < byteCount; i += 3, j += 4) {
			data[i + 0] = src[j + 0];
			data[i + 1] = src[j + 1];
			data[i + 2] = src[j + 2];
		}
	} else if (bpp == 3) {
		byteCount = w * h * 3;
		data = src;
	} else if (bpp == 2) {
		byteCount = w * h;
		// remove alpha channel
		data = new unsigned char[byteCount];
		for (int i = 0; i < w * h * 2; i += 2) {
			data[i / 2] = src[i];
		}
	} else {
		byteCount = w * h;
		data = src;
	}

	streamA.write((char*)data, byteCount);

	// clean-up
	if (data != src) {
		delete[](data);
	}

	return streamA.good();
}


