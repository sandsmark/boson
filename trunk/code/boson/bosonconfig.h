#ifndef __BOSONCONFIG_H__
#define __BOSONCONFIG_H__

#include <qstring.h>

class KConfig;

class BosonConfig
{
public:
	BosonConfig() { }
	~BosonConfig() { }

	static QString localPlayerName(KConfig* conf = 0);
	static void saveLocalPlayerName(const QString& name, KConfig* conf = 0);

	static void saveGameSpeed(int speed, KConfig* conf = 0);
	static int gameSpeed(KConfig* conf = 0);

protected:
	static void changeGroupGeneral(KConfig* conf);
};

#endif
