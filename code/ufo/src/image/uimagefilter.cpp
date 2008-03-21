/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/image/uimagefilter.cpp
    begin             : Thu Apr 1 2004
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA *
 ***************************************************************************/

#include "ufo/image/uimagefilter.hpp"

#include "ufo/image/uimageio.hpp"

using namespace ufo;

UFO_IMPLEMENT_ABSTRACT_CLASS(UImageFilter, UObject)

UImageIO * 
UImageFilter::stretch(UImageIO * oldImage, int newWidth, int newHeight) {
	oldImage->reference();

	UImageIO * ret = new UImageIO(newWidth, newHeight, oldImage->getImageComponents());

	scale(
		oldImage->getImageComponents(),
		oldImage->getWidth(), oldImage->getHeight(), oldImage->getPixels(),
		ret->getWidth(), ret->getHeight(), ret->getPixels()
	);

	oldImage->unreference();
	return ret;
}


UImageIO * 
UImageFilter::changeChannel(UImageIO * oldImage, int newChannels) {
	oldImage->reference();
	UImageIO * ret = NULL;
	if (newChannels == oldImage->getImageComponents()) {
		ret = new UImageIO(oldImage->getPixels(), oldImage->getWidth(),
			oldImage->getHeight(), newChannels);
		oldImage->unreference();
		return ret;
	}

	uint8_t * src = oldImage->getPixels();
	int oldChannels = oldImage->getImageComponents();
	int width = oldImage->getWidth();
	int height = oldImage->getHeight();

	ret = new UImageIO(width, height, newChannels);
	uint8_t * data = ret->getPixels();
	
	int byteCount = width * height * newChannels;
	int oldByteCount = width * height * oldChannels;
	bool alpha = (newChannels == 2 || newChannels == 4);
	bool oldAlpha = (oldChannels == 2 || oldChannels == 4);
	int i = 0;

	switch (newChannels) {
		case 1:
		case 2:
			switch (oldChannels) {
				case 1:
				case 2:
					for (i = 0; i < oldByteCount; i += oldChannels) {
						data[i * newChannels] = src[i];
					}
					break;
				case 3:
				case 4:
					for (i = 0; i < oldByteCount; i += oldChannels) {
						data[i * newChannels] = 
							uint8_t((float(src[i]) + src[i + 1] + src[i + 2]) / 3);
					}
					break;
				default: 
					break;
			}
			if (alpha && oldAlpha) {
				for (i = oldChannels; i < oldByteCount - oldChannels; 
						i += oldChannels) {
					data[i * (newChannels + 1) - 1] = src[i - 1];
				}
			}
			if (alpha && !oldAlpha) {
				for (i = 0; i < byteCount; i += 2) {
					// newChannels is 2
					data[i + 1] = 255;
				}
			}
			break;
		case 3:
		case 4:
			switch (oldChannels) {
				case 1:
				case 2:
					for (i = 0; i < oldByteCount; i += oldChannels) {
						data[i * newChannels] = src[i];
						data[i * newChannels + 1] = src[i];
						data[i * newChannels + 2] = src[i];
					}
					break;
				case 3:
				case 4:
					for (i = 0; i < oldByteCount; i += oldChannels) {
						data[i * newChannels] = src[i];
						data[i * newChannels + 1] = src[i + 1];
						data[i * newChannels + 2] = src[i + 2];
					}
					break;
				default: 
					break;
			}
			if (alpha && oldAlpha) {
				for (i = oldChannels; i < oldByteCount - oldChannels; 
						i += oldChannels) {
					data[i * (newChannels + 1) - 1] = src[i - 1];
				}
			}
			if (alpha && !oldAlpha) {
				for (i = 0; i < byteCount; i += 4) {
					// newChannels is 4
					data[i + 3] = 255;
				}
			}
			break;
		default:
			break;
	}
	oldImage->unreference();
	return ret;
}


UImageIO * 
UImageFilter::flipX(UImageIO * oldImagE) {
	return NULL;
}

