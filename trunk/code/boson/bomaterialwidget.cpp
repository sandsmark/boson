/*
    This file is part of the Boson game
    Copyright (C) 2003 Andreas Beckermann (b_mann@gmx.de)

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

#include "bomaterialwidget.h"
#include "bomaterialwidget.moc"

#include "../bomemory/bodummymemory.h"
#include "bonuminput.h"
#include "bomaterial.h"
#include "bovectorinput.h"

#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qintdict.h>
#include <qhbox.h>

#include <klocale.h>

class BoMaterialWidgetPrivate
{
public:
	BoMaterialWidgetPrivate()
	{
		mMaterialsList = 0;

		mActiveMaterial = 0;

		mName = 0;
		mAmbient = 0;
		mDiffuse = 0;
		mSpecular = 0;
		mShininess = 0;
	}
	bool mBlockChanges;
	QComboBox* mMaterialsList;
	QIntDict<BoMaterial> mMaterials;

	BoMaterial* mActiveMaterial;

	QLabel* mName; // AB: do NOT allow changing the name! it will appear in several lists/comboboxes outside of this widget, changing it here only would confuse the user
	BoVector4Input* mAmbient;
	BoVector4Input* mDiffuse;
	BoVector4Input* mSpecular;
	BoFloatNumInput* mShininess;
};

BoMaterialWidget::BoMaterialWidget(QWidget* parent, const char* name) : QWidget(parent, name)
{
 d = new BoMaterialWidgetPrivate;

 d->mBlockChanges = false;

 QVBoxLayout* layout = new QVBoxLayout(this);
 d->mMaterialsList = new QComboBox(this);
 connect(d->mMaterialsList, SIGNAL(activated(int)), this, SLOT(slotActiveMaterialChanged(int)));
 layout->addWidget(d->mMaterialsList);

 QHBox* hbox = 0;
 hbox = new QHBox(this);
 layout->addWidget(hbox);
 (void)new QLabel(i18n("Name:"), hbox);
 d->mName = new QLabel(hbox);

 d->mAmbient = new BoVector4Input(this);
 d->mAmbient->setLabel(i18n("Ambient:"), AlignLeft | AlignVCenter);
 d->mAmbient->setRange(0.0f, 1.0f, 0.1f);
 connect(d->mAmbient, SIGNAL(signalValueChanged(const BoVector4Float&)), this, SLOT(slotUpdateMaterial()));
 layout->addWidget(d->mAmbient);

 d->mDiffuse = new BoVector4Input(this);
 d->mDiffuse->setLabel(i18n("Diffuse:"), AlignLeft | AlignVCenter);
 d->mDiffuse->setRange(0.0f, 1.0f, 0.1f);
 connect(d->mDiffuse, SIGNAL(signalValueChanged(const BoVector4Float&)), this, SLOT(slotUpdateMaterial()));
 layout->addWidget(d->mDiffuse);

 d->mSpecular = new BoVector4Input(this);
 d->mSpecular->setLabel(i18n("Specular:"), AlignLeft | AlignVCenter);
 d->mSpecular->setRange(0.0f, 1.0f, 0.1f);
 connect(d->mSpecular, SIGNAL(signalValueChanged(const BoVector4Float&)), this, SLOT(slotUpdateMaterial()));
 layout->addWidget(d->mSpecular);

 d->mShininess = new BoFloatNumInput(this);
 d->mShininess->setLabel(i18n("Shininess:"), AlignLeft | AlignVCenter);
 d->mShininess->setRange(0.0f, 1.0f, 0.1f);
 connect(d->mShininess, SIGNAL(signalValueChanged(float)), this, SLOT(slotUpdateMaterial()));
 layout->addWidget(d->mShininess);


 slotActiveMaterialChanged(-1);
}

BoMaterialWidget::~BoMaterialWidget()
{
 clearMaterials();
 delete d;
}

void BoMaterialWidget::addMaterial(BoMaterial* mat)
{
 int index = d->mMaterials.count();
 d->mMaterials.insert(index, mat);
 d->mMaterialsList->insertItem(mat->name(), index);
 if (!d->mActiveMaterial) {
	slotActiveMaterialChanged(index);
 }
}

void BoMaterialWidget::clearMaterials()
{
 d->mMaterials.clear();
 d->mMaterialsList->clear();
 slotActiveMaterialChanged(0);
}

void BoMaterialWidget::slotActiveMaterialChanged(int index)
{
 if (index < 0 || (unsigned int)index >= d->mMaterials.count()) {
	d->mActiveMaterial = 0;
	d->mName->setText(i18n("None"));
	return;
 }

 d->mBlockChanges = true;
 d->mActiveMaterial = d->mMaterials[index];
 d->mName->setText(d->mActiveMaterial->name());
 d->mAmbient->setValue4(d->mActiveMaterial->ambient());
 d->mDiffuse->setValue4(d->mActiveMaterial->diffuse());
 d->mSpecular->setValue4(d->mActiveMaterial->specular());
 d->mShininess->setValue(d->mActiveMaterial->shininess());


 d->mBlockChanges = false;
}

void BoMaterialWidget::slotUpdateMaterial()
{
 if (!d->mActiveMaterial) {
	return;
 }
 if (d->mBlockChanges) {
	return;
 }
 d->mBlockChanges = true;
 d->mActiveMaterial->setAmbient(d->mAmbient->value4());
 d->mActiveMaterial->setDiffuse(d->mDiffuse->value4());
 d->mActiveMaterial->setSpecular(d->mSpecular->value4());
 d->mActiveMaterial->setShininess(d->mShininess->value());

 d->mBlockChanges = false;
}

