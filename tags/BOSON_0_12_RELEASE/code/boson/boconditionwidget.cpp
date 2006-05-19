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

#include "boconditionwidget.h"
#include "boconditionwidget.moc"

#include "../bomemory/bodummymemory.h"
#include "bo3dtools.h"
#include "gameengine/boeventmatching.h"
#include "gameengine/boevent.h"
#include "bodebug.h"
#include "gameengine/bocondition.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qpushbutton.h>
#include <qvgroupbox.h>
#include <qcheckbox.h>
#include <qdom.h>
#include <qtooltip.h>
#include <qwidgetstack.h>

#include <klistbox.h>
#include <klocale.h>
#include <knuminput.h>
#include <kmessagebox.h>

BoConditionOverviewWidget::BoConditionOverviewWidget(QWidget* parent)
	: QWidget(parent)
{
 QVBoxLayout* topLayout = new QVBoxLayout(this);

 QVBoxLayout* vlayout = new QVBoxLayout(topLayout);
 QLabel* conditionLabel = new QLabel(i18n("Conditions:"), this);
 vlayout->addWidget(conditionLabel);
 mConditions = new QListBox(this);
 vlayout->addWidget(mConditions);
 connect(mConditions, SIGNAL(highlighted(int)), this, SIGNAL(signalSelectCondition(int)));

 QHBoxLayout* hlayout = new QHBoxLayout(topLayout);
 mEvents = new QPushButton(i18n("Events"), this);
 mStatusConditions = new QPushButton(i18n("Status Conditions"), this);
 mAction = new QPushButton(i18n("Action"), this);
 hlayout->addWidget(mEvents);
 hlayout->addWidget(mStatusConditions);
 hlayout->addWidget(mAction);
 connect(mEvents, SIGNAL(clicked()), this, SIGNAL(signalShowEvents()));
 connect(mStatusConditions, SIGNAL(clicked()), this, SIGNAL(signalShowStatusConditions()));
 connect(mAction, SIGNAL(clicked()), this, SIGNAL(signalShowAction()));

 hlayout = new QHBoxLayout(topLayout);
 mAddCondition = new QPushButton(i18n("New Condition"), this);
 mDeleteCondition = new QPushButton(i18n("Delete Condition"), this);
 hlayout->addWidget(mAddCondition);
 hlayout->addWidget(mDeleteCondition);
 connect(mAddCondition, SIGNAL(clicked()), this, SIGNAL(signalAddNewCondition()));
 connect(mDeleteCondition, SIGNAL(clicked()),
		this, SLOT(slotDeleteCurrentCondition()));
}

BoConditionOverviewWidget::~BoConditionOverviewWidget()
{
}

void BoConditionOverviewWidget::reset()
{
 mConditions->clear();
 mEvents->setEnabled(false);
 mStatusConditions->setEnabled(false);
 mAction->setEnabled(false);
}

void BoConditionOverviewWidget::addCondition(const QString& name)
{
 new QListBoxText(mConditions, name);
 mEvents->setEnabled(true);
// mStatusConditions->setEnabled(true);
 mAction->setEnabled(true);
}

void BoConditionOverviewWidget::slotDeleteCurrentCondition()
{
 int index = mConditions->currentItem();
 if (index < 0) {
	return;
 }
 emit signalDeleteCondition(index);
}

void BoConditionOverviewWidget::deleteCondition(int index)
{
 if (index < 0) {
	return;
 }
 if ((unsigned int)index >= mConditions->count()) {
	return;
 }
 mConditions->removeItem(index);
 if (mConditions->count() == 0) {
	reset();
 }
}

int BoConditionOverviewWidget::currentCondition() const
{
 return mConditions->currentItem();
}

class BoConditionWidgetPrivate
{
public:
	BoConditionWidgetPrivate()
	{
		mOverview = 0;
		mConditionAspect = 0;
		mConditionEvents = 0;
//		mConditionStatusConditions = 0;
		mConditionAction = 0;

		mConditionDocument = 0;
	}

	BoConditionOverviewWidget* mOverview;
	QWidgetStack* mConditionAspect;
	BoConditionEventsWidget* mConditionEvents;
//	Foobar* mConditionStatusConditions;
	BoConditionActionWidget* mConditionAction;