UImageIO * 
UImageFilter::flipY(UImageIO * oldImagE) {
	return NULL;
}


/* The scaling code is ripped from the Mesa 3-D library.
 * Version:  3.4
 * Copyright (C) 1995-2000  Brian Paul
 * Released under the LGPL
 */
void 
UImageFilter::scale(int components, 
		int widthin, int heightin, const uint8_t * datain,
		int widthout, int heightout, uint8_t * dataout)
{
	float sx = (float) (widthin - 1) / (float) (widthout - 1);
	float sy = (float) (heightin - 1) / (float) (heightout - 1);

#ifdef POINT_EXAMPLE
	for (int i = 0; i < heightout; i++) {
		int ii = i * scale_y;
		for (int j = 0; j < widthout; j++) {
			int jj = j * scale_x;

			uint8_t *src = source + (ii * widthin + jj) * components;
			uint8_t *dst = data + (i * widthout + j) * components;

			for (int k = 0; k < components; k++) {
				*dst++ = *src++;
			}
		}
	}
#else
	const uint8_t * tempin = datain;
	uint8_t * tempout = dataout;
	
	int i, j, k;
	if (sx < 1.0 && sy < 1.0) {
		/* magnify both width and height:  use weighted sample of 4 pixels */
		int i0, i1, j0, j1;
		float alpha, beta;
		const uint8_t *src00, *src01, *src10, *src11;
		float s1, s2;
		uint8_t *dst;

		for (i = 0; i < heightout; i++) {
			i0 = int(i * sy);
			i1 = i0 + 1;
			if (i1 >= heightin)
				i1 = heightin - 1;
			/*	 i1 = (i+1) * sy - EPSILON;*/
			alpha = i * sy - i0;
			for (j = 0; j < widthout; j++) {
				j0 = int(j * sx);
				j1 = j0 + 1;
				if (j1 >= widthin)
					j1 = widthin - 1;
				/*	    j1 = (j+1) * sx - EPSILON; */
				beta = j * sx - j0;

				/* compute weighted average of pixels in rect (i0,j0)-(i1,j1) */
				src00 = tempin + (i0 * widthin + j0) * components;
				src01 = tempin + (i0 * widthin + j1) * components;
				src10 = tempin + (i1 * widthin + j0) * components;
				src11 = tempin + (i1 * widthin + j1) * components;

				dst = tempout + (i * widthout + j) * components;

				for (k = 0; k < components; k++) {
					s1 = *src00++ * (1.0 - beta) + *src01++ * beta;
					s2 = *src10++ * (1.0 - beta) + *src11++ * beta;
					*dst++ = int(s1 * (1.0 - alpha) + s2 * alpha);
				}
			}
		}
	}
	else {
		/* shrink width and/or height:  use an unweighted box filter */
		int i0, i1;
		int j0, j1;
		int ii, jj;
		float sum;
		uint8_t *dst;

		for (i = 0; i < heightout; i++) {
			i0 = int(i * sy);
			i1 = i0 + 1;
			if (i1 >= heightin)
				i1 = heightin - 1;
			/*	 i1 = (i+1) * sy - EPSILON; */
			for (j = 0; j < widthout; j++) {
				j0 = int(j * sx);
				j1 = j0 + 1;
				if (j1 >= widthin)
					j1 = widthin - 1;
		/*	    j1 = (j+1) * sx - EPSILON; */

				dst = tempout + (i * widthout + j) * components;

				/* compute average of pixels in the rectangle (i0,j0)-(i1,j1) */
				for (k = 0; k < components; k++) {
					sum = 0.0;
					for (ii = i0; ii <= i1; ii++) {
						for (jj = j0; jj <= j1; jj++) {
							sum += *(tempin + (ii * widthin + jj) * components + k);
						}
					}
					sum /= (j1 - j0 + 1) * (i1 - i0 + 1);
					*dst++ = int(sum);
				}
			}
		}
	}
#endif
}
