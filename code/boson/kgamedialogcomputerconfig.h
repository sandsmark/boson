#ifndef __KGAMEDIALOGCOMPUTERCONFIG_H__
#define __KGAMEDIALOGCOMPUTERCONFIG_H__

#include <kgame/kgamedialogconfig.h>

class Player;

class KGameDialogComputerConfig : public KGameDialogConfig
{
	Q_OBJECT
public:
	KGameDialogComputerConfig(QWidget* parent = 0);
	virtual ~KGameDialogComputerConfig();

	virtual void submitToKGame(KGame*, KPlayer*) { }

protected slots:
	void slotSpeciesChanged(int index);
	void slotAddComputerPlayer();

signals:
	void signalAddComputerPlayer(Player*);

private:
	class KGameDialogComputerConfigPrivate;
	KGameDialogComputerConfigPrivate* d;
};

#endif
