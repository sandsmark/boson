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
#ifndef BOSONPROPERTYXML_H
#define BOSONPROPERTYXML_H

#include <qobject.h>

class QString;
class QDomElement;
class KGamePropertyBase;
class KGamePropertyHandler;

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

#endif
