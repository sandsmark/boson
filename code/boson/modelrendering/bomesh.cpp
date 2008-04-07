/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2003-2005 Rivo Laks (rivolaks@hot.ee)

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

#include "bomesh.h"

#include "../../bomemory/bodummymemory.h"
#include "bodebug.h"
#include "../bo3dtools.h"
#include "../bomaterial.h"
#include "bomeshrenderermanager.h"
#include "bomeshrenderer.h"
#include "bosonmodel.h"

#include <qcolor.h>



BoMesh::BoMesh()
{
 init();
}

BoMesh::~BoMesh()
{
 if (mMeshRendererMeshData) {
	boWarning(100) << "meshrenderer forgot to delete mesh data" << endl;
 }
 delete mMeshRendererMeshData;
}

void BoMesh::init()
{
 mIsTeamColor = false;
 mMaterial = 0;
 mPointCount = 0;
 mPointOffset = 0;
 mIndexCount = 0;
 mIndices = 0;
 mUseIndices = true;
 mRenderMode = GL_TRIANGLES;

 mMeshRendererMeshData = 0;
}

int BoMesh::pointSize()
{
 // Point consists of:
 //  * position (3 floats)
 //  * normal (3 floats)
 //  * texcoord (2 floats)
 return (3 + 3 + 2);
}

int BoMesh::vertexPos()
{
 return 0;
}

int BoMesh::normalPos()
{
 return 3;
}

int BoMesh::texelPos()
{
 return 6;
}

BoTexture* BoMesh::textureObject() const
{
 if (material()) {
	return material()->textureObject();
 }
 return 0;
}

void BoMesh::renderMesh(const BoMatrix* itemMeshMatrix, const BoMatrix* matrix, const QColor& teamColor, RenderFlags flags)
{
 BoMeshRenderer* renderer = BoMeshRendererManager::manager()->currentRenderer();
 if (!renderer) {
	BO_NULL_ERROR(renderer);
	return;
 }
 if (!matrix) {
	BO_NULL_ERROR(matrix);
	return;
 }
 if (mPointCount == 0) {
	// nothing to render. avoid the multmatrix
	return;
 }

 glPushMatrix();
 if (itemMeshMatrix) {
	glMultMatrixf(itemMeshMatrix->data());
 }
 glMultMatrixf(matrix->data());
 renderer->renderMesh(teamColor, this, flags);
 glPopMatrix();
}

void BoMesh::renderVertexPoints(const BosonModel* model)
{
 // TODO: we could use some array rendering function here, even though it's not
 //  speed-critical method.
 float* points = model->pointArray();
 glBegin(GL_POINTS);
	for (unsigned int p = 0; p < mPointCount; p++) {
		float* v = &points[(mPointOffset + p) * pointSize() + vertexPos()];
		glVertex3f(v[0], v[1], v[2]);
	}
 glEnd();
}

void BoMesh::setMeshRendererMeshData(BoMeshRendererMeshData* data)
{
 delete mMeshRendererMeshData;
 mMeshRendererMeshData = data;
}

