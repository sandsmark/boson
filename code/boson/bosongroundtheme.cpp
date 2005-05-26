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

#include "bosonmap.h"
#include "bodebug.h"
#include "defines.h"
#include "bosondata.h"
#include "bo3dtools.h"
#include "bosonconfig.h"
#include "botexture.h"
#include "boshader.h"
#include "bogl.h"

#include <ksimpleconfig.h>

#include <qimage.h>
#include <qvaluevector.h>
#include <qdir.h>


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


BosonGroundType::BosonGroundType()
{
 id = -1;
 textures = 0;
 bumptextures = 0;
 shader = 0;
 icon = 0;
 animationDelay = 1;
}

BosonGroundType::~BosonGroundType()
{
 delete textures;
 delete bumptextures;
 // Shader will NOT be deleted here (it's shared between BosonGroundType objects)
 delete icon;
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
 mBumpTextures.setAutoDelete(true);
 mShaders.setAutoDelete(true);
}

BosonGroundTheme::~BosonGroundTheme()
{
 mGroundTypes.clear();
 mBumpTextures.clear();
 mShaders.clear();
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
	ground->id = i;
	ground->name = conf.readEntry("Name", group);
	ground->texturefile = conf.readEntry("Texture", QString::null);
	if (ground->texturefile.isEmpty()) {
		boError() << k_funcinfo << file << " Group=" << group << " has no or invalid Texture key" << endl;
		ret = false;
		delete ground;
		break;
	}
	ground->bumptexturefile = conf.readEntry("BumpTexture", "bump-null");
	ground->bumpscale = (float)(conf.readDoubleNumEntry("BumpScale", 0.04f));
	ground->bumpbias = (float)(conf.readDoubleNumEntry("BumpBias", 0.5f)) * ground->bumpscale;
	ground->texturesize = (float)(conf.readDoubleNumEntry("TextureSize", 5.0f));
	ground->shaderfile = conf.readEntry("Shader", "default.shader");
	BoVector3Float color = BosonConfig::readBoVector3FloatEntry(&conf, "MiniMapColor");
	ground->color = qRgb((int)color.x(), (int)color.y(), (int)color.z());
	ground->animationDelay = conf.readUnsignedNumEntry("AnimationDelay", 1);
	ground->iconfile = conf.readEntry("Pixmap", QString::null);

	mGroundTypes.insert(i, ground);
 }

 if (!ret) {
	boError() << k_funcinfo << "Could not load ground theme config file " << file << endl;
	mGroundTypes.clear();
	return ret;
 }

 d->mId = identifier;
 return ret;
}

// AB: maybe we can use QString::null here to unload a theme
bool BosonGroundTheme::loadGroundTheme(QString dir)
{
 if (groundTypeCount() == 0) {
	boWarning() << k_funcinfo << "no textures available to be loaded in this groundTheme. probably failed at loading the index.ground file!" << endl;
	return false;
 }
 // dir is e.g. /opt/kde3/share/apps/boson/themes/grounds/earth/ -> "earth" is
 // the important part!
 if (dir.right(1) != QString::fromLatin1("/")) {
	dir += QString::fromLatin1("/");
 }

 for (unsigned int i = 0; i < groundTypeCount(); i++) {
	loadTextures(dir, i);
 }

 d->mGroundThemeDir = dir;
 return true;
}

BosonGroundType* BosonGroundTheme::groundType(unsigned int i) const
{
 return mGroundTypes[i];
}

unsigned int BosonGroundTheme::groundTypeCount() const
{
 return mGroundTypes.count();
}

void BosonGroundTheme::loadTextures(const QString& dir, unsigned int i)
{
 QDir d(dir);
 // Find all name*.png and name*.jpg files
 BosonGroundType* ground = groundType(i);
 QString name = ground->texturefile;
 QStringList files = d.entryList(name + "*.png " + name + "*.jpg", QDir::Files, QDir::Name);
 QStringList absFiles;
 for (QStringList::Iterator it = files.begin(); it != files.end(); it++) {
	absFiles.append(dir + "/" + *it);
 }
 if (absFiles.isEmpty()) {
	boError() << k_funcinfo << "No textures found from " << dir << " for ground type " << i << " (" << name << ")" << endl;
	return;
 }
 ground->textures = new BoTextureArray(absFiles, BoTexture::Terrain);

 if (boConfig->boolValue("UseGroundShaders")) {
	loadShaders(dir, ground);
 }


 // Load pixmap (for editor)
 if (ground->iconfile.isNull()) {
	// If no pixmap is given, we take the first available texture
	ground->iconfile = files.first();
 }
 QPixmap tmppix;
 if (!tmppix.load(dir + "/" + ground->iconfile)) {
	tmppix = QPixmap();
 }
 if (tmppix.isNull()) {
	boWarning() << k_funcinfo << "unable to load pixmap for ground type " << ground->name << " from " << dir + "/" + ground->iconfile << endl;
	tmppix = QPixmap(50, 50, 32);
	tmppix.fill(Qt::green);
 }
 // Final pixmap will be at most 50x50 pixels big
 int w = QMIN(tmppix.width(), 50);
 int h = QMIN(tmppix.height(), 50);
 ground->icon = new QPixmap(w, h);
 bitBlt(ground->icon, 0, 0, &tmppix, 0, 0, w, h);
}

void BosonGroundTheme::loadShaders(const QString& dir, BosonGroundType* ground)
{
 if (ground->shader) {
	return;
 }

 // Load bumpmap textures
 QDir d(dir);
 QString name = ground->bumptexturefile;
 QStringList files = d.entryList(name + "*.png " + name + "*.jpg", QDir::Files, QDir::Name);
 QPtrList<BoTexture> bumptextures;
 for (QStringList::Iterator it = files.begin(); it != files.end(); it++) {
	QString file = dir + "/" + *it;
	BoTexture* tex = mBumpTextures[file];
	if (!tex) {
		tex = new BoTexture(file, BoTexture::NormalMap);
		mBumpTextures.insert(file, tex);
	}
	bumptextures.append(tex);
 }
 if (!bumptextures.isEmpty()) {
	ground->bumptextures = new BoTextureArray(bumptextures);
 }

 // Load shader
 QString shaderfile = dir + "/" + ground->shaderfile;
 ground->shader = mShaders[shaderfile];
 if (!ground->shader) {
	ground->shader = new BoShader(shaderfile);
	mShaders.insert(shaderfile, ground->shader);
 }
}

const QString& BosonGroundTheme::identifier() const
{
 return d->mId;
}

const QString& BosonGroundTheme::themeDirectory() const
{
 return d->mGroundThemeDir;
}

bool BosonGroundTheme::shadersSupported()
{
  QStringList extensions = boglGetOpenGLExtensions();
  return (extensions.contains("GL_ARB_shader_objects") &&
      extensions.contains("GL_ARB_fragment_shader") && (boTextureManager->textureUnits() >= 3));
}

void BosonGroundTheme::setUseGroundShaders(bool use)
{
 if (!use) {
	// Maybe delete shaders and bumpmaps here?
	return;
 }

 QStringList themelist = boData->availableGroundThemes();
 for (QStringList::Iterator it = themelist.begin(); it != themelist.end(); it++) {
	BosonGroundTheme* theme = boData->groundTheme(*it);
	if (theme->themeDirectory().isNull()) {
		continue;
	}
	for (unsigned int i = 0; i < theme->groundTypeCount(); i++) {
		theme->loadShaders(theme->themeDirectory(), theme->groundType(i));
	}
 }
}

