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
#ifndef BOSONMODELTEXTURES_H
#define BOSONMODELTEXTURES_H

#include <bogl.h>

class QImage;
class QString;
class BosonModel;
class BoTexture;

/**
 * This class stores <em>all</em> textures for <em>all</em> models. Simply call
 * @ref insert once before using a texture in a model. From then on the model is
 * registered to use that texture. Once all models that use a texture have been
 * deleted the texture gets freed, too.
 * @author Andreas Beckermann <b_mann@gmx.de>
 * @short Static helper class for @ref BosonModel.
 **/
class BosonModelTextures
{
public:
	~BosonModelTextures();

	static BosonModelTextures* modelTextures();

	/**
	 * Load a texture. A texture gets loaded only once, no matter how many
	 * models may use it. All models share the same texture object.
	 *
	 * BosonModelTextures maintains a list of @ref BosonModel objects that
	 * have loaded a specific texture. Once all objects have been deleted
	 * the texture is freed.
	 * @param model The @ref BosonModel object that asks for the texture
	 * @param textureName the filename of the texture
	 **/
	BoTexture* insert(BosonModel* model, const QString& textureName);

	/**
	 * Call this in the destructor of @ref BosonModel. The model gets
	 * unregistered then and all textures get deleted, if no other model
	 * uses them.
	 **/
	void removeModel(BosonModel* model);

	/**
	 * @return The texture object assigned to the name textureName
	 **/
	BoTexture* texture(const QString& textureName) const;

	/**
	 * @return The additional texture path which is used in addition to standard
	 * kde dirs
	 **/
	const QString& additionalTexturePath() const;

	/**
	 * Set an additional texture path. This is used before looking into standard
	 * kde dirs when searching for textures
	 **/
	void setAdditionalTexturePath(const QString& dir);

  protected:
	/**
	 * Create the static object of this class (see @ref modelTextures). This
	 * is called in @ref modelTextures - no need for you to call it
	 * manually.
	 **/
	static void createStatic();

	void removeTexture(BoTexture* tex);

private:
	BosonModelTextures();
	void init();

private:
	class BosonModelTexturesPrivate;
	BosonModelTexturesPrivate* d;
	static BosonModelTextures* mModelTextures;
};

#endif

