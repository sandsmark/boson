/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosonplayfield.h"

#include "bosonmap.h"
#include "bosonscenario.h"

#include <qdom.h>
#include <qdatastream.h>
#include <qfile.h>
#include <qbuffer.h>

#include <kdebug.h>
#include <kfilterdev.h>
#include <kstandarddirs.h>
#include <ksimpleconfig.h>

#include "bosonplayfield.moc"

class BosonPlayField::BosonPlayFieldPrivate
{
public:
	BosonPlayFieldPrivate()
	{
	}
};

BosonPlayField::BosonPlayField(QObject* parent) : QObject(parent, "BosonPlayField")
{
 d = new BosonPlayFieldPrivate;

 mMap = 0;
 mScenario = 0;
}

BosonPlayField::~BosonPlayField()
{
 emit signalNewMap(0);
 delete mMap;
 delete mScenario;
 delete d;
}

QString BosonPlayField::defaultPlayField()
{
//TODO create a list of all playfield (search for .desktop files in boson/map/)
// (AB: still boson/map ?? )
// andlook if list contains "basic" - then check whether basic.bpf exists and return. 
// Otherwise look for the first .desktop file which has a .bpf file exisiting.
// Return that then.

// QString bpf = KGlobal::dirs()->findResource("data", "boson/map/basic.bpf") + "boson/map/basic.bpf";// AB: still map?
// kdDebug() << bpf << endl;



 return QString::fromLatin1("Basic");
}

QStringList BosonPlayField::availablePlayFields()
{
 QStringList list = KGlobal::dirs()->findAllResources("data", 
		"boson/map/*.desktop");
 if (list.isEmpty()) {
	kdError() << "Cannot find any playfield?!" << endl;
	return list;
 }
 QStringList validList;
 for (unsigned int i = 0; i < list.count(); i++) {
	QString fileName = list[i].left(list[i].length() -  strlen(".desktop"));
	fileName += QString::fromLatin1(".bpf");
	if (QFile::exists(fileName)) {
		validList.append(list[i]);
	}
 }
 return validList;
}

bool BosonPlayField::loadPlayField(const QString& file)
{
 QIODevice* dev = KFilterDev::deviceForFile(file);
 if (!dev) {
	kdError() << k_funcinfo << "No file " << file << endl;
	return false;
 }
 if (!dev->open(IO_ReadOnly)) {
	kdError() << k_funcinfo << "Can't open file " << file << endl;
	delete dev;
	return false;
 }
 QDomDocument doc("BosonPlayField");
 QString errorMsg;
 int lineNo, columnNo;
 if (!doc.setContent(dev->readAll(), &errorMsg, &lineNo, &columnNo)) {
	kdError() << "Parse error in line " << lineNo << ",column " << columnNo
			<< " error message: " << errorMsg << endl;
	delete dev;
	return false;
 }
 delete dev;

 QDomElement root = doc.documentElement();

 if (!loadMap(root)) {
	kdError() << "Error loading map from " << file << endl;
	return false;
 }
 if (!loadScenario(root)) {
	kdError() << "Error loading scenario from " << file << endl;
	return false;
 }
 return true;
}

bool BosonPlayField::savePlayField(const QString& fileName)
{
 if (!mMap) {
	kdError() << k_funcinfo << "NULL map" << endl;
	return false;
 }
 if (!mScenario) {
	kdError() << k_funcinfo << "NULL scenario" << endl;
	return false;
 }
 QDomDocument doc("BosonPlayField");
 QDomElement root = doc.createElement("BosonPlayField");
 doc.appendChild(root);

 if (!mScenario->saveScenario(root)) {
	kdError() << k_funcinfo << "Error saving scenario" << endl;
	return false;
 }
 if (!mMap->saveMap(root)) {
	kdError() << k_funcinfo << "Error saving map" << endl;
	return false;
 }

 QIODevice* dev = KFilterDev::deviceForFile(fileName, "application/x-gzip");
 if (!dev) {
	kdError() << k_funcinfo << "Cannot save to " << fileName << endl;
	return false;
 }
 if (!dev->open(IO_WriteOnly)) {
	kdError() << k_funcinfo << "Cannot open " << fileName << endl;
	delete dev;
	return false;
 }
 QString xml = doc.toString();
 dev->writeBlock(xml.data(), xml.length());
 dev->close();
 delete dev;
 mMap->setModified(false);
 mScenario->setModified(false);
 return true;
}

bool BosonPlayField::loadMap(QDomElement& root)
{
 QDomNodeList list = root.elementsByTagName("BosonMap");
 if (list.count() < 1) {
	kdError() << k_funcinfo << "No map found in file" << endl;
	return false;
 } else if (list.count() > 1) {
	kdWarning() << k_funcinfo << "More than one map in file - picking first" << endl;
 }
 QDomElement node = list.item(0).toElement();
 delete mMap;
 mMap = new BosonMap(this);
 bool ret = mMap->loadMap(node);
 emit signalNewMap(mMap);
 return ret;
}

bool BosonPlayField::loadScenario(QDomElement& root)
{
 QDomNodeList list = root.elementsByTagName("BosonScenario");
 if (list.count() < 1) {
	kdError() << k_funcinfo << "No scenario found in file" << endl;
	return false;
 } else if (list.count() > 1) {
	kdWarning() << k_funcinfo << "More than one scenario in file ... we probably will never support this - but definitely *not* yet. picking first" << endl;
 }
 QDomElement node = list.item(0).toElement();
 delete mScenario;
 mScenario = new BosonScenario();
 return mScenario->loadScenario(node);
}

void BosonPlayField::loadMap(QDataStream& stream)
{
 delete mMap;
 mMap = 0;
 mMap = new BosonMap(this);
 mMap->loadMapGeo(stream);
 mMap->loadCells(stream);
 emit signalNewMap(mMap);
}

void BosonPlayField::saveMap(QDataStream& stream)
{
 if (!mMap) {
	kdError() << k_funcinfo << "NULL map" << endl;
	return;
 }
 mMap->saveMapGeo(stream);
 mMap->saveCells(stream);
}

void BosonPlayField::quit()
{
 emit signalNewMap(0);
 delete mMap;
 mMap = 0;
 delete mScenario;
 mScenario = 0;
}

QString BosonPlayField::playFieldFileName(const QString& identifier)
{
 QStringList l = availablePlayFields();
 for (unsigned int i = 0; i < l.count(); i++) {
	KSimpleConfig cfg(l[i]);
	cfg.setGroup("Boson PlayField");
	if (cfg.readEntry("Identifier") == identifier) {
		QString m = l[i].left(l[i].length() - strlen(".desktop"));
		m += QString::fromLatin1(".bpf");
		if (QFile::exists(m)) {
			return m;
		} else {
			kdError() << "Cannot find " << m << " for valid .desktop file" << endl;
		}
	}
 }
 kdWarning() << "no map file found for " << identifier << endl;
 return QString::null;
}

void BosonPlayField::applyScenario(Boson* boson)
{
 delete mScenario;
 mScenario = new BosonScenario();
 mScenario->applyScenario(boson);
}

bool BosonPlayField::modified() const
{
 if (mMap && mMap->modified()) {
	return true;
 }
 if (mScenario && mScenario->modified()) {
	return true;
 }
 return false;
}