	int mCurrentCondition;

	QDomDocument* mConditionDocument;
};

BoConditionWidget::BoConditionWidget(QWidget* parent) : QWidget(parent)
{
 d = new BoConditionWidgetPrivate;
 d->mCurrentCondition = -1;
 d->mConditionDocument = new QDomDocument("Conditions");
 d->mConditionDocument->appendChild(d->mConditionDocument->createElement("Conditions"));
 QHBoxLayout* topLayout = new QHBoxLayout(this, 5, 5);
 d->mOverview = new BoConditionOverviewWidget(this);
 connect(d->mOverview, SIGNAL(signalSelectCondition(int)),
		this, SLOT(slotShowCondition(int)));
 connect(d->mOverview, SIGNAL(signalShowAction()),
		this, SLOT(slotShowAction()));
 connect(d->mOverview, SIGNAL(signalShowEvents()),
		this, SLOT(slotShowEvents()));
 connect(d->mOverview, SIGNAL(signalShowStatusConditions()),
		this, SLOT(slotShowStatusConditions()));
 connect(d->mOverview, SIGNAL(signalAddNewCondition()),
		this, SLOT(slotAddCondition()));
 connect(d->mOverview, SIGNAL(signalDeleteCondition(int)),
		this, SLOT(slotDeleteCondition(int)));
 topLayout->addWidget(d->mOverview);

 d->mConditionAspect = new QWidgetStack(this);
 topLayout->addWidget(d->mConditionAspect);
 d->mConditionEvents = new BoConditionEventsWidget(d->mConditionAspect);
 d->mConditionAspect->addWidget(d->mConditionEvents);
// d->mConditionStatusConditions = new BoOneConditionWidget(d->mConditionAspect);
// d->mConditionAspect->addWidget(d->mConditionStatusConditions);
 d->mConditionAction = new BoConditionActionWidget(d->mConditionAspect);
 d->mConditionAspect->addWidget(d->mConditionAction);
 d->mConditionAspect->raiseWidget(d->mConditionEvents);
}

BoConditionWidget::~BoConditionWidget()
{
 delete d;
}

void BoConditionWidget::slotAddCondition()
{
 QDomElement condition = d->mConditionDocument->createElement("Condition");
 condition.appendChild(d->mConditionDocument->createElement("Events"));
 condition.appendChild(d->mConditionDocument->createElement("StatusConditions"));
 QDomElement action = d->mConditionDocument->createElement("Action");
 action.setAttribute("Type", "Event");
 QDomElement actionEvent = d->mConditionDocument->createElement("Event");
 actionEvent.setAttribute("Name", "CustomStringEvent");
 actionEvent.setAttribute("Id", QString::number(0));
 actionEvent.setAttribute("UnitId", QString::number(0));
 actionEvent.setAttribute("DelayedDelivery", QString::number(0));
 actionEvent.setAttribute("HasLocation", QString::number(0));
 actionEvent.setAttribute("Data1", "");
 actionEvent.setAttribute("Data2", "");
 actionEvent.setAttribute("Location.x", QString::number(0.0f));
 actionEvent.setAttribute("Location.y", QString::number(0.0f));
 actionEvent.setAttribute("Location.z", QString::number(0.0f));
 action.appendChild(actionEvent);
 condition.appendChild(action);


 BoCondition test;
 if (!test.loadFromXML(condition)) {
	boError() << k_funcinfo << "cannot load new condition" << endl;
	return;
 }

 addCondition(condition);
}

void BoConditionWidget::addCondition(const QDomElement& condition)
{
 d->mConditionDocument->documentElement().appendChild(condition.cloneNode());
 d->mOverview->addCondition(i18n("Condition"));
}

void BoConditionWidget::slotDeleteCondition(int index)
{
 if (index == d->mOverview->currentCondition()) {
	// TODO: clear the widgets in d->mConditionAspect
 }
 d->mOverview->deleteCondition(index);
}

