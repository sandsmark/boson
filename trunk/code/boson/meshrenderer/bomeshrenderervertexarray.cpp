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
#include "bomeshrenderervertexarray.h"
#include "bomeshrenderervertexarray.moc"

#include "../bomeshrenderer.h"
#include "../bosonmodel.h"
#include "../bomesh.h"
#include "../bomaterial.h"

#include <bodebug.h>

BoMeshRendererModelDataVA::BoMeshRendererModelDataVA() : BoMeshRendererModelData()
{
 mPoints = 0;
 mPointCount = 0;

 mMaxLODCount = 0;
 mOffsets = 0;
 mPointCounts = 0;
}
BoMeshRendererModelDataVA::~BoMeshRendererModelDataVA()
{
 delete[] mPoints;

 delete[] mOffsets;
 delete[] mPointCounts;
}

class BoMeshRendererMeshLODDataVA : public BoMeshRendererMeshLODData
{
public:
	BoMeshRendererMeshLODDataVA() : BoMeshRendererMeshLODData()
	{
		mOffset = 0;
		mPointCount = 0;
	}
	~BoMeshRendererMeshLODDataVA()
	{
	}
	unsigned int mOffset;
	unsigned int mPointCount;
};

BoMeshRendererVertexArray::BoMeshRendererVertexArray() : BoMeshRenderer()
{
 mPreviousModel = 0;
}

BoMeshRendererVertexArray::~BoMeshRendererVertexArray()
{
}

BoMeshRendererModelData* BoMeshRendererVertexArray::createModelData() const
{
 return new BoMeshRendererModelDataVA;
}

BoMeshRendererMeshData* BoMeshRendererVertexArray::createMeshData() const
{
 return BoMeshRenderer::createMeshData();
}

BoMeshRendererMeshLODData* BoMeshRendererVertexArray::createMeshLODData() const
{
 return new BoMeshRendererMeshLODDataVA;
}

void BoMeshRendererVertexArray::initModelData(BosonModel* model)
{
 BO_CHECK_NULL_RET(model);

 // immediate mode doesn't need extra data, so we add a dummy object only
 // (initModelData won't get called again then)
 BoMeshRendererModelDataVA* data = (BoMeshRendererModelDataVA*)model->meshRendererModelData();
 BO_CHECK_NULL_RET(data);
 // now the difficult part starts.
 // here we have the definition point := (vertex,texel,normal). This means
 // basically that we need to fork every (vertex,texel) pair ("point" in BoMesh)
 // for every new normal.
 // as a result we have points := faces * 3. no vertex sharing takes place.

 unsigned int points = countModelPoints(model);
 data->mPoints = new float[points * 8];
 data->mPointCount = points;
 data->mMaxLODCount = 0;
 for (unsigned int i = 0; i < model->meshCount(); i++) {
	BoMesh* mesh = model->mesh(i);
	if (!mesh) {
		continue;
	}
	data->mMaxLODCount = QMAX(data->mMaxLODCount, lodCount(mesh));
 }
 if (data->mMaxLODCount != 0) {
	data->mOffsets = new unsigned int[model->meshCount() * data->mMaxLODCount];
	data->mPointCounts = new unsigned int[model->meshCount() * data->mMaxLODCount];
 }
 fillModelPointsArray(model, data->mPoints,
		data->mMaxLODCount, data->mOffsets, data->mPointCounts);

}

void BoMeshRendererVertexArray::initMeshData(BoMesh* mesh, unsigned int meshIndex)
{
 BO_CHECK_NULL_RET(mesh);
 Q_UNUSED(meshIndex);
}

