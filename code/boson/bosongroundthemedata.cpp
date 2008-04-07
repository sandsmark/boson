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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#include "bosongroundthemedata.h"

#include "../bomemory/bodummymemory.h"
#include "gameengine/bosongroundtheme.h"
#include "gameengine/bosonmap.h"
#include "bodebug.h"
#include "defines.h"
#include "bosondata.h"
#include "bo3dtools.h"
#include "bosonconfig.h"
#include "botexture.h"
#include "boshader.h"
#include "bogl.h"
#include "bosonviewdata.h"
#include "bosonprofiling.h"

#include <ksimpleconfig.h>

#include <qimage.h>
#include <q3valuevector.h>
#include <qdir.h>
//Added by qt3to4:
#include <QPixmap>
#include <Q3PtrList>


BosonGroundTypeData::BosonGroundTypeData()
{
 textures = 0;
 bumpTextures = 0;
 shader = 0;
 icon = 0;
 groundType = 0;
}

BosonGroundTypeData::~BosonGroundTypeData()
{
 delete textures;
 delete bumpTextures;
 // Shader will NOT be deleted here (it's shared between BosonGroundType objects)
 delete icon;
}

BoTexture* BosonGroundTypeData::currentTexture(int advanceCallsCount) const
{
 BO_CHECK_NULL_RET0(textures);
 return textures->texture((advanceCallsCount / groundType->animationDelay) % textures->count());
}

BoTexture* BosonGroundTypeData::currentBumpTexture(int advanceCallsCount) const
{
 BO_CHECK_NULL_RET0(bumpTextures);
 return bumpTextures->texture((advanceCallsCount / groundType->animationDelay) % bumpTextures->count());
}


class BosonGroundThemeDataPrivate
{
public:
	BosonGroundThemeDataPrivate()
	{
		mGroundTheme = 0;
	}
	const BosonGroundTheme* mGroundTheme;
	Q3IntDict<BosonGroundTypeData> mGroundTypes;
	Q3Dict<BoTexture> mBumpTextures;
	Q3Dict<BoShader> mShaders;
};

BosonGroundThemeData::BosonGroundThemeData()
{
 d = new BosonGroundThemeDataPrivate;
 d->mGroundTypes.setAutoDelete(true);
 d->mBumpTextures.setAutoDelete(true);
 d->mShaders.setAutoDelete(true);
}

BosonGroundThemeData::~BosonGroundThemeData()
{
 d->mGroundTypes.setAutoDelete(true);
 d->mGroundTypes.clear();
 d->mBumpTextures.setAutoDelete(true);
 d->mBumpTextures.clear();
 d->mShaders.setAutoDelete(true);
 d->mShaders.clear();
 delete d;
}

bool BosonGroundThemeData::loadGroundTheme(const BosonGroundTheme* theme)
{
 if (!theme) {
	BO_NULL_ERROR(theme);
	return false;
 }
 if (theme->groundTypeCount() == 0) {
	boWarning() << k_funcinfo << "no textures available to be loaded in this groundTheme. probably failed at loading the index.ground file!" << endl;
	return false;
 }

 for (unsigned int i = 0; i < theme->groundTypeCount(); i++) {
	BosonGroundTypeData* data = new BosonGroundTypeData();
	data->groundType = theme->groundType(i);
	if (!data->groundType) {
		BO_NULL_ERROR(data->groundType);
		delete data;
		return false;
	}
	d->mGroundTypes.insert(i, data);
 }

 for (unsigned int i = 0; i < theme->groundTypeCount(); i++) {
	if (!loadTextures(theme->themeDirectory(), i)) {
		boError() << k_funcinfo << "loading texture of groundtype " << i << " failed" << endl;
		return false;
	}
 }

 d->mGroundTheme = theme;
 return true;
}

const BosonGroundType* BosonGroundThemeData::groundType(unsigned int i) const
{
 BO_CHECK_NULL_RET0(d->mGroundTheme);
 return d->mGroundTheme->groundType(i);
}

const QString& BosonGroundThemeData::themeDirectory() const
{
 return d->mGroundTheme->themeDirectory();
}

BosonGroundTypeData* BosonGroundThemeData::groundTypeData(unsigned int i) const
{
 return d->mGroundTypes[i];
}

unsigned int BosonGroundThemeData::groundTypeCount() const
{
 return d->mGroundTheme->groundTypeCount();
}

const QString& BosonGroundThemeData::identifier() const
{
 return d->mGroundTheme->identifier();
}

