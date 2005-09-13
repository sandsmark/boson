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

#include "bodefaultgroundrenderer.h"
#include "bodefaultgroundrenderer.moc"

#include "../../bomemory/bodummymemory.h"
#include "bogroundrendererbase.h"
#include "../bosonmap.h"
#include "../bosonconfig.h"
#include "../defines.h"
#include "../bosongroundtheme.h"
#include "../bosongroundthemedata.h"
#include "../bomaterial.h"
#include "../boson.h"
#include "../botexture.h"
#include "../boshader.h"
#include "../bosonprofiling.h"
#include "bocolormaprenderer.h"
#include <bogl.h>
#include <bodebug.h>

#include <math.h>

#define USE_VBOS 0


BoDefaultGroundRenderer::BoDefaultGroundRenderer()
	: BoGroundRendererBase()
{
 mCurrentMap = 0;
 mVBOVertex = 0;
 mVBONormal = 0;
 mVBOColor = 0;
}

BoDefaultGroundRenderer::~BoDefaultGroundRenderer()
{
 boDebug() << k_funcinfo << endl;
 clearVBOs();
}

void BoDefaultGroundRenderer::clearVBOs()
{
#if USE_VBOS
 boglDeleteBuffers(1, &mVBOVertex);
 boglDeleteBuffers(1, &mVBONormal);
 boglDeleteBuffers(1, &mVBOColor);
#endif
}

bool BoDefaultGroundRenderer::initGroundRenderer()
{
 if (!BoGroundRendererBase::initGroundRenderer()) {
	return false;
 }

 return true;
}

void BoDefaultGroundRenderer::renderVisibleCells(int* renderCells, unsigned int cellsCount, const BosonMap* map)
{
 BO_CHECK_NULL_RET(renderCells);
 BO_CHECK_NULL_RET(map);
 BO_CHECK_NULL_RET(map->texMap());
 BO_CHECK_NULL_RET(mVertexArray);
 BO_CHECK_NULL_RET(map->normalMap());
 BO_CHECK_NULL_RET(map->groundTheme());
 BO_CHECK_NULL_RET(currentGroundThemeData());

 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "before method" << endl;
 }
 BosonGroundTheme* groundTheme = map->groundTheme();

 glEnableClientState(GL_VERTEX_ARRAY);
 glEnableClientState(GL_NORMAL_ARRAY);
 glEnableClientState(GL_COLOR_ARRAY);

#if USE_VBOS
 boglBindBuffer(GL_ARRAY_BUFFER, mVBOVertex);
 glVertexPointer(3, GL_FLOAT, 0, (void*)0);

 boglBindBuffer(GL_ARRAY_BUFFER, mVBONormal);
 glNormalPointer(GL_FLOAT, 0, (void*)0);

 boglBindBuffer(GL_ARRAY_BUFFER, mVBOColor);
#else
 glVertexPointer(3, GL_FLOAT, 0, mVertexArray);
 glNormalPointer(GL_FLOAT, 0, map->normalMap());
