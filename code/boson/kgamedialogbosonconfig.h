#ifndef __KGAMEDIALOGBOSONCONFIG_H__
#define __KGAMEDIALOGBOSONCONFIG_H__

#include <kgame/kgamedialogconfig.h>

class KGameDialogBosonConfigPrivate;
class KGameDialogBosonConfig : public KGameDialogGeneralConfig
{
	Q_OBJECT
public:
	KGameDialogBosonConfig(QWidget* parent = 0);
	virtual ~KGameDialogBosonConfig();

signals:
	void signalStartGame();
	void signalAddComputerPlayer();
	void signalMapChanged(const QString& fileName);
	void signalScenarioChanged(const QString& fileName);

public slots:
	void slotMapChanged(int index);

protected slots:
	void slotScenarioChanged(int index);

private:
	KGameDialogBosonConfigPrivate* d;
};
#endif
