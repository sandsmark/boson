/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/text/udocumentfactory.cpp
    begin             : Sat Dec 8 2001
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

#include "ufo/text/udocumentfactory.hpp"

#include <cctype>

#include "ufo/text/ubasicdocument.hpp"


using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UDocumentFactory, UObject)


UDocument *
UDocumentFactory::createMimeDocument(const std::string & typeA) {
	if (typeA == "text/plain") {
		return new UBasicDocument();
	}
	return NULL;
}

UDocument *
UDocumentFactory::createPlainDocument() {
	UDocument * doc = new UBasicDocument();
	return doc;
}

UDocument *
UDocumentFactory::createDigitDocument() {
	UDocument * doc = new UBasicDocument();
	return doc;
}


UDocument *
UDocumentFactory::createSpecialCharDocument(const std::string & allowedCharatersA) {
	UDocument * doc = new UBasicDocument();
	return doc;
}

/*
UDocumentFilter *
UDocumentFactory::createNewLineFilter() {
	//static UDocumentFilter * newlineFilter = new NewLineFilter();
	return NULL;//newlineFilter;
}

UDocumentFilter *
UDocumentFactory::createDigitFilter() {
	//static UDocumentFilter * digitFilter = new DigitFilter();
	return NULL;//digitFilter;
}
*/
/*
template <class NumT>
class UNumberDocument : public UBasicDocument {
};


template <class NumT>
UDocument *
UDocumentFactory::createNumberDocument() {
	return new UNumberDocument<NumT>;
}

*/
