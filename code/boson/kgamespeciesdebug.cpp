/*
    This file is part of the Boson game
    Copyright (C) 2003-2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "kgamespeciesdebug.h"
#include "kgamespeciesdebug.moc"

#include "../bomemory/bodummymemory.h"
#include "speciestheme.h"
#include "speciesdata.h"
#include "bodebug.h"
#include "modelrendering/bosonmodel.h"
#include "modelrendering/bomesh.h"
#include "bosonviewdata.h"

#include <klocale.h>

#include <qvgroupbox.h>
#include <qptrdict.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qintdict.h>
#include <qtabwidget.h>
#include <qvbox.h>
#include <qvaluelist.h>
#include <qtooltip.h>


class BosonModelsView : public QWidget
{
public:
	BosonModelsView(QWidget* parent);

	~BosonModelsView()
	{
	}

	void setSpecies(SpeciesTheme* species)
	{
		mSpecies = species;
	}

	void update();

protected:
	QPtrList<BosonModel> unitModels()
	{
		QPtrList<BosonModel> models;
		if (!mSpecies) {
			return models;
		}
		QValueList<unsigned long int> units = mSpecies->allFacilities();
		units += mSpecies->allMobiles();
		QValueList<unsigned long int>::Iterator it;
		for (it = units.begin(); it != units.end(); ++it) {
			if (boViewData->speciesData(mSpecies)->unitModel(*it)) {
				models.append(boViewData->speciesData(mSpecies)->unitModel(*it));
			} else {
				boWarning() << k_funcinfo << "NULL model for unit " << *it << endl;
			}
		}
		return models;
	}
	QPtrList<BosonModel> objectModels()
	{
		QPtrList<BosonModel> models;
		if (!mSpecies) {
			return models;
		}
		QStringList allObjects = mSpecies->allObjects();
		QStringList::Iterator it;
		for (it = allObjects.begin(); it != allObjects.end(); ++it) {
			if (boViewData->speciesData(mSpecies)->objectModel(*it)) {
				models.append(boViewData->speciesData(mSpecies)->objectModel(*it));
			} else {
				boWarning() << k_funcinfo << "NULL model for object " << *it << endl;
			}
		}
		return models;
	}
	QPtrList<BosonModel> allModels()
	{
		QPtrList<BosonModel> models;
		if (!mSpecies) {
			return models;
		}
		models = unitModels();
		QPtrList<BosonModel> objects = objectModels();
		QPtrListIterator<BosonModel> it(objects);
		for (; it.current(); ++it) {
			models.append(*it);
		}
		return models;
	}
	QPtrList<BoMesh> allMeshes()
	{
		QPtrList<BoMesh> meshes;
		QPtrList<BosonModel> models = allModels();
		QPtrListIterator<BosonModel> modelIt(models);
		for (; modelIt.current(); ++modelIt) {
			BoLOD* lod = modelIt.current()->lod(0);
			for (unsigned int i = 0; i < lod->meshCount(); i++) {
				meshes.append(lod->mesh(i));
			}
		}
		return meshes;
	}

	void countMeshes(unsigned int* all, unsigned int* min, unsigned int* max)
	{
		if (!all || !min || !max) {
			return;
		}
		*all = 0;
		*min = 0;
		*max = 0;
		if (!mSpecies) {
			return;
		}
		QPtrList<BosonModel> models = allModels();
		QPtrListIterator<BosonModel> it(models);
		for (; it.current(); ++it) {
			unsigned int meshCount = it.current()->lod(0)->meshCount();
			*all += meshCount;
			if (meshCount > *max) {
				*max = meshCount;
			}
			if (meshCount < *min || it.current() == models.getFirst()) {
				*min = meshCount;
			}
		}
	}
	void countFrames(unsigned int* all, unsigned int* min, unsigned int* max)
	{
		if (!all || !min || !max) {
			return;
		}
		*all = 0;
		*min = 0;
		*max = 0;
		if (!mSpecies) {
			return;
		}
		// We count both frames() and constructionSteps() as frames
		// here.
		QPtrList<BosonModel> models = allModels();
		QPtrListIterator<BosonModel> it(models);
		for (; it.current(); ++it) {
			BosonModel* m = it.current();
			unsigned int frameCount = m->lod(0)->frameCount();
			*all += frameCount;
			if (frameCount > *max) {
				*max = frameCount;
			}
			if (frameCount < *min || m == models.getFirst()) {
				*min = frameCount;
			}
		}
	}
	void countPoints(unsigned int* all, unsigned int* min, unsigned int* max)
	{
		if (!all || !min || !max) {
			return;
		}
		*all = 0;
		*min = 0;
		*max = 0;
		if (!mSpecies) {
			return;
		}

		QPtrList<BoMesh> meshes = allMeshes();
		QPtrListIterator<BoMesh> it(meshes);
		for (; it.current(); ++it) {
			BoMesh* mesh = it.current();
			*all += mesh->pointCount();
			if (mesh->pointCount() > *max) {
				*max = mesh->pointCount();
			}
			if (mesh->pointCount() < *min || mesh == meshes.getFirst()) {
				*min = mesh->pointCount();
			}
		}
	}

private:
	SpeciesTheme* mSpecies;
	QLabel* mModelCount;
	QLabel* mUnitModelCount;
	QLabel* mObjectModelCount;
	QLabel* mMeshCount;
	QLabel* mFrameCount;
	QLabel* mPointCount;
};

BosonModelsView::BosonModelsView(QWidget* parent) : QWidget(parent, "bosonmodelsview")
{
 mSpecies = 0;
 QVBoxLayout* topLayout = new QVBoxLayout(this);

 QHBoxLayout* hLayout = new QHBoxLayout(topLayout);
 QLabel* unitModelCountLabel = new QLabel(i18n("Unit Models: "), this);
 mUnitModelCount = new QLabel(this);
 hLayout->addWidget(unitModelCountLabel);
 hLayout->addWidget(mUnitModelCount);

 hLayout = new QHBoxLayout(topLayout);
 QLabel* objectModelCountLabel = new QLabel(i18n("Object Models: "), this);
 mObjectModelCount = new QLabel(this);
 hLayout->addWidget(objectModelCountLabel);
 hLayout->addWidget(mObjectModelCount);

 hLayout = new QHBoxLayout(topLayout);
 QLabel* modelCountLabel = new QLabel(i18n("Models: "), this);
 mModelCount = new QLabel(this);
 hLayout->addWidget(modelCountLabel);
 hLayout->addWidget(mModelCount);

 hLayout = new QHBoxLayout(topLayout);
 QLabel* meshCountLabel = new QLabel(i18n("Different Meshes (all / minimal per model / maximal per model): "), this);
 mMeshCount= new QLabel(this);
 hLayout->addWidget(meshCountLabel);
 hLayout->addWidget(mMeshCount);

 hLayout = new QHBoxLayout(topLayout);
 QLabel* frameCountLabel = new QLabel(i18n("Frames (all / minimal per model / maximal per model): "), this);
 mFrameCount = new QLabel(this);
 hLayout->addWidget(frameCountLabel);
 hLayout->addWidget(mFrameCount);
 QToolTip::add(mFrameCount, i18n("The number of frames in all models, including the construction steps (they are generated on the fly and are no actual model-frames)"));

 hLayout = new QHBoxLayout(topLayout);
 QLabel* pointCountLabel = new QLabel(i18n("Points (all / minimal per mesh / maximal per mesh): "), this);
 mPointCount = new QLabel(this);
 hLayout->addWidget(pointCountLabel);
 hLayout->addWidget(mPointCount);
 QToolTip::add(mPointCount, i18n("These are the points of all (different!) meshes in all models summed up.\nRemember that a single mesh can be rendered several times, but the points are counted only once here!\nThis number can give you an impression on how much memory is used for the points of the meshes - every points takes 3 coordinates + 3 normal coordinates + 2 texture coordinates, each 4 bytes.\nSo multiply the number of points by 32 and you have the number of allocated bytes for this\n\nAlso remember that every point can be used several times (even among different meshes), but for every occurance in a mesh there is an additional index variable (4 bytes) in memory.\nSo add a small overhead to you calculated number."));
}

void BosonModelsView::update()
{
 mUnitModelCount->setText(QString::number(unitModels().count()));
 mObjectModelCount->setText(QString::number(objectModels().count()));
 mModelCount->setText(QString::number(allModels().count()));

 unsigned int all = 0;
 unsigned int min = 0;
 unsigned int max = 0;
 countMeshes(&all, &min, &max);
 mMeshCount->setText(i18n("%1 / %2 / %3").arg(all).arg(min).arg(max));

 unsigned int allFrames = 0;
 unsigned int minFrames = 0;
 unsigned int maxFrames = 0;
 countFrames(&allFrames, &minFrames, &maxFrames);
 mFrameCount->setText(i18n("%1 / %2 / %3").arg(allFrames).arg(min).arg(max));

 all = 0;
 min = 0;
 max = 0;
 countPoints(&all, &min, &max);
 mPointCount->setText(i18n("%1 / %2 / %3").arg(all).arg(min).arg(max));
}


class SpeciesViewPrivate
{
public:
	SpeciesViewPrivate()
	{
		mSpecies = 0;

		mModelsView = 0;
	}
	SpeciesTheme* mSpecies;

	BosonModelsView* mModelsView;
};

SpeciesView::SpeciesView(QWidget* parent) : QWidget(parent, "speciesview")
{
 d = new SpeciesViewPrivate;
 QVBoxLayout* topLayout = new QVBoxLayout(this);
 QTabWidget* tabWidget = new QTabWidget(this, "tabwidget");
 topLayout->addWidget(tabWidget);

 d->mModelsView = new BosonModelsView(tabWidget);
 tabWidget->addTab(d->mModelsView, i18n("&Models"));
}

SpeciesView::~SpeciesView()
{
 delete d;
}

void SpeciesView::setSpecies(SpeciesTheme* species)
{
 d->mSpecies = species;
 d->mModelsView->setSpecies(species);
}

void SpeciesView::update()
{
 boDebug() << k_funcinfo << endl;
 d->mModelsView->update();
}



class KGameSpeciesDebugPrivate
{
public:
	KGameSpeciesDebugPrivate()
	{
		mSpeciesBox = 0;
		mSpeciesView = 0;
	}

	QIntDict<SpeciesTheme> mThemes;
	QComboBox* mSpeciesBox;
	SpeciesView* mSpeciesView;
};

KGameSpeciesDebug::KGameSpeciesDebug(QWidget* parent) : QWidget(parent)
{
 d = new KGameSpeciesDebugPrivate;
 d->mThemes.setAutoDelete(true);
 QVBoxLayout* topLayout = new QVBoxLayout(this);

 QHBoxLayout* speciesLayout = new QHBoxLayout(topLayout);
 QLabel* speciesLabel = new QLabel(i18n("Species: "), this);
 d->mSpeciesBox = new QComboBox(this);
 speciesLayout->addWidget(speciesLabel);
 speciesLayout->addWidget(d->mSpeciesBox, 1);
 connect(d->mSpeciesBox, SIGNAL(activated(int)),
		this, SLOT(slotChangeSpecies(int)));

 d->mSpeciesView = new SpeciesView(this);
 topLayout->addWidget(d->mSpeciesView, 1);
}

KGameSpeciesDebug::~KGameSpeciesDebug()
{
 d->mThemes.clear();
 delete d;
}

void KGameSpeciesDebug::loadSpecies()
{
 d->mThemes.clear();
 QStringList species = SpeciesTheme::availableSpecies();
 boDebug() << k_funcinfo << "loading " << species.count() << " species" << endl;
 QStringList::Iterator it;
 for (it = species.begin(); it != species.end(); ++it) {
	SpeciesTheme* s = new SpeciesTheme();
	s->loadTheme((*it).left((*it).length() - QString::fromLatin1("index.desktop").length()), red); // dummy color - don't use QColor(0,0,0)
	boViewData->addSpeciesTheme(s);
	SpeciesData* speciesData = boViewData->speciesData(s);
	speciesData->loadObjects(s->teamColor());
	speciesData->loadActions();
	s->readUnitConfigs();
	QValueList<unsigned long int> units = s->allFacilities();
	units += s->allMobiles();
	QValueList<unsigned long int>::Iterator unitsIt;
	for (unitsIt = units.begin(); unitsIt != units.end(); ++unitsIt) {
		const UnitProperties* prop = s->unitProperties(*unitsIt);
		speciesData->loadUnit(prop, s->teamColor());
	}

	s->loadTechnologies();

	d->mSpeciesBox->insertItem(s->identifier());
	d->mThemes.insert(d->mSpeciesBox->count() - 1, s);
 }
 boDebug() << k_funcinfo << "loaded " << species.count() << " species" << endl;

 d->mSpeciesBox->setCurrentItem(0);
 slotChangeSpecies(0);
 update();
}

void KGameSpeciesDebug::slotChangeSpecies(int index)
{
 if (index < 0 || index >= d->mSpeciesBox->count()) {
	boError() << k_funcinfo << "invalid index " << index << endl;
	return;
 }
 if ((unsigned int)d->mSpeciesBox->count() != d->mThemes.count()) {
	boError() << k_funcinfo << "oops!" << endl;
	return;
 }
 SpeciesTheme* species = d->mThemes[index];
 d->mSpeciesView->setSpecies(species);
 d->mSpeciesView->update();
}