void BoConditionWidget::slotShowCondition(int index)
{
 if (index < 0) {
	return;
 }
 saveCurrentCondition();
 d->mConditionEvents->reset();
// d->mConditionStatusCondition->reset();
 d->mConditionAction->reset();
 d->mCurrentCondition = index;
 QDomElement root = d->mConditionDocument->documentElement();
 int i = 0;
 for (QDomNode n = root.firstChild(); !n.isNull(); n = n.nextSibling()) {
	QDomElement e = n.toElement();
	if (e.isNull()) {
		continue;
	}
	if (e.tagName() != "Condition") {
		continue;
	}
	if (i == index) {
		d->mConditionEvents->loadCondition(e);
//		d->mConditionStatusConditions->loadCondition(e);
		d->mConditionAction->loadCondition(e);
		return;
	}
	i++;
 }
}

void BoConditionWidget::slotShowAction()
{
 d->mConditionAspect->raiseWidget(d->mConditionAction);
}

void BoConditionWidget::slotShowEvents()
{
 d->mConditionAspect->raiseWidget(d->mConditionEvents);
}

void BoConditionWidget::slotShowStatusConditions()
{
// d->mConditionAspect->raiseWidget(d->mConditionStatusConditions);
}

void BoConditionWidget::saveCurrentCondition()
{
 if (d->mCurrentCondition < 0) {
	return;
 }
 int i = 0;
 QDomElement root = d->mConditionDocument->documentElement();
 for (QDomNode n = root.firstChild(); !n.isNull(); n = n.nextSibling()) {
	QDomElement e = n.toElement();
	if (e.isNull()) {
		continue;
	}
	if (e.tagName() != "Condition") {
		continue;
	}
	if (i == d->mCurrentCondition) {
		QDomDocument eventsDoc = d->mConditionEvents->eventsDocument();
		QDomElement events = eventsDoc.documentElement().namedItem("Events").toElement();
#if 0
		QDomDocument statusConditionsDoc = d->mConditionStatusConditions->statusConditionsDocument();
		QDomElement statusConditions = eventsDoc.namedItem("StatusConditions").toElement();
#else
		QDomElement statusConditions = d->mConditionDocument->createElement("StatusConditions");
#endif
		QDomDocument actionDoc = d->mConditionAction->actionDocument();
		QDomElement action = actionDoc.namedItem("Action").toElement();

		if (events.isNull()) {
			boError() << k_funcinfo << "could not save Events" << endl;
			return;
		}
		if (statusConditions.isNull()) {
			boError() << k_funcinfo << "could not save StatusConditions" << endl;
			return;
		}
		if (action.isNull()) {
			boError() << k_funcinfo << "could not save Action" << endl;
			return;
		}
		
		while (e.hasChildNodes()) {
			e.removeChild(e.firstChild());
		}
		e.appendChild(events.cloneNode());
		e.appendChild(statusConditions.cloneNode());
		e.appendChild(action.cloneNode());
		return;
	}
	i++;
 }
}

QString BoConditionWidget::toString()
{
 saveCurrentCondition();

 return d->mConditionDocument->toString();
}

const QDomDocument& BoConditionWidget::conditionsDocument()
{
 saveCurrentCondition();
 return *d->mConditionDocument;
}

bool BoConditionWidget::loadConditions(const QDomElement& root)
{
 d->mOverview->reset();
 d->mConditionEvents->reset();
// d->mConditionStatusConditions->reset();
 d->mConditionAction->reset();
 QDomElement localRoot = d->mConditionDocument->documentElement();
 while (localRoot.hasChildNodes()) {
	localRoot.removeChild(root.firstChild());
 }

 QDomNodeList list = root.elementsByTagName("Condition");

 // first test if conditions are valid
 for (unsigned int i = 0; i < list.count(); i++) {
	QDomElement e = list.item(i).toElement();
	BoCondition test;
	if (!test.loadFromXML(e)) {
		boError() << k_funcinfo << "cannot load condition " << i << endl;
		return false;
	}
 }

 // now copy conditions
 for (unsigned int i = 0; i < list.count(); i++) {
	QDomElement e = list.item(i).toElement();
	addCondition(e);
 }

 return true;
}


