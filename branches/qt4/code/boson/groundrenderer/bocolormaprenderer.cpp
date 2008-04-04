/*
    This file is part of the Boson game
    Copyright (C) 2005 Rivo Laks (rivolaks@hot.ee)

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

#include "bocolormaprenderer.h"

#include "../defines.h"
#include "../../bomemory/bodummymemory.h"
#include "../gameengine/bosonmap.h"
#include "../botexture.h"
#include "bodebug.h"

BoColorMapRenderer::BoColorMapRenderer(BoColorMap* map)
{
 mColorMap = map;

 mTexWidth = BoTexture::nextPower2(map->width());
 mTexHeight = BoTexture::nextPower2(map->height());

 int textureSize = mTexWidth * mTexHeight * 3;
 unsigned char* data = new unsigned char[textureSize];
 for (int i = 0; i < textureSize; i++) {
	data[i] = 0;
 }

 mTexture = new BoTexture(BoTexture::FilterNearest | BoTexture::FormatRGB | BoTexture::DontCompress);
 mTexture->load(data, mTexWidth, mTexHeight);
 delete[] data;

 update(true);
}

BoColorMapRenderer::~BoColorMapRenderer()
{
 delete mTexture;
}

void BoColorMapRenderer::update(bool force)
{
 if (!mColorMap->isDirty() && !force) {
	return;
 }
 QRect r = mColorMap->dirtyRect();
 if (force) {
	r = QRect(0, 0, mColorMap->width(), mColorMap->height());
 }

 mTexture->bind();
 unsigned char* data = new unsigned char[r.width() * r.height() * 3];
 int pos = 0;
 for (int y = r.y(); y < r.y() + r.height(); y++) {
	for (int x = r.x(); x < r.x() + r.width(); x++) {
		int index = (y * mColorMap->width() + x) * 3;
		data[pos + 0] = mColorMap->textureData()[index + 0];
		data[pos + 1] = mColorMap->textureData()[index + 1];
		data[pos + 2] = mColorMap->textureData()[index + 2];
		pos += 3;
	}
 }
 glTexSubImage2D(GL_TEXTURE_2D, 0, r.x(), r.y(), r.width(), r.height(), GL_RGB, GL_UNSIGNED_BYTE, data);
 delete[] data;

 mColorMap->setNotDirty();
}

void BoColorMapRenderer::start(const BosonMap*)
{
 BO_CHECK_NULL_RET(mTexture);
 mTexture->bind();
 // Use automatic texcoord generation to map the texture to cells
 const float texPlaneS[] = { 1.0, 0.0, 0.0, 0.0 };
 const float texPlaneT[] = { 0.0, 1.0, 0.0, 0.0 };
 glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
 glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
 glTexGenfv(GL_S, GL_OBJECT_PLANE, texPlaneS);
 glTexGenfv(GL_T, GL_OBJECT_PLANE, texPlaneT);
 glEnable(GL_TEXTURE_GEN_S);
 glEnable(GL_TEXTURE_GEN_T);
 glMatrixMode(GL_TEXTURE);
 glLoadIdentity();
 glScalef(1.0f / mTexWidth, 1.0f / mTexHeight, 1.0f);
 glScalef(1, -1, 1);
 glMatrixMode(GL_MODELVIEW);
}

void BoColorMapRenderer::stop()
{
 BO_CHECK_NULL_RET(mTexture);
 glMatrixMode(GL_TEXTURE);
 glLoadIdentity();
 glMatrixMode(GL_MODELVIEW);
 boTextureManager->unbindTexture();
 glDisable(GL_TEXTURE_GEN_S);
 glDisable(GL_TEXTURE_GEN_T);
}


