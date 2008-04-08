/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "boselectiondebugwidget.h"
#include "boselectiondebugwidget.moc"

#include "../../../bomemory/bodummymemory.h"
#include "../../gameengine/bosoncanvas.h"
#include "../../gameengine/bosonmap.h"
#include "../../gameengine/playerio.h"
#include "../../gameengine/player.h"
#include "../../gameengine/unit.h"
#include "../../gameengine/unitorder.h"
#include "../../gameengine/unitproperties.h"
#include "../../gameengine/bosonpropertyxml.h"
#include "boufotabwidget.h"
#include "../../gameview/bosonitemrenderer.h"
#include "../../bosonviewdata.h"
#include "../../modelrendering/bosonmodel.h"
#include "../bosonufocanvaswidget.h"
#include "../boselection.h"
#include <bodebug.h>

#include <kgamepropertyhandler.h>

#include <klocale.h>

#include <qtimer.h>
#include <q3valuelist.h>
#include <qpoint.h>
#include <qdom.h>
#include <q3intdict.h>
//Added by qt3to4:
#include <Q3PtrList>

#include <math.h>

class BoSelectionDebugWidgetPrivate
{
public:
	BoSelectionDebugWidgetPrivate()
	{
		mTabWidget = 0;
	}
	BoUfoTabWidget* mTabWidget;

	BoSelectionGroupDebugWidget* mSelectionWidget;
	BoUnitDebugWidget* mLeaderWidget;
	BoUnitXMLDebugWidget* mLeaderXMLWidget;
};

BoSelectionDebugWidget::BoSelectionDebugWidget()
		: BoUfoWidget()
{
 setName("BoSelectionDebugWidget");
 d = new BoSelectionDebugWidgetPrivate();
 mLocalPlayerIO = 0;
 mSelection = 0;

 setLayoutClass(UVBoxLayout);

 d->mTabWidget = new BoUfoTabWidget();
 d->mTabWidget->setName("SelectionDebugTabWidget");
 addWidget(d->mTabWidget);

 d->mSelectionWidget = new BoSelectionGroupDebugWidget();
 d->mTabWidget->addTab(d->mSelectionWidget, i18n("Selection"));

 d->mLeaderWidget = new BoUnitDebugWidget();
 d->mTabWidget->addTab(d->mLeaderWidget, i18n("Leader"));

 d->mLeaderXMLWidget = new BoUnitXMLDebugWidget();
 d->mTabWidget->addTab(d->mLeaderXMLWidget, i18n("Leader XML"));

 BoUfoWidget* stretch = new BoUfoWidget();
 stretch->setName("SelectionDebugWidgetStretch");
 stretch->setStretch(1);
 addWidget(stretch);

 setVisible(false);
}

BoSelectionDebugWidget::~BoSelectionDebugWidget()
{
 delete d;
}

void BoSelectionDebugWidget::update()
{
 BoUfoWidget* v = d->mTabWidget->currentTab();
 if (!v) {
	return;
 }
 if (v == d->mSelectionWidget) {
	d->mSelectionWidget->update(selection());
 } else if (v == d->mLeaderWidget) {
	Unit* leader = 0;
	if (selection()) {
		leader = selection()->leader();
	}
	d->mLeaderWidget->update(leader);
 } else if (v == d->mLeaderXMLWidget) {
	Unit* leader = 0;
	if (selection()) {
		leader = selection()->leader();
	}
	d->mLeaderXMLWidget->update(leader);
 } else {
	boWarning() << k_funcinfo << "unknown widget visible" << endl;
 }
}


class BoSelectionGroupDebugWidgetPrivate
{
public:
	BoSelectionGroupDebugWidgetPrivate()
	{
		mText = 0;
	}
	BoUfoTextEdit* mText;
};

