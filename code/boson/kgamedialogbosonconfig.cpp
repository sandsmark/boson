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

#include "bosonmap.h"
#include "bosonscenario.h"
#include "speciestheme.h"
#include "bosonmessage.h"
#include "boson.h"
#include "player.h"

#include <kgame/kgame.h>

#include <klocale.h>
#include <kdebug.h>
#include <ksimpleconfig.h>
#include <kmessagebox.h>

#include <qpushbutton.h>
#include <qvgroupbox.h>
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
		
		mMapCombo = 0;
		mScenarioCombo = 0;
	}

	QComboBox* mPlayerSpecies;
	QComboBox* mPlayerColor;

	QMap<int, QString> mMapIndex2FileName; // index -> *.bpf file
	QMap<int, QString> mMapIndex2Comment; // index -> map comment
	QMap<int, QString> mMapIndex2Identifier; // index -> map identifier

	QMap<int, QString> mScenarioIndex2FileName; // index -> *.bsc filename
	QMap<int, QString> mScenarioIndex2Comment ; // index -> scenario comment
	QMap<int, QString> mScenarioIndex2Identifier; // index -> scenario identifier

	QMap<int, QString> mSpeciesIndex2Comment;
	QMap<int, QString> mSpeciesIndex2Identifier;

	QValueList<QColor> mColorList;

	QComboBox* mMapCombo;
	QComboBox* mScenarioCombo;
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

 QVGroupBox* mapBox = new QVGroupBox(i18n("Map and Scenario"), this);
 d->mMapCombo = new QComboBox(mapBox);
 connect(d->mMapCombo, SIGNAL(activated(int)), this, SLOT(slotMapChanged(int)));
 QStringList list = BosonMap::availableMaps();
 for (unsigned int i = 0; i < list.count(); i++) {
	KSimpleConfig cfg(list[i]);
	cfg.setGroup("Boson Map");
	d->mMapCombo->insertItem(cfg.readEntry("Name", i18n("Unknown")), i);
	d->mMapIndex2Comment.insert(i, cfg.readEntry("Comment", i18n("None")));
	QString fileName = list[i].left(list[i].length() - strlen(".desktop")) + QString::fromLatin1(".bpf");
	d->mMapIndex2FileName.insert(i, fileName);
	d->mMapIndex2Identifier.insert(i, cfg.readEntry("Identifier", i18n("Unknown")));
 }
 d->mMapCombo->setCurrentItem(0);

 d->mScenarioCombo = new QComboBox(mapBox);
 connect(d->mScenarioCombo, SIGNAL(activated(int)), 
		this, SLOT(slotScenarioChanged(int)));

 QPushButton* startGame = new QPushButton(i18n("&Start Game"), this);
 connect(startGame, SIGNAL(pressed()), this, SLOT(slotStartGame()));
}

KGameDialogBosonConfig::~KGameDialogBosonConfig()
{
 delete d;
}

void KGameDialogBosonConfig::slotMapChanged(int index)
{
 if (!admin()) {
	kdWarning() << "Only admin can change the map" << endl;
	return;
 }
 if (index >= (int)d->mMapIndex2Identifier.count()) {
	kdError() << "invalid index " << index << endl;
	return;
 }
 if (!game()) {
	kdError() << k_lineinfo << "Cannot send message" << endl;
	return;
 }
 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);
 //TODO: instead of transmitting the filename of the map we should transmit
 //the map itself!
 stream << d->mMapIndex2Identifier[index];
 game()->sendMessage(buffer, BosonMessage::ChangeMap);
}

void KGameDialogBosonConfig::slotMapChanged(const QString& identifier)
{
 // update possible scenario files:
 QStringList list = BosonScenario::availableScenarios(identifier);
 d->mScenarioCombo->clear();
 d->mScenarioIndex2Comment.clear();
 d->mScenarioIndex2FileName.clear();
 for (unsigned int i = 0; i < list.count(); i++) {
	KSimpleConfig cfg(list[i]);
	cfg.setGroup("Boson Scenario");
	d->mScenarioCombo->insertItem(cfg.readEntry("Name", i18n("Unknown")), i);
	d->mScenarioIndex2Comment.insert(i, cfg.readEntry("Comment", i18n("None")));
	d->mScenarioIndex2Identifier.insert(i, cfg.readEntry("Identifier", i18n("None")));
	QString fileName = list[i].left(list[i].length() - strlen(".desktop")) + QString::fromLatin1(".bsc");
	d->mScenarioIndex2FileName.insert(i, fileName);
 }
 
 if (d->mScenarioIndex2FileName.count() == 0) {
	kdError() << "No valid scenario files" << endl;
	KMessageBox::sorry(this, i18n("No valid scenario files for this map!"));
	return;
 }
 d->mScenarioCombo->setCurrentItem(0);
 slotScenarioChanged(0);
}

void KGameDialogBosonConfig::slotScenarioChanged(int index)
{
 if (!admin()) {
	kdWarning() << "Only admin can change the map" << endl;
	return;
 }
 if (index >= (int)d->mScenarioIndex2Identifier.count()) {
	kdError() << "invalid index " << index << endl;
	return;
 }
 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);

 //TODO: instead of transmitting the filename of the scenario we should transmit
 //the scenario itself!
 stream << d->mScenarioIndex2Identifier[index];
 game()->sendMessage(buffer, BosonMessage::ChangeScenario);
}

void KGameDialogBosonConfig::slotScenarioChanged(const QString& identifier)
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
 
// emit signalScenarioChanged(d->mScenarioIndex2FileName[index]);
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
	kdWarning() << k_funcinfo << "NULL game" << endl;
	return;
 }
 d->mPlayerColor->clear();
 d->mColorList.clear();
 Player* p = (Player*)owner();
 if (p) {
	addColor(p->speciesTheme()->teamColor());
 } else {
	kdWarning() << k_funcinfo << "NULL owner" << endl;
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
kdDebug() << "?? is " << c.rgb() << endl;
 emit signalTeamColorChanged(c);
}

void KGameDialogBosonConfig::slotTeamColorChanged(Player* p)
{
 kdDebug() << k_funcinfo << endl;
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

