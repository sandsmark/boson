/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/image/tga.cpp
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

#include "tga.h"

using namespace ufo;

// reads a RL encoded line
// dst: the destination array
// bufferSize: maximal decoded size of data before break (size of dst)
// bpp: bytes per pixel
// streamA: the stream which the RLE data is read from
int
readRLERow(unsigned char * dst, const int & bufferSize,
		const int & bpp, std::istream & streamA) {
	unsigned int value;
	unsigned char byte;   // for reading byte by byte.
	unsigned int i;
	unsigned char* q;
	int n(bufferSize);
	static unsigned char rleBuf[RLEBUFSIZ];
	
	while (n > 0) {
		streamA.read((char*)&byte, 1);
		value = byte;

		if (value & 0x80) {
			value &= 0x7f;
			value++;
			n -= value * bpp;
			if (n < 0) {
				return( -1 );
			}
			
			streamA.read((char*)rleBuf, bpp);

			while (value > 0) {
				*dst++ = rleBuf[0];
				if (bpp > 1) *dst++ = rleBuf[1];
				if (bpp > 2) *dst++ = rleBuf[2];
				if (bpp > 3) *dst++ = rleBuf[3];
				value--;
			}
		} else {
			value++;
			n -= value * bpp;
			if (n < 0)
				return(-1);

			// Maximum for value is 128 so as long as RLEBUFSIZ
			// is at least 512, and bpp is not greater than 4
			// we can read in the entire raw packet with one operation.
			
			streamA.read((char*)rleBuf, value * bpp);
			for (i = 0, q = rleBuf; i < (value * bpp); ++i) {
				*dst++ = *q++;
			}
		}
	}
	return(0);
}

// Reads a TGA header from a given stream
TGAHeader
readTGAHeader(UImageIO::IStream & streamA) {
	TGAHeader header;
	
	unsigned char temp[18];
	streamA.read((char*)temp, 18);

	// Start by reading the fields associated with the original TGA format.
	header.idLength = temp[0];

	header.mapType = temp[1];

	header.imageType = temp[2];

	// all values should be little endian
	header.mapStart = temp[3] | (unsigned int)(temp[4]) << 8;
	header.mapLength = temp[5] | (unsigned int)(temp[6]) << 8;
	header.mapDepth = temp[7];

	header.xOrigin = temp[8] | (unsigned int)(temp[9]) << 8;
	header.yOrigin = temp[10] | (unsigned int)(temp[11]) << 8;

	header.imageWidth = temp[12] | (unsigned int)(temp[13]) << 8;
	header.imageHeight = temp[14] | (unsigned int)(temp[15]) << 8;

	header.pixelDepth = temp[16];

	header.imageDesc = temp[17];

	return header;
}


// main loading function

