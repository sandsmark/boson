/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/image/uimageio.hpp
    begin             : Fri Oct 5 2001
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

#ifndef UIMAGEIO_HPP
#define UIMAGEIO_HPP

#include "../uobject.hpp"

#include "../util/udimension.hpp"

#include <vector>
#include <map>

#include <fstream>

namespace ufo {


/** @short A basic image loader and saver.
  * @ingroup drawing
  *
  * It loads given images from files of streams.
  * The loaded pixels will be freed when the image is destroyed.
  * Currently, only pnm and tga files are supported.
  * But you can add you own image load and save functions by
  * registering them with <code>registerLoader</code> resp.
  * <code>registerSaver</code>
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UImageIO : public UObject {
	UFO_DECLARE_DYNAMIC_CLASS(UImageIO)
public: // Public types

	typedef std::istream IStream;
	typedef std::ostream OStream;

	typedef unsigned char * (*LoadFuncPointer)(
		UImageIO * imageIOA, // image io which should contain the image
		IStream & streamA, // the stream to load from
		std::string * commentA, // a comment
		int * widthA, // the width in pixels
		int * heightA, // the height in pixels
		int * componentsA // the amount of color components (1..4)
	);

	typedef bool (*SaveFuncPointer) (
		UImageIO * imageIO, // image io which contains the image
		OStream & streamA // the stream to save to
	);
	/** Filter rules. Used at loading time. */
	enum FilterRule {
		NoFilterRule = 0,
		AlphaLayer = 1, // add an alpha layer
		ColorTypeRGB = 2,
		ColorTypeRGBA = ColorTypeRGB | AlphaLayer,
		ColorTypeGray = 4,
		ColorTypeGrayAlpha = ColorTypeGray | AlphaLayer,
		FlipX = 8,
		FlipY = 16
	};

	/*
	enum ColorType {
		Alpha,
		Red,
		Green,
		Blue,
		Luminance,
		LuminanceAlpha,
		RGB,
		BGR,
		RGBA,
		BGRA
	};*/

public: // c'tors
	/** Creates an empty image io object.
	  */
	UImageIO();
	/** Creates an image io object and tries to load an image from the given
	  * file name.
	  * @param fileName File name which should be used to load an image.
	  */
	UImageIO(const std::string & fileNameA);
	/**
	  * @param streamA the stream which should be
	  *	used to get the encoded image.
	  * @param extensionA The file extension describing the file type
	  *	(e.g.: ".png" for PNG files)
	  */
	UImageIO(IStream & streamA, const std::string & extensionA);
	/** Creates a UImageIO object with already decoded image data.
	  */
	UImageIO(unsigned char * dataA, int widthA, int heightA, int componentsA);
	/** Creates an empty UImageIO object. The image data may be manipulated via
	  * getPixels().
	  */
	UImageIO(int widthA, int heightA, int componentsA);
	virtual ~UImageIO();


public: // general accessors
	/**returns the raw pixels with one of these layouts:
	  * <ul>
	  * <li>gray scaled images ( 1 byte )</li>
	  * <li>gray scaled iamges with an alpha layer  ( 2 bytes )</li>
	  * <li>RGB ( 3 bytes )</li>
	  * <li>RGBA ( 4 bytes )</li>
	  *</ul>
	  */
	unsigned char * getPixels();

	int getWidth();
	int getHeight();
	const UDimension & getSize();

	/** the comments is either a file comment or the file name itsself */
	std::string getComment();

	/** Returns the bytes used for the given image.
	  * <ul>
	  * <li>1 for gray scaled images.</li>
	  * <li>2 for gray scaled iamges with an alpha layer.</li>
	  * <li>3 for rgb images.</li>
	  * <li>4 for rgba images.</li>
	  *</ul>
	  */
	int getImageComponents();

public: // Image loading methods

	/** Loads the given image file
	  * @return true, if the file could be loaded and decoded.
	  */
	bool load(const std::string & fileNameA);

	/** Searches the file archive for a file with the given file name
	  * and uses an appropriate image plugin for loading.
	  * @return true, if the file could be loaded and decoded.
	  * @see UFileArchive
	  */
	bool loadFromArchive(const std::string & fileNameA);

	/**
	  * @param streamA the stream which should be
	  *	used to get the encoded image.
	  * @param extensionA The file extension describing the file type
	  *	(e.g.: ".png" for PNG files)
	  * @return true, if the file could be loaded and decoded.
	  */
	bool load(UImageIO::IStream & streamA, const std::string & extensionA);
	//bool load(std::basic_istream<unsigned char> & streamA,
	//	std::string extensionA);


public: // Image saving methods

