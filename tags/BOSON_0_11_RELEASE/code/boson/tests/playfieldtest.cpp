#include <bosongroundtheme.h>
#include <bodebug.h>
#include <boapplication.h>
#include <boson.h>
#include <bosonplayfield.h>
#include <bosonmap.h>
//#include <bosoncanvas.h>
#include <bosondata.h>
#include <defines.h>
#include <cell.h>

#include <kcmdlineargs.h>

#include <qstringlist.h>
#include <qdom.h>
#include <qimage.h>


static bool testMap(const QString& map);
static bool testValid(BosonPlayField* field);
static bool testValidMap(const BosonMap* map);
static bool compareXML(const QByteArray& b1, const QByteArray& b2);
static bool compareElements(const QDomElement& r1, const QDomElement& r2);
static bool comparePNG(const QByteArray& b1, const QByteArray& b2);

QStringList availablePlayFields;

int main(int argc, char** argv)
{
 KCmdLineArgs::init(argc, argv, "tester", "tester", "tester", "version xyz");
 BoApplication app;

 // generate the list of available playfields
 if (!BosonPlayField::preLoadAllPlayFields()) {
	boError() << k_funcinfo << "could not preload all playfields" << endl;
	return 1;
 }
 availablePlayFields = boData->availablePlayFields();
 if (availablePlayFields.isEmpty()) {
	boError() << k_funcinfo << "could not find any playfield" << endl;
	return 1;
 }
 boDebug() << "available playfields: " << availablePlayFields.count() << endl;


 if (!BosonGroundTheme::createGroundThemeList()) {
	boError() << k_funcinfo << "grountheme list creation failed" << endl;
	return 1;
 }

 QStringList::Iterator it;
 for (it = availablePlayFields.begin(); it != availablePlayFields.end(); ++it) {
	if (!testMap(*it)) {
		boError() << k_funcinfo << "testMap() failed on map " << *it << endl;
		return 1;
	}
 }

 boDebug() << "testing completed. no bugs found." << endl;
 return 0;
}

static bool testMap(const QString& map)
{
 // load from disk to QMap
 BosonPlayField* field = boData->playField(map);
// field->preLoadPlayField(map);
 QMap<QString, QByteArray> files;
 if (!field->loadFromDiskToFiles(files)) {
	boError() << k_funcinfo << "playfield loading failed" << endl;
	return false;
 }
 field = 0;

 // load from QMap into memory
 field = new BosonPlayField(0);
 if (!field->loadPlayField(files)) {
	boError() << k_funcinfo << "could not load playfield from files" << endl;
	return false;
 }

 if (!testValid(field)) {
	boError() << k_funcinfo << map << " was not loaded correctly" << endl;
	return false;
 }

 // loading seems to work. now test saving.
 // note that saving to a single file cannot be tested here, since we would need
 // additional data (a complete game) for that. but we can save the playfield to
 // a QMap again.
 QMap<QString, QByteArray> files2;
 if (!field->savePlayFieldToFiles(files2)) {
	boError() << k_funcinfo << "could not save playfield to files" << endl;
	return false;
 }

 // AB: note that files.count() > files2.cont(), as files2 contains the
 // playfield only, whereas files contains the whole contents of the .bpf/.bsg
 // file.
 QMap<QString, QByteArray>::Iterator it;
 for (it = files2.begin(); it != files2.end(); ++it) {
	QString name = it.key();
	if (!files.contains(name)) {
		boError() << k_funcinfo << "file " << name << " is in resulting data, but not in original data" << endl;
		return false;
	}
	QByteArray b2 = *it;
	QByteArray b = files[name];
	if (b != b2) {
		bool differs = true;
		if (name.right(4) == QString(".xml")) {
			differs = compareXML(b, b2);
		} else if (name.right(4) == QString(".png")) {
			differs = comparePNG(b, b2);
		}
		if (differs) {
			// we saved something different than we loaded
			boError() << k_funcinfo << "original data and resulting data differ for file " << name << endl;
			if (name.right(4) == QString(".xml")) {
			    boDebug() << k_funcinfo << "Original:" << endl << QString(b) << endl;
			    boDebug() << k_funcinfo << "Result:" << endl << QString(b2) << endl;
			}
			return false;
		}
	}
 }
 // resulting data matches original data - saving succeeded

 return true;
}

