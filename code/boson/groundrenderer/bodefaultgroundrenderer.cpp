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
#include "../gameengine/bosonmap.h"
#include "../bosonconfig.h"
#include "../defines.h"
#include "../gameengine/bosongroundtheme.h"
#include "../bosongroundthemedata.h"
#include "../gameengine/boson.h"
#include "../botexture.h"
#include "../boshader.h"
#include "../bosonprofiling.h"
#include "bocolormaprenderer.h"
#include <bogl.h>
#include <bodebug.h>

#include <math.h>

// VBOs or plain vertex arrays
#define USE_VBOS 0
// quads or triangle strips
// WARNING: atm triangle strips use the same indices, i.e. the same strips for
//          all textures!
//          -> slower
#define USE_QUADS 1


BoDefaultGroundRenderer::BoDefaultGroundRenderer()
	: BoGroundRendererBase()
{
 mCurrentMap = 0;
 mVBOVertex = 0;
 mVBONormal = 0;
 mVBOColor = 0;
 mIndicesArray = 0;
 mIndicesArraySize = 0;
 mIndicesCount = 0;
 mIndicesDirty = true;
}

BoDefaultGroundRenderer::~BoDefaultGroundRenderer()
{
 boDebug() << k_funcinfo << endl;
 clearVBOs();
 delete[] mIndicesArray;

 boDebug() << k_funcinfo << mTextureIndices.count() << endl;
 for (unsigned int i = 0; i < mTextureIndices.count(); i++) {
	delete mTextureIndices[i];
 }
 mTextureIndices.clear();
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

void BoDefaultGroundRenderer::renderVisibleCells(int* renderCells, unsigned int cellsCount, const BosonMap* map, RenderFlags flags)
{
 BO_CHECK_NULL_RET(renderCells);
 BO_CHECK_NULL_RET(map);
 BO_CHECK_NULL_RET(map->texMap());
 BO_CHECK_NULL_RET(mVertexArray);
 BO_CHECK_NULL_RET(map->normalMap());
 BO_CHECK_NULL_RET(map->groundTheme());
 BO_CHECK_NULL_RET(currentGroundThemeData());

 glPushAttrib(GL_ALL_ATTRIB_BITS);
 glPushClientAttrib(GL_ALL_ATTRIB_BITS);


 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "before method" << endl;
 }
 BosonGroundTheme* groundTheme = map->groundTheme();
 bool depthonly = flags & DepthOnly;

 glEnableClientState(GL_VERTEX_ARRAY);
 // We don't need normals and texture weights (stored in color) for depth pass
 if (!depthonly) {
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
 }

#if USE_VBOS
 boglBindBuffer(GL_ARRAY_BUFFER, mVBOVertex);
 glVertexPointer(3, GL_FLOAT, 0, (void*)0);

 if (!depthonly) {
	boglBindBuffer(GL_ARRAY_BUFFER, mVBONormal);
	glNormalPointer(GL_FLOAT, 0, (void*)0);

	boglBindBuffer(GL_ARRAY_BUFFER, mVBOColor);
 }
#else
 glVertexPointer(3, GL_FLOAT, 0, mVertexArray);
 if (!depthonly) {
	glNormalPointer(GL_FLOAT, 0, map->normalMap());
 }
#endif
 // AB: we use textureCount different color pointers, so the glColorPointer()
 // call comes later.
 // (the color VBO remains bound)

 if (!depthonly) {
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 }

 // we draw the cells in different stages. depth test is now enabled in all
 //  stages to prevent drawing errors. Depth func GL_LEQUAL makes sure all
 //  layers get rendered (they have same z values)
 // Maybe it should be set back to GL_LESS later?
 glDepthFunc(GL_LEQUAL);

 if (!depthonly) {
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
 }

 bool useShaders = boConfig->boolValue("UseGroundShaders");
// useShaders = false;

 if (mIndicesDirty || mUsedTexturesDirty) {
	calculateIndices(renderCells, cellsCount, map);
 }


 unsigned int usedTextures = 0;
 unsigned int renderedQuads = 0;
 for (unsigned int i = 0; i < groundTheme->groundTypeCount(); i++) {
	if (depthonly && i > 0) {
		// Depth pass requires only a single pass
		break;
	}
	if (i == 1) {
		glEnable(GL_BLEND);
	}
	unsigned char* colorPointer = mColorArray + (map->cornerArrayPos(map->width(), map->height()) + 1) * 4 * i;
	if (!depthonly) {
		if (mUsedTexturesDirty) {
			mUsedTexturesDirty = false;
			bool useTexture = false;
			for (unsigned int j = 0; j < mIndicesCount && !useTexture; j++) {
				if (colorPointer[mIndicesArray[j] * 4 + 3] != 0) {
					useTexture = true;
					break;
				}
				mUsedTextures[i] = useTexture;
			}
		}
		if (!mUsedTextures[i]) {
			continue;
		}

		usedTextures++;
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
			if (bumptex) {
				bumptex->bind();
			} else {
				BO_NULL_ERROR(bumptex);
			}
			glLoadIdentity();
			glScalef(1.0f / groundData->groundType->textureSize, 1.0f / groundData->groundType->textureSize, 1.0);
			boTextureManager->activateTextureUnit(0);
			// Shader
			groundData->shader->bind();
			groundData->shader->setUniform("bumpScale", groundData->groundType->bumpScale);
			groundData->shader->setUniform("bumpBias", groundData->groundType->bumpBias);
		}

#if USE_VBOS
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, (unsigned char*)0 + (map->cornerArrayPos(map->width(), map->height()) + 1) * 4 * i);
#else // USE_VBOS
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, colorPointer);
#endif // USE_VBOS
	}  // !depthonly

