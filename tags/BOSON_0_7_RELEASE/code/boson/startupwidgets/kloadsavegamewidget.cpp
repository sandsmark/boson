/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "kloadsavegamewidget.h"
#include "kloadsavegamewidget.moc"

#include "bodebug.h"

#include <qdir.h>
#include <qscrollview.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qdatetime.h>

#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <klocale.h>

// TODO: add a "browse" button to allow a normal KFileDialog to be used

class KLoadSaveGameWidget::KLoadSaveGameWidgetPrivate
{
public:
	KLoadSaveGameWidgetPrivate()
	{
		mHeadline = 0;
		mLoadSaveButton = 0;
		mDeleteButton = 0;

		mButtonWidget = 0;
		mButtonLayout = 0;
		mSelectedGame = 0;
	}

	QDir mDir;
	bool mSave;
	QString mSuffix;

	QLabel* mHeadline;
	QPushButton* mLoadSaveButton;
	QPushButton* mDeleteButton;

	QWidget* mButtonWidget;
	QVBoxLayout* mButtonLayout;
	QPtrList<KSaveGameWidget> mButtons;
	KSaveGameWidget* mSelectedGame;
};

KLoadSaveGameWidget::KLoadSaveGameWidget(bool save, const QString& suffix, QWidget* parent, const QString& dir)
		: QWidget(parent)
{
 init();
 setSaveMode(save);
 setDirectory(dir);
 setSuffix(suffix);
 updateGames();
}

KLoadSaveGameWidget::KLoadSaveGameWidget(QWidget* parent)
		: QWidget(parent)
{
 init();
 setSaveMode(true);
 updateGames();
}

void KLoadSaveGameWidget::init()
{
 d = new KLoadSaveGameWidgetPrivate;
 d->mSave = true; // default. this is for the case that we use it before setSaveMode() gets called
 d->mButtons.setAutoDelete(true);
 d->mDir.setFilter(QDir::Files | QDir::Readable);

 QVBoxLayout* topLayout = new QVBoxLayout(this, 5, 5);
 d->mHeadline = new QLabel(this);
 QFont font;
 font.setPointSize(font.pointSize() + 4);
 font.setBold(true);
 d->mHeadline->setFont(font);
 d->mHeadline->setAlignment(AlignHCenter);
 topLayout->addWidget(d->mHeadline);

 QScrollView* scroll = new QScrollView(this, "scrollwidget");
 scroll->setResizePolicy(QScrollView::AutoOneFit);
 scroll->setHScrollBarMode(QScrollView::AlwaysOff);
 scroll->setVScrollBarMode(QScrollView::AlwaysOn);
 d->mButtonWidget = new QWidget(scroll->viewport());
 scroll->addChild(d->mButtonWidget);
 topLayout->addWidget(scroll, 1);

 // this way we canuse d->mButtonLayout->addWidget() and can ignore the stretch
 // area at the end
 QVBoxLayout* buttonLayout = new QVBoxLayout(d->mButtonWidget, 5, 5);
 d->mButtonLayout = new QVBoxLayout(buttonLayout);
 buttonLayout->addStretch(1);

 QWidget* bottom = new QWidget(this);
 topLayout->addWidget(bottom);
 QHBoxLayout* bottomLayout = new QHBoxLayout(bottom);

 d->mDeleteButton = new QPushButton(i18n("&Delete"), bottom);
 bottomLayout->addWidget(d->mDeleteButton);
 bottomLayout->addStretch(1);
 connect(d->mDeleteButton, SIGNAL(clicked()), this, SLOT(slotDelete()));

 d->mLoadSaveButton = new QPushButton(bottom);
 bottomLayout->addWidget(d->mLoadSaveButton);
 bottomLayout->addStretch(1);
 connect(d->mLoadSaveButton, SIGNAL(clicked()), this, SLOT(slotLoadSave()));

 QPushButton* cancel = new QPushButton(i18n("&Cancel"), bottom);
 bottomLayout->addWidget(cancel);
 connect(cancel, SIGNAL(clicked()), this, SIGNAL(signalCancel()));
 cancel->setFocus();

 setDefaultDir();
 setDefaultSuffix();
 setSaveMode(true);
}

