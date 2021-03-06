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
#include "bomeshrendererimmediate.h"
#include "bomeshrendererimmediate.moc"

#include "../../../bomemory/bodummymemory.h"
#include "../bosonmodel.h"
#include "../bomesh.h"
#include "../../bomaterial.h"

#include <bodebug.h>

BoMeshRendererImmediate::BoMeshRendererImmediate() : BoMeshRenderer()
{
}

BoMeshRendererImmediate::~BoMeshRendererImmediate()
{
}

void BoMeshRendererImmediate::setModel(BosonModel* model)
{
 BoMeshRenderer::setModel(model);
 if (!model) {
	return;
 }
}

void BoMeshRendererImmediate::initFrame()
{
 glPushAttrib(GL_POLYGON_BIT | GL_COLOR_BUFFER_BIT);

 glEnable(GL_CULL_FACE);
 glCullFace(GL_BACK);

 // AB: we enable the alpha test and discard any texture fragments which are
 // greater 0.0 - this allows transparent textures (_not_ translucent - a
 // fragment must either be visible or invisible)
 glEnable(GL_ALPHA_TEST);
 glAlphaFunc(GL_GREATER, 0.0f);
}

void BoMeshRendererImmediate::deinitFrame()
{
 glPopAttrib();
}

unsigned int BoMeshRendererImmediate::render(const QColor* teamColor, BoMesh* mesh, RenderFlags flags)
{
 if (!model()) {
	BO_NULL_ERROR(model());
	return 0;
 }

 if (mesh->pointCount() == 0) {
	// nothing to do.
	return 0;
 }

 int pointsize = BoMesh::pointSize();
 float* pointArray = model()->pointArray() + pointsize * mesh->pointOffset();

 bool resetColor = false; // needs to be true after we changed the current color
 bool resetCullFace = false;

 // AB: we have *lots* of faces! in numbers the maximum i found
 // so far (only a short look) was about 25 toplevel nodes and
 // rarely child nodes. sometimes 2 child nodes or so - maybe 10
 // per model (if at all).
 // but we have up to (short look only) 116 faces *per node*
 // usually it's about 10-20 faces (minimum) per node!
 //
 // so optimization should happen here - if possible at all...

 if (!(flags & DepthOnly)) {
	BoMaterial::activate(mesh->material());
	if (mesh->isTeamColor() && teamColor) {
		glPushAttrib(GL_CURRENT_BIT);
		glColor3ub(teamColor->red(), teamColor->green(), teamColor->blue());
		resetColor = true;
	}
	if (mesh->material()->twoSided()) {
		glDisable(GL_CULL_FACE);
		resetCullFace = true;
	}
 }

 unsigned int renderedPoints = 0;

 glBegin(mesh->renderMode());

 if (mesh->useIndices()) {
	for (unsigned int i = 0; i < mesh->indexCount(); i++) {
		unsigned int index;
		if (model()->indexArrayType() == GL_UNSIGNED_SHORT) {
			index = ((Q_UINT16*)mesh->indices())[i];
		} else {
			index = ((Q_UINT32*)mesh->indices())[i];
		}
		const float* p = model()->pointArray() + (index * BoMesh::pointSize());
		glNormal3fv(p + BoMesh::normalPos());
		glTexCoord2fv(p + BoMesh::texelPos());
		glVertex3fv(p + BoMesh::vertexPos());
		renderedPoints++;
	}
 } else {
	for (unsigned int i = 0; i < mesh->pointCount(); i++) {
		const float* p = pointArray + (i * pointsize);
		glNormal3fv(p + BoMesh::normalPos());
		glTexCoord2fv(p + BoMesh::texelPos());
		glVertex3fv(p + BoMesh::vertexPos());
		renderedPoints++;
	}
 }

 glEnd();


 if (resetColor) {
	// we need to reset the color (mainly for the placement preview)
	glPopAttrib();
 }
 if (resetCullFace) {
	glEnable(GL_CULL_FACE);
 }

 return renderedPoints;
}

