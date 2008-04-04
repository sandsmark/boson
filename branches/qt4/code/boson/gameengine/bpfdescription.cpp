/*
    This file is part of the Boson game
    Copyright (C) 2002 Andreas Beckermann (b_mann@gmx.de)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "bpfdescription.h"

#include "../bomemory/bodummymemory.h"
#include "bodebug.h"

#include <qdom.h>
#include <qdatastream.h>
#include <qfile.h>
#include <qfileinfo.h>

BPFDescription::BPFDescription(const QString& data)
{
 mFile.setContent(data);
}

BPFDescription::BPFDescription()
{
 mFile = QDomDocument("BosonMapDescription");
 QDomElement root = mFile.createElement("BosonMapDescription");
 mFile.appendChild(root);
 setTextForElement("Language", "C");
 setTextForElement("Name", "");
 setTextForElement("Comment", "");
}

BPFDescription::BPFDescription(const BPFDescription& data)
{
 *this = data;
}

BPFDescription::~BPFDescription()
{
}

BPFDescription& BPFDescription::operator=(const BPFDescription& data)
{
 mFile.setContent(data.toString());
 return *this;
}

QDomElement BPFDescription::element(const QString& name) const
{
 QDomNodeList list = topElement().elementsByTagName(name);
 if (list.count() == 0) {
	boError() << k_funcinfo << "no element for " << name << endl;
	return QDomElement();
 }
 if (list.count() > 1) {
	boWarning() << k_funcinfo << "more than 1 element for " << name << endl;
 }
 if (!list.item(0).isElement()) {
	boError() << k_funcinfo << name << " is not a QDomElement" << endl;
	return QDomElement();
 }
 return list.item(0).toElement();
}


void BPFDescription::setTextForElement(const QString& tagName, const QString& text)
{
 if (topElement().isNull()) {
	boError() << k_funcinfo << "oops" << endl;
	return;
 }
 if (topElement().elementsByTagName(tagName).count() == 0) {
	QDomElement e = mFile.createElement(tagName);
	topElement().appendChild(e);
 }
 QDomElement e = element(tagName);
 QDomText t = mFile.createTextNode(text);
 while (e.hasChildNodes()) {
	e.removeChild(e.firstChild());
 }
 e.appendChild(t);
}

QString BPFDescription::textOfElement(const QString& tagName) const
{
 QDomElement e = element(tagName);
 if (e.isNull()) {
	return QString::null;
 }
 QDomNodeList list = e.childNodes();
 if (list.count() == 0) {
	return QString::null;
 }
 if (list.count() > 1) {
	boWarning() << "more than one child nodes for " << tagName << endl;
 }
 if (!list.item(0).isText()) {
	boError() << k_funcinfo << "first child of " << tagName << " is not a text node" << endl;
	return QString::null;
 }
 return list.item(0).toText().nodeValue();
}




