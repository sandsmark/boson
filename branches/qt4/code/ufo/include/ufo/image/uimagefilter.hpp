/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/image/uimagefilter.hpp
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

#ifndef UIMAGEFILTER_HPP
#define UIMAGEFILTER_HPP

#include "../uobject.hpp"

namespace ufo {

class UImageIO;
/** @short A class used for image manipulation.
  * @ingroup internal
  *
  * @author Johannes Schmidt
  */
class UImageFilter : public UObject {
	UFO_DECLARE_ABSTRACT_CLASS(UImageFilter)
public:
	/** Creates a new image io object with the given parameters. Stretches the
	  * old image if necessarry.
	  *
	  * @param oldImage The image data to be used as source
	  * @param newWidth The width of the created image io
	  * @param newHeight The height of the created height
	  */
	static UImageIO * stretch(UImageIO * oldImage, int newWidth, int newHeight);

	static UImageIO * changeChannel(UImageIO * oldImage, int newChannels);

	/** Creates a new image io with horizontally flipped content. */
	static UImageIO * flipX(UImageIO * oldImagE);
	/** Creates a new image io with vertically flipped content. */
	static UImageIO * flipY(UImageIO * oldImagE);

public:
	/** A scaling method for raw byte arrays.
	  * @param components The number of color channels (1-4)
	  * @param widthin The width (in pixels) of the source image
	  * @param heightin The height (in pixels) of the source image
	  * @param datain A pointer to the source image
	  * @param widthout The width (in pixels) of the destination image
	  * @param heightout The height (in pixels) of the destination image
	  * @param dataout A pointer to the destination image (shouldn't be NULL)
	  */
	static void scale(int components,
		int widthin, int heightin, const uint8_t * datain,
		int widthout, int heightout, uint8_t * dataout);
};

} // namespace ufo

#endif // UIMAGEFILTER_HPP
