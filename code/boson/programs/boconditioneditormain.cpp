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

#include "boconditioneditormain.h"
#include "boconditioneditormain.moc"

#include "../bomemory/bodummymemory.h"
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
#include <q3listbox.h>
#include <q3ptrstack.h>
#include <qdom.h>
#include <qfile.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3CString>
#include <Q3ValueList>
#include <Q3VBoxLayout>

static KLocalizedString description =
    ki18n("BoCondition editor for Boson");

static const char *version = BOSON_VERSION_STRING;

int main(int argc, char **argv)
{
 KAboutData about("boconditioneditor",
		QByteArray(),
		ki18n(("Boson condition editor")),
		version,
		description,
		KAboutData::License_GPL,
		ki18n("(C) 2005 Andreas Beckermann"),
		KLocalizedString(),
		"http://boson.eu.org");
 about.addAuthor(ki18n("Andreas Beckermann"), ki18n("Coding & Current Maintainer"), "b_mann@gmx.de" );

 KCmdLineOptions options;
 options.add("+[FILE]", ki18n(".bpf or .bsg file to open."));

 Q3CString argv0(argv[0]);
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
	KUrl url = args->url(0);
	QString file = url.directory(false) + url.fileName();
	w->slotLoadFile(file);
 }

 args->clear();
 int r = app.exec();
 return r;
}

BoConditionEditorMain::BoConditionEditorMain()
	: QWidget(0)
{
 mFile = 0;

 Q3VBoxLayout* topLayout = new Q3VBoxLayout(this, 5, 5);

 Q3HBoxLayout* hlayout = new Q3HBoxLayout(topLayout);
 QLabel* fileLabel = new QLabel(i18n("Current file:"), this);
 mFileName = new QLabel(this);
 hlayout->addWidget(fileLabel);
 hlayout->addWidget(mFileName);

 Q3VBoxLayout* vlayout = new Q3VBoxLayout(0);
 QLabel* conditionLabel = new QLabel(i18n("Available conditions:"), this);
 mConditions = new Q3ListBox(this);
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

 mSelectSaveFile = new QPushButton(i18n("Save as..."), this);
 connect(mSelectSaveFile, SIGNAL(clicked()),
		this, SLOT(slotSelectSaveFile()));
 topLayout->addWidget(mSelectSaveFile);
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
 QString file = KFileDialog::getOpenFileName(QString(), "*.bpf *.bsg", this);
 if (file.isEmpty()) {
	return;
 }
 slotLoadFile(file);
}