#endif
 // AB: we use textureCount different color pointers, so the glColorPointer()
 // call comes later.
 // (the color VBO remains bound)

 glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
 glEnable(GL_COLOR_MATERIAL);

 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

 // we draw the cells in different stages. depth test is now enabled in all
 //  stages to prevent drawing errors. Depth func GL_LEQUAL makes sure all
 //  layers get rendered (they have same z values)
 // Maybe it should be set back to GL_LESS later?
 glDepthFunc(GL_LEQUAL);

 // Texture coordinates will be autmatically generated by OpenGL
 float texPlaneS[] = { 1.0, 0.0, 0.0, 0.0 };
 float texPlaneT[] = { 0.0, 1.0, 0.0, 0.0 };
 glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
 glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
 glEnable(GL_TEXTURE_GEN_S);
 glEnable(GL_TEXTURE_GEN_T);
 glTexGenfv(GL_S, GL_OBJECT_PLANE, texPlaneS);
 glTexGenfv(GL_T, GL_OBJECT_PLANE, texPlaneT);
 glMatrixMode(GL_TEXTURE);

 bool useShaders = boConfig->boolValue("UseGroundShaders");

 unsigned int* indices = new unsigned int[cellsCount * 4];
 for (unsigned int i = 0; i < cellsCount; i++) {
	int x;
	int y;
	int w;
	int h;
	BoGroundRenderer::getCell(renderCells, i, &x, &y, &w, &h);
	indices[i * 4 + 0] = map->cornerArrayPos(x, y);
	indices[i * 4 + 1] = map->cornerArrayPos(x, y + h);
	indices[i * 4 + 2] = map->cornerArrayPos(x + w, y + h);
	indices[i * 4 + 3] = map->cornerArrayPos(x + w, y);
 }

 unsigned int usedTextures = 0;
 unsigned int renderedQuads = 0;
 for (unsigned int i = 0; i < groundTheme->groundTypeCount(); i++) {
	if (i == 1) {
		glEnable(GL_BLEND);
	}
	BosonGroundTypeData* groundData = currentGroundThemeData()->groundTypeData(i);
	// Bind texture
	BoTexture* tex = groundData->currentTexture(boGame->advanceCallsCount());
	tex->bind();
	// Set up texture coordinate generation
	glLoadIdentity();
	glScalef(1.0f / groundData->groundType->textureSize, 1.0f / groundData->groundType->textureSize, 1.0);
	if (useShaders) {
		// Bind bump tex
		boTextureManager->activateTextureUnit(2);
		BoTexture* bumptex = groundData->currentBumpTexture(boGame->advanceCallsCount());
		bumptex->bind();
		glLoadIdentity();
		glScalef(1.0f / groundData->groundType->textureSize, 1.0f / groundData->groundType->textureSize, 1.0);
		boTextureManager->activateTextureUnit(0);
		// Shader
		groundData->shader->bind();
		groundData->shader->setUniform("bumpScale", groundData->groundType->bumpScale);
		groundData->shader->setUniform("bumpBias", groundData->groundType->bumpBias);
	}
	unsigned char* colorPointer = mColorArray + (map->cornerArrayPos(map->width(), map->height()) + 1) * 4 * i;
	bool useTexture = false;
	for (unsigned int j = 0; j < cellsCount * 4; j++) {
		if (colorPointer[indices[j] * 4 + 3] != 0) {
			useTexture = true;
			break;
		}
	}
	if (!useTexture) {
		continue;
	}
	usedTextures++;

#if USE_VBOS
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, (unsigned char*)0 + (map->cornerArrayPos(map->width(), map->height()) + 1) * 4 * i);
#else
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, colorPointer);
#endif
	glDrawElements(GL_QUADS, cellsCount * 4, GL_UNSIGNED_INT, indices);

	renderedQuads += cellsCount;
 }
 delete[] indices;
 statistics()->setRenderedQuads(renderedQuads);
 statistics()->setUsedTextures(usedTextures);
 if (useShaders) {
	boTextureManager->activateTextureUnit(2);
	glLoadIdentity();
	boTextureManager->disableTexturing();
	boTextureManager->activateTextureUnit(0);
	BoShader::unbind();
 }
 glLoadIdentity();
 glMatrixMode(GL_MODELVIEW);

 boglBindBuffer(GL_ARRAY_BUFFER, 0);

 glDisableClientState(GL_VERTEX_ARRAY);
 glDisableClientState(GL_NORMAL_ARRAY);
 glDisableClientState(GL_COLOR_ARRAY);

#warning FIXME: does NOT belong to default renderer. belongs to base class.
 if (map->activeColorMap()) {
	BoColorMapRenderer* renderer = getUpdatedColorMapRenderer(map->activeColorMap());
	if (renderer) {
		boTextureManager->disableTexturing();
		glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_LIGHTING);
		renderer->start(map);
		renderCellColors(renderCells, cellsCount, map);
		renderer->stop();
		glPopAttrib();
	}
 }

 glDisable(GL_TEXTURE_GEN_S);
 glDisable(GL_TEXTURE_GEN_T);
 glDisable(GL_BLEND);
 glColor4ub(255, 255, 255, 255);

 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "at end of method" << endl;
 }
}