BoSelectionGroupDebugWidget::BoSelectionGroupDebugWidget()
	: BoUfoWidget()
{
 setName("BoSelectionGroupDebugWidget");
 d = new BoSelectionGroupDebugWidgetPrivate;
 d->mText = new BoUfoTextEdit();
 d->mText->setName("GroupDebugText");
 d->mText->setEditable(false);
 addWidget(d->mText);
}

BoSelectionGroupDebugWidget::~BoSelectionGroupDebugWidget()
{
 delete d;
}

void BoSelectionGroupDebugWidget::update(BoSelection* selection)
{
 d->mText->setText(i18n("Nothing selected"));
 if (!selection || selection->count() == 0) {
	return;
 }
 QString text;
 text += i18n("Selected units: %1\n", selection->count());
 if (selection->leader()) {
	Unit* leader = selection->leader();
	Player* owner = leader->owner();
	text += i18n("Leader: %1 (ID=%2, type=%3), Owner: %4 (%5)\n",
			 leader->name(),
			 leader->id(),
			 leader->type(),
			 owner->name(),
			 owner->bosonId());
 } else {
	text += i18n("Leader: NULL\n");
 }
 Q3PtrList<Unit> units = selection->allUnits();
 QString ids = QString::number(units.at(0)->id());
 for (unsigned int i = 1; i < units.count(); i++) {
	ids += QString(", %1").arg(units.at(i)->id());
 }
 text += i18n("All IDs: %1\n\n", ids);
 if (selection->canShoot()) {
	text += i18n("Selection can shoot\n");
 } else {
	text += i18n("Selection can NOT shoot\n");
 }
 if (selection->hasMobileUnit()) {
	text += i18n("Selection has mobile units\n");
 } else {
	text += i18n("Selection has no mobile unit\n");
 }
 if (selection->hasMineralHarvester()) {
	text += i18n("Selection can harvest minerals\n");
 } else {
	text += i18n("Selection can NOT harvest minerals\n");
 }
 if (selection->hasOilHarvester()) {
	text += i18n("Selection can harvest oil\n");
 } else {
	text += i18n("Selection can NOT harvest oil\n");
 }

 while (!units.isEmpty()) {
	Unit* u = units.at(0);
	units.removeRef(u);
	unsigned long int type = u->type();
	unsigned int count = 1;

	QString ids = QString::number(u->id());
	Q3PtrList<Unit> tmp = units; // copy, so that we can remove() from units
	for (Q3PtrListIterator<Unit> it(tmp); it.current(); ++it) {
		if (it.current()->type() == type) {
			ids += QString(", %1").arg(it.current()->id());
			units.removeRef(it.current());
			count++;
		}
	}
	text += i18n("Units with type=%1 (%2): %3.  IDs: %4\n",
			 type,
			 u->unitProperties()->name(),
			 count,
			 ids);
 }

 d->mText->setText(text);
}



class BoUnitDebugWidgetPrivate
{
public:
	BoUnitDebugWidgetPrivate()
	{
		mText = 0;
	}
	BoUfoTextEdit* mText;
};

BoUnitDebugWidget::BoUnitDebugWidget()
	: BoUfoWidget()
{
 setName("BoUnitDebugWidget");
 d = new BoUnitDebugWidgetPrivate;
 d->mText = new BoUfoTextEdit();
 d->mText->setName("UnitDebugText");
 d->mText->setEditable(false);
 addWidget(d->mText);
}

BoUnitDebugWidget::~BoUnitDebugWidget()
{
 delete d;
}