#if USE_QUADS
#if 1
	QMemArray<unsigned int>& textureIndices = *mTextureIndices[i];
	glDrawElements(GL_QUADS, textureIndices.count(), GL_UNSIGNED_INT, textureIndices.data());
#else
	glDrawElements(GL_QUADS, mIndicesCount, GL_UNSIGNED_INT, mIndicesArray);
#endif
#else // USE_QUADS
#if 1
	glDrawElements(GL_TRIANGLE_STRIP, mIndicesCount, GL_UNSIGNED_INT, mIndicesArray);
#else
	// AB: we could also use multiple triangle strips, instead of a single
	// large one.
	// however this is disabled atm.
	int start = 0;
	for (QValueList<int>::const_iterator it = mIndicesCountList.begin(); it != mIndicesCountList.end(); ++it) {
		int indexCount = *it;
		glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, mIndicesArray + start);
		start += indexCount;
	}
#endif // 1
#endif // USE_QUADS

	renderedQuads += cellsCount;
 }
 statistics()->setRenderedQuads(renderedQuads);
 statistics()->setUsedTextures(usedTextures);
 if (!depthonly) {
	if (useShaders) {
		boTextureManager->activateTextureUnit(2);
		glLoadIdentity();
		boTextureManager->disableTexturing();
		boTextureManager->activateTextureUnit(0);
		BoShader::unbind();
	}
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
 }

#if USE_VBOS
 boglBindBuffer(GL_ARRAY_BUFFER, 0);
#endif

 glDisableClientState(GL_VERTEX_ARRAY);
 if (!depthonly) {
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
 }

#warning FIXME: does NOT belong to default renderer. belongs to base class.
 if (map->activeColorMap() && !depthonly) {
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

 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "at end of method" << endl;
 }

 glPopClientAttrib();
 glPopAttrib();

 boTextureManager->invalidateCache();
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
 mTextureIndices.resize(map->groundTheme()->groundTypeCount());
 for (unsigned int i = 0; i < map->groundTheme()->groundTypeCount(); i++) {
	mTextureIndices[i] = new QMemArray<unsigned int>();
 }
 mIndicesDirty = true;
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

