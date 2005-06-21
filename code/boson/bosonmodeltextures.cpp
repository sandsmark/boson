/*
    This file is part of the Boson game
    Copyright (C) 2002-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosonmodeltextures.h"

#include "../bomemory/bodummymemory.h"
#include "bosonconfig.h"
#include "bodebug.h"
#include "botexture.h"
#include "bosonprofiling.h"

#include <qimage.h>
#include <qgl.h>

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

	QString mTexturePath;
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
 // we cache the texture path. only a *very* slightly speedup and that only on
 // startup, but since this class has only a single object the extra variable
 // doesn't matter at all.
 d->mTexturePath = KGlobal::dirs()->findResourceDir("data", "boson/themes/textures/concrt1.jpg"); // FIXME: we depend on existence of this texture!
 d->mTexturePath += QString::fromLatin1("boson/themes/textures/");
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
	tex = new BoTexture(texturePath() + textureName, BoTexture::Model);
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

const QString& BosonModelTextures::texturePath() const
{
 return d->mTexturePath;
}
