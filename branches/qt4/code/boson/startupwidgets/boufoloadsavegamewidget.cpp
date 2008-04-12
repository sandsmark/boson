/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "boufoloadsavegamewidget.h"
#include "boufoloadsavegamewidget.moc"

#include "../../bomemory/bodummymemory.h"
#include "bodebug.h"
#include "../bofiledialog.h"

#include <qdir.h>
#include <qdatetime.h>
#include <qtimer.h>
//Added by qt3to4:
#include <Q3PtrList>

#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <KComponentData>

class BoUfoLoadSaveGameWidgetPrivate
{
public:
	BoUfoLoadSaveGameWidgetPrivate()
	{
		mLoadSaveButton = 0;
		mDeleteButton = 0;

		mButtonWidget = 0;
		mSelectedGame = 0;
		mNoGamesLabel = 0;
	}

	QDir mDir;
	bool mSave;
	QString mSuffix;

	BoUfoPushButton* mLoadSaveButton;
	BoUfoPushButton* mDeleteButton;

	BoUfoWidget* mButtonWidget;
	Q3PtrList<BoUfoSaveGameWidget> mButtons;
	BoUfoSaveGameWidget* mSelectedGame;
	BoUfoLabel* mNoGamesLabel;
};

BoUfoLoadSaveGameWidget::BoUfoLoadSaveGameWidget(bool save, const QString& suffix, const QString& dir)
		: BoUfoWidget()
{
 init();
 setSaveMode(save);
 setDirectory(dir);
 setSuffix(suffix);
 updateGames();
}

void BoUfoLoadSaveGameWidget::init()
{
 d = new BoUfoLoadSaveGameWidgetPrivate;
 d->mSave = true; // default. this is for the case that we use it before setSaveMode() gets called
 d->mDir.setFilter(QDir::Files | QDir::Readable);

#warning TODO: scroll widget
#if 0
 Q3ScrollView* scroll = new Q3ScrollView(this, "scrollwidget");
 scroll->setResizePolicy(Q3ScrollView::AutoOneFit);
 scroll->setHScrollBarMode(Q3ScrollView::AlwaysOff);
 scroll->setVScrollBarMode(Q3ScrollView::AlwaysOn);
 d->mButtonWidget = new QWidget(scroll->viewport());
 scroll->addChild(d->mButtonWidget);
#else
 BoUfoWidget* scroll = new BoUfoWidget();
 scroll->setLayoutClass(UVBoxLayout);
 scroll->setStretch(1);
 addWidget(scroll);

 BoUfoWidget* topButtonWidget = new BoUfoWidget();
 scroll->addWidget(topButtonWidget);

 d->mButtonWidget = new BoUfoWidget();
 d->mButtonWidget->setLayoutClass(UVBoxLayout);
 topButtonWidget->addWidget(d->mButtonWidget);

 BoUfoWidget* stretch = new BoUfoWidget();
 stretch->setStretch(1);
 topButtonWidget->addWidget(stretch);
#endif



 BoUfoHBox* bottom = new BoUfoHBox();
 addWidget(bottom);

 d->mDeleteButton = new BoUfoPushButton(i18n("&Delete"));
 connect(d->mDeleteButton, SIGNAL(signalClicked()), this, SLOT(slotDelete()));

 BoUfoPushButton* browse = new BoUfoPushButton(i18n("&Browse..."));
 connect(browse, SIGNAL(signalClicked()), this, SLOT(slotBrowse()));

 d->mLoadSaveButton = new BoUfoPushButton();
 connect(d->mLoadSaveButton, SIGNAL(signalClicked()), this, SLOT(slotLoadSaveTimer()));

 BoUfoPushButton* cancel = new BoUfoPushButton(i18n("&Cancel"));
 connect(cancel, SIGNAL(signalClicked()), this, SLOT(slotCancel()));

 bottom->addWidget(d->mDeleteButton);
 stretch = new BoUfoWidget();
 stretch->setStretch(1);
 bottom->addWidget(stretch);
 bottom->addWidget(browse);
 stretch = new BoUfoWidget();
 stretch->setStretch(1);
 bottom->addWidget(stretch);
 bottom->addWidget(d->mLoadSaveButton);
 stretch = new BoUfoWidget();
 stretch->setStretch(1);
 bottom->addWidget(stretch);
 bottom->addWidget(cancel);

 setDefaultDir();
 setDefaultSuffix();
 setSaveMode(true);
}