// return TRUE if files differ, otherwise FALSE
static bool compareXML(const QByteArray& b1, const QByteArray& b2)
{
 QDomDocument doc1;
 if (!doc1.setContent(QString(b1))) {
	boError() << k_funcinfo << "could not parse b1" << endl;
	return true;
 }
 QDomDocument doc2;
 if (!doc2.setContent(QString(b2))) {
	boError() << k_funcinfo << "could not parse b2" << endl;
	return true;
 }
 if (doc1.toString() == doc2.toString()) {
	return false;
 }

 // if stings do not match, this can have many reasons. e.g. attributes have
 // different order - but the docs are still equal. so we need to do further
 // testing
 QDomElement root1 = doc1.documentElement();
 QDomElement root2 = doc2.documentElement();
 if (compareElements(root1, root2)) {
	boError() << k_funcinfo << "xml files differ" << endl;
	return true;
 }
 return false;
}

// return TRUE if files differ
static bool compareElements(const QDomElement& r1, const QDomElement& r2)
{
 if (r1.tagName() != r2.tagName()) {
	boError() << k_funcinfo << "tagnames of elements differ: " << r1.tagName() << " != " << r2.tagName() << endl;
	return true;
 }
 QDomNamedNodeMap m1 = r1.attributes();
 QDomNamedNodeMap m2 = r2.attributes();
 if (m1.count() != m2.count()) {
	boError() << k_funcinfo << "element1 has " << m1.count() << " attributes, element2 " << m2.count() << " attributes. tagname=" << r1.tagName() << endl;
	return true;
 }
 for (unsigned int i = 0; i < m1.count(); i++) {
	QString name = m1.item(i).nodeName();
	if (!r1.hasAttribute(name)) {
		boError() << k_funcinfo << "tester is buggy" << endl;
		return true;
	}
	if (!r2.hasAttribute(name)) {
		boError() << k_funcinfo << "element1 has attribute " << name << " but element2 ahs not" << endl;
		return true;
	}
	QString v = r1.attribute(name);
	if (r2.attribute(name) != v) {
		boError() << k_funcinfo << "attribute " << name << " in element1 is " << v << ", in element2 " << r2.attribute(name) << endl;
		return true;
	}
 }


 QDomNode child1 = r1.firstChild();
 QDomNode child2 = r2.firstChild();
 for (; !child1.isNull(); child1 = child1.nextSibling(), child2 = child2.nextSibling()) {
	if (child1.isElement() != child2.isElement()) {
		boError() << k_funcinfo << "isElement flag differs for childs. tagname of parent=" << r1.tagName() << endl;
		return true;
	}
	QDomElement e1 = child1.toElement();
	QDomElement e2 = child2.toElement();
	if (compareElements(e1, e2)) {
		return true;
	}
 }


 return false;
}

// return TRUE if files differ, otherwise FALSE
static bool comparePNG(const QByteArray& b1, const QByteArray& b2)
{
 QImage h1(b1);
 if (h1.isNull()) {
	boError() << k_funcinfo << "b1 is invalid heightmap image" << endl;
	return true;
 }
 QImage h2(b2);
 if (h2.isNull()) {
	boError() << k_funcinfo << "b2 is invalid heightmap image" << endl;
	return true;
 }
 if (!h1.isGrayscale()) {
	boError() << k_funcinfo << "b1 is not grayscale" << endl;
	return true;
 }
 if (!h2.isGrayscale()) {
	boError() << k_funcinfo << "b2 is not grayscale" << endl;
	return true;
 }
 if (h1.width() != h2.width() || h1.height() != h2.height()) {
	boError() << k_funcinfo << "dimensions differ: b1=" << h1.width() << "," << h1.height() << " h2=" << h2.width() << "," << h2.height() << endl;
	return true;
 }
 for (int x = 0; x < h1.width(); x++) {
	for (int y = 0; y < h1.height(); y++) {
		if (h1.pixel(x, y) != h2.pixel(x, y)) {
			boError() << k_funcinfo << "pixels differ at " << x << "," << y << endl;
			boError() << h1.pixel(x, y) << " != " << h2.pixel(x, y) << endl;
			return true;
		}
	}
 }
 return false; // do not differ
}


