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
#include "bosongroundtheme.h"

#include "bosontexturearray.h"
#include "bosonmap.h"
#include "bodebug.h"
#include "defines.h"
#include "bosondata.h"

#include <ksimpleconfig.h>

#include <qimage.h>
#include <qvaluevector.h>

struct TextureGroundType
{
	QString mFile; // relative to groundTheme dir
	int mGroundType; // AB: we need a groundType enum for this. of any kind... this variable mustn't depend on that enum though.
	unsigned char mAmountOfLand;
	unsigned char mAmountOfWater;
	QRgb mMiniMapColor;
};


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

	virtual bool load();

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

bool BosonGroundData::load()
{
 if (!groundTheme()) {
	BO_NULL_ERROR(groundTheme());
	return false;
 }
 QString dir = file();
 dir = dir.left(dir.length() - QString::fromLatin1("index.ground").length());
 return groundTheme()->loadGroundTheme(dir);
}


class BosonGroundThemePrivate
{
public:
	BosonGroundThemePrivate()
	{
	}
	QValueVector<TextureGroundType> mGroundTypes;
	QString mId;
};

BosonGroundTheme::BosonGroundTheme()
{
 d = new BosonGroundThemePrivate;
 mTextures = 0;
}

BosonGroundTheme::~BosonGroundTheme()
{
 delete mTextures;
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
		delete data;
	}
 }
 return true;
}


bool BosonGroundTheme::loadGroundThemeConfig(const QString& file)
{
 KSimpleConfig conf(file);
 conf.setGroup("Boson Ground");
 QString identifier = conf.readEntry("Identifier", QString::null);
 if (identifier.isEmpty()) {
	boError() << k_funcinfo << "No Identifier in " << file << endl;
	return false;
 }
 unsigned int textures = conf.readUnsignedNumEntry("Textures", 0);
 if (textures == 0) {
	boError() << k_funcinfo << "need at least one texture!" << endl;
	return false;
 } else if (textures > 200) {
	boError() << k_funcinfo << "more than 200 textures - this must be "
				<< "config file error!" << endl;
	return false;
 }
 QValueVector<TextureGroundType> types(textures);
 bool ret = true;
 for (unsigned int i = 0; i < textures && ret; i++) {
	QString group = QString::fromLatin1("Texture_%1").arg(i);
	if (!conf.hasGroup(group)) {
		boError() << k_funcinfo << file << " has no group " << group << endl;
		ret = false;
		continue;
	}
	conf.setGroup(group);
	QString texFile = conf.readEntry("File", QString::null);
	if (texFile.isEmpty()) {
		boError() << k_funcinfo << file
				<< " Group=" << group
				<< " has no or invalid File key"
				<< endl;
		ret = false;
		continue;
	}
	types[i].mFile = texFile;
	// the other values have usable defaults.
	types[i].mAmountOfLand = (unsigned char)conf.readUnsignedNumEntry("AmountOfLand", 0);
	types[i].mAmountOfWater = (unsigned char)conf.readUnsignedNumEntry("AmountOfWater", 0);
	types[i].mMiniMapColor = conf.readUnsignedNumEntry("MiniMapColor", 0);


#warning FIXME
	// currently we use 0=grass,1=desert,2=water in both code and
	// index.ground. but it is rather a workaround - we should fix it.
	types[i].mGroundType = (int)i;
 }
 if (!ret) {
	boError() << k_funcinfo << "Could not load ground theme config file " << file << endl;
	types.clear();
	return ret;
 }
 d->mGroundTypes = types;
 d->mId = identifier;
 return ret;
}

// AB: maybe we can use QString::null here to unload a theme
bool BosonGroundTheme::loadGroundTheme(QString dir)
{
 if (textureCount() == 0) {
	boWarning() << k_funcinfo << "no textures available to be loaded in this groundTheme. probably failed at loading the index.ground file!" << endl;
	return false;
 }
 // dir is e.g. /opt/kde3/share/apps/boson/themes/grounds/earth/ -> "earth" is
 // the important part!
 if (dir.right(1) != QString::fromLatin1("/")) {
	dir += QString::fromLatin1("/");
 }
 delete mTextures;
 mTextures = 0;

 QValueList<QImage> images;
 for (unsigned int i = 0; i < textureCount(); i++) {
	QImage image = loadTextureImage(dir, groundType(i), amountOfLand(i), amountOfWater(i));
	if (image.isNull()) {
		boError() << k_funcinfo << "Null image" << endl;
		return false;
	}
	images.append(image);
 }

 mTextures = new BosonTextureArray(images);
 return true;
}

unsigned char BosonGroundTheme::amountOfLand(unsigned int texture) const
{
 if (texture >= textureCount()) {
	return 0;
 }
 return d->mGroundTypes[texture].mAmountOfLand;
}

unsigned char BosonGroundTheme::amountOfWater(unsigned int texture) const
{
 if (texture >= textureCount()) {
	return 0;
 }
 return d->mGroundTypes[texture].mAmountOfWater;
}

QRgb BosonGroundTheme::miniMapColor(unsigned int texture) const
{
 if (texture >= textureCount()) {
	return 0;
 }
 return d->mGroundTypes[texture].mMiniMapColor;
}

int BosonGroundTheme::groundType(unsigned int texture) const
{
 if (texture >= textureCount()) {
	return 0;
 }
 return d->mGroundTypes[texture].mGroundType;
}

unsigned int BosonGroundTheme::textureCount() const
{
 return d->mGroundTypes.count();
}

QImage BosonGroundTheme::loadTextureImage(const QString& dir, int groundType, unsigned char amountOfLand, unsigned char amountOfWater)
{
 QString file = dir + groundType2Name(groundType) + QString("-00.png");
 QImage image(file);
 if (image.isNull()) {
	boWarning() << k_funcinfo << "Could not find texture image for " << groundType << ". expected file: " << file << endl;


	// if the groundType cannot be found we should try to find a close
	// replacement using amountOfLand/Water.
	Q_UNUSED(amountOfLand);
	Q_UNUSED(amountOfWater);

	// load dummy image.
	image = QImage(64, 64, 32);
	image.fill(Qt::green.rgb());
 }
 return image;
}

const QString& BosonGroundTheme::identifier() const
{
 return d->mId;
}

QString BosonGroundTheme::groundType2Name(int groundType)
{
#warning TODO: add an enum
 switch (groundType) {
	case 0: // GroundGrass
		return QString::fromLatin1("grass");
	case 1: // GroundDesert
		return QString::fromLatin1("desert");
	case 2: // GroundWater
		return QString::fromLatin1("water");
	default:
		boError() << "Invalid GroundType " << groundType << endl;
		break;
 }
 return QString::null;
}