BoConditionActionWidget::BoConditionActionWidget(QWidget* parent) : QWidget(parent)
{
 QHBoxLayout* layout = new QHBoxLayout(this);

 // atm we will always emit "CustomStringEvent" with one parameter
 // LABEL: Action (parameter to event)
 mAction = new BoEventMatchingWidget(this, false);
 QToolTip::add(mAction, i18n("This is the action that will be fired once the condition is met.\nCurrently only an event is supported."));

 layout->addWidget(mAction);
}

BoConditionActionWidget::~BoConditionActionWidget()
{
}

void BoConditionActionWidget::reset()
{
 mAction->slotClear();
}

QDomDocument BoConditionActionWidget::actionDocument()
{
 QDomDocument doc;
 QDomElement action = doc.createElement("Action");
 doc.appendChild(action);

 action.setAttribute("Type", "Event");
 QDomElement actionEvent = mAction->event();
 action.appendChild(actionEvent);

 return doc;
}

bool BoConditionActionWidget::loadCondition(const QDomElement& condition)
{
 if (condition.isNull()) {
	return false;
 }
 if (condition.namedItem("Action").toElement().isNull()) {
	boError() << k_funcinfo << "no Action tag" << endl;
	return false;
 }

 if (!mAction->displayEvent(condition.namedItem("Action").namedItem("Event").toElement())) {
	boError() << k_funcinfo << "cannot display Action Event" << endl;
	return false;
 }
// mAction->setText(condition.attribute("EventCaused"));
 return true;
}


BoConditionEventsWidget::BoConditionEventsWidget(QWidget* parent) : QWidget(parent)
{
 QVBoxLayout* topLayout = new QVBoxLayout(this);
 QHBoxLayout* layout = new QHBoxLayout(topLayout);

 mConditionDocument = new QDomDocument("Condition");
 clearXML();

 mEventMatchings = new KListBox(this);
 connect(mEventMatchings, SIGNAL(highlighted(int)),
		this, SLOT(slotSelectedEventMatching(int)));
 QToolTip::add(mEventMatchings, i18n("A list of events that need to be matched before the action fires.\nALL of these events have to matched."));

 QVBox* eventMatchingButtonsBox = new QVBox(this);
 mAddMatching = new QPushButton(i18n("Add Event"), eventMatchingButtonsBox);
 mRemoveMatching = new QPushButton(i18n("Remove Event"), eventMatchingButtonsBox);
 connect(mAddMatching, SIGNAL(clicked()),
		this, SLOT(slotAddEventMatching()));
 connect(mRemoveMatching, SIGNAL(clicked()),
		this, SLOT(slotRemoveCurrentEventMatching()));

 layout->addWidget(mEventMatchings);
 layout->addWidget(eventMatchingButtonsBox);

 mEventMatchingWidget = new BoEventMatchingWidget(this);
 connect(mEventMatchingWidget, SIGNAL(signalEventMatching(const QDomElement&)),
		this, SLOT(slotEventMatchingUpdated(const QDomElement&)));
 mEventMatchingWidget->setEnabled(false);
 QToolTip::add(mEventMatchingWidget, i18n("Edit the currently selected event matching here"));
 topLayout->addWidget(mEventMatchingWidget);

 mCurrentEventMatchingIndex = -1;
}

BoConditionEventsWidget::~BoConditionEventsWidget()
{
 delete mConditionDocument;
}

void BoConditionEventsWidget::reset()
{
 clearXML();
 reloadEventMatchings();
 mEventMatchingWidget->setEnabled(false);
}