BoUfoLoadSaveGameWidget::~BoUfoLoadSaveGameWidget()
{
 delete d;
}

void BoUfoLoadSaveGameWidget::slotCancel()
{
 QTimer::singleShot(0, this, SIGNAL(signalCancel()));
}

QString BoUfoLoadSaveGameWidget::defaultDir()
{
 QString dir = KGlobal::dirs()->saveLocation("data",
		KGlobal::mainComponent().componentName() + "/savegames/",
		true);
 if (dir.isEmpty()) {
	boError() << k_funcinfo << "cannot find default dir?!" << endl;
	return QString();
 }
 if (dir.right(dir.length() - 1) != QString::fromLatin1("/")) {
	dir += QString::fromLatin1("/");
 }
 return dir;
}

void BoUfoLoadSaveGameWidget::setDefaultDir()
{
 d->mDir.cd(defaultDir());
}

void BoUfoLoadSaveGameWidget::setDefaultSuffix()
{
 d->mSuffix = "";
 d->mDir.setNameFilter("*");
}

void BoUfoLoadSaveGameWidget::setDirectory(const QString& dir)
{
 if (dir.isEmpty()) {
	setDefaultDir();
	return;
 }
 if (dir[0] != '/') {
	boWarning() << k_funcinfo << "directory must be absolute" << endl;
	return;
 }
 d->mDir.cd(dir);
}

QString BoUfoLoadSaveGameWidget::directory() const
{
 return d->mDir.path();
}

void BoUfoLoadSaveGameWidget::setSuffix(const QString& suffix)
{
 if (suffix.isEmpty()) {
	setDefaultSuffix();
	return;
 }
 d->mSuffix = suffix;
 if (d->mSuffix[0] == '.') {
	d->mSuffix = d->mSuffix.right(d->mSuffix.length() - 1);
 }
 QString filter = QString("*.%1").arg(d->mSuffix);
 d->mDir.setNameFilter(filter);
}

const QString& BoUfoLoadSaveGameWidget::suffix() const
{
 return d->mSuffix;
}

void BoUfoLoadSaveGameWidget::updateGames()
{
 Q3PtrListIterator<BoUfoSaveGameWidget> it(d->mButtons);
 for (; it.current(); ++it) {
	it.current()->hide();
 }

 QStringList list = entryList();
 for (int i = 0; i < list.count(); i++) {
	QString file = d->mDir.absPath() + QString::fromLatin1("/") + list[i];
	readFile(file, i);
 }
 if (!d->mSave && d->mButtons.count() == 0) {
	if (!d->mNoGamesLabel) {
		d->mNoGamesLabel = new BoUfoLabel(i18n("No saved games available"));
		d->mButtonWidget->addWidget(d->mNoGamesLabel);
		d->mNoGamesLabel->show();
	}
 }

 if (d->mSave) {
	// TODO: here should be a button for a new file!
 }
}

void BoUfoLoadSaveGameWidget::readFile(const QString& file, int position)
{
 QString description;
 QString level;
 description = file.section('/', -1, -1);
 QFileInfo info(file);
 addFile(file, description, level, info.lastModified(), position);
}

