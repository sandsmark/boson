#ifndef __BOSONCANVAS_H__
#define __BOSONCANVAS_H__

#include <qcanvas.h>

#include <qpixmap.h>

class BosonMap;
class Cell;
class KPlayer;
class Player;
class VisualUnit;

class BosonCanvasPrivate;
class BosonCanvas : public QCanvas
{
	Q_OBJECT
public:
	BosonCanvas(QPixmap p, unsigned int w, unsigned int h);
	BosonCanvas(QObject* parent);
	~BosonCanvas();

	/**
	 * Create the @ref Cell array
	 **/
	void createCells(int w, int y);

	/**
	 * Initialize this @ref Cell.
	 *
	 * All this currently does is to set the tile. See @ref QCanvas::setTile
	 **/
	void initCell(int x, int y);

	/**
	 * @return The unit on this coordinates of the canvas
	 **/
	VisualUnit* findUnitAt(const QPoint& pos);

	bool canGo(VisualUnit* unit, const QRect& rect) const;

	void setMap(BosonMap* map);
	
	/**
	 * @param tileFile currently always "earth.png
	 **/
	void initMap(const QString& tileFile);

	/**
	 * Reimlemented from QCanvas::addAnimation because of @ref advance
	 **/
	virtual void addAnimation(QCanvasItem*);
	/**
	 * Reimlemented from QCanvas::removeAnimation because of @ref advance
	 **/
	virtual void removeAnimation(QCanvasItem*);

	/**
	 * Called by @ref VisualUnit. This informs the canvas about a moved
	 * unit. Should e.g. adjust the destination of units which have this
	 * unit as target.
	 *
	 * Also adjust the mini map - see @ref signalUnitMoved
	 **/
	void unitMoved(VisualUnit* unit, double oldX, double oldY);

	/**
	 * Called by @ref VisualUnit. One unit damages/shoots at another unit.
	 **/
	void shootAtUnit(VisualUnit* target, VisualUnit* damagedBy, long int damage);

public slots:
	/**
	 * The game (@ref Boson) reports that a unit shall be added - lets do
	 * that :-)
	 **/
	void slotAddUnit(VisualUnit* unit, int x, int y);
	void slotLoadTiles(const QString&);
	virtual void advance();
	
signals:
	void signalUnitMoved(VisualUnit* unit, double oldX, double oldY);
	void signalUnitDestroyed(VisualUnit* unit);

protected:
	Cell* cell(int x, int y) const;
	void play(const QString& fileName); // perhaps in public

protected slots:
//	void slotLoadCell(int x, int y, int groundType, unsigned char b);


private:
	void init();

private:
	BosonCanvasPrivate* d;
};

#endif
