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
#include "bomeshrenderervertexarray.h"
#include "bomeshrenderervertexarray.moc"

#include "../../../bomemory/bodummymemory.h"
#include "../bomeshrenderer.h"
#include "../bosonmodel.h"
#include "../bomesh.h"
#include "../../bomaterial.h"

#include <bodebug.h>



BoMeshRendererVertexArray::BoMeshRendererVertexArray() : BoMeshRenderer()
{
 mPreviousModel = 0;
}

BoMeshRendererVertexArray::~BoMeshRendererVertexArray()
{
}

void BoMeshRendererVertexArray::setModel(BosonModel* model)
{
 BoMeshRenderer::setModel(model);
 if (!model) {
	return;
 }
 if (mPreviousModel == model) {
	return;
 }

 const int stride = (3 + 3 + 2) * sizeof(float);
 glVertexPointer(3, GL_FLOAT, stride, model->pointArray());
 glNormalPointer(GL_FLOAT, stride, model->pointArray() + 3);
 glTexCoordPointer(2, GL_FLOAT, stride, model->pointArray() + 3 + 3);

 mPreviousModel = model;
}

void BoMeshRendererVertexArray::initFrame()
{
 glPushAttrib(GL_POLYGON_BIT | GL_COLOR_BUFFER_BIT);

 glEnable(GL_CULL_FACE);
 glCullFace(GL_BACK);
 glEnableClientState(GL_VERTEX_ARRAY);
 glEnableClientState(GL_NORMAL_ARRAY);
 glEnableClientState(GL_TEXTURE_COORD_ARRAY);

 // AB: we enable the alpha test and discard any texture fragments which are
 // greater 0.0 - this allows transparent textures (_not_ translucent - a
 // fragment must either be visible or invisible)
 glEnable(GL_ALPHA_TEST);
 glAlphaFunc(GL_GREATER, 0.0f);

 mPreviousModel = 0;
}

void BoMeshRendererVertexArray::deinitFrame()
{
 glPopAttrib();

 glDisableClientState(GL_VERTEX_ARRAY);
 glDisableClientState(GL_NORMAL_ARRAY);
 glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}


unsigned int BoMeshRendererVertexArray::render(const QColor* teamColor, BoMesh* mesh, RenderFlags flags)
{
 if (mesh->pointCount() == 0) {
	return 0;
 }

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

 if (mesh->useIndices()) {
	unsigned int minindex = mesh->pointOffset();
	unsigned int maxindex = mesh->pointOffset() + mesh->pointCount() - 1;
	glDrawRangeElements(mesh->renderMode(), minindex, maxindex, mesh->indexCount(), model()->indexArrayType(), mesh->indices());
	renderedPoints = mesh->indexCount();
 } else {
	glDrawArrays(mesh->renderMode(), mesh->pointOffset(), mesh->pointCount());
	renderedPoints = mesh->pointCount();
 }

 if (resetColor) {
	// we need to reset the color (mainly for the placement preview)
	glPopAttrib();
 }
 if (resetCullFace) {
	glEnable(GL_CULL_FACE);
 }

 return renderedPoints;
}