BoUfoSaveGameWidget* BoUfoLoadSaveGameWidget::addFile(const QString& file, const QString& description, const QString& level, const QDateTime& date, int position)
{
 if (position < 0) {
	// add to the end
	position = d->mButtons.count();
 }
 if ((unsigned int)position > d->mButtons.count()) {
	// invalid
	boWarning() << k_funcinfo << "Cannot add to " << position << " - adding to the end" << endl;
	position = d->mButtons.count();
 }
 BoUfoSaveGameWidget* w = 0;
 if ((unsigned int)position < d->mButtons.count()) {
	w = d->mButtons.at(position);
	if (!w) {
		boError() << k_funcinfo << "Cannot find button at " << position << endl;
	}
 }
 if (d->mNoGamesLabel) {
	d->mNoGamesLabel->hide();
 }

 if (!w) {
	// add to the end
	w = createButton();
	connect(w, SIGNAL(signalClicked(BoUfoSaveGameWidget*)),
			this, SLOT(slotClicked(BoUfoSaveGameWidget*)));
	d->mButtons.append(w);
	d->mButtonWidget->addWidget(w);
 }
 w->setDescription(description);
 w->setLevel(level);
 w->setDateTime(date);
 w->setFile(file);
 w->show();
 return w;
}

BoUfoSaveGameWidget* BoUfoLoadSaveGameWidget::createButton()
{
 return new BoUfoSaveGameWidget();
}

void BoUfoLoadSaveGameWidget::slotDelete()
{
 BoUfoSaveGameWidget* w = selectedGame();
 if (!w) {
	// no game selected
	return;
 }
 QString file = w->file();
 int r = KMessageBox::questionYesNoCancel(0,
		i18n("Do you really want to delete %1 ?", QFileInfo(file).fileName()),
		QString(), KStandardGuiItem::yes(), KStandardGuiItem::no(), KStandardGuiItem::cancel(),
		"ConfirmDeleteGame");
 if (r != KMessageBox::Yes) {
	return;
 }
 slotClicked(w);
 d->mSelectedGame = 0; // just in case
 QFile::remove(file);

 w->hide();
 updateGames();
}

void BoUfoLoadSaveGameWidget::slotLoadSaveTimer()
{
 QTimer::singleShot(0, this, SLOT(slotLoadSave()));
}

void BoUfoLoadSaveGameWidget::slotLoadSave()
{
 BoUfoSaveGameWidget* w = selectedGame();
 if (!w) {
	// no game selected

	// TODO!
	// FIXME: currently when no game is selected and we are saving, then we
	// emit the following signal. Correct solution would be to display an
	// empty savegame button
	if (d->mSave) {
		emit signalSaveGame(saveFileName(), QString());
	}
	return;
 }
 QString file = w->file();
 QString description = w->description();

 if (d->mSave) {
	emit signalSaveGame(file, description);
 } else {
	emit signalLoadGame(file);
 }
}

BoUfoSaveGameWidget* BoUfoLoadSaveGameWidget::selectedGame() const
{
 return d->mSelectedGame;
}

void BoUfoLoadSaveGameWidget::slotClicked(BoUfoSaveGameWidget* w)
{
 if (!w) {
	return;
 }
 if (w->isSelected()) {
	d->mSelectedGame = w;
	Q3PtrListIterator<BoUfoSaveGameWidget> it(d->mButtons);
	for (; it.current(); ++it) {
		if (it.current()->isSelected() && it.current() != d->mSelectedGame) {
			it.current()->blockSignals(true);
			it.current()->unselect();
			it.current()->blockSignals(false);
		}
	}
 } else {
	d->mSelectedGame = 0;
 }
 if (d->mSave) {
	// saving without a button selected will create a new file
	d->mLoadSaveButton->setEnabled(true);
 } else {
	d->mLoadSaveButton->setEnabled(w->isSelected());
 }
 d->mDeleteButton->setEnabled(w->isSelected());
}

void BoUfoLoadSaveGameWidget::setSaveMode(bool save)
{
 d->mSave = save;
 if (save) {
	d->mLoadSaveButton->setText(i18n("&Save Game"));
 } else {
	d->mLoadSaveButton->setText(i18n("&Load Game"));
 }
 d->mLoadSaveButton->setEnabled(save);
 d->mDeleteButton->setEnabled(false);
 Q3PtrListIterator<BoUfoSaveGameWidget> it(d->mButtons);
 for (; it.current(); ++it) {
	it.current()->unselect();
 }
 d->mSelectedGame = 0;
 updateGames();
}

