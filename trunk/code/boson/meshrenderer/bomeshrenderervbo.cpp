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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#define GLX_GLXEXT_PROTOTYPES 1
#define QT_CLEAN_NAMESPACE
#include "bomeshrenderervbo.h"
#include "bomeshrenderervbo.moc"

#include "../bomeshrenderer.h"
#include "../bosonmodel.h"
#include "../bomesh.h"
#include "../bomaterial.h"
#include "../info/boinfo.h"

#include <bodebug.h>

#include <GL/gl.h>
#include <GL/glx.h>

#ifndef GLsizeiptrARB
typedef int GLsizeiptrARB; // AB: hm?
#endif

typedef void (*_bo_glDeleteBuffersARB)(GLsizei, const GLuint*);
typedef void (*_bo_glGenBuffersARB)(GLsizei, GLuint*);
typedef void (*_bo_glBindBufferARB)(GLenum, GLuint);
typedef void (*_bo_glBufferDataARB)(GLenum, GLsizeiptrARB, const GLvoid*, GLenum);

static _bo_glDeleteBuffersARB bo_glDeleteBuffersARB = 0;
static _bo_glGenBuffersARB bo_glGenBuffersARB = 0;
static _bo_glBindBufferARB bo_glBindBufferARB = 0;
static _bo_glBufferDataARB bo_glBufferDataARB = 0;

#ifndef GL_ARRAY_BUFFER_ARB
#define GL_ARRAY_BUFFER_ARB 0x8892
#endif
#ifndef GL_STATIC_DRAW_ARB
#define GL_STATIC_DRAW_ARB 0x88E4
#endif

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
			if (!bo_glDeleteBuffersARB) {
				BO_NULL_ERROR(bo_glDeleteBuffersARB);
			} else {
				bo_glDeleteBuffersARB(1, &mVBO);
			}
		}
	}
	unsigned int mVBO;
};

BoMeshRendererVBO::BoMeshRendererVBO() : BoMeshRendererVertexArray()
{
 // try to find the necessary functions
#ifdef GLX_ARB_get_proc_address
 bo_glDeleteBuffersARB = (_bo_glDeleteBuffersARB)glXGetProcAddressARB((const GLubyte*)"glDeleteBuffersARB");
 bo_glGenBuffersARB = (_bo_glGenBuffersARB)glXGetProcAddressARB((const GLubyte*)"glGenBuffersARB");
 bo_glBindBufferARB = (_bo_glBindBufferARB)glXGetProcAddressARB((const GLubyte*)"glBindBufferARB");
 bo_glBufferDataARB = (_bo_glBufferDataARB)glXGetProcAddressARB((const GLubyte*)"glBufferDataARB");
 if (hasVBOExtension()) { // also tests whether the above functions are valid
	boDebug() << k_funcinfo << "VBO is available" << endl;
 } else {
	boDebug() << k_funcinfo << "VBO is NOT available" << endl;
 }
#else
#warning cannot find GLX_ARB_get_proc_address - cant use vbo extension
 boWarning() << k_funcinfo << "GLX_ARB_get_proc_address not available. please report this with information about your system!" << endl;
#endif
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
	bo_glGenBuffersARB(1, &data->mVBO);
	if (data->mVBO == 0) {
		boError() << k_funcinfo << "no VBO??" << endl;
	} else {
		bo_glBindBufferARB(GL_ARRAY_BUFFER_ARB, data->mVBO);
		bo_glBufferDataARB(GL_ARRAY_BUFFER_ARB,
				model->pointArraySize() * BoMesh::pointSize() * sizeof(float),
				model->pointArray(),
				GL_STATIC_DRAW_ARB);
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
	bo_glBindBufferARB(GL_ARRAY_BUFFER_ARB, data->mVBO);
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
	bo_glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

	return;
 }
}

unsigned int BoMeshRendererVBO::render(const QColor* teamColor, BoMesh* mesh)
{
 // VBOs are rendered exactly like vertex arrays
 return BoMeshRendererVertexArray::render(teamColor, mesh);
}


bool BoMeshRendererVBO::useVBO() const
{
 // TODO: remove
 return true;
}

bool BoMeshRendererVBO::hasVBOExtension() const
{
 if (!BoInfo::boInfo()->openGLExtensions().contains("GL_ARB_vertex_buffer_object")) {
	return false;
 }
 // AB: testing one of these function should be sufficient
 if (!bo_glDeleteBuffersARB) {
	return false;
 }
 if (!bo_glGenBuffersARB) {
	return false;
 }
 if (!bo_glBindBufferARB) {
	return false;
 }
 if (!bo_glBufferDataARB) {
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

