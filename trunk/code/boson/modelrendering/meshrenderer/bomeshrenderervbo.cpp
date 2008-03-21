/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann (b_mann@gmx.de)

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

#include <bogl.h>

#include "bomeshrenderervbo.h"
#include "bomeshrenderervbo.moc"

#include "../../../bomemory/bodummymemory.h"
#include "../bosonmodel.h"
#include "../bomesh.h"
#include "../../bomaterial.h"
#include "../../info/boinfo.h"

#include <bodebug.h>


class BoMeshRendererModelDataVBO : public BoMeshRendererModelData
{
public:
	BoMeshRendererModelDataVBO() : BoMeshRendererModelData()
	{
		mVBO = 0;
	}

	~BoMeshRendererModelDataVBO()
	{
		if (mVBO) {
			if (!glDeleteBuffers) {
				BO_NULL_ERROR(glDeleteBuffers);
			} else {
				glDeleteBuffers(1, &mVBO);
			}
		}
	}
	unsigned int mVBO;
};

BoMeshRendererVBO::BoMeshRendererVBO() : BoMeshRendererVertexArray()
{
 // try to find the necessary functions
 if (hasVBOExtension()) { // also tests whether the vbo functions are valid
	boDebug() << k_funcinfo << "VBO is available" << endl;
 } else {
	boDebug() << k_funcinfo << "VBO is NOT available" << endl;
 }
 mPreviousModel = 0;
}

BoMeshRendererVBO::~BoMeshRendererVBO()
{
}

BoMeshRendererModelData* BoMeshRendererVBO::createModelData() const
{
 return new BoMeshRendererModelDataVBO;
}

void BoMeshRendererVBO::initModelData(BosonModel* model)
{
 BO_CHECK_NULL_RET(model);
 BoMeshRendererVertexArray::initModelData(model);

 BoMeshRendererModelDataVBO* data = (BoMeshRendererModelDataVBO*)model->meshRendererModelData();
 BO_CHECK_NULL_RET(data);

 if (hasVBOExtension()) {
	glGenBuffers(1, &data->mVBO);
	if (data->mVBO == 0) {
		boError() << k_funcinfo << "no VBO??" << endl;
	} else {
		glBindBuffer(GL_ARRAY_BUFFER, data->mVBO);
		glBufferData(GL_ARRAY_BUFFER,
				model->pointArraySize() * BoMesh::pointSize() * sizeof(float),
				model->pointArray(),
				GL_STATIC_DRAW);
	}
 }
}

void BoMeshRendererVBO::deinitModelData(BosonModel* model)
{
 BO_CHECK_NULL_RET(model);
 BoMeshRendererVertexArray::deinitModelData(model);
}

void BoMeshRendererVBO::setModel(BosonModel* model)
{
 if (!model) {
	BoMeshRenderer::setModel(model);
	return;
 }
 if (!hasVBOExtension()) {
	// if we don't use VBO, we use usual vertex arrays.
	BoMeshRendererVertexArray::setModel(model);
	return;
 }
 if (model == mPreviousModel) {
	return;
 }

 mPreviousModel = model;
 BoMeshRendererModelDataVBO* data = (BoMeshRendererModelDataVBO*)model->meshRendererModelData();
 BO_CHECK_NULL_RET(data);
 if (data->mVBO) { // VBO activated
	BoMeshRenderer::setModel(model);

	void* vertexPtr = 0;
	void* normalPtr = 0;
	void* texelPtr = 0;

	void* startPtr = 0;
	glBindBuffer(GL_ARRAY_BUFFER, data->mVBO);
	// AB: endianness problem?
#define OFFSET(i) ((i) * sizeof(float))
	const int stride = 8 * sizeof(float);
	vertexPtr = (void*)((char*)startPtr + OFFSET(0));
	normalPtr = (void*)((char*)startPtr + OFFSET(0 + 3));
	texelPtr = (void*)((char*)startPtr + OFFSET(0 + 3 + 3));
#undef OFFSET
	glVertexPointer(3, GL_FLOAT, stride, vertexPtr);
	glNormalPointer(GL_FLOAT, stride, normalPtr);
	glTexCoordPointer(2, GL_FLOAT, stride, texelPtr);

	// Disable VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return;
 }
}

unsigned int BoMeshRendererVBO::render(const QColor* teamColor, BoMesh* mesh, RenderFlags flags)
{
 // VBOs are rendered exactly like vertex arrays
 return BoMeshRendererVertexArray::render(teamColor, mesh, flags);
}


bool BoMeshRendererVBO::useVBO() const
{
 // TODO: remove
 return true;
}

bool BoMeshRendererVBO::hasVBOExtension() const
{
 if (!BoInfo::boInfo()->gl()->openGLExtensions().contains("GL_ARB_vertex_buffer_object")) {
	if (BoInfo::boInfo()->gl()->openGLVersion() < MAKE_VERSION(2,0,0)) {
		return false;
	}
 }
 // AB: testing one of these function should be sufficient
 if (!glDeleteBuffers) {
	return false;
 }
 if (!glGenBuffers) {
	return false;
 }
 if (!glBindBuffer) {
	return false;
 }
 if (!glBufferData) {
	return false;
 }
 return true;
}

void BoMeshRendererVBO::initFrame()
{
 BoMeshRendererVertexArray::initFrame();
 mPreviousModel = 0;
}

void BoMeshRendererVBO::deinitFrame()
{
 BoMeshRendererVertexArray::deinitFrame();
}

