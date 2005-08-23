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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "boconditioneditormain.h"
#include "boconditioneditormain.moc"

#include "../bomemory/bodummymemory.h"
#include "bodebugdcopiface.h"
#include "bodebug.h"
#include "boversion.h"
#include "boapplication.h"
#include "boconditionwidget.h"

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <ktar.h>
#include <kmessagebox.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qptrstack.h>
#include <qdom.h>

static const char *description =
    I18N_NOOP("BoCondition editor for Boson");

static const char *version = BOSON_VERSION_STRING;

static KCmdLineOptions options[] =
{
    { "+[FILE]", I18N_NOOP(".bpf or .bsg file to open."), 0},
    { 0, 0, 0 }
};

int main(int argc, char **argv)
{
 KAboutData about("boconditioneditor",
		I18N_NOOP("Boson condition editor"),
		version,
		description,
		KAboutData::License_GPL,
		"(C) 2005 Andreas Beckermann",
		0,
		"http://boson.eu.org");
 about.addAuthor( "Andreas Beckermann", I18N_NOOP("Coding & Current Maintainer"), "b_mann@gmx.de" );

 QCString argv0(argv[0]);
 KCmdLineArgs::init(argc, argv, &about);
 KCmdLineArgs::addCmdLineOptions(options);
 BoApplication app(argv0);

    // register ourselves as a dcop client
//    app.dcopClient()->registerAs(app.name(), false);

 KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
 QObject::connect(kapp, SIGNAL(lastWindowClosed()), kapp, SLOT(quit()));

 BoConditionEditorMain* w = new BoConditionEditorMain();
 w->show();
 app.setMainWidget(w);

 if (args->count() > 0) {
	KURL url = args->url(0);
	QString file = url.directory(false) + url.fileName();
	w->slotLoadFile(file);
 }

 BoDebugDCOPIface* iface = new BoDebugDCOPIface();
 args->clear();
 int r = app.exec();
 delete iface;
 return r;
}

BoConditionEditorMain::BoConditionEditorMain()
	: QWidget(0)
{
 mFile = 0;

 QVBoxLayout* topLayout = new QVBoxLayout(this);

 QHBoxLayout* hlayout = new QHBoxLayout(topLayout);
 QLabel* fileLabel = new QLabel(i18n("Current file:"), this);
 mFileName = new QLabel(this);
 hlayout->addWidget(fileLabel);
 hlayout->addWidget(mFileName);

 QVBoxLayout* vlayout = new QVBoxLayout(0);
 QLabel* conditionLabel = new QLabel(i18n("Available conditions:"), this);
 mConditions = new QListBox(this);
 connect(mConditions, SIGNAL(selected(int)),
		this, SLOT(slotEditConditions()));
 vlayout->addWidget(conditionLabel);
 vlayout->addWidget(mConditions);
 topLayout->addLayout(vlayout, 1);

 mEditConditions = new QPushButton(i18n("Edit conditions"), this);
 connect(mEditConditions, SIGNAL(clicked()),
		this, SLOT(slotEditConditions()));
 mEditConditions->setEnabled(false);
 topLayout->addWidget(mEditConditions);

 mSelectFile = new QPushButton(i18n("File..."), this);
 connect(mSelectFile, SIGNAL(clicked()),
		this, SLOT(slotSelectFile()));
 topLayout->addWidget(mSelectFile);

 reset();
}

BoConditionEditorMain::~BoConditionEditorMain()
{
}

void BoConditionEditorMain::slotSelectFile()
{
 QString file = KFileDialog::getOpenFileName(QString::null, "*.bpf *.bsg", this);
 if (file.isEmpty()) {
	return;
 }
 slotLoadFile(file);
}

void BoConditionEditorMain::slotLoadFile(const QString& file)
{
 reset();
 mFile = new KTar(file, QString::fromLatin1("application/x-gzip"));
 if (!mFile->open(IO_ReadOnly)) {
	KMessageBox::sorry(this, i18n("Could not open %1 for reading").arg(file));
	mFile->close();
	reset();
	return;
 }
 const KArchiveDirectory* topLevelDir = 0;
 if (mFile->directory()) {
	if (mFile->directory()->entries().count() == 1) {
		const KArchiveEntry* entry;
		entry = mFile->directory()->entry(mFile->directory()->entries()[0]);
		if (entry && entry->isDirectory()) {
			topLevelDir = (const KArchiveDirectory*)entry;
		}
	}
 }
 if (!topLevelDir) {
	KMessageBox::sorry(this, i18n("Error while reading %1. Cannot retrieve toplevel directory.").arg(file));
	mFile->close();
	reset();
	return;
 }

 QPtrStack<const KArchiveDirectory> dirs;
 dirs.push(topLevelDir);
 while (!dirs.isEmpty()) {
	const KArchiveDirectory* dir = dirs.pop();
	QStringList entries = dir->entries();
	for (QStringList::iterator it = entries.begin(); it != entries.end(); ++it) {
		const KArchiveEntry* e = dir->entry(*it);
		if (!e) {
			continue;
		}
		if (e->isDirectory()) {
			dirs.push((const KArchiveDirectory*)e);
		} else if (e->isFile()) {
			const KArchiveFile* f = (const KArchiveFile*)e;
			if ((*it).endsWith(".xml")) {
				if (!loadXMLFile(f)) {
					KMessageBox::sorry(this, i18n("File format error in %1: Loading %2 failed.").arg(file).arg(*it));
					reset();
					return;
				}
				if ((*it) == "players.xml") {
					if (!parsePlayerIds(f)) {
						KMessageBox::sorry(this, i18n("File format error in %1: Parsing PlayerIds in %2 failed. Maybe the file format was too old.").arg(file).arg(*it));
						reset();
						return;
					}
				}
			}
		}
	}
 }

 if (mItem2Element.count() == 0) {
	KMessageBox::information(this, i18n("The file was loaded successfully, but no XML file with EventListener tags was found. Cannot edit any Conditions in this file. Maybe the file format was too old - Boson 0.10 and before did not support Conditions."));
 }

 mFileName->setText(file);
 mEditConditions->setEnabled(true);
}

