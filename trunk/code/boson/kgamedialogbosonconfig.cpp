#include "kgamedialogbosonconfig.h"

#include "bosonmap.h"
#include "bosonscenario.h"

#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <ksimpleconfig.h>
#include <kmessagebox.h>

#include <qpushbutton.h>
#include <qvgroupbox.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qmap.h>

#include "kgamedialogbosonconfig.moc"

class KGameDialogBosonConfigPrivate
{
public:
	KGameDialogBosonConfigPrivate()
	{
		mMapCombo = 0;
	}

	QMap<int, QString> mMapIndex2Comment; // index -> map comment
	QMap<int, QString> mMapIndex2FileName; // index -> *.bpf file
	QMap<int, QString> mMapIndex2Identifier; // index -> map identifier

	QMap<int, QString> mScenarioIndex2FileName; // index -> *.bsc filename
	QMap<int, QString> mScenarioIndex2Comment ; // index -> scenario comment

	QComboBox* mMapCombo;
	QComboBox* mScenarioCombo;
};

KGameDialogBosonConfig::KGameDialogBosonConfig(QWidget* parent) 
		: KGameDialogGeneralConfig(parent, true)
{
 d = new KGameDialogBosonConfigPrivate;

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

 QPushButton* addComputerPlayer = new QPushButton(i18n("&Add Computer Player"), this);
 connect(addComputerPlayer, SIGNAL(pressed()), 
		this, SIGNAL(signalAddComputerPlayer())); // TODO: name, difficulty, ...

 QPushButton* startGame = new QPushButton(i18n("&Start Game"), this);
 connect(startGame, SIGNAL(pressed()), this, SIGNAL(signalStartGame()));
}

KGameDialogBosonConfig::~KGameDialogBosonConfig()
{
 kdDebug() << "~KGameDialogBosonConfig()" << endl;
 delete d;
 kdDebug() << "~KGameDialogBosonConfig() done" << endl;
}

void KGameDialogBosonConfig::slotMapChanged(int index)
{
 if (!admin()) {
	kdWarning() << "Only admin can change the map" << endl;
	return;
 }
 if (index >= (int)d->mMapIndex2FileName.count()) {
	kdError() << "invalid index " << index << endl;
	return;
 }

 // update possible scenario files:
 QStringList list = BosonScenario::availableScenarios(d->mMapIndex2Identifier[index]);
 d->mScenarioCombo->clear();
 for (unsigned int i = 0; i < list.count(); i++) {
	KSimpleConfig cfg(list[i]);
	cfg.setGroup("Boson Scenario");
	d->mScenarioCombo->insertItem(cfg.readEntry("Name", i18n("Unknown")), i);
	d->mScenarioIndex2Comment.insert(i, cfg.readEntry("Comment", i18n("None")));
	QString fileName = list[i].left(list[i].length() - strlen(".desktop")) + QString::fromLatin1(".bsc");
	d->mScenarioIndex2FileName.insert(i, fileName);
 }
 
 if (d->mScenarioIndex2FileName.count() == 0) {
	kdError() << "No valid scenario files" << endl;
	KMessageBox::sorry(this, i18n("No valid scenario files for this map!"));
	return;
 }
 emit signalMapChanged(d->mMapIndex2FileName[index]);
 d->mScenarioCombo->setCurrentItem(0);
 slotScenarioChanged(0);
}

void KGameDialogBosonConfig::slotScenarioChanged(int index)
{
 if (!admin()) {
	kdWarning() << "Only admin can change the map" << endl;
	return;
 }
 if (index >= (int)d->mScenarioIndex2FileName.count()) {
	kdError() << "invalid index " << index << endl;
	return;
 }
 kdDebug() << d->mScenarioIndex2FileName[index] << endl;
 emit signalScenarioChanged(d->mScenarioIndex2FileName[index]);
}