KLoadSaveGameWidget::~KLoadSaveGameWidget()
{
 d->mButtons.clear();
 delete d;
}

void KLoadSaveGameWidget::setDefaultDir()
{
 QString dir = KGlobal::dirs()->saveLocation("data",
		KGlobal::instance()->instanceName() + "/savegames/",
		true);
 d->mDir.cd(dir);
}

void KLoadSaveGameWidget::setDefaultSuffix()
{
 d->mSuffix = "";
 d->mDir.setNameFilter("*");
}

void KLoadSaveGameWidget::setDirectory(const QString& dir)
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

QString KLoadSaveGameWidget::directory() const
{
 return d->mDir.path();
}

void KLoadSaveGameWidget::setSuffix(const QString& suffix)
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

const QString& KLoadSaveGameWidget::suffix() const
{
 return d->mSuffix;
}

void KLoadSaveGameWidget::updateGames()
{
 QPtrListIterator<KSaveGameWidget> it(d->mButtons);
 for (; it.current(); ++it) {
	it.current()->hide();
 }

 QStringList list = entryList();
 for (unsigned int i = 0; i < list.count(); i++) {
	QString file = d->mDir.absPath() + QString::fromLatin1("/") + list[i];
	readFile(file, i);
 }

 if (d->mSave) {
	// TODO: here should be a button for a new file!
 }
}

void KLoadSaveGameWidget::readFile(const QString& file, int position)
{
 QString description;
 QString level;
 description = file.section('/', -1, -1);
 QFileInfo info(file);
 addFile(file, description, level, info.lastModified(), position);
}

KSaveGameWidget* KLoadSaveGameWidget::addFile(const QString& file, const QString& description, const QString& level, const QDateTime& date, int position)
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
 KSaveGameWidget* w = 0;
 if ((unsigned int)position < d->mButtons.count()) {
	w = d->mButtons.at(position);
	if (!w) {
		boError() << k_funcinfo << "Cannot find button at " << position << endl;
	}
 }

 if (!w) {
	// add to the end
	w = createButton(d->mButtonWidget);
	connect(w, SIGNAL(signalClicked(KSaveGameWidget*)),
			this, SLOT(slotClicked(KSaveGameWidget*)));
	d->mButtons.append(w);
	d->mButtonLayout->addWidget(w);
 }
 w->setDescription(description);
 w->setLevel(level);
 w->setDateTime(date);
 w->setFile(file);
 w->show();
 return w;
}

KSaveGameWidget* KLoadSaveGameWidget::createButton(QWidget* parent)
{
 return new KSaveGameWidget(parent);
}

void KLoadSaveGameWidget::slotDelete()
{
 KSaveGameWidget* w = selectedGame();
 if (!w) {
	// no game selected
	return;
 }
 QString description;
 int r = KMessageBox::questionYesNoCancel(this, 
		i18n("Do you really want to delete %1 ?").arg(description),
		QString::null, KStdGuiItem::yes(), KStdGuiItem::no(), "ConfirmDeleteGame");
 if (r != KMessageBox::Yes) {
	return;
 }
 // TODO
 boDebug() << k_funcinfo << "not yet implemented" << endl;
}