bool BosonGroundThemeData::loadTextures(const QString& dir, unsigned int i)
{
 BosonProfiler prof("BosonGroundThemeData::loadTextures()");
 QDir d(dir);
 // Find all name*.png and name*.jpg files
 BosonGroundTypeData* groundData = groundTypeData(i);
 if (!groundData) {
	BO_NULL_ERROR(groundTypeData);
	return false;
 }
 QString name = groundData->groundType->textureFile;
 QStringList files = d.entryList(name + "*.png " + name + "*.jpg", QDir::Files, QDir::Name);
 QStringList absFiles;
 for (QStringList::Iterator it = files.begin(); it != files.end(); it++) {
	absFiles.append(dir + "/" + *it);
 }
 if (absFiles.isEmpty()) {
	boError() << k_funcinfo << "No textures found from " << dir << " for ground type " << i << " (" << name << ")" << endl;
	return false;
 }
 groundData->textures = new BoTextureArray(absFiles, BoTexture::Terrain);

 if (boConfig->boolValue("UseGroundShaders")) {
	loadShaders(dir, groundData);
 }


 // Load pixmap (for editor)
 QString iconFile = groundData->groundType->iconFile;
 if (iconFile.isNull()) {
	// If no pixmap is given, we take the first available texture
	iconFile = files.first();
 }
 QPixmap tmppix;
 if (!tmppix.load(dir + "/" + iconFile)) {
	tmppix = QPixmap();
 }
 if (tmppix.isNull()) {
	boWarning() << k_funcinfo << "unable to load pixmap for ground type " << groundData->groundType->name << " from " << dir + "/" + iconFile << endl;
	tmppix = QPixmap(50, 50);
	tmppix.fill(Qt::green);
 }
 // Final pixmap will be at most 50x50 pixels big
 int w = qMin(tmppix.width(), 50);
 int h = qMin(tmppix.height(), 50);
 groundData->icon = new QPixmap(w, h);
 bitBlt(groundData->icon, 0, 0, &tmppix, 0, 0, w, h);
 return true;
}

void BosonGroundThemeData::loadShaders(const QString& dirName, BosonGroundTypeData* ground)
{
 if (ground->shader) {
	return;
 }
 BosonProfiler prof("BosonGroundThemeData::loadShaders()");

 // Load bumpmap textures
 QDir dir(dirName);
 QString name = ground->groundType->bumpTextureFile;
 QStringList files = dir.entryList(name + "*.png " + name + "*.jpg", QDir::Files, QDir::Name);
 Q3PtrList<BoTexture> bumpTextures;
 for (QStringList::Iterator it = files.begin(); it != files.end(); it++) {
	QString file = dirName + "/" + *it;
	BoTexture* tex = d->mBumpTextures[file];
	if (!tex) {
		tex = new BoTexture(file, BoTexture::NormalMap);
		d->mBumpTextures.insert(file, tex);
	}
	bumpTextures.append(tex);
 }
 if (!bumpTextures.isEmpty()) {
	ground->bumpTextures = new BoTextureArray(bumpTextures);
 }

 // Load shader
 ground->shader = d->mShaders[ground->groundType->shaderFile];
 if (!ground->shader) {
	ground->shader = new BoShader(ground->groundType->shaderFile);
	d->mShaders.insert(ground->groundType->shaderFile, ground->shader);
 }
}


bool BosonGroundThemeData::shadersSupported()
{
 QStringList extensions = boglGetOpenGLExtensions();
 return (extensions.contains("GL_ARB_shader_objects") &&
		extensions.contains("GL_ARB_fragment_shader") &&
		(boTextureManager->textureUnits() >= 3));
}

void BosonGroundThemeData::setUseGroundShaders(bool use)
{
 if (!use) {
	// Maybe delete shaders and bumpmaps here?
	return;
 }

 QStringList themelist = boData->availableGroundThemes();
 for (QStringList::Iterator it = themelist.begin(); it != themelist.end(); it++) {
	const BosonGroundTheme* theme = boData->groundTheme(*it);
	BosonGroundThemeData* data = boViewData->groundThemeData(theme);
	if (!data) {
		continue;
	}
	if (data->themeDirectory().isNull()) {
		continue;
	}
	for (unsigned int i = 0; i < data->groundTypeCount(); i++) {
		data->loadShaders(data->themeDirectory(), data->groundTypeData(i));
	}
 }
}

