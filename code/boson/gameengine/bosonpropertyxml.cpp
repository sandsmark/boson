/*
    This file is part of the Boson game
    Copyright (C) 2003 Andreas Beckermann (b_mann@gmx.de)

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

#include "bosonpropertyxml.h"
#include "bosonpropertyxml.moc"

#include "../bomemory/bodummymemory.h"
#include "bodebug.h"
#include "../bomath.h"
#include "../bo3dtools.h"

#include <kgame/kgameproperty.h>
#include <kgame/kgamepropertylist.h>
#include <kgame/kgamepropertyarray.h>
#include <kgame/kgamepropertyhandler.h>

#include <qdom.h>
#include <qintdict.h>
#include <qpointarray.h>

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
 } else if (*t == typeid(bofixed)) {
	value = QString::number(((KGameProperty<bofixed>*)prop)->value());
 } else if (*t == typeid(QString)) {
	value = ((KGameProperty<QString>*)prop)->value();
 } else if (*t == typeid(Q_INT8)) {
	value = QString::number(((KGameProperty<Q_INT8>*)prop)->value());
 } else if (*t == typeid(Q_UINT8)) {
	value = QString::number(((KGameProperty<Q_UINT8>*)prop)->value());
 }
 if (value.isNull()) {
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
	((KGameProperty<float>*)prop)->setValue(value.toFloat(&ok));
 } else if (*t == typeid(bofixed)) {
	((KGameProperty<bofixed>*)prop)->setValue(value.toFloat(&ok));
 } else if (*t == typeid(QString)) {
	((KGameProperty<QString>*)prop)->setValue(value);
 } else if (*t == typeid(Q_INT8)) {
	Q_INT8 v = 0;
	if (value == QString::fromLatin1("True")) {
		v = 1;
	} else if (value == QString::fromLatin1("False")) {
		v = 0;
	} else {
		short s = value.toShort(&ok);
		if (s < -128 || s > 127) {
			ok = false;
		}
		if (!ok) {
			s = 0;
		}
		v = (Q_INT8)s;
	}
	((KGameProperty<Q_INT8>*)prop)->setValue(v);
 } else if (*t == typeid(Q_UINT8)) {
	Q_UINT8 v = 0;
	unsigned short s = value.toUShort(&ok);
	if (s > 255) {
		ok = false;
	}
	if (!ok) {
		s = 0;
	}
	v = (Q_UINT8)s;
	((KGameProperty<Q_UINT8>*)prop)->setValue(v);
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
 QIntDict<KGamePropertyBase>& dict = dataHandler->dict();
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
 return true;
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


QTextStream& operator<<(QTextStream& s, const QPoint& p)
{
 // inefficient but easy to code and read.
 // just stream x coordinate, followed by y coordinate, separated by
 // space.
 s << QString::number(p.x());
 s << char(' ');
 s << QString::number(p.y());
 return s;
}

QTextStream& operator>>(QTextStream& s, QPoint& p)
{
 int x, y;
 QChar c;
 s >> x;
 s >> c; // single space
 s >> y;
 p.setX(x);
 p.setY(y);
 return s;
}

QTextStream& operator<<(QTextStream& s, const BoVector2Fixed& v)
{
 // inefficient but easy to code and read.
 // just stream x coordinate, followed by y coordinate, separated by
 // space.
 s << (float)v.x();
 s << char(' ');
 s << (float)v.y();
 return s;
}

QTextStream& operator>>(QTextStream& s, BoVector2Fixed& v)
{
 float x, y;
 QChar c;
 s >> x;
 s >> c; // single space
 s >> y;
 v.setX(x);
 v.setY(y);
 return s;
}

BosonCustomPropertyXML::BosonCustomPropertyXML(QObject* parent) : BosonPropertyXML(parent)
{
 connect(this, SIGNAL(signalRequestValue(KGamePropertyBase*, QString&)),
		this, SLOT(slotRequestValue(KGamePropertyBase*, QString&)));
 connect(this, SIGNAL(signalRequestSetValue(KGamePropertyBase*, const QString&)),
		this, SLOT(slotRequestSetValue(KGamePropertyBase*, const QString&)));
}

void BosonCustomPropertyXML::slotRequestValue(KGamePropertyBase* prop, QString& value)
{
 BO_CHECK_NULL_RET(prop);
 const type_info* t = prop->typeinfo();
 if (*t == typeid(QPoint)) {
	KGameProperty<QPoint>* p = (KGameProperty<QPoint>*)prop;
	QTextStream s(&value, IO_WriteOnly);
	s << p->value();
 } else if (*t == typeid(BoVector2Fixed)) {
	KGameProperty<BoVector2Fixed>* p = (KGameProperty<BoVector2Fixed>*)prop;
	QString x = QString::number(p->value().x());
	QString y = QString::number(p->value().y());
	value = QString("%1,%2").arg(x).arg(y);
 } else if (*t == typeid(BoVector3Fixed)) {
	KGameProperty<BoVector3Fixed>* p = (KGameProperty<BoVector3Fixed>*)prop;
	QString x = QString::number(p->value().x());
	QString y = QString::number(p->value().y());
	QString z = QString::number(p->value().z());
	value = QString("%1,%2,%3").arg(x).arg(y).arg(z);
 } else if (*t == typeid(BoVector4Fixed)) {
	KGameProperty<BoVector4Fixed>* p = (KGameProperty<BoVector4Fixed>*)prop;
	QString x = QString::number(p->value().x());
	QString y = QString::number(p->value().y());
	QString z = QString::number(p->value().z());
	QString w = QString::number(p->value().w());
	value = QString("%1,%2,%3,%4").arg(x).arg(y).arg(z).arg(w);
 } else if (*t == typeid(BoVector2Float)) {
	KGameProperty<BoVector2Float>* p = (KGameProperty<BoVector2Float>*)prop;
	QString x = QString::number(p->value().x());
	QString y = QString::number(p->value().y());
	value = QString("%1,%2").arg(x).arg(y);
 } else if (*t == typeid(BoVector3Float)) {
	KGameProperty<BoVector3Float>* p = (KGameProperty<BoVector3Float>*)prop;
	QString x = QString::number(p->value().x());
	QString y = QString::number(p->value().y());
	QString z = QString::number(p->value().z());
	value = QString("%1,%2,%3").arg(x).arg(y).arg(z);
 } else if (*t == typeid(BoVector4Float)) {
	KGameProperty<BoVector4Float>* p = (KGameProperty<BoVector4Float>*)prop;
	QString x = QString::number(p->value().x());
	QString y = QString::number(p->value().y());
	QString z = QString::number(p->value().z());
	QString w = QString::number(p->value().w());
	value = QString("%1,%2,%3,%4").arg(x).arg(y).arg(z).arg(w);
 } else if (*t == typeid(KGamePropertyBase*)) {
	// prop->typeinfo() returns info on a KGamePropertyBase pointer when
	// typeinfo() wasn't reimplemented. This is the case for all
	// non-KGameProperty<foobar> objects.
	// we need to do our own type checking here.
	if (typeid(*prop) == typeid(KGamePropertyList<QPoint>)) {
		KGamePropertyList<QPoint>* list = (KGamePropertyList<QPoint>*)prop;
		// inefficient but easy to code and read.
		// first we stream the length of the array and then every point.
		// each separated by a space.
		QTextStream s(&value, IO_WriteOnly);
		s << QString::number(list->count());
		KGamePropertyList<QPoint>::Iterator it;
		for (it = list->begin(); it != list->end(); ++it) {
			s << ' ';
			s << *it;
		}
	} else if (typeid(*prop) == typeid(KGamePropertyList<BoVector2Fixed>)) {
		KGamePropertyList<BoVector2Fixed>* list = (KGamePropertyList<BoVector2Fixed>*)prop;
		// inefficient but easy to code and read.
		// first we stream the length of the array and then every vector.
		// each separated by a space.
		QTextStream s(&value, IO_WriteOnly);
		s << QString::number(list->count());
		KGamePropertyList<BoVector2Fixed>::Iterator it;
		for (it = list->begin(); it != list->end(); ++it) {
			s << ' ';
			s << *it;
		}
	} else if (typeid(*prop) == typeid(KGamePropertyList<Q_INT32>)) {
		KGamePropertyList<Q_INT32>* list = (KGamePropertyList<Q_INT32>*)prop;
		QTextStream s(&value, IO_WriteOnly);
		s << QString::number(list->count());
		KGamePropertyList<Q_INT32>::Iterator it;
		for (it = list->begin(); it != list->end(); ++it) {
			s << ' ';
			s << *it;
		}
	} else if (typeid(*prop) == typeid(KGamePropertyList<Q_UINT32>)) {
		KGamePropertyList<Q_UINT32>* list = (KGamePropertyList<Q_UINT32>*)prop;
		QTextStream s(&value, IO_WriteOnly);
		s << QString::number(list->count());
		KGamePropertyList<Q_UINT32>::Iterator it;
		for (it = list->begin(); it != list->end(); ++it) {
			s << ' ';
			s << *it;
		}
	} else if (typeid(*prop) == typeid(KGamePropertyArray<Q_INT32>)) {
		KGamePropertyArray<Q_INT32>* array = (KGamePropertyArray<Q_INT32>*)prop;
		QTextStream s(&value, IO_WriteOnly);
		for (unsigned int i = 0; i < array->size(); i++) {
			s << (*array)[i];
			s << ' ';
		}
	} else if (typeid(*prop) == typeid(KGamePropertyArray<Q_UINT32>)) {
		KGamePropertyArray<Q_UINT32>* array = (KGamePropertyArray<Q_UINT32>*)prop;
		QTextStream s(&value, IO_WriteOnly);
		for (unsigned int i = 0; i < array->size(); i++) {
			s << (*array)[i];
			s << ' ';
		}
	}
 }

}

void BosonCustomPropertyXML::slotRequestSetValue(KGamePropertyBase* prop, const QString& value)
{
 BO_CHECK_NULL_RET(prop);
 const type_info* t = prop->typeinfo();
 if (*t == typeid(QPoint)) {
	QTextStream s((QString*)&value, IO_WriteOnly);
	KGameProperty<QPoint>* p = (KGameProperty<QPoint>*)prop;
	QPoint point;
	s >> point;
	p->setValue(point);
 } else if (*t == typeid(BoVector2Fixed)) {
	QStringList l = QStringList::split(',', value);
	if (l.count() != 2) {
		boError() << k_funcinfo << "invalid number of elements in vector. expected 2, found " << l.count() << endl;
		return;
	}
	bool ok0;
	bool ok1;
	bofixed x = bofixed(l[0].toFloat(&ok0));
	bofixed y = bofixed(l[1].toFloat(&ok1));
	if (!ok0 || !ok1) {
		boError() << k_funcinfo << "invalid number for vector element" << endl;
		return;
	}
	KGameProperty<BoVector2Fixed>* p = (KGameProperty<BoVector2Fixed>*)prop;
	p->setValue(BoVector2Fixed(x, y));
 } else if (*t == typeid(BoVector3Fixed)) {
	QStringList l = QStringList::split(',', value);
	if (l.count() != 3) {
		boError() << k_funcinfo << "invalid number of elements in vector. expected 3, found " << l.count() << endl;
		return;
	}
	bool ok0;
	bool ok1;
	bool ok2;
	bofixed x = bofixed(l[0].toFloat(&ok0));
	bofixed y = bofixed(l[1].toFloat(&ok1));
	bofixed z = bofixed(l[2].toFloat(&ok2));
	if (!ok0 || !ok1 || !ok2) {
		boError() << k_funcinfo << "invalid number for vector element" << endl;
		return;
	}
	KGameProperty<BoVector3Fixed>* p = (KGameProperty<BoVector3Fixed>*)prop;
	p->setValue(BoVector3Fixed(x, y, z));
 } else if (*t == typeid(BoVector4Fixed)) {
	QStringList l = QStringList::split(',', value);
	if (l.count() != 4) {
		boError() << k_funcinfo << "invalid number of elements in vector. expected 4, found " << l.count() << endl;
		return;
	}
	bool ok0;
	bool ok1;
	bool ok2;
	bool ok3;
	bofixed x = bofixed(l[0].toFloat(&ok0));
	bofixed y = bofixed(l[1].toFloat(&ok1));
	bofixed z = bofixed(l[2].toFloat(&ok2));
	bofixed w = bofixed(l[3].toFloat(&ok3));
	if (!ok0 || !ok1 || !ok2 || !ok3) {
		boError() << k_funcinfo << "invalid number for vector element" << endl;
		return;
	}
	KGameProperty<BoVector4Fixed>* p = (KGameProperty<BoVector4Fixed>*)prop;
	p->setValue(BoVector4Fixed(x, y, z, w));
 } else if (*t == typeid(BoVector2Float)) {
	QStringList l = QStringList::split(',', value);
	if (l.count() != 2) {
		boError() << k_funcinfo << "invalid number of elements in vector. expected 2, found " << l.count() << endl;
		return;
	}
	bool ok0;
	bool ok1;
	float x = l[0].toFloat(&ok0);
	float y = l[1].toFloat(&ok1);
	if (!ok0 || !ok1) {
		boError() << k_funcinfo << "invalid number for vector element" << endl;
		return;
	}
	KGameProperty<BoVector2Float>* p = (KGameProperty<BoVector2Float>*)prop;
	p->setValue(BoVector2Float(x, y));
 } else if (*t == typeid(BoVector3Float)) {
	QStringList l = QStringList::split(',', value);
	if (l.count() != 3) {
		boError() << k_funcinfo << "invalid number of elements in vector. expected 3, found " << l.count() << endl;
		return;
	}
	bool ok0;
	bool ok1;
	bool ok2;
	float x = l[0].toFloat(&ok0);
	float y = l[1].toFloat(&ok1);
	float z = l[2].toFloat(&ok2);
	if (!ok0 || !ok1 || !ok2) {
		boError() << k_funcinfo << "invalid number for vector element" << endl;
		return;
	}
	KGameProperty<BoVector3Float>* p = (KGameProperty<BoVector3Float>*)prop;
	p->setValue(BoVector3Float(x, y, z));
 } else if (*t == typeid(BoVector4Float)) {
	QStringList l = QStringList::split(',', value);
	if (l.count() != 4) {
		boError() << k_funcinfo << "invalid number of elements in vector. expected 4, found " << l.count() << endl;
		return;
	}
	bool ok0;
	bool ok1;
	bool ok2;
	bool ok3;
	float x = l[0].toFloat(&ok0);
	float y = l[1].toFloat(&ok1);
	float z = l[2].toFloat(&ok2);
	float w = l[3].toFloat(&ok3);
	if (!ok0 || !ok1 || !ok2 || !ok3) {
		boError() << k_funcinfo << "invalid number for vector element" << endl;
		return;
	}
	KGameProperty<BoVector4Float>* p = (KGameProperty<BoVector4Float>*)prop;
	p->setValue(BoVector4Float(x, y, z, w));
 } else if (*t == typeid(KGamePropertyBase*)) {
	if (typeid(*prop) == typeid(KGamePropertyList<QPoint>)) {
		KGamePropertyList<QPoint>* list = (KGamePropertyList<QPoint>*)prop;
		list->clear();
		unsigned int count = 0;
		QChar c;
		QPoint point;
		QTextStream s((QString*)&value, IO_ReadOnly);
		s >> count;
		for (unsigned int i = 0; i < count; i++) {
			s >> c;
			s >> point;
			list->append(point);
		}
	} else if (typeid(*prop) == typeid(KGamePropertyList<BoVector2Fixed>)) {
		KGamePropertyList<BoVector2Fixed>* list = (KGamePropertyList<BoVector2Fixed>*)prop;
		list->clear();
		unsigned int count = 0;
		QChar c;
		BoVector2Fixed point;
		QTextStream s((QString*)&value, IO_ReadOnly);
		s >> count;
		for (unsigned int i = 0; i < count; i++) {
			s >> c;
			s >> point;
			list->append(point);
		}
	} else if (typeid(*prop) == typeid(KGamePropertyList<Q_UINT32>)) {
		KGamePropertyList<Q_UINT32>* list = (KGamePropertyList<Q_UINT32>*)prop;
		list->clear();
		unsigned int count = 0;
		QChar c;
		Q_UINT32 v;
		QTextStream s((QString*)&value, IO_ReadOnly);
		s >> count;
		for (unsigned int i = 0; i < count; i++) {
			s >> c; // space
			s >> v;
			list->append(v);
		}
	} else if (typeid(*prop) == typeid(KGamePropertyList<Q_INT32>)) {
		KGamePropertyList<Q_INT32>* list = (KGamePropertyList<Q_INT32>*)prop;
		list->clear();
		unsigned int count = 0;
		QChar c;
		Q_INT32 v;
		QTextStream s((QString*)&value, IO_ReadOnly);
		s >> count;
		for (unsigned int i = 0; i < count; i++) {
			s >> c; // space
			s >> v;
			list->append(v);
		}
	} else if (typeid(*prop) == typeid(KGamePropertyArray<Q_INT32>)) {
		KGamePropertyArray<Q_INT32>* array = (KGamePropertyArray<Q_INT32>*)prop;
		QChar c;
		Q_INT32 v;
		QTextStream s((QString*)&value, IO_ReadOnly);
		for (unsigned int i = 0; i < array->size(); i++) {
			s >> v;
			s >> c; // space
			(*array)[i] = v;
		}
	} else if (typeid(*prop) == typeid(KGamePropertyArray<Q_UINT32>)) {
		KGamePropertyArray<Q_UINT32>* array = (KGamePropertyArray<Q_UINT32>*)prop;
		QChar c;
		Q_UINT32 v;
		QTextStream s((QString*)&value, IO_ReadOnly);
		for (unsigned int i = 0; i < array->size(); i++) {
			s >> v;
			s >> c; // space
			(*array)[i] = v;
		}
	}
 }
}

QString BosonPropertyXML::propertyValue(const QDomElement& root, unsigned long int id)
{
 QDomNodeList list = root.elementsByTagName(QString::fromLatin1("KGameProperty"));
 for (unsigned int i = 0; i < list.count(); i++) {
	QDomElement element = list.item(i).toElement();
	if (!element.isNull()) {
		boError() << k_funcinfo << "item " << i << " is not an element" << endl;
		continue;
	}
	if (element.attribute(QString::fromLatin1("Id")).compare(QString::number(id)) == 0) {
		return element.text();
	}
 }
 return QString::null;
}

bool BosonPropertyXML::removeProperty(QDomElement& root, unsigned long int id)
{
 bool ret = false;
 QDomNodeList list = root.elementsByTagName(QString::fromLatin1("KGameProperty"));
 for (unsigned int i = 0; i < list.count(); i++) {
	QDomElement element = list.item(i).toElement();
	if (element.isNull()) {
		boError() << k_funcinfo << "item " << i << " is not an element" << endl;
		continue;
	}
	if (element.attribute(QString::fromLatin1("Id")).compare(QString::number(id)) == 0) {
		root.removeChild(element);
		i--; // the element will be removed from the list :(
		ret = true;
	}
 }
 return ret;
}