// note that we do NOT test the scenario!
// -> invalid player counts or invalid units are not recognized
static bool testValid(BosonPlayField* field)
{
 if (!field) {
	return false;
 }
 if (!field->map()) {
	BO_NULL_ERROR(field->map());
	return false;
 }
 if (!field->description()) {
	BO_NULL_ERROR(field->description());
	return false;
 }
 if (!field->information()) {
	BO_NULL_ERROR(field->information());
	return false;
 }

 // AB: the identifier is loaded only when preloading the playfield. not when
 // actually loading a file
#if 0
 if (field->identifier().isEmpty()) {
	boError() << k_funcinfo << "empty map identifier" << endl;
	return false;
 }
#endif
 if (field->playFieldName().isEmpty()) {
	boError() << k_funcinfo << "empty map name" << endl;
	return false;
 }
#if 0
 // AB: isPreLoaded() is true only, when we preload the file, not when we
 // actually load it.
 if  (!field->isPreLoaded()) {
 }
#endif

 const BosonPlayFieldInformation* i = field->information();
 if (i->mapWidth() == 0 || i->mapWidth() > MAX_MAP_WIDTH || i->mapHeight() == 0 || i->mapHeight() > MAX_MAP_HEIGHT) {
	boError() << k_funcinfo << "invalid map dimensions " << i->mapWidth() << "," << i->mapHeight() << endl;
	return false;
 }
 if (i->minPlayers() < 1) {
	boError() << k_funcinfo << "minPlayers==" << i->minPlayers() << " < 1" << endl;
	return false;
 }
 if (i->maxPlayers() < (int)i->minPlayers()) {
	boError() << k_funcinfo << "maxPlayers==" << i->maxPlayers() << " < minPlayers" << endl;
	return false;
 }
 if (i->maxPlayers() > BOSON_MAX_PLAYERS) {
	boError() << k_funcinfo << "maxPlayers > BOSON_MAX_PLAYERS" << endl;
	return false;
 }
 const BosonMap* map = field->map();
 if (i->mapWidth() != map->width() || i->mapHeight() != map->height()) {
	boError() << k_funcinfo << "information object contains wrong data" << endl;
	return false;
 }
 if (!testValidMap(map)) {
	boError() << k_funcinfo << "map not valid" << endl;
	return false;
 }


 // TODO:
 //  - water
 //  - C/description

#warning TODO



 return true;
}

static bool testValidMap(const BosonMap* map)
{
 if (!map) {
	return false;
 }
 if (map->width() <= 0 || map->height() <= 0 || map->width() > MAX_MAP_WIDTH || map->height() > MAX_MAP_HEIGHT) {
	boError() << k_funcinfo << "invalid map dimenstions " << map->width() << "x" << map->height() << endl;
	return false;
 }
 if (!map->cells()) {
	BO_NULL_ERROR(map->cells());
	return false;
 }
 if (!map->heightMap()) {
	BO_NULL_ERROR(map->heightMap());
	return false;
 }
 if (!map->normalMap()) {
	BO_NULL_ERROR(map->normalMap());
	return false;
 }
 if (!map->colorMap()) {
	BO_NULL_ERROR(map->colorMap());
	return false;
 }
 if (!map->groundTheme()) {
	BO_NULL_ERROR(map->groundTHeme());
	return false;
 }
 for (unsigned int x = 0; x < map->width(); x++) {
	for (unsigned int y = 0; y < map->height(); y++) {
		if (!map->isValidCell(x, y)) {
			boError() << k_funcinfo << "not a valid cell" << endl;
			return false;
		}
		Cell* c = map->cell(x, y);
		if (!c) {
			BO_NULL_ERROR(c);
			return false;
		}
		if (c != &map->cells()[map->cellArrayPos(x, y)]) {
			boError() << k_funcinfo << "cell() gives other cells than cells()[cellArrayPos()]" << endl;
			return false;
		}

		if (c->x() != (int)x || c->y() != (int)y) {
			boError() << k_funcinfo << "cell has invalid coordinates" << endl;
			return false;
		}

	}
 }

 return true;
}

