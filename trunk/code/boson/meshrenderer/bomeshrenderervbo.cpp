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
#define GL_GLEXT_PROTOTYPES 1
#include "bomeshrenderervbo.h"
#include "bomeshrenderervbo.moc"

#include "../bomeshrenderer.h"
#include "../bosonmodel.h"
#include "../bomesh.h"
#include "../bomaterial.h"
#include "../info/boinfo.h"

#if HAVE_GL_GLEXT_H
#include <GL/glext.h>
#endif
#include <GL/gl.h>

#include <bodebug.h>

class BoMeshRendererModelDataVBO : public BoMeshRendererModelDataVA
{
public:
	BoMeshRendererModelDataVBO() : BoMeshRendererModelDataVA()
	{
		mVBO = 0;
	}

	~BoMeshRendererModelDataVBO()
	{
		if (mVBO) {
#ifdef GL_ARB_vertex_buffer_object
			glDeleteBuffersARB(1, &mVBO);
#endif
		}
	}
	unsigned int mVBO;
};

BoMeshRendererVBO::BoMeshRendererVBO() : BoMeshRendererVertexArray()
{
}

BoMeshRendererVBO::~BoMeshRendererVBO()
{
}

BoMeshRendererModelData* BoMeshRendererVBO::createModelData() const
{
 return new BoMeshRendererModelDataVBO;
}

BoMeshRendererMeshData* BoMeshRendererVBO::createMeshData() const
{
 return BoMeshRendererVertexArray::createMeshData();
}

BoMeshRendererMeshLODData* BoMeshRendererVBO::createMeshLODData() const
{
 return BoMeshRendererVertexArray::createMeshLODData();
}

void BoMeshRendererVBO::initModelData(BosonModel* model)
{
 BO_CHECK_NULL_RET(model);
 BoMeshRendererVertexArray::initModelData(model);

 BoMeshRendererModelDataVBO* data = (BoMeshRendererModelDataVBO*)model->meshRendererModelData();
 BO_CHECK_NULL_RET(data);

 const unsigned int pointSize = 8;
#ifdef GL_ARB_vertex_buffer_object
 if (BoInfo::boInfo()->openGLExtensions().contains("GL_ARB_vertex_buffer_object")) {
	glGenBuffersARB(1, &data->mVBO);
	if (data->mVBO == 0) {
		boError() << k_funcinfo << "no VBO??" << endl;
	} else {
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, data->mVBO);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB,
				data->mPointCount * pointSize * sizeof(float),
				data->mPoints,
				GL_STATIC_DRAW_ARB);
	}
 }
#endif // GL_ARB_vertex_buffer_object
}

void BoMeshRendererVBO::initMeshData(BoMesh* mesh, unsigned int meshIndex)
{
 BO_CHECK_NULL_RET(mesh);
 BoMeshRendererVertexArray::initMeshData(mesh, meshIndex);
}

void BoMeshRendererVBO::initMeshLODData(BoMeshLOD* meshLOD, unsigned int meshIndex, unsigned int lod)
{
 BO_CHECK_NULL_RET(meshLOD);
 BoMeshRendererVertexArray::initMeshLODData(meshLOD, meshIndex, lod);
}

void BoMeshRendererVBO::deinitModelData(BosonModel* model)
{
 BO_CHECK_NULL_RET(model);
 BoMeshRendererVertexArray::deinitModelData(model);
}

void BoMeshRendererVBO::deinitMeshData(BoMesh* mesh)
{
 BO_CHECK_NULL_RET(mesh);
 BoMeshRendererVertexArray::deinitMeshData(mesh);
}

void BoMeshRendererVBO::deinitMeshLODData(BoMeshLOD* meshLOD)
{
 BO_CHECK_NULL_RET(meshLOD);
 BoMeshRendererVertexArray::deinitMeshLODData(meshLOD);
}

void BoMeshRendererVBO::setModel(BosonModel* model)
{
 if (!model) {
	BoMeshRenderer::setModel(model);
	return;
 }
#ifdef GL_ARB_vertex_buffer_object
 BoMeshRendererModelDataVBO* data = (BoMeshRendererModelDataVBO*)model->meshRendererModelData();
 BO_CHECK_NULL_RET(data);
 if (data->mVBO) { // VBO activated
	BoMeshRenderer::setModel(model);

	void* vertexPtr = 0;
	void* normalPtr = 0;
	void* texelPtr = 0;

	void* startPtr = 0;
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, data->mVBO);
	// AB: endianness problem?
#define OFFSET(i) ((i) * sizeof(float))
	const int stride = 8 * sizeof(float);
	vertexPtr = (void*)((char*)startPtr + OFFSET(0));
	texelPtr = (void*)((char*)startPtr + OFFSET(0 + 3));
	normalPtr = (void*)((char*)startPtr + OFFSET(0 + 3 + 2));
#undef OFFSET
	glVertexPointer(3, GL_FLOAT, stride, vertexPtr);
	glNormalPointer(GL_FLOAT, stride, normalPtr);
	glTexCoordPointer(2, GL_FLOAT, stride, texelPtr);

#warning FIXME
	// atm we keep the data->mPoints around, because of useVBO() which can
	// be changed on the fly.
	// since we can change the renderer easily to plain vertex-arrays i
	// believe we can remove that configure option and rather delete the
	// points here (saves memory).
	// keep in mind that the vertexarray renderer (the vbo depends on that)
	// must not access data->mPoints anymore then
	return;
 }
#endif

 // if we don't use VBO, we use usual vertex arrays.
 BoMeshRendererVertexArray::setModel(model);
}

void BoMeshRendererVBO::render(const QColor* teamColor, BoMesh* mesh, BoMeshLOD* lod)
{
 // VBOs are rendered exactly like vertex arrays
 BoMeshRendererVertexArray::render(teamColor, mesh, lod);
}


bool BoMeshRendererVBO::useVBO() const
{
 // TODO: remove
 return true;
}

