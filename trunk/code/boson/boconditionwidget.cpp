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

#include "bo3dtools.h"
#include "boeventmatching.h"
#include "boevent.h"
#include "bodebug.h"

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

#include <klistbox.h>
#include <klocale.h>
#include <knuminput.h>
#include <kmessagebox.h>

class BoConditionWidgetPrivate
{
public:
	BoConditionWidgetPrivate()
	{
		mConditionsWidget = 0;
		mEventMatchingWidget = 0;
		mCurrentEventMatchingCondition = 0;
	}

	QVBox* mConditionsWidget;
	QPtrList<BoOneConditionWidget> mConditions;

	BoEventMatchingWidget* mEventMatchingWidget;

	BoOneConditionWidget* mCurrentEventMatchingCondition;
	int mCurrentEventMatchingIndex;
};

BoConditionWidget::BoConditionWidget(QWidget* parent) : QWidget(parent)
{
 d = new BoConditionWidgetPrivate;
 d->mConditions.setAutoDelete(true);
 d->mCurrentEventMatchingIndex = -1;

 QVBoxLayout* layout = new QVBoxLayout(this);

 QVGroupBox* conditions = new QVGroupBox(i18n("Conditions"), this);
 layout->addWidget(conditions);

 d->mConditionsWidget = new QVBox(conditions);

 QHBox* moreLessWidget = new QHBox(conditions);
 QPushButton* more = new QPushButton(i18n("More"), moreLessWidget);
 connect(more, SIGNAL(clicked()), this, SLOT(slotAddCondition()));

 QPushButton* less = new QPushButton(i18n("Fewer"), moreLessWidget);
 connect(less, SIGNAL(clicked()), this, SLOT(slotRemoveCondition()));

 d->mEventMatchingWidget = new BoEventMatchingWidget(this);
 layout->addWidget(d->mEventMatchingWidget);
 connect(d->mEventMatchingWidget, SIGNAL(signalEventMatching(const QDomElement&)),
		this, SLOT(slotEventMatchingUpdated(const QDomElement&)));
 d->mEventMatchingWidget->setEnabled(false);


 slotAddCondition();
}

BoConditionWidget::~BoConditionWidget()
{
 d->mConditions.clear();
 delete d;
}

void BoConditionWidget::slotAddCondition()
{
 BoOneConditionWidget* condition = new BoOneConditionWidget(d->mConditionsWidget);
 connect(condition, SIGNAL(signalEditEventMatching(BoOneConditionWidget*, int, const QDomElement&)),
		this, SLOT(slotEditEventMatching(BoOneConditionWidget*, int, const QDomElement&)));
 condition->show();

 d->mConditions.append(condition);
}

void BoConditionWidget::slotRemoveCondition()
{
 if (d->mConditions.count() <= 1) {
	return;
 }
 d->mConditions.removeLast();
}

void BoConditionWidget::slotEditEventMatching(BoOneConditionWidget* c, int index, const QDomElement& m)
{
 if (d->mEventMatchingWidget->isEnabled()) {
	d->mEventMatchingWidget->slotApply();
 }
 QPtrListIterator<BoOneConditionWidget> it(d->mConditions);
 while (it.current()) {
	if (it.current() == c) {
		++it;
		continue;
	}
	it.current()->unselectEventMatching();
	++it;
 }

 d->mEventMatchingWidget->display(m);
 d->mCurrentEventMatchingCondition = c;
 d->mCurrentEventMatchingIndex = index;
}

void BoConditionWidget::slotEventMatchingUpdated(const QDomElement& root)
{
 // AB: we use this loop to make sure that mCurrentEventMatchingCondition is
 // actually a valid pointer
 QPtrListIterator<BoOneConditionWidget> it(d->mConditions);
 while (it.current()) {
	if (it.current() != d->mCurrentEventMatchingCondition) {
		++it;
		continue;
	}
	it.current()->updateEventMatching(root, d->mCurrentEventMatchingIndex);
	++it;
 }
}

