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

#include <klocale.h>

#define MAX_STATISTICS 300
#define MAX_STATISTICS_SHORT 20
#define MAX_STATISTICS_DELAYED 30 // flush the statistics every 30 frames

#if MAX_STATISTICS_SHORT >= MAX_STATISTICS
#error MAX_STATISTICS_SHORT must be less than MAX_STATISTICS
#endif

class BoMeshRendererStatisticsCollectionPrivate
{
public:
	BoMeshRendererStatisticsCollectionPrivate()
	{
	}
	QPtrList<BoMeshRendererStatistics> mStatistics;
	QPtrList<BoMeshRendererStatistics> mDelayedStatistics;

};

BoMeshRendererStatisticsCollection::BoMeshRendererStatisticsCollection()
{
 d = new BoMeshRendererStatisticsCollectionPrivate;
 mMaxEntries = 0;
 mMeshes = 0;
 mPoints = 0;
 mTakeOwnership = true;

 setTakeOwnership(takeOwnership());
}

BoMeshRendererStatisticsCollection::~BoMeshRendererStatisticsCollection()
{
 d->mStatistics.clear();
 d->mDelayedStatistics.clear();
 delete d;
}

void BoMeshRendererStatisticsCollection::setTakeOwnership(bool o)
{
 mTakeOwnership = o;
 d->mStatistics.setAutoDelete(takeOwnership());
 d->mDelayedStatistics.setAutoDelete(takeOwnership());
}

void BoMeshRendererStatisticsCollection::add(BoMeshRendererStatistics* stat)
{
 if (!stat) {
	return;
 }
 d->mDelayedStatistics.append(stat);
 if (d->mDelayedStatistics.count() > MAX_STATISTICS_DELAYED) {
	QPtrListIterator<BoMeshRendererStatistics> it(d->mDelayedStatistics);
	while (it.current()) {
		d->mStatistics.append(it.current());
		addStatistics(it.current());
		++it;
	}
	d->mDelayedStatistics.setAutoDelete(false);
	d->mDelayedStatistics.clear();
	d->mDelayedStatistics.setAutoDelete(takeOwnership());
 }
 while (d->mStatistics.count() > mMaxEntries) {
	BoMeshRendererStatistics* stat = d->mStatistics.getFirst();
	removeStatistics(stat);
	d->mStatistics.removeFirst();
 }
}

void BoMeshRendererStatisticsCollection::addStatistics(const BoMeshRendererStatistics* stat)
{
 mMeshes += stat->meshes();
 mPoints += stat->points();
}

void BoMeshRendererStatisticsCollection::removeStatistics(const BoMeshRendererStatistics* stat)
{
 mMeshes -= stat->meshes();
 mPoints -= stat->points();
}

QString BoMeshRendererStatisticsCollection::statisticsData() const
{
 QString data;
 unsigned int frames = d->mStatistics.count();
 data += i18n("Frames: %1\n").arg(frames);
 if (frames == 0) {
	return data;
 }
 data += i18n("Meshes: %1   Meshes per Frame: %2\n").arg(mMeshes).arg(mMeshes / frames);
 data += i18n("Points: %1   Points per Frame: %2\n").arg(mPoints).arg(mPoints / frames);
 return data;
}


BoMeshRendererStatistics::BoMeshRendererStatistics()
{
 mMeshes = 0;
 mPoints = 0;
}

BoMeshRendererStatistics::~BoMeshRendererStatistics()
{
}

void BoMeshRendererStatistics::addMesh(unsigned int renderedPoints)
{
 mMeshes++;
 mPoints += renderedPoints;
}

void BoMeshRendererStatistics::finalize(BoMeshRendererStatisticsCollection* c)
{
 c->add(this);
}


class BoMeshRendererPrivate
{
public:
	BoMeshRendererPrivate()
	{
	}
};

BoMeshRenderer::BoMeshRenderer() : QObject(0, "meshrenderer")
{
 d = new BoMeshRendererPrivate;
 mModel = 0;
 mStatistics = 0;
 mShortStatistics = 0;
 mCurrentStatistics = 0;
}

BoMeshRenderer::~BoMeshRenderer()
{
 delete mStatistics;
 delete mShortStatistics;
 delete d;
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

void BoMeshRenderer::startModelRendering()
{
 addFrameStatistics();
 initFrame();
}

void BoMeshRenderer::stopModelRendering()
{
 deinitFrame();
 completeFrameStatistics();
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
 unsigned int renderedPoints = render(teamColor, mesh, lod);
 if (renderedPoints > 0) {
	currentStatistics()->addMesh(renderedPoints);
 }
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

void BoMeshRenderer::addFrameStatistics()
{
 initStatisticsCollection();
 BO_CHECK_NULL_RET(mStatistics);
 BO_CHECK_NULL_RET(mShortStatistics);
 if (mCurrentStatistics) {
	boError() << k_funcinfo << "current statistics still set - deleting" << endl;
 }
 delete mCurrentStatistics;
 mCurrentStatistics = new BoMeshRendererStatistics();
}

void BoMeshRenderer::completeFrameStatistics()
{
 if (!mCurrentStatistics) {
	boError() << k_funcinfo << "no current statistics set" << endl;
	return;
 }
 mCurrentStatistics->finalize(mStatistics);
 mCurrentStatistics->finalize(mShortStatistics);
 mCurrentStatistics = 0;
}

QString BoMeshRenderer::statisticsData() const
{
 if (!mStatistics) {
	return i18n("No statistics available");
 }
 QString data = mStatistics->statisticsData();
 if (mShortStatistics) {
	data += i18n("\n");
	data += mShortStatistics->statisticsData();
 }
 return data;
}

void BoMeshRenderer::initStatisticsCollection()
{
 if (mStatistics) {
	return;
 }
 mStatistics = createStatisticsCollection();
 mStatistics->setMaxEntries(MAX_STATISTICS);
 mShortStatistics = createStatisticsCollection();
 mShortStatistics->setTakeOwnership(false);
 mShortStatistics->setMaxEntries(MAX_STATISTICS_SHORT);
}

BoMeshRendererStatisticsCollection* BoMeshRenderer::createStatisticsCollection()
{
 return new BoMeshRendererStatisticsCollection();
}

