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
#ifndef BOSONPROPERTYXML_H
#define BOSONPROPERTYXML_H

#include <qobject.h>
//Added by qt3to4:
#include <Q3TextStream>
#include <Q3PointArray>

class QString;
class QDomElement;
class QPoint;
class Q3TextStream;
class KGamePropertyBase;
class KGamePropertyHandler;
class bofixed;
template<class T> class BoVector2;
typedef BoVector2<bofixed> BoVector2Fixed;

/**
 * Helper class for @ref KGamePropertyHandler. You can use it to store a
 * property handler in an xml file completely (and of course load it again).
 *
 * All you need to do is to create an object of this class and call @ref
 * saveAsXML() as long as you use "simple" datatypes only in your @ref
 * KGameProperty objects. "simple" are all primitive datatypes (int, float,
 * bool, double, char, ...) as well as a @ref QString.
 *
 * If you use additional datatypes you need to write your own code to convert
 * them into a string a back from a string. Just connect that code to @ref
 * signalRequestValue and @ref signalRequestSetValue.
 * @short Store a @ref KGamePropertyHandler in a XML file
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonPropertyXML : public QObject
{
	Q_OBJECT
public:
	BosonPropertyXML(QObject* parent = 0);
	~BosonPropertyXML();

	QString propertyValue(KGamePropertyBase* prop);
	void propertySetValue(KGamePropertyBase* prop, const QString& value);

	bool saveAsXML(QDomElement& root, const KGamePropertyHandler* dataHandler);
	bool loadFromXML(const QDomElement& root, KGamePropertyHandler* dataHandler);

	static QString propertyValue(const QDomElement& root, unsigned long int id);

	/**
	 * @return TRUE when the property got removed, FALSE if it does not
	 * exist at all.
	 **/
	static bool removeProperty(QDomElement& root, unsigned long int id);

signals:
	/**
	 * The type of the value for @ref propertyValue is unknwon, so this
	 * signal is emitted in order to ask you to fill in the proper value to
	 * @p value.
	 **/
	void signalRequestValue(KGamePropertyBase* prop, QString& value);

	/**
	 * This is the little brother of @ref signalRequestValue - it is used by
	 * @ref propertySetValue and asks you to assign the value from @p value
	 * to @p prop.
	 **/
	void signalRequestSetValue(KGamePropertyBase* prop, const QString& value);
};

/**
 * This class does exactly the same as @ref BosonPropertyXML, but is more
 * specific to boson. We implement more complex datatypes here, such as a @ref
 * QPointArray.
 *
 * If you want to add additional datatypes to boson's xml files - add them in
 * this class!
 * @short Class that implements more complex datatypes for @ref BosonPropertyXML
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonCustomPropertyXML : public BosonPropertyXML
{
	Q_OBJECT
public:
	BosonCustomPropertyXML(QObject* parent = 0);

protected slots:
	/**
	 * Retrieve the value from @p prop and place it (encoded as a string)
	 * into @ref value.
	 * @param prop The property that should get saved
	 * @param value The string that will be placed into the XML file. Set to
	 * @ref QString::null if you can't handle this property.
	 **/
	void slotRequestValue(KGamePropertyBase* prop, QString& value);

	/**
	 * Set the property @p prop to the value specified in @p value.
	 * @param prop The property that is the destination of the new value.
	 * @param value The text from the XML file
	 **/
	void slotRequestSetValue(KGamePropertyBase* prop, const QString& value);

protected:
	void save(const QPoint& point, QString& string);
	void save(const Q3PointArray& pointArray, QString& string);
};

Q3TextStream& operator>>(Q3TextStream& s, QPoint& p);
Q3TextStream& operator<<(Q3TextStream& s, const QPoint& p);
Q3TextStream& operator>>(Q3TextStream& s, BoVector2Fixed& p);
Q3TextStream& operator<<(Q3TextStream& s, const BoVector2Fixed& p);

#endif
