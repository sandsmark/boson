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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#include "ufo/text/udocumentfactory.hpp"

#include <cctype>

#include "ufo/text/ubasicdocument.hpp"


using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UDocumentFactory, UObject)
UFO_IMPLEMENT_ABSTRACT_CLASS(UDocumentFilter, UObject)

class DigitFilter : public UDocumentFilter {
	UFO_DECLARE_DYNAMIC_CLASS(DigitFilter)
public:
	std::string filterText(const std::string & input) {
		char * filteredText = new char[input.length() + 1];
		
		int index = 0;
		for (std::string::const_iterator iter = input.begin();
				iter != input.end();
				iter++) {
			if (/*std::*/isdigit(*iter)) {
				filteredText[index++] = *iter;
			}
		}
		filteredText[index] = '\0';
		
		std::string ret = filteredText;
		delete[](filteredText);
		
		return ret;
	}
};
UFO_IMPLEMENT_DYNAMIC_CLASS(DigitFilter, UDocumentFilter)

class UCharacterFilter : public UDocumentFilter {
	UFO_DECLARE_ABSTRACT_CLASS(UCharacterFilter)
public:
	UCharacterFilter(const std::string & allowedA) : m_allowed(allowedA) {}
	
	std::string filterText(const std::string & input) {
		char * filteredText = new char[input.length() + 1];
		
		int index = 0;
		for (std::string::const_iterator iter = input.begin();
				iter != input.end();
				iter++) {
			if (m_allowed.find(*iter) != std::string::npos) {
				filteredText[index++] = *iter;
			}
		}
		filteredText[index] = '\0';
		
		std::string ret = filteredText;
		delete[](filteredText);
		
		return ret;
	}
private:
	std::string m_allowed;
};
UFO_IMPLEMENT_ABSTRACT_CLASS(UCharacterFilter, UDocumentFilter)

class NewLineFilter : public UDocumentFilter {
	UFO_DECLARE_DYNAMIC_CLASS(NewLineFilter)
public:
	std::string filterText(const std::string & input) {
		std::string filteredText = input;
		
		bool done = false;
		while (!done) {
			unsigned int index = filteredText.find('\n');
			if (index == std::string::npos) {
				done = true;
			} else {
				filteredText.erase(index, 1);
			}
		}
		return filteredText;
	}
};
UFO_IMPLEMENT_DYNAMIC_CLASS(NewLineFilter, UDocumentFilter)



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
	doc->setDocumentFilter(createNewLineFilter());
	return doc;
}

UDocument *
UDocumentFactory::createDigitDocument() {
	UDocument * doc = new UBasicDocument();
	doc->setDocumentFilter(createDigitFilter());
	return doc;
}


UDocument *
UDocumentFactory::createSpecialCharDocument(const std::string & allowedCharatersA) {
	UDocument * doc = new UBasicDocument();
	doc->setDocumentFilter(new UCharacterFilter(allowedCharatersA));
	return doc;
}


UDocumentFilter *
UDocumentFactory::createNewLineFilter() {
	static UDocumentFilter * newlineFilter = new NewLineFilter();
	return newlineFilter;
}

UDocumentFilter *
UDocumentFactory::createDigitFilter() {
	static UDocumentFilter * digitFilter = new DigitFilter();
	return digitFilter;
}

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
