/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "bosonpropertyxml.h"
#include "bosonpropertyxml.moc"

#include "bodebug.h"

#include <kgame/kgameproperty.h>
#include <kgame/kgamepropertyhandler.h>

#include <qdom.h>
#include <qintdict.h>

BosonPropertyXML::BosonPropertyXML(QObject* parent) : QObject(parent)
{
}

BosonPropertyXML::~BosonPropertyXML()
{
}

QString BosonPropertyXML::propertyValue(KGamePropertyBase* prop)
{
 QString value;
 const type_info* t = prop->typeinfo();
 if (*t == typeid(int)) {
	value = QString::number(((KGameProperty<int>*)prop)->value());
 } else if (*t == typeid(unsigned int)) {
	value = QString::number(((KGameProperty<unsigned int>*)prop)->value());
 } else if (*t == typeid(long int)) {
	value = QString::number(((KGameProperty<long int>*)prop)->value());
 } else if (*t == typeid(unsigned long int)) {
	value = QString::number(((KGameProperty<unsigned long int>*)prop)->value());
 } else if (*t == typeid(float)) {
	value = QString::number(((KGameProperty<float>*)prop)->value());
 } else if (*t == typeid(QString)) {
	value = ((KGameProperty<QString>*)prop)->value();
 } else if (*t == typeid(Q_INT8)) {
	// AB: do not use i18n() here!
	value = ((KGameProperty<Q_INT8>*)prop)->value() ? QString::fromLatin1("True") : QString::fromLatin1("False");
 } else {
	emit signalRequestValue(prop, value);
 }
 if (value.isNull()) {
	boWarning() << k_funcinfo << "Unknown value for " << prop->id() << endl;
 }
 return value;
}

void BosonPropertyXML::propertySetValue(KGamePropertyBase* prop, const QString& value)
{
 if (value.isNull()) {
	boError() << k_funcinfo << "null value!" << endl;
	return;
 }
 const type_info* t = prop->typeinfo();

 // should be set to false if a converting error occured. you should set the
 // value anyway! e.g. for "1a" integer we might be able to get "1", even though
 // it is an invalid string.
 bool ok = true;

 if (*t == typeid(int)) {
	((KGameProperty<int>*)prop)->setValue(value.toInt(&ok));
 } else if (*t == typeid(unsigned int)) {
	((KGameProperty<unsigned int>*)prop)->setValue(value.toUInt(&ok));
 } else if (*t == typeid(long int)) {
	((KGameProperty<long int>*)prop)->setValue(value.toLong(&ok));
 } else if (*t == typeid(unsigned long int)) {
	((KGameProperty<unsigned long int>*)prop)->setValue(value.toULong(&ok));
 } else if (*t == typeid(float)) {
	((KGameProperty<float>*)prop)->setValue(value.toULong(&ok));
 } else if (*t == typeid(QString)) {
	((KGameProperty<QString>*)prop)->setValue(value);
 } else if (*t == typeid(Q_INT8)) {
	bool v = false;
	if (value == QString::fromLatin1("True")) {
		v = true;
	} else if (value == QString::fromLatin1("False")) {
		v = false;
	} else {
		v = false;
		ok = false;
	}
	((KGameProperty<Q_INT8>*)prop)->setValue(v);
 } else {
	emit signalRequestSetValue(prop, value);
 }
 if (!ok) {
	boError() << k_funcinfo << "Could not convert string " << value << " for " << prop->id() << endl;
 }
}

bool BosonPropertyXML::saveAsXML(QDomElement& root, const KGamePropertyHandler* dataHandler)
{
 if (!dataHandler) {
	BO_NULL_ERROR(dataHandler);
	return QString::null;
 }
 QDomDocument doc = root.ownerDocument();
 QIntDict<KGamePropertyBase> dict = dataHandler->dict();
 QIntDictIterator<KGamePropertyBase> it(dict);
 for (; it.current(); ++it) {
	QString value = propertyValue(it.current());
	if (value.isNull()) {
		boWarning() << k_funcinfo << "invalid null value for " << it.current()->id() << endl;
		continue;
	}
	QDomElement element = doc.createElement(QString::fromLatin1("KGameProperty"));
	element.setAttribute(QString::fromLatin1("Id"), it.current()->id());
	element.appendChild(doc.createTextNode(value));
	root.appendChild(element);
 }
 return doc.toString();
}

bool BosonPropertyXML::loadFromXML(const QDomElement& root, KGamePropertyHandler* dataHandler)
{
 if (!dataHandler) {
	BO_NULL_ERROR(dataHandler);
	return false;
 }
 dataHandler->blockSignals(true);
 QDomNodeList list = root.elementsByTagName(QString::fromLatin1("KGameProperty"));
 for (unsigned int i = 0; i < list.count(); i++) {
	QDomElement element = list.item(i).toElement();
	if (!element.isElement()) {
		boError() << k_funcinfo << "item " << i << " is not an element" << endl;
		continue;
	}
	bool ok = false;
	int id = element.attribute(QString::fromLatin1("Id")).toInt(&ok);
	if (!ok) {
		boError() << k_funcinfo << "attribute Id is not a valid number: " << element.attribute(QString::fromLatin1("Id")) << endl;
		continue;
	}
	KGamePropertyBase* prop = dataHandler->find(id);
	if (!prop) {
		boError() << k_funcinfo << "Can't find property " << id << " in datahandler " << dataHandler->id() << endl;
		continue;
	}
	QString value = element.text();
	if (value.isEmpty()) {
		boError() << k_funcinfo << "empty value for property " << id << endl;
		continue;
	}
	propertySetValue(prop, value);
 }
 dataHandler->blockSignals(false);
 return true;
}