void BoUnitDebugWidget::update(Unit* unit)
{
 d->mText->setText(i18n("No unit available"));
 if (!unit) {
	return;
 }
 QString text;
 text += i18n("Name: %1, Type: %2, Id: %3, RTTI: %4\n", unit->name(), unit->type(), unit->id(), unit->rtti());

 text += i18n("Location: (%1, %2, %3)", unit->centerX(), unit->centerY(), unit->z());
 text += i18n("Rotation: (%1, %2, %3)", unit->xRotation(), unit->yRotation(), unit->rotation());
 text += i18n("AdvanceWork: %1", unit->advanceWork()); // TODO: int -> string
 text += i18n("CurrentOrder type: %1", unit->currentOrder() ? unit->currentOrder()->type() : 0); // TODO: int -> string


 text += "\n";
 text += i18n("KGameProperty objects:\n");

 BosonCustomPropertyXML propertyXML;
 QMultiHash<int, KGamePropertyBase*> dict = unit->dataHandler()->dict();
 QHashIterator<int, KGamePropertyBase*> it(dict);
 while (it.hasNext()) {
	KGamePropertyBase* prop = it.next().value();
	QString value = propertyXML.propertyValue(prop);
	if (value.isNull()) {
		value = i18n("<value could not be retrieved>");
	}
	QString name = unit->propertyName(prop->id());
	if (name.isEmpty()) {
		name = i18n("<unknown>");
	}
	text += i18n("%1 (ID=%2) = %3\n", name, prop->id(), value);
 }
 text += "\n";

 // TODO: upgrades
 // TODO: weapon properties ?
 // TODO: pathinfo

 BosonItemContainer* container = boViewData->itemContainer(unit);
 if (!container) {
	BO_NULL_ERROR(container);
	text += i18n("Cannot retrieve item container\n");
	d->mText->setText(text);
	return;
 }

 BosonItemEffects* effects = container->effects();
 if (!effects) {
	text += i18n("No item effects\n");
 } else {
	text += i18n("%1 item effects\n", effects->effects().count());
 }
 BosonItemRenderer* r = container->itemRenderer();
 if (!r) {
	BO_NULL_ERROR(r);
	text += i18n("No item renderer\n");
 } else {
	BosonItemModelRenderer* mr = 0;
	BosonModel* m = 0;
	QString type = i18n("<unknown>");
	if (r->rtti() == BosonItemRenderer::SimpleRenderer) {
		type = i18n("SimpleRenderer");
	} else if (r->rtti() == BosonItemRenderer::ModelRenderer) {
		type = i18n("ModelRenderer");
		mr = (BosonItemModelRenderer*)r;
		m = mr->model();
	}
	text += i18n("Renderer Type: %1 (%2)\n", type, r->rtti());
	text += i18n("Animation mode: %1\n", r->animationMode());
	if (mr) {
		text += i18n("Current Frame: %1\n", mr->currentFrame());
		text += i18n("LOD count: %1\n", mr->lodCount());
	}
	if (m) {
		text += i18n("Model file: %1\n", m->file());
		text += i18n("Model pointarray size: %1\n", m->pointArraySize());
		text += i18n("Model indexarray size: %1\n", m->indexArraySize());
		// ...
	}
	text += i18n("\n");
 }

 d->mText->setText(text);
}


class BoUnitXMLDebugWidgetPrivate
{
public:
	BoUnitXMLDebugWidgetPrivate()
	{
		mText = 0;
	}
	BoUfoTextEdit* mText;
};

BoUnitXMLDebugWidget::BoUnitXMLDebugWidget()
	: BoUfoWidget()
{
 setName("BoUnitXMLDebugWidget");
 d = new BoUnitXMLDebugWidgetPrivate;
 d->mText = new BoUfoTextEdit();
 d->mText->setName("UnitXMLDebugText");
 d->mText->setEditable(false);
 addWidget(d->mText);
}

BoUnitXMLDebugWidget::~BoUnitXMLDebugWidget()
{
 delete d;
}

void BoUnitXMLDebugWidget::update(Unit* unit)
{
 d->mText->setText(i18n("No unit available"));
 if (!unit) {
	return;
 }
 QString text;
 text += i18n("XML representation of unit:\n");
 QDomDocument doc;
 QDomElement root = doc.createElement("Unit");
 doc.appendChild(root);
 if (!unit->saveAsXML(root)) {
	text += i18n("Error while saving unit to XML!");
 } else {
	text += doc.toString();
 }

 d->mText->setText(text);
}

