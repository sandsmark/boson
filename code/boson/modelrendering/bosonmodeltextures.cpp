/*
    This file is part of the Boson game
    Copyright (C) 2002-2004 Andreas Beckermann (b_mann@gmx.de)

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

#include "bosonmodeltextures.h"

#include "../../bomemory/bodummymemory.h"
#include "../bosonconfig.h"
#include "bodebug.h"
#include "../botexture.h"
#include "../bosonprofiling.h"

#include <qimage.h>
#include <qgl.h>
#include <qfile.h>

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kstaticdeleter.h>

BosonModelTextures* BosonModelTextures::mModelTextures = 0;
static KStaticDeleter<BosonModelTextures> sd;

class BosonModelTextures::BosonModelTexturesPrivate
{
public:
	BosonModelTexturesPrivate()
	{
	}
	QMap<QString, BoTexture*> mName2Texture;
	QMap<BoTexture*, QValueList<BosonModel*> > mModels;

	QString mAdditionalTexturePath;
};

BosonModelTextures::BosonModelTextures()
{
 init();
}

void BosonModelTextures::createStatic()
{
 if (mModelTextures) {
	return;
 }
 mModelTextures = new BosonModelTextures();
 sd.setObject(mModelTextures);
}

BosonModelTextures* BosonModelTextures::modelTextures()
{
 if (!mModelTextures) {
	createStatic();
 }
 return mModelTextures;
}

void BosonModelTextures::init()
{
 d = new BosonModelTexturesPrivate;
}

BosonModelTextures::~BosonModelTextures()
{
 boDebug(110) << k_funcinfo << endl;
 if (d->mName2Texture.count() != 0) {
	boWarning(110) << k_funcinfo << "Not all textures have been deleted yet!!" << endl;
 }
 delete d;
 boDebug(110) << k_funcinfo << "done" << endl;
}

BoTexture* BosonModelTextures::insert(BosonModel* model, const QString& textureName)
{
 BosonProfiler prof("BosonModelTextures::insert");
 if (!model) {
	boError(110) << k_funcinfo << "NULL model" << endl;
	return 0;
 }
 BoTexture* tex = 0;
 if (!d->mName2Texture.contains(textureName)) {
	BosonProfiler prof("BosonModelTextures::insert: load texture");
	QString textureAbsPath = d->mAdditionalTexturePath + textureName;
	if (d->mAdditionalTexturePath.isEmpty() || !QFile::exists(textureAbsPath)) {
		textureAbsPath = locate("data", "boson/themes/textures/" + textureName);
	}
	tex = new BoTexture(textureAbsPath, BoTexture::Model);
	d->mName2Texture.insert(textureName, tex);
 } else {
	tex = texture(textureName);
 }
 d->mModels[tex].append(model);
 return tex;
}

void BosonModelTextures::removeModel(BosonModel* model)
{
 if (!model) {
	boError(110) << k_funcinfo << "NULL model" << endl;
	return;
 }
 QValueList<BoTexture*> remove;
 QMap<BoTexture*, QValueList<BosonModel*> >::Iterator it(d->mModels.begin());
 for (; it != d->mModels.end(); ++it) {
	(*it).remove(model);
	if ((*it).count() == 0) {
		// all model referencing this texture are gone. delete the
		// texture.
		// do not remove directly here, because the iterator might get
		// influenced
		remove.append(it.key());
	}
 }
 for (unsigned int i = 0; i < remove.count(); i++) {
	removeTexture(remove[i]);
 }

}

void BosonModelTextures::removeTexture(BoTexture* tex)
{
 if (d->mModels[tex].count() != 0) {
	boError(110) << k_funcinfo << "There are still models referencing this texture! Deleting anyway..." << endl;
 }
 d->mModels.remove(tex);
 QMap<QString, BoTexture*>::Iterator it = d->mName2Texture.begin();
 for (; it != d->mName2Texture.end() && it.data() != tex; ++it) {
	// nothing else to do
 }
 if (it != d->mName2Texture.end()) {
	d->mName2Texture.remove(it);
 }
 delete tex;
 boDebug(110) << k_funcinfo << "texture has been deleted" << endl;
}

BoTexture* BosonModelTextures::texture(const QString& texName) const
{
 if (!d->mName2Texture.contains(texName)) {
	return 0;
 }
 return d->mName2Texture[texName];
}

void BosonModelTextures::setAdditionalTexturePath(const QString& dir)
{
 d->mAdditionalTexturePath = dir;
}

const QString& BosonModelTextures::additionalTexturePath() const
{
 return d->mAdditionalTexturePath;
}