	/** Saves to currently loaded byte array to the given image.
	  * The file name must have an extension returned by
	  * <code>getAvaiableSavingExtensions()</code>
	  *
	  * @return true, if the file could be loaded and decoded.
	  */
	bool save(const std::string & fileNameA);
	/**
	  * @param streamA the stream which should be
	  *	used to save the image.
	  * @param extensionA The file extension describing the file type
	  *	(e.g.: ".png" for PNG files)
	  * @return true, if the file could be saved and encoded.
	  */
	bool save(UImageIO::OStream & streamA, const std::string & extensionA);

public: // filter rules
	/** Sets the desired filter rules at loading time.
	  * The image loader tries to perform the desired filter operations.
	  * If succesful, the specified filter rule will be removed, i.e.
	  * if the image was flipped horizontally, FlipX will be removed from
	  * the filter rules.
	  *
	  * @param filterRule
	  * 	An int describing the new desired filter rules. Can be OR'ed.
	  * @see FilterRule
	  */
	//void setFilterRule(UImageIO::FilterRule filterRule);
	/** returns the current desired filter rule.
	  * @see setFilterRule
	  */
	//UImageIO::FilterRule getFilterRule();


	/** Tries to apply the changes to the given byte array.
	  * Till now, only adding an alpha layer is supported.
	  * The alpha layer will be the gray value of the colors.
	  */
	//void apply();

public: // Public static methods
	/** Returns the possible extensions (and file formats) which can
	  * be decoded by UImageIO
	  */
	static std::vector<std::string> getAvailableLoadingExtensions();
	/** Registers a new image loader function and returns the old registered
	  * image loader (or NULL).
	  * You can pass a NULL pointer as image saving
	  * functions to get the current image saving functions for the given
	  * extension.
	  *
	  *@param loaderA The new image loading function.
	  *@param extensionA The extension/format which can be loaded by the
	  * 	image loading function (e.g. .pnm for pnm images)
	  *@return The old image loading function pointer or NULL
	  */
	static UImageIO::LoadFuncPointer registerLoader(
			UImageIO::LoadFuncPointer loaderA,
			const std::string & extensionA);
	/** Unregisters an image loader. This method checks whether the current
	  * image load functions matches the given one (on the same extension)
	  * and removes it.
	  */
	static void unregisterLoader(UImageIO::LoadFuncPointer saverA,
			const std::string & extensionA);


	/** Returns the possible extensions (and file formats) which can
	  * be decoded by UImageIO
	  */
	static std::vector<std::string> getAvailableSavingExtensions();
	/** Registers a new image saving function and returns the old registered
	  * image saver (or NULL).
	  * You can pass a NULL pointer as image saving
	  * functions to get the current image saving functions for the given
	  * extension.
	  *
	  *@param loaderA The new image saving function.
	  *@param extensionA The extension/format which can be saved by the
	  * 	image loading function
	  *@return The old image saving function pointer or NULL
	  */
	static UImageIO::SaveFuncPointer registerSaver(UImageIO::SaveFuncPointer saverA,
			const std::string & extensionA);
	/** Unregisters an image saver. This method checks whether the current
	  * image save functions matches the given one (on the same extension)
	  * and removes it.
	  */
	static void unregisterSaver(UImageIO::SaveFuncPointer saverA,
			const std::string & extensionA);

	/** Initializes UImageIO. This means, registering png and bmp support.
	  * You must call this function just once.
	  */
	static void init();

protected:  // Overrides UObject
	virtual std::ostream & paramString(std::ostream & os) const;

public:  // Public attributes

	static const int ALPHA_LAYER;

	static const int COLOR_TYPE_RGB;
	/** rgba is equal COLOR_TYPE_RGB | ALPHA_LAYER
	  */
	static const int COLOR_TYPE_RGB_ALPHA;

	static const int COLOR_TYPE_GRAY;

	/** gray alpha is equal COLOR_TYPE_GRAY | ALPHA_LAYER
	  */
	static const int COLOR_TYPE_GRAY_ALPHA;

	static const int FLIP_X;
	static const int FLIP_Y;

private:  // Private attributes
	std::string m_comment;
	unsigned char * m_data;
	UDimension m_size;
	int m_components;
	//FilterRule m_filterRule;

	typedef std::map<std::string, LoadFuncPointer> LoadMap_t;
	typedef std::map<std::string, SaveFuncPointer> SaveMap_t;
	static LoadMap_t m_loadFuncs;
	static SaveMap_t m_saveFuncs;
};

} // namespace ufo

#endif // UIMAGEIO_HPP
