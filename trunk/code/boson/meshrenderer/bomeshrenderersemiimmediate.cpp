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
#include "bomeshrenderersemiimmediate.h"
#include "bomeshrenderersemiimmediate.moc"

#include "../bomeshrenderer.h"
#include "../bosonmodel.h"
#include "../bomesh.h"
#include "../bomaterial.h"

#include <bodebug.h>

BoMeshRendererSemiImmediate::BoMeshRendererSemiImmediate() : BoMeshRenderer()
{
}

BoMeshRendererSemiImmediate::~BoMeshRendererSemiImmediate()
{
}

void BoMeshRendererSemiImmediate::setModel(BosonModel* model)
{
 BoMeshRenderer::setModel(model);
 if (!model) {
	return;
 }

 int stride = BoMesh::pointSize() * sizeof(float);
 float* points = model->pointArray();
 glVertexPointer(3, GL_FLOAT, stride, points + BoMesh::vertexPos());
 glTexCoordPointer(2, GL_FLOAT, stride, points + BoMesh::texelPos());
}

void BoMeshRendererSemiImmediate::startModelRendering()
{
 glPushAttrib(GL_POLYGON_BIT); // GL_CULL_FACE

 glEnable(GL_CULL_FACE);
 glCullFace(GL_BACK);

 glEnableClientState(GL_VERTEX_ARRAY);
 glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

void BoMeshRendererSemiImmediate::stopModelRendering()
{
 glPopAttrib();

 glDisableClientState(GL_VERTEX_ARRAY);
 glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void BoMeshRendererSemiImmediate::render(const QColor* teamColor, BoMesh* mesh, BoMeshLOD* lod)
{
 unsigned int* pointsCache = lod->pointsCache();
 unsigned int pointsCacheCount = lod->pointsCacheCount();
 int type = lod->type();
 BoFaceNode* nodes = lod->nodes();

 if (!nodes) {
	// nothing to do.
	return;
 }

 bool resetColor = false; // needs to be true after we changed the current color

 // AB: we have *lots* of faces! in numbers the maximum i found
 // so far (only a short look) was about 25 toplevel nodes and
 // rarely child nodes. sometimes 2 child nodes or so - maybe 10
 // per model (if at all).
 // but we have up to (short look only) 116 faces *per node*
 // usually it's about 10-20 faces (minimum) per node!
 //
 // so optimization should happen here - if possible at all...

 BoMaterial::activate(mesh->material());
 if (!mesh->material()) {
	if (mesh->isTeamColor()) {
		if (teamColor) {
			glPushAttrib(GL_CURRENT_BIT);
			glColor3ub(teamColor->red(), teamColor->green(), teamColor->blue());
			resetColor = true;
		}
	}
 } else if (mesh->material()->textureName().isEmpty()){
	glPushAttrib(GL_CURRENT_BIT);
	glColor3fv(mesh->material()->diffuse().data());
	resetColor = true;
 }

 {
	if (!pointsCache || pointsCacheCount == 0) {
		boError() << k_funcinfo << "no point cache!" << endl;
	} else {
		glBegin(type);

		int vertices = 0;
		int faces = 0;
		BoFaceNode* node = nodes;
		while (node) {
			const BoFace* face = node->face();
			const int* points = face->pointIndex();

			// AB: this is only partially immediate mode!
			glNormal3fv(face->normal(0).data());
			glArrayElement(points[0]);
			glNormal3fv(face->normal(1).data());
			glArrayElement(points[1]);
			glNormal3fv(face->normal(2).data());
			glArrayElement(points[2]);

			vertices += 3;
			faces++;

			node = node->next();
		}

		glEnd();
#if 0
		// FIXME for meshrenderer
		glstat_item_vertices += vertices;
		glstat_item_faces += faces;
#endif

		// reset the normal...
		// (better solution: don't enable light when rendering
		// selection rect!)
		const float n[] = { 0.0f, 0.0f, 1.0f };
		glNormal3fv(n);
	}
 }


 if (resetColor) {
	// we need to reset the color (mainly for the placement preview)
	glPopAttrib();
	resetColor = false;
 }

 // we need this currently, because of the selection rects. we should avoid
 // this.
 // maybe place BoMaterial::deactivate() into SelectBox ?
 if (mesh->material()) {
	mesh->material()->deactivate();
 }
}


