
#include "bosonminimap.h"

#include "cell.h"
#include "defines.h"
#include "unit.h"
#include "player.h"
#include "speciestheme.h"
#include "visualunit.h"
#include "bosonmap.h"
#include "unitproperties.h"

#include <kdebug.h>

#include <qpixmap.h>
#include <qpainter.h>

#include "bosonminimap.moc"

#define COLOR_UNKNOWN black // should never appear

class BosonMiniMapPrivate
{
public:
	BosonMiniMapPrivate()
	{
		mGround = 0;

		mMapWidth = -1;
		mMapHeight = -1;

		mMap = 0;
	}

	QPixmap* mGround;
	int mMapWidth;
	int mMapHeight;

	QSize mSize;
	QPoint mPos;

	BosonMap* mMap;
};

BosonMiniMap::BosonMiniMap(QWidget* parent) : QWidget(parent)
{
 d = new BosonMiniMapPrivate;
 setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
}

BosonMiniMap::~BosonMiniMap()
{
 delete d;
}

int BosonMiniMap::mapWidth() const
{
 return d->mMapWidth;
}

int BosonMiniMap::mapHeight() const
{
 return d->mMapHeight;
}

QPixmap* BosonMiniMap::ground() const
{
 return d->mGround;
}

void BosonMiniMap::slotCreateMap(int w, int h)
{
 if (w == mapWidth() && h == mapHeight()) {
	return;
 }
 if (d->mGround) {
	delete d->mGround;
 }
 d->mMapWidth = w;
 d->mMapHeight = h;
 d->mGround = new QPixmap(mapWidth(), mapHeight());
 d->mGround->fill(black);

 setMinimumWidth(mapWidth() + 5);
 setMinimumHeight(mapHeight() + 5);
 updateGeometry();
}

void BosonMiniMap::slotAddCell(int x, int y, int groundType, unsigned char)
{
 if (x < 0 || x >= mapWidth()) {
	kdError() << "BosonMiniMap::slotAddCell(): invalid cell! x=" << x << endl;
	return;
 }
 if (y < 0 || y >= mapHeight()) {
	kdError() << "BosonMiniMap::slotAddCell(): invalid cell! y=" << y << endl;
	return;
 }
 if (!ground()) {
	kdError() << "BosonMiniMap::slotAddCell(): map not yet created" << endl;
	return;
 }
 switch (groundType) {
	case Cell::GroundWater:
		setPoint(x, y, blue);
		break;
	case Cell::GroundGrass:
	case Cell::GroundGrassOil:
	case Cell::GroundGrassMineral:
		setPoint(x, y, green);
		break;
	case Cell::GroundDesert:
		setPoint(x, y, darkYellow);
		break;
	default:
		setPoint(x, y, black);
		break;
 }
// repaint(false);// performance - units will be added and therefore repaint()
// is already called
}

void BosonMiniMap::setPoint(int x, int y, const QColor& color)
{
 if (!ground()) {
	kdError() << "NULL ground" << endl;
	return;
 }
 QPainter p;
 p.begin(ground());
 p.setPen(color);
 p.drawPoint(x, y);
 p.end();
}

void BosonMiniMap::paintEvent(QPaintEvent*)
{
 if (!ground()) {
	return;
 }
 QPainter p;
 p.begin(this);
 p.drawPixmap(0, 0, *ground());
 
 // the little rectangle 
 p.setPen(white);
 p.setRasterOp(XorROP);
 p.drawRect( QRect( d->mPos, d->mSize) ); 

 p.end();
}

void BosonMiniMap::mousePressEvent(QMouseEvent *e)
{
 if (e->button() & LeftButton) {
	emit signalReCenterView( e->pos() );
	return;
 }
}

void BosonMiniMap::slotAddUnit(VisualUnit* unit, int x, int y)
{
 if (!unit) {
	kdError() << "BosonMiniMap::slotAddUnit(): NULL unit" << endl;
	return;
 }
 Player* owner = unit->owner();
 if (!owner) {
	kdError() << "BosonMiniMap::slotAddUnit(): NULL owner" << endl;
	return;
 }
 SpeciesTheme* theme = owner->speciesTheme();
 QColor color;
 if (!theme) {
	kdError() << "BosonMiniMap::slotAddUnit(): NULL species theme" << endl;
	color = COLOR_UNKNOWN;
 } else {
	color = unit->owner()->speciesTheme()->teamColor();
 }
 if (color == green) { // green on gren ...
	color = darkGreen;
 }
 const UnitProperties* prop = unit->unitProperties();
 if (!prop) {
	kdError() << "NULL unitProperties" << endl;
	setPoint(x, y, color); // let's hope..
	return;
 }
 if (prop->isFacility()) {
	setPoint(x, y, color);
 } else {
	setPoint(x, y, color);
 }
 repaint(false);
}

void BosonMiniMap::slotMoveRect(int x, int y)
{
 d->mPos = QPoint(x / BO_TILE_SIZE, y / BO_TILE_SIZE);
 repaint(false);
}

void BosonMiniMap::slotResizeRect(int w, int h)
{
 d->mSize = QSize(w / BO_TILE_SIZE, h / BO_TILE_SIZE);
 repaint(false);
}

void BosonMiniMap::setMap(BosonMap* map)
{
 d->mMap = map;
}

void BosonMiniMap::initMap()
{
 if (!d->mMap) {
	kdError() << "BosonMiniMap::initMap(): NULL map" << endl;
	return;
 }
 slotCreateMap(d->mMap->width(), d->mMap->height());
 for (int i = 0; i < d->mMap->width(); i++) {
	for (int j = 0; j < d->mMap->height(); j++) {
		Cell* c = d->mMap->cell(i, j);
		if (c) {
			slotAddCell(i, j, c->groundType(), c->version());
		}
	}
 }
}

void BosonMiniMap::slotMoveUnit(VisualUnit* unit, double oldX, double oldY)
{
 if (!d->mMap) {
	kdError() << "BosonMiniMap::slotMoveUnit(): NULL map" << endl;
	return;
 }
 if (!unit) {
	kdError() << "BosonMiniMap::slotMoveUnit(): NULL unit" << endl;
	return;
 }
 int x = (int)(oldX / BO_TILE_SIZE);
 int y = (int)(oldY / BO_TILE_SIZE);
 Cell* c = d->mMap->cell(x, y);
 if (c) {
	slotAddCell(x, y, c->groundType(), c->version()); // FIXME not yet fully working
 } else {
	kdWarning() << "slotMoveUnit(): NULL cell" << endl;
 }
 x = (int)(unit->x() / BO_TILE_SIZE);
 y = (int)(unit->y() / BO_TILE_SIZE);
 slotAddUnit(unit, x, y);
}

void BosonMiniMap::slotUnitDestroyed(VisualUnit* unit)
{
 if (!unit) {
	kdError() << "NULL unit" << endl;
	return;
 }
 int x = (int)(unit->x() / BO_TILE_SIZE);
 int y = (int)(unit->y() / BO_TILE_SIZE);
 Cell* c = d->mMap->cell(x, y);
 if (!c) {
	kdError() << "slotUnitDestroyed(): NULL cell" << endl;
	return;
 }
 slotAddCell(x, y, c->groundType(), c->version());
}