QString BoConditionWidget::toString()
{
 QDomDocument doc("Conditions");
 QDomElement root = doc.createElement("Conditions");
 doc.appendChild(root);

 d->mEventMatchingWidget->slotApply();

 QPtrListIterator<BoOneConditionWidget> it(d->mConditions);
 while (it.current()) {
	QDomElement e = it.current()->element();
	root.appendChild(e.cloneNode());
	++it;
 }
 return doc.toString();
}

bool BoConditionWidget::loadConditions(const QDomElement& root)
{
 QDomNodeList list = root.elementsByTagName("Condition");
 for (unsigned int i = 0; i < list.count(); i++) {
	if (d->mConditions.count() < i + 1) {
		slotAddCondition();
	}
	QDomElement e = list.item(i).toElement();
	BoOneConditionWidget* c = d->mConditions.at(i);
	if (!c->loadCondition(e)) {
		return false;
	}
 }
 return true;
}


BoOneConditionWidget::BoOneConditionWidget(QWidget* parent) : QWidget(parent)
{
 // AB: the BoConditionWidget should have labels above all of these.
 // maybe we should add a bool isFirstWidget flag to BoOneConditionWidget and
 // display that label if the flag is true

 QHBoxLayout* layout = new QHBoxLayout(this);

 mConditionDocument = new QDomDocument("Condition");
 clearXML();

 // LABEL: "For player"
 mForPlayer = new QComboBox(this);
 int playerCount = 0; // TODO
 for (int i = 0; i < playerCount; i++) {
	QString playerName; // TODO
	mForPlayer->insertItem(i18n("For %1").arg(playerName));
 }

 // this is equal to creating playerCount conditions, one for each player in the
 // game.
 // actually this is exactly what this will do.
 // LABEL: Event Matchings
 mForPlayer->insertItem("For all players");

 mEventMatchings = new KListBox(this);
 connect(mEventMatchings, SIGNAL(highlighted(int)),
		this, SLOT(slotSelectedEventMatching(int)));

 QVBox* eventMatchingButtonsBox = new QVBox(this);
 mAddMatching = new QPushButton(i18n("Add"), eventMatchingButtonsBox);
 mRemoveMatching = new QPushButton(i18n("Remove"), eventMatchingButtonsBox);
 connect(mAddMatching, SIGNAL(clicked()),
		this, SLOT(slotAddEventMatching()));
 connect(mRemoveMatching, SIGNAL(clicked()),
		this, SLOT(slotRemoveCurrentEventMatching()));

 // atm we will always emit "CustomStringEvent" with one parameter
 // LABEL: Action (parameter to event)
 mAction = new QLineEdit(this);

 layout->addWidget(mForPlayer);
 layout->addWidget(mEventMatchings);
 layout->addWidget(eventMatchingButtonsBox);
 layout->addWidget(mAction);
}

BoOneConditionWidget::~BoOneConditionWidget()
{
 delete mConditionDocument;
}

void BoOneConditionWidget::updateEventMatching(const QDomElement& root, int index)
{
 if (index < 0 || index >= (int)mEventMatchings->count()) {
	return;
 }
 QDomNodeList eventMatchingList = mConditionDocument->documentElement().namedItem("Events").toElement().elementsByTagName("EventMatching");
 if (index >= (int)eventMatchingList.count()) {
	return;
 }
 BoEventMatching* matching = new BoEventMatching();
 bool success = matching->load(root);
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

QDomElement BoOneConditionWidget::element()
{
 QDomElement root = mConditionDocument->documentElement();
 root.setAttribute("EventCaused", mAction->text());
 return root;
}

QListBoxItem* BoOneConditionWidget::createEventMatchingItem(const BoEventMatching* m)
{
 if (!m || !m->event()) {
	return new QListBoxText("");
 }
 return new QListBoxText(m->event()->name());
}

void BoOneConditionWidget::updateEventMatching(int index, const BoEventMatching* m)
{
 mEventMatchings->changeItem(createEventMatchingItem(m), index);
}

void BoOneConditionWidget::reloadEventMatchings()
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
	bool success = matching->load(e);
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
		continue;
	}
	mEventMatchings->insertItem(item);
 }
}

