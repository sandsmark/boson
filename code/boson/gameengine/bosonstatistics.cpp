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

#include "bosonstatistics.h"

#include "../bomemory/bodummymemory.h"
#include "unitbase.h"
#include "player.h"
#include "bodebug.h"

//#include <qdatastream.h>
#include <qdom.h>

BosonStatistics::BosonStatistics()
{
 mShots = 0;
 mMinedMinerals = 0;
 mMinedOil = 0;
 mRefinedMinerals = 0;
 mRefinedOil = 0;
 mProducedMobileUnits = 0;
 mProducedFacilities = 0;
 mDestroyedMobileUnits = 0;
 mDestroyedFacilities= 0;
 mDestroyedOwnMobileUnits = 0;
 mDestroyedOwnFacilities= 0;
 mLostMobileUnits = 0;
 mLostFacilities= 0;

 mPoints = 0;
}

BosonStatistics::~BosonStatistics()
{
}

quint32 BosonStatistics::shots() const
{
// FIXME: in a long game this value can be greater than the maximal unsigned long
// value! we need to take care of this! One solution might be to add a variable
// mBillionShots and store every new billion there. THen we must return a
// QString here!
 return mShots;
}

void BosonStatistics::increaseShots()
{
// FIXME: in a long game this value can be greater than the maximal unsigned long
// value! we need to take care of this! One solution might be to add a variable
// mBillionShots and store every new billion there. 
// e.g.: mShots++; if (mShots == 1000000) { mBillionShots++; mShots = 0; } 

 mShots++;
}

void BosonStatistics::increaseMinedMinerals(unsigned int increaseBy)
{
 mMinedMinerals += increaseBy;
}

void BosonStatistics::increaseMinedOil(unsigned int increaseBy)
{
 mMinedOil += increaseBy;
}

void BosonStatistics::increaseRefinedMinerals(unsigned int increaseBy)
{
 mRefinedMinerals += increaseBy;
}

void BosonStatistics::increaseRefinedOil(unsigned int increaseBy)
{
 mRefinedOil += increaseBy;
}

void BosonStatistics::addDestroyedMobileUnit(UnitBase* destroyed, Player* destroyedBy)
{
 if (destroyed->owner() == destroyedBy) {
	mDestroyedOwnMobileUnits++;
	mPoints += pointsPerDestroyedOwnMobileUnit();
	return;
 }
 mDestroyedMobileUnits++;
 mPoints += pointsPerDestroyedMobileUnit();
}

void BosonStatistics::addDestroyedFacility(UnitBase* destroyed, Player* destroyedBy)
{
 if (destroyed->owner() == destroyedBy) {
	mDestroyedOwnFacilities++;
	mPoints += pointsPerDestroyedOwnFacility();
	return;
 }
 mDestroyedFacilities++;
 mPoints += pointsPerDestroyedFacility();
}

void BosonStatistics::addLostMobileUnit(UnitBase*)
{
 mLostMobileUnits++;
 mPoints += pointsPerLostMobileUnit();
}

void BosonStatistics::addLostFacility(UnitBase*)
{
 mLostFacilities++;
 mPoints += pointsPerLostFacility();
}

void BosonStatistics::addProducedMobileUnit(UnitBase*, ProductionPlugin*)
{
 mProducedMobileUnits++;
 mPoints += pointsPerProducedMobileUnit();
}

void BosonStatistics::addProducedFacility(UnitBase*, ProductionPlugin*)
{
 mProducedFacilities++;
 mPoints += pointsPerProducedFacility();
}

qint32 BosonStatistics::points() const
{
 qint32 points = mPoints;

// I'm not sure if shot's should get points at all!
// Probably it's better to use an inverse here. less shots is better, more shots
// are worse. but perhaps we simply don't give shots at all.
// points += (quint32)(shots() * 0.001);

 points += (quint32)(refinedMinerals() * pointsPerRefinedMinerals());
 points += (quint32)(refinedOil() * pointsPerRefinedOil());

 return points;
}


float BosonStatistics::pointsPerRefinedMinerals()
{
 return 0.1;
}

float BosonStatistics::pointsPerRefinedOil()
{
 return 0.1;
}

unsigned int BosonStatistics::pointsPerDestroyedMobileUnit()
{
 return 2;
}

unsigned int BosonStatistics::pointsPerDestroyedFacility()
{
 return 5;
}

int BosonStatistics::pointsPerDestroyedOwnMobileUnit()
{
 return -2;
}

int BosonStatistics::pointsPerDestroyedOwnFacility()
{
 return -5;
}

unsigned int BosonStatistics::pointsPerProducedMobileUnit()
{
 return 5;
}

unsigned int BosonStatistics::pointsPerProducedFacility()
{
 return 10;
}

