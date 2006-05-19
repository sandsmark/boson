/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/image/uimageio.cpp
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

#include "ufo/image/uimageio.hpp"

#include <fstream>

#include "ufo/util/ufilearchive.hpp"

using namespace ufo;

// init static members

UImageIO::LoadMap_t UImageIO::m_loadFuncs;
UImageIO::SaveMap_t UImageIO::m_saveFuncs;

const int UImageIO::ALPHA_LAYER = 1;
const int UImageIO::COLOR_TYPE_RGB = 2;
/** rgba is equal COLOR_TYPE_RGB | ALPHA_LAYER
  */
const int UImageIO::COLOR_TYPE_RGB_ALPHA =
    UImageIO::COLOR_TYPE_RGB | UImageIO::ALPHA_LAYER;

const int UImageIO::COLOR_TYPE_GRAY = 4;

/** gray alpha is equal COLOR_TYPE_GRAY | ALPHA_LAYER
  */
const int UImageIO::COLOR_TYPE_GRAY_ALPHA =
    UImageIO::COLOR_TYPE_GRAY | UImageIO::ALPHA_LAYER;

const int UImageIO::FLIP_X = 8;
const int UImageIO::FLIP_Y = 16;

/** private functions to get the file extension */
std::string getFileExtension(const std::string & fileNameA) {
	unsigned int pos = fileNameA.rfind('.');

	if (pos < fileNameA.length()) {
		return fileNameA.substr(pos);
	} else {
		return "";
	}
}


UFO_IMPLEMENT_DYNAMIC_CLASS(UImageIO, UObject)

UImageIO::UImageIO()
	: m_comment()
	, m_data(NULL)
	, m_size()
	, m_components(0)
	//, m_filterRule(NoFilterRule)
{
}

UImageIO::UImageIO(const std::string & fileNameA)
		: m_comment()
		, m_data(NULL)
		, m_size()
		, m_components(0)
		//, m_filterRule(NoFilterRule)
{
	load(fileNameA);
}

UImageIO::UImageIO(UImageIO::IStream & streamA, const std::string & extensionA)
		: m_comment()
		, m_data(NULL)
		, m_size()
		, m_components(0)
		//, m_filterRule(NoFilterRule)
{
	load(streamA, extensionA);
}

UImageIO::UImageIO(unsigned char * dataA, int widthA, int heightA, int componentsA)
		: m_comment()
		, m_data(NULL)
		, m_size(widthA, heightA)
		, m_components(componentsA)
		//, m_filterRule(NoFilterRule)
{
	m_data = new unsigned char[widthA * heightA * componentsA];
	//
	//FIXME use memcpy or similar
	for (int i = 0; i < widthA * heightA * componentsA; ++i) {
		m_data[i] = dataA[i];
	}
	//memcpy(m_data, dataA, widthA * heightA * componentsA);
}


UImageIO::UImageIO(int widthA, int heightA, int componentsA)
		: m_comment()
		, m_data(NULL)
		, m_size(widthA, heightA)
		, m_components(componentsA)
		//, m_filterRule(NoFilterRule)
{
	m_data = new unsigned char[widthA * heightA * componentsA];
}

UImageIO::~UImageIO() {
	if (m_data) {
		delete[](m_data);
	}
}

//
// loading functions
//

bool
UImageIO::load(const std::string & fileNameA) {
	m_comment = fileNameA;

	std::ifstream streamL(fileNameA.c_str(), std::ios_base::binary);
	if (!streamL) {
		uError() << "UImageIO: file " << fileNameA << " does not exist" << "\n";
		return false;
	}

	bool valid = load(streamL, getFileExtension(fileNameA));
	if (!valid) {
		uError() << "UImageIO: Failed to load image " << fileNameA << "\n";
	}
	return valid;
}

bool
UImageIO::loadFromArchive(const std::string & fileNameA) {
	m_comment = fileNameA;

	std::string absolutPath = UFileArchive::getDefault()->getAbsolutePath(fileNameA);

	if (absolutPath.empty()) {
		// NULL pointers should happen
		uError() << "UImageIO: file " << fileNameA
		<< " does not exist in the default file archive" << "\n";
		return false;
	}

	std::ifstream fileStream(absolutPath.c_str(), std::ios_base::in | std::ios_base::binary);

	bool valid = load(fileStream, getFileExtension(fileNameA));

	if (!valid) {
		uError() << "UImageIO: Failed to load image " << absolutPath << "\n";
	}

	return valid;
}

bool
UImageIO::load(UImageIO::IStream & streamA, const std::string & extensionA) {
	LoadMap_t::const_iterator iter;

	iter = m_loadFuncs.find(extensionA);
	if (iter != m_loadFuncs.end() ) {
		LoadFuncPointer loader = (*iter).second;

		unsigned char * new_data = loader(this, streamA, &m_comment,
		               &m_size.w, &m_size.h, &m_components);

		if (new_data) {
			if (m_data) {
				// delete old image
				delete[](m_data);
			}
			m_data = new_data;
			return true;
		}
	} else {
		uError() << "UImageIO: Couldn't find a plugin to load image "
		<< m_comment
		<< "\nof type " << extensionA << "\n";
	}
	return false;
}

//
// saving functions
//

bool
UImageIO::save(const std::string & fileNameA) {
	std::ofstream streamL(fileNameA.c_str(), std::ios_base::binary);

	return save(streamL, getFileExtension(fileNameA));
}