void BoConditionEventsWidget::slotAddEventMatching()
{
 boDebug() << k_funcinfo << "adding dummy event matching" << endl;
 QDomElement root = mConditionDocument->documentElement();
 QDomElement events = root.namedItem("Events").toElement();
 QDomElement eventMatching = mConditionDocument->createElement("EventMatching");
 eventMatching.setAttribute("IsLeft", QString::number(1));
 eventMatching.setAttribute("IgnoreUnitId", true);
 eventMatching.setAttribute("IgnorePlayerId", true);
 eventMatching.setAttribute("IgnoreData1", true);
 eventMatching.setAttribute("IgnoreData2", true);
 QDomElement event = mConditionDocument->createElement("Event");
 eventMatching.appendChild(event);
 BoEvent* saveEvent = new BoEvent("AllUnitsDestroyed");
 bool success = saveEvent->saveAsXML(event);
 delete saveEvent;
 saveEvent = 0;
 if (!success) {
	boError() << k_funcinfo << "could not save dummy event" << endl;
	KMessageBox::sorry(this, i18n("Unable to save a dummy event. Cannot add an event matching."));
	return;
 }
 BoEventMatching* matching = new BoEventMatching();
 success = matching->loadFromXML(eventMatching);
 delete matching;
 matching = 0;
 if (!success) {
	boError() << k_funcinfo << "could not load dummy event" << endl;
	KMessageBox::sorry(this, i18n("Unable to re-load a dummy event matching. Cannot add an event matching."));
	return;
 }
 events.appendChild(eventMatching);

 reloadEventMatchings();
 mEventMatchings->setCurrentItem(mEventMatchings->count() - 1);
 slotSelectedEventMatching(mEventMatchings->currentItem());
}

void BoConditionEventsWidget::reloadEventMatchings()
{
 unselectEventMatching();
 QDomElement root = mConditionDocument->documentElement();
 QDomElement events = root.namedItem("Events").toElement();

 mEventMatchings->clear();
 QDomNodeList list = events.elementsByTagName("EventMatching");
 boDebug() << k_funcinfo << "displaying " << list.count() << " event matchings" << endl;
 for (unsigned int i = 0; i < list.count(); i++) {
	QDomElement e = list.item(i).toElement();
	BoEventMatching* matching = new BoEventMatching();
	bool success = matching->loadFromXML(e);
	QListBoxItem* item = 0;
	if (matching->event()) {
		item = createEventMatchingItem(matching);
	} else {
		success = false;
	}
	delete matching;
	matching = 0;
	if (!success) {
		boError() << k_funcinfo << "unable to load event matching " << i << endl;
		delete item;
		continue;
	}
	mEventMatchings->insertItem(item);
 }
}

void BoConditionEventsWidget::unselectEventMatching()
{
 if (mEventMatchings->currentItem() < 0) {
	return;
 }
 mEventMatchings->clearFocus();
 mEventMatchings->clearSelection();
 mEventMatchings->setCurrentItem(-1);
}

void BoConditionEventsWidget::updateEventMatching(const QDomElement& root, int index)
{
 if (index < 0 || index >= (int)mEventMatchings->count()) {
	return;
 }
 QDomNodeList eventMatchingList = mConditionDocument->documentElement().namedItem("Events").toElement().elementsByTagName("EventMatching");
 if (index >= (int)eventMatchingList.count()) {
	return;
 }
 BoEventMatching* matching = new BoEventMatching();
 bool success = matching->loadFromXML(root);
 if (success) {
	boDebug() << k_funcinfo << "updating event matching " << index << endl;

	QDomElement old = eventMatchingList.item(index).toElement();
	mConditionDocument->documentElement().namedItem("Events").replaceChild(root.cloneNode(), old);

	int currentItem = mEventMatchings->currentItem();
	mEventMatchings->blockSignals(true);
	mEventMatchings->changeItem(createEventMatchingItem(matching), index);
	mEventMatchings->setCurrentItem(currentItem);
	mEventMatchings->blockSignals(false);
 }
 delete matching;
 matching = 0;
 if (!success) {
	boError() << k_funcinfo << "received invalid event matching" << endl;
	return;
 }
}

QDomDocument BoConditionEventsWidget::eventsDocument()
{
 if (mEventMatchingWidget->isEnabled()) {
	mEventMatchingWidget->slotApply();
 }
 QDomDocument doc = mConditionDocument->cloneNode().toDocument();
 return doc;
}

QListBoxItem* BoConditionEventsWidget::createEventMatchingItem(const BoEventMatching* m)
{
 if (!m || !m->event()) {
	return new QListBoxText("");
 }
 return new QListBoxText(m->event()->name());
}

