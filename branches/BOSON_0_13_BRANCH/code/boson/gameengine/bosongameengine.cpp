#include "bosongameengine.h"
#include "bosongameengine.moc"

#include "bosonplayfield.h"
#include "bosongroundtheme.h"
#include "bodebug.h"
#include "boson.h"

class BosonGameEnginePrivate
{
public:
	BosonGameEnginePrivate()
	{
	}
};

BosonGameEngine::BosonGameEngine(QObject* parent)
	: QObject(parent)
{
 d = new BosonGameEnginePrivate();
}

BosonGameEngine::~BosonGameEngine()
{
 delete d;
}

bool BosonGameEngine::preloadData()
{
 if (!BosonGroundTheme::createGroundThemeList()) {
	boError() << "Unable to load groundThemes. Check your installation!" << endl;
	return false;
 }
 if (!BosonPlayField::preLoadAllPlayFields()) {
	boError() << k_funcinfo << "Unable to preload playFields. Check your installation!" << endl;
	return false;
 }
 return true;
}

void BosonGameEngine::endGameAndDeleteBoson()
{
 if (boGame) {
	boGame->removeAllPlayers();
	boGame->quitGame();
	emit signalBosonObjectAboutToBeDestroyed(boGame);
	Boson::deleteBoson();
 }
}

void BosonGameEngine::slotResetGame()
{
 if (boGame) {
	boGame->removeAllPlayers();
 }

 endGameAndDeleteBoson();
 initGame();
}

void BosonGameEngine::initGame()
{
 if (boGame) {
	boError() << k_funcinfo << "Boson object still around. deleting..." << endl;
	endGameAndDeleteBoson();
 }
 Boson::initBoson();
}