unsigned char *
tgaLoader(
		UImageIO * imageIOA,
		UImageIO::IStream &streamA,
		std::string * commentA,
		int * widthA,
		int * heightA,
		int * componentsA)
{
	int width;
	int height;
	int bytesPerPixel;
	std::string idString;

	TGAHeader header = readTGAHeader(streamA);

	// FIXME
	// currently only 8 and 24 and 32 bit is supported
	if (header.pixelDepth != 8 && header.pixelDepth != 24 && header.pixelDepth != 32)
	{
		//Alert( false, "Importer only supports 24 and 32bit TGA." );
		int pixelDepth = header.pixelDepth;
		uError() << "TGA loading: "
		<< "Only 8, 24 and 32 color depth targa images supported" << "\n"
		<< "pixel depth: " << pixelDepth << "\n";
		return NULL;
	}
	
	//: header.imageType = ???  (* == supported)
	//   ImageType	Datatype		Colormap	Encoding
	//   0			no image data	n			n
	//   1			colormapped		y			n
	// * 2			truecolor		n			n
	// * 3			monochrome		n			n
	//   9			colormapped		y			y
	// * 10			truecolor		n			y
	//   11			monochrome		n			y

	//: currently only truecolor or monochrome (compressed, and uncompressed) is supported
	if (header.imageType != 3 && header.imageType != 11 && //monochrome
		header.imageType != 2 && header.imageType != 10)   //truecolor
	{
		uError() << "TGA loading: "
		<< "Only Truecolor (24 and 32bit) "
		<< "and monochrome (8bit) targa images supported" << "\n";
		return NULL;
	}

	// read the image id string.
	if (header.idLength > 0)
	{
		char * idStringChars = new char[header.idLength + 1];
		// read the string
		// NOTE: as long as idLength is char, it wont overflow the buffer.
		streamA.read(idStringChars, header.idLength);
		idStringChars[header.idLength] = '\0';
		*commentA = idStringChars;
	}

	// initialize the Image
	width = header.imageWidth - header.xOrigin;
	height = header.imageHeight - header.yOrigin;
	bytesPerPixel = (header.pixelDepth + 7) >> 3;
	
	char * data = new char[width * height * bytesPerPixel];
	
	// figure out stats for the colormap info (if any), we'll need the length so we can find the pixel area.
	int colorMapLength = 0;
	if (header.mapType == 1)
	{
		int bytesPerPaletteElement = (header.mapDepth + 7) >> 3;
		colorMapLength = bytesPerPaletteElement * header.mapLength;
	}

	// Truecolor data starts sizeof(TGAOriginalHeader) from beginning. (18)
	// This is true for both RLE and uncompressed scanlines.
	int dataAreaOffset = 18 + header.idLength + colorMapLength;
	streamA.seekg(dataAreaOffset);
	
	
	bool isRLE = false;
	if ((header.imageType == 10 || header.imageType == 11)) {
		isRLE = true;
	}
	
	//: Read Image Data --> truecolor/monochrome and compressed (easy)
	//: Read Image Data --> truecolor/monochrome and UNcompressed
	if ( header.imageType == 2 || header.imageType == 3
			|| header.imageType == 10 || header.imageType == 11) {
		int pitch = width * bytesPerPixel;

		//is the image stored upside down?
		bool isUpsideDown = ((header.imageDesc & (0x0001<<5)) == (0x0001<<5));

		// we use a coordinate system with top left corner as origin
		isUpsideDown = !isUpsideDown;

		//is the image stored flipped?
		bool isHorzFlipped = ((header.imageDesc & (0x0001<<4)) == (0x0001<<4));
/*
		int filterRule = imageIOA->getFilterRule();
		// Check now if we can already apply some desired states
		if (filterRule & UImageIO::FlipY) {
			isUpsideDown = !isUpsideDown;
			filterRule &= ~UImageIO::FlipY;
		}
		if (imageIOA->getFilterRule() & UImageIO::FlipX) {
			isHorzFlipped = !isHorzFlipped;
			filterRule &= ~UImageIO::FlipX;
		}
		imageIOA->setFilterRule(UImageIO::FilterRule(filterRule));
*/
		// pointer to the current row in data array
		char * rowPtr;
		
		// temp buffer for reading a chunk of bytes
		char * buf = new char[pitch];

		if (isUpsideDown) {
			rowPtr = data + (height - 1) * pitch;
		} else {
			rowPtr = data;
		}
		for (int y = 0; y < height; ++y) {
			if (isRLE) {
				readRLERow((unsigned char*)buf, pitch, bytesPerPixel, streamA );
			} else {
				streamA.read(buf, pitch);
			}

			for (int x = 0; x < pitch; x += bytesPerPixel) {
				switch (bytesPerPixel) {
					case 1:
						if (isHorzFlipped) {
							rowPtr[x] = buf[pitch - x - 1];
						} else {
							rowPtr[x] = buf[x];
						}
						break;
					case 3:
						// TGA saves image data as BGR
						if (isHorzFlipped) {
							rowPtr[x] = buf[pitch - x - 1];
							rowPtr[x + 1] = buf[pitch - x - 2];
							rowPtr[x + 2] = buf[pitch - x - 3];
						} else {
							rowPtr[x] = buf[x + 2];
							rowPtr[x + 1] = buf[x + 1];
							rowPtr[x + 2] = buf[x];
						}
						break;
					case 4:
						// TGA saves image data as BGRA
						if (isHorzFlipped) {
							rowPtr[x + 0] = buf[pitch - x - 2];
							rowPtr[x + 1] = buf[pitch - x - 3];
							rowPtr[x + 2] = buf[pitch - x - 4];
							rowPtr[x + 3] = buf[pitch - x - 1];
						} else {
							rowPtr[x + 0] = buf[x + 2];
							rowPtr[x + 1] = buf[x + 1];
							rowPtr[x + 2] = buf[x + 0];
							rowPtr[x + 3] = buf[x + 3];
							/*rowPtr[x] = buf[x];
							rowPtr[x + 1] = buf[x + 3];
							rowPtr[x + 2] = buf[x + 2];
							rowPtr[x + 3] = buf[x + 1];*/
						}
						break;
					default:
						break;
				}
			}
			
			if (isUpsideDown) {
				rowPtr -= pitch;
			} else {
				rowPtr += pitch;
			}
		}
	}

	*widthA = width;
	*heightA = height;
	*componentsA = bytesPerPixel;
#ifdef DEBUG
	std::cout << "  :: tga comment " << *commentA << std::endl;
	
	std::cout << "  :: tga width " << *widthA
	<< "   height " << *heightA
	<< "   components " << *componentsA
	<< std::endl;
#endif
	return (unsigned char*)data;
}