void BoConditionEventsWidget::slotSelectedEventMatching(int index)
{
 boDebug() << k_funcinfo << index << endl;
 if (index < 0) {
	return;
 }
 QDomElement root = mConditionDocument->documentElement();
 QDomElement events = root.namedItem("Events").toElement();
 QDomNodeList list = events.elementsByTagName("EventMatching");
 if (index >= (int)list.count()) {
	boError() << k_funcinfo << "have only " << list.count() << " event matchings, need at least " << index + 1 << endl;
	reloadEventMatchings();
	return;
 }
 QDomElement e = list.item(index).toElement();
 if (e.isNull()) {
	boError() << k_funcinfo << "item at " << index << " is not a valid element" << endl;
	reloadEventMatchings();
	return;
 }
 BoEventMatching* m = new BoEventMatching();
 bool success = m->loadFromXML(e);
 delete m;
 m = 0;
 if (!success) {
	boError() << k_funcinfo << "could not load event matching from internal document" << endl;
	KMessageBox::sorry(this, i18n("Loading the event matching from the internal XML document failed. cannot edit."));
	reloadEventMatchings();
	return;
 }
 slotEditEventMatching(index, e);
}

void BoConditionEventsWidget::updateEventMatching(int index, const BoEventMatching* m)
{
 mEventMatchings->changeItem(createEventMatchingItem(m), index);
}

void BoConditionEventsWidget::slotRemoveCurrentEventMatching()
{
 int current = mEventMatchings->currentItem();
 boDebug() << k_funcinfo << current << endl;
 if (current < 0) {
	return;
 }
 if (!mEventMatchings->item(current)->isSelected()) {
	// special case - the currenItem() is still > 0 in many cases, e.g. when
	// we add items to this listbox, then leave the listbox (selection is
	// cleared) and then (without selecting anything) call
	// slotRemoveCurrentEventMatching(), the previously selected item is
	// still the currentItem.
	return;
 }
 unselectEventMatching();
 slotEditEventMatching(current, QDomElement());

 QDomElement root = mConditionDocument->documentElement();
 QDomElement events = root.namedItem("Events").toElement();
 QDomNodeList list = root.elementsByTagName("EventMatching");
 if (current >= (int)list.count()) {
	return;
 }
 QDomElement e = list.item(current).toElement();
 events.removeChild(e);
 reloadEventMatchings();
}

void BoConditionEventsWidget::clearXML()
{
 QDomElement oldRoot = mConditionDocument->documentElement();
 mConditionDocument->removeChild(oldRoot);

 QDomElement root = mConditionDocument->createElement("Condition");
 QDomElement statusConditions = mConditionDocument->createElement("StatusConditions");
 QDomElement events = mConditionDocument->createElement("Events");
 root.appendChild(statusConditions);
 root.appendChild(events);
 mConditionDocument->appendChild(root);
}

bool BoConditionEventsWidget::loadCondition(const QDomElement& condition)
{
 if (condition.isNull()) {
	return false;
 }
 if (condition.namedItem("Events").toElement().isNull()) {
	return false;
 }
 if (condition.namedItem("Action").toElement().isNull()) {
	boError() << k_funcinfo << "no Action tag" << endl;
	return false;
 }
 clearXML();
 QDomElement root = mConditionDocument->documentElement();
 QDomElement events = root.namedItem("Events").toElement();
 QDomNodeList list = condition.namedItem("Events").toElement().elementsByTagName("EventMatching");
 for (unsigned int i = 0; i < list.count(); i++) {
	events.appendChild(list.item(i).cloneNode());
 }
 reloadEventMatchings();

 return true;
}

void BoConditionEventsWidget::slotEditEventMatching(int index, const QDomElement& m)
{
 if (mEventMatchingWidget->isEnabled()) {
	mEventMatchingWidget->slotApply();
 }
 unselectEventMatching();

 mCurrentEventMatchingIndex = index;
 mEventMatchings->blockSignals(true);
 mEventMatchings->setCurrentItem(index);
 mEventMatchings->setSelected(index, true);
 mEventMatchings->blockSignals(false);
 mEventMatchingWidget->displayEventMatching(m);
}

void BoConditionEventsWidget::slotEventMatchingUpdated(const QDomElement& root)
{
 updateEventMatching(root, mCurrentEventMatchingIndex);
}











