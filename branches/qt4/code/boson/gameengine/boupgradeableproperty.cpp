/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "boupgradeableproperty.h"

#include "../bomemory/bodummymemory.h"
#include <bodebug.h>
#include "upgradeproperties.h"
#include "speciestheme.h"

#include <qdom.h>

class BaseValueEntryBase
{
public:
	BaseValueEntryBase()
	{
	}
	virtual ~BaseValueEntryBase()
	{
	}
	virtual unsigned long int toULong() const = 0;
	virtual long int toLong() const = 0;
	virtual bofixed toBoFixed() const = 0;

	virtual void setValue(unsigned long int v) = 0;
	virtual void setValue(long int v) = 0;
	virtual void setValue(bofixed v) = 0;
};

template<class T> class BaseValueEntry : public BaseValueEntryBase
{
public:
	BaseValueEntry() : BaseValueEntryBase()
	{
	}
	BaseValueEntry(const BaseValueEntry& v) : BaseValueEntryBase()
	{
		*this = v;
	}
	BaseValueEntry(const T& v) : BaseValueEntryBase()
	{
		mValue = v;
	}
	const BaseValueEntry& operator=(const BaseValueEntry& e)
	{
		mValue = e.mValue;
		return *this;
	}
	const BaseValueEntry& operator=(const T& v)
	{
		mValue = v;
		return *this;
	}
	operator T() const
	{
		return mValue;
	}

	virtual unsigned long int toULong() const { return (unsigned long int)mValue; }
	virtual long int toLong() const { return (long int)mValue; }
	virtual bofixed toBoFixed() const { return (bofixed)mValue; }

	virtual void setValue(unsigned long int v) { mValue = (T)v; }
	virtual void setValue(long int v)  { mValue = (T)v; }
	virtual void setValue(bofixed v)  { mValue = (T)v; }

private:
	T mValue;
};

class BoBaseValueCollectionPrivate
{
public:
	BoBaseValueCollectionPrivate()
	{
	}
	QMap< QString, QMap<QString, BaseValueEntryBase*> > mProperties;
};

BoBaseValueCollection::BoBaseValueCollection()
{
 d = new BoBaseValueCollectionPrivate;
}

BoBaseValueCollection::~BoBaseValueCollection()
{
 QMap< QString, QMap<QString, BaseValueEntryBase*> >::iterator it;
 for (it = d->mProperties.begin(); it != d->mProperties.end(); ++it) {
	QMap<QString, BaseValueEntryBase*>::iterator it2;
	for (it2 = (*it).begin(); it2 != (*it).end(); ++it2) {
		delete *it2;
	}
	(*it).clear();
 }
 delete d;
}

bool BoBaseValueCollection::insertULongBaseValue(unsigned long int v, const QString& name, const QString& type, bool replace)
{
 if (type != "MaxValue" && type != "MinValue") {
	boError() << k_funcinfo << "invalid type " << type << endl;
	return false;
 }
 if (!(d->mProperties[type]).contains(name)) {
	BaseValueEntryBase* value = new BaseValueEntry<unsigned long int>(v);
	(d->mProperties[type]).insert(name, value);
 }
 if (replace) {
	(d->mProperties[type])[name]->setValue(v);
 }
 return true;
}

bool BoBaseValueCollection::insertLongBaseValue(long int v, const QString& name, const QString& type, bool replace)
{
 if (type != "MaxValue" && type != "MinValue") {
	boError() << k_funcinfo << "invalid type " << type << endl;
	return false;
 }
 if (!(d->mProperties[type]).contains(name)) {
	BaseValueEntryBase* value = new BaseValueEntry<long int>(v);
	(d->mProperties[type]).insert(name, value);
 }
 if (replace) {
	(d->mProperties[type])[name]->setValue(v);
 }
 return true;
}

bool BoBaseValueCollection::insertBoFixedBaseValue(bofixed v, const QString& name, const QString& type, bool replace)
{
 if (type != "MaxValue" && type != "MinValue") {
	boError() << k_funcinfo << "invalid type " << type << endl;
	return false;
 }
 if (!(d->mProperties[type]).contains(name)) {
	BaseValueEntryBase* value = new BaseValueEntry<bofixed>(v);
	(d->mProperties[type]).insert(name, value);
 }
 if (replace) {
	(d->mProperties[type])[name]->setValue(v);
 }
 return true;
}

bool BoBaseValueCollection::getBaseValue(unsigned long int* ret, const QString& name, const QString& type) const
{
 if (type != "MaxValue" && type != "MinValue") {
	boError() << k_funcinfo << "invalid type " << type << endl;
	return false;
 }
 if (!(d->mProperties[type]).contains(name)) {
	boError() << k_funcinfo << "no such property " << name << endl;
	return false;
 }
 *ret = (d->mProperties[type])[name]->toULong();
 return true;
}

bool BoBaseValueCollection::getBaseValue(long int* ret, const QString& name, const QString& type) const
{
 if (type != "MaxValue" && type != "MinValue") {
	boError() << k_funcinfo << "invalid type " << type << endl;
	return false;
 }
 if (!(d->mProperties[type]).contains(name)) {
	boError() << k_funcinfo << "no such property " << name << endl;
	return false;
 }
 *ret = (d->mProperties[type])[name]->toLong();
 return true;
}

bool BoBaseValueCollection::getBaseValue(bofixed* ret, const QString& name, const QString& type) const
{
 if (type != "MaxValue" && type != "MinValue") {
	boError() << k_funcinfo << "invalid type " << type << endl;
	return false;
 }
 if (!(d->mProperties[type]).contains(name)) {
	boError() << k_funcinfo << "no such property " << name << endl;
	return false;
 }
 *ret = (d->mProperties[type])[name]->toBoFixed();
 return true;
}

