/*
    This file is part of the Boson game
    Copyright (C) 2003-2008 Andreas Beckermann (b_mann@gmx.de)

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
#include "bosongroundtheme.h"

#include "../../bomemory/bodummymemory.h"
#include "bosonmap.h"
#include "bodebug.h"
#include "../defines.h"
#include "../bosondata.h"
#include "../bo3dtools.h"
#include "../bosonconfig.h"

#include <ksimpleconfig.h>

BosonGroundType::BosonGroundType()
{
 index = -1;
 bumpScale = 1.0f;
 bumpBias = 0.0f;
 textureSize = 0.0f;
 animationDelay = 1;
 color = Qt::black.rgb();
}

BosonGroundType::~BosonGroundType()
{
}


class BosonGroundThemePrivate
{
public:
	BosonGroundThemePrivate()
	{
	}
	QString mGroundThemeDir;
	QString mId;
};

BosonGroundTheme::BosonGroundTheme()
{
 d = new BosonGroundThemePrivate;
 mGroundTypes.setAutoDelete(true);
}

BosonGroundTheme::~BosonGroundTheme()
{
 mGroundTypes.clear();
 delete d;
}

bool BosonGroundTheme::createGroundThemeList()
{
 if (BosonData::bosonData()->availableGroundThemes().count() != 0) {
	boWarning() << k_funcinfo << "groundthemes already loaded" << endl;
	return true;
 }
 QStringList list = groundThemeFiles();
 if (list.isEmpty()) {
	boWarning() << k_funcinfo << "Cannot find any ground themes!" << endl;
	return false;
 }
 QStringList::Iterator it;
 for (it = list.begin(); it != list.end(); ++it) {
	BosonGroundTheme* theme = new BosonGroundTheme();
	if (!theme->loadGroundThemeConfig(*it)) {
		boError() << k_funcinfo << "unable to load ground theme config file " << *it << endl;
		delete theme;
		continue;
	}

	BosonGenericDataObject* data = new BosonGenericDataObject(*it, theme->identifier(), theme);
	if (!BosonData::bosonData()->insertGroundTheme(data)) {
		boWarning() << k_funcinfo << "unable to insert theme " << *it << endl;
		delete data;
		continue;
	}
 }
 if (BosonData::bosonData()->availableGroundThemes().count() == 0) {
	boError() << k_funcinfo << "no valid groundTheme found" << endl;
	return false;
 }
 return true;
}

QStringList BosonGroundTheme::groundThemeFiles()
{
 QStringList list = BosonData::availableFiles(QString::fromLatin1("themes/grounds/*/index.ground"));
 return list;
}


bool BosonGroundTheme::loadGroundThemeConfig(const QString& file)
{
 QString dir = file;
 if (!file.endsWith("index.ground")) {
	boError() << k_funcinfo << "invalid filename " << file << " - it must end with index.ground" << endl;
	return false;
 }
 dir = dir.left(dir.length() - QString::fromLatin1("index.ground").length());
 if (dir.right(1) != QString::fromLatin1("/")) {
	dir += QString::fromLatin1("/");
 }

 KSimpleConfig conf(file);
 conf.setGroup("Boson Ground");
 QString identifier = conf.readEntry("Identifier", QString::null);
 if (identifier.isEmpty()) {
	boError() << k_funcinfo << "No Identifier in " << file << endl;
	return false;
 }
 unsigned int grounds = conf.readUnsignedNumEntry("Grounds", 0);
 if (grounds == 0) {
	boError() << k_funcinfo << "need at least one ground type!" << endl;
	return false;
 } else if (grounds > 200) {
	boError() << k_funcinfo << "more than 200 ground types - this must be "
			<< "config file error!" << endl;
	return false;
 }

 bool ret = true;

 QPtrVector<BosonGroundType> types(grounds);
 for (unsigned int i = 0; i < grounds && ret; i++) {
	BosonGroundType* ground = loadGroundType(conf, i);
	if (!ground) {
		boError() << k_funcinfo << "unable to load groundtype " << i << " from file " << file << endl;
		ret = false;
		continue;
	}
	types.insert(i, ground);
 }


 if (!ret) {
	boError() << k_funcinfo << "Could not load ground types from config file " << file << endl;
	types.setAutoDelete(true);
	types.clear();
	return ret;
 }

 types.setAutoDelete(false);
 if (!applyGroundThemeConfig(identifier, types, dir)) {
	types.setAutoDelete(true);
	types.clear();
	boError() << k_funcinfo << "Could not apply ground theme loaded from config file " << file << endl;
	return false;
 }

 return ret;
}

bool BosonGroundTheme::applyGroundThemeConfig(const QString& identifier, const QPtrVector<BosonGroundType>& types, const QString& themeDir)
{
 if (!d->mId.isNull()) {
	boError() << k_funcinfo << "theme already loaded in this object" << endl;
	return false;
 }
 mGroundTypes = types;
 mGroundTypes.setAutoDelete(true);
 d->mId = identifier;
 d->mGroundThemeDir = themeDir;

 for (unsigned int i = 0; i < mGroundTypes.count(); i++) {
	mGroundTypes[i]->index = i;
 }

 return true;
}

BosonGroundType* BosonGroundTheme::loadGroundType(KSimpleConfig& conf, unsigned int index)
{
 QString group = QString::fromLatin1("Ground_%1").arg(index);
 if (!conf.hasGroup(group)) {
	boError() << k_funcinfo << "no such group " << group << endl;
	return 0;
}
 conf.setGroup(group);

 BosonGroundType* ground = new BosonGroundType;
 ground->index = index;
 ground->name = conf.readEntry("Name", group);
 ground->textureFile = conf.readEntry("Texture", QString::null);
 if (ground->textureFile.isEmpty()) {
	boError() << k_funcinfo << "Group=" << group << " has no or invalid Texture key" << endl;
	delete ground;
	return 0;
 }
 ground->bumpTextureFile = conf.readEntry("BumpTexture", "bump-null");
 ground->bumpScale = (float)(conf.readDoubleNumEntry("BumpScale", 0.04f));
 ground->bumpBias = (float)(conf.readDoubleNumEntry("BumpBias", 0.5f)) * ground->bumpScale;
 ground->textureSize = (float)(conf.readDoubleNumEntry("TextureSize", 5.0f));
 ground->shaderFile = conf.readEntry("Shader", "ground-default");
 BoVector3Float color = BosonConfig::readBoVector3FloatEntry(&conf, "MiniMapColor");
 ground->color = qRgb((int)color.x(), (int)color.y(), (int)color.z());
 ground->animationDelay = conf.readUnsignedNumEntry("AnimationDelay", 1);
 ground->iconFile = conf.readEntry("Pixmap", QString::null);

 return ground;
}

BosonGroundType* BosonGroundTheme::groundType(unsigned int i) const
{
 if (i >= mGroundTypes.count()) {
	boError() << k_funcinfo << "index " << i << " is out of range. count=" << mGroundTypes.count() << endl;
	return 0;
 }
 return mGroundTypes[i];
}

unsigned int BosonGroundTheme::groundTypeCount() const
{
 return mGroundTypes.count();
}

const QString& BosonGroundTheme::identifier() const
{
 return d->mId;
}

const QString& BosonGroundTheme::themeDirectory() const
{
 return d->mGroundThemeDir;
}