BoEventMatchingWidget::BoEventMatchingWidget(QWidget* parent, bool eventMatching) : QWidget(parent)
{
 mIsEventMatching = eventMatching;

 QGridLayout* layout = new QGridLayout(this, 3, 5);

 QLabel* label;
 label = new QLabel(i18n("Event name"), this);
 mName = new QLineEdit(this);
 layout->addWidget(label, 0, 0);
 layout->addWidget(mName, 1, 0);

 label = new QLabel(i18n("Unit Id"), this);
 // FIXME: maybe we should use a QComboBox here
 mUnitId = new KIntNumInput(this);
 mIgnoreUnitId = new QCheckBox(i18n("Ignore"), this);
 connect(mIgnoreUnitId, SIGNAL(toggled(bool)),
		this, SLOT(slotIgnoreUnitIdChanged(bool)));
 layout->addWidget(label, 0, 1);
 layout->addWidget(mUnitId, 1, 1);
 layout->addWidget(mIgnoreUnitId, 2, 1);

 label = new QLabel(i18n("Player Id"), this);
 // FIXME: we should use a QComboBox here!
 mPlayerId = new KIntNumInput(this);
 mIgnorePlayerId = new QCheckBox(i18n("Ignore"), this);
 connect(mIgnorePlayerId, SIGNAL(toggled(bool)),
		this, SLOT(slotIgnorePlayerIdChanged(bool)));
 layout->addWidget(label, 0, 2);
 layout->addWidget(mPlayerId, 1, 2);
 layout->addWidget(mIgnorePlayerId, 2, 2);

 label = new QLabel(i18n("Data1"), this);
 mData1 = new QLineEdit(this);
 mIgnoreData1 = new QCheckBox(i18n("Ignore"), this);
 connect(mIgnoreData1, SIGNAL(toggled(bool)),
		this, SLOT(slotIgnoreData1Changed(bool)));
 layout->addWidget(label, 0, 3);
 layout->addWidget(mData1, 1, 3);
 layout->addWidget(mIgnoreData1, 2, 3);

 label = new QLabel(i18n("Data2"), this);
 mData2 = new QLineEdit(this);
 mIgnoreData2 = new QCheckBox(i18n("Ignore"), this);
 connect(mIgnoreData2, SIGNAL(toggled(bool)),
		this, SLOT(slotIgnoreData2Changed(bool)));
 layout->addWidget(label, 0, 4);
 layout->addWidget(mData2, 1, 4);
 layout->addWidget(mIgnoreData2, 2, 4);

 if (!mIsEventMatching) {
	mIgnoreUnitId->hide();
	mIgnorePlayerId->hide();
	mIgnoreData1->hide();
	mIgnoreData2->hide();
 }

 if (mIsEventMatching) {
	displayEventMatching(QDomElement());
 } else {
	// display a default event
	BoEvent e("CustomStringEvent", "Foobar");
	QDomDocument doc;
	QDomElement event = doc.createElement("Event");
	e.saveAsXML(event);
	displayEvent(event);
 }
}

BoEventMatchingWidget::~BoEventMatchingWidget()
{
}

bool BoEventMatchingWidget::displayEvent(const QDomElement& root)
{
 boDebug() << k_funcinfo << endl;

 if (root.isNull()) {
	setEnabled(false);
	return false;
 }
 QDomElement matching = root.ownerDocument().createElement("EventMatching");
 matching.setAttribute("IgnoreUnitId", false);
 matching.setAttribute("IgnorePlayerId", false);
 matching.setAttribute("IgnoreData1", false);
 matching.setAttribute("IgnoreData2", false);
 matching.appendChild(root.cloneNode());
 return displayEventMatching(matching);
}

bool BoEventMatchingWidget::displayEventMatching(const QDomElement& matching)
{
 boDebug() << k_funcinfo << endl;
 slotClear();

 BoEventMatching m;
 if (matching.isNull() || !m.loadFromXML(matching)) {
	setEnabled(false);
	return false;
 }
 setEnabled(true);
 mName->setText(m.event()->name());
 mUnitId->setValue(m.event()->unitId());
 mPlayerId->setValue(m.event()->playerId());
 mData1->setText(m.event()->data1());
 mData2->setText(m.event()->data2());

 mIgnoreUnitId->setChecked(m.ignoreUnitId());
 mIgnorePlayerId->setChecked(m.ignorePlayerId());
 mIgnoreData1->setChecked(m.ignoreData1());
 mIgnoreData2->setChecked(m.ignoreData2());

 slotIgnoreUnitIdChanged(mIgnoreUnitId->isChecked());
 slotIgnorePlayerIdChanged(mIgnorePlayerId->isChecked());
 slotIgnoreData1Changed(mIgnoreData1->isChecked());
 slotIgnoreData2Changed(mIgnoreData2->isChecked());
 return true;
}