void BoOneConditionWidget::unselectEventMatching()
{
 if (mEventMatchings->currentItem() < 0) {
	return;
 }
 mEventMatchings->clearFocus();
 mEventMatchings->clearSelection();
 mEventMatchings->setCurrentItem(-1);
}

void BoOneConditionWidget::slotSelectedEventMatching(int index)
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
 bool success = m->load(e);
 delete m;
 m = 0;
 if (!success) {
	boError() << k_funcinfo << "could not load event matching from internal document" << endl;
	KMessageBox::sorry(this, i18n("Loading the event matching from the internal XML document failed. cannot edit."));
	reloadEventMatchings();
	return;
 }
 emit signalEditEventMatching(this, index, e);
}

void BoOneConditionWidget::slotAddEventMatching()
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
 bool success = saveEvent->save(event, 0);
 delete saveEvent;
 saveEvent = 0;
 if (!success) {
	boError() << k_funcinfo << "could not save dummy event" << endl;
	KMessageBox::sorry(this, i18n("Unable to save a dummy event. Cannot add an event matching."));
	return;
 }
 BoEventMatching* matching = new BoEventMatching();
 success = matching->load(eventMatching);
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

void BoOneConditionWidget::slotRemoveCurrentEventMatching()
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
 emit signalEditEventMatching(this, current, QDomElement());

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

void BoOneConditionWidget::clearXML()
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

bool BoOneConditionWidget::loadCondition(const QDomElement& condition)
{
 clearXML();
 QDomElement root = mConditionDocument->documentElement();
 QDomElement events = root.namedItem("Events").toElement();
 mAction->setText(condition.attribute("EventCaused"));
 QDomNodeList list = condition.namedItem("Events").toElement().elementsByTagName("EventMatching");
 for (unsigned int i = 0; i < list.count(); i++) {
	events.appendChild(list.item(i).cloneNode());
 }
 reloadEventMatchings();
 return true;
}


BoEventMatchingWidget::BoEventMatchingWidget(QWidget* parent) : QWidget(parent)
{
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

 display(QDomElement());
}

BoEventMatchingWidget::~BoEventMatchingWidget()
{
}

void BoEventMatchingWidget::display(const QDomElement& matching)
{
 boDebug() << k_funcinfo << endl;
 slotClear();
 BoEventMatching m;
 if (matching.isNull() || !m.load(matching)) {
	setEnabled(false);
	return;
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

 mIgnoreUnitId->setChecked(true);
 mIgnorePlayerId->setChecked(true);
 mIgnoreData1->setChecked(true);
 mIgnoreData2->setChecked(true);

 slotIgnoreUnitIdChanged(mIgnoreUnitId->isChecked());
 slotIgnorePlayerIdChanged(mIgnorePlayerId->isChecked());
 slotIgnoreData1Changed(mIgnoreData1->isChecked());
 slotIgnoreData2Changed(mIgnoreData2->isChecked());
}

void BoEventMatchingWidget::slotApply()
{
 boDebug() << k_funcinfo << endl;
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
 if (!mIgnorePlayerId->isChecked()) {
	saveEvent->setPlayerId(mPlayerId->value());
 }
 bool success = saveEvent->save(event, 0);
 delete saveEvent;
 saveEvent = 0;
 if (!success) {
	boError() << k_funcinfo << "could not save event" << endl;
	KMessageBox::sorry(this, i18n("An error occured while saving the event of the event matching"));
	return;
 }

 BoEventMatching* test = new BoEventMatching();
 success = test->load(root);
 delete test;
 test = 0;
 if (!success) {
	boError() << k_funcinfo << "could not load event matching" << endl;
	KMessageBox::sorry(this, i18n("An error occured while saving the event matching"));
	return;
 }

 emit signalEventMatching(root);
}