void BoDefaultGroundRenderer::cellFogChanged(int x1, int y1, int x2, int y2)
{
 BoGroundRendererBase::cellFogChanged(x1, y1, x2, y2);
 if (!mIndicesDirty) {
	if (isCellInRectVisible(x1, y1, x2, y2)) {
		mIndicesDirty = true;
	}
 }
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

void BoDefaultGroundRenderer::generateCellList(const BosonMap* map)
{
 BoGroundRendererBase::generateCellList(map);
 mIndicesDirty = true;
}


void BoDefaultGroundRenderer::calculateIndices(int* renderCells, unsigned int cellsCount, const BosonMap* map)
{
 if (cellsCount < 1) {
	boError() << k_funcinfo << endl;
	return;
 }
 if (cellsCount > map->width() * map->height()) {
	boError() << k_funcinfo << "cellsCount > total cellscount of map" << endl;
	return;
 }
 delete[] mIndicesArray;
 mIndicesArray = 0;

 if (mTextureIndices.count() != map->groundTheme()->groundTypeCount()) {
	boError() << k_funcinfo << "oops" << endl;
	return;
 }

#if USE_QUADS
 mIndicesArraySize = cellsCount * 4;
 mIndicesCount = mIndicesArraySize;
 mIndicesArray = new unsigned int[mIndicesArraySize];

 for (unsigned int i = 0; i < map->groundTheme()->groundTypeCount(); i++) {
	QMemArray<unsigned int>& textureIndices = *mTextureIndices[i];
	textureIndices.resize(cellsCount * 4);
 }
 for (unsigned int i = 0; i < cellsCount; i++) {
	int x;
	int y;
	int w;
	int h;
	BoGroundRenderer::getCell(renderCells, i, &x, &y, &w, &h);
	mIndicesArray[i * 4 + 0] = map->cornerArrayPos(x, y);
	mIndicesArray[i * 4 + 1] = map->cornerArrayPos(x, y + h);
	mIndicesArray[i * 4 + 2] = map->cornerArrayPos(x + w, y + h);
	mIndicesArray[i * 4 + 3] = map->cornerArrayPos(x + w, y);
 }
 for (unsigned int i = 0; i < map->groundTheme()->groundTypeCount(); i++) {
	QMemArray<unsigned int>& textureIndices = *mTextureIndices[i];
	const unsigned char* colorPointer = mColorArray + (map->cornerArrayPos(map->width(), map->height()) + 1) * 4 * i;

	unsigned int count = 0;
	for (unsigned int j = 0; j < cellsCount; j++) {
		int x;
		int y;
		int w;
		int h;
		BoGroundRenderer::getCell(renderCells, j, &x, &y, &w, &h);
		int pos1 = map->cornerArrayPos(x, y);
		int pos2 = map->cornerArrayPos(x, y + h);
		int pos3 = map->cornerArrayPos(x + w, y + h);
		int pos4 = map->cornerArrayPos(x + w, y);
		if (colorPointer[pos1 * 4 + 3] != 0 ||
				colorPointer[pos2 * 4 + 3] != 0 ||
				colorPointer[pos3 * 4 + 3] != 0 ||
				colorPointer[pos4 * 4 + 3] != 0) {
			textureIndices[count + 0] = pos1;
			textureIndices[count + 1] = pos2;
			textureIndices[count + 2] = pos3;
			textureIndices[count + 3] = pos4;
			count += 4;
		}
	}

	// shrinks the array or doesn't touch it at all
	textureIndices.resize(count);

	if (count == 0) {
		mUsedTextures[i] = false;
	} else {
		mUsedTextures[i] = true;
	}
 }
 mUsedTexturesDirty = false;
#else // USE_QUADS
 mIndicesArraySize = cellsCount * 4; // AB: in worst case we need one strip per quad, i.e. cellsCount * 4 vertices. usually much less.
 mIndicesCount = mIndicesArraySize;
 mIndicesArray = new unsigned int[mIndicesArraySize];
 int previousEndX = -1;
 int previousEndY = -1;
 int previousStartY = -1;
 bool start = true;
 int pos = 0;
 int totalIndexCount = 0;
 for (unsigned int i = 0; i < cellsCount; i++) {
	int x;
	int y;
	int w;
	int h;
	BoGroundRenderer::getCell(renderCells, i, &x, &y, &w, &h);

#if 0
	// start a new strip
	//
	// this must not happen atm - we use a single strip only. see
	// also below.
	if ((previousEndX != x || previousStartY != y || previousEndY != y + h) && !start) {
		start = true;
		totalIndexCount += pos;
		mIndicesCountList.append(pos);
		pos = 0;
	}
#endif

	if (start) {
		mIndicesArray[totalIndexCount + pos++] = map->cornerArrayPos(x, y);
		mIndicesArray[totalIndexCount + pos++] = map->cornerArrayPos(x, y + h);
		mIndicesArray[totalIndexCount + pos++] = map->cornerArrayPos(x + w, y);
		mIndicesArray[totalIndexCount + pos++] = map->cornerArrayPos(x + w, y + h);
		start = false;
	} else {
		// AB: when one of these parameters differs between two quads,
		// we can do
		// a) start a new strip
		// b) insert a dummy triangle that is never being rendered
		//    (width==0) to continue the strip
		//
		// it seems that there is no speed difference between these
		// alternatives.
		if (previousEndX == x && previousEndY == y + h && previousStartY == y) {
			// same LOD level as previous quad - continue strip
			mIndicesArray[totalIndexCount + pos++] = map->cornerArrayPos(x + w, y);
			mIndicesArray[totalIndexCount + pos++] = map->cornerArrayPos(x + w, y + h);
		} else {
			// insert dummy triangle
			mIndicesArray[totalIndexCount + pos++] = map->cornerArrayPos(x, y);

			// now the actual quad
			mIndicesArray[totalIndexCount + pos++] = map->cornerArrayPos(x, y + h);
			mIndicesArray[totalIndexCount + pos++] = map->cornerArrayPos(x + w, y);
			mIndicesArray[totalIndexCount + pos++] = map->cornerArrayPos(x + w, y + h);
		}
	}
	previousEndX = x + w;
	previousStartY = y;
	previousEndY = y + h;

 }
 totalIndexCount += pos;
#if 0
 // this is used when we use multiple strips (see above).
 // atm disabled.
 mIndicesCountList.append(pos);
#endif

 mIndicesCount = totalIndexCount;
#endif

 mIndicesDirty = false;
}