void BoConditionEditorMain::slotLoadFile(const QString& file)
{
 reset();
 mFile = new KTar(file, QString::fromLatin1("application/x-gzip"));
 if (!mFile->open(QIODevice::ReadOnly)) {
	KMessageBox::sorry(this, i18n("Could not open %1 for reading", file));
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
	KMessageBox::sorry(this, i18n("Error while reading %1. Cannot retrieve toplevel directory.", file));
	mFile->close();
	reset();
	return;
 }

 Q3PtrStack<const KArchiveDirectory> dirs;
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
					KMessageBox::sorry(this, i18n("File format error in %1: Loading %2 failed.", file, *it));
					reset();
					return;
				}
				if ((*it) == "players.xml") {
					if (!parsePlayerIds(f)) {
						KMessageBox::sorry(this, i18n("File format error in %1: Parsing PlayerIds in %2 failed. Maybe the file format was too old.", file, *it));
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
 mSelectSaveFile->setEnabled(true);
}

void BoConditionEditorMain::slotEditConditions()
{
 int index = mConditions->currentItem();
 if (index < 0) {
	return;
 }
 Q3ListBoxItem* item = mConditions->item(index);
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
 e = e.cloneNode().toElement();
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
 mSelectSaveFile->setEnabled(false);
 mConditions->clear();
 mItem2Element.clear();
 for (QMap<Q3ListBoxItem*,QWidget*>::iterator it = mItem2Widget.begin(); it != mItem2Widget.end(); ++it) {
	delete it.data();
 }
 mItem2Widget.clear();
 mPlayerIds.clear();
 mFile2XML.clear();
 mFile2Item.clear();
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
	QString name = i18n("%1", file->name());
	if (file->name() == "canvas.xml") {
		name = i18n("Global conditions (%1)", file->name());
	}
	Q3ListBoxText* item = new Q3ListBoxText(mConditions, name);
	mItem2Element.insert(item, e);
	mFile2XML.insert(file, doc);
	mFile2Item.insert(file, item);
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

 Q3ValueList<unsigned long int> ids;
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

void BoConditionEditorMain::slotSelectSaveFile()
{
 QString file = KFileDialog::getSaveFileName(QString(), "*.bpf *.bsg", this);
 if (file.isEmpty()) {
	return;
 }
 if (QFile::exists(file)) {
	int r = KMessageBox::questionYesNo(this, i18n("The file %1 already exists. Overwrite?", file));
	if (r != KMessageBox::Yes) {
		return;
	}
 }
 slotSaveFile(file);
}

void BoConditionEditorMain::slotSaveFile(const QString& fileName)
{
 if (fileName.isEmpty()) {
	return;
 }
 if (!mFile) {
	return;
 }
 KTar save(fileName, QString::fromLatin1("application/x-gzip"));
 if (!save.open(QIODevice::WriteOnly)) {
	KMessageBox::sorry(this, i18n("Could not open %1 for saving", fileName));
	return;
 }
 const KArchiveDirectory* fromRoot = mFile->directory();
 if (!fromRoot) {
	BO_NULL_ERROR(fromRoot);
	return;
 }
 const KArchiveEntry* e = 0;
 if (fromRoot->entries().count() != 1) {
	boError() << k_funcinfo << "not exactly one toplevel entry" << endl;
	return;
 }
 e = fromRoot->entry(fromRoot->entries()[0]);
 if (!e->isDirectory()) {
	boError() << k_funcinfo << "toplevel entry is not a directory" << endl;
	return;
 }
 if (!saveFile(&save, e->name(), (const KArchiveDirectory*)e)) {
	KMessageBox::sorry(this, i18n("Saving failed"));
	save.close();
	return;
 }
 save.close();
}

bool BoConditionEditorMain::saveFile(KTar* save, const QString& path, const KArchiveDirectory* from)
{
 if (!save || !from) {
	return false;
 }
 QStringList entries = from->entries();
 for (QStringList::iterator it = entries.begin(); it != entries.end(); ++it) {
	const KArchiveEntry* e = from->entry(*it);
	if (!e) {
		BO_NULL_ERROR(e);
		return false;
	}
	QString fullName = path + "/" + e->name();
	if (e->isDirectory()) {
		bool ret = saveFile(save, fullName, (const KArchiveDirectory*)e);
		if (!ret) {
			boError() << k_funcinfo << "error saving " << e->name() << " in " << path << endl;
			return false;
		}
	} else if (e->isFile()) {
		const KArchiveFile* f = (const KArchiveFile*)e;
		if (mFile2XML.contains(f)) {
			QDomDocument doc = mFile2XML[f];
			if (doc.documentElement().isNull()) {
				boError() << k_funcinfo << "NULL root element" << endl;
				return false;
			}
			if (mFile2Item.contains(f)) {
				Q3ListBoxItem* item = mFile2Item[f];
				QDomElement oldConditions;
				QDomElement newConditions;
				if (mItem2Element.contains(item)) {
					oldConditions = mItem2Element[item];
				}
				if (mItem2Widget.contains(item)) {
					BoConditionWidget* widget = (BoConditionWidget*)mItem2Widget[item];
					QDomDocument conditionDoc = widget->conditionsDocument();
					newConditions = conditionDoc.documentElement().cloneNode().toElement();
					if (newConditions.isNull()) {
						boError() << k_funcinfo << "NULL newConditions" << endl;
						return false;
					}
				}
				if (!oldConditions.isNull()) {
					oldConditions.parentNode().replaceChild(newConditions, oldConditions);
				}
			}

			QByteArray xml = doc.toByteArray();
			if (!save->writeFile(fullName, from->user(), from->group(), xml.data(), xml.length())) {
				boError() << k_funcinfo << "could not save " << fullName << endl;
				return false;
			}
		} else {
			if (!save->writeFile(fullName, from->user(), from->group(), f->data(), f->size())) {
				boError() << k_funcinfo << "could not save " << fullName << endl;
				return false;
			}
		}
	} else {
		boError() << k_funcinfo << fullName << " has unexpected type" << endl;
		return false;
	}
 }
 return true;
}