bool
UImageIO::save(UImageIO::OStream & streamA, const std::string & extensionA) {
	SaveMap_t::const_iterator iter;
	iter = m_saveFuncs.find(extensionA);
	if (iter != m_saveFuncs.end()) {
		SaveFuncPointer saver = (*iter).second;
		return saver(this, streamA);
	} else {
		uError() << "UImageIO: Couldn´t find a plugin to save image "
		<< "of type " << extensionA << "\n";
	}
	return false;
}



unsigned char *
UImageIO::getPixels() {
	return m_data;
}


int
UImageIO::getWidth() {
	return m_size.w;
}
int
UImageIO::getHeight() {
	return m_size.h;
}
const UDimension &
UImageIO::getSize() {
	return m_size;
}

std::string
UImageIO::getComment() {
	return m_comment;
}


int
UImageIO::getImageComponents() {
	return m_components;
}

/*
void
UImageIO::setFilterRule(FilterRule filterRule) {
	m_filterRule = filterRule;
}

UImageIO::FilterRule
UImageIO::getFilterRule() {
	return m_filterRule;
}

void
UImageIO::apply() {
	if (!m_data) {
		// no pixel array which we can apply changes to
		return ;
	}
	if (m_desiredStates & AlphaLayer) {
#ifdef DEBUG
		std::cout << "add alpha channel to image " << m_comment << std::endl;
#endif

		if (m_components == 1) {
			unsigned char * newData =
			    new unsigned char[m_size.w * m_size.h * 2];

			for (int i = 0;i < m_size.w * m_size.h;i++) {
				newData[2 * i] = m_data[i];
				newData[2 * i + 1] = m_data[i];
			}
			delete[](m_data);
			m_data = newData;
			m_components = 2;
		}
		if (m_components == 3) {
			unsigned char * newData =
			    new unsigned char[m_size.w * m_size.h * 4];

			for (int i = 0;i < m_size.w * m_size.h * 3;i++) {
				newData[4 * i] = m_data[3 * i];
				newData[4 * i + 1] = m_data[3 * i + 1];
				newData[4 * i + 2] = m_data[3 * i + 2];
				newData[4 * i + 3] =
				    (m_data[3 * i] + m_data[3 * i + 1] + m_data[3 * i + 2]) / 3;
			}
			delete[](m_data);
			m_data = newData;
			m_components = 4;
		}
	}
}
*/
//
// Public static methods
//

//
// loading

std::vector<std::string>
UImageIO::getAvailableLoadingExtensions() {
	std::vector<std::string> retL;

	LoadMap_t::iterator iter = m_loadFuncs.begin();

	for (;iter != m_loadFuncs.end(); iter ++) {
		retL.push_back((*iter).first);
	}

	return retL;
}

UImageIO::LoadFuncPointer
UImageIO::registerLoader(UImageIO::LoadFuncPointer loaderA,
		const std::string & extensionA) {

	UImageIO::LoadFuncPointer retL = NULL;
	LoadMap_t::iterator iter = m_loadFuncs.find(extensionA);

	if (iter != m_loadFuncs.end()) {
		retL = (*iter).second;
	}

	if (loaderA) {
		m_loadFuncs[extensionA] = loaderA;
	}
	return retL;
}

void
UImageIO::unregisterLoader(UImageIO::LoadFuncPointer loaderA,
		const std::string & extensionA) {

	LoadMap_t::iterator iter = m_loadFuncs.find(extensionA);

	if (iter != m_loadFuncs.end()) {
		if (loaderA == (*iter).second) {
			m_loadFuncs.erase(iter);
		}
	}
}

//
// saving

std::vector<std::string>
UImageIO::getAvailableSavingExtensions() {
	std::vector<std::string> retL;

	SaveMap_t::iterator iter = m_saveFuncs.begin();

	for (;iter != m_saveFuncs.end(); iter ++) {
		retL.push_back((*iter).first);
	}

	return retL;
}

UImageIO::SaveFuncPointer
UImageIO::registerSaver(UImageIO::SaveFuncPointer saverA,
		const std::string & extensionA) {

	UImageIO::SaveFuncPointer retL = NULL;
	SaveMap_t::iterator iter = m_saveFuncs.find(extensionA);

	if (iter != m_saveFuncs.end()) {
		retL = (*iter).second;
	}

	if (saverA) {
		m_saveFuncs[extensionA] = saverA;
	}
	return retL;
}


void
UImageIO::unregisterSaver(UImageIO::SaveFuncPointer saverA,
		const std::string & extensionA) {

	SaveMap_t::iterator iter = m_saveFuncs.find(extensionA);

	if (iter != m_saveFuncs.end()) {
		if (saverA == (*iter).second) {
			m_saveFuncs.erase(iter);
		}
	}
}

#include "pnm.h"
#include "tga.h"
//#include "bmp.h"

void
UImageIO::init() {
	registerLoader(&pnmLoader, ".pnm");
	registerSaver(&pnmSaver, ".pnm");

	registerLoader(&tgaLoader, ".tga");
	registerSaver(&tgaSaver, ".tga");

	//registerLoader(&bmpLoader, ".bmp");
}


std::ostream &
UImageIO::paramString(std::ostream & os) const {
	os << "bpp " << m_components
	<< " " << m_size.w << "x" << m_size.h
	<< " pixel src " << std::hex << reinterpret_cast<long>(m_data)
	<< " comment " << m_comment;
	return os;
}