void KLoadSaveGameWidget::slotLoadSave()
{
 KSaveGameWidget* w = selectedGame();
 if (!w) {
	// no game selected

	// TODO!
	// FIXME: currently when no game is selected and we are saving, then we
	// emit the following signal. Correct solution would be to display an
	// empty savegame button
	if (d->mSave) {
		emit signalSaveGame(saveFileName(), QString::null);
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

KSaveGameWidget* KLoadSaveGameWidget::selectedGame() const
{
 return d->mSelectedGame;
}

void KLoadSaveGameWidget::slotClicked(KSaveGameWidget* w)
{
 if (!w) {
	return;
 }
 d->mSelectedGame = w;
 QPtrListIterator<KSaveGameWidget> it(d->mButtons);
 for (; it.current(); ++it) {
	if (it.current()->isSelected() && it.current() != d->mSelectedGame) {
		it.current()->unselect();
	}
 }
 if (d->mSave) {
	// saving without a button selected will create a new file
	d->mLoadSaveButton->setEnabled(true);
 } else {
	d->mLoadSaveButton->setEnabled(w->isSelected());
 }
 d->mDeleteButton->setEnabled(w->isSelected());
}

void KLoadSaveGameWidget::setSaveMode(bool save)
{
 d->mSave = save;
 if (save) {
	d->mHeadline->setText(i18n("Save Game"));
	d->mLoadSaveButton->setText(i18n("&Save Game"));
 } else {
	d->mHeadline->setText(i18n("Load Game"));
	d->mLoadSaveButton->setText(i18n("&Load Game"));
 }
 d->mLoadSaveButton->setEnabled(save);
 d->mDeleteButton->setEnabled(false);
 QPtrListIterator<KSaveGameWidget> it(d->mButtons);
 for (; it.current(); ++it) {
	it.current()->unselect();
 }
 d->mSelectedGame = 0;
 updateGames();
}

QStringList KLoadSaveGameWidget::entryList()
{
 // this looks like a qt bug to me. entryList() doesn't get updated. when we
 // save a game and call this again there are still the same entries (without
 // the newly saved game)
 d->mDir.setPath(d->mDir.path());
 return d->mDir.entryList();
}

QString KLoadSaveGameWidget::saveFileName()
{
 boDebug() << k_funcinfo << endl;
 if (!d->mSave) {
	boWarning() << k_funcinfo << "not in save mode" << endl;
	return QString::null;
 }
 QString base = "ksavegame-";
 QString s;
 if (!suffix().isEmpty()) {
	s = QString(".%1").arg(suffix());
 }
 QStringList files = entryList();
 QString file;
 bool ok = false;
 unsigned int i = 0;
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


class KSaveGameWidget::KSaveGameWidgetPrivate
{
public:
	KSaveGameWidgetPrivate()
	{
		mButton = 0;
		mLevel = 0;
		mTime = 0;
		mDate = 0;
	}

	QPushButton* mButton;
	QLabel* mLevel;
	QLabel* mDate;
	QLabel* mTime;
	QString mFile;
};

KSaveGameWidget::KSaveGameWidget(QWidget* parent)
		: QWidget(parent)
{
 d = new KSaveGameWidgetPrivate;
 QHBoxLayout* topLayout = new QHBoxLayout(this);
 topLayout->setAutoAdd(true);

 d->mButton = new QPushButton(this);
 d->mLevel = new QLabel(this);
 d->mDate = new QLabel(this);
 d->mTime = new QLabel(this);

 d->mButton->setToggleButton(true);

 connect(d->mButton, SIGNAL(clicked()), this, SLOT(slotClicked()));
}

KSaveGameWidget::~KSaveGameWidget()
{
 delete d;
}

void KSaveGameWidget::setDateTime(const QDateTime& date)
{
 d->mTime->setText(KGlobal::locale()->formatTime(date.time()));
 d->mDate->setText(KGlobal::locale()->formatDate(date.date()));
}

void KSaveGameWidget::setLevel(const QString& l)
{
 d->mLevel->setText(l);
}

void KSaveGameWidget::setDescription(const QString& desc)
{
 d->mButton->setText(desc);
}

void KSaveGameWidget::setFile(const QString& file)
{
 d->mFile = file;
}

const QString& KSaveGameWidget::file() const
{
 return d->mFile;
}

QString KSaveGameWidget::description() const
{
 return d->mButton->text();
}

bool KSaveGameWidget::isSelected() const
{
 return d->mButton->isOn();
}

void KSaveGameWidget::unselect()
{
 d->mButton->setOn(false);
}

