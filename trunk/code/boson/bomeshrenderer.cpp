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
#include "bomeshrenderer.h"
#include "bomeshrenderer.moc"

#include "bosonmodel.h"
#include "bomesh.h"
#include "bomaterial.h"

#include <bodebug.h>


BoMeshRenderer::BoMeshRenderer() : QObject(0, "meshrenderer")
{
 mModel = 0;
}

BoMeshRenderer::~BoMeshRenderer()
{
}

void BoMeshRenderer::setModel(BosonModel* model)
{
 mModel = model;
 if (!mModel) {
	return;
 }
 if (!mModel->meshRendererModelData()) {
	boError() << k_funcinfo << "meshrenderer model data NULL" << endl;
 }
}

void BoMeshRenderer::renderMesh(const QColor* teamColor, BoMesh* mesh, unsigned int _lod)
{
 BO_CHECK_NULL_RET(mesh);
 BO_CHECK_NULL_RET(model());
 // FIXME: we should check now whether mesh belongs to mModel. But it is required
 // to have that in a very fast O(1) way, i.e. only a pointer compare.


 if (_lod >= mesh->lodCount()) {
	_lod = mesh->lodCount() - 1;
 }
 BoMeshLOD* lod = mesh->levelOfDetail(_lod);
 if (!lod) {
	BO_NULL_ERROR(lod);
	return;
 }
 render(teamColor, mesh, lod);
}

void BoMeshRenderer::initializeData(BosonModel* model)
{
 BO_CHECK_NULL_RET(model);
 setModel(0);
 mModel = model;
 mModel->setMeshRendererModelData(createModelData());
 for (unsigned int i = 0; i < model->meshCount(); i++) {
	BoMesh* mesh = model->mesh(i);
	if (!mesh) {
		BO_NULL_ERROR(mesh);
		continue;
	}
	mesh->setMeshRendererMeshData(createMeshData());
	for (unsigned int j = 0; j < lodCount(mesh); j++) {
		BoMeshLOD* lod = levelOfDetail(mesh, j);
		if (!lod) {
			BO_NULL_ERROR(lod);
			continue;
		}
		lod->setMeshRendererMeshLODData(createMeshLODData());
	}
 }

 // now give the meshrenderer a chance to initialize the data
 initModelData(model);
 for (unsigned int i = 0; i < model->meshCount(); i++) {
	BoMesh* mesh = model->mesh(i);
	if (!mesh) {
		BO_NULL_ERROR(mesh);
		continue;
	}
	initMeshData(mesh, i);
	for (unsigned int j = 0; j < lodCount(mesh); j++) {
		BoMeshLOD* lod = levelOfDetail(mesh, j);
		if (!lod) {
			BO_NULL_ERROR(lod);
			continue;
		}
		initMeshLODData(lod, i, j);
	}
 }
 mModel = 0;
}

void BoMeshRenderer::deinitializeData(BosonModel* model)
{
 BO_CHECK_NULL_RET(model);
 setModel(0);
 mModel = model;

 // give the meshrenderer a chance to remove custom data
 for (unsigned int i = 0; i < model->meshCount(); i++) {
	BoMesh* mesh = model->mesh(i);
	if (!mesh) {
		BO_NULL_ERROR(mesh);
		continue;
	}
	for (unsigned int j = 0; j < lodCount(mesh); j++) {
		BoMeshLOD* lod = levelOfDetail(mesh, j);
		if (!lod) {
			BO_NULL_ERROR(lod);
			continue;
		}
		deinitMeshLODData(lod);
	}
	deinitMeshData(mesh);
 }
 deinitModelData(mModel);

 // remove the data from model and meshes/meshLODs
 for (unsigned int i = 0; i < model->meshCount(); i++) {
	BoMesh* mesh = model->mesh(i);
	if (!mesh) {
		BO_NULL_ERROR(mesh);
		continue;
	}
	for (unsigned int j = 0; j < lodCount(mesh); j++) {
		BoMeshLOD* lod = levelOfDetail(mesh, j);
		if (!lod) {
			BO_NULL_ERROR(lod);
			continue;
		}
		lod->setMeshRendererMeshLODData(0);
	}
	mesh->setMeshRendererMeshData(0);
 }
 mModel->setMeshRendererModelData(0);
 mModel = 0;
}

void BoMeshRenderer::initModelData(BosonModel* model)
{
 Q_UNUSED(model);
}

void BoMeshRenderer::initMeshData(BoMesh* mesh, unsigned int meshIndex)
{
 Q_UNUSED(mesh);
 Q_UNUSED(meshIndex);
}

void BoMeshRenderer::initMeshLODData(BoMeshLOD* meshLOD, unsigned int meshIndex, unsigned int lod)
{
 Q_UNUSED(meshLOD);
 Q_UNUSED(meshIndex);
 Q_UNUSED(lod);
}

void BoMeshRenderer::deinitModelData(BosonModel* model)
{
 Q_UNUSED(model);
}

void BoMeshRenderer::deinitMeshData(BoMesh* mesh)
{
 Q_UNUSED(mesh);
}

void BoMeshRenderer::deinitMeshLODData(BoMeshLOD* meshLOD)
{
 Q_UNUSED(meshLOD);
}

BoMeshRendererModelData* BoMeshRenderer::createModelData() const
{
 return new BoMeshRendererModelData;
}

BoMeshRendererMeshData* BoMeshRenderer::createMeshData() const
{
 return new BoMeshRendererMeshData;
}

BoMeshRendererMeshLODData* BoMeshRenderer::createMeshLODData() const
{
 return new BoMeshRendererMeshLODData;
}

unsigned int BoMeshRenderer::lodCount(const BoMesh* mesh) const
{
 if (!mesh) {
	return 0;
 }
 return mesh->lodCount();
}

BoMeshLOD* BoMeshRenderer::levelOfDetail(const BoMesh* mesh, unsigned int lod) const
{
 if (!mesh) {
	return 0;
 }
 return mesh->levelOfDetail(lod);
}