void BoDefaultGroundRenderer::renderCellColors(int* cells, int count, const BosonMap* map)
{
 const unsigned char alpha = 128;

 glColor4ub(255, 255, 255, alpha);

 glEnableClientState(GL_VERTEX_ARRAY);

 glTranslatef(0.0f, 0.0f, 0.05f);
 glBegin(GL_QUADS);

 for (int i = 0; i < count; i++) {
	int x;
	int y;
	int w;
	int h;
	BoGroundRenderer::getCell(cells, i, &x, &y, &w, &h);

	glArrayElement(map->cornerArrayPos(x, y));
	glArrayElement(map->cornerArrayPos(x, y+h));
	glArrayElement(map->cornerArrayPos(x+w, y+h));
	glArrayElement(map->cornerArrayPos(x+w, y));
 }
 glEnd();
 glTranslatef(0.0f, 0.0f, -0.05f);
 glDisableClientState(GL_VERTEX_ARRAY);
}

// AB: we should place the VBO objects into the base class?!
void BoDefaultGroundRenderer::updateMapCache(const BosonMap* map)
{
 bool newMap = false;
 if (mCurrentMap != map) {
	newMap = true;
 }
 BoGroundRendererBase::updateMapCache(map);
 mCurrentMap = map;
 if (!newMap) {
	return;
 }
 clearVBOs();
 if (!map) {
	return;
 }
 boDebug() << k_funcinfo << map->width() << " " << map->height() << endl;
#if USE_VBOS
 boglGenBuffers(1, &mVBOVertex);
 boglGenBuffers(1, &mVBONormal);
 boglGenBuffers(1, &mVBOColor);

 boDebug() << k_funcinfo << endl;

 // apply data to the vertex VBO
 updateVertexVBO();

 // apply data to the color VBO (alpha values from the texmaps)
 updateColorVBO();

 // apply data to the normal VBO
 int vertexCount = mCurrentMap->cornerArrayPos(mCurrentMap->width(), mCurrentMap->height()) + 1;
 boglBindBuffer(GL_ARRAY_BUFFER, mVBONormal);
 boglBufferData(GL_ARRAY_BUFFER, vertexCount * 3 * sizeof(float), map->normalMap(), GL_STATIC_DRAW);


#endif
}

void BoDefaultGroundRenderer::cellHeightChanged(int x1, int y1, int x2, int y2)
{
 BoGroundRendererBase::cellHeightChanged(x1, y1, x2, y2);
 updateVertexVBO();
}

void BoDefaultGroundRenderer::updateVertexVBO()
{
#if USE_VBOS
 if (mVBOVertex == 0) {
	return;
 }
 boDebug() << k_funcinfo << endl;
 BO_CHECK_NULL_RET(mCurrentMap);
 int vertexCount = mCurrentMap->cornerArrayPos(mCurrentMap->width(), mCurrentMap->height()) + 1;
 boglBindBuffer(GL_ARRAY_BUFFER, mVBOVertex);
 // AB: could probably be done more efficiently
 boglBufferData(GL_ARRAY_BUFFER, vertexCount * 3 * sizeof(float), mVertexArray, GL_STATIC_DRAW);
#endif
}

void BoDefaultGroundRenderer::cellTextureChanged(int x1, int y1, int x2, int y2)
{
 BoGroundRendererBase::cellTextureChanged(x1, y1, x2, y2);
 updateColorVBO();
}

// TODO: make more efficient: only values in a given rect changed
// -> however this would speed up certain tasks in editor only, so this is of
// _very_ little importance
void BoDefaultGroundRenderer::updateColorVBO()
{
#if USE_VBOS
 if (mVBOColor == 0) {
	return;
 }
 boDebug() << k_funcinfo << endl;
 BO_CHECK_NULL_RET(mCurrentMap);
 int vertexCount = mCurrentMap->cornerArrayPos(mCurrentMap->width(), mCurrentMap->height()) + 1;
 boglBindBuffer(GL_ARRAY_BUFFER, mVBOColor);
 // AB: could probably be done more efficiently
 boglBufferData(GL_ARRAY_BUFFER, vertexCount * mCurrentMap->groundTheme()->groundTypeCount() * 4 * sizeof(unsigned char), mColorArray, GL_STATIC_DRAW);
#endif
}