void BoConditionEditorMain::slotEditConditions()
{
 int index = mConditions->currentItem();
 if (index < 0) {
	return;
 }
 QListBoxItem* item = mConditions->item(index);
 if (!item) {
	return;
 }
 if (!mItem2Element.contains(item)) {
	boError() << k_funcinfo << "unexpected item" << endl;
	return;
 }
 QDomElement e = mItem2Element[item];
 if (e.isNull()) {
	boError() << k_funcinfo << "NULL element" << endl;
	return;
 }
 if (mItem2Widget.contains(item)) {
	BoConditionWidget* widget = (BoConditionWidget*)mItem2Widget[item];
	int r = KMessageBox::questionYesNo(this, i18n("The conditions for this EventListener have been loaded already. Do you want to reload?"));
	if (r != KMessageBox::Yes) {
		widget->show();
		widget->raise();
		widget->setActiveWindow();
		return;
	}
	delete widget;
	mItem2Widget.remove(item);
 }
 BoConditionWidget* widget = new BoConditionWidget(0);
 QDomElement conditions = e.namedItem("Conditions").toElement(); // AB: NULL is valid
 if (!widget->loadConditions(conditions)) {
	boError() << k_funcinfo << "cannot load conditions" << endl;
	KMessageBox::sorry(this, i18n("Unable to load conditions"));
	delete widget;
	return;
 }
 mItem2Widget.insert(item, widget);
 widget->show();
}

void BoConditionEditorMain::reset()
{
 delete mFile;
 mFile = 0;
 mFileName->setText(i18n("No file loaded"));
 mEditConditions->setEnabled(false);
 mConditions->clear();
 mItem2Element.clear();
 for (QMap<QListBoxItem*,QWidget*>::iterator it = mItem2Widget.begin(); it != mItem2Widget.end(); ++it) {
	delete it.data();
 }
 mItem2Widget.clear();
 mPlayerIds.clear();
}

bool BoConditionEditorMain::loadXMLFile(const KArchiveFile* file)
{
 if (!file) {
	BO_NULL_ERROR(file);
	return false;
 }
 QString xml = file->data(); // conversion to QString intended! (same as in Boson code)
 if (xml.length() == 0) {
	return true;
 }
 QDomDocument doc;
 if (!doc.setContent(xml)) {
	boError() << k_funcinfo << "cannot load xml from " << file->name() << endl;
	return false;
 }

 QDomElement root = doc.documentElement();

 // AB: only EventListeners have Condition tags.
 //     we do not search for Condition tags, because we can have a nicer GUI if
 //     we can state explicitly that this is an EventListener.
 //     -> this also means that the code must be adapted if non-EventListeners
 //        can ever have Conditions.
 QDomNodeList list = root.elementsByTagName("EventListener");
 if (list.count() > 1) {
	boWarning() << k_funcinfo << "only one EventListener tag supported currently! file " << file->name() << " has " << list.count() << " EventListener tags" << endl;
 }
 if (list.count() >= 1) {
	QDomElement e = list.item(0).toElement();
	if (e.isNull()) {
		boError() << k_funcinfo << "NULL element?!" << endl;
		return false;
	}
	QString name = i18n("%1").arg(file->name());
	if (file->name() == "canvas.xml") {
		name = i18n("Global conditions (%1)").arg(file->name());
	}
	QListBoxText* item = new QListBoxText(mConditions, name);
	mItem2Element.insert(item, e.cloneNode().toElement());
 }

 return true;
}

bool BoConditionEditorMain::parsePlayerIds(const KArchiveFile* file)
{
 if (!file) {
	BO_NULL_ERROR(file);
	return false;
 }
 QString xml = file->data(); // conversion to QString intended! (same as in Boson code)
 if (xml.length() == 0) {
	return true;
 }
 QDomDocument doc;
 if (!doc.setContent(xml)) {
	boError() << k_funcinfo << "cannot load xml from " << file->name() << endl;
	return false;
 }

 QValueList<unsigned long int> ids;
 QDomElement root = doc.documentElement();
 for (QDomNode n = root.firstChild(); !n.isNull(); n = n.nextSibling()) {
	QDomElement p = n.toElement();
	if (p.isNull() || p.tagName() != "Player") {
		continue;
	}
	if (!p.hasAttribute("PlayerId")) {
		boError() << k_funcinfo << "no PlayerId attribute" << endl;
		return false;
	}
	bool ok;
	unsigned long int id = p.attribute("PlayerId").toULong(&ok);
	if (!ok) {
		boError() << k_funcinfo << "value of PlayerId attribute is not a valid number" << endl;
		return false;
	}
	ids.append(id);
 }
 if (ids.count() < 2) {
	// 2 are always present: one player and one neutral player.
	boError() << k_funcinfo << "less than 2 PlayerIds found" << endl;
	return false;
 }
 mPlayerIds = ids;
 return true;
}