void BoEventMatchingWidget::slotIgnoreUnitIdChanged(bool on)
{
 mUnitId->setEnabled(!on);
}

void BoEventMatchingWidget::slotIgnorePlayerIdChanged(bool on)
{
 mPlayerId->setEnabled(!on);
}

void BoEventMatchingWidget::slotIgnoreData1Changed(bool on)
{
 mData1->setEnabled(!on);
}

void BoEventMatchingWidget::slotIgnoreData2Changed(bool on)
{
 mData2->setEnabled(!on);
}

void BoEventMatchingWidget::slotClear()
{
 mName->setText("");
 mUnitId->setValue(0);
 mPlayerId->setValue(0);
 mData1->setText("");
 mData2->setText("");

 mIgnoreUnitId->setChecked(mIsEventMatching);
 mIgnorePlayerId->setChecked(mIsEventMatching);
 mIgnoreData1->setChecked(mIsEventMatching);
 mIgnoreData2->setChecked(mIsEventMatching);

 slotIgnoreUnitIdChanged(mIgnoreUnitId->isChecked());
 slotIgnorePlayerIdChanged(mIgnorePlayerId->isChecked());
 slotIgnoreData1Changed(mIgnoreData1->isChecked());
 slotIgnoreData2Changed(mIgnoreData2->isChecked());
}

QDomElement BoEventMatchingWidget::event() const
{
 QDomElement m = eventMatching();
 if (m.isNull()) {
	return m;
 }
 QDomElement event = m.namedItem("Event").toElement();
 return event.cloneNode().toElement();
}

QDomElement BoEventMatchingWidget::eventMatching() const
{
 QDomDocument doc;
 QDomElement root = doc.createElement("EventMatching");
 root.setAttribute("IsLeft", QString::number(1));

 doc.appendChild(root);
 root.setAttribute("IgnoreUnitId", mIgnoreUnitId->isChecked());
 root.setAttribute("IgnorePlayerId", mIgnorePlayerId->isChecked());
 root.setAttribute("IgnoreData1", mIgnoreData1->isChecked());
 root.setAttribute("IgnoreData2", mIgnoreData2->isChecked());

 QDomElement event = doc.createElement("Event");
 root.appendChild(event);
 BoEvent* saveEvent = new BoEvent(QCString(mName->text()), mData1->text(), mData2->text());
 saveEvent->setUnitId(mUnitId->value()); // even if mIgnoreUnitId is checked!
 if (!mIgnorePlayerId->isChecked() && mPlayerId->value() != 0) {
	// note that 0 is an invalid ID!
	saveEvent->setPlayerId(mPlayerId->value());
 }
 bool success = saveEvent->saveAsXML(event);
 delete saveEvent;
 saveEvent = 0;
 if (!success) {
	boError() << k_funcinfo << "could not save event" << endl;
	return QDomElement();
 }

 BoEventMatching* test = new BoEventMatching();
 success = test->loadFromXML(root);
 delete test;
 test = 0;
 if (!success) {
	boError() << k_funcinfo << "could not load event matching" << endl;
	return QDomElement();
 }

 return root;
}

void BoEventMatchingWidget::slotApply()
{
 boDebug() << k_funcinfo << endl;

 QDomElement root = eventMatching();
 if (root.isNull()) {
	boError() << k_funcinfo << "null element" << endl;
	KMessageBox::sorry(this, i18n("An error occured while saving the event matching"));
	return;
 }
 QDomElement event = root.namedItem("Event").toElement();
 if (event.isNull()) {
	boError() << k_funcinfo << "EventMatching has no Event tag" << endl;
	KMessageBox::sorry(this, i18n("An error occured while saving the event matching. No event was saved."));
	return;
 }

 emit signalEvent(event);
 emit signalEventMatching(root);
}

