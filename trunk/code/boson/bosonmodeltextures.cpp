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

#include "bosonmodeltextures.h"

#include "bosontexturearray.h"
#include "bosonconfig.h"
#include "bodebug.h"

#include <qimage.h>
#include <qgl.h>

#include <kglobal.h>
#include <kstandarddirs.h>

class BosonModelTextures::BosonModelTexturesPrivate
{
public:
	BosonModelTexturesPrivate()
	{
	}
	QMap<QString, GLuint> mName2Texture;
	QMap<GLuint, QValueList<BosonModel*> > mModels;
};

BosonModelTextures::BosonModelTextures()
{
 init();
}

void BosonModelTextures::init()
{
 // we cache the texture path. only a *very* slightly speedup and that only on
 // startup, but since this class has only a single object the extra variable
 // doesn't matter at all.
 mTexturePath = KGlobal::dirs()->findResourceDir("data", "boson/themes/textures/concrt1.jpg"); // FIXME: we depend on existence of this texture!
 mTexturePath += QString::fromLatin1("boson/themes/textures/");
 d = new BosonModelTexturesPrivate;
}

BosonModelTextures::~BosonModelTextures()
{
 // should not do anything. all models should get deleted first and when all
 // models are removed all textures should have been deleted, too
 boDebug(110) << k_funcinfo << endl;
 if (d->mName2Texture.count() != 0) {
	boWarning(110) << k_funcinfo << "Not all textures have been deleted yet!!" << endl;
 }
 delete d;
 boDebug(110) << k_funcinfo << "done" << endl;
}

void BosonModelTextures::insert(BosonModel* model, const QString& textureName)
{
 if (!model) {
	boError(110) << k_funcinfo << "NULL model" << endl;
	return;
 }
 GLuint tex = 0;
 if (!d->mName2Texture.contains(textureName)) {
	glGenTextures(1, &tex);
	loadTexture(textureName, tex);
	d->mName2Texture.insert(textureName, tex);
 } else {
	tex = texture(textureName);
 }
 d->mModels[tex].append(model);
}

void BosonModelTextures::loadTexture(const QString& textureName, GLuint tex)
{
 QImage image(texturePath() + textureName);
 if (image.isNull()) {
	boError(110) << k_funcinfo << "Could not load " << textureName << " from " << texturePath() << endl;
	image = QImage(64, 64, 32);
	image.fill(Qt::red.rgb());
 }
 BosonTextureArray::createTexture(image, tex, boConfig->modelTexturesMipmaps());
}

void BosonModelTextures::removeModel(BosonModel* model)
{
 if (!model) {
	boError(110) << k_funcinfo << "NULL model" << endl;
	return;
 }
 QValueList<GLuint> remove;
 QMap<GLuint, QValueList<BosonModel*> >::Iterator it(d->mModels.begin());
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

void BosonModelTextures::removeTexture(GLuint tex)
{
 if (d->mModels[tex].count() != 0) {
	boError(110) << k_funcinfo << "There are still models referencing this texture! Deleting anyway..." << endl;
 }
 d->mModels.remove(tex);
 QMap<QString, GLuint>::Iterator it = d->mName2Texture.begin();
 for (; it != d->mName2Texture.end() && it.data() != tex; ++it) {
	// nothing else to do
 }
 d->mName2Texture.remove(it);
 glDeleteTextures(1, &tex);
 boDebug(110) << k_funcinfo << tex << " has been deleted" << endl;
}

GLuint BosonModelTextures::texture(const QString& texName) const
{
 if (!d->mName2Texture.contains(texName)) {
	return 0;
 }
 return d->mName2Texture[texName];
}

void BosonModelTextures::reloadTextures()
{
 QMap<QString, GLuint>::Iterator it = d->mName2Texture.begin();
 for (; it != d->mName2Texture.end(); ++it) {
	QString textureName = it.key();
	GLuint texture = it.data();
	// note: the texture must be deleted, but the newly generated texture
	// mast have the *same* number as before!
	glDeleteTextures(1, &texture);
	loadTexture(textureName, texture);
 }
}