QStringList BoUfoLoadSaveGameWidget::entryList()
{
 // this looks like a qt bug to me. entryList() doesn't get updated. when we
 // save a game and call this again there are still the same entries (without
 // the newly saved game)
 d->mDir.setPath(d->mDir.path());
 return d->mDir.entryList();
}

QString BoUfoLoadSaveGameWidget::saveFileName()
{
 boDebug() << k_funcinfo << endl;
 if (!d->mSave) {
	boWarning() << k_funcinfo << "not in save mode" << endl;
	return QString();
 }
 QString base = "ksavegame-";
 QString s;
 if (!suffix().isEmpty()) {
	s = QString(".%1").arg(suffix());
 }
 QStringList files = entryList();
 QString file;
 bool ok = false;
 int i = 0;
 for (i = 0; i < files.count() + 1 && !ok; i++) {
	file.sprintf("ksavegame-%04d", i);
	file += s;
	if (!files.contains(file)) {
		ok = true;
	}
 }
 QString r = directory();
 if (r.right(r.length() - 1) != QString::fromLatin1("/")) {
	r += QString::fromLatin1("/");
 }
 r += file;
 return r;
}

void BoUfoLoadSaveGameWidget::slotBrowse()
{
 QString file;
 if (d->mSave) {
	file = BoFileDialog::getSaveFileName();
 } else {
	file = BoFileDialog::getOpenFileName();
 }

 if (file.isEmpty()) {
	return;
 }

 if (d->mSave) {
	// TODO: use the same way to enter the description as we use in
	// slotLoadSave() !
	// I mean once we use it at all...
	emit signalSaveGame(file, QString());
 } else {
	emit signalLoadGame(file);
 }
}





class BoUfoSaveGameWidgetPrivate
{
public:
	BoUfoSaveGameWidgetPrivate()
	{
		mButton = 0;
		mLevel = 0;
		mTime = 0;
		mDate = 0;
	}

	BoUfoPushButton* mButton;
	BoUfoLabel* mLevel;
	BoUfoLabel* mDate;
	BoUfoLabel* mTime;
	QString mFile;
};

BoUfoSaveGameWidget::BoUfoSaveGameWidget()
		: BoUfoWidget()
{
 d = new BoUfoSaveGameWidgetPrivate;
 setLayoutClass(UHBoxLayout);

 d->mButton = new BoUfoPushButton();
 d->mLevel = new BoUfoLabel();
 d->mDate = new BoUfoLabel();
 d->mTime = new BoUfoLabel();

 addWidget(d->mButton);
 addWidget(d->mLevel);
 addWidget(d->mDate);
 addWidget(d->mTime);

 d->mButton->setToggleButton(true);

 connect(d->mButton, SIGNAL(signalClicked()), this, SLOT(slotClicked()));
}

BoUfoSaveGameWidget::~BoUfoSaveGameWidget()
{
 delete d;
}

void BoUfoSaveGameWidget::setDateTime(const QDateTime& date)
{
 d->mTime->setText(KGlobal::locale()->formatTime(date.time()));
 d->mDate->setText(KGlobal::locale()->formatDate(date.date()));
}

void BoUfoSaveGameWidget::setLevel(const QString& l)
{
 d->mLevel->setText(l);
}

void BoUfoSaveGameWidget::setDescription(const QString& desc)
{
 d->mButton->setText(desc);
}

void BoUfoSaveGameWidget::setFile(const QString& file)
{
 d->mFile = file;
}

const QString& BoUfoSaveGameWidget::file() const
{
 return d->mFile;
}

QString BoUfoSaveGameWidget::description() const
{
 return d->mButton->text();
}

bool BoUfoSaveGameWidget::isSelected() const
{
 return d->mButton->isOn();
}

void BoUfoSaveGameWidget::unselect()
{
 d->mButton->setOn(false);
}

