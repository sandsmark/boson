/*
    This file is part of the Boson game
    Copyright (C) 2001 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "kgamedialogbosonconfig.h"

#include "bosonplayfield.h"
#include "speciestheme.h"
#include "bosonmessage.h"
#include "boson.h"
#include "player.h"
#include "bodebug.h"

#include <kgame/kgame.h>

#include <klocale.h>
#include <ksimpleconfig.h>

#include <qpushbutton.h>
#include <qcombobox.h>
#include <qmap.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qpainter.h>

#include "kgamedialogbosonconfig.moc"

class KGameDialogBosonConfig::KGameDialogBosonConfigPrivate
{
public:
	KGameDialogBosonConfigPrivate()
	{
		mPlayerSpecies = 0;
		mPlayerColor = 0;
		mPlayFieldCombo = 0;
	}

	QComboBox* mPlayerSpecies;
	QComboBox* mPlayerColor;
	QComboBox* mPlayFieldCombo;

	QMap<int, QString> mPlayFieldIndex2FileName; // index -> *.bpf file
	QMap<int, QString> mPlayFieldIndex2Comment; // index -> playfield comment
	QMap<int, QString> mPlayFieldIndex2Identifier; // index -> playfield identifier

	QMap<int, QString> mSpeciesIndex2Comment;
	QMap<int, QString> mSpeciesIndex2Identifier;

	QValueList<QColor> mColorList;
};

KGameDialogBosonConfig::KGameDialogBosonConfig(QWidget* parent) 
		: KGameDialogGeneralConfig(parent, true)
{
 d = new KGameDialogBosonConfigPrivate;

 QHBox* speciesWidget = new QHBox(this);
 (void)new QLabel(i18n("Your species"), speciesWidget);
 d->mPlayerSpecies = new QComboBox(speciesWidget);
 connect(d->mPlayerSpecies, SIGNAL(activated(int)), this, SLOT(slotSpeciesChanged(int)));

 QHBox* colorWidget = new QHBox(this);
 (void)new QLabel(i18n("Your team color"), colorWidget);
 d->mPlayerColor = new QComboBox(colorWidget);
 connect(d->mPlayerColor, SIGNAL(activated(int)), this, SLOT(slotTeamColorChanged(int)));

 QHBox* playFieldBox = new QHBox(this);
 d->mPlayFieldCombo = new QComboBox(playFieldBox);
 connect(d->mPlayFieldCombo, SIGNAL(activated(int)), this, SLOT(slotPlayFieldChanged(int)));
 QStringList list = BosonPlayField::availablePlayFields();
 for (unsigned int i = 0; i < list.count(); i++) {
	KSimpleConfig cfg(list[i]);
	cfg.setGroup("Boson PlayField");
	d->mPlayFieldCombo->insertItem(cfg.readEntry("Name", i18n("Unknown")), i);
	d->mPlayFieldIndex2Comment.insert(i, cfg.readEntry("Comment", i18n("None")));
	QString fileName = list[i].left(list[i].length() - strlen(".boson"))
			+ QString::fromLatin1(".bpf");
	d->mPlayFieldIndex2FileName.insert(i, fileName);
	d->mPlayFieldIndex2Identifier.insert(i, cfg.readEntry("Identifier", 
			i18n("Unknown")));
 }
 d->mPlayFieldCombo->setCurrentItem(0);

 QPushButton* startGame = new QPushButton(i18n("&Start Game"), this);
 connect(startGame, SIGNAL(pressed()), this, SLOT(slotStartGame()));
}

KGameDialogBosonConfig::~KGameDialogBosonConfig()
{
 delete d;
}

void KGameDialogBosonConfig::slotPlayFieldChanged(int index)
{
 if (!admin()) {
	boWarning() << "Only admin can change the map" << endl;
	return;
 }
 if (index >= (int)d->mPlayFieldIndex2Identifier.count()) {
	boError() << "invalid index " << index << endl;
	return;
 }
 if (!game()) {
	boError() << k_lineinfo << "Cannot send message" << endl;
	return;
 }
 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);
 // transmit the identifier/name so that the remote newgame dialogs will be able
 // to display the newly selected playfield
 stream << d->mPlayFieldIndex2Identifier[index];
 game()->sendMessage(buffer, BosonMessage::ChangePlayField);
}

void KGameDialogBosonConfig::slotPlayFieldChanged(const QString& identifier)
{
 // update possible species:
 d->mPlayerSpecies->clear();
 d->mSpeciesIndex2Comment.clear();
 d->mSpeciesIndex2Identifier.clear();
 //TODO: some scenarios might not provide all species!
 QStringList list = SpeciesTheme::availableSpecies();
 for (unsigned int i = 0; i < list.count(); i++) {
	KSimpleConfig cfg(list[i]);
	cfg.setGroup("Boson Species");
	d->mPlayerSpecies->insertItem(cfg.readEntry("Name", i18n("Unknown")), i);
	d->mSpeciesIndex2Comment.insert(i, cfg.readEntry("Comment", i18n("None")));
	d->mSpeciesIndex2Identifier.insert(i, cfg.readEntry("Identifier", "Unknown"));
 }
 d->mPlayerSpecies->setCurrentItem(0);
 slotSpeciesChanged(0);
}

void KGameDialogBosonConfig::slotStartGame()
{
 submitToKGame(game(), owner()); // emulate a apply click first
 emit signalStartGame();
}

void KGameDialogBosonConfig::slotSpeciesChanged(int index)
{
 if (index < 0) {
	return;
 }
 emit signalSpeciesChanged(d->mSpeciesIndex2Identifier[index]);
}

void KGameDialogBosonConfig::regenerateColors()
{
 Boson* g = (Boson*)game();
 if (!g) {
	boWarning() << k_funcinfo << "NULL game" << endl;
	return;
 }
 d->mPlayerColor->clear();
 d->mColorList.clear();
 Player* p = (Player*)owner();
 if (p) {
	addColor(p->speciesTheme()->teamColor());
 } else {
	boWarning() << k_funcinfo << "NULL owner" << endl;
 }
 QValueList<QColor> colors = g->availableTeamColors();
 for (unsigned int i = 0; i < colors.count(); i++) {
	addColor(colors[i]);
 }
}

void KGameDialogBosonConfig::slotSpeciesChanged(Player* p)
{
 slotTeamColorChanged(p);
}

void KGameDialogBosonConfig::slotTeamColorChanged(int index)
{
 QColor c = d->mColorList[index];
boDebug() << "?? is " << c.rgb() << endl;
 emit signalTeamColorChanged(c);
}

void KGameDialogBosonConfig::slotTeamColorChanged(Player* p)
{
 boDebug() << k_funcinfo << endl;
 regenerateColors();
}

void KGameDialogBosonConfig::addColor(const QColor& c)
{
 d->mColorList.append(c);
 QPainter painter;
 QRect rect(0, 0, d->mPlayerColor->width(), QFontMetrics(painter.font()).height() + 4);
 QPixmap pixmap(rect.width(), rect.height());
 painter.begin(&pixmap);
 painter.fillRect(rect, QBrush(c));
 painter.end();
 d->mPlayerColor->insertItem(pixmap);
}