void
writeTGAHeader(UImageIO::OStream & streamA, const TGAHeader & header) {
	unsigned char temp[18];

	// Start by reading the fields associated with the original TGA format.
	temp[0] = header.idLength;

	temp[1] = header.mapType;

	temp[2] = header.imageType;

	// FIXME
	// short values assume little temp[0] = endian
	temp[3] = (header.mapStart & 0xFF);
	temp[4] = ((header.mapStart >> 8) & 0xFF); // little endian

	temp[5] = (header.mapLength & 0xFF);
	temp[6] = ((header.mapLength >> 8) & 0xFF);

	temp[7] = header.mapDepth;

	temp[8] = (header.xOrigin & 0xFF);
	temp[9] = ((header.xOrigin >> 8) & 0xFF);

	temp[10] = (header.yOrigin & 0xFF);
	temp[11] = ((header.yOrigin >> 8) & 0xFF);

	temp[12] = (header.imageWidth & 0xFF);
	temp[13] = ((header.imageWidth >> 8) & 0xFF);

	temp[14] = (header.imageHeight & 0xFF);
	temp[15] = ((header.imageHeight >> 8) & 0xFF);

	temp[16] = header.pixelDepth;

	temp[17] = header.imageDesc;

	// write to the stream
	streamA.write((char*)temp, 18);
}

bool
tgaSaver(UImageIO * imageIOA, UImageIO::OStream & streamA) {
	register int bpp = imageIOA->getImageComponents();
	register UDimension size = imageIOA->getSize();
	// Fill out the TGA header.
	TGAHeader header;
	header.idLength = 0;
	header.mapType = 0;
	// saving only uncompressed
	header.imageType = 2;
	header.mapStart = 0;
	header.mapLength = 0;
	header.mapDepth = 0;
	header.xOrigin = 0;
	header.yOrigin = 0;
	header.imageWidth = size.w;
	header.imageHeight = size.h;
	header.pixelDepth = bpp * 8;
	header.imageDesc = 0;
	if (bpp == 4) {
		// add alpha channel
		header.imageDesc |= 1 << 3;
	}
	char idString[256] = "";

	// the first data to be written is the standard header based on the
	// original TGA specification.
	writeTGAHeader(streamA, header);
	if (!streamA) {
		std::cerr << "stream invalid\n";
		return false;
	}

	if (header.idLength) {
		streamA.write(idString, header.idLength);
		if (!streamA) {
			return false;
		}
	}

	// swap the red and blue for saving out to the file.
	// also strip off any padding that may be in the data (TGA's don't have padding)
	unsigned char * pixels = imageIOA->getPixels();
	unsigned char * data;

	// y-flip
	data = new unsigned char[imageIOA->getWidth() * imageIOA->getHeight() * imageIOA->getImageComponents()];
	for (int y = 0; y < imageIOA->getHeight(); ++y)
	{
		int row =  imageIOA->getHeight() *  imageIOA->getImageComponents();
		unsigned char * src = pixels + y * row;
		unsigned char * dest = data + (imageIOA->getHeight() - 1 - y) * row;
		for (int x = 0; x < imageIOA->getWidth() * imageIOA->getImageComponents(); ++x) {
			dest[x + 2] = src[x + 2];
			dest[x + 1] = src[x + 1];
			dest[x + 0] = src[x + 0];
			if (imageIOA->getImageComponents() == 4) {
				dest[x + 3] = src[x + 3];
			}
		}
	}
	
	//: Write Truecolor UNcompressed
	if (header.imageType == 2)
	{
		streamA.write((char*)data, imageIOA->getWidth() * imageIOA->getHeight() * imageIOA->getImageComponents());
		// save it to the file uncompressed
		//int size = ::fwrite( tempImage.data(), sizeof(unsigned char), tempImage.size(), fp );
		//Alert( size == tempImage.size(), "Couldn't write the entire image" );
	}

	delete[](data);
	

/*
	//: Write Truecolor Compressed
	if (header.imageType == 10)
	{
		//: allocate plenty of data for one encoded scanline
		int ebs = tempImage.width() * tempImage.channels() * 2;
		unsigned char* encodedBuffer = new unsigned char[ebs];
		int rleCount = 0;

		for ( int i = 0; i < image.height(); ++i )
		{
			rleCount = TgaExporter::RLEncodeRow( tempImage.row( i ), tempImage.width(), tempImage.channels(), encodedBuffer );
			if ( ::fwrite( encodedBuffer, sizeof(unsigned char), rleCount, fp ) != rleCount )
			{
				Alert( false, "Couldn't write all image data" );
				//return false;
			}
		}
		delete []encodedBuffer;
	}
*/
	return true;
}
