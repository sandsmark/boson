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
#include "bo3dtools.h"
#include "bosonconfig.h"

#include <ksimpleconfig.h>

#include <qimage.h>
#include <qvaluevector.h>
#include <qdir.h>

struct TextureGroundType
{
	QString mFile; // relative to groundTheme dir
	QRgb mMiniMapColor;
	int mAnimationDelay;
	QString mPixmapFile;
};


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
	QString mGroundThemeDir;
	QValueVector<TextureGroundType> mGroundTypes;
	QString mId;
};

BosonGroundTheme::BosonGroundTheme()
{
 d = new BosonGroundThemePrivate;
 mTextures.setAutoDelete(true);
 mPixmaps.setAutoDelete(true);
}

BosonGroundTheme::~BosonGroundTheme()
{
 mTextures.clear();
 mPixmaps.clear();
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
	BoVector3Float color = BosonConfig::readBoVector3FloatEntry(&conf, "MiniMapColor");
	types[i].mMiniMapColor = qRgb((int)color.x(), (int)color.y(), (int)color.z());
	types[i].mAnimationDelay = conf.readUnsignedNumEntry("AnimationDelay", 1);
	types[i].mPixmapFile = conf.readEntry("Pixmap", QString::null);
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
 mTextures.clear();

 for (unsigned int i = 0; i < textureCount(); i++) {
	loadTextureImages(dir, i);
 }

 d->mGroundThemeDir = dir;
 return true;
}

int BosonGroundTheme::textureAnimationDelay(unsigned int texture) const
{
 if (texture >= textureCount()) {
	return 1;
 }
 return d->mGroundTypes[texture].mAnimationDelay;
}

QRgb BosonGroundTheme::miniMapColor(unsigned int texture) const
{
 if (texture >= textureCount()) {
	return 0;
 }
 return d->mGroundTypes[texture].mMiniMapColor;
}

QString BosonGroundTheme::textureFileName(unsigned int texture) const
{
 if (texture >= textureCount()) {
	return QString::null;
 }
 return d->mGroundTypes[texture].mFile;
}

QString BosonGroundTheme::texturePixmapFileName(unsigned int texture) const
{
 if (texture >= textureCount()) {
	return QString::null;
 }
 return d->mGroundTypes[texture].mPixmapFile;
}

unsigned int BosonGroundTheme::textureCount() const
{
 return d->mGroundTypes.count();
}

void BosonGroundTheme::loadTextureImages(const QString& dir, unsigned int texture)
{
 QDir d(dir);
 // Find all name*.png and name*.jpg files
 QString name = textureFileName(texture);
 QStringList files = d.entryList(name + "*.png " + name + "*.jpg", QDir::Files, QDir::Name);
 QStringList absFiles;
 for (QStringList::Iterator it = files.begin(); it != files.end(); it++) {
	absFiles.append(dir + "/" + *it);
 }
 if (absFiles.isEmpty()) {
	boError() << k_funcinfo << "No files found from " << dir << " for texture " << texture << " (" << name << ")" << endl;
	return;
 }
 BosonTextureArray* t = new BosonTextureArray(absFiles);
 mTextures.insert(texture, t);

 // Load pixmap (for editor)
 QString pixmapfile = texturePixmapFileName(texture);
 if (pixmapfile.isNull()) {
	// If no pixmap is given, we take the first available texture
	pixmapfile = files.first();
 }
 QPixmap tmppix;
 if (!tmppix.load(dir + "/" + pixmapfile)) {
	tmppix = QPixmap();
 }
 if (tmppix.isNull()) {
	boWarning() << k_funcinfo << "unable to load pixmap for texture " << texture << " from " << dir + "/" + pixmapfile << endl;
	tmppix = QPixmap(50, 50, 32);
	tmppix.fill(Qt::green);
 }
 // Final pixmap will be at most 50x50 pixels big
 int w = QMIN(tmppix.width(), 50);
 int h = QMIN(tmppix.height(), 50);
 QPixmap* pix = new QPixmap(w, h);
 bitBlt(pix, 0, 0, &tmppix, 0, 0, w, h);
 mPixmaps.insert(texture, pix);
}

const QString& BosonGroundTheme::identifier() const
{
 return d->mId;
}

QPixmap BosonGroundTheme::pixmap(unsigned int texture)
{
 return *mPixmaps[texture];
}

BosonTextureArray* BosonGroundTheme::textures(int i) const
{
 return mTextures[i];
}
