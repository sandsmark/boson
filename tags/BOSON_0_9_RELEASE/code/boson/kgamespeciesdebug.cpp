/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "speciestheme.h"
#include "bodebug.h"
#include "bosonmodel.h"
#include "bomesh.h"

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
			if (mSpecies->unitModel(*it)) {
				models.append(mSpecies->unitModel(*it));
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
			if (mSpecies->objectModel(*it)) {
				models.append(mSpecies->objectModel(*it));
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
			QIntDict<BoMesh> allMeshes = modelIt.current()->allMeshes();
			QIntDictIterator<BoMesh> meshIt(allMeshes);
			for (; meshIt.current(); ++meshIt) {
				meshes.append(meshIt.current());
				if (meshIt.current()->points() == 0) {
					boWarning() << k_funcinfo << "0 points in mesh from model " << modelIt.current()->file() << endl;
				}
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
			*all += it.current()->meshCount();
			if (it.current()->meshCount() > *max) {
				*max = it.current()->meshCount();
			}
			if (it.current()->meshCount() < *min || it.current() == models.getFirst()) {
				*min = it.current()->meshCount();
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
			*all += (m->frames() + m->constructionSteps());
			if ((m->frames() + m->constructionSteps()) > *max) {
				*max = (m->frames() + m->constructionSteps());
			}
			if ((m->frames() + m->constructionSteps()) < *min || m == models.getFirst()) {
				*min = (m->frames() + m->constructionSteps());
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
			*all += mesh->points();
			if (mesh->points() > *max) {
				*max = mesh->points();
			}
			if (mesh->points() < *min || mesh == meshes.getFirst()) {
				*min = mesh->points();
			}
		}
	}
	void countNodes(unsigned int* all, unsigned int* min, unsigned int* max)
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
			*all += mesh->facesCount(0);
			if (mesh->facesCount(0) > *max) {
				*max = mesh->facesCount(0);
			}
			if (mesh->facesCount(0) < *min || mesh == meshes.getFirst()) {
				*min = mesh->facesCount(0);
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
	QLabel* mNodeCount;
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
 QToolTip::add(mPointCount, i18n("These are the points of all (different!) meshes in all models summed up.\nRemember that a single mesh can be rendered several times, but the points are counted only once here!\nThis number can give you an impression on how much memory is used for the points of the meshes - every points takes 3 coordinates + 2 texture coordinates, each 4 bytes.\nSo multiply the number of points by 20 and you have the number of allocated bytes for this\n\nAlso remember that every point can be used several times (even among different meshes), but for every occurance in a mesh there is an additional index variable (4 bytes) in memory.\nSo add a small overhead to you calculated number."));

 hLayout = new QHBoxLayout(topLayout);
 QLabel* nodeCountLabel = new QLabel(i18n("Nodes (all / minimal per mesh / maximal per mesh): "), this);
 mNodeCount = new QLabel(this);
 hLayout->addWidget(nodeCountLabel);
 hLayout->addWidget(mNodeCount);
 QToolTip::add(mNodeCount, i18n("These are the nodes of all (different!) meshes in all models summed up.\nNodes are an internal representation of faces/triangles (3 points) - if a single face (i.e. 3 points) is rendered 10 times (e.g. in 10 different meshes) it will have 10 nodes.\n\nNote that if a single mesh appears 10 times in a model, then all nodes are counted only once (and it is only once in memory)."));


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

 all = 0;
 min = 0;
 max = 0;
 countNodes(&all, &min, &max);
 mNodeCount->setText(i18n("%1 / %2 / %3").arg(all).arg(min).arg(max));

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
	SpeciesTheme* s = new SpeciesTheme((*it).left((*it).length() - QString::fromLatin1("index.desktop").length()), red); // dummy color - don't use QColor(0,0,0)
	s->loadObjects();
	s->loadParticleSystems();
	s->loadActions();
	s->readUnitConfigs();
	QValueList<unsigned long int> units = s->allFacilities();
	units += s->allMobiles();
	QValueList<unsigned long int>::Iterator unitsIt;
	for (unitsIt = units.begin(); unitsIt != units.end(); ++unitsIt) {
		s->loadUnit(*unitsIt);
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

