/*
    This file is part of the Boson game
    Copyright (C) 2003-2005 Andreas Beckermann (b_mann@gmx.de)

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
#include "bosongroundtheme.h"

#include "../bomemory/bodummymemory.h"
#include "bosonmap.h"
#include "bodebug.h"
#include "defines.h"
#include "bosondata.h"
#include "bo3dtools.h"
#include "bosonconfig.h"

#include <ksimpleconfig.h>

/**
 * @short plugin for BosonData providing access to a @ref BosonGroundTheme object
 **/
class BosonGroundData : public BosonDataObject
{
public:
	/**
	 * @param groundFile The index.ground file of this groundTheme.
	 * @param groundTheme The @ref BosonGroundTheme object we are operating
	 * on. Note that this class will take ownership of the pointer and
	 * delete it on destruction!
	 **/
	BosonGroundData(const QString& groundFile, BosonGroundTheme* groundTheme);

	virtual ~BosonGroundData()
	{
		delete mGroundTheme;
	}

	/*
	 * @return The identifier of this groundTheme. Should be in the
	 * index.ground file (e.g. "earth").
	 **/
	virtual QString idString() const
	{
		return mId;
	}
	virtual void* pointer() const
	{
		return (void*)groundTheme();
	}
	BosonGroundTheme* groundTheme() const
	{
		return mGroundTheme;
	}

	virtual bool load()
	{
		if (!groundTheme()) {
			BO_NULL_ERROR(groundTheme());
			return false;
		}
		return true;
	}

private:
	BosonGroundTheme* mGroundTheme;
	QString mId;
};

BosonGroundData::BosonGroundData(const QString& groundFile, BosonGroundTheme* groundTheme)
	: BosonDataObject(groundFile)
{
 mGroundTheme = groundTheme;
 if (!mGroundTheme->loadGroundThemeConfig(groundFile)) {
	boError() << k_funcinfo << "unable to load ground theme config file " << groundFile << endl;
	mId = QString::null;
	return;
 }
 mId = mGroundTheme->identifier();
 if (mId.isEmpty()) {
	boError() << k_funcinfo << "no identifier in " << groundFile << endl;
	mId = QString::null;
	return;
 }
}



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
 QStringList list = BosonData::availableFiles(QString::fromLatin1("themes/grounds/*/index.ground"));
 if (list.isEmpty()) {
	boWarning() << k_funcinfo << "Cannot find any ground themes!" << endl;
	return false;
 }
 QStringList::Iterator it;
 for (it = list.begin(); it != list.end(); ++it) {
	BosonGroundData* data = new BosonGroundData(*it, new BosonGroundTheme());
	if (data->idString().isEmpty()) {
		// probably loadGroundThemeConfig() error
		boError() << k_funcinfo << *it << " could not be loaded" << endl;
		delete data;
		continue;
	}
	if (!BosonData::bosonData()->insertGroundTheme(data)) {
		boWarning() << k_funcinfo << "unable to insert theme " << *it << endl;
		delete data;
	}
 }
 if (BosonData::bosonData()->availableGroundThemes().count() == 0) {
	boError() << k_funcinfo << "no valid groundTheme found" << endl;
	return false;
 }
 return true;
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

 for (unsigned int i = 0; i < grounds && ret; i++) {
	QString group = QString::fromLatin1("Ground_%1").arg(i);
	if (!conf.hasGroup(group)) {
		boError() << k_funcinfo << file << " has no group " << group << endl;
		ret = false;
		continue;
	}
	conf.setGroup(group);

	BosonGroundType* ground = new BosonGroundType;
	ground->index = i;
	ground->name = conf.readEntry("Name", group);
	ground->textureFile = conf.readEntry("Texture", QString::null);
	if (ground->textureFile.isEmpty()) {
		boError() << k_funcinfo << file << " Group=" << group << " has no or invalid Texture key" << endl;
		ret = false;
		delete ground;
		break;
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

	mGroundTypes.insert(i, ground);
 }

 if (!ret) {
	boError() << k_funcinfo << "Could not load ground theme config file " << file << endl;
	mGroundTypes.clear();
	return ret;
 }

 d->mId = identifier;
 d->mGroundThemeDir = dir;
 return ret;
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