unsigned long int BoBaseValueCollection::ulongBaseValue(const QString& name, const QString& type, unsigned long int defaultValue) const
{
 unsigned long int v = defaultValue;
 if (!getBaseValue(&v, name, type)) {
	return defaultValue;
 }
 return v;
}

long int BoBaseValueCollection::longBaseValue(const QString& name, const QString& type, long int defaultValue) const
{
 long int v = defaultValue;
 if (!getBaseValue(&v, name, type)) {
	return defaultValue;
 }
 return v;
}

bofixed BoBaseValueCollection::bofixedBaseValue(const QString& name, const QString& type, bofixed defaultValue) const
{
 bofixed v = defaultValue;
 if (!getBaseValue(&v, name, type)) {
	return defaultValue;
 }
 return v;
}




BoUpgradesCollection::BoUpgradesCollection()
{
 mUpgrades = new QValueList<const UpgradeProperties*>();

 // all properties' counters are initialized with 0, so when we
 // set this counter to 1, all properties are initiall dirty
 mUpgradesCacheCounter = 1;
}

BoUpgradesCollection::~BoUpgradesCollection()
{
 delete mUpgrades;
}

void BoUpgradesCollection::addUpgrade(const UpgradeProperties* upgrade)
{
 BO_CHECK_NULL_RET(upgrade);
 mUpgrades->append(upgrade);

 // AB: the counter is increased whenever the upgrades of the player change in
 // any way. this causes all values that depend on upgrades to be recalculated
 // when required.
 mUpgradesCacheCounter++;
}

void BoUpgradesCollection::clearUpgrades()
{
 while (!upgrades()->isEmpty()) {
	removeUpgrade(upgrades()->first());
 }
}


const UpgradeProperties* BoUpgradesCollection::findUpgrade(unsigned long int id) const
{
 QValueList<const UpgradeProperties*>::const_iterator it;
 for (it = upgrades()->begin(); it != upgrades()->end(); ++it) {
	if ((*it)->id() == id) {
		return *it;
	}
 }
 return 0;
}

bool BoUpgradesCollection::removeUpgrade(const UpgradeProperties* upgrade)
{
 if (!upgrade) {
	return false;
 }
 bool ret = false;
 if (mUpgrades->remove(upgrade) > 0) {
	ret = true;
	mUpgradesCacheCounter++;
 }

 return ret;
}

bool BoUpgradesCollection::saveAsXML(QDomElement& root) const
{
 if (root.isNull()) {
	return false;
 }
 QDomDocument doc = root.ownerDocument();
 QValueList<const UpgradeProperties*>::const_iterator it;
 for (it = upgrades()->begin(); it != upgrades()->end(); ++it) {
	QDomElement e = doc.createElement("Upgrade");
	e.setAttribute("Id", QString::number((*it)->id()));
	e.setAttribute("Type", (*it)->type());
	root.appendChild(e);
 }
 return true;
}

bool BoUpgradesCollection::loadFromXML(const SpeciesTheme* speciesTheme, const QDomElement& root)
{
 clearUpgrades();
 if (!speciesTheme) {
	BO_NULL_ERROR(speciesTheme);
	return false;
 }
 if (root.isNull()) {
	return false;
 }
 QDomDocument doc = root.ownerDocument();
 for (QDomNode n = root.firstChild(); !n.isNull(); n = n.nextSibling()) {
	QDomElement e = n.toElement();
	if (e.isNull()) {
		continue;
	}
	if (e.tagName() != "Upgrade") {
		continue;
	}
	bool ok = false;
	unsigned long int id = e.attribute("Id").toULong(&ok);
	if (!ok) {
		boError() << k_funcinfo << "invalid Id attribute" << endl;
		return false;
	}
	QString type = e.attribute("Type");
	if (type.isEmpty()) {
		boError() << k_funcinfo << "no Type given" << endl;
		return false;
	}

	const UpgradeProperties* upgrade = speciesTheme->upgrade(type, id);
	if (!upgrade) {
		boError() << k_funcinfo << "cannot find upgrade type=" << type << " id=" << id << endl;
		return false;
	}
	addUpgrade(upgrade);
 }
 return true;
}


bool BoUpgradeablePropertyBase::upgradeValue(const QValueList<const UpgradeProperties*>* list, unsigned long int* v) const
{
 QValueList<const UpgradeProperties*>::const_iterator it;
 for (it = list->begin(); it != list->end(); ++it) {
	if (!(*it)->upgradeValue(name(), v, type())) {
		boError() << k_funcinfo << "upgrade failed" << endl;
		return false;
	}
 }
 return true;
}

bool BoUpgradeablePropertyBase::upgradeValue(const QValueList<const UpgradeProperties*>* list, long int* v) const
{
 QValueList<const UpgradeProperties*>::const_iterator it;
 for (it = list->begin(); it != list->end(); ++it) {
	if (!(*it)->upgradeValue(name(), v, type())) {
		boError() << k_funcinfo << "upgrade failed" << endl;
		return false;
	}
 }
 return true;
}

bool BoUpgradeablePropertyBase::upgradeValue(const QValueList<const UpgradeProperties*>* list, bofixed* v) const
{
 QValueList<const UpgradeProperties*>::const_iterator it;
 for (it = list->begin(); it != list->end(); ++it) {
	if (!(*it)->upgradeValue(name(), v, type())) {
		boError() << k_funcinfo << "upgrade failed" << endl;
		return false;
	}
 }
 return true;
}


