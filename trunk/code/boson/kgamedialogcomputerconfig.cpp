#include "kgamedialogcomputerconfig.h"

#include "player.h"
#include "bosoncomputerio.h"
#include "speciestheme.h"

#include <klocale.h>
#include <kdebug.h>

#include <qpushbutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>

#include "kgamedialogcomputerconfig.moc"

class KGameDialogComputerConfig::KGameDialogComputerConfigPrivate
{
public:
	KGameDialogComputerConfigPrivate()
	{
		mName = 0;
	}
	QLineEdit* mName;
};


KGameDialogComputerConfig::KGameDialogComputerConfig(QWidget* parent) : KGameDialogConfig(parent)
{
 d = new KGameDialogComputerConfigPrivate;
 QGridLayout* layout = new QGridLayout(this, -1, 2);

 QLabel* nameLabel = new QLabel(i18n("Name of the player"), this);
 d->mName = new QLineEdit(this);
 d->mName->setText(i18n("Computer"));
 layout->addWidget(nameLabel, 0, 0);
 layout->addWidget(d->mName, 0, 1);
 
 QPushButton* addComputerPlayer = new QPushButton(i18n("&Add Computer Player"), this);
 connect(addComputerPlayer, SIGNAL(pressed()), 
		this, SLOT(slotAddComputerPlayer())); // TODO: name, difficulty, ...
 layout->addMultiCellWidget(addComputerPlayer, 1, 1, 0, 1);
}

KGameDialogComputerConfig::~KGameDialogComputerConfig()
{
 delete d;
}

void KGameDialogComputerConfig::slotAddComputerPlayer()
{
 // TODO
 Player* p = new Player;
 QString name = d->mName->text();
 if (name.isNull()) {
	name = i18n("Computer");
 }
 p->setName(name);
 
 // FIXME: MUST be sent over network! problem: at this point the ID of the
 // player is unknown. Possible solution would be to load the theme as soon as
 // the player is being added.
 p->loadTheme(SpeciesTheme::speciesDirectory(SpeciesTheme::defaultSpecies()), SpeciesTheme::defaultColor());

 BosonComputerIO* io = new BosonComputerIO();
 io->setReactionPeriod(50);
 p->addGameIO(io);
 emit signalAddComputerPlayer(p);
}

void KGameDialogComputerConfig::slotSpeciesChanged(int index)
{
 if (index < 0) {
	return;
 }
// emit signalSpeciesChanged(d->mSpeciesIndex2Directory[index]);
}

