#include "kgamedialogbosonconfig.h"

#include <klocale.h>
#include <kdebug.h>

#include <qpushbutton.h>

#include "kgamedialogbosonconfig.moc"

class KGameDialogBosonConfigPrivate
{
public:
	KGameDialogBosonConfigPrivate()
	{
	}
};

KGameDialogBosonConfig::KGameDialogBosonConfig(QWidget* parent) 
		: KGameDialogGeneralConfig(parent, true)
{
 d = new KGameDialogBosonConfigPrivate;

 QPushButton* startGame = new QPushButton(i18n("&Start Game"), this);
 connect(startGame, SIGNAL(pressed()), this, SIGNAL(signalStartGame()));

 QPushButton* addComputerPlayer = new QPushButton(i18n("&Add Computer Player"), this);
 connect(addComputerPlayer, SIGNAL(pressed()), 
		this, SIGNAL(signalAddComputerPlayer())); // TODO: name, difficulty, ...
}

KGameDialogBosonConfig::~KGameDialogBosonConfig()
{
 kdDebug() << "~KGameDialogBosonConfig()" << endl;
 delete d;
 kdDebug() << "~KGameDialogBosonConfig() done" << endl;
}

