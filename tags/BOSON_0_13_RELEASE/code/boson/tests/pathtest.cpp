#include <bosonpath.h>
#include <bosongroundtheme.h>
#include <bodebug.h>
#include <boapplication.h>
#include <boson.h>
#include <bosonplayfield.h>
#include <bosonmap.h>
#include <bosoncanvas.h>
#include <bosondata.h>
#include <boeventloop.h>

#include <kcmdlineargs.h>

#include <qstringlist.h>

static bool createGame();
static bool createMap();
static bool testSearchPathFrom(int fromX, int fromY);

BosonPlayField* g_playField = 0;
BosonMap* g_map = 0;
BosonCanvas* g_canvas = 0;

int main(int argc, char** argv)
{
 KCmdLineArgs::init(argc, argv, "tester", "tester", "tester", "version xyz");
 BoEventLoop eventLoop;
 BoApplication app;
 if (!createGame()) {
	return 1;
 }
 boDebug() << k_funcinfo << "game created" << endl;

 for (int x = 0; x < (int)g_map->width(); x += 5) {
	for (int y = 0; y < (int)g_map->height(); y += 5) {
		if (!testSearchPathFrom(x, y)) {
			boError() << k_funcinfo << "testing failed at from=(" << x << "," << y << ")" << endl;
			return 1;
		}
	}
 }

 return 0;
}

static bool createGame()
{
 if (!createMap()) {
	return false;
 }
 Boson::initBoson();
 boGame->createCanvas();
 g_canvas = boGame->canvasNonConst();
 g_canvas->setMap(g_playField->map());

 boDebug() << k_funcinfo << "init pathfinder" << endl;
 g_canvas->initPathfinder();

 boDebug() << k_funcinfo << "Trying searching sample path" << endl;
 BosonPathInfo i;
 i.start = QPoint(5 * BO_TILE_SIZE, 5 * BO_TILE_SIZE);
 i.dest = QPoint(45 * BO_TILE_SIZE, 35 * BO_TILE_SIZE);
 boDebug() << k_funcinfo << "Let's go!" << endl;
 g_canvas->pathfinder()->findPath(&i);
 boDebug() << k_funcinfo << "sample path searching complete" << endl;

 qDebug("testing completed. no bugs found.");
 return true;
}

static bool createMap()
{
 if (g_playField) {
	return false;
 }
 if (!BosonGroundTheme::createGroundThemeList()) {
	return false;
 }
 BosonPlayField* field = new BosonPlayField(0);
 QStringList available = BosonData::availableFiles("maps/*.bpf"); // we dont care about subdirs
 if (available.isEmpty()) {
	return false;
 }

 qDebug("using %s", available[0].latin1());
 field->preLoadPlayField(available[0]);

 QMap<QString, QByteArray> files;
 if (!field->loadFromDiskToFiles(files)) {
	boError() << k_funcinfo << "playfield loading failed" << endl;
	return false;
 }
 delete field;
 field = 0;
 g_playField = new BosonPlayField(0);
 if (!g_playField->loadPlayField(files)) {
	boError() << k_funcinfo << "could not load playfield from files" << endl;
	return false;
 }
 g_map = g_playField->map();
 boDebug() << k_funcinfo << "playfield loaded" << endl;
 return true;
}

static bool testSearchPathFrom(int fromX, int fromY)
{
 for (int x = 0; x < (int)g_map->width(); x += 5) {
	for (int y = 0; y < (int)g_map->height(); y += 5) {
		BosonPathInfo info;
		info.start = QPoint(fromX * BO_TILE_SIZE, fromY * BO_TILE_SIZE);
		info.dest = QPoint(x * BO_TILE_SIZE, y * BO_TILE_SIZE);
		g_canvas->pathfinder()->findPath(&info);
	}
 }
 return true;
}

