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
#include "bomeshrendererimmediate.h"
#include "bomeshrendererimmediate.moc"

#include "../bomeshrenderer.h"
#include "../bosonmodel.h"
#include "../bomesh.h"
#include "../bomaterial.h"

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
 glPushAttrib(GL_POLYGON_BIT); // GL_CULL_FACE

 glEnable(GL_CULL_FACE);
 glCullFace(GL_BACK);
}

void BoMeshRendererImmediate::deinitFrame()
{
 glPopAttrib();
}

unsigned int BoMeshRendererImmediate::render(const QColor* teamColor, BoMesh* mesh, BoMeshLOD* lod)
{
 if (!model()) {
	BO_NULL_ERROR(model());
	return 0;
 }
 unsigned int* pointsCache = lod->pointsCache();
 unsigned int pointsCacheCount = lod->pointsCacheCount();
 int type = lod->type();
 BoFaceNode* nodes = lod->nodes();

 if (!nodes || pointsCacheCount == 0) {
	// nothing to do.
	return 0;
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

 unsigned int renderedPoints = 0;

 {
	if (!pointsCache || pointsCacheCount == 0) {
		boError() << k_funcinfo << "no point cache!" << endl;
	} else {
		glBegin(type);

		BoFaceNode* node = nodes;
		while (node) {
			const BoFace* face = node->face();
			const int* points = face->pointIndex();

			// AB: this is only partially immediate mode!
			glNormal3fv(face->normal(0).data());
			glTexCoord2fv(model()->texel(points[0]).data()); // AB: this is pretty slow, as it creates a BoVector3 object!
			glVertex3fv(model()->vertex(points[0]).data()); // AB: this is pretty slow, as it creates a BoVector3 object!
			glNormal3fv(face->normal(1).data());
			glTexCoord2fv(model()->texel(points[1]).data());
			glVertex3fv(model()->vertex(points[1]).data());
			glNormal3fv(face->normal(2).data());
			glTexCoord2fv(model()->texel(points[2]).data());
			glVertex3fv(model()->vertex(points[2]).data());

			renderedPoints += 3;

			node = node->next();
		}

		glEnd();

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
 return renderedPoints;
}


