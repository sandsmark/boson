#include <bosongroundtheme.h>
#include <bodebug.h>
#include <boapplication.h>
#include <boson.h>
#include <bosonplayfield.h>
#include <bosonmap.h>
#include <bosondata.h>
#include <defines.h>
#include <cell.h>

#include <kcmdlineargs.h>

#include <qstringlist.h>
#include <qdom.h>
#include <qimage.h>


static bool testHeightMap();
static bool testTexMap();
static bool testNormalMap();

int main(int argc, char** argv)
{
 KCmdLineArgs::init(argc, argv, "tester", "tester", "tester", "version xyz");
 BoApplication app;

 if (!testHeightMap()) {
	boError() << k_funcinfo << "heightmap test failed" << endl;
	return 1;
 }

 if (!testTexMap()) {
	boError() << k_funcinfo << "texmap test failed" << endl;
	return 1;
 }
 if (!testNormalMap()) {
	boError() << k_funcinfo << "normalmap test failed" << endl;
	return 1;
 }

 // AB: I think there isn't much left that we could test here.
 // some things are already tested in the playfield test, but even those things
 // are trivial tests, I don't think they are worth the effort.
 // maybe save/load could be tested here, but due to the water patch, these are
 // under active development, so the tests would get obsoleted soon.
 //
 // the rest of the class is mostly an interface to the classes tested above.

 qDebug("testing completed. no bugs found."); // use qDebug(), so that we see msg even if debug area 0 is disabled
 return 0;
}


static bool testHeightMap()
{
 const int w = 10;
 const int h = 10;
 BoHeightMap m(w, h);
 if ((int)m.width() != w || (int)m.height() != h) {
	return false;
 }

 // initially all heights must be 0.0f
 for (int x = 0; x < w; x++) {
	for (int y = 0; y < h; y++) {
		if (m.heightAt(x, y) != 0.0f) {
			boError() << k_funcinfo << "invalid initial value" << endl;
			return false;
		}
	}
 }

 // test fill()
 m.fill(1.0f);
 for (int x = 0; x < w; x++) {
	for (int y = 0; y < h; y++) {
		if (m.heightAt(x, y) != 1.0f) {
			boError() << k_funcinfo << "invalid value after fill()" << endl;
			return false;
		}
	}
 }

 // test arrayPos()
 for (int x = 0; x < w; x++) {
	for (int y = 0; y < h; y++) {
		if (m.arrayPos(x, y) != BoHeightMap::arrayPos(x, y, w)) {
			boError() << k_funcinfo << "arrayPos() buggy" << endl;
			return false;
		}
	}
 }

 float v = 0.0f;
 for (int x = 0; x < w; x++) {
	for (int y = 0; y < h; y++) {
		m.setHeightAt(x, y, v);
		if (m.heightAt(x, y) != v) {
			boError() << k_funcinfo << "setHeightAt didn't work?" << endl;
			boError() << k_funcinfo << m.heightAt(x, y) << " should be " << v << endl;
			return false;
		}
		if (m.heightMap()[m.arrayPos(x, y)] != v) {
			boError() << k_funcinfo << "directly accessing heightmap gave different value" << endl;
			return false;
		}
		v += 0.1f; // AB: note that only values in [-15,15] are valid!
	}
 }

 QByteArray b1;
 QDataStream s1(b1, IO_WriteOnly);
 if (!m.save(s1)) {
	boError() << k_funcinfo << "saving failed" << endl;
	return false;
 }
 BoHeightMap m2(w, h);
 QDataStream s2(b1, IO_ReadOnly);
 if (!m2.load(s2)) {
	boError() << k_funcinfo << "loading failed" << endl;
	return false;
 }
 for (int x = 0; x < w; x++) {
	for (int y = 0; y < h; y++) {
		if (m.heightAt(x, y) != m2.heightAt(x, y)) {
			boError() << k_funcinfo << "loaded map gives different values than saved map" << endl;
			boError() << k_funcinfo << m2.heightAt(x, y) << " != " << m.heightAt(x, y) << " at x,y=" << x << "," << y << endl;
			return false;
		}
	}
 }

 // finally testing all possible pixel values and their height.
 // note that it is important to find out whether there are problems with
 // floating point numbers (i.e. with the precision)
 for (int p = 0; p <= 255; p++) {
	float f = BoHeightMap::pixelToHeight(p);
	int p2 = BoHeightMap::heightToPixel(f);
	if (p != p2) {
		boError() << k_funcinfo << "pixel " << p << " is height " << f << " but converted back to pixel " << p2 << endl;
		return false;
	}
	m.setHeightAt(0, 0, f);
	float f2 = m.heightAt(0, 0);
	int p3 = BoHeightMap::heightToPixel(f2);
	if (p != p3) {
		boError() << k_funcinfo << "heightmap stored height " << f2 << " for height " << f << ", i.e. pixel values stored: " << p3 << " should be: " << p << endl;
		return false;
	}
 }

 return true;
}


static bool testTexMap()
{
 const int w = 10;
 const int h = 10;
 const int textures = 3;
 BoTexMap m(textures, w, h);
 if ((int)m.textureCount() != textures) {
	return false;
 }
 if ((int)m.width() != w) {
	return false;
 }
 if ((int)m.height() != h) {
	return false;
 }

 // AB: we don't have much logic in this class, so I don't think we need to test
 // anything
 // TODO: test save() and load()
 return true;
}

static bool testNormalMap()
{
 const int w = 10;
 const int h = 10;
 BoNormalMap m(w, h);
 if ((int)m.width() != w) {
	return false;
 }
 if ((int)m.height() != h) {
	return false;
 }

 // AB: we don't have much logic in this class, so I don't think we need to test
 // anything
 // TODO: test save() and load()
 return true;
}