int BosonStatistics::pointsPerLostMobileUnit()
{
 return -2;
}

int BosonStatistics::pointsPerLostFacility()
{
 return -5;
}

unsigned int BosonStatistics::winningPoints()
{
 return 5000;
}

void BosonStatistics::saveULong(QDomElement& root, const QString& tagName, quint32 value) const
{
 QDomDocument doc = root.ownerDocument();
 QDomElement element = doc.createElement(tagName);
 QDomText text = doc.createTextNode(QString::number(value));
 element.appendChild(text);
 root.appendChild(element);
}

void BosonStatistics::saveLong(QDomElement& root, const QString& tagName, qint32 value) const
{
 QDomDocument doc = root.ownerDocument();
 QDomElement element = doc.createElement(tagName);
 QDomText text = doc.createTextNode(QString::number(value));
 element.appendChild(text);
 root.appendChild(element);
}

void BosonStatistics::loadULong(const QDomElement& root, const QString& tagName, quint32* value)
{
 QDomElement element = root.namedItem(tagName).toElement();
 if (element.isNull()) {
	boError() << k_funcinfo << "null element for tag: " << tagName << endl;
	// do not touch value
	return;
 }
 bool ok = false;
 *value = element.text().toULong(&ok);
 if (!ok) {
	boWarning() << k_funcinfo << "Value for " << tagName << " is not a valid number" << endl;
 }
}

void BosonStatistics::loadLong(const QDomElement& root, const QString& tagName, qint32* value)
{
 QDomElement element = root.namedItem(tagName).toElement();
 if (element.isNull()) {
	boError() << k_funcinfo << "null element for tag: " << tagName << endl;
	// do not touch value
	return;
 }
 bool ok = false;
 *value = element.text().toLong(&ok);
 if (!ok) {
	boWarning() << k_funcinfo << "Value for " << tagName << " is not a valid number" << endl;
 }
}

void BosonStatistics::save(QDomElement& root) const
{
 saveULong(root, QString::fromLatin1("Shots"), mShots);
 saveULong(root, QString::fromLatin1("MinedMinerals"), mMinedMinerals);
 saveULong(root, QString::fromLatin1("MinedOil"), mMinedOil);
 saveULong(root, QString::fromLatin1("RefinedMinerals"), mRefinedMinerals);
 saveULong(root, QString::fromLatin1("RefinedOil"), mRefinedOil);
 saveULong(root, QString::fromLatin1("ProducedMobileUnits"), mProducedMobileUnits);
 saveULong(root, QString::fromLatin1("ProducedFacilities"), mProducedFacilities);
 saveULong(root, QString::fromLatin1("DestroyedMobileUnits"), mDestroyedMobileUnits);
 saveULong(root, QString::fromLatin1("DestroyedFacilities"), mDestroyedFacilities);
 saveULong(root, QString::fromLatin1("DestroyedOwnMobileUnits"), mDestroyedOwnMobileUnits);
 saveULong(root, QString::fromLatin1("DestroyedOwnFacilities"), mDestroyedOwnFacilities);
 saveULong(root, QString::fromLatin1("LostMobileUnits"), mLostMobileUnits);
 saveULong(root, QString::fromLatin1("LostFacilities"), mLostFacilities);
 saveLong(root, QString::fromLatin1("Points"), mPoints);
}

void BosonStatistics::load(const QDomElement& root)
{
 loadULong(root, QString::fromLatin1("Shots"), &mShots);
 loadULong(root, QString::fromLatin1("MinedMinerals"), &mMinedMinerals);
 loadULong(root, QString::fromLatin1("MinedOil"), &mMinedOil);
 loadULong(root, QString::fromLatin1("RefinedMinerals"), &mRefinedMinerals);
 loadULong(root, QString::fromLatin1("RefinedOil"), &mRefinedOil);
 loadULong(root, QString::fromLatin1("ProducedMobileUnits"), &mProducedMobileUnits);
 loadULong(root, QString::fromLatin1("ProducedFacilities"), &mProducedFacilities);
 loadULong(root, QString::fromLatin1("DestroyedMobileUnits"), &mDestroyedMobileUnits);
 loadULong(root, QString::fromLatin1("DestroyedFacilities"), &mDestroyedFacilities);
 loadULong(root, QString::fromLatin1("DestroyedOwnMobileUnits"), &mDestroyedOwnMobileUnits);
 loadULong(root, QString::fromLatin1("DestroyedOwnFacilities"), &mDestroyedOwnFacilities);
 loadULong(root, QString::fromLatin1("LostMobileUnits"), &mLostMobileUnits);
 loadULong(root, QString::fromLatin1("LostFacilities"), &mLostFacilities);
 loadLong(root, QString::fromLatin1("Points"), &mPoints);
}