void BoMeshRendererVertexArray::initMeshLODData(BoMeshLOD* meshLOD, unsigned int meshIndex, unsigned int lod)
{
 BO_CHECK_NULL_RET(meshLOD);
 BO_CHECK_NULL_RET(model());
 BoMeshRendererModelDataVA* modelData = (BoMeshRendererModelDataVA*)model()->meshRendererModelData();
 BO_CHECK_NULL_RET(modelData);
 BO_CHECK_NULL_RET(modelData->mOffsets);
 BO_CHECK_NULL_RET(modelData->mPointCounts);

 BoMeshRendererMeshLODDataVA* data = (BoMeshRendererMeshLODDataVA*)meshLOD->meshRendererMeshLODData();
 BO_CHECK_NULL_RET(data);

 int i = meshIndex * modelData->mMaxLODCount + lod;
 data->mOffset = modelData->mOffsets[i];
 data->mPointCount = modelData->mPointCounts[i];
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
 BoMeshRendererModelDataVA* data = (BoMeshRendererModelDataVA*)model->meshRendererModelData();
 BO_CHECK_NULL_RET(data);

 const int stride = (3 + 2 + 3) * sizeof(float);
 glVertexPointer(3, GL_FLOAT, stride, data->mPoints);
 glTexCoordPointer(2, GL_FLOAT, stride, data->mPoints + 3);
 glNormalPointer(GL_FLOAT, stride, data->mPoints + (3 + 2));

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


unsigned int BoMeshRendererVertexArray::render(const QColor* teamColor, BoMesh* mesh, BoMeshLOD* lod)
{
 if (!lod) {
	BO_NULL_ERROR(lod);
	return 0;
 }
 if (lod->pointsCacheCount() == 0) {
	return 0;
 }
 if (!lod->pointsCache()) {
	BO_NULL_ERROR(lod->pointsCache());
	return 0;
 }
 BoMeshRendererMeshLODDataVA* lodData = (BoMeshRendererMeshLODDataVA*)lod->meshRendererMeshLODData();
 if (!lodData) {
	BO_NULL_ERROR(lodData);
	return 0;
 }
 int type = lod->type();
 if (type != GL_TRIANGLES) {
	boError() << k_funcinfo << "only GL_TRIANGLES supported" << endl;
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

 BoMaterial::activate(mesh->material());
 if (!mesh->material()) {
	if (mesh->isTeamColor()) {
		if (teamColor) {
			glPushAttrib(GL_CURRENT_BIT);
			glColor3ub(teamColor->red(), teamColor->green(), teamColor->blue());
			resetColor = true;
		}
	}
 } else {
	BoMaterial* mat = mesh->material();
	if (mat->textureName().isEmpty()) {
		glPushAttrib(GL_CURRENT_BIT);
		glColor3fv(mesh->material()->diffuse().data());
		resetColor = true;
	}
	if (mat->twoSided()) {
		glDisable(GL_CULL_FACE);
		resetCullFace = true;
	}
 }
 unsigned int renderedPoints = 0;

// glDrawElements(type, lod->pointsCacheCount(), GL_UNSIGNED_INT, lod->pointsCache());
 glDrawArrays(type, lodData->mOffset, lodData->mPointCount);
 renderedPoints = lodData->mPointCount;

 // reset the normal...
 // (better solution: don't enable light when rendering
 // selection rect!)
 const float n[] = { 0.0f, 0.0f, 1.0f };
 glNormal3fv(n);


 if (resetColor) {
	// we need to reset the color (mainly for the placement preview)
	glPopAttrib();
	resetColor = false;
 }
 if (resetCullFace) {
	glEnable(GL_CULL_FACE);
	resetCullFace = false;
 }

 return renderedPoints;
}


unsigned int BoMeshRendererVertexArray::countModelPoints(const BosonModel* model) const
{
 if (!model) {
	return 0;
 }
 unsigned int points = 0;
 for (unsigned int i = 0; i < model->meshCount(); i++) {
	BoMesh* mesh = model->mesh(i);
	BO_CHECK_NULL_RET0(mesh);
	for (unsigned int j = 0; j < lodCount(mesh); j++) {
		BoMeshLOD* lod = levelOfDetail(mesh, j);
		BO_CHECK_NULL_RET0(lod);
		points += lod->facesCount() * 3;
	}
 }
 return points;
}

void BoMeshRendererVertexArray::fillModelPointsArray(BosonModel* model, float* array, unsigned int maxLODCount, unsigned int* offsets, unsigned int* pointCounts)
{
 BO_CHECK_NULL_RET(model);
 BO_CHECK_NULL_RET(array);
 BO_CHECK_NULL_RET(offsets);
 BO_CHECK_NULL_RET(pointCounts);
 unsigned int totalAddedPoints = 0;
 for (unsigned int i = 0; i < model->meshCount(); i++) {
	BoMesh* mesh = model->mesh(i);
	BO_CHECK_NULL_RET(mesh);
	unsigned int meshAddedPoints = 0;
	float* meshArray = array + (totalAddedPoints * 8);
	for (unsigned int j = 0; j < lodCount(mesh); j++) {
		BoMeshLOD* lod = levelOfDetail(mesh, j);
		BO_CHECK_NULL_RET(lod);
		offsets[i * maxLODCount + j] = totalAddedPoints + meshAddedPoints;
		pointCounts[i * maxLODCount + j] = lod->facesCount() * 3;
		for (unsigned int f = 0; f < lod->facesCount(); f++) {
			const BoFace* face = lod->face(f);
			BO_CHECK_NULL_RET(face);
			float* point = meshArray + (meshAddedPoints * 8);
			for (int v = 0; v < 3; v++) {
				BO_CHECK_NULL_RET(face->pointIndex());
				int p = face->pointIndex()[v];
				BoVector3Float vertex = model->vertex(p);
				BoVector3Float texel = model->texel(p);
				BoVector3Float normal = face->normal(v);
				point[v * 8 + 0] = vertex[0];
				point[v * 8 + 1] = vertex[1];
				point[v * 8 + 2] = vertex[2];
				point[v * 8 + 3] = texel[0];
				point[v * 8 + 4] = texel[1];
				point[v * 8 + 5] = normal[0];
				point[v * 8 + 6] = normal[1];
				point[v * 8 + 7] = normal[2];
			}
			meshAddedPoints += 3;
		}
	}
	totalAddedPoints += meshAddedPoints;
 }

}

